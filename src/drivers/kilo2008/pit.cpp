/*
 *      pit.cpp
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

#include "pit.h"

#include "kdriver.h"

const double
  Pit::SPEED_LIMIT_MARGIN = 0.5;    // [m/s] safety margin to avoid pit speeding.


Pit::Pit(const tSituation * s, KDriver * driver, const double pitoffset)
{
  m_track = driver->getTrackPtr();
  m_car = driver->getCarPtr();
  m_mypit = driver->getCarPtr()->_pit;
  m_pitinfo = &m_track->pits;
  m_pitstop = m_inpitlane = false;
  m_pittimer = 0.0;

  if(m_mypit != NULL) {
    m_speedlimit = m_pitinfo->speedLimit - SPEED_LIMIT_MARGIN;
    m_speedlimitsqr = pow(m_speedlimit, 2);
    m_pitspeedlimitsqr = pow(m_pitinfo->speedLimit, 2);

    // Compute pit spline points along the track.
    m_p[3].x = m_mypit->pos.seg->lgfromstart + m_mypit->pos.toStart;
    m_p[2].x = m_p[3].x - m_pitinfo->len;
    m_p[4].x = m_p[3].x + m_pitinfo->len;
    m_p[0].x = m_pitinfo->pitEntry->lgfromstart + pitoffset;
    m_p[1].x = m_pitinfo->pitStart->lgfromstart;
    m_p[5].x = m_pitinfo->pitStart->lgfromstart + m_pitinfo->nPitSeg * m_pitinfo->len; // Use nPitSeg to respect the pit speed limit on Migrants e.a.
    m_p[6].x = m_pitinfo->pitExit->lgfromstart;

    m_pitentry = m_p[0].x;
    m_pitexit = m_p[6].x;

    // Normalizing spline segments to >= 0.0.
    for(int i = 0; i < NPOINTS; i++) {
      m_p[i].s = 0.0;
      m_p[i].x = toSplineCoord(m_p[i].x);
    }//for i

    // Fix broken pit exit.
    if(m_p[6].x < m_p[5].x)
      m_p[6].x = m_p[5].x + 50.0;

    // Fix point for first pit if necessary.
    if(m_p[1].x > m_p[2].x)
      m_p[1].x = m_p[2].x;

    // Fix point for last pit if necessary.
    if(m_p[4].x > m_p[5].x)
      m_p[5].x = m_p[4].x;

    double sign = (m_pitinfo->side == TR_LFT) ? 1.0 : -1.0;
    m_p[0].y = 0.0;
    m_p[6].y = 0.0;
    for(int i = 1; i < NPOINTS - 1; i++) {
      m_p[i].y = fabs(m_pitinfo->driversPits->pos.toMiddle) - m_pitinfo->width;
      m_p[i].y *= sign;
    }//for i

    m_p[3].y = fabs(m_pitinfo->driversPits->pos.toMiddle + 1.0) * sign;
    m_spline = new Spline(NPOINTS, m_p);
  }//if pit not null
}//Pit::Pit


Pit::~Pit()
{
  if(m_mypit != NULL)
    delete m_spline;
}//Pit::~Pit


// Transforms track coordinates to spline parameter coordinates.
double
Pit::toSplineCoord(const double x) const
{
  double ret = x - m_pitentry;
  while(ret < 0.0)
    ret += m_track->length;
  
  return ret;
}//toSplineCoord


// Computes offset to track middle for trajectory.
double
Pit::getPitOffset(const double offset, double fromstart)
{
  if(m_mypit != NULL) {
    if(getInPit() || (getPitstop() && isBetween(fromstart))) {
      fromstart = toSplineCoord(fromstart);
      return m_spline->evaluate(fromstart);
    }
  }
  return offset;
}//getPitOffset


// Sets the pitstop flag if we are not in the pit range.
void
Pit::setPitstop(bool pitstop)
{
  if(m_mypit != NULL) {
    double fromstart = m_car->_distFromStartLine;

    if(!isBetween(fromstart)) {
      m_pitstop = pitstop;
    } else {
      if(!pitstop) {
        m_pitstop = pitstop;
        m_pittimer = 0.0;
      }
    }
  }
}//setPitstop


// Check if the argument fromstart is in the range of the pit.
bool
Pit::isBetween(const double fromstart) const
{
  bool ret = false;
  
  if(m_pitentry <= m_pitexit) {
    if(fromstart >= m_pitentry && fromstart <= m_pitexit)
      ret = true;
  } else {
    // Warning: TORCS reports sometimes negative values for "fromstart"!
    if(fromstart <= m_pitexit || fromstart >= m_pitentry)
      ret = true;
  }//if pitentry <= pitexit
  
  return ret;
}//isBetween


// Checks if we stay too long without getting captured by the pit.
// Distance is the distance to the pit along the track, when the pit is
// ahead it is > 0, if we overshoot the pit it is < 0.
bool
Pit::isTimeout(const double distance)
{
  bool ret = false;
  
  if(m_car->_speed_x > 1.0 || distance > 3.0 || !getPitstop()) {
    m_pittimer = 0.0;
  } else {
    m_pittimer += RCM_MAX_DT_ROBOTS;
    if(m_pittimer > 3.0) {
      m_pittimer = 0.0;
      ret = true;
    }
  }
  
  return ret;
}//isTimeout


// Update pit data and strategy.
void
Pit::update()
{
  if(m_mypit != NULL) {
    if(isBetween(m_car->_distFromStartLine)) {
      if(getPitstop())
        setInPit(true);
    } else {
      setInPit(false);
    }

    if(getPitstop())
      m_car->_raceCmd = RM_CMD_PIT_ASKED;
  }
}//update
