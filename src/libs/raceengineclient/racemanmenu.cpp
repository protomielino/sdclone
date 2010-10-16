/***************************************************************************

    file        : racemanmenu.cpp
    created     : Fri Jan  3 22:24:41 CET 2003
    copyright   : (C) 2003 by Eric Espie                        
    email       : eric.espie@torcs.org   
    version     : $Id: racemanmenu.cpp,v 1.5 2004/08/11 17:44:06 torcs $

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
    @version	$Id: racemanmenu.cpp,v 1.5 2004/08/11 17:44:06 torcs $
*/

#include <cstdlib>
#include <cstdio>

#include <network.h>
#include <tgfclient.h>
#include <raceman.h>
#include <racescreens.h>
#include <playerconfig.h>

#include "racesituation.h"
#include "racemain.h"
#include "raceinit.h"
#include "racestate.h"

#include "racemanmenu.h"
#include "networkingmenu.h"

// VC++ 2005 or newer ...
#if defined(_CRT_SECURE_NO_DEPRECATE) // used with vc++ 2005
#undef snprintf 
#define snprintf _snprintf_s
#endif
// ... VC++ 2005 or newer

// VC++ 6.0 ...
#if defined(WIN32) && !defined(snprintf_s) 
#undef snprintf 
#define snprintf _snprintf 
#endif


// Raceman menu.
static void	*RacemanMenuHdle = NULL;

static int TitleLabelId = 0;
static int LoadRaceButtonId = 0;
static int SaveRaceButtonId = 0;

// New track menu.
static void	*NewTrackMenuHdle = NULL;

static tRmTrackSelect	ts;
static tRmDriverSelect	ds;
static tRmRaceParam	rp;
static tRmFileSelect    fs;


static void reConfigRunState(void);

static void
reConfigBack(void)
{
	void* params = ReInfo->params;

	/* Go back one step in the conf */
	GfParmSetNum(params, RM_SECT_CONF, RM_ATTR_CUR_CONF, NULL, 
				 GfParmGetNum(params, RM_SECT_CONF, RM_ATTR_CUR_CONF, NULL, 1) - 2);

	reConfigRunState();
}


/***************************************************************/
/* Callback hooks used only to run the automaton on activation */
static void	*configHookHandle = 0;

static void
configHookActivate(void * /* dummy */)
{
	reConfigRunState();
}

static void *
reConfigHookInit(void)
{
	if (configHookHandle)
		return configHookHandle;

	configHookHandle = GfuiHookCreate(0, configHookActivate);

	return configHookHandle;
}

/***************************************************************/
/* Config Back Hook */

static void	*ConfigBackHookHandle = 0;

static void
ConfigBackHookActivate(void * /* dummy */)
{
	reConfigBack();
}

static void *
reConfigBackHookInit(void)
{
	if (ConfigBackHookHandle)
		return ConfigBackHookHandle;

	ConfigBackHookHandle = GfuiHookCreate(0, ConfigBackHookActivate);

	return ConfigBackHookHandle;
}

static void
reConfigRunState(void)
{
	char	path[256];
	int		i;
	int		curConf;
	const char	*conf;
	int		numOpt;
	const char	*opt;
	void	*params = ReInfo->params;

	curConf = (int)GfParmGetNum(params, RM_SECT_CONF, RM_ATTR_CUR_CONF, NULL, 1);
	if (curConf > GfParmGetEltNb(params, RM_SECT_CONF)) {
		GfLogInfo("%s configuration finished.\n", ReInfo->_reName);
		GfParmWriteFile(NULL, ReInfo->params, ReInfo->_reName);
		GfuiScreenActivate(RacemanMenuHdle); /* Back to the race menu */
		return;
	}
	
	sprintf(path, "%s/%d", RM_SECT_CONF, curConf);
	conf = GfParmGetStr(params, path, RM_ATTR_TYPE, 0);
	if (!conf) {
		GfLogError("No %s here (%s) !\n", RM_ATTR_TYPE, path);
		GfuiScreenActivate(RacemanMenuHdle); /* Back to the race menu */
		return;
	}

	GfLogInfo("%s configuration now in '%s' stage.\n", ReInfo->_reName, conf);
	if (!strcmp(conf, RM_VAL_TRACKSEL)) {
		/* Track Select Menu */
		ts.nextScreen = reConfigHookInit();
		if (curConf == 1) {
			ts.prevScreen = RacemanMenuHdle;
		} else {
			ts.prevScreen = reConfigBackHookInit();
		}
		ts.param = ReInfo->params;
		ts.trackItf = ReInfo->_reTrackItf;
		RmTrackSelect(&ts);

	} else if (!strcmp(conf, RM_VAL_DRVSEL)) {
		/* Drivers select menu */
		ds.nextScreen = reConfigHookInit();
		if (curConf == 1) {
			ds.prevScreen = RacemanMenuHdle;
		} else {
			ds.prevScreen = reConfigBackHookInit();
		}
		ds.param = ReInfo->params;
		RmDriversSelect(&ds);

	} else if (!strcmp(conf, RM_VAL_RACECONF)) {
		/* Race Options menu */
		rp.nextScreen = reConfigHookInit();
		if (curConf == 1) {
			rp.prevScreen = RacemanMenuHdle;
		} else {
			rp.prevScreen = reConfigBackHookInit();
		}
		rp.param = ReInfo->params;
		rp.title = GfParmGetStr(params, path, RM_ATTR_RACE, "Race");
		/* Select options to configure */
		rp.confMask = 0;
		sprintf(path, "%s/%d/%s", RM_SECT_CONF, curConf, RM_SECT_OPTIONS);
		numOpt = GfParmGetEltNb(params, path);
		for (i = 1; i < numOpt + 1; i++) {
			sprintf(path, "%s/%d/%s/%d", RM_SECT_CONF, curConf, RM_SECT_OPTIONS, i);
			opt = GfParmGetStr(params, path, RM_ATTR_TYPE, "");
			if (!strcmp(opt, RM_VAL_CONFRACELEN)) {
				/* Configure race length */
				rp.confMask |= RM_CONF_RACE_LEN;
			} else {
				if (!strcmp(opt, RM_VAL_CONFDISPMODE)) {
					/* Configure display mode */
					rp.confMask |= RM_CONF_DISP_MODE;
				}
			}
		}
		RmRaceParamMenu(&rp);
	}

	curConf++;
	GfParmSetNum(params, RM_SECT_CONF, RM_ATTR_CUR_CONF, NULL, curConf);
}

void ReSetRacemanMenuHandle(void * handle)
{
	RacemanMenuHdle = handle;
}

void
ReConfigureMenu(void * /* dummy */)
{
	void *params = ReInfo->params;

	/* Reset configuration automaton */
	GfParmSetNum(params, RM_SECT_CONF, RM_ATTR_CUR_CONF, NULL, 1);
	reConfigRunState();

}

static void
reLoadRaceFromResultsFile(const char *filename)
{
	char buf[256];

	sprintf(buf, "%sresults/%s/%s", GetLocalDir(), ReInfo->_reFilename, filename);
	GfLogInfo("Loading saved race from %s ...\n", buf);

	// Update race data.
	ReInfo->mainResults = GfParmReadFile(buf, GFPARM_RMODE_STD | GFPARM_RMODE_CREAT);
	ReInfo->results = ReInfo->mainResults;
	ReInfo->_reRaceName = ReInfo->_reName;
	
	GfParmRemoveVariable (ReInfo->params, "/", "humanInGroup");
	GfParmSetVariable (ReInfo->params, "/", "humanInGroup", ReHumanInGroup() ? 1 : 0);

	// Fire standings screen.
	RmShowStandings(ReInfo->_reGameScreen, ReInfo);
}

static void
reLoadRaceFromConfigFile(const char *filename)
{
	char pszSelFilePathName[256];
	sprintf(pszSelFilePathName, "%sconfig/raceman/%s/%s",
			GetLocalDir(), ReInfo->_reFilename, filename);
	GfLogInfo("Loading saved race from %s ...\n", pszSelFilePathName);

	// Replace the main race file by the selected one.
	char pszMainFilePathName[256];
	sprintf(pszMainFilePathName, "%sconfig/raceman/%s%s",
			GetLocalDir(), ReInfo->_reFilename, PARAMEXT);
	if (!GfFileCopy(pszSelFilePathName, pszMainFilePathName))
	{
		GfLogError("Failed to load selected race file %s", pszSelFilePathName);
		return;
	}
	
	// Update race data.
	GfParmReleaseHandle(ReInfo->params);
	ReInfo->mainParams = ReInfo->params = GfParmReadFile(pszMainFilePathName, GFPARM_RMODE_STD);
	ReInfo->_reName = GfParmGetStr(ReInfo->params, RM_SECT_HEADER, RM_ATTR_NAME, "");
	ReInfo->_reRaceName = ReInfo->_reName;
	
	GfParmRemoveVariable (ReInfo->params, "/", "humanInGroup");
	GfParmSetVariable (ReInfo->params, "/", "humanInGroup", ReHumanInGroup() ? 1 : 0);

	// Update raceman info (the params pointer changed).
	char pszFileName[64];
	sprintf(pszFileName, "%s%s", ReInfo->_reFilename, PARAMEXT);
	ReUpdateRaceman(pszFileName, ReInfo->params);
}

static void
reSaveRaceToConfigFile(const char *filename)
{
	// Note: No need to write the main file here, already done at the end of race configuration.
	char pszMainFilePathName[256];
	sprintf(pszMainFilePathName, "%sconfig/raceman/%s%s",
			GetLocalDir(), ReInfo->_reFilename, PARAMEXT);

	// Add .xml extension if not there.
	char pszSelFilePathName[256];
	sprintf(pszSelFilePathName, "%sconfig/raceman/%s/%s",
			GetLocalDir(), ReInfo->_reFilename, filename);
	const char* pszFileExt = strrchr(pszSelFilePathName, '.');
	if (!pszFileExt || strcmp(pszFileExt, PARAMEXT))
		strcat(pszSelFilePathName, PARAMEXT);

	// Copy the main file to the selected one (overwrite if already there).
	GfLogInfo("Saving race config to %s ...\n", pszSelFilePathName);
	if (!GfFileCopy(pszMainFilePathName, pszSelFilePathName))
		GfLogError("Failed to save race to selected config file %s", pszSelFilePathName);
}

static void
reOnPlayerConfig(void * /* dummy */)
{
	/* Here, we need to call OptionOptionInit each time the firing button
	   is pressed, and not only once at the Raceman menu initialization,
	   because the previous menu has to be saved (ESC, Back) and because it can be this menu,
	   as well as the Main menu */
	GfuiScreenActivate(PlayerConfigMenuInit(RacemanMenuHdle));
}

static char*
reGetLoadFileDir(char* pszDirPath, int nMaxLen)
{
	void *params = ReInfo->params;

	// For race types with more than 1 event (= 1 race on 1 track), load a race result file,
	// as the previous race standings has an influence on the next race starting grid.
	if (GfParmGetEltNb(params, RM_SECT_TRACKS) > 1)
		snprintf(pszDirPath, nMaxLen, "%sresults/%s", GetLocalDir(), ReInfo->_reFilename);

	// But for race types with only 1 event (= 1 race on 1 track), load a race config file.
	else
		snprintf(pszDirPath, nMaxLen, "%sconfig/raceman/%s", GetLocalDir(), ReInfo->_reFilename);

	return pszDirPath;
}
static void
reOnLoadRaceFromFile(void *pPrevMenu)
{
	void *params = ReInfo->params;

	fs.title = ReInfo->_reName;
	fs.prevScreen = pPrevMenu;
	fs.mode = RmFSModeLoad;

	char pszDirPath[256];
	fs.path = reGetLoadFileDir(pszDirPath, 256);

	// For race types with more than 1 event (= 1 race on 1 track), load a race result file,
	// as the previous race standings has an influence on the next race starting grid.
	if (GfParmGetEltNb(params, RM_SECT_TRACKS) > 1)
		fs.select = reLoadRaceFromResultsFile;
	
	// But for race types with only 1 event (= 1 race on 1 track), load a race config file.
	else
		fs.select = reLoadRaceFromConfigFile;

	// Fire the file selection menu.
	GfuiScreenActivate(RmFileSelect(&fs));
}

static void
reOnSaveRaceToFile(void *pPrevMenu)
{
	void *params = ReInfo->params;

	// Fill-in file selection descriptor
	fs.title = ReInfo->_reName;
	fs.prevScreen = pPrevMenu;
	fs.mode = RmFSModeSave;

	char pszDirPath[256];
	snprintf(pszDirPath, 256, "%sconfig/raceman/%s", GetLocalDir(), ReInfo->_reFilename);
	fs.path = pszDirPath;

	fs.select = reSaveRaceToConfigFile;

	// Fire the file selection menu.
	GfuiScreenActivate(RmFileSelect(&fs));
}

static bool
reCanLoadRace()
{
	void *params = ReInfo->params;

	// Determine the source folder.
	char pszDirPath[256];
	reGetLoadFileDir(pszDirPath, 256);

	// Get the list of files in the target folder.
	tFList *pFileList = GfDirGetListFiltered(pszDirPath, "", PARAMEXT);

	// Now we know what to answer.
	const bool bAnswer = (pFileList != 0);

	// Free the file list.
	GfDirFreeList(pFileList, 0, true, true);

	// Answer.
	return bAnswer;
}

static bool
reCanSaveRace()
{
	void *params = ReInfo->params;

	// Multi-events race types are automatically saved in config/raceman/results
	return GfParmGetEltNb(params, RM_SECT_TRACKS) == 1;
}

static void
reOnActivate(void * /* dummy */)
{
	void *params = ReInfo->params;

	// Set title.
	GfuiLabelSetText(RacemanMenuHdle, TitleLabelId, ReInfo->_reName);

	// Show Load/Save race buttons as needed.
	GfuiVisibilitySet(RacemanMenuHdle, LoadRaceButtonId, 
					  reCanLoadRace() ? GFUI_VISIBLE : GFUI_INVISIBLE);
	GfuiVisibilitySet(RacemanMenuHdle, SaveRaceButtonId, 
					  reCanSaveRace() ? GFUI_VISIBLE : GFUI_INVISIBLE);
}

int
ReRacemanMenu(void)
{
	void *params = ReInfo->params;

	if (strcmp(ReInfo->_reName,"Online Race")==0)
	{
		if (GetNetwork())
		{
			if (GetNetwork()->IsConnected())
			{
				if (IsClient())
				{
					reNetworkClientConnectMenu(NULL);
					return RM_ASYNC | RM_NEXT_STEP;
				}
				else if (IsServer())
				{
					reNetworkHostMenu(NULL);
					return RM_ASYNC | RM_NEXT_STEP;
				}
			}
		}
		else
		{
			reNetworkMenu(NULL);
			return RM_ASYNC | RM_NEXT_STEP;
		}

	}

	if (RacemanMenuHdle)
		GfuiScreenRelease(RacemanMenuHdle);

	// Create screen, load menu XML descriptor and create static controls.
	RacemanMenuHdle = GfuiScreenCreateEx(NULL, 
					 NULL, reOnActivate, 
					 NULL, (tfuiCallback)NULL, 
					 1);
	void *menuXMLDescHdle = LoadMenuXML("racemanmenu.xml");
	
	CreateStaticControls(menuXMLDescHdle, RacemanMenuHdle);

	// Create variable title label.
	TitleLabelId = CreateLabelControl(RacemanMenuHdle, menuXMLDescHdle, "TitleLabel");

	// Create New race, Configure race, Configure players and Back buttons.
	CreateButtonControl(RacemanMenuHdle, menuXMLDescHdle, "StartRaceButton",
						NULL, ReStartNewRace);
	CreateButtonControl(RacemanMenuHdle, menuXMLDescHdle, "ConfigureRaceButton",
						NULL, ReConfigureMenu);
	CreateButtonControl(RacemanMenuHdle, menuXMLDescHdle, "ConfigurePlayersButton",
						NULL, reOnPlayerConfig);
	
	CreateButtonControl(RacemanMenuHdle, menuXMLDescHdle, "BackButton",
						ReInfo->_reMenuScreen, GfuiScreenActivate);

	// Create Load / Save race buttons.
	LoadRaceButtonId = CreateButtonControl(RacemanMenuHdle, menuXMLDescHdle, "LoadRaceButton",
										   RacemanMenuHdle, reOnLoadRaceFromFile);
	SaveRaceButtonId = CreateButtonControl(RacemanMenuHdle, menuXMLDescHdle, "SaveRaceButton",
										   RacemanMenuHdle, reOnSaveRaceToFile);
	
	// Close menu XML descriptor.
	GfParmReleaseHandle(menuXMLDescHdle);
	
	// Register keyboard shortcuts.
	GfuiMenuDefaultKeysAdd(RacemanMenuHdle);
	GfuiAddKey(RacemanMenuHdle, GFUIK_ESCAPE, "Back to Main menu",
			   ReInfo->_reMenuScreen, GfuiScreenActivate, NULL);

	// Activate screen.
	GfuiScreenActivate(RacemanMenuHdle);

	return RM_ASYNC | RM_NEXT_STEP;
}

static void
reStateManage(void * /* dummy */)
{
	ReStateManage();
}

int
ReNewTrackMenu(void)
{
	char buf[128];

	void	*params = ReInfo->params;
	void	*results = ReInfo->results;
	int		raceNumber;
	int		 xx;

	if (NewTrackMenuHdle) {
		GfuiScreenRelease(NewTrackMenuHdle);
	}

	// Create screen, load menu XML descriptor and create static controls.
	NewTrackMenuHdle = GfuiScreenCreateEx(NULL, 
										  NULL, (tfuiCallback)NULL, 
										  NULL, (tfuiCallback)NULL, 
										  1);
	void *menuXMLDescHdle = LoadMenuXML("newtrackmenu.xml");
	CreateStaticControls(menuXMLDescHdle,NewTrackMenuHdle);

	// Create background image from race params.
	const char* pszBGImg = GfParmGetStr(params, RM_SECT_HEADER, RM_ATTR_BGIMG, 0);
	if (pszBGImg) {
		GfuiScreenAddBgImg(NewTrackMenuHdle, pszBGImg);
	}

	// Create variable title label from race params.
	int titleId = CreateLabelControl(NewTrackMenuHdle, menuXMLDescHdle, "titlelabel");
	GfuiLabelSetText(NewTrackMenuHdle, titleId, ReInfo->_reName);

	// Calculate which race of the series this is
	raceNumber = 1;
	for (xx = 1; xx < (int)GfParmGetNum(results, RE_SECT_CURRENT, RE_ATTR_CUR_TRACK, NULL, 1); ++xx) 
	{
		sprintf(buf, "%s/%d", RM_SECT_TRACKS, xx);
		if (!strcmp( GfParmGetStr(ReInfo->params, buf, RM_ATTR_NAME, "free"), "free") == 0)
			++raceNumber;
	}

	// Create variable subtitle label from race params.
	sprintf(buf, "Race Day #%d/%d on %s",
			raceNumber,
			(int)GfParmGetNum(params, RM_SECT_TRACKS, RM_ATTR_NUMBER, NULL, -1 ) >= 0 ?
			(int)GfParmGetNum(params, RM_SECT_TRACKS, RM_ATTR_NUMBER, NULL, -1 ) :
			GfParmGetEltNb(params, RM_SECT_TRACKS), 
			ReInfo->track->name);
	int subTitleId = CreateLabelControl(NewTrackMenuHdle, menuXMLDescHdle, "subtitlelabel");
	GfuiLabelSetText(NewTrackMenuHdle, subTitleId, buf);

	// Create Start and Abandon buttons.
	CreateButtonControl(NewTrackMenuHdle, menuXMLDescHdle, "startbutton", NULL, reStateManage);
	CreateButtonControl(NewTrackMenuHdle, menuXMLDescHdle, "abandonbutton", ReInfo->_reMenuScreen, GfuiScreenActivate);

	// Close menu XML descriptor.
	GfParmReleaseHandle(menuXMLDescHdle);
	
	// Register keyboard shortcuts.
	GfuiMenuDefaultKeysAdd(NewTrackMenuHdle);
	GfuiAddKey(NewTrackMenuHdle, GFUIK_RETURN, "Start Event", NULL, reStateManage, NULL);
	GfuiAddKey(NewTrackMenuHdle, GFUIK_ESCAPE, "Abandon", ReInfo->_reMenuScreen, GfuiScreenActivate, NULL);

	// Activate screen.
	GfuiScreenActivate(NewTrackMenuHdle);

	return RM_ASYNC | RM_NEXT_STEP;
}

