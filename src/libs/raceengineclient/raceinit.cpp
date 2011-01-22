/***************************************************************************

    file        : raceinit.cpp
    created     : Sat Nov 16 10:34:35 CET 2002
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
        
    @author <a href=mailto:eric.espie@torcs.org>Eric Espie</a>
    @version  $Id$
*/


/* Race initialization routines */

#include <cstdlib>
#include <cstdio>
#include <string>
#include <map>

#include <portability.h>
#include <raceman.h>
#include <robot.h>
#include <teammanager.h>
#include <timeanalysis.h> // RtTimeStamp, RtDuration ; TODO: Use GfSleep (after moving GfuiSleep to tgf)
#include <robottools.h>
#include <racemanagers.h>

#include <racescreens.h>

#include "racesituation.h"
#include "racemain.h"
#include "racestate.h"
#include "racegl.h"
#include "raceresults.h"
#include "racecareer.h"

#include "raceinit.h"


static const char *level_str[] = { ROB_VAL_ROOKIE, ROB_VAL_AMATEUR, ROB_VAL_SEMI_PRO, ROB_VAL_PRO };

static tModList *reEventModList = 0;

tModList *ReRaceModList = 0;


/* Race Engine Initialization */
void
ReInit(void)
{
  const char *dllname;
  char key[256];
  tRmMovieCapture *capture;

  ReShutdown();

  ReInfo = (tRmInfo *)calloc(1, sizeof(tRmInfo));
  ReInfo->s = (tSituation *)calloc(1, sizeof(tSituation));
  ReInfo->modList = &ReRaceModList;

  char buf[512];
  snprintf(buf, sizeof(buf), "%s%s", GetLocalDir(), RACE_ENG_CFG);

  ReInfo->_reParam = GfParmReadFile(buf, GFPARM_RMODE_REREAD | GFPARM_RMODE_CREAT);

  GfLogInfo("Loading Track Loader ...\n");
  dllname = GfParmGetStr(ReInfo->_reParam, "Modules", "track", "");
  snprintf(key, sizeof(key), "%smodules/track/%s.%s", GetLibDir (), dllname, DLLEXT);
  if (GfModLoad(0, key, &reEventModList))
	  return;
  reEventModList->modInfo->fctInit(reEventModList->modInfo->index, &ReInfo->_reTrackItf);

  /* The graphic modules isn't loaded at this moment, so make sure _reGraphicItf equals NULL */
  memset (&ReInfo->_reGraphicItf, 0, sizeof(tGraphicItf));

  capture = &(ReInfo->movieCapture);
  if (!strcmp(GfParmGetStr(ReInfo->_reParam, RM_SECT_MOVIE_CAPTURE, RM_ATT_CAPTURE_ENABLE, "no"), "no")){
    capture->enabled = 0;
    capture->outputBase = 0;
    GfLogInfo("Movie capture disabled\n");
  } else {
    capture->enabled = 1;
    capture->state = 0;
    capture->deltaFrame = 1.0 / GfParmGetNum(ReInfo->_reParam, RM_SECT_MOVIE_CAPTURE, RM_ATT_CAPTURE_FPS, NULL, 25.0);
    capture->deltaSimu = RCM_MAX_DT_SIMU;
    char pszDefOutputBase[256];
    snprintf(pszDefOutputBase, sizeof(pszDefOutputBase), "%s%s",
        GetLocalDir(), GfParmGetStr(ReInfo->_reParam, RM_SECT_MOVIE_CAPTURE,
                      RM_ATT_CAPTURE_OUT_DIR, "captures"));
    capture->outputBase = strdup(pszDefOutputBase);
    GfDirCreate(pszDefOutputBase); // In case not already done.
    GfLogInfo("Movie capture enabled (%.0f FPS, PNG frames in %s)\n", 
			  1.0 / capture->deltaFrame, capture->outputBase);
  }

  ReInfo->_reGameScreen = ReHookInit();
}


/* Race Engine Exit */
void ReShutdown(void)
{
  /* Free previous situation */
  if (ReInfo) {
    ReInfo->_reTrackItf.trkShutdown();

    GfModUnloadList(&reEventModList);

    memset( &ReInfo->_reGraphicItf, 0, sizeof(tGraphicItf) );

    if (ReInfo->results) 
    {
          if (ReInfo->mainResults != ReInfo->results)
        GfParmReleaseHandle(ReInfo->mainResults);
            GfParmReleaseHandle(ReInfo->results);
    }
    if (ReInfo->_reParam) {
      GfParmReleaseHandle(ReInfo->_reParam);
    }
    if (ReInfo->params != ReInfo->mainParams) 
    {
      GfParmReleaseHandle(ReInfo->params);
      ReInfo->params = ReInfo->mainParams;
    }
    if (ReInfo->movieCapture.outputBase)
      free(ReInfo->movieCapture.outputBase);
    free(ReInfo->s);
    free(ReInfo->carList);
    free(ReInfo->rules);
    
    FREEZ(ReInfo);
    }
}


void
ReStartNewRace(void * /* dummy */)
{
  if (strcmp(GfParmGetStr(ReInfo->params, RM_SECT_SUBFILES, RM_ATTR_HASSUBFILES, RM_VAL_NO), RM_VAL_NO) == 0)
    ReInitResults();
  else
    ReCareerNew();
  ReStateManage();
}


// Start configuring a race for the given manager
void
ReRaceConfigure(GfRaceManager* pRaceman)
{
  ReInfo->mainParams = ReInfo->params = pRaceman->getDescriptorHandle();
  ReInfo->_reName = pRaceman->getName().c_str();
  ReInfo->_reFilename = pRaceman->getId().c_str();

  std::string strFullType(pRaceman->getType());
  if (!pRaceman->getSubType().empty())
  {
	  strFullType += " / ";
	  strFullType += pRaceman->getSubType();
  }
  GfLogTrace("%s selected\n", strFullType.c_str());
  
  // Enter CONFIG state.
  ReStateApply(RE_STATE_CONFIG);
}

/*
 * Function
 *  initStartingGrid
 *
 * Description
 *  Place the cars on the starting grid
 *
 * Parameters
 *  Race Information structure initialized
 *
 * Return
 *  none
 */
static void
initStartingGrid(void)
{
  char path[64];
  int i;
  tTrackSeg *curseg;
  int rows;
  tdble a, b, wi2;
  tdble d1, d2,d3;
  tdble startpos, tr, ts;
  tdble speedInit;
  tdble heightInit;
  tCarElt *car;
  const char *pole;
  void *trHdle = ReInfo->track->params;
  void *params = ReInfo->params;

  snprintf(path, sizeof(path), "%s/%s", ReInfo->_reRaceName, RM_SECT_STARTINGGRID);

  /* Search for the first turn for find the pole side */
  curseg = ReInfo->track->seg->next;
  while (curseg->type == TR_STR) {
    /* skip the straight segments */
    curseg = curseg->next;
  }
  /* Set the pole for the inside of the first turn */
  if (curseg->type == TR_LFT) {
    pole = GfParmGetStr(params, path, RM_ATTR_POLE, "left");
  } else {
    pole = GfParmGetStr(params, path, RM_ATTR_POLE, "right");
  }
  /* Tracks definitions can force the pole side */
  pole = GfParmGetStr(trHdle, RM_SECT_STARTINGGRID, RM_ATTR_POLE, pole);

  if (strcmp(pole, "left") == 0) {
    a = ReInfo->track->width;
    b = -a;
  } else {
    a = 0;
    b = ReInfo->track->width;
  }
  wi2 = ReInfo->track->width * 0.5;

  rows = (int)GfParmGetNum(params, path, RM_ATTR_ROWS, (char*)NULL, 2);
  rows = (int)GfParmGetNum(trHdle, RM_SECT_STARTINGGRID, RM_ATTR_ROWS, (char*)NULL, rows);
  d1 = GfParmGetNum(params, path, RM_ATTR_TOSTART, (char*)NULL, 10);
  d1 = GfParmGetNum(trHdle, RM_SECT_STARTINGGRID, RM_ATTR_TOSTART, (char*)NULL, d1);
  d2 = GfParmGetNum(params, path, RM_ATTR_COLDIST, (char*)NULL, 10);
  d2 = GfParmGetNum(trHdle, RM_SECT_STARTINGGRID, RM_ATTR_COLDIST, (char*)NULL, d2);
  d3 = GfParmGetNum(params, path, RM_ATTR_COLOFFSET, (char*)NULL, 5);
  d3 = GfParmGetNum(trHdle, RM_SECT_STARTINGGRID, RM_ATTR_COLOFFSET, (char*)NULL, d3);
  speedInit = GfParmGetNum(params, path, RM_ATTR_INITSPEED, (char*)NULL, 0.0);
  heightInit = GfParmGetNum(params, path, RM_ATTR_INITHEIGHT, (char*)NULL, 0.3);
  heightInit = GfParmGetNum(trHdle, RM_SECT_STARTINGGRID, RM_ATTR_INITHEIGHT, (char*)NULL, heightInit);

  if (rows < 1) {
    rows = 1;
  }
  for (i = 0; i < ReInfo->s->_ncars; i++) {
    car = &(ReInfo->carList[i]);
    car->_speed_x = speedInit;
    startpos = ReInfo->track->length - (d1 + (i / rows) * d2 + (i % rows) * d3);
    tr = a + b * ((i % rows) + 1) / (rows + 1);
    curseg = ReInfo->track->seg;  /* last segment */
    while (startpos < curseg->lgfromstart) {
      curseg = curseg->prev;
    }
    ts = startpos - curseg->lgfromstart;
    car->_trkPos.seg = curseg;
    car->_trkPos.toRight = tr;
    switch (curseg->type) {
      case TR_STR:
        car->_trkPos.toStart = ts;
        RtTrackLocal2Global(&(car->_trkPos), &(car->_pos_X), &(car->_pos_Y), TR_TORIGHT);
        car->_yaw = curseg->angle[TR_ZS];
        break;
      case TR_RGT:
        car->_trkPos.toStart = ts / curseg->radius;
        RtTrackLocal2Global(&(car->_trkPos), &(car->_pos_X), &(car->_pos_Y), TR_TORIGHT);
        car->_yaw = curseg->angle[TR_ZS] - car->_trkPos.toStart;
        break;
      case TR_LFT:
        car->_trkPos.toStart = ts / curseg->radius;
        RtTrackLocal2Global(&(car->_trkPos), &(car->_pos_X), &(car->_pos_Y), TR_TORIGHT);
        car->_yaw = curseg->angle[TR_ZS] + car->_trkPos.toStart;
        break;
    }
    car->_pos_Z = RtTrackHeightL(&(car->_trkPos)) + heightInit;

    NORM0_2PI(car->_yaw);
    ReInfo->_reSimItf.config(car, ReInfo);
  }
}


static void
initPits(void)
{
  tTrackPitInfo *pits;
  int i, j;

  /*
  typedef std::map<std::string, int> tTeamsMap;
  typedef tTeamsMap::const_iterator tTeamsMapIterator;
  tTeamsMap teams;
  tTeamsMapIterator teamsIterator;

  // create a list with the teams, a pit can just be used by one team.
  for (i = 0; i < ReInfo->s->_ncars; i++) {
    tCarElt *car = &(ReInfo->carList[i]);
    teams[car->_teamname] = teams[car->_teamname] + 1; 
  } 

  for (teamsIterator = teams.begin(); teamsIterator != teams.end(); ++teamsIterator) {
    GfLogDebug("----------------- %s\t%d\n", (teamsIterator->first).c_str(), teamsIterator->second); 
  }
  */

  // How many cars are sharing a pit?
  int carsPerPit = (int) GfParmGetNum(ReInfo->params, ReInfo->_reRaceName, RM_ATTR_CARSPERPIT, NULL, 1.0f);
  if (carsPerPit < 1) {
    carsPerPit = 1;
  } else if (carsPerPit > TR_PIT_MAXCARPERPIT) {
    carsPerPit = TR_PIT_MAXCARPERPIT;
  }
  GfLogInfo("Cars per pit: %d\n", carsPerPit);

  switch (ReInfo->track->pits.type) {
    case TR_PIT_ON_TRACK_SIDE:
      pits = &(ReInfo->track->pits);
      pits->driversPitsNb = ReInfo->s->_ncars;
      pits->carsPerPit = carsPerPit;

      // Initialize pit data for every pit, necessary because of restarts
      // (track gets not reloaded, no calloc).
      for (i = 0; i < pits->nMaxPits; i++) {
        tTrackOwnPit *pit = &(pits->driversPits[i]);
        pit->freeCarIndex = 0;
        pit->pitCarIndex = TR_PIT_STATE_FREE;
        for (j = 0; j < TR_PIT_MAXCARPERPIT; j++) {
          pit->car[j] = NULL;
        }
      }

      // Assign cars to pits. Inefficient (O(n*n)), but just at initialization, so do not care.
      // One pit can just host cars of one team (this matches with the reality) 
      for (i = 0; i < ReInfo->s->_ncars; i++) {
        // Find pit for the cars team.
        tCarElt *car = &(ReInfo->carList[i]);
        for (j = 0; j < pits->nMaxPits; j++) {
          tTrackOwnPit *pit = &(pits->driversPits[j]);
          // Put car in this pit if the pit is unused or used by a teammate and there is
          // space left.
          if (pit->freeCarIndex <= 0 ||
            (strcmp(pit->car[0]->_teamname, car->_teamname) == 0 && pit->freeCarIndex < carsPerPit))
          {
            // Assign car to pit.
            pit->car[pit->freeCarIndex] = car;
            // If this is the first car set up more pit values, assumtion: the whole team
            // uses the same car. If not met it does not matter much, but the car might be
            // captured a bit too easy or too hard.
            if (pit->freeCarIndex == 0) {
              pit->pitCarIndex = TR_PIT_STATE_FREE;
              pit->lmin = pit->pos.seg->lgfromstart + pit->pos.toStart - pits->len / 2.0 + car->_dimension_x / 2.0;
              if (pit->lmin > ReInfo->track->length) {
                pit->lmin -= ReInfo->track->length;
              }
              pit->lmax = pit->pos.seg->lgfromstart + pit->pos.toStart + pits->len / 2.0 - car->_dimension_x / 2.0;
              if (pit->lmax > ReInfo->track->length) {
                pit->lmax -= ReInfo->track->length;
              }
            }
            (pit->freeCarIndex)++;
            ReInfo->carList[i]._pit = pit;
            // Assigned, continue with next car.
            break;
          }
        }
      }

      // Print out assignments.
      for (i = 0; i < pits->nMaxPits; i++) {
        tTrackOwnPit *pit = &(pits->driversPits[i]);
        for (j = 0; j < pit->freeCarIndex; j++) {
          if (j == 0) {
            GfLogTrace("Pit %d, Team: %s, ", i, pit->car[j]->_teamname); 
          }
          GfLogTrace("%d: %s ", j, pit->car[j]->_name);
        }
        if (j > 0) {
          GfOut("\n");
        }
      }

      break;
    case TR_PIT_ON_SEPARATE_PATH:
      break;
    case TR_PIT_NONE:
      break;
  }
}

/**
 * Function to load a car.
 *
 * @param carindex The index whichs will be used as car->index for the car.
 * @param listindex The listindex in RM_SECT_DRIVERS_RACING
 * @param modindex The index of the mod; must be MAX_MOD_ITF if normal_carname is FALSE.
 * @param robotIdx The index of the robot.
 * @param normal_carname If this member is TRUE, the car is treated as an ordinary car;
 *                       if this member is FALSE, then the car used is the one given
 *                       in the xml-file, and there is no restriction on the number of instances.
 * @param cardllname The dllname of the driver
 * @return A pointer to the newly created car if successfull; NULL otherwise
 */
static tCarElt* reLoadSingleCar( int carindex, int listindex, int modindex, int relativeRobotIdx, char normal_carname, char const *cardllname )
{
  tCarElt *elt;
  tMemoryPool oldPool;
  char path[256];
  char path2[256];
  char buf[256];
  char buf2[256];
  char const *str;
  char const *category;
  char const *teamname;
  tModInfoNC *curModInfo;
  tRobotItf *curRobot;
  void *robhdle;
  void *cathdle;
  void *carhdle;
  void *handle;
  int k;
  int xx;
  char isHuman;
  int robotIdx = relativeRobotIdx;

  /* good robot found */
  curModInfo = &((*(ReInfo->modList))->modInfo[modindex]);
  GfLogInfo("Driver's name: %s\n", curModInfo->name);

  isHuman = strcmp( cardllname, "human" ) == 0 || strcmp( cardllname, "networkhuman" ) == 0;

  if (!normal_carname && !isHuman) /*Extended is forced for humans, so no need to increase robotIdx*/
    robotIdx += curModInfo->index;

  /* Retrieve the driver interface (function pointers) */
  curRobot = (tRobotItf*)calloc(1, sizeof(tRobotItf));
  /* ... and initialize the driver */
  if (ReInfo->_displayMode != RM_DISP_MODE_SIMU_SIMU) {
    curModInfo->fctInit(robotIdx, (void*)(curRobot));
  } else {
    curRobot->rbNewTrack = NULL;
    curRobot->rbNewRace  = NULL;
    curRobot->rbDrive    = NULL;
    curRobot->rbPitCmd   = NULL;
    curRobot->rbEndRace  = NULL;
    curRobot->rbShutdown = NULL;
    curRobot->index      = 0;
  }
  /* Retrieve and load the driver type XML file :
     1) from user settings dir (local dir)
     2) from installed data dir */
  snprintf(buf, sizeof(buf), "%sdrivers/%s/%s.xml", GetLocalDir(), cardllname, cardllname);
  robhdle = GfParmReadFile(buf, GFPARM_RMODE_STD);
  if (!robhdle) {
    snprintf(buf, sizeof(buf), "drivers/%s/%s.xml", cardllname, cardllname);
    robhdle = GfParmReadFile(buf, GFPARM_RMODE_STD);
  }

  if (normal_carname || isHuman)
    snprintf(path, sizeof(path), "%s/%s/%d", ROB_SECT_ROBOTS, ROB_LIST_INDEX, robotIdx);
  else
    snprintf(path, sizeof(path), "%s", ROB_SECT_ARBITRARY);
  
  /* Load car/driver info (in race engine data structure) */
  if (robhdle)
  {
    elt = &(ReInfo->carList[carindex]);
    GF_TAILQ_INIT(&(elt->_penaltyList));

    const std::string strDType = GfParmGetStr(robhdle, path, ROB_ATTR_TYPE, ROB_VAL_ROBOT);
    if (strDType == ROB_VAL_ROBOT){
      elt->_driverType = RM_DRV_ROBOT;
      elt->_networkPlayer = 0;
    } 
    else if (strDType == ROB_VAL_HUMAN)
    {
      elt->_driverType = RM_DRV_HUMAN;
      std::string strNetPlayer = GfParmGetStr(robhdle, path, "networkrace", "no");
      elt->_networkPlayer = (strNetPlayer == "yes") ? 1 : 0;
    }

    elt->index = carindex;
    elt->robot = curRobot;
    elt->_paramsHandle = robhdle;
    elt->_driverIndex = robotIdx;
    elt->_moduleIndex = relativeRobotIdx;
    strncpy(elt->_modName, cardllname, MAX_NAME_LEN - 1);
    elt->_modName[MAX_NAME_LEN - 1] = 0;

    //snprintf(path, sizeof(path), "%s/%s/%d", ROB_SECT_ROBOTS, ROB_LIST_INDEX, robotIdx);
    snprintf( path2, 256, "%s/%s/%d/%d", RM_SECT_DRIVERINFO, elt->_modName, normal_carname ? 0 : 1, elt->_moduleIndex );
    if (normal_carname || elt->_driverType == RM_DRV_HUMAN)
      strncpy(elt->_name, GfParmGetStr(robhdle, path, ROB_ATTR_NAME, "none"), MAX_NAME_LEN - 1);
    else
      strncpy(elt->_name, GfParmGetStr(ReInfo->params, path2, ROB_ATTR_NAME, "none"), MAX_NAME_LEN - 1);
    elt->_name[MAX_NAME_LEN - 1] = 0;

    teamname = GfParmGetStr(robhdle, path, ROB_ATTR_TEAM, "none");
    teamname = GfParmGetStr(ReInfo->params, path2, ROB_ATTR_TEAM, teamname ); //Use the name in params if it has a team name
    strncpy(elt->_teamname, teamname, MAX_NAME_LEN - 1);
    elt->_teamname[MAX_NAME_LEN - 1] = 0;

    elt->_driveSkill = GfParmGetNum(ReInfo->params, path2, RM_ATTR_SKILLLEVEL, NULL, -1.0f);

    if (normal_carname) /* Even if we get a normal_carname for humans we use it despite of forced extended mode*/
      strncpy(elt->_carName, GfParmGetStr(robhdle, path, ROB_ATTR_CAR, ""), MAX_NAME_LEN - 1);
    else
      strncpy(elt->_carName, GfParmGetStr(ReInfo->params, path2, RM_ATTR_CARNAME, ""), MAX_NAME_LEN - 1);
    elt->_carName[MAX_NAME_LEN - 1] = 0; /* XML file name */
    
    // Load custom skin name and targets from race info (if specified).
    snprintf(path2, sizeof(path2), "%s/%d", RM_SECT_DRIVERS_RACING, listindex);
    if (GfParmGetStr(ReInfo->params, path2, RM_ATTR_SKINNAME, 0))
    {
      strncpy(elt->_skinName, GfParmGetStr(ReInfo->params, path2, RM_ATTR_SKINNAME, ""), MAX_NAME_LEN - 1);
      elt->_skinName[MAX_NAME_LEN - 1] = 0; // Texture name
    }
	elt->_skinTargets = (int)GfParmGetNum(ReInfo->params, path2, RM_ATTR_SKINTARGETS, (char*)NULL, 0);
	
    // Load other data from robot descriptor.
    elt->_raceNumber = (int)GfParmGetNum(robhdle, path, ROB_ATTR_RACENUM, (char*)NULL, 0);
    if (!normal_carname && elt->_driverType != RM_DRV_HUMAN) // Increase racenumber if needed
      elt->_raceNumber += elt->_moduleIndex;
    elt->_skillLevel = 0;
    str = GfParmGetStr(robhdle, path, ROB_ATTR_LEVEL, ROB_VAL_SEMI_PRO);
    for(k = 0; k < (int)(sizeof(level_str)/sizeof(char*)); k++) {
      if (strcmp(level_str[k], str) == 0) {
        elt->_skillLevel = k;
        break;
      }
    }
    elt->_startRank  = carindex;
    elt->_pos        = carindex+1;
    elt->_remainingLaps = ReInfo->s->_totLaps;

    elt->_newTrackMemPool = NULL;
    elt->_newRaceMemPool = NULL;
    elt->_endRaceMemPool = NULL;
    elt->_shutdownMemPool = NULL;

    GfLogTrace("Driver #%d(%d) : module='%s', name='%s', car='%s', cat='%s', skin='%s' on %x\n",
			   carindex, listindex, elt->_modName, elt->_name, elt->_carName,
			   elt->_category, elt->_skinName, elt->_skinTargets);
  
    /* Retrieve and load car specs : merge car default specs,
       category specs and driver modifications (=> handle) */
    /* Read Car model specifications */
    snprintf(buf, sizeof(buf), "cars/%s/%s.xml", elt->_carName, elt->_carName);
    carhdle = GfParmReadFile(buf, GFPARM_RMODE_STD | GFPARM_RMODE_CREAT);
    category = GfParmGetStr(carhdle, SECT_CAR, PRM_CATEGORY, NULL);
    GfLogTrace("Loading/merging %s specs for %s ...\n", elt->_carName, curModInfo->name);
    if (category) {
      strncpy(elt->_category, category, MAX_NAME_LEN - 1);
      elt->_category[MAX_NAME_LEN - 1] = 0;
      /* Read Car Category specifications */
      snprintf(buf2, sizeof(buf2), "categories/%s.xml", elt->_category);
      cathdle = GfParmReadFile(buf2, GFPARM_RMODE_STD | GFPARM_RMODE_CREAT);
	  int errorcode = 0;
      if (errorcode = GfParmCheckHandle(cathdle, carhdle)) 
	  {
        switch (errorcode)
		{
		  case -1:
            GfLogError("Car %s NOT in category %s (driver %s) !!!\n", elt->_carName, category, elt->_name);
		    break;

		  case -2:
            GfLogError("Parameters out of bound for car %s (driver %s)!!!\n",elt->_carName, elt->_name);
		    break;

		  case -3:
            GfLogError("Parameter not allowed for car %s (driver %s)!!!\n",elt->_carName, elt->_name);
		    break;

		  default:
            GfLogError("Unknown error for %s (driver %s)!!!\n",elt->_carName, elt->_name);
		    break;
	    } 
        return NULL;
      }
      carhdle = GfParmMergeHandles(cathdle, carhdle,
                                   GFPARM_MMODE_SRC | GFPARM_MMODE_DST | GFPARM_MMODE_RELSRC | GFPARM_MMODE_RELDST);
	  
      /* The code below stores the carnames to a separate xml-file
		 such that at newTrack it is known which car is used.
		 TODO: find a better method for this */
      snprintf (buf, sizeof(buf), "%sdrivers/curcarnames.xml", GetLocalDir());
      handle = GfParmReadFile(buf, GFPARM_RMODE_CREAT);
      if (handle) {
        snprintf(path, sizeof(path), "drivers/%s/%d", cardllname, elt->_driverIndex);
        GfParmSetStr (handle, path, RM_ATTR_CARNAME, elt->_carName);
        GfParmWriteFile (0, handle, "Car names");
        GfParmReleaseHandle (handle);
      }
      if (ReInfo->_displayMode != RM_DISP_MODE_SIMU_SIMU)
      {
        GfPoolMove(&elt->_newTrackMemPool, &oldPool);
        curRobot->rbNewTrack(elt->_driverIndex, ReInfo->track, carhdle, &handle, ReInfo->s);
        GfPoolFreePool( &oldPool );
      }
      else
        handle = NULL;
      if (handle) {
        if (GfParmCheckHandle(carhdle, handle)) {
          GfLogError("Bad Car parameters for driver %s\n", elt->_name);
          return NULL;
        }
        handle = GfParmMergeHandles(carhdle, handle,
                                    GFPARM_MMODE_SRC | GFPARM_MMODE_DST | GFPARM_MMODE_RELSRC | GFPARM_MMODE_RELDST);
      } else {
        handle = carhdle;
      }
      elt->_carHandle = handle;

      /* Initialize sectors */
      elt->_currentSector = 0;
      elt->_curSplitTime = (double*)malloc( sizeof(double) * ( ReInfo->track->numberOfSectors - 1 ) );
      elt->_bestSplitTime = (double*)malloc( sizeof(double) * ( ReInfo->track->numberOfSectors - 1 ) );
      for (xx = 0; xx < ReInfo->track->numberOfSectors - 1; ++xx)
      {
        elt->_curSplitTime[xx] = -1.0f;
        elt->_bestSplitTime[xx] = -1.0f;
      }
    } else {
      elt->_category[ 0 ] = '\0';
      GfLogError("Bad Car category for driver %s\n", elt->_name);
      return NULL;
    }

    return elt;
  } else {
    GfLogError("No description file for robot %s\n", cardllname);
  }
  return NULL;
}


/** Initialize the cars for a race.
    The cars are positionned on the starting grid.
    @return 0 Ok, -1 Error
 */
int
ReInitCars(void)
{
  char buf[512];
  char path[512];
  int nCars;
  int index;
  int i, j;
  const char *robotModuleName;
  int robotIdx;
  void *robhdle;
  tCarElt *elt;
  const char *focused;
  int focusedIdx;
  void *params = ReInfo->params;

  /* Get the number of cars (= drivers) racing */
  nCars = GfParmGetEltNb(params, RM_SECT_DRIVERS_RACING);
  GfOut("Loading %d cars\n", nCars);

  FREEZ(ReInfo->carList);
  ReInfo->carList = (tCarElt*)calloc(nCars, sizeof(tCarElt));
  FREEZ(ReInfo->rules);
  ReInfo->rules = (tRmCarRules*)calloc(nCars, sizeof(tRmCarRules));
  focused = GfParmGetStr(ReInfo->params, RM_SECT_DRIVERS, RM_ATTR_FOCUSED, "");
  focusedIdx = (int)GfParmGetNum(ReInfo->params, RM_SECT_DRIVERS, RM_ATTR_FOCUSEDIDX, NULL, 0);
  index = 0;

  /* For each car/driver : */
  for (i = 1; i < nCars + 1; i++) 
  {
    /* Get the name of the module (= shared library) of the driver type */
    snprintf(path, sizeof(path), "%s/%d", RM_SECT_DRIVERS_RACING, i);
    robotModuleName = GfParmGetStr(ReInfo->params, path, RM_ATTR_MODULE, "");
    robotIdx = (int)GfParmGetNum(ReInfo->params, path, RM_ATTR_IDX, NULL, 0);
    snprintf(path, sizeof(path), "%sdrivers/%s/%s.%s", GetLibDir(), robotModuleName, robotModuleName, DLLEXT);

    /* Load the driver type shared library */
    if (GfModLoad(CAR_IDENT, path, ReInfo->modList)) 
    {
      GfLogError("Failed to load robot module %s\n", path);
      break;
    }

    /* Load the racing driver info in the race data structure */
    elt = NULL;
    snprintf(path, sizeof(path), "%s/%d", RM_SECT_DRIVERS_RACING, i);
    if ((int)GfParmGetNum(ReInfo->params, path, RM_ATTR_EXTENDED, NULL, 0) == 0) 
    {
      /* Search for the index of the racing driver in the list of interfaces
         of the module */
      for (j = 0; j < (*(ReInfo->modList))->modInfoSize; j++) 
      {
        if ((*(ReInfo->modList))->modInfo[j].name && (*(ReInfo->modList))->modInfo[j].index == robotIdx) 
        {
          /* We have the right driver : load it */
          elt = reLoadSingleCar( index, i, j, robotIdx, TRUE, robotModuleName );
          if (!elt)
		  {
            GfLogError("No descriptor file for robot %s or parameter errors (1)\n", robotModuleName);
			snprintf(buf, sizeof(buf), "Error: May be no driver or parameters out of bound!!!");
	        RmLoadingScreenSetText(buf);
			snprintf(buf, sizeof(buf), "Error: Have a look to the console window to get the detailed error messages!!!");
	        RmLoadingScreenSetText(buf);
			snprintf(buf, sizeof(buf), "Error: Will go back to the config menu in 10 secs!!!");
	        RmLoadingScreenSetText(buf);
			// Wait some time to allow to read the message!
            RtInitTimer(); // Check existance of Performance Counter Hardware
	        double Duration; 
	        double StartTimeStamp = RtTimeStamp(); 
			do
			{
              Duration = RtDuration(StartTimeStamp);
			} while (Duration < 10000); // 10 seconds
		  }
        }
      }
    }
    else 
    {
      GfLogTrace("Loading robot %s descriptor file\n", robotModuleName );
      snprintf(buf, sizeof(buf), "%sdrivers/%s/%s.xml", GetLocalDir(), robotModuleName, robotModuleName);
      robhdle = GfParmReadFile(buf, GFPARM_RMODE_STD);
      if (!robhdle) 
      {
        snprintf(buf, sizeof(buf), "drivers/%s/%s.xml", robotModuleName, robotModuleName);
        robhdle = GfParmReadFile(buf, GFPARM_RMODE_STD);
      }
      if (robhdle && ( strcmp( robotModuleName, "human" ) == 0 || strcmp( robotModuleName, "networkhuman" ) == 0 ) )
      {
        /* Human driver */
        elt = reLoadSingleCar( index, i, robotIdx - (*(ReInfo->modList))->modInfo[0].index, robotIdx, FALSE, robotModuleName );
      }
      else if (robhdle && ( strcmp( GfParmGetStr( robhdle, ROB_SECT_ARBITRARY, ROB_ATTR_TEAM, "foo" ),
                              GfParmGetStr( robhdle, ROB_SECT_ARBITRARY, ROB_ATTR_TEAM, "bar" ) ) == 0 ) )
      {
        elt = reLoadSingleCar( index, i, (*(ReInfo->modList))->modInfoSize, robotIdx, FALSE, robotModuleName );
      }
      else
        GfLogError("No descriptor for robot %s (2)\n", robotModuleName );

    }

    if (elt)
      ++index;
  }

  nCars = index; /* real number of cars */
  if (nCars == 0) 
  {
    GfLogError("No driver for that race ; exiting ...\n");
    return -1;
  }
  else 
  {
    GfLogInfo("%d driver(s) ready to race\n", nCars);
  }

  ReInfo->s->_ncars = nCars;
  FREEZ(ReInfo->s->cars);
  ReInfo->s->cars = (tCarElt **)calloc(nCars, sizeof(tCarElt *));
  for (i = 0; i < nCars; i++) 
  {
    ReInfo->s->cars[i] = &(ReInfo->carList[i]);
  }
  ReInfo->_reInPitMenuCar = 0;

  // TODO: reconsider splitting the call into one for cars, track and maybe other objects.
  // I stuff for now anything into one call because collision detection works with the same
  // library on all objects, so it is a bit dangerous to distribute the handling to various
  // locations (because the library maintains global state like a default collision handler etc.).
    ReInfo->_reSimItf.init(nCars, ReInfo->track);

    initStartingGrid();

    initPits();

    return 0;
}

/**
 * This functions initialized the graphics.
 * This function must be called after the cars are loaded and the track is loaded.
 * The track will be unloaded if the event end. The graphics module is keps open
 * if more then one race is driven
 */
void ReInitGraphics()
{
  char const *dllname;
  char key[256];

  /* Check if the module is already loaded */
  if( ReInfo->_reGraphicItf.refresh != 0 )
    return; /* Module is already loaded: don't load again */
    
  /* Load the graphic module */
  GfLogInfo("Loading Graphic Engine...\n");
  dllname = GfParmGetStr(ReInfo->_reParam, "Modules", "graphic", "");
  snprintf(key, sizeof(key), "%smodules/graphic/%s.%s", GetLibDir (), dllname, DLLEXT);
  if (GfModLoad(0, key, &reEventModList))
	  return;
  reEventModList->modInfo->fctInit(reEventModList->modInfo->index, &ReInfo->_reGraphicItf);

  ReInfo->_reGraphicItf.inittrack(ReInfo->track);
}

void
ReRaceCleanup(void)
{
  ReInfo->_reGameScreen = ReHookInit();
  ReInfo->_reSimItf.shutdown();
  if (ReInfo->_displayMode == RM_DISP_MODE_NORMAL) 
  {
    if (ReInfo->_reGraphicItf.shutdowncars)
      ReInfo->_reGraphicItf.shutdowncars();
    else
      GfLogError("WARNING: normal race display but graphical module not loaded !\n" );
  }
  ReStoreRaceResults(ReInfo->_reRaceName);
  ReRaceCleanDrivers();
}


void
ReRaceCleanDrivers(void)
{
  int i;
  tRobotItf *robot;
  int nCars;
  tMemoryPool oldPool = NULL;

  nCars = ReInfo->s->_ncars;
  for (i = 0; i < nCars; i++) 
  {
    robot = ReInfo->s->cars[i]->robot;
    GfPoolMove( &ReInfo->s->cars[i]->_shutdownMemPool, &oldPool );
    if (robot->rbShutdown && ReInfo->_displayMode != RM_DISP_MODE_SIMU_SIMU) 
    {
      robot->rbShutdown(robot->index);
    }
    GfPoolFreePool( &oldPool );
    GfParmReleaseHandle(ReInfo->s->cars[i]->_paramsHandle);
    free(robot);
    free(ReInfo->s->cars[i]->_curSplitTime);
    free(ReInfo->s->cars[i]->_bestSplitTime);
  }
  RtTeamManagerRelease();

  FREEZ(ReInfo->s->cars);
  ReInfo->s->cars = 0;
  ReInfo->s->_ncars = 0;
  GfModUnloadList(&ReRaceModList);
}


char *
ReGetCurrentRaceName(void)
{
	char path[64];
    int   curRaceIdx;
    void  *params = ReInfo->params;
    void  *results = ReInfo->results;

    curRaceIdx = (int)GfParmGetNum(results, RE_SECT_CURRENT, RE_ATTR_CUR_RACE, NULL, 1);
    snprintf(path, sizeof(path), "%s/%d", RM_SECT_RACES, curRaceIdx);
    return GfParmGetStrNC(params, path, RM_ATTR_NAME, 0);
}

char *
ReGetPrevRaceName(void)
{
	char path[64];
    int   curRaceIdx;
    void  *params = ReInfo->params;
    void  *results = ReInfo->results;

    curRaceIdx = (int)GfParmGetNum(results, RE_SECT_CURRENT, RE_ATTR_CUR_RACE, NULL, 1) - 1;
    snprintf(path, sizeof(path), "%s/%d", RM_SECT_RACES, curRaceIdx);
    return GfParmGetStrNC(params, path, RM_ATTR_NAME, 0);
}
