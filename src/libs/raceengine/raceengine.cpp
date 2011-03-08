/***************************************************************************

    file        : raceengine.cpp
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

#include "racesituation.h"
#include "racemain.h"
#include "raceinit.h"
#include "racestate.h"
#include "raceupdate.h"

#include "raceengine.h"


// The singleton.
RaceEngine* RaceEngine::_pSelf = 0;


RaceEngine::RaceEngine()
{
}

// From racesituation.
tRmInfo* RaceEngine::data()
{
	return ::ReSituation();
}

// From racemain
void RaceEngine::setExitMenuInitFunc(void* (*func)(void*))
{
	::ReSetExitMenuInitFunc(func);
}

// From raceinit
void RaceEngine::startNewRace()
{
	::ReStartNewRace();
}
void RaceEngine::resumeRace()
{
	::ReResumeRace();
}

void RaceEngine::initialize(void)
{
	::ReInit();
}
void RaceEngine::shutdown(void)
{
	::ReShutdown();
}

void RaceEngine::selectRaceman(GfRaceManager* pRaceMan)
{
	::ReRaceSelectRaceman(pRaceMan);
}
void RaceEngine::restoreRace(void* hparmResults)
{
	::ReRaceRestore(hparmResults);
}
void RaceEngine::configureRace(bool bInteractive)
{
	::ReRaceConfigure(bInteractive);
}

GfRace* RaceEngine::race()
{
	return ::ReGetRace();
}

// From racestate
void RaceEngine::initializeState(void *prevMenu)
{
	::ReStateInit(prevMenu);
}

void RaceEngine::updateState(void)
{
	::ReStateManage();
}

void RaceEngine::applyState(int state)
{
	::ReStateApply((void*)state);
}

// From raceupdate
void RaceEngine::start(void)
{
	::ReStart();
}

void RaceEngine::stop(void)
{
	::ReStop();
}

#ifdef DEBUG
void RaceEngine::step(double dt)
{
	::ReOneStep(dt);
}
#endif

// Accessor to the singleton.
IRaceEngine& RaceEngine::self()
{
	if (!_pSelf)
		_pSelf = new RaceEngine;
	
	return *_pSelf;
}

 
