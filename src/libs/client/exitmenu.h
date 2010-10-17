/***************************************************************************

    file                 : exitmenu.h
    created              : Sat Mar 18 23:42:22 CET 2000
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
 

#ifndef _EXITMENU_H_
#define _EXITMENU_H_

#include "client.h"


CLIENT_API void* ExitMenuInit(void *menu);
CLIENT_API void* MainExitMenuInit(void *mainMenu);

#endif /* _EXITMENU_H_ */ 



