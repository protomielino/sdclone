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
 
#include <iraceengine.h>

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
: GfModule(strShLibName, hShLibHandle), _piRaceEngine(0)
{
}

// Implementation of IUserInterface ****************************************
bool LegacyMenu::activate()
{
	// Initialize the screen.
	bool bStatus = GfScrInit();

	if (bStatus)
		
		// Enter the menu system.
		bStatus = ::MenuEntry();

	return bStatus;
}

void LegacyMenu::quit()
{
	// Quit the event loop next time.
    GfelQuit();
}

void LegacyMenu::shutdown()
{
	// Shutdown the screen.
    GfScrShutdown();
}

void LegacyMenu::update()
{
	// Note: For the moment, the graphics engine related stuff is not there ... WIP.
	
	// Update the menu part of the GUI if requested.
	if (raceEngine().data()->_refreshDisplay)
		GfuiDisplay();

	// Request that the GUI is redisplayed at the end of next event loop.
	GfelPostRedisplay();
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

void LegacyMenu::setRaceEngine(IRaceEngine& raceEngine)
{
	_piRaceEngine = &raceEngine;
}

// Accessor to the race engine.
IRaceEngine& LegacyMenu::raceEngine()
{
	return *_piRaceEngine;
}


