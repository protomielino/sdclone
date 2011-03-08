/***************************************************************************

    file        : raceengine.h
    copyright   : (C) 2010 by Jean-Philippe Meuret                        
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
    		The race engine implementation of its IRaceEngine interface
    @version    $Id$
*/

#ifndef _RACEENGINE_H_
#define _RACEENGINE_H_

#include <iraceengine.h>

// DLL exported symbols declarator for Windows.
#ifdef WIN32
# ifdef RACEENGINE_DLL
#  define RACEENGINE_API __declspec(dllexport)
# else
#  define RACEENGINE_API __declspec(dllimport)
# endif
#else
# define RACEENGINE_API
#endif

class RACEENGINE_API RaceEngine : public IRaceEngine
{
public:
// #include <racesituation.h>
	virtual tRmInfo* data();

// #include <racemain.h>
	virtual void setExitMenuInitFunc(void* (*func)(void*));

// #include <raceinit.h>
	virtual void startNewRace();
	virtual void resumeRace();

	virtual void initialize();
	virtual void shutdown();

	virtual void selectRaceman(GfRaceManager* pRaceMan);
	virtual void restoreRace(void* hparmResults);
	virtual void configureRace(bool bInteractive = true);

	virtual GfRace* race();

// #include <racestate.h>
	virtual void initializeState(void *prevMenu);
	virtual void updateState();
	virtual void applyState(int state);

// #include <raceupdate.h>
	virtual void start();
	virtual void stop();
#ifdef DEBUG
	virtual void step(double dt);
#endif

	// Accessor to the singleton.
	static IRaceEngine& self();

protected:

	// Protected constructor to avoid instanciation outside of self().
	RaceEngine();
	
	// The singleton.
	static RaceEngine* _pSelf;
};

#endif /* _RACEENGINE_H_ */ 
