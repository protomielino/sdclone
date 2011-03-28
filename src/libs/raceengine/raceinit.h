/***************************************************************************

    file        : raceinit.h
    created     : Sat Nov 16 12:24:26 CET 2002
    copyright   : (C) 2002 by Eric Espie
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

#ifndef _RACEINIT_H_
#define _RACEINIT_H_

class GfRaceManager;
class GfRace;


extern void ReStartNewRace(void);
extern void ReResumeRace(void);

extern void ReInit(void);
extern int  ReExit();
extern void ReShutdown(void);

extern void ReRaceSelectRaceman(GfRaceManager* pRaceMan);
extern void ReRaceRestore(void* hparmResults);
extern void ReRaceConfigure(bool bInteractive = true);

extern int  ReInitCars(void);

extern void ReInitGraphics(void);

extern void ReRaceCleanup(void);
extern void ReRaceCleanDrivers(void);

extern char *ReGetCurrentRaceName(void);

extern char *ReGetPrevRaceName(void);

extern tModList *ReRaceModList;

// The race (temporarily partly duplicates ReInfo, as long as not merged together).
extern GfRace* ReGetRace();

#endif /* _RACEINIT_H_ */ 



