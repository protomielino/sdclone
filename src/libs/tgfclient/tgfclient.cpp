/***************************************************************************
                          tgfclient.cpp -- The Gaming Framework UI
                             -------------------                                         
    created              : Fri Aug 13 22:31:43 CEST 1999
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

#include "gui.h"

// WDB test ...
#ifdef __DEBUG_MEMORYMANAGER__

// Avoid memory leaks ...
int NumberOfScreens = 0;
tGfuiScreen* OwnerOfScreens[MAXSCREENS];

void RegisterScreens(void* screen)
{
	tGfuiScreen* _screen = (tGfuiScreen*) screen;
	if (NumberOfScreens < MAXSCREENS)
		OwnerOfScreens[NumberOfScreens++] = _screen;
	else
		GfLogInfo("NumberOfScreens: %d > MAXSCREENS\n", NumberOfScreens);
}

void FreeScreens()
{
	// For debugging purposes:
	//doaccept(); // Do not free the blocks, just take it out of the list

	for (int I = 0; I <= NumberOfScreens; I++)
	{
		// This screen is corrupted!
		// TODO: Find out why
		if (I == 2)
			continue;

		// Get the screen from the owner
		tGfuiScreen* screen = OwnerOfScreens[I];
		if (screen) 
		{
			GfuiScreenRelease(screen); // Free all resources
		}
	}

	// Back to normal mode
	dofree(); // Free the blocks

}
// ... Avoid memory leaks
#else
void RegisterScreens(void* screen){};
void FreeScreens(){};
#endif
// ... WDB test


void GfuiInit(void)
{
    gfuiInit();
}

void GfuiShutdown(void)
{
    gfuiShutdown();

	FreeScreens();
	
	GfScrShutdown();
}
