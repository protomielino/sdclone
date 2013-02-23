/***************************************************************************

    file                 : aero.h
    created              : Sun Mar 19 00:04:59 CET 2000
    copyright            : (C) 2000 by Eric Espie
    email                : torcs@free.fr
    version              : $Id: aero.h 2917 2010-10-17 19:03:40Z pouillot $

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _AERO_H_
#define _AERO_H_

typedef struct
{
    /* dynamic */
    tdble	drag;		/* drag force along car x axis */
    tdble	lift[2];	/* front & rear lift force along car z axis */

    /* static */
    tdble	SCx2;
    tdble	Clift[2];	/* front & rear lift due to body not wings */
    tdble	Cd;		    /* for aspiration */
    tdble	CdBody;	    /* for aspiration, value without wings, for variable wing angles */
} tAero;


typedef struct
{
    /* dynamic */
    t3Dd	forces;
    tdble	Kx;
    tdble	Kz;
	tdble	angle;
    
    /* static */
    t3Dd	staticPos;
    
} tWing;



#endif /* _AERO_H_  */ 



