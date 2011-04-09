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

	virtual void update();

	virtual void *createRaceScreen();
	virtual void captureRaceScreen(const char* pszTargetFilename);
	virtual void *createRaceEventLoopHook();

	virtual void setRaceMessage(const char *msg);
	virtual void setRaceBigMessage(const char *msg);

	virtual void activateLoadingScreen(const char *title, const char *bgimg);
	virtual void addLoadingMessage(const char *text);
	virtual void shutdownLoadingScreen();

	virtual void activateGameScreen();
	
	virtual int activateRacemanMenu();
	virtual int activateNextEventMenu();

	virtual void activateStartRaceMenu();
	virtual void activateStopRaceMenu();

	virtual void activatePitMenu(struct CarElt *car, void (*callback)(void*));

	virtual void *createResultsMenu();
	virtual void activateResultsMenu(void *prevHdle, struct RmInfo *reInfo);
	virtual void setResultsMenuTrackName(const char *trackName);
	virtual void setResultsMenuTitle(const char *title);
	virtual void addResultsMenuLine(const char *text);
	virtual void setResultsMenuLine(const char *text, int line, int clr);
	virtual void removeResultsMenuLine(int line);
	virtual void showResultsMenuContinueButton();
	virtual int  getResultsMenuLineCount();
	virtual void eraseResultsMenu();

	virtual void activateStandingsMenu(void *prevHdle, struct RmInfo *info, int start = 0);

	// Graphics engine control.
	virtual bool initializeGraphics();
	virtual bool loadTrackGraphics(struct Track* pTrack);
	virtual bool loadCarsGraphics(struct Situation *pSituation);
	virtual bool setupGraphicsView();
	virtual void updateGraphicsView(struct Situation *pSituation);
	virtual void unloadCarsGraphics();
	virtual void unloadTrackGraphics();
	virtual void shutdownGraphics();
	virtual void setRaceEngine(IRaceEngine& raceEngine);

	// Accessor to the singleton.
	static LegacyMenu& self();

	// Accessor to the race engine.
	IRaceEngine& raceEngine();

protected:

	// Protected constructor to avoid instanciation outside (but friends).
	LegacyMenu(const std::string& strShLibName, void* hShLibHandle);
	
protected:

	// The singleton.
	static LegacyMenu* _pSelf;

	// The race engine.
	IRaceEngine* _piRaceEngine;

	// The graphics engine.
	IGraphicsEngine* _piGraphicsEngine;

	// Make the C interface functions nearly member functions.
	friend int openGfModule(const char* pszShLibName, void* hShLibHandle);
	friend int closeGfModule();
};

#endif /* _LEGACYMENU_H_ */ 
