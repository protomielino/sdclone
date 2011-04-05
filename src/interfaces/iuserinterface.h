/***************************************************************************
                 iuserinterface.h -- Interface for any user interface module

    created              : Mon Mar 7 19:32:14 CEST 2011
    copyright            : (C) 2011 by Jean-Philippe Meuret                         
    web                  : http://www.speed-dreams.org
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

/** @file   
    	Interface for any user interface module
    @version	$Id$
*/

#ifndef __IUSERINTERFACE__H__
#define __IUSERINTERFACE__H__

class IRaceEngine;

struct RmInfo;
struct CarElt;

class IUserInterface
{
public:

	//! Activation of the user interface (splash if any, main menu ...).
	virtual bool activate() = 0;
	
	virtual void quit() = 0;
	
	virtual void shutdown() = 0;

	virtual void update() = 0;

	virtual void *createRaceScreen() = 0;
	virtual void captureRaceScreen(const char* pszTargetFilename) = 0;
	
	virtual void *createRaceEventLoopHook() = 0;
	virtual void setRaceMessage(const char *msg) = 0;
	virtual void setRaceBigMessage(const char *msg) = 0;

	virtual void activateLoadingScreen(const char *title, const char *bgimg) = 0;
	virtual void addLoadingMessage(const char *text) = 0;
	virtual void shutdownLoadingScreen() = 0;

	virtual void activateGameScreen() = 0;

	virtual int activateRacemanMenu() = 0;
	virtual int activateNextEventMenu() = 0;

	virtual void activateStartRaceMenu() = 0;
	virtual void activateStopRaceMenu() = 0;

	virtual void activatePitMenu(struct CarElt *car, void (*callback)(void*)) = 0;

	virtual void *createResultsMenu() = 0;
	virtual void activateResultsMenu(void *prevHdle, struct RmInfo *reInfo) = 0;
	virtual void setResultsMenuTrackName(const char *trackName) = 0;
	virtual void setResultsMenuTitle(const char *title) = 0;
	virtual void addResultsMenuLine(const char *text) = 0;
	virtual void setResultsMenuLine(const char *text, int line, int clr) = 0;
	virtual void removeResultsMenuLine(int line) = 0;
	virtual void showResultsMenuContinueButton() = 0;
	virtual int  getResultsMenuLineCount() = 0;
	virtual void eraseResultsMenu() = 0;

	virtual void activateStandingsMenu(void *prevHdle, struct RmInfo *info, int start = 0) = 0;

	// Graphics engine control.
	virtual bool initializeGraphics() = 0;
	virtual bool loadTrackGraphics(struct Track* pTrack) = 0;
	virtual bool loadCarsGraphics(struct Situation *pSituation) = 0;
	virtual bool setupGraphicsView() = 0;
	virtual void updateGraphicsView(struct Situation *pSituation) = 0;
	virtual void unloadCarsGraphics() = 0;
	virtual void unloadTrackGraphics() = 0;
	virtual void shutdownGraphics() = 0;

	virtual void setRaceEngine(IRaceEngine& raceEngine) = 0;
};

#include <iraceengine.h>

#endif // __IUSERINTERFACE__H__
