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

	virtual void* createRaceScreen() = 0;
	
	virtual void* createRaceEventLoopHook() = 0;

	virtual void activateLoadingScreen(const char* title, const char* bgimg) = 0;
	virtual void addLoadingMessage(const char* text) = 0;
	virtual void shutdownLoadingScreen() = 0;

	virtual void activateGameScreen() = 0;

	virtual int activateRacemanMenu() = 0;
	virtual int activateNextEventMenu() = 0;

	virtual void activateStartRaceMenu() = 0;
	virtual void activateStopRaceMenu() = 0;

	// Results table management.
	virtual void* createResultsMenu() = 0;
	virtual void activateResultsMenu(void* prevHdle, struct RmInfo* reInfo) = 0;
	virtual void setResultsTableTitles(const char* pszTitle, const char* pszSubTitle) = 0;
	virtual void setResultsTableHeader(const char* pszHeader) = 0;
	virtual void addResultsTableRow(const char* pszText) = 0;
	virtual void setResultsTableRow(int nIndex, const char* pszText,
									bool bHighlight = false) = 0;
	virtual void removeResultsTableRow(int nIndex) = 0;
	virtual void eraseResultsTable() = 0;
	virtual int  getResultsTableRowCount() const = 0;

	virtual void activateStandingsMenu(void* prevHdle, struct RmInfo* info, int start = 0) = 0;

	// TODO: Move this to a new separate IGraphicsUserInterface interface ?
	// Graphics engine control.
	virtual bool initializeGraphics() = 0;
	virtual bool loadTrackGraphics(struct Track* pTrack) = 0;
	virtual bool loadCarsGraphics(struct Situation* pSituation) = 0;
	virtual bool setupGraphicsView() = 0;
	virtual void shutdownGraphicsView() = 0;
	virtual void unloadCarsGraphics() = 0;
	virtual void unloadTrackGraphics() = 0;
	virtual void shutdownGraphics() = 0;

	virtual void setRaceEngine(IRaceEngine& raceEngine) = 0;
};

#include <iraceengine.h>

#endif // __IUSERINTERFACE__H__
