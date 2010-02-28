/*
 *      pit.h
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

#ifndef _PIT_H_
#define _PIT_H_

#include "driver.h"
#include "spline.h"

class Pit
{
public:
  Pit(tSituation * s, Driver * driver, float PitOffset);
   ~Pit();

  void setPitstop(bool pitstop);
  bool getPitstop() {return pitstop;}
  void setInPit(bool inpitlane) {this->inpitlane = inpitlane;}
  bool getInPit() {return inpitlane;}

  float getPitOffset(float offset, float fromstart);

  bool isBetween(float fromstart);
  bool isTimeout(float distance);

  float getNPitStart() {return p[1].x;}
  float getNPitLoc()   {return p[3].x;}
  float getNPitEnd()   {return p[5].x;}
  float getNPitEntry() {return p[0].x;}

  float toSplineCoord(float x);

  float getSpeedlimitSqr() {return speedlimitsqr;}
  float getSpeedlimit() {return speedlimit;}
  float getSpeedLimitBrake(float speedsqr);

  void update();

private:
  tTrack * track;
  tCarElt *car;
  tTrackOwnPit *mypit;      // Pointer to my pit.
  tTrackPitInfo *pitinfo;   // General pit info.

  enum
  { NPOINTS = 7 };
  SplinePoint p[NPOINTS];   // Spline points.
  Spline *spline;       // Spline.

  bool pitstop;         // Pitstop planned.
  bool inpitlane;       // We are still in the pit lane.
  float pitentry;       // Distance to start line of the pit entry.
  float pitexit;        // Distance to the start line of the pit exit.

  float speedlimitsqr;      // Pit speed limit squared.
  float speedlimit;     // Pit speed limit.
  float pitspeedlimitsqr;   // The original speedlimit squared.

  float pittimer;       // Timer for pit timeouts.

  static const float SPEED_LIMIT_MARGIN;
};

#endif // _PIT_H_
