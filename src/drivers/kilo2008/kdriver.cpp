/*
 *      kdriver.cpp
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

#include "src/drivers/kilo2008/kdriver.h"

#include <portability.h>  // snprintf under MSVC
#include <robottools.h>  // Rt*
#include <robot.h>  // ROB_PIT_IM

#include <sstream>
#include <list>
#include <string>

// #define DEBUG
#ifdef DEBUG
#include <iostream>  // NOLINT(readability/streams), used for logging only
using ::std::cout;
#endif

#include "src/drivers/kilo2008/opponent.h"
#include "src/drivers/kilo2008/strategy.h"
#include "src/drivers/kilo2008/pit.h"
#include "src/drivers/kilo2008/raceline.h"
#include "src/drivers/kilo2008/util.h"

using ::std::string;
using ::std::stringstream;
using ::std::list;

// "I AM DEATH, NOT TAXES.  *I* TURN UP ONLY ONCE."  --  Death
// Fear was theirs, not yers.

static int
  pitstatus[128] = { 0 };
static double colour[] = {1.0, 0.0, 0.0, 0.0};

// Constants

// [radians] If the angle of the car on the track is smaller,
// we assume we are not stuck.
const double KDriver::MAX_UNSTUCK_ANGLE = 15.0 / 180.0 * PI;
// [s] We try to get unstuck after this time.
const double KDriver::UNSTUCK_TIME_LIMIT = 2.0;
// [m/s] Below this speed we consider being stuck.
const double KDriver::MAX_UNSTUCK_SPEED = 5.0;
// [m] If we are closer to the middle we assume to be not stuck.
const double KDriver::MIN_UNSTUCK_DIST = 3.0;
// [m/(s*s)] Welcome on Earth.
const double KDriver::G = 9.81;
// [-] (% of rpmredline) When do we like to shift gears.
const double KDriver::SHIFT = 0.95;
// [m/s] Avoid oscillating gear changes.
const double KDriver::SHIFT_MARGIN = 4.4;
// [m/s] range [0..10]
const double KDriver::ABS_SLIP = 2.5;
// [m/s] range [0..10]
const double KDriver::ABS_RANGE = 5.0;
// [m/s] Below this speed the ABS is disabled
// (numeric, division by small numbers).
const double KDriver::ABS_MINSPEED = 3.0;
// [m/s] range [0..10]
const double KDriver::TCL_SLIP = 2.0;
// [m/s] range [0..10]
const double KDriver::TCL_RANGE = 10.0;
// [m]
const double KDriver::LOOKAHEAD_CONST = 18.0;
const double KDriver::LOOKAHEAD_FACTOR = 0.33;
// [-] Defines the percentage of the track to use (2/WIDTHDIV).
const double KDriver::WIDTHDIV = 2.0;
// [m]
const double KDriver::BORDER_OVERTAKE_MARGIN = 1.0;
// [m/s] Offset change speed.
const double KDriver::OVERTAKE_OFFSET_SPEED = 5.0;
// [m] Lookahead to stop in the pit.
const double KDriver::PIT_LOOKAHEAD = 6.0;
// [m] Workaround for "broken" pitentries.
const double KDriver::PIT_BRAKE_AHEAD = 200.0;
// [-] Friction of pit concrete.
const double KDriver::PIT_MU = 0.4;
// [m/s] Speed to compute the percentage of brake to apply., 350 km/h
const double KDriver::MAX_SPEED = 350.0 / 3.6;
// [m/s]
const double KDriver::CLUTCH_SPEED = 5.0;
// [m] How far to look, terminate 'while'' loops.
const double KDriver::DISTCUTOFF = 400.0;
// [m] Increment faster if speed is slow [1.0..10.0].
const double KDriver::MAX_INC_FACTOR = 8.0;
// [-] select MIN(catchdist, dist*CATCH_FACTOR) to overtake.
const double KDriver::CATCH_FACTOR = 10.0;
const double KDriver::TEAM_REAR_DIST = 50.0;
// Reduce speed with this factor when being overlapped
const double KDriver::LET_OVERTAKE_FACTOR = 0.6;
// When to change position in the team?
const int KDriver::TEAM_DAMAGE_CHANGE_LEAD = 800;

// Static variables.
Cardata *KDriver::m_cardata = NULL;
double KDriver::m_currentSimTime;
static char const *WheelSect[4] = { SECT_FRNTRGTWHEEL, SECT_FRNTLFTWHEEL,
                        SECT_REARRGTWHEEL, SECT_REARLFTWHEEL };

#define DEFAULTCARTYPE "trb1-cavallo-360rb"

#define SLOW_TRACK_LIMIT 2.4
#define FAST_TRACK_LIMIT 4.0


KDriver::KDriver(int index) {
  INDEX = index;
  m_rgtinc = m_lftinc = 0.0;
  m_minoffset = m_maxoffset = 0.0;
  m_rInverse = 0.0;
}


KDriver::~KDriver() {
  delete m_raceline;
  delete m_opponents;
  delete m_pit;
  delete m_strategy;
  if (m_cardata != NULL) {
    delete m_cardata;
    m_cardata = NULL;
  }
}


/**
 * Drive during the race.
 * 
 * @param[in] s Situation provided by the sim.
 */
void KDriver::drive(tSituation * s) {
  memset(&m_car->ctrl, 0, sizeof(tCarCtrl));
  update(s);
  // sprintf(m_m_car->_msgCmd[0], "%d", (int)(m_m_car->_distFromStartLine));
  // memcpy(m_m_car->_msgColorCmd, colour, sizeof(m_m_car->_msgColorCmd));

  string sMsg;
  if (isStuck()) {
    m_car->_steerCmd = -m_mycardata->getCarAngle() / m_car->_steerLock;
    m_car->_gearCmd = -1;     // Reverse gear.
    m_car->_accelCmd = 1.0;   // 100% accelerator pedal.
    m_car->_brakeCmd = 0.0;   // No brakes.
    m_car->_clutchCmd = 0.0;  // Full clutch (gearbox connected with engine).
    sMsg = "Truagh :(";
  } else {
    m_car->_steerCmd = getSteer(s);
    m_car->_gearCmd = getGear();
    calcSpeed();
    m_car->_brakeCmd =
      filterABS(filterBrakeSpeed(filterBColl(filterBPit(getBrake()))));
    if (m_car->_brakeCmd == 0.0) {
      m_car->_accelCmd = filterTCL(filterTrk(filterOverlap(getAccel())));
      sMsg = "Thig comhla ruinn!";
    } else {
      m_car->_accelCmd = 0.0;
      sMsg = "Sguir!";
    }
    m_car->_clutchCmd = getClutch();
  }  // if isStuck

  snprintf(m_car->_msgCmd[0], RM_MSG_LEN, "%s", sMsg.c_str());
  memcpy(m_car->_msgColorCmd, colour, sizeof(m_car->_msgColorCmd));

  m_laststeer = m_car->_steerCmd;
  m_lastmode = m_mode;
}  // drive


void KDriver::endRace(tSituation * s) {
  // Nothing for now.
}  // endRace


/**
 * Checks if I'm stuck.
 * 
 * @return true if stuck
 */
bool KDriver::isStuck() {
  bool ret = false;

  if (fabs(m_mycardata->getCarAngle()) > MAX_UNSTUCK_ANGLE
            && m_car->_speed_x < MAX_UNSTUCK_SPEED
            && fabs(m_car->_trkPos.toMiddle) > MIN_UNSTUCK_DIST) {
    if (m_stuckCounter > MAX_UNSTUCK_COUNT
        && m_car->_trkPos.toMiddle * m_mycardata->getCarAngle() < 0.0) {
        ret = true;
    } else {
      m_stuckCounter++;
    }
  } else {
    m_stuckCounter = 0;
  }

  return ret;
}  // isStuck


/**
 * Reduces the brake value such that it fits the speed
 * (more downforce -> more braking).
 * 
 * @param[in] brake Original braking value
 * @return  Modified braking value
 */
double KDriver::filterBrakeSpeed(double brake) {
  double weight = m_mass * G;
  double maxForce = weight + CA * pow(MAX_SPEED, 2);
  double force = weight + CA * m_currentSpeedSqr;
  return brake * force / maxForce;
}  // filterBrakeSpeed


// Compute offset to normal target point
// for overtaking or let pass an opponent.
double KDriver::getOffset() {
  m_mincatchdist = 500.0;
  Opponent *o = NULL;

  m_myoffset = m_car->_trkPos.toMiddle;
  m_avoidmode = 0;
  m_avoidLftOffset = MAX(m_myoffset, m_car->_trkPos.seg->width / 2.0 - 1.5);
  m_avoidRgtOffset = MIN(m_myoffset, -(m_car->_trkPos.seg->width / 2.0 - 1.5));

  // Increment speed dependent.
  m_rInverse = m_raceline->getRInverse();
  double incspeed = MIN(60.0, MAX(45.0, getSpeed())) - 18.0;
  double incfactor = (MAX_INC_FACTOR - MIN(fabs(incspeed) / MAX_INC_FACTOR,
                                            (MAX_INC_FACTOR - 1.0)))
                      * (12.0 + MAX(0.0, (CA-1.9) * 14));
  m_rgtinc = incfactor * MIN(1.3, MAX(0.4,
                    1.0 + m_rInverse * (m_rInverse < 0.0 ? 20 : 80)));
  m_lftinc = incfactor * MIN(1.3, MAX(0.4,
                    1.0 - m_rInverse * (m_rInverse > 0.0 ? 20 : 80)));

  int offlft = m_myoffset > m_car->_trkPos.seg->width / 2 - 1.0;
  int offrgt = m_myoffset < -(m_car->_trkPos.seg->width / 2 - 1.0);

  if (offlft) {
    m_myoffset -= OVERTAKE_OFFSET_INC * m_rgtinc / 2;
  } else if (offrgt) {
    m_myoffset += OVERTAKE_OFFSET_INC * m_lftinc / 2;
  }

  m_avoidLftOffset = MAX(m_avoidLftOffset,
                          m_myoffset - OVERTAKE_OFFSET_INC * m_rgtinc
                          * (offlft ? 6 : 2));
  m_avoidRgtOffset = MIN(m_avoidRgtOffset,
                          m_myoffset + OVERTAKE_OFFSET_INC * m_lftinc
                          * (offrgt ? 6 : 2));

  // limit to the left
  m_maxoffset = m_track->width / 2 - m_car->_dimension_y;
  // limit to the right
  // m_minoffset = -(m_track->width / 2 - m_car->_dimension_y);
  m_minoffset = -m_maxoffset;

  if (m_myoffset < m_minoffset) {
    // we're already outside right limit, bring us back towards track
    m_minoffset = m_myoffset + OVERTAKE_OFFSET_INC * m_lftinc;
    m_maxoffset = MIN(m_maxoffset,
                        m_myoffset + OVERTAKE_OFFSET_INC * m_lftinc * 2);
  } else if (m_myoffset > m_maxoffset) {
    // outside left limit, bring us back
    m_maxoffset = m_myoffset - OVERTAKE_OFFSET_INC * m_rgtinc;
    m_minoffset = MAX(m_minoffset,
                        m_myoffset - OVERTAKE_OFFSET_INC * m_rgtinc * 2);
  } else {
    // set tighter limits based on how far we're allowed to move
    m_maxoffset = MIN(m_maxoffset,
                        m_myoffset + OVERTAKE_OFFSET_INC * m_lftinc * 2);
    m_minoffset = MAX(m_minoffset,
                        m_myoffset - OVERTAKE_OFFSET_INC * m_rgtinc * 2);
  }

  // Check for side collision
  o = getSidecollOpp();
  if (o != NULL)
    return filterSidecollOffset(o, incfactor);


  // If we have someone to take over, let's try it
  o = getTakeoverOpp();
  if (o != NULL)
    return filterTakeoverOffset(o);


  // If there is someone overlapping, move out of the way
  o = getOverlappingOpp();
  if (o != NULL)
    return filterOverlappedOffset(o);


  // no-one to avoid, work back towards raceline
  if (m_mode != NORMAL && fabs(m_myoffset - m_raceOffset) > 1.0) {
    if (m_myoffset > m_raceOffset + OVERTAKE_OFFSET_INC * m_rgtinc / 4) {
      m_myoffset -= OVERTAKE_OFFSET_INC * m_rgtinc / 4;
    } else if (m_myoffset < m_raceOffset + OVERTAKE_OFFSET_INC * m_lftinc / 4) {
      m_myoffset += OVERTAKE_OFFSET_INC * m_lftinc / 4;
    }
  }  // if m_mode

  if (m_simTime > 2.0) {
    if (m_myoffset > m_raceOffset) {
      m_myoffset = MAX(m_raceOffset,
                        m_myoffset - OVERTAKE_OFFSET_INC * incfactor / 2);
    } else {
      m_myoffset = MIN(m_raceOffset,
                        m_myoffset + OVERTAKE_OFFSET_INC * incfactor / 2);
    }
  }  // if m_simTime

  m_myoffset = MIN(m_maxoffset, MAX(m_minoffset, m_myoffset));
  return m_myoffset;
}  // getOffset


/**
 * Decide if there is a car behind overlapping us.
 * 
 * A1) Teammate behind with more laps should overtake.
 * A2) Slipstreaming: the less damaged teammate is also allowed to pass
 *      if on the same lap.
 *      The position change happens when the damage difference is greater
 *      than TEAM_DAMAGE_CHANGE_LEAD.
 * B) Let other, overlapping opponents get by.

 * @return  overlapping car pointer or NULL
 */
Opponent * KDriver::getOverlappingOpp() {
  Opponent *ret = NULL;
  double mindist = -1000.0;

  for (list<Opponent>::iterator it = m_opponents->begin();
        it != m_opponents->end();
        it++) {
    tCarElt *ocar = it->getCarPtr();
    double oppDistance = it->getDistance();

    if ((  // If teammate has more laps under his belt,
      (it->isTeammate() && ocar->race.laps > m_car->race.laps)
        ||  // or teammate is less damaged, let him go
        it->isQuickerTeammate(m_car))
        && (oppDistance > -TEAM_REAR_DIST)  // if close enough
        && (oppDistance < -m_car->_dimension_x)) {
      // Behind, larger distances are smaller ("more negative").
      if (oppDistance > mindist) {
        mindist = oppDistance;
        ret = &(*it);
      }
    } else if (it->isState(OPP_LETPASS)) {
      // Behind, larger distances are smaller ("more negative").
      if (oppDistance > mindist) {
        mindist = oppDistance;
        ret = &(*it);
      }
    }  // else if
  }  // for i

  return ret;
}  // getOverlappingOpp


/**
 * Modifies the member 'm_myoffset' so that the car moves out of the way
 * of the overlapping opponent.
 * 
 * @param [in] o: the opponent we should let go
 * @return    new offset. Equals member 'm_myoffset'
 * 
 */
double KDriver::filterOverlappedOffset(Opponent *o) {
  double w = m_car->_trkPos.seg->width / WIDTHDIV - BORDER_OVERTAKE_MARGIN;

  if (o->isOnRight(m_car->_trkPos.toMiddle)) {
    if (m_myoffset < w) {
      m_myoffset += OVERTAKE_OFFSET_INC * m_lftinc / 1;  // 2;
    }
  } else {
    if (m_myoffset > -w) {
      m_myoffset -= OVERTAKE_OFFSET_INC * m_rgtinc / 1;  // 2;
    }
  }
  setMode(BEING_OVERLAPPED);

  m_myoffset = MIN(m_avoidLftOffset, MAX(m_avoidRgtOffset, m_myoffset));
  return m_myoffset;
}  // filterOverlappedOffset


/** 
 * If there is an opponent overlapping us, reduce accelerator.
 *
 * @param [in]  accel: original acceleration value
 * @return      possibly reduced acceleration value
 */
double KDriver::filterOverlap(double accel) {
  return (m_opponents->getOppByState(OPP_LETPASS)
    ? MIN(accel, LET_OVERTAKE_FACTOR)
    : accel);
}  // filterOverlap


/**
 * If opponent is too much on either side of the track,
 * (doesn't occupy center part of the segment)
 * and we are 5+ metres far
 * 
 * @param [in]  ocar  the opponent car
 * @return      true if the opp. is too far on either side
*/
bool KDriver::oppTooFarOnSide(tCarElt *ocar) {
  bool ret = false;
  if (fabs(ocar->_trkPos.toMiddle) > m_car->_trkPos.seg->width / 2 + 3.0
      && fabs(m_car->_trkPos.toMiddle - ocar->_trkPos.toMiddle) >= 5.0)
    ret = true;
  return ret;
}  // oppTooFarOnSide


/**
 * Decide if there is a car ahead we can take over.
 * 
 * @return  Overlap car pointer or NULL
 */
Opponent* KDriver::getTakeoverOpp() {
  Opponent *ret = NULL;

  m_mincatchdist = MAX(30.0, 1500.0 - fabs(m_rInverse) * 10000);
  int otrySuccess = 0;

  for (int otry = 0; otry <= 1; otry++) {
    for (list<Opponent>::iterator it = m_opponents->begin();
          it != m_opponents->end();
          it++) {
      tCarElt *ocar = it->getCarPtr();

      // If opponent is clearly ahead of us, we don't care
      if (it->isState(OPP_FRONT_FOLLOW))
        continue;

      if (oppTooFarOnSide(ocar))
        continue;

      // If opponent is in pit, let him be ;)
      if (ocar->_state > RM_CAR_STATE_PIT)
        continue;

      // If opponent is ahead, and is not a quicker teammate of ours
      if ((it->isState(OPP_FRONT))
          && !it->isQuickerTeammate(m_car)) {
        double otry_factor = otry
            ? (0.2 + (1.0 - ((m_currentSimTime - m_avoidTime) / 7.0)) * 0.8)
            : 1.0;
        // How far ahead is he?
        double distance = it->getDistance() * otry_factor;
        double speed = MIN(m_avoidSpeed,
                            getSpeed() + MAX(0.0, 10.0 - distance));
        double ospeed = it->getSpeed();  // opponent's speed
        // When will we reach up to the opponent?
        double catchdist = MIN(speed * distance / (speed - ospeed),
                          distance * CATCH_FACTOR) * otry_factor;

        // If we are close enough,
        // check again with avoidance speed taken into account
        if (catchdist < m_mincatchdist
            && distance < fabs(speed - ospeed) * 2) {
          m_mincatchdist = catchdist;
          ret = &(*it);  // This is the guy we need to take over
          otrySuccess = otry;
        }
      }  // if it state
    }  // for it
    if (ret) break;
    if (m_mode != AVOIDING) break;
  }  // for otry

  if (ret != NULL && otrySuccess == 0)
    m_avoidTime = m_currentSimTime;

  return ret;
}  // getTakeoverOpp


/**
 * Change offset value if  we are to overtake a car.
 *
 * @param [in]  o the opponent
 * @return      new offset
 */
double KDriver::filterTakeoverOffset(Opponent *o) {
  setMode(AVOIDING);
  tCarElt *ocar = o->getCarPtr();

  // Compute the opponent's distance to the middle.
  double otm = ocar->_trkPos.toMiddle;
  double sidemargin = o->getWidth() + getWidth() + 1.0;
  double sidedist = fabs(ocar->_trkPos.toLeft - m_car->_trkPos.toLeft);

  // Avoid more if on the outside of opponent on a bend.
  // Stops us from cutting in too much and colliding...
  if ((otm < -(ocar->_trkPos.seg->width - 5.0) && m_rInverse < 0.0)
      || (otm > (ocar->_trkPos.seg->width - 5.0) && m_rInverse > 0.0))
    sidemargin += fabs(m_rInverse) * 150;

  if (otm > (ocar->_trkPos.seg->width - 5.0)
      || (m_car->_trkPos.toLeft > ocar->_trkPos.toLeft
          && (sidedist < sidemargin || o->isState(OPP_COLL)))) {
    m_myoffset -= OVERTAKE_OFFSET_INC * m_rgtinc;
    setAvoidLeft();
  } else if (otm < -(ocar->_trkPos.seg->width - 5.0)
              || (m_car->_trkPos.toLeft < ocar->_trkPos.toLeft
                  && (sidedist < sidemargin || o->isState(OPP_COLL)))) {
    m_myoffset += OVERTAKE_OFFSET_INC * m_lftinc;
    setAvoidRight();
  } else {
    // If the opponent is near the middle we try to move the offset toward
    // the inside of the expected turn.
    // Try to find out the characteristic of the track up to catchdist.
    tTrackSeg *seg = m_car->_trkPos.seg;
    double length = getDistToSegEnd();
    double oldlen, seglen = length;
    double lenright = 0.0, lenleft = 0.0;
    m_mincatchdist = MIN(m_mincatchdist, DISTCUTOFF);

    do {
      switch (seg->type) {
        case TR_LFT:
          lenleft += seglen;
          break;
        case TR_RGT:
          lenright += seglen;
          break;
        default:
          // Do nothing.
          break;
      }  // switch seg->type
      seg = seg->next;
      seglen = seg->length;
      oldlen = length;
      length += seglen;
    } while (oldlen < m_mincatchdist);

    // If we are on a straight look for the next turn.
    if (lenleft == 0.0 && lenright == 0.0) {
      while (seg->type == TR_STR)
        seg = seg->next;

      // Assume: left or right if not straight.
      if (seg->type == TR_LFT)
        lenleft = 1.0;
      else
        lenright = 1.0;
    }  // if lenleft/lenright == 0

    // Because we are inside we can go to the limit.
    if ((lenleft > lenright && m_rInverse < 0.0)
         || (lenleft <= lenright && m_rInverse > 0.0)) {
        // avoid more if on the outside of opponent on a bend.  Stops us
        // from cutting in too much and colliding...
        sidemargin += fabs(m_rInverse) * 150;
    }

    if (sidedist < sidemargin || o->isState(OPP_COLL)) {
      if (lenleft > lenright) {
        m_myoffset += OVERTAKE_OFFSET_INC * m_lftinc;  // * 0.7;
        setAvoidRight();
      } else {
        m_myoffset -= OVERTAKE_OFFSET_INC * m_rgtinc;  // * 0.7;
        setAvoidLeft();
      }  // if lenleft > lenright
    }  // if sidedist
  }  // if opp near middle

  m_myoffset = MIN(m_avoidLftOffset, MAX(m_avoidRgtOffset, m_myoffset));
  m_myoffset = MIN(m_maxoffset, MAX(m_minoffset, m_myoffset));
  return m_myoffset;
}  // filterTakeoverOffset


/**
 * Decide if there is a car on the side we are to collide with...
 * 
 * @return  Side collision car pointer or NULL
 */
Opponent *KDriver::getSidecollOpp() {
  Opponent *ret = NULL;
  for (list<Opponent>::iterator it = m_opponents->begin();
        it != m_opponents->end();
        it++) {
    tCarElt *ocar = it->getCarPtr();

    if (ocar->_state > RM_CAR_STATE_PIT)  // Dont care for opponents in the pit
      continue;

    if (oppTooFarOnSide(ocar))
      continue;

    if (it->isState(OPP_SIDE)) {  // If opponent is on our side
      setMode(AVOIDING);
      ret = &(*it);
      break;
    }  // if OPP_SIDE
  }  // for it

  return ret;
}  // getSidecollOpp


double KDriver::filterSidecollOffset(Opponent *o, const double incfactor) {
  double myToLeft = m_car->_trkPos.toLeft;
  double oppToLeft = o->getCarPtr()->_trkPos.toLeft;
  double sidedist = fabs(oppToLeft - myToLeft);
  double sidemargin = o->getWidth() + getWidth() + 2.0;
  bool oppOnRight = o->isOnRight(m_car->_trkPos.toMiddle);

  // Avoid more if on the outside of opponent on a bend.
  // Stops us from cutting in too much and colliding...
  if ((oppOnRight && m_rInverse < 0.0)
      || (!oppOnRight && m_rInverse > 0.0))
    sidemargin += fabs(m_rInverse) * 150;

  if (oppOnRight) {
    sidemargin -= MIN(0.0, m_rInverse * 100);
  } else {
    sidemargin += MAX(0.0, m_rInverse * 100);
  }
  sidedist = MIN(sidedist, sidemargin);

  if (sidedist < sidemargin) {
    double sdiff = 3.0 - (sidemargin - sidedist) / sidemargin;

    if (oppOnRight) {  // He is on the right, we must move to the left
      m_myoffset += OVERTAKE_OFFSET_INC * m_lftinc * MAX(0.2, MIN(1.0, sdiff));
    } else {           // He is on the left, we must move to the right
      m_myoffset -= OVERTAKE_OFFSET_INC * m_rgtinc * MAX(0.2, MIN(1.0, sdiff));
    }
  } else if (sidedist > sidemargin + 3.0) {
    if (m_raceOffset > m_myoffset + OVERTAKE_OFFSET_INC * incfactor) {
      m_myoffset += OVERTAKE_OFFSET_INC * m_lftinc / 4;
    } else if (m_raceOffset < m_myoffset - OVERTAKE_OFFSET_INC * incfactor) {
      m_myoffset -= OVERTAKE_OFFSET_INC * m_rgtinc / 4;
    }
  }

  oppOnRight ? setAvoidRight() : setAvoidLeft();
  m_avoidmode |= AVOIDSIDE;

  m_myoffset = MIN(m_maxoffset, MAX(m_minoffset, m_myoffset));
  return m_myoffset;
}  // filterSidecollOffset


/**
 * Initialize the robot on a track.
 * For this reason it looks up any setup files.
 * 
 * Setup files are in a directory path like:
 * drivers/kilo
 *          |- default.xml  (skill enable)
 *          tracks
 *          | |-<trackname>.xml   (track-specific parameters)
 *          |
 *          |
 *          <carname>
 *            |-<trackname>.xml   (setup for the given track)
 *            |-def-slow.xml      (setup for undefined, slow tracks)
 *            |-def-norm.xml      (setup for undefined, normal tracks)
 *            |-def-fast.xml      (setup for undefined, fast tracks)
 * 
 * @param [in]  t the track
 * @param [out] carHandle
 * @param [out] carParmHandle
 * @param [in]  s Situation provided by the sim
 */
void KDriver::initTrack(tTrack * t, void *carHandle,
                        void **carParmHandle, tSituation * s) {
  initSkill(s);

  stringstream buf;

  // Load a custom setup if one is available.
  m_track = t;
  // Ptr to the track filename
  char *trackname = strrchr(m_track->filename, '/') + 1;
  stringstream botPath;

  // Try to load the default setup
  botPath << "drivers/" << bot << "/";  // << INDEX << "/";
  buf << botPath.str() << "default.xml";
  *carParmHandle = GfParmReadFile(buf.str().c_str(), GFPARM_RMODE_STD);
#ifdef DEBUG
  cout << "Default: " << buf.str() << endl;
  if (carParmHandle)
    cout << "default xml loaded, carParmHandle: " << *carParmHandle << endl;
#endif

  // Try to load the track-based informations
  buf.str(string());
  buf << botPath.str() << "tracks/" << trackname;
  void *newhandle = GfParmReadFile(buf.str().c_str(), GFPARM_RMODE_STD);
#ifdef DEBUG
  cout << "track-based: " << buf.str() << endl;
  if (newhandle)
    cout << "track-based xml loaded, newhandle: " << newhandle << endl;
  else
    cout << "no track-based xml present" << endl;
#endif

  mergeCarSetups(carParmHandle, newhandle);
#ifdef DEBUG
  cout << "merge #1, carParmHandle: " << *carParmHandle
        << " newhandle: " << newhandle << endl;
#endif

  // Discover somehow the name of the car used
  if (m_carType.empty()) {
    stringstream ssSection;
    ssSection << ROB_SECT_ROBOTS << "/" << ROB_LIST_INDEX << "/" << INDEX;
    m_carType = GfParmGetStr(carHandle, ssSection.str().c_str(),
        ROB_ATTR_CAR, DEFAULTCARTYPE);
  }  // if carType empty
#ifdef DEBUG
  cout << "Cartype: " << m_carType << endl;
#endif

  // Load setup tailored for car+track
  buf.str(string());
  buf << botPath.str() << m_carType << "/" << trackname;
  newhandle = GfParmReadFile(buf.str().c_str(), GFPARM_RMODE_STD);
#ifdef DEBUG
  cout << "custom setup: " << buf.str() << endl;
  if (newhandle)
    cout << "car+track xml loaded, newhandle: " << newhandle << endl;
  else
    cout << "no car+track xml present" << endl;
#endif

  // If there is no tailored setup, let's load a default one
  // based on the track charateristics.
  if (!newhandle)
    newhandle = loadDefaultSetup();
#ifdef DEBUG
  if (newhandle)
    cout << "default setup xml loaded, newhandle: " << newhandle << endl;
  else
    cout << "no default setup loaded???" << endl;
#endif

  // Merge the above two setups
  mergeCarSetups(carParmHandle, newhandle);
#ifdef DEBUG
  cout << "merge #2" << endl;
#endif

  // Load and set parameters.
  MU_FACTOR = GfParmGetNum(*carParmHandle, KILO_SECT_PRIV,
                            KILO_ATT_MUFACTOR, NULL, 0.69f);
  m_pitOffset = GfParmGetNum(*carParmHandle, KILO_SECT_PRIV,
                            KILO_ATT_PITOFFSET, NULL, 10.0);
  m_brakeDelay = GfParmGetNum(*carParmHandle, KILO_SECT_PRIV,
                            KILO_ATT_BRDELAY, NULL, 10.0);

  // Create a pit stop strategy object.
  m_strategy = new KStrategy();
  // Init fuel.
  m_strategy->SetFuelAtRaceStart(t, carParmHandle, s, INDEX);

  m_raceline = new LRaceLine;
  // m_raceline->setSkill(m_skill);
  m_raceline->InitTrack(m_track, carParmHandle, s, m_filterSideSkill);
}  // initTrack


/**
 * Update own private data on every timestep.
 * 
 * @param [in]  s situation provided by the sim
 */
void KDriver::update(tSituation * s) {
  // Update global car data (shared by all instances) just once per timestep.
  if (m_currentSimTime != s->currentTime) {
    m_currentSimTime = s->currentTime;
    m_cardata->update();
  }

  // Update the rest of local data
  m_speedangle = -(m_mycardata->getTrackangle() -
                    atan2(m_car->_speed_Y, m_car->_speed_X));
  NORM_PI_PI(m_speedangle);
  m_mass = CARMASS + m_car->_fuel;
  m_currentSpeedSqr = pow(m_car->_speed_x, 2);

  m_opponents->update(s, this);
  m_strategy->Update();

  checkPitStatus(s);
  m_pit->update();
  m_simTime = s->currentTime;

  double trackangle = RtTrackSideTgAngleL(&(m_car->_trkPos));
  m_angle = trackangle - m_car->_yaw;
  NORM_PI_PI(m_angle);
  m_angle = -m_angle;
}  // update

/**
 * Checks if we need to plan a pitstop.
 * If yes, checks availability of the pit,
 * is it free or occupied by teammate.
 * 
 * @param [in]  s Situation provided by the sim
 */
void KDriver::checkPitStatus(tSituation *s) {
  if (m_car->_state <= RM_CAR_STATE_PIT) {  // If our car is still in the race
    if (!m_pit->getPitstop()) {  // if no pit is planned yet
      // and we are not in the pit range
      // or we are very low on fuel
      // then we can check if pitting is needed by strategy.
      if ((m_car->_distFromStartLine < m_pit->getNPitEntry()
                || m_car->_distFromStartLine > m_pit->getNPitEnd())
                || m_car->_fuel < 5.0) {
          m_pit->setPitstop(m_strategy->NeedPitstop());
      }
    }  // if no pit planned

    if (m_pit->getPitstop() && m_car->_pit) {  // if pitting is planned
      pitstatus[m_carIndex] = 1;  // flag our car's pit as occupied

      for (list<Opponent>::iterator it = m_opponents->begin();
            it != m_opponents->end();
            it++) {
        tCarElt *ocar = it->getCarPtr();
        // If the other car is our teammate, still in the race
        if (it->isTeammate() && ocar->_state <= RM_CAR_STATE_PIT) {
          int idx = it->getIndex();
          if (pitstatus[idx] == 1
              || ((pitstatus[idx]
                  || (ocar-> _fuel < m_car->_fuel - 1.0
                      && m_car->_dammage < 5000))
                  && fabs(m_car->_trkPos.toMiddle)
                      <= m_car->_trkPos.seg->width / 2)) {
            m_pit->setPitstop(false);
            pitstatus[m_carIndex] = 0;
          }
          break;
        }  // if our teammate
      }  // for it
    } else if (!m_pit->getInPit()) {  // If we are not in the pitlane
      pitstatus[m_carIndex] = 0;  // sign the pit as free
    } else {
      pitstatus[m_carIndex] = 0;
    }
  }
}  // checkPitStatus


/**
 * Brake filter for collision avoidance.
 * If there is an opponent we are to collide, brake brake brake!
 * 
 * @param [in]  brake Original brake value
 * @return  Possibly modified brake value
 */
double KDriver::filterBColl(const double brake) {
  double ret = brake;

  if (m_simTime >= 1.5) {
    double mu = m_car->_trkPos.seg->surface->kFriction;
    for (list<Opponent>::iterator it = m_opponents->begin();
          it != m_opponents->end();
          it++) {
      if (it->isState(OPP_COLL)) {  // Endangered species
        double ospeed = it->getSpeed();
        if (brakeDist(ospeed, mu) + MIN(1.0,
                                    0.5 + MAX(0.0, (getSpeed() - ospeed) / 4))
            > it->getDistance()) {  // Damn, too close, brake hard!!!
          m_accelCmd = 0.0;
          ret = 1.0;
          break;
        }  // if brakeDist
      }  // if state OPP_COLL
    }  // for it
  }  // if m_simTime

  return ret;
}  // filterBColl


// Set pitstop commands.
int KDriver::pitCommand(tSituation * s) {
  m_car->_pitRepair = m_strategy->PitRepair();
  m_car->_pitFuel = m_strategy->PitRefuel();
  // This should be the only place where the pit stop is set to false!
  m_pit->setPitstop(false);
  return ROB_PIT_IM;        // return immediately.
}  // pitCommand


void KDriver::newRace(tCarElt * car, tSituation * s) {
  m_strategy->set_car(car);

  double deltaTime = static_cast<double>(RCM_MAX_DT_ROBOTS);
  MAX_UNSTUCK_COUNT = static_cast<int>(UNSTUCK_TIME_LIMIT / deltaTime);
  OVERTAKE_OFFSET_INC = OVERTAKE_OFFSET_SPEED * deltaTime;
  m_stuckCounter = 0;
  m_clutchTime = 0.0;
  m_oldLookahead = m_laststeer = m_lastNSasteer = 0.0;
  m_car = car;
  CARMASS = GfParmGetNum(m_car->_carHandle, SECT_CAR, PRM_MASS, NULL, 1000.0);
  m_myoffset = 0.0;
  m_simTime = m_correctTimer = 0.0;
  m_correctLimit = 1000.0;
  initCa();
  initCw();
  initTireMu();
  initTCLFilter();

  // Create just one instance of cardata shared by all drivers.
  if (m_cardata == NULL)
    m_cardata = new Cardata(s);
  m_mycardata = m_cardata->findCar(m_car);
  m_currentSimTime = s->currentTime;

  // initialize the list of opponents.
  m_opponents = new Opponents(s, this, m_cardata);
  m_opponents->setTeamMate(m_car);

  // create the pit object.
  m_pit = new Pit(s, this, m_pitOffset);

  // set initial mode
  // we set it to CORRECTING so the robot will steer towards the raceline
  setMode(CORRECTING);
  m_lastmode = CORRECTING;

  for (m_carIndex = 0; m_carIndex < s->_ncars; m_carIndex++) {
    if (s->cars[m_carIndex] == m_car)
      break;
  }

  m_raceline->setCar(m_car);
  m_raceline->NewRace();
}  // newRace


void KDriver::calcSpeed() {
  m_accelCmd = m_brakeCmd = 0.0;
  double speed;

  switch (m_mode) {
    case AVOIDING:
    case BEING_OVERLAPPED:
      speed = m_avoidSpeed;
      break;

    case CORRECTING:
      speed = m_raceSpeed -
              (m_raceSpeed - m_avoidSpeed)
                * MAX(0.0, (m_correctTimer - m_simTime) / 7.0);
      break;

    default:
      speed = m_raceSpeed;
  }  // switch m_mode

  double x = (10 + m_car->_speed_x) * (speed - m_car->_speed_x) / 200;

  if (x > 0.0)
    m_accelCmd = x;
  else
    m_brakeCmd = MIN(1.0, -(MAX(10.0, m_brakeDelay * 0.7)) * x);
}  // calcSpeed


/**
 * Decides the character of the track and chooses 1 of 3 default setups.
 * Loads the appropriate setup file and returns it's handler.
 * 
 * @return  Handler to the loaded default setup
 */
void* KDriver::loadDefaultSetup() const {
  void *ret = NULL;

  double dLength = 0.0;
  double dCurves = 0.0;

  // Count length and degrees of all turns
  tTrackSeg *pSeg = m_track->seg;
  do {
    if (pSeg->type == TR_STR) {
      dLength += pSeg->length;
    } else {
      dLength += pSeg->radius * pSeg->arc;
      dCurves += RAD2DEG(pSeg->arc);
    }

    pSeg = pSeg->next;
  } while (pSeg != m_track->seg);

  double dRatio = dLength / dCurves;

#ifdef DEBUG
  cout << "Track " << m_track->name
    << " length: " << dLength
    << " curves: " << dCurves
    << " ratio: " << dRatio
    << endl;
#endif

  stringstream buf;
  buf << "drivers/" << bot << "/" << m_carType;

  if (dRatio < SLOW_TRACK_LIMIT) {
    // Slow track
    buf << "/def-slow.xml";
  } else if (dRatio < FAST_TRACK_LIMIT) {
    // Normal track
    buf << "/def-norm.xml";
  } else {
    // Fast track
    buf << "/def-fast.xml";
  }  // if dRatio

  ret = GfParmReadFile(buf.str().c_str(), GFPARM_RMODE_STD);
#ifdef DEBUG
  cout << "Decision of setup: " << buf.str() << endl;
  if (ret)
    cout << "Def-XXX xml loaded" << endl;
#endif

  return ret;
}  // loadDefaultSetup


void KDriver::mergeCarSetups(void **oldHandle, void *newHandle) {
  if (newHandle) {
    if (*oldHandle)
      *oldHandle = GfParmMergeHandles(*oldHandle, newHandle,
          (GFPARM_MMODE_SRC | GFPARM_MMODE_DST |
          GFPARM_MMODE_RELSRC | GFPARM_MMODE_RELDST));
    // Otherwise no need to merge
    else
      *oldHandle = newHandle;
  }  // if newHandle
}  // mergeCarSetups


// Compute aerodynamic downforce coefficient CA.
void KDriver::initCa() {
  double rearWingArea =
    GfParmGetNum(m_car->_carHandle, SECT_REARWING, PRM_WINGAREA, NULL, 0.0);
  double rearWingAngle =
    GfParmGetNum(m_car->_carHandle, SECT_REARWING, PRM_WINGANGLE, NULL, 0.0);
  double wingCa = 1.23 * rearWingArea * sin(rearWingAngle);

  double cl =
    GfParmGetNum(m_car->_carHandle, SECT_AERODYNAMICS, PRM_FCL, NULL, 0.0) +
    GfParmGetNum(m_car->_carHandle, SECT_AERODYNAMICS, PRM_RCL, NULL, 0.0);

  double h = 0.0;
  for (int i = 0; i < 4; i++)
    h += GfParmGetNum(m_car->_carHandle, WheelSect[i],
                      PRM_RIDEHEIGHT, NULL, 0.2);
  h *= 1.5;
  h = pow(h, 4);
  h = 2.0 * exp(-3.0 * h);
  CA = h * cl + 4.0 * wingCa;
}  // initCa


// Compute aerodynamic drag coefficient CW.
void KDriver::initCw() {
  double cx =
    GfParmGetNum(m_car->_carHandle, SECT_AERODYNAMICS, PRM_CX, NULL, 0.0);
  double frontArea =
    GfParmGetNum(m_car->_carHandle, SECT_AERODYNAMICS, PRM_FRNTAREA, NULL, 0.0);

  CW = 0.645 * cx * frontArea;
}  // initCw


// Init the friction coefficient of the the tires.
void KDriver::initTireMu() {
  double tm = DBL_MAX;
  for (int i = 0; i < 4; i++)
    tm = MIN(tm,
            GfParmGetNum(m_car->_carHandle, WheelSect[i], PRM_MU, NULL, 1.0));

  TIREMU = tm;
}  // initTireMu


// Traction Control (TCL) setup.
void KDriver::initTCLFilter() {
  const string traintype = GfParmGetStr(m_car->_carHandle,
                                        SECT_DRIVETRAIN, PRM_TYPE,
                                        VAL_TRANS_RWD);

  if (traintype == VAL_TRANS_RWD)
    GET_DRIVEN_WHEEL_SPEED = &KDriver::filterTCL_RWD;
  else if (traintype == VAL_TRANS_FWD)
    GET_DRIVEN_WHEEL_SPEED = &KDriver::filterTCL_FWD;
  else if (traintype == VAL_TRANS_4WD)
    GET_DRIVEN_WHEEL_SPEED = &KDriver::filterTCL_4WD;
}  // initTCLFilter


// TCL filter plugin for rear wheel driven cars.
double KDriver::filterTCL_RWD() {
  return (m_car->_wheelSpinVel(REAR_RGT) + m_car->_wheelSpinVel(REAR_LFT)) *
    m_car->_wheelRadius(REAR_LFT) / 2.0;
}


// TCL filter plugin for front wheel driven cars.
double KDriver::filterTCL_FWD() {
  return (m_car->_wheelSpinVel(FRNT_RGT) + m_car->_wheelSpinVel(FRNT_LFT)) *
    m_car->_wheelRadius(FRNT_LFT) / 2.0;
}


// TCL filter plugin for all wheel driven cars.
double KDriver::filterTCL_4WD() {
  return ((m_car->_wheelSpinVel(FRNT_RGT) + m_car->_wheelSpinVel(FRNT_LFT)) *
      m_car->_wheelRadius(FRNT_LFT) +
      (m_car->_wheelSpinVel(REAR_RGT) + m_car->_wheelSpinVel(REAR_LFT)) *
      m_car->_wheelRadius(REAR_LFT)) / 4.0;
}


// TCL filter for accelerator pedal.
double KDriver::filterTCL(const double accel) {
  double ret = accel;

  if (m_simTime >= 3.0) {
    ret = MIN(1.0, accel);
    double accel1 = ret, accel2 = ret, accel3 = ret;

    if (m_car->_speed_x > 10.0) {
      tTrackSeg *seg = m_car->_trkPos.seg;
      tTrackSeg *wseg0 = m_car->_wheelSeg(0);
      tTrackSeg *wseg1 = m_car->_wheelSeg(1);
      int count = 0;

      if (
        wseg0->surface->kRoughness > MAX(0.02, seg->surface->kRoughness * 1.2)
        || wseg0->surface->kFriction < seg->surface->kFriction * 0.8
        || wseg0->surface->kRollRes > MAX(0.005, seg->surface->kRollRes * 1.2))
        count++;

      if (
        wseg1->surface->kRoughness > MAX(0.02, seg->surface->kRoughness * 1.2)
        || wseg1->surface->kFriction < seg->surface->kFriction * 0.8
        || wseg1->surface->kRollRes > MAX(0.005, seg->surface->kRollRes * 1.2))
        count++;

      if (count) {
        if (m_mode != NORMAL
          && ((seg->type == TR_RGT && seg->radius <= 200.0
            && m_car->_trkPos.toLeft < 3.0)
            || (seg->type == TR_LFT && seg->radius <= 200.0
            && m_car->_trkPos.toRight < 3.0)))
          count++;

          accel1 = MAX(0.0, MIN(accel1, (1.0 - (0.25 * count)) -
                            MAX(0.0, (getSpeed() - m_car->_speed_x) / 10.0)));
          }  // if count

      if (fabs(m_angle) > 1.0)
        accel1 = MIN(accel1, 1.0 - (fabs(m_angle) - 1.0) * 1.3);
      }  // if m_car->_speed_x

    if (fabs(m_car->_steerCmd) > 0.02) {
      double decel = ((fabs(m_car->_steerCmd) - 0.02) *
          (1.0 + fabs(m_car->_steerCmd)) * 0.7);
      accel2 = MIN(accel2, MAX(0.45, 1.0 - decel));
    }  // if m_car->_steerCmd

    double slip = (this->*GET_DRIVEN_WHEEL_SPEED) () - m_car->_speed_x;
    if (slip > TCL_SLIP)
      accel3 = accel3 - MIN(accel3, (slip - TCL_SLIP) / TCL_RANGE);

    ret = MIN(accel1, MIN(accel2, accel3));
  }  // if m_simTime

  return ret;
}  // filterTCL


// Hold car on the track.
double KDriver::filterTrk(const double accel) {
  double ret = accel;

  tTrackSeg *seg = m_car->_trkPos.seg;

  if (m_car->_speed_x >= MAX_UNSTUCK_SPEED  // Not too slow
        && !m_pit->getInPit()  // Not on pit stop
        && m_car->_trkPos.toMiddle * -m_speedangle <= 0.0) {
    // Speedvector points to the outside of the turn
    if (seg->type == TR_STR) {
      double tm = fabs(m_car->_trkPos.toMiddle);
      double w = (seg->width - m_car->_dimension_y) / 2.0;
      ret = (tm > w) ? 0.0 : accel;
    } else {
      double sign = (seg->type == TR_RGT) ? -1.0 : 1.0;
      if (m_car->_trkPos.toMiddle * sign > 0.0) {
        ret = accel;
      } else {
        double tm = fabs(m_car->_trkPos.toMiddle);
        double w = seg->width / WIDTHDIV;
        ret = (tm > w) ? 0.0 : accel;
      }
    }  // if seg->type
  }  // if not too slow
  return ret;
}  // filterTrk


// Compute the needed distance to brake.
double KDriver::brakeDist(double allowedspeed, double mu) {
  double c = mu * G;
  double d = (CA * mu + CW) / m_mass;
  double v1sqr = m_currentSpeedSqr;
  double v2sqr = pow(allowedspeed, 2);
  double ret = -log((c + v2sqr * d) / (c + v1sqr * d)) / (2.0 * d);
  ret /= m_filterBrakeSkill;
  return ret;
}  // brakeDist


// Antilocking filter for brakes.
double KDriver::filterABS(double brake) {
  double ret = brake;

  if (m_car->_speed_x >= ABS_MINSPEED) {
    double origbrake = brake;
    double rearskid = MAX(0.0, MAX(m_car->_skid[2], m_car->_skid[3]) -
                    MAX(m_car->_skid[0], m_car->_skid[1]));

    double slip = 0.0;
    for (int i = 0; i < 4; i++)
      slip += m_car->_wheelSpinVel(i) * m_car->_wheelRadius(i);

    slip *=
      1.0 + MAX(rearskid, MAX(fabs(m_car->_yaw_rate) / 5, fabs(m_angle) / 6));
    slip = m_car->_speed_x - slip / 4.0;

    if (slip > ABS_SLIP)
      ret = brake - MIN(brake, (slip - ABS_SLIP) / ABS_RANGE);

    ret = MAX(ret, MIN(origbrake, 0.1f));
  }

  return ret;
}  // filterABS


// Brake filter for pit stop.
double KDriver::filterBPit(double brake) {
  double mu = m_car->_trkPos.seg->surface->kFriction * TIREMU * PIT_MU;

  if (m_pit->getPitstop() && !m_pit->getInPit()) {
    tdble dl, dw;
    RtDistToPit(m_car, m_track, &dl, &dw);
    if (dl < PIT_BRAKE_AHEAD) {
      if (brakeDist(0.0, mu) > dl)
        return 1.0;
    }
  }

  if (m_pit->getInPit()) {
    double s = m_pit->toSplineCoord(m_car->_distFromStartLine);

    // Pit entry.
    if (m_pit->getPitstop()) {
      if (s < m_pit->getNPitStart()) {
        // Brake to pit speed limit.
        double dist = m_pit->getNPitStart() - s;
        if (brakeDist(m_pit->getSpeedlimit(), mu) > dist)
          return 1.0;
      } else {
        // Hold speed limit.
        if (m_currentSpeedSqr > m_pit->getSpeedlimitSqr())
          return m_pit->getSpeedLimitBrake(m_currentSpeedSqr);
      }  // if s

      // Brake into pit (speed limit 0.0 to stop)
      double dist = m_pit->getNPitLoc() - s;
      if (m_pit->isTimeout(dist)) {
        m_pit->setPitstop(false);
        return 0.0;
      } else {
        if (brakeDist(0.0, mu) > dist)
          return 1.0;
        else if (s > m_pit->getNPitLoc())
          return 1.0;  // Stop in the pit.
      }  // if pit timeout
    } else {
      // Pit exit.
      if (s < m_pit->getNPitEnd()) {
        // Pit speed limit.
        if (m_currentSpeedSqr > m_pit->getSpeedlimitSqr())
          return m_pit->getSpeedLimitBrake(m_currentSpeedSqr);
      }
    }
  }

  return brake;
}  // filterBPit


double KDriver::calcSteer(double targetAngle, int rl) {
  double rearskid = MAX(0.0,
                        MAX(m_car->_skid[2], m_car->_skid[3])
                        - MAX(m_car->_skid[0], m_car->_skid[1]))
                        + MAX(m_car->_skid[2], m_car->_skid[3])
                          * fabs(m_angle) * 0.9;

  double angle_correction = 0.0;
  double factor = (rl ? 1.4f : (m_mode == CORRECTING ? 1.1f : 1.2f));
  if (m_angle < 0.0)
    angle_correction = MAX(m_angle,
                            MIN(0.0, m_angle / 2.0)
                              / MAX(15.0, 70.0 - m_car->_speed_x) * factor);
  else
    angle_correction = MIN(m_angle,
                            MAX(0.0, m_angle / 2.0)
                              / MAX(15.0, 70.0 - m_car->_speed_x) * factor);

  double steer_direction = targetAngle - m_car->_yaw + angle_correction;
  NORM_PI_PI(steer_direction);

  if (m_car->_speed_x > 10.0) {
    double speedsteer = (80.0 - MIN(70.0, MAX(40.0, getSpeed()))) /
          ((185.0 * MIN(1.0, m_car->_steerLock / 0.785)) +
          (185.0 * (MIN(1.3, MAX(1.0, 1.0 + rearskid))) - 185.0));
    if (fabs(steer_direction) > speedsteer)
      steer_direction = MAX(-speedsteer, MIN(speedsteer, steer_direction));
  }

  double steer = steer_direction / m_car->_steerLock;

  // smooth steering. check for separate function for this!
  if (m_mode != PITTING) {
    double minspeedfactor = (((105.0 -
        MAX(40.0, MIN(70.0, getSpeed() + MAX(0.0, m_car->_accel_x * 5.0))))
         / 300.0) * (5.0 + MAX(0.0, (CA-1.9) * 20.0)));
    double maxspeedfactor = minspeedfactor;
    double rInverse = m_raceline->getRInverse();

    if (rInverse > 0.0) {
      minspeedfactor = MAX(minspeedfactor / 3, minspeedfactor - rInverse * 80);
      maxspeedfactor = MAX(maxspeedfactor / 3, maxspeedfactor + rInverse * 20);
    } else {
      maxspeedfactor = MAX(maxspeedfactor / 3, maxspeedfactor + rInverse * 80);
      minspeedfactor = MAX(minspeedfactor / 3, minspeedfactor + rInverse * 20);
    }  // if rInverse

    steer = MAX(m_lastNSasteer - minspeedfactor,
                MIN(m_lastNSasteer + maxspeedfactor, steer));
  }  // if mode != PITTING

  m_lastNSasteer = steer;

  if (fabs(m_angle) > fabs(m_speedangle)) {
    // steer into the skid
    double sa = MAX(-0.3, MIN(0.3, m_speedangle / 3));
    double anglediff = (sa - m_angle)
                        * (0.7 - MAX(0.0, MIN(0.3, m_car->_accel_x / 100)));
    // anglediff += m_raceline->getRInverse() * 10;
    steer += anglediff * 0.7;
  }

  if (fabs(m_angle) > 1.2) {
    steer = sign(steer);
  } else if (fabs(m_car->_trkPos.toMiddle)
              - m_car->_trkPos.seg->width / 2 > 2.0) {
    steer = MIN(1.0,
              MAX(-1.0,
                  steer * (1.0 + (fabs(m_car->_trkPos.toMiddle) -
                    m_car->_trkPos.seg->width / 2) / 14 + fabs(m_angle) / 2)));
  }

  if (m_mode != PITTING) {
    // limit how far we can steer against raceline
    double limit = (90.0 - MAX(40.0,
                                MIN(60.0,
                                    m_car->_speed_x))) /
                                    (50 + fabs(m_angle) * fabs(m_angle) * 3);
    steer = MAX(m_raceSteer - limit, MIN(m_raceSteer + limit, steer));
  }

  return steer;
}  // calcSteer


double KDriver::correctSteering(double avoidsteer, double racesteer) {
  double steer = avoidsteer;
  // double accel = MIN(0.0, m_car->_accel_x);
  double speed = MAX(50.0, getSpeed());
  // double changelimit = MIN(1.0, m_raceline->correctLimit());
  double changelimit = MIN(m_raceline->correctLimit(),
        (((120.0 - getSpeed()) / 6000)
        * (0.5 + MIN(fabs(avoidsteer), fabs(racesteer)) / 10)));

  if (m_mode == CORRECTING && m_simTime > 2.0) {
    // move steering towards racesteer...
    if (m_correctLimit < 900.0) {
      if (steer < racesteer) {
        steer = (m_correctLimit >= 0.0)
          ? racesteer
          : MIN(racesteer, MAX(steer, racesteer + m_correctLimit));
      } else {
        steer = (m_correctLimit <= 0.0)
          ? racesteer
          : MAX(racesteer, MIN(steer, racesteer + m_correctLimit));
      }
    }

    speed -= m_car->_accel_x / 10;
    speed = MAX(55.0, MIN(150.0, speed + (speed * speed / 55.0)));
    double rInverse = m_raceline->getRInverse() *
          (m_car->_accel_x < 0.0 ? 1.0 + fabs(m_car->_accel_x) / 10.0 : 1.0);
    double correctspeed = 0.5;
    if ((rInverse > 0.0 && racesteer > steer)
        || (rInverse < 0.0 && racesteer < steer))
      correctspeed += rInverse * 110.0;

    steer = (racesteer > steer)
      ? MIN(racesteer, steer + changelimit)
      : MAX(racesteer, steer - changelimit);
    /* steer = MIN(racesteer,
     *              steer + (((155.0 - speed) / 10000.0) * correctspeed));
     * steer = MAX(racesteer,
     *              steer - (((155.0-speed)/10000) * correctspeed));
     */
    m_correctLimit = (steer - racesteer);  // * 1.08;
  }  // if mode=correcting

  return steer;
}  // correctSteering


// try to limit sudden changes in steering
// to avoid loss of control through oversteer.
double KDriver::smoothSteering(double steercmd) {
  double speedfactor = (((60.0 -
       (MAX(40.0, MIN(70.0, getSpeed() + MAX(0.0, m_car->_accel_x * 5)))
        - 25)) / 300) * 2.5) / 0.585;
  // double rearskid = MAX(0.0,
  //                        MAX(m_car->_skid[2], m_car->_skid[3])
  //                        - MAX(m_car->_skid[0], m_car->_skid[1]));

  if (fabs(steercmd) < fabs(m_laststeer)
      && fabs(steercmd) <= fabs(m_laststeer - steercmd))
    speedfactor *= 2;

  double lftspeedfactor = speedfactor - MIN(0.0f, m_car->_yaw_rate / 10.0);
  double rgtspeedfactor = speedfactor + MAX(0.0f, m_car->_yaw_rate / 10.0);

  steercmd = MAX(m_laststeer - rgtspeedfactor,
                        MIN(m_laststeer + lftspeedfactor, steercmd));
  return steercmd;
}  // smoothSteering


/**
 * getSteer
 * Computes steer value.
 * @param s global situation
 * @return steer value range -1...1
 */
double KDriver::getSteer(tSituation *s) {
  double steer = 0.0;

  m_raceline->GetRaceLineData(s, &m_raceTarget,
                              &m_raceSpeed, &m_avoidSpeed,
                              &m_raceOffset, &m_rLookahead,
                              &m_raceSteer);
  vec2f target = getTargetPoint();

  double targetAngle =
    atan2(target.y - m_car->_pos_Y, target.x - m_car->_pos_X);
  double avoidsteer = calcSteer(targetAngle, 0);

  if (m_mode == PITTING)
    return avoidsteer;

  targetAngle =
    atan2(m_raceTarget.y - m_car->_pos_Y, m_raceTarget.x - m_car->_pos_X);
  // uncomment the following if we want to use BT steering rather than K1999
  // m_raceSteer = calcSteer( targetAngle, 1 );

  if (m_mode == AVOIDING &&
     (!m_avoidmode
      || (m_avoidmode == AVOIDRIGHT
          && m_raceOffset >= m_myoffset
          && m_raceOffset < m_avoidLftOffset)
      || (m_avoidmode == AVOIDLEFT
          && m_raceOffset <= m_myoffset
          && m_raceOffset > m_avoidRgtOffset))) {
    // we're avoiding, but trying to steer somewhere the raceline takes us.
    // hence we'll just correct towards the raceline instead.
    setMode(CORRECTING);
  }

  if (m_mode == CORRECTING
      && (m_lastmode == NORMAL
          || (fabs(m_angle) < 0.2f && fabs(m_raceSteer) < 0.4f
          && fabs(m_laststeer - m_raceSteer) < 0.05
          && ((fabs(m_car->_trkPos.toMiddle)
                < m_car->_trkPos.seg->width / 2 - 1.0)
          || m_car->_speed_x < 10.0) && m_raceline->isOnLine()))) {
    // we're CORRECTING & are now close enough to the raceline to
    // switch back to 'normal' mode...
    setMode(NORMAL);
  }

  if (m_mode == NORMAL) {
    steer = m_raceSteer;
    m_lastNSasteer = m_raceSteer * 0.8;
  } else {
    if (m_mode != CORRECTING) {
      m_correctLimit = 1000.0;
      m_correctTimer = m_simTime + 7.0;
      steer = smoothSteering(avoidsteer);
    } else {
      steer = smoothSteering(correctSteering(avoidsteer, m_raceSteer));
    }  // mode != CORRECTING

    if (fabs(m_angle) >= 1.6) {
      steer = (steer > 0.0) ? 1.0 : -1.0;
    }
  }  // mode != NORMAL

#if 0
  fprintf(stderr,
      "%s %d: %c%c %.3f (a%.3f k%.3f) cl=%.3f ct=%.1f myof=%.3f->%.3f\n",
      m_car->_name, m_car->_dammage,
      (m_mode ==
       NORMAL ? 'n' : (m_mode ==
               AVOIDING ? 'a' : (m_mode ==
                         CORRECTING ? 'c' : 'p'))),
      (m_avoidmode ==
       AVOIDLEFT ? 'l' : (m_avoidmode ==
                  AVOIDRIGHT ? 'r' : (m_avoidmode ==
                          (AVOIDLEFT +
                           AVOIDRIGHT) ? 'b' :
                          ' '))), steer,
      avoidsteer, m_raceSteer, m_correctLimit, (m_correctTimer - m_simTime),
      m_car->_trkPos.toMiddle, m_myoffset);
  fflush(stderr);
#endif
  return steer;
}  // getSteer


/**
 * getTargetPoint
 * Computes target point for steering.
 * @return 2D coords of the target
 */
vec2f KDriver::getTargetPoint() {
  double lookahead;

  if (m_pit->getInPit()) {
    // To stop in the pit we need special lookahead values.
    lookahead = PIT_LOOKAHEAD;
    if (m_currentSpeedSqr > m_pit->getSpeedlimitSqr())
      lookahead += m_car->_speed_x * LOOKAHEAD_FACTOR;
  } else {
    // Usual lookahead.
    double speed = MAX(20.0, getSpeed());  // + MAX(0.0, m_car->_accel_x));
    lookahead = LOOKAHEAD_CONST * 1.2 + speed * 0.60;
    lookahead = MIN(lookahead, LOOKAHEAD_CONST + (pow(speed, 2) / 7 * 0.15));

    // Prevent "snap back" of lookahead on harsh braking.
    double cmplookahead = m_oldLookahead - m_car->_speed_x * RCM_MAX_DT_ROBOTS;
    lookahead = MAX(cmplookahead, lookahead);
  }  // if getInPit

  lookahead *= m_filterLookaheadSkill;

  m_oldLookahead = lookahead;

  // Search for the segment containing the target point.
  tTrackSeg *seg = m_car->_trkPos.seg;
  double length = getDistToSegEnd();
  while (length < lookahead) {
    seg = seg->next;
    length += seg->length;
  }

  length = lookahead - length + seg->length;
  double fromstart = seg->lgfromstart + length;

  // Compute the target point.
  double offset = getOffset();
  double pitoffset = m_pit->getPitOffset(-100.0, fromstart);
  if ((m_pit->getPitstop() || m_pit->getInPit()) && pitoffset != -100.0) {
    setMode(PITTING);
    offset = m_myoffset = pitoffset;
  } else if (m_mode == PITTING) {
    setMode(CORRECTING);
  }

  vec2f s;
  if (m_mode != PITTING) {
    m_raceline->GetPoint(offset, lookahead, &s);
    return s;
  }

  s.x = (seg->vertex[TR_SL].x + seg->vertex[TR_SR].x) / 2.0;
  s.y = (seg->vertex[TR_SL].y + seg->vertex[TR_SR].y) / 2.0;

  if (seg->type == TR_STR) {
    vec2f n((seg->vertex[TR_EL].x - seg->vertex[TR_ER].x) / seg->length,
      (seg->vertex[TR_EL].y - seg->vertex[TR_ER].y) / seg->length);
    n.normalize();
    vec2f d((seg->vertex[TR_EL].x - seg->vertex[TR_SL].x) / seg->length,
      (seg->vertex[TR_EL].y - seg->vertex[TR_SL].y) / seg->length);
    return s + d * length + static_cast<float>(offset) * n;
  } else {
    vec2f c(seg->center.x, seg->center.y);
    double arc = length / seg->radius;
    double arcsign = (seg->type == TR_RGT) ? -1.0 : 1.0;
    arc = arc * arcsign;
    s = s.rotate(c, arc);

    vec2f n, t, rt;
    n = c - s;
    n.normalize();
    t = s + static_cast<float>(arcsign) * static_cast<float>(offset) * n;

    if (m_mode != PITTING) {
      // bugfix - calculates target point a different way, thus
      // bypassing an error in the BT code that sometimes made
      // the target closer to the car than lookahead...
      m_raceline->GetPoint(offset, lookahead, &rt);
      double dx = t.x - m_car->_pos_X;
      double dy = t.y - m_car->_pos_Y;
      double dist1 = Mag(dx, dy);
      dx = rt.x - m_car->_pos_X;
      dy = rt.y - m_car->_pos_Y;
      double dist2 = Mag(dx, dy);
      if (dist2 > dist1)
        t = rt;
    }  // if !PITTING

    return t;
  }  // if seg->type
}  // getTargetPoint


/** 
 * getDistToSegEnd
 * Computes the length to the end of the segment.
 * 
 * @return distance to segment end, in meter.
 */
double KDriver::getDistToSegEnd() {
  const tTrackSeg *seg = m_car->_trkPos.seg;
  double ret = (seg->type == TR_STR)
    ? seg->length - m_car->_trkPos.toStart  // simple when straight
    : (seg->arc - m_car->_trkPos.toStart) * seg->radius;  // uhm, arc
  return ret;
}  // getDistToSegEnd


void KDriver::setMode(int newmode) {
  if (m_mode != newmode) {
    if (m_mode == NORMAL || m_mode == PITTING) {
      m_correctTimer = m_simTime + 7.0;
      m_correctLimit = 1000.0;
    }
    m_mode = newmode;
  }  // m_mode != newmode
}  // setMode


/** 
 * getAccel
 * Computes fitting acceleration.
 * 
 * @return Acceleration scaled 0-1.
 */
double KDriver::getAccel() {
  double ret = 1.0;

  if (m_car->_gear > 0) {
    m_accelCmd = MIN(1.0, m_accelCmd);
    if (fabs(m_angle) > 0.8 && getSpeed() > 10.0) {
      m_accelCmd = MAX(0.0, MIN(m_accelCmd,
                                1.0 - getSpeed() / 100.0 * fabs(m_angle)));
    }
    ret = m_accelCmd;
    ret *= (m_car->_gear > 1) ? m_filterAccelSkill : 1.0;
  }  // if m_car->_gear

  return ret;
}  // getAccel


/**
 * getBrake
 * Computes initial brake value.
 * 
 * @return brake scaled 0-1
 */
double KDriver::getBrake() {
  double ret;

  if (m_car->_speed_x < -MAX_UNSTUCK_SPEED) {
    ret = 1.0;  // Car drives backward, brake
  } else {
    ret = m_brakeCmd;   // Car drives forward, normal braking.
    // ret *= m_filterBrakeSkill;  //Brake earlier if less skilled
  }

  return ret;
}  // getBrake


// Compute gear.
int KDriver::getGear() {
  int ret = m_car->_gear <= 0 ? 1 : m_car->_gear;

  if (m_car->_gear > 0) {
    double gr_up = m_car->_gearRatio[m_car->_gear + m_car->_gearOffset];
    double omega = m_car->_enginerpmRedLine / gr_up;
    double wr = m_car->_wheelRadius(2);

    if (omega * wr * SHIFT < m_car->_speed_x) {
      ret = m_car->_gear + 1;
    } else {
      double gr_down = m_car->_gearRatio[m_car->_gear + m_car->_gearOffset - 1];
      omega = m_car->_enginerpmRedLine / gr_down;
      if (m_car->_gear > 1
          && omega * wr * SHIFT > m_car->_speed_x + SHIFT_MARGIN)
        ret = m_car->_gear - 1;
    }
  }  // if gear > 0

  return ret;
}  // getGear


// Compute the clutch value.
double KDriver::getClutch() {
  // ???
  if (1 || m_car->_gearCmd > 1) {
    double maxtime = MAX(0.06, 0.32 - (m_car->_gearCmd / 65.0));
    if (m_car->_gear != m_car->_gearCmd)
      m_clutchTime = maxtime;
    if (m_clutchTime > 0.0)
      m_clutchTime -= (RCM_MAX_DT_ROBOTS *
         (0.02 + (m_car->_gearCmd / 8.0)));
    return 2.0 * m_clutchTime;
  } else {  // unreachable code???
    double drpm = m_car->_enginerpm - m_car->_enginerpmRedLine / 2.0;
    double ctlimit = 0.9;
    if (m_car->_gearCmd > 1)
      ctlimit -= 0.15 + m_car->_gearCmd / 13.0;
    m_clutchTime = MIN(ctlimit, m_clutchTime);
    if (m_car->_gear != m_car->_gearCmd)
      m_clutchTime = 0.0;
    double clutcht = (ctlimit - m_clutchTime) / ctlimit;
    if (m_car->_gear == 1 && m_car->_accelCmd > 0.0)
      m_clutchTime += RCM_MAX_DT_ROBOTS;

    if (m_car->_gearCmd == 1 || drpm > 0) {
      double speedr;
      if (m_car->_gearCmd == 1) {
        // Compute corresponding speed to engine rpm.
        double omega =
          m_car->_enginerpmRedLine / m_car->_gearRatio[m_car->_gear +
                       m_car->_gearOffset];
        double wr = m_car->_wheelRadius(2);
        speedr = (CLUTCH_SPEED +
                  MAX(0.0, m_car->_speed_x)) / fabs(wr * omega);
        double clutchr = MAX(0.0,
            (1.0 - speedr * 2.0 * drpm /
             m_car->_enginerpmRedLine)) *
            (m_car->_gearCmd ==
              1 ? 0.95 : (0.7 - (m_car->_gearCmd) / 30.0));
        return MIN(clutcht, clutchr);
      } else {
        // For the reverse gear.
        m_clutchTime = 0.0;
        return 0.0;
      }
    } else {
      return clutcht;
    }
  }
}  // getClutch


/** 
 * initSkill
 * Reads the global and the driver-specific skill values.
 * 
 * @return The accumulated skill value
 */
double KDriver::initSkill(tSituation * s) {
  double globalSkill = m_skill = 0.0;
  m_filterBrakeSkill = 1.0;
  m_filterAccelSkill = 1.0;
  m_filterLookaheadSkill = 1.0;
  m_filterSideSkill = 1.0;

  if (s->_raceType != RM_TYPE_PRACTICE) {
    stringstream buf;
    // load the global skill level, range 0 - 10
    buf << GetLocalDir() << "config/raceman/extra/skill.xml";
    void *skillHandle = GfParmReadFile(buf.str().c_str(), GFPARM_RMODE_REREAD);
    if (!skillHandle) {
      buf.str(string());
      buf << GetDataDir() << "config/raceman/extra/skill.xml";
      skillHandle = GfParmReadFile(buf.str().c_str(), GFPARM_RMODE_REREAD);
    }  // if !skillHandle

    if (skillHandle) {
      globalSkill = GfParmGetNum(skillHandle, KILO_SECT_SKILL,
                                 KILO_SKILL_LEVEL, NULL, 10.0);
      globalSkill = MIN(10.0, MAX(0.0, globalSkill));
    }

    // load the driver skill level, range 0 - 1
    double driverSkill = 0.0;
    buf.str(string());
    buf << "drivers/" << bot << "/" << INDEX << "/skill.xml";
    skillHandle = GfParmReadFile(buf.str().c_str(), GFPARM_RMODE_STD);
    if (skillHandle) {
      driverSkill = GfParmGetNum(skillHandle, KILO_SECT_SKILL,
                                 KILO_SKILL_LEVEL, NULL, 0.0);
      // driver_aggression = GfParmGetNum(skillHandle, SECT_SKILL,
      //                                  KILO_SKILL_AGGRO, NULL, 0.0);
      driverSkill = MIN(1.0, MAX(0.0, driverSkill));
    }

    // limits: 0.0 - 24.0
    m_skill = (globalSkill + driverSkill * 2.0) * (1.0 + driverSkill);

    // Set up different handicap values
    m_filterBrakeSkill = MAX(0.85, 1.0 - m_skill / 24.0 * 0.15);
    m_filterAccelSkill = MAX(0.80, 1.0 - m_skill / 24.0 * 0.20);
    m_filterLookaheadSkill = 1.0 / (1.0 + m_skill / 24.0);
    m_filterSideSkill = 1.0 + m_skill / 24.0;
  }   // if not practice

#ifdef DEBUG
  cout << "Skill: " << m_skill
    << ", brake: " << m_filterBrakeSkill
    << ", accel: " << m_filterAccelSkill
    << ", lookA: " << m_filterLookaheadSkill
    << ", sides: " << m_filterSideSkill
    << endl;
#endif

  return m_skill;
}   // initSkill
