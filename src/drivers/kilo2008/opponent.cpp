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

#include "driver.h"

// class variables and constants.
tTrack *
  Opponent::m_track;
const float
  Opponent::FRONTCOLLDIST = 200.0f; // [m] distance on the track to check other cars.
const float
  Opponent::BACKCOLLDIST = 50.0f;   // [m] distance on the track to check other cars.
const float
  Opponent::LENGTH_MARGIN = 1.0f;   // [m] safety margin.
const float
  Opponent::SIDE_MARGIN = 1.0f; // [m] safety margin.
const float
  Opponent::EXACT_DIST = 12.0f; // [m] if the estimated distance is smaller, compute it more accurate
const float
  Opponent::LAP_BACK_TIME_PENALTY = -30.0f; // [s]
const float
  Opponent::OVERLAP_WAIT_TIME = 5.0f;   // [s] overlaptimer must reach this time before we let the opponent pass.
const float
  Opponent::SPEED_PASS_MARGIN = 5.0f;   // [m/s] avoid overlapping opponents to stuck behind me.


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
  float oppToStart = m_car->_trkPos.seg->lgfromstart + getDistToSegStart();
  m_distance = oppToStart - mycar->_distFromStartLine;
  if(m_distance > m_track->length / 2.0f)
      m_distance -= m_track->length;
  else if(m_distance < -m_track->length / 2.0f)
      m_distance += m_track->length;

  float SIDECOLLDIST = MAX(m_car->_dimension_x, mycar->_dimension_x);

  // Is opponent in relevant range -BACKCOLLDIST..FRONTCOLLDIST?
  if(m_distance > -BACKCOLLDIST && m_distance < FRONTCOLLDIST)
    {
      // Is opponent aside?
      if(m_distance > -SIDECOLLDIST && m_distance < SIDECOLLDIST)
        {
          m_sidedist = m_car->_trkPos.toMiddle - mycar->_trkPos.toMiddle;
          m_state |= OPP_SIDE;
        }
      
      // Is opponent in front and slower?
      if(m_distance > SIDECOLLDIST && getSpeed() <= driver->getSpeed())
        {
          m_state |= OPP_FRONT;

          if(is_quicker_teammate(mycar))
            m_state |= OPP_FRONT_FOLLOW;

          m_distance -= MAX(m_car->_dimension_x, mycar->_dimension_x);
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

              float mindist = FLT_MAX;
              for(int i = 0; i < 4; i++)
                {
                  vec2f corner(m_car->_corner_x(i), m_car->_corner_y(i));
                  float dist = carFrontLine.dist(corner);
                  mindist = MIN(dist, mindist);
                }
              m_distance = MIN(m_distance, mindist);
            }

          m_catchdist =
            driver->getSpeed() * m_distance / (driver->getSpeed() - getSpeed());

          m_sidedist = m_car->_trkPos.toMiddle - mycar->_trkPos.toMiddle;
          float cardist = fabs(m_sidedist) - fabs(getWidth() / 2.0f) -
            mycar->_dimension_y / 2.0f;
          if(cardist < SIDE_MARGIN)
            m_state |= OPP_COLL;
        }
      // Is opponent behind and faster.
      else if(m_distance < -SIDECOLLDIST
          && getSpeed() > driver->getSpeed() - SPEED_PASS_MARGIN)
        {
          m_catchdist = driver->getSpeed() * m_distance / (getSpeed() - driver->getSpeed());
          m_state |= OPP_BACK;
          m_distance -= MAX(m_car->_dimension_x, mycar->_dimension_x);
          m_distance -= LENGTH_MARGIN;
        }
      // Opponent is in front and faster.
      else if(m_distance > SIDECOLLDIST && getSpeed() > driver->getSpeed())
        {
          m_state |= OPP_FRONT_FAST;
          if(is_quicker_teammate(mycar))
            m_state |= OPP_FRONT_FOLLOW;
          m_distance -= MAX(m_car->_dimension_x, mycar->_dimension_x);
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
float
Opponent::getDistToSegStart()
{
  if(m_car->_trkPos.seg->type == TR_STR)
    return m_car->_trkPos.toStart;
  else
    return m_car->_trkPos.toStart * m_car->_trkPos.seg->radius;
}


// Update overlaptimers of opponents.
void
Opponent::updateOverlapTimer(tSituation * const s, tCarElt * const mycar)
{
  if((m_car->race.laps > mycar->race.laps) || is_quicker_teammate(mycar))
    {
      if(is_state(OPP_BACK | OPP_SIDE))
        m_overlaptimer += (float)s->deltaTime;
      else if(is_state(OPP_FRONT))
        m_overlaptimer = LAP_BACK_TIME_PENALTY;
      else
        {
          if(m_overlaptimer > 0.0f)
            {
              if(is_state(OPP_FRONT_FAST))
                m_overlaptimer = MIN(0.0f, m_overlaptimer);
              else
                m_overlaptimer -= (float)s->deltaTime;
            }
          else
            m_overlaptimer += (float)s->deltaTime;
        }
    }
  else
    m_overlaptimer = 0.0;
}


/*
 *
 * name: is_quicker_teammate
 *
 * Returns true, if the other car is our teammate
 * and has significantly less damage
 * (defined in Driver::TEAM_DAMAGE_CHANGE_LEAD)
 *
 * @param: tCarElt *, pointer to the other car
 * @return: bool
 */
bool
Opponent::is_quicker_teammate(tCarElt * const mycar)
{
  return (is_teammate()
    && (mycar->_dammage - m_car->_dammage > Driver::TEAM_DAMAGE_CHANGE_LEAD));
}


// Constructor
// Initialize the list of opponents.
// Don't store our own car as opponent.
Opponents::Opponents(tSituation * s, Driver * driver, Cardata * c)
{
  m_opps = new list<Opponent>;
  
  //Step through all the cars
  for(int i = 0; i < s->_ncars; i++)
    {
      //If it is not our own car
      if(s->cars[i] != driver->getCarPtr())
        {
          //Create and set up new opponent
          Opponent opp(
            s->cars[i], //car ptr
            c->findCar(s->cars[i]), //car data ptr
            i //index
            ); 
          m_opps->push_back(opp);   //Store it in list
        }
    }
  Opponent::setTrackPtr(driver->getTrackPtr());
}


// Updates all opponents' own data
void
Opponents::update(tSituation * s, Driver * driver)
{
  for(list<Opponent>::iterator it = begin(); it != end(); it++)
    it->update(s, driver);
}


// Search the opponents for our teammate,
// based on teammate name given in config
void
Opponents::setTeamMate(const tCarElt *car)
{
  // Set team mate.
  string teammate;
  try
    {
      teammate =
        GfParmGetStr(car->_carHandle, BT_SECT_PRIV, BT_ATT_TEAMMATE, "");
  
      if(!teammate.empty())
        {  
          for(list<Opponent>::iterator it = begin(); it != end(); it++)
            {
              if(it->getCarPtr()->_name == teammate)
                {
                  it->markAsTeamMate();
                  break;        // Name should be unique, so we can stop.
                }
            }//for it
        }//if teammate
    }
  catch(...)
    {
     // cerr << "BONG" << endl;
    //      teammate.assign("");
    }
}//setTeamMate


void
TeamTacticsMatrix(tCarElt *car_A, tCarElt *car_B, int *order_A, int *order_B)
{
    tCarElt *car_front;
    tCarElt *car_behind;
    
    //Decide which car is ahead
    if(car_A->_distRaced >= car_B->_distRaced)
      {
        car_front = car_A;
        car_behind = car_B;
      }
    else
      {
        car_front = car_B;
        car_behind = car_A;
      }
    
    //For easier handling, f_: front, b_: behind
    int f_laps = car_front->_laps;
    int b_laps = car_behind->_laps;
    //int f_damage = car_front->_dammage;
    //int b_damage = car_behind->_dammage;
    
    
    if(b_laps > f_laps) //Case 1,2,3
      {
        
      }
    else
      {
        if(b_laps == f_laps)    //Case 4,5,6
          {
          }
        else    //Case 7,8,9
          {
            
          }
      }    
}
