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

#include <tgf.hpp>
#include <tgfclient.h>

#include "client.h"
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
: GfModule(strShLibName, hShLibHandle), _piRaceEngine(0), _piGraphicsEngine(0)
{
}

// Implementation of IUserInterface ****************************************
bool LegacyMenu::activate()
{
	return ::MenuEntry();
}

void LegacyMenu::quit()
{
	// Quit the event loop next time.
	GfuiApp().eventLoop().postQuit();
}

void LegacyMenu::shutdown()
{
	// Nothing to do here.
}

void LegacyMenu::update()
{
	// Note: For the moment, the graphics engine related stuff is not there ... WIP.
	
	// Update the menu part of the GUI if requested.
	if (raceEngine().data()->_refreshDisplay)
		GfuiDisplay();

	// Request that the GUI is redisplayed at the end of next event loop.
	GfuiApp().eventLoop().postRedisplay();
}

void *LegacyMenu::createRaceScreen()
{
	return ::RmScreenInit();
}

void LegacyMenu::captureRaceScreen(const char* pszTargetFilename)
{
    ::RmScreenCapture(pszTargetFilename);
}

void *LegacyMenu::createRaceEventLoopHook()
{
	return ::RmHookInit();
}

void LegacyMenu::setRaceMessage(const char *msg)
{
	::RmSetRaceMsg(msg);
}

void LegacyMenu::setRaceBigMessage(const char *msg)
{
	::RmSetRaceBigMsg(msg);
}

void LegacyMenu::activateLoadingScreen(const char *title, const char *bgimg)
{
	::RmLoadingScreenStart(title, bgimg);
}
void LegacyMenu::addLoadingMessage(const char *msg)
{
	::RmLoadingScreenSetText(msg);
}
void LegacyMenu::shutdownLoadingScreen()
{
	::RmLoadingScreenShutdown();
}

void LegacyMenu::activateGameScreen()
{
	::RmGameScreen();
}

int LegacyMenu::activateRacemanMenu()
{
	return ::RmRacemanMenu();
}
int LegacyMenu::activateNextEventMenu()
{
	return ::RmNextEventMenu();
}

void LegacyMenu::activateStartRaceMenu()
{
	::RmDisplayStartRace();
}

void LegacyMenu::activateStopRaceMenu()
{
	::RmStopRaceScreen();
}

void LegacyMenu::activatePitMenu(tCarElt *car, tfuiCallback callback)
{
	::RmPitMenuStart(car, callback);
}

void *LegacyMenu::createResultsMenu()
{
	return ::RmResScreenInit();
}
void LegacyMenu::activateResultsMenu(void *prevHdle, tRmInfo *reInfo)
{
	::RmShowResults(prevHdle, reInfo);
}
void LegacyMenu::setResultsMenuTrackName(const char *trackName)
{
	::RmResScreenSetTrackName(trackName);
}
void LegacyMenu::setResultsMenuTitle(const char *title)
{
	::RmResScreenSetTitle(title);
}
void LegacyMenu::addResultsMenuLine(const char *text)
{
	::RmResScreenAddText(text);
}
void LegacyMenu::setResultsMenuLine(const char *text, int line, int clr)
{
	::RmResScreenSetText(text, line, clr);
}
void LegacyMenu::removeResultsMenuLine(int line)
{
	::RmResScreenRemoveText(line);
}
void LegacyMenu::showResultsMenuContinueButton()
{
	::RmResShowCont();
}
int  LegacyMenu::getResultsMenuLineCount()
{
	return ::RmResGetLines();
}
void LegacyMenu::eraseResultsMenu()
{
	::RmResEraseScreen();
}

void LegacyMenu::activateStandingsMenu(void *prevHdle, tRmInfo *reInfo, int start)
{
	::RmShowStandings(prevHdle, reInfo, start);
}

// Graphics engine control.
bool LegacyMenu::initializeGraphics()
{
	// Check if the module is already loaded, and do nothing more if so.
	if (_piGraphicsEngine)
		return true;

	// Load the graphics module
	std::ostringstream ossModLibName;
	ossModLibName << GfLibDir() << "modules/graphic/"
				  << GfParmGetStr(_piRaceEngine->data()->_reParam, "Modules", "graphic", "")
				  << '.' << DLLEXT;
	GfModule* pmodGrEngine = GfModule::load(ossModLibName.str());

	// Check that it implements IGraphicsEngine.
	if (pmodGrEngine)
		_piGraphicsEngine = pmodGrEngine->getInterface<IGraphicsEngine>();
	if (!_piGraphicsEngine)
		GfLogError("IGraphicsEngine not implemented by %s\n", ossModLibName.str().c_str());

	return _piGraphicsEngine != 0;
}

bool LegacyMenu::loadTrackGraphics(struct Track* pTrack)
{
	return _piGraphicsEngine ? _piGraphicsEngine->loadTrack(pTrack) : false;
}

bool LegacyMenu::loadCarsGraphics(struct Situation *pSituation)
{
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
	
	// Setup the graphics view.
	return _piGraphicsEngine->setupView((sw-vw)/2, (sh-vh)/2, vw, vh,
										_piRaceEngine->data()->_reGameScreen);
}
void LegacyMenu::updateGraphicsView(struct Situation *pSituation)
{
	if (_piGraphicsEngine)
		_piGraphicsEngine->updateView(pSituation);
}

void LegacyMenu::unloadCarsGraphics()
{
	if (_piGraphicsEngine)
		_piGraphicsEngine->unloadCars();
}

void LegacyMenu::unloadTrackGraphics()
{
	if (_piGraphicsEngine)
		_piGraphicsEngine->unloadTrack();
}

void LegacyMenu::shutdownGraphics()
{
	// Do nothing if the module has already been unloaded.
	if (!_piGraphicsEngine)
		return;

	// Unload the graphics module.
	GfModule* pmodGrEngine = dynamic_cast<GfModule*>(_piGraphicsEngine);
	GfModule::unload(pmodGrEngine);

	// And remember it was.
	_piGraphicsEngine = 0;
}

void LegacyMenu::setRaceEngine(IRaceEngine& raceEngine)
{
	_piRaceEngine = &raceEngine;
}

// Accessor to the race engine.
IRaceEngine& LegacyMenu::raceEngine()
{
	return *_piRaceEngine;
}


