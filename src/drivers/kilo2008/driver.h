/*
 *      driver.h
 *      
 *      Copyright 2009 kilo aka Gabor Kmetyko <kg.kilo@gmail.com>
 *      Based on work by Bernhard Wymann and Andrew Sumner.
 *      
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *      
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *      
 *      You should have received a copy of the GNU General Public License
 *      along with this program; if not, write to the Free Software
 *      Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *      MA 02110-1301, USA.
 * 
 *      $Id$
 * 
 */

#ifndef _DRIVER_H_
#define _DRIVER_H_

#include "cardata.h"
#include "linalg.h"     //v2d

#include <track.h>      //tTrack
#include <car.h>        //tCarElt
#include <raceman.h>    //tSituation

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
class KStrategy;
class LRaceLine;

enum
{ NORMAL = 1, AVOIDING, CORRECTING, PITTING, BEING_OVERLAPPED };
enum
{ TEAM_FRIEND = 1, TEAM_FOE };
enum
{ AVOIDLEFT = 1, AVOIDRIGHT = 2, AVOIDSIDE = 4 };

class Driver
{
public:
  Driver(int index);
  virtual ~Driver();

  // Callback functions called from TORCS.
  virtual void newRace(tCarElt * car, tSituation * s);
  virtual void drive(tSituation * s);
  virtual int pitCommand(tSituation * s) = 0;
  void endRace(tSituation * s);

  tCarElt *getCarPtr()  { return car;}
  tTrack *getTrackPtr() { return track;}
  double getSpeed()     { return mycardata->getSpeedInTrackDirection(); }

  static const int TEAM_DAMAGE_CHANGE_LEAD; //Used in opponent.cpp too

protected:

  // Constants.
  enum { NORMAL = 1, AVOIDING, CORRECTING, PITTING, OVERLAPPED };
  enum { TEAM_FRIEND = 1, TEAM_FOE };
  enum { AVOIDLEFT = 1, AVOIDRIGHT = 2, AVOIDSIDE = 4 };

  // Utility functions.
  virtual bool isStuck();
  virtual void update(tSituation *) = 0;
  double getAccel();
  double getDistToSegEnd();
  double getBrake();
  int getGear();
  double getSteer(tSituation * s);
  double getClutch();
  vec2f getTargetPoint();
  virtual double getOffset() = 0;
  double brakedist(double allowedspeed, double mu);
  double smoothSteering(double steercmd);
  double correctSteering(double avoidsteer, double racesteer);
  double calcSteer(double targetAngle, int rl, double racesteer);
  void setMode(int newmode);
  double getWidth()      { return mycardata->getWidthOnTrack();}
  virtual void calcSpeed();
  virtual void setAvoidRight() = 0;
  virtual void setAvoidLeft() = 0;
  virtual bool oppTooFarOnSide(tCarElt *) = 0;

  virtual double filterOverlap(double accel) = 0;
  virtual double filterBColl(double brake) = 0;
  double filterABS(double brake);
  double filterBPit(double brake);
  virtual double filterBrakeSpeed(double brake);
  double filterTurnSpeed(double brake);

  double filterTCL(const double accel);
  void initTCLfilter();
  double filterTCL_RWD();
  double filterTCL_FWD();
  double filterTCL_4WD();
  double filterTrk(double accel);

  void initCa();
  void initCw();
  void initTireMu();

  // Per robot global data.
  int mode;
  int avoidmode;
  int lastmode;
  int stuckCounter;
  double speedangle;     // the angle of the speed vector relative to trackangle, > 0.0 points to right.
  double angle;
  double mass;           // Mass of car + fuel.
  double myoffset;       // Offset to the track middle.
  double laststeer;
  double lastNSasteer;
  tCarElt *car;         // Pointer to tCarElt struct.
  LRaceLine *raceline;

  Opponents *opponents;     // The container for opponents.
  Pit *pit;         // Pointer to the pit instance.
  KStrategy *strategy;   // Pit stop strategy.

  static Cardata *cardata;  // Data about all cars shared by all instances.
  SingleCardata *mycardata; // Pointer to "global" data about my car.
  static double currentsimtime; // Store time to avoid useless updates.

  double simtime;           // how long since the race started
  double avoidtime;         // how long since we began avoiding
  double correcttimer;      // how long we've been correcting
  double correctlimit;      // level of divergence with raceline steering
  double brakedelay;
  double currentspeedsqr;    // Square of the current speed_x.
  double clutchtime;         // Clutch timer.
  double oldlookahead;       // Lookahead for steering in the previous step.
  double racesteer;          // steer command to get to raceline
  double rlookahead;         // how far ahead on the track we look for steering
  double raceoffset;         // offset from middle of track towards which raceline is steering
  double avoidlftoffset;     // closest opponent on the left
  double avoidrgtoffset;     // closest opponent on the right
  double racespeed;          // how fast raceline code says we should be going
  double avoidspeed;         // how fast we should go if avoiding
  double accelcmd;
  double brakecmd;
  double PitOffset;
  v2d racetarget;           // the 2d point the raceline is driving at.

  int carindex;

  // Data that should stay constant after first initialization.
  int MAX_UNSTUCK_COUNT;
  int INDEX;
  double CARMASS;            // Mass of the car only [kg].
  double CA;                 // Aerodynamic downforce coefficient.
  double CW;                 // Aerodynamic drag coefficient.
  double TIREMU;             // Friction coefficient of tires.
  double (Driver::*GET_DRIVEN_WHEEL_SPEED) ();
  double OVERTAKE_OFFSET_INC;    // [m/timestep]
  double MU_FACTOR;          // [-]

  // Class constants.
  static const double MAX_UNSTUCK_ANGLE;
  static const double UNSTUCK_TIME_LIMIT;
  static const double MAX_UNSTUCK_SPEED;
  static const double MIN_UNSTUCK_DIST;
  static const double G;
  static const double SHIFT;
  static const double SHIFT_MARGIN;
  static const double ABS_SLIP;
  static const double ABS_RANGE;
  static const double ABS_MINSPEED;
  static const double TCL_SLIP;
  static const double LOOKAHEAD_CONST;
  static const double LOOKAHEAD_FACTOR;
  static const double WIDTHDIV;
  static const double BORDER_OVERTAKE_MARGIN;
  static const double OVERTAKE_OFFSET_SPEED;
  static const double PIT_LOOKAHEAD;
  static const double PIT_BRAKE_AHEAD;
  static const double PIT_MU;
  static const double MAX_SPEED;
  static const double TCL_RANGE;
  static const double CLUTCH_SPEED;
  static const double DISTCUTOFF;
  static const double MAX_INC_FACTOR;
  static const double CATCH_FACTOR;

  static const double TEAM_REAR_DIST;
  static const double LET_OVERTAKE_FACTOR;

  // Track variables.
  tTrack *track;
};

#endif // _DRIVER_H_
