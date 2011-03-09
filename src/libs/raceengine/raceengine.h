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

	// Implementation of IRaceEngine.
	virtual void initialize();
	virtual void shutdown();

	virtual void setUserInterface(IUserInterface& userItf);

	virtual void initializeState(void *prevMenu);
	virtual void updateState();
	virtual void applyState(int state);

	virtual void selectRaceman(GfRaceManager* pRaceMan);
	virtual void restoreRace(void* hparmResults);
	virtual void configureRace(bool bInteractive = true);

	virtual void startNewRace();
	virtual void resumeRace();

	virtual void accelerateTime(double fMultFactor);
	virtual void start();
	virtual void stop();
#ifdef DEBUG
	virtual void step(double dt);
#endif

	virtual GfRace* race();
	virtual struct RmInfo* data();

	// Accessor to the singleton.
	static RaceEngine& self();

	// Accessor to the user interface.
	IUserInterface& userInterface();

protected:

	// Protected constructor to avoid instanciation outside of self().
	RaceEngine();
	
protected:

	// The singleton.
	static RaceEngine* _pSelf;

	// The user interface.
	IUserInterface* _piUserItf;
};

#endif /* _RACEENGINE_H_ */ 
