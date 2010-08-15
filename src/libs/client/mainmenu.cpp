/***************************************************************************

    file                 : mainmenu.cpp
    created              : Sat Mar 18 23:42:38 CET 2000
    copyright            : (C) 2000 by Eric Espie
    email                : torcs@free.fr
    version              : $Id: mainmenu.cpp,v 1.4.2.1 2008/08/16 23:59:54 berniw Exp $

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
#include <singleplayer.h>
#include <racemain.h>
#include <playerconfig.h>

#include "mainmenu.h"
#include "exitmenu.h"
#include "optionmenu.h"
#include "creditsmenu.h"


void *MenuHandle = 0;

tModList *RacemanModLoaded = 0;

static void
PlayerConfigActivate(void * /* dummy */)
{
   /* Here, we need to call OptionOptionInit each time the firing button
       is pressed, and not only once at the Main menu initialization,
       because the previous menu has to be saved (ESC, Back) and because it can be this menu,
       as well as the Raceman menu */
    GfuiScreenActivate(PlayerConfigMenuInit(MenuHandle));
}

static void
MainMenuActivate(void * /* dummy */)
{
    if (RacemanModLoaded) {
	GfModUnloadList(&RacemanModLoaded);
    }
}

/*
 * Function
 *	MainMenuInit
 *
 * Description
 *	init the main menu
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

int
MainMenuInit(void)
{
    // Initialize only once.
    if (MenuHandle)
        return 0;

    MenuHandle = GfuiScreenCreateEx((float*)NULL, 
				    NULL, MainMenuActivate, 
				    NULL, (tfuiCallback)NULL, 
				    1);

    void *menuDescHdle = LoadMenuXML("mainmenu.xml");

    CreateStaticControls(menuDescHdle,MenuHandle);

    //Add buttons and create based on xml
    CreateButtonControl(MenuHandle, menuDescHdle, "race", ReSinglePlayerInit(MenuHandle), GfuiScreenActivate);
    CreateButtonControl(MenuHandle, menuDescHdle, "configure", NULL, PlayerConfigActivate);
    CreateButtonControl(MenuHandle, menuDescHdle, "options",OptionOptionInit(MenuHandle), GfuiScreenActivate);
    CreateButtonControl(MenuHandle, menuDescHdle, "credits", MenuHandle, CreditsScreenActivate);
    void* exitMenu = MainExitMenuInit(MenuHandle);
    CreateButtonControl(MenuHandle, menuDescHdle, "quit", exitMenu, GfuiScreenActivate);

    GfParmReleaseHandle(menuDescHdle);

    GfuiMenuDefaultKeysAdd(MenuHandle);
    GfuiAddKey(MenuHandle, GFUIK_ESCAPE, "Quit the game", exitMenu, GfuiScreenActivate, NULL);

    // Register the ExitMenu init func in the race engine.
    ReSetExitMenuInitFunc(ExitMenuInit);
	
    return 0;
}

/*
 * Function
 *	MainMenuRun
 *
 * Description
 *	Activate the main menu
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
int
MainMenuRun(void)
{
    GfuiScreenActivate(MenuHandle);
    return 0;
}
