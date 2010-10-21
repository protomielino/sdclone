/*
 *      kdriver.h
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

#ifndef _KDRIVER_H_
#define _KDRIVER_H_

#include "cardata.h"
#include "linalg.h"     //v2d

#include <car.h>        //tCarElt
#include <raceman.h>    //tSituation

#include <string>

class Opponents;
class Opponent;
class Pit;
class KStrategy;
class LRaceLine;

//Custom setup features
#define KILO_SECT_PRIV      "KiloPrivate"
#define KILO_ATT_FUELPERLAP "FuelPerLap"
#define KILO_ATT_MUFACTOR   "MuFactor"
#define KILO_ATT_PITTIME    "PitTime"
#define KILO_ATT_BESTLAP    "BestLap"
#define KILO_ATT_WORSTLAP   "WorstLap"
#define KILO_ATT_TEAMMATE   "Teammate"
#define KILO_ATT_MINCORNER  "MinCornerInverse"
#define KILO_ATT_CORNERSP   "CornerSpeed"
#define KILO_ATT_AVOIDSP    "AvoidSpeedAdjust"
#define KILO_ATT_CORNERACC  "CornerAccel"
#define KILO_ATT_INTMARG    "IntMargin"
#define KILO_ATT_EXTMARG    "ExtMargin"
#define KILO_ATT_BRDELAY    "BrakeDelay"
#define KILO_ATT_SECRAD     "SecurityRadius"
#define KILO_ATT_PITOFFSET  "PitOffset"
#define KILO_SECT_SKILL     "Skill"
#define KILO_SKILL_LEVEL    "Level"
#define KILO_SKILL_AGGRO    "Aggression"


enum { NORMAL = 1, AVOIDING, CORRECTING, PITTING, BEING_OVERLAPPED };
enum { TEAM_FRIEND = 1, TEAM_FOE };
enum { AVOIDLEFT = 1, AVOIDRIGHT = 2, AVOIDSIDE = 4 };


class KDriver
{
public:
  KDriver(int index);
  virtual ~KDriver();

  //Callback functions for the sim
  void drive(tSituation * s);
  int pitCommand(tSituation * s);
  void initTrack(tTrack * t, void *carHandle, void **carParmHandle,
          tSituation * s);
  void newRace(tCarElt * car, tSituation * s);
  void endRace(tSituation * s);
  std::string bot;   //to make it possible to differentiate between Kilo & Dots

  //Used by Opponents
  tCarElt *getCarPtr()  { return m_car;}
  tTrack *getTrackPtr() { return m_track;}
  double getSpeed()     { return m_mycardata->getSpeedInTrackDirection(); }
  
protected:
  //Initialize
  void initCa();
  void initCw();
  void initTireMu();
  void initTCLFilter();
  double initSkill(tSituation * s);
  
  //Driving aids
  double filterTCL_RWD();
  double filterTCL_FWD();
  double filterTCL_4WD();
  double filterTCL(const double accel);
  double filterTrk(double accel);
  double brakeDist(double allowedspeed, double mu);
  double filterABS(double brake);
  double filterBPit(double brake);
  
  //Steering
  double getSteer(tSituation * s);
  double calcSteer(double targetAngle, int rl);  
  double correctSteering(double avoidsteer, double racesteer);
  double smoothSteering(double steercmd);
  vec2f getTargetPoint();
  
  //'own' utilities
  void update(tSituation * s);
  double filterBrakeSpeed(double brake);
  double filterBColl(double brake);
  double filterOverlap(double accel);
  void calcSpeed();
  void setAvoidRight() {m_avoidmode |= AVOIDRIGHT;}
  void setAvoidLeft() {m_avoidmode |= AVOIDLEFT;}
  void setMode(int newmode);
  bool oppTooFarOnSide(tCarElt *ocar);
  bool isStuck();
  double getDistToSegEnd();
  double getOffset();
  double getAccel();
  double getBrake();
  int getGear();
  double getClutch();
  double getWidth() { return m_mycardata->getWidthOnTrack();}
  void checkPitStatus(tSituation *s);
  void * loadDefaultSetup() const;
  void mergeCarSetups(void **oldHandle, void *newHandle);

  //Opponent handling  
  Opponent * getOverlappingOpp();
  Opponent * getTakeoverOpp();
  Opponent * getSidecollOpp();
  double filterOverlappedOffset(Opponent *o);
  double filterTakeoverOffset(Opponent *o);
  double filterSidecollOffset(Opponent *o, const double);
  
  
  // Constants.
  enum { NORMAL = 1, AVOIDING, CORRECTING, PITTING, OVERLAPPED };
  enum { TEAM_FRIEND = 1, TEAM_FOE };
  enum { AVOIDLEFT = 1, AVOIDRIGHT = 2, AVOIDSIDE = 4 };

  tCarElt *m_car;         // Pointer to tCarElt struct.
  LRaceLine *m_raceline;
  Opponents *m_opponents; // The container for opponents.
  Pit *m_pit;             // Pointer to the pit instance.
  KStrategy *m_strategy;  // Pit stop strategy.
  tTrack *m_track;        // Track variables.

  static Cardata *m_cardata;  // Data about all cars shared by all instances.
  SingleCardata *m_mycardata; // Pointer to "global" data about my car.
  
  static double m_currentSimTime; // Store time to avoid useless updates.

  //Per robot global data
  int m_mode;
  int m_avoidmode;
  int m_lastmode;
  int m_stuckCounter;
  double m_speedangle;  // the angle of the speed vector relative to trackangle, > 0.0 points to right.
  double m_angle;
  double m_mass;        // Mass of car + fuel.
  double m_myoffset;    // Offset to the track middle.
  double m_laststeer;
  double m_lastNSasteer;
  double m_simTime;         // how long since the race started
  double m_avoidTime;       // how long since we began avoiding
  double m_correctTimer;    // how long we've been correcting
  double m_correctLimit;    // level of divergence with raceline steering
  double m_brakeDelay;      
  double m_currentSpeedSqr; // Square of the current speed_x.
  double m_clutchTime;      // Clutch timer.
  double m_oldLookahead;    // Lookahead for steering in the previous step.
  double m_raceSteer;       // steer command to get to raceline
  double m_rLookahead;      // how far ahead on the track we look for steering
  double m_raceOffset;      // offset from middle of track towards which raceline is steering
  double m_avoidLftOffset;  // closest opponent on the left
  double m_avoidRgtOffset;  // closest opponent on the right
  double m_raceSpeed;       // how fast raceline code says we should be going
  double m_avoidSpeed;      // how fast we should go if avoiding
  double m_accelCmd;
  double m_brakeCmd;
  double m_pitOffset;
  v2d    m_raceTarget;      // the 2d point the raceline is driving at.
  double m_mincatchdist;
  double m_rgtinc;
  double m_lftinc;
  double m_maxoffset;
  double m_minoffset;
  double m_rInverse;
  std::string m_carType;
  int m_carIndex;
  
  //Skilling
  double m_skill;
  double m_filterBrakeSkill;
  double m_filterAccelSkill;
  double m_filterLookaheadSkill;
  double m_filterSideSkill;
  
  // Data that should stay constant after first initialization.
  int MAX_UNSTUCK_COUNT;
  int INDEX;
  double CARMASS;            // Mass of the car only [kg].
  double CA;                 // Aerodynamic downforce coefficient.
  double CW;                 // Aerodynamic drag coefficient.
  double TIREMU;             // Friction coefficient of tires.
  double OVERTAKE_OFFSET_INC;    // [m/timestep]
  double MU_FACTOR;          // [-]
  double (KDriver::*GET_DRIVEN_WHEEL_SPEED) ();

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
public:
  static const int TEAM_DAMAGE_CHANGE_LEAD; //Used in opponent.cpp too
};

#endif // _KDRIVER_H_
