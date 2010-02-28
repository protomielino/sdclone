// -*- Mode: c++ -*-
/***************************************************************************

    file                 : grsound.h
    created              : Thu Aug 17 23:57:35 CEST 2000
    copyright            : (C) 2000-2004 by Eric Espie, Christos Dimitrakakis
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

#ifndef _GRSOUND_H_
#define _GRSOUND_H_

#include <raceman.h>	//tSituation
class cGrCamera;	//Declared in ""grcam.h"

extern void grInitSound(tSituation* s, int ncars);
extern void grShutdownSound(int ncars);
extern float grRefreshSound(tSituation *s, cGrCamera *camera);

#endif /* _GRSOUND_H_ */ 
