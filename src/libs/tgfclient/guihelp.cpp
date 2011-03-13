/***************************************************************************
                       guihelp.cpp -- automatic help on keys                                
                             -------------------                                         
    created              : Fri Aug 13 22:20:37 CEST 1999
    copyright            : (C) 1999 by Eric Espie                         
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

/** @file   
    		GUI help screen management.
    @author	<a href=mailto:torcs@free.fr>Eric Espie</a>
    @version	$Id$
    @ingroup	gui
*/

#include <cstdlib>

#ifdef WIN32
#ifndef HAVE_CONFIG_H
#define HAVE_CONFIG_H
#endif
#endif

#ifdef HAVE_CONFIG_H
#include "version.h"
#endif

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
    tGfuiScreen	*pscr = (tGfuiScreen*)prevScreen;
    
    // Create screen, load menu XML descriptor and create static controls.
    scrHandle = GfuiScreenCreate();
    
    void *menuXMLDescHdle = LoadMenuXML("helpmenu.xml");

    CreateStaticControls(menuXMLDescHdle, scrHandle);

    // Create 2 columns table for the keyboard shortcuts explainations
    const int dx = 80;
    const int xs  = 30;
    const int dy  = -12;
    int ys  = 380;
    const int xn = 330;
    int yn  = 380;
    
    tGfuiKey *curKey = pscr->userKeys;
    do {
		if (curKey) {
			curKey = curKey->next;
			switch(curKey->key) {
				case GFUIK_BACKSPACE:
				case GFUIK_F1:
				case GFUIK_F2:
				case GFUIK_F3:
				case GFUIK_F4:
				case GFUIK_F5:
				case GFUIK_F6:
				case GFUIK_F7:
				case GFUIK_F8:
				case GFUIK_F9:
				case GFUIK_F10:
				case GFUIK_F11:
				case GFUIK_F12:
				case GFUIK_LEFT:
				case GFUIK_UP:
				case GFUIK_RIGHT:
				case GFUIK_DOWN:
				case GFUIK_PAGEUP:
				case GFUIK_PAGEDOWN:
				case GFUIK_HOME:
				case GFUIK_END:
				case GFUIK_INSERT:
				case GFUIK_DELETE:
				case GFUIK_CLEAR:
				case GFUIK_PAUSE:
					GfuiLabelCreate(scrHandle, curKey->name, GFUI_FONT_SMALL_C,
									xs, ys, GFUI_ALIGN_HL_VB, 0, fgColor1);
					GfuiLabelCreate(scrHandle, curKey->descr, GFUI_FONT_SMALL_C,
									xs + dx, ys, GFUI_ALIGN_HL_VB, 0, fgColor2);
					ys += dy;
					break;

				default:
					GfuiLabelCreate(scrHandle, curKey->name, GFUI_FONT_SMALL_C,
									xn, yn, GFUI_ALIGN_HL_VB, 0, fgColor1);
					GfuiLabelCreate(scrHandle, curKey->descr, GFUI_FONT_SMALL_C,
									xn + dx, yn, GFUI_ALIGN_HL_VB, 0, fgColor2);
					yn += dy;
					break;
			}
		}
	
		if (curKey == pscr->userKeys)
			curKey = (tGfuiKey*)NULL;

    } while (curKey);
    

    // Create Back button.
    CreateButtonControl(scrHandle, menuXMLDescHdle, "backbutton", prevScreen, GfuiScreenActivate);

    // Create version label.
    const int versionId = CreateLabelControl(scrHandle, menuXMLDescHdle, "versionlabel");
    GfuiLabelSetText(scrHandle, versionId, VERSION_LONG);

    // Close menu XML descriptor.
    GfParmReleaseHandle(menuXMLDescHdle);
    
    // Add keyboard shortcuts.
    GfuiAddKey(scrHandle, GFUIK_ESCAPE, "", prevScreen, GfuiScreenReplace, NULL);
    GfuiAddKey(scrHandle, GFUIK_RETURN, "", prevScreen, GfuiScreenReplace, NULL);
    GfuiAddKey(scrHandle, GFUIK_F1, "", prevScreen, GfuiScreenReplace, NULL);

    GfuiMenuDefaultKeysAdd(scrHandle);

    GfuiScreenActivate(scrHandle);
}

