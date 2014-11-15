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
int NextScreenID = 0;
int NumberOfScreens = 0;
tGfuiScreen* OwnerOfScreens[MAXSCREENS];

// Register all screens that are allocated
void RegisterScreens(void* screen)
{
	tGfuiScreen* _screen = (tGfuiScreen*) screen;
	_screen->ScreenID = ++NextScreenID;

	// Find a deleted entry
	for (int I = 0; I < NumberOfScreens; I++)
	{
		if (OwnerOfScreens[I] == NULL)
		{
			OwnerOfScreens[I] = _screen;
			return;
		}
	}

	if (NumberOfScreens < MAXSCREENS)
		OwnerOfScreens[NumberOfScreens++] = _screen;
	else
		GfLogInfo("NumberOfScreens: %d > MAXSCREENS\n", NumberOfScreens);
}

// Unregister all screens that are released
void UnregisterScreens(void* screen)
{
	// Find the entry
	for (int I = 0; I <= NumberOfScreens; I++)
	{
		if (OwnerOfScreens[I] == screen)
		{
			OwnerOfScreens[I] = NULL;
			return;
		}
	}
}

// Free screens that are stil allocated
void FreeScreens()
{
	// For debugging purposes:
	//doaccept(); // Do not free the blocks, just take it out of the list

	for (int I = 0; I < NumberOfScreens; I++)
	{
		// Get the screen from the owner
		tGfuiScreen* screen = OwnerOfScreens[I];
		if (screen) 
			GfuiScreenRelease(screen); // Free all resources
	}

	// Back to normal mode
	dofree(); // Free the blocks

}
// ... Avoid memory leaks
#else
void RegisterScreens(void* screen){};
void FreeScreens(){};
void UnregisterScreens(void* screen){};
#endif
// ... WDB test


void GfuiInit(void)
{
    gfuiInit();
}

void GfuiShutdown(void)
{
    gfuiShutdown();

	// Free screens that are stil allocated
	FreeScreens();
	
	GfScrShutdown();
}
