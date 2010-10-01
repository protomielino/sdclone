/*
 *      driver.cpp
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

#include "driver.h"
#include "opponent.h"
#include "pit.h"
#include "strategy.h"
#include "raceline.h"
#include "util.h" //sign
#include <robottools.h>

const double
  Driver::MAX_UNSTUCK_ANGLE = 15.0 / 180.0 * PI;    // [radians] If the angle of the car on the track is smaller, we assume we are not stuck.
const double
  Driver::UNSTUCK_TIME_LIMIT = 2.0;    // [s] We try to get unstuck after this time.
const double
  Driver::MAX_UNSTUCK_SPEED = 5.0; // [m/s] Below this speed we consider being stuck.
const double
  Driver::MIN_UNSTUCK_DIST = 3.0;  // [m] If we are closer to the middle we assume to be not stuck.
const double
  Driver::G = 9.81;        // [m/(s*s)] Welcome on Earth.
const double
  Driver::SHIFT = 0.96;    // [-] (% of rpmredline) When do we like to shift gears.
const double
  Driver::SHIFT_MARGIN = 4.5;  // [m/s] Avoid oscillating gear changes.
const double
  Driver::ABS_SLIP = 2.5;  // [m/s] range [0..10]
const double
  Driver::ABS_RANGE = 5.0; // [m/s] range [0..10]
const double
  Driver::ABS_MINSPEED = 3.0;  // [m/s] Below this speed the ABS is disabled (numeric, division by small numbers).
const double
  Driver::TCL_SLIP = 2.0;  // [m/s] range [0..10]
const double
  Driver::TCL_RANGE = 10.0;    // [m/s] range [0..10]
const double
  Driver::LOOKAHEAD_CONST = 18.0;  // [m]
const double
  Driver::LOOKAHEAD_FACTOR = 0.33; // [-]
const double
  Driver::WIDTHDIV = 2.0;  // [-] Defines the percentage of the track to use (2/WIDTHDIV).
const double
  Driver::BORDER_OVERTAKE_MARGIN = 1.0;    // [m]
const double
  Driver::OVERTAKE_OFFSET_SPEED = 5.0; // [m/s] Offset change speed.
const double
  Driver::PIT_LOOKAHEAD = 6.0; // [m] Lookahead to stop in the pit.
const double
  Driver::PIT_BRAKE_AHEAD = 200.0; // [m] Workaround for "broken" pitentries.
const double
  Driver::PIT_MU = 0.4;    // [-] Friction of pit concrete.
const double
  Driver::MAX_SPEED = 350.0 / 3.6;    // [m/s] Speed to compute the percentage of brake to apply., 350 km/h
const double
  Driver::CLUTCH_SPEED = 5.0;  // [m/s]
const double
  Driver::DISTCUTOFF = 500.0;  // [m] How far to look, terminate while loops.
const double
  Driver::MAX_INC_FACTOR = 6.0;    // [m] Increment faster if speed is slow [1.0..10.0].
const double
  Driver::CATCH_FACTOR = 10.0; // [-] select MIN(catchdist, dist*CATCH_FACTOR) to overtake.
const double
  Driver::TEAM_REAR_DIST = 50.0;   //
const int
  Driver::TEAM_DAMAGE_CHANGE_LEAD = 800;    // When to change position in the team?
const double
  Driver::LET_OVERTAKE_FACTOR = 0.6;  //Reduce speed with this factor when being overlapped
  
// Static variables.
Cardata *
  Driver::m_cardata = NULL;
double
  Driver::m_currentSimTime;
static const char *WheelSect[4] = { SECT_FRNTRGTWHEEL, SECT_FRNTLFTWHEEL,
                        SECT_REARRGTWHEEL, SECT_REARLFTWHEEL };


Driver::Driver(int index)
{
  INDEX = index;
}


Driver::~Driver()
{
  delete m_raceline;
  delete m_opponents;
  delete m_pit;
  delete m_strategy;
  if(m_cardata != NULL) {
    delete m_cardata;
    m_cardata = NULL;
  }
}


// Start a new race.
void
Driver::newRace(tCarElt * car, tSituation * s)
{
  double deltaTime = (double) RCM_MAX_DT_ROBOTS;
  MAX_UNSTUCK_COUNT = int (UNSTUCK_TIME_LIMIT / deltaTime);
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
  initTCLfilter();

  // Create just one instance of cardata shared by all drivers.
  if(m_cardata == NULL)
    m_cardata = new Cardata(s);
  m_mycardata = m_cardata->findCar(m_car);
  m_currentSimTime = s->currentTime;

  // initialize the list of opponents.
  m_opponents = new Opponents(s, this, m_cardata);
  m_opponents->setTeamMate(m_car);
  
  // create the pit object.
  m_pit = new Pit(s, this, m_pitOffset);
  
  // set initial mode
  //we set it to CORRECTING so the robot will steer towards the raceline
  setMode(CORRECTING);
  m_lastmode = CORRECTING;

  for(m_carIndex = 0; m_carIndex < s->_ncars; m_carIndex++) {
    if(s->cars[m_carIndex] == m_car)
      break;
  }

  m_raceline->setCar(m_car);
  m_raceline->NewRace();
}//newRace


void
Driver::calcSpeed()
{
  m_accelCmd = m_brakeCmd = 0.0;
  double speed = m_raceSpeed;

  if(m_mode == AVOIDING || m_mode == BEING_OVERLAPPED)
    speed = m_avoidSpeed;
  else if(m_mode == CORRECTING)
    speed = m_raceSpeed - (m_raceSpeed - m_avoidSpeed) * MAX(0.0,
                         (m_correctTimer - m_simTime) / 7.0);

  double x = (10 + m_car->_speed_x) * (speed - m_car->_speed_x) / 200;

  if(x > 0)
    m_accelCmd = x;
  else
    m_brakeCmd = MIN(1.0, -(MAX(10.0, m_brakeDelay * 0.7)) * x);
}


// Drive during race.
void
Driver::drive(tSituation * s)
{
  memset(&m_car->ctrl, 0, sizeof(tCarCtrl));

  update(s);

  //m_pit->setPitstop(true);

  if(isStuck()) {
    m_car->_steerCmd = -m_mycardata->getCarAngle() / m_car->_steerLock;
    m_car->_gearCmd = -1;     // Reverse gear.
    m_car->_accelCmd = 1.0;   // 100% accelerator pedal.
    m_car->_brakeCmd = 0.0;   // No brakes.
    m_car->_clutchCmd = 0.0;  // Full clutch (gearbox connected with engine).
  }
  else {
    m_car->_steerCmd = getSteer(s);
    m_car->_gearCmd = getGear();
    calcSpeed();
    m_car->_brakeCmd = 
      filterABS(filterBrakeSpeed(filterBColl(filterBPit(getBrake()))));
    if(m_car->_brakeCmd == 0.0)
      m_car->_accelCmd = filterTCL(filterTrk(filterOverlap(getAccel())));
    else
      m_car->_accelCmd = 0.0;
    m_car->_clutchCmd = getClutch();
  }

  m_laststeer = m_car->_steerCmd;
  m_lastmode = m_mode;
}//drive


// End of the current race.
void
Driver::endRace(tSituation * s)
{
  // Nothing for now.
}


/***************************************************************************
 *
 * utility functions
 *
***************************************************************************/


/** 
 * getDistToSegEnd
 * Computes the length to the end of the segment.
 * 
 * @return distance to segment end, in meter.
 */
double
Driver::getDistToSegEnd()
{
  const tTrackSeg *seg = m_car->_trkPos.seg;
  double ret = (seg->type == TR_STR)
    ? seg->length - m_car->_trkPos.toStart  //simple when straight
    : (seg->arc - m_car->_trkPos.toStart) * seg->radius;  //uhm, arc
  return ret;
}//getDistToSegEnd


/** 
 * getAccel
 * Computes fitting acceleration.
 * 
 * @return Acceleration scales 0-1.
 */
double
Driver::getAccel()
{
  double ret = 1.0;
  
  if(m_car->_gear > 0)
    {
      m_accelCmd = MIN(1.0, m_accelCmd);
      if(fabs(m_angle) > 0.8 && getSpeed() > 10.0)
        m_accelCmd =
          MAX(0.0, MIN(m_accelCmd, 1.0 - getSpeed() / 100.0 * fabs(m_angle)));
      ret = m_accelCmd;
    }//if m_car->_gear
    
  return ret;
}//getAccel


/**
 * getBrake
 * Computes initial brake value.
 * 
 * @return brake scaled 0-1
 */
double
Driver::getBrake()
{
  double ret = (m_car->_speed_x < -MAX_UNSTUCK_SPEED)
    ? 1.0        //Car drives backward, brake
    : m_brakeCmd;   //Car drives forward, normal braking.
    
  return ret;
}//getBrake


// Compute gear.
int
Driver::getGear()
{
  if(m_car->_gear <= 0)
    {
      return 1;
    }
  double gr_up = m_car->_gearRatio[m_car->_gear + m_car->_gearOffset];
  double omega = m_car->_enginerpmRedLine / gr_up;
  double wr = m_car->_wheelRadius(2);

  if(omega * wr * SHIFT < m_car->_speed_x)
    {
      return m_car->_gear + 1;
    }
  else
    {
      double gr_down = m_car->_gearRatio[m_car->_gear + m_car->_gearOffset - 1];
      omega = m_car->_enginerpmRedLine / gr_down;
      if(m_car->_gear > 1 && omega * wr * SHIFT > m_car->_speed_x + SHIFT_MARGIN)
        {
          return m_car->_gear - 1;
        }
    }
  return m_car->_gear;
}


void
Driver::setMode(int newmode)
{
  if(m_mode != newmode) {
    if(m_mode == NORMAL || m_mode == PITTING) {
      m_correctTimer = m_simTime + 7.0;
      m_correctLimit = 1000.0;
    }
    m_mode = newmode;
  }//m_mode != newmode
}//setMode



/**
 * getSteer
 * Computes steer value.
 * @param s global situation
 * @return steer value range -1...1
 */
double
Driver::getSteer(tSituation *s)
{
  double steer = 0.0;

  m_raceline->GetRaceLineData(s, &m_raceTarget, &m_raceSpeed, &m_avoidSpeed,
               &m_raceOffset, &m_rLookahead, &m_raceSteer);
  vec2f target = getTargetPoint();

  double targetAngle = atan2(target.y - m_car->_pos_Y, target.x - m_car->_pos_X);
  double avoidsteer = calcSteer(targetAngle, 0);

  if(m_mode == PITTING)
    return avoidsteer;

  targetAngle = atan2(m_raceTarget.y - m_car->_pos_Y, m_raceTarget.x - m_car->_pos_X);
  // uncomment the following if we want to use BT steering rather than K1999
  // m_raceSteer = calcSteer( targetAngle, 1 );

  if(m_mode == AVOIDING &&
     (!m_avoidmode
      || (m_avoidmode == AVOIDRIGHT && m_raceOffset >= m_myoffset && m_raceOffset < m_avoidLftOffset)
      || (m_avoidmode == AVOIDLEFT  && m_raceOffset <= m_myoffset && m_raceOffset > m_avoidRgtOffset)))
    {
      // we're avoiding, but trying to steer somewhere the raceline takes us.
      // hence we'll just correct towards the raceline instead.
      setMode(CORRECTING);
    }

  if(m_mode == CORRECTING &&
      (m_lastmode == NORMAL ||
      (fabs(m_angle) < 0.2f && fabs(m_raceSteer) < 0.4f
      && fabs(m_laststeer - m_raceSteer) < 0.05
      && ((fabs(m_car->_trkPos.toMiddle) < m_car->_trkPos.seg->width / 2 - 1.0)
      || m_car->_speed_x < 10.0) && m_raceline->isOnLine())))
    {
      // we're CORRECTING & are now close enough to the raceline to
      // switch back to 'normal' mode...
      setMode(NORMAL);
    }

  if(m_mode == NORMAL)
    {
      steer = m_raceSteer;
      m_lastNSasteer = m_raceSteer * 0.8;
    }
  else
    {
      if(m_mode != CORRECTING)
        {
          m_correctLimit = 1000.0;
          m_correctTimer = m_simTime + 7.0;
          steer = smoothSteering(avoidsteer);
        }
      else
        steer = smoothSteering(correctSteering(avoidsteer, m_raceSteer));

      if(fabs(m_angle) >= 1.6)
        {
          if(steer > 0.0)
            steer = 1.0;
          else
            steer = -1.0;
        }
    }

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
}


double
Driver::calcSteer(double targetAngle, int rl)
{
  double rearskid = MAX(0.0, MAX(m_car->_skid[2], m_car->_skid[3])
                    - MAX(m_car->_skid[0], m_car->_skid[1]))
                    + MAX(m_car->_skid[2], m_car->_skid[3]) * fabs(m_angle) * 0.9;

  double angle_correction = 0.0;
  double factor = (rl ? 1.4f : (m_mode == CORRECTING ? 1.1f : 1.2f));
  if(m_angle < 0.0)
    angle_correction = MAX(m_angle, MIN(0.0, m_angle / 2.0) / MAX(15.0, 70.0 - m_car->_speed_x) * factor);
  else
    angle_correction = MIN(m_angle, MAX(0.0, m_angle / 2.0) / MAX(15.0, 70.0 - m_car->_speed_x) * factor);
  
  double steer_direction = targetAngle - m_car->_yaw + angle_correction;
  NORM_PI_PI(steer_direction);
  
  if(m_car->_speed_x > 10.0)
    {
      double speedsteer = (80.0 - MIN(70.0, MAX(40.0, getSpeed()))) /
            ((185.0 * MIN(1.0, m_car->_steerLock / 0.785)) +
            (185.0 * (MIN(1.3, MAX(1.0, 1.0 + rearskid))) - 185.0));
      if(fabs(steer_direction) > speedsteer)
        {
        steer_direction = MAX(-speedsteer, MIN(speedsteer, steer_direction));
        }       
    }
    
  double steer = (double)(steer_direction / m_car->_steerLock);
    
  //smooth steering. check for separate function for this!
  if(m_mode != PITTING)
    {
      double minspeedfactor = (((105.0 - 
          MAX(40.0, MIN(70.0, getSpeed() + MAX(0.0, m_car->_accel_x * 5.0))))
           / 300.0) * (5.0 + MAX(0.0, (CA-1.9) * 20.0)));
      double maxspeedfactor = minspeedfactor;
      double rInverse = m_raceline->getRInverse();
        
      if(rInverse > 0.0)
        {
          minspeedfactor = MAX(minspeedfactor / 3, minspeedfactor - rInverse * 80.0);
          maxspeedfactor = MAX(maxspeedfactor / 3, maxspeedfactor + rInverse * 20.0);
        }
      else
        {
          maxspeedfactor = MAX(maxspeedfactor / 3, maxspeedfactor + rInverse * 80.0);
          minspeedfactor = MAX(minspeedfactor / 3, minspeedfactor + rInverse * 20.0);
        }
       
      steer = MAX(m_lastNSasteer - minspeedfactor, MIN(m_lastNSasteer + maxspeedfactor, steer));
    }
    
  m_lastNSasteer = steer;
  
  if(fabs(m_angle) > fabs(m_speedangle))
    {           
      //steer into the skid
      double sa = MAX(-0.3, MIN(0.3, m_speedangle / 3));
      double anglediff = (sa - m_angle) * (0.7 - MAX(0.0, MIN(0.3, m_car->_accel_x/100)));
      //anglediff += m_raceline->getRInverse() * 10;
      steer += anglediff * 0.7;
    }            
    
  if(fabs(m_angle) > 1.2)
    {
      steer = sign(steer);
    }
  else if(fabs(m_car->_trkPos.toMiddle) - m_car->_trkPos.seg->width / 2 > 2.0)
    steer = MIN(1.0,
          MAX(-1.0, steer * (1.0 +
                   (fabs(m_car->_trkPos.toMiddle) -
                    m_car->_trkPos.seg->width / 2) / 14 + fabs(m_angle) / 2)));

  if (m_mode != PITTING)
    {
      // limit how far we can steer against raceline 
      double limit = (90.0 - MAX(40.0, MIN(60.0, m_car->_speed_x))) / (50 + fabs(m_angle) * fabs(m_angle) * 3);
      steer = MAX(m_raceSteer - limit, MIN(m_raceSteer + limit, steer));
    }

  return steer;
}


double
Driver::correctSteering(double avoidsteer, double racesteer)
{
  double steer = avoidsteer;
  //double accel = MIN(0.0, m_car->_accel_x);
  double speed = MAX(50.0, getSpeed());
  //double changelimit = MIN(1.0, m_raceline->correctLimit());
  double changelimit = MIN(m_raceline->correctLimit(),
        (((120.0 - getSpeed()) / 6000)
        * (0.5 + MIN(fabs(avoidsteer), fabs(racesteer)) / 10)));

  if(m_mode == CORRECTING && m_simTime > 2.0)
    {
      // move steering towards racesteer...
      if(m_correctLimit < 900.0)
        {
          if(steer < racesteer)
            {
              if(m_correctLimit >= 0.0)
                steer = racesteer;
              else
                steer = MIN(racesteer,
                    MAX(steer, racesteer + m_correctLimit));
            }
          else
            {
              if(m_correctLimit <= 0.0)
                steer = racesteer;
              else
                steer = MAX(racesteer,
                    MIN(steer, racesteer + m_correctLimit));
            }
        }

      speed -= m_car->_accel_x / 10;
      speed = MAX(55.0, MIN(150.0, speed + (speed * speed / 55.0)));
      double rInverse = m_raceline->getRInverse() *
            (m_car->_accel_x < 0.0 ? 1.0 + fabs(m_car->_accel_x) / 10.0 : 1.0);
      double correctspeed = 0.5;
      if((rInverse > 0.0 && racesteer > steer) || (rInverse < 0.0 && racesteer < steer))
        correctspeed += rInverse * 110.0;
        
      if(racesteer > steer)
        steer = MIN(racesteer, steer + changelimit);
        //steer = MIN(racesteer, steer + (((155.0 - speed) / 10000.0) * correctspeed));
      else
        steer = MAX(racesteer, steer - changelimit);
        //steer = MAX(racesteer, steer - (((155.0-speed)/10000) * correctspeed));
      
#if 0
      if(racesteer > avoidsteer)
        steer = MIN(racesteer, avoidsteer + changelimit);
      else
        steer = MAX(racesteer, avoidsteer - changelimit);
#endif

      m_correctLimit = (steer - racesteer);// * 1.08;
    }

  return steer;
}


double
Driver::smoothSteering(double steercmd)
{
  // try to limit sudden changes in steering
  // to avoid loss of control through oversteer. 
  double speedfactor = (((60.0 -
       (MAX(40.0, MIN(70.0, getSpeed() + MAX(0.0, m_car->_accel_x * 5)))
        - 25)) / 300) * 1.2) / 0.785;
  //double rearskid = MAX(0.0, MAX(m_car->_skid[2], m_car->_skid[3]) - MAX(m_car->_skid[0], m_car->_skid[1]));

  if(fabs(steercmd) < fabs(m_laststeer)
     && fabs(steercmd) <= fabs(m_laststeer - steercmd))
    speedfactor *= 2;

  steercmd = MAX(m_laststeer - speedfactor,
                        MIN(m_laststeer + speedfactor, steercmd));
  return steercmd;
}

// Compute the clutch value.
double
Driver::getClutch()
{
  if(1 || m_car->_gearCmd > 1)
    {
      double maxtime = MAX(0.06, 0.32 - (m_car->_gearCmd / 65.0));
      if(m_car->_gear != m_car->_gearCmd)
        m_clutchTime = maxtime;
      if(m_clutchTime > 0.0)
        m_clutchTime -= (RCM_MAX_DT_ROBOTS *
           (0.02 + (m_car->_gearCmd / 8.0)));
      return 2.0 * m_clutchTime;
    }
  else
    {
      double drpm = m_car->_enginerpm - m_car->_enginerpmRedLine / 2.0;
      double ctlimit = 0.9;
      if(m_car->_gearCmd > 1)
        ctlimit -= 0.15 + m_car->_gearCmd / 13.0;
      m_clutchTime = MIN(ctlimit, m_clutchTime);
      if(m_car->_gear != m_car->_gearCmd)
        m_clutchTime = 0.0;
      double clutcht = (ctlimit - m_clutchTime) / ctlimit;
      if(m_car->_gear == 1 && m_car->_accelCmd > 0.0)
        {
          m_clutchTime += RCM_MAX_DT_ROBOTS;
        }

      if(m_car->_gearCmd == 1 || drpm > 0)
        {
          double speedr;
          if(m_car->_gearCmd == 1)
            {
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
            }
          else
            {
              // For the reverse gear.
              m_clutchTime = 0.0;
              return 0.0;
            }
        }
      else
        {
          return clutcht;
        }
    }
}

/**
 * getTargetPoint
 * Computes target point for steering.
 * @return 2D coords of the target
 */
vec2f
Driver::getTargetPoint()
{
  double lookahead;

  if(m_pit->getInPit())
    {
      // To stop in the pit we need special lookahead values.
      lookahead = PIT_LOOKAHEAD;
      if(m_currentSpeedSqr > m_pit->getSpeedlimitSqr())
        {
          lookahead += m_car->_speed_x * LOOKAHEAD_FACTOR;
        }
    }
  else
    {
      // Usual lookahead.
      lookahead = m_rLookahead;
#if 1
      double speed = MAX(20.0, getSpeed()); //+ MAX(0.0, m_car->_accel_x));
      lookahead = LOOKAHEAD_CONST * 1.2 + speed * 0.60;
      lookahead = MIN(lookahead,
        LOOKAHEAD_CONST + ((speed * (speed / 7)) * 0.15));
#endif
      // Prevent "snap back" of lookahead on harsh braking.
      double cmplookahead = m_oldLookahead - m_car->_speed_x * RCM_MAX_DT_ROBOTS;
      lookahead = MAX(cmplookahead, lookahead);
    }

  m_oldLookahead = lookahead;

  // Search for the segment containing the target point.
  tTrackSeg *seg = m_car->_trkPos.seg;
  double length = getDistToSegEnd();
  while(length < lookahead)
    {
      seg = seg->next;
      length += seg->length;
    }

  length = lookahead - length + seg->length;
  double fromstart = seg->lgfromstart;
  fromstart += length;

  // Compute the target point.
  double offset = getOffset();
  double pitoffset = m_pit->getPitOffset(-100.0, fromstart);
  if((m_pit->getPitstop() || m_pit->getInPit()) && pitoffset != -100.0)
    {
      setMode(PITTING);
      offset = m_myoffset = pitoffset;
    }
  else if(m_mode == PITTING)
    setMode(CORRECTING);

  vec2f s;
  if(m_mode != PITTING)
    {
      m_raceline->GetPoint(offset, lookahead, &s);
      return s;
    }

  s.x = (seg->vertex[TR_SL].x + seg->vertex[TR_SR].x) / 2.0;
  s.y = (seg->vertex[TR_SL].y + seg->vertex[TR_SR].y) / 2.0;

  if(seg->type == TR_STR)
    {
      vec2f n((seg->vertex[TR_EL].x - seg->vertex[TR_ER].x) / seg->length,
        (seg->vertex[TR_EL].y - seg->vertex[TR_ER].y) / seg->length);
      n.normalize();
      vec2f d((seg->vertex[TR_EL].x - seg->vertex[TR_SL].x) / seg->length,
        (seg->vertex[TR_EL].y - seg->vertex[TR_SL].y) / seg->length);
      return s + d * length + (float)offset * n;
    }
  else
    {
      vec2f c(seg->center.x, seg->center.y);
      double arc = length / seg->radius;
      double arcsign = (seg->type == TR_RGT) ? -1.0 : 1.0;
      arc = arc * arcsign;
      s = s.rotate(c, arc);

      vec2f n, t, rt;
      n = c - s;
      n.normalize();
      t = s + (float)arcsign * (float)offset * n;

      if(m_mode != PITTING)
        {
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
          if(dist2 > dist1)
            t = rt;
        }

      return t;
    }
}//getTargetPoint


// Check if I'm stuck.
bool
Driver::isStuck()
{
  bool ret = false;
  
  if(fabs(m_mycardata->getCarAngle()) > MAX_UNSTUCK_ANGLE &&
        m_car->_speed_x < MAX_UNSTUCK_SPEED &&
        fabs(m_car->_trkPos.toMiddle) > MIN_UNSTUCK_DIST)
    {
      if(m_stuckCounter > MAX_UNSTUCK_COUNT
        && m_car->_trkPos.toMiddle * m_mycardata->getCarAngle() < 0.0)
          ret = true;
      else
          m_stuckCounter++;
    }
  else
      m_stuckCounter = 0;
  
  return ret;
}


// Compute aerodynamic downforce coefficient CA.
void
Driver::initCa()
{
  double rearwingarea =
    GfParmGetNum(m_car->_carHandle, SECT_REARWING, PRM_WINGAREA,
         (char *) NULL, 0.0);
  double rearwingangle =
    GfParmGetNum(m_car->_carHandle, SECT_REARWING, PRM_WINGANGLE,
         (char *) NULL, 0.0);
  double wingca = 1.23 * rearwingarea * sin(rearwingangle);

  double cl = GfParmGetNum(m_car->_carHandle, SECT_AERODYNAMICS, PRM_FCL,
              (char *) NULL, 0.0) + 
              GfParmGetNum(m_car->_carHandle, SECT_AERODYNAMICS, PRM_RCL,
              (char *) NULL, 0.0);

  double h = 0.0;
  for(int i = 0; i < 4; i++)
    h += GfParmGetNum(m_car->_carHandle, (char*)WheelSect[i], PRM_RIDEHEIGHT,
              (char *) NULL, 0.2);
  h *= 1.5;
  h = h * h;
  h = h * h;
  h = 2.0 * exp(-3.0 * h);
  CA = h * cl + 4.0 * wingca;
}


// Compute aerodynamic drag coefficient CW.
void
Driver::initCw()
{
  double cx = GfParmGetNum(m_car->_carHandle, SECT_AERODYNAMICS, PRM_CX,
            (char *) NULL, 0.0);
  double frontarea = GfParmGetNum(m_car->_carHandle, SECT_AERODYNAMICS,
            PRM_FRNTAREA, (char *) NULL, 0.0);
  CW = 0.645 * cx * frontarea;
}


// Init the friction coefficient of the the tires.
void
Driver::initTireMu()
{
  double tm = FLT_MAX;
  for(int i = 0; i < 4; i++)
    {
      tm = MIN(tm, GfParmGetNum(m_car->_carHandle, WheelSect[i], PRM_MU,
                (char *) NULL, 1.0));
    }
  TIREMU = tm;
}


// Reduces the brake value such that it fits the speed (more downforce -> more braking).
double
Driver::filterBrakeSpeed(double brake)
{
  double weight = (CARMASS + m_car->_fuel) * G;
  double maxForce = weight + CA * MAX_SPEED * MAX_SPEED;
  double force = weight + CA * m_currentSpeedSqr;
  return brake * force / maxForce;
}


// Brake filter for pit stop.
double
Driver::filterBPit(double brake)
{
  double mu = m_car->_trkPos.seg->surface->kFriction * TIREMU * PIT_MU;
  
  if(m_pit->getPitstop() && !m_pit->getInPit())
    {
      tdble dl, dw;
      RtDistToPit(m_car, m_track, &dl, &dw);
      if(dl < PIT_BRAKE_AHEAD)
        {
          if(brakedist(0.0, mu) > dl)
            return 1.0;
        }
    }

  if(m_pit->getInPit())
    {
      double s = m_pit->toSplineCoord(m_car->_distFromStartLine);
      // Pit entry.
      if(m_pit->getPitstop())
        {
          if(s < m_pit->getNPitStart())
            {
              // Brake to pit speed limit.
              double dist = m_pit->getNPitStart() - s;
              if(brakedist(m_pit->getSpeedlimit(), mu) > dist)
                return 1.0;
            }
          else
            {
              // Hold speed limit.
              if(m_currentSpeedSqr > m_pit->getSpeedlimitSqr())
                return m_pit->getSpeedLimitBrake(m_currentSpeedSqr);
            }
      
          // Brake into pit (speed limit 0.0 to stop)
          double dist = m_pit->getNPitLoc() - s;
          if(m_pit->isTimeout(dist))
            {
              m_pit->setPitstop(false);
              return 0.0;
            }
          else
            {
              if(brakedist(0.0, mu) > dist)
                return 1.0;
              else if(s > m_pit->getNPitLoc())
                return 1.0; // Stop in the pit.
            }
        }
      else
        {
          // Pit exit.
          if(s < m_pit->getNPitEnd())
            {
              // Pit speed limit.
              if(m_currentSpeedSqr > m_pit->getSpeedlimitSqr())
                return m_pit->getSpeedLimitBrake(m_currentSpeedSqr);
            }
        }
    }

  return brake;
}


// Antilocking filter for brakes.
double
Driver::filterABS(double brake)
{
  if(m_car->_speed_x < ABS_MINSPEED)
    return brake;
  double origbrake = brake;
  double rearskid = MAX(0.0, MAX(m_car->_skid[2], m_car->_skid[3]) -
                    MAX(m_car->_skid[0], m_car->_skid[1]));

  double slip = 0.0;
  for(int i = 0; i < 4; i++)
    {
      slip += m_car->_wheelSpinVel(i) * m_car->_wheelRadius(i);
    }
  slip *=
    1.0 + MAX(rearskid, MAX(fabs(m_car->_yaw_rate) / 5, fabs(m_angle) / 6));
  slip = m_car->_speed_x - slip / 4.0;
  
  if(slip > ABS_SLIP)
    brake = brake - MIN(brake, (slip - ABS_SLIP) / ABS_RANGE);
  
  brake = MAX(brake, MIN(origbrake, 0.1f));
  return brake;
}


// TCL filter for accelerator pedal.
double
Driver::filterTCL(const double accel)
{
  double ret = accel;
  
  if(m_simTime >= 3.0) {
    ret = MIN(1.0, accel);
    double accel1 = ret, accel2 = ret, accel3 = ret;

    if(m_car->_speed_x > 10.0) {
      tTrackSeg *seg = m_car->_trkPos.seg;
      tTrackSeg *wseg0 = m_car->_wheelSeg(0);
      tTrackSeg *wseg1 = m_car->_wheelSeg(1);
      int count = 0;

      if(
        wseg0->surface->kRoughness > MAX(0.02, seg->surface->kRoughness * 1.2)
        || wseg0->surface->kFriction < seg->surface->kFriction * 0.8
        || wseg0->surface->kRollRes > MAX(0.005, seg->surface->kRollRes * 1.2))
        count++;
          
      if(
        wseg1->surface->kRoughness > MAX(0.02, seg->surface->kRoughness * 1.2)
        || wseg1->surface->kFriction < seg->surface->kFriction * 0.8
        || wseg1->surface->kRollRes > MAX(0.005, seg->surface->kRollRes * 1.2))
        count++;

      if(count) {
        if(m_mode != NORMAL
          && ((seg->type == TR_RGT && seg->radius <= 200.0
            && m_car->_trkPos.toLeft < 3.0)
            || (seg->type == TR_LFT && seg->radius <= 200.0
            && m_car->_trkPos.toRight < 3.0)))
          count++;
        
          accel1 = MAX(0.0, MIN(accel1, (1.0 - (0.25 * count)) -
                            MAX(0.0, (getSpeed() - m_car->_speed_x) / 10.0)));
          }//if count

      if(fabs(m_angle) > 1.0)
        accel1 = MIN(accel1, 1.0 - (fabs(m_angle) - 1.0) * 1.3);
      }//if m_car->_speed_x

    if(fabs(m_car->_steerCmd) > 0.02) {
      double decel = ((fabs(m_car->_steerCmd) - 0.02) *
          (1.0 + fabs(m_car->_steerCmd)) * 0.7);
      accel2 = MIN(accel2, MAX(0.45, 1.0 - decel));
    }//if m_car->_steerCmd

    double slip = (this->*GET_DRIVEN_WHEEL_SPEED) () - m_car->_speed_x;
    if(slip > TCL_SLIP)
      accel3 = accel3 - MIN(accel3, (slip - TCL_SLIP) / TCL_RANGE);

    ret = MIN(accel1, MIN(accel2, accel3));
  }//if m_simTime
  
  return ret;
}//filterTCL


// Traction Control (TCL) setup.
void
Driver::initTCLfilter()
{
  const string traintype = GfParmGetStr(m_car->_carHandle, SECT_DRIVETRAIN, PRM_TYPE,
                 VAL_TRANS_RWD);

  if(traintype == VAL_TRANS_RWD)
    {
      GET_DRIVEN_WHEEL_SPEED = &Driver::filterTCL_RWD;
    }
  else if(traintype == VAL_TRANS_FWD)
    {
      GET_DRIVEN_WHEEL_SPEED = &Driver::filterTCL_FWD;
    }
  else if(traintype == VAL_TRANS_4WD)
    {
      GET_DRIVEN_WHEEL_SPEED = &Driver::filterTCL_4WD;
    }
}


// TCL filter plugin for rear wheel driven cars.
double
Driver::filterTCL_RWD()
{
  return (m_car->_wheelSpinVel(REAR_RGT) + m_car->_wheelSpinVel(REAR_LFT)) *
    m_car->_wheelRadius(REAR_LFT) / 2.0;
}


// TCL filter plugin for front wheel driven cars.
double
Driver::filterTCL_FWD()
{
  return (m_car->_wheelSpinVel(FRNT_RGT) + m_car->_wheelSpinVel(FRNT_LFT)) *
    m_car->_wheelRadius(FRNT_LFT) / 2.0;
}


// TCL filter plugin for all wheel driven cars.
double
Driver::filterTCL_4WD()
{
  return ((m_car->_wheelSpinVel(FRNT_RGT) + m_car->_wheelSpinVel(FRNT_LFT)) *
      m_car->_wheelRadius(FRNT_LFT) +
      (m_car->_wheelSpinVel(REAR_RGT) + m_car->_wheelSpinVel(REAR_LFT)) *
      m_car->_wheelRadius(REAR_LFT)) / 4.0;
}


// Hold car on the track.
double
Driver::filterTrk(double accel)
{
  tTrackSeg *seg = m_car->_trkPos.seg;

  if(m_car->_speed_x < MAX_UNSTUCK_SPEED ||   // Too slow.
        m_pit->getInPit() ||     // Pit stop.
        m_car->_trkPos.toMiddle * -m_speedangle > 0.0) // Speedvector points to the inside of the turn.
    return accel;

  if(seg->type == TR_STR)
    {
      double tm = fabs(m_car->_trkPos.toMiddle);
      double w = (seg->width - m_car->_dimension_y) / 2.0;
      if(tm > w)
        return 0.0;
      else
        return accel;
    }
  else
    {
      double sign = (seg->type == TR_RGT) ? -1.0 : 1.0;
      if(m_car->_trkPos.toMiddle * sign > 0.0)
        return accel;
      else
        {
          double tm = fabs(m_car->_trkPos.toMiddle);
          double w = seg->width / WIDTHDIV;
          if(tm > w)
            return 0.0;
          else
            return accel;
        }
    }
}//filterTrk


// Compute the needed distance to brake.
double
Driver::brakedist(double allowedspeed, double mu)
{
  double c = mu * G;
  double d = (CA * mu + CW) / m_mass;
  double v1sqr = m_currentSpeedSqr;
  double v2sqr = pow(allowedspeed, 2);
  return -log((c + v2sqr * d) / (c + v1sqr * d)) / (2.0 * d);
}
