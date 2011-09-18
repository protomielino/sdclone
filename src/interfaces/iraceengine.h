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
struct CarPitCmd;


class IRaceEngine
{
public:

	virtual void reset() = 0;
	virtual void cleanup() = 0;
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
	virtual void restartRace() = 0;

	virtual void start() = 0;
	virtual void stop() = 0;
#ifdef DEBUG
	virtual void step(double dt) = 0;
#endif

	virtual GfRace* race() = 0;

	//! Temporary input / modifiable situation, for commanding the race engine from the outside
	//! Aimed at being removed when dedicated setters are ready.
	virtual struct RmInfo* inData() = 0;

	//! Output / read-only situation, generated by the race engine for external use.
	virtual const struct RmInfo* outData() const = 0;

	// WIP : dedicated situation setters, for commanding the race engine from the outside.
	virtual bool setSchedulingSpecs(double fSimuRate, double fOutputRate = 0) = 0;
	virtual void accelerateTime(double fMultFactor) = 0;
	virtual void setPitCommand(int nCarIndex, const struct CarPitCmd* pPitCmd) = 0;
	
};

#include <iuserinterface.h>

#endif // __IRACEENGINE__H__
