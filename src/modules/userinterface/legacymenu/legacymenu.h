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
#include <igraphicsengine.h>

#include <tgf.hpp>

class IGraphicsEngine;

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
public:

	// Implementation of IUserInterface.
	virtual bool activate();
	
	virtual void quit();
	
	virtual void shutdown();

	virtual void* createRaceScreen();
	virtual void* createRaceEventLoopHook();

	virtual void activateLoadingScreen(const char* title, const char* bgimg);
	virtual void addLoadingMessage(const char* text);
	virtual void shutdownLoadingScreen();

	virtual void activateGameScreen();
	
	virtual int activateRacemanMenu();
	virtual int activateNextEventMenu();

	virtual void activateStartRaceMenu();
	virtual void activateStopRaceMenu();

	// Results table management.
	virtual void* createResultsMenu();
	virtual void activateResultsMenu(void* prevHdle, struct RmInfo* reInfo);
	virtual void setResultsTableTitles(const char* pszTitle, const char* pszSubTitle);
	virtual void setResultsTableHeader(const char* pszHeader);
	virtual void addResultsTableRow(const char* pszText);
	virtual void setResultsTableRow(int nIndex, const char* pszText, bool bHighlight = false);
	virtual void removeResultsTableRow(int nIndex);
	virtual void eraseResultsTable();
	virtual int  getResultsTableRowCount() const;

	virtual void activateStandingsMenu(void* prevHdle, struct RmInfo* info, int start = 0);

	// Graphics engine control.
	virtual bool initializeGraphics();
	virtual bool loadTrackGraphics(struct Track* pTrack);
	virtual bool loadCarsGraphics(struct Situation* pSituation);
	virtual bool setupGraphicsView();
	virtual void shutdownGraphicsView();
	virtual void unloadCarsGraphics();
	virtual void unloadTrackGraphics();
	virtual void shutdownGraphics();
	
	// Setter for the race engine.
	virtual void setRaceEngine(IRaceEngine& raceEngine);

	// Accessor to the singleton.
	static LegacyMenu& self();

	//! Accessor to the race engine.
	IRaceEngine& raceEngine();

	//! Accessor to the race engine.
	IGraphicsEngine* graphicsEngine();

protected:

	// Protected constructor to avoid instanciation outside (but friends).
	LegacyMenu(const std::string& strShLibName, void* hShLibHandle);
	
	// Make the C interface functions nearly member functions.
	friend int openGfModule(const char* pszShLibName, void* hShLibHandle);
	friend int closeGfModule();

 protected:

	// The singleton.
	static LegacyMenu* _pSelf;

	// The race engine.
	IRaceEngine* _piRaceEngine;

	// The graphics engine.
	IGraphicsEngine* _piGraphicsEngine;
};

//! Shortcut to the race engine.
inline extern IRaceEngine& LmRaceEngine()
{
	return LegacyMenu::self().raceEngine();
}
				  
//! Shortcut to the graphics engine.
inline extern IGraphicsEngine* LmGraphicsEngine()
{
	return LegacyMenu::self().graphicsEngine();
}
				  
#endif /* _LEGACYMENU_H_ */ 
