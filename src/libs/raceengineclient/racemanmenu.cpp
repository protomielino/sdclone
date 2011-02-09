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
static int TrackTitleLabelId;
static int SaveRaceConfigButtonId;
static int LoadRaceConfigButtonId;
static int LoadRaceResultsButtonId;
static int ResumeRaceButtonId;
static int StartNewRaceButtonId;
static int TrackOutlineImageId;
static int CompetitorsScrollListId;

// Vector to hold competitors scroll-list elements.
static std::vector<std::string> VecCompetitorsInfo;

// Pre-declarations of local functions.
static void reOnRaceDataChanged();

// Accessors to the menu handle -------------------------------------------------------------
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

// Callbacks for the File Select menu --------------------------------------------------------
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
	void* hparmRaceMan =
		GfParmReadFile(strMainFileName.c_str(), GFPARM_RMODE_STD | GFPARM_RMODE_REREAD);
	if (hparmRaceMan)
	{
		pRaceMan->reset(hparmRaceMan, /* bClosePrevHdle= */ true);

		// (Re-)initialize the race from the selected race manager.
		ReGetRace()->load(pRaceMan);

		// Notify the race engine of the changes (this is a non-interactive config., actually).
		ReRaceConfigure(/* bInteractive */ false);
	}
	
	// Update GUI.
	reOnRaceDataChanged();
}

static void
reLoadRaceFromResultsFile(const char *filename)
{
	GfRaceManager* pRaceMan = ReGetRace()->getManager();

	// Determine the full path-name of the result file.
	std::ostringstream ossResFileName;
	ossResFileName << GfLocalDir() << "results/" << pRaceMan->getId() << '/' << filename;

 	GfLogInfo("Restoring race from results %s ...\n", ossResFileName.str().c_str());

	// (Re-)initialize the race from the selected race manager and results params.
	void* hparmResults =
		GfParmReadFile(ossResFileName.str().c_str(), GFPARM_RMODE_STD | GFPARM_RMODE_REREAD);
	if (hparmResults)
	{
		ReGetRace()->load(pRaceMan, hparmResults);

		// Restore the race from the result file.
		ReRaceRestore(hparmResults);
	}
	
	// Update GUI.
	reOnRaceDataChanged();
}

// Callbacks for the current menu ------------------------------------------------------------
static void
reOnActivate(void * /* dummy */)
{
	GfLogTrace("Entering Race Manager menu\n");

	// Update GUI.
	reOnRaceDataChanged();
}

static void
reOnRaceDataChanged()
{
	GfRace* pRace = ReGetRace();
	const GfRaceManager* pRaceMan = pRace->getManager();

	// Get the currently selected track for the race (should never fail, unless no track at all).
	const GfTrack* pTrack = ReGetRace()->getTrack();

	// Set title (race type + track name).
	std::ostringstream ossText;
	ossText << "at " << pTrack->getName();
	GfuiLabelSetText(ScrHandle, TrackTitleLabelId, ossText.str().c_str());

	// Display track name, outline image and preview image
	GfuiScreenAddBgImg(ScrHandle, pTrack->getPreviewFile().c_str());
	GfuiStaticImageSet(ScrHandle, TrackOutlineImageId, pTrack->getOutlineFile().c_str());

	// Show/Hide "Load race" buttons as needed.
	const bool bIsMultiEvent = pRaceMan->isMultiEvent();
	GfuiVisibilitySet(ScrHandle, LoadRaceConfigButtonId,
					  !bIsMultiEvent ? GFUI_VISIBLE : GFUI_INVISIBLE);
	GfuiVisibilitySet(ScrHandle, LoadRaceResultsButtonId,
					  bIsMultiEvent ? GFUI_VISIBLE : GFUI_INVISIBLE);

	// Enable/Disable "Load/Save race" buttons as needed.
	GfuiEnable(ScrHandle, SaveRaceConfigButtonId, 
			   !bIsMultiEvent ? GFUI_ENABLE : GFUI_DISABLE);
	GfuiEnable(ScrHandle, LoadRaceConfigButtonId, 
			   !bIsMultiEvent && pRaceMan->hasSavedConfigsFiles() ? GFUI_ENABLE : GFUI_DISABLE);
	GfuiEnable(ScrHandle, LoadRaceResultsButtonId, 
			   bIsMultiEvent && pRaceMan->hasResultsFiles() ? GFUI_ENABLE : GFUI_DISABLE);

	// Show/Hide "Start / Resume race" buttons as needed.
	const std::vector<GfDriver*>& vecCompetitors = ReGetRace()->getCompetitors();
	const bool bWasLoadedFromResults = ReGetRace()->getResultsDescriptorHandle() != 0;
	GfuiVisibilitySet(ScrHandle, StartNewRaceButtonId,
					  !vecCompetitors.empty() && !bWasLoadedFromResults
					  ? GFUI_VISIBLE : GFUI_INVISIBLE);
	GfuiVisibilitySet(ScrHandle, ResumeRaceButtonId,
					  !vecCompetitors.empty() && bWasLoadedFromResults
					  ? GFUI_VISIBLE : GFUI_INVISIBLE);

	// Re-load competitors scroll list from the race.
	GfuiScrollListClear(ScrHandle, CompetitorsScrollListId);
	VecCompetitorsInfo.clear();
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

	// Show the driver at the pole position.
	if (!vecCompetitors.empty())
		GfuiScrollListShowElement(ScrHandle, CompetitorsScrollListId, 0);
}

static void
reOnSelectCompetitor(void * /* dummy */)
{
	// TODO: Display some details somewhere about the selected competitor ?
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
reOnLoadRaceFromConfigFile(void *pPrevMenu)
{
	GfRaceManager* pRaceMan = ReGetRace()->getManager();
	
	fs.title = pRaceMan->getName();
	fs.mode = RmFSModeLoad;
	fs.path = pRaceMan->getSavedConfigsDir();
	fs.select = reLoadRaceFromConfigFile;
	fs.prevScreen = pPrevMenu;

	// Fire the file selection menu.
	GfuiScreenActivate(RmFileSelect(&fs));
}

static void
reOnLoadRaceFromResultsFile(void *pPrevMenu)
{
	GfRaceManager* pRaceMan = ReGetRace()->getManager();
	
	fs.title = pRaceMan->getName();
	fs.mode = RmFSModeLoad;
	fs.path = pRaceMan->getResultsDir();
	fs.select = reLoadRaceFromResultsFile;
	fs.prevScreen = pPrevMenu;

	// Fire the file selection menu.
	GfuiScreenActivate(RmFileSelect(&fs));
}

static void
reOnSaveRaceToConfigFile(void *pPrevMenu)
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

// Init. function for the current menu -----------------------------------------------------
int
ReRacemanMenu()
{
	// Special case of the online race
	// TODO: Integrate it better.
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

	const GfRaceManager* pRaceMan = ReGetRace()->getManager();

	// Create screen, load menu XML descriptor and create static controls.
	ScrHandle = GfuiScreenCreateEx(NULL, NULL, reOnActivate, 
										 NULL, (tfuiCallback)NULL, 1);
	void *menuXMLDescHdle = LoadMenuXML("racemanmenu.xml");
	
	CreateStaticControls(menuXMLDescHdle, ScrHandle);

	// Create and initialize static title label (race mode name).
	const int nRaceModeTitleLabelId =
		CreateLabelControl(ScrHandle, menuXMLDescHdle, "RaceModeTitleLabel");
	GfuiLabelSetText(ScrHandle, nRaceModeTitleLabelId, pRaceMan->getName().c_str());

	// Create variable title label (track name).
	TrackTitleLabelId = CreateLabelControl(ScrHandle, menuXMLDescHdle, "TrackTitleLabel");

	// Create Configure race, Configure players and Back buttons.
	CreateButtonControl(ScrHandle, menuXMLDescHdle, "ConfigureRaceButton",
						NULL, ReConfigureRace);
	CreateButtonControl(ScrHandle, menuXMLDescHdle, "ConfigurePlayersButton",
						NULL, reOnPlayerConfig);
	
	CreateButtonControl(ScrHandle, menuXMLDescHdle, "PreviousButton",
						ReInfo->_reMenuScreen, GfuiScreenActivate);

	// Create "Load / Resume / Save race" buttons.
	SaveRaceConfigButtonId =
		CreateButtonControl(ScrHandle, menuXMLDescHdle, "SaveRaceConfigButton",
							ScrHandle, reOnSaveRaceToConfigFile);
	LoadRaceConfigButtonId =
		CreateButtonControl(ScrHandle, menuXMLDescHdle, "LoadRaceConfigButton",
							ScrHandle, reOnLoadRaceFromConfigFile);
	LoadRaceResultsButtonId =
		CreateButtonControl(ScrHandle, menuXMLDescHdle, "LoadRaceResultsButton",
							ScrHandle, reOnLoadRaceFromResultsFile);

	// Create "Resume / Start race" buttons.
	ResumeRaceButtonId =
		CreateButtonControl(ScrHandle, menuXMLDescHdle, "ResumeRaceButton",
							NULL, ReResumeRace);
	StartNewRaceButtonId =
		CreateButtonControl(ScrHandle, menuXMLDescHdle, "StartNewRaceButton",
							NULL, ReStartNewRace);

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
