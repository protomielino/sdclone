/***************************************************************************

    file        : genparoptv1.h
    copyright   : (C) 2012 by Wolf-Dieter Beelitz
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
  		A race engine module designed for optimising car and AI driver setups
		(parameters) through a genetic algorithm
    @version    $Id$
*/

#ifndef _GENPAROPTV1_H_
#define _GENPAROPTV1_H_

#include <iphysicsengine.h>
#include <iraceengine.h>
#include <itrackloader.h>


#include <tgf.hpp>
#include <tgfclient.h>

// DLL exported symbols declarator for Windows.
#ifdef WIN32
# ifdef GENPAROPTV1_DLL
#  define GENPAROPTV1_API __declspec(dllexport)
# else
#  define GENPAROPTV1_API __declspec(dllimport)
# endif
#else
# define GENPAROPTV1_API
#endif

// The C interface of the module.
extern "C" int GENPAROPTV1_API openGfModule(const char* pszShLibName, void* hShLibHandle);
extern "C" int GENPAROPTV1_API closeGfModule();

// The module main class
// (Singleton, inherits GfModule, and implements IRaceEngine).

class GENPAROPTV1_API GenParOptV1 : public GfModule, public IRaceEngine
{
public:

	// Implementation of IRaceEngine.
	virtual void reset();
	virtual void cleanup();
	virtual void shutdown();

	virtual void setUserInterface(IUserInterface& userItf);

	virtual void initializeState(void *prevMenu);
	virtual void updateState();
	virtual void applyState(int state);

	virtual void selectRaceman(GfRaceManager* pRaceMan, bool bKeepHumans = true);
	virtual void restoreRace(void* hparmResults);
	virtual void configureRace(bool bInteractive = true);

	virtual void startNewRace();
	virtual void resumeRace();

	virtual void startRace();
	virtual void abandonRace();
	virtual void abortRace();
	virtual void skipRaceSession();
	virtual void restartRace();

	virtual bool setSchedulingSpecs(double fSimuRate, double fOutputRate = 0);
	virtual void accelerateTime(double fMultFactor);
	virtual void start();
	virtual void stop();

	virtual bool supportsHumanDrivers();

#ifdef SD_DEBUG
	virtual void step(double dt);
#endif

#ifdef STARTPAUSED
   virtual void stopPreracePause(){};
#endif

#ifdef COOLDOWN
   virtual void stopCooldown(){};
#endif

	virtual GfRace* race();
	virtual const GfRace* race() const;
	
	//! Temporary input / modifiable situation, for commanding the race engine from the outside
	//! Aimed at being removed when dedicated setters are ready.
	virtual struct RmInfo* inData();

	//! Output / read-only situation, generated by the race engine for external use.
	virtual const struct RmInfo* outData() const;

	// WIP : dedicated situation setters, for commanding the race engine from the outside.
	virtual void setPitCommand(int nCarIndex, const struct CarPitCmd* pPitCmd);
	
	// Accessor to the singleton.
	static GenParOptV1& self();

	// Accessor to the track loader.
	ITrackLoader& trackLoader();

	// Accessor to the physics engine.
	IPhysicsEngine& physicsEngine();

	// Physics engine management.
	bool loadPhysicsEngine();
	void unloadPhysicsEngine();

	// Destructor.
	virtual ~GenParOptV1();

	// Accessor to the user interface.
	IUserInterface& userInterface();

protected:

	// Protected constructor to avoid instanciation outside (but friends).
	GenParOptV1(const std::string& strShLibName, void* hShLibHandle);
	
	// Make the C interface functions nearly member functions.
	friend int openGfModule(const char* pszShLibName, void* hShLibHandle);
	friend int closeGfModule();
	
protected:

	// The singleton.
	static GenParOptV1* _pSelf;

	// The user interface.
	IUserInterface* _piUserItf;

	// The track loader.
	ITrackLoader* _piTrkLoader;
	
	// The physics engine.
	IPhysicsEngine* _piPhysEngine;

	// The race.
	GfRace* _pRace;
};

//! Shortcut to the user interface.
inline extern IUserInterface& ReUI()
{
	return GenParOptV1::self().userInterface();
}
				  
//! Shortcut to the physics engine.
inline extern IPhysicsEngine& RePhysicsEngine()
{
	return GenParOptV1::self().physicsEngine();
}
				  
//! Shortcut to the track loader.
inline extern ITrackLoader& ReTrackLoader()
{
	return GenParOptV1::self().trackLoader();
}
				  
// The "Optim" logger instance.
extern GfLogger* RePLogOptim;
#define ReLogOptim (*RePLogOptim)

#endif /* _GENPAROPTV1_H_ */ 
