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

    void *param = LoadMenuXML("welcomemenu.xml");

    CreateStaticControls(param,menuHandle);

    //Add buttons and create based on xml
    CreateButtonControl(menuHandle,param,"race",ReSinglePlayerInit(menuHandle), GfuiScreenActivate);
    CreateButtonControl(menuHandle,param,"configure",DriverMenuInit(menuHandle),GfuiScreenActivate);
    CreateButtonControl(menuHandle,param,"options",OptionOptionInit(menuHandle),GfuiScreenActivate);
    CreateButtonControl(menuHandle,param,"credits",menuHandle,CreditsScreenActivate);
    CreateButtonControl(menuHandle,param,"quit",MainExitMenuInit(menuHandle), GfuiScreenActivate);

    GfuiMenuDefaultKeysAdd(menuHandle);

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
