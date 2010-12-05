/***************************************************************************

    file        : singleplayer.cpp
    created     : Sat Nov 16 09:36:29 CET 2002
    copyright   : (C) 2002 by Eric Espié                        
    email       : eric.espie@torcs.org   
    version     : $Id$                                  

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

#include <cstdlib>
#include <cstdio>

#include <tgfclient.h>
#include <raceman.h>

#include "racesituation.h"
#include "racemain.h"
#include "raceinit.h"
#include "racestate.h"
#include "singleplayer.h"

static void *singlePlayerHandle = NULL;

/* Called when the menu is activated */
static void
singlePlayerMenuActivate(void * /* dummy */)
{
    /* Race engine init */
    ReInit();
    ReInfo->_reMenuScreen = singlePlayerHandle;
}

/* Exit from Race engine */
static void
singlePlayerShutdown(void *prevMenu)
{
    GfuiScreenActivate(prevMenu);
    ReShutdown();
}


/* Initialize the single player menu */
void *
ReSinglePlayerInit(void *prevMenu)
{
    if (singlePlayerHandle) 
	return singlePlayerHandle;
    
    // Create screen, load menu XML descriptor and create static controls.
    singlePlayerHandle = GfuiScreenCreateEx((float*)NULL, 
					    NULL, singlePlayerMenuActivate, 
					    NULL, (tfuiCallback)NULL, 
					    1);
    void *menuXMLDescHdle = LoadMenuXML("singleplayermenu.xml");
    CreateStaticControls(menuXMLDescHdle, singlePlayerHandle);

    // Display the raceman buttons (1 for each race type)
    ReAddRacemanListButton(singlePlayerHandle, menuXMLDescHdle);

    // Create Back button
    CreateButtonControl(singlePlayerHandle, menuXMLDescHdle, "backbutton", prevMenu, singlePlayerShutdown);

    // Close menu XML descriptor.
    GfParmReleaseHandle(menuXMLDescHdle);
    
    // Register keyboard shortcuts.
    GfuiMenuDefaultKeysAdd(singlePlayerHandle);
    GfuiAddKey(singlePlayerHandle, GFUIK_ESCAPE, "Back To Main", prevMenu, singlePlayerShutdown, NULL);

    // Give the race engine the menu to come back to.
    ReStateInit(singlePlayerHandle);

    return singlePlayerHandle;
}
