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

#include "kdriver.h"

#include <iostream>
#include <string>
#include <sstream>

#include "opponent.h"
#include "strategy.h"
#include "pit.h"
#include "raceline.h"
#include "util.h"

#include <portability.h> //snprintf under MSVC
#include <robottools.h> //Rt*
#include <robot.h>  //ROB_PIT_IM

#define DEBUG

//"I AM DEATH, NOT TAXES.  *I* TURN UP ONLY ONCE."  --  Death
//Fear was theirs, not yers.

static int
  pitstatus[128] = { 0 };
static double colour[] = {1.0, 0.0, 0.0, 0.0};
#define DEFAULTCARTYPE "trb1-cavallo-360rb"

#define SLOW_TRACK_LIMIT 2.4
#define FAST_TRACK_LIMIT 4.0


KDriver::KDriver(int index):Driver(index)
{
  m_rgtinc = m_lftinc = 0.0;
  m_minoffset = m_maxoffset = 0.0;
  m_rInverse = 0.0;
}


/**
 * Drive during the race.
 * 
 * @param[in] s Situation provided by the sim.
 */
void
KDriver::drive(tSituation * s)
{
  memset(&m_car->ctrl, 0, sizeof(tCarCtrl));
  update(s);
  //~ sprintf(m_m_car->_msgCmd[0], "%d", (int)(m_m_car->_distFromStartLine));
  //~ memcpy(m_m_car->_msgColorCmd, colour, sizeof(m_m_car->_msgColorCmd));
  
  string sMsg;
  if(isStuck()) {
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
    if(m_car->_brakeCmd == 0.0) {
      m_car->_accelCmd = filterTCL(filterTrk(filterOverlap(getAccel())));
      sMsg = "Thig comhla ruinn!";
    } else {
      m_car->_accelCmd = 0.0;
      sMsg = "Sguir!";
    }
    m_car->_clutchCmd = getClutch();
  }//if isStuck

  sprintf(m_car->_msgCmd[0], "%s", sMsg.c_str());
  memcpy(m_car->_msgColorCmd, colour, sizeof(m_car->_msgColorCmd));

  m_laststeer = m_car->_steerCmd;
  m_lastmode = m_mode;
}//drive


/**
 * Checks if I'm stuck.
 * 
 * @return true if stuck
 */
bool
KDriver::isStuck()
{
  bool ret = false;
  
  if(fabs(m_mycardata->getCarAngle()) > MAX_UNSTUCK_ANGLE &&
        m_car->_speed_x < MAX_UNSTUCK_SPEED &&
        fabs(m_car->_trkPos.toMiddle) > MIN_UNSTUCK_DIST) {
    if(m_stuckCounter > MAX_UNSTUCK_COUNT
      && m_car->_trkPos.toMiddle * m_mycardata->getCarAngle() < 0.0)
        ret = true;
    else
        m_stuckCounter++;
  } else {
    m_stuckCounter = 0;
  }
  
  return ret;
}//isStuck


/**
 * Reduces the brake value such that it fits the speed
 * (more downforce -> more braking).
 * 
 * @param[in] brake Original braking value
 * @return  Modified braking value
 */
double
KDriver::filterBrakeSpeed(double brake)
{
  double weight = m_mass * G;
  double maxForce = weight + CA * MAX_SPEED * MAX_SPEED;
  double force = weight + CA * m_currentSpeedSqr;
  return brake * force / maxForce;
}//filterBrakeSpeed


// Compute offset to normal target point for overtaking or let pass an opponent.
double
KDriver::getOffset()
{
  m_mincatchdist = 500.0;
  Opponent *o = NULL;
  
  m_myoffset = m_car->_trkPos.toMiddle;
  m_avoidmode = 0;
  m_avoidLftOffset = MAX(m_myoffset, m_car->_trkPos.seg->width / 2.0 - 1.5);
  m_avoidRgtOffset = MIN(m_myoffset, -(m_car->_trkPos.seg->width / 2.0 - 1.5));

  // Increment speed dependent.
  m_rInverse = m_raceline->getRInverse();
  double incspeed = MIN(60.0, MAX(45.0, getSpeed())) - 18.0;
  double incfactor = (MAX_INC_FACTOR - MIN(fabs(incspeed) / MAX_INC_FACTOR, (MAX_INC_FACTOR - 1.0))) * (12.0 + MAX(0.0, (CA-1.9) * 14));
  m_rgtinc = incfactor * MIN(1.3, MAX(0.4, 1.0 + m_rInverse * (m_rInverse < 0.0 ? 20 : 80)));
  m_lftinc = incfactor * MIN(1.3, MAX(0.4, 1.0 - m_rInverse * (m_rInverse > 0.0 ? 20 : 80)));
  
  int offlft = m_myoffset > m_car->_trkPos.seg->width / 2 - 1.0;
  int offrgt = m_myoffset < -(m_car->_trkPos.seg->width / 2 - 1.0);
  
  if (offlft)
    m_myoffset -= OVERTAKE_OFFSET_INC * m_rgtinc / 2;
  else if (offrgt)
    m_myoffset += OVERTAKE_OFFSET_INC * m_lftinc / 2;
    
  m_avoidLftOffset = MAX(m_avoidLftOffset, m_myoffset - OVERTAKE_OFFSET_INC * m_rgtinc * (offlft ? 6 : 2));
  m_avoidRgtOffset = MIN(m_avoidRgtOffset, m_myoffset + OVERTAKE_OFFSET_INC * m_lftinc * (offrgt ? 6 : 2));
  
  m_maxoffset = m_track->width / 2 - m_car->_dimension_y; // limit to the left
  //m_minoffset = -(m_track->width / 2 - m_car->_dimension_y); // limit to the right
  m_minoffset = -m_maxoffset; // limit to the right

  if (m_myoffset < m_minoffset) // we're already outside right limit, bring us back towards track
    {
      m_minoffset = m_myoffset + OVERTAKE_OFFSET_INC * m_lftinc;
      m_maxoffset = MIN(m_maxoffset, m_myoffset + OVERTAKE_OFFSET_INC * m_lftinc * 2);
    }
  else if (m_myoffset > m_maxoffset) // outside left limit, bring us back
    {
      m_maxoffset = m_myoffset - OVERTAKE_OFFSET_INC * m_rgtinc;
      m_minoffset = MAX(m_minoffset, m_myoffset - OVERTAKE_OFFSET_INC * m_rgtinc * 2);
    }
  else
    {
      // set tighter limits based on how far we're allowed to move
      m_maxoffset = MIN(m_maxoffset, m_myoffset + OVERTAKE_OFFSET_INC * m_lftinc * 2);
      m_minoffset = MAX(m_minoffset, m_myoffset - OVERTAKE_OFFSET_INC * m_rgtinc * 2);
    }

  //Check for side collision
  o = getSidecollOpp();
  if(o != NULL)
    return filterSidecollOffset(o, incfactor);
    
    
  // If we have someone to take over, let's try it
  o = getTakeoverOpp();
  if(o != NULL)
    return filterTakeoverOffset(o);

  
  // If there is someone overlapping, move out of the way
  o = getOverlappingOpp();
  if(o != NULL)
    return filterOverlappedOffset(o);

  
  // no-one to avoid, work back towards raceline
  if(m_mode != NORMAL && fabs(m_myoffset - m_raceOffset) > 1.0) {
    if (m_myoffset > m_raceOffset + OVERTAKE_OFFSET_INC * m_rgtinc / 4)
      m_myoffset -= OVERTAKE_OFFSET_INC * m_rgtinc / 4;
    else if (m_myoffset < m_raceOffset + OVERTAKE_OFFSET_INC * m_lftinc / 4)
      m_myoffset += OVERTAKE_OFFSET_INC * m_lftinc / 4;
  } // if m_mode
    
  if(m_simTime > 2.0) {
    if(m_myoffset > m_raceOffset)
      m_myoffset = MAX(m_raceOffset, m_myoffset - OVERTAKE_OFFSET_INC * incfactor / 2);
    else
      m_myoffset = MIN(m_raceOffset, m_myoffset + OVERTAKE_OFFSET_INC * incfactor / 2);
  } // if m_simTime

  m_myoffset = MIN(m_maxoffset, MAX(m_minoffset, m_myoffset));
  return m_myoffset;
}//getOffset


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
Opponent *
KDriver::getOverlappingOpp()
{
  Opponent *ret = NULL;
  double mindist = -1000.0;
  
  for(list<Opponent>::iterator it = m_opponents->begin();
        it != m_opponents->end();
        it++) {
    tCarElt *ocar = it->getCarPtr();
    double oppDistance = it->getDistance();
      
    if((//if teammate has more laps under his belt,
      (it->isTeammate() && ocar->race.laps > m_car->race.laps)
      || //or teammate is less damaged, let him go
      it->isQuickerTeammate(m_car))
      && (oppDistance > -TEAM_REAR_DIST)  //if close enough
      && (oppDistance < -m_car->_dimension_x)) {
      // Behind, larger distances are smaller ("more negative").
      if(oppDistance > mindist) {
        mindist = oppDistance;
        ret = &(*it);
      }
    } else if(it->isState(OPP_LETPASS)) {
      // Behind, larger distances are smaller ("more negative").
      if(oppDistance > mindist) {
        mindist = oppDistance;
        ret = &(*it);
      }
    } // else if
  } // for i
  
  return ret;
}//getOverlappingOpp


/**
 * Modifies the member 'm_myoffset' so that the car moves out of the way
 * of the overlapping opponent.
 * 
 * @param [in] o: the opponent we should let go
 * @return    new offset. Equals member 'm_myoffset'
 * 
 */
double
KDriver::filterOverlappedOffset(Opponent *o)
{
  double w = m_car->_trkPos.seg->width / WIDTHDIV - BORDER_OVERTAKE_MARGIN;
  
  if(o->isOnRight(m_car->_trkPos.toMiddle)) {
    if(m_myoffset < w)
      m_myoffset += OVERTAKE_OFFSET_INC * m_lftinc / 1;//2;
  } else {
    if(m_myoffset > -w)
      m_myoffset -= OVERTAKE_OFFSET_INC * m_rgtinc / 1;//2;
  }
  setMode(BEING_OVERLAPPED);

  m_myoffset = MIN(m_avoidLftOffset, MAX(m_avoidRgtOffset, m_myoffset));
  return m_myoffset;
}//filterOverlappedOffset


/** 
 * If there is an opponent overlapping us, reduce accelerator.
 *
 * @param [in]  accel: original acceleration value
 * @return      possibly reduced acceleration value
 */
double
KDriver::filterOverlap(double accel)
{
  return (m_opponents->getOppByState(OPP_LETPASS)
    ? MIN(accel, LET_OVERTAKE_FACTOR)
    : accel);
}//filterOverlap


/**
 * If opponent is too much on either side of the track,
 * (doesn't occupy center part of the segment)
 * and we are 5+ metres far
 * 
 * @param [in]  ocar  the opponent car
 * @return      true if the opp. is too far on either side
*/
bool
KDriver::oppTooFarOnSide(tCarElt *ocar)
{
  bool ret = false;
  if(fabs(ocar->_trkPos.toMiddle) > m_car->_trkPos.seg->width / 2 + 3.0
    && fabs(m_car->_trkPos.toMiddle - ocar->_trkPos.toMiddle) >= 5.0)
    ret = true;
  return ret;
}//oppTooFarOnSide


/**
 * Decide if there is a car ahead we can take over.
 * 
 * @return  Overlap car pointer or NULL
 */
Opponent*
KDriver::getTakeoverOpp()
{
  Opponent *ret = NULL;

  m_mincatchdist = MAX(30.0, 1500.0 - fabs(m_rInverse) * 10000);
  int otrySuccess = 0;
  
  for(int otry = 0; otry <= 1; otry++) {
    for(list<Opponent>::iterator it = m_opponents->begin();
        it != m_opponents->end();
        it++) {
      tCarElt *ocar = it->getCarPtr();

      // If opponent is clearly ahead of us, we don't care
      if(it->isState(OPP_FRONT_FOLLOW))
        continue;

      if(oppTooFarOnSide(ocar))
        continue;

      // If opponent is in pit, let him be ;)
      if(ocar->_state > RM_CAR_STATE_PIT)
        continue;

      // If opponent is ahead, and is not a quicker teammate of ours
      if((it->isState(OPP_FRONT))
          && !it->isQuickerTeammate(m_car)) {
        double otry_factor = (otry ? (0.2 + (1.0 - ((m_currentSimTime - m_avoidTime) / 7.0)) * 0.8) : 1.0);
        double distance = it->getDistance() * otry_factor;  //how far ahead is he
        double speed = MIN(m_avoidSpeed, getSpeed() + MAX(0.0, 10.0 - distance));
        double ospeed = it->getSpeed();   //opponent's speed
        double catchdist = MIN(speed * distance / (speed - ospeed),
                          distance * CATCH_FACTOR) * otry_factor;   //when will we reach the opponent

        //if we are close enough, check again with avoidance speed taken into account
        if(catchdist < m_mincatchdist && distance < fabs(speed - ospeed) * 2) {
          m_mincatchdist = catchdist;
          ret = &(*it); //This is the guy we need to take over
          otrySuccess = otry;
        }
      }//if it state
    } //for it
    if (ret) break;
    if (m_mode != AVOIDING) break;
  } //for otry

  if(ret != NULL && otrySuccess == 0)
    m_avoidTime = m_currentSimTime;

  return ret;
}//getTakeoverOpp


/**
 * Change offset value if  we are to overtake a car.
 *
 * @param [in]  o the opponent
 * @return      new offset
 */
double
KDriver::filterTakeoverOffset(Opponent *o)
{
  setMode(AVOIDING);
  tCarElt *ocar = o->getCarPtr();

  // Compute the opponent's distance to the middle.
  double otm = ocar->_trkPos.toMiddle;
  double sidemargin = o->getWidth() + getWidth() + 1.0;
  double sidedist = fabs(ocar->_trkPos.toLeft - m_car->_trkPos.toLeft);

  // avoid more if on the outside of opponent on a bend.
  // Stops us from cutting in too much and colliding...
  if ((otm < -(ocar->_trkPos.seg->width - 5.0) && m_rInverse < 0.0) ||
      (otm > (ocar->_trkPos.seg->width - 5.0) && m_rInverse > 0.0))
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
      switch(seg->type) {
        case TR_LFT:
          lenleft += seglen;
          break;
        case TR_RGT:
          lenright += seglen;
          break;
        default:
          // Do nothing.
          break;
      }//switch seg->type
      seg = seg->next;
      seglen = seg->length;
      oldlen = length;
      length += seglen;
    } while(oldlen < m_mincatchdist);

    // If we are on a straight look for the next turn.
    if(lenleft == 0.0 && lenright == 0.0) {
      while(seg->type == TR_STR)
        seg = seg->next;
        
      // Assume: left or right if not straight.
      if(seg->type == TR_LFT)
        lenleft = 1.0;
      else
        lenright = 1.0;
    }//if lenleft/lenright == 0

    // Because we are inside we can go to the limit.
    if ((lenleft > lenright && m_rInverse < 0.0) ||
       (lenleft <= lenright && m_rInverse > 0.0)) {
        // avoid more if on the outside of opponent on a bend.  Stops us
        // from cutting in too much and colliding...
        sidemargin += fabs(m_rInverse) * 150;
    }
      
    if(sidedist < sidemargin || o->isState(OPP_COLL)) {
      if(lenleft > lenright) {
        m_myoffset += OVERTAKE_OFFSET_INC * m_lftinc;// * 0.7;
        setAvoidRight();
      } else {
        m_myoffset -= OVERTAKE_OFFSET_INC * m_rgtinc;// * 0.7;
        setAvoidLeft();
      }//if lenleft > lenright
    }//if sidedist
  }//if opp near middle
        
  m_myoffset = MIN(m_avoidLftOffset, MAX(m_avoidRgtOffset, m_myoffset));
  m_myoffset = MIN(m_maxoffset, MAX(m_minoffset, m_myoffset));
  return m_myoffset;
}//filterTakeoverOffset


/**
 * Decide if there is a car on the side we are to collide with...
 * 
 * @return  Side collision car pointer or NULL
 */
Opponent *
KDriver::getSidecollOpp()
{
  Opponent *ret = NULL;
  for(list<Opponent>::iterator it = m_opponents->begin();
        it != m_opponents->end();
        it++) {
    tCarElt *ocar = it->getCarPtr();

    if(ocar->_state > RM_CAR_STATE_PIT)   //Dont care for opponents in the pit
      continue;

    if(oppTooFarOnSide(ocar))
      continue;

    if(it->isState(OPP_SIDE)) {  //If opponent is on our side
      setMode(AVOIDING);
      ret = &(*it);
      break;
    } //if OPP_SIDE
  } //for it
  
  return ret;
}//getSidecollOpp


double
KDriver::filterSidecollOffset(Opponent *o, const double incfactor)
{
  double myToLeft = m_car->_trkPos.toLeft;
  double oppToLeft = o->getCarPtr()->_trkPos.toLeft;
  double sidedist = fabs(oppToLeft - myToLeft);
  double sidemargin = o->getWidth() + getWidth() + 2.0;
  
  bool oppOnRight = o->isOnRight(m_car->_trkPos.toMiddle);
  // avoid more if on the outside of opponent on a bend.
  // Stops us from cutting in too much and colliding...
  if((oppOnRight && m_rInverse < 0.0)
    || (!oppOnRight && m_rInverse > 0.0))
    sidemargin += fabs(m_rInverse) * 150;

  if(oppOnRight)
    sidemargin -= MIN(0.0, m_rInverse * 100);
  else
    sidemargin += MAX(0.0, m_rInverse * 100);
  sidedist = MIN(sidedist, sidemargin);

  if(sidedist < sidemargin) {
    double sdiff = 3.0 - (sidemargin - sidedist) / sidemargin;
    
    if(oppOnRight)    //He is on the right, we must move to the left
      m_myoffset += OVERTAKE_OFFSET_INC * m_lftinc * MAX(0.2, MIN(1.0, sdiff));
    else              //He is on the left, we must move to the right
      m_myoffset -= OVERTAKE_OFFSET_INC * m_rgtinc * MAX(0.2, MIN(1.0, sdiff));
  } else if(sidedist > sidemargin + 3.0) {
    if(m_raceOffset > m_myoffset + OVERTAKE_OFFSET_INC * incfactor)
      m_myoffset += OVERTAKE_OFFSET_INC * m_lftinc / 4;
    else if (m_raceOffset < m_myoffset - OVERTAKE_OFFSET_INC * incfactor)
      m_myoffset -= OVERTAKE_OFFSET_INC * m_rgtinc / 4;
  }

  oppOnRight ? setAvoidRight() : setAvoidLeft();
  m_avoidmode |= AVOIDSIDE;

  m_myoffset = MIN(m_maxoffset, MAX(m_minoffset, m_myoffset));
  return m_myoffset;
}//filterSidecollOffset


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
void
KDriver::initTrack(tTrack * t, void *carHandle, void **carParmHandle,
          tSituation * s)
{
  m_track = t;

  //Load a custom setup if one is available.
  char *trackname = strrchr(m_track->filename, '/') + 1;    //Ptr to the track filename
  stringstream buf;
  stringstream botPath;

  //Try to load the default setup
  botPath << "drivers/" << bot << "/"; //<< INDEX << "/";
  buf << botPath.str() << "default.xml";
  *carParmHandle = GfParmReadFile(buf.str().c_str(), GFPARM_RMODE_STD);
#ifdef DEBUG
  cout << "Default: " << buf.str() << endl;
  if(carParmHandle)
    cout << "default xml loaded" << endl;
#endif
    
  //Try to load the track-based informations
  buf.str(string());
  buf << botPath.str() << "tracks/" << trackname;
  void *newhandle = GfParmReadFile(buf.str().c_str(), GFPARM_RMODE_STD);
#ifdef DEBUG
  cout << "track-based: " << buf.str() << endl;
  if(newhandle)
    cout << "track-based xml loaded" << endl;
#endif  
  
  mergeCarSetups(*carParmHandle, newhandle);
#ifdef DEBUG
  cout << "merge #1" << endl;
#endif  
  
  //Discover somehow the name of the car used
  if(m_carType.empty()) {
    stringstream ssSection;
    ssSection << ROB_SECT_ROBOTS << "/" << ROB_LIST_INDEX << "/" << INDEX;
    m_carType = GfParmGetStr(carHandle, ssSection.str().c_str(),
        ROB_ATTR_CAR, DEFAULTCARTYPE);
  }//if carType empty
#ifdef DEBUG
  cout << "Cartype: " << m_carType << endl;
#endif  

  //Load setup tailored for car+track
  buf.str(string());
  buf << botPath.str() << m_carType << "/" << trackname;
  newhandle = GfParmReadFile(buf.str().c_str(), GFPARM_RMODE_STD);
#ifdef DEBUG
  cout << "custom setup: " << buf.str() << endl;
  if(newhandle)
    cout << "car+track xml loaded" << endl;
#endif  

  //If there is no tailored setup, let's load a default one
  // based on the track charateristics.
  if(!newhandle)
    newhandle = loadDefaultSetup();
    
  //~ mergeCarSetups(*carParmHandle, newhandle);
  //Merge the above two setups
  if(newhandle) {
    //If there is a default setup loaded, merge settings with custom track setup
    if(*carParmHandle)
      *carParmHandle =
        GfParmMergeHandles(*carParmHandle, newhandle,
               (GFPARM_MMODE_SRC | GFPARM_MMODE_DST |
                GFPARM_MMODE_RELSRC | GFPARM_MMODE_RELDST));
    //Otherwise no need to merge
    else
      *carParmHandle = newhandle;
  }//if newhandle
#ifdef DEBUG
  cout << "merge #2" << endl;
#endif  


  // Create a pit stop strategy object.
  m_strategy = new KStrategy();
  // Init fuel.
  m_strategy->setFuelAtRaceStart(t, carParmHandle, s, INDEX);

  m_raceline = new LRaceLine;

  // Load and set parameters.
  MU_FACTOR = GfParmGetNum(*carParmHandle, BT_SECT_PRIV, BT_ATT_MUFACTOR,
         (char *) NULL, 0.69f);

  m_pitOffset = GfParmGetNum(*carParmHandle, BT_SECT_PRIV, "PitOffset",
                (char *) NULL, 10.0);
  m_brakeDelay = GfParmGetNum(*carParmHandle, BT_SECT_PRIV, "BrakeDelay", (char*) NULL, 10.0);
  
  m_raceline->InitTrack(m_track, carParmHandle, s);
}//initTrack


/**
 * Update own private data on every timestep.
 * 
 * @param [in]  s situation provided by the sim
 */
void
KDriver::update(tSituation * s)
{
  // Update global car data (shared by all instances) just once per timestep.
  if(m_currentSimTime != s->currentTime) {
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
  m_strategy->update();
  
  checkPitStatus(s);
  m_pit->update();
  m_simTime = s->currentTime;

  double trackangle = RtTrackSideTgAngleL(&(m_car->_trkPos));
  m_angle = trackangle - m_car->_yaw;
  NORM_PI_PI(m_angle);
  m_angle = -m_angle;
}//update

/**
 * Checks if we need to plan a pitstop.
 * If yes, checks availability of the pit,
 * is it free or occupied by teammate.
 * 
 * @param [in]  s Situation provided by the sim
 */
void
KDriver::checkPitStatus(tSituation *s)
{
  //If our car is still in the race
  if(m_car->_state <= RM_CAR_STATE_PIT)
    {
      if(!m_pit->getPitstop()) //if no pit is planned yet
        {
          if((m_car->_distFromStartLine < m_pit->getNPitEntry() //and we are not in the pit range
                || m_car->_distFromStartLine > m_pit->getNPitEnd())
                || m_car->_fuel < 5.0) //or we are very low on fuel
            {
              m_pit->setPitstop(m_strategy->needPitstop()); //then we can plan a pit, if needed.
            }
        }
        
      if(m_pit->getPitstop() && m_car->_pit) //if pitting is planned
        {
          pitstatus[m_carIndex] = 1;  //flag our car's pit as occupied
          for(list<Opponent>::iterator it = m_opponents->begin();
            it != m_opponents->end();
            it++)
            {
              tCarElt *ocar = it->getCarPtr();
              //If the other car is our teammate, still in the race
              if(it->isTeammate() && ocar->_state <= RM_CAR_STATE_PIT)
                {
                  int idx = it->getIndex();
                  if(pitstatus[idx] == 1
                    || ((pitstatus[idx]
                        || (ocar-> _fuel < m_car->_fuel - 1.0 && m_car->_dammage < 5000))
                    && fabs(m_car->_trkPos.toMiddle) <= m_car->_trkPos.seg->width / 2))
                    {
                      m_pit->setPitstop(false);
                      pitstatus[m_carIndex] = 0;
                    }
                  break;
                } //if our teammate
            } //for it
        } //if pitting is planned
      else if(!m_pit->getInPit()) //If we are not in the pitlane
        pitstatus[m_carIndex] = 0;  //sign the pit as free
    }
  else
    pitstatus[m_carIndex] = 0;
}//checkPitStatus


/**
 * Brake filter for collision avoidance.
 * If there is an opponent we are to collide, brake brake brake!
 * 
 * @param [in]  brake Original brake value
 * @return  Possibly modified brake value
 */
double
KDriver::filterBColl(const double brake)
{
  double ret = brake;
  
  if(m_simTime >= 1.5) {
    double mu = m_car->_trkPos.seg->surface->kFriction;
    for(list<Opponent>::iterator it = m_opponents->begin();
          it != m_opponents->end();
          it++) {
      if(it->isState(OPP_COLL)) { //Endangered species
        double ospeed = it->getSpeed();
        if(brakedist(ospeed, mu) + MIN(1.0, 0.5 + MAX(0.0,(getSpeed() - ospeed) / 4))
            > it->getDistance()) {  //Damn, too close, brake hard!!!
          m_accelCmd = 0.0;
          ret = 1.0;
          break;
        }//if brakedist
      }//if state OPP_COLL
    }//for it
  }//if m_simTime
      
  return ret;
}//filterBColl


// Set pitstop commands.
int
KDriver::pitCommand(tSituation * s)
{
  m_car->_pitRepair = m_strategy->pitRepair();
  m_car->_pitFuel = m_strategy->pitRefuel();
  // This should be the only place where the pit stop is set to false!
  m_pit->setPitstop(false);
  return ROB_PIT_IM;        // return immediately.
}//pitCommand


void
KDriver::newRace(tCarElt * car, tSituation * s)
{
  m_strategy->setCar(car);
  Driver::newRace(car, s);
}//newRace


void
KDriver::calcSpeed()
{
  m_accelCmd = m_brakeCmd = 0.0;
  double speed;

  switch(m_mode) {
    case AVOIDING:
    case BEING_OVERLAPPED:
      speed = m_avoidSpeed;
      break;
      
    case CORRECTING:
      speed = m_raceSpeed -
              (m_raceSpeed - m_avoidSpeed) * MAX(0.0, (m_correctTimer - m_simTime) / 7.0);
      break;
          
    default:
      speed = m_raceSpeed;
  }//switch m_mode
    
  double x = (10 + m_car->_speed_x) * (speed - m_car->_speed_x) / 200;

  if(x > 0.0)
    m_accelCmd = x;
  else
    m_brakeCmd = MIN(1.0, -(MAX(10.0, m_brakeDelay * 0.7)) * x);
}//calcSpeed


/**
 * Decides the character of the track
 * and chooses 1 of 3 default setups.
 * Loads the appropriate setup file and
 * returns it's handler.
 * 
 * @return  Handler to the loaded default setup
 */
void *
KDriver::loadDefaultSetup() const
{
  void *ret = NULL;
  
  double dLength = 0.0;
  double dCurves = 0.0;
  
  //Count length and degrees of all turns
  tTrackSeg *pSeg = m_track->seg;
  do {
    if(pSeg->type == TR_STR)
      dLength += pSeg->length;
    else {
      dLength += pSeg->radius * pSeg->arc;
      dCurves += RAD2DEG(pSeg->arc);
    }

    pSeg = pSeg->next;
  } while(pSeg != m_track->seg);
  
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

  if(dRatio < SLOW_TRACK_LIMIT) {
    //Slow track
    buf << "/def-slow.xml";
  } else if (dRatio < FAST_TRACK_LIMIT) {
    //Normal track
    buf << "/def-norm.xml";
  } else {
    //Fast track
    buf << "/def-fast.xml";
  }//if dRatio

  ret = GfParmReadFile(buf.str().c_str(), GFPARM_RMODE_STD);
#ifdef DEBUG
  cout << "Decision of setup: " << buf.str() << endl;
  if(ret)
    cout << "Def-XXX xml loaded" << endl;
#endif  

  return ret;
}//loadDefaultSetup

void
KDriver::mergeCarSetups(void *oldHandle, void *newHandle)
{
  if(newHandle) {
    if(oldHandle)
      oldHandle = GfParmMergeHandles(oldHandle, newHandle,
          (GFPARM_MMODE_SRC | GFPARM_MMODE_DST |
          GFPARM_MMODE_RELSRC | GFPARM_MMODE_RELDST));
    //Otherwise no need to merge
    else
      oldHandle = newHandle;
  }//if newHandle
}//mergeCarSetups
