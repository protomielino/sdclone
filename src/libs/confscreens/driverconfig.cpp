/***************************************************************************

    file                 : driverconfig.cpp
    created              : Wed Apr 26 20:05:12 CEST 2000
    copyright            : (C) 2000 by Eric Espie
    email                : torcs@free.fr
    version              : $Id: driverconfig.cpp,v 1.8 2008/03/27 21:26:51 torcs Exp $

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <deque>
#include <tgfclient.h>
#include <track.h>
#include <robot.h>
#include <playerpref.h>
#include <controlconfig.h>

#include "driverconfig.h"
#include <string>

static const int   MAX_DRV_NAME_LEN	= 16;
static const char *DRV_NAME_PROMPT		= "-- Enter name --";
static const char *NO_DRV			= "-- No one --";
static const char *HumanDriverModuleName	= "human";

static const char *SkillLevelString[] = { ROB_VAL_ROOKIE, ROB_VAL_AMATEUR, ROB_VAL_SEMI_PRO, ROB_VAL_PRO };
static const int NbSkillLevels = sizeof(SkillLevelString) / sizeof(SkillLevelString[0]);

static char buf[1024];

static const float LabelColor[] = {1.0, 0.0, 1.0, 1.0};

static int	ScrollList;
static void	*ScrHandle = NULL;
static void	*PrevScrHandle = NULL;

static void	*PrefHdle = NULL;
static void	*DrvHdle = NULL;

static int NameEditId;
static int CarEditId;
static int CatEditId;
static int RaceNumEditId;
static int GearChangeEditId;
static int PitsEditId;
static int SkillEditId;
static int AutoReverseEditId;
static int AutoReverseLeftId;
static int AutoReverseRightId;
static int AutoReverseLabelId;

/* Struct to define a generic ("internal name / id", "displayable name") pair */
typedef struct tInfo
{
    char	*name;
    char	*dispname;
} tInfo;

/* Car names and car categories double-linked lists structs */
struct tCarInfo;
struct tCatInfo;

GF_TAILQ_HEAD(CarsInfoHead, struct tCarInfo);
GF_TAILQ_HEAD(CatsInfoHead, struct tCatInfo);

typedef struct tCatInfo
{
    struct tCatInfo	*next;
    struct tCatInfo	*prev;
    tInfo		info;
    tCarsInfoHead	CarsInfoList;
    GF_TAILQ_ENTRY(struct tCatInfo) link;
} tCatInfo;

typedef struct tCarInfo
{
    struct tCarInfo	*next;
    struct tCarInfo	*prev;
    tInfo		info;
    tCatInfo		*cat;
    GF_TAILQ_ENTRY(struct tCarInfo) link;
} tCarInfo;

/* The car category / name double-linked lists */
static tCatsInfoHead CatsInfoList;

/* Player info struct */
struct tPlayerInfo
{
public:

  tPlayerInfo(const char *name = HumanDriverModuleName, const char *dispname = 0, tCarInfo *carinfo = 0, 
			  int racenumber = 0, int skilllevel = 0, float *color = 0, 
			  tGearChangeMode gearchangemode = GEAR_MODE_AUTO, int autoreverse = 0, 
			  int nbpitstops = 0) 
  {
	_info.name = 0;
	setName(name);
	_info.dispname = 0;
	setDispName(dispname);
	_carinfo = carinfo ? carinfo : GF_TAILQ_FIRST(&((GF_TAILQ_FIRST(&CatsInfoList))->CarsInfoList));
	_racenumber = racenumber; 
	_gearchangemode = gearchangemode; 
	_nbpitstops = nbpitstops; 
	_skilllevel = skilllevel; 
	_autoreverse = autoreverse; 
	_color[0] = color ? color[0] : 1.0; 
	_color[1] = color ? color[1] : 1.0; 
	_color[2] = color ? color[2] : 0.5; 
	_color[3] = color ? color[3] : 1.0;
  }

  tPlayerInfo(const tPlayerInfo &src)
  {
	_info.name = 0;
	setName(src._info.name);
	_info.dispname = 0;
	setDispName(src._info.dispname);
	_carinfo = src._carinfo;
	_racenumber = src._racenumber; 
	_gearchangemode = src._gearchangemode; 
	_nbpitstops = src._nbpitstops; 
	_skilllevel = src._skilllevel; 
	_autoreverse = src._autoreverse; 
	_color[0] = src._color[0]; 
	_color[1] = src._color[1]; 
	_color[2] = src._color[2]; 
	_color[3] = src._color[3];
  }

  const char *name()  const { return _info.name; };
  const char *dispName()  const { return _info.dispname; }
  tCarInfo *carInfo() const { return _carinfo; }
  int raceNumber() const { return _racenumber; }
  tGearChangeMode gearChangeMode() const { return _gearchangemode; }
  int nbPitStops() const { return _nbpitstops; }
  float color(int idx) const { return (idx >= 0 && idx < 4) ? _color[idx] : 0.0; }
  int skillLevel() const { return _skilllevel; }
  int autoReverse() const { return _autoreverse; }

  void setName(const char *name)
  {
    if (_info.name)
	    delete[] _info.name;
	if (!name || strlen(name) == 0)
	  name = HumanDriverModuleName;
	_info.name = new char[strlen(name)+1];
	strcpy(_info.name, name); // Can't use strdup : free crashes in destructor !?
  }
  void setDispName(const char *dispname)
  {
    if (_info.dispname)
	    delete[] _info.dispname;
	if (!dispname || strlen(dispname) == 0)
	  dispname = NO_DRV;
	_info.dispname = new char[strlen(dispname)+1];
	strcpy(_info.dispname, dispname); // Can't use strdup : free crashes in destructor !?
  }
  void setCarInfo(tCarInfo *carInfo) { _carinfo = carInfo; }
  void setRaceNumber(int raceNumber) { _racenumber = raceNumber; }
  void setGearChangeMode(tGearChangeMode gearChangeMode) { _gearchangemode = gearChangeMode; }
  void setNbPitStops(int nbPitStops) { _nbpitstops = nbPitStops; }
  void setSkillLevel(int skillLevel) { _skilllevel = skillLevel; }
  void setAutoReverse(int autoReverse) { _autoreverse = autoReverse; }

  ~tPlayerInfo() 
  {
	if (_info.dispname)
	  delete[] _info.dispname;
	if (_info.name)
	  delete[] _info.name;
  }

  // Gear change mode enum to string conversion
  const char *gearChangeModeString() const
  { 
	const char *gearChangeStr;
	
	if (_gearchangemode == GEAR_MODE_AUTO) {
	  gearChangeStr = HM_VAL_AUTO;
	} else if (_gearchangemode == GEAR_MODE_GRID) {
	  gearChangeStr = HM_VAL_GRID;
	} else {
	  gearChangeStr = HM_VAL_SEQ;
	}
 
	return gearChangeStr;
  }

private:

  tInfo			_info;
  tCarInfo*		_carinfo;
  int				_racenumber;
  tGearChangeMode	_gearchangemode;
  int				_nbpitstops;
  float			_color[4];
  int				_skilllevel;
  int				_autoreverse;
};


/* The human driver (= player) info list */
typedef std::deque<tPlayerInfo*> tPlayerInfoList;
static tPlayerInfoList PlayersInfo;

/* The currently selected player (PlayersInfo.end() if none) */
static tPlayerInfoList::iterator CurrPlayer;

/* A bool to ("yes", "no") conversion table */
static const char *Yn[] = {HM_VAL_YES, HM_VAL_NO};

static int ReloadValues = 1;

/* Load screen editable fields with relevant values :
   - all empty if no player selected,
   - from selected player current settings otherwise 
*/
static void
refreshEditVal(void)
{
    int autoRevVisible = GFUI_INVISIBLE;

    if (CurrPlayer == PlayersInfo.end()) {

	GfuiEditboxSetString(ScrHandle, NameEditId, "");
	GfuiEnable(ScrHandle, NameEditId, GFUI_DISABLE);

	GfuiEditboxSetString(ScrHandle, RaceNumEditId, "");
	GfuiEnable(ScrHandle, RaceNumEditId, GFUI_DISABLE);

	GfuiLabelSetText(ScrHandle, CarEditId, "");
	GfuiEnable(ScrHandle, CarEditId, GFUI_DISABLE);
	
	GfuiLabelSetText(ScrHandle, CatEditId, "");
	GfuiEnable(ScrHandle, CatEditId, GFUI_DISABLE);

	GfuiLabelSetText(ScrHandle, GearChangeEditId, "");
	GfuiEnable(ScrHandle, GearChangeEditId, GFUI_DISABLE);

	GfuiEditboxSetString(ScrHandle, PitsEditId, "");
	GfuiEnable(ScrHandle, PitsEditId, GFUI_DISABLE);

	GfuiLabelSetText(ScrHandle, SkillEditId, "");
	GfuiEnable(ScrHandle, SkillEditId, GFUI_DISABLE);

	GfuiLabelSetText(ScrHandle, AutoReverseEditId, "");
	GfuiEnable(ScrHandle, AutoReverseEditId, GFUI_DISABLE);

    } else {

        if (strcmp((*CurrPlayer)->dispName(), NO_DRV)) {
	    GfuiEditboxSetString(ScrHandle, NameEditId, (*CurrPlayer)->dispName());
	} else {
	    GfuiEditboxSetString(ScrHandle, NameEditId, DRV_NAME_PROMPT);
	}
	GfuiEnable(ScrHandle, NameEditId, GFUI_ENABLE);

	sprintf(buf, "%d", (*CurrPlayer)->raceNumber());
	GfuiEditboxSetString(ScrHandle, RaceNumEditId, buf);
	GfuiEnable(ScrHandle, RaceNumEditId, GFUI_ENABLE);

	GfuiLabelSetText(ScrHandle, CarEditId, (*CurrPlayer)->carInfo()->info.dispname);
	GfuiEnable(ScrHandle, CarEditId, GFUI_ENABLE);

	GfuiLabelSetText(ScrHandle, CatEditId, (*CurrPlayer)->carInfo()->cat->info.dispname);
	GfuiEnable(ScrHandle, CatEditId, GFUI_ENABLE);

	GfuiLabelSetText(ScrHandle, GearChangeEditId, (*CurrPlayer)->gearChangeModeString());
	GfuiEnable(ScrHandle, GearChangeEditId, GFUI_ENABLE);

	sprintf(buf, "%d", (*CurrPlayer)->nbPitStops());
	GfuiEditboxSetString(ScrHandle, PitsEditId, buf);
	GfuiEnable(ScrHandle, PitsEditId, GFUI_ENABLE);

	GfuiLabelSetText(ScrHandle, SkillEditId, SkillLevelString[(*CurrPlayer)->skillLevel()]);
	GfuiEnable(ScrHandle, SkillEditId, GFUI_ENABLE);

	GfuiLabelSetText(ScrHandle, AutoReverseEditId, Yn[(*CurrPlayer)->autoReverse()]);
	GfuiEnable(ScrHandle, AutoReverseEditId, GFUI_ENABLE);

	if ((*CurrPlayer)->gearChangeMode() == GEAR_MODE_AUTO)
	    autoRevVisible = GFUI_VISIBLE;
    }

    GfuiVisibilitySet(ScrHandle, AutoReverseLabelId, autoRevVisible);
    GfuiVisibilitySet(ScrHandle, AutoReverseLeftId, autoRevVisible);
    GfuiVisibilitySet(ScrHandle, AutoReverseEditId, autoRevVisible);
    GfuiVisibilitySet(ScrHandle, AutoReverseRightId, autoRevVisible);
    
}

static void
onSelect(void * /* Dummy */)
{
    tPlayerInfoList::difference_type elemIdx;
    GfuiScrollListGetSelectedElement(ScrHandle, ScrollList, (void**)&elemIdx);

    CurrPlayer = PlayersInfo.begin() + elemIdx;

    refreshEditVal();
}

/* Load the car category info / car info double-linked lists
   from installed category dirs and car description xml files */
static void
GenCarsInfo(void)
{
    tCarInfo	*curCar;
    tCatInfo	*curCat;
    tCatInfo	*tmpCat;
    tFList	*files;
    tFList	*curFile;
    void	*carparam;
    void	*hdle;
    
    /* Empty the cars/categories lists */
    while ((curCat = GF_TAILQ_FIRST(&CatsInfoList)) != NULL) {
        GF_TAILQ_REMOVE(&CatsInfoList, curCat, link);
	while ((curCar = GF_TAILQ_FIRST(&(curCat->CarsInfoList))) != NULL) {
	    GF_TAILQ_REMOVE(&(curCat->CarsInfoList), curCar, link);
	    free(curCar->info.name);
	    free(curCar);
	}
	free(curCat);
    }
    
    /* Load the category list */
    files = GfDirGetList("categories");
    curFile = files;
    if ((curFile != NULL) && (curFile->name[0] != '.')) {
        do {
	    curFile = curFile->next;
	    curCat = (tCatInfo*)calloc(1, sizeof(tCatInfo));
	    GF_TAILQ_INIT(&(curCat->CarsInfoList));
	    char *str = strchr(curFile->name, '.');
	    *str = '\0';
	    curCat->info.name = strdup(curFile->name);
	    sprintf(buf, "categories/%s.xml", curFile->name);
	    hdle = GfParmReadFile(buf, GFPARM_RMODE_STD);
	    if (!hdle) {
	        continue;
	    }
	    curCat->info.dispname = GfParmGetName(hdle);
	    GF_TAILQ_INSERT_TAIL(&CatsInfoList, curCat, link);
	} while (curFile != files);
    }
    GfDirFreeList(files, NULL, true, true);
    
    /* Load the car info list */
    files = GfDirGetList("cars");
    curFile = files;
    if ((curFile != NULL) && (curFile->name[0] != '.')) {
	do {
	    curFile = curFile->next;
	    sprintf(buf, "cars/%s/%s.xml", curFile->name, curFile->name);
	    carparam = GfParmReadFile(buf, GFPARM_RMODE_STD);
	    if (!carparam) {
		continue;
	    }
	    curCar = (tCarInfo*)calloc(1, sizeof(tCarInfo));
	    curCar->info.name = strdup(curFile->name);
	    curCar->info.dispname = GfParmGetName(carparam);
	    /* search for the category */
	    const char *str = GfParmGetStr(carparam, SECT_CAR, PRM_CATEGORY, "");
	    curCat = GF_TAILQ_FIRST(&CatsInfoList);
	    if (curCat != NULL) {
		do {
		    if (strcmp(curCat->info.name, str) == 0) {
			break;
		    }
		} while ((curCat = GF_TAILQ_NEXT(curCat, link)) != NULL);
	    }
	    curCar->cat = curCat;
	    GF_TAILQ_INSERT_TAIL(&(curCat->CarsInfoList), curCar, link);
	} while (curFile != files);
    }
    GfDirFreeList(files, NULL, true, true);

    /* Remove the empty categories */
    curCat = GF_TAILQ_FIRST(&CatsInfoList);
    do {
	curCar = GF_TAILQ_FIRST(&(curCat->CarsInfoList));
	tmpCat = curCat;
	curCat = GF_TAILQ_NEXT(curCat, link);
	if (curCar == NULL) {
	    GfOut("Removing empty category %s\n", tmpCat->info.dispname);
	    GF_TAILQ_REMOVE(&CatsInfoList, tmpCat, link);
	    free(tmpCat->info.name);
	    free(tmpCat);
	}
    } while (curCat  != NULL);
    
}

/* Update players scroll-list from PlayersInfo array */
static void
UpdtScrollList(void)
{
    const char	*str;
    int		i;
    void	*tmp;

    /* free the previous scrollist elements */
    while((str = GfuiScrollListExtractElement(ScrHandle, ScrollList, 0, (void**)&tmp)) != NULL) {
    }
    for (i = 0; i < (int)PlayersInfo.size(); i++) {
	GfuiScrollListInsertElement(ScrHandle, ScrollList, PlayersInfo[i]->dispName(), i, (void*)i);
    }

    if (CurrPlayer != PlayersInfo.end()) {
      GfuiScrollListShowElement(ScrHandle, ScrollList, (int)(CurrPlayer - PlayersInfo.begin()));
    }
}

/* Put given player settings (from PlayersInfo array) to the human drivers and preferences params (index is the identification number in params, beginning at 1)*/
static void
PutDrvSettings(unsigned index)
{
    char drvSectionPath[256];

    if (!DrvHdle || !PrefHdle) {
	return;
    }

    tPlayerInfo *player = PlayersInfo[index-1];

    // Human driver params
    sprintf(drvSectionPath, "%s/%s/%u", ROB_SECT_ROBOTS, ROB_LIST_INDEX, index);
    GfParmSetStr(DrvHdle, drvSectionPath, ROB_ATTR_NAME, player->dispName());
    GfParmSetStr(DrvHdle, drvSectionPath, ROB_ATTR_CAR, player->carInfo()->info.name);
    GfParmSetNum(DrvHdle, drvSectionPath, ROB_ATTR_RACENUM, (char*)NULL, player->raceNumber());
    GfParmSetNum(DrvHdle, drvSectionPath, ROB_ATTR_RED, (char*)NULL, player->color(0));
    GfParmSetNum(DrvHdle, drvSectionPath, ROB_ATTR_GREEN, (char*)NULL, player->color(1));
    GfParmSetNum(DrvHdle, drvSectionPath, ROB_ATTR_BLUE, (char*)NULL, player->color(2));
    GfParmSetStr(DrvHdle, drvSectionPath, ROB_ATTR_TYPE, ROB_VAL_HUMAN);
    GfParmSetStr(DrvHdle, drvSectionPath, ROB_ATTR_LEVEL, SkillLevelString[player->skillLevel()]);

    // Driver preferences params
    sprintf(drvSectionPath, "%s/%s/%u", HM_SECT_PREF, HM_LIST_DRV, index);
    GfParmSetStr(PrefHdle, drvSectionPath, HM_ATT_TRANS, player->gearChangeModeString());
    GfParmSetNum(PrefHdle, drvSectionPath, HM_ATT_NBPITS, (char*)NULL, (tdble)player->nbPitStops());
    GfParmSetStr(PrefHdle, drvSectionPath, HM_ATT_AUTOREVERSE, Yn[player->autoReverse()]);
    
    /* Allow neutral gear in sequential mode if no reverse gear command defined */
    if (player->gearChangeMode() == GEAR_MODE_SEQ
	&& !strcmp(GfParmGetStr(PrefHdle, drvSectionPath, HM_ATT_GEAR_R, "-"), "-"))
        GfParmSetStr(PrefHdle, drvSectionPath, HM_ATT_SEQSHFT_ALLOW_NEUTRAL, HM_VAL_YES);
    else
        GfParmSetStr(PrefHdle, drvSectionPath, HM_ATT_SEQSHFT_ALLOW_NEUTRAL, HM_VAL_NO);
    
    /* Release gear lever goes neutral in grid mode if no neutral gear command defined */
    if (player->gearChangeMode() == GEAR_MODE_GRID
	&& !strcmp(GfParmGetStr(PrefHdle, drvSectionPath, HM_ATT_GEAR_N, "-"), "-"))
        GfParmSetStr(PrefHdle, drvSectionPath, HM_ATT_REL_BUT_NEUTRAL, HM_VAL_YES);
    else
        GfParmSetStr(PrefHdle, drvSectionPath, HM_ATT_REL_BUT_NEUTRAL, HM_VAL_NO);
}

/* Create a brand new player into the list, and update players scroll-list and all editable fields*/
static void
NewPlayer(void * /* dummy */)
{
    unsigned newPlayerIdx, playerIdx;
    char sectionPath[256];
    char driverId[5];
    char newDriverId[5];

    // Insert new player after current (or after last if no current)
    CurrPlayer = PlayersInfo.insert(CurrPlayer + (CurrPlayer == PlayersInfo.end() ? 0 : 1),
				   new tPlayerInfo(HumanDriverModuleName));

    // Get new current player index (= identification number in params).
    newPlayerIdx = (unsigned)(CurrPlayer - PlayersInfo.begin()) + 1;

    // Update preferences and drivers params (rename those after, add new).
    sprintf(sectionPath, "%s/%s", HM_SECT_PREF, HM_LIST_DRV);
    for (playerIdx = PlayersInfo.size() - 1; playerIdx >= newPlayerIdx; playerIdx--) {
        sprintf(driverId, "%u", playerIdx);
	sprintf(newDriverId, "%u", playerIdx+1);
	GfParmListRenameElt(PrefHdle, sectionPath, driverId, newDriverId);
    }

    sprintf(sectionPath, "%s/%s", ROB_SECT_ROBOTS, ROB_LIST_INDEX);
    for (playerIdx = PlayersInfo.size() - 1; playerIdx >= newPlayerIdx; playerIdx--) {
        sprintf(driverId, "%u", playerIdx);
	sprintf(newDriverId, "%u", playerIdx+1);
	GfParmListRenameElt(DrvHdle, sectionPath, driverId, newDriverId);
    }

    PutDrvSettings(newPlayerIdx);

    // Update GUI.
    refreshEditVal();
    UpdtScrollList();
}

/* Create a new player into the list by copying the currently selected one, and update players scroll-list and all editable fields (copy also the control settings) */
static void
CopyPlayer(void * /* dummy */)
{
    unsigned curPlayerIdx, newPlayerIdx;
    unsigned playerIdx;
    char sectionPath[256];
    char driverId[5];
    char newDriverId[5];

    tGearChangeMode gearChange;
    
    if (CurrPlayer != PlayersInfo.end()) {

        // Get current (source) player index (= identification number in params).
        curPlayerIdx = (unsigned)(CurrPlayer - PlayersInfo.begin()) + 1;

	// Save current player gear change mode
	gearChange = (*CurrPlayer)->gearChangeMode();

	 // Get current player control settings.
        ControlGetSettings(PrefHdle, curPlayerIdx);

	// Insert new player after current
        CurrPlayer = PlayersInfo.insert(CurrPlayer + 1, new tPlayerInfo(**CurrPlayer));

        // Get new (copy) player index (= identification number in params).
        newPlayerIdx = (unsigned)(CurrPlayer - PlayersInfo.begin()) + 1;

	// Update preferences and drivers params (rename those after, add new).
	sprintf(sectionPath, "%s/%s", HM_SECT_PREF, HM_LIST_DRV);
	for (playerIdx = PlayersInfo.size() - 1; playerIdx >= newPlayerIdx; playerIdx--) {
	    sprintf(driverId, "%u", playerIdx);
	    sprintf(newDriverId, "%u", playerIdx+1);
	    GfParmListRenameElt(PrefHdle, sectionPath, driverId, newDriverId);
	}
    
	sprintf(sectionPath, "%s/%s", ROB_SECT_ROBOTS, ROB_LIST_INDEX);
	for (playerIdx = PlayersInfo.size() - 1; playerIdx >= newPlayerIdx; playerIdx--) {
	    sprintf(driverId, "%u", playerIdx);
	    sprintf(newDriverId, "%u", playerIdx+1);
	    GfParmListRenameElt(DrvHdle, sectionPath, driverId, newDriverId);
	}

	PutDrvSettings(newPlayerIdx);

	// Set new player control settings (copy of previous current one's).
	ControlPutSettings(PrefHdle, newPlayerIdx, gearChange);

	// Update GUI.
        refreshEditVal();
        UpdtScrollList();
    }
}

/* Remove the selected player from the list, and update players scroll-list and all editable fields*/
static void
DeletePlayer(void * /* dummy */)
{
    int delPlayerIdx;
    unsigned int playerIdx;
    char sectionPath[256];
    char driverId[5];
    char newDriverId[5];
    
    if (CurrPlayer != PlayersInfo.end()) {

        // Get current player index (= identification number in params).
        delPlayerIdx = CurrPlayer - PlayersInfo.begin() + 1;

	// Remove current player from list. Select next if any.
	delete *CurrPlayer;
        CurrPlayer = PlayersInfo.erase(CurrPlayer);

	// Update preferences and drivers params.
	sprintf(sectionPath, "%s/%s", HM_SECT_PREF, HM_LIST_DRV);
	sprintf(driverId, "%d", delPlayerIdx);
	if (!GfParmListRemoveElt(PrefHdle, sectionPath, driverId)) {
	    for (playerIdx = delPlayerIdx; playerIdx <= PlayersInfo.size(); playerIdx++) {
	        sprintf(driverId, "%u", playerIdx+1);
	        sprintf(newDriverId, "%u", playerIdx);
	        GfParmListRenameElt(PrefHdle, sectionPath, driverId, newDriverId);
	    }
	}

	sprintf(sectionPath, "%s/%s", ROB_SECT_ROBOTS, ROB_LIST_INDEX);
	sprintf(driverId, "%d", delPlayerIdx);
	if (!GfParmListRemoveElt(DrvHdle, sectionPath, driverId)) {
	    for (playerIdx = delPlayerIdx; playerIdx <= PlayersInfo.size(); playerIdx++) {
	        sprintf(driverId, "%u", playerIdx+1);
	        sprintf(newDriverId, "%u", playerIdx);
	        GfParmListRenameElt(DrvHdle, sectionPath, driverId, newDriverId);
	    }
	}

	// Update GUI.
	refreshEditVal();
	UpdtScrollList();
    }
}

/* Activate the control config screen if a player is selected */
static void
ConfControls(void * /* dummy */ )
{
    unsigned curPlayerIdx;
    
    if (CurrPlayer != PlayersInfo.end()) {

        ReloadValues = 0;

        curPlayerIdx = (unsigned)(CurrPlayer - PlayersInfo.begin()) + 1;
	GfuiScreenActivate(ControlMenuInit(ScrHandle, PrefHdle, curPlayerIdx, (*CurrPlayer)->gearChangeMode()));
    }
}

/* Load human driver (=player) info list (PlayersInfo) from preferences and human drivers files ;
   load associated scroll list */
static int
GenDrvList(void)
{
    char sstring[256];
    int i;
    int j;
    const char *driver;
    int skilllevel;
    tCarInfo *car, *carinfo;
    tCatInfo *cat;
    const char *str;
    int found;
    int racenumber;
    float color[4];
    
    /* Reset players list */
    tPlayerInfoList::iterator playerIter;
    for (playerIter = PlayersInfo.begin(); playerIter != PlayersInfo.end(); playerIter++)
        delete *playerIter;
    PlayersInfo.clear();

    /* Load players settings from human.xml file*/
    sprintf(buf, "%s%s", GetLocalDir(), HM_DRV_FILE);
    DrvHdle = GfParmReadFile(buf, GFPARM_RMODE_REREAD);
    if (DrvHdle == NULL) {
        return -1;
    }

    for (i = 0; ; i++) {
        sprintf(sstring, "%s/%s/%d", ROB_SECT_ROBOTS, ROB_LIST_INDEX, i+1);
	driver = GfParmGetStr(DrvHdle, sstring, ROB_ATTR_NAME, "");
	if (strlen(driver) == 0) {
	    break; // Exit at end of driver list.
	} else {
	    str = GfParmGetStr(DrvHdle, sstring, ROB_ATTR_LEVEL, SkillLevelString[0]);
	    skilllevel = 0;
	    for(j = 0; j < NbSkillLevels; j++) {
	        if (strcmp(SkillLevelString[j], str) == 0) {
		    skilllevel = j;
		    break;
		}
	    }
	    str = GfParmGetStr(DrvHdle, sstring, ROB_ATTR_CAR, "");
	    found = 0;
	    carinfo = 0;
	    cat = GF_TAILQ_FIRST(&CatsInfoList);
	    do {
	        car = GF_TAILQ_FIRST(&(cat->CarsInfoList));
		if (car != NULL) {
		    do {
		        if (strcmp(car->info.name, str) == 0) {
			    found = 1;
			    carinfo = car;
			}
		    } while (!found && ((car = GF_TAILQ_NEXT(car, link)) != NULL));
		}
	    } while (!found && ((cat = GF_TAILQ_NEXT(cat, link)) != NULL));
	    racenumber  = (int)GfParmGetNum(DrvHdle, sstring, ROB_ATTR_RACENUM, (char*)NULL, 0);
	    color[0]    = (float)GfParmGetNum(DrvHdle, sstring, ROB_ATTR_RED, (char*)NULL, 1.0);
	    color[1]    = (float)GfParmGetNum(DrvHdle, sstring, ROB_ATTR_GREEN, (char*)NULL, 1.0);;
	    color[2]    = (float)GfParmGetNum(DrvHdle, sstring, ROB_ATTR_BLUE, (char*)NULL, 0.5);;
	    color[3]    = 1.0;
	    PlayersInfo.push_back(new tPlayerInfo(HumanDriverModuleName, // Driver module name
						  driver,  // Driver (display) name
						  carinfo, // Car
						  racenumber, // Race number
						  skilllevel, // skill level
						  color)); // Colors
	}
    }

	/* No currently selected driver */
    CurrPlayer = PlayersInfo.end();

	/* Update scroll-list from PlayersInfo */
    UpdtScrollList();

    /* Load players settings from human.xml file*/
    sprintf(buf, "%s%s", GetLocalDir(), HM_PREF_FILE);
    PrefHdle = GfParmReadFile(buf, GFPARM_RMODE_REREAD);
    if (!PrefHdle) {
        return -1;
    }

    for (i = 0; i < (int)PlayersInfo.size(); i++) {
        sprintf(sstring, "%s/%s/%d", HM_SECT_PREF, HM_LIST_DRV, i+1);
	str = GfParmGetStr(PrefHdle, sstring, HM_ATT_TRANS, HM_VAL_AUTO);
	if (!strcmp(str, HM_VAL_AUTO)) {
	    PlayersInfo[i]->setGearChangeMode(GEAR_MODE_AUTO);
	} else if (!strcmp(str, HM_VAL_GRID)) {
	    PlayersInfo[i]->setGearChangeMode(GEAR_MODE_GRID);
	} else {
	    PlayersInfo[i]->setGearChangeMode(GEAR_MODE_SEQ);
	} /* Note: Deprecated "manual" value (after R1.3.0) smoothly converted to "sequential" (backward compatibility) */
	PlayersInfo[i]->setNbPitStops(GfParmGetNum(PrefHdle, sstring, HM_ATT_NBPITS, (char*)NULL, 0));
	if (!strcmp(GfParmGetStr(PrefHdle, sstring, HM_ATT_AUTOREVERSE, Yn[0]), Yn[0])) {
	    PlayersInfo[i]->setAutoReverse(0);
	} else {
	    PlayersInfo[i]->setAutoReverse(1);
	}
    }

    return 0;
}

/* Quit driver config menu (changes must have been saved before if needed) */
static void
QuitDriverConfig(void * /* dummy */)
{
    // Next time, we'll have to reload the player settings from the files.
    ReloadValues = 1;

    // Reset player list
    tPlayerInfoList::iterator playerIter;
    for (playerIter = PlayersInfo.begin(); playerIter != PlayersInfo.end(); playerIter++)
        delete *playerIter;
    PlayersInfo.clear();

    // Close driver and preference params files.
    GfParmReleaseHandle(DrvHdle);
    DrvHdle = 0;

    GfParmReleaseHandle(PrefHdle);
    PrefHdle = 0;

    // Activate caller screen/menu
    GfuiScreenActivate(PrevScrHandle);

    return;
}

/* Save players info (from PlayersInfo array) to the human drivers and preferences XML files */
static void
SaveDrvList(void * /* dummy */)
{
    int		index;

    if (!DrvHdle || !PrefHdle) {
	return;
    }

    for (index = 1; index <= (int)PlayersInfo.size(); index++) {
        PutDrvSettings(index);
    }

    GfParmWriteFile(NULL, DrvHdle, HumanDriverModuleName);
    GfParmWriteFile(NULL, PrefHdle, "preferences");

    QuitDriverConfig(0 /* dummy */);

    return;
}

static void
ChangeName(void * /* dummy */)
{
    char	*val;

    if (CurrPlayer != PlayersInfo.end()) {
        val = GfuiEditboxGetString(ScrHandle, NameEditId);
        (*CurrPlayer)->setDispName(strcmp(val, DRV_NAME_PROMPT) ? val : NO_DRV);
    }

    UpdtScrollList();
}

static void
ChangeNum(void * /* dummy */)
{
    char	*val;
    
    if (CurrPlayer != PlayersInfo.end()) {
       val = GfuiEditboxGetString(ScrHandle, RaceNumEditId);
       (*CurrPlayer)->setRaceNumber((int)strtol(val, (char **)NULL, 0));
	sprintf(buf, "%d", (*CurrPlayer)->raceNumber());
	GfuiEditboxSetString(ScrHandle, RaceNumEditId, buf);
    }
}

static void
ChangePits(void * /* dummy */)
{
    char	*val;
    
    if (CurrPlayer != PlayersInfo.end()) {    
        val = GfuiEditboxGetString(ScrHandle, PitsEditId);
        (*CurrPlayer)->setNbPitStops((int)strtol(val, (char **)NULL, 0));
	sprintf(buf, "%d", (*CurrPlayer)->nbPitStops());
	GfuiEditboxSetString(ScrHandle, PitsEditId, buf);
    }
}

static void
ChangeCar(void *vp)
{
    tCarInfo	*car;
    tCatInfo	*cat;
    
    if (CurrPlayer == PlayersInfo.end()) {
	return;
    }

    cat = (*CurrPlayer)->carInfo()->cat;
    if (vp == 0) {
	car = GF_TAILQ_PREV((*CurrPlayer)->carInfo(), CarsInfoHead, link);
	if (car == NULL) {
	    car = GF_TAILQ_LAST(&(cat->CarsInfoList), CarsInfoHead);
	}
    } else {
	car = GF_TAILQ_NEXT((*CurrPlayer)->carInfo(), link);
	if (car == NULL) {
	    car = GF_TAILQ_FIRST(&(cat->CarsInfoList));
	}
    }
    (*CurrPlayer)->setCarInfo(car);

    refreshEditVal();
}

static void
ChangeCat(void *vp)
{
    tCarInfo	*car;
    tCatInfo	*cat;
    
    if (CurrPlayer == PlayersInfo.end()) {
	return;
    }

    cat = (*CurrPlayer)->carInfo()->cat;
    if (vp == 0) {
	do {
	    cat = GF_TAILQ_PREV(cat, CatsInfoHead, link);
	    if (cat == NULL) {
		cat = GF_TAILQ_LAST(&CatsInfoList, CatsInfoHead);
	    }
	    car = GF_TAILQ_FIRST(&(cat->CarsInfoList));
	} while (car == NULL);	/* skip empty categories */
    } else {
	do {
	    cat = GF_TAILQ_NEXT(cat, link);
	    if (cat == NULL) {
		cat = GF_TAILQ_FIRST(&CatsInfoList);
	    }
	    car = GF_TAILQ_FIRST(&(cat->CarsInfoList));
	} while (car == NULL);	/* skip empty categories */
    }
    (*CurrPlayer)->setCarInfo(car);

    refreshEditVal();
}

static void
ChangeLevel(void *vp)
{
    if (CurrPlayer == PlayersInfo.end()) {
	return;
    }

    int skillLevel = (*CurrPlayer)->skillLevel();
    if (vp == 0) {
	skillLevel--;
	if (skillLevel < 0) {
	    skillLevel = NbSkillLevels - 1;
	}
    } else {
	skillLevel++;
	if (skillLevel == NbSkillLevels) {
	    skillLevel = 0;
	}
    }
    (*CurrPlayer)->setSkillLevel(skillLevel);

    refreshEditVal();
}

static void
ChangeReverse(void *vdelta)
{
    const long delta = (long)vdelta;
    
    if (CurrPlayer == PlayersInfo.end()) {
	return;
    }

    int autoReverse = (*CurrPlayer)->autoReverse();
    autoReverse += (int)delta;
    if (autoReverse < 0) {
	autoReverse = 1;
    } else if (autoReverse > 1) {
	autoReverse = 0;
    }
    (*CurrPlayer)->setAutoReverse(autoReverse);

    refreshEditVal();
}


/* Gear change mode change callback */
static void
ChangeGearChange(void *vp)
{
    if (CurrPlayer == PlayersInfo.end()) {
	return;
    }
    tGearChangeMode gearChangeMode = (*CurrPlayer)->gearChangeMode();
    if (vp == 0) {
	if (gearChangeMode == GEAR_MODE_AUTO) {
	    gearChangeMode = GEAR_MODE_GRID;
	} else if (gearChangeMode == GEAR_MODE_SEQ) {
	    gearChangeMode = GEAR_MODE_AUTO;
	}
	else {
	    gearChangeMode = GEAR_MODE_SEQ;
	}
    } else {
	if (gearChangeMode == GEAR_MODE_AUTO) {
	    gearChangeMode = GEAR_MODE_SEQ;
	} else if (gearChangeMode == GEAR_MODE_SEQ) {
	    gearChangeMode = GEAR_MODE_GRID;
	}
	else {
	    gearChangeMode = GEAR_MODE_AUTO;
	}
    }
    (*CurrPlayer)->setGearChangeMode(gearChangeMode);

    refreshEditVal();
}

static void
onActivate(void * /* dummy */)
{
    if (ReloadValues) {
	  
	    /* Load car/categories and players settings */
	    GenCarsInfo();
		GenDrvList();
    }
	
    /* Display editable fields values */
    refreshEditVal();
}

void *
DriverMenuInit(void *prevMenu)
{
    static int	firstTime = 1;

    void *param = LoadMenuXML("drivermenu.xml");

    /* Initialize cars and categories info if not already done */
    if (firstTime) {
	firstTime = 0;
	GF_TAILQ_INIT(&CatsInfoList);
    }
    
    /* Save previous screen handle for exit time */ 
    PrevScrHandle = prevMenu;

    /* Screen already created : nothing more to do */
    if (ScrHandle) {
	return ScrHandle;
    }

    /* Create the screen */
    ScrHandle = GfuiScreenCreateEx((float*)NULL, NULL, onActivate, NULL, (tfuiCallback)NULL, 1);

    CreateStaticControls(param,ScrHandle);


    std::string strText,strTip;
    //int textsize;
    //int alignment;

    ScrollList = CreateScrollListControl(ScrHandle,param,"playerscrolllist",NULL, onSelect);

    /* New player button */
    CreateButtonControl(ScrHandle,param,"new",NULL,NewPlayer);
    /* Copy player button */
    CreateButtonControl(ScrHandle,param,"copy",NULL,CopyPlayer);
    /* Delete player button */
    CreateButtonControl(ScrHandle,param,"delete",NULL,DeletePlayer);
    /* Access to control screen button*/
    CreateButtonControl(ScrHandle,param,"controls",NULL,ConfControls);

    /* Player name label and associated editbox */
    NameEditId = CreateEditControl(ScrHandle,param,"nameedit",NULL,NULL,ChangeName);

    /* Player skill level and associated "combobox" (left arrow, label, right arrow) */
    CreateButtonControl(ScrHandle,param,"levelleftarrow",NULL, ChangeLevel);
    CreateButtonControl(ScrHandle,param,"levelrightarrow",NULL, ChangeLevel);
    SkillEditId = CreateLabelControl(ScrHandle,param,"skillstext");

    RaceNumEditId = CreateEditControl(ScrHandle,param,"racenumedit",NULL,NULL,ChangeNum);

    PitsEditId = CreateEditControl(ScrHandle,param,"pitstopedit",NULL,NULL,ChangePits);
    
    //Combobox like control
    CreateButtonControl(ScrHandle,param,"gearleftarrow",NULL, ChangeGearChange);
    CreateButtonControl(ScrHandle,param,"gearrightarrow",NULL, ChangeGearChange);
    GearChangeEditId = CreateLabelControl(ScrHandle,param,"geartext");

    /* Gear changing auto-reverse flag and associated "combobox" (left arrow, label, right arrow) */
    //Combobox like control
    AutoReverseLabelId = CreateLabelControl(ScrHandle,param,"autoreversetext");

    AutoReverseLeftId = CreateButtonControl(ScrHandle,param,"autoleftarrow",(void*)-1, ChangeReverse);
    AutoReverseRightId = CreateButtonControl(ScrHandle,param,"autorightarrow",(void*)1, ChangeReverse);
    AutoReverseEditId = CreateLabelControl(ScrHandle,param,"autotext");

    /* Car category and associated "combobox" (left arrow, label, right arrow) */
    //Combobox like control
    CreateButtonControl(ScrHandle,param,"categoryleftarrow",NULL, ChangeCat);
    CreateButtonControl(ScrHandle,param,"categoryrightarrow",NULL, ChangeCat);
    CatEditId = CreateLabelControl(ScrHandle,param,"categorylabel");

    //Combobox like control
    CreateButtonControl(ScrHandle,param,"carleftarrow",NULL, ChangeCar);
    CreateButtonControl(ScrHandle,param,"carrightarrow",NULL, ChangeCar);
    CarEditId = CreateLabelControl(ScrHandle,param,"carlabel");

    CreateButtonControl(ScrHandle,param,"accept",NULL, SaveDrvList);
    CreateButtonControl(ScrHandle,param,"cancel",NULL, QuitDriverConfig);

    /* Keybord shortcuts */
    GfuiAddKey(ScrHandle, 13 /* Return */, "Save Drivers", NULL, SaveDrvList, NULL);
    GfuiAddKey(ScrHandle, 27 /* Escape */, "Cancel Selection", NULL, QuitDriverConfig, NULL);
    GfuiAddSKey(ScrHandle, GLUT_KEY_F12, "Screen-Shot", NULL, GfuiScreenShot, NULL);
    GfuiAddSKey(ScrHandle, GLUT_KEY_LEFT, "Previous Car", (void*)0, ChangeCar, NULL);
    GfuiAddSKey(ScrHandle, GLUT_KEY_RIGHT, "Next Car", (void*)1, ChangeCar, NULL);
    GfuiAddSKey(ScrHandle, GLUT_KEY_UP, "Previous Car Category", (void*)0, ChangeCat, NULL);
    GfuiAddSKey(ScrHandle, GLUT_KEY_DOWN, "Next Car Category", (void*)1, ChangeCat, NULL);

    return ScrHandle;
}

