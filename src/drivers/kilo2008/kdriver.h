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

#include "driver.h" //class Driver
#include <string>

class KDriver: public Driver
{
public:
  KDriver(int index);
  virtual ~KDriver() {};

  //Callback for TORCS
  void drive(tSituation * s);
  int pitCommand(tSituation * s);
  void initTrack(tTrack * t, void *carHandle, void **carParmHandle,
          tSituation * s);
  void newRace(tCarElt * car, tSituation * s);
  std::string bot;   //to make it possible to differentiate between Kilo & Dots
  
protected:
  //inherited from Driver
  bool isStuck();
  void update(tSituation * s);
  double filterBrakeSpeed(double brake);
  double filterBColl(double brake);
  double filterOverlap(double accel);
  double getOffset();
  void calcSpeed();
  void setAvoidRight() {avoidmode |= AVOIDRIGHT;}
  void setAvoidLeft() {avoidmode |= AVOIDLEFT;}
  bool oppTooFarOnSide(tCarElt *ocar);
  
  //'own' utilities
  Opponent * getOverlappingOpp();
  Opponent * getTakeoverOpp();
  Opponent * getSidecollOpp();
  double filterOverlappedOffset(Opponent *o);
  double filterTakeoverOffset(Opponent *o);
  double filterSidecollOffset(Opponent *o, const double);
  void checkPitStatus(tSituation *s);
    
  //'own' variables
  double m_mincatchdist;
  double m_rgtinc;
  double m_lftinc;
  double m_maxoffset;
  double m_minoffset;
  double m_rInverse;
  std::string m_carType;
};

#endif // _KDRIVER_H_
