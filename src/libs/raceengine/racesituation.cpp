/***************************************************************************

    file        : racesituation.cpp
    copyright   : (C) 2010 by Jean-Philippe Meuret
    web         : www.speed-dreams.org 
    version     : $Id$

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
    		The central raceman data structure (situation + other race infos)
    @author	    Jean-Philippe Meuret
    @version	$Id$
*/

#include <cstdlib>

#include <robot.h>
#include <raceman.h>

#include "racecars.h"
#include "racesituation.h"


// The race situation
tRmInfo	*ReInfo = 0;
static int NInitDrivers = 0;

tRmInfo* ReSituation()
{
	return ReInfo;
}

tRmInfo* ReSituationAllocInit(const tRmInfo* pSource)
{
	tRmInfo* pTarget;

	// Save the initial number of drivers.
	NInitDrivers = pSource->s->_ncars;
	
	// Allocate main structure (level 0).
	pTarget = (tRmInfo *)calloc(1, sizeof(tRmInfo));

	// Allocate variable level 1 structures.
	pTarget->carList = (tCarElt*)calloc(NInitDrivers, sizeof(tCarElt));
	pTarget->s = (tSituation *)calloc(1, sizeof(tSituation));
	pTarget->rules = (tRmCarRules*)calloc(NInitDrivers, sizeof(tRmCarRules));

	// Assign level 1 constants.
	pTarget->track = pSource->track; // Only read during the race.
	pTarget->params = pSource->params; // Never read/written during the race.
	pTarget->mainParams = pSource->mainParams; // Never read/written during the race.
	pTarget->results = pSource->results; // Never read/written during the race.
	pTarget->mainResults = pSource->mainResults; // Never read/written during the race.
	pTarget->modList = pSource->modList; // Not used / written by updater.
	pTarget->movieCapture = pSource->movieCapture; // Not used by updater.

	// Assign level 2 constants and initialize lists in carList field.
	for (int nCarInd = 0; nCarInd < NInitDrivers; nCarInd++)
	{
		tCarElt* pTgtCar = &pTarget->carList[nCarInd];
		tCarElt* pSrcCar = &pSource->carList[nCarInd];

		pTgtCar->_curSplitTime = (double*)malloc(sizeof(double) * (pSource->track->numberOfSectors - 1));
		pTgtCar->_bestSplitTime = (double*)malloc(sizeof(double) * (pSource->track->numberOfSectors - 1));

		GF_TAILQ_INIT(&(pTgtCar->_penaltyList)); // Not used by the graphics engine.

		memcpy(&pTgtCar->info, &pSrcCar->info, sizeof(tInitCar)); // Not changed + only read during the race.
		memcpy(&pTgtCar->priv, &pSrcCar->priv, sizeof(tPrivCar)); // Partly only read during the race ; other copied in vars below.
		pTgtCar->robot = pSrcCar->robot; // Not changed + only read during the race.
	}

	// Allocate level 2 structures in s field.
	pTarget->s->cars = (tCarElt **)calloc(NInitDrivers, sizeof(tCarElt *));

	// Allocate level 2 structures in raceEngineInfo field.
	pTarget->_reCarInfo = (tReCarInfo*)calloc(NInitDrivers, sizeof(tReCarInfo));
		
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

tRmInfo* ReSituationCopy(tRmInfo*& pTarget, const tRmInfo* pSource)
{
	tCarElt* pTgtCar;
	tCarElt* pSrcCar;

	// Copy variable data from source to target.
	// I) pSource->carList
	for (int nCarInd = 0; nCarInd < NInitDrivers; nCarInd++)
	{
		pTgtCar = &pTarget->carList[nCarInd];
		pSrcCar = &pSource->carList[nCarInd];

		// 1) index
		pTgtCar->index = pSrcCar->index;

		// 2) pub (raw mem copy)
		memcpy(&pTgtCar->pub, &pSrcCar->pub, sizeof(tPublicCar));

		// 3) race (field by field copy)
		// 3a) all fields except _penaltyList and _pit.
		pTgtCar->_bestLapTime = pSrcCar->_bestLapTime;
		memcpy(pTgtCar->_bestSplitTime, pSrcCar->_bestSplitTime,
			   sizeof(double) * (pSource->track->numberOfSectors - 1));
		pTgtCar->_deltaBestLapTime = pSrcCar->_deltaBestLapTime;
		pTgtCar->_curLapTime = pSrcCar->_curLapTime;
		memcpy(pTgtCar->_curSplitTime, pSrcCar->_curSplitTime,
			   sizeof(double) * (pSource->track->numberOfSectors - 1));
		pTgtCar->_lastLapTime = pSrcCar->_lastLapTime;
		pTgtCar->_curTime = pSrcCar->_curTime;
		pTgtCar->_topSpeed = pSrcCar->_topSpeed;
		pTgtCar->_laps = pSrcCar->_laps;
		pTgtCar->_nbPitStops = pSrcCar->_nbPitStops;
		pTgtCar->_remainingLaps = pSrcCar->_remainingLaps;
		pTgtCar->_pos = pSrcCar->_pos;
		pTgtCar->_timeBehindLeader = pSrcCar->_timeBehindLeader;
		pTgtCar->_lapsBehindLeader = pSrcCar->_lapsBehindLeader;
		pTgtCar->_timeBehindPrev = pSrcCar->_timeBehindPrev;
		pTgtCar->_timeBeforeNext = pSrcCar->_timeBeforeNext;
		pTgtCar->_distRaced = pSrcCar->_distRaced;
		pTgtCar->_distFromStartLine = pSrcCar->_distFromStartLine;
		pTgtCar->_currentSector = pSrcCar->_currentSector;
		pTgtCar->_scheduledEventTime = pSrcCar->_scheduledEventTime;
		//pTgtCar->_pit ... // Not used by the graphics engine (robots and situ. updater only).
		pTgtCar->_event = pSrcCar->_event;

		// Note: Commented-out because not used by the graphics engine (situ. updater only).
		// 3b) Clear target penalty list, and then copy the source one into it.
		//     TODO if profiling shows its usefull : optimize (reuse already allocated entries
		//       to minimize mallocs / frees).
		//tCarPenalty *penalty;
		//while ((penalty = GF_TAILQ_FIRST(&(pTgtCar->_penaltyList))))
		//{
		//	GfLogDebug("ReSituationCopy(car #%d) : Clearing penalty %p\n",
		//			   pSrcCar->index, penalty);
		//	GF_TAILQ_REMOVE (&(pTgtCar->_penaltyList), penalty, link);
		//	free(penalty);
        //}
		//GF_TAILQ_INIT(&(pTgtCar->_penaltyList));
		//penalty = GF_TAILQ_FIRST(&(pSrcCar->_penaltyList));
		//while (penalty)
		//{
		//	tCarPenalty *newPenalty = (tCarPenalty*)malloc(sizeof(tCarPenalty));
		//	newPenalty->penalty = penalty->penalty;
		//	newPenalty->lapToClear = penalty->lapToClear;
		//	GfLogDebug("ReSituationCopy(car #%d) : Copying penalty %p to %p\n",
		//			   pSrcCar->index, penalty, newPenalty);
		//	GF_TAILQ_INSERT_TAIL(&(pTgtCar->_penaltyList), newPenalty, link);
		//	penalty = GF_TAILQ_NEXT(penalty, link);
		//}

		// 4) priv (field by field copy)
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
		//pTgtCar->_memoryPool ...; // ???? Memory pool copy ??????

		// 5) ctrl (raw mem copy)
		memcpy(&pTgtCar->ctrl, &pSrcCar->ctrl, sizeof(tCarCtrl));

		// 6) pitcmd (raw mem copy)
		memcpy(&pTgtCar->pitcmd, &pSrcCar->pitcmd, sizeof(tCarPitCmd));

		// 7) next : ever used anywhere ? Seems not.
		//struct CarElt	*next;
	}
	
	// II) pSource->s
	pTarget->s->raceInfo = pSource->s->raceInfo;
	pTarget->s->deltaTime = pSource->s->deltaTime;
	pTarget->s->currentTime = pSource->s->currentTime;
	pTarget->s->nbPlayers = pSource->s->nbPlayers;
	for (int nCarInd = 0; nCarInd < NInitDrivers; nCarInd++)
		pTarget->s->cars[nCarInd] =
			pTarget->carList + (pSource->s->cars[nCarInd] - pSource->carList);
	
	// III) pSource->rules (1 int per driver) // Not used by the graphics engine (situ. updater only).
	//memcpy(pTarget->rules, pSource->rules, NInitDrivers*sizeof(tRmCarRules));
	
	// IV) pSource->raceEngineInfo
	//     TODO: Make _reMessage and _reBigMessage inline arrays inside raceEngineInfo
	//           to avoid strdups (optimization) ?
	pTarget->_reState = pSource->_reState;
	memcpy(pTarget->_reCarInfo, pSource->_reCarInfo, NInitDrivers*sizeof(tReCarInfo));
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
	{
		//GfLogDebug("ReSituationCopy: Pit menu request forwarded.\n");
		pTarget->_reInPitMenuCar =
			pTarget->carList + (pSource->_reInPitMenuCar - pSource->carList);
	}
	else
		pTarget->_reInPitMenuCar = 0;

	return pTarget;
}

void ReSituationAcknowlegdeEvents(tRmInfo* pCurrSituation, const tRmInfo* pPrevSituation)
{
	// Acknowlegde collision events for each car.
	for (int nCarInd = 0; nCarInd < pCurrSituation->s->_ncars; nCarInd++)
	{
		tCarElt* pCar = pCurrSituation->s->cars[nCarInd];
		pCar->priv.collision = 0;
		
		// Note: This one is only for SimuV3, and not yet used actually
		// (WIP on collision code issues ; see simuv3/collide.cpp).
		pCar->priv.collision_state.collision_count = 0;
	}

	// Acknowlegde human pit event if any (update the car pit command in current situation
	// with the one modified by the Pit menu in previous situation).
	if (pPrevSituation->_reInPitMenuCar)
	{
		//GfLogDebug("ReSituationAcknowlegdeEvents: Pit menu request cleared.\n");
		pCurrSituation->_reInPitMenuCar = 0;
	}
}

void ReSituationFreez(tRmInfo*& pSituation)
{
	if (pSituation)
	{
		// carList
		if (pSituation->carList)
		{
			for (int nCarInd = 0; nCarInd < NInitDrivers; nCarInd++)
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
