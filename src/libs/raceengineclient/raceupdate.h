/***************************************************************************

    file        : raceupdate.h
    created     : Sat Nov 23 09:35:21 CET 2002
    copyright   : (C) 2002 by Eric Espie
    email       : eric.espie@torcs.org   
    version     : $Id: raceupdate.h,v 1.4 2004/04/05 18:25:00 olethros Exp $                                  

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
    @version	$Id: raceupdate.h,v 1.4 2004/04/05 18:25:00 olethros Exp $
*/

#ifndef _RACEUPDATE_H_
#define _RACEUPDATE_H_

extern void ReInitUpdaters();
extern void ReInitCarGraphics();
extern void ReShutdownUpdaters();

extern void ReStart(void);
extern void ReStop(void);
extern int  ReUpdate(void);

#ifdef DEBUG
extern void ReOneStep(double dt);
#endif

#endif /* _RACEUPDATE_H_ */ 



