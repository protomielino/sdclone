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
static const int CloudLayersValues[] = {1, 2, 3};
static const int NbCloudLayersValues = sizeof(CloudLayersValues) / sizeof(CloudLayersValues[0]);
static const char* BackgroundSkyValues[] = { GR_ATT_BGSKY_DISABLED, GR_ATT_BGSKY_ENABLED };
static const int NbBackgroundSkyValues = sizeof(BackgroundSkyValues) / sizeof(BackgroundSkyValues[0]);

static void	*ScrHandle = NULL;

static int	FovEditId;
static int	SmokeEditId;
static int	SkidEditId;
static int	LodFactorEditId;
static int	SkyDomeDistLabelId;
static int	DynamicSkyDomeLabelId, DynamicSkyDomeLeftButtonId, DynamicSkyDomeRightButtonId;
static int	PrecipDensityLabelId;
static int	CloudLayerLabelId;
static int	BackgroundSkyLabelId, BackgroundSkyLeftButtonId, BackgroundSkyRightButtonId;

static int	FovFactorValue = 100;
static int	SmokeValue = 300;
static int	SkidValue = 20;
static tdble	LodFactorValue = 1.0;
static int 	SkyDomeDistIndex = 0;
static int 	DynamicSkyDomeIndex = 0;
static int 	PrecipDensityIndex = NbPrecipDensityValues - 1;
static int	CloudLayerIndex = 0;
static int	BackgroundSkyIndex = 0;

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
    GfParmSetNum(grHandle, GR_SCT_GRAPHIC, GR_ATT_CLOUDLAYER, NULL, CloudLayersValues[CloudLayerIndex]);
    GfParmSetStr(grHandle, GR_SCT_GRAPHIC, GR_ATT_BGSKY, BackgroundSkyValues[BackgroundSkyIndex]);
    
    GfParmWriteFile(NULL, grHandle, "graph");
    
    GfParmReleaseHandle(grHandle);
    
    ExitGraphicOptions(prevMenu);
}

static void
ChangeSkyDomeDist(void* vp);

static void
ChangeBackgroundSky(void* vp);

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
		(int)(GfParmGetNum(grHandle, GR_SCT_GRAPHIC, GR_ATT_SKYDOMEDISTANCE, NULL, 0) + 0.5);
    for (int i = 0; i < NbSkyDomeDistValues; i++) 
    {
		if (nSkyDomeDist <= SkyDomeDistValues[i]) 
		{
			SkyDomeDistIndex = i;
			break;
		}
    }
    snprintf(buf, sizeof(buf), "%d", SkyDomeDistValues[SkyDomeDistIndex]);
    GfuiLabelSetText(ScrHandle, SkyDomeDistLabelId, buf);

	if (nSkyDomeDist > 0)
	{
		// Enable controls for Dynamic Skydome and Background
		GfuiEnable(ScrHandle, DynamicSkyDomeLeftButtonId, GFUI_ENABLE);
		GfuiEnable(ScrHandle, DynamicSkyDomeRightButtonId, GFUI_ENABLE);
		GfuiEnable(ScrHandle, BackgroundSkyLeftButtonId, GFUI_ENABLE);
		GfuiEnable(ScrHandle, BackgroundSkyRightButtonId, GFUI_ENABLE);

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

		BackgroundSkyIndex = 0; // Default value index, in case file value not found in list.
		const char* pszBackgroundSky =
			GfParmGetStr(grHandle, GR_SCT_GRAPHIC, GR_ATT_BGSKY, GR_ATT_BGSKY_DISABLED);
		for (int i = 0; i < NbBackgroundSkyValues; i++) 
		{
			if (!strcmp(pszBackgroundSky, BackgroundSkyValues[i]))
			{
				BackgroundSkyIndex = i;
				break;
			}
		}
		GfuiLabelSetText(ScrHandle, BackgroundSkyLabelId, BackgroundSkyValues[BackgroundSkyIndex]);


		// FOV not taken into account when sky dome enabled.
		GfuiEnable(ScrHandle, FovEditId, GFUI_DISABLE);
	}
	else
	{
		// No dynamic time if no sky dome
		ChangeSkyDomeDist(0);
		ChangeBackgroundSky(0);

		// Disable controls for Dynamic Skydome and Background
		GfuiEnable(ScrHandle, DynamicSkyDomeLeftButtonId, GFUI_DISABLE);
		GfuiEnable(ScrHandle, DynamicSkyDomeRightButtonId, GFUI_DISABLE);
		GfuiEnable(ScrHandle, BackgroundSkyLeftButtonId, GFUI_DISABLE);
		GfuiEnable(ScrHandle, BackgroundSkyRightButtonId, GFUI_DISABLE);
	}

    PrecipDensityIndex = NbPrecipDensityValues - 1; // Default value index, in case file value not found in list.
    const int nPrecipDensity =
		(int)(GfParmGetNum(grHandle, GR_SCT_GRAPHIC, GR_ATT_PRECIPDENSITY, "%", 100) + 0.5f);
    for (int i = 0; i < NbPrecipDensityValues; i++) 
    {
		if (nPrecipDensity <= PrecipDensityValues[i]) 
		{
			PrecipDensityIndex = i;
			break;
		}
    }
    snprintf(buf, sizeof(buf), "%d", PrecipDensityValues[PrecipDensityIndex]);
    GfuiLabelSetText(ScrHandle, PrecipDensityLabelId, buf);

    const int nCloudLayer =
		(int)(GfParmGetNum(grHandle, GR_SCT_GRAPHIC, GR_ATT_CLOUDLAYER, NULL, 1) + 0.5f);
    for (int i = 0; i < NbCloudLayersValues; i++) 
    {
		if (nCloudLayer <= CloudLayersValues[i]) 
		{
			CloudLayerIndex = i;
			break;
		}
    }
    snprintf(buf, sizeof(buf), "%d", CloudLayersValues[CloudLayerIndex]);
    GfuiLabelSetText(ScrHandle, CloudLayerLabelId, buf);

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
ChangeBackgroundSky(void* vp)
{
    const long delta = (long)vp;
    BackgroundSkyIndex = (BackgroundSkyIndex + NbBackgroundSkyValues + delta) % NbBackgroundSkyValues;
    GfuiLabelSetText(ScrHandle, BackgroundSkyLabelId, BackgroundSkyValues[BackgroundSkyIndex]);
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
		ChangeBackgroundSky(0);

		// Make it clear that it is
		GfuiEnable(ScrHandle, DynamicSkyDomeLeftButtonId, GFUI_DISABLE);
		GfuiEnable(ScrHandle, DynamicSkyDomeRightButtonId, GFUI_DISABLE);

		// Make it clear that it is
		GfuiEnable(ScrHandle, BackgroundSkyLeftButtonId, GFUI_DISABLE);
		GfuiEnable(ScrHandle, BackgroundSkyRightButtonId, GFUI_DISABLE);

		// Enable FOV editbox
		GfuiEnable(ScrHandle, FovEditId, GFUI_ENABLE);
	}
	else
	{
		// Enable changes of dynamic time of day
		GfuiEnable(ScrHandle, DynamicSkyDomeLeftButtonId, GFUI_ENABLE);
		GfuiEnable(ScrHandle, DynamicSkyDomeRightButtonId, GFUI_ENABLE);

		GfuiEnable(ScrHandle, BackgroundSkyLeftButtonId, GFUI_ENABLE);
		GfuiEnable(ScrHandle, BackgroundSkyRightButtonId, GFUI_ENABLE);

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

static void
ChangeCloudLayer(void* vp)
{
    const long delta = (long)vp;
    CloudLayerIndex = (CloudLayerIndex + NbCloudLayersValues + delta) % NbCloudLayersValues;
    snprintf(buf, sizeof(buf), "%d", CloudLayersValues[CloudLayerIndex]);
    GfuiLabelSetText(ScrHandle, CloudLayerLabelId, buf);
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

	GfuiMenuCreateButtonControl(ScrHandle, param, "cloudlayernbleftarrow", (void*)-1, ChangeCloudLayer);
    GfuiMenuCreateButtonControl(ScrHandle, param, "cloudlayernbrightarrow", (void*)1, ChangeCloudLayer);
    CloudLayerLabelId = GfuiMenuCreateLabelControl(ScrHandle, param, "cloudlayerlabel");

	BackgroundSkyLeftButtonId =
		GfuiMenuCreateButtonControl(ScrHandle, param, "bgskyleftarrow", (void*)-1, ChangeBackgroundSky);
    BackgroundSkyRightButtonId =
		GfuiMenuCreateButtonControl(ScrHandle, param, "bgskyrightarrow", (void*)1, ChangeBackgroundSky);
    BackgroundSkyLabelId = GfuiMenuCreateLabelControl(ScrHandle, param, "bgskydomelabel");

    GfuiMenuCreateButtonControl(ScrHandle, param, "accept", prevMenu, SaveGraphicOptions);
    GfuiMenuCreateButtonControl(ScrHandle, param, "cancel", prevMenu, GfuiScreenActivate);
    
    GfParmReleaseHandle(param);
    
    GfuiAddKey(ScrHandle, GFUIK_RETURN, "Save", prevMenu, SaveGraphicOptions, NULL);
    GfuiAddKey(ScrHandle, GFUIK_ESCAPE, "Cancel", prevMenu, GfuiScreenActivate, NULL);
    GfuiAddKey(ScrHandle, GFUIK_F1, "Help", ScrHandle, GfuiHelpScreen, NULL);
    GfuiAddKey(ScrHandle, GFUIK_F12, "Screen-Shot", NULL, GfuiScreenShot, NULL);

    return ScrHandle;
}
