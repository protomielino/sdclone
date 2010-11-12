/***************************************************************************

    file        : raceresults.cpp
    created     : Thu Jan  2 12:43:10 CET 2003
    copyright   : (C) 2002 by Eric Espi�                        
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

#include <ctime>
#include <vector>
#include <algorithm>
#include <string>

#include <portability.h>
#include <tgfclient.h>
#include <racescreens.h>
#include <robot.h>

#include "racesituation.h"
#include "racegl.h"
#include "racestate.h"
#include "raceresults.h"


static char buf[1024];
static char path[1024];
static char path2[1024];


typedef struct
{
	std::string drvName;
	std::string modName;
	std::string carName;
	int         extended;
	int         drvIdx;
	int         points;
} tReStandings;


void
ReInitResults(void)
{
	struct tm	*stm;
	time_t	t;
	void	*results;
	
	t = time(NULL);
	stm = localtime(&t);
	sprintf(buf, "%sresults/%s/results-%4d-%02d-%02d-%02d-%02d.xml",
		GetLocalDir(),
		ReInfo->_reFilename,
		stm->tm_year+1900,
		stm->tm_mon+1,
		stm->tm_mday,
		stm->tm_hour,
		stm->tm_min);
	
	ReInfo->results = GfParmReadFile(buf, GFPARM_RMODE_STD | GFPARM_RMODE_CREAT);
	ReInfo->mainResults = ReInfo->results;
	results = ReInfo->results;
	GfParmSetNum(results, RE_SECT_HEADER, RE_ATTR_DATE, NULL, (tdble)t);
	GfParmSetNum(results, RE_SECT_CURRENT, RE_ATTR_CUR_TRACK, NULL, 1);
	GfParmSetNum(results, RE_SECT_CURRENT, RE_ATTR_CUR_RACE, NULL, 1);
	GfParmSetNum(results, RE_SECT_CURRENT, RE_ATTR_CUR_DRIVER, NULL, 1);	
}

void
ReEventInitResults(void)
{
	int		nCars;
	int		i;
	void	*results = ReInfo->results;
	void	*params = ReInfo->params;
	
	nCars = GfParmGetEltNb(params, RM_SECT_DRIVERS);
	for (i = 1; i < nCars + 1; i++) 
	{
		sprintf(path, "%s/%s/%d", ReInfo->track->name, RM_SECT_DRIVERS, i);
		sprintf(path2, "%s/%d", RM_SECT_DRIVERS, i);
		GfParmSetStr(results, path, RE_ATTR_DLL_NAME,
					 GfParmGetStr(params, path2, RM_ATTR_MODULE, ""));
		GfParmSetNum(results, path, RE_ATTR_INDEX, NULL,
					 GfParmGetNum(params, path2, RM_ATTR_IDX, (char*)NULL, 0));
		GfParmSetNum(results, path, RM_ATTR_EXTENDED, NULL,
					 GfParmGetNum(params, path2, RM_ATTR_EXTENDED, (char*)NULL, 0));
		//GfParmSetStr(results, path, ROB_ATTR_NAME,
		//			 GfParmGetStr(params, path2, ROB_ATTR_NAME, ""));
		//GfParmSetStr(results, path, ROB_ATTR_CAR,
		//			 GfParmGetStr(params, path2, ROB_ATTR_CAR, ""));
	}
}


//for sort()
inline bool sortByScore(const tReStandings& a, const tReStandings& b)
	{return (a.points > b.points);}
	
//for find()
inline bool operator ==(const tReStandings& a, const std::string b)
	{return !a.drvName.compare(b);}

void
ReUpdateStandings(void)
{
	tReStandings st;
	std::string drvName;
	std::vector<tReStandings> *standings;
	std::vector<tReStandings>::iterator found;
	std::vector<tReStandings>::iterator it;
	int runDrv, curDrv;
	int i;
	void *results = ReInfo->results;

	sprintf(path, "%s/%s/%s/%s", ReInfo->track->name, RE_SECT_RESULTS, ReInfo->_reRaceName, RE_SECT_RANK);
	runDrv = GfParmGetEltNb(results, path);
	curDrv = GfParmGetEltNb(results, RE_SECT_STANDINGS);
	
	standings = new std::vector<tReStandings>;

	standings->reserve(curDrv);

	/* Read the current standings */
	for (i = 0; i < curDrv; i++) 
	{
		sprintf(path2, "%s/%d", RE_SECT_STANDINGS, i + 1);
		st.drvName = GfParmGetStr(results, path2, RE_ATTR_NAME, 0);
		st.modName = GfParmGetStr(results, path2, RE_ATTR_MODULE, 0);
		st.carName = GfParmGetStr(results, path2, RE_ATTR_CAR, 0);
		st.extended = (int)GfParmGetNum(results, path2, RM_ATTR_EXTENDED, NULL, 0);
		st.drvIdx  = (int)GfParmGetNum(results, path2, RE_ATTR_IDX, NULL, 0);
		st.points  = (int)GfParmGetNum(results, path2, RE_ATTR_POINTS, NULL, 0);
		standings->push_back(st);
	}//for i

	//Void the stored results
	GfParmListClean(results, RE_SECT_STANDINGS);
	
	//Checks last races' drivers and search their name in the results.
	//If found there, adds recent points.
	//If not found, adds the driver
	for (i = 0; i < runDrv; i++) {
		//Search the driver name in the standings
		sprintf(path, "%s/%s/%s/%s/%d", ReInfo->track->name, RE_SECT_RESULTS, ReInfo->_reRaceName, RE_SECT_RANK, i + 1);
		drvName = GfParmGetStr(results, path, RE_ATTR_NAME, 0);
		found = std::find(standings->begin(), standings->end(), drvName);
		
		if(found == standings->end()) {
			//No such driver in the standings, let's add it
			st.drvName = drvName;
			st.modName = GfParmGetStr(results, path, RE_ATTR_MODULE, 0);
			st.carName = GfParmGetStr(results, path, RE_ATTR_CAR, 0);
			st.extended = (int)GfParmGetNum(results, path, RM_ATTR_EXTENDED, NULL, 0);
			st.drvIdx  = (int)GfParmGetNum(results, path, RE_ATTR_IDX, NULL, 0);
			st.points  = (int)GfParmGetNum(results, path, RE_ATTR_POINTS, NULL, 0);
			standings->push_back(st);
		} else {
			//Driver found, add recent points
			found->points += (int)GfParmGetNum(results, path, RE_ATTR_POINTS, NULL, 0);
		}//if found
	}//for i
	
	//sort standings by score
	std::sort(standings->begin(), standings->end(), sortByScore);
	
	//Store the standing back
	for(it = standings->begin(), i = 0; it != standings->end(); ++it, ++i) {
		sprintf(path, "%s/%d", RE_SECT_STANDINGS, i + 1);
		GfParmSetStr(results, path, RE_ATTR_NAME, it->drvName.c_str());
		GfParmSetStr(results, path, RE_ATTR_MODULE, it->modName.c_str());
		GfParmSetStr(results, path, RE_ATTR_CAR, it->carName.c_str());
		GfParmSetNum(results, path, RE_ATTR_IDX, NULL, it->drvIdx);
		GfParmSetNum(results, path, RE_ATTR_POINTS, NULL, it->points);
	}//for it
	delete standings;
	
	char		str1[1024], str2[1024];
	sprintf(str1, "%sconfig/params.dtd", GetDataDir());
	sprintf(str2, "<?xml-stylesheet type=\"text/xsl\" href=\"file:///%sconfig/style.xsl\"?>", GetDataDir());
	
	GfParmSetDTD (results, str1, str2);
	GfParmWriteFile(0, results, "Results");
}//ReUpdateStandings

static void ReCalculateClassPoints(char const *race)
{
	double points;
	char *path3;
	int rank = 1;
	int count;

	snprintf(buf, 1024, "%s/%s/%s/%s", ReInfo->track->name, RE_SECT_RESULTS, ReInfo->_reRaceName, RE_SECT_RANK);
	path3 = strdup(buf);
	if (GfParmListSeekFirst(ReInfo->results, path3) != 0)
	{
		free(path3);
		return; /* No result found */
	}
	count = GfParmGetEltNb(ReInfo->results, path3);
	do {
		snprintf( path2, 1024, "%s/%s", race, RM_SECT_CLASSPOINTS );
		if (GfParmListSeekFirst( ReInfo->params, path2 ) != 0) {
			GfLogDebug( "First not found (path2 = %s)\n", path2 );
			continue;
		}
		do {
			snprintf( buf, 1024, "%s/%s", path2, GfParmListGetCurEltName( ReInfo->params, path2 ) );
			snprintf( path, 1024, "%s/%s/%d/%d/%s", RE_SECT_CLASSPOINTS,
			          GfParmGetCurStr (ReInfo->results, path3, RE_ATTR_MODULE, ""),
			          (int)GfParmGetCurNum (ReInfo->results, path3, RM_ATTR_EXTENDED, NULL, 0),
			          (int)GfParmGetCurNum (ReInfo->results, path3, RE_ATTR_IDX, NULL, 0),
			          GfParmGetStr( ReInfo->params, buf, RM_ATTR_SUFFIX, "" ) );
			points = GfParmGetNum (ReInfo->results, path, RE_ATTR_POINTS, NULL, 0);
			GfParmSetVariable (ReInfo->params, buf, "pos", rank);
			GfParmSetVariable (ReInfo->params, buf, "cars", count);
			GfLogDebug( "pos = %d; count = %d\n", rank, count);
			GfLogDebug( "GfParmGetNum (..., %s, %s, NULL, 0)\n", buf, RM_ATTR_POINTS );
			points += ( GfParmGetNum (ReInfo->params, buf, RM_ATTR_POINTS, NULL, 0) /
			            GfParmGetNum (ReInfo->params, RM_SECT_TRACKS, RM_ATTR_NUMBER, NULL, 1) );
			GfParmRemoveVariable (ReInfo->params, buf, "pos");
			GfParmRemoveVariable (ReInfo->params, buf, "cars");
			GfParmSetNum (ReInfo->results, path, RE_ATTR_POINTS, NULL, points);
		} while (GfParmListSeekNext( ReInfo->params, path2 ) == 0);
		++rank;
	} while (GfParmListSeekNext (ReInfo->results, path3) == 0);
	free(path3);
}

void
ReStoreRaceResults(const char *race)
{
	int		i;
	int		nCars;
	tCarElt	*car;
	tSituation 	*s = ReInfo->s;
	char	*carName;
	void	*carparam;
	void	*results = ReInfo->results;
	void	*params = ReInfo->params;
	
	/* Store the number of laps of the race */
	switch (ReInfo->s->_raceType) {
		case RM_TYPE_RACE:
			car = s->cars[0];
			if (car->_laps > s->_totLaps) car->_laps = s->_totLaps + 1;

			sprintf(path, "%s/%s/%s", ReInfo->track->name, RE_SECT_RESULTS, race);
			GfParmListClean(results, path);
			GfParmSetNum(results, path, RE_ATTR_LAPS, NULL, car->_laps - 1);
			
			for (i = 0; i < s->_ncars; i++) {
				sprintf(path, "%s/%s/%s/%s/%d", ReInfo->track->name, RE_SECT_RESULTS, race, RE_SECT_RANK, i + 1);
				car = s->cars[i];
				if (car->_laps > s->_totLaps)
					car->_laps = s->_totLaps + 1;
			
				GfParmSetStr(results, path, RE_ATTR_NAME, car->_name);
			
				sprintf(buf, "cars/%s/%s.xml", car->_carName, car->_carName);
				carparam = GfParmReadFile(buf, GFPARM_RMODE_STD);
				carName = GfParmGetName(carparam);
			
				GfParmSetStr(results, path, RE_ATTR_CAR, carName);
				GfParmSetNum(results, path, RE_ATTR_INDEX, NULL, car->index);
			
				GfParmSetNum(results, path, RE_ATTR_LAPS, NULL, car->_laps - 1);
				GfParmSetNum(results, path, RE_ATTR_TIME, NULL, car->_curTime);
				GfParmSetNum(results, path, RE_ATTR_BEST_LAP_TIME, NULL, car->_bestLapTime);
				GfParmSetNum(results, path, RE_ATTR_TOP_SPEED, NULL, car->_topSpeed);
				GfParmSetNum(results, path, RE_ATTR_DAMMAGES, NULL, car->_dammage);
				GfParmSetNum(results, path, RE_ATTR_NB_PIT_STOPS, NULL, car->_nbPitStops);
			
				GfParmSetStr(results, path, RE_ATTR_MODULE, car->_modName);
				GfParmSetNum(results, path, RE_ATTR_IDX, NULL, car->_moduleIndex);
				sprintf(path2, "%s/%d", RM_SECT_DRIVERS_RACING, car->index + 1 );
				GfParmSetNum(results, path, RM_ATTR_EXTENDED, NULL,
							 GfParmGetNum(params, path2, RM_ATTR_EXTENDED, NULL, 0));
				GfParmSetStr(results, path, ROB_ATTR_CAR, car->_carName);
				sprintf(path2, "%s/%s/%d", race, RM_SECT_POINTS, i + 1);
				GfParmSetNum(results, path, RE_ATTR_POINTS, NULL,
							 (int)GfParmGetNum(params, path2, RE_ATTR_POINTS, NULL, 0));
				if (strlen(car->_skinName) > 0)
					GfParmSetStr(results, path, RM_ATTR_SKINNAME, car->_skinName);
				GfParmSetNum(results, path, RM_ATTR_SKINTARGETS, NULL, car->_skinTargets);

				GfParmReleaseHandle(carparam);
			}
			break;
			
		case RM_TYPE_PRACTICE:
			if (s->_ncars == 1)
			{
				car = s->cars[0];
				sprintf(path, "%s/%s/%s", ReInfo->track->name, RE_SECT_RESULTS, race);
				GfParmSetStr(results, path, RM_ATTR_DRVNAME, car->_name);
				GfParmSetStr(results, path, RE_ATTR_CAR, car->_carName);
				break;
			}
			/* Otherwise, fall through */
			
		case RM_TYPE_QUALIF:
			if (s->_ncars == 1)
			{
				car = s->cars[0];
				sprintf(path, "%s/%s/%s/%s", ReInfo->track->name, RE_SECT_RESULTS, race, RE_SECT_RANK);
				nCars = GfParmGetEltNb(results, path);
				for (i = nCars; i > 0; i--) {
					sprintf(path, "%s/%s/%s/%s/%d", ReInfo->track->name, RE_SECT_RESULTS, race, RE_SECT_RANK, i);
					float opponentBestLapTime = GfParmGetNum(results, path, RE_ATTR_BEST_LAP_TIME, NULL, 0);
				
					if (car->_bestLapTime != 0.0 
						&& (car->_bestLapTime < opponentBestLapTime || opponentBestLapTime == 0.0))
					{
						/* shift */
						sprintf(path2, "%s/%s/%s/%s/%d",
								ReInfo->track->name, RE_SECT_RESULTS, race, RE_SECT_RANK, i + 1);
						GfParmSetStr(results, path2, RE_ATTR_NAME,
									 GfParmGetStr(results, path, RE_ATTR_NAME, ""));
						GfParmSetStr(results, path2, RE_ATTR_CAR,
									 GfParmGetStr(results, path, RE_ATTR_CAR, ""));
						GfParmSetNum(results, path2, RE_ATTR_BEST_LAP_TIME, NULL,
									 GfParmGetNum(results, path, RE_ATTR_BEST_LAP_TIME, NULL, 0));
						GfParmSetStr(results, path2, RE_ATTR_MODULE,
									 GfParmGetStr(results, path, RM_ATTR_MODULE, ""));
						GfParmSetNum(results, path2, RE_ATTR_IDX, NULL,
									 GfParmGetNum(results, path, RM_ATTR_IDX, NULL, 0));
						GfParmSetNum(results, path2, RM_ATTR_EXTENDED, NULL,
									 GfParmGetNum(results, path, RM_ATTR_EXTENDED, NULL, 0));
						GfParmSetStr(results, path2, ROB_ATTR_CAR,
									 GfParmGetStr(results, path, ROB_ATTR_CAR, ""));
						GfParmSetStr(results, path2, ROB_ATTR_NAME,
									 GfParmGetStr(results, path, ROB_ATTR_NAME, ""));
						sprintf(path, "%s/%s/%d", race, RM_SECT_POINTS, i + 1);
						GfParmSetNum(results, path2, RE_ATTR_POINTS, NULL,
									 (int)GfParmGetNum(params, path, RE_ATTR_POINTS, NULL, 0));
						if (GfParmGetStr(results, path, RM_ATTR_SKINNAME, 0))
							GfParmSetStr(results, path2, RM_ATTR_SKINNAME,
										 GfParmGetStr(results, path, RM_ATTR_SKINNAME, 0));
						GfParmSetNum(results, path2, RM_ATTR_SKINTARGETS, NULL,
									 GfParmGetNum(results, path, RM_ATTR_SKINTARGETS, NULL, 0));
					} else {
						break;
					}
				}
				/* insert after */
				sprintf(path, "%s/%s/%s/%s/%d", ReInfo->track->name, RE_SECT_RESULTS, race, RE_SECT_RANK, i + 1);
				GfParmSetStr(results, path, RE_ATTR_NAME, car->_name);
				
				sprintf(buf, "cars/%s/%s.xml", car->_carName, car->_carName);
				carparam = GfParmReadFile(buf, GFPARM_RMODE_STD);
				carName = GfParmGetName(carparam);
				
				GfParmSetStr(results, path, RE_ATTR_CAR, carName);
				GfParmSetNum(results, path, RE_ATTR_BEST_LAP_TIME, NULL, car->_bestLapTime);
				GfParmSetStr(results, path, RE_ATTR_MODULE, car->_modName);
				GfParmSetNum(results, path, RE_ATTR_IDX, NULL, car->_moduleIndex);
				GfParmSetStr(results, path, ROB_ATTR_CAR, car->_carName);
				GfParmSetStr(results, path, ROB_ATTR_NAME, car->_name);
				sprintf(path2, "%s/%d", RM_SECT_DRIVERS_RACING, car->index + 1 );
				GfParmSetNum(results, path, RM_ATTR_EXTENDED, NULL,
							 GfParmGetNum(params, path2, RM_ATTR_EXTENDED, NULL, 0));
				sprintf(path2, "%s/%s/%d", race, RM_SECT_POINTS, i + 1);
				GfParmSetNum(results, path, RE_ATTR_POINTS, NULL,
							 (int)GfParmGetNum(params, path2, RE_ATTR_POINTS, NULL, 0));
				if (strlen(car->_skinName) > 0)
					GfParmSetStr(results, path, RM_ATTR_SKINNAME, car->_skinName);
				GfParmSetNum(results, path, RM_ATTR_SKINTARGETS, NULL, car->_skinTargets);
			
				GfParmReleaseHandle(carparam);
				break;
			} else {
				car = s->cars[0];
	
				if (s->_totTime < 0.0f)
					GfLogWarning("Saving results of multicar non-race session, but it was not timed!\n" );
				sprintf(path, "%s/%s/%s", ReInfo->track->name, RE_SECT_RESULTS, race);
				GfParmListClean(results, path);
				GfParmSetNum(results, path, RE_ATTR_SESSIONTIME, NULL, s->_totTime);
				
				for (i = 0; i < s->_ncars; i++) {
					sprintf(path, "%s/%s/%s/%s/%d", ReInfo->track->name, RE_SECT_RESULTS, race, RE_SECT_RANK, i + 1);
					car = s->cars[i];
				
					GfParmSetStr(results, path, RE_ATTR_NAME, car->_name);
				
					sprintf(buf, "cars/%s/%s.xml", car->_carName, car->_carName);
					carparam = GfParmReadFile(buf, GFPARM_RMODE_STD);
					carName = GfParmGetName(carparam);
				
					GfParmSetStr(results, path, RE_ATTR_CAR, carName);
					GfParmSetNum(results, path, RE_ATTR_INDEX, NULL, car->index);
				
					GfParmSetNum(results, path, RE_ATTR_LAPS, NULL, car->_laps - 1);
					GfParmSetNum(results, path, RE_ATTR_TIME, NULL, car->_curTime);
					GfParmSetNum(results, path, RE_ATTR_BEST_LAP_TIME, NULL, car->_bestLapTime);
					GfParmSetNum(results, path, RE_ATTR_TOP_SPEED, NULL, car->_topSpeed);
					GfParmSetNum(results, path, RE_ATTR_DAMMAGES, NULL, car->_dammage);
					GfParmSetNum(results, path, RE_ATTR_NB_PIT_STOPS, NULL, car->_nbPitStops);
				
					GfParmSetStr(results, path, RE_ATTR_MODULE, car->_modName);
					GfParmSetNum(results, path, RE_ATTR_IDX, NULL, car->_moduleIndex);
					sprintf(path2, "%s/%d", RM_SECT_DRIVERS_RACING, car->index + 1 );
					GfParmSetNum(results, path, RM_ATTR_EXTENDED, NULL,
								 GfParmGetNum(params, path2, RM_ATTR_EXTENDED, NULL, 0));
					GfParmSetStr(results, path, ROB_ATTR_CAR, car->_carName);
					sprintf(path2, "%s/%s/%d", race, RM_SECT_POINTS, i + 1);
					GfParmSetNum(results, path, RE_ATTR_POINTS, NULL,
								 (int)GfParmGetNum(params, path2, RE_ATTR_POINTS, NULL, 0));
					if (strlen(car->_skinName) > 0)
						GfParmSetStr(results, path, RM_ATTR_SKINNAME, car->_skinName);
					GfParmSetNum(results, path, RM_ATTR_SKINTARGETS, NULL, car->_skinTargets);
			
					GfParmReleaseHandle(carparam);
				}
				break;
			}
	}
}

void
ReUpdatePracticeCurRes(tCarElt *car)
{
	ReUpdateQualifCurRes(car);
}

void
ReUpdateQualifCurRes(tCarElt *car)
{
	int		i;
	int		xx;
	int		nCars;
	int		printed;
	int		maxLines;
	void	*carparam;
	char	*carName;
	const char	*race = ReInfo->_reRaceName;
	void	*results = ReInfo->results;
	char	*tmp_str;
	double		time_left;
	

	ReResScreenSetTrackName(ReInfo->track->name);

	if (ReInfo->s->_ncars == 1)
	{
		ReResEraseScreen();
		maxLines = ReResGetLines();
		
		snprintf(buf, sizeof(buf), "cars/%s/%s.xml", car->_carName, car->_carName);
		carparam = GfParmReadFile(buf, GFPARM_RMODE_STD);
		carName = GfParmGetName(carparam);

		if (ReInfo->s->_raceType == RM_TYPE_PRACTICE)
			snprintf(buf, sizeof(buf), "%s (%s)", car->_name, carName);
		else
			snprintf(buf, sizeof(buf), "%s (%s) - Lap %d", car->_name, carName, car->_laps);
		ReResScreenSetTitle(buf);
		
		printed = 0;
		sprintf(path, "%s/%s/%s/%s", ReInfo->track->name, RE_SECT_RESULTS, race, RE_SECT_RANK);
		nCars = GfParmGetEltNb(results, path);
		nCars = MIN(nCars + 1, maxLines);
		for (i = 1; i < nCars; i++) {
			sprintf(path, "%s/%s/%s/%s/%d", ReInfo->track->name, RE_SECT_RESULTS, race, RE_SECT_RANK, i);
			if (!printed) {
				if ((car->_bestLapTime != 0.0) && (car->_bestLapTime < GfParmGetNum(results, path, RE_ATTR_BEST_LAP_TIME, NULL, 0))) {
					tmp_str = GfTime2Str(car->_bestLapTime, "  ", false, 2);
					sprintf(buf, "%d - %s - %s (%s)", i, tmp_str, car->_name, carName);
					free(tmp_str);
					ReResScreenSetText(buf, i - 1, 1);
					printed = 1;
				}
			}
			tmp_str = GfTime2Str(GfParmGetNum(results, path, RE_ATTR_BEST_LAP_TIME, NULL, 0), "  ", false, 2);
			sprintf(buf, "%d - %s - %s (%s)", i + printed, tmp_str, GfParmGetStr(results, path, RE_ATTR_NAME, ""),
			                                  GfParmGetStr(results, path, RE_ATTR_CAR, ""));
			free (tmp_str);
			ReResScreenSetText(buf, i - 1 + printed, 0);
		}
	
		if (!printed) {
			tmp_str = GfTime2Str(car->_bestLapTime, "  ", false, 2);
			sprintf(buf, "%d - %s - %s (%s)", i, tmp_str, car->_name, carName);
			free(tmp_str);
			ReResScreenSetText(buf, i - 1, 1);
		}
	
		GfParmReleaseHandle(carparam);
		ReInfo->_refreshDisplay = 1;
	}
	else
	{
		nCars = ReInfo->s->_ncars;
		if (nCars > ReResGetLines())
			nCars = ReResGetLines();
		if (ReInfo->s->_totTime > ReInfo->s->currentTime)
		{
			time_left = ReInfo->s->_totTime - ReInfo->s->currentTime;
			sprintf( buf, "%d:%02d:%02d", (int)floor( time_left / 3600.0f ), (int)floor( time_left / 60.0f ) % 60,
			         (int)floor( time_left ) % 60 );
		}
		else
		{
			sprintf( buf, "%d laps", ReInfo->s->_totLaps );
		}
		ReResScreenSetTitle(buf);
		
		for (xx = 0; xx < nCars; ++xx) {
			car = ReInfo->s->cars[ xx ];
			sprintf(buf, "cars/%s/%s.xml", car->_carName, car->_carName);
			carparam = GfParmReadFile(buf, GFPARM_RMODE_STD);
			carName = strdup(GfParmGetName(carparam));
			GfParmReleaseHandle(carparam);
			
			if (car->_state & RM_CAR_STATE_DNF) {
				sprintf(buf, "out -      - %s (%s)", car->_name, carName);
			} else if (car->_bestLapTime <= 0.0f) {
				sprintf(buf, "%d -      --:-- - %s (%s)", xx + 1, car->_name, carName);
			} else {
				if (xx == 0) {
					tmp_str = GfTime2Str(car->_bestLapTime, "  ", false, 2);
					sprintf(buf, "%d -  %s - %s (%s)", xx + 1, tmp_str, car->_name, carName);
					free(tmp_str);
				} else {
					tmp_str = GfTime2Str(car->_bestLapTime - ReInfo->s->cars[0]->_bestLapTime, "+", false, 2);
					sprintf(buf, "%d -  %s - %s (%s)", xx + 1, tmp_str, car->_name, carName);
					free(tmp_str);
				}
			}
			ReResScreenSetText(buf, xx, 0);
			FREEZ(carName);
		}
		ReInfo->_refreshDisplay = 1;
	}
}

void
ReUpdateRaceCurRes()
{
    int ncars;
    int xx;
    void *carparam;
    char *carName;
    tCarElt *car;
    char *tmp_str;
    double time_left;

	ReResScreenSetTrackName(ReInfo->track->name);
    ncars = ReInfo->s->_ncars;
    if (ncars > ReResGetLines())
    	ncars = ReResGetLines();
    if (ReInfo->s->_totTime > ReInfo->s->currentTime)
    {
    	time_left = ReInfo->s->_totTime - ReInfo->s->currentTime;
    	sprintf( buf, "%d:%02d:%02d", (int)floor( time_left / 3600.0f ), (int)floor( time_left / 60.0f ) % 60, (int)floor( time_left ) % 60 );
    }
    else
    {
    	sprintf( buf, "%d laps", ReInfo->s->_totLaps );
    }
    ReResScreenSetTitle(buf);

    for (xx = 0; xx < ncars; ++xx) {
    	car = ReInfo->s->cars[ xx ];
        sprintf(buf, "cars/%s/%s.xml", car->_carName, car->_carName);
        carparam = GfParmReadFile(buf, GFPARM_RMODE_STD);
        carName = strdup(GfParmGetName(carparam));
        GfParmReleaseHandle(carparam);
	
	if (car->_state & RM_CAR_STATE_DNF) {
	    sprintf(buf, "out -      - %s (%s)", car->_name, carName);
	} else if (car->_timeBehindLeader == 0.0f) {
	    if (xx != 0)
	        sprintf(buf, "%d -      --:-- - %s (%s)", xx + 1, car->_name, carName);
	    else
	        sprintf(buf, "%d -   %d Laps - %s (%s)", xx + 1, car->_laps - 1, car->_name, carName);
	} else {
	    if (xx == 0) {
	        sprintf(buf, "%d - %d Laps - %s (%s)", xx + 1, car->_laps - 1, car->_name, carName);
	    } else {
	        if (car->_lapsBehindLeader == 0)
		{
		    tmp_str = GfTime2Str(car->_timeBehindLeader, "  ", false, 2);
		    sprintf(buf, "%d -  %s - %s (%s)", xx + 1, tmp_str, car->_name, carName);
		    free(tmp_str);
		}
		else if (car->_lapsBehindLeader == 1)
		    sprintf(buf, "%d -     1 Lap - %s (%s)", xx + 1, car->_name, carName);
		else
		    sprintf(buf, "%d -    %d Laps - %s (%s)", xx + 1, car->_lapsBehindLeader, car->_name, carName);
	    }
	}
	ReResScreenSetText(buf, xx, 0);
	FREEZ(carName);
    }
    ReInfo->_refreshDisplay = 1;
}

void
ReSavePracticeLap(tCarElt *car)
{
    void	*results = ReInfo->results;
    tReCarInfo	*info = &(ReInfo->_reCarInfo[car->index]);

    sprintf(path, "%s/%s/%s/%d", ReInfo->track->name, RE_SECT_RESULTS, ReInfo->_reRaceName, car->_laps - 1);
    GfParmSetNum(results, path, RE_ATTR_TIME, NULL, car->_lastLapTime);
    GfParmSetNum(results, path, RE_ATTR_BEST_LAP_TIME, NULL, car->_bestLapTime);
    GfParmSetNum(results, path, RE_ATTR_TOP_SPEED, NULL, info->topSpd);
    GfParmSetNum(results, path, RE_ATTR_BOT_SPEED, NULL, info->botSpd);
    GfParmSetNum(results, path, RE_ATTR_DAMMAGES, NULL, car->_dammage);
    
}

int
ReDisplayResults(void)
{
    void	*params = ReInfo->params;
    ReCalculateClassPoints (ReInfo->_reRaceName);

    if ((!strcmp(GfParmGetStr(params, ReInfo->_reRaceName, RM_ATTR_DISPRES, RM_VAL_YES), RM_VAL_YES)) ||
	(ReInfo->_displayMode == RM_DISP_MODE_NORMAL)) {
	RmShowResults(ReInfo->_reGameScreen, ReInfo);
    } else 
    {
    	return RM_SYNC | RM_NEXT_STEP;
	//ReResShowCont();
    }

    return RM_ASYNC | RM_NEXT_STEP;
}


void
ReDisplayStandings(void)
{
    RmShowStandings(ReInfo->_reGameScreen, ReInfo);
}
