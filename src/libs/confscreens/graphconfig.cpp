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

static void	*scrHandle = NULL;
static char	buf[1024];
static int	FovEditId;
static int	FovFactorValue = 100;
static int	SmokeEditId;
static int	SmokeValue = 300;
static int	SkidEditId;
static int	SkidValue = 20;
static int	LodFactorEditId;
static tdble	LodFactorValue = 1.0;

static void
ExitGraphicOptions(void *prevMenu)
{
    GfuiScreenActivate(prevMenu);
}

static void
SaveGraphicOptions(void *prevMenu)
{
    sprintf(buf, "%s%s", GetLocalDir(), GR_PARAM_FILE);
    void * grHandle = GfParmReadFile(buf, GFPARM_RMODE_STD | GFPARM_RMODE_CREAT);
    
    GfParmSetNum(grHandle, GR_SCT_GRAPHIC, GR_ATT_FOVFACT, "%", FovFactorValue);
    GfParmSetNum(grHandle, GR_SCT_GRAPHIC, GR_ATT_SMOKENB, NULL, SmokeValue);
    GfParmSetNum(grHandle, GR_SCT_GRAPHIC, GR_ATT_MAXSTRIPBYWHEEL, NULL, SkidValue);
    GfParmSetNum(grHandle, GR_SCT_GRAPHIC, GR_ATT_LODFACTOR, NULL, LodFactorValue);
    //GfParmSetNum(grHandle, GR_SCT_GRAPHIC, GR_ATT_WHEEL3D, NULL, Wheel3dOtionId);
    //GfParmSetNum(grHandle, GR_SCT_GRAPHIC, GR_ATT_ANIMDRIVER, NULL, AnimDriverOptionId);
    
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
    
    CreateButtonControl(scrHandle, param, "accept", prevMenu, SaveGraphicOptions);
    CreateButtonControl(scrHandle, param, "cancel", prevMenu, GfuiScreenActivate);
    
    GfParmReleaseHandle(param);
    
    GfuiAddKey(scrHandle, 13, "Save", prevMenu, SaveGraphicOptions, NULL);
    GfuiAddKey(scrHandle, 27, "Cancel", prevMenu, GfuiScreenActivate, NULL);
    GfuiAddSKey(scrHandle, GLUT_KEY_F1, "Help", scrHandle, GfuiHelpScreen, NULL);
    GfuiAddSKey(scrHandle, GLUT_KEY_F12, "Screen-Shot", NULL, GfuiScreenShot, NULL);
  
    return scrHandle;
}
