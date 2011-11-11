/***************************************************************************

    file                 : graphconfig.cpp
    created              : Sun Jun  9 16:30:25 CEST 2002
    copyright            : (C) 2001 by Eric Espie
    email                : eric.espie@torcs.org
    version              : $Id$

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

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <portability.h>
#include <tgfclient.h>
#include <graphic.h>

#include "graphconfig.h"

static const int SkyDomeDistValues[] = {0, 12000, 20000, 40000, 80000};
static const int NbSkyDomeDistValues = sizeof(SkyDomeDistValues) / sizeof(SkyDomeDistValues[0]);

static const char* DynamicSkyDomeValues[] =
	{ GR_ATT_DYNAMICSKYDOME_DISABLED, GR_ATT_DYNAMICSKYDOME_ENABLED };
static const int NbDynamicSkyDomeValues = sizeof(DynamicSkyDomeValues) / sizeof(DynamicSkyDomeValues[0]);
static const int PrecipDensityValues[] = {0, 20, 40, 60, 80, 100};
static const int NbPrecipDensityValues = sizeof(PrecipDensityValues) / sizeof(PrecipDensityValues[0]);

static void	*ScrHandle = NULL;

static int	FovEditId;
static int	SmokeEditId;
static int	SkidEditId;
static int	LodFactorEditId;
static int	SkyDomeDistLabelId;
static int	DynamicSkyDomeLabelId, DynamicSkyDomeLeftButtonId, DynamicSkyDomeRightButtonId;
static int	PrecipDensityLabelId;

static int	FovFactorValue = 100;
static int	SmokeValue = 300;
static int	SkidValue = 20;
static tdble	LodFactorValue = 1.0;
static int 	SkyDomeDistIndex = 0;
static int 	DynamicSkyDomeIndex = 0;
static int 	PrecipDensityIndex = NbPrecipDensityValues - 1;

static char	buf[512];


static void
ExitGraphicOptions(void *prevMenu)
{
    GfuiScreenActivate(prevMenu);
}

static void
SaveGraphicOptions(void *prevMenu)
{
    // Force current edit to loose focus (if one has it) and update associated variable.
    GfuiUnSelectCurrent();

    snprintf(buf, sizeof(buf), "%s%s", GfLocalDir(), GR_PARAM_FILE);
    void* grHandle = GfParmReadFile(buf, GFPARM_RMODE_STD | GFPARM_RMODE_CREAT);
    
    GfParmSetNum(grHandle, GR_SCT_GRAPHIC, GR_ATT_FOVFACT, "%", FovFactorValue);
    GfParmSetNum(grHandle, GR_SCT_GRAPHIC, GR_ATT_SMOKENB, NULL, SmokeValue);
    GfParmSetNum(grHandle, GR_SCT_GRAPHIC, GR_ATT_MAXSTRIPBYWHEEL, NULL, SkidValue);
    GfParmSetNum(grHandle, GR_SCT_GRAPHIC, GR_ATT_LODFACTOR, NULL, LodFactorValue);
    GfParmSetNum(grHandle, GR_SCT_GRAPHIC, GR_ATT_SKYDOMEDISTANCE, NULL, SkyDomeDistValues[SkyDomeDistIndex]);
    GfParmSetStr(grHandle, GR_SCT_GRAPHIC, GR_ATT_DYNAMICSKYDOME, DynamicSkyDomeValues[DynamicSkyDomeIndex]);
    GfParmSetNum(grHandle, GR_SCT_GRAPHIC, GR_ATT_PRECIPDENSITY, "%", PrecipDensityValues[PrecipDensityIndex]);
    
    GfParmWriteFile(NULL, grHandle, "graph");
    
    GfParmReleaseHandle(grHandle);
    
    ExitGraphicOptions(prevMenu);
}

static void
ChangeSkyDomeDist(void* vp);

static void
LoadGraphicOptions()
{
    snprintf(buf, sizeof(buf), "%s%s", GfLocalDir(), GR_PARAM_FILE);
    void* grHandle = GfParmReadFile(buf, GFPARM_RMODE_STD | GFPARM_RMODE_CREAT);
    
    FovFactorValue = (int)GfParmGetNum(grHandle, GR_SCT_GRAPHIC, GR_ATT_FOVFACT, "%", 100.0);
    snprintf(buf, sizeof(buf), "%d", FovFactorValue);
    GfuiEditboxSetString(ScrHandle, FovEditId, buf);
      
    SmokeValue = (int)GfParmGetNum(grHandle, GR_SCT_GRAPHIC, GR_ATT_SMOKENB, NULL, 300.0);
    snprintf(buf, sizeof(buf), "%d", SmokeValue);
    GfuiEditboxSetString(ScrHandle, SmokeEditId, buf);

    SkidValue = (int)GfParmGetNum(grHandle, GR_SCT_GRAPHIC, GR_ATT_MAXSTRIPBYWHEEL, NULL, 20.0);
    snprintf(buf, sizeof(buf), "%d", SkidValue);
    GfuiEditboxSetString(ScrHandle, SkidEditId, buf);

    LodFactorValue = GfParmGetNum(grHandle, GR_SCT_GRAPHIC, GR_ATT_LODFACTOR, NULL, 1.0);
    snprintf(buf, sizeof(buf), "%g", LodFactorValue);
    GfuiEditboxSetString(ScrHandle, LodFactorEditId, buf);

    SkyDomeDistIndex = 0; // Default value index, in case file value not found in list.
    const int nSkyDomeDist =
		(int)GfParmGetNum(grHandle, GR_SCT_GRAPHIC, GR_ATT_SKYDOMEDISTANCE, NULL, 0);
    for (int i = 0; i < NbSkyDomeDistValues; i++) 
    {
		if (nSkyDomeDist == SkyDomeDistValues[i]) 
		{
			SkyDomeDistIndex = i;
			break;
		}
    }
    snprintf(buf, sizeof(buf), "%d", SkyDomeDistValues[SkyDomeDistIndex]);
    GfuiLabelSetText(ScrHandle, SkyDomeDistLabelId, buf);

	if (nSkyDomeDist > 0)
	{
		DynamicSkyDomeIndex = 0; // Default value index, in case file value not found in list.
		const char* pszDynamicSkyDome =
			GfParmGetStr(grHandle, GR_SCT_GRAPHIC, GR_ATT_DYNAMICSKYDOME, GR_ATT_DYNAMICSKYDOME_DISABLED);
		for (int i = 0; i < NbDynamicSkyDomeValues; i++) 
		{
			if (!strcmp(pszDynamicSkyDome, DynamicSkyDomeValues[i]))
			{
				DynamicSkyDomeIndex = i;
				break;
			}
		}
		GfuiLabelSetText(ScrHandle, DynamicSkyDomeLabelId, DynamicSkyDomeValues[DynamicSkyDomeIndex]);

		// FOV not taken into account when sky dome enabled.
		GfuiEnable(ScrHandle, FovEditId, GFUI_DISABLE);
	}
	else
	{
		// No dynamic time if no sky dome
		ChangeSkyDomeDist(0);
	}

    PrecipDensityIndex = NbPrecipDensityValues - 1; // Default value index, in case file value not found in list.
    const int nPrecipDensity =
		(int)GfParmGetNum(grHandle, GR_SCT_GRAPHIC, GR_ATT_PRECIPDENSITY, "%", 100.0);
    for (int i = 0; i < NbPrecipDensityValues; i++) 
    {
		if (nPrecipDensity == PrecipDensityValues[i]) 
		{
			PrecipDensityIndex = i;
			break;
		}
    }
    snprintf(buf, sizeof(buf), "%d", PrecipDensityValues[PrecipDensityIndex]);
    GfuiLabelSetText(ScrHandle, PrecipDensityLabelId, buf);

    GfParmReleaseHandle(grHandle);
}

static void
ChangeFov(void* /* dummy */)
{
    char	*val;

    val = GfuiEditboxGetString(ScrHandle, FovEditId);
    FovFactorValue = strtol(val, (char **)NULL, 0);
    snprintf(buf, sizeof(buf), "%d", FovFactorValue);
    GfuiEditboxSetString(ScrHandle, FovEditId, buf);
}

static void
ChangeLodFactor(void* /* dummy */)
{
    char	*val;

    val = GfuiEditboxGetString(ScrHandle, LodFactorEditId);
    sscanf(val, "%g", &LodFactorValue);
    snprintf(buf, sizeof(buf), "%g", LodFactorValue);
    GfuiEditboxSetString(ScrHandle, LodFactorEditId, buf);
}

static void
ChangeSmoke(void* /* dummy */)
{
    char	*val;

    val = GfuiEditboxGetString(ScrHandle, SmokeEditId);
    SmokeValue = strtol(val, (char **)NULL, 0);
    snprintf(buf, sizeof(buf), "%d", SmokeValue);
    GfuiEditboxSetString(ScrHandle, SmokeEditId, buf);
}

static void
ChangeSkid(void* /* dummy */)
{
    char	*val;

    val = GfuiEditboxGetString(ScrHandle, SkidEditId);
    SkidValue = strtol(val, (char **)NULL, 0);
    snprintf(buf, sizeof(buf), "%d", SkidValue);
    GfuiEditboxSetString(ScrHandle, SkidEditId, buf);
}

static void
ChangeDynamicSkyDome(void* vp)
{
    const long delta = (long)vp;
    DynamicSkyDomeIndex = (DynamicSkyDomeIndex + NbDynamicSkyDomeValues + delta) % NbDynamicSkyDomeValues;
    GfuiLabelSetText(ScrHandle, DynamicSkyDomeLabelId, DynamicSkyDomeValues[DynamicSkyDomeIndex]);
} 

static void
ChangeSkyDomeDist(void* vp)
{
    const long delta = (long)vp;
    SkyDomeDistIndex = (SkyDomeDistIndex + NbSkyDomeDistValues + delta) % NbSkyDomeDistValues;
    snprintf(buf, sizeof(buf), "%d", SkyDomeDistValues[SkyDomeDistIndex]);
    GfuiLabelSetText(ScrHandle, SkyDomeDistLabelId, buf);

	// If realistic sky dome not enabled :
	if (!SkyDomeDistValues[SkyDomeDistIndex])
	{
		// Disable dynamic time of day
		DynamicSkyDomeIndex = 0;
		ChangeDynamicSkyDome(0);

		// Make it clear that it is
		GfuiEnable(ScrHandle, DynamicSkyDomeLeftButtonId, GFUI_DISABLE);
		GfuiEnable(ScrHandle, DynamicSkyDomeRightButtonId, GFUI_DISABLE);

		// Enable FOV editbox
		GfuiEnable(ScrHandle, FovEditId, GFUI_ENABLE);
	}
	else
	{
		// Enable changes of dynamic time of day
		GfuiEnable(ScrHandle, DynamicSkyDomeLeftButtonId, GFUI_ENABLE);
		GfuiEnable(ScrHandle, DynamicSkyDomeRightButtonId, GFUI_ENABLE);

		// Enable FOV editbox
		GfuiEnable(ScrHandle, FovEditId, GFUI_DISABLE);
	}
} 

static void
ChangePrecipDensity(void* vp)
{
    const long delta = (long)vp;
    PrecipDensityIndex = (PrecipDensityIndex + NbPrecipDensityValues + delta) % NbPrecipDensityValues;
    snprintf(buf, sizeof(buf), "%d", PrecipDensityValues[PrecipDensityIndex]);
    GfuiLabelSetText(ScrHandle, PrecipDensityLabelId, buf);
}

static void onActivate(void* /* dummy */)
{
    LoadGraphicOptions();
}

void*
GraphMenuInit(void* prevMenu)
{
    /* screen already created */
    if (ScrHandle) {
		return ScrHandle;
    }

    ScrHandle = GfuiScreenCreate((float*)NULL, NULL, onActivate, NULL, (tfuiCallback)NULL, 1);

    void* param = GfuiMenuLoad("graphicconfig.xml");

    GfuiMenuCreateStaticControls(ScrHandle, param);

    FovEditId = GfuiMenuCreateEditControl(ScrHandle, param, "fovedit", NULL, NULL, ChangeFov);
    SmokeEditId = GfuiMenuCreateEditControl(ScrHandle, param, "smokeedit", NULL, NULL, ChangeSmoke);
    SkidEditId = GfuiMenuCreateEditControl(ScrHandle, param, "skidedit", NULL, NULL, ChangeSkid);
    LodFactorEditId = GfuiMenuCreateEditControl(ScrHandle, param, "lodedit", NULL, NULL, ChangeLodFactor);

    GfuiMenuCreateButtonControl(ScrHandle, param, "skydomedistleftarrow", (void*)-1, ChangeSkyDomeDist);
    GfuiMenuCreateButtonControl(ScrHandle, param, "skydomedistrightarrow", (void*)1, ChangeSkyDomeDist);
    SkyDomeDistLabelId = GfuiMenuCreateLabelControl(ScrHandle, param, "skydomedistlabel");
    
    DynamicSkyDomeLeftButtonId =
		GfuiMenuCreateButtonControl(ScrHandle, param, "dynamicskydomeleftarrow", (void*)-1, ChangeDynamicSkyDome);
    DynamicSkyDomeRightButtonId =
		GfuiMenuCreateButtonControl(ScrHandle, param, "dynamicskydomerightarrow", (void*)1, ChangeDynamicSkyDome);
    DynamicSkyDomeLabelId = GfuiMenuCreateLabelControl(ScrHandle, param, "dynamicskydomelabel");
    
    GfuiMenuCreateButtonControl(ScrHandle, param, "precipdensityleftarrow", (void*)-1, ChangePrecipDensity);
    GfuiMenuCreateButtonControl(ScrHandle, param, "precipdensityrightarrow", (void*)1, ChangePrecipDensity);
    PrecipDensityLabelId = GfuiMenuCreateLabelControl(ScrHandle, param, "precipdensitylabel");

    GfuiMenuCreateButtonControl(ScrHandle, param, "accept", prevMenu, SaveGraphicOptions);
    GfuiMenuCreateButtonControl(ScrHandle, param, "cancel", prevMenu, GfuiScreenActivate);
    
    GfParmReleaseHandle(param);
    
    GfuiAddKey(ScrHandle, GFUIK_RETURN, "Save", prevMenu, SaveGraphicOptions, NULL);
    GfuiAddKey(ScrHandle, GFUIK_ESCAPE, "Cancel", prevMenu, GfuiScreenActivate, NULL);
    GfuiAddKey(ScrHandle, GFUIK_F1, "Help", ScrHandle, GfuiHelpScreen, NULL);
    GfuiAddKey(ScrHandle, GFUIK_F12, "Screen-Shot", NULL, GfuiScreenShot, NULL);

    return ScrHandle;
}
