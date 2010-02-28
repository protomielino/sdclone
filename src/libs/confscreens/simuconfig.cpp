/***************************************************************************

    file        : simuconfig.cpp
    created     : Wed Nov  3 21:48:26 CET 2004
    copyright   : (C) 2004 by Eric Espi�                       
    email       : eric.espie@free.fr  
    version     : $Id: simuconfig.cpp,v 1.4 2006/10/06 20:44:51 berniw Exp $                                  

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
    		
    @version	$Id: simuconfig.cpp,v 1.4 2006/10/06 20:44:51 berniw Exp $
*/

#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <tgfclient.h>
#include <raceinit.h>

#include "simuconfig.h"
#include <portability.h>

#include "gui.h"

/* list of available simulation engine */
static const char *simuVersionList[] = {"simuv2", "simuv3"};
static const int nbVersions = sizeof(simuVersionList) / sizeof(simuVersionList[0]);
static int curVersion = 0;

/* gui label id */
static int SimuVersionId;

/* gui screen handles */
static void	*scrHandle = NULL;
static void	*prevHandle = NULL;


static void ReadSimuCfg(void)
{
	const char *versionName;
	int i;

	char buf[1024];
	snprintf(buf, 1024, "%s%s", GetLocalDir(), RACE_ENG_CFG);

	void *paramHandle = GfParmReadFile(buf, GFPARM_RMODE_REREAD | GFPARM_RMODE_CREAT);
	versionName = GfParmGetStr(paramHandle, "Modules", "simu", simuVersionList[0]);

	for (i = 0; i < nbVersions; i++) {
		if (strcmp(versionName, simuVersionList[i]) == 0) {
			curVersion = i;
			break;
		}
	}

	GfParmReleaseHandle(paramHandle);

	GfuiLabelSetText(scrHandle, SimuVersionId, simuVersionList[curVersion]);
}


/* Save the choosen values in the corresponding parameter file */
static void SaveSimuVersion(void * /* dummy */)
{
	char buf[1024];
	snprintf(buf, 1024, "%s%s", GetLocalDir(), RACE_ENG_CFG);

	void *paramHandle = GfParmReadFile(buf, GFPARM_RMODE_REREAD | GFPARM_RMODE_CREAT);
	GfParmSetStr(paramHandle, "Modules", "simu", simuVersionList[curVersion]);
	GfParmWriteFile(NULL, paramHandle, "raceengine");
	GfParmReleaseHandle(paramHandle);
	
	/* return to previous screen */
	GfuiScreenActivate(prevHandle);
	return;
}

/* change the simulation version */
static void
ChangeSimuVersion(void *vp)
{
    if (vp == 0) {
	curVersion--;
	if (curVersion < 0) {
	    curVersion = nbVersions - 1;
	}
    } else {
	curVersion++;
	if (curVersion == nbVersions) {
	    curVersion = 0;
	}
    }
    GfuiLabelSetText(scrHandle, SimuVersionId, simuVersionList[curVersion]);
}


static void onActivate(void * /* dummy */)
{
    ReadSimuCfg();
}


/* Menu creation */
void *
SimuMenuInit(void *prevMenu)
{
    /* screen already created */
    if (scrHandle) {
	return scrHandle;
    }
    prevHandle = prevMenu;

    scrHandle = GfuiScreenCreateEx((float*)NULL, NULL, onActivate, NULL, (tfuiCallback)NULL, 1);

    void *param = LoadMenuXML("simulationmenu.xml");
    CreateStaticControls(param,scrHandle);

    CreateButtonControl(scrHandle,param,"simvleftarrow",(void*)-1,ChangeSimuVersion);
    CreateButtonControl(scrHandle,param,"simvrightarrow",(void*)1,ChangeSimuVersion);

    SimuVersionId = CreateLabelControl(scrHandle,param,"simulabel");
    CreateButtonControl(scrHandle,param,"accept",prevMenu,SaveSimuVersion);
    CreateButtonControl(scrHandle,param,"cancel",prevMenu,GfuiScreenActivate);


    GfParmReleaseHandle(param);
    
    GfuiAddKey(scrHandle, GFUIK_RETURN, "Save", NULL, SaveSimuVersion, NULL);
    GfuiAddKey(scrHandle, GFUIK_ESCAPE, "Cancel Selection", prevMenu, GfuiScreenActivate, NULL);
    GfuiAddSKey(scrHandle, GFUIK_F1, "Help", scrHandle, GfuiHelpScreen, NULL);
    GfuiAddSKey(scrHandle, GFUIK_F12, "Screen-Shot", NULL, GfuiScreenShot, NULL);
    GfuiAddSKey(scrHandle, GFUIK_LEFT, "Previous Option in list", (void*)0, ChangeSimuVersion, NULL);
    GfuiAddSKey(scrHandle, GFUIK_RIGHT, "Next Option in list", (void*)1, ChangeSimuVersion, NULL);

    return scrHandle;  
}
