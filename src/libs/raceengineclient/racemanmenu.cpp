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

#include <vector>
#include <string>
#include <sstream>

#include <portability.h>
#include <tgfclient.h>

#include <raceman.h>

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
	ReConfigRunState(/*bStart=*/true);
}

// TODO: ossPathDirPath ...
static std::string
reGetLoadFileDir()
{
	const GfRaceManager* pRaceMan = ReGetRace()->getManager();

	// For race types with more than 1 event (= 1 race on 1 track), load a race result file,
	// as the previous race standings has an influence on the next race starting grid.
	std::string strDirPath(GfLocalDir());

	if (pRaceMan->getEventCount() > 1)
		strDirPath += "results/";
	
	// But for race types with only 1 event (= 1 race on 1 track), load a race config file.
	else
		strDirPath += "config/raceman/";
	
	strDirPath += pRaceMan->getId();

	return strDirPath;
}

static bool
reCanLoadRace()
{
	// Get the list of files in the target folder.
	std::string strDirPath = reGetLoadFileDir();
	tFList *pFileList = GfDirGetListFiltered(strDirPath.c_str(), "", PARAMEXT);

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
	const GfRaceManager* pRaceMan = ReGetRace()->getManager();

	// Multi-events race types are automatically saved in config/raceman/results
	return pRaceMan->getEventCount() == 1;
}

static void
reOnRaceDataChanged()
{
	const GfRaceManager* pRaceMan = ReGetRace()->getManager();

	// Get the current track.
	const GfTrack* pTrack = ReGetRace()->getTrack();

	// Set title (race type + track name).
	std::ostringstream ossText;
	ossText << pRaceMan->getName() << " at " << pTrack->getName();
	GfuiLabelSetText(ScrHandle, TitleLabelId, ossText.str().c_str());

	// Display track name, outline image and preview image
	GfuiScreenAddBgImg(ScrHandle, pTrack->getPreviewFile().c_str());
	GfuiStaticImageSet(ScrHandle, TrackOutlineImageId, pTrack->getOutlineFile().c_str());

	// Enable/Disable Load/Save race buttons as needed.
	GfuiEnable(ScrHandle, LoadRaceButtonId, 
			   reCanLoadRace() ? GFUI_ENABLE : GFUI_DISABLE);
	GfuiEnable(ScrHandle, SaveRaceButtonId, 
			   reCanSaveRace() ? GFUI_ENABLE : GFUI_DISABLE);

	// Re-load competitors scroll list from the race.
	GfuiScrollListClear(ScrHandle, CompetitorsScrollListId);
	VecCompetitorsInfo.clear();
	const std::vector<GfDriver*>& vecCompetitors = ReGetRace()->getCompetitors();
    for (int nCompIndex = 0; nCompIndex < (int)vecCompetitors.size(); nCompIndex++)
	{
		const GfDriver* pComp = vecCompetitors[nCompIndex];
		ossText.str("");
		ossText << pComp->getName() << " (" << pComp->getCar()->getName() << ')';
		VecCompetitorsInfo.push_back(ossText.str());
		GfuiScrollListInsertElement(ScrHandle, CompetitorsScrollListId,
									VecCompetitorsInfo.back().c_str(), nCompIndex+1, (void*)pComp);
		GfLogDebug("Added competitor %s (%s#%d)\n", ossText.str().c_str(),
				   pComp->getModuleName().c_str(),  pComp->getInterfaceIndex());
	}
}

static void
reLoadRaceFromResultsFile(const char *filename)
{
	const GfRaceManager* pRaceMan = ReGetRace()->getManager();

	// Determine the full path-name of the result file.
	std::ostringstream ossResFileName;
	ossResFileName << GfLocalDir() << "results/" << pRaceMan->getId() << '/' << filename;

	GfLogInfo("Restoring race from results %s ...\n", ossResFileName.str().c_str());

	// Restore the race from the result file.
	ReRaceRestore(ReGetRace()->getManager(), ossResFileName.str().c_str());
}

static void
reLoadRaceFromConfigFile(const char *filename)
{
	GfRaceManager* pRaceMan = ReGetRace()->getManager();

	// Determine the full path-name of the selected race config file.
	std::ostringstream ossSelFileName;
	ossSelFileName << GfLocalDir() << "config/raceman/" << pRaceMan->getId() << '/' << filename;

	GfLogInfo("Loading saved race from config %s ...\n", ossSelFileName.str().c_str());

	// Replace the main race config file by the selected one.
	const std::string strMainFileName = pRaceMan->getDescriptorFileName();
	if (!GfFileCopy(ossSelFileName.str().c_str(), strMainFileName.c_str()))
	{
		GfLogError("Failed to load selected race config file %s", strMainFileName.c_str());
		return;
	}
	
	// Update the race manager.
	void* hparmRaceMan = GfParmReadFile(strMainFileName.c_str(), GFPARM_RMODE_STD);
	pRaceMan->reset(hparmRaceMan, /* bClosePrevHdle= */ true);

	// Notify the race engine of the changes.
	ReRaceSelectRaceman(pRaceMan);
	
	// Update GUI.
	reOnRaceDataChanged();
}

static void
reSaveRaceToConfigFile(const char *filename)
{
	// Note: No need to write the main file here, already done at the end of race configuration.
	const GfRaceManager* pRaceMan = ReGetRace()->getManager();

	// Determine the full path-name of the target race config file (add .xml ext. if not there).
	std::ostringstream ossTgtFileName;
	ossTgtFileName << GfLocalDir() << "config/raceman/" << pRaceMan->getId() << '/' << filename;
	if (ossTgtFileName.str().rfind(PARAMEXT) != ossTgtFileName.str().length() - strlen(PARAMEXT))
		ossTgtFileName << PARAMEXT;

	// Copy the main file to the selected one (overwrite if already there).
	const std::string strMainFileName = pRaceMan->getDescriptorFileName();
	GfLogInfo("Saving race config to %s ...\n", strMainFileName.c_str());
	if (!GfFileCopy(strMainFileName.c_str(), ossTgtFileName.str().c_str()))
		GfLogError("Failed to save race to selected config file %s", ossTgtFileName.str().c_str());
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
	const GfRaceManager* pRaceMan = ReGetRace()->getManager();
	
	fs.title = pRaceMan->getName();
	fs.prevScreen = pPrevMenu;
	fs.mode = RmFSModeLoad;

	std::string strDirPath = reGetLoadFileDir();
	fs.path = strDirPath;

	// For race types with more than 1 event (= 1 race on 1 track), load a race result file,
	// as the previous race standings has an influence on the next race starting grid.
	if (pRaceMan->getEventCount() > 1)
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
	const GfRaceManager* pRaceMan = ReGetRace()->getManager();
	
	// Fill-in file selection descriptor
	fs.title = pRaceMan->getName();
	fs.prevScreen = pPrevMenu;
	fs.mode = RmFSModeSave;

	fs.path = GfLocalDir();
	fs.path += "config/raceman/";
	fs.path += pRaceMan->getId();

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
ReRacemanMenu()
{
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
