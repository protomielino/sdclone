/***************************************************************************
                 iraceengine.h -- Interface for any race engine

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
    	Interface for any race engine
    @version	$Id$
*/

#ifndef __IRACEENGINE__H__
#define __IRACEENGINE__H__

class IUserInterface;
struct RmInfo;
class GfRaceManager;
class GfRace;


class IRaceEngine
{
public:

	virtual void initialize() = 0;
	virtual void shutdown() = 0;

	virtual void setUserInterface(IUserInterface& userItf) = 0;

	virtual void initializeState(void *prevMenu) = 0;
	virtual void updateState() = 0;
	virtual void applyState(int state) = 0;

	virtual void selectRaceman(GfRaceManager* pRaceMan) = 0;
	virtual void restoreRace(void* hparmResults) = 0;
	virtual void configureRace(bool bInteractive = true) = 0;

	virtual void startNewRace() = 0;
	virtual void resumeRace() = 0;

	virtual void startRace() = 0;
	virtual void abandonRace() = 0;
	virtual void abortRace() = 0;
	virtual void skipRaceSession() = 0;
	virtual void continueRace() = 0;
	virtual void restartRace() = 0;

	virtual void accelerateTime(double fMultFactor) = 0;
	virtual void start() = 0;
	virtual void stop() = 0;
#ifdef DEBUG
	virtual void step(double dt) = 0;
#endif

	virtual GfRace* race() = 0;
	virtual struct RmInfo* data() = 0;

};

#include <iuserinterface.h>

#endif // __IRACEENGINE__H__
