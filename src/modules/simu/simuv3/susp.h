/***************************************************************************

    file                 : susp.h
    created              : Sun Mar 19 00:08:53 CET 2000
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

#ifndef _SUSP_H__
#define _SUSP_H__


typedef struct
{
    tdble C1, b1, v1; /* coefs for slow */
    tdble C2, b2;     /* coefs for fast */
} tDamperDef;

typedef struct
{
    tDamperDef bump;
    tDamperDef rebound;
	tdble efficiency;
} tDamper;

typedef struct
{
    tdble K;          /* spring coef */
    tdble F0;         /* initial force */
    tdble x0;         /* initial suspension travel */
    tdble xMax;       /* maxi suspension travel */
    tdble bellcrank;  /* ratio of movement between wheel and suspension */
    tdble packers;     /* packer size (min susp. travel) */
} tSpring;


typedef enum SuspensionType {
    Ideal, Simple, Wishbone
} eSuspensionType;


typedef struct Suspension
{
    tSpring spring;
    tDamper damper;

    tdble x; /* suspension travel */
    tdble over_x; /* suspension travel beyond packers */
    tdble v; /* suspension travel speed */
    tdble fx; // pure elastic collision
    tdble fy; // pure elastic collision
    eSuspensionType type;
    t3Dd dynamic_angles;
    t3Dd link;
    tdble force;        /* generated force */
    int    state;        /* indicate the state of the suspension */
#define SIM_SUSP_COMP   1  	/* the suspension is fully compressed */
#define SIM_SUSP_EXT    2  	/* the suspension is fully extended */
#define SIM_SUSP_OVERCOMP 4 /* the suspension is overcompressed */
} tSuspension;

#define PRM_SUSPENSION_TYPE "suspension type"

#endif /* _SUSP_H__ */
