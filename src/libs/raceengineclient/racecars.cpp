/***************************************************************************
 
    file        : racecars.cpp
    created     : Sat Nov 23 09:05:23 CET 2002
    copyright   : (C) 2002 by Eric Espie 
    email       : eric.espie@torcs.org 
    version     : $Id: racecars.cpp,v 1.19 2007/11/06 20:43:32 torcs Exp $

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
    @version	$Id: racecars.cpp,v 1.19 2007/11/06 20:43:32 torcs Exp $
*/

#include <cstdlib>

#include <portability.h>
#include <raceman.h>
#include <network.h>
#include <tgfclient.h>
#include <robot.h>
#include <robottools.h>

#include "racesituation.h"
#include "racemessage.h"
#include "raceupdate.h"
#include "raceresults.h"
#include "racegl.h"
#include "racecars.h"


/* Compute Pit stop time */
void
ReCarsUpdateCarPitTime(tCarElt *car)
{
	tSituation *s = ReInfo->s;
	tReCarInfo *info = &(ReInfo->_reCarInfo[car->index]);
	tCarPenalty *penalty;
	int i;

	//GfLogDebug("ReCarsUpdateCarPitTime(%s) : typ=%d, fuel=%f, rep=%d\n",
	//		   car->_name, car->_pitStopType, car->_pitFuel, car->_pitRepair);

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
			GfLogInfo("%s in repair pit stop for %.1f s (refueled %.1f l, repaired %d).\n",
					  car->_name, info->totalPitTime, car->_pitFuel, car->_pitRepair);
			break;
		case RM_PIT_STOPANDGO:
			penalty = GF_TAILQ_FIRST(&(car->_penaltyList));
			if (penalty && penalty->penalty == RM_PENALTY_10SEC_STOPANDGO)
				info->totalPitTime = 10.0;
			else
				info->totalPitTime = 0.0;
			car->_scheduledEventTime = s->currentTime + info->totalPitTime; 
	
			GfLogInfo("%s in stop&go pit stop for %.1f s.\n", car->_name, info->totalPitTime);
			break;
	}
}


/* Prepare to open the pit menu when back in the main updater (thread) */
static void
reCarsSchedulePitMenu(tCarElt *car)
{
	// Do nothing if one car is already scheduled for the pit menu
	// (this one will have to wait for the current one exiting from the menu)
	if (ReInfo->_reInPitMenuCar)
	{
		GfLogInfo("%s would like to pit, but the pit menu is already in use\n", car->_name);
		return;
	}

	// Otherwise, "post" a pit menu request for this car.
	ReInfo->_reInPitMenuCar = car;
}


static void
reCarsAddPenalty(tCarElt *car, int penalty)
{
	char msg[64];
	tCarPenalty *newPenalty;

	if (! (ReInfo->s->_features & RM_FEATURE_PENALTIES) )
		return;	/* Penalties not enabled this race: do not add penalty */
	
	if (penalty == RM_PENALTY_DRIVETHROUGH)
		sprintf(msg, "%s DRIVETHROUGH PENALTY", car->_name);
	else if (penalty == RM_PENALTY_STOPANDGO)
		sprintf(msg, "%s STOP&GO PENALTY", car->_name);
	else if (penalty == RM_PENALTY_10SEC_STOPANDGO)
		sprintf(msg, "%s 10 SEC STOP&GO PENALTY", car->_name);
	else if (penalty == RM_PENALTY_DISQUALIFIED)
		sprintf(msg, "%s DISQUALIFIED", car->_name);

	ReRaceMsgSet(ReInfo, msg, 5);

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

/* Compute the race rules and penalties */
static void
reCarsRaceRules(tCarElt *car)
{
	char msg[64];
    tCarPenalty		*penalty;
    tTrack		*track = ReInfo->track;
    tRmCarRules		*rules = &(ReInfo->rules[car->index]);
    tTrackSeg		*seg = RtTrackGetSeg(&(car->_trkPos));
    tReCarInfo		*info = &(ReInfo->_reCarInfo[car->index]);
    tTrackSeg		*prevSeg = RtTrackGetSeg(&(info->prevTrkPos));
    static const float	color[] = {0.0, 0.0, 1.0, 1.0};

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
	    reCarsAddPenalty(car, RM_PENALTY_DISQUALIFIED);
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
		    sprintf(msg, "%s DRIVE THROUGH PENALTY CLEANING", car->_name);
		    ReRaceMsgSet(ReInfo, msg, 5);
		    rules->ruleState |= RM_PNST_DRIVETHROUGH;
		    break;
		case RM_PENALTY_STOPANDGO:
		case RM_PENALTY_10SEC_STOPANDGO:
		    sprintf(msg, "%s STOP&GO PENALTY CLEANING", car->_name);
		    ReRaceMsgSet(ReInfo, msg, 5);
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
		    /* it's no more a drive through */
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
		sprintf(msg, "%s penalty cleared", car->_name);
		ReRaceMsgSet(ReInfo, msg, 5);
		penalty = GF_TAILQ_FIRST(&(car->_penaltyList));
		GF_TAILQ_REMOVE(&(car->_penaltyList), penalty, link);
		FREEZ(penalty);
	    }
	    rules->ruleState = 0;
	} else {
	    /* went out of the pit lane illegally... */
	    /* it's a new stop and go... */
	    if (!(rules->ruleState & RM_PNST_STNGO)) {
	    	reCarsAddPenalty(car, RM_PENALTY_STOPANDGO);
		rules->ruleState = RM_PNST_STNGO;
	    }
	}
    } else if (seg->raceInfo & TR_PITEND) {
	rules->ruleState = 0;
    } else if (seg->raceInfo & TR_PIT) {
	/* entrered the pits not from the pit entry... */
	/* it's a new stop and go... */
	if (!(rules->ruleState & RM_PNST_STNGO)) {
	    reCarsAddPenalty(car, RM_PENALTY_STOPANDGO);
	    rules->ruleState = RM_PNST_STNGO;
	}
    }

    if (seg->raceInfo & TR_SPEEDLIMIT) {
	if (!(rules->ruleState & (RM_PNST_SPD | RM_PNST_STNGO)) && (car->_speed_x > track->pits.speedLimit)) {
	    rules->ruleState |= RM_PNST_SPD;
	    reCarsAddPenalty(car, RM_PENALTY_DRIVETHROUGH);
	}
    }
}

void
ReCarsManageCar(tCarElt *car, bool& bestLapChanged)
{
	char msg[128];
	int i;
	int xx;
	tTrackSeg *sseg;
	tdble wseg;
	static const float color[] = {0.0, 0.0, 1.0, 1.0};
	tSituation *s = ReInfo->s;
	
	tReCarInfo *info = &(ReInfo->_reCarInfo[car->index]);

	// Update top speeds.
	if (car->_speed_x > car->_topSpeed) {
		car->_topSpeed = car->_speed_x;
	}

	// (practice and qualification only).
	if (car->_speed_x > info->topSpd) {
		info->topSpd = car->_speed_x;
	}
	if (car->_speed_x < info->botSpd) {
		info->botSpd = car->_speed_x;
	}
	
	// Pitstop management.
	if (car->_pit) {

		// If the driver can ask for a pit, update control messages whether slot occupied or not.
		if (car->ctrl.raceCmd & RM_CMD_PIT_ASKED) {
			// Pit already occupied?
			if (car->_pit->pitCarIndex == TR_PIT_STATE_FREE) {
				sprintf(car->ctrl.msg[2], "Can Pit");
			} else {
				sprintf(car->ctrl.msg[2], "Pit Occupied");
			}
			memcpy(car->ctrl.msgColor, color, sizeof(car->ctrl.msgColor));
		}

		// If pitting, check if pitting delay over, and end up with pitting process if so.
		if (car->_state & RM_CAR_STATE_PIT) {
			car->ctrl.raceCmd &= ~RM_CMD_PIT_ASKED; // clear the flag.
			// Note: Due to asynchronous behaviour of the main updater and the situation updater,
			//       we have to wait for car->_scheduledEventTime being set to smthg > 0.
			if (car->_scheduledEventTime > 0.0) {
				if (car->_scheduledEventTime < s->currentTime) {
					car->_state &= ~RM_CAR_STATE_PIT;
					car->_pit->pitCarIndex = TR_PIT_STATE_FREE;
					sprintf(msg, "%s pit stop %.1f s", car->_name, info->totalPitTime);
					ReRaceMsgSet(ReInfo, msg, 5);
					GfLogInfo("%s exiting pit (%.1f s elapsed).\n", car->_name, info->totalPitTime);
				} else {
					sprintf(car->ctrl.msg[2], "In pits %.1f s",
							s->currentTime - info->startPitTime);
				}
			}
			
		// If the driver asks for a pit, check if the car is in the right conditions
		// (position, speed, ...) and start up pitting process if so.
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
					(fabs(car->_speed_x) < 1.0) && (fabs(car->_speed_y) < 1.0))
				{
					// All conditions fullfilled => enter pitting process
					car->_state |= RM_CAR_STATE_PIT;
					car->_scheduledEventTime = 0.0; // Pit will really start when set to smthg > 0.
					car->_nbPitStops++;
					for (i = 0; i < car->_pit->freeCarIndex; i++) {
						if (car->_pit->car[i] == car) {
							car->_pit->pitCarIndex = i;
							break;
						}
					}
					info->startPitTime = s->currentTime;
					sprintf(msg, "%s in pits", car->_name);
					ReRaceMsgSet(ReInfo, msg, 5);
					GfLogInfo("%s entering in pit slot.\n", car->_name);
					if (car->robot->rbPitCmd(car->robot->index, car, s) == ROB_PIT_MENU) {
						// the pit cmd is modified by menu.
						reCarsSchedulePitMenu(car);
					} else {
						ReCarsUpdateCarPitTime(car);
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
					bestLapChanged = true;
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
					t1 = GfTime2Str(car->_lastLapTime, "  ", false, 2);
					t2 = GfTime2Str(car->_bestLapTime, "  ", false, 2);
					sprintf(msg,"lap: %02d   time: %s  best: %s  top spd: %.2f    min spd: %.2f    damage: %d",
						car->_laps - 1, t1, t2,
						info->topSpd * 3.6, info->botSpd * 3.6, car->_dammage);
					ReResScreenAddText(msg);
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
				sprintf(msg, "Winner %s", car->_name);
				ReRaceMsgSetBig(ReInfo, msg, 10);
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
				sprintf(msg, "%s finished %d%s", car->_name, car->_pos, numSuffix);
				ReRaceMsgSet(ReInfo, msg, 5);
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
	reCarsRaceRules(car);
	
	info->prevTrkPos = car->_trkPos;
	car->_curLapTime = s->currentTime - info->sTime;
	car->_distFromStartLine = car->_trkPos.seg->lgfromstart +
	(car->_trkPos.seg->type == TR_STR ? car->_trkPos.toStart : car->_trkPos.toStart * car->_trkPos.seg->radius);
	car->_distRaced = (car->_laps - 1) * ReInfo->track->length + car->_distFromStartLine;
}

void 
ReCarsSortCars(void)
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
