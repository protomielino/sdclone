/***************************************************************************
                 iraceengine.h -- Tools for module interface management

    created              : Mon Mar 7 19:32:14 CEST 2011
    copyright            : (C) 2011 by Jean-Philippe Meuret                         
    web                  : jpmeuret@free.fr
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
    	Interface for a race engine
    @version	$Id$
*/

// TODO: Rename all the member functions (first lower case letter, remove ReRace, ...)

#ifndef __IRACEENGINE__H__
#define __IRACEENGINE__H__

struct RmInfo;
class GfRaceManager;
class GfRace;


class IRaceEngine
{
public:
// From racesituation
	virtual struct RmInfo* data() = 0;

// From racemain
	virtual void setExitMenuInitFunc(void* (*func)(void*)) = 0;

// From raceinit
	virtual void startNewRace() = 0;
	virtual void resumeRace() = 0;

	virtual void initialize() = 0;
	virtual void shutdown() = 0;

	virtual void selectRaceman(GfRaceManager* pRaceMan) = 0;
	virtual void restoreRace(void* hparmResults) = 0;
	virtual void configureRace(bool bInteractive = true) = 0;

	virtual GfRace* race() = 0;

// From racestate
	virtual void initializeState(void *prevMenu) = 0;
	virtual void updateState() = 0;
	virtual void applyState(int state) = 0;

// From raceupdate
	virtual void start() = 0;
	virtual void stop() = 0;
#ifdef DEBUG
	virtual void step(double dt) = 0;
#endif
};

#endif // __IRACEENGINE__H__
