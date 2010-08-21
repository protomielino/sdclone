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

	// Assign level 2 constants in carList field.
	for (int nCarInd = 0; nCarInd < NInitDrivers; nCarInd++)
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
	// 1) pSource->carList
	for (int nCarInd = 0; nCarInd < NInitDrivers; nCarInd++)
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
	for (int nCarInd = 0; nCarInd < NInitDrivers; nCarInd++)
	{
		pTarget->s->cars[nCarInd] =
			pTarget->carList + (pSource->s->cars[nCarInd] - pSource->carList);
	}
	
	// 3) pSource->rules (1 int per driver)
	memcpy(pTarget->rules, pSource->rules, NInitDrivers*sizeof(tRmCarRules));
	
	// 4) pSource->raceEngineInfo
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
		pTarget->_reInPitMenuCar =
			pTarget->carList + (pSource->_reInPitMenuCar - pSource->carList);

	return pTarget;
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
