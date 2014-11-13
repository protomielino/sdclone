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

// Avoid memory leaks ...
int NumberOfScreens = -1;
tGfuiScreen* OwnerOfScreens[MAXSCREENS];

void RegisterScreens(void* screen)
{
	if (++NumberOfScreens < MAXSCREENS)
		OwnerOfScreens[NumberOfScreens] = (tGfuiScreen*) screen;
	else
		GfLogInfo("NumberOfScreens: %d > MAXSCREENS\n", NumberOfScreens);
}

void FreeScreens()
{
	for (int I = 0; I <= NumberOfScreens; I++)
	{
		tGfuiScreen* screen = OwnerOfScreens[I];
		if (screen)
		{
			tGfuiObject* object = screen->objects;
			while (object)
			{
				tGfuiObject* release = object;
				object = object->next;
				if (object == release)
					object = NULL;
				if (object == screen->objects)
					object = NULL;
				gfuiReleaseObject(release);
			}

			tGfuiKey* key = screen->userKeys;
			while (key)
			{
				tGfuiKey* relkey = key;
				key = key->next;
				if (key == relkey)
					key = NULL;
				if (key == screen->userKeys)
					key = NULL;
				free(relkey->name);
				free(relkey->descr);
				free(relkey);
			}

			free(screen);
		}
	}
}
// ... Avoid memory leaks

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
