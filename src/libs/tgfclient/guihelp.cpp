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
    
    void *hmenu = GfuiMenuLoad("helpmenu.xml");

    GfuiMenuCreateStaticControls(hmenu, scrHandle);

	// Get menu properties.
	const int nXLeftColumn = (int)GfuiMenuGetNumProperty(hmenu, "xLeftColumn", 30);
	const int nXRightColumn = (int)GfuiMenuGetNumProperty(hmenu, "xRightColumn", 330);
	const int nNameFieldWidth = (int)GfuiMenuGetNumProperty(hmenu, "nameFieldWidth", 80);
	const int nLineShift = (int)GfuiMenuGetNumProperty(hmenu, "lineShift", 12);
	const int nYTopLine = (int)GfuiMenuGetNumProperty(hmenu, "yTopLine", 380);

    // Create 2 columns table for the keyboard shortcuts explanations
    int ys = nYTopLine;
    int yn = nYTopLine;
    
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
					GfuiMenuCreateLabelControl(scrHandle, hmenu, "keyName", true, // from template
											   curKey->name, nXLeftColumn, ys);
					GfuiMenuCreateLabelControl(scrHandle, hmenu, "keyDesc", true, // from template
											   curKey->descr, nXLeftColumn + nNameFieldWidth, ys);
					ys -= nLineShift;
					break;

				default:
					GfuiMenuCreateLabelControl(scrHandle, hmenu, "keyName", true, // from template
											   curKey->name, nXRightColumn, yn);
					GfuiMenuCreateLabelControl(scrHandle, hmenu, "keyDesc", true, // from template
											   curKey->descr, nXRightColumn + nNameFieldWidth, yn);
					yn -= nLineShift;
					break;
			}
		}
	
		if (curKey == pscr->userKeys)
			curKey = (tGfuiKey*)NULL;

    } while (curKey);
    

    // Create Back button.
    GfuiMenuCreateButtonControl(scrHandle, hmenu, "backbutton", prevScreen, GfuiScreenActivate);

    // Create version label.
    const int versionId = GfuiMenuCreateLabelControl(scrHandle, hmenu, "versionlabel");
    GfuiLabelSetText(scrHandle, versionId, VERSION_LONG);

    // Close menu XML descriptor.
    GfParmReleaseHandle(hmenu);
    
    // Add keyboard shortcuts.
    GfuiAddKey(scrHandle, GFUIK_ESCAPE, "", prevScreen, GfuiScreenReplace, NULL);
    GfuiAddKey(scrHandle, GFUIK_RETURN, "", prevScreen, GfuiScreenReplace, NULL);
    GfuiAddKey(scrHandle, GFUIK_F1, "", prevScreen, GfuiScreenReplace, NULL);

    GfuiMenuDefaultKeysAdd(scrHandle);

    GfuiScreenActivate(scrHandle);
}

