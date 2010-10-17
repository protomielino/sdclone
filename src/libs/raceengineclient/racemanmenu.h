/***************************************************************************

    file        : racemanmenu.h
    created     : Fri Jan  3 22:25:02 CET 2003
    copyright   : (C) 2003 by Eric Espi�                        
    email       : eric.espie@torcs.org   
    version     : $Id$                                  

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
    @version	$Id$
*/

#ifndef _RACEMANMENU_H_
#define _RACEMANMENU_H_

#include "raceengineclient.h"


RACEENGINECLIENT_API int ReRacemanMenu(void);
RACEENGINECLIENT_API int ReNewTrackMenu(void);
RACEENGINECLIENT_API void ReConfigureMenu(void * /* dummy */);
RACEENGINECLIENT_API void ReSetRacemanMenuHandle( void * handle);

#endif /* _RACEMANMENU_H_ */ 



