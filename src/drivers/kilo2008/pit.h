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

#include <track.h>      //tTrack
#include <raceman.h>    //tSituation
#include "spline.h"

class Driver;

class Pit
{
public:
  Pit(const tSituation * s, Driver * driver, const double PitOffset);
   ~Pit();

  void setPitstop(const bool pitstop);
  inline bool getPitstop() const {return pitstop;}
  inline void setInPit(const bool inpitlane) {this->inpitlane = inpitlane;}
  inline bool getInPit() const {return inpitlane;}

  double getPitOffset(const double offset, double fromstart);

  bool isBetween(const double fromstart) const;
  bool isTimeout(const double distance);

  inline double getNPitStart() const {return p[1].x;}
  inline double getNPitLoc()   const {return p[3].x;}
  inline double getNPitEnd()   const {return p[5].x;}
  inline double getNPitEntry() const {return p[0].x;}

  double toSplineCoord(double x) const;

  inline double getSpeedlimitSqr() const {return speedlimitsqr;}
  inline double getSpeedlimit() const {return speedlimit;}
  inline double getSpeedLimitBrake(const double speedsqr) const
    {return (speedsqr - speedlimitsqr) / (pitspeedlimitsqr - speedlimitsqr);}

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
  double pitentry;      // Distance to start line of the pit entry.
  double pitexit;       // Distance to the start line of the pit exit.

  double speedlimitsqr;     // Pit speed limit squared.
  double speedlimit;        // Pit speed limit.
  double pitspeedlimitsqr;  // The original speedlimit squared.

  double pittimer;       // Timer for pit timeouts.

  static const double SPEED_LIMIT_MARGIN;
};

#endif // _PIT_H_
