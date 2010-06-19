/***************************************************************************

    file                 : networkingmenu.h
    created              : July 2009
    copyright            : (C) 2009 Brian Gavin
    web                  : speed-dreams.sourceforge.net

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _NETWORKINGMENU_H_
#define _NETWORKINGMENU_H_

void ReLoadQuickRace();
void ServerPrepareStartNetworkRace(void *pVoid);

void reNetworkClientConnectMenu(void *pVoid);
void reNetworkMenu(void * /* dummy */);
void reNetworkHostMenu(void * /* dummy */);
void reNetworkClientMenu(void * /* dummy */);

#endif // _NETWORKINGMENU_H_
