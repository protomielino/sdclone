/***************************************************************************

    file                 : opponent.h
    created              : Thu Apr 22 01:20:19 CET 2003
    copyright            : (C) 2003-2004 Bernhard Wymann
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
//#include <portability.h>

#include "driver.h"
#include "cardata.h"

class Driver;

// Opponent maintains the data for one opponent RELATIVE to the drivers car.
class Opponent
{
    public:
        Opponent();

        void setCarPtr(tCarElt *car) { this->car = car; }
        void setCarDataPtr(SingleCardata *cardata) { this->cardata = cardata; }
        static void setTrackPtr(tTrack *track) { Opponent::track = track; }

        inline tCarElt *getCarPtr() { return car; }
        inline int getState() { return state; }
        inline float getCatchDist() { return catchdist; }
        inline double getDistance() { return distance; }
        inline float getBrakeDistance() { return brakedistance; }
        inline float getSideDist() { return sidedist; }
        inline float getWidth() { return cardata->getWidthOnTrack(); }
        inline float getSpeed() { return cardata->getSpeedInTrackDirection(); }
        inline float getOverlapTimer() { return (float)overlaptimer; }
        inline float getBrakeOvertake () { return brake_overtake_filter; }
        inline float getCollSpeed () { return collspeed; }
        inline double getTimeImpact() { return t_impact; }
        inline double getTimeToSide() { return t_toside; }
        inline double getCurrentLft() { return currentLft; }
        inline double getCurrentRgt() { return currentRgt; }
        inline double getImpactLft(double timpact) { return currentLft + getAvgLateralMovt() * MIN(3.0, timpact) / deltaTime; }
        inline double getImpactRgt(double timpact) { return currentRgt + getAvgLateralMovt() * MIN(3.0, timpact) / deltaTime; }
        inline double getImpactLft() { return impactLft; }
        inline double getImpactRgt() { return impactRgt; }
        inline double getSideSpeed() { return prevToLeft - toLeft; }
        inline double getTrueSpeedDiff() { return trueSpeedDiff; }
        inline double getProjectedSpeedAngle(double time) { return speedAngle[0] + (speedAngle[0] - speedAngle[3]) * (time/(4*deltaTime))*0.5; }
        inline bool getWithinBrakeDist() { return withinBrakeDist; }
        inline bool getHasSlowerSpeed() { return hasSlowerSpeed; }

        inline bool isTeamMate() { return teammate; }
        inline int getDamage() { return car->_dammage; }
        inline int getTeam() { return team; }
        void markAsTeamMate() { teammate = true; }
        inline void setIndex(int i) { index = i; }
        inline int getIndex() { return index; }
        inline float getSpeedAngle() { return speedangle; }
        inline float getAvgLateralMovt() { return (float)avgLateralMovt; }

        //void init (tCarElt *ocar, tCarElt *mcar);
        void update(tSituation *s, Driver *driver);
        void initState();
        void calcState(float Speed, Driver *driver);
        void calcDist();
        void calcSpeed();
        double getSpeedAngle(double angle);
        double getAvgAccelX() { return avgAccelX; }
        float getCornerDist();
        float getCornerDist(tCarElt *car1, tCarElt *car2);
        double speed;
        float oppSpeed;
        float mySpeed;
        int state;        // The relation to the opponent (E.G opponent is in front me or behind...):
        bool oppFront;
        bool oppBehind;
        bool oppFaraway;
        bool oppSlower;
        bool oppFaster;
        bool oppLetPass;
        double brake_multiplier;
        double brake_warn_multiplier;
        double deltamult;
        double deltaTime;
        double simTime;
        double left_toMid;
        double right_toMid;

    private:
        float getDistToSegStart(tCarElt *theCar);
        void updateOverlapTimer(tSituation *s, tCarElt *mycar);
        int polyOverlap(tPosd *op, tPosd *dp);
        int testLinearCollision2(Driver *driver);
        int testRacelineCollision(Driver *driver, double distance, double t_impact, double future_left = -999, double future_right = -999);
        int testQuadraticCollision(Driver *driver);
        int testCalculatedCollision(Driver *driver);
        double CalcCollSpeed(Driver *driver);

        double distance;        // approximation of the real distance, negative if the opponent is behind.
        double distance2;
        double distance3;
        float brakedistance;        // distance minus opponent car length
        float catchdist;    // distance needed to catch the opponent (linear estimate).
        float sidedist;        // approx distance of center of gravity of the cars.
        double t_impact;
        double t_toside;
        float collspeed;
        double prev_speed_X;
        double prev_speed_Y;
        double d_prev_speed_X;
        double d_prev_speed_Y;
        double average_AX;
        double average_AY;
        double avgLateralMovt;
        double avgAccelX;
        double prevToLeft;
        double speedAngle[4];
        double speedDelta;
        double prevSpeed;

        double currentLft;
        double currentRgt;
        double impactLft;
        double impactRgt;

        double left_speed_y;
        double right_speed_y;
        double last_left_toMid;
        double last_right_toMid;

        float brake_overtake_filter;

        int team;
        int index;
        double overlaptimer;
        float speedangle;
        float relativeangle;
        double toLeft;

        bool hasSlowerSpeed;

        double trueSpeedDiff;

        tCarElt *car;            // pointer to the opponents car
        tCarElt *mycar;          // pointer to the my car
        SingleCardata *cardata;        // Pointer to global data about this opponent.
        bool teammate;            // Is this opponent a team mate of me (configure it in setup XML)?
        bool firstTime;
        bool withinBrakeDist;

        // class variables.
        static tTrack *track;

        // constants.
        double FRONTCOLL_MARGIN;
        double SIDECOLL_MARGIN;
        static const float FRONTCOLLDIST;
        static const float BACKCOLLDIST;

        static const float EXACT_DIST;
        static const float LAP_BACK_TIME_PENALTY;
        static const float OVERLAP_WAIT_TIME;
        static const float SPEED_PASS_MARGIN;
        static const int MAX_DAMAGE_DIFF;
};


// The Opponents class holds an array of all Opponents.
class Opponents {
    public:
        Opponents(tSituation *s, Driver *driver, Cardata *cardata, double brake_multiplier, double brake_warn_multiplier);
        ~Opponents();

        void update(tSituation *s, Driver *driver);
        Opponent *getOpponentPtr() { return opponent; }
        int getNOpponents() { return nopponents; }
        void setTeamMate(const char *teammate);
        tCarElt *getTeamMateCar();
        bool isBehindFriend(int pos);

    private:
        Opponent *opponent;
        int nopponents;
};


#endif // _OPPONENT_H_
