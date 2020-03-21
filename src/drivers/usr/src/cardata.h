/***************************************************************************

    file                 : cardata.h
    created              : Thu Sep 23 12:31:33 CET 2004
    copyright            : (C) 2004 Bernhard Wymann
    email                : berniw@bluewin.ch
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

/*
    This class holds global facts about cars, therefore no data relative to
    each other (for that is the class Opponents/Opponent responsible).
*/

#ifndef _BT_CARDATA_H_
#define _BT_CARDATA_H_

#include <stdio.h>
#include <math.h>
#include <car.h>
#include <robottools.h>
#include <raceman.h>

#include "linalg.h"
#include "globaldefs.h"

class SingleCardata
{
public:
    void init(CarElt *car);

    inline float  getSpeedInTrackDirection() { return speed; }
    inline float  getWidthOnTrack() { return width; }
    inline float  getLengthOnTrack() { return length; }
    inline float  getTrackangle() { return trackangle; }
    inline float  getCarAngle() { return angle; }
    inline double getAvgAccelX() { return avgAccelX; }

    inline bool   thisCar(tCarElt *Car) { return (Car == this->car); }

    inline tPosd  *getCorner1() { return corner1; }
    inline tPosd  *getCorner2() { return corner2; }
    inline tPosd  *getLastSpeed() { return lastspeed; }

    void update();
    void updateModel();

    bool   HasTYC;
    bool   HasABS;
    bool   HasESP;
    bool   HasTCL;

protected:
    static float getSpeed(tCarElt *car, float trackangle);

    float  speed;		// speed in direction of the track.
    float  width;		// the cars needed width on the track.
    float  length;		// the cars needed length on the track.
    float  trackangle;	// Track angle at the opponents position.
    float  angle;		// The angle of the car relative to the track tangent.
    float  fuelMassFactor;
    double avgAccelX;

    tPosd corner1[4];
    tPosd corner2[4];
    tPosd lastspeed[3];

    tdble t_m;
    tdble t_m_f;
    tdble t_m_r;
    tdble currentMass;

    double CTFactor;

    tdble RH;
    tdble CA;
    tdble CA_RW;
    tdble CA_FW;
    tdble CA_GE;

    tdble baseMass;

    tCarElt *car;		// For identification.

    double TyreConditionFront();
    double TyreConditionRear();
    double TyreConditionLeft();
    double TyreConditionRight();
    double TyreTreadDepthFront();
    double TyreTreadDepthRear();
    double FrontAxleSlipTangential() const;

public:
    double lTT;
    double GRIP_FACTOR;
    double GRIP_FACTOR_F;
    double GRIP_FACTOR_R;
    double GRIP_FACTOR_LEFT;
    double GRIP_FACTOR_RIGHT;
    tdble  TYREWEAR;
    tdble  CRITICAL_TYREWEAR;
    double TARGET_SLIP;	// amount of slip to give maximum grip.
    double MAX_SLIP;		// amount of slip where grip level drops below 99% of maximum grip.
    tdble lftOH;
    tdble rgtOH;
    tdble carMu;
    tdble fullCarMu;
    tdble offlineFuelCarMu;
    tdble baseCarMu;
    tdble muscale;
    tdble basebrake;
    tdble fuel;
    tdble damage;
};


// TODO: use singleton pattern.
class Cardata
{
public:
    Cardata(tSituation *s);
    ~Cardata();

    void update();
    SingleCardata *findCar(tCarElt *car);

protected:
    SingleCardata *data;	// Array with car data.
    int ncars;				// # of elements in data.
};

#endif
