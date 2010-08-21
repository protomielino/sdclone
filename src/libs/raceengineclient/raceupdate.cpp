/***************************************************************************

    file        : raceupdate.cpp
    created     : Sat Nov 23 09:05:23 CET 2002
    copyright   : (C) 2002 by Eric Espie
    email       : eric.espie@torcs.org 
    version     : $Id: raceupdate.cpp,v 1.19 2007/11/06 20:43:32 torcs Exp $

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
    		
    @author	<a href=mailto:eric.espie@torcs.org>Eric Espie</a>
    @version	$Id: raceupdate.cpp,v 1.19 2007/11/06 20:43:32 torcs Exp $
*/

#include <cstdlib>

#include <SDL.h>
#include <SDL_thread.h>

#include <portability.h>
#include <network.h>
#include <tgfclient.h>
#include <robot.h>
#include <raceman.h>
#include <racescreens.h>

#include "racegl.h"
#include "raceresults.h"
#include "racesimusimu.h"

#include "racesituation.h"
#include "racemessage.h"
#include "racenetwork.h"
#include "racecars.h"
#include "raceupdate.h"


// Index of the CPU to use for thread affinity if any and if there are at least 2 ones.
static const int NGraphicsCPUId = 0;
static const int NUpdaterCPUId = 1;


// The situation updater class.
class reSituationUpdater
{
public:
	
	//! Constructor.
	reSituationUpdater(tRmInfo* pReInfo);

	//! Destructor.
	~reSituationUpdater();

	//! Stop (pause) the updater if it is running (return its exit status).
	void stop();
	
	//! (Re)start (after a stop) the updater if it is not running.
	void start();
	
	//! Terminate the updater (return its exit status ; wait for the thread to return).
	int terminate();
	
	//! Get the situation for the previous step
	tRmInfo* getPreviousStep();
	
	//! Start computing the situation for the current step
	void computeCurrentStep();

private:

	//! Reserve exclusive access on the race engine data.
	bool lockData(const char* pszLocker);
	
	//! Release exclusive access on the race engine data.
	bool unlockData(const char* pszLocker);
	
	//! Copy (after allocating if pTarget is null) the source situation into the target (deep copy for RW data, shallow copy for RO data).
	tRmInfo* deliverSituation(tRmInfo*& pTarget, const tRmInfo* pSource);
	
	//! Allocate and initialize a situation (set constants from source).
	tRmInfo* initSituation(const tRmInfo* pSource);
	
	//! Free the given situation
	void freezSituation(tRmInfo*& pSituation);
	
	//! The thread function.
	int threadLoop();
	
	//! The C wrapper on the thread function.
	friend int reSituationUpdaterThreadLoop(void *);
	
private:

	//! Initial number of drivers racing
	int _nInitDrivers;

	//! The previous step of the situation
	tRmInfo* _pPrevReInfo;

	//! The current step of the situation
	tRmInfo* _pCurrReInfo;

	//! The mutex to protect the situation data
	SDL_mutex* _pDataMutex;

	//! The situation updater thread
	SDL_Thread* _pUpdateThread;

	//! True if the updater is actually threaded (may be not the case)
	bool _bThreaded;

	//! True if thread affinity has to be applied (even in case !_bThreaded)
	bool _bThreadAffinity;

	//! Flag to set in order to terminate the updater.
	bool _bTerminate;
};

static reSituationUpdater* situationUpdater = 0;


void
reOneStep(double deltaTimeIncrement)
{
	int i;
	tRobotItf *robot;
	tSituation *s = ReInfo->s;

	// Race messages life cycle management.
	ReRaceMsgManage(ReInfo);
	
	if (GetNetwork())
	{
		// Resync clock in case computer falls behind
		if (s->currentTime < 0.0)
		{
			s->currentTime = GfTimeClock() - GetNetwork()->GetRaceStartTime();
		}

		if (s->currentTime < -2.0)
		{
			char msg[32];
			int t = -s->currentTime;
			sprintf(msg,"Race will start in %i seconds", t);
			ReRaceMsgSetBig(ReInfo, msg, 1.0);
		}
	}

	if (floor(s->currentTime) == -2.0) {
		ReRaceMsgSetBig(ReInfo, "Ready", 1.0);
	} else if (floor(s->currentTime) == -1.0) {
		ReRaceMsgSetBig(ReInfo, "Set", 1.0);
	} else if (floor(s->currentTime) == 0.0) {
		ReRaceMsgSetBig(ReInfo, "Go", 1.0);
		if (s->currentTime==0.0)
			GfOut("Race start time is %lf\n", GfTimeClock());
	}

	ReInfo->_reCurTime += deltaTimeIncrement * ReInfo->_reTimeMult; /* "Real" time */
	s->currentTime += deltaTimeIncrement; /* Simulated time */


	if (s->currentTime < 0) {
		/* no simu yet */
		ReInfo->s->_raceState = RM_RACE_PRESTART;
	} else if (ReInfo->s->_raceState == RM_RACE_PRESTART) {
		ReInfo->s->_raceState = RM_RACE_RUNNING;
		s->currentTime = 0.0; /* resynchronize */
		ReInfo->_reLastTime = 0.0;
	}

	GfProfStartProfile("rbDrive*");
	GfSchedBeginEvent("raceupdate", "robots");
	if ((s->currentTime - ReInfo->_reLastTime) >= RCM_MAX_DT_ROBOTS) {
		s->deltaTime = s->currentTime - ReInfo->_reLastTime;
		for (i = 0; i < s->_ncars; i++) {
			if ((s->cars[i]->_state & RM_CAR_STATE_NO_SIMU) == 0) {
				robot = s->cars[i]->robot;
				robot->rbDrive(robot->index, s->cars[i], s);
			}
			else if (! (s->cars[i]->_state & RM_CAR_STATE_ENDRACE_CALLED ) && ( s->cars[i]->_state & RM_CAR_STATE_OUT ) == RM_CAR_STATE_OUT )
			{ // No simu, look if it is out
				robot = s->cars[i]->robot;
				if (robot->rbEndRace)
					robot->rbEndRace(robot->index, s->cars[i], s);
				s->cars[i]->_state |= RM_CAR_STATE_ENDRACE_CALLED;
			}
		}
		ReInfo->_reLastTime = s->currentTime;
	}
	GfSchedEndEvent("raceupdate", "robots");
	GfProfStopProfile("rbDrive*");


	if (GetNetwork())
	{
		ReNetworkOneStep();
	}

	GfProfStartProfile("_reSimItf.update*");
	GfSchedBeginEvent("raceupdate", "physics");
	ReInfo->_reSimItf.update(s, deltaTimeIncrement, -1);
	bool bestLapChanged = false;
	for (i = 0; i < s->_ncars; i++) {
		ReCarsManageCar(s->cars[i], bestLapChanged);
	}
	GfSchedEndEvent("raceupdate", "physics");
	GfProfStopProfile("_reSimItf.update*");
	
	ReCarsSortCars();

	/* Update screens if a best lap changed */
	if (ReInfo->_displayMode == RM_DISP_MODE_NONE && s->_ncars > 1 && bestLapChanged)
	{
		if (ReInfo->s->_raceType == RM_TYPE_PRACTICE)
			ReUpdatePracticeCurRes(ReInfo->s->cars[0]);
		else if (ReInfo->s->_raceType == RM_TYPE_QUALIF)
			ReUpdateQualifCurRes(ReInfo->s->cars[0]);
	}
}

void ReInitUpdater()
{
	ReInfo->_reRunning = 0;
 	if (!situationUpdater)
 		situationUpdater = new reSituationUpdater(ReInfo);

	// Configure schedule spy          (nMaxEv, ignoreDelay)
	GfSchedConfigureEventLog("raceupdate", "graphics", 10000, 0.0);
	GfSchedConfigureEventLog("raceupdate", "situCopy", 10000, 0.0);
	GfSchedConfigureEventLog("raceupdate", "robots",   10000, 0.0);
	GfSchedConfigureEventLog("raceupdate", "physics",  10000, 0.0);
}

void ReInitCarGraphics(void)
{
	tRmInfo* pPrevReInfo = situationUpdater->getPreviousStep();
	pPrevReInfo->_reGraphicItf.initcars(pPrevReInfo->s);
}

void
ReStart(void)
{
	GfSchedBeginSession("raceupdate");
	situationUpdater->start();
}

void
ReStop(void)
{
	situationUpdater->stop();
	GfSchedEndSession("raceupdate");
}

void ReShutdownUpdater()
{
	// Destroy the situation updater.
 	if (situationUpdater)
	{
		delete situationUpdater;
		situationUpdater = 0;
	}

	GfSchedEndSession("raceupdate");
	//                                          (timeResol, durUnit, durResol)
	GfSchedPrintReport("raceupdate", "schedule.csv", 1.0e-3, 1.0e-3, 1.0e-6);
}

static void
reCapture(void)
{
	char filename[512];
    int vw, vh;
    tRmMovieCapture	*capture = &(ReInfo->movieCapture);
    
    unsigned char* img = GfScrCapture(&vw, &vh);
    if (!img)
		return;
    
    sprintf(filename, "%s/sd-%4.4d-%8.8d.png", capture->outputBase,
			capture->currentCapture, capture->currentFrame++);
    GfTexWriteImageToPNG(img, filename, vw, vh);
	
    free(img);
}

// Multi-threaded ReUpdate ===============================================================

int reSituationUpdaterThreadLoop(void *pUpdater)
{
	return static_cast<reSituationUpdater*>(pUpdater)->threadLoop();
}

int reSituationUpdater::threadLoop()
{
	// Wait delay for each loop, from bRunning value (index 0 = false, 1 = true).
	static const double KWaitDelayMS[2] = { 1.0, RCM_MAX_DT_SIMU * 1000 / 10 };

	// Termination flag.
	bool bEnd = false;

	// Local state (false = paused, true = simulating).
	bool bRunning = false;

	// Current real time.
	double realTime;
	
	// Apply thread affinity to the current = situation updater thread if specified.
	// Note: No need to reset the affinity, as the thread is just born.
	if (_bThreadAffinity)
		GfSetThreadAffinity(NUpdaterCPUId);

	GfOut("SituationUpdater thread is started.\n");
	
	do
	{
		// Let's make current step the next one (update).
		// 1) Lock the race engine data.
		lockData("reSituationUpdater::threadLoop");

		// 2) Check if time to terminate has come.
		if (_bTerminate)
			
			bEnd = true;
		
		// 3) If not time to terminate, and running, do the update job.
		else if (_pCurrReInfo->_reRunning)
		{
			if (!bRunning)
			{
				bRunning = true;
				GfOut("SituationUpdater thread is running.\n");
			}
			
			realTime = GfTimeClock();
		
			GfProfStartProfile("reOneStep*");
		
			while (_pCurrReInfo->_reRunning
				   && ((realTime - _pCurrReInfo->_reCurTime) > RCM_MAX_DT_SIMU))
			{
				// One simu + may be robots step
				reOneStep(RCM_MAX_DT_SIMU);
			}
		
			GfProfStopProfile("reOneStep*");
		
			// Send car physics to network if needed
			if (GetNetwork())
				GetNetwork()->SendCarControlsPacket(_pCurrReInfo->s);
		}
		
		// 3) If not time to terminate, and not running, do nothing.
		else
		{
			if (bRunning)
			{
				bRunning = false;
				GfOut("SituationUpdater thread is paused.\n");
			}
		}
			
		// 4) Unlock the race engine data.
		unlockData("reSituationUpdater::threadLoop");
		
		// 5) Let the CPU take breath if possible (but after unlocking data !).
		SDL_Delay(KWaitDelayMS[(int)bRunning]);
	}
	while (!bEnd);

	GfOut("SituationUpdater thread has been terminated.\n");
	
	return 0;
}

reSituationUpdater::reSituationUpdater(tRmInfo* pReInfo)
{
	// Save the race engine info (state + situation) pointer for the current step.
	_pCurrReInfo = pReInfo;
	_nInitDrivers = _pCurrReInfo->s->_ncars;

	// Determine if we have a dedicated separate thread or not
	// (according to the user settings, and the actual number of CPUs).
	char path[512];
	snprintf(path, 512, "%s%s", GetLocalDir(), RACE_ENG_CFG);
	void *paramHandle = GfParmReadFile(path, GFPARM_RMODE_REREAD | GFPARM_RMODE_CREAT);
	const char* pszMultiThreadScheme =
		GfParmGetStr(paramHandle, RM_SECT_RACE_ENGINE, RM_ATTR_MULTI_THREADING, RM_VAL_AUTO);

	if (!strcmp(pszMultiThreadScheme, RM_VAL_OFF))
		_bThreaded = false;
	else if (!strcmp(pszMultiThreadScheme, RM_VAL_ON))
		_bThreaded = true;
	else // Can't be anything else than RM_VAL_AUTO
		_bThreaded = GfGetNumberOfCPUs() > 1;

	// Determine if we apply some thread affinity or not (according to the user settings).
	const char* pszThreadAffinityScheme =
		GfParmGetStr(paramHandle, RM_SECT_RACE_ENGINE, RM_ATTR_THREAD_AFFINITY, RM_VAL_OFF);
	_bThreadAffinity = strcmp(pszThreadAffinityScheme, RM_VAL_ON) == 0;

	GfParmReleaseHandle(paramHandle);

	// Apply thread affinity to the current = main = graphics thread
	// (and don't forget to reset it when specified :
	//  user settings may have changed since last race).
	GfSetThreadAffinity(_bThreadAffinity ? NGraphicsCPUId : GfAffinityAnyCPU);

	// Initialize termination flag.
	_bTerminate = false;

	if (_bThreaded)
	{
		// Initialize the race engine info (state + situation) pointer for the previous step.
		_pPrevReInfo = initSituation(_pCurrReInfo);

		// Create the data mutex.
		_pDataMutex = SDL_CreateMutex();
		
		// Create and start the updater thread.
		_pUpdateThread = SDL_CreateThread(reSituationUpdaterThreadLoop, this);
	}
	else
	{
		_pPrevReInfo = 0;
		_pDataMutex = 0;
		_pUpdateThread = 0;
	}

	GfOut("SituationUpdater initialized (%sseparate thread, CPU affinity %s).\n",
	      (_bThreaded ? "" : "no "), (_bThreadAffinity ? "On" : "Off"));
}

reSituationUpdater::~reSituationUpdater()
{
	terminate(); // In case not already done.

	if (_bThreaded)
	{
		if (_pDataMutex)
			SDL_DestroyMutex(_pDataMutex);
		
		if (_pPrevReInfo)
			freezSituation(_pPrevReInfo);
	}
}

bool reSituationUpdater::lockData(const char* pszLocker)
{
	if (!_bThreaded)
		return true;
	
	const bool bStatus = SDL_mutexP(_pDataMutex) == 0;
	if (!bStatus)
		GfOut("%s : Failed to lock data mutex\n", pszLocker);

	return bStatus;
}
	
bool reSituationUpdater::unlockData(const char* pszLocker)
{
	if (!_bThreaded)
		return true;
	
	const bool bStatus = SDL_mutexV(_pDataMutex) == 0;
	if (!bStatus)
		GfOut("%s : Failed to unlock data mutex\n", pszLocker);

	return bStatus;
}


void reSituationUpdater::start()
{
	GfOut("Unpausing race engine.\n");

	// Lock the race engine data.
	lockData("reSituationUpdater::start");

	// Set the running flags.
    _pCurrReInfo->_reRunning = 1;
	_pCurrReInfo->s->_raceState &= ~RM_RACE_PAUSED;

	// Resynchronize simulation time.
    _pCurrReInfo->_reCurTime = GfTimeClock() - RCM_MAX_DT_SIMU;
	
	// Unlock the race engine data.
	unlockData("reSituationUpdater::start");
}

void reSituationUpdater::stop()
{
	GfOut("Pausing race engine.\n");

	// Lock the race engine data.
	lockData("reSituationUpdater::stop");

	// Reset the running flags.
	_pCurrReInfo->_reRunning = 0;
	_pCurrReInfo->s->_raceState |= RM_RACE_PAUSED;
		
	// Unlock the race engine data.
	unlockData("reSituationUpdater::stop");
}

int reSituationUpdater::terminate()
{
	int status = 0;
	
	GfOut("Terminating race engine.\n");

	// Lock the race engine data.
	lockData("reSituationUpdater::terminate");

	// Set the death flag.
    _bTerminate = true;
	
	// Unlock the race engine data.
	unlockData("reSituationUpdater::terminate");
	
	// Wait for the thread to gracefully terminate if any.
	if (_bThreaded)
	{
		SDL_WaitThread(_pUpdateThread, &status);
		_pUpdateThread = 0;
 	}

	return status;
}

void reSituationUpdater::freezSituation(tRmInfo*& pSituation)
{
	ReSituationFreez(pSituation);
}

tRmInfo* reSituationUpdater::initSituation(const tRmInfo* pSource)
{
	return ReSituationAllocInit(pSource);
}

tRmInfo* reSituationUpdater::deliverSituation(tRmInfo*& pTarget, const tRmInfo* pSource)
{
	return ReSituationCopy(pTarget, pSource);
}


tRmInfo* reSituationUpdater::getPreviousStep()
{
	if (!_bThreaded)
		
		// No multi-threading : no need to really copy.
		_pPrevReInfo = _pCurrReInfo;

	else
	{
		// Lock the race engine data.
		if (!lockData("reSituationUpdater::getPreviousStep"))
			return 0;

		// Get the situation data.
		deliverSituation(_pPrevReInfo, _pCurrReInfo);
	
		// Unlock the race engine data.
		if (!unlockData("reSituationUpdater::getPreviousStep"))
			return 0;
	}

	return _pPrevReInfo;
}

void reSituationUpdater::computeCurrentStep()
{
	// Nothing to do if actually threaded :
	// the updater thread is already doing the job on his side.
	if (!_bThreaded)
	{
		const double t = GfTimeClock();
		
		GfProfStartProfile("reOneStep*");
		
		while (_pCurrReInfo->_reRunning && ((t - _pCurrReInfo->_reCurTime) > RCM_MAX_DT_SIMU))
			
			reOneStep(RCM_MAX_DT_SIMU);
		
		GfProfStopProfile("reOneStep*");
		
		// Send car physics to network if needed
		if (GetNetwork())
			GetNetwork()->SendCarControlsPacket(_pCurrReInfo->s);
	}
}

int
ReUpdate(void)
{
	tRmInfo* pPrevReInfo;
	
    GfProfStartProfile("ReUpdate");
    ReInfo->_refreshDisplay = 0;
    switch (ReInfo->_displayMode)
	{
		case RM_DISP_MODE_NORMAL:

			// Get the situation for the previous step.
			GfSchedBeginEvent("raceupdate", "situCopy");
			pPrevReInfo = situationUpdater->getPreviousStep();
			GfSchedEndEvent("raceupdate", "situCopy");

			// Start computing the situation for the current step.
			situationUpdater->computeCurrentStep();
			
			// Next screen will be the pit menu if one human driver is in pit. 
			if (pPrevReInfo->_reInPitMenuCar)

				RmPitMenuStart(pPrevReInfo->s, pPrevReInfo->_reInPitMenuCar,
							   (void*)pPrevReInfo->_reInPitMenuCar, ReCarsUpdateCarPitCmd);

			// Update racing messages for the user
			ReRaceMsgUpdate(pPrevReInfo);
	
			GfuiDisplay();
			
			GfSchedBeginEvent("raceupdate", "graphics");
			pPrevReInfo->_reGraphicItf.refresh(pPrevReInfo->s);
			GfSchedEndEvent("raceupdate", "graphics");
			
			GfelPostRedisplay();	/* Callback -> reDisplay */
			break;
	
		case RM_DISP_MODE_NONE:
			
			reOneStep(RCM_MAX_DT_SIMU);
			
			ReRaceMsgUpdate(ReInfo);
			
			if (ReInfo->_refreshDisplay)
				GfuiDisplay();
			GfelPostRedisplay();	/* Callback -> reDisplay */
			break;

		case RM_DISP_MODE_SIMU_SIMU:
			
			ReSimuSimu();
			ReCarsSortCars();
			if (ReInfo->_refreshDisplay)
				GfuiDisplay();
			GfelPostRedisplay();
			break;

		case RM_DISP_MODE_CAPTURE:
		{
			tRmMovieCapture	*capture = &(ReInfo->movieCapture);
			while ((ReInfo->_reCurTime - capture->lastFrame) < capture->deltaFrame)
			{
				reOneStep(capture->deltaSimu);
				
				ReRaceMsgUpdate(ReInfo);
			}

			capture->lastFrame = ReInfo->_reCurTime;
	
			GfuiDisplay();
			ReInfo->_reGraphicItf.refresh(ReInfo->s);
			reCapture();
			GfelPostRedisplay();	/* Callback -> reDisplay */
			break;
		}
	
    }

	// Check for end of online race.
	if (GetNetwork() && GetNetwork()->FinishRace(ReInfo->s->currentTime))
		ReInfo->s->_raceState = RM_RACE_ENDED;
	
    GfProfStopProfile("ReUpdate");

    return RM_ASYNC;
}

