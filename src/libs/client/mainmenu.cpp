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


#include <stdio.h>
#include <tgfclient.h>
#include <singleplayer.h>
#include <driverconfig.h>

#include "mainmenu.h"
#include "exitmenu.h"
#include "optionmenu.h"
#include "credits.h"


void *menuHandle = NULL;
tModList *RacemanModLoaded = (tModList*)NULL;

static void
onActivatePlayerConfig(void * /* dummy */)
{
    /* Here, we need to call OptionOptionInit each time the firing button
       is pressed, and not only once at the Main menu initialization,
       because the previous menu has to be saved (ESC, Back) and because it can be this menu,
       as well as the Raceman menu */
    GfuiScreenActivate(DriverMenuInit(menuHandle));
}

static void
MainMenuActivate(void * /* dummy */)
{
	if (RacemanModLoaded != NULL) {
		GfModUnloadList(&RacemanModLoaded);
	}
}

/*
 * Function
 *	MainMenuInit
 *
 * Description
 *	init the main menus
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
    menuHandle = GfuiScreenCreateEx((float*)NULL, 
				    NULL, MainMenuActivate, 
				    NULL, (tfuiCallback)NULL, 
				    1);

    GfuiScreenAddBgImg(menuHandle, "data/img/splash-main.png");

    GfuiTitleCreate(menuHandle, "Speed Dreams", 0);

    GfuiLabelCreate(menuHandle,
		    "The Open Racing Car Simulator - Next Generation",
		    GFUI_FONT_LARGE, 320, 420, GFUI_ALIGN_HC_VB, 0);

    GfuiMenuButtonCreate(menuHandle,
			 "Race", "Races Menu",
			 ReSinglePlayerInit(menuHandle), GfuiScreenActivate);

    GfuiMenuButtonCreate(menuHandle,
			 "Configure Players", "Players configuration menu",
			 NULL, onActivatePlayerConfig);

    GfuiMenuButtonCreate(menuHandle,
			 "Options", "Configure",
			 OptionOptionInit(menuHandle), GfuiScreenActivate);
    
    GfuiMenuButtonCreate(menuHandle,
			 "Credits", "Thanks to all contributors",
			 menuHandle, CreditsScreenActivate);
    
    GfuiMenuDefaultKeysAdd(menuHandle);

    GfuiMenuBackQuitButtonCreate(menuHandle,
				 "Quit", "Quit game",
				 MainExitMenuInit(menuHandle), GfuiScreenActivate);

    return 0;
}

/*
 * Function
 *	
 *
 * Description
 *	
 *
 * Parameters
 *	
 *
 * Return
 *	
 *
 * Remarks
 *	
 */
int
MainMenuRun(void)
{
    GfuiScreenActivate(menuHandle);
    return 0;
}
