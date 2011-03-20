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

#include "client.h"
#include "exitmenu.h"
#include "racescreens.h"

#include "legacymenu.h"


// The LegacyMenu singleton.
LegacyMenu* LegacyMenu::_pSelf = 0;

int GfModuleOpen(const char* pszShLibName, void* hShLibHandle)
{
	// Instanciate the (only) module instance.
	LegacyMenu::_pSelf = new LegacyMenu(pszShLibName, hShLibHandle);

	// Register it to the GfModule module manager if OK.
	if (LegacyMenu::_pSelf)
		GfModule::register_(LegacyMenu::_pSelf);

	// Report about success or error.
	return LegacyMenu::_pSelf ? 0 : 1;
}

int GfModuleClose()
{
	// Delete the (only) module instance.
	delete LegacyMenu::_pSelf;
	LegacyMenu::_pSelf = 0;

	// Always successfull.
	return 0;
}

LegacyMenu& LegacyMenu::self()
{
	// Pre-condition : 1 successfull GfModuleOpen call.
	return *_pSelf;
}

LegacyMenu::LegacyMenu(const std::string& strShLibName, void* hShLibHandle)
: GfModule(strShLibName, hShLibHandle), _piRaceEngine(0)
{
}

// Implementation of IUserInterface.
bool LegacyMenu::activate()
{
	return ::MenuEntry();
}

void* LegacyMenu::createExitMenu(void* prevHdle)
{
	return ::ExitMenuInit(prevHdle);
}

void *LegacyMenu::createRaceScreen()
{
	return ::ReScreenInit();
}

void *LegacyMenu::createRaceEventLoopHook()
{
	return ::ReHookInit();
}

void LegacyMenu::setRaceMessage(const char *msg)
{
	::ReSetRaceMsg(msg);
}

void LegacyMenu::setRaceBigMessage(const char *msg)
{
	::ReSetRaceBigMsg(msg);
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

int LegacyMenu::activateRacemanMenu()
{
	return ::ReRacemanMenu();
}
int LegacyMenu::activateNextEventMenu()
{
	return ::ReNextEventMenu();
}

void LegacyMenu::activateStartRaceMenu(tRmInfo *reInfo, void *startScr, void *abortScr)
{
	::RmDisplayStartRace(reInfo, startScr, abortScr);
}

void *LegacyMenu::activateStopRaceMenu(const char* title,
									   const char* label1, const char* tip1, void *screen1,
									   const char* label2, const char* tip2, void *screen2,
									   const char* label3, const char* tip3, void *screen3,
									   const char* label4, const char* tip4, void *screen4,
									   const char* label5, const char* tip5, void *screen5)
{
	return ::RmStopRaceScreen(title,
					   label1, tip1, screen1, label2, tip2, screen2,
					   label3, tip3, screen3, label4, tip4, screen4,
					   label5, tip5, screen5);
}

void LegacyMenu::activatePitMenu(tCarElt *car, tfuiCallback callback)
{
	::RmPitMenuStart(car, callback);
}

void *LegacyMenu::createResultsMenu()
{
	return ::ReResScreenInit();
}
void LegacyMenu::activateResultsMenu(void *prevHdle, tRmInfo *reInfo)
{
	::RmShowResults(prevHdle, reInfo);
}
void LegacyMenu::setResultsMenuTrackName(const char *trackName)
{
	::ReResScreenSetTrackName(trackName);
}
void LegacyMenu::setResultsMenuTitle(const char *title)
{
	::ReResScreenSetTitle(title);
}
void LegacyMenu::addResultsMenuLine(const char *text)
{
	::ReResScreenAddText(text);
}
void LegacyMenu::setResultsMenuLine(const char *text, int line, int clr)
{
	::ReResScreenSetText(text, line, clr);
}
void LegacyMenu::removeResultsMenuLine(int line)
{
	::ReResScreenRemoveText(line);
}
void LegacyMenu::showResultsMenuContinueButton()
{
	::ReResShowCont();
}
int  LegacyMenu::getResultsMenuLineCount()
{
	return ::ReResGetLines();
}
void LegacyMenu::eraseResultsMenu()
{
	::ReResEraseScreen();
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


