/***************************************************************************

    file        : raceupdate.cpp
    created     : Sat Nov 23 09:05:23 CET 2002
    copyright   : (C) 2002 by Eric Espie
    email       : eric.espie@torcs.org 
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
    		Race engine core : physics engine, drivers and graphics direction
    @author	<a href=mailto:eric.espie@torcs.org>Eric Espie</a>
    @version	$Id$
*/

#include <cstdlib>
#include <sstream>

#include <SDL/SDL.h>
#include <SDL/SDL_thread.h>

#include <portability.h>
#include <network.h>
#include <robot.h>
#include <raceman.h>

#include "raceengine.h"

#include "raceresults.h"
#include "racesimusimu.h"
#include "racemessage.h"
#include "racesituation.h"
#include "racenetwork.h"
#include "racecars.h"

#include "raceupdate.h"


// Comment-out to activate FPS limiter.
#define NoFPSLimiter 1

// Index of the CPU to use for thread affinity if any and if there are at least 2 ones.
static const int NGraphicsCPUId = 0;
static const int NUpdaterCPUId = 1;


// The situation updater class ==========================================================
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
	
	//! Compute 1 situation update step.
	void runOneStep(double deltaTimeIncrement);

	//! Start computing the situation for the current step
	void computeCurrentStep();

	//! Update pit command for the given car (back from a pit menu).
	void updateCarPitCmd(int nCarIndex, const tCarPitCmd *pPitCmd);

	//! Acknowledge the situation events (simu / graphics synchronization).
	void acknowledgeEvents();

private:

	//! Reserve exclusive access on the race engine data.
	bool lockData(const char* pszLocker);
	
	//! Release exclusive access on the race engine data.
	bool unlockData(const char* pszLocker);
	
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

// The situation updater instance (intialized in ReInitUpdaters).
static reSituationUpdater* situationUpdater = 0;


void reSituationUpdater::runOneStep(double deltaTimeIncrement)
{
	int i;
	tRobotItf *robot;
	tSituation *s = _pCurrReInfo->s;

	// Race messages life cycle management.
	ReRaceMsgManage(_pCurrReInfo);
	
	if (GetNetwork())
	{
		// Resync clock in case computer falls behind
		if (s->currentTime < 0.0)
		{
			s->currentTime = GfTimeClock() - GetNetwork()->GetRaceStartTime();
		}

		if (s->currentTime < -2.0)
		{
			std::ostringstream ossMsg;
			ossMsg << "Race will start in " << -(int)s->currentTime << " seconds";
			ReRaceMsgSetBig(_pCurrReInfo, ossMsg.str().c_str(), 1.0);
		}
	}

	if (s->currentTime >= -2.0 && s->currentTime < deltaTimeIncrement - 2.0) {
		ReRaceMsgSetBig(_pCurrReInfo, "Ready", 1.0);
		GfLogInfo("Ready.\n");
	} else if (s->currentTime >= -1.0 && s->currentTime < deltaTimeIncrement - 1.0) {
		ReRaceMsgSetBig(_pCurrReInfo, "Set", 1.0);
		GfLogInfo("Set.\n");
	} else if (s->currentTime >= 0.0 && s->currentTime < deltaTimeIncrement) {
		ReRaceMsgSetBig(_pCurrReInfo, "Go", 1.0);
		GfLogInfo("Go.\n");
	}

	_pCurrReInfo->_reCurTime += deltaTimeIncrement * _pCurrReInfo->_reTimeMult; /* "Real" time */
	s->currentTime += deltaTimeIncrement; /* Simulated time */

	if (s->currentTime < 0) {
		/* no simu yet */
		_pCurrReInfo->s->_raceState = RM_RACE_PRESTART;
	} else if (_pCurrReInfo->s->_raceState == RM_RACE_PRESTART) {
		_pCurrReInfo->s->_raceState = RM_RACE_RUNNING;
		s->currentTime = 0.0; /* resynchronize */
		_pCurrReInfo->_reLastTime = 0.0;
	}

	GfProfStartProfile("rbDrive*");
	GfSchedBeginEvent("raceupdate", "robots");
	if ((s->currentTime - _pCurrReInfo->_reLastTime) >= RCM_MAX_DT_ROBOTS) {
		s->deltaTime = s->currentTime - _pCurrReInfo->_reLastTime;
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
		_pCurrReInfo->_reLastTime = s->currentTime;
	}
	GfSchedEndEvent("raceupdate", "robots");
	GfProfStopProfile("rbDrive*");


	if (GetNetwork())
	{
		ReNetworkOneStep();
	}

	GfProfStartProfile("_reSimItf.update*");
	GfSchedBeginEvent("raceupdate", "physics");
	_pCurrReInfo->_reSimItf.update(s, deltaTimeIncrement, -1);
	bool bestLapChanged = false;
	for (i = 0; i < s->_ncars; i++) {
		ReCarsManageCar(s->cars[i], bestLapChanged);
	}
	GfSchedEndEvent("raceupdate", "physics");
	GfProfStopProfile("_reSimItf.update*");
	
	ReCarsSortCars();

	/* Update screens if a best lap changed */
	if (_pCurrReInfo->_displayMode == RM_DISP_MODE_NONE && s->_ncars > 1 && bestLapChanged)
	{
		if (_pCurrReInfo->s->_raceType == RM_TYPE_PRACTICE)
			ReUpdatePracticeCurRes(_pCurrReInfo->s->cars[0]);
		else if (_pCurrReInfo->s->_raceType == RM_TYPE_QUALIF)
			ReUpdateQualifCurRes(_pCurrReInfo->s->cars[0]);
	}
}

int reSituationUpdaterThreadLoop(void *pUpdater)
{
	return static_cast<reSituationUpdater*>(pUpdater)->threadLoop();
}

int reSituationUpdater::threadLoop()
{
	// Wait delay for each loop, from bRunning value (index 0 = false, 1 = true).
	static const unsigned KWaitDelayMS[2] = { 1, (unsigned)(RCM_MAX_DT_SIMU * 1000 / 10) };

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

	GfLogInfo("SituationUpdater thread is started.\n");
	
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
				GfLogInfo("SituationUpdater thread is running.\n");
			}
			
			realTime = GfTimeClock();
		
			GfProfStartProfile("reOneStep*");
		
			while (_pCurrReInfo->_reRunning
				   && ((realTime - _pCurrReInfo->_reCurTime) > RCM_MAX_DT_SIMU))
			{
				// One simu + may be robots step
				runOneStep(RCM_MAX_DT_SIMU);
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
				GfLogInfo("SituationUpdater thread is paused.\n");
			}
		}
			
		// 4) Unlock the race engine data.
		unlockData("reSituationUpdater::threadLoop");
		
		// 5) Let the CPU take breath if possible (but after unlocking data !).
		SDL_Delay(KWaitDelayMS[(int)bRunning]);
	}
	while (!bEnd);

	GfLogInfo("SituationUpdater thread has been terminated.\n");
	
	return 0;
}

reSituationUpdater::reSituationUpdater(tRmInfo* pReInfo)
{
	// Save the race engine info (state + situation) pointer for the current step.
	_pCurrReInfo = pReInfo;
	_nInitDrivers = _pCurrReInfo->s->_ncars;

	// Determine if we have a dedicated separate thread or not
	// (according to the user settings, and the actual number of CPUs).
	std::ostringstream ossConfFile;
	ossConfFile << GfLocalDir() << RACE_ENG_CFG;
	void *paramHandle =
		GfParmReadFile(ossConfFile.str().c_str(), GFPARM_RMODE_REREAD | GFPARM_RMODE_CREAT);
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

	// Disable movie capture feature in dual-threaded mode
	// (only available in mono-thread mode, because of a non-"real-time" behaviour).
	if (_bThreaded && _pCurrReInfo->movieCapture.enabled)
	{
		_pCurrReInfo->movieCapture.enabled = 0;
		GfLogInfo("Movie capture disabled (not implemented in multi-threaded mode)\n");
	}
	
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

	GfLogInfo("SituationUpdater initialized (%sseparate thread, CPU affinity %s).\n",
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
		GfLogInfo("%s : Failed to lock data mutex\n", pszLocker);

	return bStatus;
}
	
bool reSituationUpdater::unlockData(const char* pszLocker)
{
	if (!_bThreaded)
		return true;
	
	const bool bStatus = SDL_mutexV(_pDataMutex) == 0;
	if (!bStatus)
		GfLogInfo("%s : Failed to unlock data mutex\n", pszLocker);

	return bStatus;
}


void reSituationUpdater::start()
{
	GfLogInfo("Starting race engine.\n");

	// Lock the race engine data.
	lockData("reSituationUpdater::start");

	// Set the running flags.
    _pCurrReInfo->_reRunning = 1;
	//_pCurrReInfo->s->_raceState &= ~RM_RACE_PAUSED;

	// Resynchronize simulation time.
    _pCurrReInfo->_reCurTime = GfTimeClock() - RCM_MAX_DT_SIMU;
	
	// Unlock the race engine data.
	unlockData("reSituationUpdater::start");
}

void reSituationUpdater::stop()
{
	GfLogInfo("Stopping race engine.\n");

	// Lock the race engine data.
	lockData("reSituationUpdater::stop");

	// Reset the running flags.
	_pCurrReInfo->_reRunning = 0;
	//_pCurrReInfo->s->_raceState |= RM_RACE_PAUSED;
		
	// Unlock the race engine data.
	unlockData("reSituationUpdater::stop");
}

int reSituationUpdater::terminate()
{
	int status = 0;
	
	GfLogInfo("Terminating race engine.\n");

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

void reSituationUpdater::acknowledgeEvents()
{
	ReSituationAcknowlegdeEvents(_pCurrReInfo, _pPrevReInfo);
}

tRmInfo* reSituationUpdater::getPreviousStep()
{
	if (!_bThreaded)
	{
		// No multi-threading : no need to really copy.
		_pPrevReInfo = _pCurrReInfo;
	}
	else
	{
		// Lock the race engine data.
		if (!lockData("reSituationUpdater::getPreviousStep"))
			return 0;

		// Get the situation data.
		ReSituationCopy(_pPrevReInfo, _pCurrReInfo);

		// Unlock the race engine data.
		if (!unlockData("reSituationUpdater::getPreviousStep"))
			return 0;
	}

	return _pPrevReInfo;
}

// This member function decorates the situation updater as a normal function,
// thus hiding the possible separate thread behind to the main updater. 
void reSituationUpdater::computeCurrentStep()
{
	// Nothing to do if actually threaded :
	// the updater thread is already doing the job on his side.
	if (!_bThreaded)
	{
		GfProfStartProfile("reOneStep*");
			
		if (_pCurrReInfo->_displayMode == RM_DISP_MODE_CAPTURE)
		{
			tRmMovieCapture	*capture = &(_pCurrReInfo->movieCapture);
			while ((_pCurrReInfo->_reCurTime - capture->lastFrame) < capture->deltaFrame)

				runOneStep(capture->deltaSimu);

			capture->lastFrame = _pCurrReInfo->_reCurTime;
		}
		else
		{
			const double t = GfTimeClock();
		
			while (_pCurrReInfo->_reRunning && ((t - _pCurrReInfo->_reCurTime) > RCM_MAX_DT_SIMU))
				
				runOneStep(RCM_MAX_DT_SIMU);
		}

		GfProfStopProfile("reOneStep*");
		
		// Send car physics to network if needed
		if (GetNetwork())
			GetNetwork()->SendCarControlsPacket(_pCurrReInfo->s);
	}
}

void reSituationUpdater::updateCarPitCmd(int nCarIndex, const tCarPitCmd *pPitCmd)
{
	if (_bThreaded)
	{
		// Lock the race engine data.
		if (!lockData("reSituationUpdater::updateCarPitCmd"))
			return;
	}
	
	// Retrieve the car in pCurrSituation corresponding to pPrevCar
	// (beware: they may have been moved in pCurrSituation->s->cars
	//          since pPrevSituation was built).
	//GfLogDebug("reSituationUpdater::updateCarPitCmd(i=%d)\n", nCarIndex);
	tCarElt* pCurrCar = 0;
	for (int nCarInd = 0; nCarInd < _pCurrReInfo->s->_ncars; nCarInd++)
	{
		if (_pCurrReInfo->s->cars[nCarInd]->index == nCarIndex)
		{
			// Found : update its pit command information from pit menu data.
			pCurrCar = _pCurrReInfo->s->cars[nCarInd];
			pCurrCar->_pitFuel = pPitCmd->fuel;
			pCurrCar->_pitRepair = pPitCmd->repair;
			pCurrCar->_pitStopType = pPitCmd->stopType;
			break;
		}
	}
	
	// Compute and set pit time.
	if (pCurrCar)
		ReCarsUpdateCarPitTime(pCurrCar);
	else
		GfLogError("Failed to retrieve car with index %d when computing pit time\n", nCarIndex);
	
	if (_bThreaded)
	{
		// Unlock the race engine data.
		if (!unlockData("reSituationUpdater::updateCarPitCmd"))
			return;
	}
}
// The main updater class ============================================================
class reMainUpdater
{
public:
	
	//! Constructor.
	reMainUpdater(reSituationUpdater* pSituUpdater);

	//! Initialize the graphics engine about cars.
	void initCarGraphics(void);

	//! Return from pit menu
	void onBackFromPitMenu(tCarElt *car);

	//! Capture currently displayed screen frame to a PNG file.
	void captureScreen(void);
	
	//! The real updating funtion.
	int operator()(void);

private:

	//! The last step situation got from the situationUpdater.
	tRmInfo* _pReInfo;

	//! The associated situation updater.
	reSituationUpdater* _pSituationUpdater;

#ifndef NoFPSLimiter
	//! Forced max. graphics update rate (Hz) (0 means no maximum).
	double _dMaxRate;

	//! Last time a real graphics update was done (only used if _dMaxRate).
	double _dLastTime;
#endif
};

// The main updater instance (intialized in ReInitUpdaters).
static reMainUpdater* mainUpdater = 0;


// Return from pit menu (onDeactivate pit menu callback)
static void reOnBackFromPitMenu(void *pvcar)
{
	mainUpdater->onBackFromPitMenu((tCarElt*)pvcar);
}

reMainUpdater::reMainUpdater(reSituationUpdater* pSituUpdater)
: _pReInfo(pSituUpdater->getPreviousStep()), _pSituationUpdater(pSituUpdater)
{
#ifndef NoFPSLimiter
	// Get the max. refresh rate from screen config params file.
	std::ostringstream ossConfFile;
	ossConfFile << GfLocalDir() << GFSCR_CONF_FILE;
	void* hScrConfParams = GfParmReadFile(ossConfFile.str().c_str(), GFPARM_RMODE_STD);
	_dLastTime = 0.0;
	_dMaxRate =
		GfParmGetNum(hScrConfParams, GFSCR_SECT_PROP, GFSCR_ATT_MAXREFRESH, NULL, 0.0);
	if (_dMaxRate)
		GfLogInfo("FPS limiter is on (%d Hz).\n", (int)_dMaxRate);
	else
		GfLogInfo("FPS limiter is off.\n");
	
	GfParmReleaseHandle(hScrConfParams);
#endif
}

void reMainUpdater::initCarGraphics(void)
{
	RaceEngine::self().userInterface().loadCarsGraphics(_pReInfo->s);
}

void reMainUpdater::onBackFromPitMenu(tCarElt *car)
{
	_pSituationUpdater->updateCarPitCmd(car->index, &car->pitcmd);

	RaceEngine::self().userInterface().activateGameScreen();
}

void reMainUpdater::captureScreen(void)
{
	char filename[256];
    tRmMovieCapture	*capture = &(_pReInfo->movieCapture);
    
    snprintf(filename, sizeof(filename), "%s/sd-%4.4d-%8.8d.png", capture->outputBase,
			 capture->currentCapture, capture->currentFrame++);
	
    RaceEngine::self().userInterface().captureRaceScreen(filename);
}

int reMainUpdater::operator()(void)
{
	// Note: In reMainUpdater, we should not read/write ReInfo (only _pReInfo).
	//       But this is not _yet_ the case for _displayMode and _refreshDisplay,
	//       as long as we don't have finished the planned race engine code cleanup.
	
    GfProfStartProfile("ReUpdate");

	IUserInterface& userItf = RaceEngine::self().userInterface();
	
	switch (ReInfo->_displayMode)
	{
		case RM_DISP_MODE_CAPTURE:
		case RM_DISP_MODE_NORMAL:

			ReInfo->_refreshDisplay = 1;
			
#ifndef NoFPSLimiter
			// Auto FPS limitation if specified.
			if (_dMaxRate > 0)
			{
				// If too early to refresh graphics, do nothing more than wait a little.
				const double dCurrentTime = GfTimeClock();
				if (dCurrentTime < _dLastTime + 1.0 / _dMaxRate)
				{
					// Wait a little, to let the CPU take breath.
					// Note : Theorical resolution is 1ms, but actual one is from far more
					//        (10-15ms under Windows, even worse under Linux ?)
					//        which explains a lower than expected actual FPS mean.
					GfSleep(0.001);

					// Only giving back control to the scheduler gives good results
					// as for the actual mean FPS, but keeps the CPU 100 % (not very cool).
					//GfSleep(0.0);
			
					break;
				}

				// Otherwise, last update time is now : go on with graphics update.
				_dLastTime = dCurrentTime;
			}
#endif

			// Do the frame capture if specified.
			// Moved here, right before situationUpdater->getPreviousStep,
			// as we can only capture the already displayed frame,
			// (because the one generated by _reGraphicItf.refresh will be displayed
			//  _after_ ReUpdate returns : see guieventloop.cpp::GfefPostReDisplay)
			if (ReInfo->_displayMode == RM_DISP_MODE_CAPTURE)
				captureScreen();

			// Get the situation for the previous step.
			GfSchedBeginEvent("raceupdate", "situCopy");
			_pReInfo = situationUpdater->getPreviousStep();
			GfSchedEndEvent("raceupdate", "situCopy");

			// Compute the situation for the current step (mono-threaded race engine)
			// or do nothing (dual-threaded race engine : the updater thread does the job itself).
			_pSituationUpdater->computeCurrentStep();
			
			// If one (human) driver is in pit, switch the display loop to the pit menu.
			if (_pReInfo->_reInPitMenuCar) // Does this really work in capture mode ?
			{
				//if (ReInfo->_displayMode != RM_DISP_MODE_CAPTURE) {
				
				// First, stop the race engine.
				ReStop();

				// Then open the pit menu (will return in ReCarsUpdateCarPitCmd).
				userItf.activatePitMenu(_pReInfo->_reInPitMenuCar, reOnBackFromPitMenu);
			}
			else
			{
				// Update the graphics (display the current frame).
				GfSchedBeginEvent("raceupdate", "graphics");
				userItf.updateGraphicsView(_pReInfo->s);
				GfSchedEndEvent("raceupdate", "graphics");
				
				// Update on-screen racing messages for the user.
				ReRaceMsgUpdate(_pReInfo);
			}

			// Acknowledge the collision and human pit events occurred
			// since the last graphics update : we know now that they all have been processed
			// or at least being processed by the graphics engine / menu system
			// (the main thread's job).
			_pSituationUpdater->acknowledgeEvents();
			
			break;
	
		case RM_DISP_MODE_NONE:
			
			ReInfo->_refreshDisplay = 0;
	
			_pSituationUpdater->runOneStep(RCM_MAX_DT_SIMU);
			
			ReRaceMsgUpdate(_pReInfo);
			
			break;

		case RM_DISP_MODE_SIMU_SIMU:
			
			ReInfo->_refreshDisplay = 0;

			ReSimuSimu();

			ReCarsSortCars();

			break;
    }

	ReNetworkCheckEndOfRace();

	// Update the GUI and Schedule next call by the event loop (through reDisplay).
	RaceEngine::self().userInterface().update();

	GfProfStopProfile("ReUpdate");

    return RM_ASYNC;
}

// Public C API of raceupdate =========================================================

void ReInitUpdaters()
{
	ReInfo->_reRunning = 0;

 	if (!situationUpdater)
 		situationUpdater = new reSituationUpdater(ReInfo);

 	if (!mainUpdater)
 		mainUpdater = new reMainUpdater(situationUpdater);

	// Configure schedule spy                        (nMaxEv, ignoreDelay)
	GfSchedConfigureEventLog("raceupdate", "graphics", 10000, 0.0);
	GfSchedConfigureEventLog("raceupdate", "situCopy", 10000, 0.0);
	GfSchedConfigureEventLog("raceupdate", "robots",   10000, 0.0);
	GfSchedConfigureEventLog("raceupdate", "physics",  10000, 0.0);
}

void ReInitCarGraphics(void)
{
	mainUpdater->initCarGraphics();
}

void ReAccelerateTime(double fMultFactor)
{
	ReInfo->_reTimeMult *= fMultFactor;
	if (fMultFactor == 0.0)
	    ReInfo->_reTimeMult = 1.0;
	else if (ReInfo->_reTimeMult > 64.0)
	    ReInfo->_reTimeMult = 64.0;
	else if (ReInfo->_reTimeMult < 0.25)
	    ReInfo->_reTimeMult = 0.25;

	char buf[32];
    sprintf(buf, "Time x%.2f", 1.0 / ReInfo->_reTimeMult);
    ReRaceMsgSet(ReInfo, buf, 5); // TODO: Thread-safe access to ReInfo in multi-threaded mode !
}

void ReStart(void)
{
	GfSchedBeginSession("raceupdate");
	situationUpdater->start();
}

#ifdef DEBUG
void ReOneStep(double dt)
{
	situationUpdater->runOneStep(dt);
}
#endif

int ReUpdate(void)
{
	return (*mainUpdater)();
}

void ReStop(void)
{
	situationUpdater->stop();
	GfSchedEndSession("raceupdate");
}

void ReShutdownUpdaters()
{
	// Destroy the situation updater.
	delete situationUpdater;
	situationUpdater = 0;
	
	// Destroy the main updater.
	delete mainUpdater;
	mainUpdater = 0;

	// Close schedule spy session and print the report.
	GfSchedEndSession("raceupdate");
	//                                          (timeResol, durUnit, durResol)
	GfSchedPrintReport("raceupdate", "schedule.csv", 1.0e-3, 1.0e-3, 1.0e-6);
}

