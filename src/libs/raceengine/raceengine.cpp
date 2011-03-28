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

#include <iuserinterface.h>

#include "racesituation.h"
#include "racemain.h"
#include "raceinit.h"
#include "racestate.h"
#include "raceupdate.h"

#include "raceengine.h"


// The singleton.
RaceEngine* RaceEngine::_pSelf = 0;

RaceEngine& RaceEngine::self()
{
	if (!_pSelf)
		_pSelf = new RaceEngine;
	
	return *_pSelf;
}

RaceEngine::RaceEngine()
: _piUserItf(0)
{
}

// Implementation of IRaceEngine.
void RaceEngine::initialize(void)
{
	::ReInit();
}
void RaceEngine::shutdown(void)
{
	::ReShutdown();
}

void RaceEngine::setUserInterface(IUserInterface& userItf)
{
	_piUserItf = &userItf;
}

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

//************************************************************
void RaceEngine::startNewRace()
{
	::ReStartNewRace();
}

void RaceEngine::resumeRace()
{
	::ReResumeRace();
}

//************************************************************
void RaceEngine::startRace()
{
	::ReRaceRealStart();
}

void RaceEngine::abandonRace()
{
	::ReRaceAbandon();
}

void RaceEngine::abortRace()
{
	::ReRaceAbort();
}

void RaceEngine::skipRaceSession()
{
	::ReRaceSkipSession();
}

void RaceEngine::continueRace()
{
	::ReRaceContinue();
}

void RaceEngine::restartRace()
{
	::ReRaceRestart();
}

//************************************************************
void RaceEngine::accelerateTime(double fMultFactor)
{
	::ReAccelerateTime(fMultFactor);
}

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

//************************************************************
GfRace* RaceEngine::race()
{
	return ::ReGetRace();
}

tRmInfo* RaceEngine::data()
{
	return ::ReSituation();
}

// Accessor to the user interface.
IUserInterface& RaceEngine::userInterface()
{
	return *_piUserItf;
}
