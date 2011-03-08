/***************************************************************************

    file        : racesituation.h
    copyright   : (C) 2010 by Jean-Philippe Meuret
    web         : www.speed-dreams.org 
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
    		
    @author	    Jean-Philippe Meuret
    @version	$Id$
*/

#ifndef _RACESITUATION_H_
#define _RACESITUATION_H_

#include "raceman.h"


// The race situation data structure.
extern tRmInfo *ReInfo;

extern tRmInfo* ReSituation();

extern tRmInfo* ReSituationAllocInit(const tRmInfo* pSource);
extern tRmInfo* ReSituationCopy(tRmInfo*& pTarget, const tRmInfo* pSource);
extern void ReSituationAcknowlegdeEvents(tRmInfo* pCurrSituation,
										 const tRmInfo* pPrevSituation = 0);
extern void ReSituationFreez(tRmInfo*& pSituation);

#endif /* _RACESITUATION_H_ */ 



