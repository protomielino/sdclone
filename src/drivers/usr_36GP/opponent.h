/***************************************************************************

    file                 : opponent.h
    created              : Thu Apr 22 01:20:19 CET 2003
    copyright            : (C) 2003-2004 Bernhard Wymann
    email                : berniw@bluewin.ch
    version              : $Id: opponent.h,v 1.1 2008/02/11 00:53:10 andrew Exp $

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _OPPONENT_H_
#define _OPPONENT_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <tgf.h>
#include <track.h>
#include <car.h>
#include <raceman.h>
#include <robottools.h>
#include <robot.h>

#include "driver.h"
#include "cardata.h"

#define OPP_IGNORE		0
#define OPP_FRONT		(1<<0)
#define OPP_BACK		(1<<1)
#define OPP_SIDE		(1<<2)
#define OPP_COLL		(1<<3)
#define OPP_LETPASS		(1<<4)
#define OPP_FRONT_FAST	(1<<5)
#define OPP_FRONT_FOLLOW	(1<<6)
#define OPP_SIDE_COLL		(1<<7)


class Driver;

// Opponent maintains the data for one opponent RELATIVE to the drivers car.
class Opponent {
	public:
		Opponent();

		void setCarPtr(tCarElt *car) { this->car = car; }
		void setCarDataPtr(SingleCardata *cardata) { this->cardata = cardata; }
		static void setTrackPtr(tTrack *track) { Opponent::track = track; }

		tCarElt *getCarPtr() { return car; }
		int getState() { return state; }
		float getCatchDist() { return catchdist; }
		float getDistance() { return distance; }
		float getBrakeDistance() { return brakedistance; }
		float getSideDist() { return sidedist; }
		float getWidth() { return cardata->getWidthOnTrack(); }
		float getSpeed() { return cardata->getSpeedInTrackDirection(); }
		float getOverlapTimer() { return overlaptimer; }
		float getTrueSpeed() { return getSpeed(); /*cardata->getTrueSpeed();*/ }

		bool isTeamMate() { return teammate; }
		int getDamage() { return car->_dammage; }
		int getTeam() { return team; }
		double getNextLeft() { return nextleft; }
		void markAsTeamMate() { teammate = true; }
		void setIndex(int i) { index = i; }
		int getIndex() { return index; }
		float getTimeImpact() { return t_impact; }

		void update(tSituation *s, Driver *driver, int DebugMsg);
		float getSpeedAngle() { return speedangle; }
		float getAngle() { return angle; }

	private:
		float getDistToSegStart();
		int polyOverlap(tPosd *op, tPosd *dp);
		int testCollision(Driver *driver, double impact, double speedincr, vec2f *targ = NULL);
		void updateOverlapTimer(tSituation *s, tCarElt *mycar, int alone);
		float GetCloseDistance( float distn, tCarElt *mycar );

		float distance;		// approximation of the real distance, negative if the opponent is behind.
		float brakedistance;		// distance minus opponent car length
		float catchdist;	// distance needed to catch the opponent (linear estimate).
		float sidedist;		// approx distance of center of gravity of the cars.
		float deltamult;
		float speedangle;
		float prevspeedangle;
		float angle;
		float lastyr;
		float nextleft;
		float prevleft;
		float t_impact;
		int state;			// State variable to characterize the relation to the opponent, e. g. opponent is behind.
		int team;
		int teamindex;
		int index;
		float overlaptimer;

		tCarElt *car;
		SingleCardata *cardata;		// Pointer to global data about this opponent.
		bool teammate;				// Is this opponent a team mate of me (configure it in setup XML)?

		// class variables.
		static tTrack *track;

		// constants.
		static const float FRONTCOLLDIST;
		static const float BACKCOLLDIST;
		static const float LENGTH_MARGIN;
		static const float SIDE_MARGIN;
		static const float EXACT_DIST;
		static const float LAP_BACK_TIME_PENALTY;
		static const float OVERLAP_WAIT_TIME;
		static const float SPEED_PASS_MARGIN;
};


// The Opponents class holds an array of all Opponents.
class Opponents {
	public:
		Opponents(tSituation *s, Driver *driver, Cardata *cardata);
		~Opponents();

		void update(tSituation *s, Driver *driver, int DebugMsg);
		Opponent *getOpponentPtr() { return opponent; }
		int getNOpponents() { return nopponents; }
		void setTeamMate(const char *teammate);

	private:
		Opponent *opponent;
		int nopponents;
};


#endif // _OPPONENT_H_
