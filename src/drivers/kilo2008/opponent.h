/*
 *      opponent.h
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

#ifndef _OPPONENT_H_
#define _OPPONENT_H_

#include "cardata.h"
#include <list>
#include <string>
using namespace std;

#define OPP_IGNORE      0
#define OPP_FRONT       (1<<0)
#define OPP_BACK        (1<<1)
#define OPP_SIDE        (1<<2)
#define OPP_COLL        (1<<3)
#define OPP_LETPASS     (1<<4)
#define OPP_FRONT_FAST  (1<<5)
#define OPP_FRONT_FOLLOW    (1<<6)


class Driver;

// Opponent maintains the data for one opponent RELATIVE to the drivers car.
class Opponent
{
public:
  Opponent(tCarElt *car, SingleCardata *cardata, int index);

  static void setTrackPtr(tTrack * const track) {Opponent::m_track = track;}

  tCarElt *getCarPtr() {return m_car;}
  float getDistance() {return m_distance;}
  float getWidth() {return m_cardata->getWidthOnTrack();}
  float getSpeed() {return m_cardata->getSpeedInTrackDirection();}
  int getIndex() {return m_index;}
  
  bool is_state(const int state) {return bool(m_state & state);}
  bool is_teammate() {return m_teammate;}
  bool is_quicker_teammate(tCarElt * const mycar);

  void markAsTeamMate() {m_teammate = true;}
  void update(tSituation * s, Driver * driver);

private:
  float getDistToSegStart();
  void updateOverlapTimer(tSituation * const s, tCarElt * const mycar);

  float m_distance;         // approximation of the real distance, negative if the opponent is behind.
  float m_brakedistance;    // distance minus opponent car length
  float m_catchdist;        // distance needed to catch the opponent (linear estimate).
  float m_sidedist;         // approx distance of center of gravity of the cars.
  int m_state;              // State variable to characterize the relation to the opponent, e. g. opponent is behind.
  int m_index;
  float m_overlaptimer;

  tCarElt *m_car;
  SingleCardata *m_cardata; // Pointer to global data about this opponent.
  bool m_teammate;          // Is this opponent a team mate of me (configure it in setup XML)?
  
  // class variables.
  static tTrack *m_track;

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


// The Opponents class holds a list of all Opponents.
// Uses STL's list template.
class Opponents
{
public:
  Opponents(tSituation *s, Driver *driver, Cardata *cardata);
  ~Opponents() {delete m_opps;}

  void update(tSituation *s, Driver *driver);
  void setTeamMate(const tCarElt *car);
  inline list<Opponent>::iterator begin() {return m_opps->begin();}
  inline list<Opponent>::iterator end() {return m_opps->end();}
  
private:
  list<Opponent> *m_opps;
};


#endif // _OPPONENT_H_
