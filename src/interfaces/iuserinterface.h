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
	
	virtual void *createRaceScreen() = 0;
	virtual void *createRaceEventLoopHook() = 0;
	virtual void setRaceMessage(const char *msg) = 0;
	virtual void setRaceBigMessage(const char *msg) = 0;

	virtual void activateLoadingScreen(const char *title, const char *bgimg) = 0;
	virtual void addLoadingMessage(const char *text) = 0;
	virtual void shutdownLoadingScreen() = 0;

	virtual int activateRacemanMenu() = 0;
	virtual int activateNextEventMenu() = 0;

	virtual void activateStartRaceMenu(struct RmInfo *info, void *startScr, void *abortScr) = 0;
	virtual void *activateStopRaceMenu(const char* title,
									   const char* label1, const char* tip1, void *screen1,
									   const char* label2, const char* tip2, void *screen2,
									   const char* label3 = 0, const char* tip3 = 0, void *screen3 = 0,
									   const char* label4 = 0, const char* tip4 = 0, void *screen4 = 0,
									   const char* label5 = 0, const char* tip5 = 0, void *screen5 = 0) = 0;

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

	virtual void setRaceEngine(IRaceEngine& raceEngine) = 0;
};

#include <iraceengine.h>

#endif // __IUSERINTERFACE__H__
