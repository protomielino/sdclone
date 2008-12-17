/***************************************************************************

    file                 : driver.cpp
    created              : Thu Dec 20 01:21:49 CET 2002
    copyright            : (C) 2002-2004 Bernhard Wymann
    email                : berniw@bluewin.ch
    version              : $Id: driver.cpp,v 1.16 2006/04/27 22:32:29 berniw Exp $

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "driver.h"

const float Driver::MAX_UNSTUCK_ANGLE = (float)(15.0f/180.0f*PI);	// [radians] If the angle of the car on the track is smaller, we assume we are not stuck.
const float Driver::UNSTUCK_TIME_LIMIT = 2.0f;				// [s] We try to get unstuck after this time.
const float Driver::MAX_UNSTUCK_SPEED = 5.0f;				// [m/s] Below this speed we consider being stuck.
const float Driver::MIN_UNSTUCK_DIST = 3.0f;				// [m] If we are closer to the middle we assume to be not stuck.
const float Driver::G = 9.81f;								// [m/(s*s)] Welcome on Earth.
const float Driver::FULL_ACCEL_MARGIN = 1.0f;				// [m/s] Margin reduce oscillation of brake/acceleration.
const float Driver::SHIFT = 0.96f;							// [-] (% of rpmredline) When do we like to shift gears.
const float Driver::SHIFT_MARGIN = 4.0f;					// [m/s] Avoid oscillating gear changes.
const float Driver::ABS_SLIP = 2.5f;						// [m/s] range [0..10]
const float Driver::ABS_RANGE = 5.0f;						// [m/s] range [0..10]
const float Driver::ABS_MINSPEED = 3.0f;					// [m/s] Below this speed the ABS is disabled (numeric, division by small numbers).
const float Driver::TCL_SLIP = 2.0f;						// [m/s] range [0..10]
const float Driver::TCL_RANGE = 10.0f;						// [m/s] range [0..10]
const float Driver::LOOKAHEAD_CONST = 18.0f;				// [m]
const float Driver::LOOKAHEAD_FACTOR = 0.33f;				// [-]
const float Driver::WIDTHDIV = 3.0f;						// [-] Defines the percentage of the track to use (2/WIDTHDIV).
const float Driver::SIDECOLL_MARGIN = 3.0f;					// [m] Distance between car centers to avoid side collisions.
const float Driver::BORDER_OVERTAKE_MARGIN = 0.5f;			// [m]
const float Driver::OVERTAKE_OFFSET_SPEED = 5.0f;			// [m/s] Offset change speed.
const float Driver::PIT_LOOKAHEAD = 6.0f;					// [m] Lookahead to stop in the pit.
const float Driver::PIT_BRAKE_AHEAD = 200.0f;				// [m] Workaround for "broken" pitentries.
const float Driver::PIT_MU = 0.4f;							// [-] Friction of pit concrete.
const float Driver::MAX_SPEED = 84.0f;						// [m/s] Speed to compute the percentage of brake to apply.
const float Driver::MAX_FUEL_PER_METER = 0.0008f;			// [liter/m] fuel consumtion.
const float Driver::CLUTCH_SPEED = 5.0f;					// [m/s]
const float Driver::CENTERDIV = 0.1f;						// [-] (factor) [0.01..0.6].
const float Driver::DISTCUTOFF = 200.0f;					// [m] How far to look, terminate while loops.
const float Driver::MAX_INC_FACTOR = 6.0f;					// [m] Increment faster if speed is slow [1.0..10.0].
const float Driver::CATCH_FACTOR = 10.0f;					// [-] select MIN(catchdist, dist*CATCH_FACTOR) to overtake.
const float Driver::CLUTCH_FULL_MAX_TIME = 2.0f;			// [s] Time to apply full clutch.
const float Driver::USE_LEARNED_OFFSET_RANGE = 0.2f;		// [m] if offset < this use the learned stuff

const float Driver::TEAM_REAR_DIST = 50.0f;					//
const int Driver::TEAM_DAMAGE_CHANGE_LEAD = 700;			// When to change position in the team?

// Static variables.
static int pitstatus[128] = {0};
Cardata *Driver::cardata = NULL;
double Driver::currentsimtime;


Driver::Driver(int index)
{
	INDEX = index;
}


Driver::~Driver()
{
	delete opponents;
	delete pit;
	delete [] radius;
	delete strategy;
	if (cardata != NULL) {
		delete cardata;
		cardata = NULL;
	}
}


// Called for every track change or new race.
void Driver::initTrack(tTrack* t, void *carHandle, void **carParmHandle, tSituation *s)
{
	track = t;

	const int BUFSIZE = 256;
	char buffer[BUFSIZE];
	// Load a custom setup if one is available.
	// Get a pointer to the first char of the track filename.
	char* trackname = strrchr(track->filename, '/') + 1;

	snprintf(buffer, BUFSIZE, "drivers/locus/%d/default.xml", INDEX);
	*carParmHandle = GfParmReadFile(buffer, GFPARM_RMODE_STD);
	void *newhandle;

	switch (s->_raceType) {
		case RM_TYPE_PRACTICE:
			snprintf(buffer, BUFSIZE, "drivers/locus/%d/practice/%s", INDEX, trackname);
			break;
		case RM_TYPE_QUALIF:
			snprintf(buffer, BUFSIZE, "drivers/locus/%d/qualifying/%s", INDEX, trackname);
			break;
		case RM_TYPE_RACE:
			snprintf(buffer, BUFSIZE, "drivers/locus/%d/race/%s", INDEX, trackname);
			break;
		default:
			break;
	}

	newhandle = GfParmReadFile(buffer, GFPARM_RMODE_STD);
	if (newhandle)
	{
		if (*carParmHandle)
			*carParmHandle = GfParmMergeHandles(*carParmHandle, newhandle, (GFPARM_MMODE_SRC|GFPARM_MMODE_DST|GFPARM_MMODE_RELSRC|GFPARM_MMODE_RELDST));
		else
			*carParmHandle = newhandle;
  }
	else
	{
		snprintf(buffer, BUFSIZE, "drivers/locus/%d/%s", INDEX, trackname);
	  newhandle = GfParmReadFile(buffer, GFPARM_RMODE_STD);
		if (newhandle)
		{
			if (*carParmHandle)
				*carParmHandle = GfParmMergeHandles(*carParmHandle, newhandle, (GFPARM_MMODE_SRC|GFPARM_MMODE_DST|GFPARM_MMODE_RELSRC|GFPARM_MMODE_RELDST));
			else
				*carParmHandle = newhandle;
	  }
	}

	// Create a pit stop strategy object.
	strategy = new SimpleStrategy2();

	// Init fuel.
	strategy->setFuelAtRaceStart(t, carParmHandle, s, INDEX);

	// Load and set parameters.
	MU_FACTOR = GfParmGetNum(*carParmHandle, BT_SECT_PRIV, BT_ATT_MUFACTOR, (char*)NULL, 0.69f);

	double MinCornerInverse = GfParmGetNum( *carParmHandle, BT_SECT_PRIV, "MinCornerInverse", (char *)NULL, 0.002f );
	double CornerSpeed = GfParmGetNum( *carParmHandle, BT_SECT_PRIV, "CornerSpeed", (char *)NULL, 15.0 );
	double AvoidSpeed = GfParmGetNum( *carParmHandle, BT_SECT_PRIV, "AvoidSpeedAdjust", (char *)NULL, 2.0 );
	double CornerAccel = GfParmGetNum( *carParmHandle, BT_SECT_PRIV, "CornerAccel", (char *)NULL, 1.0 );
	double IntMargin = GfParmGetNum( *carParmHandle, BT_SECT_PRIV, "IntMargin", (char *)NULL, 1.0 );
	double ExtMargin = GfParmGetNum( *carParmHandle, BT_SECT_PRIV, "ExtMargin", (char *)NULL, 2.0 );
	brakedelay = GfParmGetNum( *carParmHandle, BT_SECT_PRIV, "BrakeDelay", (char *)NULL, 10.0 );
	PitOffset = GfParmGetNum( *carParmHandle, BT_SECT_PRIV, "PitOffset", (char *)NULL, 10.0 );

	raceline.setMinCornerInverse( MinCornerInverse );
	raceline.setAvoidSpeedAdjust( AvoidSpeed );
	raceline.setCornerSpeed( CornerSpeed );
	raceline.setCornerAccel( CornerAccel );
	raceline.setBrakeDelay( brakedelay );
	raceline.setIntMargin( IntMargin );
	raceline.setExtMargin( ExtMargin );

	raceline.InitTrack(track, s);
}


// Start a new race.
void Driver::newRace(tCarElt* car, tSituation *s)
{
	float deltaTime = (float) RCM_MAX_DT_ROBOTS;
	MAX_UNSTUCK_COUNT = int(UNSTUCK_TIME_LIMIT/deltaTime);
	OVERTAKE_OFFSET_INC = OVERTAKE_OFFSET_SPEED*deltaTime;
	stuck = 0;
	alone = 1;
	clutchtime = 0.0f;
	oldlookahead = laststeer = lastNSasteer = 0.0f;
	this->car = car;
	CARMASS = GfParmGetNum(car->_carHandle, SECT_CAR, PRM_MASS, NULL, 1000.0f);
	myoffset = 0.0f;
	simtime = correcttimer = 0.0;
	correctlimit = 1000.0;
	initCa();
	initCw();
	initTireMu();
	initTCLfilter();

	// Create just one instance of cardata shared by all drivers.
	if (cardata == NULL) {
		cardata = new Cardata(s);
	}
	mycardata = cardata->findCar(car);
	currentsimtime = s->currentTime;

	// initialize the list of opponents.
	opponents = new Opponents(s, this, cardata);
	opponent = opponents->getOpponentPtr();

	// Set team mate.
	const char *teammate = GfParmGetStr(car->_carHandle, BT_SECT_PRIV, BT_ATT_TEAMMATE, NULL);
	if (teammate != NULL) {
		opponents->setTeamMate(teammate);
	}

	// Initialize radius of segments.
	radius = new float[track->nseg];
	computeRadius(radius);

	// create the pit object.
	pit = new Pit(s, this, PitOffset);
	setMode( correcting );
	lastmode = correcting;

	carindex = 0;

	for (int i = 0; i < s->_ncars; i++)
	{
		if (s->cars[i] == car)
		{
			carindex = i;
			break;
 		}
 	}

	raceline.NewRace( car, s );
}

void Driver::calcSpeed()
{
	accelcmd = brakecmd = 0.0f;
	double speed = racespeed;

	if (mode == avoiding)
		speed = avoidspeed;
	else if (mode == correcting)
		speed = racespeed - (racespeed-avoidspeed) * MAX(0.0, (correcttimer - simtime) / 7.0);

	double x = (10 + car->_speed_x) * (speed - car->_speed_x) / 200;

	if (x > 0)
		accelcmd = (float) x;
	else
		brakecmd = MIN(1.0f, (float) (-(MAX(10.0, brakedelay*0.7))*x));
}


// Drive during race.
void Driver::drive(tSituation *s)
{
	memset(&car->ctrl, 0, sizeof(tCarCtrl));

	update(s);

	//pit->setPitstop(true);

	if (isStuck()) {
		car->_steerCmd = -mycardata->getCarAngle() / car->_steerLock;
		car->_gearCmd = -1;		// Reverse gear.
		car->_accelCmd = 1.0f;	// 100% accelerator pedal.
		car->_brakeCmd = 0.0f;	// No brakes.
		car->_clutchCmd = 0.0f;	// Full clutch (gearbox connected with engine).
	} else {
		car->_steerCmd = getSteer(s);
		car->_gearCmd = getGear();
		calcSpeed();
		car->_brakeCmd = filterABS(filterBrakeSpeed(filterBColl(filterBPit(getBrake()))));
		if (car->_brakeCmd == 0.0f) {
			car->_accelCmd = filterTCL(filterTrk(filterOverlap(getAccel())));
		} else {
			car->_accelCmd = 0.0f;
  	}
		car->_clutchCmd = getClutch();

	}

	laststeer = car->_steerCmd;
	lastmode = mode;
}


// Set pitstop commands.
int Driver::pitCommand(tSituation *s)
{
	car->_pitRepair = strategy->pitRepair(car, s);
	car->_pitFuel = strategy->pitRefuel(car, s);
	// This should be the only place where the pit stop is set to false!
	pit->setPitstop(false);
	return ROB_PIT_IM; // return immediately.
}


// End of the current race.
void Driver::endRace(tSituation *s)
{
	// Nothing for now.
}


/***************************************************************************
 *
 * utility functions
 *
***************************************************************************/


void Driver::computeRadius(float *radius)
{
	float lastturnarc = 0.0f;
	int lastsegtype = TR_STR;

	tTrackSeg *currentseg, *startseg = track->seg;
	currentseg = startseg;

	do {
		if (currentseg->type == TR_STR) {
			lastsegtype = TR_STR;
			radius[currentseg->id] = FLT_MAX;
		} else {
			if (currentseg->type != lastsegtype) {
				float arc = 0.0f;
				tTrackSeg *s = currentseg;
				lastsegtype = currentseg->type;

				while (s->type == lastsegtype && arc < PI/2.0f) {
					arc += s->arc;
					s = s->next;
				}
				lastturnarc = (float) (arc/(PI/2.0f));
			}
			radius[currentseg->id] = (float) (currentseg->radius + currentseg->width/2.0)/lastturnarc;
		}
		currentseg = currentseg->next;
	} while (currentseg != startseg);

}


// Compute the length to the end of the segment.
float Driver::getDistToSegEnd()
{
	if (car->_trkPos.seg->type == TR_STR) {
		return car->_trkPos.seg->length - car->_trkPos.toStart;
	} else {
		return (car->_trkPos.seg->arc - car->_trkPos.toStart)*car->_trkPos.seg->radius;
	}
}


// Compute fitting acceleration.
float Driver::getAccel()
{
	if (car->_gear > 0) {
		accelcmd = MIN(1.0f, accelcmd);
		if (fabs(angle) > 0.8 && getSpeed() > 10.0f)
			accelcmd = MAX(0.0f, MIN(accelcmd, 1.0f - getSpeed()/100.0f * fabs(angle)));
		return accelcmd;
	} else {
		return 1.0;
	}
}


// If we get lapped reduce accelerator.
float Driver::filterOverlap(float accel)
{
	int i;
	for (i = 0; i < opponents->getNOpponents(); i++) {
		if (opponent[i].getState() & OPP_LETPASS) {
			return MIN(accel, 0.5f);
		}
	}
	return accel;
}


// Compute initial brake value.
float Driver::getBrake()
{
	// Car drives backward?
	if (car->_speed_x < -MAX_UNSTUCK_SPEED) {
		// Yes, brake.
		return 1.0;
	} else {
		// We drive forward, normal braking.
		return brakecmd;
	}
}


// Compute gear.
int Driver::getGear()
{
	if (car->_gear <= 0) {
		return 1;
	}
	float gr_up = car->_gearRatio[car->_gear + car->_gearOffset];
	float omega = car->_enginerpmRedLine/gr_up;
	float wr = car->_wheelRadius(2);

	if (omega*wr*SHIFT < car->_speed_x) {
		return car->_gear + 1;
	} else {
		float gr_down = car->_gearRatio[car->_gear + car->_gearOffset - 1];
		omega = car->_enginerpmRedLine/gr_down;
		if (car->_gear > 1 && omega*wr*SHIFT > car->_speed_x + SHIFT_MARGIN) {
			return car->_gear - 1;
		}
	}
	return car->_gear;
}


void Driver::setMode( int newmode )
{
	if (mode == newmode)
		return;

	if (mode == normal || mode == pitting)
	{
		correcttimer = simtime + 7.0;
		correctlimit = 1000.0;
	}

	mode = newmode;
}


// Compute steer value.
float Driver::getSteer(tSituation *s)
{
	float targetAngle, racesteer;
	raceline.GetRaceLineData( s, &racetarget, &racespeed, &avoidspeed, &raceoffset, &rlookahead, &racesteer );
	vec2f	target = getTargetPoint();
	float avoidsteer = 0.0f;
	float steer = 0.0f;

	{
		targetAngle = atan2(target.y - car->_pos_Y, target.x - car->_pos_X);
		avoidsteer = calcSteer( targetAngle, 0, racesteer );
	}

	if (mode == pitting)
	{
		return avoidsteer;
	}

	targetAngle = atan2(racetarget.y - car->_pos_Y, racetarget.x - car->_pos_X);
	// uncomment the following if we want to use BT steering rather than K1999
	// racesteer = calcSteer( targetAngle, 1 );

	if (mode == avoiding && 
			(!avoidmode ||
			 (avoidmode == avoidright && raceoffset >= myoffset && raceoffset < avoidlftoffset) ||
			 (avoidmode == avoidleft && raceoffset <= myoffset && raceoffset > avoidrgtoffset)))
	{
		// we're avoiding, but trying to steer somewhere the raceline takes us.
		// hence we'll just correct towards the raceline instead.
		setMode( correcting );
	}

	if (mode == correcting && 
			(lastmode == normal ||
			 (fabs(angle) < 0.2f &&
				fabs(racesteer) < 0.4f &&
				fabs(laststeer - racesteer) < 0.05 &&
				((fabs(car->_trkPos.toMiddle) < car->_trkPos.seg->width/2 - 1.0) || car->_speed_x < 10.0) &&
			  (raceline.isOnLine()))))
	{
		// we're correcting & are now close enough to the raceline to
		// switch back to 'normal' mode...
		setMode( normal );
	}

	if (mode == normal)
	{
		steer = racesteer;
		lastNSasteer = racesteer*0.8;
	}
	else
	{
		if (mode != correcting)
		{
			correctlimit = 1000.0;
			correcttimer = simtime + 7.0;
			steer = smoothSteering( avoidsteer );
		}
		else
			steer = smoothSteering(correctSteering( avoidsteer, racesteer ));
	
		if (fabs(angle) >= 1.6)
		{
			if (steer > 0.0)
				steer = 1.0;
			else
				steer = -1.0;
		}
	}

#if 0
fprintf(stderr,"%s %d: %c%c %.3f (a%.3f k%.3f) cl=%.3f ct=%.1f myof=%.3f->%.3f\n",car->_name,car->_dammage,(mode==normal?'n':(mode==avoiding?'a':(mode==correcting?'c':'p'))),(avoidmode==avoidleft?'L':(avoidmode==avoidright?'R':(avoidmode==(avoidleft+avoidright)?'B':' '))),steer,avoidsteer,racesteer,correctlimit,(correcttimer-simtime),car->_trkPos.toMiddle,myoffset);fflush(stderr);
#endif
	return steer;
}


float Driver::calcSteer( float targetAngle, int rl, float racesteer )
{
	double rearskid = MAX(0.0, MAX(car->_skid[2], car->_skid[3]) - MAX(car->_skid[0], car->_skid[1])) + MAX(car->_skid[2], car->_skid[3]) * fabs(angle)*0.9;

	double steer_direction = targetAngle - car->_yaw;

	NORM_PI_PI(steer_direction);

	if (car->_speed_x > 10.0)
	{
		double speedsteer = (80.0 - MIN(70.0, MAX(40.0, getSpeed()))) / 
		                     ((185.0 * MIN(1.0, car->_steerLock / 0.785)) +
		                     (185.0 * (MIN(1.3, MAX(1.0, 1.0 + rearskid))) - 185.0));
		if (fabs(steer_direction) > speedsteer)
		{
			steer_direction = MAX(-speedsteer, MIN(speedsteer, steer_direction));
		}
	}

	float steer = (float) (steer_direction/car->_steerLock);
//fprintf(stderr,"steer=%.3f",steer);

	// smooth steering.  I know there's a separate function for this, but what the hey!
	if (mode != pitting)
	{
		double minspeedfactor = (((80.0 - (MAX(40.0, MIN(70.0, getSpeed() + MAX(0.0, car->_accel_x*5))) - 25)) / 300) * (5.0 + MAX(0.0, (CA-1.9)*20)));
		double maxspeedfactor = minspeedfactor;
		double rInverse = raceline.getRInverse();

		if (rInverse > 0.0)
		{
			minspeedfactor = MAX(minspeedfactor/3, minspeedfactor - rInverse*80.0);//getSpeed() + MAX(0.0f, angle/100));
			maxspeedfactor = MAX(maxspeedfactor/3, maxspeedfactor + rInverse*20.0);//getSpeed() + MIN(0.0f, angle/100));
		}
		else
		{
			maxspeedfactor = MAX(maxspeedfactor/3, maxspeedfactor + rInverse*80.0);//getSpeed() + MIN(0.0f, angle/100));
			minspeedfactor = MAX(minspeedfactor/3, minspeedfactor + rInverse*20.0);//getSpeed() + MAX(0.0f, angle/100));
		}
			
		steer = (float) MAX(lastNSasteer - minspeedfactor, MIN(lastNSasteer + maxspeedfactor, steer));
	}
//fprintf(stderr," B%.3f",steer);

	lastNSasteer = steer;

	if (fabs(angle) > fabs(speedangle))
	{
		// steer into the skid
		double sa = MAX(-0.3, MIN(0.3, speedangle/3));
		double anglediff = (sa - angle) * (0.7 - MAX(0.0, MIN(0.3, car->_accel_x/100)));
		//anglediff += raceline.getRInverse() * 10;
		steer += (float) (anglediff*0.7);
	}
//fprintf(stderr," C%.3f",steer);

	if (fabs(angle) > 1.2)
	{
		if (steer > 0.0f)
			steer = 1.0f;
		else
			steer = -1.0f;
	}
	else if (fabs(car->_trkPos.toMiddle) - car->_trkPos.seg->width/2 > 2.0)
	{
		steer = (float) MIN(1.0f, MAX(-1.0f, steer * (1.0f + (fabs(car->_trkPos.toMiddle) - car->_trkPos.seg->width/2)/14 + fabs(angle)/2)));
	}
//fprintf(stderr," D%.3f",steer);

	if (mode != pitting)
   	{
	   // limit how far we can steer against raceline 
	   double limit = (90.0 - MAX(40.0, MIN(60.0, car->_speed_x))) / (50 + fabs(angle)*fabs(angle)*3);
	   steer = MAX(racesteer-limit, MIN(racesteer+limit, steer));
   	}
//fprintf(stderr," E%.3f\n",steer);

	return steer;
}


float Driver::correctSteering( float avoidsteer, float racesteer )
{
	float steer = avoidsteer;
	float accel = MIN(0.0f, car->_accel_x);
	double speed = MAX(50.0, getSpeed());
	double changelimit = MIN(raceline.correctLimit(), (((120.0-getSpeed())/6000) * (0.5 + MIN(fabs(avoidsteer),fabs(racesteer))/10)));

	if (mode == correcting && simtime > 2.0f)
	{
		// move steering towards racesteer...
		if (correctlimit < 900.0)
		{
			if (steer < racesteer)
			{
				if (correctlimit >= 0.0)
				{
					steer = racesteer;
				}
				else
				{
					steer = (float) MIN(racesteer, MAX(steer, racesteer + correctlimit));
				}
			}
			else
			{
				if (correctlimit <= 0.0)
				{
					steer = racesteer;
				}
				else
				{
					steer = (float) MAX(racesteer, MIN(steer, racesteer + correctlimit));
				}
			}
		}
	
		speed -= car->_accel_x/10;
		speed = MAX(55.0, MIN(150.0, speed + (speed*speed/55)));
		double rInverse = raceline.getRInverse() * (car->_accel_x<0.0 ? 1.0 + fabs(car->_accel_x)/10.0 : 1.0);
		double correctspeed = 0.5;
		if ((rInverse > 0.0 && racesteer > steer) || (rInverse < 0.0 && racesteer < steer))
			correctspeed += rInverse*110;

		if (racesteer > steer)
			steer = (float) MIN(racesteer, steer + changelimit);
			//steer = (float) MIN(racesteer, steer + (((155.0-speed)/10000) * correctspeed));
		else
			steer = (float) MAX(racesteer, steer - changelimit);
			//steer = (float) MAX(racesteer, steer - (((155.0-speed)/10000) * correctspeed));
#if 0
		if (racesteer > avoidsteer)
			steer = (float) MIN(racesteer, avoidsteer + changelimit);
		else
			steer = (float) MAX(racesteer, avoidsteer - changelimit);
#endif
	
 		correctlimit = (steer - racesteer); // * 1.08;
	}

	return steer;
}


float Driver::smoothSteering( float steercmd )
{
	// try to limit sudden changes in steering to avoid loss of control through oversteer. 
	double speedfactor = (((60.0 - (MAX(40.0, MIN(70.0, getSpeed() + MAX(0.0, car->_accel_x*5))) - 25)) / 300) * 1.2) / 0.785;
	double rearskid = MAX(0.0, MAX(car->_skid[2], car->_skid[3]) - MAX(car->_skid[0], car->_skid[1]));

	if (fabs(steercmd) < fabs(laststeer) && fabs(steercmd) <= fabs(laststeer - steercmd))
		speedfactor *= 2;

	steercmd = (float) MAX(laststeer - speedfactor, MIN(laststeer + speedfactor, steercmd));
	return steercmd;
}

// Compute the clutch value.
float Driver::getClutch()
{
	if (1 || car->_gearCmd > 1) 
	{
		float maxtime = MAX(0.06f, 0.32f - ((float) car->_gearCmd / 65.0f));
		if (car->_gear != car->_gearCmd)
			clutchtime = maxtime;
		if (clutchtime > 0.0f)
			clutchtime -= (float) (RCM_MAX_DT_ROBOTS * (0.02f + ((float) car->_gearCmd / 8.0f)));
		return 2.0f * clutchtime;
	} else {
		float drpm = car->_enginerpm - car->_enginerpmRedLine/2.0f;
		float ctlimit = 0.9f;
		if (car->_gearCmd > 1)
			ctlimit -= 0.15f + (float) car->_gearCmd/13.0f;
		clutchtime = MIN(ctlimit, clutchtime);
		if (car->_gear != car->_gearCmd)
			clutchtime = 0.0f;
		float clutcht = (ctlimit - clutchtime) / ctlimit;
		if (car->_gear == 1 && car->_accelCmd > 0.0f) {
			clutchtime += (float) RCM_MAX_DT_ROBOTS;
		}

		if (car->_gearCmd == 1 || drpm > 0) {
			float speedr;
			if (car->_gearCmd == 1) {
				// Compute corresponding speed to engine rpm.
				float omega = car->_enginerpmRedLine/car->_gearRatio[car->_gear + car->_gearOffset];
				float wr = car->_wheelRadius(2);
				speedr = (CLUTCH_SPEED + MAX(0.0f, car->_speed_x))/fabs(wr*omega);
				float clutchr = MAX(0.0f, (1.0f - speedr*2.0f*drpm/car->_enginerpmRedLine)) * (car->_gearCmd == 1 ? 0.95f : (0.7f - (float)(car->_gearCmd)/30.0f));
				return MIN(clutcht, clutchr);
			} else {
				// For the reverse gear.
				clutchtime = 0.0f;
				return 0.0f;
			}
		} else {
			return clutcht;
		}
	}
}

// Compute target point for steering.
vec2f Driver::getTargetPoint()
{
	tTrackSeg *seg = car->_trkPos.seg;
	float lookahead;
	float length = getDistToSegEnd();
	float offset = getOffset();
	float pitoffset = -100.0f;
//fprintf(stderr,"avoiding=%d%c offset=%.1f->%.1f\n",(mode==avoiding),((avoidmode&avoidleft)?'L':((avoidmode&avoidright)?'R':' ')),car->_trkPos.toMiddle,offset);fflush(stderr);

	if (pit->getInPit()) {
		// To stop in the pit we need special lookahead values.
		if (currentspeedsqr > pit->getSpeedlimitSqr()) {
			lookahead = PIT_LOOKAHEAD + car->_speed_x*LOOKAHEAD_FACTOR;
		} else {
			lookahead = PIT_LOOKAHEAD;
		}
	} else {
		// Usual lookahead.
		lookahead = rlookahead;
#if 1
		double speed = MAX(20.0, getSpeed());// + MAX(0.0, car->_accel_x));
		lookahead = (float) (LOOKAHEAD_CONST * 1.2 + speed * 0.60);
		lookahead = MIN(lookahead, (float) (LOOKAHEAD_CONST + ((speed*(speed/7)) * 0.15)));
#endif
		// Prevent "snap back" of lookahead on harsh braking.
		float cmplookahead = oldlookahead - car->_speed_x*RCM_MAX_DT_ROBOTS;
		if (lookahead < cmplookahead) {
			lookahead = cmplookahead;
		}
	}

	oldlookahead = lookahead;

	// Search for the segment containing the target point.
	while (length < lookahead) {
		seg = seg->next;
		length += seg->length;
	}

	length = lookahead - length + seg->length;
	float fromstart = seg->lgfromstart;
	fromstart += length;

	// Compute the target point.
	pitoffset = pit->getPitOffset(pitoffset, fromstart);
	if ((pit->getPitstop() || pit->getInPit()) && pitoffset != -100.0f)
	{
		setMode(pitting);
		offset = myoffset = pitoffset;
	}
	else if (mode == pitting)
		setMode(correcting);

	vec2f s;
	if (mode != pitting)
	{
		raceline.GetPoint( offset, lookahead, &s );
		return s;
	}

	s.x = (seg->vertex[TR_SL].x + seg->vertex[TR_SR].x)/2.0f;
	s.y = (seg->vertex[TR_SL].y + seg->vertex[TR_SR].y)/2.0f;

	if ( seg->type == TR_STR) {
		vec2f d, n;
		n.x = (seg->vertex[TR_EL].x - seg->vertex[TR_ER].x)/seg->length;
		n.y = (seg->vertex[TR_EL].y - seg->vertex[TR_ER].y)/seg->length;
		n.normalize();
		d.x = (seg->vertex[TR_EL].x - seg->vertex[TR_SL].x)/seg->length;
		d.y = (seg->vertex[TR_EL].y - seg->vertex[TR_SL].y)/seg->length;
		return s + d*length + offset*n;
	} else {
		vec2f c, n, t, rt;
		c.x = seg->center.x;
		c.y = seg->center.y;
		float arc = length/seg->radius;
		float arcsign = (seg->type == TR_RGT) ? -1.0f : 1.0f;
		arc = arc*arcsign;
		s = s.rotate(c, arc);

		n = c - s;
		n.normalize();
		t = s + arcsign*offset*n;

		if (mode != pitting)
		{
			// bugfix - calculates target point a different way, thus
			// bypassing an error in the BT code that sometimes made
			// the target closer to the car than lookahead...
			raceline.GetPoint( offset, lookahead, &rt );
			double dx = t.x - car->_pos_X;
			double dy = t.y - car->_pos_Y;
			double dist1 = sqrt(dx*dx + dy*dy);
			dx = rt.x - car->_pos_X;
			dy = rt.y - car->_pos_Y;
			double dist2 = sqrt(dx*dx + dy*dy);
			if (dist2 > dist1)
				t = rt;
		}

		return t;
	}
}


// Compute offset to normal target point for overtaking or let pass an opponent.
float Driver::getOffset()
{
	int i;
	float catchdist, mincatchdist = 500.0f, mindist = -1000.0f;
	Opponent *o = NULL;
	avoidmode = 0;

	myoffset = car->_trkPos.toMiddle;
	avoidlftoffset = MAX(myoffset, car->_trkPos.seg->width / 2 - 1.5);
	avoidrgtoffset = MIN(myoffset, -(car->_trkPos.seg->width / 2 - 1.5));

	// Increment speed dependent.
	double rInverse = raceline.getRInverse();
	double incspeed = MIN(60.0, MAX(45.0, getSpeed())) - 20.0;
	double incfactor = (MAX_INC_FACTOR - MIN(fabs(incspeed)/MAX_INC_FACTOR, (MAX_INC_FACTOR - 1.0f))) * (12.0f + MAX(0.0, (CA-1.9)*14));
	double rgtinc = incfactor * MIN(1.3, MAX(0.4, 1.0 + rInverse * (rInverse<0.0?20:80)));
	double lftinc = incfactor * MIN(1.3, MAX(0.4, 1.0 - rInverse * (rInverse>0.0?20:80)));

	{
		int offlft = (myoffset > car->_trkPos.seg->width/2 - 1.5);
		int offrgt = (myoffset < -(car->_trkPos.seg->width/2 - 1.5));

		if (offlft)
			myoffset -= OVERTAKE_OFFSET_INC*rgtinc/2;
		else if (offrgt)
			myoffset += OVERTAKE_OFFSET_INC*lftinc/2;

		avoidlftoffset = MAX(avoidlftoffset, myoffset - OVERTAKE_OFFSET_INC*rgtinc*(offlft ? 6 : 2));
		avoidrgtoffset = MIN(avoidrgtoffset, myoffset + OVERTAKE_OFFSET_INC*lftinc*(offrgt ? 6 : 2));
	}

	double maxoffset = track->width/2 - (car->_dimension_y); // limit to the left
	double minoffset = -(track->width/2 - (car->_dimension_y)); // limit to the right

	if (myoffset < minoffset) // we're already outside right limit, bring us back towards track
	{
		minoffset = myoffset + OVERTAKE_OFFSET_INC*lftinc;
		maxoffset = MIN(maxoffset, myoffset+OVERTAKE_OFFSET_INC*lftinc*2);
	}
	else if (myoffset > maxoffset) // outside left limit, bring us back
	{
		maxoffset = myoffset - OVERTAKE_OFFSET_INC*rgtinc;
		minoffset = MAX(minoffset, myoffset-OVERTAKE_OFFSET_INC*rgtinc*2);
	}
	else
	{
		// set tighter limits based on how far we're allowed to move
		maxoffset = MIN(maxoffset, myoffset+OVERTAKE_OFFSET_INC*lftinc*2);
		minoffset = MAX(minoffset, myoffset-OVERTAKE_OFFSET_INC*rgtinc*2);
	}

	// Side Collision.
	for (i = 0; i < opponents->getNOpponents(); i++) 
	{
		tCarElt *ocar = opponent[i].getCarPtr();

		if (ocar->_state > RM_CAR_STATE_PIT)
			continue;

		if (fabs(ocar->_trkPos.toMiddle) > car->_trkPos.seg->width/2 + 3.0 && 
				fabs(car->_trkPos.toMiddle-ocar->_trkPos.toMiddle) >= 5.0)
			continue;

		if ((opponent[i].getState() & OPP_SIDE))
		{
			setMode( avoiding );
			o = &opponent[i];
			
			float sidedist = fabs(ocar->_trkPos.toLeft - car->_trkPos.toLeft);
			float sidemargin = opponent[i].getWidth() + getWidth() + 2.0f;
			float side = car->_trkPos.toMiddle - ocar->_trkPos.toMiddle;

			if ((side > 0.0 && rInverse < 0.0) ||
			    (side < 0.0 && rInverse > 0.0))
			{
				// avoid more if on the outside of opponent on a bend.  Stops us
				// from cutting in too much and colliding...
				sidemargin += fabs(rInverse) * 150;
			}

			double sidedist2 = sidedist;
			if (side > 0.0)
			{
				sidedist2 -= (o->getSpeedAngle() - speedangle) * 40;
				sidemargin -= MIN(0.0, rInverse*100);
			}
			else
			{
				sidedist2 -= (speedangle - o->getSpeedAngle()) * 40;
				sidemargin += MAX(0.0, rInverse*100);
			}
			sidedist = MIN(sidedist, sidemargin);

			if (sidedist < sidemargin)
			{
				//float w = car->_trkPos.seg->width/WIDTHDIV-BORDER_OVERTAKE_MARGIN;
				double sdiff = (3.0 - (sidemargin-sidedist)/sidemargin);

				if (side > 0.0f) {
					myoffset += (float) (OVERTAKE_OFFSET_INC*lftinc*MAX(0.2f, MIN(1.0f, sdiff)));
				} else {
					myoffset -= (float) (OVERTAKE_OFFSET_INC*rgtinc*MAX(0.2f, MIN(1.0f, sdiff)));
				}
			}
			else if (sidedist > sidemargin + 3.0 && raceoffset > myoffset + OVERTAKE_OFFSET_INC*incfactor)
				myoffset += OVERTAKE_OFFSET_INC*lftinc/4;
			else if (sidedist > sidemargin + 3.0 && raceoffset < myoffset - OVERTAKE_OFFSET_INC*incfactor)
				myoffset -= OVERTAKE_OFFSET_INC*rgtinc/4;

#if 1
			if (ocar->_trkPos.toLeft > car->_trkPos.toLeft)
			{
				//avoidrgtoffset = (float) MAX(avoidrgtoffset, ocar->_trkPos.toMiddle + (o->getWidth()+1.0f));
				avoidmode |= avoidright;
			}
			else
			{
				//avoidlftoffset = (float) MIN(avoidlftoffset, ocar->_trkPos.toMiddle - (o->getWidth()+1.0f));
				avoidmode |= avoidleft;
			}
			avoidmode |= avoidside;
#endif

			myoffset = MIN(avoidlftoffset, MAX(avoidrgtoffset, myoffset));
			myoffset = MIN(maxoffset, MAX(minoffset, myoffset));
			return myoffset;
		}
	}


	// Overtake.
	mincatchdist = MAX(30.0, 1500.0 - fabs(raceline.getRInverse()) * 10000);
	int otry_success = 0;

	for (int otry=0; otry<=1; otry++)
	{
		for (i = 0; i < opponents->getNOpponents(); i++) 
		{
			tCarElt *ocar = opponent[i].getCarPtr();

			if ((opponent[i].getState() & OPP_FRONT_FOLLOW))
				continue;

			if (fabs(ocar->_trkPos.toMiddle) > car->_trkPos.seg->width/2 + 3.0 && 
			    fabs(car->_trkPos.toMiddle-ocar->_trkPos.toMiddle) >= 5.0)
				continue;

			if (ocar->_state > RM_CAR_STATE_PIT)
				continue;
	
			if ((opponent[i].getState() & OPP_FRONT) &&
			    !(opponent[i].isTeamMate() && car->race.laps <= opponent[i].getCarPtr()->race.laps))
			{
				int segid = car->_trkPos.seg->id;
				int osegid = opponent[i].getCarPtr()->_trkPos.seg->id;
				double otry_factor = (otry ? (0.2 + (1.0 - ((currentsimtime-avoidtime)/7.0)) * 0.8) : 1.0);
				double distance = opponent[i].getDistance() * otry_factor;
				double speed = MIN(avoidspeed, getSpeed() + MAX(0.0, (10.0 - distance)));
				double ospeed = opponent[i].getSpeed();
				catchdist = (float) MIN(speed*distance/(speed-ospeed), distance*CATCH_FACTOR) * otry_factor;

				if (catchdist < mincatchdist && distance < fabs(speed-ospeed)*2)
				{
//fprintf(stderr,"%.1f %s: OVERTAKE! (cd %.1f<%.1f) (dist %.1f < (%.1f-%.1f)*2 = %.1f\n",otry_factor,ocar->_name,catchdist,mincatchdist,distance,speed,ospeed,fabs(speed-ospeed)*2);
					mincatchdist = catchdist;
					o = &opponent[i];
					otry_success = otry;
				}
//else
//fprintf(stderr,"%.1f %s: FAIL!!!!! (cd %.1f<%.1f) (dist %.1f < (%.1f-%.1f)*2 = %.1f\n",otry_factor,ocar->_name,catchdist,mincatchdist,distance,speed,ospeed,fabs(speed-ospeed)*2);
			}
		}

		if (o) break;
		if (mode != avoiding)
			break;
	}

	if (o != NULL) {
		setMode( avoiding );
		if (otry_success == 0)
			avoidtime = currentsimtime;
		tCarElt *ocar = o->getCarPtr();

		// Compute the width around the middle which we can use for overtaking.
		float w = ocar->_trkPos.seg->width/WIDTHDIV-BORDER_OVERTAKE_MARGIN;
		// Compute the opponents distance to the middle.
		float otm = ocar->_trkPos.toMiddle;
		// Define the with of the middle range.
		float wm = ocar->_trkPos.seg->width*CENTERDIV;
		float sidemargin = o->getWidth() + getWidth() + 2.0f;
		float sidedist = fabs(ocar->_trkPos.toLeft - car->_trkPos.toLeft);

		if ((otm < -(ocar->_trkPos.seg->width - 5.0) && rInverse < 0.0) ||
		    (otm > (ocar->_trkPos.seg->width - 5.0) && rInverse > 0.0))
		{
			// avoid more if on the outside of opponent on a bend.  Stops us
			// from cutting in too much and colliding...
			sidemargin += fabs(rInverse) * 150;
		}

		if (otm > (ocar->_trkPos.seg->width - 5.0) ||
		    (car->_trkPos.toLeft > ocar->_trkPos.toLeft && (sidedist < sidemargin || (o->getState() & OPP_COLL))))
		{
			myoffset -= OVERTAKE_OFFSET_INC*rgtinc;
			avoidmode |= avoidleft;
		} 
		else if (otm < -(ocar->_trkPos.seg->width - 5.0) ||
		         (car->_trkPos.toLeft < ocar->_trkPos.toLeft && (sidedist < sidemargin || (o->getState() & OPP_COLL))))
		{
			myoffset += OVERTAKE_OFFSET_INC*lftinc;
			avoidmode |= avoidright;
		} 
		else 
		{
			// If the opponent is near the middle we try to move the offset toward
			// the inside of the expected turn.
			// Try to find out the characteristic of the track up to catchdist.
			tTrackSeg *seg = car->_trkPos.seg;
			float length = getDistToSegEnd();
			float oldlen, seglen = length;
			float lenright = 0.0f, lenleft = 0.0f;
			mincatchdist = MIN(mincatchdist, DISTCUTOFF);

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
				}
				seg = seg->next;
				seglen = seg->length;
				oldlen = length;
				length += seglen;
			} while (oldlen < mincatchdist);

			// If we are on a straight look for the next turn.
			if (lenleft == 0.0f && lenright == 0.0f) {
				while (seg->type == TR_STR) {
					seg = seg->next;
				}
				// Assume: left or right if not straight.
				if (seg->type == TR_LFT) {
					lenleft = 1.0f;
				} else {
					lenright = 1.0f;
				}
			}

			// Because we are inside we can go to the border.
			float maxoff = (float) ((ocar->_trkPos.seg->width - car->_dimension_y)/2.0f-BORDER_OVERTAKE_MARGIN*2);

			if ((lenleft>lenright && rInverse < 0.0) ||
			    (lenleft<=lenright && rInverse > 0.0))
			{
				// avoid more if on the outside of opponent on a bend.  Stops us
				// from cutting in too much and colliding...
				sidemargin += fabs(rInverse) * 150;
			}

			if (lenleft > lenright) {
				if (sidedist<sidemargin || (o->getState() & OPP_COLL)) {
					myoffset += OVERTAKE_OFFSET_INC*lftinc*0.7;
				}
				avoidmode |= avoidright;
			} else {
				if (sidedist<sidemargin || (o->getState() & OPP_COLL)) {
					myoffset -= OVERTAKE_OFFSET_INC*rgtinc*0.7;
				}
				avoidmode |= avoidleft;
			}
		}

		myoffset = MIN(avoidlftoffset, MAX(avoidrgtoffset, myoffset));
		myoffset = MIN(maxoffset, MAX(minoffset, myoffset));
		return myoffset;
	}

	// Let overlap or let less damaged team mate pass.
	o = NULL;

	for (i = 0; i < opponents->getNOpponents(); i++) {
		// Let the teammate with less damage overtake to use slipstreaming.
		// The position change happens when the damage difference is greater than
		// TEAM_DAMAGE_CHANGE_LEAD.
		if (((opponent[i].getState() & OPP_LETPASS) && !opponent[i].isTeamMate()) ||
			(opponent[i].isTeamMate() && (car->_dammage - opponent[i].getDamage() > TEAM_DAMAGE_CHANGE_LEAD) &&
			(opponent[i].getDistance() > -TEAM_REAR_DIST) && (opponent[i].getDistance() < -car->_dimension_x) &&
			car->race.laps == opponent[i].getCarPtr()->race.laps))
		{
			// Behind, larger distances are smaller ("more negative").
			if (opponent[i].getDistance() > mindist) {
				mindist = opponent[i].getDistance();
				o = &opponent[i];
			}
		}
	}

	if (o != NULL) {
		tCarElt *ocar = o->getCarPtr();
		float side = car->_trkPos.toMiddle - ocar->_trkPos.toMiddle;
		float w = car->_trkPos.seg->width/WIDTHDIV-BORDER_OVERTAKE_MARGIN;
		if (side > 0.0f) {
			if (myoffset < w) {
				myoffset += OVERTAKE_OFFSET_INC*lftinc/2;
			}
		} else {
			if (myoffset > -w) {
				myoffset -= OVERTAKE_OFFSET_INC*rgtinc/2;
			}
		}
		setMode( avoiding );

#if 0
		if (ocar->_trkPos.toLeft > car->_trkPos.toLeft)
		{
			avoidrgtoffset = (float) MAX(avoidrgtoffset, ocar->_trkPos.toMiddle + (o->getWidth()+1.0f));
			avoidmode |= avoidright;
		}
		else
		{
			avoidlftoffset = (float) MIN(avoidlftoffset, ocar->_trkPos.toMiddle - (o->getWidth()+1.0f));
			avoidmode |= avoidleft;
		}
#endif

		myoffset = MIN(avoidlftoffset, MAX(avoidrgtoffset, myoffset));
		myoffset = MIN(maxoffset, MAX(minoffset, myoffset));
		return myoffset;
	} 

#if 1
	// no-one to avoid, work back towards raceline
	if (mode != normal && fabs(myoffset-raceoffset) > 1.0)
	{
		if (myoffset > raceoffset + OVERTAKE_OFFSET_INC * rgtinc/4)
			myoffset -= OVERTAKE_OFFSET_INC * rgtinc/4;
		else if (myoffset < raceoffset + OVERTAKE_OFFSET_INC * lftinc/4)
			myoffset += OVERTAKE_OFFSET_INC * lftinc/4;
	}
#endif
	if (simtime > 2.0f)
	{
		if (myoffset > raceoffset)
			myoffset = MAX(raceoffset, myoffset - OVERTAKE_OFFSET_INC*incfactor/2);
		else
			myoffset = MIN(raceoffset, myoffset + OVERTAKE_OFFSET_INC*incfactor/2);
	}

	myoffset = MIN(maxoffset, MAX(minoffset, myoffset));
	return myoffset;
}


// Update my private data every timestep.
void Driver::update(tSituation *s)
{
	// Update global car data (shared by all instances) just once per timestep.
	if (currentsimtime != s->currentTime) {
		currentsimtime = s->currentTime;
		cardata->update();
	}

	// Update the local data rest.
	speedangle = (float) -(mycardata->getTrackangle() - atan2(car->_speed_Y, car->_speed_X));
	NORM_PI_PI(speedangle);
	mass = CARMASS + car->_fuel;
	currentspeedsqr = car->_speed_x*car->_speed_x;
	opponents->update(s, this);
	strategy->update(car, s);

	if (car->_state <= RM_CAR_STATE_PIT)
	{
		if (!pit->getPitstop() && (car->_distFromStartLine < pit->getNPitEntry() || car->_distFromStartLine > pit->getNPitEnd())) 
		{
			pit->setPitstop(strategy->needPitstop(car, s));
		}

		if (pit->getPitstop() && car->_pit)
		{
			pitstatus[carindex] = 1;
			for (int i=0; i<opponents->getNOpponents(); i++)
			{
				int idx = opponent[i].getIndex();
				if (opponent[i].getTeam() != TEAM_FRIEND) continue;
				if (opponent[i].getCarPtr() == car) continue;
				if (opponent[i].getCarPtr()->_state > RM_CAR_STATE_PIT)
					continue;
 
				if (pitstatus[idx] == 1 ||
				    ((pitstatus[idx] || (opponent[i].getCarPtr()->_fuel < car->_fuel-1.0 && car->_dammage < 5000))
				    && fabs(car->_trkPos.toMiddle) <= car->_trkPos.seg->width/2))
				{
					pit->setPitstop( 0 );
					pitstatus[carindex] = 0;
				}
				break;
			}
		}
		else if (!pit->getInPit())
			pitstatus[carindex] = 0;
	}
	else
		pitstatus[carindex] = 0;

	pit->update();
	alone = isAlone();
	simtime = s->currentTime;

	float trackangle = RtTrackSideTgAngleL(&(car->_trkPos));
	angle = trackangle - car->_yaw;
	NORM_PI_PI(angle);
	angle = -angle;
}


int Driver::isAlone()
{
	int i;
	for (i = 0; i < opponents->getNOpponents(); i++) {
		if (opponent[i].getState() & (OPP_COLL | OPP_LETPASS)) {
			return 0;	// Not alone.
		}
	}
	return 1;	// Alone.
}


// Check if I'm stuck.
bool Driver::isStuck()
{
	if (fabs(mycardata->getCarAngle()) > MAX_UNSTUCK_ANGLE &&
		car->_speed_x < MAX_UNSTUCK_SPEED &&
		fabs(car->_trkPos.toMiddle) > MIN_UNSTUCK_DIST) 
	{
		if (stuck > MAX_UNSTUCK_COUNT && car->_trkPos.toMiddle*mycardata->getCarAngle() < 0.0) {
			return true;
		} else {
			stuck++;
			return false;
		}
	} else {
		stuck = 0;
		return false;
	}
}


// Compute aerodynamic downforce coefficient CA.
void Driver::initCa()
{
	char *WheelSect[4] = {SECT_FRNTRGTWHEEL, SECT_FRNTLFTWHEEL, SECT_REARRGTWHEEL, SECT_REARLFTWHEEL};
	float rearwingarea = GfParmGetNum(car->_carHandle, SECT_REARWING, PRM_WINGAREA, (char*) NULL, 0.0f);
	float rearwingangle = GfParmGetNum(car->_carHandle, SECT_REARWING, PRM_WINGANGLE, (char*) NULL, 0.0f);
	float wingca = 1.23f*rearwingarea*sin(rearwingangle);

	float cl = GfParmGetNum(car->_carHandle, SECT_AERODYNAMICS, PRM_FCL, (char*) NULL, 0.0f) +
			   GfParmGetNum(car->_carHandle, SECT_AERODYNAMICS, PRM_RCL, (char*) NULL, 0.0f);
	float h = 0.0f;
	int i;
	for (i = 0; i < 4; i++)
		h += GfParmGetNum(car->_carHandle, WheelSect[i], PRM_RIDEHEIGHT, (char*) NULL, 0.20f);
	h*= 1.5f; h = h*h; h = h*h; h = 2.0f * exp(-3.0f*h);
	CA = h*cl + 4.0f*wingca;
//fprintf(stderr,"CA=%.3f\n",CA);
}


// Compute aerodynamic drag coefficient CW.
void Driver::initCw()
{
	float cx = GfParmGetNum(car->_carHandle, SECT_AERODYNAMICS, PRM_CX, (char*) NULL, 0.0f);
	float frontarea = GfParmGetNum(car->_carHandle, SECT_AERODYNAMICS, PRM_FRNTAREA, (char*) NULL, 0.0f);
	CW = 0.645f*cx*frontarea;
//fprintf(stderr,"CW=%.3f\n",CW);
}


// Init the friction coefficient of the the tires.
void Driver::initTireMu()
{
	char *WheelSect[4] = {SECT_FRNTRGTWHEEL, SECT_FRNTLFTWHEEL, SECT_REARRGTWHEEL, SECT_REARLFTWHEEL};
	float tm = FLT_MAX;
	int i;

	for (i = 0; i < 4; i++) {
		tm = MIN(tm, GfParmGetNum(car->_carHandle, WheelSect[i], PRM_MU, (char*) NULL, 1.0f));
	}
	TIREMU = tm;
}


// Reduces the brake value such that it fits the speed (more downforce -> more braking).
float Driver::filterBrakeSpeed(float brake)
{
	float weight = (CARMASS + car->_fuel)*G;
	float maxForce = weight + CA*MAX_SPEED*MAX_SPEED;
	float force = weight + CA*currentspeedsqr;
	return brake*force/maxForce;
}


// Brake filter for pit stop.
float Driver::filterBPit(float brake)
{
	if (pit->getPitstop() && !pit->getInPit()) {
		float dl, dw;
		RtDistToPit(car, track, &dl, &dw);
		if (dl < PIT_BRAKE_AHEAD) {
			float mu = car->_trkPos.seg->surface->kFriction*TIREMU*PIT_MU;
			if (brakedist(0.0f, mu) > dl) {
				return 1.0f;
			}
		}
	}

	if (pit->getInPit()) {
		float s = pit->toSplineCoord(car->_distFromStartLine);
		// Pit entry.
		if (pit->getPitstop()) {
			float mu = car->_trkPos.seg->surface->kFriction*TIREMU*PIT_MU;
			if (s < pit->getNPitStart()) {
				// Brake to pit speed limit.
				float dist = pit->getNPitStart() - s;
				if (brakedist(pit->getSpeedlimit(), mu) > dist) {
					return 1.0f;
				}
			} else {
				// Hold speed limit.
				if (currentspeedsqr > pit->getSpeedlimitSqr()) {
					return pit->getSpeedLimitBrake(currentspeedsqr);
				}
			}
			// Brake into pit (speed limit 0.0 to stop)
			float dist = pit->getNPitLoc() - s;
			if (pit->isTimeout(dist)) {
				pit->setPitstop(false);
				return 0.0f;
			} else {
				if (brakedist(0.0f, mu) > dist) {
					return 1.0f;
				} else if (s > pit->getNPitLoc()) {
					// Stop in the pit.
			 		return 1.0f;
				}
			}
		} else {
			// Pit exit.
			if (s < pit->getNPitEnd()) {
				// Pit speed limit.
				if (currentspeedsqr > pit->getSpeedlimitSqr()) {
					return pit->getSpeedLimitBrake(currentspeedsqr);
				}
			}
		}
	}

	return brake;
}


// Brake filter for collision avoidance.
float Driver::filterBColl(float brake)
{
	if (simtime < 1.5)
		return brake;

	float mu = car->_trkPos.seg->surface->kFriction;
	int i;
	for (i = 0; i < opponents->getNOpponents(); i++) 
	{
		if (opponent[i].getState() & OPP_COLL) 
		{
			float ospeed = opponent[i].getSpeed();
			if (brakedist(ospeed, mu) + MIN(1.0, 0.5 + MAX(0.0, (getSpeed()-ospeed)/4)) > opponent[i].getDistance()) 
			{
				accelcmd = 0.0f;
				return 1.0f;
			}
		}
	}
	return brake;
}


// Antilocking filter for brakes.
float Driver::filterABS(float brake)
{
	if (car->_speed_x < ABS_MINSPEED) return brake;
	float origbrake = brake;
	float rearskid = MAX(0.0f, MAX(car->_skid[2], car->_skid[3]) - MAX(car->_skid[0], car->_skid[1]));
	int i;
	float slip = 0.0f;
	for (i = 0; i < 4; i++) {
		slip += car->_wheelSpinVel(i) * car->_wheelRadius(i);
	}
	slip *= 1.0f + MAX(rearskid, MAX(fabs(car->_yaw_rate)/5, fabs(angle)/6));
	slip = car->_speed_x - slip/4.0f;
	if (slip > ABS_SLIP) {
		brake = brake - MIN(brake, (slip - ABS_SLIP)/ABS_RANGE);
	}
	brake = MAX(brake, MIN(origbrake, 0.1f));
	return brake;
}


// TCL filter for accelerator pedal.
float Driver::filterTCL(float accel)
{
	if (simtime < 3.0)
		return accel;

	accel = MIN(1.0f, accel);
	float accel1 = accel, accel2 = accel, accel3 = accel;

	if (car->_speed_x > 10.0f)
	{
		tTrackSeg *seg = car->_trkPos.seg;
		tTrackSeg *wseg0 = car->_wheelSeg(0);
		tTrackSeg *wseg1 = car->_wheelSeg(1);
	  int count = 0;

		if ((wseg0->surface->kRoughness > MAX(0.02, seg->surface->kRoughness*1.2) ||
		     wseg0->surface->kFriction < seg->surface->kFriction*0.8 ||
 	       wseg0->surface->kRollRes > MAX(0.005, seg->surface->kRollRes*1.2)))
			count++;
		if ((wseg1->surface->kRoughness > MAX(0.02, seg->surface->kRoughness*1.2) ||
		     wseg1->surface->kFriction < seg->surface->kFriction*0.8 ||
 	       wseg1->surface->kRollRes > MAX(0.005, seg->surface->kRollRes*1.2)))
			count++;

		if (count)
		{
			if (mode != normal &&
			    ((seg->type == TR_RGT && seg->radius <= 200.0f && car->_trkPos.toLeft < 3.0f) ||
			    (seg->type == TR_LFT && seg->radius <= 200.0f && car->_trkPos.toRight < 3.0f)))
				count++;
			accel1 = MAX(0.0f, MIN(accel1, (1.0f-(0.25f*count)) - MAX(0.0f, (getSpeed()-car->_speed_x)/10.0f)));
		}

		if (fabs(angle) > 1.0)
	 		accel1 = (float) MIN(accel1, 1.0f - (fabs(angle)-1.0)*1.3);
	}

	if (fabs(car->_steerCmd) > 0.02)
	{
		float decel = ((fabs(car->_steerCmd)-0.02f) * (1.0f+fabs(car->_steerCmd)) * 0.7f);
		accel2 = MIN(accel2, MAX(0.45f, 1.0f-decel));
	}

	float slip = (this->*GET_DRIVEN_WHEEL_SPEED)() - car->_speed_x;
	if (slip > TCL_SLIP) {
		accel3 = accel3 - MIN(accel3, (slip - TCL_SLIP)/TCL_RANGE);
	}
	return MIN(accel1, MIN(accel2, accel3));
}


// Traction Control (TCL) setup.
void Driver::initTCLfilter()
{
	const char *traintype = GfParmGetStr(car->_carHandle, SECT_DRIVETRAIN, PRM_TYPE, VAL_TRANS_RWD);
	if (strcmp(traintype, VAL_TRANS_RWD) == 0) {
		GET_DRIVEN_WHEEL_SPEED = &Driver::filterTCL_RWD;
	} else if (strcmp(traintype, VAL_TRANS_FWD) == 0) {
		GET_DRIVEN_WHEEL_SPEED = &Driver::filterTCL_FWD;
	} else if (strcmp(traintype, VAL_TRANS_4WD) == 0) {
		GET_DRIVEN_WHEEL_SPEED = &Driver::filterTCL_4WD;
	}
}


// TCL filter plugin for rear wheel driven cars.
float Driver::filterTCL_RWD()
{
	return (car->_wheelSpinVel(REAR_RGT) + car->_wheelSpinVel(REAR_LFT)) *
			car->_wheelRadius(REAR_LFT) / 2.0f;
}


// TCL filter plugin for front wheel driven cars.
float Driver::filterTCL_FWD()
{
	return (car->_wheelSpinVel(FRNT_RGT) + car->_wheelSpinVel(FRNT_LFT)) *
			car->_wheelRadius(FRNT_LFT) / 2.0f;
}


// TCL filter plugin for all wheel driven cars.
float Driver::filterTCL_4WD()
{
	return ((car->_wheelSpinVel(FRNT_RGT) + car->_wheelSpinVel(FRNT_LFT)) *
			car->_wheelRadius(FRNT_LFT) +
		   (car->_wheelSpinVel(REAR_RGT) + car->_wheelSpinVel(REAR_LFT)) *
			car->_wheelRadius(REAR_LFT)) / 4.0f;
}


// Hold car on the track.
float Driver::filterTrk(float accel)
{
	return accel;

	tTrackSeg* seg = car->_trkPos.seg;

	if (car->_speed_x < MAX_UNSTUCK_SPEED ||		// Too slow.
		pit->getInPit() ||							// Pit stop.
		car->_trkPos.toMiddle*-speedangle > 0.0f)	// Speedvector points to the inside of the turn.
	{
		return accel;
	}

	if (seg->type == TR_STR) {
		float tm = fabs(car->_trkPos.toMiddle);
		float w = (seg->width - car->_dimension_y)/2.0f ;
		if (tm > w) {
			return 0.0f;
		} else {
			return accel;
		}
	} else {
		float sign = (seg->type == TR_RGT) ? -1.0f : 1.0f;
		if (car->_trkPos.toMiddle*sign > 0.0f) {
			return accel;
		} else {
			float tm = fabs(car->_trkPos.toMiddle);
			float w = seg->width/WIDTHDIV;
			if (tm > w) {
				return 0.0f;
			} else {
				return accel;
			}
		}
	}
}


// Compute the needed distance to brake.
float Driver::brakedist(float allowedspeed, float mu)
{
	float c = mu*G;
	float d = (CA*mu + CW)/mass;
	float v1sqr = currentspeedsqr;
	float v2sqr = allowedspeed*allowedspeed;
	return -log((c + v2sqr*d)/(c + v1sqr*d))/(2.0f*d);
}

