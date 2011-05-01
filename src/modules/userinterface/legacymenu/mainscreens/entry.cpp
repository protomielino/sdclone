/***************************************************************************

    file                 : entry.cpp
    created              : Sat Mar 18 23:41:55 CET 2000
    copyright            : (C) 2000 by Eric Espie
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

#include "client.h"
#include "splash.h"

/*
 * Function
 *	GameEntry
 *
 * Description
 *	Entry point of the game.
 *
 * Parameters
 *	None
 *
 * Return
 *	true on success, false if anything bad happened.
 *
 * Remarks
 *	
 */
bool
MenuEntry(void)
{
    // Open the splash screen, load menus in "background" and finally open the main menu.
    return SplashScreen();
}
