/***************************************************************************

    file                 : axle.h
    created              : Sun Mar 19 00:05:17 CET 2000
    copyright            : (C) 2000 by Eric Espie
    email                : torcs@free.fr

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/



#ifndef _AXLE_H__
#define _AXLE_H__

#include "differential.h"

typedef struct
{
    tdble xpos;

    tSuspension arbSusp;	/* anti-roll bar */
    tSuspension heaveSusp;  /* heave/3rd/central spring */
    tdble	wheight0;

    /* dynamic */
    tdble	force[2]; /* right and left */

    tdble	I;	/* including differential inertia but not wheels */
} tAxle;


#endif /* _AXLE_H__ */
