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
    		
    @author	<a href=mailto:eric.espie@torcs.org>Eric Espie</a>
    @version	$Id$
*/

/* The Race Engine State Automaton */
#include <cstdlib>
#include <cstdio>

#include <tgfclient.h>
#include <raceman.h>
#include <racescreens.h>

#include "racesituation.h"
#include "racemain.h"
#include "raceinit.h"
#include "racenetwork.h"
#include "raceupdate.h"
#include "racegl.h"
#include "raceresults.h"
#include "racemanmenu.h"

#include "racestate.h"

static void *mainMenu;


/* State Automaton Init */
void
ReStateInit(void *prevMenu)
{
	mainMenu = prevMenu;
}


/* State Automaton Management         */
/* Called when a race menu is entered */
void
ReStateManage(void)
{
	int mode = RM_SYNC | RM_NEXT_STEP;

	do {
		switch (ReInfo->_reState) {
			case RE_STATE_CONFIG:
				GfLogInfo("%s now in CONFIG state\n", ReInfo->_reName);
				/* Display the race specific menu */
				mode = ReRacemanMenu();
				if (mode & RM_NEXT_STEP) {
					ReInfo->_reState = RE_STATE_EVENT_INIT;
				}
				break;

			case RE_STATE_EVENT_INIT:
				GfLogInfo("%s now in EVENT_INIT state\n", ReInfo->_reName);
				/* Load the event description (track and drivers list) */
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
				if (mode & RM_NEXT_STEP) {
					ReInfo->_reState = RE_STATE_NETWORK_WAIT;
				}
				break;

			case RE_STATE_NETWORK_WAIT:
				mode = ReNetworkWaitReady();
				if (mode & RM_NEXT_STEP) {
					/* Not an online race, or else all online players ready */
					ReInfo->_reState = RE_STATE_RACE;
				}
				break;

			case RE_STATE_RACE:
				mode = ReUpdate();
				if (ReInfo->s->_raceState == RM_RACE_ENDED) {
					/* Race is finished */
					ReInfo->_reState = RE_STATE_RACE_END;
				} else if (mode & RM_END_RACE) {
					/* Race was interrupted (paused) by the player */
					ReInfo->_reState = RE_STATE_RACE_STOP;
				}
				break;

			case RE_STATE_RACE_STOP:
				GfLogInfo("%s now in RACE_STOP state\n", ReInfo->_reName);
				/* Race was interrupted (paused) by the player */
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

			case RE_STATE_EVENT_SHUTDOWN:
				GfLogInfo("%s now in EVENT_SHUTDOWN state\n", ReInfo->_reName);

				mode = ReEventShutdown();
				if (mode & RM_NEXT_STEP) {
					ReInfo->_reState = RE_STATE_SHUTDOWN;
				} else if (mode & RM_NEXT_RACE) {
					ReInfo->_reState = RE_STATE_EVENT_INIT;
				}
				break;

			case RE_STATE_SHUTDOWN:
			case RE_STATE_ERROR:
				GfLogInfo("%s now in SHUTDOWN state\n", ReInfo->_reName);
				/* Back to race menu */
				ReInfo->_reState = RE_STATE_CONFIG;
				mode = RM_SYNC;
				break;

			case RE_STATE_EXIT:
				GfScrShutdown();
				exit (0);		/* brutal isn't it ? */
				break;
		}

		// If this mode is set, there was a serious error:
		// I.e. no driver in the race (no one selected OR parameters out of range!
		// Instead of exit(0) go back to the config mode to allow to read the 
		// error messages in the console window!
		// TODO: Define another screen showing the error messages instead of
		// only having it in the console window!
		if (mode & RM_QUIT) {
			GfScrShutdown();
			ReInfo->_reState = RE_STATE_CONFIG;
			mode = RM_SYNC;
		}
//	} while ((mode & (RM_SYNC | RM_QUIT)) == RM_SYNC);
	} while ((mode & RM_SYNC) == RM_SYNC);

/* Will not happen any longer
	if (mode & RM_QUIT) {
		GfScrShutdown();
		exit (0);		// brutal isn't it ? 
	}
*/
	if (mode & RM_ACTIVGAMESCR) {
		GfuiScreenActivate(ReInfo->_reGameScreen);
	}
}

/* Change and Execute a New State  */
void
ReStateApply(void *vstate)
{
	long state = (long)vstate;

	ReInfo->_reState = (int)state;
	ReStateManage();
}
