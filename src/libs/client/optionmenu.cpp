/***************************************************************************

    file                 : optionmenu.cpp
    created              : Mon Apr 24 14:22:53 CEST 2000
    copyright            : (C) 2000, 2001 by Eric Espie
    email                : torcs@free.fr
    version              : $Id: optionmenu.cpp,v 1.5 2005/06/03 23:51:19 berniw Exp $

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <stdio.h>
#include <tgfclient.h>
#include "optionmenu.h"
#include <graphconfig.h>
#include <simuconfig.h>
#include <soundconfig.h>
#include <openglconfig.h>

static void *optionHandle = NULL;

void *
OptionOptionInit(void *prevMenu)
{
    if (optionHandle) 
	return optionHandle;

    optionHandle = GfuiScreenCreateEx((float*)NULL, 
					    NULL, NULL, 
					    NULL, (tfuiCallback)NULL, 
					    1);

    void *param = LoadMenuXML("optionsmenu.xml");

    CreateStaticControls(param,optionHandle);
    
    CreateButtonControl(optionHandle,param,"graphic",GraphMenuInit(optionHandle),GfuiScreenActivate);
    CreateButtonControl(optionHandle,param,"display",GfScrMenuInit(optionHandle),GfuiScreenActivate);
    CreateButtonControl(optionHandle,param,"simulation",SimuMenuInit(optionHandle),GfuiScreenActivate);
    CreateButtonControl(optionHandle,param,"sound",SoundMenuInit(optionHandle),GfuiScreenActivate);
    CreateButtonControl(optionHandle,param,"opengl",OpenGLMenuInit(optionHandle),GfuiScreenActivate);
    CreateButtonControl(optionHandle,param,"back",prevMenu,GfuiScreenActivate);

    GfParmReleaseHandle(param);

    GfuiMenuDefaultKeysAdd(optionHandle);
    GfuiAddKey(optionHandle, 27, "Back", prevMenu, GfuiScreenActivate, NULL);

    return optionHandle;
}
