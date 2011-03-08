/***************************************************************************

    file        : racegl.h
    created     : Sat Nov 16 19:02:56 CET 2002
    copyright   : (C) 2002 by Eric Espié                        
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

#ifndef _RACEGL_H_
#define _RACEGL_H_

#include "racescreens.h"


RACESCREENS_API void *ReScreenInit(void);
RACESCREENS_API void  ReScreenShutdown(void);
RACESCREENS_API void *ReHookInit(void);
RACESCREENS_API void ReHookShutdown(void);
RACESCREENS_API void ReSetRaceMsg(const char *msg);
RACESCREENS_API void ReSetRaceBigMsg(const char *msg);

RACESCREENS_API void *ReResScreenInit(void);
RACESCREENS_API void ReResScreenSetTrackName(const char *trackName);
RACESCREENS_API void ReResScreenSetTitle(const char *title);
RACESCREENS_API void ReResScreenAddText(const char *text);
RACESCREENS_API void ReResScreenSetText(const char *text, int line, int clr);
RACESCREENS_API void ReResScreenRemoveText(int line);
RACESCREENS_API void ReResShowCont(void);
RACESCREENS_API int  ReResGetLines(void);
RACESCREENS_API void ReResEraseScreen(void);

#endif /* _RACEGL_H_ */ 



