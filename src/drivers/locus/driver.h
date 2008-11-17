/***************************************************************************

    file                 : driver.h
    created              : Thu Dec 20 01:20:19 CET 2002
    copyright            : (C) 2002-2004 Bernhard Wymann
    email                : berniw@bluewin.ch
    version              : $Id: driver.h,v 1.12 2006/03/06 22:43:50 berniw Exp $

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _DRIVER_H_
#define _DRIVER_H_

#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <math.h>

#include <tgf.h>
#include <track.h>
#include <car.h>
#include <raceman.h>
#include <robottools.h>
#include <robot.h>
#include <portability.h>

#include "opponent.h"
#include "pit.h"
#include "strategy.h"
#include "cardata.h"
#include "raceline.h"

#define BT_SECT_PRIV "private"
#define BT_ATT_FUELPERLAP "fuelperlap"
#define BT_ATT_MUFACTOR "mufactor"
#define BT_ATT_PITTIME "pittime"
#define BT_ATT_BESTLAP "bestlap"
#define BT_ATT_WORSTLAP "worstlap"
#define BT_ATT_TEAMMATE "teammate"


class Opponents;
class Opponent;
class Pit;
class AbstractStrategy;

enum { normal=1, avoiding, correcting, pitting };
enum { TEAM_FRIEND=1, TEAM_FOE };
enum { avoidleft=1, avoidright=2, avoidside=4 };

class Driver {
	public:
		Driver(int index);
		~Driver();

		// Callback functions called from TORCS.
		void initTrack(tTrack* t, void *carHandle, void **carParmHandle, tSituation *s);
		void newRace(tCarElt* car, tSituation *s);
		void drive(tSituation *s);
		int pitCommand(tSituation *s);
		void endRace(tSituation *s);

		tCarElt *getCarPtr() { return car; }
		tTrack *getTrackPtr() { return track; }
		float getSpeed() { return mycardata->getSpeedInTrackDirection(); /*speed;*/ }

	private:
		// Utility functions.
		bool isStuck();
		void update(tSituation *s);
		float getAccel();
		float getDistToSegEnd();
		float getBrake();
		int getGear();
		float getSteer(tSituation *s);
		float getClutch();
		vec2f getTargetPoint();
		float getOffset();
		float brakedist(float allowedspeed, float mu);
		float smoothSteering( float steercmd );
		float correctSteering( float avoidsteer, float racesteer );
		float calcSteer( float targetAngle, int rl, float racesteer );
		void setMode( int newmode );
		float getWidth() { return mycardata->getWidthOnTrack(); }
		void calcSpeed();

		float filterOverlap(float accel);
		float filterBColl(float brake);
		float filterABS(float brake);
		float filterBPit(float brake);
		float filterBrakeSpeed(float brake);
		float filterTurnSpeed(float brake);

		float filterTCL(float accel);
		float filterTrk(float accel);

		float filterTCL_RWD();
		float filterTCL_FWD();
		float filterTCL_4WD();
		void initTCLfilter();

		void initCa();
		void initCw();
		void initTireMu();

		void computeRadius(float *radius);
		int isAlone();

		// Per robot global data.
		int mode;
		int avoidmode;
		int lastmode;
		int stuck;
		float speedangle;		// the angle of the speed vector relative to trackangle, > 0.0 points to right.
		float angle;
		float mass;				// Mass of car + fuel.
		float myoffset;			// Offset to the track middle.
		float laststeer;
		float lastNSasteer;
		tCarElt *car;			// Pointer to tCarElt struct.
		LRaceLine raceline;

		Opponents *opponents;	// The container for opponents.
		Opponent *opponent;		// The array of opponents.

		Pit *pit;						// Pointer to the pit instance.
		AbstractStrategy *strategy;		// Pit stop strategy.

		static Cardata *cardata;		// Data about all cars shared by all instances.
		SingleCardata *mycardata;		// Pointer to "global" data about my car.
		static double currentsimtime;	// Store time to avoid useless updates.

		double simtime;       // how long since the race started
		double avoidtime;    // how long since we began avoiding
		double correcttimer; // how long we've been correcting
		double correctlimit; // level of divergence with raceline steering
		double brakedelay;
		float currentspeedsqr;	// Square of the current speed_x.
		float clutchtime;		// Clutch timer.
		float oldlookahead;		// Lookahead for steering in the previous step.
		float racesteer;     // steer command to get to raceline
		float rlookahead;    // how far ahead on the track we look for steering
		float raceoffset;    // offset from middle of track towards which raceline is steering
		float avoidlftoffset; // closest opponent on the left
		float avoidrgtoffset; // closest opponent on the right
		float racespeed;     // how fast raceline code says we should be going
		float avoidspeed;    // how fast we should go if avoiding
		float accelcmd, brakecmd;  
		float PitOffset;
		v2d racetarget;      // the 2d point the raceline is driving at.

		float *radius;
		int alone;
		int carindex;

		// Data that should stay constant after first initialization.
		int MAX_UNSTUCK_COUNT;
		int INDEX;
		float CARMASS;		// Mass of the car only [kg].
		float CA;			// Aerodynamic downforce coefficient.
		float CW;			// Aerodynamic drag coefficient.
		float TIREMU;		// Friction coefficient of tires.
		float (Driver::*GET_DRIVEN_WHEEL_SPEED)();
		float OVERTAKE_OFFSET_INC;		// [m/timestep]
		float MU_FACTOR;				// [-]

		// Class constants.
		static const float MAX_UNSTUCK_ANGLE;
		static const float UNSTUCK_TIME_LIMIT;
		static const float MAX_UNSTUCK_SPEED;
		static const float MIN_UNSTUCK_DIST;
		static const float G;
		static const float FULL_ACCEL_MARGIN;
		static const float SHIFT;
		static const float SHIFT_MARGIN;
		static const float ABS_SLIP;
		static const float ABS_RANGE ;
		static const float ABS_MINSPEED;
		static const float TCL_SLIP;
		static const float LOOKAHEAD_CONST;
		static const float LOOKAHEAD_FACTOR;
		static const float WIDTHDIV;
		static const float SIDECOLL_MARGIN;
		static const float BORDER_OVERTAKE_MARGIN;
		static const float OVERTAKE_OFFSET_SPEED;
		static const float PIT_LOOKAHEAD;
		static const float PIT_BRAKE_AHEAD;
		static const float PIT_MU;
		static const float MAX_SPEED;
		static const float TCL_RANGE;
		static const float MAX_FUEL_PER_METER;
		static const float CLUTCH_SPEED;
		static const float CENTERDIV;
		static const float DISTCUTOFF;
		static const float MAX_INC_FACTOR;
		static const float CATCH_FACTOR;
		static const float CLUTCH_FULL_MAX_TIME;
		static const float USE_LEARNED_OFFSET_RANGE;

		static const float TEAM_REAR_DIST;
		static const int TEAM_DAMAGE_CHANGE_LEAD;

		// Track variables.
		tTrack* track;
};

#endif // _DRIVER_H_

