/***************************************************************************

    file                 : textonly.cpp
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

#include <portability.h>

#include <racemanagers.h>
#include <race.h>

#include "textonly.h"


// The TextOnlyUI singleton.
TextOnlyUI* TextOnlyUI::_pSelf = 0;

int openGfModule(const char* pszShLibName, void* hShLibHandle)
{
	// Instanciate the (only) module instance.
	TextOnlyUI::_pSelf = new TextOnlyUI(pszShLibName, hShLibHandle);

	// Register it to the GfModule module manager if OK.
	if (TextOnlyUI::_pSelf)
		GfModule::register_(TextOnlyUI::_pSelf);

	// Report about success or error.
	return TextOnlyUI::_pSelf ? 0 : 1;
}

int closeGfModule()
{
	// Unregister it from the GfModule module manager.
	if (TextOnlyUI::_pSelf)
		GfModule::unregister(TextOnlyUI::_pSelf);

	// Delete the (only) module instance.
	delete TextOnlyUI::_pSelf;
	TextOnlyUI::_pSelf = 0;

	// Report about success or error.
	return 0;
}

TextOnlyUI& TextOnlyUI::self()
{
	// Pre-condition : 1 successfull openGfModule call.
	return *_pSelf;
}

TextOnlyUI::TextOnlyUI(const std::string& strShLibName, void* hShLibHandle)
: GfModule(strShLibName, hShLibHandle), _piRaceEngine(0)
{
}

// Implementation of IUserInterface ****************************************
bool TextOnlyUI::activate()
{
	// TODO: Make the LegacyMenu version work, and then adapt it here !!!!!!!!!!!

	// Get the race to start.
	std::string strRaceToStart;
	if (!GfApp().hasOption("startrace", strRaceToStart) || strRaceToStart.empty())
		return false; // Should never happen (checked in main).

	// And run it if there's such a race manager.
	GfRaceManager* pSelRaceMan = GfRaceManagers::self()->getRaceManager(strRaceToStart);
	if (pSelRaceMan)
	{
		// Initialize the race engine.
		raceEngine().reset();

		// Give the selected race manager to the race engine.
		raceEngine().selectRaceman(pSelRaceMan);
		
		// Configure the new race.
		raceEngine().configureRace(/* bInteractive */ false);

		// Start the race engine state automaton
		raceEngine().startNewRace();
	}
	else
	{
		GfLogError("No such race manager '%s'\n", strRaceToStart.c_str());
		
		return false;
	}

	return true;
}

void TextOnlyUI::quit()
{
	// Quit the event loop next time.
	GfApp().eventLoop().postQuit();
}

void TextOnlyUI::shutdown()
{
    raceEngine().cleanup();
}

void TextOnlyUI::addLoadingMessage(const char* pszText)
{
}

void TextOnlyUI::onRaceConfiguring()
{
	// Force "result-only" mode for all the sessions of the race.
	const std::vector<std::string> vecSessionNames =
		raceEngine().race()->getManager()->getSessionNames();
	std::vector<std::string>::const_iterator itSesName;
	for (itSesName = vecSessionNames.begin(); itSesName != vecSessionNames.end(); itSesName++)
	{
		GfRace::Parameters* pSessionParams =
			raceEngine().race()->getParameters(*itSesName);
		//if (pRaceSessionParams->eDisplayMode != GfRace::nDisplayModeNumber)
		pSessionParams->eDisplayMode = GfRace::eDisplayResultsOnly;
	}
}

void TextOnlyUI::onRaceEventInitializing()
{
}

void TextOnlyUI::onRaceEventStarting()
{
}

void TextOnlyUI::onRaceInitializing()
{
}

void TextOnlyUI::onRaceStarting()
{
	// Switch to Start Race menu only if required.
	// tRmInfo* pReInfo = _piRaceEngine->inData();
	// if (!strcmp(GfParmGetStr(pReInfo->params, pReInfo->_reRaceName, RM_ATTR_SPLASH_MENU, RM_VAL_NO), RM_VAL_YES))
	// {
	// 	//GfLogDebug("TextOnlyUI::onRaceStarting() => RmLoadingScreenShutdown\n");
	// 	::RmLoadingScreenShutdown();
	
	// 	::RmDisplayStartRace();
	// }
}

void TextOnlyUI::onRaceLoadingDrivers()
{
	// // Create the game screen according to the actual display mode.
	// if (_piRaceEngine->inData()->_displayMode == RM_DISP_MODE_NORMAL)
	// 	_hscrGame = ::RmScreenInit();
	// else
	// 	_hscrGame = ::RmResScreenInit();
}

void TextOnlyUI::onRaceDriversLoaded()
{
}

void TextOnlyUI::onRaceSimulationReady()
{
}

void TextOnlyUI::onRaceStarted()
{
	// Simply activate the game screen.
	// GfuiScreenActivate(_hscrGame);
}

void TextOnlyUI::onRaceInterrupted()
{
	// ::RmStopRaceScreen();
}

void TextOnlyUI::onRaceFinished()
{
	// if (_piRaceEngine->inData()->_displayMode == RM_DISP_MODE_NORMAL)
	// {
	// 	unloadCarsGraphics();
	// 	shutdownGraphicsView();
	// 	unloadTrackGraphics();
	// }
	// else
	// {
	// 	RmResScreenShutdown();
	// }
}

void TextOnlyUI::onRaceEventFinished()
{
}

void TextOnlyUI::setResultsTableTitles(const char* pszTitle, const char* pszSubTitle)
{
	// ::RmResScreenSetTitles(pszTitle, pszSubTitle);
}

void TextOnlyUI::setResultsTableHeader(const char* pszHeader)
{
	// ::RmResScreenSetHeader(pszHeader);
}

void TextOnlyUI::addResultsTableRow(const char* pszText)
{
	// ::RmResScreenAddText(pszText);
}

void TextOnlyUI::setResultsTableRow(int nIndex, const char* pszText, bool bHighlight)
{
	// ::RmResScreenSetText(pszText, nIndex, bHighlight ? 1 : 0);
}

void TextOnlyUI::removeResultsTableRow(int nIndex)
{
	// ::RmResScreenRemoveText(nIndex);
}

int TextOnlyUI::getResultsTableRowCount() const
{
	return 0; //::RmResGetRows();
}

void TextOnlyUI::eraseResultsTable()
{
	// ::RmResEraseScreen();
}

void TextOnlyUI::showResults()
{
	// // Create the "Race Engine update state" hook if not already done.
	// if (!_hscrReUpdateStateHook)
	// 	_hscrReUpdateStateHook = ::RmInitReUpdateStateHook();

	// // This is now the "game" screen.
	// _hscrGame = _hscrReUpdateStateHook;

	// // Display the results menu (will activate the game screen on exit).
	// ::RmShowResults(_hscrGame, _piRaceEngine->inData());
}

void TextOnlyUI::showStandings()
{
	// // Create the "Race Engine update state" hook if not already done.
	// if (!_hscrReUpdateStateHook)
	// 	_hscrReUpdateStateHook = ::RmInitReUpdateStateHook();

	// // This is now the "game" screen.
	// _hscrGame = _hscrReUpdateStateHook;

	// // Display the standings menu (will activate the game screen on exit).
	// ::RmShowStandings(_hscrGame, _piRaceEngine->inData(), 0);
}

//=========================================================================
void TextOnlyUI::setRaceEngine(IRaceEngine& raceEngine)
{
	_piRaceEngine = &raceEngine;
}

// Accessor to the race engine.
IRaceEngine& TextOnlyUI::raceEngine()
{
	return *_piRaceEngine;
}
