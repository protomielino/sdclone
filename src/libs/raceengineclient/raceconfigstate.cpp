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

#include <racescreens.h>

#include "racesituation.h"
//#include "racemain.h"
//#include "raceinit.h"
//#include "racestate.h"

#include "raceenginemenus.h"


// Data for the race configuration menus.
static tRmTrackSelect  ts;
static tRmDriverSelect ds;
static tRmRaceParam    rp;


static void
reConfigBack(void)
{
	void* params = ReInfo->params;

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
ReConfigRunState(void)
{
	char	path[256];
	int		i;
	int		curConf;
	const char	*conf;
	int		numOpt;
	const char	*opt;
	void	*params = ReInfo->params;

	curConf = (int)GfParmGetNum(params, RM_SECT_CONF, RM_ATTR_CUR_CONF, NULL, 1);
	if (curConf > GfParmGetEltNb(params, RM_SECT_CONF)) {
		GfLogInfo("%s configuration finished.\n", ReInfo->_reName);
		GfParmWriteFile(NULL, ReInfo->params, ReInfo->_reName);
		GfuiScreenActivate(ReGetRacemanMenuHandle()); /* Back to the race manager menu */
		return;
	}
	
	snprintf(path, sizeof(path), "%s/%d", RM_SECT_CONF, curConf);
	conf = GfParmGetStr(params, path, RM_ATTR_TYPE, 0);
	if (!conf) {
		GfLogError("No %s here (%s) !\n", RM_ATTR_TYPE, path);
		GfuiScreenActivate(ReGetRacemanMenuHandle()); /* Back to the race manager menu */
		return;
	}

	GfLogInfo("%s configuration now in '%s' stage.\n", ReInfo->_reName, conf);
	if (!strcmp(conf, RM_VAL_TRACKSEL)) {
		
		/* Track Select Menu */
		ts.nextScreen = reConfigHookInit();
		if (curConf == 1) {
			ts.prevScreen = ReGetRacemanMenuHandle();
		} else {
			ts.prevScreen = reConfigBackHookInit();
		}
		ts.param = ReInfo->params;
		ts.trackItf = ReInfo->_reTrackItf;
		RmTrackSelect(&ts);

	} else if (!strcmp(conf, RM_VAL_DRVSEL)) {
		
		/* Drivers select menu */
		ds.nextScreen = reConfigHookInit();
		if (curConf == 1) {
			ds.prevScreen = ReGetRacemanMenuHandle();
		} else {
			ds.prevScreen = reConfigBackHookInit();
		}
		ds.param = ReInfo->params;
		RmDriversSelect(&ds);

	} else if (!strcmp(conf, RM_VAL_RACECONF)) {
		
		/* Race Options menu */
		rp.nextScreen = reConfigHookInit();
		if (curConf == 1) {
			rp.prevScreen = ReGetRacemanMenuHandle();
		} else {
			rp.prevScreen = reConfigBackHookInit();
		}
		rp.param = ReInfo->params;
		rp.title = GfParmGetStr(params, path, RM_ATTR_RACE, "Race");
		
		/* Select options to configure */
		rp.confMask = 0;
		snprintf(path, sizeof(path), "%s/%d/%s", RM_SECT_CONF, curConf, RM_SECT_OPTIONS);
		numOpt = GfParmGetEltNb(params, path);
		for (i = 1; i < numOpt + 1; i++) {
			snprintf(path, sizeof(path), "%s/%d/%s/%d", RM_SECT_CONF, curConf, RM_SECT_OPTIONS, i);
			opt = GfParmGetStr(params, path, RM_ATTR_TYPE, "");
			if (!strcmp(opt, RM_VAL_CONFRACELEN)) {
				/* Configure race length */
				rp.confMask |= RM_CONF_RACE_LEN;
			} else if (!strcmp(opt, RM_VAL_CONFDISPMODE)) {
				/* Configure display mode */
				rp.confMask |= RM_CONF_DISP_MODE;
			} else if (!strcmp(opt, RM_VAL_CONFTIMEOFDAY)) {
				/* Configure time of day */
				rp.confMask |= RM_CONF_TIME_OF_DAY;
			} else if (!strcmp(opt, RM_VAL_CONFCLOUDCOVER)) {
				/* Configure cloud cover */
				rp.confMask |= RM_CONF_CLOUD_COVER;
			} else if (!strcmp(opt, RM_VAL_CONFRAINFALL)) {
				/* Configure rain fall and dry/wet track */
				rp.confMask |= RM_CONF_RAIN_FALL;
			}
		}

		/* Check if really something we can configure (given the graphic options) */
		if ((rp.confMask & (RM_CONF_TIME_OF_DAY | RM_CONF_CLOUD_COVER)) == rp.confMask) {
 	
			snprintf(path, sizeof(path), "%s%s", GetLocalDir(), GR_PARAM_FILE);
			void *grHandle = GfParmReadFile(path, GFPARM_RMODE_STD);
			const bool bSkyDomeEnabled =
				(int)GfParmGetNum(grHandle, GR_SCT_GRAPHIC, GR_ATT_SKYDOMEDISTANCE, NULL, 0) != 0;
			GfParmReleaseHandle(grHandle);
 	
			if (!bSkyDomeEnabled)
			{
				GfLogInfo("Skipping Race Params menu because Sky Dome is disabled"
						  " and is needed for all the configurable options\n");
				GfuiScreenActivate(ReGetRacemanMenuHandle()); /* Back to the race menu */
				return;
			}
		}
		GfLogTrace("Race configuration mask : 0x%02X\n", rp.confMask);
 	
		/* All's right till now : enter the Race Params menu */
		RmRaceParamsMenu(&rp);
	}

	curConf++;
	GfParmSetNum(params, RM_SECT_CONF, RM_ATTR_CUR_CONF, NULL, curConf);
}
