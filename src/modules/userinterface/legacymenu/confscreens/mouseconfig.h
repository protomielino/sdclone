/***************************************************************************

    file        : mouseconfig.h
    created     : Thu Mar 13 21:29:35 CET 2003
    copyright   : (C) 2003 by Eric Espie
    email       : eric.espie@torcs.org

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

    @author	<a href=mailto:torcs@free.fr>Eric Espie</a>
*/

#ifndef _MOUSECONFIG_H_
#define _MOUSECONFIG_H_

#include "confscreens.h"


/* nextMenu : the menu to go to when "next" button is pressed */
extern void *MouseCalMenuInit(void *prevMenu, void *nextMenu, tCmdInfo *cmd, int maxcmd);

#endif /* _MOUSECONFIG_H_ */
