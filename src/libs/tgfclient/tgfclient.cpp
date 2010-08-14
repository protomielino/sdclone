/***************************************************************************
                          tgf.cpp -- The Gaming Framework                            
                             -------------------                                         
    created              : Fri Aug 13 22:31:43 CEST 1999
    copyright            : (C) 1999 by Eric Espie                         
    email                : torcs@free.fr   
    version              : $Id: tgfclient.cpp,v 1.2 2003/06/24 21:02:25 torcs Exp $                                  
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
#include "tgfclient.h"

extern void gfScreenInit(void);
extern void gfMenuInit(void);

void
GfInitClient(void)
{
    gfuiInit();
    gfMenuInit();
    gfScreenInit();
}
