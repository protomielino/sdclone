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

void* LegacyMenu::createRaceScreen()
{
	return ::RmScreenInit();
}

void* LegacyMenu::createRaceEventLoopHook()
{
	return ::RmHookInit();
}

void LegacyMenu::activateLoadingScreen(const char* title, const char* bgimg)
{
	::RmLoadingScreenStart(title, bgimg);
}

void LegacyMenu::addLoadingMessage(const char* msg)
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

// Results table management.
void* LegacyMenu::createResultsMenu()
{
	return ::RmResScreenInit();
}

void LegacyMenu::activateResultsMenu(void* prevHdle, tRmInfo* reInfo)
{
	::RmShowResults(prevHdle, reInfo);
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

void LegacyMenu::activateStandingsMenu(void* prevHdle, tRmInfo* reInfo, int start)
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

	return _piGraphicsEngine != 0;
}

bool LegacyMenu::loadTrackGraphics(struct Track* pTrack)
{
	return _piGraphicsEngine ? _piGraphicsEngine->loadTrack(pTrack) : false;
}

bool LegacyMenu::loadCarsGraphics(struct Situation* pSituation)
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
										_piRaceEngine->inData()->_reGameScreen);
}
// void LegacyMenu::updateGraphicsView(struct Situation* pSituation)
// {
// 	if (_piGraphicsEngine)
// 		_piGraphicsEngine->updateView(pSituation);
// }

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

void LegacyMenu::shutdownGraphicsView()
{
	if (_piGraphicsEngine)
		_piGraphicsEngine->shutdownView();
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

// Accessor to the graphics engine.
IGraphicsEngine* LegacyMenu::graphicsEngine()
{
	return _piGraphicsEngine;
}


