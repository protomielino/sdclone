/***************************************************************************

    file        : racestate.cpp
    created     : Sat Nov 16 12:00:42 CET 2002
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
    		The Race Engine State Automaton
    @author	<a href=mailto:eric.espie@torcs.org>Eric Espie</a>
    @version	$Id$
*/

#include <raceman.h>

#include "genparoptv1.h"

#include "racesituation.h"
#include "racemain.h"
#include "raceinit.h"
#include "racenetwork.h"
#include "raceupdate.h"
#include "raceresults.h"
#include "portability.h"
#include "genetic.h"
#include "geneticglobal.h"

#include "racestate.h"


// State Automaton Init
void
ReStateInit(void *prevMenu)
{
}


// State Automaton Management
void
ReStateManage(void)
{
	if (genOptInit)
	{
      MyResults.TrackName = &(MyResults.TrackNameBuffer[0]);
      MyResults.CarType = &(MyResults.CarTypeBuffer[0]);
      MyResults.RobotName = &(MyResults.RobotNameBuffer[0]);

      genLoops = 1000;

	  //MyResults.QualifyingLapTime = FLT_MAX;
	  MyResults.RaceLapTime = FLT_MAX;
	  MyResults.BestTotalLapTime = FLT_MAX;

	  MyResults.WeightedBestLapTime = FLT_MAX;
	  MyResults.LastWeightedBestLapTime = FLT_MAX;

	  MyResults.BestLapTime = FLT_MAX;
	  MyResults.LastBestLapTime = FLT_MAX;

	  MyResults.DamagesTotal = 0;
	  MyResults.LastDamagesTotal = 0;

	  MyResults.MinSpeed = FLT_MAX;
	  MyResults.LastMinSpeed = FLT_MAX;

	  MyResults.TopSpeed = 0;
	  MyResults.LastTopSpeed = 0;

	}

	int mode = RM_SYNC | RM_NEXT_STEP;

	do {
		switch (ReInfo->_reState) {
			case RE_STATE_CONFIG:
				GfLogInfo("%s now in CONFIG state\n", ReInfo->_reName);
				genOptimization = true;
				if (genOptimization)
					genLoops = 1;
				// Race configuration
				mode = ReConfigure();
				if (mode & RM_NEXT_STEP) {
					ReInfo->_reState = RE_STATE_EVENT_INIT;
				}
				break;

			case RE_STATE_EVENT_INIT:
				GfLogInfo("%s now in EVENT_INIT state\n", ReInfo->_reName);
				// Load the event description (track and drivers list)
//	if (OptiCounter2 == 4)
//		_tgf_memCheck(false, true);
//	if (OptiCounter2 == 6)
//		_tgf_memCheck(false, false);
//	OptiCounter2++;
//	_tgf_memLoop(OptiCounter2);
				mode = ReRaceEventInit();
				if (mode & RM_NEXT_STEP) {
					ReInfo->_reState = RE_STATE_PRE_RACE;
				}
				break;

			case RE_STATE_PRE_RACE:
				GfLogInfo("%s now in PRE_RACE state\n", ReInfo->_reName);
				mode = RePreRace();
				if (mode & RM_NEXT_RACE) {
					if (mode & RM_NEXT_STEP) {
						ReInfo->_reState = RE_STATE_EVENT_SHUTDOWN;
					}
				} else if (mode & RM_NEXT_STEP) {
					ReInfo->_reState = RE_STATE_RACE_START;
				}
				break;

			case RE_STATE_RACE_START:
				GfLogInfo("%s now in RACE_START state\n", ReInfo->_reName);
				mode = ReRaceStart();
				
				if (genOptInit)
				{
				  genOptInit = false;

				  // Setup pointer to car data and track, car type and robot name
				  MyResults.car = ReInfo->s->cars[0];
  			      snprintf(MyResults.TrackNameBuffer, sizeof(MyResults.TrackNameBuffer),
					"%s", ReInfo->track->internalname);
  			      snprintf(MyResults.CarTypeBuffer, sizeof(MyResults.CarTypeBuffer),
					"%s", ReInfo->s->cars[0]->_carName);
  			      snprintf(MyResults.RobotNameBuffer, sizeof(MyResults.RobotNameBuffer),
					"%s", ReInfo->s->cars[0]->_teamname);
                
				  // Setup path to car setup file
	              char buf[255];
                  snprintf(buf,sizeof(buf),"%sdrivers/%s/%s/%s.xml",
		            GetLocalDir(),MyResults.RobotName,MyResults.CarType,MyResults.TrackName);
                  MyResults.Handle = GfParmReadFile(buf, GFPARM_RMODE_REREAD);
				  MyResults.GetInitialVal = true;

				  // In case the car setup file does not exist ...
				  if (!MyResults.Handle)
				  {
				    // ... use default setup file ...
   				    snprintf(buf,sizeof(buf),"%sdrivers/%s/%s/default.xml",
		              GetLocalDir(),MyResults.RobotName,MyResults.CarType);
                    void* Handle = GfParmReadFile(buf, GFPARM_RMODE_REREAD);

					if (Handle) // ... if existing ...
					{			// ... to create it.
						snprintf(buf,sizeof(buf),"drivers/%s/%s/%s.xml",
						MyResults.RobotName,MyResults.CarType,MyResults.TrackName);
						GfParmWriteFileSDHeader (buf, Handle, MyResults.CarType, "Wolf-Dieter Beelitz");
					}
					else		// ... else create an empty setup file
					{			
   					    snprintf(buf,sizeof(buf),"%sdrivers/%s/%s/%s.xml",
			              GetLocalDir(),MyResults.RobotName,MyResults.CarType,MyResults.TrackName);
				        void* Handle = GfParmReadFile(buf, GFPARM_RMODE_REREAD | GFPARM_RMODE_CREAT);

						snprintf(buf,sizeof(buf),"drivers/%s/%s/%s.xml",
						MyResults.RobotName,MyResults.CarType,MyResults.TrackName);
						GfParmWriteFileSDHeader (buf, Handle, MyResults.CarType, "Wolf-Dieter Beelitz");
						MyResults.GetInitialVal = false;
					}
					GfParmReleaseHandle(Handle);

					// Open created car type track setup file
					snprintf(buf,sizeof(buf),"%sdrivers/%s/%s/%s.xml",
		              GetLocalDir(),MyResults.RobotName,MyResults.CarType,MyResults.TrackName);
				    MyResults.Handle = GfParmReadFile(buf, GFPARM_RMODE_REREAD);

				  }

  				  ReImportGeneticParameters(&MyResults);
				}

				if (mode & RM_NEXT_STEP) {
					ReInfo->_reState = RE_STATE_NETWORK_WAIT;
					GfLogInfo("%s now in NETWORK_WAIT state\n", ReInfo->_reName);
				}
				break;

			case RE_STATE_NETWORK_WAIT:
				mode = ReNetworkWaitReady();
				if (mode & RM_NEXT_STEP) {
					// Not an online race, or else all online players ready
					ReInfo->_reState = RE_STATE_RACE;
					GfLogInfo("%s now in RACE state\n", ReInfo->_reName);
				}
				break;

			case RE_STATE_RACE:
				mode = ReUpdate();
				if (ReInfo->s->_raceState == RM_RACE_ENDED) {
					// Race is finished
					ReInfo->_reState = RE_STATE_RACE_END;
				} else if (mode & RM_END_RACE) {
					// Race was interrupted (paused) by the player
					ReInfo->_reState = RE_STATE_RACE_STOP;
				}
				break;

			case RE_STATE_RACE_STOP:
				GfLogInfo("%s now in RACE_STOP state\n", ReInfo->_reName);
				// Race was interrupted (paused) by the player
				mode = ReRaceStop();
				if (mode & RM_NEXT_STEP) {
					ReInfo->_reState = RE_STATE_RACE_END;
				}
				break;

			case RE_STATE_RACE_END:
				GfLogInfo("%s now in RACE_END state\n", ReInfo->_reName);
				mode = ReRaceEnd();
				if (mode & RM_NEXT_STEP) {
					ReInfo->_reState = RE_STATE_POST_RACE;
				} else if (mode & RM_NEXT_RACE) {
					ReInfo->_reState = RE_STATE_RACE_START;
				}
				break;

			case RE_STATE_POST_RACE:
				GfLogInfo("%s now in POST_RACE state\n", ReInfo->_reName);
				mode = RePostRace();
				if (mode & RM_NEXT_STEP) {
					ReInfo->_reState = RE_STATE_EVENT_SHUTDOWN;
				} else if (mode & RM_NEXT_RACE) {
					ReInfo->_reState = RE_STATE_PRE_RACE;
				}
				break;

			case RE_STATE_EVOLUTION:
				GfLogInfo("RaceEngine: state = RE_STATE_EVOLUTION\n");
				if (genOptimization)
		  			mode = ReEvolution(genLoops * genLoops / 1000000.0);
				if ((genOptimization) && (genLoops--))
				  /* Back to optimization */
				  ReInfo->_reState = RE_STATE_EVENT_INIT;
				else
				  /* Next step */
				  ReInfo->_reState = RE_STATE_SHUTDOWN;
				break;

			case RE_STATE_EVENT_SHUTDOWN:
				GfLogInfo("%s now in EVENT_SHUTDOWN state\n", ReInfo->_reName);
				mode = ReRaceEventShutdown();
				if (mode & RM_NEXT_STEP) {
					ReInfo->_reState = RE_STATE_EVOLUTION;
				} else if (mode & RM_NEXT_RACE) {
					ReInfo->_reState = RE_STATE_EVENT_INIT;
				}
				break;

			case RE_STATE_SHUTDOWN:
				GfLogInfo("%s now in SHUTDOWN state\n", ReInfo->_reName);
				ReEvolutionCleanup();
				// Back to the race manager menu
				ReInfo->_reState = RE_STATE_CONFIG;
				mode = RM_SYNC;
				break;

			case RE_STATE_ERROR:
				// If this state is set, there was a serious error:
				// i.e. no driver in the race (no one selected OR parameters out of range)
				// Error messages are normally dumped in the game trace stream !
				// TODO: Define another screen showing the error messages instead of
				// only having it in the console window!
				GfLogInfo("%s now in ERROR state\n", ReInfo->_reName);
				// Back to race manager menu
				ReInfo->_reState = RE_STATE_CONFIG;
				mode = RM_SYNC;
				break;

			case RE_STATE_EXIT:
				// Exit the race engine.
				mode = ReExit();
				break;
		}

		if (mode & RM_ERROR) {
			GfLogError("Race engine error (see above messages)\n");
			ReInfo->_reState = RE_STATE_ERROR;
			mode = RM_SYNC;
		}

		//GfLogDebug("ReStateManage : New state 0x%X, %sing.\n",
		//		   ReInfo->_reState, (mode & RM_SYNC) ? "loop" : "return");
		
	} while (mode & RM_SYNC);
}

// Change and Execute a New State
void
ReStateApply(void *pvState)
{
	ReInfo->_reState = (int)(long)pvState;

	ReStateManage();
}
