/***************************************************************************

    file                 : entry.cpp
    created              : Sat Mar 18 23:41:55 CET 2000
    copyright            : (C) 2000 by Eric Espie
    email                : torcs@free.fr
    version              : $Id: entry.cpp,v 1.4 2005/02/01 15:55:51 berniw Exp $

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
#include "mainmenu.h"
#include "splash.h"

/*
 * Function
 *	GameEntry
 *
 * Description
 *	entry point of the game
 *
 * Parameters
 *	none
 *
 * Return
 *	none
 *
 * Remarks
 *	
 */
void
GameEntry(void)
{
	// Initialize gaming framework.
    GfInitClient();

	// Open the splash screen, load menus in "backgroud" and finally open the main menu.
    SplashScreen();
}
