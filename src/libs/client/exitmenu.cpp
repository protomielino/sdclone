/***************************************************************************

    file                 : exitmenu.cpp
    created              : Sat Mar 18 23:42:12 CET 2000
    copyright            : (C) 2000 by Eric Espie
    email                : torcs@free.fr
    version              : $Id: exitmenu.cpp,v 1.4 2006/10/05 21:25:55 berniw Exp $

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
#include "exitmenu.h"
#include "mainmenu.h"

static void 
endofprog(void * /* dummy */)
{
    STOP_ACTIVE_PROFILES();
    PRINT_PROFILE();
    GfScrShutdown();
    exit(0);
}

static void *exitmenuHandle = NULL;
static void *exitMainMenuHandle = NULL;


void * exitMenuInit(void *prevMenu, void *menuHandle)
{
    if (menuHandle) {
		GfuiScreenRelease(menuHandle);
    }

    menuHandle = GfuiScreenCreate();

    void *param = LoadMenuXML("quitscreen.xml");

    CreateStaticControls(param,menuHandle);
    CreateButtonControl(menuHandle,param,"yesquit",NULL,endofprog);
    CreateButtonControl(menuHandle,param,"nobacktogame",prevMenu,GfuiScreenActivate);

    GfParmReleaseHandle(param);
    
    GfuiMenuDefaultKeysAdd(menuHandle);
    GfuiAddKey(menuHandle, GFUIK_ESCAPE, "No, Back to Game", prevMenu, GfuiScreenActivate, NULL);

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
void * ExitMenuInit(void *menu)
{
	exitmenuHandle = exitMenuInit(menu, exitmenuHandle);
	return exitmenuHandle;
}


void * MainExitMenuInit(void *mainMenu)
{
	exitMainMenuHandle = exitMenuInit(mainMenu, exitMainMenuHandle);
	return exitMainMenuHandle;
}
