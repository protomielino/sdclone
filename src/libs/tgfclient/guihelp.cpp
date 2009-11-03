/***************************************************************************
                       guihelp.cpp -- automatic help on keys                                
                             -------------------                                         
    created              : Fri Aug 13 22:20:37 CEST 1999
    copyright            : (C) 1999 by Eric Espie                         
    email                : torcs@free.fr   
    version              : $Id: guihelp.cpp,v 1.3 2004/02/28 08:39:13 torcs Exp $                                  
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
    		GUI help screen management.
    @author	<a href=mailto:torcs@free.fr>Eric Espie</a>
    @version	$Id: guihelp.cpp,v 1.3 2004/02/28 08:39:13 torcs Exp $
    @ingroup	gui
*/

#include <stdlib.h>
#ifdef WIN32
#include <windows.h>
#ifndef HAVE_CONFIG_H
#define HAVE_CONFIG_H
#endif
#endif
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "tgfclient.h"
#include "gui.h"

static void	*scrHandle;
static float	*fgColor1 = &(GfuiColor[GFUI_HELPCOLOR1][0]);
static float	*fgColor2 = &(GfuiColor[GFUI_HELPCOLOR2][0]);

void
gfuiHelpInit(void)
{
}

/** Generate a help screen.
    @ingroup	gui
    @param	prevScreen	Previous screen to return to
    @warning	The help screen is activated.
 */
void
GfuiHelpScreen(void *prevScreen)
{
    int		x, x2, dx, y;
    tGfuiKey	*curKey;
    tGfuiKey	*curSKey;
    tGfuiScreen	*pscr = (tGfuiScreen*)prevScreen;
    
    // Create screen, load menu XML descriptor and create static controls.
    scrHandle = GfuiScreenCreate();
    
    void *menuXMLDescHdle = LoadMenuXML("helpmenu.xml");

    CreateStaticControls(menuXMLDescHdle, scrHandle);

    // Create 2 columns table for the keyboard shortcuts explainations
    x  = 30;
    dx = 80;
    x2 = 330;
    y  = 380;
    
    curSKey = pscr->userSpecKeys;
    curKey = pscr->userKeys;
    do {
	if (curSKey) {
	    curSKey = curSKey->next;
	    GfuiLabelCreateEx(scrHandle, curSKey->name, fgColor1, GFUI_FONT_SMALL_C, x, y, GFUI_ALIGN_HL_VB, 0);
	    GfuiLabelCreateEx(scrHandle, curSKey->descr, fgColor2, GFUI_FONT_SMALL_C, x + dx, y, GFUI_ALIGN_HL_VB, 0);
	}

	if (curKey) {
	    curKey = curKey->next;
	    GfuiLabelCreateEx(scrHandle, curKey->name, fgColor1, GFUI_FONT_SMALL_C, x2, y, GFUI_ALIGN_HL_VB, 0);
	    GfuiLabelCreateEx(scrHandle, curKey->descr, fgColor2, GFUI_FONT_SMALL_C, x2 + dx, y, GFUI_ALIGN_HL_VB, 0);
	}
	y -= 12;
	
	if (curKey == pscr->userKeys) curKey = (tGfuiKey*)NULL;
	if (curSKey == pscr->userSpecKeys) curSKey = (tGfuiKey*)NULL;

    } while (curKey || curSKey);
    

    // Create Back button.
    CreateButtonControl(scrHandle, menuXMLDescHdle, "backbutton", prevScreen, GfuiScreenActivate);

    // Create version label.
    const int versionId = CreateLabelControl(scrHandle, menuXMLDescHdle, "versionlabel");
    GfuiLabelSetText(scrHandle, versionId, VERSION);

    // Close menu XML descriptor.
    GfParmReleaseHandle(menuXMLDescHdle);
    
    // Add keyboard shortcuts.
    GfuiAddKey(scrHandle, (unsigned char)27, "", prevScreen, GfuiScreenReplace, NULL);
    GfuiAddKey(scrHandle, (unsigned char)13, "", prevScreen, GfuiScreenReplace, NULL);
    GfuiAddSKey(scrHandle, GLUT_KEY_F1, "", prevScreen, GfuiScreenReplace, NULL);

    GfuiMenuDefaultKeysAdd(scrHandle);

    GfuiScreenActivate(scrHandle);
}

