/***************************************************************************

    file                 : exitmenu.cpp
    created              : Sat Mar 18 23:42:12 CET 2000
    copyright            : (C) 2000 by Eric Espie
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

#include <tgfclient.h>

#include "legacymenu.h"

#include "exitmenu.h"
#include "mainmenu.h"


static void *exitmenuHandle = NULL;
static void *exitMainMenuHandle = NULL;

static void 
onAcceptExit(void * /* dummy */)
{
	LegacyMenu::self().quit();
}

void* exitMenuInit(void *prevMenu, void *menuHandle)
{
    if (menuHandle) {
		GfuiScreenRelease(menuHandle);
    }

    menuHandle = GfuiScreenCreate();

    void *param = GfuiMenuLoad("exitmenu.xml");

    GfuiMenuCreateStaticControls(menuHandle, param);
    GfuiMenuCreateButtonControl(menuHandle, param, "yesquit", NULL, onAcceptExit);
    GfuiMenuCreateButtonControl(menuHandle, param, "nobacktogame", prevMenu, GfuiScreenActivate);

    GfParmReleaseHandle(param);
    
    GfuiMenuDefaultKeysAdd(menuHandle);
    GfuiAddKey(menuHandle, GFUIK_RETURN, "Yes, quit the game", NULL, onAcceptExit, NULL);
    GfuiAddKey(menuHandle, GFUIK_ESCAPE, "No, back to the game", prevMenu, GfuiScreenActivate, NULL);

    return menuHandle;
}

/*
 * Function
 *	ExitMenuInit
 *
 * Description
 *	init the exit menus
 *
 * Parameters
 *	none
 *
 * Return
 *	0 ok -1 nok
 *
 * Remarks
 *	
 */
void* ExitMenuInit(void *menu)
{
	exitmenuHandle = exitMenuInit(menu, exitmenuHandle);
	return exitmenuHandle;
}


void* MainExitMenuInit(void *mainMenu)
{
	exitMainMenuHandle = exitMenuInit(mainMenu, exitMainMenuHandle);
	return exitMainMenuHandle;
}
