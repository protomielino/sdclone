/***************************************************************************

    file                 : optionsmenu.cpp
    created              : Mon Apr 24 14:22:53 CEST 2000
    copyright            : (C) 2000, 2001 by Eric Espie
    email                : torcs@free.fr
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

#include <cstdio>

#include <tgfclient.h>

#include <displayconfig.h>
#include <graphconfig.h>
#include <openglconfig.h>
#include <soundconfig.h>
#include <simuconfig.h>
#include <aiconfig.h>

#include "optionsmenu.h"


static void *MenuHandle = NULL;


static void
onDisplayMenuActivate(void * /* dummy */)
{
    GfuiScreenActivate(DisplayMenuInit(MenuHandle));
}

static void
onGraphMenuActivate(void * /* dummy */)
{
    GfuiScreenActivate(GraphMenuInit(MenuHandle));
}

static void
onOpenGLMenuActivate(void * /* dummy */)
{
    GfuiScreenActivate(OpenGLMenuInit(MenuHandle));
}

static void
onSoundMenuActivate(void * /* dummy */)
{
    GfuiScreenActivate(SoundMenuInit(MenuHandle));
}

static void
onSimuMenuActivate(void * /* dummy */)
{
    GfuiScreenActivate(SimuMenuInit(MenuHandle));
}

static void
onAIMenuActivate(void * /* dummy */)
{
    GfuiScreenActivate(AIMenuInit(MenuHandle));
}

void *
OptionsMenuInit(void *prevMenu)
{
    if (MenuHandle) 
		return MenuHandle;

    MenuHandle = GfuiScreenCreateEx((float*)NULL, NULL, NULL, NULL, (tfuiCallback)NULL, 1);

    void *param = LoadMenuXML("optionsmenu.xml");

    CreateStaticControls(param,MenuHandle);
    
    CreateButtonControl(MenuHandle, param, "display", NULL, onDisplayMenuActivate);
    CreateButtonControl(MenuHandle, param, "graphic", NULL, onGraphMenuActivate);
    CreateButtonControl(MenuHandle, param, "opengl", NULL, onOpenGLMenuActivate);
    CreateButtonControl(MenuHandle, param, "sound", NULL, onSoundMenuActivate);
    CreateButtonControl(MenuHandle, param, "simulation", NULL, onSimuMenuActivate);
    CreateButtonControl(MenuHandle, param, "ai", NULL, onAIMenuActivate);
    CreateButtonControl(MenuHandle, param, "back", prevMenu, GfuiScreenActivate);

    GfParmReleaseHandle(param);

    GfuiMenuDefaultKeysAdd(MenuHandle);
    GfuiAddKey(MenuHandle, GFUIK_ESCAPE, "Back", prevMenu, GfuiScreenActivate, NULL);

    return MenuHandle;
}
