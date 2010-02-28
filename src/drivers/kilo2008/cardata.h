/*
 *      cardata.h
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

/*
    This class holds global facts about cars, therefore no data relative to
    each other (for that is the class Opponents/Opponent responsible).
*/

#ifndef _BT_CARDATA_H_
#define _BT_CARDATA_H_

#include <raceman.h>
#include <list>

#include "linalg.h"

class SingleCardata
{
public:
  void init(const CarElt * car);

  inline float getSpeedInTrackDirection() const { return speed; }
  inline float getWidthOnTrack() const { return width; }
  inline float getLengthOnTrack() const { return length; }
  inline float getTrackangle() const { return trackangle; }
  inline float getCarAngle() const { return angle; }
  inline bool thisCar(const tCarElt * car) const { return (car == this->car); }
  inline tPosd *getCorner1() { return corner1; }
  inline tPosd *getCorner2() { return corner2; }
  inline tPosd *getLastSpeed() { return lastspeed;}

  void update();
  //void operator() (void) {this->update();}
  
protected:
  static float getSpeed(const tCarElt * car, const float trackangle);

  float speed;          // speed in direction of the track.
  float width;          // the cars needed width on the track.
  float length;         // the cars needed length on the track.
  float trackangle;     // Track angle at the opponents position.
  float angle;          // The angle of the car relative to the track tangent.

  tPosd corner1[4];
  tPosd corner2[4];
  tPosd lastspeed[3];

  const tCarElt *car;         // For identification.
};


// TODO: use singleton pattern.
class Cardata
{
public:
  Cardata(tSituation * s);
  ~Cardata();

  void update() const;
  SingleCardata *findCar(const tCarElt * car) const;

protected:
  std::list<SingleCardata> *data; // List with car data.
};

#endif // _BT_CARDATA_H_
