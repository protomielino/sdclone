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

#include <car.h>        //tCarElt
#include <raceman.h>    //tSituation
#include "cardata.h"
#include <list>
#include <string>
using namespace std;

class KDriver;

#define OPP_IGNORE          0
#define OPP_FRONT           (1<<0)
#define OPP_BACK            (1<<1)
#define OPP_SIDE            (1<<2)
#define OPP_COLL            (1<<3)
#define OPP_LETPASS         (1<<4)
#define OPP_FRONT_FAST      (1<<5)
#define OPP_FRONT_FOLLOW    (1<<6)


// Opponent maintains the data for one opponent RELATIVE to the driver's car.
class Opponent
{
public:
  Opponent(tCarElt *car, SingleCardata *cardata, int index);

  static void setTrackPtr(tTrack * const track) {Opponent::m_track = track;}

  tCarElt *getCarPtr() const {return m_car;}
  double getDistance() const {return m_distance;}
  double getWidth() const {return m_cardata->getWidthOnTrack();}
  double getSpeed() const {return m_cardata->getSpeedInTrackDirection();}
  int getIndex() const {return m_index;}
  
  inline bool isState(const int state) const {return bool(m_state & state);}
  inline bool isTeammate() const {return m_teammate;}
  bool isQuickerTeammate(tCarElt * const mycar);
  inline bool isOnRight(const double dMiddle)
    {return (dMiddle > m_car->_trkPos.toMiddle) ? true : false;}

  inline void markAsTeamMate() {m_teammate = true;}
  void update(tSituation *s, KDriver *driver);

private:
  double getDistToSegStart() const;
  void updateOverlapTimer(tSituation * const s, tCarElt * const mycar);

  double m_distance;         // approximation of the real distance, negative if the opponent is behind.
  double m_brakedistance;    // distance minus opponent car length
  double m_catchdist;        // distance needed to catch the opponent (linear estimate).
  double m_sidedist;         // approx distance of center of gravity of the cars.
  int m_state;              // State variable to characterize the relation to the opponent, e. g. opponent is behind.
  int m_index;
  double m_overlaptimer;

  tCarElt *m_car;
  SingleCardata *m_cardata; // Pointer to global data about this opponent.
  bool m_teammate;          // Is this opponent a team mate of me (configure it in setup XML)?
  
  // class variables.
  static tTrack *m_track;

  // constants.
  static const double FRONTCOLLDIST;
  static const double BACKCOLLDIST;
  static const double LENGTH_MARGIN;
  static const double SIDE_MARGIN;
  static const double EXACT_DIST;
  static const double LAP_BACK_TIME_PENALTY;
  static const double OVERLAP_WAIT_TIME;
  static const double SPEED_PASS_MARGIN;
};


// The Opponents class holds a list of all Opponents.
// Uses STL's list template.
class Opponents
{
public:
  Opponents(tSituation *s, KDriver *driver, Cardata *cardata);
  ~Opponents() {delete m_opps;}

  void update(tSituation *s, KDriver *driver);
  void setTeamMate(const tCarElt *car);
  Opponent *getOppByState(const int state);
  
  inline list<Opponent>::iterator begin() {return m_opps->begin();}
  inline list<Opponent>::iterator end() {return m_opps->end();}
  
private:
  list<Opponent> *m_opps;
};


#endif // _OPPONENT_H_
