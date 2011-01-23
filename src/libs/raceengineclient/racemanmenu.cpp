/***************************************************************************

    file        : racemanmenu.cpp
    created     : Fri Jan  3 22:24:41 CET 2003
    copyright   : (C) 2003 by Eric Espie                        
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
    		The race manager menu (where you can configure, load, save, start a race)
    @author	<a href=mailto:eric.espie@torcs.org>Eric Espie</a>
    @version	$Id$
*/

#include <cstdlib>
#include <cstdio>
#include <vector>

#include <portability.h>

#include <raceman.h>
#include <tgfclient.h>
#include <racemanagers.h>
#include <race.h>
#include <tracks.h>
#include <drivers.h>
#include <cars.h>

#include <playerconfig.h>
#include <racescreens.h>
#include <network.h>

#include "racesituation.h"
#include "racemain.h"
#include "raceinit.h"
#include "racestate.h"

#include "raceenginemenus.h"
#include "networkingmenu.h"


// Raceman menu.
static void	*ScrHandle = NULL;

// Data for the race configuration menus.
static tRmFileSelect   fs;

// Menu control Ids
static int TitleLabelId;
static int LoadRaceButtonId;
static int SaveRaceButtonId;
static int TrackOutlineImageId;
static int CompetitorsScrollListId;

// Vector to hold competitors scroll-list elements.
static std::vector<std::string> VecCompetitorsInfo;


void ReSetRacemanMenuHandle(void * handle)
{
	ScrHandle = handle;
}
void* ReGetRacemanMenuHandle()
{
	return ScrHandle;
}

void
ReConfigureRace(void * /* dummy */)
{
	void *params = ReInfo->params;

	/* Reset configuration automaton */
	GfParmSetNum(params, RM_SECT_CONF, RM_ATTR_CUR_CONF, NULL, 1);
	ReConfigRunState();
}

static char*
reGetLoadFileDir(char* pszDirPath, int nMaxLen)
{
	void *params = ReInfo->params;

	// For race types with more than 1 event (= 1 race on 1 track), load a race result file,
	// as the previous race standings has an influence on the next race starting grid.
	if (GfParmGetEltNb(params, RM_SECT_TRACKS) > 1)
		snprintf(pszDirPath, nMaxLen, "%sresults/%s", GfLocalDir(), ReInfo->_reFilename);

	// But for race types with only 1 event (= 1 race on 1 track), load a race config file.
	else
		snprintf(pszDirPath, nMaxLen, "%sconfig/raceman/%s", GfLocalDir(), ReInfo->_reFilename);

	return pszDirPath;
}

static bool
reCanLoadRace()
{
	void *params = ReInfo->params;

	// Determine the source folder.
	char pszDirPath[256];
	reGetLoadFileDir(pszDirPath, sizeof(pszDirPath));

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
reOnRaceDataChanged()
{
	char buf[128];
	void *params = ReInfo->params;

	// Retrieve track infos.
	const char* pszTrackId = GfParmGetStr(ReInfo->params, "Tracks/1", RM_ATTR_NAME, "");
	const GfTrack* pTrack = GfTracks::self()->getTrack(pszTrackId);

	// Set title (race type + track name).
	snprintf(buf, sizeof(buf), "%s at %s", ReInfo->_reName, pTrack->getName().c_str());
	GfuiLabelSetText(ScrHandle, TitleLabelId, buf);

	// Display track name, outline image and preview image
	GfuiScreenAddBgImg(ScrHandle, pTrack->getPreviewFile().c_str());
	GfuiStaticImageSet(ScrHandle, TrackOutlineImageId, pTrack->getOutlineFile().c_str());

	// Enable/Disable Load/Save race buttons as needed.
	GfuiEnable(ScrHandle, LoadRaceButtonId, 
			   reCanLoadRace() ? GFUI_ENABLE : GFUI_DISABLE);
	GfuiEnable(ScrHandle, SaveRaceButtonId, 
			   reCanSaveRace() ? GFUI_ENABLE : GFUI_DISABLE);

	// Re-load competitors scroll list from the race file.
	GfuiScrollListClear(ScrHandle, CompetitorsScrollListId);
	VecCompetitorsInfo.clear();
	const int nCompetitors = GfParmGetEltNb(ReInfo->params, RM_SECT_DRIVERS);
    for (int nCompIndex = 1; nCompIndex <= nCompetitors; nCompIndex++)
	{
		snprintf(buf, sizeof(buf), "%s/%d", RM_SECT_DRIVERS, nCompIndex);
		const char* pszCompModuleName = GfParmGetStr(ReInfo->params, buf, RM_ATTR_MODULE, "");
		int nCompItfIdx = (int)GfParmGetNum(ReInfo->params, buf, RM_ATTR_IDX, (char*)NULL, 0);

		const GfDriver* pComp =
			GfDrivers::self()->getDriver(pszCompModuleName, nCompItfIdx);
		if (pComp)
		{
			snprintf(buf, sizeof(buf), "%s (%s)", pComp->getName().c_str(), pComp->getCar()->getName().c_str());
			VecCompetitorsInfo.push_back(buf);
			GfuiScrollListInsertElement(ScrHandle, CompetitorsScrollListId,
										VecCompetitorsInfo.back().c_str(), nCompIndex, (void*)pComp);
			GfLogDebug("Added competitor %s (%s#%d)\n", buf, pszCompModuleName, nCompItfIdx);
		}
		else
			GfLogWarning("Ignoring competitor %s#%d (no such driver available)\n",
						 pszCompModuleName, nCompItfIdx);
	}
}

static void
reLoadRaceFromResultsFile(const char *filename)
{
	char pszFileName[256];

	snprintf(pszFileName, sizeof(pszFileName), "%sresults/%s/%s", GfLocalDir(), ReInfo->_reFilename, filename);
	GfLogInfo("Loading saved race from %s ...\n", pszFileName);

	// Update race data.
	ReInfo->mainResults = GfParmReadFile(pszFileName, GFPARM_RMODE_STD | GFPARM_RMODE_CREAT);
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
	snprintf(pszSelFilePathName, sizeof(pszSelFilePathName), "%sconfig/raceman/%s/%s",
			 GfLocalDir(), ReInfo->_reFilename, filename);
	GfLogInfo("Loading saved race from %s ...\n", pszSelFilePathName);

	// Replace the main race file by the selected one.
	char pszMainFilePathName[256];
	snprintf(pszMainFilePathName, sizeof(pszMainFilePathName), "%sconfig/raceman/%s%s",
			 GfLocalDir(), ReInfo->_reFilename, PARAMEXT);
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

	// Update the race manager (the params handle changed).
	GfRaceManager* pRaceMan = GfRaceManagers::self()->getRaceManager(ReInfo->_reFilename);
	if (pRaceMan)
		pRaceMan->setDescriptorHandle(ReInfo->params);
	else
		GfLogError("No such race manager (id=%s)\n", ReInfo->_reFilename);

	// Update GUI.
	reOnRaceDataChanged();
}

static void
reSaveRaceToConfigFile(const char *filename)
{
	// Note: No need to write the main file here, already done at the end of race configuration.
	char pszMainFilePathName[256];
	snprintf(pszMainFilePathName, sizeof(pszMainFilePathName), "%sconfig/raceman/%s%s",
			 GfLocalDir(), ReInfo->_reFilename, PARAMEXT);

	// Add .xml extension if not there.
	char pszSelFilePathName[256];
	snprintf(pszSelFilePathName, sizeof(pszSelFilePathName), "%sconfig/raceman/%s/%s",
			GfLocalDir(), ReInfo->_reFilename, filename);
	const char* pszFileExt = strrchr(pszSelFilePathName, '.');
	if (!pszFileExt || strcmp(pszFileExt, PARAMEXT))
		strcat(pszSelFilePathName, PARAMEXT);

	// Copy the main file to the selected one (overwrite if already there).
	GfLogInfo("Saving race config to %s ...\n", pszSelFilePathName);
	if (!GfFileCopy(pszMainFilePathName, pszSelFilePathName))
		GfLogError("Failed to save race to selected config file %s", pszSelFilePathName);
}

static void
reOnSelectCompetitor(void * /* dummy */)
{
	GfDriver* pComp = 0;
    const char *pszElementText =
		GfuiScrollListGetSelectedElement(ScrHandle, CompetitorsScrollListId, (void**)&pComp);
	if (pszElementText && pComp)
		GfLogDebug("Selecting %s\n", pComp->getName().c_str());
}

static void
reOnPlayerConfig(void * /* dummy */)
{
	/* Here, we need to call OptionOptionInit each time the firing button
	   is pressed, and not only once at the Raceman menu initialization,
	   because the previous menu has to be saved (ESC, Back) and because it can be this menu,
	   as well as the Main menu */
	GfuiScreenActivate(PlayerConfigMenuInit(ScrHandle));
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
	snprintf(pszDirPath, sizeof(pszDirPath), "%sconfig/raceman/%s",
			 GfLocalDir(), ReInfo->_reFilename);
	fs.path = pszDirPath;

	fs.select = reSaveRaceToConfigFile;

	// Fire the file selection menu.
	GfuiScreenActivate(RmFileSelect(&fs));
}

static void
reOnActivate(void * /* dummy */)
{
	// Initialize GfTracks' track module interface (needed for some track infos).
	GfTracks::self()->setTrackInterface(&ReInfo->_reTrackItf);
	
	// Update GUI.
	reOnRaceDataChanged();
}

int
ReRacemanMenu(void)
{
	void *params = ReInfo->params;

	// Special case of the online race.
	if (!strcmp(ReInfo->_reName, "Online Race"))
	{
		if (GetNetwork())
		{
			if (GetNetwork()->IsConnected())
			{
				if (IsClient())
				{
					ReNetworkClientConnectMenu(NULL);
					return RM_ASYNC | RM_NEXT_STEP;
				}
				else if (IsServer())
				{
					ReNetworkHostMenu(NULL);
					return RM_ASYNC | RM_NEXT_STEP;
				}
			}
		}
		else
		{
			ReNetworkMenu(NULL);
			return RM_ASYNC | RM_NEXT_STEP;
		}
	}

	// Don't do this twice.
	if (ScrHandle)
		GfuiScreenRelease(ScrHandle);

	// Create screen, load menu XML descriptor and create static controls.
	ScrHandle = GfuiScreenCreateEx(NULL, NULL, reOnActivate, 
										 NULL, (tfuiCallback)NULL, 1);
	void *menuXMLDescHdle = LoadMenuXML("racemanmenu.xml");
	
	CreateStaticControls(menuXMLDescHdle, ScrHandle);

	// Create variable title label.
	TitleLabelId = CreateLabelControl(ScrHandle, menuXMLDescHdle, "TitleLabel");

	// Create Start race, Configure race, Configure players and Back buttons.
	CreateButtonControl(ScrHandle, menuXMLDescHdle, "StartRaceButton",
						NULL, ReStartNewRace);
	CreateButtonControl(ScrHandle, menuXMLDescHdle, "ConfigureRaceButton",
						NULL, ReConfigureRace);
	CreateButtonControl(ScrHandle, menuXMLDescHdle, "ConfigurePlayersButton",
						NULL, reOnPlayerConfig);
	
	CreateButtonControl(ScrHandle, menuXMLDescHdle, "PreviousButton",
						ReInfo->_reMenuScreen, GfuiScreenActivate);

	// Create Load / Save race buttons.
	LoadRaceButtonId = CreateButtonControl(ScrHandle, menuXMLDescHdle, "LoadRaceButton",
										   ScrHandle, reOnLoadRaceFromFile);
	SaveRaceButtonId = CreateButtonControl(ScrHandle, menuXMLDescHdle, "SaveRaceButton",
										   ScrHandle, reOnSaveRaceToFile);

	// Track outline image.
	TrackOutlineImageId =
		CreateStaticImageControl(ScrHandle, menuXMLDescHdle, "TrackOutlineImage");

	// Competitors scroll-list
	CompetitorsScrollListId =
		CreateScrollListControl(ScrHandle, menuXMLDescHdle, "CompetitorsScrollList",
								NULL, reOnSelectCompetitor);

	// Close menu XML descriptor.
	GfParmReleaseHandle(menuXMLDescHdle);
	
	// Register keyboard shortcuts.
	GfuiMenuDefaultKeysAdd(ScrHandle);
	GfuiAddKey(ScrHandle, GFUIK_RETURN, "Start the race",
			   NULL, ReStartNewRace, NULL);
	GfuiAddKey(ScrHandle, GFUIK_ESCAPE, "Back to the Main menu",
			   ReInfo->_reMenuScreen, GfuiScreenActivate, NULL);

	// Activate screen.
	GfuiScreenActivate(ScrHandle);

	return RM_ASYNC | RM_NEXT_STEP;
}
