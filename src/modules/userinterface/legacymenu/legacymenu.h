/***************************************************************************

    file        : legacymenu.h
    copyright   : (C) 2011 by Jean-Philippe Meuret
    email       : pouillot@users.sourceforge.net
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
    		The "legacy menu" user interface module
    @version    $Id$
*/

#ifndef _LEGACYMENU_H_
#define _LEGACYMENU_H_

#include <iuserinterface.h>

#include <tgf.hpp>

class IGraphicsEngine;
struct Situation;


// DLL exported symbols declarator for Windows.
#ifdef WIN32
# ifdef LEGACYMENU_DLL
#  define LEGACYMENU_API __declspec(dllexport)
# else
#  define LEGACYMENU_API __declspec(dllimport)
# endif
#else
# define LEGACYMENU_API
#endif


// The C interface of the module.
extern "C" int LEGACYMENU_API openGfModule(const char* pszShLibName, void* hShLibHandle);
extern "C" int LEGACYMENU_API closeGfModule();

// The module main class (inherits GfModule, and implements IUserInterface).
class LEGACYMENU_API LegacyMenu : public GfModule, public IUserInterface
{
	// Implementation of IUserInterface.
public:

	//! Activation of the user interface (splash if any, main menu ...).
	virtual bool activate();
	
	//! Request exit of the event loop.
	virtual void quit();
	
	//! Termination of the user interface.
	virtual void shutdown();

	// Race state change notifications.
	virtual void onRaceConfiguring();
	virtual void onRaceEventInitializing();
	virtual void onRaceEventStarting();
	virtual void onRaceInitializing();
	virtual void onRaceStarting();
	virtual void onRaceLoadingDrivers();
	virtual void onRaceDriversLoaded();
	virtual void onRaceSimulationReady();
	virtual void onRaceStarted();
	virtual void onRaceInterrupted();
	virtual void onRaceFinished();
	virtual void onRaceEventFinished();
	
	// Loading messages management.
	virtual void addLoadingMessage(const char* pszText);

	// Blind-race results table management.
	virtual void setResultsTableTitles(const char* pszTitle, const char* pszSubTitle);
	virtual void setResultsTableHeader(const char* pszHeader);
	virtual void addResultsTableRow(const char* pszText);
	virtual void setResultsTableRow(int nIndex, const char* pszText, bool bHighlight = false);
	virtual void removeResultsTableRow(int nIndex);
	virtual void eraseResultsTable();
	virtual int  getResultsTableRowCount() const;

	// Results and standings tables.
	virtual void showResults();
	virtual void showStandings();

	// Setter for the race engine.
	virtual void setRaceEngine(IRaceEngine& raceEngine);

public:

	// Accessor to the singleton.
	static LegacyMenu& self();

	//! Accessor to the race engine.
	IRaceEngine& raceEngine();

	// Graphics engine control.
	void redrawGraphicsView(struct Situation* pSituation);
	void shutdownGraphics();

	// Loading screen management.
	virtual void activateLoadingScreen();
	virtual void shutdownLoadingScreen();

	//! Game screen management.
	void activateGameScreen();

	//! Accessor to the graphics engine.
	IGraphicsEngine* graphicsEngine();

 protected:

	// Protected constructor to avoid instanciation outside (but friends).
	LegacyMenu(const std::string& strShLibName, void* hShLibHandle);
	
	// Make the C interface functions nearly member functions.
	friend int openGfModule(const char* pszShLibName, void* hShLibHandle);
	friend int closeGfModule();

	// Graphics engine control.
	bool initializeGraphics();
	bool loadTrackGraphics(struct Track* pTrack);
	bool loadCarsGraphics(struct Situation* pSituation);
	bool setupGraphicsView();
	void shutdownGraphicsView();
	void unloadCarsGraphics();
	void unloadTrackGraphics();
	
	
 protected:

	// The singleton.
	static LegacyMenu* _pSelf;

	// The race engine.
	IRaceEngine* _piRaceEngine;

	// The graphics engine.
	IGraphicsEngine* _piGraphicsEngine;

	// The "Race Engine update state" hook (a GfuiScreenActivate'able object).
	void* _hscrReUpdateStateHook;
	
	// The game screen.
	void* _hscrGame;
};

//! Shortcut to the race engine.
inline extern IRaceEngine& LmRaceEngine()
{
	return LegacyMenu::self().raceEngine();
}
				  
#endif /* _LEGACYMENU_H_ */ 
