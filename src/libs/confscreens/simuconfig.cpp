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

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <raceman.h>
#include <portability.h>
#include <tgfclient.h>

#include "simuconfig.h"

#include "gui.h"


/* list of available simulation engine */
static const char *SimuVersionList[] = {RM_VAL_MOD_SIMU_V2, RM_VAL_MOD_SIMU_V3};
static const int NbSimuVersions = sizeof(SimuVersionList) / sizeof(SimuVersionList[0]);
static int CurSimuVersion = 0;

/* list of available multi-threading schemes */
static const char *MultiThreadSchemeList[] = {RM_VAL_AUTO, RM_VAL_ON, RM_VAL_OFF};
static const int NbMultiThreadSchemes = sizeof(MultiThreadSchemeList) / sizeof(MultiThreadSchemeList[0]);

/* list of available thread affinity schemes */
static const char *ThreadAffinitySchemeList[] = {RM_VAL_ON, RM_VAL_OFF};
static const int NbThreadAffinitySchemes = sizeof(ThreadAffinitySchemeList) / sizeof(ThreadAffinitySchemeList[0]);

static int CurMultiThreadScheme = 0;    // Auto
static int CurThreadAffinityScheme = 0; // On

/* gui label ids */
static int SimuVersionId;
static int MultiThreadSchemeId;
static int ThreadAffinitySchemeId;

/* gui screen handles */
static void *ScrHandle = NULL;
static void *PrevScrHandle = NULL;


static void loadSimuCfg(void)
{
	const char *simuVersionName;
	const char *multiThreadSchemeName;
	const char *threadAffinitySchemeName;
	int i;

	char buf[1024];
	snprintf(buf, 1024, "%s%s", GetLocalDir(), RACE_ENG_CFG);

	void *paramHandle = GfParmReadFile(buf, GFPARM_RMODE_REREAD | GFPARM_RMODE_CREAT);
	
	simuVersionName = GfParmGetStr(paramHandle, RM_SECT_MODULES, RM_ATTR_MOD_SIMU, SimuVersionList[0]);
	for (i = 0; i < NbSimuVersions; i++) {
		if (strcmp(simuVersionName, SimuVersionList[i]) == 0) {
			CurSimuVersion = i;
			break;
		}
	}

	multiThreadSchemeName = GfParmGetStr(paramHandle, RM_SECT_RACE_ENGINE, RM_ATTR_MULTI_THREADING, MultiThreadSchemeList[0]);
	for (i = 0; i < NbMultiThreadSchemes; i++) {
		if (strcmp(multiThreadSchemeName, MultiThreadSchemeList[i]) == 0) {
			CurMultiThreadScheme = i;
			break;
		}
	}

	threadAffinitySchemeName = GfParmGetStr(paramHandle, RM_SECT_RACE_ENGINE, RM_ATTR_THREAD_AFFINITY, ThreadAffinitySchemeList[0]);
	for (i = 0; i < NbThreadAffinitySchemes; i++) {
		if (strcmp(threadAffinitySchemeName, ThreadAffinitySchemeList[i]) == 0) {
			CurThreadAffinityScheme = i;
			break;
		}
	}

	GfParmReleaseHandle(paramHandle);

	GfuiLabelSetText(ScrHandle, SimuVersionId, SimuVersionList[CurSimuVersion]);
	GfuiLabelSetText(ScrHandle, MultiThreadSchemeId, MultiThreadSchemeList[CurMultiThreadScheme]);
	GfuiLabelSetText(ScrHandle, ThreadAffinitySchemeId, ThreadAffinitySchemeList[CurThreadAffinityScheme]);
}


/* Save the choosen values in the corresponding parameter file */
static void storeSimuCfg(void * /* dummy */)
{
	char buf[1024];
	snprintf(buf, 1024, "%s%s", GetLocalDir(), RACE_ENG_CFG);

	void *paramHandle = GfParmReadFile(buf, GFPARM_RMODE_REREAD | GFPARM_RMODE_CREAT);
	GfParmSetStr(paramHandle, RM_SECT_MODULES, RM_ATTR_MOD_SIMU, SimuVersionList[CurSimuVersion]);
	GfParmSetStr(paramHandle, RM_SECT_RACE_ENGINE, RM_ATTR_MULTI_THREADING, MultiThreadSchemeList[CurMultiThreadScheme]);
	GfParmSetStr(paramHandle, RM_SECT_RACE_ENGINE, RM_ATTR_THREAD_AFFINITY, ThreadAffinitySchemeList[CurThreadAffinityScheme]);
	GfParmWriteFile(NULL, paramHandle, "raceengine");
	GfParmReleaseHandle(paramHandle);
	
	/* return to previous screen */
	GfuiScreenActivate(PrevScrHandle);
	return;
}

/* Change the simulation version */
static void
onChangeSimuVersion(void *vp)
{
	CurSimuVersion = (CurSimuVersion + NbSimuVersions + (int)(long)vp) % NbSimuVersions;
	
	GfuiLabelSetText(ScrHandle, SimuVersionId, SimuVersionList[CurSimuVersion]);
}


/* Change the multi-threading scheme */
static void
onChangeMultiThreadScheme(void *vp)
{
	CurMultiThreadScheme =
		(CurMultiThreadScheme + NbMultiThreadSchemes + (int)(long)vp) % NbMultiThreadSchemes;
	
	GfuiLabelSetText(ScrHandle, MultiThreadSchemeId, MultiThreadSchemeList[CurMultiThreadScheme]);
}


/* Change the thread affinity scheme */
static void
onChangeThreadAffinityScheme(void *vp)
{
	CurThreadAffinityScheme =
		(CurThreadAffinityScheme + NbThreadAffinitySchemes + (int)(long)vp) % NbThreadAffinitySchemes;
	
	GfuiLabelSetText(ScrHandle, ThreadAffinitySchemeId, ThreadAffinitySchemeList[CurThreadAffinityScheme]);
}


static void onActivate(void * /* dummy */)
{
    loadSimuCfg();
}


/* Menu creation */
void *
SimuMenuInit(void *prevMenu)
{
    /* screen already created */
    if (ScrHandle) {
	return ScrHandle;
    }
    PrevScrHandle = prevMenu;

    ScrHandle = GfuiScreenCreateEx((float*)NULL, NULL, onActivate, NULL, (tfuiCallback)NULL, 1);

    void *menuDescHdle = LoadMenuXML("simulationmenu.xml");
    CreateStaticControls(menuDescHdle, ScrHandle);

    SimuVersionId = CreateLabelControl(ScrHandle,menuDescHdle,"simulabel");
    CreateButtonControl(ScrHandle, menuDescHdle, "simuleftarrow", (void*)-1, onChangeSimuVersion);
    CreateButtonControl(ScrHandle, menuDescHdle, "simurightarrow", (void*)1, onChangeSimuVersion);

    MultiThreadSchemeId = CreateLabelControl(ScrHandle, menuDescHdle, "mthreadlabel");

    CreateButtonControl(ScrHandle, menuDescHdle, "mthreadleftarrow", (void*)-1, onChangeMultiThreadScheme);
    CreateButtonControl(ScrHandle, menuDescHdle, "mthreadrightarrow", (void*)1, onChangeMultiThreadScheme);

    ThreadAffinitySchemeId = CreateLabelControl(ScrHandle, menuDescHdle, "threadafflabel");

    CreateButtonControl(ScrHandle, menuDescHdle, "threadaffleftarrow", (void*)-1, onChangeThreadAffinityScheme);
    CreateButtonControl(ScrHandle, menuDescHdle, "threadaffrightarrow", (void*)1, onChangeThreadAffinityScheme);
	
    CreateButtonControl(ScrHandle, menuDescHdle, "accept", PrevScrHandle, storeSimuCfg);
    CreateButtonControl(ScrHandle, menuDescHdle, "cancel", PrevScrHandle, GfuiScreenActivate);


    GfParmReleaseHandle(menuDescHdle);
    
    GfuiAddKey(ScrHandle, GFUIK_RETURN, "Save", NULL, storeSimuCfg, NULL);
    GfuiAddKey(ScrHandle, GFUIK_ESCAPE, "Cancel Selection", PrevScrHandle, GfuiScreenActivate, NULL);
    GfuiAddKey(ScrHandle, GFUIK_F1, "Help", ScrHandle, GfuiHelpScreen, NULL);
    GfuiAddKey(ScrHandle, GFUIK_F12, "Screen-Shot", NULL, GfuiScreenShot, NULL);
    GfuiAddKey(ScrHandle, GFUIK_LEFT, "Previous simu engine version", (void*)-1, onChangeSimuVersion, NULL);
    GfuiAddKey(ScrHandle, GFUIK_RIGHT, "Next simu engine version", (void*)1, onChangeSimuVersion, NULL);
    GfuiAddKey(ScrHandle, GFUIK_UP, "Previous multi-threading scheme", (void*)-1, onChangeMultiThreadScheme, NULL);
    GfuiAddKey(ScrHandle, GFUIK_DOWN, "Next multi-threading scheme", (void*)1, onChangeMultiThreadScheme, NULL);

    return ScrHandle;  
}
