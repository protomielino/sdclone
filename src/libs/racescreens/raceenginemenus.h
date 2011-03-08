/***************************************************************************

    file        : raceenginemenus.h
    created     : Fri Jan  3 22:25:02 CET 2003
    copyright   : (C) 2003 by Eric Espié                        
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

#ifndef _RACEENGINEMENUS_H_
#define _RACEENGINEMENUS_H_

#include "racescreens.h"


RACESCREENS_API int ReRacemanMenu();
RACESCREENS_API int ReNextEventMenu(void);
RACESCREENS_API void ReConfigureRace(void * /* dummy */);
RACESCREENS_API void ReSetRacemanMenuHandle(void * handle);

extern void* ReGetRacemanMenuHandle();

extern void ReConfigRunState(bool bStart = false);

#endif /* _RACEENGINEMENU_H_ */ 



