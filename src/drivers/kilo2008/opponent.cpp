/*
 *      opponent.cpp
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

#include "opponent.h"

#include <iostream>
#include <algorithm>
#include "driver.h"
#include "util.h"   //Between

// class variables and constants.
tTrack *
  Opponent::m_track;
const double
  Opponent::FRONTCOLLDIST = 200.0; // [m] distance on the track ahead to check other cars.
const double
  Opponent::BACKCOLLDIST = -50.0;  // [m] distance on the track behind to check other cars.
const double
  Opponent::LENGTH_MARGIN = 1.0;   // [m] safety margin.
const double
  Opponent::SIDE_MARGIN = 1.0; // [m] safety margin.
const double
  Opponent::EXACT_DIST = 12.0; // [m] if the estimated distance is smaller, compute it more accurate
const double
  Opponent::LAP_BACK_TIME_PENALTY = -30.0; // [s]
const double
  Opponent::OVERLAP_WAIT_TIME = 5.0;   // [s] overlaptimer must reach this time before we let the opponent pass.
const double
  Opponent::SPEED_PASS_MARGIN = 5.0;   // [m/s] avoid overlapping opponents to stuck behind me.


//Constructor
Opponent::Opponent(tCarElt *car, SingleCardata * cardata, int index)
{
  m_car = car;
  m_cardata = cardata;
  m_index = index;
  m_teammate = false;
}


void
Opponent::update(tSituation *s, Driver *driver)
{
  // Init state of opponent to ignore.
  m_state = OPP_IGNORE;

  // If this car is out of the simulation or is in pits, ignore it.
  if((m_car->_state & RM_CAR_STATE_NO_SIMU) || (m_car->_state & RM_CAR_STATE_PIT))
      return;

  // Updating distance along the middle.
  tCarElt *mycar = driver->getCarPtr();
  double oppToStart = m_car->_trkPos.seg->lgfromstart + getDistToSegStart();
  m_distance = oppToStart - mycar->_distFromStartLine;
  if(m_distance > m_track->length / 2.0)
      m_distance -= m_track->length;
  else if(m_distance < -m_track->length / 2.0)
      m_distance += m_track->length;

  const double SIDECOLLDIST = MAX(m_car->_dimension_x, mycar->_dimension_x);

  // Is opponent in relevant range BACKCOLLDIST..FRONTCOLLDIST?
  if(BetweenStrict(m_distance, BACKCOLLDIST, FRONTCOLLDIST))
    {
      // Is opponent aside?
      if(BetweenStrict(m_distance, -SIDECOLLDIST, SIDECOLLDIST))
        {
          m_sidedist = m_car->_trkPos.toMiddle - mycar->_trkPos.toMiddle;
          m_state |= OPP_SIDE;
        }
      
      // Is opponent in front and slower?
      if(m_distance > SIDECOLLDIST && getSpeed() <= driver->getSpeed())
        {
          m_state |= OPP_FRONT;

          if(isQuickerTeammate(mycar))
            m_state |= OPP_FRONT_FOLLOW;

          m_distance -= SIDECOLLDIST;
          m_distance -= LENGTH_MARGIN;

          // If the distance is small we compute it more accurate.
          if(m_distance < EXACT_DIST)
            {
              straight2f carFrontLine(mycar->_corner_x(FRNT_LFT),
                      mycar->_corner_y(FRNT_LFT),
                      mycar->_corner_x(FRNT_RGT) -
                      mycar->_corner_x(FRNT_LFT),
                      mycar->_corner_y(FRNT_RGT) -
                      mycar->_corner_y(FRNT_LFT));

              double mindist = FLT_MAX;
              for(int i = 0; i < 4; i++)
                {
                  vec2f corner(m_car->_corner_x(i), m_car->_corner_y(i));
                  double dist = carFrontLine.dist(corner);
                  mindist = MIN(dist, mindist);
                }
              m_distance = MIN(m_distance, mindist);
            }

          m_catchdist =
            driver->getSpeed() * m_distance / (driver->getSpeed() - getSpeed());

          m_sidedist = m_car->_trkPos.toMiddle - mycar->_trkPos.toMiddle;
          double cardist = fabs(m_sidedist) - fabs(getWidth() / 2.0) -
            mycar->_dimension_y / 2.0;
          if(cardist < SIDE_MARGIN)
            m_state |= OPP_COLL;
        }
      // Is opponent behind and faster.
      else if(m_distance < -SIDECOLLDIST
          && getSpeed() > driver->getSpeed() - SPEED_PASS_MARGIN)
        {
          m_catchdist = driver->getSpeed() * m_distance / (getSpeed() - driver->getSpeed());
          m_state |= OPP_BACK;
          m_distance -= SIDECOLLDIST;
          m_distance -= LENGTH_MARGIN;
        }
      // Opponent is in front and faster.
      else if(m_distance > SIDECOLLDIST && getSpeed() > driver->getSpeed())
        {
          m_state |= OPP_FRONT_FAST;
          if(isQuickerTeammate(mycar))
            m_state |= OPP_FRONT_FOLLOW;
          m_distance -= SIDECOLLDIST;
          if (m_distance < 20.0 - (getSpeed() - driver->getSpeed()) * 4)
            m_state |= OPP_FRONT;
        }
    }

  // Check if we should let overtake the opponent.
  updateOverlapTimer(s, mycar);
  if(m_overlaptimer > OVERLAP_WAIT_TIME)
    m_state |= OPP_LETPASS;

  m_brakedistance = m_distance - m_car->_dimension_x;
}


// Compute the length to the start of the segment.
double
Opponent::getDistToSegStart() const
{
  double ret = (m_car->_trkPos.seg->type == TR_STR)
    ? m_car->_trkPos.toStart
    : m_car->_trkPos.toStart * m_car->_trkPos.seg->radius;
  return ret;
}//getDistToSegStart


// Update overlaptimers of opponents.
void
Opponent::updateOverlapTimer(tSituation * const s, tCarElt * const mycar)
{
  if((m_car->race.laps > mycar->race.laps) || isQuickerTeammate(mycar))
    {
      if(isState(OPP_BACK | OPP_SIDE))
        m_overlaptimer += s->deltaTime;
      else if(isState(OPP_FRONT))
        m_overlaptimer = LAP_BACK_TIME_PENALTY;
      else
        {
          if(m_overlaptimer > 0.0)
            {
              if(isState(OPP_FRONT_FAST))
                m_overlaptimer = MIN(0.0, m_overlaptimer);
              else
                m_overlaptimer -= s->deltaTime;
            }
          else
            m_overlaptimer += s->deltaTime;
        }
    }
  else
    m_overlaptimer = 0.0;
}


/**
 * Returns true, if the other car is our teammate
 * and has significantly less damage
 * (defined in Driver::TEAM_DAMAGE_CHANGE_LEAD)
 *
 * @param mycar pointer to the other car
 * @return true, if the opponent is our teammate
 */
bool
Opponent::isQuickerTeammate(tCarElt * const mycar)
{
  return (isTeammate()
    && (mycar->_dammage - m_car->_dammage > Driver::TEAM_DAMAGE_CHANGE_LEAD));
}//isQuickerTeammate


/** 
 * Constructor
 * Initializes the list of opponents.
 * Checks and doesn't store out own car as an opponent.
 * 
 * @param s Situation provided by TORCS
 * @param driver Our own robot
 * @param c Opponent car data
 */
Opponents::Opponents(tSituation *s, Driver *driver, Cardata *c)
{
  m_opps = new list<Opponent>;
  const tCarElt *ownCar = driver->getCarPtr();
  
  //Step through all the cars
  for(int i = 0; i < s->_ncars; i++)
    {
      //If it is not our own car
      if(s->cars[i] != ownCar)
        {
          //Create and set up new opponent
          Opponent opp(
            s->cars[i], //car ptr
            c->findCar(s->cars[i]), //car data ptr
            i //index
            ); 
          m_opps->push_back(opp);   //Store it in list
        }//if s->cars[i]
    }//for i
    
  Opponent::setTrackPtr(driver->getTrackPtr());
}//Opponents::Opponents


/** 
 * update
 * Makes every opponent update its own data.
 * 
 * @param   s   Situation provided by TORCS
 * @param   driver  Our own robot
 */
void
Opponents::update(tSituation *s, Driver *driver)
{
  //for_each(begin(), end(), update);
  for(list<Opponent>::iterator it = begin(); it != end(); it++)
    it->update(s, driver);
}//update


//for find()
inline bool operator==(const Opponent& o, const std::string s)
    { return !s.compare(o.getCarPtr()->_name); }
/** 
 * setTeamMate
 * Search the opponent list for our own teammate,
 * based on the teammate name given in the config as "teammate".
 * 
 * @param car Our own car, to read its config
 */
void
Opponents::setTeamMate(const tCarElt *car)
{
  string teammate(
    GfParmGetStr(car->_paramsHandle, BT_SECT_PRIV, BT_ATT_TEAMMATE, ""));

  list<Opponent>::iterator found = find(begin(), end(), teammate);
  if(found != end())
    found->markAsTeamMate();
}//setTeamMate


/** 
 * Searches the first opponent with the given state.
 *
 * @param [in]  state: we only care for an opponent in this state
 * @return      pointer to the opponent we've found, or NULL
 */
Opponent *
Opponents::getOppByState(const int state)
{
  Opponent *ret = NULL;
  for(list<Opponent>::iterator it = begin(); it != end(); it++) {
    if(it->isState(state)) {
      ret = &(*it);
      break;
    }
  }//for it
  return ret;
}//getOppByState
