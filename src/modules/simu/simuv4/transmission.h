/***************************************************************************

    file                 : transmission.h
    created              : Mon Apr 16 16:04:36 CEST 2001
    copyright            : (C) 2001 by Eric Espie
    email                : Eric.Espie@torcs.org

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
*/

#ifndef _TRANSMISSION_H_
#define _TRANSMISSION_H_

typedef struct
{
    int		gear;
    int		gearMin;
    int		gearMax;
    int		gearNext;
    tdble   shiftTime; /* time of shifting in sec */
    tdble   timeToEngage; /* time to engage gearNext, in sec */
} tGearbox;

typedef struct
{
    int		state;
#define CLUTCH_APPLIED	 1
#define CLUTCH_RELEASED  0
#define CLUTCH_RELEASING 2
    int		mode;
#define CLUTCH_AUTO	0
#define CLUTCH_MANUAL	1
    tdble	timeToRelease;	/* remaining time before releasing the clutch pedal */
    tdble	releaseTime;	/* time needed for releasing the clutch pedal */
    tdble	transferValue;	/* 1.0 -> released, 0.0 -> applied */
} tClutch;

typedef struct
{
    tGearbox	gearbox;
    tClutch	clutch;
    tDriveType	type;
    tdble	overallRatio[MAX_GEARS];	/* including final drive ratio */
    tdble   gearI[MAX_GEARS];       /* raw gear inertia */
    tdble	driveI[MAX_GEARS];		/* Inertia (including engine) */
    tdble	freeI[MAX_GEARS];		/* Inertia when clutch is applied (wheels side) */
    tdble	gearEff[MAX_GEARS];		/* Gear Efficiency */
    tdble	curOverallRatio;
    tdble	curI;

#define TRANS_FRONT_DIFF	0
#define TRANS_REAR_DIFF		1
#define TRANS_CENTRAL_DIFF	2
    tDifferential	differential[3];
} tTransmission;


#endif /* _TRANSMISSION_H_ */
