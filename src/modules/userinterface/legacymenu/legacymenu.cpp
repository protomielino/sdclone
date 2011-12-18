/***************************************************************************

    file                 : legacymenu.cpp
    copyright            : (C) 2011 by Jean-Philippe Meuret                        
    email                : pouillot@users.sourceforge.net   
    version              : $Id$

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
 
#include <string>
#include <sstream>

#include <iraceengine.h>
#include <igraphicsengine.h>
#include <isoundengine.h>

#include <tgf.hpp>
#include <portability.h>
#include <tgfclient.h>

#include <racemanagers.h>

#include "splash.h"
#include "mainmenu.h"
#include "exitmenu.h"
#include "racescreens.h"

#include "legacymenu.h"


// The LegacyMenu singleton.
LegacyMenu* LegacyMenu::_pSelf = 0;

int openGfModule(const char* pszShLibName, void* hShLibHandle)
{
	// Instanciate the (only) module instance.
	LegacyMenu::_pSelf = new LegacyMenu(pszShLibName, hShLibHandle);

	// Register it to the GfModule module manager if OK.
	if (LegacyMenu::_pSelf)
		GfModule::register_(LegacyMenu::_pSelf);

	// Report about success or error.
	return LegacyMenu::_pSelf ? 0 : 1;
}

int closeGfModule()
{
	// Unregister it from the GfModule module manager.
	if (LegacyMenu::_pSelf)
		GfModule::unregister(LegacyMenu::_pSelf);

	// Delete the (only) module instance.
	delete LegacyMenu::_pSelf;
	LegacyMenu::_pSelf = 0;

	// Report about success or error.
	return 0;
}

LegacyMenu& LegacyMenu::self()
{
	// Pre-condition : 1 successfull openGfModule call.
	return *_pSelf;
}

LegacyMenu::LegacyMenu(const std::string& strShLibName, void* hShLibHandle)
: GfModule(strShLibName, hShLibHandle), _piRaceEngine(0), _piGraphicsEngine(0),
  _hscrReUpdateStateHook(0), _hscrGame(0), _bfGraphicsState(0)
{
}

bool LegacyMenu::backLoad()
{
	GfLogTrace("Pre-loading menu and game data ...\n");

	// Pre-load the main and race select menus
	// (to be able to get back to them, even when directly starting a given race).
	if (!RmRaceSelectInit(MainMenuInit()))
		return false;

	// Pre-load race managers, drivers, tracks, cars stuff.
	if (!GfRaceManagers::self())
		return false;

	GfLogTrace("Pre-loading menu and game data completed.\n");

	return true;
}

bool LegacyMenu::activateMainMenu()
{
	return MainMenuRun() == 0;
}

bool LegacyMenu::startRace()
{
	// Get the race to start.
	std::string strRaceToStart;
	if (!GfApp().hasOption("startrace", strRaceToStart))
		return false;

	// And run it if there's such a race manager.
	GfRaceManager* pSelRaceMan = GfRaceManagers::self()->getRaceManager(strRaceToStart);
	if (pSelRaceMan) // Should never happen (checked in activate).
	{
		// Initialize the race engine.
		LmRaceEngine().reset();

		// Give the selected race manager to the race engine.
		LmRaceEngine().selectRaceman(pSelRaceMan);
		
		// Configure the new race (but don't enter the config. menu tree).
		LmRaceEngine().configureRace(/* bInteractive */ false);

		// Start the race engine state automaton
		LmRaceEngine().startNewRace();
	}
	else
	{
		GfLogError("No such race manager '%s'\n", strRaceToStart.c_str());
		
		return false;
	}

	return true;
}

// Implementation of IUserInterface ****************************************
bool LegacyMenu::activate()
{
	bool (*fnSplashBackWork)(void) = LegacyMenu::backLoad;
	bool (*fnOnSplashClosed)(void) = 0;
	bool bInteractive = true;

	// Get the race to start, if any specified.
	std::string strRaceToStart;
	if (!GfApp().hasOption("startrace", strRaceToStart) || strRaceToStart.empty())
	{
		// If not specified, simply open the splash screen, load the menus in the background
		// and finally open the main menu.
		fnOnSplashClosed = LegacyMenu::activateMainMenu;
	}
	
	// Or else run the selected race.
	else
	{
		// Open the splash screen, load some stuff in the background
		// and finally start the specified race.
		fnOnSplashClosed = LegacyMenu::startRace;
		bInteractive = false;
	}

	return SplashScreen(fnSplashBackWork, fnOnSplashClosed, bInteractive);
}

void LegacyMenu::quit()
{
	// Quit the event loop next time.
	GfuiApp().eventLoop().postQuit();
}

void LegacyMenu::shutdown()
{
	// Shutdown graphics in case relevant and not already done.
	if (_piRaceEngine->inData()->_displayMode == RM_DISP_MODE_NORMAL)
	{
		unloadCarsGraphics();
		shutdownGraphicsView();
		unloadTrackGraphics();
		shutdownGraphics(/*bUnloadModule=*/true);
	}

	// Shutdown stuff that needs it.
	::RmStopRaceMenuShutdown();
	::RmStartRaceMenuShutdown();
	::RmShutdownReUpdateStateHook();
}

void LegacyMenu::activateLoadingScreen()
{
	::RmLoadingScreenStart(_piRaceEngine->inData()->_reName, "data/img/splash-raceload.jpg");
}

void LegacyMenu::addLoadingMessage(const char* pszText)
{
	::RmLoadingScreenSetText(pszText);
}

void LegacyMenu::shutdownLoadingScreen()
{
	::RmLoadingScreenShutdown();
}

void LegacyMenu::onRaceConfiguring()
{
	::RmRacemanMenu();
}

void LegacyMenu::onRaceEventInitializing()
{
	activateLoadingScreen();
}

bool LegacyMenu::onRaceEventStarting()
{
	tRmInfo* pReInfo = _piRaceEngine->inData();
	if (GfParmGetEltNb(pReInfo->params, RM_SECT_TRACKS) > 1)
	{
		::RmNextEventMenu();

		return false; // Tell the race engine state automaton to stop looping (enter the menu).
	}
		
	return true; // Tell the race engine state automaton to go on looping.
}

void LegacyMenu::onRaceInitializing()
{
	// Activate the loading screen only if not a practice or qualification session,
	// or else if we are loading the 1st competitor of the race.
	tRmInfo* pReInfo = _piRaceEngine->inData();
	if ((pReInfo->s->_raceType == RM_TYPE_QUALIF || pReInfo->s->_raceType == RM_TYPE_PRACTICE)
		&& pReInfo->s->_totTime < 0.0f) // <= What's this time test for ?
	{
		if ((int)GfParmGetNum(pReInfo->results, RE_SECT_CURRENT, RE_ATTR_CUR_DRIVER, 0, 1) == 1)
		{
			activateLoadingScreen();
		}
		else
		{
			shutdownLoadingScreen();
		}
	}
	else
	{
		activateLoadingScreen();
	}
}

void LegacyMenu::onRaceStarting()
{
	// Switch to Start Race menu only if required (no loading screen in this case).
	tRmInfo* pReInfo = _piRaceEngine->inData();
	if (!strcmp(GfParmGetStr(pReInfo->params, pReInfo->_reRaceName, RM_ATTR_SPLASH_MENU, RM_VAL_NO), RM_VAL_YES))
	{
		shutdownLoadingScreen();
	
		::RmStartRaceMenu();
	}
}

void LegacyMenu::onRaceLoadingDrivers()
{
	// Create the game screen according to the actual display mode.
	if (_piRaceEngine->inData()->_displayMode == RM_DISP_MODE_NORMAL)
		_hscrGame = ::RmScreenInit();
	else
		_hscrGame = ::RmResScreenInit();
	
	// If neither a qualification, nor a practice, or else 1st driver,
	// activate race loading screen.
	if (!(_piRaceEngine->inData()->s->_raceType == RM_TYPE_QUALIF
		  || _piRaceEngine->inData()->s->_raceType == RM_TYPE_PRACTICE)
		|| (int)GfParmGetNum(_piRaceEngine->inData()->results, RE_SECT_CURRENT, RE_ATTR_CUR_DRIVER, NULL, 1) == 1)
	{
		activateLoadingScreen();
	}
}

void LegacyMenu::onRaceDriversLoaded()
{
	if (_piRaceEngine->inData()->_displayMode == RM_DISP_MODE_NORMAL)
	{
		// It must be done after the cars are loaded and the track is loaded.
		// The track will be unloaded if the event ends.
		// The graphics module is kept open if more than one race is driven.

		// Initialize the graphics engine.
		if (initializeGraphics())
		{
			char buf[128];
			snprintf(buf, sizeof(buf), "Loading graphics for %s track ...",
					 _piRaceEngine->inData()->track->name);
			addLoadingMessage(buf);

			// Initialize the track graphics.
			loadTrackGraphics(_piRaceEngine->inData()->track);
		}
	}
}

void LegacyMenu::onRaceSimulationReady()
{
	if (_piRaceEngine->inData()->_displayMode == RM_DISP_MODE_NORMAL)
	{
		// Initialize the graphics view.
		setupGraphicsView();

		// Initialize cars graphics.
		addLoadingMessage("Loading graphics for all cars ...");
		
		loadCarsGraphics(_piRaceEngine->outData()->s);
	}
}

void LegacyMenu::onRaceStarted()
{
	// Shutdown the loading screen if not already done.
	shutdownLoadingScreen();
	
	// Activate the game screen.
	GfuiScreenActivate(_hscrGame);
}

void LegacyMenu::onLapCompleted(int nLapIndex)
{
	if (nLapIndex <= 0)
		return;

	GfLogInfo("Lap #%d completed.\n", nLapIndex);
}

void LegacyMenu::onRaceInterrupted()
{
	::RmStopRaceMenu();
}

void LegacyMenu::onRaceFinished()
{
	if (_piRaceEngine->inData()->_displayMode == RM_DISP_MODE_NORMAL)
	{
		unloadCarsGraphics();
		shutdownGraphicsView();
		unloadTrackGraphics();
		RmScreenShutdown();
	}
	else
	{
		RmResScreenShutdown();
	}
}

void LegacyMenu::onRaceEventFinished()
{
	if (_piRaceEngine->inData()->_displayMode == RM_DISP_MODE_NORMAL)
	{
		unloadTrackGraphics();

		shutdownGraphicsView();
	}
}

void LegacyMenu::setResultsTableTitles(const char* pszTitle, const char* pszSubTitle)
{
	::RmResScreenSetTitles(pszTitle, pszSubTitle);
}

void LegacyMenu::setResultsTableHeader(const char* pszHeader)
{
	::RmResScreenSetHeader(pszHeader);
}

void LegacyMenu::addResultsTableRow(const char* pszText)
{
	::RmResScreenAddText(pszText);
}

void LegacyMenu::setResultsTableRow(int nIndex, const char* pszText, bool bHighlight)
{
	::RmResScreenSetText(pszText, nIndex, bHighlight ? 1 : 0);
}

void LegacyMenu::removeResultsTableRow(int nIndex)
{
	::RmResScreenRemoveText(nIndex);
}

int LegacyMenu::getResultsTableRowCount() const
{
	return ::RmResGetRows();
}

void LegacyMenu::eraseResultsTable()
{
	::RmResEraseScreen();
}

bool LegacyMenu::showResults()
{
	// Create the "Race Engine update state" hook if not already done.
	if (!_hscrReUpdateStateHook)
		_hscrReUpdateStateHook = ::RmInitReUpdateStateHook();

	// This is now the "game" screen.
	_hscrGame = _hscrReUpdateStateHook;

	// Display the results menu (will activate the game screen on exit).
	::RmShowResults(_hscrGame, _piRaceEngine->inData());

	return false; // Tell the race engine state automaton to stop looping (enter the menu).
}

bool LegacyMenu::showStandings()
{
	// Create the "Race Engine update state" hook if not already done.
	if (!_hscrReUpdateStateHook)
		_hscrReUpdateStateHook = ::RmInitReUpdateStateHook();

	// This is now the "game" screen.
	_hscrGame = _hscrReUpdateStateHook;

	// Display the standings menu (will activate the game screen on exit).
	::RmShowStandings(_hscrGame, _piRaceEngine->inData(), 0);

	// Tell the race engine state automaton to stop looping (enter the standings menu).
	return false;
}

void LegacyMenu::activateGameScreen()
{
	GfuiScreenActivate(_hscrGame);
}

// Graphics engine control =====================================================
bool LegacyMenu::initializeGraphics()
{
	// Check if the module is already loaded, and do nothing more if so.
	if (_piGraphicsEngine)
		return true;

	// Load the graphics module
	std::ostringstream ossModLibName;
	ossModLibName << GfLibDir() << "modules/graphic/"
				  << GfParmGetStr(_piRaceEngine->inData()->_reParam, "Modules", "graphic", "")
				  << '.' << DLLEXT;
	GfModule* pmodGrEngine = GfModule::load(ossModLibName.str());

	// Check that it implements IGraphicsEngine.
	if (pmodGrEngine)
		_piGraphicsEngine = pmodGrEngine->getInterface<IGraphicsEngine>();
	if (pmodGrEngine && !_piGraphicsEngine)
	{
		GfModule::unload(pmodGrEngine);
		GfLogError("IGraphicsEngine not implemented by %s\n", ossModLibName.str().c_str());
	}

	_bfGraphicsState = 0;

	return _piGraphicsEngine != 0;
}

bool LegacyMenu::loadTrackGraphics(struct Track* pTrack)
{
	if (!_piGraphicsEngine)
		return false;

	_bfGraphicsState |= eTrackLoaded;
	
	return _piGraphicsEngine->loadTrack(pTrack);
}

bool LegacyMenu::loadCarsGraphics(struct Situation* pSituation)
{
	if (!_piGraphicsEngine)
		return false;

	_bfGraphicsState |= eCarsLoaded;
	
	return _piGraphicsEngine ? _piGraphicsEngine->loadCars(pSituation) : false;
}

bool LegacyMenu::setupGraphicsView()
{
	// Initialize the graphics view.
	if (!_piGraphicsEngine)
		return false;
	
	// Retrieve the screen dimensions.
	int sw, sh, vw, vh;
	GfScrGetSize(&sw, &sh, &vw, &vh);
	
	_bfGraphicsState |= eViewSetup;
	
	// Setup the graphics view.
	return _piGraphicsEngine->setupView((sw-vw)/2, (sh-vh)/2, vw, vh, _hscrGame);
}

void LegacyMenu::redrawGraphicsView(struct Situation* pSituation)
{
	if (!_piGraphicsEngine)
		return;
	
	_piGraphicsEngine->redrawView(pSituation);
}

void LegacyMenu::unloadCarsGraphics()
{
	if (!_piGraphicsEngine)
		return;
	
	if (_bfGraphicsState & eCarsLoaded)
	{
		_piGraphicsEngine->unloadCars();
		_bfGraphicsState &= ~eCarsLoaded;
	}
}

void LegacyMenu::unloadTrackGraphics()
{
	if (!_piGraphicsEngine)
		return;
	
	if (_bfGraphicsState & eTrackLoaded)
	{
		_piGraphicsEngine->unloadTrack();
		_bfGraphicsState &= ~eTrackLoaded;
	}
}

void LegacyMenu::shutdownGraphicsView()
{
	if (!_piGraphicsEngine)
		return;
	
	if (_bfGraphicsState & eViewSetup)
	{
		_piGraphicsEngine->shutdownView();
		_bfGraphicsState &= ~eViewSetup;
	}
}

void LegacyMenu::shutdownGraphics(bool bUnloadModule)
{
	// Do nothing if the module has already been unloaded.
	if (!_piGraphicsEngine)
		return;

	if (bUnloadModule)
	{
		// Unload the graphics module.
		GfModule* pmodGrEngine = dynamic_cast<GfModule*>(_piGraphicsEngine);
#ifndef UNLOAD_SSGGRAPH
		if (pmodGrEngine->getSharedLibName().find("ssggraph") == std::string::npos)
#endif
		GfModule::unload(pmodGrEngine);

		// And remember it was.
		_piGraphicsEngine = 0;
	}

	// A little consistency check.
	if (_bfGraphicsState)
		GfLogWarning("Graphics shutdown procedure not smartly completed (state = 0x%x)\n",
					 _bfGraphicsState);
}

//=========================================================================
void LegacyMenu::setRaceEngine(IRaceEngine& raceEngine)
{
	_piRaceEngine = &raceEngine;
}

// Accessor to the race engine.
IRaceEngine& LegacyMenu::raceEngine()
{
	return *_piRaceEngine;
}

// Accessor to the graphics engine.
IGraphicsEngine* LegacyMenu::graphicsEngine()
{
	return _piGraphicsEngine;
}

// Accessor to the sound engine.
ISoundEngine* LegacyMenu::soundEngine()
{
	return dynamic_cast<ISoundEngine*>(_piGraphicsEngine);
}
