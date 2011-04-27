/**************************************************************************

    file        : racemain.cpp
    created     : Sat Nov 16 12:13:31 CET 2002
    copyright   : (C) 2002 by Eric Espie
    email       : eric.espie@torcs.org   
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
    		
    @author	<a href=mailto:eric.espie@torcs.org>Eric Espie</a>
    @version	$Id$
*/

#include <portability.h>

#include <robot.h>
#include <network.h>

#include "raceengine.h"

#include "raceutil.h" // RmGetFeaturesList
#include "racesituation.h"
#include "racecareer.h"
#include "raceinit.h"
#include "raceupdate.h"
#include "raceresults.h"
#include "racestate.h"
#include "racetrack.h" // ReTrackInit, ReTrackUpdate

#include "teammanager.h"

#include "racemain.h"


//Utility

/** 
 * ReHumanInGroup
 * Checks if there is a human-driven car among the racing cars.
 * 
 * @return True if there is a human.
 */
bool
ReHumanInGroup()
{
	if (GfParmListSeekFirst(ReInfo->params, RM_SECT_DRIVERS) == 0) {
		do {
			if (strcmp (GfParmGetCurStr(ReInfo->params, RM_SECT_DRIVERS, RM_ATTR_MODULE, ""), "human") == 0)
				return true;
		} while (GfParmListSeekNext(ReInfo->params, RM_SECT_DRIVERS) == 0);
	}
	return false;
}//ReHumanInGroup


/***************************************************************/

void ReRaceAbandon()
{
	// Shutdown current event.
	ReEventShutdown();

	// Return to race menu
	ReInfo->_reState = RE_STATE_CONFIG;
}

void ReRaceAbort()
{
	ReShutdownUpdaters();

	ReInfo->_reSimItf.shutdown();

	// TODO: Move these 3 XXGraphicsYY calls to the user interface module ?
	if (ReInfo->_displayMode == RM_DISP_MODE_NORMAL)
	{
		ReUI().unloadCarsGraphics();
		ReUI().shutdownGraphicsView();
		ReUI().unloadTrackGraphics();
	}
	
	ReRaceCleanDrivers();

	if (GetNetwork())
	{
		GetNetwork()->Disconnect();
	}

	FREEZ(ReInfo->_reCarInfo);
	
	// Return to race menu
	if (ReInfo->params != ReInfo->mainParams)
	{
		GfParmReleaseHandle(ReInfo->params);
		ReInfo->params = ReInfo->mainParams;
	}
	ReInfo->_reState = RE_STATE_CONFIG;
}

void ReRaceSkipSession()
{
	ReInfo->_reState = RE_STATE_RACE_END;
}

int
ReRaceEventInit(void)
{
	void *mainParams = ReInfo->mainParams;
	void *params = ReInfo->params;
	
	/* Look if it is necessary to open another file */
	if (strcmp(GfParmGetStr(mainParams, RM_SECT_SUBFILES, RM_ATTR_HASSUBFILES, RM_VAL_NO), RM_VAL_YES) == 0) {
		/* Close previous params */
		if (params != mainParams)
			GfParmReleaseHandle(params);

		/* Read the new params */
		ReInfo->params = GfParmReadFile( GfParmGetStr( ReInfo->mainResults, RE_SECT_CURRENT, RE_ATTR_CUR_FILE, "" ), GFPARM_RMODE_STD );
		GfLogDebug("Career : ReInfo->mainResults gives curfile = %s\n",
				   GfParmGetStr( ReInfo->mainResults, RE_SECT_CURRENT, RE_ATTR_CUR_FILE, ""));
		if (!params)
			GfLogWarning( "Params weren't read correctly !!!\n" );
		params = ReInfo->params;

		/* Close previous results */
		if (ReInfo->results != ReInfo->mainResults) {
			GfParmWriteFile(NULL, ReInfo->results, NULL);
			GfParmReleaseHandle(ReInfo->results);
		}

		/* Read the new results */
		ReInfo->results = GfParmReadFile( GfParmGetStr( params, RM_SECT_SUBFILES, RM_ATTR_RESULTSUBFILE, ""), GFPARM_RMODE_STD );
		if (!ReInfo->results)
			GfLogWarning( "Results weren't read correctly !!!\n" );
	}

	// Initialize the race session name.
	ReInfo->_reRaceName = ReGetCurrentRaceName();
	GfLogInfo("Starting %s session\n", ReInfo->_reRaceName);

	ReUI().activateLoadingScreen(ReInfo->_reName, "data/img/splash-raceload.jpg");
	
	ReInfo->s->_features = RmGetFeaturesList(params);

	ReTrackInit();
	
	ReEventInitResults();

	if (GfParmGetEltNb(params, RM_SECT_TRACKS) > 1) {
		ReUI().activateNextEventMenu();
		return RM_ASYNC | RM_NEXT_STEP;
	}
	
	return RM_SYNC | RM_NEXT_STEP;
}


int
RePreRace(void)
{
	char path[64];
	tdble dist;
	const char *raceName;
	const char *raceType;
	void *params = ReInfo->params;
	void *results = ReInfo->results;
	int curRaceIdx;

	raceName = ReInfo->_reRaceName = ReGetCurrentRaceName();
	
	GfParmRemoveVariable (ReInfo->params, "/", "humanInGroup");
	GfParmRemoveVariable (ReInfo->params, "/", "eventNb");
	GfParmSetVariable (ReInfo->params, "/", "humanInGroup", ReHumanInGroup() ? 1.0f : 0.0f);
	GfParmSetVariable (ReInfo->params, "/", "eventNb", GfParmGetNum (ReInfo->results, RE_SECT_CURRENT, RE_ATTR_CUR_TRACK, NULL, 1.0 ) );
	if (!raceName) {
		return RM_ERROR;
	}

	if (strcmp(GfParmGetStr(params, raceName, RM_ATTR_ENABLED, RM_VAL_YES), RM_VAL_NO) == 0) {
		GfLogDebug( "||||++|||| NOT ENABLED!\n" );
		curRaceIdx = (int)GfParmGetNum(results, RE_SECT_CURRENT, RE_ATTR_CUR_RACE, NULL, 1);
		if (curRaceIdx < GfParmGetEltNb(params, RM_SECT_RACES)) {
			GfLogDebug( "||||++|||| NOT LAST RACE!\n" );
			curRaceIdx++;
			GfLogTrace("Race Nb %d\n", curRaceIdx);
			GfParmSetNum(results, RE_SECT_CURRENT, RE_ATTR_CUR_RACE, NULL, (tdble)curRaceIdx);
	
			return RM_SYNC | RM_NEXT_RACE;
		}
	
		GfParmSetNum(results, RE_SECT_CURRENT, RE_ATTR_CUR_RACE, NULL, 1);
		return RM_SYNC | RM_NEXT_RACE | RM_NEXT_STEP;
	}

	dist = GfParmGetNum(params, raceName, RM_ATTR_DISTANCE, NULL, 0);
	if (dist < 0.001) {
		ReInfo->s->_totLaps = (int)GfParmGetNum(params, raceName, RM_ATTR_LAPS, NULL, 30);
	} else {
		ReInfo->s->_totLaps = ((int)(dist / ReInfo->track->length)) + 1;
	}
	ReInfo->s->_totTime = GfParmGetNum(params, raceName, RM_ATTR_SESSIONTIME, NULL, -60.0f);
	ReInfo->s->_maxDammage = (int)GfParmGetNum(params, raceName, RM_ATTR_MAX_DMG, NULL, 10000);
	ReInfo->s->_extraLaps = ReInfo->s->_totLaps;

	if (ReInfo->s->_totTime > 0.0f && ( ReInfo->s->_features & RM_FEATURE_TIMEDSESSION ) == 0 )
	{
		/* Timed session not supported: add 2 km for every minute */
		ReInfo->s->_totLaps += (int)floor(ReInfo->s->_totTime * 2000.0f / ReInfo->track->length + 0.5f);
		ReInfo->s->_totTime = -60.0f;
	}

	if (ReInfo->s->_totTime <= 0.0f)
	{
		ReInfo->s->_totTime = -60.0f;	/* Make sure that if no time is set, the set is far below zero */
		ReInfo->s->_extraLaps = 0;
	}

	raceType = GfParmGetStr(params, raceName, RM_ATTR_TYPE, RM_VAL_RACE);
	if (!strcmp(raceType, RM_VAL_RACE)) {
		ReInfo->s->_raceType = RM_TYPE_RACE;
	} else if (!strcmp(raceType, RM_VAL_QUALIF)) {
		ReInfo->s->_raceType = RM_TYPE_QUALIF;
	} else if (!strcmp(raceType, RM_VAL_PRACTICE)) {
		ReInfo->s->_raceType = RM_TYPE_PRACTICE;
	}

	if (ReInfo->s->_raceType != RM_TYPE_RACE && ReInfo->s->_extraLaps > 0)
	{
		/* During timed practice or qualification, there are no extra laps */
		ReInfo->s->_extraLaps = 0;
		ReInfo->s->_totLaps = 0;
	}

	ReInfo->s->_raceState = 0;

	/* Cleanup results */
	snprintf(path, sizeof(path), "%s/%s/%s", ReInfo->track->name, RE_SECT_RESULTS, raceName);
	GfParmListClean(results, path);

	return RM_SYNC | RM_NEXT_STEP;
}

/* return state mode */
int
ReRaceRealStart(void)
{
	int i, j;
	tRobotItf *robot;
	tReCarInfo *carInfo;
	const char *dllname;
	char path[256];
	char buf[128];
	int foundHuman;
	void *params = ReInfo->params;
	void *results = ReInfo->results;
	tSituation *s = ReInfo->s;
	tMemoryPool oldPool = NULL;
	void* carHdle;

	// Load simulation engine
	dllname = GfParmGetStr(ReInfo->_reParam, "Modules", "simu", "");
	snprintf(buf, sizeof(buf), "Loading simulation engine (%s) ...", dllname);
	ReUI().addLoadingMessage(buf);
	snprintf(path, sizeof(path), "%smodules/simu/%s.%s", GfLibDir (), dllname, DLLEXT);
	if (GfModLoad(0, path, &ReRaceModList)) 
		return RM_ERROR;
	ReRaceModList->modInfo->fctInit(ReRaceModList->modInfo->index, &ReInfo->_reSimItf);

	// Check if there is a human in the driver list
	foundHuman = ReHumanInGroup() ? 2 : 0;

	// TODO: Replace this _displayMode dirty hack by adding a new bSimuSimu arg to ReInitCars ?
	// Set _displayMode here because then robot->rbNewTrack isn't called. This is a lot faster for simusimu
	if (strcmp(GfParmGetStr(params, ReInfo->_reRaceName, RM_ATTR_DISPMODE, RM_VAL_VISIBLE), RM_VAL_SIMUSIMU) == 0 && foundHuman == 0)
		ReInfo->_displayMode = RM_DISP_MODE_SIMU_SIMU;
	else
		ReInfo->_displayMode = RM_DISP_MODE_NORMAL;

	// Initialize & place cars
	if (ReInitCars())
		return RM_ERROR;

	// Check if there is a human in the current race
	for (i = 0; i < s->_ncars; i++) {
		if (s->cars[i]->_driverType == RM_DRV_HUMAN) {
			foundHuman = 1;
			break;
		}//if human
	}//for i

	// Determine the display mode.
	ReInfo->_displayMode = RM_DISP_MODE_NORMAL;

	if (foundHuman != 1)
	{
		// No human in current race
		const std::string strDispMode =
			GfParmGetStr(params, ReInfo->_reRaceName, RM_ATTR_DISPMODE, RM_VAL_VISIBLE);
		if (strDispMode == RM_VAL_INVISIBLE)
		{
			ReInfo->_displayMode = RM_DISP_MODE_NONE;
		}
		else if (strDispMode == RM_VAL_SIMUSIMU)
		{
			// Human in driver list ...
			if (foundHuman == 2)
			{
				// ... but not in current race
				if (ReInfo->s->_raceType == RM_TYPE_QUALIF
					|| ReInfo->s->_raceType == RM_TYPE_PRACTICE)
				{
					ReInfo->_displayMode = RM_DISP_MODE_NONE;
				} /* Else: normally visible */
			}
			else
			{
				ReInfo->_displayMode = RM_DISP_MODE_SIMU_SIMU;
			}//if foundHuman == 2
		}
	}//if foundHuman != 1

	// Create the game screen according to the actual display mode.
	if (ReInfo->_displayMode == RM_DISP_MODE_NORMAL)
		ReInfo->_reGameScreen = ReUI().createRaceScreen();
	else
		ReInfo->_reGameScreen = ReUI().createResultsMenu();
	
	// If neither a qualification, nor a practice, or else 1st driver, activate race loading screen.
	if (!(ReInfo->s->_raceType == RM_TYPE_QUALIF || ReInfo->s->_raceType == RM_TYPE_PRACTICE)
		|| (int)GfParmGetNum(results, RE_SECT_CURRENT, RE_ATTR_CUR_DRIVER, NULL, 1) == 1)
	{
		ReUI().activateLoadingScreen(ReInfo->_reName, "data/img/splash-raceload.jpg");
	}

	// Load drivers for the race
	for (i = 0; i < s->_ncars; i++) {
		snprintf(buf, sizeof(buf), "cars/%s/%s.xml",
				 s->cars[i]->_carName, s->cars[i]->_carName);
		carHdle = GfParmReadFile(buf, GFPARM_RMODE_STD);
		snprintf(buf, sizeof(buf), "Loading driver %s (%s) ...",
				 s->cars[i]->_name, GfParmGetName(carHdle));
		ReUI().addLoadingMessage(buf);
		if (ReInfo->_displayMode != RM_DISP_MODE_SIMU_SIMU) { //Tell robots they are to start a new race
			robot = s->cars[i]->robot;
			GfPoolMove( &s->cars[i]->_newRaceMemPool, &oldPool );
			robot->rbNewRace(robot->index, s->cars[i], s);
			GfPoolFreePool( &oldPool );
		}//if ! simusimu
	}//for i
	carInfo = ReInfo->_reCarInfo;
	RtTeamManagerStart();

	// TODO: Try and move this into the user interface module,
	//       right after it called ReRaceStart ?
	// Initialize the graphics engine
	if (ReInfo->_displayMode == RM_DISP_MODE_NORMAL)
	{
		// It must be done after the cars are loaded and the track is loaded.
		// The track will be unloaded if the event ends.
		// The graphics module is kept open if more than one race is driven.

		// Initialize the graphics engine.
		if (ReUI().initializeGraphics())
		{
			// Initialize the track graphics.
			ReUI().loadTrackGraphics(ReInfo->track);
		}
	}
	
	// Initialize the physics engine
	ReInfo->_reSimItf.update(s, RCM_MAX_DT_SIMU, -1);
	for (i = 0; i < s->_ncars; i++) {
		carInfo[i].prevTrkPos = s->cars[i]->_trkPos;
	}

	// All cars start with max brakes on
	ReUI().addLoadingMessage("Running Prestart ...");
	for (i = 0; i < s->_ncars; i++) {
		memset(&(s->cars[i]->ctrl), 0, sizeof(tCarCtrl));
		s->cars[i]->ctrl.brakeCmd = 1.0;
	}
	for (j = 0; j < (int)(1.0 / RCM_MAX_DT_SIMU); j++) {
		ReInfo->_reSimItf.update(s, RCM_MAX_DT_SIMU, -1);
	}

	if (ReInfo->_displayMode != RM_DISP_MODE_NORMAL) {
		if (ReInfo->s->_raceType == RM_TYPE_QUALIF) {
			ReUpdateQualifCurRes(s->cars[0]);
		} else if (ReInfo->s->_raceType == RM_TYPE_PRACTICE && s->_ncars > 1) {
			ReUpdatePracticeCurRes(s->cars[0]);
		} else {
			ReUI().setResultsMenuTrackName(ReInfo->s->_raceType, ReInfo->track->name);
			snprintf(buf, sizeof(buf), "%s (%s)", s->cars[0]->_name, s->cars[0]->_carName);
			ReUI().setResultsMenuTitle(buf);
		}
	}//if displayMode != normal

	ReInfo->_reTimeMult = 1.0;
	ReInfo->_reLastRobTime = -1.0;
	if (GetNetwork())
		ReInfo->s->currentTime = GfTimeClock() - GetNetwork()->GetRaceStartTime();
	else
		ReInfo->s->currentTime = -2.0;	// We start 2 seconds before the real race start
	ReInfo->s->deltaTime = RCM_MAX_DT_SIMU;
	ReInfo->s->_raceState = RM_RACE_STARTING;

	// TODO: Try and move this into the user interface module,
	//       right after it called ReRaceStart ?
	// Initialize the graphics view.
	if (ReInfo->_displayMode == RM_DISP_MODE_NORMAL)
		ReUI().setupGraphicsView();

	ReInfo->_rePitRequester = 0;
	ReInfo->_reMessage = 0;
	ReInfo->_reMessageEnd = 0.0;
	ReInfo->_reBigMessage = 0;
	ReInfo->_reBigMessageEnd = 0.0;
	
	ReInitUpdaters();

	// TODO: Try and move this into the user interface module,
	//       right after it called ReRaceStart ?
	// Initialize cars graphics.
	if (ReInfo->_displayMode == RM_DISP_MODE_NORMAL)
	{
		ReUI().addLoadingMessage("Loading cars ...");
		
		ReUI().loadCarsGraphics(ReOutputSituation()->s);
	}

	if (GetNetwork())
	{
		ReUI().addLoadingMessage("Preparing online race ...");
		
		GetNetwork()->RaceInit(ReOutputSituation()->s);
		GetNetwork()->SetRaceActive(true);
	}

	ReUI().addLoadingMessage("Ready.");

	ReUI().activateGameScreen();

	return RM_SYNC | RM_NEXT_STEP;
}//ReRaceRealStart

/* return state mode */
int
ReRaceStart(void)
{
	char path[128];
	char path2[128];
	int i;
	int nCars;
	int maxCars;
	char *prevRaceName;
	const char *gridType;
	const char *raceName = ReInfo->_reRaceName;
	void *params = ReInfo->params;
	void *results = ReInfo->results;

	// Reallocate and reset car info for the race.
	FREEZ(ReInfo->_reCarInfo);
	ReInfo->_reCarInfo =
		(tReCarInfo*)calloc(GfParmGetEltNb(params, RM_SECT_DRIVERS), sizeof(tReCarInfo));

	GfLogInfo("Starting %s %s session at %s\n", ReInfo->_reName, raceName, ReInfo->track->name);

	// Drivers starting order
	GfParmListClean(params, RM_SECT_DRIVERS_RACING);
	if ((ReInfo->s->_raceType == RM_TYPE_QUALIF || ReInfo->s->_raceType == RM_TYPE_PRACTICE)
		&& ReInfo->s->_totTime < 0.0f) // <= What's this time test for ?
	{
		// Race loading screen
		i = (int)GfParmGetNum(results, RE_SECT_CURRENT, RE_ATTR_CUR_DRIVER, NULL, 1);
		if (i == 1) {
			ReUI().activateLoadingScreen(ReInfo->_reName, "data/img/splash-raceload.jpg");
			ReUI().addLoadingMessage("Preparing Starting Grid ...");
		} else {
			ReUI().shutdownLoadingScreen();
		}

		// Propagate competitor drivers info to the real race starting grid
		snprintf(path, sizeof(path), "%s/%d", RM_SECT_DRIVERS, i);
		snprintf(path2, sizeof(path2), "%s/%d", RM_SECT_DRIVERS_RACING, 1);

		GfParmSetStr(params, path2, RM_ATTR_MODULE,
					 GfParmGetStr(params, path, RM_ATTR_MODULE, ""));
		GfParmSetNum(params, path2, RM_ATTR_IDX, NULL,
					 GfParmGetNum(params, path, RM_ATTR_IDX, NULL, 0));
		GfParmSetNum(params, path2, RM_ATTR_EXTENDED, NULL,
					 GfParmGetNum(params, path, RM_ATTR_EXTENDED, NULL, 0));
		GfParmSetNum(params, path2, RM_ATTR_SKINTARGETS, NULL,
					 GfParmGetNum(params, path, RM_ATTR_SKINTARGETS, NULL, 0));
		if (GfParmGetStr(params, path, RM_ATTR_SKINNAME, 0))
			GfParmSetStr(params, path2, RM_ATTR_SKINNAME,
						 GfParmGetStr(params, path, RM_ATTR_SKINNAME, ""));
	}
	else
	{
		// Race loading screen
		ReUI().activateLoadingScreen(ReInfo->_reName, "data/img/splash-raceload.jpg");
		ReUI().addLoadingMessage("Preparing Starting Grid ...");

		gridType = GfParmGetStr(params, raceName, RM_ATTR_START_ORDER, RM_VAL_DRV_LIST_ORDER);
		
		nCars = GfParmGetEltNb(params, RM_SECT_DRIVERS);
		maxCars = (int)GfParmGetNum(params, raceName, RM_ATTR_MAX_DRV, NULL, 100);
		nCars = MIN(nCars, maxCars);
		
		// Starting grid in the arrival order of the previous race (or qualification session)
		if (!strcmp(gridType, RM_VAL_LAST_RACE_ORDER))
		{
			GfLogTrace("Starting grid in the order of the last race\n");
			
			prevRaceName = ReGetPrevRaceName();
			if (!prevRaceName) {
				return RM_ERROR;
			}

			for (i = 1; i < nCars + 1; i++) {
				snprintf(path, sizeof(path), "%s/%s/%s/%s/%d",
						 ReInfo->track->name, RE_SECT_RESULTS, prevRaceName, RE_SECT_RANK, i);
				snprintf(path2, sizeof(path2), "%s/%d", RM_SECT_DRIVERS_RACING, i);
				GfParmSetStr(params, path2, RM_ATTR_MODULE,
							 GfParmGetStr(results, path, RE_ATTR_MODULE, ""));
				GfParmSetNum(params, path2, RM_ATTR_IDX, NULL,
							 GfParmGetNum(results, path, RE_ATTR_IDX, NULL, 0));
				GfParmSetNum(params, path2, RM_ATTR_EXTENDED, NULL,
							 GfParmGetNum(results, path, RM_ATTR_EXTENDED, NULL, 0));
				GfParmSetNum(params, path2, RM_ATTR_SKINTARGETS, NULL,
							 GfParmGetNum(results, path, RM_ATTR_SKINTARGETS, NULL, 0));
				if (GfParmGetStr(results, path, RM_ATTR_SKINNAME, 0))
					GfParmSetStr(params, path2, RM_ATTR_SKINNAME,
								 GfParmGetStr(results, path, RM_ATTR_SKINNAME, ""));
			}
		}
		
		// Starting grid in the reversed arrival order of the previous race
		else if (!strcmp(gridType, RM_VAL_LAST_RACE_RORDER))
		{
			GfLogTrace("Starting grid in the reverse order of the last race\n");

			prevRaceName = ReGetPrevRaceName();
			if (!prevRaceName) {
				return RM_ERROR;
			}

			for (i = 1; i < nCars + 1; i++) {
				snprintf(path, sizeof(path), "%s/%s/%s/%s/%d",
						ReInfo->track->name, RE_SECT_RESULTS, prevRaceName, RE_SECT_RANK, nCars - i + 1);
				snprintf(path2, sizeof(path2), "%s/%d", RM_SECT_DRIVERS_RACING, i);
				GfParmSetStr(params, path2, RM_ATTR_MODULE,
							 GfParmGetStr(results, path, RE_ATTR_MODULE, ""));
				GfParmSetNum(params, path2, RM_ATTR_IDX, NULL,
							 GfParmGetNum(results, path, RE_ATTR_IDX, NULL, 0));
				GfParmSetNum(params, path2, RM_ATTR_EXTENDED, NULL,
							 GfParmGetNum(results, path, RM_ATTR_EXTENDED, NULL, 0));
				GfParmSetNum(params, path2, RM_ATTR_SKINTARGETS, NULL,
							 GfParmGetNum(results, path, RM_ATTR_SKINTARGETS, NULL, 0));
				if (GfParmGetStr(results, path, RM_ATTR_SKINNAME, 0))
					GfParmSetStr(params, path2, RM_ATTR_SKINNAME,
								 GfParmGetStr(results, path, RM_ATTR_SKINNAME, ""));
			}
		}

		// Starting grid in the drivers list order
		else
		{
			GfLogTrace("Starting grid in the order of the driver list\n");

			for (i = 1; i < nCars + 1; i++) {
				snprintf(path, sizeof(path), "%s/%d", RM_SECT_DRIVERS, i);
				snprintf(path2, sizeof(path2), "%s/%d", RM_SECT_DRIVERS_RACING, i);
				GfParmSetStr(params, path2, RM_ATTR_MODULE,
							 GfParmGetStr(params, path, RM_ATTR_MODULE, ""));
				GfParmSetNum(params, path2, RM_ATTR_IDX, NULL,
							 GfParmGetNum(params, path, RM_ATTR_IDX, NULL, 0));
				GfParmSetNum(params, path2, RM_ATTR_EXTENDED, NULL,
							 GfParmGetNum(params, path, RM_ATTR_EXTENDED, NULL, 0));
				GfParmSetNum(params, path2, RM_ATTR_SKINTARGETS, NULL,
							 GfParmGetNum(params, path, RM_ATTR_SKINTARGETS, NULL, 0));
				if (GfParmGetStr(params, path, RM_ATTR_SKINNAME, 0))
					GfParmSetStr(params, path2, RM_ATTR_SKINNAME,
								 GfParmGetStr(params, path, RM_ATTR_SKINNAME, ""));
			}
		}
	}
	
	//ReTrackUpdate();

	if (!strcmp(GfParmGetStr(params, ReInfo->_reRaceName, RM_ATTR_SPLASH_MENU, RM_VAL_NO), RM_VAL_YES)) {
		ReUI().shutdownLoadingScreen();
		ReUI().activateStartRaceMenu();
		return RM_ASYNC | RM_NEXT_STEP;
	}

	return ReRaceRealStart();
}

void ReRaceRestart()
{
	ReShutdownUpdaters();

	ReRaceCleanup();
	
	ReInfo->_reState = RE_STATE_PRE_RACE;
}

int
ReRaceStop(void)
{
	ReStop();

	ReUI().activateStopRaceMenu();
	
	return RM_ASYNC | RM_NEXT_STEP;
}

int
ReRaceEnd(void)
{
	int curDrvIdx;
	void *params = ReInfo->params;
	void *results = ReInfo->results;

	ReShutdownUpdaters();

	ReRaceCleanup();

	if (GetNetwork())
		GetNetwork()->RaceDone();

	// If we are at the end of a qualification or practice session,
	// select the next competitor : it is his turn for the session.
	// If no more competitor, display the results of the session for all the competitors.
	if ((ReInfo->s->_raceType == RM_TYPE_QUALIF || ReInfo->s->_raceType == RM_TYPE_PRACTICE)
		&& !(ReInfo->s->_features & RM_FEATURE_TIMEDSESSION)) 
	{
		curDrvIdx = (int)GfParmGetNum(results, RE_SECT_CURRENT, RE_ATTR_CUR_DRIVER, NULL, 1);
		curDrvIdx++;
		if (curDrvIdx > GfParmGetEltNb(params, RM_SECT_DRIVERS)) 
		{
			GfParmSetNum(results, RE_SECT_CURRENT, RE_ATTR_CUR_DRIVER, NULL, 1);
			return ReDisplayResults();
		}
		GfParmSetNum(results, RE_SECT_CURRENT, RE_ATTR_CUR_DRIVER, NULL, (tdble)curDrvIdx);

		// Next competitor.
		return RM_SYNC | RM_NEXT_RACE;
	}

	// Otherwise, display the results of the session for all the competitors
	return ReDisplayResults();
}


int
RePostRace(void)
{
	int curRaceIdx;
	void *results = ReInfo->results;
	void *params = ReInfo->params;

	curRaceIdx = (int)GfParmGetNum(results, RE_SECT_CURRENT, RE_ATTR_CUR_RACE, NULL, 1);
	if (curRaceIdx < GfParmGetEltNb(params, RM_SECT_RACES)) {
		curRaceIdx++;
		GfLogInfo("Race Nb %d\n", curRaceIdx);
		GfParmSetNum(results, RE_SECT_CURRENT, RE_ATTR_CUR_RACE, NULL, (tdble)curRaceIdx);
		ReUpdateStandings();
		return RM_SYNC | RM_NEXT_RACE;
	}

	ReUpdateStandings();
	
	GfParmSetNum(results, RE_SECT_CURRENT, RE_ATTR_CUR_RACE, NULL, 1);
	
	return RM_SYNC | RM_NEXT_STEP;
}


int
ReEventShutdown(void)
{
	char buf[64];
	int curTrkIdx;
	void *params = ReInfo->params;
	int nbTrk;
	int ret = 0;
	void *results = ReInfo->results;
	int curRaceIdx;
	char lastRaceOfRound;
	char careerMode = FALSE;
	char first = TRUE;

	ReTrackShutdown();

	ReUI().unloadTrackGraphics();
	ReUI().shutdownGraphicsView();

	do {
		nbTrk = GfParmGetEltNb(params, RM_SECT_TRACKS);
		lastRaceOfRound = TRUE;
		curRaceIdx =(int)GfParmGetNum(results, RE_SECT_CURRENT, RE_ATTR_CUR_RACE, NULL, 1);
		curTrkIdx = (int)GfParmGetNum(results, RE_SECT_CURRENT, RE_ATTR_CUR_TRACK, NULL, 1);

		if (curRaceIdx == 1) {
			if (curTrkIdx < nbTrk) {
				// Next track.
				curTrkIdx++;
			} else if (curTrkIdx >= nbTrk) {
				// Back to the beginning.
				curTrkIdx = 1;
			}
		}

		GfParmSetNum(results, RE_SECT_CURRENT, RE_ATTR_CUR_TRACK, NULL, (tdble)curTrkIdx);

		if (strcmp(GfParmGetStr(ReInfo->mainParams, RM_SECT_SUBFILES, RM_ATTR_HASSUBFILES, RM_VAL_NO), RM_VAL_YES) == 0) {
			careerMode = TRUE;
			lastRaceOfRound = strcmp(GfParmGetStr(params, RM_SECT_SUBFILES, RM_ATTR_LASTSUBFILE, RM_VAL_YES), RM_VAL_YES) == 0;
	
			GfParmSetStr(ReInfo->mainResults, RE_SECT_CURRENT, RE_ATTR_CUR_FILE,
			GfParmGetStr(params, RM_SECT_SUBFILES, RM_ATTR_NEXTSUBFILE, ""));
			GfParmWriteFile(NULL, ReInfo->mainResults, NULL);
			/* Check if the next competition has a free weekend */
			if( !first ) {
				/* Close old params */
				GfParmWriteFile( NULL, results, NULL );
				GfParmReleaseHandle( results );
				GfParmReleaseHandle( params );
			}//if !first
			/* Open params of next race */
			params = GfParmReadFile( GfParmGetStr(ReInfo->mainResults, RE_SECT_CURRENT, RE_ATTR_CUR_FILE, "" ), GFPARM_RMODE_STD );
			if( !params )
				break;
			results = GfParmReadFile( GfParmGetStr(params, RM_SECT_SUBFILES, RM_ATTR_RESULTSUBFILE, ""), GFPARM_RMODE_STD );
			if( !results ) {
				GfParmReleaseHandle( results );
				break;
			}
	
			if (lastRaceOfRound && curTrkIdx == 1) {
				ReCareerNextSeason();
			}
			if ((int)GfParmGetNum(results, RE_SECT_CURRENT, RE_ATTR_CUR_TRACK, NULL, 1) == 1) {
				GfParmListClean(results, RE_SECT_STANDINGS);
				GfParmWriteFile(NULL, results, NULL);
			}
	
			/* Check if it is free */
			snprintf( buf, sizeof(buf), "%s/%d", RM_SECT_TRACKS,
					  (int)GfParmGetNum(results, RE_SECT_CURRENT, RE_ATTR_CUR_TRACK, NULL, 1) );
			if( !strcmp(GfParmGetStr(params, buf, RM_ATTR_NAME, "free"), "free") == 0) {
				/* Not a free weekend */
				GfParmReleaseHandle( results );
				GfParmReleaseHandle( params );
				break;
			}
			first = FALSE;
		} else {
			/* Normal case: no subfiles, so free weekends possible, so nothing to check */
			break;
		}
	} while( true );
	
	ret = (curTrkIdx != 1 || careerMode) ? RM_NEXT_RACE : RM_NEXT_STEP;

	if (nbTrk != 1) {
		ReDisplayStandings();
		return RM_ASYNC | ret;
	}
	FREEZ(ReInfo->_reCarInfo);

	return RM_SYNC | ret;
}

