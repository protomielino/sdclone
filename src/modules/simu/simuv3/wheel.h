/***************************************************************************

    file                 : wheel.h
    created              : Sun Mar 19 00:09:18 CET 2000
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

#ifndef _WHEEL_H__
#define _WHEEL_H__

#include "differential.h"

#undef USE_THICKNESS
#define N_THICKNESS_SEGMENTS 16
#define SEGMENT_RANGE 4
#define PRM_DYNAMIC_CAMBER "steering dynamic camber rate"

typedef struct
{

    /* internal data */
    tSuspension  susp;		/* associated suspension */
    tBrake       brake;		/* associated brake disk */

    /* dynamic */
    t3Dd	forces;		/* forces acting on car, using car's FOR */
    tdble	rollRes;	/* Rolling Resistance (summed over the car) */
    tdble	rideHeight;	/* height of the bottom of the car */
    tdble	zRoad;		/* z of the road */
    t3Dd   	pos;	   	/* world related */
    t3Dd	bodyVel;	/* world related */
    tdble  	driveTq;   	/* engine torque */
    tdble	vt;

    tdble  	spinTq;		/* spin torque feedback */
    tdble  	spinVel;   	/* spin velocity */
    tdble  	prespinVel;   	/* spin velocity */
    int     	state;     	/* wheel state */
    /* 1 and 2 are for suspension state */
#define SIM_WH_SPINNING 4	/* the wheel is spinning */
#define SIM_WH_LOCKED   8	/* the wheel is locked */
#define SIM_WH_FREE     16  /* the wheel is non longer on the car */
#define SIM_WH_BURST    32  /* the wheel has exploded */
    tdble	axleFz;		/* force from axle (anti-roll bar) */
    tTrkLocPos	trkPos;		/* current track position */
    tPosd	relPos;		/* relative pos / GC */
    tdble	sa;		/* slip angle */
    tdble	sx;		/* longitudinal slip value */
    tdble	steer;

    /* static */
    tPosd	staticPos;	/* pos relative to the GC (z is suspension
						   travel at rest) and angles are camber (ax),
						   caster (ay) and toe (az) */
    tdble	rollCenter;

    tdble  	weight0;	/* initial weight on this wheel */
    tdble	tireSpringRate;
    tdble  	radius;
    tdble   width;
    tdble  	mu;
    tdble  	I;       	/* I = inertial moment of the wheel */
    tdble  	curI;       	/* Current inertia for the wheel (including transmission) */
    tdble	mfC;		/* Magic Formula C coeff */
    tdble	mfB;		/* Magic Formula B coeff */
    tdble	mfE;		/* Magic Formula E coeff */
    tdble   mfT;            /* Temperature-dependent coeff */

	/* Tyre wear */
    tdble   Ca;         /* Adherence coefficient */
    tdble   T_current;      /* Temperature */
    tdble   T_operating;    /* Operating temperature */
    tdble   condition;      /* Tyre condition */
    tdble   T_range;
	/* Modelling uneven thickness due to wear */
#ifdef USE_THICKNESS
	tdble   thickness[N_THICKNESS_SEGMENTS];
	tdble   segtemp[N_THICKNESS_SEGMENTS];
#endif

    tdble	lfMax;		/* Load factor */
    tdble	lfMin;		/* Load factor */
    tdble	lfK;		/* Load factor */
    tdble	opLoad;		/* Operating load */
    tdble	mass;		/* total wheel mass (incl. brake) (unsprung mass) */
    tdble	camber;		/* camber, negative toward exterior on both sides */
    tdble	pressure;	/* tire pressure */
    tdble   rel_vel;    /* relative velocity - used for realstic suspension movement*/
    tdble   dynamic_camber; /* steering dynamic camber angle */
    tdble   bump_force;  /* bumps due to realistic suspension movement */

	/* axis damage */
	tdble rotational_damage_x;
	tdble rotational_damage_z;
	tdble bent_damage_x;
	tdble bent_damage_z;
    tDynAxis	in;
    tDynAxis	feedBack;

    tdble	preFn, preFt;
	tdble   Em; // estimate of mass
	tdble   s_old;
	tdble   F_old;

    t3Dd   	normal;

} tWheel;



#endif /* _WHEEL_H__ */
