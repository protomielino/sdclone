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

//#include <track.h>      //tTrack
#include <raceman.h>    //tSituation
#include "spline.h"

class KDriver;
//class tTrack;

class Pit
{
public:
  Pit(const tSituation * s, KDriver * driver, const double PitOffset);
   ~Pit();

  void setPitstop(const bool pitstop);
  inline bool getPitstop() const {return m_pitstop;}
  inline void setInPit(const bool inpitlane) {m_inpitlane = inpitlane;}
  inline bool getInPit() const {return m_inpitlane;}

  double getPitOffset(const double offset, double fromstart);

  bool isBetween(const double fromstart) const;
  bool isTimeout(const double distance);

  inline double getNPitStart() const {return m_p[1].x;}
  inline double getNPitLoc()   const {return m_p[3].x;}
  inline double getNPitEnd()   const {return m_p[5].x;}
  inline double getNPitEntry() const {return m_p[0].x;}

  double toSplineCoord(double x) const;

  inline double getSpeedlimitSqr() const {return m_speedlimitsqr;}
  inline double getSpeedlimit() const {return m_speedlimit;}
  inline double getSpeedLimitBrake(const double speedsqr) const
    {return (speedsqr - m_speedlimitsqr)
      / (m_pitspeedlimitsqr - m_speedlimitsqr);}

  void update();

private:
  tTrack *m_track;
  tCarElt *m_car;
  tTrackOwnPit *m_mypit;    // Pointer to my pit.
  tTrackPitInfo *m_pitinfo; // General pit info.

  enum
  { NPOINTS = 7 };
  SplinePoint m_p[NPOINTS];   // Spline points.
  Spline *m_spline;       // Spline.

  bool m_pitstop;         // Pitstop planned.
  bool m_inpitlane;       // We are still in the pit lane.
  double m_pitentry;      // Distance to start line of the pit entry.
  double m_pitexit;       // Distance to the start line of the pit exit.

  double m_speedlimitsqr;     // Pit speed limit squared.
  double m_speedlimit;        // Pit speed limit.
  double m_pitspeedlimitsqr;  // The original speedlimit squared.

  double m_pittimer;       // Timer for pit timeouts.

  static const double SPEED_LIMIT_MARGIN;
};

#endif // _PIT_H_
