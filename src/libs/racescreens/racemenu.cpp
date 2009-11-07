/***************************************************************************

    file                 : racemenu.cpp
    created              : Thu May  2 22:02:51 CEST 2002
    copyright            : (C) 2001 by Eric Espie
    email                : eric.espie@torcs.org
    version              : $Id: racemenu.cpp,v 1.2 20 Mar 2006 04:31:12 torcs Exp $

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
    		This is the race options menu.
    @ingroup	racemantools
    @author	<a href=mailto:eric.espie@torcs.org>Eric Espie</a>
    @version	$Id: racemenu.cpp,v 1.2 2003/06/24 21:02:24 torcs Exp $
*/

#include <stdlib.h>
#include <stdio.h>
#include <cstring>
#ifdef WIN32
#include <windows.h>
#endif

#include <tgfclient.h>
#include <track.h>
#include <car.h>
#include <raceman.h>
#include <robot.h>
#include <graphic.h>

#include "racescreens.h"


// Constants.
static const char *DispModeValues[] = { RM_VAL_VISIBLE, RM_VAL_INVISIBLE};

// Global variables.
static void		*scrHandle;
static tRmRaceParam	*rp;

static int		rmrpDistId;
static int		rmrpLapsId;
static int		rmrpDispModeId;

static int		rmrpDistance;
static int		rmrpLaps;
static int		rmrpDispMode;


static void
rmrpDeactivate(void *screen)
{
    GfuiScreenRelease(scrHandle);
    
    if (screen) {
	GfuiScreenActivate(screen);
    }
}

static void
rmrpUpdDist(void * /* dummy */)
{
    char	buf[32];
    char	*val;

    val = GfuiEditboxGetString(scrHandle, rmrpDistId);
    rmrpDistance = strtol(val, (char **)NULL, 0);
    if (rmrpDistance == 0) {
	strcpy(buf, "---");
    } else {
	sprintf(buf, "%d", rmrpDistance);
	rmrpLaps = 0;
	GfuiEditboxSetString(scrHandle, rmrpLapsId, "---");
    }
    GfuiEditboxSetString(scrHandle, rmrpDistId, buf);
}

static void
rmrpUpdLaps(void * /* dummy */)
{
    char	buf[32];
    char	*val;

    val = GfuiEditboxGetString(scrHandle, rmrpLapsId);
    rmrpLaps = strtol(val, (char **)NULL, 0);
    if (rmrpLaps == 0) {
	strcpy(buf, "---");
    } else {
	sprintf(buf, "%d", rmrpLaps);
	rmrpDistance = 0;
	GfuiEditboxSetString(scrHandle, rmrpDistId, "---");
    }
    GfuiEditboxSetString(scrHandle, rmrpLapsId, buf);
}

void
rmChangeDisplayMode(void * /* dummy */)
{
    rmrpDispMode = 1 - rmrpDispMode;
    GfuiLabelSetText(scrHandle, rmrpDispModeId, DispModeValues[rmrpDispMode]);
}

static void
rmrpValidate(void * /* dummy */)
{
    GfuiUnSelectCurrent();

    if (rp->confMask & RM_CONF_RACE_LEN) {
	GfParmSetNum(rp->param, rp->title, RM_ATTR_DISTANCE, "km", rmrpDistance);
	GfParmSetNum(rp->param, rp->title, RM_ATTR_LAPS, (char*)NULL, rmrpLaps);
    }

    if (rp->confMask & RM_CONF_DISP_MODE) {
	GfParmSetStr(rp->param, rp->title, RM_ATTR_DISPMODE, DispModeValues[rmrpDispMode]);
    }

    rmrpDeactivate(rp->nextScreen);
}

static void
rmrpAddKeys(void)
{
    GfuiAddKey(scrHandle, 13, "Accept", NULL, rmrpValidate, NULL);
    GfuiAddKey(scrHandle, 27, "Cancel", rp->prevScreen, rmrpDeactivate, NULL);
    GfuiAddSKey(scrHandle, GLUT_KEY_F1, "Help", scrHandle, GfuiHelpScreen, NULL);
    GfuiAddSKey(scrHandle, GLUT_KEY_F12, "Screen-Shot", NULL, GfuiScreenShot, NULL);
}

void
RmRaceParamMenu(void *vrp)
{
    char	buf[64];

    rp = (tRmRaceParam*)vrp;

    // Create screen, load menu XML descriptor and create static controls.
    scrHandle = GfuiScreenCreateEx((float*)NULL, NULL, NULL, NULL, (tfuiCallback)NULL, 1);   
    void *menuXMLDescHdle = LoadMenuXML("racemenu.xml");
    CreateStaticControls(menuXMLDescHdle,scrHandle);

    // Create variable title label.
    int titleId = CreateLabelControl(scrHandle,menuXMLDescHdle,"title");
    sprintf(buf, "%s Options", rp->title);
    GfuiLabelSetText(scrHandle,titleId,buf);
    
    if (rp->confMask & RM_CONF_RACE_LEN) 
    {
	// Create Race distance label.
	CreateLabelControl(scrHandle,menuXMLDescHdle,"racedistance");

	// Create and initialize Race distance edit.
	rmrpDistance = (int)GfParmGetNum(rp->param, rp->title, RM_ATTR_DISTANCE, "km", 0);
	if (rmrpDistance == 0) 
	{
	    strcpy(buf, "---");
	    rmrpLaps = (int)GfParmGetNum(rp->param, rp->title, RM_ATTR_LAPS, NULL, 25);
	} 
	else 
	{
	    sprintf(buf, "%d", rmrpDistance);
	    rmrpLaps = 0;
	}

	rmrpDistId = CreateEditControl(scrHandle,menuXMLDescHdle,"racedistanceedit",NULL,NULL,rmrpUpdDist);
	GfuiEditboxSetString(scrHandle,rmrpDistId,buf);

	// Create Laps label.
	CreateLabelControl(scrHandle,menuXMLDescHdle,"laps");
	
	// Create and initialize Laps edit.
	if (rmrpLaps == 0) 
	{
	    strcpy(buf, "---");
	} 
	else 
	{
	    sprintf(buf, "%d", rmrpLaps);
	}

	rmrpLapsId = CreateEditControl(scrHandle,menuXMLDescHdle,"lapsedit",NULL,NULL,rmrpUpdLaps);
	GfuiEditboxSetString(scrHandle,rmrpLapsId,buf);
    }

    if (rp->confMask & RM_CONF_DISP_MODE) 
    {
	if (!strcmp(GfParmGetStr(rp->param, rp->title, RM_ATTR_DISPMODE, RM_VAL_VISIBLE), RM_VAL_INVISIBLE)) 
	{
	    rmrpDispMode = 1;
	}
	else 
	{
	    rmrpDispMode = 0;
	}

	CreateButtonControl(scrHandle,menuXMLDescHdle,"displayleftarrow",(void*)0, rmChangeDisplayMode);
	CreateButtonControl(scrHandle,menuXMLDescHdle,"displayrightarrow",(void*)1, rmChangeDisplayMode);
	rmrpDispModeId = CreateLabelControl(scrHandle,menuXMLDescHdle,"display");
	GfuiLabelSetText(scrHandle,rmrpDispModeId,DispModeValues[rmrpDispMode]);
    }
	
    // Create Accept and Cancel buttons
    CreateButtonControl(scrHandle,menuXMLDescHdle,"accept",NULL,rmrpValidate);
    CreateButtonControl(scrHandle,menuXMLDescHdle,"cancel",rp->prevScreen,rmrpDeactivate);
    
    // Close menu XML descriptor.
    GfParmReleaseHandle(menuXMLDescHdle);
    
    // Register keyboard shortcuts.
    rmrpAddKeys();
    
    GfuiScreenActivate(scrHandle);
}
