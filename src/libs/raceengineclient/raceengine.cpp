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

#include <stdlib.h>
#include <stdio.h>
#include "network.h"
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

DeclareEventType(Robots);
DeclareEventType(Simu);
DeclareEventType(Graphic);


static char	buf[1024];
static char	bestLapChanged = FALSE;
static double	msgDisp;
static double	bigMsgDisp;

tRmInfo	*ReInfo = 0;

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
	//ReStart(); /* resynchro */
	GfuiScreenActivate(ReInfo->_reGameScreen);
}

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
						ReStop();
						RmPitMenuStart(s, car, (void*)car, ReUpdtPitCmd);
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
				sprintf(buf, "%s Finished %d%s", car->_name, car->_pos, numSuffix);
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
	GfOut("Setting lap status\n");
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
			if (timeDelta>=0)
			{
				SetNetworkCarPhysics(timeDelta,&pNData->m_vecCarCtrls[i]);
			}
			else if (timeDelta<=-1.0)
			{
				GfOut("ignoring physics packet delta is %lf \n",timeDelta);
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

	if (GetNetwork())
	{
		//Resync clock in case computer falls behind
		if (s->currentTime<0.0)
		{
			double time = GfTimeClock()-GetNetwork()->GetRaceStartTime();
			s->currentTime = time;
		}

		if (s->currentTime<-2.0)
		{
			int wait = fabs(s->currentTime);
			char buf[255];
			sprintf(buf,"Race will start in %i seconds",wait);
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
			GfOut("Start time is %lf\n",GfTimeClock());
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
		SignalEvent(Robots);
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
	


	ReRaceMsgUpdate();
	ReSortCars();

	/* Update screens if a best lap changed */
	if (ReInfo->_displayMode == RM_DISP_MODE_NONE && s->_ncars > 1 && bestLapChanged)
	{
		GfOut( "UPDATE\n" );
		if (ReInfo->s->_raceType == RM_TYPE_PRACTICE)
			ReUpdatePracticeCurRes(ReInfo->s->cars[0]);
		else if (ReInfo->s->_raceType == RM_TYPE_QUALIF)
			ReUpdateQualifCurRes(ReInfo->s->cars[0]);
	}
	bestLapChanged = FALSE;
}

void
ReStart(void)
{
    ReInfo->_reRunning = 1;
    ReInfo->_reCurTime = GfTimeClock() - RCM_MAX_DT_SIMU;
}

void
ReStop(void)
{
    ReInfo->_reRunning = 0;
}

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
	SignalEvent(Graphic);
	ReInfo->_reGraphicItf.refresh(ReInfo->s);
	GfelPostRedisplay();	/* Callback -> reDisplay */
	
	PrintEvent(Robots, "Events: ", 0);
	PrintEvent(Simu, 0, 0);
	PrintEvent(Graphic, 0, "\n");
	break;
	
    case RM_DISP_MODE_NONE:
	ReOneStep(RCM_MAX_DT_SIMU);
	PrintEvent(Robots, "Events: ", 0);
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
