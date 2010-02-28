/***************************************************************************

    file        : racemanmenu.cpp
    created     : Fri Jan  3 22:24:41 CET 2003
    copyright   : (C) 2003 by Eric Espi�                        
    email       : eric.espie@torcs.org   
    version     : $Id: racemanmenu.cpp,v 1.5 2004/08/11 17:44:06 torcs Exp $                                  

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
    @version	$Id: racemanmenu.cpp,v 1.5 2004/08/11 17:44:06 torcs Exp $
*/

#include <stdlib.h>
#include <stdio.h>


#include "network.h"
#include <tgfclient.h>
#include <raceman.h>
#include <racescreens.h>
#include <driverconfig.h>

#include "raceengine.h"
#include "racemain.h"
#include "raceinit.h"
#include "racestate.h"

#include "racemanmenu.h"
#include "networkingmenu.h"



static void		*racemanMenuHdle = NULL;
static void		*newTrackMenuHdle = NULL;
static tRmTrackSelect	ts;
static tRmDrvSelect	ds;
static tRmRaceParam	rp;
static tRmFileSelect    fs;


static void reConfigRunState(void);

static void
reConfigBack(void)
{
    void	*params = ReInfo->params;

    /* Go back one step in the conf */
    GfParmSetNum(params, RM_SECT_CONF, RM_ATTR_CUR_CONF, NULL, 
		 GfParmGetNum(params, RM_SECT_CONF, RM_ATTR_CUR_CONF, NULL, 1) - 2);

    reConfigRunState();
}


/***************************************************************/
/* Callback hooks used only to run the automaton on activation */
static void	*configHookHandle = 0;

static void
configHookActivate(void * /* dummy */)
{
    reConfigRunState();
}

static void *
reConfigHookInit(void)
{
    if (configHookHandle) {
	return configHookHandle;
    }

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
    if (ConfigBackHookHandle) {
	return ConfigBackHookHandle;
    }

    ConfigBackHookHandle = GfuiHookCreate(0, ConfigBackHookActivate);

    return ConfigBackHookHandle;
}

static void
reConfigRunState(void)
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
	GfOut("End of configuration\n");
	GfParmWriteFile(NULL, ReInfo->params, ReInfo->_reName);
	goto menuback;
    }
    
    sprintf(path, "%s/%d", RM_SECT_CONF, curConf);
    conf = GfParmGetStr(params, path, RM_ATTR_TYPE, 0);
    if (!conf) {
	GfOut("no %s here %s\n", RM_ATTR_TYPE, path);
	goto menuback;
    }

    GfOut("Configuration step %s\n", conf);
    if (!strcmp(conf, RM_VAL_TRACKSEL)) {
	/* Track Select Menu */
	ts.nextScreen = reConfigHookInit();
	if (curConf == 1) {
	    ts.prevScreen = racemanMenuHdle;
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
	    ds.prevScreen = racemanMenuHdle;
	} else {
	    ds.prevScreen = reConfigBackHookInit();
	}
	ds.param = ReInfo->params;
	RmDriversSelect(&ds);

    } else if (!strcmp(conf, RM_VAL_RACECONF)) {
	/* Race Options menu */
	rp.nextScreen = reConfigHookInit();
	if (curConf == 1) {
	    rp.prevScreen = racemanMenuHdle;
	} else {
	    rp.prevScreen = reConfigBackHookInit();
	}
	rp.param = ReInfo->params;
	rp.title = GfParmGetStr(params, path, RM_ATTR_RACE, "Race");
	/* Select options to configure */
	rp.confMask = 0;
	sprintf(path, "%s/%d/%s", RM_SECT_CONF, curConf, RM_SECT_OPTIONS);
	numOpt = GfParmGetEltNb(params, path);
	for (i = 1; i < numOpt + 1; i++) {
	    sprintf(path, "%s/%d/%s/%d", RM_SECT_CONF, curConf, RM_SECT_OPTIONS, i);
	    opt = GfParmGetStr(params, path, RM_ATTR_TYPE, "");
	    if (!strcmp(opt, RM_VAL_CONFRACELEN)) {
		/* Configure race length */
		rp.confMask |= RM_CONF_RACE_LEN;
	    } else {
		if (!strcmp(opt, RM_VAL_CONFDISPMODE)) {
		    /* Configure display mode */
		    rp.confMask |= RM_CONF_DISP_MODE;
		}
	    }
	}
	RmRaceParamMenu(&rp);
    }

    curConf++;
    GfParmSetNum(params, RM_SECT_CONF, RM_ATTR_CUR_CONF, NULL, curConf);

    return;

    /* Back to the race menu */
 menuback:
    GfuiScreenActivate(racemanMenuHdle);
    return;
}

void SetRacemanMenuHandle( void * handle)
{
	racemanMenuHdle = handle;
}

void
reConfigureMenu(void * /* dummy */)
{
    void *params = ReInfo->params;

    /* Reset configuration automaton */
    GfParmSetNum(params, RM_SECT_CONF, RM_ATTR_CUR_CONF, NULL, 1);
    reConfigRunState();

}

static void
reSelectLoadFile(char *filename)
{
    char buf[512];

    sprintf(buf, "%sresults/%s/%s", GetLocalDir(), ReInfo->_reFilename, filename);
    GfOut("Loading Saved File %s...\n", buf);
    ReInfo->mainResults = GfParmReadFile(buf, GFPARM_RMODE_STD | GFPARM_RMODE_CREAT);
    ReInfo->results = ReInfo->mainResults;
    ReInfo->_reRaceName = ReInfo->_reName;
    GfParmRemoveVariable (ReInfo->params, "/", "humanInGroup");
    GfParmSetVariable (ReInfo->params, "/", "humanInGroup", humanInGroup() ? 1 : 0);
    RmShowStandings(ReInfo->_reGameScreen, ReInfo);
}

static void
reLoadMenu(void *prevHandle)
{
    char buf[512];

    const char *str;
    void *params = ReInfo->params;

    fs.prevScreen = prevHandle;
    fs.select = reSelectLoadFile;

    str = GfParmGetStr(params, RM_SECT_HEADER, RM_ATTR_NAME, 0);
    if (str) {
	fs.title = str;
    }
    sprintf(buf, "%sresults/%s", GetLocalDir(), ReInfo->_reFilename);
    fs.path = buf;

    RmFileSelect((void*)&fs);
}

static void
rePlayerConfig(void * /* dummy */)
{
    /* Here, we need to call OptionOptionInit each time the firing button
       is pressed, and not only once at the Raceman menu initialization,
       because the previous menu has to be saved (ESC, Back) and because it can be this menu,
       as well as the Main menu */
    GfuiScreenActivate(DriverMenuInit(racemanMenuHdle));
}

int
ReRacemanMenu(void)
{
    const char	*str;
    void	*params = ReInfo->params;

    str = GfParmGetStr(params, RM_SECT_HEADER, RM_ATTR_NAME, 0);
    if (strcmp(str,"Online Race")==0)
    {
		if (GetNetwork())
		{
			if (GetNetwork()->IsConnected())
			{
				if (IsClient())
				{
					reNetworkClientConnectMenu(NULL);
					return RM_ASYNC | RM_NEXT_STEP;
				}
				else if (IsServer())
				{
					reNetworkHostMenu(NULL);
					return RM_ASYNC | RM_NEXT_STEP;
				}
			}
		}
		else
		{
			reNetworkMenu(NULL);
			return RM_ASYNC | RM_NEXT_STEP;
		}

    }

    if (racemanMenuHdle) {
	GfuiScreenRelease(racemanMenuHdle);
    }

    // Create screen, load menu XML descriptor and create static controls.
    racemanMenuHdle = GfuiScreenCreateEx(NULL, 
					 NULL, (tfuiCallback)NULL, 
					 NULL, (tfuiCallback)NULL, 
					 1);
    void *menuXMLDescHdle = LoadMenuXML("racechoicemenu.xml");
    CreateStaticControls(menuXMLDescHdle,racemanMenuHdle);

    // Create variable title label.
    str = GfParmGetStr(params, RM_SECT_HEADER, RM_ATTR_NAME, 0);
    if (str) {
	int id = CreateLabelControl(racemanMenuHdle,menuXMLDescHdle,"title");
	GfuiLabelSetText(racemanMenuHdle,id,str);
    }

    // Create New race, Configure race, Configure players and Back buttons.
    CreateButtonControl(racemanMenuHdle,menuXMLDescHdle,"newrace",NULL,ReStartNewRace);
    CreateButtonControl(racemanMenuHdle,menuXMLDescHdle,"configurerace",NULL,reConfigureMenu);
    CreateButtonControl(racemanMenuHdle,menuXMLDescHdle,"configureplayers",NULL,rePlayerConfig);
    
    CreateButtonControl(racemanMenuHdle,menuXMLDescHdle,"backtomain",ReInfo->_reMenuScreen,GfuiScreenActivate);

    // Create Load race button if we are in a Champ' like race type.
    if (GfParmGetEltNb(params, RM_SECT_TRACKS) > 1) {
	CreateButtonControl(racemanMenuHdle,menuXMLDescHdle,"load",racemanMenuHdle,reLoadMenu);
    }
    
    // Close menu XML descriptor.
    GfParmReleaseHandle(menuXMLDescHdle);
    // Register keyboard shortcuts.
    GfuiMenuDefaultKeysAdd(racemanMenuHdle);
    GfuiAddKey(racemanMenuHdle, GFUIK_ESCAPE, "Back to Main menu", ReInfo->_reMenuScreen, GfuiScreenActivate, NULL);

    // Activate screen.
    GfuiScreenActivate(racemanMenuHdle);

    return RM_ASYNC | RM_NEXT_STEP;
}

static void
reStateManage(void * /* dummy */)
{
    ReStateManage();
}

int
ReNewTrackMenu(void)
{
    char buf[128];

    const char	*str;
    void	*params = ReInfo->params;
    void	*results = ReInfo->results;
    int		raceNumber = 0; // Never used ???

    if (newTrackMenuHdle) {
	GfuiScreenRelease(newTrackMenuHdle);
    }

    // Create screen, load menu XML descriptor and create static controls.
    newTrackMenuHdle = GfuiScreenCreateEx(NULL, 
					  NULL, (tfuiCallback)NULL, 
					  NULL, (tfuiCallback)NULL, 
					  1);
    void *menuXMLDescHdle = LoadMenuXML("newtrackmenu.xml");
    CreateStaticControls(menuXMLDescHdle,newTrackMenuHdle);

    // Create background image from race params.
    str = GfParmGetStr(params, RM_SECT_HEADER, RM_ATTR_BGIMG, 0);
    if (str) {
	GfuiScreenAddBgImg(newTrackMenuHdle, str);
    }

    // Create variable title label from race params.
    str = GfParmGetStr(params, RM_SECT_HEADER, RM_ATTR_NAME, "");
    int titleId = CreateLabelControl(newTrackMenuHdle, menuXMLDescHdle, "titlelabel");
    GfuiLabelSetText(newTrackMenuHdle, titleId, str);

    // Create variable subtitle label from race params.
    sprintf(buf, "Race Day #%d/%d on %s",
    raceNumber,
    (int)GfParmGetNum(params, RM_SECT_TRACKS, RM_ATTR_NUMBER, NULL, -1 ) >= 0 ?
    (int)GfParmGetNum(params, RM_SECT_TRACKS, RM_ATTR_NUMBER, NULL, -1 ) :
    GfParmGetEltNb(params, RM_SECT_TRACKS), 
    ReInfo->track->name);
    int subTitleId = CreateLabelControl(newTrackMenuHdle, menuXMLDescHdle, "subtitlelabel");
    GfuiLabelSetText(newTrackMenuHdle, subTitleId, buf);

    // Create Start and Abandon buttons.
    CreateButtonControl(newTrackMenuHdle, menuXMLDescHdle, "startbutton", NULL, reStateManage);
    CreateButtonControl(newTrackMenuHdle, menuXMLDescHdle, "abandonbutton", ReInfo->_reMenuScreen, GfuiScreenActivate);

    // Close menu XML descriptor.
    GfParmReleaseHandle(menuXMLDescHdle);
    
    // Register keyboard shortcuts.
    GfuiMenuDefaultKeysAdd(newTrackMenuHdle);
    GfuiAddKey(newTrackMenuHdle, GFUIK_RETURN, "Start Event", NULL, reStateManage, NULL);
    GfuiAddKey(newTrackMenuHdle, GFUIK_ESCAPE, "Abandon", ReInfo->_reMenuScreen, GfuiScreenActivate, NULL);

    // Activate screen.
    GfuiScreenActivate(newTrackMenuHdle);

    return RM_ASYNC | RM_NEXT_STEP;
}

