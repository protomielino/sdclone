/***************************************************************************

    file        : raceconfigstate.cpp
    created     : Fri Jan  3 22:24:41 CET 2003
    copyright   : (C) 2003 by Eric Espie                        
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
    		The automaton for race configuration
    @author	<a href=mailto:eric.espie@torcs.org>Eric Espie</a>
    @version	$Id$
*/

#include <cstdlib>
#include <cstdio>

#include <portability.h>
#include <tgfclient.h>

#include <raceman.h>

#include <race.h>

#include "legacymenu.h"
#include "racescreens.h"


// Data for the race configuration menus.
static tRmTrackSelect  ts;
static tRmDriverSelect ds;
static tRmRaceParam    rp;


static void
reConfigBack(void)
{
	tRmInfo* reInfo = LegacyMenu::self().raceEngine().data();
	void* params = reInfo->params;

	/* Go back one step in the conf */
	GfParmSetNum(params, RM_SECT_CONF, RM_ATTR_CUR_CONF, NULL, 
				 GfParmGetNum(params, RM_SECT_CONF, RM_ATTR_CUR_CONF, NULL, 1) - 2);

	ReConfigRunState();
}


/***************************************************************/
/* Callback hooks used only to run the automaton on activation */
static void	*configHookHandle = 0;

static void
configHookActivate(void * /* dummy */)
{
	ReConfigRunState();
}

static void *
reConfigHookInit(void)
{
	if (configHookHandle)
		return configHookHandle;

	configHookHandle = GfuiHookCreate(0, configHookActivate);

	return configHookHandle;
}

/***************************************************************/
/* Config Back Hook */

static void	*ConfigBackHookHandle = 0;

static void
ConfigBackHookActivate(void * /* dummy */)
{
	reConfigBack();
}

static void *
reConfigBackHookInit(void)
{
	if (ConfigBackHookHandle)
		return ConfigBackHookHandle;

	ConfigBackHookHandle = GfuiHookCreate(0, ConfigBackHookActivate);

	return ConfigBackHookHandle;
}

void
ReConfigRunState(bool bStart)
{
	char	path[256];
	int		curConf;
	const char	*conf;
	tRmInfo* reInfo = LegacyMenu::self().raceEngine().data();
	void	*params = reInfo->params;

	// TODO: Replace any read/write to params to get/set from/to race/raceman instances ?
	
	// Reset config automaton to the "start" state if specified.
	if (bStart)
		GfParmSetNum(params, RM_SECT_CONF, RM_ATTR_CUR_CONF, NULL, 1);

	// If configuration finished, save race config to disk and go back to the raceman menu.
	curConf = (int)GfParmGetNum(params, RM_SECT_CONF, RM_ATTR_CUR_CONF, NULL, 1);
	if (curConf > GfParmGetEltNb(params, RM_SECT_CONF)) {
		GfLogInfo("%s configuration finished.\n", reInfo->_reName);
		LegacyMenu::self().raceEngine().race()->store(); // Save race data to params.
		GfParmWriteFile(NULL, params, reInfo->_reName); // Save params to disk.
		GfuiScreenActivate(ReGetRacemanMenuHandle()); // Back to the race manager menu
		return;
	}

	// If wrong configuration data, back to the raceman menu.
	snprintf(path, sizeof(path), "%s/%d", RM_SECT_CONF, curConf);
	conf = GfParmGetStr(params, path, RM_ATTR_TYPE, 0);
	if (!conf) {
		GfLogError("No '%s' field in '%s' section of %s\n",
				   RM_ATTR_TYPE, path, GfParmGetFileName(params));
		GfuiScreenActivate(ReGetRacemanMenuHandle()); /* Back to the race manager menu */
		return;
	}

	// Normal configuration steps :
	GfLogInfo("%s configuration now in #%d '%s' stage.\n", reInfo->_reName, curConf, conf);
	
	if (!strcmp(conf, RM_VAL_TRACKSEL)) {
		
		// Track Select Menu 
		ts.nextScreen = reConfigHookInit();
		if (curConf == 1) {
			ts.prevScreen = ReGetRacemanMenuHandle();
		} else {
			ts.prevScreen = reConfigBackHookInit();
		}
		ts.pRace = LegacyMenu::self().raceEngine().race();
		ts.trackItf = reInfo->_reTrackItf;
		RmTrackSelect(&ts);

	} else if (!strcmp(conf, RM_VAL_DRVSEL)) {
		
		// Drivers select menu
		ds.nextScreen = reConfigHookInit();
		if (curConf == 1) {
			ds.prevScreen = ReGetRacemanMenuHandle();
		} else {
			ds.prevScreen = reConfigBackHookInit();
		}
		ds.pRace = LegacyMenu::self().raceEngine().race();
		RmDriversSelect(&ds);

	} else if (!strcmp(conf, RM_VAL_RACECONF)) {
		
		// Race (= session) Options menu
		rp.nextScreen = reConfigHookInit();
		if (curConf == 1) {
			rp.prevScreen = ReGetRacemanMenuHandle();
		} else {
			rp.prevScreen = reConfigBackHookInit();
		}
		rp.pRace = LegacyMenu::self().raceEngine().race();
		rp.session = GfParmGetStr(params, path, RM_ATTR_RACE, RM_VAL_ANYRACE);
		RmRaceParamsMenu(&rp);
	}
	
	// Prepare next configuration if any.
	curConf++;
	GfParmSetNum(params, RM_SECT_CONF, RM_ATTR_CUR_CONF, NULL, curConf);
}
