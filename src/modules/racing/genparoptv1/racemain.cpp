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

#include <sstream>

#include <portability.h>

#include <robot.h>
#include <network.h>
#include <time.h>

#include "genparoptv1.h"

#include "raceutil.h" // RmGetFeaturesList
#include "racesituation.h"
#include "racecareer.h"
#include "raceinit.h"
#include "raceupdate.h"
#include "raceresults.h"
#include "racestate.h"
#include "racetrack.h" // ReTrackInit, ReTrackUpdate

#include "teammanager.h"
#include "genetic.h"

#include "racemain.h"

// DEBUG
/*
#define DIV 1024
#define WIDTH 7
DWORDLONG lastFreeMem = 0;
*/

int OptiCounter = 0;
tgenResult* TGeneticParameter::MyResults; 

typedef struct
{
	char *racename;
	int startpos;
	int endpos;
	int diffpos;
}tReGridPart;

int *ReStartingOrderIdx = NULL; //array to hold indexes of cars (in params/RM_SECT_DRIVERS) in starting order

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

int ReConfigure()
{
	ReUI().onRaceConfiguring();

	return RM_ASYNC | RM_NEXT_STEP;
}

void ReRaceAbandon()
{
	// Notify the UI that the race event is finishing now.
	ReUI().onRaceEventFinishing();

	// Shutdown track-physics-related stuff.
	ReTrackShutdown();

	// Cleanup needed stuff.
	FREEZ(ReInfo->_reCarInfo);

	if (ReInfo->params != ReInfo->mainParams)
	{
		GfParmReleaseHandle(ReInfo->params);
		ReInfo->params = ReInfo->mainParams;
	}

	// Return to race configuration step
	ReStateApply((void*)RE_STATE_CONFIG);
}

void ReRaceAbort()
{
	ReShutdownUpdaters();

	RePhysicsEngine().shutdown();
	GenParOptV1::self().unloadPhysicsEngine();

	ReUI().onRaceFinishing();

	ReRaceCleanDrivers();

	if (NetGetNetwork())
		NetGetNetwork()->Disconnect();

	FREEZ(ReInfo->_reCarInfo);
	
	if (ReInfo->params != ReInfo->mainParams)
	{
		GfParmReleaseHandle(ReInfo->params);
		ReInfo->params = ReInfo->mainParams;
	}

	// Return to race configuration step
	ReStateApply((void*)RE_STATE_CONFIG);
}

void ReRaceSkipSession()
{
	ReStateApply((void*)RE_STATE_RACE_END);
}

int
ReRaceEventInit(void)
{
	void *mainParams = ReInfo->mainParams;
	void *params = ReInfo->params;

	const bool careerMode = strcmp(GfParmGetStr(ReInfo->mainParams, RM_SECT_SUBFILES, RM_ATTR_HASSUBFILES, RM_VAL_NO), RM_VAL_YES) == 0;
	
	/* Career mode : Look if it is necessary to open another file */
	if (strcmp(GfParmGetStr(mainParams, RM_SECT_SUBFILES, RM_ATTR_HASSUBFILES, RM_VAL_NO), RM_VAL_YES) == 0)
	{
		/* Close previous params */
		if (params != mainParams)
			GfParmReleaseHandle(params);

		/* Read the new params */
		ReInfo->params = GfParmReadFile( GfParmGetStr( ReInfo->mainResults, RE_SECT_CURRENT, RE_ATTR_CUR_FILE, "" ), GFPARM_RMODE_STD );
		GfLogTrace("Career : New params file is %s (from main results file)\n",
				   GfParmGetStr( ReInfo->mainResults, RE_SECT_CURRENT, RE_ATTR_CUR_FILE, ""));
		if (!ReInfo->params)
			GfLogWarning( "Career : MainResults params weren't read correctly\n" );

		/* Close previous results */
		if (ReInfo->results != ReInfo->mainResults)
		{
			GfParmWriteFile(NULL, ReInfo->results, NULL);
			GfParmReleaseHandle(ReInfo->results);
		}

		/* Read the new results */
		ReInfo->results = GfParmReadFile( GfParmGetStr( ReInfo->params, RM_SECT_SUBFILES, RM_ATTR_RESULTSUBFILE, ""), GFPARM_RMODE_STD );
		if (!ReInfo->results)
			GfLogWarning( "Career : New results weren't read correctly\n" );
	}

	// Initialize the race session name.
	ReInfo->_reRaceName = ReGetCurrentRaceName();
	GfLogInfo("Starting new event (%s session)\n", ReInfo->_reRaceName);

	ReUI().onRaceEventInitializing();
	
	ReInfo->s->_features = RmGetFeaturesList(ReInfo->params);

	ReTrackInit();
	
	ReEventInitResults();

	const bool bGoOnLooping = ReUI().onRaceEventStarting(careerMode && !ReHumanInGroup());

	return (bGoOnLooping ? RM_SYNC : RM_ASYNC) | RM_NEXT_STEP;
}

/* parse advanced starting order strings */
/* grid part format: "sessionname[startpos:endpos]" */
/* returns: 0, when failed to parse; 1, when successfully parsed */
int
ReParseStartingOrder(const char *StartingOrder, tReGridPart **pGridList, int nCars, int &nGridList)
{
	char path[128];
	char *tempstr;
	int curRaceIdx;
	int i, nGL;
	void  *params = ReInfo->params;
	void  *results = ReInfo->results;
	tReGridPart *GridList;
	
	//input sanity check
	if ((StartingOrder == NULL) || (nCars<1)){nGridList = 0; return 0;}
	//find the number of parts, that is the number of '[' characters
	nGL = 0;
	i = 0;
	while (StartingOrder[i] != '\0') {
		if (StartingOrder[i] == '['){nGL++;}
		i++;
	}
	
	curRaceIdx = (int)GfParmGetNum(results, RE_SECT_CURRENT, RE_ATTR_CUR_RACE, NULL, 1);
	// check whether it is a name of an earlier session
	if (nGL == 0) {
		for ( i = 1; i < curRaceIdx; i++ ) {
			snprintf(path, sizeof(path), "%s/%d", RM_SECT_RACES, i);
			tempstr = GfParmGetStrNC(params, path, RM_ATTR_NAME, 0);
			if (strcmp(tempstr, StartingOrder) == 0 ) {
				GridList = new tReGridPart[1];
				if (GridList == NULL){return 0;}
				GridList[0].racename = tempstr;
				GridList[0].startpos = 1;
				GridList[0].endpos = nCars;
				GridList[0].diffpos = 1;
				nGridList = 1;
				*pGridList = GridList;
				return 1;
			}
		}
		//badly formatted GridList
		nGridList = 0;
		*pGridList = NULL;
		return 0;
	}
	
	// now try to parse it
	char *tempstr2 = new char[strlen(StartingOrder)];
	int stri;
	int GLi = 0;
	GridList = new tReGridPart[nGL];

	for (i = 0; i < nGL; i++) {
		//search for session name
		stri = 0;
		while (StartingOrder[GLi] != '[') {
			tempstr2[stri] = StartingOrder[GLi];
			stri++;
			GLi++;
		}
		tempstr2[stri] = '\0';
		GLi++;
		GridList[i].racename = NULL;
		for ( int j = 1; j < curRaceIdx; j++ ) {
			snprintf(path, sizeof(path), "%s/%d", RM_SECT_RACES, j);
			tempstr = GfParmGetStrNC(params, path, RM_ATTR_NAME, 0);
			if (strcmp(tempstr, tempstr2) == 0 ) {
				GridList[i].racename = tempstr;
				break;
			}
		}
		if (GridList[i].racename == NULL) {
			// failed to find session
			nGridList = 0;
			delete[] GridList;
			delete[] tempstr2;
			*pGridList = NULL;
			return 0;
		}
		//find indexes
		stri = 0;
		while (StartingOrder[GLi] != ']') {
			tempstr2[stri] = StartingOrder[GLi];
			stri++;
			GLi++;
		}
		tempstr2[stri] = '\0';
		GLi++;
		GridList[i].startpos = GridList[i].endpos = -1;
		sscanf(tempstr2, "%d:%d", &(GridList[i].startpos), &(GridList[i].endpos));
		if (GridList[i].startpos <= 0) {
			nGridList = 0;
			delete[] GridList;
			delete[] tempstr2;
			*pGridList = NULL;
			return 0;
		} else if (GridList[i].endpos <= 0) {
			GridList[i].endpos = GridList[i].startpos;
		}
		if (GridList[i].endpos < GridList[i].startpos)
			{GridList[i].diffpos = -1;}
		else {GridList[i].diffpos = 1;}
	}
	
	delete[] tempstr2;
	nGridList = nGL;
	*pGridList = GridList;
	return 1;
}

// Find driver position in params/RM_SECT_DRIVERS based on module name and ID
// returns with the driver position or -1 when not found or error
int
ReFindDriverIdx (const char *modulename, int idx)
{
	char path[128];
	void *params = ReInfo->params;
	
	for (int i = 1; i <= GfParmGetEltNb(params, RM_SECT_DRIVERS); i++) {
		snprintf(path, sizeof(path), "%s/%d", RM_SECT_DRIVERS, i);
		if (( (int)GfParmGetNum(params, path, RE_ATTR_IDX, NULL, 0) == idx ) && 
		     (strcmp(modulename,
			         GfParmGetStr(params, path, RE_ATTR_MODULE, "")) == 0) ) {
			//car found
			return i;
		}
	}
	//car not found
	return -1;
}


int
RePreRace(void)
{
	char path[128];
	const char *raceName;
	const char *raceType;
	void *params = ReInfo->params;
	void *results = ReInfo->results;
	int curRaceIdx;
	int timedLapsReplacement = 0;
	char *prevRaceName;
	
	raceName = ReInfo->_reRaceName = ReGetCurrentRaceName();
	
	GfParmRemoveVariable (params, "/", "humanInGroup");
	GfParmRemoveVariable (params, "/", "eventNb");
	GfParmSetVariable (params, "/", "humanInGroup", ReHumanInGroup() ? 1.0f : 0.0f);
	GfParmSetVariable (params, "/", "eventNb", GfParmGetNum (ReInfo->results, RE_SECT_CURRENT, RE_ATTR_CUR_TRACK, NULL, 1.0 ) );
	if (!raceName) {
		return RM_ERROR;
	}

	if (strcmp(GfParmGetStr(params, raceName, RM_ATTR_ENABLED, RM_VAL_YES), RM_VAL_NO) == 0) {
		GfLogTrace( "Race %s disabled\n",  raceName);
		curRaceIdx = (int)GfParmGetNum(results, RE_SECT_CURRENT, RE_ATTR_CUR_RACE, NULL, 1);
		if (curRaceIdx < GfParmGetEltNb(params, RM_SECT_RACES)) {
			curRaceIdx++;
			GfLogTrace( "Race %s is not the last one, but the #%d\n",  raceName, curRaceIdx);
			GfParmSetNum(results, RE_SECT_CURRENT, RE_ATTR_CUR_RACE, NULL, (tdble)curRaceIdx);
	
			return RM_SYNC | RM_NEXT_RACE;
		}
	
		GfParmSetNum(results, RE_SECT_CURRENT, RE_ATTR_CUR_RACE, NULL, 1);
		return RM_SYNC | RM_NEXT_RACE | RM_NEXT_STEP;
	}

	// Get session max dammages.
	ReInfo->s->_maxDammage = (int)GfParmGetNum(params, raceName, RM_ATTR_MAX_DMG, NULL, 10000);

	// Get session type (race, qualification or practice).
	raceType = GfParmGetStr(params, raceName, RM_ATTR_TYPE, RM_VAL_RACE);
	if (!strcmp(raceType, RM_VAL_RACE)) {
		ReInfo->s->_raceType = RM_TYPE_RACE;
	} else if (!strcmp(raceType, RM_VAL_QUALIF)) {
		ReInfo->s->_raceType = RM_TYPE_QUALIF;
	} else if (!strcmp(raceType, RM_VAL_PRACTICE)) {
		ReInfo->s->_raceType = RM_TYPE_PRACTICE;
	}

	// Get session duration (defaults to "All sessions" one, or else -60).
	ReInfo->s->_totTime = GfParmGetNum(params, raceName, RM_ATTR_SESSIONTIME, NULL, -1);
	if (ReInfo->s->_totTime < 0)
		ReInfo->s->_totTime = GfParmGetNum(params, RM_VAL_ANYRACE, RM_ATTR_SESSIONTIME, NULL, -60.0f);

	// Determine the actual session duration and/or number of laps.
	ReInfo->s->_extraLaps = 0; // TODO: Does this is ever needed ?
	ReInfo->s->_totLaps = 0; // Make sure it is initialized

	if (ReInfo->s->_totTime > 0 && !(ReInfo->s->_features & RM_FEATURE_TIMEDSESSION)) {
		// Timed session not supported: add 1 km for every minute in parctise or qualifying,
		// and 150 km for every hour (2.5 km for every minute) in race
		if (ReInfo->s->_raceType == RM_TYPE_RACE) {
			ReInfo->s->_totLaps = (int)floor(ReInfo->s->_totTime * 2500.0f / 60.0f / ReInfo->track->length + 0.5f);
		} else {
			ReInfo->s->_totLaps = (int)floor(ReInfo->s->_totTime * 1000.0f / 60.0f / ReInfo->track->length + 0.5f);
		}
		timedLapsReplacement = ReInfo->s->_totLaps;
		ReInfo->s->_totTime = -60.0f;
	}
	
	// Timed session doesn't exclude additional laps after the time finishes
	// Make sure that if no time set, we set far below zero
	if(ReInfo->s->_totTime <= 0.0f )
		ReInfo->s->_totTime = -60.0f;
		
	// Get session distance (defaults to "All sessions" one, or else 0).
	tdble dist = GfParmGetNum(params, raceName, RM_ATTR_DISTANCE, NULL, -1);
	if (dist < 0)
		dist = GfParmGetNum(params, RM_VAL_ANYRACE, RM_ATTR_DISTANCE, NULL, 0);
	
	// If a (> 0) session distance was specified, deduce the number of laps
	// in case the race settings don't specify it, and it is not a timed race.
	if ( (dist >= 0.001) && (ReInfo->s->_totTime < 0.0f) ) { // Why not 'if (dist > 0)' ???
		ReInfo->s->_totLaps = (int)(dist / ReInfo->track->length) + 1;
		ReInfo->s->_extraLaps = ReInfo->s->_totLaps; // Extralaps are used to find out how many laps there are after the time is up in timed sessions
	} else {dist = -1;}
	
	// Get the number of laps (defaults to "All sessions" one,
	// or else the already computed one from the session distance, or 0).
	int laps = (int)GfParmGetNum(params, raceName, RM_ATTR_LAPS, NULL, -1);
	if (laps < 0)
		laps = (int)GfParmGetNum(params, RM_VAL_ANYRACE, RM_ATTR_LAPS, NULL, 0);

	// Use lap number only when race distance is not in use.
	if ( (laps > 0) && (dist <= 0.0) && (timedLapsReplacement <= 0) ) {
		ReInfo->s->_totLaps = laps;
		ReInfo->s->_extraLaps = ReInfo->s->_totLaps; //Extralaps are used to find out how many laps there are after the time is up in timed sessions
	}

	// Make sure we have at least 1 lap race length.
	if ( (laps <= 0) && (dist <=0) && (ReInfo->s->_totTime < 0) ) {
		ReInfo->s->_totLaps = 1;
		ReInfo->s->_extraLaps = ReInfo->s->_totLaps; //Extralaps are used to find out how many laps there are after the time is up in timed sessions
	}

	// Correct extra laps (possible laps run after the winner arrived ?) :
	// during timed practice or qualification, there are none.
	if (ReInfo->s->_raceType != RM_TYPE_RACE && ReInfo->s->_totTime > 0) {
		ReInfo->s->_extraLaps = 0; //Extralaps are used to find out how many laps there are after the time is up in timed sessions
		ReInfo->s->_totLaps = 0;
	}

	GfLogInfo("Race length : time=%.0fs, laps=%d (extra=%d)\n",
			  ReInfo->s->_totTime, ReInfo->s->_totLaps, ReInfo->s->_extraLaps);
	
	// Initialize race state.
	ReInfo->s->_raceState = 0;

	// Cleanup results
	snprintf(path, sizeof(path), "%s/%s/%s", ReInfo->track->name, RE_SECT_RESULTS, raceName);
	GfParmListClean(results, path);

	// Drivers starting order
	// The starting order is decided here,
	// then car indexes are stored in ReStartingOrderIdx, in the starting order.
	// The actual grid is assembled in ReRaceStart().
	// In case of a race, when all cars start at the same time,
	// cars are simply added to the starting list in the order stored in ReStartingOrderIdx.
	// If only one car is at the track at a time (not timed session qualifying or practice),
	// the race is divided into many sub-races.
	// For a sub-race, only the results/RE_ATTR_CUR_DRIVER-th driver in ReStartingOrderIdx
	// is added to the starting grid.
	// RE_ATTR_CUR_DRIVER is refreshed after every sub-race in ReRaceEnd().
	int nCars = GfParmGetEltNb(params, RM_SECT_DRIVERS);
	GfParmListClean(params, RM_SECT_DRIVERS_RACING);
	if (nCars == 0)
	{
		// This may happen, when playing with the text-only mode,
		// and forgetting that human are automatically excluded then,
		// or when getting back to the GUI mode, and not reconfiguring the competitors list.
		GfLogError("No competitor in this race : cancelled.\n");
		return RM_ERROR;
	}
	else
	{
		ReUI().addLoadingMessage("Determining Starting Order ...");

		const char* gridType =
			GfParmGetStr(params, raceName, RM_ATTR_START_ORDER, RM_VAL_DRV_LIST_ORDER);
		
		int maxCars = (int)GfParmGetNum(params, raceName, RM_ATTR_MAX_DRV, NULL, 100);
		nCars = MIN(nCars, maxCars);
		
		tReGridPart *GridList = NULL;
		int nGridList = 0;
		
		// Initialize the array of car indexes for starting order
		if (ReStartingOrderIdx != NULL) {
			delete[] ReStartingOrderIdx;
			ReStartingOrderIdx = NULL;
		}
		ReStartingOrderIdx = new int[nCars];
		for (int i = 0; i < nCars; i++) {
			ReStartingOrderIdx[i] = -1;
		}
		
		// Starting grid in the arrival order of the previous race (or qualification session)
		if (!strcmp(gridType, RM_VAL_LAST_RACE_ORDER))
		{
			GfLogTrace("Starting grid in the order of the last race\n");
			
			prevRaceName = ReGetPrevRaceName(/* bLoop = */false);
			if (!prevRaceName) {
				return RM_ERROR;
			}

			for (int i = 1; i < nCars + 1; i++) {
				snprintf(path, sizeof(path), "%s/%s/%s/%s/%d",
						 ReInfo->track->name, RE_SECT_RESULTS, prevRaceName, RE_SECT_RANK, i);
				ReStartingOrderIdx[i-1] = 
					ReFindDriverIdx (GfParmGetStr(results, path, RE_ATTR_MODULE, ""),
									(int)GfParmGetNum(results, path, RE_ATTR_IDX, NULL, 0));
			}
		}
		
		// Starting grid in the reversed arrival order of the previous race
		else if (!strcmp(gridType, RM_VAL_LAST_RACE_RORDER))
		{
			GfLogTrace("Starting grid in the reverse order of the last race\n");

			prevRaceName = ReGetPrevRaceName(/* bLoop = */false);
			if (!prevRaceName) {
				return RM_ERROR;
			}

			for (int i = 1; i < nCars + 1; i++) {
				snprintf(path, sizeof(path), "%s/%s/%s/%s/%d",
						ReInfo->track->name, RE_SECT_RESULTS, prevRaceName, RE_SECT_RANK, nCars - i + 1);
				ReStartingOrderIdx[i-1] = 
					ReFindDriverIdx (GfParmGetStr(results, path, RE_ATTR_MODULE, ""),
									(int)GfParmGetNum(results, path, RE_ATTR_IDX, NULL, 0));
			}
		}

		// Starting grid as a mix from the results of earlier sessions
		else if (ReParseStartingOrder(gridType, &GridList, nCars, nGridList))
		{
			GfLogTrace("Starting grid as a mix from the results of earlier sessions\n");
			
			int idx;
			int gridpos = 1;
			int carnr;
			const char *modulename;
			for (int i = 0; i < nGridList; i++) {
				if (gridpos > nCars) {break;}
				if (GridList[i].diffpos == -1) {//reversed
					for ( int j = GridList[i].startpos; j >= GridList[i].endpos; j--) {
						if (gridpos > nCars) {break;}
						snprintf(path, sizeof(path), "%s/%s/%s/%s/%d",
								ReInfo->track->name, RE_SECT_RESULTS, GridList[i].racename, RE_SECT_RANK, j);
						idx = (int)GfParmGetNum(results, path, RE_ATTR_IDX, NULL, 0);
						modulename = GfParmGetStr(results, path, RE_ATTR_MODULE, "");
						carnr = ReFindDriverIdx(modulename, idx);
						for (int k = 0; k < gridpos-1; k++) {
							if ( carnr == ReStartingOrderIdx[k] ) {
								//oops: same car twice
								GfLogWarning("The same car appears twice in the advanced grid!\n");
								carnr = -1;
								break;
							}
						}
						//adding car to the list
						if (carnr != -1) {
							ReStartingOrderIdx[gridpos-1] = carnr;
							gridpos++;
						}
					}
				} else if (GridList[i].diffpos == 1){//straight order
					for ( int j = GridList[i].startpos; j <= GridList[i].endpos; j++) {
						if (gridpos > nCars) {break;}
						snprintf(path, sizeof(path), "%s/%s/%s/%s/%d",
								ReInfo->track->name, RE_SECT_RESULTS, GridList[i].racename, RE_SECT_RANK, j);
						idx = (int)GfParmGetNum(results, path, RE_ATTR_IDX, NULL, 0);
						modulename = GfParmGetStr(results, path, RE_ATTR_MODULE, "");
						carnr = ReFindDriverIdx(modulename, idx);
						for (int k = 0; k < gridpos-1; k++) {
							if ( carnr == ReStartingOrderIdx[k] ) {
								//oops: same car twice
								GfLogWarning("The same car appears twice in the advanced grid!\n");
								carnr = -1;
								break;
							}
						}
						//adding car to the list
						if (carnr != -1) {
							ReStartingOrderIdx[gridpos-1] = carnr;
							gridpos++;
						}
					}
				}
			}
			//cleaning up memory
			if (nGridList > 0){delete[] GridList;}
		}

		// Starting grid in the drivers list order
		else
		{
			GfLogTrace("Starting grid in the order of the driver list\n");

			for (int i = 1; i < nCars + 1; i++) {
				snprintf(path, sizeof(path), "%s/%d", RM_SECT_DRIVERS, i);
				ReStartingOrderIdx[i-1] = 
					ReFindDriverIdx (GfParmGetStr(params, path, RE_ATTR_MODULE, ""),
									(int)GfParmGetNum(params, path, RE_ATTR_IDX, NULL, 0));
			}
		}
	}
	
	GfParmSetNum(results, RE_SECT_CURRENT, RE_ATTR_CUR_DRIVER, NULL, 1.0);

	return RM_SYNC | RM_NEXT_STEP;
}

/* return state mode */
int
ReRaceRealStart(void)
{
	int i, j;
	tRobotItf *robot;
	tReCarInfo *carInfo;
	char buf[128];
	int foundHuman;
	void *params = ReInfo->params;
	tSituation *s = ReInfo->s;
	tMemoryPool oldPool = NULL;
	void* carHdle;

	// Load the physics engine
	if (!GenParOptV1::self().loadPhysicsEngine())
		return RM_ERROR;

	// Get the session display mode (default to "All sessions" ones, or else "normal").
	std::string strDispMode =
		GfParmGetStr(params, ReInfo->_reRaceName, RM_ATTR_DISPMODE, "");
	if (strDispMode.empty())
		strDispMode =
			GfParmGetStr(params, RM_VAL_ANYRACE, RM_ATTR_DISPMODE, RM_VAL_VISIBLE);

	if (strDispMode == RM_VAL_INVISIBLE)
		ReInfo->_displayMode = RM_DISP_MODE_NONE;
	else if (strDispMode == RM_VAL_VISIBLE)
		ReInfo->_displayMode = RM_DISP_MODE_NORMAL;
	else if (strDispMode == RM_VAL_SIMUSIMU)
		ReInfo->_displayMode = RM_DISP_MODE_SIMU_SIMU;
	else
	{
		GfLogError("Unsupported display mode '%s' loaded from race file ; "
				   "assuming 'normal'\n", strDispMode.c_str());
		ReInfo->_displayMode = RM_DISP_MODE_NORMAL;
	}

	//GfLogDebug("ReRaceRealStart: Loaded dispMode=0x%x\n", ReInfo->_displayMode);
	
	// Check if there is a human in the driver list
	foundHuman = ReHumanInGroup() ? 2 : 0;

	// Reset SimuSimu bit if any human in the race.
	// Note: Done here in order to make SimuSimu faster. 
	if ((ReInfo->_displayMode & RM_DISP_MODE_SIMU_SIMU) && foundHuman)
	{
		ReInfo->_displayMode &= ~RM_DISP_MODE_SIMU_SIMU;
	}
	
	// Initialize & place cars
	// Note: if SimuSimu display mode, robot->rbNewTrack isn't called. This is a lot faster.
	if (ReInitCars())
		return RM_ERROR;

	// Check if there is a human in the current race
	// Warning: Don't move this before ReInitCars (initializes s->cars).
	for (i = 0; i < s->_ncars; i++) {
		if (s->cars[i]->_driverType == RM_DRV_HUMAN) {
			foundHuman = 1;
			break;
		}//if human
	}//for i

	// Force "normal" display mode if any human in the session
	if (foundHuman == 1)
	{
		ReInfo->_displayMode = RM_DISP_MODE_NORMAL;
	}
	// Force "result only" mode in Practice / Qualif. sessions without any human,
	// but at least 1 in another session (why this ?), and SimuSimu bit on.
	else if (foundHuman == 2 && (ReInfo->_displayMode & RM_DISP_MODE_SIMU_SIMU)
		     && (ReInfo->s->_raceType == RM_TYPE_QUALIF
				 || ReInfo->s->_raceType == RM_TYPE_PRACTICE))
	{
		ReInfo->_displayMode = RM_DISP_MODE_NONE;
	}

	GfLogInfo("Display mode : %s\n",
			  (ReInfo->_displayMode & RM_DISP_MODE_SIMU_SIMU) ? "SimuSimu" :
			  ((ReInfo->_displayMode & RM_DISP_MODE_NORMAL) ? "Normal" : "Results-only"));
	
	// Notify the UI that it's "race loading time".
	ReUI().onRaceLoadingDrivers();

	// Load drivers for the race
	for (i = 0; i < s->_ncars; i++)
	{
		snprintf(buf, sizeof(buf), "cars/%s/%s.xml",
				 s->cars[i]->_carName, s->cars[i]->_carName);
		carHdle = GfParmReadFile(buf, GFPARM_RMODE_STD);
		snprintf(buf, sizeof(buf), "Loading %s driver (%s) ...",
				 s->cars[i]->_name, GfParmGetName(carHdle));
		
		ReUI().addLoadingMessage(buf);

		if (!(ReInfo->_displayMode & RM_DISP_MODE_SIMU_SIMU))
		{
			//Tell robots they are to start a new race
			robot = s->cars[i]->robot;
			GfPoolMove( &s->cars[i]->_newRaceMemPool, &oldPool );
			robot->rbNewRace(robot->index, s->cars[i], s);
			GfPoolFreePool( &oldPool );
		}//if ! simusimu
	}//for i
	
	RtTeamManagerStart();

	// Notify the UI that the drivers have been loaded now.
	ReUI().onRaceDriversLoaded();

	// Initialize the physics engine
	RePhysicsEngine().updateSituation(s, RCM_MAX_DT_SIMU);

	carInfo = ReInfo->_reCarInfo;
	for (i = 0; i < s->_ncars; i++) {
		carInfo[i].prevTrkPos = s->cars[i]->_trkPos;
	}

	// All cars start with max brakes on
	ReUI().addLoadingMessage("Running Prestart ...");
	
	for (i = 0; i < s->_ncars; i++)
	{
		memset(&(s->cars[i]->ctrl), 0, sizeof(tCarCtrl));
		s->cars[i]->ctrl.brakeCmd = 1.0;
	}

	for (j = 0; j < (int)(1.0 / RCM_MAX_DT_SIMU); j++)
		RePhysicsEngine().updateSituation(s, RCM_MAX_DT_SIMU);

	// Initialize current result manager.
	ReInitCurRes();

	// More initializations.
	ReInfo->_reTimeMult = 1.0;
	ReInfo->_reLastRobTime = -1.0;
	if (NetGetNetwork())
		ReInfo->s->currentTime = GfTimeClock() - NetGetNetwork()->GetRaceStartTime();
	else
		ReInfo->s->currentTime = -2.0;	// We start 2 seconds before the real race start
	ReInfo->s->deltaTime = RCM_MAX_DT_SIMU;
	ReInfo->s->_raceState = RM_RACE_STARTING;

	ReInfo->_rePitRequester = 0;
	ReInfo->_reMessage = 0;
	ReInfo->_reMessageEnd = 0.0;
	ReInfo->_reBigMessage = 0;
	ReInfo->_reBigMessageEnd = 0.0;
	
	ReInitUpdaters();

	// Notify the UI that the race simulation is ready now.
	ReUI().onRaceSimulationReady();

	// Initialize the network if needed.
	if (NetGetNetwork())
	{
		ReUI().addLoadingMessage("Preparing online race ...");
		
		NetGetNetwork()->RaceInit(ReOutputSituation()->s);
		NetGetNetwork()->SetRaceActive(true);
	}

	// Notify the UI that the race is now started.
	ReUI().addLoadingMessage("Ready.");

	ReUI().onRaceStarted();

	// And go on looping the race state automaton.
	return RM_SYNC | RM_NEXT_STEP;
}//ReRaceRealStart

/* return state mode */
int
ReRaceStart(void)
{
	char path[128];
	char path2[128];
	const char *sessionName = ReInfo->_reRaceName;
	void *params = ReInfo->params;
	void *results = ReInfo->results;
	int mode = 0;

	// Trace race session identification (more to say for the Carer mode).
	char pszSessionId[128];
	if (!strcmp(GfParmGetStr(ReInfo->mainParams, RM_SECT_SUBFILES, RM_ATTR_HASSUBFILES, RM_VAL_NO), RM_VAL_YES))
	{
		const char* pszGroup = GfParmGetStr(params, RM_SECT_HEADER, RM_ATTR_NAME, "<no group>");
		snprintf(pszSessionId, sizeof(pszSessionId), "%s %s %s", ReInfo->_reName, pszGroup, sessionName);
	}
	else
		snprintf(pszSessionId, sizeof(pszSessionId), "%s %s", ReInfo->_reName, sessionName);
	
	GfLogInfo("Starting %s session at %s\n", pszSessionId, ReInfo->track->name);

	// Reallocate and reset car info for the race.
	FREEZ(ReInfo->_reCarInfo);
	ReInfo->_reCarInfo =
		(tReCarInfo*)calloc(GfParmGetEltNb(params, RM_SECT_DRIVERS), sizeof(tReCarInfo));

	ReUI().onRaceInitializing();
	
	// Drivers starting order
	int nCars = GfParmGetEltNb(params, RM_SECT_DRIVERS);
	GfParmListClean(params, RM_SECT_DRIVERS_RACING);
	if (nCars == 0)
	{
		// This may happen, when playing with the text-only mode,
		// and forgetting that human are automatically excluded then,
		// or when getting back to the GUI mode, and not reconfiguring the competitors list.
		GfLogError("No competitor in this race : cancelled.\n");
		mode = RM_ERROR;
	}
	else if ((ReInfo->s->_raceType == RM_TYPE_QUALIF || ReInfo->s->_raceType == RM_TYPE_PRACTICE)
		&& ReInfo->s->_totTime < 0.0f /* Timed session? */)
	//Checks if there is only one driver per session allowed, so practice, qualification without timed session. 
	{
		// non-timed Qualification or Practice session => 1 driver at a time = the "current" one.
		int nCurrDrvInd =
			(int)GfParmGetNum(results, RE_SECT_CURRENT, RE_ATTR_CUR_DRIVER, NULL, 1);
		if (nCurrDrvInd <= 0)
			return RM_ERROR;

		// Propagate competitor drivers info to the real race starting grid
		snprintf(path, sizeof(path), "%s/%d", RM_SECT_DRIVERS, ReStartingOrderIdx[nCurrDrvInd-1]);
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
		// For a race, add cars to the starting grid in the order stored in ReStartingOrderIdx.
		ReUI().addLoadingMessage("Preparing Starting Grid ...");
		
		int maxCars = (int)GfParmGetNum(params, sessionName, RM_ATTR_MAX_DRV, NULL, 100);
		nCars = MIN(nCars, maxCars);
		int currDriver = -1;
		int aCars = 0;
		
		for (int i = 1; i < nCars + 1; i++)
		{
			currDriver = ReStartingOrderIdx[i-1];
			if (currDriver == -1)
				continue;
			aCars++;
			snprintf(path, sizeof(path), "%s/%d", RM_SECT_DRIVERS, currDriver);
			snprintf(path2, sizeof(path2), "%s/%d", RM_SECT_DRIVERS_RACING, i);
			GfParmSetStr(params, path2, RM_ATTR_MODULE,
						 GfParmGetStr(params, path, RE_ATTR_MODULE, ""));
			GfParmSetNum(params, path2, RM_ATTR_IDX, NULL,
						 GfParmGetNum(params, path, RE_ATTR_IDX, NULL, 0));
			GfParmSetNum(params, path2, RM_ATTR_EXTENDED, NULL,
						 GfParmGetNum(params, path, RM_ATTR_EXTENDED, NULL, 0));
			GfParmSetNum(params, path2, RM_ATTR_SKINTARGETS, NULL,
						 GfParmGetNum(params, path, RM_ATTR_SKINTARGETS, NULL, 0));
			if (GfParmGetStr(params, path, RM_ATTR_SKINNAME, 0))
				GfParmSetStr(params, path2, RM_ATTR_SKINNAME,
							 GfParmGetStr(params, path, RM_ATTR_SKINNAME, ""));
		}
		
		//no valid drivers present in the list
		if (aCars == 0)
		{
			GfLogError("No competitor in this race : cancelled.\n");
			mode = RM_ERROR;
		}
	}
	
	//ReTrackUpdate();

	if (!(mode & RM_ERROR))
	{
		// According to what the UI answers, start the race right now or not.
		mode = RM_ASYNC | RM_NEXT_STEP;
		const bool bGoOn = ReUI().onRaceStarting();
		if (bGoOn)
			mode = ReRaceRealStart();
	}
	
	return mode;
}

void ReRaceRestart()
{
	ReShutdownUpdaters();

	ReUI().onRaceFinishing();
	
	ReRaceCleanup();

	ReStateApply((void*)RE_STATE_PRE_RACE);
}

int
ReRaceStop(void)
{
	ReStop();

	ReUI().onRaceInterrupted();
	
	return RM_ASYNC | RM_NEXT_STEP;
}

int
ReRaceEnd(void)
{
	int curDrvIdx;
	int nCars;
	void *params = ReInfo->params;
	void *results = ReInfo->results;
	const char *sessionName = ReInfo->_reRaceName;
	tgenResult *MyResults = TGeneticParameter::MyResults;

//	if (genOptimization)
	{
		// Optimization ...
		MyResults->car = ReInfo->s->cars[0];
		MyResults->DamagesTotal = MyResults->car->_dammage;
		MyResults->TopSpeed = MyResults->car->_topSpeed;
//		MyResults->MinSpeed = MyResults->car->_minSpeed;
		MyResults->MinSpeed = 0;
		if (MyResults->car->_bestLapTime > 0)
			MyResults->BestLapTime = MyResults->car->_bestLapTime;
		else
			MyResults->BestLapTime = 99*60;

		MyResults->WeightedBestLapTime = MyResults->BestLapTime + MyResults->WeightOfDamages * MyResults->DamagesTotal * 0.007f;
		// ... Optimization
	}

	ReShutdownUpdaters();

	ReUI().onRaceFinishing();
	
	ReRaceCleanup();

	if (NetGetNetwork())
		NetGetNetwork()->RaceDone();

	// If we are at the end of a qualification or practice session for a competitor,
	// select the next competitor : it is his turn for the same session.
	// If no more competitor, this is the end of the session for all the competitors.
	bool bEndOfSession = true;
	if ((ReInfo->s->_raceType == RM_TYPE_QUALIF || ReInfo->s->_raceType == RM_TYPE_PRACTICE)
		&& ReInfo->s->_totTime < 0.0f)
	{
		// Get the index of the current competitor (the one who just completed his race).
		curDrvIdx = (int)GfParmGetNum(results, RE_SECT_CURRENT, RE_ATTR_CUR_DRIVER, NULL, 1);

		// Up to the next competitor now, if not the last one.
		curDrvIdx++;
		nCars = MIN(GfParmGetEltNb(params, RM_SECT_DRIVERS),
				(int)GfParmGetNum(params, sessionName, RM_ATTR_MAX_DRV, NULL, 100));
		if (curDrvIdx <= nCars) 
			bEndOfSession = false;
		else
			curDrvIdx = 1; // Was the last one : end of session !

		GfParmSetNum(results, RE_SECT_CURRENT, RE_ATTR_CUR_DRIVER, NULL, (tdble)curDrvIdx);
	}

	// Calculate class points if we just finished a session.
	if (bEndOfSession)
	{
		ReCalculateClassPoints (ReInfo->_reRaceName);
	}
	
	// Determine the new race state automation mode.
//	const bool bGoOn = ReUI().onRaceFinished(bEndOfSession);
	const bool bGoOn = true; // Always set RM_SYNC
	bEndOfSession = true;    // Always set RM_NEXT_STEP
	
	return (bEndOfSession ? RM_NEXT_STEP : RM_NEXT_RACE) | (bGoOn ? RM_SYNC : RM_ASYNC);
}


int
RePostRace(void)
{
	int curRaceIdx;
	void *results = ReInfo->results;
	void *params = ReInfo->params;

	// Prepare for next session if any left in the event.
	curRaceIdx = (int)GfParmGetNum(results, RE_SECT_CURRENT, RE_ATTR_CUR_RACE, NULL, 1);
	if (curRaceIdx < GfParmGetEltNb(params, RM_SECT_RACES)) {

		// Next session.
		curRaceIdx++;
		GfLogInfo("Next session will be #%d\n", curRaceIdx);
		GfParmSetNum(results, RE_SECT_CURRENT, RE_ATTR_CUR_RACE, NULL, (tdble)curRaceIdx);
		
		// Update standings in the results file.
		ReUpdateStandings();
		
		return RM_SYNC | RM_NEXT_RACE;
	}

	// No more session in the event : update standings in the results file.
	ReUpdateStandings();

	// Next event if any will start with its first session.
	GfParmSetNum(results, RE_SECT_CURRENT, RE_ATTR_CUR_RACE, NULL, 1);
	
	return RM_SYNC | RM_NEXT_STEP;
}


int
ReRaceEventShutdown(void)
{
	char buf[64];
	int curTrkIdx;
	void *params = ReInfo->params;
	int nbTrk;
	void *results = ReInfo->results;
	int curRaceIdx;
	bool careerMode = false;
	bool first = true;

	// Notify the UI that the race event is finishing now.
	ReUI().onRaceEventFinishing();

	// Shutdown track-physics-related stuff.
	ReTrackShutdown();

	// Determine the track of the next event to come, if not the last one
	// and, if Career mode, prepare race params / results for the next event or season.
	do {
		nbTrk = GfParmGetEltNb(params, RM_SECT_TRACKS);
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

		// Career mode.
		if (!strcmp(GfParmGetStr(ReInfo->mainParams, RM_SECT_SUBFILES, RM_ATTR_HASSUBFILES, RM_VAL_NO), RM_VAL_YES)) {
			careerMode = true;
			const bool lastRaceOfRound = strcmp(GfParmGetStr(params, RM_SECT_SUBFILES, RM_ATTR_LASTSUBFILE, RM_VAL_YES), RM_VAL_YES) == 0;

			// Previous file <= Current file.
			GfParmSetStr(ReInfo->mainResults, RE_SECT_CURRENT, RE_ATTR_PREV_FILE,
						 GfParmGetStr(ReInfo->mainResults, RE_SECT_CURRENT, RE_ATTR_CUR_FILE, ""));
			// Current file <= Next file.
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
			first = false;
		} else {
			// Normal mode (no subfiles, so free weekends possible, so nothing to check)
			break;
		}
	} while( true );

	// Determine new race state automaton mode.
	int mode = (curTrkIdx != 1 || careerMode) ? RM_NEXT_RACE : RM_NEXT_STEP;
	bool careerNonHumanGroup = careerMode && !ReHumanInGroup();

	mode |= ReUI().onRaceEventFinished(nbTrk != 1, careerNonHumanGroup) ? RM_SYNC : RM_ASYNC;;
	
	if (mode & RM_NEXT_STEP)
		FREEZ(ReInfo->_reCarInfo);

	return mode;
}


void
ReImportGeneticParameters(tgenResult *MyResults)
{
	GfLogOpt("\n\nReImportGeneticParameters\n\n");

	// Setup pointer to structure
	TGeneticParameter::MyResults = MyResults;

	// Initialize flags
	MyResults->First = true;

	// Initialize values
	MyResults->TotalWeight = 0.0;
 	MyResults->NextIdx = 0;

	MyResults->Type = 0;
	// Get race type
	// 0: Race; 1: Qualifying
    //MyResults->Type = (int) GfParmGetNum(MyResults->Handle, 
	//	"simplix private", "qualification", 0, 0);

	// Get tank capacity from car type setup file
	// Setup path to car type file
	char buf[261];
	snprintf(buf,sizeof(buf),"%scars/%s/%s.xml",
	GetDataDir(),MyResults->CarType,MyResults->CarType);
	void* Handle = GfParmReadFile(buf, GFPARM_RMODE_REREAD);
	MyResults->MaxFuel = (float) GfParmGetNum(Handle, 
		"Car", "fuel tank", "l", (float) 60.0);
	GfParmReleaseHandle(Handle);

	// Store tank capacity as initial fuel
	GfParmSetNumEx(MyResults->Handle, "simplix private", "start fuel",    // Set fuel to max
		(char*) NULL, MyResults->MaxFuel, -1.0, MyResults->MaxFuel);

	// Build path to meta data file
    snprintf(buf,sizeof(buf),"%sdrivers/%s/%s/genetic-%s.xml",
      GetLocalDir(),MyResults->RobotName,MyResults->CarType,MyResults->TrackName);

	// Read meta data file
	void *MetaDataFile = GfParmReadFile(buf, GFPARM_RMODE_REREAD);
    if (!MetaDataFile)
		assert( 0 );

	// Read table of content of meta data file
	TGeneticParameterTOC* TOC = new TGeneticParameterTOC(MetaDataFile);
	TOC->Get();

	// Store number of parameter groups defined
	MyResults->NbrOfParts = TOC->ParamsGroupCount;

	// We need at least one part to store the offset as index to the last global parameter
	// Allocate memory for list of parts
	if (MyResults->NbrOfParts < 1)
  	  MyResults->Part = new tgenPart[1];
	else
	  MyResults->Part = new tgenPart[MyResults->NbrOfParts];

	// Offset to first parameter of the first group
	MyResults->Part[0].Offset = TOC->GlobalParamCount;
	// How to handle damage as time penalty
	MyResults->WeightOfDamages = TOC->WeightOfDamages;

	// Initialize counter for number of parameters
	MyResults->NbrOfParam = TOC->GlobalParamCount;

	// Check the state we found opening the track setup file:
	// If we created an empty file, we cannot get the initial values from it!
	if (!MyResults->GetInitialVal)
		TOC->GetInitialVal = false;

	// Loop over all local parameter groups
	for (int I = 0; I < MyResults->NbrOfParts; I++)
	{
		// Read number of parameters defined in the current group
		char buf[64];
	    snprintf(buf,sizeof(buf),"part/%d/counter",I+1);
		TGeneticParameterCounter* GroupParam = new TGeneticParameterCounter(
			MetaDataFile,"Group Params Count", buf);
		GroupParam->Get(1+I);

		// Read number of sections defined in the first local parameter group
		TGeneticParameterCounter* TrackParam = new TGeneticParameterCounter(
			MyResults->Handle, "Track Params Count", TOC->Private, "track param count");

		// Store the data to the list of parts
		if (TrackParam->Parameter)
			MyResults->Part[I].Parameter = strdup(TrackParam->Parameter);
		else
			MyResults->Part[I].Parameter = NULL;
		if (GroupParam->Subsection)
			MyResults->Part[I].Subsection = strdup(GroupParam->Subsection);
		else
			MyResults->Part[I].Subsection = NULL;
		MyResults->Part[I].Active = GroupParam->Active;
		MyResults->Part[I].Count = GroupParam->Count;
		MyResults->Part[I].NbrOfSect = TrackParam->Count;
	
		MyResults->NbrOfParam += GroupParam->Count * TrackParam->Count;

		delete TrackParam;
		delete GroupParam;
	}

	// NbrOfParam now defines the total number of parameters
	// Allocate a list of pointers, one for each parameter
	// GP is the owner of the allocated memory
	MyResults->GP = (TGeneticParameter**) new TGeneticParameter*[MyResults->NbrOfParam];
	TGeneticParameter* NewGP = NULL;

	//
	// Import global parameters data
	//
	// Check section "global"
	int N = 0;
	if (GfParmExistsSection(MetaDataFile,"global"))
	{
		// Get real number of global parameters
		N = GfParmGetEltNb(MetaDataFile,"global");
	}

	// Check TOC against the real value
	if (N != TOC->GlobalParamCount)
	{
		Beep(200,200);
		TOC->GlobalParamCount = N;
	}

	// Initialize robot and car parameters
	TGeneticParameter::Reset();
    for (int I = 0; I < TOC->GlobalParamCount; I++)
	{
		NewGP = new TGeneticParameter();
		NewGP->Handle = MetaDataFile;
		// Read meta data from mete data file
//		NewGP->Get(NULL,I);  
		NewGP->Get();  
		if (NewGP->Active)			// if parameter is set active
		{
			if (TOC->GetInitialVal)	// and the flag is set
			{						// we read the starting value from the opened car setup file
				NewGP->GetVal(MyResults->Handle);
			}
			else
				TGeneticParameter::Skipped();
			//NewGP->DisplayParameter();
			// Calculate the total of the individual parameter weights to define 100% probability
			MyResults->TotalWeight = MyResults->TotalWeight + NewGP->Weight;
			// Store parameter at the owner
			MyResults->GP[MyResults->NextIdx++] = NewGP;
		}
		else 
		  delete NewGP; // If not active free memory

	}

	// This is the true offset to the first local parameter 
	// (may be different caused by inactive parameters)
	MyResults->Part[0].Offset = MyResults->NextIdx;


	// Import local parameters data

	// Loop over all parts
	for (int I = 0; I < MyResults->NbrOfParts; I++)
	{
		if (MyResults->Part[I].Active) // If the counter is set to active
		{	// we look for the details
			MyResults->Part[I].Offset = MyResults->NextIdx;

			// Get real number of local sections defined in the car setup
			int N = 0;
			char buf[64];
			snprintf(buf,sizeof(buf),"%s/%s",TOC->Private,MyResults->Part[I].Subsection);
			if (GfParmExistsSection(MyResults->Handle,buf))
			{
				// Get real number of global parameters
				N = GfParmGetEltNb(MyResults->Handle,buf);
			}

			// Check defined number against the real value
			if (N != MyResults->Part[I].NbrOfSect)
			{
				Beep(200,200);
				GfLogOpt("\n\nPart[%d].NbrOfSect = %d != %d\n\n",I,MyResults->Part[I].NbrOfSect,N);
				assert( 0 ); // TODO: Error handling
			}

			// Prepare the section depending on the part number
			snprintf(buf,sizeof(buf),"part/%d/parameter",I+1);

			// Check section defined in buf
			N = 0;
			if (GfParmExistsSection(MetaDataFile,buf))
			{
				// Get real number of global parameters
				N = GfParmGetEltNb(MetaDataFile,buf);
			}

			// Check TOC against the real value
			if (N != MyResults->Part[I].Count)
			{
				Beep(200,200);
				GfLogOpt("\n\nPart[%d].Count = %d != %d\n\n",I,MyResults->Part[I].Count,N);
				assert( 0 ); // TODO: Error handling
			}

			// Loop over all sections in the group
			for (int J = 0; J < MyResults->Part[I].NbrOfSect; J++)
			{
				// Loop over all parameters in the part
				TGeneticParameter::Reset();
				for (int K = 0; K < MyResults->Part[I].Count; K++)
				{
					NewGP = new TGeneticParameter;
					NewGP->Handle = MetaDataFile;
//					NewGP->Get(buf,K+1);
					NewGP->Get(buf);
					if (!NewGP->Active)
						NewGP->Weight = 0;
					if (TOC->GetInitialVal)	// and the flag is set
					{						// we read the starting value from the opened car setup file
						NewGP->GetVal(MyResults->Handle,true);
					}
					else
						TGeneticParameter::Skipped();

					//NewGP->DisplayParameter();
					MyResults->TotalWeight = MyResults->TotalWeight + NewGP->Weight;
					MyResults->GP[MyResults->NextIdx++] = NewGP;
				}
			}
		}
	}

	delete TOC;

	GfParmReleaseHandle(MetaDataFile);
}

void
ReEvolutionCleanup()
{
	// Setup pointer to structure
	tgenResult *MyResults = TGeneticParameter::MyResults;

	// Free all parameters allocated
	for (int I = 0; I < MyResults->NextIdx; I++)
	{
		delete MyResults->GP[I];
	}

	// Free list of pointers allocated
	delete [] MyResults->GP;

	// Free all strings allocated
	for (int I = 0; I < MyResults->NbrOfParts; I++)
	{
		free(MyResults->Part[I].Parameter);
		free(MyResults->Part[I].Subsection);
	}

	// Free list of strutures allocated
	delete [] MyResults->Part;

	// Release file handle
	GfParmReleaseHandle(MyResults->Handle);

}

int
ParameterIndex(tgenResult *MyResults, float Parameter)
{
	// Identify parameter index from total of parameter weight up to here
	float Total = 0.0;

	for (int I = 0; I < MyResults->NextIdx; I++)
	{
		Total += MyResults->GP[I]->Weight;

		// If we are in the allowed range and the parameter is not selected already
		if ((Total > Parameter) && (!MyResults->GP[I]->Selected))
			return I; // we will select it
	}

	return -1; // Parameter already selected or out of range
}

int
ReEvolution(double Scale)
{
	printf (">>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");

	/* DEBUG, ONLY FOR WINDOWS
	MEMORYSTATUSEX statex;
	statex.dwLength = sizeof (statex);
	GlobalMemoryStatusEx (&statex);
	printf (TEXT("0. There are  %*I64d KB\n"), WIDTH, lastFreeMem);
	DWORDLONG FreeMem = statex.ullAvailPhys/DIV;
	printf (TEXT("1. There are  %*I64d KB\n"), WIDTH, FreeMem);
	DWORDLONG diff = lastFreeMem - statex.ullAvailPhys/DIV;
	printf (TEXT("=. Difference %*I64d KB\n"), WIDTH, diff);
	lastFreeMem = statex.ullAvailPhys/DIV;
	_tgf_mallocBalance(); // For Debug check allocation/free balance
	*/

	// Setup shortcuts
	tgenResult *MyResults = TGeneticParameter::MyResults;
	TGeneticParameter* Param = MyResults->GP[0];
	void* Handle = MyResults->Handle; 

	// Optimization status
	int Status = -1;

	// Define local variables
	int P = 0;
	double Change; 
	double OldValue;
	double TotalLapTime = 0;

	TotalLapTime = MyResults->BestLapTime;

	if (MyResults->First) // First race was done using the initial parameters
	{
		// First race is done with the initial parameters to get the reference laptime
		GfLogOpt("Initial Lap Time : %g\n",TotalLapTime);

		// Get range for number of parameters to select for variation
		MyResults->MaxSelected = MIN(8,MyResults->NbrOfParam);
		if (MyResults->MaxSelected < 1)
			assert( 0 );
	}
	else
	{
		// Count the loops
		OptiCounter++;
	}

	/* Optimisation */
	if (TotalLapTime < MyResults->BestTotalLapTime)
	{
		Status = 2; // New opt
		GfLogOpt("New best Lap Time: %g\n",TotalLapTime);

		MyResults->BestTotalLapTime = TotalLapTime;
		MyResults->LastWeightedBestLapTime = MyResults->WeightedBestLapTime;
		MyResults->LastBestLapTime = MyResults->BestLapTime;
		MyResults->LastDamagesTotal = MyResults->DamagesTotal;	
		MyResults->LastTopSpeed = MyResults->TopSpeed;
		MyResults->LastMinSpeed = MyResults->MinSpeed;

		// Store parameters
		for (int I = 0; I < MyResults->NextIdx; I++)
			MyResults->GP[I]->LastVal = MyResults->GP[I]->OptVal = MyResults->GP[I]->Val;

		char buf[255];
		snprintf(buf,sizeof(buf),"drivers/%s/%s/%s.opt",
		  MyResults->RobotName,MyResults->CarType,MyResults->TrackName);
		GfParmWriteFileSDHeader (buf, Handle, MyResults->CarType, "Wolf-Dieter Beelitz");
		GfLogOpt("Stored to .opt\n");
	}
	else if (0.99 * TotalLapTime < MyResults->BestTotalLapTime)
	{
		Status = 1; // Next try based on the last parameters
		GfLogOpt("Total Lap Time   : %g\n",TotalLapTime);
	}
	else
	{
		Status = 0; // Next try based on the last optimal parameters
		GfLogOpt("Total Lap Time   : %g\n",TotalLapTime);

		MyResults->DamagesTotal = MyResults->LastDamagesTotal;	
		MyResults->WeightedBestLapTime = MyResults->LastWeightedBestLapTime;
		MyResults->BestLapTime = MyResults->LastBestLapTime;
		MyResults->TopSpeed = MyResults->LastTopSpeed;
		MyResults->MinSpeed = MyResults->LastMinSpeed;

		for (int I = 0; I < MyResults->NextIdx; I++)
			MyResults->GP[I]->Val = MyResults->GP[I]->OptVal;

		GfLogOpt("Back to last .opt\n");
		GfLogOpt("Old Best Lap Time: %g\n",MyResults->BestLapTime);
	}

	if (MyResults->First)
	{
		GfLogOpt("\nStart Optimisation\n");
	}

	//
	// Next Race -> try other parameters
	//
	GfLogOpt("\nRandom parameter variation\n");
	GfLogOpt("Scale: %g\n",Scale); // Show current random variation scaling

	// Reset selection flags
	for (int I = 0; I < MyResults->NextIdx; I++)
		MyResults->GP[I]->Selected = false;

	// Select random number of parameters
	double RandomFloat = (MyResults->MaxSelected * rand())/RAND_MAX;
	int N = (int) (1 + RandomFloat);

	// Loop over wanted selections
	for (int I = 0; I < N; I++)
	{
		do // Repeat until number of distinct parameters is selected
		{
			// Initialize
			Change = 0.0; 

			// Generate random variation factor
			RandomFloat = (1.0 * rand())/RAND_MAX - 0.5;
			double factor = MIN(1.0,1.1 * Scale) * RandomFloat;

			// Generate random parameter index
			RandomFloat = (1.0 * rand())/RAND_MAX;
			float Parameter = (float)((MyResults->TotalWeight - 0.00001) * RandomFloat); 

			// While first races only use global parameters
			if ((Parameter > MyResults->Part[0].Offset) && (OptiCounter < 3))
				continue;

			// Check allowed range
			if (Parameter < MyResults->TotalWeight)
			{
				// Select parameter based on probability weight
				P = ParameterIndex(MyResults,Parameter);

				// If parameter cannot be selected (to always get distinct seleted parameters)
				if (P == -1)
				{
					do // Repeat until valid selection
					{
						// try next parameter instead
						Parameter = Parameter + 1;
						// If last was taken restart
						if (Parameter > MyResults->TotalWeight)
							Parameter = Parameter - MyResults->TotalWeight;
						P = ParameterIndex(MyResults,Parameter);
					} while (P == -1); // Repeat until valid selection
				}

				//GfLogOpt("\nParameter: %g (Factor: %g) P: %d\n\n",Parameter,factor,P);

				// get parameter from index
				Param = MyResults->GP[P];

				// Statistics
				Param->Tries += 1;

				// Calculate a variation that can be stored to the xml files
				double Change0 = Param->Scale * factor; 
				Change = ((int) (Param->Round * Change0)/Param->Round);

//				GfLogOpt("%s: (%g<%g<%g): %g * %g = %g -> %g\n",Param->oLabel,Param->Min,Param->Val,Param->Max,Param->Scale,factor,Change0,Change);

				// Check allowed parameter min-max-range
				OldValue = Param->Val;
				Param->Val += (float) Change;
				if (Param->Val < Param->Min)
				{	// Use min instead
//					GfLogOpt("%s: = Min (%g)\n",Param->oLabel,Param->Val);
					Param->Val = Param->Min;
				}
				else if (Param->Val > Param->Max)
				{	// use max instead
//					GfLogOpt("%s: = Max (%g)\n",Param->oLabel,Param->Val);
					Param->Val = Param->Max;
				}
				if (fabs(OldValue - Param->Val) < 0.00000001) 
				{	// no change after reading from xml file
//					GfLogOpt("%s: Change too small %g\n",Param->oLabel,fabs(OldValue - Param->Val));
					Change = 0.0;
				}
				else
				{	// successfully changed parameter
					GfLogOpt("%s: Val: %g (Change: %g)\n",Param->oLabel,Param->Val,Change);
					Param->Selected = true;
					Param->Changed += 1;
					Param->DisplayParameter();
				}

				// Rescale
				Scale += 0.0001;
				Scale *= 1.1;
			}
			else
				Scale += 0.0001;

		} while (fabs(Change) < 0.0000001); // repeat if no change
	} // Loop over selections

/*
	for (int I = 0; I < MyResults->Part[0].Offset; I++)
	{
		if (MyResults->GP[I]->Active)
			MyResults->GP[I]->DisplayStatistik();
	}
*/
/*
	for (int I = 0; I < MyResults->Part[0].Offset; I++)
	{
		if (MyResults->GP[I]->Active)
			MyResults->GP[I]->DisplayParameter();
	}
*/
	//
	// Export global parameter data
	//
	// Loop over all global parameters
	for (int I = 0; I < MyResults->Part[0].Offset; I++)
	{
		if (MyResults->GP[I]->Active)
			MyResults->GP[I]->SetVal(Handle);
	} // Loop over all global parameters

	//
	// Export local parameter data
	//
	// Loop over all parts
	for (int I = 0; I < MyResults->NbrOfParts; I++)
	{
		if (MyResults->Part[I].Active)
		{
			// Loop over all sections
			for (int J = 0; J < MyResults->Part[I].NbrOfSect; J++)
			{
				// Loop over all parameters per section
				for (int K = 0; K < MyResults->Part[I].Count; K++)
				{
					int Index = MyResults->Part[I].Offset + MyResults->Part[I].Count * J + K;
					if (MyResults->GP[Index]->Active)
					  MyResults->GP[Index]->SetVal(Handle,J+1);
				} // Loop over all parameters per section
			} // Loop over all sections
		} // if Active
	} // Loop over all parts

	// Write parameters to xml file
	char buf[255];
	snprintf(buf,sizeof(buf),"drivers/%s/%s/%s.xml",
	MyResults->RobotName,MyResults->CarType,MyResults->TrackName);
	GfParmWriteFileSDHeader (buf, Handle, MyResults->CarType, "Wolf-Dieter Beelitz");

	printf ("<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n");

	// Reset flag
	MyResults->First = false;

	return RM_SYNC;
}


