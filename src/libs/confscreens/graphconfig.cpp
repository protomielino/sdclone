/***************************************************************************

    file                 : graphconfig.cpp
    created              : Sun Jun  9 16:30:25 CEST 2002
    copyright            : (C) 2001 by Eric Espie
    email                : eric.espie@torcs.org
    version              : $Id: graphconfig.cpp,v 1.5 2005/07/21 21:27:14 berniw Exp $

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
    @version	$Id: graphconfig.cpp,v 1.5 2005/07/21 21:27:14 berniw Exp $
*/

#include <stdio.h>
#include <stdlib.h>
#include <cstring>


#include <tgfclient.h>
#include <graphic.h>

#include "graphconfig.h"
#include "gui.h"

static void	*scrHandle = NULL;
static char	buf[1024];
//static char	valuebuf[10];

static int	FovEditId;
static int	FovFactorValue = 100;
static int	SmokeEditId;
static int	SmokeValue = 300;
static int	SkidEditId;
static int	SkidValue = 20;
static int	LodFactorEditId;
static tdble	LodFactorValue = 1.0;
static int	SkyDomeOptionId;
static int 	SkyDomeValueIndex = 0;
static int	DynamicTimeOptionId;
static int	DynamicTimeOptionValue = 0;


//static int	RainDensityOptionId;
//static int	RainDensityValueIndex = 0;

static const int SkyDomeSizeValues[] = {0, 12000, 20000, 40000, 80000};
static const int NbSkyDomeValues = sizeof(SkyDomeSizeValues) / sizeof(SkyDomeSizeValues[0]);
//static const int RainDensityValues[] = {0, 100, 200, 300};
//static const int NbRainDensityValues = sizeof(RainDensityValues) / sizeof(RainDensityValues[0]);
static const char *DynamicTimeValues[] = {"Disable", "Enable"};
static const int NbDynamicTimeValues = sizeof(DynamicTimeValues) / sizeof(DynamicTimeValues[0]); 

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

    sprintf(buf, "%s%s", GetLocalDir(), GR_PARAM_FILE);
    void * grHandle = GfParmReadFile(buf, GFPARM_RMODE_STD | GFPARM_RMODE_CREAT);
    
    GfParmSetNum(grHandle, GR_SCT_GRAPHIC, GR_ATT_FOVFACT, "%", FovFactorValue);
    GfParmSetNum(grHandle, GR_SCT_GRAPHIC, GR_ATT_SMOKENB, NULL, SmokeValue);
    GfParmSetNum(grHandle, GR_SCT_GRAPHIC, GR_ATT_MAXSTRIPBYWHEEL, NULL, SkidValue);
    GfParmSetNum(grHandle, GR_SCT_GRAPHIC, GR_ATT_LODFACTOR, NULL, LodFactorValue);
    GfParmSetNum(grHandle, GR_SCT_GRAPHIC, GR_ATT_SKYDOME, NULL, SkyDomeSizeValues[SkyDomeValueIndex]);
    //GfParmSetNum(grHandle, GR_SCT_GRAPHIC, GR_ATT_RAINDENSITY, NULL, RainDensityValue);
    GfParmSetNum(grHandle, GR_SCT_GRAPHIC, GR_ATT_DYNAMICTIME, NULL, DynamicTimeOptionValue);
    GfParmSetNum(grHandle, GR_SCT_GRAPHIC, GR_ATT_RAINBOOL, NULL, 0);
    
    GfParmWriteFile(NULL, grHandle, "graph");
    
    GfParmReleaseHandle(grHandle);
    
    ExitGraphicOptions(prevMenu);
}

static void
LoadGraphicOptions()
{
    sprintf(buf, "%s%s", GetLocalDir(), GR_PARAM_FILE);
    void * grHandle = GfParmReadFile(buf, GFPARM_RMODE_STD | GFPARM_RMODE_CREAT);
    
    FovFactorValue = (int)GfParmGetNum(grHandle, GR_SCT_GRAPHIC, GR_ATT_FOVFACT, "%", 100.0);
    sprintf(buf, "%d", FovFactorValue);
    GfuiEditboxSetString(scrHandle,FovEditId,buf);
      
    SmokeValue = (int)GfParmGetNum(grHandle, GR_SCT_GRAPHIC, GR_ATT_SMOKENB, NULL, 300.0);
    sprintf(buf, "%d", SmokeValue);
    GfuiEditboxSetString(scrHandle,SmokeEditId,buf);

    SkidValue = (int)GfParmGetNum(grHandle, GR_SCT_GRAPHIC, GR_ATT_MAXSTRIPBYWHEEL, NULL, 20.0);
    sprintf(buf, "%d", SkidValue);
    GfuiEditboxSetString(scrHandle,SkidEditId,buf);

    LodFactorValue = GfParmGetNum(grHandle, GR_SCT_GRAPHIC, GR_ATT_LODFACTOR, NULL, 1.0);
    sprintf(buf, "%g", LodFactorValue);
    GfuiEditboxSetString(scrHandle,LodFactorEditId,buf);

    SkyDomeValueIndex = 0; // Default value index, In case file value not found in list.
    const int nSkyDomeSize = (int)GfParmGetNum(grHandle, GR_SCT_GRAPHIC, GR_ATT_SKYDOME, NULL, 0);
    for (int i = 0; i < NbSkyDomeValues; i++) 
    {
	if (nSkyDomeSize == SkyDomeSizeValues[i]) 
	{
	    SkyDomeValueIndex = i;
	    break;
	}
    }
    sprintf(buf, "%d", SkyDomeSizeValues[SkyDomeValueIndex]); // In case value not found in list.
    GfuiLabelSetText(scrHandle,SkyDomeOptionId,buf);

    //RainDensityValue = (int)GfParmGetNum(grHandle, GR_SCT_GRAPHIC, GR_ATT_RAINDENSITY, NULL, 0.0);
    //sprintf(buf, "%d", RainDensityValue);
    //GfuiEditboxSetString(scrHandle,RainEditId,buf);
    DynamicTimeOptionValue = GfParmGetNum(grHandle, GR_SCT_GRAPHIC, GR_ATT_DYNAMICTIME, NULL, 0);
    sprintf(buf, "%d", DynamicTimeValues[DynamicTimeOptionValue]);
    GfuiLabelSetText(scrHandle, DynamicTimeOptionId, DynamicTimeValues[DynamicTimeOptionValue]);

    if (SkyDomeValueIndex == 0)
    {
    	GfuiVisibilitySet(scrHandle,DynamicTimeOptionId, 0);
    	DynamicTimeOptionValue = 0;
    }

    GfParmReleaseHandle(grHandle);
}

static void
ChangeFov(void * /* dummy */)
{
    char	*val;

    val = GfuiEditboxGetString(scrHandle, FovEditId);
    FovFactorValue = strtol(val, (char **)NULL, 0);
    sprintf(buf, "%d", FovFactorValue);
    GfuiEditboxSetString(scrHandle, FovEditId, buf);
}

static void
ChangeLodFactor(void * /* dummy */)
{
    char	*val;

    val = GfuiEditboxGetString(scrHandle, LodFactorEditId);
    sscanf(val, "%g", &LodFactorValue);
    sprintf(buf, "%g", LodFactorValue);
    GfuiEditboxSetString(scrHandle, LodFactorEditId, buf);
}

static void
ChangeSmoke(void * /* dummy */)
{
    char	*val;

    val = GfuiEditboxGetString(scrHandle, SmokeEditId);
    SmokeValue = strtol(val, (char **)NULL, 0);
    sprintf(buf, "%d", SmokeValue);
    GfuiEditboxSetString(scrHandle, SmokeEditId, buf);
}

static void
ChangeSkid(void * /* dummy */)
{
    char	*val;

    val = GfuiEditboxGetString(scrHandle, SkidEditId);
    SkidValue = strtol(val, (char **)NULL, 0);
    sprintf(buf, "%d", SkidValue);
    GfuiEditboxSetString(scrHandle, SkidEditId, buf);
}

static void
ChangeSkyDome(void *vp)
{
    const long delta = (long)vp;
    SkyDomeValueIndex = (SkyDomeValueIndex + NbSkyDomeValues + delta) % NbSkyDomeValues;
    sprintf(buf, "%d", SkyDomeSizeValues[SkyDomeValueIndex]);
    GfuiLabelSetText(scrHandle, SkyDomeOptionId, buf);

    if (SkyDomeValueIndex == 0)
    {
    GfuiVisibilitySet(scrHandle,DynamicTimeOptionId, 0);
    DynamicTimeOptionValue = 0;
    //ChangeDynamicTime(0);
    }
    else
    {
    GfuiVisibilitySet(scrHandle,DynamicTimeOptionId, 1);
    }
} 

/*static void
ChangeRain(void * dummy)
{
    char	*val;

    val = GfuiEditboxGetString(scrHandle, RainEditId);
    RainDensityValue = strtol(val, (char **)NULL, 0);
    sprintf(buf, "%d", RainDensityValue);
    GfuiEditboxSetString(scrHandle, RainEditId, buf);
}*/

static void
ChangeDynamicTime(void *vp)
{
    const long delta = (long)vp;
    DynamicTimeOptionValue = (DynamicTimeOptionValue + NbDynamicTimeValues + delta) % NbDynamicTimeValues;
    GfuiLabelSetText(scrHandle, DynamicTimeOptionId, DynamicTimeValues[DynamicTimeOptionValue]);
} 

static void onActivate(void * /* dummy */)
{
    LoadGraphicOptions();
}

void *
GraphMenuInit(void *prevMenu)
{
    /* screen already created */
    if (scrHandle) {
	return scrHandle;
    }

    scrHandle = GfuiScreenCreateEx((float*)NULL, NULL, onActivate, NULL, (tfuiCallback)NULL, 1);

    void *param = LoadMenuXML("graphicconfig.xml");

    CreateStaticControls(param,scrHandle);

    FovEditId = CreateEditControl(scrHandle,param,"fovedit",NULL,NULL,ChangeFov);
    SmokeEditId = CreateEditControl(scrHandle,param,"smokeedit",NULL,NULL,ChangeSmoke);
    SkidEditId = CreateEditControl(scrHandle,param,"skidedit",NULL,NULL,ChangeSkid);
    LodFactorEditId = CreateEditControl(scrHandle,param,"lodedit",NULL,NULL,ChangeLodFactor);

    CreateButtonControl(scrHandle,param,"skydomeleftarrow",(void*)-1, ChangeSkyDome);
    CreateButtonControl(scrHandle,param,"skydomerightarrow",(void*)1, ChangeSkyDome);
    SkyDomeOptionId = CreateLabelControl(scrHandle,param,"skydomelabel");
    
    if (SkyDomeValueIndex == 0)
    {
    	GfuiVisibilitySet(scrHandle,DynamicTimeOptionId, 0);
    	DynamicTimeOptionValue = 0;
    	ChangeDynamicTime(0);
    }
    else
    {
    	GfuiVisibilitySet(scrHandle,DynamicTimeOptionId, 1);
    }
    CreateButtonControl(scrHandle,param,"dynamictimeleftarrow",(void*)-1, ChangeDynamicTime);
    CreateButtonControl(scrHandle,param,"dynamictimerightarrow",(void*)1, ChangeDynamicTime);
    DynamicTimeOptionId = CreateLabelControl(scrHandle,param,"dynamictimelabel");

    //Keep
    //RainEditId = CreateEditControl(scrHandle,param,"rainedit",NULL,NULL,ChangeRain);

    CreateButtonControl(scrHandle, param, "accept", prevMenu, SaveGraphicOptions);
    CreateButtonControl(scrHandle, param, "cancel", prevMenu, GfuiScreenActivate);
    
    GfParmReleaseHandle(param);
    
    GfuiAddKey(scrHandle, GFUIK_RETURN, "Save", NULL, SaveGraphicOptions, NULL);
    GfuiAddKey(scrHandle, GFUIK_ESCAPE, "Cancel", prevMenu, GfuiScreenActivate, NULL);
    GfuiAddKey(scrHandle, GFUIK_F1, "Help", scrHandle, GfuiHelpScreen, NULL);
    GfuiAddKey(scrHandle, GFUIK_F12, "Screen-Shot", NULL, GfuiScreenShot, NULL);

    return scrHandle;
}
