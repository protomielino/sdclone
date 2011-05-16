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

#include <sstream>

#include <tgf.hpp>

#include <itrackloader.h>
#include <iphysicsengine.h>
#include <car.h> // tCarPitCmd.

#include <tracks.h>

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
: _piUserItf(0), _piPhysEngine(0)
{
}

// Implementation of IRaceEngine.
void RaceEngine::initialize(void)
{
	GfLogInfo("Initializing race engine.\n");

	// Cleanup everything in case no yet done.
	shutdown();
	
	// Internal init.
	::ReInit();

	// Load and initialize the track loader module.
	GfLogInfo("Loading Track Loader ...\n");
	std::ostringstream ossModLibName;
	const char* pszModName =
		GfParmGetStr(ReSituation::self().data()->_reParam, "Modules", "track", "");
	ossModLibName << GfLibDir() << "modules/track/" << pszModName << '.' << DLLEXT;
	GfModule* pmodTrkLoader = GfModule::load(ossModLibName.str());

	// Check that it implements ITrackLoader.
	ITrackLoader* piTrkLoader = 0;
	if (pmodTrkLoader)
		piTrkLoader = pmodTrkLoader->getInterface<ITrackLoader>();
	if (pmodTrkLoader && !piTrkLoader)
	{
		GfModule::unload(pmodTrkLoader);
		return;
	}

	// Initialize GfTracks' track module interface (needed for some track infos).
	GfTracks::self()->setTrackLoader(piTrkLoader);
}

void RaceEngine::shutdown(void)
{
	GfLogInfo("Terminating race engine.\n");

	// Internal cleanup.
	::ReShutdown();

	// Unload the track if not already done.
	ITrackLoader* piTrkLoader = GfTracks::self()->getTrackLoader();
	if (piTrkLoader)
		piTrkLoader->unload();

    // Unload the Track loader module if not already done.
	if (piTrkLoader)
	{
		GfModule* pmodTrkLoader = dynamic_cast<GfModule*>(piTrkLoader);
		if (pmodTrkLoader)
		{
			GfModule::unload(pmodTrkLoader);
			GfTracks::self()->setTrackLoader(0);
		}
	}

    // Unload the Physics engine module if not already done.
	if (_piPhysEngine)
	{
		GfModule* pmodPhysEngine = dynamic_cast<GfModule*>(_piPhysEngine);
		if (pmodPhysEngine)
		{
			GfModule::unload(pmodPhysEngine);
			_piPhysEngine = 0;
		}
	}
	
    // Shutdown the Graphics modules if not already done.
	if (_piUserItf)
		_piUserItf->shutdownGraphics(); // => onRaceEngineShutdown ?
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

void RaceEngine::restartRace()
{
	::ReRaceRestart();
}

//************************************************************
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

// TODO: Remove when safe dedicated setters ready.
tRmInfo* RaceEngine::inData()
{
	return ReSituation::self().data(); // => ReInfo
}

const tRmInfo* RaceEngine::outData() const
{
	return ::ReOutputSituation();
}

// Accessor to the user interface.
IUserInterface& RaceEngine::userInterface()
{
	return *_piUserItf;
}

// Physics engine management.
bool RaceEngine::loadPhysicsEngine()
{
    // Load the Physics engine module if not already done.
	if (_piPhysEngine)
		return true;

	const char* pszModName =
		GfParmGetStr(ReSituation::self().data()->_reParam, "Modules", "simu", "");
	std::ostringstream ossLoadMsg;
	ossLoadMsg << "Loading physics engine (" << pszModName<< ") ...";
	if (_piUserItf)
		_piUserItf->addLoadingMessage(ossLoadMsg.str().c_str());

	std::ostringstream ossModLibName;
	ossModLibName << GfLibDir() << "modules/simu/" << pszModName << '.' << DLLEXT;
	GfModule* pmodPhysEngine = GfModule::load(ossModLibName.str());
	if (pmodPhysEngine)
		_piPhysEngine = pmodPhysEngine->getInterface<IPhysicsEngine>();
	if (pmodPhysEngine && !_piPhysEngine)
		GfModule::unload(pmodPhysEngine);

	return _piPhysEngine ? true : false;
}

void RaceEngine::unloadPhysicsEngine()
{
    // Unload the Physics engine module if not already done.
	if (!_piPhysEngine)
		return;
	
	GfModule* pmodPhysEngine = dynamic_cast<GfModule*>(_piPhysEngine);
	if (pmodPhysEngine)
		GfModule::unload(pmodPhysEngine);
	_piPhysEngine = 0;
}

// Accessor to the physics engine.
IPhysicsEngine& RaceEngine::physicsEngine()
{
	return *_piPhysEngine;
}


//************************************************************
// WIP : dedicated situation setters.

bool RaceEngine::setSchedulingSpecs(double fSimuRate, double fOutputRate)
{
	return ::ReSetSchedulingSpecs(fSimuRate, fOutputRate);
}

void RaceEngine::accelerateTime(double fMultFactor)
{
	ReSituation::self().accelerateTime(fMultFactor);
}

void RaceEngine::setPitCommand(int nCarIndex, const struct CarPitCmd* pPitCmd)
{
	ReSituation::self().setPitCommand(nCarIndex, pPitCmd);
}
