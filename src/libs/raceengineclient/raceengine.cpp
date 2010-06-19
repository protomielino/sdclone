/***************************************************************************

    file        : raceengine.cpp
    created     : Sat Nov 23 09:05:23 CET 2002
    copyright   : (C) 2002 by Eric Espié 
    email       : eric.espie@torcs.org 
    version     : $Id: raceengine.cpp,v 1.19 2007/11/06 20:43:32 torcs Exp $

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/** @file   
    		
    @author	<a href=mailto:eric.espie@torcs.org>Eric Espie</a>
    @version	$Id: raceengine.cpp,v 1.19 2007/11/06 20:43:32 torcs Exp $
*/

#include <cstdlib>
#include <cstdio>

#ifdef ReMultiThreaded
#include <SDL.h>
#include <SDL_thread.h>
#endif

#include <portability.h>
#include <network.h>
#include <tgfclient.h>
#include <robot.h>
#include <raceman.h>
#include <racescreens.h>
#include <robottools.h>

#include "racemain.h"
#include "racegl.h"
#include "raceinit.h"
#include "raceresults.h"
#include "racesimusimu.h"

#include "raceengine.h"


// Small event log system for ReUpdate/ReOneStep dispatching analysis.
#define LogEvents 0

#if (LogEvents)
# define DeclareEventType(name) \
	static int nLast##name = 0; \
	static int nPrev##name = 0; \
	static double tLast##name = 0.0; \
	static double tPrev##name = 0.0;
#else
# define DeclareEventType(name)
#endif

#if (LogEvents)
# define SignalEvent(name) \
	nLast##name++; \
	tLast##name = GfTimeClock();
#else
# define SignalEvent(name)
#endif

#if (LogEvents)
# define PrintEvent(name, header, footer)	 \
	if (header) GfOut(header); \
	if (tPrev##name == 0.0) tPrev##name = tLast##name; \
	GfOut("%s %2d %7.3f ", #name, nLast##name - nPrev##name, (tLast##name - tPrev##name)*1000); \
	nPrev##name = nLast##name; \
	tPrev##name = tLast##name; \
	if (footer) GfOut(footer);
#else
# define PrintEvent(name, header, footer)
#endif

DeclareEventType(Bots);
DeclareEventType(Simu);
DeclareEventType(Graph);


static char	buf[1024];
static char	bestLapChanged = FALSE;
static double	msgDisp;
static double	bigMsgDisp;

tRmInfo	*ReInfo = 0;

tRmInfo* ReGetSituation()
{
	return ReInfo;
}


#ifdef ReMultiThreaded //==========================================================

#include "SDL/SDL.h"
#include "SDL/SDL_thread.h"

class reSituationUpdater
{
public:
	
	//! Constructor.
	reSituationUpdater(tRmInfo* pReInfo);

	//! Destructor.
	~reSituationUpdater();

	//! Stop (pause) the updater if it is running (return its exit status).
	void stop();
	
	//! (Re)start (after a stop) the updater if it is not running.
	void start();
	
	//! Terminate the updater (return its exit status ; wait for the thread to return).
	int terminate();
	
	//! Get the situation for the previous step
	tRmInfo* getPreviousStep();
	
	//! Start computing the situation for the current step
	void computeCurrentStep();

private:

	//! Reserve exclusive access on the race engine data.
	bool lockData(const char* pszLocker);
	
	//! Release exclusive access on the race engine data.
	bool unlockData(const char* pszLocker);
	
	//! Copy (after allocating if pTarget is null) the source situation into the target (deep copy for RW data, shallow copy for RO data).
	tRmInfo* deliverSituation(tRmInfo*& pTarget, const tRmInfo* pSource);
	
	//! Allocate and initialize a situation (set constants from source).
	tRmInfo* initSituation(const tRmInfo* pSource);
	
	//! Free the given situation
	void freeSituation(tRmInfo*& pSituation);
	
	//! The thread function.
	int threadLoop();
	
	//! The C wrapper on the thread function.
	friend int reSituationUpdaterThreadLoop(void *);
	
private:

	//! Initial number of drivers racing
	int _nInitDrivers;

	//! The previous step of the situation
	tRmInfo* _pPrevReInfo;

	//! The current step of the situation
	tRmInfo* _pCurrReInfo;

	//! The mutex to protect the situation data
	SDL_mutex* _pDataMutex;

	//! The situation updater thread
	SDL_Thread* _pUpdateThread;

	//! True if the updater is actually threaded (may be not the case)
	bool _bThreaded;

	//! Flag to set in order to terminate the updater.
	bool _bTerminate;
};

static reSituationUpdater* situationUpdater = 0;
#endif // ReMultiThreaded ========================================================


static void ReRaceRules(tCarElt *car);


/* Compute Pit stop time */
static void
ReUpdtPitTime(tCarElt *car)
{
	tSituation *s = ReInfo->s;
	tReCarInfo *info = &(ReInfo->_reCarInfo[car->index]);
	tCarPenalty *penalty;
	int i;

	switch (car->_pitStopType) {
		case RM_PIT_REPAIR:
			info->totalPitTime = 2.0f + fabs((double)(car->_pitFuel)) / 8.0f + (tdble)(fabs((double)(car->_pitRepair))) * 0.007f;
			car->_scheduledEventTime = s->currentTime + info->totalPitTime;
			ReInfo->_reSimItf.reconfig(car);
			for (i=0; i<4; i++) {
				car->_tyreCondition(i) = 1.01;
				car->_tyreT_in(i) = 50.0;
				car->_tyreT_mid(i) = 50.0;
				car->_tyreT_out(i) = 50.0;
			}
			break;
		case RM_PIT_STOPANDGO:
			penalty = GF_TAILQ_FIRST(&(car->_penaltyList));
			if (penalty && penalty->penalty == RM_PENALTY_10SEC_STOPANDGO)
				info->totalPitTime = 10.0;
			else
				info->totalPitTime = 0.0;
			car->_scheduledEventTime = s->currentTime + info->totalPitTime; 
			break;
	}
}

/* Return from interactive pit information */
static void
ReUpdtPitCmd(void *pvcar)
{
	tCarElt *car = (tCarElt*)pvcar;

	ReUpdtPitTime(car);
	GfuiScreenActivate(ReInfo->_reGameScreen);
}

#ifdef ReMultiThreaded // ==================================================

/* Prepare to open the pit menu when back in the main thread */
static void
rePreparePit(tCarElt *car)
{
	ReInfo->_reInPitMenuCar = car;
	ReInfo->_reRunning = 0;
}


static void
reRaceMsgUpdate(tRmInfo* pReInfo)
{
	if (pReInfo->_reMessage)
	{
		ReSetRaceMsg(pReInfo->_reMessage);
		//free(pReInfo->_reMessage);
		//pReInfo->_reMessage = 0;
	}		
	else //if (pReInfo->_reCurTime > pReInfo->_reMessageEnd)
		ReSetRaceMsg(0);
	
	if (pReInfo->_reBigMessage)
	{
		ReSetRaceMsg(pReInfo->_reBigMessage);
		//free(pReInfo->_reBigMessage);
		//pReInfo->_reBigMessage = 0;
	}		
	else //if (pReInfo->_reCurTime > pReInfo->_reBigMessageEnd)
		ReSetRaceBigMsg(0);
}

static void
ReRaceMsgUpdate()
{
	reRaceMsgUpdate(ReInfo);
}

static void
reRaceMsgManage(tRmInfo* pReInfo)
{
	if (pReInfo->_reMessage && pReInfo->_reCurTime > pReInfo->_reMessageEnd)
	{
		free(pReInfo->_reMessage);
		pReInfo->_reMessage = 0;
	}
	
	if (pReInfo->_reBigMessage && pReInfo->_reCurTime > pReInfo->_reBigMessageEnd)
	{
		free(pReInfo->_reBigMessage);
		pReInfo->_reBigMessage = 0;
	}
}

static void
reRaceMsgSet(tRmInfo* pReInfo, const char *msg, double life)
{
    if (pReInfo->_reMessage)
		free(pReInfo->_reMessage);
    if (msg)
		pReInfo->_reMessage = strdup(msg);
    else
		pReInfo->_reMessage = 0;
	pReInfo->_reMessageEnd = pReInfo->_reCurTime + life;
}

static void
ReRaceMsgSet(const char *msg, double life)
{
	reRaceMsgSet(ReInfo, msg, life);
}

static void
reRaceBigMsgSet(tRmInfo* pReInfo, const char *msg, double life)
{
    if (pReInfo->_reBigMessage)
		free(pReInfo->_reBigMessage);
    if (msg)
		pReInfo->_reBigMessage = strdup(msg);
    else
		pReInfo->_reBigMessage = 0;
	pReInfo->_reBigMessageEnd = pReInfo->_reCurTime + life;
}

static void
ReRaceBigMsgSet(const char *msg, double life)
{
	reRaceBigMsgSet(ReInfo, msg, life);
}

#else // ReMultiThreaded ==================================================

static void
ReRaceMsgUpdate(void)
{
	if (ReInfo->_reCurTime > msgDisp) {
		ReSetRaceMsg("");
	}
	if (ReInfo->_reCurTime > bigMsgDisp) {
		ReSetRaceBigMsg("");
	}
}

static void
ReRaceMsgSet(const char *msg, double life)
{
	ReSetRaceMsg(msg);
	msgDisp = ReInfo->_reCurTime + life;
}


static void
ReRaceBigMsgSet(const char *msg, double life)
{
	ReSetRaceBigMsg(msg);
	bigMsgDisp = ReInfo->_reCurTime + life;
}

#endif // ReMultiThreaded

static void
ReAddPenalty(tCarElt *car, int penalty)
{
	tCarPenalty *newPenalty;

	if (! (ReInfo->s->_features & RM_FEATURE_PENALTIES) )
		return;	/* Penalties not enabled this race: do not add penalty */
	
	if (penalty == RM_PENALTY_DRIVETHROUGH)
		sprintf(buf, "%s DRIVETHROUGH PENALTY", car->_name);
	else if (penalty == RM_PENALTY_STOPANDGO)
		sprintf(buf, "%s STOP&GO PENALTY", car->_name);
	else if (penalty == RM_PENALTY_10SEC_STOPANDGO)
		sprintf(buf, "%s 10 SEC STOP&GO PENALTY", car->_name);
	else if (penalty == RM_PENALTY_DISQUALIFIED)
		sprintf(buf, "%s DISQUALIFIED", car->_name);

	ReRaceMsgSet(buf, 5);

	/* If disqualified, remove the car from the track */
	if (penalty == RM_PENALTY_DISQUALIFIED)
	{
		car->_state |= RM_CAR_STATE_ELIMINATED;
		return;
	}

	newPenalty = (tCarPenalty*)calloc(1, sizeof(tCarPenalty));
	newPenalty->penalty = penalty;
	newPenalty->lapToClear = car->_laps + 5;
	GF_TAILQ_INSERT_TAIL(&(car->_penaltyList), newPenalty, link);
}

static void
ReManage(tCarElt *car)
{
	int i, pitok;
	int xx;
	tTrackSeg *sseg;
	tdble wseg;
	static float color[] = {0.0, 0.0, 1.0, 1.0};
	tSituation *s = ReInfo->s;
	
	tReCarInfo *info = &(ReInfo->_reCarInfo[car->index]);
	
	if (car->_speed_x > car->_topSpeed) {
		car->_topSpeed = car->_speed_x;
	}

	// For practice and qualif.
	if (car->_speed_x > info->topSpd) {
		info->topSpd = car->_speed_x;
	}
	if (car->_speed_x < info->botSpd) {
		info->botSpd = car->_speed_x;
	}
	
	// Pitstop.
	if (car->_pit) {
		if (car->ctrl.raceCmd & RM_CMD_PIT_ASKED) {
			// Pit already occupied?
			if (car->_pit->pitCarIndex == TR_PIT_STATE_FREE) {
				sprintf(car->ctrl.msg[2], "Can Pit");
			} else {
				sprintf(car->ctrl.msg[2], "Pit Occupied");
			}
			memcpy(car->ctrl.msgColor, color, sizeof(car->ctrl.msgColor));
		}
		
		if (car->_state & RM_CAR_STATE_PIT) {
			car->ctrl.raceCmd &= ~RM_CMD_PIT_ASKED; // clear the flag.
			if (car->_scheduledEventTime < s->currentTime) {
				car->_state &= ~RM_CAR_STATE_PIT;
				car->_pit->pitCarIndex = TR_PIT_STATE_FREE;
				sprintf(buf, "%s pit stop %.1fs", car->_name, info->totalPitTime);
				ReRaceMsgSet(buf, 5);
			} else {
				sprintf(car->ctrl.msg[2], "in pits %.1fs", s->currentTime - info->startPitTime);
			}
		} else if ((car->ctrl.raceCmd & RM_CMD_PIT_ASKED) &&
					car->_pit->pitCarIndex == TR_PIT_STATE_FREE &&	
				   (s->_maxDammage == 0 || car->_dammage <= s->_maxDammage))
		{
			tdble lgFromStart = car->_trkPos.seg->lgfromstart;
			
			switch (car->_trkPos.seg->type) {
				case TR_STR:
					lgFromStart += car->_trkPos.toStart;
					break;
				default:
					lgFromStart += car->_trkPos.toStart * car->_trkPos.seg->radius;
					break;
			}
		
			if ((lgFromStart > car->_pit->lmin) && (lgFromStart < car->_pit->lmax)) {
				pitok = 0;
				int side;
				tdble toBorder;
				if (ReInfo->track->pits.side == TR_RGT) {
					side = TR_SIDE_RGT;
					toBorder = car->_trkPos.toRight;
				} else {
					side = TR_SIDE_LFT;
					toBorder = car->_trkPos.toLeft;
				}
				
				sseg = car->_trkPos.seg->side[side];
				wseg = RtTrackGetWidth(sseg, car->_trkPos.toStart);
				if (sseg->side[side]) {
					sseg = sseg->side[side];
					wseg += RtTrackGetWidth(sseg, car->_trkPos.toStart);
				}
				if (((toBorder + wseg) < (ReInfo->track->pits.width - car->_dimension_y / 2.0)) &&
					(fabs(car->_speed_x) < 1.0) &&
					(fabs(car->_speed_y) < 1.0))
				{
					pitok = 1;
				}
				
				if (pitok) {
					car->_state |= RM_CAR_STATE_PIT;
					car->_nbPitStops++;
					for (i = 0; i < car->_pit->freeCarIndex; i++) {
						if (car->_pit->car[i] == car) {
							car->_pit->pitCarIndex = i;
							break;
						}
					}
					info->startPitTime = s->currentTime;
					sprintf(buf, "%s in pits", car->_name);
					ReRaceMsgSet(buf, 5);
					if (car->robot->rbPitCmd(car->robot->index, car, s) == ROB_PIT_MENU) {
						// the pit cmd is modified by menu.
#ifdef ReMultiThreaded
						rePreparePit(car);
#else
						// Question: And what if 2 (human) drivers want to pit at the same time ?
						// Answer: IIUC, only one will get the menu ; as for the other ... ?
						ReStop();
						RmPitMenuStart(s, car, (void*)car, ReUpdtPitCmd);
#endif
					} else {
						ReUpdtPitTime(car);
					}
				}
			}
		}
	}

	/* Check if it is in a new sector */
	while (true)
	{
		if (car->_currentSector < ReInfo->track->numberOfSectors - 1 && car->_laps > 0 && info->lapFlag == 0)
		{
			/* Must pass at least one sector before the finish */
			if (RtGetDistFromStart(car) > ReInfo->track->sectors[car->_currentSector])
			{
				/* It is in a new sector. Update split time */
				car->_curSplitTime[car->_currentSector] = car->_curLapTime;
				++car->_currentSector;
				continue;
			}
		}
		break;
	}
	
	/* Start Line Crossing */
	if (info->prevTrkPos.seg != car->_trkPos.seg) {
	if ((info->prevTrkPos.seg->raceInfo & TR_LAST) && (car->_trkPos.seg->raceInfo & TR_START)) {
		if (info->lapFlag == 0) {
		if ((car->_state & RM_CAR_STATE_FINISH) == 0) {
			car->_laps++;

			if (GetNetwork())
				GetNetwork()->SendLapStatusPacket(car);

			car->_remainingLaps--;
			if (car->_pos == 1 && s->currentTime < s->_totTime && s->_raceType == RM_TYPE_RACE)
			{
				/* First car passed finish time before the time ends: increase the number of laps for everyone */
				for (xx = 0; xx < s->_ncars; ++xx)
					++ReInfo->s->cars[xx]->_remainingLaps;
				++s->_totLaps;
			}
			car->_currentSector = 0;
			if (car->_laps > 1) {
			car->_lastLapTime = s->currentTime - info->sTime;
			car->_curTime += car->_lastLapTime;
			if (car->_bestLapTime != 0) {
				car->_deltaBestLapTime = car->_lastLapTime - car->_bestLapTime;
			}
			if ((car->_lastLapTime < car->_bestLapTime) || (car->_bestLapTime == 0)) {
				car->_bestLapTime = car->_lastLapTime;
				memcpy(car->_bestSplitTime, car->_curSplitTime, sizeof(double)*(ReInfo->track->numberOfSectors - 1) );
				if (s->_raceType != RM_TYPE_RACE && s->_ncars > 1)
				{
					/* Best lap time is made better. Update times behind leader */
					bestLapChanged = TRUE;
					car->_timeBehindLeader = car->_bestLapTime - s->cars[0]->_bestLapTime;
					if (car->_pos > 1)
					{
						car->_timeBehindPrev = car->_bestLapTime - s->cars[car->_pos - 1]->_bestLapTime;
					}
					else
					{
						/* New best time for the leader: update the differences */
						for (xx = 1; xx < s->_ncars; ++xx)
						{
							if (s->cars[xx]->_bestLapTime > 0.0f)
								s->cars[xx]->_timeBehindLeader = s->cars[xx]->_bestLapTime - car->_bestLapTime;
						}
					}
					if (car->_pos + 1 < s->_ncars && s->cars[car->_pos+1]->_bestLapTime > 0.0f)
						car->_timeBeforeNext = s->cars[car->_pos + 1]->_bestLapTime - car->_bestLapTime;
					else
						car->_timeBeforeNext = 0;
				}
			}
			if (car->_pos != 1 && s->_raceType == RM_TYPE_RACE) {
				car->_timeBehindLeader = car->_curTime - s->cars[0]->_curTime;
				car->_lapsBehindLeader = s->cars[0]->_laps - car->_laps;
				car->_timeBehindPrev = car->_curTime - s->cars[car->_pos - 2]->_curTime;
				s->cars[car->_pos - 2]->_timeBeforeNext = car->_timeBehindPrev;
			} else if (s->_raceType == RM_TYPE_RACE) {
				car->_timeBehindLeader = 0;
				car->_lapsBehindLeader = 0;
				car->_timeBehindPrev = 0;
			}
			info->sTime = s->currentTime;
			switch (ReInfo->s->_raceType) {
			case RM_TYPE_PRACTICE:
				if (ReInfo->_displayMode == RM_DISP_MODE_NONE && s->_ncars <= 1) {
					ReInfo->_refreshDisplay = 1;
					char *t1, *t2;
					t1 = GfTime2Str(car->_lastLapTime, 0);
					t2 = GfTime2Str(car->_bestLapTime, 0);
					sprintf(buf,"lap: %02d   time: %s  best: %s  top spd: %.2f    min spd: %.2f    damage: %d",
						car->_laps - 1, t1, t2,
						info->topSpd * 3.6, info->botSpd * 3.6, car->_dammage);
					ReResScreenAddText(buf);
					free(t1);
					free(t2);
				}
				/* save the lap result */
				if (s->_ncars == 1)
					ReSavePracticeLap(car);
				break;
				
			case RM_TYPE_QUALIF:
				if (ReInfo->_displayMode == RM_DISP_MODE_NONE && s->_ncars <= 1) {
					ReUpdateQualifCurRes(car);
				}
				break;
			case RM_TYPE_RACE:
				if (ReInfo->_displayMode == RM_DISP_MODE_NONE)
					ReUpdateRaceCurRes();
				break;
			}
		} else {
			if (ReInfo->_displayMode == RM_DISP_MODE_NONE)
			{
				switch(s->_raceType)
				{
				case RM_TYPE_PRACTICE:
					ReUpdatePracticeCurRes(car);
					break;
				case RM_TYPE_QUALIF:
					ReUpdateQualifCurRes(car);
					break;
				case RM_TYPE_RACE:
					ReUpdateRaceCurRes();
					break;
				default:
					break;
				}
			}
		}	
	
			info->topSpd = car->_speed_x;
			info->botSpd = car->_speed_x;
			if ((car->_remainingLaps < 0 && s->currentTime > s->_totTime) || (s->_raceState == RM_RACE_FINISHING)) {
			car->_state |= RM_CAR_STATE_FINISH;
			s->_raceState = RM_RACE_FINISHING;
			if (ReInfo->s->_raceType == RM_TYPE_RACE) {
				if (car->_pos == 1) {
				sprintf(buf, "Winner %s", car->_name);
				ReRaceBigMsgSet(buf, 10);
				if (GetServer())
					{
					GetServer()->SetFinishTime(s->currentTime+FINISHDELAY);
					}
				} else {
				const char *numSuffix = "th";
				if (abs(12 - car->_pos) > 1) { /* leave suffix as 'th' for 11 to 13 */
					switch (car->_pos % 10) {
					case 1:
					numSuffix = "st";
					break;
					case 2:
					numSuffix = "nd";
					break;
					case 3:
					numSuffix = "rd";
					break;
					default:
					break;
					}
				}
				sprintf(buf, "%s finished %d%s", car->_name, car->_pos, numSuffix);
				ReRaceMsgSet(buf, 5);
				}
			}
			}
		} else {
			/* prevent infinite looping of cars around track, allow one lap after finish for the first car */
			for (i = 0; i < s->_ncars; i++) {
				s->cars[i]->_state |= RM_CAR_STATE_FINISH;
			}
			return;
		}
		} else {
		info->lapFlag--;
		}
	}
	if ((info->prevTrkPos.seg->raceInfo & TR_START) && (car->_trkPos.seg->raceInfo & TR_LAST)) {
		/* going backward through the start line */
		info->lapFlag++;
	}
	}
	ReRaceRules(car);
	
	info->prevTrkPos = car->_trkPos;
	car->_curLapTime = s->currentTime - info->sTime;
	car->_distFromStartLine = car->_trkPos.seg->lgfromstart +
	(car->_trkPos.seg->type == TR_STR ? car->_trkPos.toStart : car->_trkPos.toStart * car->_trkPos.seg->radius);
	car->_distRaced = (car->_laps - 1) * ReInfo->track->length + car->_distFromStartLine;
}

static void 
ReSortCars(void)
{
    int		i,j;
    int		xx;
    tCarElt	*car;
    int		allfinish;
    tSituation	*s = ReInfo->s;

    if ((s->cars[0]->_state & RM_CAR_STATE_FINISH) == 0) {
	allfinish = 0;
    } else {
	allfinish = 1;
    }
    
    for (i = 1; i < s->_ncars; i++) {
	j = i;
	while (j > 0) {
	    if ((s->cars[j]->_state & RM_CAR_STATE_FINISH) == 0) {
		allfinish = 0;
		if ((ReInfo->s->_raceType == RM_TYPE_RACE && s->cars[j]->_distRaced > s->cars[j-1]->_distRaced) ||
		    (ReInfo->s->_raceType != RM_TYPE_RACE && s->cars[j]->_bestLapTime > 0.0f && ( s->cars[j]->_bestLapTime < s->cars[j-1]->_bestLapTime ||
		                                                                                  s->cars[j-1]->_bestLapTime <= 0.0f))) {
		    car = s->cars[j];
		    s->cars[j] = s->cars[j-1];
		    s->cars[j-1] = car;
		    s->cars[j]->_pos = j+1;
		    s->cars[j-1]->_pos = j;
		    if (s->_raceType != RM_TYPE_RACE)
		    {
		    	if (j-1 > 0)
			{
			    s->cars[j-1]->_timeBehindPrev = s->cars[j-1]->_bestLapTime - s->cars[j-2]->_bestLapTime;
			}
			else
			{
			    s->cars[j-1]->_timeBehindPrev = 0;
			    for (xx = 1; xx < s->_ncars; ++xx)
			    {
			    	/* New leader */
				if (s->cars[xx]->_bestLapTime > 0.0f)
			    	    s->cars[xx]->_timeBehindLeader = s->cars[xx]->_bestLapTime - s->cars[0]->_bestLapTime;
			    }
			}
			if (s->cars[j]->_bestLapTime)
			    s->cars[j-1]->_timeBeforeNext = s->cars[j-1]->_bestLapTime - s->cars[j]->_bestLapTime;
			else
			    s->cars[j-1]->_timeBeforeNext = 0;
			s->cars[j]->_timeBehindPrev = s->cars[j]->_bestLapTime - s->cars[j-1]->_bestLapTime;
			if (j+1 < s->_ncars && s->cars[j+1]->_bestLapTime > 0.0f)
			    s->cars[j]->_timeBeforeNext = s->cars[j]->_bestLapTime - s->cars[j+1]->_bestLapTime;
			else
			    s->cars[j]->_timeBeforeNext = 0;
		    }
		    j--;
		    continue;
		}
	    }
	    j = 0;
	}
    }
    if (allfinish) {
	ReInfo->s->_raceState = RM_RACE_ENDED;
    }
}

/* Compute the race rules and penalties */
static void
ReRaceRules(tCarElt *car)
{
    tCarPenalty		*penalty;
    tTrack		*track = ReInfo->track;
    tRmCarRules		*rules = &(ReInfo->rules[car->index]);
    tTrackSeg		*seg = RtTrackGetSeg(&(car->_trkPos));
    tReCarInfo		*info = &(ReInfo->_reCarInfo[car->index]);
    tTrackSeg		*prevSeg = RtTrackGetSeg(&(info->prevTrkPos));
    static float	color[] = {0.0, 0.0, 1.0, 1.0};

	// DNF cars which need too much time for the current lap, this is mainly to avoid
	// that a "hanging" driver can stop the quali from finishing.
	// Allowed time is longest pitstop possible + time for tracklength with speed??? (currently fixed 10 [m/s]).
	// for simplicity. Human driver is an exception to this rule, to allow explorers
	// to enjoy the landscape.
	// Also - don't remove cars that are currently being repaired in pits
	// TODO: Make it configurable.
	if ((car->_curLapTime > 84.5 + ReInfo->track->length/10.0) &&
	    !(car->_state & RM_CAR_STATE_PIT) &&
	    (car->_driverType != RM_DRV_HUMAN))
	{
		car->_state |= RM_CAR_STATE_ELIMINATED;
	    return;
	}

	if (car->_skillLevel < 3) {
	/* only for the pros */
	return;
    }

	penalty = GF_TAILQ_FIRST(&(car->_penaltyList));
    if (penalty) {
	if (car->_laps > penalty->lapToClear) {
	    /* too late to clear the penalty, out of race */
	    ReAddPenalty(car, RM_PENALTY_DISQUALIFIED);
	    return;
	}
	switch (penalty->penalty) {
	case RM_PENALTY_DRIVETHROUGH:
	    sprintf(car->ctrl.msg[3], "Drive Through Penalty");
	    break;
	case RM_PENALTY_STOPANDGO:
	    sprintf(car->ctrl.msg[3], "Stop And Go Penalty");
	    break;
	case RM_PENALTY_10SEC_STOPANDGO:
	    sprintf(car->ctrl.msg[3], "10 Sec Stop And Go Penalty");
	    break;
	default:
	    *(car->ctrl.msg[3]) = 0;
	    break;
	}
	memcpy(car->ctrl.msgColor, color, sizeof(car->ctrl.msgColor));
    }
    

    if (prevSeg->raceInfo & TR_PITSTART) {
	/* just entered the pit lane */
	if (seg->raceInfo & TR_PIT) {
	    /* may be a penalty can be cleaned up */
	    if (penalty) {
		switch (penalty->penalty) {
		case RM_PENALTY_DRIVETHROUGH:
		    sprintf(buf, "%s DRIVE THROUGH PENALTY CLEANING", car->_name);
		    ReRaceMsgSet(buf, 5);
		    rules->ruleState |= RM_PNST_DRIVETHROUGH;
		    break;
		case RM_PENALTY_STOPANDGO:
		case RM_PENALTY_10SEC_STOPANDGO:
		    sprintf(buf, "%s STOP&GO PENALTY CLEANING", car->_name);
		    ReRaceMsgSet(buf, 5);
		    rules->ruleState |= RM_PNST_STOPANDGO;
		    break;
		}
	    }
	}
    } else if (prevSeg->raceInfo & TR_PIT) {
	if (seg->raceInfo & TR_PIT) {
	    /* the car stopped in pits */
	    if (car->_state & RM_CAR_STATE_PIT) {
		if (rules->ruleState & RM_PNST_DRIVETHROUGH) {
		    /* it's not more a drive through */
		    rules->ruleState &= ~RM_PNST_DRIVETHROUGH;
		} else if (rules->ruleState & RM_PNST_STOPANDGO) {
		    rules->ruleState |= RM_PNST_STOPANDGO_OK;
		}
	    } else {
                if(rules->ruleState & RM_PNST_STOPANDGO_OK && car->_pitStopType != RM_PIT_STOPANDGO) {
		    rules->ruleState &= ~ ( RM_PNST_STOPANDGO | RM_PNST_STOPANDGO_OK );
		}
	    }
	} else if (seg->raceInfo & TR_PITEND) {
	    /* went out of the pit lane, check if the current penalty is cleared */
	    if (rules->ruleState & (RM_PNST_DRIVETHROUGH | RM_PNST_STOPANDGO_OK)) {
		/* clear the penalty */
		sprintf(buf, "%s penalty cleared", car->_name);
		ReRaceMsgSet(buf, 5);
		penalty = GF_TAILQ_FIRST(&(car->_penaltyList));
		GF_TAILQ_REMOVE(&(car->_penaltyList), penalty, link);
		FREEZ(penalty);
	    }
	    rules->ruleState = 0;
	} else {
	    /* went out of the pit lane illegally... */
	    /* it's a new stop and go... */
	    if (!(rules->ruleState & RM_PNST_STNGO)) {
	    	ReAddPenalty(car, RM_PENALTY_STOPANDGO);
		rules->ruleState = RM_PNST_STNGO;
	    }
	}
    } else if (seg->raceInfo & TR_PITEND) {
	rules->ruleState = 0;
    } else if (seg->raceInfo & TR_PIT) {
	/* entrered the pits not from the pit entry... */
	/* it's a new stop and go... */
	if (!(rules->ruleState & RM_PNST_STNGO)) {
	    ReAddPenalty(car, RM_PENALTY_STOPANDGO);
	    rules->ruleState = RM_PNST_STNGO;
	}
    }

    if (seg->raceInfo & TR_SPEEDLIMIT) {
	if (!(rules->ruleState & (RM_PNST_SPD | RM_PNST_STNGO)) && (car->_speed_x > track->pits.speedLimit)) {
	    rules->ruleState |= RM_PNST_SPD;
	    ReAddPenalty(car, RM_PENALTY_DRIVETHROUGH);
	}
    }


}

static void
SetNetworkCarPhysics(double timeDelta,CarControlsData *pCt)
{
	tDynPt *pDynCG = NULL;
	pDynCG = ReInfo->_reSimItf.getsimcartable(pCt->startRank);

	double errX = pDynCG->pos.x-pCt->DynGCg.pos.x;
	double errY = pDynCG->pos.y-pCt->DynGCg.pos.y;
	double errZ = pDynCG->pos.z-pCt->DynGCg.pos.z;

	int idx = GetNetwork()->GetCarIndex(pCt->startRank,ReInfo->s);
	
	//Car controls (steering,gas,brake, gear
	tCarElt *pCar = ReInfo->s->cars[idx];
	pCar->ctrl.accelCmd = pCt->throttle;
	pCar->ctrl.brakeCmd = pCt->brake;
	pCar->ctrl.clutchCmd = pCt->clutch;
	pCar->ctrl.gear = pCt->gear;
	pCar->ctrl.steer = pCt->steering;

	pDynCG->pos = pCt->DynGCg.pos;
	pDynCG->acc = pCt->DynGCg.acc;
	pDynCG->vel = pCt->DynGCg.vel;

	double step = 0.0;
	if (timeDelta>0.0)
	{
		//predict car position
		while(timeDelta>0.0)
		{
			if (timeDelta>RCM_MAX_DT_SIMU)
			{
				step = RCM_MAX_DT_SIMU;
			}
			else
				step = timeDelta;

			timeDelta-=step;
			ReInfo->_reSimItf.singleupdate(pCt->startRank,step,ReInfo->s);
		}
	}

	//printf("Network position error is %lf %lf %lf and delta is %lf\n",errX,errY,errZ,timeDelta);

	//Car physics
//	ReInfo->_reSimItf.updatesimcartable(pCt->DynGCg,pCt->startRank);

}

static void
SetNetworkCarStatus(CarStatus *pStatus)
{
	int idx = GetNetwork()->GetCarIndex(pStatus->startRank,ReInfo->s);

	tCarElt *pCar = ReInfo->s->cars[idx];

	if (pStatus->dammage > 0.0)
		pCar->priv.dammage = pStatus->dammage;
	if (pStatus->fuel >0.0)
		pCar->priv.fuel = pStatus->fuel;
	if (pStatus->topSpeed >0.0)
		pCar->race.topSpeed = pStatus->topSpeed;

	pCar->pub.state = pStatus->state;
	

}

static void
SetNetworkLapStatus(LapStatus *pStatus)
{
	int idx = GetNetwork()->GetCarIndex(pStatus->startRank,ReInfo->s);

	tCarElt *pCar = ReInfo->s->cars[idx];
	pCar->race.bestLapTime = pStatus->bestLapTime;
	*pCar->race.bestSplitTime = (double)pStatus->bestSplitTime;
	pCar->race.laps = pStatus->laps;
	GfOut("Setting network lap status\n");
}

static void
NetworkPlayStep()
{
	tSituation *s = ReInfo->s;

	//Do network updates if needed
	//CarControlsData *pControls = NULL;
	int numCars = 0;
	double time = 0.0f;
	
	MutexData *pNData = GetNetwork()->LockNetworkData();

	numCars = pNData->m_vecCarCtrls.size();
	if (numCars>0)
	{
		for (int i=0;i<numCars;i++)
		{
			double timeDelta = s->currentTime-pNData->m_vecCarCtrls[i].time;
			if (timeDelta >= 0)
			{
				SetNetworkCarPhysics(timeDelta,&pNData->m_vecCarCtrls[i]);
			}
			else if (timeDelta <= -1.0)
			{
				GfOut("Ignoring physics packet (delta is %lf)\n", timeDelta);
			}
		}
	}

	GetNetwork()->SetCurrentTime(s->currentTime);
	pNData->m_vecCarCtrls.clear();

	//do car status updates if needed
	CarStatus *pStatus = NULL;
	numCars = pNData->m_vecCarStatus.size();
	time = 0.0f;

	if (numCars>0)
	{
		for (int i=0;i<numCars;i++)
		{
			double delta = s->currentTime-pNData->m_vecCarStatus[i].time;
			if (delta>=0)
				SetNetworkCarStatus(&pNData->m_vecCarStatus[i]);
		}
	}

	std::vector<CarControlsData>::iterator p = pNData->m_vecCarCtrls.begin();
	while(p!=pNData->m_vecCarCtrls.end())
	{
		if(p->time<s->currentTime)
			p = pNData->m_vecCarCtrls.erase(p);
		else 
			p++;
	}

	//do lap status updates if needed
	LapStatus *pLapStatus = NULL;
	numCars = 0;
	time = 0.0f;
	numCars = pNData->m_vecLapStatus.size();
	if (numCars>0)
	{
		for (int i=0;i<numCars;i++)
		{
			SetNetworkLapStatus(&pNData->m_vecLapStatus[i]);
		}
	}

	pNData->m_vecLapStatus.clear();

	GetNetwork()->UnlockNetworkData();
}

static void
ReOneStep(double deltaTimeIncrement)
{
	int i;
	tRobotItf *robot;
	tSituation *s = ReInfo->s;

#ifdef ReMultiThreaded
	// Race messages life cycle management.
	reRaceMsgManage(ReInfo);
#endif
	
	if (GetNetwork())
	{
		// Resync clock in case computer falls behind
		if (s->currentTime < 0.0)
		{
			s->currentTime = GfTimeClock() - GetNetwork()->GetRaceStartTime();
		}

		if (s->currentTime < -2.0)
		{
			sprintf(buf,"Race will start in %i seconds", -s->currentTime);
			ReRaceBigMsgSet(buf, 1.0);
		}
	}

	if (floor(s->currentTime) == -2.0) {
		ReRaceBigMsgSet("Ready", 1.0);
	} else if (floor(s->currentTime) == -1.0) {
		ReRaceBigMsgSet("Set", 1.0);
	} else if (floor(s->currentTime) == 0.0) {
		ReRaceBigMsgSet("Go", 1.0);
		if (s->currentTime==0.0)
			GfOut("Race start time is %lf\n", GfTimeClock());
	}

	ReInfo->_reCurTime += deltaTimeIncrement * ReInfo->_reTimeMult; /* "Real" time */
	s->currentTime += deltaTimeIncrement; /* Simulated time */


	if (s->currentTime < 0) {
		/* no simu yet */
		ReInfo->s->_raceState = RM_RACE_PRESTART;
	} else if (ReInfo->s->_raceState == RM_RACE_PRESTART) {
		ReInfo->s->_raceState = RM_RACE_RUNNING;
		s->currentTime = 0.0; /* resynchronize */
		ReInfo->_reLastTime = 0.0;
	}

	START_PROFILE("rbDrive*");
	if ((s->currentTime - ReInfo->_reLastTime) >= RCM_MAX_DT_ROBOTS) {
		SignalEvent(Bots);
		s->deltaTime = s->currentTime - ReInfo->_reLastTime;
		for (i = 0; i < s->_ncars; i++) {
			if ((s->cars[i]->_state & RM_CAR_STATE_NO_SIMU) == 0) {
				robot = s->cars[i]->robot;
				robot->rbDrive(robot->index, s->cars[i], s);
			}
			else if (! (s->cars[i]->_state & RM_CAR_STATE_ENDRACE_CALLED ) && ( s->cars[i]->_state & RM_CAR_STATE_OUT ) == RM_CAR_STATE_OUT )
			{ // No simu, look if it is out
				robot = s->cars[i]->robot;
				if (robot->rbEndRace)
					robot->rbEndRace(robot->index, s->cars[i], s);
				s->cars[i]->_state |= RM_CAR_STATE_ENDRACE_CALLED;
			}
		}
		ReInfo->_reLastTime = s->currentTime;
	}
	STOP_PROFILE("rbDrive*");


	if (GetNetwork())
	{
		NetworkPlayStep();
	}

	SignalEvent(Simu);
	START_PROFILE("_reSimItf.update*");
	ReInfo->_reSimItf.update(s, deltaTimeIncrement, -1);
	for (i = 0; i < s->_ncars; i++) {
		ReManage(s->cars[i]);
	}
	STOP_PROFILE("_reSimItf.update*");
	
	ReSortCars();

	/* Update screens if a best lap changed */
	if (ReInfo->_displayMode == RM_DISP_MODE_NONE && s->_ncars > 1 && bestLapChanged)
	{
		if (ReInfo->s->_raceType == RM_TYPE_PRACTICE)
			ReUpdatePracticeCurRes(ReInfo->s->cars[0]);
		else if (ReInfo->s->_raceType == RM_TYPE_QUALIF)
			ReUpdateQualifCurRes(ReInfo->s->cars[0]);
	}
	bestLapChanged = FALSE;
}

#ifdef ReMultiThreaded

void ReInitUpdater()
{
	ReInfo->_reRunning = 0;
 	if (!situationUpdater)
 		situationUpdater = new reSituationUpdater(ReInfo);

	GfssConfigureEventLog("graphics", 10000, 0.0);
}

void ReInitCarGraphics(void)
{
	tRmInfo* pPrevReInfo = situationUpdater->getPreviousStep();
	pPrevReInfo->_reGraphicItf.initcars(pPrevReInfo->s);
}

#endif // ReMultiThreaded

void
ReStart(void)
{
#ifdef ReMultiThreaded
	GfssBeginSession();
	situationUpdater->start();
#else // ReMultiThreaded
    ReInfo->_reRunning = 1;
    ReInfo->_reCurTime = GfTimeClock() - RCM_MAX_DT_SIMU;
#endif // ReMultiThreaded
}

void
ReStop(void)
{
#ifdef ReMultiThreaded
	situationUpdater->stop();
	GfssEndSession();
#else
    ReInfo->_reRunning = 0;
#endif // ReMultiThreaded
}

#ifdef ReMultiThreaded
void ReShutdownUpdater()
{
	// Destroy the situation updater.
 	if (situationUpdater)
	{
		delete situationUpdater;
		situationUpdater = 0;
	}
	GfssPrintReport("schedule.csv", 1.0e-4);
}
#endif // ReMultiThreaded

static void
reCapture(void)
{
    unsigned char	*img;
    int			sw, sh, vw, vh;
    tRmMovieCapture	*capture = &(ReInfo->movieCapture);
    
    GfScrGetSize(&sw, &sh, &vw, &vh);
    img = (unsigned char*)malloc(vw * vh * 3);
    if (img == NULL) {
	return;
    }
    
    glPixelStorei(GL_PACK_ROW_LENGTH, 0);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadBuffer(GL_FRONT);
    glReadPixels((sw-vw)/2, (sh-vh)/2, vw, vh, GL_RGB, GL_UNSIGNED_BYTE, (GLvoid*)img);

    sprintf(buf, "%s/torcs-%4.4d-%8.8d.png", capture->outputBase, capture->currentCapture, capture->currentFrame++);
    GfTexWritePng(img, buf, vw, vh);
    free(img);
}

#ifndef ReMultiThreaded

int
ReUpdate(void)
{
    double 		t;
    tRmMovieCapture	*capture;
    

    START_PROFILE("ReUpdate");
    ReInfo->_refreshDisplay = 0;
    switch (ReInfo->_displayMode) {
    case RM_DISP_MODE_NORMAL:
	t = GfTimeClock();

	START_PROFILE("ReOneStep*");
	while (ReInfo->_reRunning && ((t - ReInfo->_reCurTime) > RCM_MAX_DT_SIMU)) {
	    ReOneStep(RCM_MAX_DT_SIMU);
	}
	STOP_PROFILE("ReOneStep*");

	//Send car physics to network if needed
	if (GetNetwork())
		GetNetwork()->SendCarControlsPacket(ReInfo->s);

	GfuiDisplay();
	SignalEvent(Graph);
	ReInfo->_reGraphicItf.refresh(ReInfo->s);
	GfelPostRedisplay();	/* Callback -> reDisplay */
	
	PrintEvent(Bots, "Events: ", 0);
	PrintEvent(Simu, 0, 0);
	PrintEvent(Graph, 0, "\n");
	break;
	
    case RM_DISP_MODE_NONE:
	ReOneStep(RCM_MAX_DT_SIMU);
	PrintEvent(Bots, "Events: ", 0);
	PrintEvent(Simu, 0, "\n");
	if (ReInfo->_refreshDisplay) {
	    GfuiDisplay();
	}
	GfelPostRedisplay();	/* Callback -> reDisplay */
	break;

    case RM_DISP_MODE_SIMU_SIMU:
    	ReSimuSimu();
	ReSortCars();
	if (ReInfo->_refreshDisplay) {
	    GfuiDisplay();
	}
	GfelPostRedisplay();
	break;

    case RM_DISP_MODE_CAPTURE:
	capture = &(ReInfo->movieCapture);
	while ((ReInfo->_reCurTime - capture->lastFrame) < capture->deltaFrame) {
	    ReOneStep(capture->deltaSimu);
	}
	capture->lastFrame = ReInfo->_reCurTime;
	
	GfuiDisplay();
	ReInfo->_reGraphicItf.refresh(ReInfo->s);
	reCapture();
	GfelPostRedisplay();	/* Callback -> reDisplay */
	break;
	
    }
    STOP_PROFILE("ReUpdate");

    return RM_ASYNC;
}

#endif // ReMultiThreaded

void
ReTimeMod (void *vcmd)
{
    long cmd = (long)vcmd;
    
    switch ((int)cmd) {
    case 0:
	ReInfo->_reTimeMult *= 2.0;
	if (ReInfo->_reTimeMult > 64.0) {
	    ReInfo->_reTimeMult = 64.0;
	}
	break;
    case 1:
	ReInfo->_reTimeMult *= 0.5;
	if (ReInfo->_reTimeMult < 0.25) {
	    ReInfo->_reTimeMult = 0.25;
	}
	break;
    case 2:
    default:
	ReInfo->_reTimeMult = 1.0;
	break;
    }
    sprintf(buf, "Time x%.2f", 1.0 / ReInfo->_reTimeMult);
    ReRaceMsgSet(buf, 5);
}

// Multi-threaded ReUpdate ===============================================================

#ifdef ReMultiThreaded

int reSituationUpdaterThreadLoop(void *pUpdater)
{
	return static_cast<reSituationUpdater*>(pUpdater)->threadLoop();
}

int reSituationUpdater::threadLoop()
{
	// Wait delay for each loop, from bRunning value (index 0 = false, 1 = true).
	static const double KWaitDelayMS[2] = { 1.0, RCM_MAX_DT_SIMU * 1000 / 10 };

	// Termination flag.
	bool bEnd = false;

	// Local state (false = paused, true = simulating).
	bool bRunning = false;

	// Current real time.
	double realTime;
	
	GfOut("SituationUpdater thread is started.\n");
	
	do
	{
		// Let's make current step the next one (update).
		// 1) Lock the race engine data.
		lockData("reSituationUpdater::threadLoop");

		// 2) Check if time to terminate has come.
		if (_bTerminate)
			
			bEnd = true;
		
		// 3) If not time to terminate, and running, do the update job.
		else if (_pCurrReInfo->_reRunning)
		{
			if (!bRunning)
			{
				bRunning = true;
				GfOut("SituationUpdater thread is running.\n");
			}
			
			PrintEvent(Bots, "Updater:", 0);
			PrintEvent(Simu, 0, "\n");

			realTime = GfTimeClock();
		
			START_PROFILE("ReOneStep*");
		
			while (_pCurrReInfo->_reRunning
				   && ((realTime - _pCurrReInfo->_reCurTime) > RCM_MAX_DT_SIMU))
			{
				// One simu + may be robots step
				ReOneStep(RCM_MAX_DT_SIMU);
			}
		
			STOP_PROFILE("ReOneStep*");
		
			// Send car physics to network if needed
			if (GetNetwork())
				GetNetwork()->SendCarControlsPacket(_pCurrReInfo->s);
		}
		
		// 3) If not time to terminate, and not running, do nothing.
		else
		{
			if (bRunning)
			{
				bRunning = false;
				GfOut("SituationUpdater thread is paused.\n");
			}
		}
			
		// 4) Unlock the race engine data.
		unlockData("reSituationUpdater::threadLoop");
		
		// 5) Let the CPU take breath if possible (but after unlocking data !).
		SDL_Delay(KWaitDelayMS[(int)bRunning]);
	}
	while (!bEnd);

	GfOut("SituationUpdater thread has been terminated.\n");
	
	return 0;
}

reSituationUpdater::reSituationUpdater(tRmInfo* pReInfo)
{
	// Save the race engine info (state + situation) pointer for the current step.
	_pCurrReInfo = pReInfo;
	_nInitDrivers = _pCurrReInfo->s->_ncars;

	// No dedicated thread if only 1 CPU/core.
	snprintf(buf, 1024, "%s%s", GetLocalDir(), RACE_ENG_CFG);
	void *paramHandle = GfParmReadFile(buf, GFPARM_RMODE_REREAD | GFPARM_RMODE_CREAT);
	const char* pszMultiThreadScheme =
		GfParmGetStr(paramHandle, RM_SECT_RACE_ENGINE, RM_ATTR_MULTI_THREADING, RM_VAL_AUTO);

	if (!strcmp(pszMultiThreadScheme, RM_VAL_OFF))
		_bThreaded = false;
	else if (!strcmp(pszMultiThreadScheme, RM_VAL_ON))
		_bThreaded = true;
	else // Can't be anything else than RM_VAL_AUTO
		_bThreaded = GfGetNumberOfCPUs() > 1;

	GfParmReleaseHandle(paramHandle);

	_bTerminate = false;

	if (_bThreaded)
	{
		// Initialize the race engine info (state + situation) pointer for the previous step.
		_pPrevReInfo = initSituation(_pCurrReInfo);

		// Create the data mutex.
		_pDataMutex = SDL_CreateMutex();
		
		// Create and start the updater thread.
		_pUpdateThread = SDL_CreateThread(reSituationUpdaterThreadLoop, this);
	}
	else
	{
		_pPrevReInfo = 0;
		_pDataMutex = 0;
		_pUpdateThread = 0;
	}
}

reSituationUpdater::~reSituationUpdater()
{
	terminate(); // In case not already done.

	if (_bThreaded)
	{
		if (_pDataMutex)
			SDL_DestroyMutex(_pDataMutex);
		
		if (_pPrevReInfo)
			freeSituation(_pPrevReInfo);
	}
}

bool reSituationUpdater::lockData(const char* pszLocker)
{
	if (!_bThreaded)
		return true;
	
	const bool bStatus = SDL_mutexP(_pDataMutex) == 0;
	if (!bStatus)
		GfOut("%s : Failed to lock data mutex\n", pszLocker);

	return bStatus;
}
	
bool reSituationUpdater::unlockData(const char* pszLocker)
{
	if (!_bThreaded)
		return true;
	
	const bool bStatus = SDL_mutexV(_pDataMutex) == 0;
	if (!bStatus)
		GfOut("%s : Failed to unlock data mutex\n", pszLocker);

	return bStatus;
}


void reSituationUpdater::start()
{
	GfOut("Unpausing race engine.\n");

	// Lock the race engine data.
	lockData("reSituationUpdater::start");

	// Set the running flags.
    _pCurrReInfo->_reRunning = 1;
	_pCurrReInfo->s->_raceState &= ~RM_RACE_PAUSED;

	// Resynchronize simulation time.
    _pCurrReInfo->_reCurTime = GfTimeClock() - RCM_MAX_DT_SIMU;
	
	// Unlock the race engine data.
	unlockData("reSituationUpdater::start");
}

void reSituationUpdater::stop()
{
	GfOut("Pausing race engine.\n");

	// Lock the race engine data.
	lockData("reSituationUpdater::stop");

	// Reset the running flags.
	_pCurrReInfo->_reRunning = 0;
	_pCurrReInfo->s->_raceState |= RM_RACE_PAUSED;
		
	// Unlock the race engine data.
	unlockData("reSituationUpdater::stop");
}

int reSituationUpdater::terminate()
{
	int status = 0;
	
	GfOut("Terminating race engine.\n");

	// Lock the race engine data.
	lockData("reSituationUpdater::terminate");

	// Set the death flag.
    _bTerminate = true;
	
	// Unlock the race engine data.
	unlockData("reSituationUpdater::terminate");
	
	// Wait for the thread to gracefully terminate if any.
	if (_bThreaded)
	{
		SDL_WaitThread(_pUpdateThread, &status);
		_pUpdateThread = 0;
 	}

	return status;
}

void reSituationUpdater::freeSituation(tRmInfo*& pSituation)
{
	if (pSituation)
	{
		// carList
		if (pSituation->carList)
		{
			for (int nCarInd = 0; nCarInd < _nInitDrivers; nCarInd++)
			{
				tCarElt* pTgtCar = &pSituation->carList[nCarInd];
		
				tCarPenalty *penalty;
				while ((penalty = GF_TAILQ_FIRST(&(pTgtCar->_penaltyList)))
					   != GF_TAILQ_END(&(pTgtCar->_penaltyList)))
				{
					GF_TAILQ_REMOVE (&(pTgtCar->_penaltyList), penalty, link);
					free(penalty);
				}
				free(pTgtCar->_curSplitTime);
				free(pTgtCar->_bestSplitTime);
			}
		
			free(pSituation->carList);
		}
		
		// s
		if (pSituation->s)
			free(pSituation->s);

		// rules
		if (pSituation->rules)
			free(pSituation->rules);

		// raceEngineInfo
		if (pSituation->_reMessage)
			free(pSituation->_reMessage);
		if (pSituation->_reBigMessage)
			free(pSituation->_reBigMessage);
		if (pSituation->_reCarInfo)
			free(pSituation->_reCarInfo);
		
		free(pSituation);
		pSituation = 0;
	}
}

tRmInfo* reSituationUpdater::initSituation(const tRmInfo* pSource)
{
	tRmInfo* pTarget;

	// Allocate main structure (level 0).
	pTarget = (tRmInfo *)calloc(1, sizeof(tRmInfo));

	// Allocate variable level 1 structures.
	pTarget->carList = (tCarElt*)calloc(_nInitDrivers, sizeof(tCarElt));
	pTarget->s = (tSituation *)calloc(1, sizeof(tSituation));
	pTarget->rules = (tRmCarRules*)calloc(_nInitDrivers, sizeof(tRmCarRules));

	// Assign level 1 constants.
	pTarget->track = pSource->track; // Only read during the race.
	pTarget->params = pSource->params; // Never read/written during the race.
	pTarget->mainParams = pSource->mainParams; // Never read/written during the race.
	pTarget->results = pSource->results; // Never read/written during the race.
	pTarget->mainResults = pSource->mainResults; // Never read/written during the race.
	pTarget->modList = pSource->modList; // Not used / written by updater.
	pTarget->movieCapture = pSource->movieCapture; // Not used by updater.

	// Assign level 2 constants in carList field.
	for (int nCarInd = 0; nCarInd < _nInitDrivers; nCarInd++)
	{
		tCarElt* pTgtCar = &pTarget->carList[nCarInd];
		tCarElt* pSrcCar = &pSource->carList[nCarInd];

		pTgtCar->_curSplitTime = (double*)malloc(sizeof(double) * (pSource->track->numberOfSectors - 1));
		pTgtCar->_bestSplitTime = (double*)malloc(sizeof(double) * (pSource->track->numberOfSectors - 1));

		memcpy(&pTgtCar->info, &pSrcCar->info, sizeof(tInitCar)); // Not changed + only read during the race.
		memcpy(&pTgtCar->priv, &pSrcCar->priv, sizeof(tPrivCar)); // Partly only read during the race ; other copied in vars below.
		pTgtCar->robot = pSrcCar->robot; // Not changed + only read during the race.
	}

	// Allocate level 2 structures in s field.
	pTarget->s->cars = (tCarElt **)calloc(_nInitDrivers, sizeof(tCarElt *));

	// Allocate level 2 structures in raceEngineInfo field.
	pTarget->_reCarInfo = (tReCarInfo*)calloc(_nInitDrivers, sizeof(tReCarInfo));
		
	// Assign level 2 constants in raceEngineInfo field.
	pTarget->_reParam = pSource->_reParam; // Not used / written by updater.
	pTarget->_reTrackItf = pSource->_reTrackItf; // Not used / written by updater.
	pTarget->_reGraphicItf = pSource->_reGraphicItf; // Not used / written by updater.
	pTarget->_reSimItf = pSource->_reSimItf; // Not used / written by updater.
	pTarget->_reGameScreen = pSource->_reGameScreen; // Nor changed nor shared during the race.
	pTarget->_reMenuScreen = pSource->_reMenuScreen; // Nor changed nor shared during the race.
	pTarget->_reFilename = pSource->_reFilename; // Not used during the race.
	pTarget->_reName = pSource->_reName; // Not changed + only read during the race.
	pTarget->_reRaceName = pSource->_reRaceName; // Not changed + only read during the race.

	return pTarget;
}

tRmInfo* reSituationUpdater::deliverSituation(tRmInfo*& pTarget, const tRmInfo* pSource)
{
	tCarElt* pTgtCar;
	tCarElt* pSrcCar;

	// Copy variable data from source to target.
	// 1) pSource->carList
	for (int nCarInd = 0; nCarInd < _nInitDrivers; nCarInd++)
	{
		pTgtCar = &pTarget->carList[nCarInd];
		pSrcCar = &pSource->carList[nCarInd];
		
		pTgtCar->index = pSrcCar->index;
		memcpy(&pTgtCar->pub, &pSrcCar->pub, sizeof(tPublicCar));

		// race
		double* pBestSplitTime = pTgtCar->_bestSplitTime;
		double* pCurSplitTime = pTgtCar->_curSplitTime;
		memcpy(&pTgtCar->race, &pSrcCar->race, sizeof(tCarRaceInfo));
		pTgtCar->_bestSplitTime = pBestSplitTime;
		memcpy(pTgtCar->_bestSplitTime, pSrcCar->_bestSplitTime,
			   sizeof(double) * (pSource->track->numberOfSectors - 1));
		pTgtCar->_curSplitTime = pCurSplitTime;
		memcpy(pTgtCar->_curSplitTime, pSrcCar->_curSplitTime,
			   sizeof(double) * (pSource->track->numberOfSectors - 1));

		// Clear target penalty list, and then copy the source one into it.
		tCarPenalty *penalty;
        while ((penalty = GF_TAILQ_FIRST(&(pTgtCar->_penaltyList)))
			   != GF_TAILQ_END(&(pTgtCar->_penaltyList)))
		{
			GF_TAILQ_REMOVE (&(pTgtCar->_penaltyList), penalty, link);
			free(penalty);
        }
		GF_TAILQ_INIT(&(pTgtCar->_penaltyList));
		penalty = GF_TAILQ_FIRST(&(pSrcCar->_penaltyList));
        while (penalty)
		{
			tCarPenalty *newPenalty = (tCarPenalty*)calloc(1, sizeof(tCarPenalty));
			newPenalty->penalty = penalty->penalty;
			newPenalty->lapToClear = penalty->lapToClear;
			GF_TAILQ_INSERT_TAIL(&(pTgtCar->_penaltyList), newPenalty, link);
			penalty = GF_TAILQ_NEXT(penalty, link);
		}

		// priv
		memcpy(&pTgtCar->priv.wheel[0], &pSrcCar->priv.wheel[0], 4*sizeof(tWheelState));
		memcpy(&pTgtCar->priv.corner[0], &pSrcCar->priv.corner[0], 4*sizeof(tPosd));
		pTgtCar->_gear = pSrcCar->_gear;
		pTgtCar->_fuel = pSrcCar->_fuel;
		pTgtCar->_fuelTotal = pSrcCar->_fuelTotal;
		pTgtCar->_fuelInstant = pSrcCar->_fuelInstant;
		pTgtCar->_enginerpm = pSrcCar->_enginerpm;
		memcpy(&pTgtCar->priv.skid[0], &pSrcCar->priv.skid[0], 4*sizeof(tdble));
		memcpy(&pTgtCar->priv.reaction[0], &pSrcCar->priv.reaction[0], 4*sizeof(tdble));
		pTgtCar->_collision = pSrcCar->_collision;
		pTgtCar->_smoke = pSrcCar->_smoke;
		pTgtCar->_normal = pSrcCar->_normal;
		pTgtCar->_coll2Pos = pSrcCar->_coll2Pos;
		pTgtCar->_dammage = pSrcCar->_dammage;
		//pTgtCar->_debug = pSrcCar->_debug; // Ever used anywhere ?
		pTgtCar->priv.collision_state = pSrcCar->priv.collision_state;
		//pTgtCar->_memoryPool = pSrcCar->_; // ???? Memory pool copy ??????

		// ctrl
		memcpy(&pTgtCar->ctrl, &pSrcCar->ctrl, sizeof(tCarCtrl));

		// pitcmd
		memcpy(&pTgtCar->pitcmd, &pSrcCar->pitcmd, sizeof(tCarPitCmd));

		// next : ever used anywhere ?
		//struct CarElt	*next;
	}
	
	// 2) pSource->s
	pTarget->s->raceInfo = pSource->s->raceInfo;
	pTarget->s->deltaTime = pSource->s->deltaTime;
	pTarget->s->currentTime = pSource->s->currentTime;
	pTarget->s->nbPlayers = pSource->s->nbPlayers;
	for (int nCarInd = 0; nCarInd < _nInitDrivers; nCarInd++)
	{
		pTarget->s->cars[nCarInd] =
			pTarget->carList + (pSource->s->cars[nCarInd] - pSource->carList);
	}
	
	// 3) pSource->rules (1 int per driver)
	memcpy(pTarget->rules, pSource->rules, _nInitDrivers*sizeof(tRmCarRules));
	
	// 4) pSource->raceEngineInfo
	pTarget->_reState = pSource->_reState;
	memcpy(pTarget->_reCarInfo, pSource->_reCarInfo, _nInitDrivers*sizeof(tReCarInfo));
	pTarget->_reCurTime = pSource->_reCurTime;
	pTarget->_reTimeMult = pSource->_reTimeMult;
	pTarget->_reRunning = pSource->_reRunning;
	pTarget->_reLastTime = pSource->_reLastTime;
	pTarget->_displayMode = pSource->_displayMode;
	pTarget->_refreshDisplay = pSource->_refreshDisplay;
	if (pTarget->_reMessage)
	{
		free(pTarget->_reMessage);
		pTarget->_reMessage = 0;
	}
	if (pSource->_reMessage)
	{
		free(pTarget->_reMessage);
		pTarget->_reMessage = strdup(pSource->_reMessage);
	}
	pTarget->_reMessageEnd = pSource->_reMessageEnd;
	if (pTarget->_reBigMessage)
	{
		free(pTarget->_reBigMessage);
		pTarget->_reBigMessage = 0;
	}
	if (pSource->_reBigMessage)
	{
		free(pTarget->_reBigMessage);
		pTarget->_reBigMessage = strdup(pSource->_reBigMessage);
	}
	pTarget->_reBigMessageEnd = pSource->_reBigMessageEnd;

	if (pSource->_reInPitMenuCar)
		pTarget->_reInPitMenuCar =
			pTarget->carList + (pSource->_reInPitMenuCar - pSource->carList);

	return pTarget;
}


tRmInfo* reSituationUpdater::getPreviousStep()
{
	if (!_bThreaded)
		
		// No multi-threading : no need to really copy.
		_pPrevReInfo = _pCurrReInfo;

	else
	{
		// Lock the race engine data.
		if (!lockData("reSituationUpdater::getPreviousStep"))
			return 0;

		// Get the situation data.
		deliverSituation(_pPrevReInfo, _pCurrReInfo);
	
		// Unlock the race engine data.
		if (!unlockData("reSituationUpdater::getPreviousStep"))
			return 0;
	}

	return _pPrevReInfo;
}

void reSituationUpdater::computeCurrentStep()
{
	// Nothing to do if actually threaded :
	// the updater thread is already doing the job on his side.
	if (!_bThreaded)
	{
		const double t = GfTimeClock();
		
		START_PROFILE("ReOneStep*");
		
		while (_pCurrReInfo->_reRunning && ((t - _pCurrReInfo->_reCurTime) > RCM_MAX_DT_SIMU))
			
			ReOneStep(RCM_MAX_DT_SIMU);
		
		STOP_PROFILE("ReOneStep*");
		
		// Send car physics to network if needed
		if (GetNetwork())
			GetNetwork()->SendCarControlsPacket(_pCurrReInfo->s);
	}
}

int
ReUpdate(void)
{
	tRmInfo* pPrevReInfo;
	
    START_PROFILE("ReUpdate");
    ReInfo->_refreshDisplay = 0;
    switch (ReInfo->_displayMode)
	{
		case RM_DISP_MODE_NORMAL:

			PrintEvent(Graph, "Main", "\n");

			// Get the situation for the previous step.
			pPrevReInfo = situationUpdater->getPreviousStep();

			// Start computing the situation for the current step.
			situationUpdater->computeCurrentStep();
			
			// Next screen will be the pit menu if one human driver is in pit. 
			if (pPrevReInfo->_reInPitMenuCar)

				RmPitMenuStart(pPrevReInfo->s, pPrevReInfo->_reInPitMenuCar,
							   (void*)pPrevReInfo->_reInPitMenuCar, ReUpdtPitCmd);

			// Update racing messages for the user
			reRaceMsgUpdate(pPrevReInfo);
	
			GfuiDisplay();
			
			SignalEvent(Graph);
			
			GfssBeginEvent("graphics");
			pPrevReInfo->_reGraphicItf.refresh(pPrevReInfo->s);
			GfssEndEvent("graphics");
			
			GfelPostRedisplay();	/* Callback -> reDisplay */
			break;
	
		case RM_DISP_MODE_NONE:
			
			ReOneStep(RCM_MAX_DT_SIMU);
			
			reRaceMsgUpdate(ReInfo);
			
			PrintEvent(Bots, "Main: ", 0);
			PrintEvent(Simu, 0, "\n");
			if (ReInfo->_refreshDisplay)
				GfuiDisplay();
			GfelPostRedisplay();	/* Callback -> reDisplay */
			break;

		case RM_DISP_MODE_SIMU_SIMU:
			
			ReSimuSimu();
			ReSortCars();
			if (ReInfo->_refreshDisplay)
				GfuiDisplay();
			GfelPostRedisplay();
			break;

		case RM_DISP_MODE_CAPTURE:
		{
			tRmMovieCapture	*capture = &(ReInfo->movieCapture);
			while ((ReInfo->_reCurTime - capture->lastFrame) < capture->deltaFrame)
			{
				ReOneStep(capture->deltaSimu);
				
				reRaceMsgUpdate(ReInfo);
			}

			capture->lastFrame = ReInfo->_reCurTime;
	
			GfuiDisplay();
			ReInfo->_reGraphicItf.refresh(ReInfo->s);
			reCapture();
			GfelPostRedisplay();	/* Callback -> reDisplay */
			break;
		}
	
    }
    STOP_PROFILE("ReUpdate");

    return RM_ASYNC;
}

#endif // ReMultiThreaded

