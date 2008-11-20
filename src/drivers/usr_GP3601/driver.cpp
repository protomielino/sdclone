/***************************************************************************

    file                 : driver.cpp
    created              : Thu Dec 20 01:21:49 CET 2002
    copyright            : (C) 2002-2004 Bernhard Wymann
    email                : berniw@bluewin.ch
    version              : $Id: driver.cpp,v 1.1 2008/02/11 00:52:01 andrew Exp $

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

#define robot_name "usr_GP3601"

//#define CONTROL_SKILL

//const float Driver::MAX_UNSTUCK_ANGLE = (float)(15.0f/180.0f*PI);	// [radians] If the angle of the car on the track is smaller, we assume we are not stuck.
const float Driver::MAX_UNSTUCK_ANGLE = 1.3f;
const float Driver::UNSTUCK_TIME_LIMIT = 2.5f;				// [s] We try to get unstuck after this time.
const float Driver::MAX_UNSTUCK_SPEED = 5.0f;				// [m/s] Below this speed we consider being stuck.
const float Driver::MIN_UNSTUCK_DIST = -1.0f;				// [m] If we are closer to the middle we assume to be not stuck.
const float Driver::G = 9.81f;								// [m/(s*s)] Welcome on Earth.
const float Driver::FULL_ACCEL_MARGIN = 1.0f;				// [m/s] Margin reduce oscillation of brake/acceleration.
const float Driver::SHIFT = 0.9f;							// [-] (% of rpmredline) When do we like to shift gears.
const float Driver::SHIFT_MARGIN = 4.0f;					// [m/s] Avoid oscillating gear changes.
const float Driver::ABS_RANGE = 5.0f;						// [m/s] range [0..10]
const float Driver::ABS_MINSPEED = 3.0f;					// [m/s] Below this speed the ABS is disabled (numeric, division by small numbers).
const float Driver::TCL_RANGE = 10.0f;						// [m/s] range [0..10]
const float Driver::LOOKAHEAD_CONST = 18.0f;				// [m]
const float Driver::LOOKAHEAD_FACTOR = 0.33f;				// [-]
const float Driver::WIDTHDIV = 3.0f;						// [-] Defines the percentage of the track to use (2/WIDTHDIV).
const float Driver::SIDECOLL_MARGIN = 3.0f;					// [m] Distance between car centers to avoid side collisions.
const float Driver::BORDER_OVERTAKE_MARGIN = 1.0f;			// [m]
const float Driver::OVERTAKE_OFFSET_SPEED = 5.0f;			// [m/s] Offset change speed.
const float Driver::PIT_LOOKAHEAD = 6.0f;					// [m] Lookahead to stop in the pit.
const float Driver::PIT_BRAKE_AHEAD = 200.0f;				// [m] Workaround for "broken" pitentries.
const float Driver::PIT_MU = 0.4f;							// [-] Friction of pit concrete.
const float Driver::MAX_SPEED = 84.0f;						// [m/s] Speed to compute the percentage of brake to apply.
const float Driver::MAX_FUEL_PER_METER = 0.0008f;			// [liter/m] fuel consumtion.
const float Driver::CLUTCH_SPEED = 5.0f;					// [m/s]
const float Driver::CENTERDIV = 0.1f;						// [-] (factor) [0.01..0.6].
const float Driver::DISTCUTOFF = 200.0f;					// [m] How far to look, terminate while loops.
const float Driver::MAX_INC_FACTOR = 12.0f;					// [m] Increment faster if speed is slow [1.0..10.0].
const float Driver::CATCH_FACTOR = 8.0f;					// [-] select MIN(catchdist, dist*CATCH_FACTOR) to overtake.
const float Driver::CLUTCH_FULL_MAX_TIME = 2.0f;			// [s] Time to apply full clutch.
const float Driver::USE_LEARNED_OFFSET_RANGE = 0.2f;		// [m] if offset < this use the learned stuff

const float Driver::TEAM_REAR_DIST = 50.0f;					//
const int Driver::TEAM_DAMAGE_CHANGE_LEAD = 700;			// When to change position in the team?

enum { FLYING_FRONT = 1, FLYING_BACK = 2, FLYING_SIDE = 4 };

// Static variables.
Cardata *Driver::cardata = NULL;


Driver::Driver(int index) :
		NoTeamWaiting(0),
		TeamWaitTime(0.0f),
		truespeed(0.0f),
		deltaTime(0.0f),
		FuelSpeedUp(0.0f),
		TclSlip(2.0f),
		AbsSlip(2.5f),
		random_seed(0),
		DebugMsg(0),
		racetype(0),
		mode(0),
		avoidmode(0),
		lastmode(0),
		allow_stuck(1),
		stuckcheck(0),
		stuck_timer(0.0f),
		prefer_side(0),
		allowcorrecting(0),
		pitpos(0),
		prevspeedangle(0.0f),
		speedangle(0.0f),
		angle(0.0f),
		mass(0.0f),
		maxfuel(0.0f),
		myoffset(0.0f),
		pitoffset(0.0f),
		laststeer(0.0f),
		lastbrake(0.0f),
		lastaccel(0.0f),
		lastNSasteer(0.0f),
		lastNSksteer(0.0f),
		avgaccel_x(0.0f),
		car(NULL),
		raceline(NULL),
		opponents(NULL),
		opponent(NULL),
		pit(NULL),
		strategy(NULL),
		mycardata(NULL),
		simtime(0.0),
		avoidtime(0.0),
		frontavoidtime(0.0),
		correcttimer(0.0),
		correctlimit(1000.0),
		aligned_timer(0.0),
		stopped_timer(0.0),
		brakedelay(0.0),
		deltamult(0.0),
		nextCRinverse(0.0),
		sideratio(100.0),
		laststeer_direction(0.0),
		steerLock(0.4),
		currentspeedsqr(0.0f),
		clutchtime(0.0f),
		oldlookahead(0.0f),
		racesteer(0.0f),
		stucksteer(0.0f),
		prevleft(0.0f),
		rldata(NULL),
		avoidlftoffset(0.0f),
		avoidrgtoffset(0.0f),
		accelcmd(0.0f),
		brakecmd(0.0f),
		faccelcmd(0.0f),
		fbrakecmd(0.0f),
		TurnDecel(0.0f),
		PitOffset(0.0f),
		RevsChangeDown(0.0f),
		RevsChangeUp(0.0f),
		RevsChangeDownMax(0.0f),
		CollBrake(0.0f),
		SmoothSteer(1.0f),
		LookAhead(1.0f),
		IncFactor(1.0f),
		SideMargin(0.0f),
		OutSteerFactor(0.0f),
		StuckAccel(0.8f),
		FollowMargin(0.0f),
		SteerLookahead(0.0f),
		SteerMaxRI(0.008),
		lookahead(10.0f),
		MaxGear(0),
		NoPit(0),
		radius(NULL),
		alone(0),
		carindex(0),
		collision(0.0f),
		global_skill(0.0f),
		skill(0.0f),
		speed_adjust_limit(0.0),
		speed_adjust_timer(0.0),
		speed_adjust_targ(0.0),
		speed_adjust_perc(0.0),
		fuelperlap(5.0f),
		MAX_UNSTUCK_COUNT(0),
		INDEX(0),
		CARMASS(0.0f),
		CA(0.0f),
		CW(0.0f),
		TIREMU(0.0f),
		OVERTAKE_OFFSET_INC(0.0f),
		MU_FACTOR(0.0f),
		track(NULL)
{
	INDEX = index;
}


Driver::~Driver()
{
    if (raceline)
    {
        raceline->FreeTrack();
        delete raceline;
    }
	delete opponents;
	delete pit;
	delete [] radius;
	delete strategy;
	delete rldata;
	if (cardata != NULL) {
		delete cardata;
		cardata = NULL;
	}
}

#define RANDOM_SEED 0xfded
#define RANDOM_A    1664525
#define RANDOM_C    1013904223

void Driver::SetRandomSeed(unsigned int seed)
{
 random_seed = seed ? seed : RANDOM_SEED;
 return;
}

unsigned int Driver::getRandom()
{
 random_seed = RANDOM_A * random_seed + RANDOM_C;
 return (random_seed >> 16);
}


// Called for every track change or new race.
void Driver::initTrack(tTrack* t, void *carHandle, void **carParmHandle, tSituation *s)
{
	track = t;

	const int BUFSIZE = 256;
	char buffer[BUFSIZE];

	global_skill = skill = speed_adjust_perc = 0.0;
#ifdef CONTROL_SKILL
 // load the skill level
	//if (s->_raceType == RM_TYPE_PRACTICE)
	//	global_skill = 0.0;
	//else
	{
		snprintf(buffer, BUFSIZE, "config/raceman/extra/skill.xml");
		void *skillHandle = GfParmReadFile(buffer, GFPARM_RMODE_STD);
		if (skillHandle)
			global_skill = GfParmGetNum(skillHandle, "skill", "level", (char *) NULL, 0.0);
		else
			global_skill = 0.0;
		global_skill = MAX(0.0, MIN(10.0, global_skill));
	}
#endif

	// Load a custom setup if one is available.
	// Get a pointer to the first char of the track filename.
	char* trackname = strrchr(track->filename, '/') + 1;
	char carName[256];
	{
		char *path = SECT_GROBJECTS "/" LST_RANGES "/" "1";
		char *key = PRM_CAR;
		strncpy( carName, GfParmGetStr(carHandle, path, key, ""), sizeof(carName) );
		char *p = strrchr(carName, '.');
		if (p) *p = '\0';
	}

	snprintf(buffer, BUFSIZE, "drivers/%s/cars/%s/default.xml", robot_name, carName);
	*carParmHandle = GfParmReadFile(buffer, GFPARM_RMODE_STD);
	void *newhandle;

	switch (s->_raceType) {
		case RM_TYPE_PRACTICE:
			snprintf(buffer, BUFSIZE, "drivers/%s/cars/%s/practice/%s", robot_name, carName, trackname);
			break;
		case RM_TYPE_QUALIF:
			snprintf(buffer, BUFSIZE, "drivers/%s/cars/%s/qualifying/%s", robot_name, carName, trackname);
			break;
		case RM_TYPE_RACE:
			snprintf(buffer, BUFSIZE, "drivers/%s/cars/%s/race/%s", robot_name, carName, trackname);
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
		snprintf(buffer, BUFSIZE, "drivers/%s/cars/%s/%s", robot_name, carName, trackname);
		newhandle = GfParmReadFile(buffer, GFPARM_RMODE_STD);
		if (newhandle)
		{
			if (*carParmHandle)
				*carParmHandle = GfParmMergeHandles(*carParmHandle, newhandle, (GFPARM_MMODE_SRC|GFPARM_MMODE_DST|GFPARM_MMODE_RELSRC|GFPARM_MMODE_RELDST));
			else
				*carParmHandle = newhandle;
		}
	}

	skill = 0.0;
#ifdef CONTROL_SKILL
	snprintf(buffer, BUFSIZE, "drivers/%s/%d/skill.xml", robot_name, INDEX);
	newhandle = GfParmReadFile(buffer, GFPARM_RMODE_STD);
	if (newhandle)
	{
		if (*carParmHandle)
			*carParmHandle = GfParmMergeHandles(*carParmHandle, newhandle, (GFPARM_MMODE_SRC|GFPARM_MMODE_DST|GFPARM_MMODE_RELSRC|GFPARM_MMODE_RELDST));
		else
			*carParmHandle = newhandle;
	}
	
	//if (s->_raceType == RM_TYPE_PRACTICE)
	//	skill = 0.0;
	//else
	{
		skill = GfParmGetNum(*carParmHandle, BT_SECT_PRIV, "Skill", (char *) NULL, 0.0);
		skill = MAX(0.0, MIN(10.0, skill));
		skill += global_skill;
	}
#endif

	// Create a pit stop strategy object.
	strategy = new SimpleStrategy2();
	strategy->setTrack(track);

	// Init fuel.
	strategy->setFuelAtRaceStart(t, carParmHandle, s, INDEX);

	// Load and set parameters.
	MU_FACTOR = GfParmGetNum(*carParmHandle, BT_SECT_PRIV, BT_ATT_MUFACTOR, (char*)NULL, 0.69f);

	PitOffset = GfParmGetNum( *carParmHandle, BT_SECT_PRIV, "PitOffset", (char *)NULL, 10.0f );
	TurnDecel = GfParmGetNum( *carParmHandle, BT_SECT_PRIV, "TurnDecel", (char *)NULL, 1.0f );
	RevsChangeUp = GfParmGetNum( *carParmHandle, BT_SECT_PRIV, "RevsChangeUp", (char *)NULL, 0.96f );
	RevsChangeDown = GfParmGetNum( *carParmHandle, BT_SECT_PRIV, "RevsChangeDown", (char *)NULL, 0.75f );
	RevsChangeDownMax = GfParmGetNum( *carParmHandle, BT_SECT_PRIV, "RevsChangeDownMax", (char *)NULL, 0.85f );
	CollBrake = GfParmGetNum( *carParmHandle, BT_SECT_PRIV, "CollBrake", (char *)NULL, 1.0f );
	SmoothSteer = GfParmGetNum( *carParmHandle, BT_SECT_PRIV, "SmoothSteer", (char *)NULL, 1.0f );
	LookAhead = GfParmGetNum( *carParmHandle, BT_SECT_PRIV, "LookAhead", (char *)NULL, 1.0f );
	IncFactor = GfParmGetNum( *carParmHandle, BT_SECT_PRIV, "IncFactor", (char *)NULL, 1.0f );
	SideMargin = GfParmGetNum( *carParmHandle, BT_SECT_PRIV, "SideMargin", (char *)NULL, 0.0f );
	OutSteerFactor = GfParmGetNum( *carParmHandle, BT_SECT_PRIV, "OutSteerFactor", (char *)NULL, 1.0f );
	StuckAccel = GfParmGetNum( *carParmHandle, BT_SECT_PRIV, "StuckAccel", (char *)NULL, 0.8f );
	FollowMargin = GfParmGetNum( *carParmHandle, BT_SECT_PRIV, "FollowMargin", (char *)NULL, 0.0f );
	SteerLookahead = GfParmGetNum( *carParmHandle, BT_SECT_PRIV, "SteerLookahead", (char *)NULL, 0.0f );
	SteerMaxRI = GfParmGetNum( *carParmHandle, BT_SECT_PRIV, "SteerMaxRI", (char *)NULL, 0.008f );
	MaxGear = (int) GfParmGetNum( *carParmHandle, BT_SECT_PRIV, "MaxGear", (char *)NULL, 6.0f );
	NoPit = (int) GfParmGetNum( *carParmHandle, BT_SECT_PRIV, "NoPit", (char *)NULL, 0.0f );
	NoTeamWaiting = (int) GfParmGetNum( *carParmHandle, BT_SECT_PRIV, "NoTeamWaiting", (char *)NULL, 0.0f );
	TeamWaitTime = (int) GfParmGetNum( *carParmHandle, BT_SECT_PRIV, "TeamWaitTime", (char *)NULL, 0.0f );
}


// Start a new race.
void Driver::newRace(tCarElt* car, tSituation *s)
{
	deltaTime = (float) RCM_MAX_DT_ROBOTS;
	MAX_UNSTUCK_COUNT = int(UNSTUCK_TIME_LIMIT/deltaTime);
	OVERTAKE_OFFSET_INC = OVERTAKE_OFFSET_SPEED*deltaTime;
	random_seed = 0;
	alone = allow_stuck = 1;
	stuckcheck = 0;
	clutchtime = stuck_timer = 0.0f;
	oldlookahead = laststeer = lastbrake = lastaccel = avgaccel_x = lastNSasteer = lastNSksteer = 0.0f;
	speed_adjust_perc = speed_adjust_targ = 0.0f;
	prevleft = car->_trkPos.toLeft;
	this->car = car;
	int stdebug = (int) GfParmGetNum(car->_carHandle, BT_SECT_PRIV, "SteerDebug", NULL, 0);
	int otdebug = (int) GfParmGetNum(car->_carHandle, BT_SECT_PRIV, "OvertakeDebug", NULL, 0);
	int brdebug = (int) GfParmGetNum(car->_carHandle, BT_SECT_PRIV, "BrakeDebug", NULL, 0);
  	if ((RM_TYPE_PRACTICE == s->_raceType && stdebug >= 0) || stdebug > 0) DebugMsg |= debug_steer;
	if (otdebug) DebugMsg |= debug_overtake;
	if (brdebug) DebugMsg |= debug_brake;
	FuelSpeedUp = GfParmGetNum(car->_carHandle, BT_SECT_PRIV, "FuelSpeedUp", NULL, 0.0f);
	TclSlip = GfParmGetNum(car->_carHandle, BT_SECT_PRIV, "TclSlip", NULL, 2.0f);
	AbsSlip = GfParmGetNum(car->_carHandle, BT_SECT_PRIV, "AbsSlip", NULL, 2.5f);
	fuelperlap = GfParmGetNum(car->_carHandle, BT_SECT_PRIV, "FuelPerLap", NULL, 5.0f);
	CARMASS = GfParmGetNum(car->_carHandle, SECT_CAR, PRM_MASS, NULL, 1000.0f);
	maxfuel = GfParmGetNum(car->_carHandle, SECT_CAR, PRM_TANK, NULL, 100.0f);
	steerLock = GfParmGetNum(car->_carHandle, SECT_STEER, PRM_STEERLOCK, (char *)NULL, 4.0f);
	myoffset = 0.0f;
	simtime = correcttimer = speed_adjust_timer = speed_adjust_limit = aligned_timer = stopped_timer = 0.0;
	avoidtime = frontavoidtime = 0.0;
	correctlimit = 1000.0;
	deltamult = 1.0 / s->deltaTime;
	racetype = s->_raceType;
	initWheelPos();
	initCa();
	initCw();
	initTireMu();
	initTCLfilter();

	raceline = new LRaceLine();
	raceline->NewRace( car, s );
	raceline->setSkill( skill );
	raceline->InitTrack(track, s);

	rldata = new LRaceLineData();
	memset(rldata, 0, sizeof(LRaceLineData));

	// Create just one instance of cardata shared by all drivers.
	if (cardata == NULL) {
		cardata = new Cardata(s);
	}
	mycardata = cardata->findCar(car);
	simtime = s->currentTime;
	speed_adjust_timer = -1;

	// initialize the list of opponents.
	opponents = new Opponents(s, this, cardata);
	opponent = opponents->getOpponentPtr();

	// Set team mate.
	char *teammate = GfParmGetStr(car->_carHandle, BT_SECT_PRIV, BT_ATT_TEAMMATE, NULL);
	if (teammate != NULL) {
		opponents->setTeamMate(teammate);
	}

	// Initialize radius of segments.
	radius = new float[track->nseg];
	computeRadius(radius);

	// create the pit object.
	pit = new Pit(s, this, PitOffset);
	setMode( mode_correcting );
	lastmode = mode_correcting;

	carindex = 0;

	for (int i = 0; i < s->_ncars; i++)
	{
		if (s->cars[i] == car)
		{
			carindex = i;
			break;
 		}
 	}
}

void Driver::calcSpeed()
{
	accelcmd = brakecmd = 0.0f;
	faccelcmd = fbrakecmd = 0.0f;
	double speed = rldata->speed;
	double avspeed = MAX((getSpeed()+0.4)-(MAX(0.0, 0.9-fabs(MAX(0.0, angle-speedangle))*5)), rldata->avspeed);
	double slowavspeed = rldata->slowavspeed;

	if (mode == mode_avoiding && !allowcorrecting)
	{
		speed = avspeed;
		if ((avoidmode & avoidside) && !rldata->insideline && sideratio < 1.0 &&
		    ((rldata->rInverse > 0.0 && (avoidmode & avoidright) && speedangle < -(sideratio/10)) ||
		     (rldata->rInverse < 0.0 && (avoidmode & avoidleft) && speedangle > (sideratio/10))))
		{
			speed = slowavspeed;
		}
	}
	else if ((mode == mode_correcting || (simtime - aligned_timer < 2.0)) && rldata->insideline && rldata->closing)
	{
		speed = slowavspeed;
	}
	else if ((mode == mode_correcting || (simtime - aligned_timer < 5.0)))
	{
		//avspeed = MIN(rldata->speed - MIN(3.0, fabs(rldata->rInverse)*400), avspeed);
		avspeed = MIN(rldata->speed, (slowavspeed+avspeed)/2);
		double rlspeed = rldata->speed;//MAX(avspeed, rldata->speed-1.0);
		//speed = avspeed + (rlspeed-avspeed) * MAX(0.0, MIN(1.0, 1.0 - MAX(fabs(correctlimit*2), fabs(fabs(laststeer)-fabs(rldata->rInverse*80))/2)));
		speed = avspeed + (rlspeed-avspeed) * MAX(0.0, MIN(1.0, 1.0 - (fabs(correctlimit*2) + fabs(angle - rldata->rlangle)*5)));
	}

	if (mode == mode_normal && fabs(rldata->rInverse) < 0.001 && fabs(car->_steerCmd) < 0.01 && fabs(angle) < 0.01 && fabs(car->_yaw_rate) < 0.01)
		aligned_timer = 0.0;

	double x = (10 + car->_speed_x) * (speed - car->_speed_x) / 200;
	double lane2left = car->_trkPos.toLeft / track->width;
	double lane2right = car->_trkPos.toRight / track->width;
	int sidedanger = ((rldata->rInverse > 0.0 && speedangle < -(rldata->rInverse) * lane2left*2) || (rldata->rInverse < 0.0 && speedangle > rldata->rInverse*lane2right*2));
	double skid = MAX(0.0, (car->_skid[2] + car->_skid[3] + car->_skid[0] + car->_skid[1])) * 3;

	brakecmd = 0.0f;
	accelcmd = 100.0f;

	{
		double skidangle = angle;
		if (mode != mode_normal)
		{
			if ((angle > 0.0 && speedangle < angle) ||
			    (angle < 0.0 && speedangle > angle))
				skidangle += speedangle/2;
		}

		if (//(mode == mode_normal || !sidedanger) &&
		    (skidangle < 0.0 && laststeer > 0.0 && rldata->rInverse < -0.001) ||
		    (skidangle > 0.0 && laststeer < 0.0 && rldata->rInverse > 0.001))
		{
			// increase acceleration if correcting a skid
			double diff = MAX(0.0, MIN(fabs(laststeer), MAX(fabs(skidangle/7)/50, fabs(rldata->rInverse * 50)))) * (7-skid);
			if (collision)
				diff *= MIN(1.0, collision / 3.0f)*0.8;
			x += diff;
		}
		else if (mode != mode_normal && 
		         (car->_accel_x < 1.0 || sidedanger) &&
		         ((angle > 0.0 && laststeer > 0.0 && rldata->rInverse < -0.001) ||
		          (angle < 0.0 && laststeer < 0.0 && rldata->rInverse > 0.001)))
		{
			// understeering, decrease speed
			double diff = MIN(fabs(laststeer), MAX(fabs(angle)/50, fabs(rldata->rInverse * 50))) * 4;
			x -= diff;
		}

#if 0
		// check if we're really close behind someone on a bend
		// if so (and we're not overtaking) make sure we don't under-brake or
		// over-accelerate and thus lose control.
		fbrakecmd = faccelcmd = 0.0f;

		if (mode == mode_normal)
		{
			for (int i=0; i<opponents->getNOpponents(); i++)
			{
				tCarElt *ocar = opponent[i].getCarPtr();

				if (ocar == car)
					continue;

				if (opponent[i].getTeam() != TEAM_FRIEND)
					continue;

				if (!(opponent[i].getState() & OPP_FRONT) || opponent[i].getDistance() > 3.0)
					continue;

				if (fabs(car->_trkPos.toLeft - ocar->_trkPos.toLeft) > car->_dimension_x*0.8)
					continue;

				if (mode == mode_normal && opponent[i].isTeamMate() && opponent[i].getDistance() < 1.5)
				{
					 accelcmd = ocar->_accelCmd;
					 if (accelcmd > 0.0f)
						 faccelcmd = accelcmd;
				}

				/*
				if ((rldata->rInverse > 0.001 && car->_trkPos.toLeft > rldata->lane * car->_trkPos.seg->width) ||
				    (rldata->rInverse < -0.001 && car->_trkPos.toLeft < rldata->lane * car->_trkPos.seg->width))
				{
					double change = MAX(getSpeed()-rldata->speed, fabs(car->_trkPos.toLeft - (rldata->lane * car->_trkPos.seg->width)));

					x -= MAX(0.0, MIN(1.0, change));
					break;
				}
				*/

				if (opponent[i].getDistance() < 1.5 && ocar->_brakeCmd > 0.0 && (opponent[i].getState() & OPP_COLL))
				{
					brakecmd = ocar->_brakeCmd*MAX(1.0, getSpeed() / opponent[i].getSpeed());
					if (brakecmd > 0.0)
					{
						x = MIN(x, -0.001);
						fbrakecmd = brakecmd;
					}
				}

				break;
			}
		}
#endif
	}

	if (x > 0)
	{
		if (accelcmd < 100.0f)
			accelcmd = MIN(accelcmd, (float) x);
		else
			accelcmd = (float) x;
	}
	else
		brakecmd = MIN(1.0f, MAX(brakecmd, (float) (-(MAX(10.0, brakedelay*0.7))*x)));
}

int Driver::rearOffTrack()
{
  int right_bad = (car->_wheelSeg(REAR_RGT) != car->_trkPos.seg &&
       (car->_wheelSeg(REAR_RGT)->surface->kFriction < car->_trkPos.seg->surface->kFriction*0.8 ||
        car->_wheelSeg(REAR_RGT)->surface->kRoughness > MAX(0.02, car->_trkPos.seg->surface->kRoughness*1.2) ||
        car->_wheelSeg(REAR_RGT)->surface->kRollRes > MAX(0.005, car->_trkPos.seg->surface->kRollRes*1.2)));

  int left_bad =  (car->_wheelSeg(REAR_LFT) != car->_trkPos.seg &&
       (car->_wheelSeg(REAR_LFT)->surface->kFriction < car->_trkPos.seg->surface->kFriction*0.8 ||
        car->_wheelSeg(REAR_LFT)->surface->kRoughness > MAX(0.02, car->_trkPos.seg->surface->kRoughness*1.2) ||
        car->_wheelSeg(REAR_LFT)->surface->kRollRes > MAX(0.005, car->_trkPos.seg->surface->kRollRes*1.2)));

  if (left_bad && right_bad) return 1;
  
  if (car->_speed_x < 10.0 && (left_bad || right_bad)) return 1;

  return 0;
}

// Drive during race.
void Driver::drive(tSituation *s)
{
	laststeer = car->_steerCmd;
	memset(&car->ctrl, 0, sizeof(tCarCtrl));

	update(s);

	//pit->setPitstop(true);

	car->_steerCmd = getSteer(s);

	if (isStuck()) {
		car->_steerCmd = -stucksteer;
		car->_gearCmd = -1;		// Reverse gear.
		car->_accelCmd = 0.5f;	// 100% accelerator pedal.
		if (fabs(car->_speed_x) < 5.0 && car->_gearCmd < 0)
			car->_accelCmd = MAX(car->_accelCmd, 0.75);
		else if (fabs(angle) > 0.8 && car->_gearCmd > 0)
			car->_accelCmd = MAX(MIN(car->_accelCmd, 0.4), 1.0 - fabs(angle)/3);
		car->_brakeCmd = 0.0f;	// No brakes.
		car->_clutchCmd = 0.0f;	// Full clutch (gearbox connected with engine).
		if (DebugMsg & debug_steer)
			fprintf(stderr,"%s %d/%d: STUCK ",car->_name,rldata->thisdiv,rldata->nextdiv);
	} else {
		car->_gearCmd = getGear();
		calcSpeed();
		car->_brakeCmd = filterABS(filterBrakeSpeed(filterBColl(filterBPit(getBrake()))));
		if (car->_brakeCmd == 0.0f) {
			car->_accelCmd = filterTCL(filterTrk(filterTeam(filterOverlap(getAccel()))));
		} else {
			car->_accelCmd = 0.0f;
		}

		if (!collision && stuckcheck == 1)
		{
			car->_steerCmd = stuckSteering( stucksteer );
			if (GET_DRIVEN_WHEEL_SPEED == &Driver::filterTCL_RWD && rearOffTrack())
			{
				// drive wheels at rear and we're off the track, so be careful!
				car->_accelCmd = MAX(MAX((5.0f-car->_speed_x)/10.0f, 0.15f), car->_accelCmd - fabs(car->_yaw_rate));
			}
			else if (car->_speed_x < 10.0f && !collision && fabs(angle) > 1.0f)
			{
				// rev accel high to turn the car around (less if no angle or not steering hard)
				car->_accelCmd = MIN(StuckAccel, MAX(MAX(0.2f, car->_accelCmd), 1.0f - MAX(MAX(0.0f, 0.8-fabs(angle)), 1.0-fabs(car->_steerCmd))));
			}
			if (fabs(angle) - fabs(car->_yaw_rate) < 1.0)
 				car->_accelCmd = MIN(car->_accelCmd, MAX(0.2f, car->_accelCmd - fabs(car->_yaw_rate)));
			if (mode == mode_normal)
				car->_steerCmd -= car->_yaw_rate / 10;
			car->_brakeCmd = 0.0f;
		}
		car->_clutchCmd = getClutch();
		if (DebugMsg & debug_steer)
			fprintf(stderr,"%s %d/%d: ",car->_name,rldata->thisdiv,rldata->nextdiv);

	}

	if (DebugMsg & debug_steer)
	{
		double skid = (car->_skid[0]+car->_skid[1]+car->_skid[2]+car->_skid[3])/2;
		fprintf(stderr,"%d%c%c%c s%.2f k%.2f ss%.2f cl%.3f g%d->%d brk%.3f acc%.2f dec%.2f coll%.1f",mode,((mode==mode_avoiding)?'A':' '),(avoidmode==avoidleft?'L':(avoidmode==avoidright?'R':' ')),(mode==mode_correcting?'c':' '),car->_steerCmd,rldata->ksteer,stucksteer,correctlimit,car->_gear,car->_gearCmd,car->_brakeCmd,car->_accelCmd,rldata->decel,collision);
		fprintf(stderr," spd%.1f|k%.1f|a%.1f/%.1f|t%.1f angle=%.2f/%.2f/%.2f yr=%.2f skid=%.2f acxy=%.2f/%.2f nCRi=%.3f/%.3f\n",(double)getSpeed(),(double)rldata->speed,(double)rldata->avspeed,(double)rldata->slowavspeed,(double)getTrueSpeed(),angle,speedangle,rldata->rlangle,car->_yaw_rate,skid,car->_accel_x,car->_accel_y,nextCRinverse,rldata->mInverse);fflush(stderr);
	}

	laststeer = car->_steerCmd;
	lastbrake = car->_brakeCmd;
	lastaccel = car->_accelCmd;
	lastmode = mode;
	prevleft = car->_trkPos.toLeft;
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

	if (!(avoidmode & avoidback))
		return accel;

	for (i = 0; i < opponents->getNOpponents(); i++) {
		if ((opponent[i].getState() & OPP_LETPASS) &&
		    fabs(car->_trkPos.toLeft - opponent[i].getCarPtr()->_trkPos.toLeft) > 5.0)
		{
			return accel*0.6f;
		}
	}
	return accel;
}

// slows us down to allow a team-member to catch up
double Driver::filterTeam(double accel)
{
 if (mode != mode_normal) return accel;

 double minaccel = accel;
 double closest = -10000.0;
 int i;

 // first filter for following closely
#if 0
 for (i = 0; i < opponents->getNOpponents(); i++)
 {
  if (opponent[i].getCarPtr() == car) continue;
  if ((opponent[i].getTeam() != TEAM_FRIEND) || (opponent[i].getCarPtr()->_msgColorCmd[0] == 1.0)) 
   continue;

  if (opponent[i].getDistance() > 3.0 || opponent[i].getDistance() < 0.0)
   break;

  minaccel = accel = MIN(accel, opponent[i].getCarPtr()->_accelCmd*0.9);
  car->_brakeCmd = MAX(car->_brakeCmd, opponent[i].getCarPtr()->_brakeCmd*0.7);
 }
#endif
 
 if (NoTeamWaiting) return accel;

 // now filter to wait for catching up
 for (i = 0; i < opponents->getNOpponents(); i++)
 {
  if (opponent[i].getCarPtr() == car) continue;
  if (opponent[i].getTeam() & TEAM_FRIEND) continue;

  if (opponent[i].getDistance() < 0.0 && opponent[i].getDistance() > closest)
   closest = opponent[i].getDistance();

  if (opponent[i].getCarPtr()->_pos < car->_pos)
  {
   if (opponent[i].getDistance() < -150.0)
    return accel;
  }

  if (opponent[i].getCarPtr()->_pos >= car->_pos + 2 && 
      opponent[i].getCarPtr()->_laps == car->_laps &&
      opponent[i].getDistance() > -(car->_speed_x*2) &&
      opponent[i].getDistance() < 0.0)
  {
   return accel;
  }
 }

 for (i = 0; i < opponents->getNOpponents(); i++) 
 {
  if (opponent[i].getCarPtr()->_state == RM_CAR_STATE_PIT 
      || opponent[i].getCarPtr()->_state == RM_CAR_STATE_PULLUP
      || opponent[i].getCarPtr()->_state == RM_CAR_STATE_PULLDN
      || opponent[i].getCarPtr()->_state == RM_CAR_STATE_OUT) 
   continue;

  if (opponent[i].getCarPtr() == car) continue;
  if (!(opponent[i].getTeam() & TEAM_FRIEND))
   continue;
  if (opponent[i].getDistance() > -25.0)
   continue;

  double time_behind = fabs(opponent[i].getDistance()) / opponent[i].getCarPtr()->_speed_x;

  if ((opponent[i].getTeam() & TEAM_FRIEND) && 
      opponent[i].getCarPtr()->_laps >= car->_laps &&
      opponent[i].getCarPtr()->_dammage < car->_dammage + 2000 &&
      ((time_behind <= TeamWaitTime && time_behind > 0.4) ||
       (opponent[i].getDistance() < 0.0 && opponent[i].getDistance() > -(car->_speed_x * TeamWaitTime))) &&
      opponent[i].getDistance() > closest &&
      opponent[i].getDistance() < -25.0)
  {
   return MIN(accel, 0.9);
  }
 }


 return minaccel;
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
  car->_gearCmd = car->_gear;
	if (car->_gear <= 0) {
		return 1;
	}
#if 1
	// Hymie gear changing
	float speed = getSpeed();
	float *tRatio = car->_gearRatio + car->_gearOffset;
	float rpm = (float) ((speed + 0.5) * tRatio[car->_gear] / car->_wheelRadius(2));
	float down_rpm = (float) (car->_gear > 1 ? (speed + 0.5) * tRatio[car->_gear-1] / car->_wheelRadius(2) : rpm);

	if (rpm + MAX(0.0, (double) (car->_gear-3) * (car->_gear-3)*5) > car->_enginerpmMax * RevsChangeUp && car->_gear < MaxGear)
		car->_gearCmd = car->_gear + 1;

	if (car->_gear > 1 &&
			rpm < car->_enginerpmMax * RevsChangeDown &&
			down_rpm < car->_enginerpmMax * RevsChangeDownMax)
		car->_gearCmd = car->_gear - 1;
#else
	// BT gear changing
	float gr_up = car->_gearRatio[car->_gear + car->_gearOffset];
	float omega = car->_enginerpmRedLine/gr_up;
	float wr = car->_wheelRadius(2);

	if (omega*wr*SHIFT < car->_speed_x) {
		car->_gearCmd = car->_gear + 1;
	} else {
		float gr_down = car->_gearRatio[car->_gear + car->_gearOffset - 1];
		omega = car->_enginerpmRedLine/gr_down;
		if (car->_gear > 1 && omega*wr*SHIFT > car->_speed_x + SHIFT_MARGIN) {
			car->_gearCmd = car->_gear - 1;
		}
	}
#endif
	
	return car->_gearCmd;
}


void Driver::setMode( int newmode )
{
	if (mode == newmode)
		return;

	if (mode == mode_normal || mode == mode_pitting)
	{
		correcttimer = simtime + 7.0;
		//correctlimit = 1000.0;
	}

	if (newmode == mode_avoiding && mode != mode_avoiding)
		avoidtime = simtime;

	mode = newmode;
}

void Driver::calcSkill()
{
#ifdef CONTROL_SKILL
  //if (RM_TYPE_PRACTICE != racetype)
  {
   if (speed_adjust_timer == -1.0 || simtime - speed_adjust_timer > speed_adjust_limit)
   {
    double rand1 = (double) getRandom() / 65536.0;  // how long we'll change speed for
    double rand2 = (double) getRandom() / 65536.0;  // the actual speed change
    double rand3 = (double) getRandom() / 65536.0;  // whether change is positive or negative

    speed_adjust_limit = 5.0 + rand1 * 20;
    speed_adjust_timer = simtime;
    speed_adjust_targ = (skill/40.0 + (rand2 * (skill / 60.0)))/3;

    if (rand3 < 0.04)
     speed_adjust_targ = -(speed_adjust_targ)/5;
   }

   if (speed_adjust_perc < speed_adjust_targ)
    speed_adjust_perc += MIN(deltaTime/4, speed_adjust_targ - speed_adjust_perc);
   else
    speed_adjust_perc -= MIN(deltaTime/4, speed_adjust_perc - speed_adjust_targ);
  }
#endif
}

double Driver::getFollowDistance()
{
	double mindist = 1000.0;

	if (mode != mode_normal)
		return mindist;

	for (int i = 0; i < opponents->getNOpponents(); i++)
	{
		if (opponent[i].getCarPtr() == car) continue;
		if (opponent[i].getTeam() != TEAM_FRIEND) continue;
		if (!(opponent[i].getState() & OPP_FRONT))
  			continue;

		if (opponent[i].getDistance() > 5.0)
			continue;

		mindist = MIN(mindist, opponent[i].getDistance()) - FollowMargin;
	}
	return mindist;
}

// Compute steer value.
float Driver::getSteer(tSituation *s)
{
	double targetAngle;
	memset(rldata, 0, sizeof(LRaceLineData));
	rldata->angle = angle;
	rldata->speedangle = speedangle;
	rldata->mode = mode;
	rldata->avoidmode = avoidmode;
	rldata->collision = collision;
	rldata->steer = rldata->laststeer = laststeer;
	rldata->alone = alone;
	rldata->followdist = getFollowDistance();
	raceline->GetRaceLineData( s, rldata );
	if (FuelSpeedUp)
	{
		double fuel = (car->_fuel/maxfuel);
		fuel = MIN(1.0, fuel * (fuel+0.15));
		rldata->speed += FuelSpeedUp * (1.0 - fuel);
	}
#ifdef CONTROL_SKILL
	calcSkill();
	if (getTrueSpeed() < rldata->avspeed)
	{
		if ((rldata->offset > car->_trkPos.toMiddle && rldata->rInverse > 0.0) ||
		    (rldata->offset < car->_trkPos.toMiddle && rldata->rInverse < 0.0))
			truespeed = rldata->avspeed;
	}
	double newspeed = rldata->speed - rldata->speed * speed_adjust_perc;
	if (newspeed < rldata->speed && rldata->speed < getSpeed())
		newspeed = MAX(MIN(rldata->speed, getSpeed()-0.7), newspeed);
	rldata->speed = newspeed;
#endif
	double steer = 0.0, tmpsteer = 0.0;
	double avoidsteer = 0.0;
	double racesteer = (rldata->ksteer);
	vec2f	target;
	if (SteerLookahead > 6.0f)
	{
		raceline->GetSteerPoint( (double) SteerLookahead, &target );
		targetAngle = atan2(target.y - car->_pos_Y, target.x - car->_pos_X);
		racesteer = calcSteer( targetAngle, 0 );
		double rIf = MIN(1.0, fabs(rldata->rInverse) / SteerMaxRI);
		racesteer = racesteer + (rldata->ksteer - racesteer) * rIf;
	}

	target = getTargetPoint();
	steer = avoidsteer = racesteer;
	lastNSksteer = rldata->NSsteer;

	if (mode != mode_normal || SteerLookahead < 6.0f)
	{
		targetAngle = atan2(target.y - car->_pos_Y, target.x - car->_pos_X);
		tmpsteer = calcSteer( targetAngle, 0 );
	}


	if (mode != mode_normal)
	{
		avoidsteer = tmpsteer;

		if (mode == mode_pitting)
		{
			correctlimit = (avoidsteer - racesteer);
			return avoidsteer;
		}

		//targetAngle = atan2(rldata->target.y - car->_pos_Y, rldata->target.x - car->_pos_X);
		//float racesteer = calcSteer( targetAngle, 1 );

		allowcorrecting = 0;
		if (mode == mode_avoiding && 
				(!avoidmode ||
				 (avoidmode == avoidright && racesteer > avoidsteer) ||
				 (avoidmode == avoidleft && racesteer < avoidsteer)))
		{
			// we're avoiding, but trying to steer somewhere the raceline takes us.
			// hence we'll just correct towards the raceline instead.
			allowcorrecting = 1;
		}

		bool yr_ok = (fabs(car->_yaw_rate) < 0.1 || (car->_yaw_rate > rldata->rInverse*100-0.1 && car->_yaw_rate < rldata->rInverse*100+0.1));
		bool angle_ok = (angle > rldata->rlangle-0.06 && angle < rldata->rlangle+0.06);
		double rlsteer = (fabs(rldata->rInverse) >= 0.004 ? rldata->rInverse * 12 : 0.0);
		bool steer_ok = (racesteer<laststeer+0.05 && racesteer>laststeer-0.05);

		double skid = (car->_skid[0] + car->_skid[1] + car->_skid[2] + car->_skid[3]) / 2;
		if (mode == mode_correcting)
		{
			if (lastmode == mode_normal ||
			    (angle_ok &&
			     yr_ok &&
			     skid < 0.1 &&
			     steer_ok &&
			     ((fabs(car->_trkPos.toMiddle) < car->_trkPos.seg->width/2 - 1.0) || car->_speed_x < 10.0) &&
			     (raceline->isOnLine())))
			{
				// we're correcting & are now close enough to the raceline to
				// switch back to 'normal' mode...
				setMode( mode_normal );
				aligned_timer = simtime;
				steer = racesteer;
				if (DebugMsg & debug_steer)
					fprintf(stderr,"ALIGNED steer_ok=%d avsteer=%.3f racest=%.3f\n",steer_ok,avoidsteer,racesteer);
			}
			else if (DebugMsg & debug_steer)
				fprintf(stderr,"NOT ALIGNED %d %d %d %d %.2f %.2f %.2f\n",angle_ok,yr_ok,(skid<0.1),steer_ok,avoidsteer,racesteer,laststeer);
		}

		if (mode != mode_normal)
		{
			if (mode != mode_correcting && !allowcorrecting)
			{
				int flying = checkFlying();
				if (flying & FLYING_FRONT)
				{
					steer = 0.0;
				}
				else if (flying & FLYING_BACK)
				{
					steer /= 3.0;
				}
				else
				{
					//correctlimit = 1000.0;
					correcttimer = simtime + 7.0;
					steer = ( avoidsteer );
				}
				double climit = (steer - racesteer);
				if (fabs(climit) > fabs(correctlimit))
					correctlimit = climit;
			}
			else
			{
				steer = (correctSteering( avoidsteer, racesteer ));
				correctlimit = (steer - racesteer);
			}
	
			if (fabs(angle) >= 1.6)
			{
				if (steer > 0.0)
					steer = 1.0;
				else
					steer = -1.0;
			}
		}
		else
			correctlimit = (steer - racesteer);
	}
	else
	{
		raceline->NoAvoidSteer();
		lastNSasteer = rldata->NSsteer;
		correctlimit = (steer - racesteer);
	}


	if (mode == mode_avoiding && 
	    (lastmode == mode_normal || lastmode == mode_correcting) && 
	    !((avoidmode & avoidright) && (avoidmode & avoidleft)))
	{
		// if we're only avoiding on one side, and racesteer avoids more than avoidsteer, and just prior we
		// weren't avoiding, return to prior mode.
		if ((racesteer >= steer && avoidmode == avoidright) ||
		    (racesteer <= steer && avoidmode == avoidleft))
		{
			if (lastmode == mode_normal)
				steer = racesteer;
			setMode(lastmode);
		}
	}

	return steer;
}


double Driver::calcSteer( double targetAngle, int rl )
{
#if 0
	if (mode != mode_pitting)
	{
		float kksteer = raceline->getAvoidSteer(myoffset, rldata);
		return kksteer;
	}
#endif

	double rearskid = MAX(0.0, MAX(car->_skid[2], car->_skid[3]) - MAX(car->_skid[0], car->_skid[1])) + MAX(car->_skid[2], car->_skid[3]) * fabs(angle)*0.9;
	double steer = 0.0;

#if 0  // olethros steering
	double steer_direction = targetAngle - car->_yaw - 0.1 * car->_yaw_rate;

	double avoidance = 0.0;
	if (!(pit->getInPit()))
	{
		if (car->_trkPos.toRight < car->_dimension_y)
			avoidance = tanh(0.2 * (car->_dimension_y - car->_trkPos.toRight));
		else if (car->_trkPos.toLeft < car->_dimension_y)
			avoidance = tanh(0.2 * (car->_trkPos.toLeft - car->_dimension_y));
	}

	tTrackSeg *seg = car->_trkPos.seg;
	double correct_drift = -0.01 * atan2(car->_speed_Y, car->_speed_X);
	NORM_PI_PI(steer_direction);

	steer = avoidance + correct_drift + steer_direction/car->_steerLock;


#else  // old usr steering
	double steer_direction = targetAngle - car->_yaw;

	NORM_PI_PI(steer_direction);
	if (DebugMsg & debug_steer)
		fprintf(stderr,"STEER tm%.2f off%.2f sd%.3f",car->_trkPos.toMiddle,myoffset,steer_direction);

	if (car->_speed_x > 10.0 && mode != mode_normal && mode != mode_pitting)
	{
		double limit = MAX(20.0, 90.0 - car->_speed_x) * (((avoidmode & avoidside) ? 0.0074 : 0.0045) * SmoothSteer);
		double rgtlimit = limit, lftlimit = limit;
		if (laststeer_direction > 0.0) rgtlimit = MIN(laststeer_direction, rgtlimit*2);
		if (laststeer_direction < 0.0) lftlimit = MIN(-laststeer_direction, lftlimit*2);

		steer_direction = MAX(laststeer_direction - rgtlimit, MIN(laststeer_direction + lftlimit, steer_direction));

#if 1
		double speedsteer = (80.0 - MIN(70.0, MAX(40.0, getSpeed()))) / 
		                     ((185.0 * MIN(1.0, car->_steerLock / 0.785)) +
		                     (185.0 * (MIN(1.3, MAX(1.0, 1.0 + rearskid))) - 185.0));
		if (fabs(steer_direction) > speedsteer)
		{
			steer_direction = MAX(-speedsteer, MIN(speedsteer, steer_direction));
		}
#endif
	}
	laststeer_direction = steer_direction;

	steer = (steer_direction/car->_steerLock);
	if (DebugMsg & debug_steer)
		fprintf(stderr,"/sd%.3f a%.3f",steer_direction,steer);

	if (rldata->rgtmargin)
		steer = MAX(steer, rldata->ksteer - rldata->rgtmargin);
	if (rldata->lftmargin)
		steer = MIN(steer, rldata->ksteer + rldata->lftmargin);
	if (DebugMsg & debug_steer)
		fprintf(stderr," b%.3f",steer);

	lastNSasteer = steer;

	if (fabs(angle) > fabs(speedangle))
	{
		// steer into the skid
		double sa = MAX(-0.3, MIN(0.3, speedangle/3));
		//double anglediff = (sa - angle) * (0.3 + fabs(angle)/6);
		double anglediff = (speedangle - angle) * (0.1 + fabs(angle)/6);
		steer += (float) (anglediff*0.5);
	}

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
#endif  // old usr steering

	if (DebugMsg & debug_steer)
		fprintf(stderr," d%.3f",steer);

	if (mode != mode_pitting)
   	{
	   // limit how far we can steer against last steer...
	   double limit = 0.0;
	   limit = ((90.0 - MAX(40.0, MIN(60.0, car->_speed_x))) / (120)) * SmoothSteer; // (130);
	   if (fabs(laststeer) > fabs(steer))
		   limit = MAX(limit, fabs(laststeer)/2);
	   steer = MAX(laststeer-limit, MIN(laststeer+limit, steer));
	   //steer = MAX(rldata->steer-limit, MIN(rldata->steer+limit, steer));

#if 1
	   if (simtime > 3.0)
	   {
	      // and against raceline...
		double climit = fabs(correctlimit);
#if 0
		if (climit > 0.0 && angle - rldata->rlangle > 0.0)
			climit += (angle - rldata->rlangle)/2;
		else if (climit < 0.0 && angle - rldata->rlangle < 0.0)
			climit -= fabs(angle - rldata->rlangle)/2;
#endif

		double limitchange = ((90.0 - MAX(40.0, MIN(60.0, car->_speed_x))) / ((avoidmode & avoidside) ? 130 : 200)) * 3;
		climit += limitchange;
   		steer = MAX(rldata->ksteer-climit, MIN(rldata->ksteer+climit, steer));
	   }
#endif
	   steer = smoothSteering(steer);
   	}
#if 0
	else if (getSpeed() > MAX(25.0, pit->getSpeedlimit()))
	{
		// stop sudden steer changes while pitting
		double limit = MAX(0.1, (40 - fabs(getSpeed() - pit->getSpeedlimit())) * 0.025);
		steer = MIN(limit, MAX(-limit, steer));
	}
#endif

	if (DebugMsg & debug_steer)
		fprintf(stderr," e%.3f\n",steer);

	return steer;
}

int Driver::checkFlying()
{
 int i = 0;
 if (car->_speed_x < 20)
  return 0;

 if (car->priv.wheel[0].relPos.z < wheelz[0] &&
     car->priv.wheel[1].relPos.z < wheelz[1])
 {
  i += FLYING_FRONT;
 }
 if (car->priv.wheel[2].relPos.z < wheelz[2]-0.05 &&
     car->priv.wheel[3].relPos.z < wheelz[3]-0.05)
 {
  i += FLYING_BACK;
 }
 if (!i)
 {
  if ((car->priv.wheel[0].relPos.z < wheelz[0] &&
       car->priv.wheel[2].relPos.z < wheelz[2] - 0.05) ||
      (car->priv.wheel[1].relPos.z < wheelz[1] &&
       car->priv.wheel[3].relPos.z < wheelz[3] - 0.05))
  {
   i = FLYING_SIDE;
  }
 }

 return i;
}

float Driver::correctSteering( float avoidsteer, float racesteer )
{
	float steer = avoidsteer;
	float accel = MIN(0.0f, car->_accel_x);
	double speed = 50.0; //MAX(50.0, getSpeed());
	double changelimit = MIN(raceline->correctLimit(avoidsteer, racesteer), (((120.0-getSpeed())/6000) * (0.1 + fabs(rldata->mInverse/4)))) * SmoothSteer;

if (DebugMsg & debug_steer)
fprintf(stderr,"CORRECT: cl=%.3f as=%.3f rs=%.3f NS=%.3f",correctlimit,avoidsteer,racesteer,lastNSasteer);
	if (/*mode == mode_correcting &&*/ simtime > 2.0f)
	{
		// move steering towards racesteer...
		if (correctlimit < 900.0)
		{
			if (steer < racesteer)
			{
				if (correctlimit >= 0.0)
				{
					//steer = (float) MIN(racesteer, steer + correctlimit);
if (DebugMsg & debug_steer) fprintf(stderr," RA%.3f",racesteer);
					steer = racesteer;
					lastNSasteer = rldata->NSsteer;
				}
				else
				{
					steer = (float) MIN(racesteer, MAX(steer, racesteer + correctlimit));
					lastNSasteer = (float) MIN(rldata->NSsteer, MAX(lastNSasteer, rldata->NSsteer + correctlimit));
if (DebugMsg & debug_steer) fprintf(stderr," MA%.3f",steer);
				}
			}
			else
			{
				if (correctlimit <= 0.0)
				{
					//steer = (float) MAX(racesteer, steer-correctlimit);
					steer = racesteer;
					lastNSasteer = rldata->NSsteer;
if (DebugMsg & debug_steer) fprintf(stderr," RB%.3f",racesteer);
				}
				else
				{
					steer = (float) MAX(racesteer, MIN(steer, racesteer + correctlimit));
					lastNSasteer = (float) MAX(rldata->NSsteer, MIN(lastNSasteer, rldata->NSsteer + correctlimit));
if (DebugMsg & debug_steer) fprintf(stderr," MB%.3f",steer);
				}
			}
		}
		//else
		{
			speed -= avgaccel_x/10;
			speed = MAX(55.0, MIN(150.0, speed + (speed*speed/55)));
			double rInverse = rldata->mInverse * (avgaccel_x<0.0 ? 1.0 + fabs(avgaccel_x)/10.0 : 1.0);
			double correctspeed = 0.5;
			if ((rInverse > 0.0 && racesteer > steer) || (rInverse < 0.0 && racesteer < steer))
				correctspeed += rInverse*110;

			if (racesteer > steer)
				//steer = (float) MIN(racesteer, steer + (((155.0-speed)/10000) * correctspeed));
				steer = (float) MIN(racesteer, steer + changelimit);
			else
				//steer = (float) MAX(racesteer, steer - (((155.0-speed)/10000) * correctspeed));
				steer = (float) MAX(racesteer, steer - changelimit);
			if (fabs(racesteer) < fabs(steer))
			{
				if (racesteer > steer)
					steer += (fabs(steer) - fabs(racesteer)) / 2;
				else
					steer -= (fabs(steer) - fabs(racesteer)) / 2;
			}

			if (lastNSksteer > lastNSasteer)
				//lastNSasteer = (float) MIN(rldata->NSsteer, lastNSasteer + (((155.0-speed)/10000) * correctspeed));
				lastNSasteer = (float) MIN(rldata->NSsteer, lastNSasteer + changelimit);
			else
				//lastNSasteer = (float) MAX(rldata->NSsteer, lastNSasteer - (((155.0-speed)/10000) * correctspeed));
				lastNSasteer = (float) MAX(rldata->NSsteer, lastNSasteer - changelimit);
if (DebugMsg & debug_steer) fprintf(stderr," I%.3f",steer);
		}
	
	}

if (DebugMsg & debug_steer) fprintf(stderr," %.3f NS=%.3f\n",steer,lastNSasteer);

	return steer;
}


float Driver::smoothSteering( float steercmd )
{
	if (pitoffset != -100.0f)
		return steercmd;
#if 1
	// experimental smoothing code, beware!!!
	double steer = steercmd;
	double stdelta = steer - laststeer;//car->_steerCmd;
	double maxSpeed = MAX(200.0, 300.0 - car->_speed_x*2) * (PI/180.0);

	//if (mode == mode_normal)
	//	maxSpeed = 200.0 * (PI / 180.0);

	if ((fabs(stdelta) / deltaTime) > maxSpeed)
		steer = SIGN(stdelta) * maxSpeed * deltaTime + laststeer;//car->_steerCmd;

	steercmd = steer;

	// limit amount of steer according to speed & raceline
	double a_error_factor = ((rldata->exiting && rldata->outsideline) ? 0.9 : 0.8);
	double smangle = angle * (0.5 + fabs(angle*2));
	double angle_error = (smangle - rldata->rlangle/2) * a_error_factor;
	double lstlimit = MAX(40.0, 80.0 - car->_speed_x) * 0.004 - MIN(0.0, MAX(-0.5, angle_error));
	double rstlimit = -(MAX(40.0, 80.0 - car->_speed_x) * 0.004 + MAX(0.0, MIN(0.5, angle_error)));
	double strate = 61.0 + lastaccel*10;

	if (rldata->rInverse*strate > lstlimit)
		lstlimit = rldata->rInverse*strate;
	if (rldata->rInverse*strate < rstlimit)
		rstlimit = rldata->rInverse*strate;
	steercmd = MAX(rstlimit, MIN(lstlimit, steercmd));
#endif

	return steercmd;
#if 0
	// try to limit sudden changes in steering to avoid loss of control through oversteer. 
	double lftspeedfactor = ((((60.0 - (MAX(40.0, MIN(70.0, getSpeed() + MAX(0.0, car->_accel_x*5))) - 25)) / 300) * 2.5) / 0.585) * SmoothSteer;
	double rgtspeedfactor = lftspeedfactor;

	if (fabs(steercmd) < fabs(laststeer) && fabs(steercmd) <= fabs(laststeer - steercmd))
	{
		lftspeedfactor *= 2;
		rgtspeedfactor *= 2;
	}

	lftspeedfactor -= MIN(0.0f, car->_yaw_rate/10);
	rgtspeedfactor += MAX(0.0f, car->_yaw_rate/10);

	steercmd = (float) MAX(laststeer - rgtspeedfactor, MIN(laststeer + lftspeedfactor, steercmd));
	return steercmd;
#endif
}

// Compute the clutch value.
float Driver::getClutch()
{
	if (1 || car->_gearCmd > 1) 
	{
		float maxtime = MAX(0.06f, 0.32f - ((float) car->_gearCmd / 65.0f));
		if (car->_gear != car->_gearCmd && car->_gearCmd < MaxGear)
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

// move offset to allow for bends (if any)
float Driver::adjustOffset(float offset)
{
return offset;
	float adjustment = (float) (rldata->rInverse * 10);
	float width = (float) (car->_trkPos.seg->width * 0.75);

#if 0
#endif

	//if (mode==mode_avoiding)
	{
		// we want to adjust outwards a bit if in close to the corner (more if avoiding
		// a car on the inside, way less otherwise).  If the car is on the outside third
		// of the track we don't want to adjust at all.  Inbetween the inside third and 
		// the outside third we want a gradual decline in the adjust amount...
		if (adjustment < 0.0)
		{
			if (car->_trkPos.toRight > width*0.7)
				adjustment = 0.0;
			else if (car->_trkPos.toRight > width*0.3)
				adjustment *= MAX(0.0, 1.0 - ((car->_trkPos.toRight-width*0.3)/width*0.4));
		}
		else if (adjustment > 0.0)
		{
			if (car->_trkPos.toLeft > width*0.7)
				adjustment = 0.0;
			else if (car->_trkPos.toLeft > width*0.3)
				adjustment *= MAX(0.0, 1.0 - ((car->_trkPos.toLeft-width*0.3)/width*0.4));
		}
#if 0
		if (adjustment < 0.0 && car->_trkPos.toLeft < width)
			adjustment *= MAX(0.1, MIN(1.0, car->_trkPos.toLeft*1.6 / width));
		else if (adjustment > 0.0 && car->_trkPos.toRight < width)
			adjustment *= MAX(0.1, MIN(1.0, car->_trkPos.toRight*1.6 / width));
#endif

		//adjustment *= 1.0 + MAX(0.0, getSpeed() / rldata->avspeed)*2;
		if ((avoidmode == avoidright && adjustment < 0.0) ||
				(avoidmode == avoidleft && adjustment > 0.0))
			adjustment *= 1;
		else
			adjustment /= 2;
	}

	double speed = getSpeed();
	double xspeed = MIN(rldata->speed, rldata->avspeed);
	if (speed < xspeed)
		adjustment *= MIN(2.0, xspeed / speed);

	offset -= adjustment;

	return offset;
}

// Compute target point for steering.
vec2f Driver::getTargetPoint()
{
	tTrackSeg *seg = car->_trkPos.seg;
	float length = getDistToSegEnd();
	float offset = getOffset();
	pitoffset = -100.0f;

	if (pit->getInPit()) {
		// To stop in the pit we need special lookahead values.
		if (currentspeedsqr > pit->getSpeedlimitSqr()) {
			lookahead = PIT_LOOKAHEAD + car->_speed_x*LOOKAHEAD_FACTOR;
		} else {
			lookahead = PIT_LOOKAHEAD;
		}
	} else {
		// Usual lookahead.
		lookahead = (float) rldata->lookahead;

#if 0
	  if ((mode == mode_avoiding || mode == mode_correcting) && fabs(rldata->rInverse) > 0.002 &&
				((rldata->rInverse > 0.0 && car->_trkPos.toLeft <= 1.0) ||
				 (rldata->rInverse < 0.0 && car->_trkPos.toRight <= 1.0)))
		{
			// close in on a corner while we're not on raceline - decrease lookahead
			// to prevent car from cutting the corner too close.
			lookahead *= MAX(0.5, 1.0 - fabs(rldata->rInverse)*40);
		}
#endif

#if 1
		double speed = MAX(20.0, getSpeed());// + MAX(0.0, car->_accel_x));
		lookahead = (float) (LOOKAHEAD_CONST * 1.2 + speed * 0.60);
		lookahead = MIN(lookahead, (float) (LOOKAHEAD_CONST + ((speed*(speed/7)) * 0.15)));
#if 1
		if (fabs(rldata->rInverse) > 0.001)
		{
			// increase lookahead if on the outside of a corner
			double cornerfx = 1.0;
			double cornerlmt = MIN(track->width/2, 1.0 + fabs(rldata->rInverse)*500);
			if (rldata->rInverse > 0.0 && car->_trkPos.toRight < cornerlmt)
				cornerfx = 1.0 + MAX(0.0, (cornerlmt-car->_trkPos.toRight)/cornerlmt)*1.5;
			else if (rldata->rInverse < 0.0 && car->_trkPos.toLeft < cornerlmt)
				cornerfx = (1.0 - MAX(0.0, (cornerlmt-car->_trkPos.toLeft)/cornerlmt))*1.5;
			lookahead *= cornerfx;
		}
#endif
		//lookahead = MAX(lookahead, LOOKAHEAD_CONST + ((speed*(speed/3)) * (0.4*cornerfx)));
#endif
#if 0
		lookahead = LOOKAHEAD_CONST + car->_speed_x*LOOKAHEAD_FACTOR;
		lookahead = MAX(lookahead, LOOKAHEAD_CONST + ((car->_speed_x*(car->_speed_x/2)) / 60.0));
#endif

#if 1
		lookahead *= LookAhead;

		// Prevent "snap back" of lookahead on harsh braking.
		float cmplookahead = oldlookahead - (car->_speed_x*RCM_MAX_DT_ROBOTS)*0.55f;//0.40f;
		if (lookahead < cmplookahead) {
			lookahead = cmplookahead;
		}
#endif
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
	pitoffset = pit->getPitOffset(pitoffset, fromstart, pitpos);
	if ((pit->getPitstop() || pit->getInPit()) && pitoffset != -100.0f)
	{
		setMode(mode_pitting);
		offset = myoffset = pitoffset;
	}
	else if (mode == mode_pitting)
	{
		setMode(mode_correcting);
	}

	//if (mode == mode_correcting || mode == mode_avoiding)
	//	offset = adjustOffset( offset );

	vec2f s;
	//if (mode != mode_pitting)
	{
		raceline->GetPoint( offset, &s, NULL );
		return s;
	}

	// all the BT code below is for steering into pits only.
	s.x = (seg->vertex[TR_SL].x + seg->vertex[TR_SR].x)/2;
	s.y = (seg->vertex[TR_SL].y + seg->vertex[TR_SR].y)/2;
	double dx, dy;
	vec2f t, rt;

	if ( seg->type == TR_STR) {
		vec2f d, n;
		n.x = (seg->vertex[TR_EL].x - seg->vertex[TR_ER].x)/seg->length;
		n.y = (seg->vertex[TR_EL].y - seg->vertex[TR_ER].y)/seg->length;
		n.normalize();
		d.x = (seg->vertex[TR_EL].x - seg->vertex[TR_SL].x)/seg->length;
		d.y = (seg->vertex[TR_EL].y - seg->vertex[TR_SL].y)/seg->length;
		t = s + d*length + offset*n;

		return t;
	} else {
		vec2f c, n;
		c.x = seg->center.x;
		c.y = seg->center.y;
		float arc = length/seg->radius;
		float arcsign = (seg->type == TR_RGT) ? -1.0f : 1.0f;
		arc = arc*arcsign;
		s = s.rotate(c, arc);
	
		n = c - s;
		n.normalize();
		t = s + arcsign*offset*n;

		return t;
	}
}


bool Driver::canOvertake( Opponent *o, double *mincatchdist, bool outside, bool lenient )
{
	if (!o) return false;

#if 1
	int segid = car->_trkPos.seg->id;
	tCarElt *ocar = o->getCarPtr();
	int osegid = ocar->_trkPos.seg->id;
	double otry_factor = (lenient ? (0.2 + (1.0 - ((simtime-frontavoidtime)/7.0)) * 0.8) : 1.0);
	double overtakecaution = rldata->overtakecaution + (outside ? MIN(0.0, car->_accel_x/8) : 0.0);
	double distance = o->getDistance() * otry_factor * (1.0 + overtakecaution) - (ocar->_pos > car->_pos ? MIN(o->getDistance()/2, 3.0) : 0.0);
	double speed = MIN(rldata->avspeed, getSpeed() + MAX(0.0, (10.0 - distance)/3));
	double ospeed = o->getSpeed();
	if (outside)
		ospeed *= 1.0 + fabs(rldata->rInverse)*3;
	double catchdist = (double) MIN(speed*distance/(speed-ospeed), distance*CATCH_FACTOR) * otry_factor;

	if (catchdist < *mincatchdist+0.1 && distance < fabs(speed-ospeed)*2)
	{
		if (DebugMsg & debug_overtake)
				fprintf(stderr,"%.1f %s: OVERTAKE! (cd %.1f<%.1f) (dist %.1f < (%.1f-%.1f)*2 = %.1f caut=%.1f\n",otry_factor,ocar->_name,catchdist,*mincatchdist,distance,speed,ospeed,fabs(speed-ospeed)*2,overtakecaution);
		*mincatchdist = catchdist;
		return true;
	}
	else if (DebugMsg & debug_overtake)
		fprintf(stderr,"%.1f %s: FAIL!!!!! (cd %.1f<%.1f) (dist %.1f < (%.1f-%.1f)*2 = %.1f caut=%.1f\n",otry_factor,ocar->_name,catchdist,*mincatchdist,distance,speed,ospeed,fabs(speed-ospeed)*2,overtakecaution);
#else
	double distance = o->getDistance();
	double speed = MIN(rldata->avspeed, getSpeed() + MAX(0.0, (getSpeed()/3 - distance)/2));
	double ospeed = o->getSpeed();
	double caution = overtakecaution + (outside == true ? 0.5 : 0.0);
	//double rInverse = (fabs(nextCRinverse)>fabs(rldata->rInverse) ? nextCRinverse : rldata->rInverse);
	double rInverse = rldata->aInverse; //rldata->rInverse;

	{
		tTrackSeg *wseg = (rInverse > 0.0 ? car->_wheelSeg(FRNT_LFT) : car->_wheelSeg(FRNT_RGT));
		if (wseg->surface->kFriction > car->_trkPos.seg->surface->kFriction)
			caution += (wseg->surface->kFriction - car->_trkPos.seg->surface->kFriction) * 4;
	}

	distance += fabs(rInverse) * 200;
	if (outside)
	{
		distance += fabs(rInverse) * 800;
		if ((rInverse > 0.0 && angle > 0.0) || (rInverse < 0.0 && angle < 0.0))
			caution += fabs(speedangle-angle) * 20;
	}

	double catchdist = MIN(speed*distance/(speed-ospeed), distance*CATCH_FACTOR) * (lenient ? (0.7 + MIN(0.3, (simtime-frontavoidtime)/10.0)) : 1.0) * (1.0 + caution/2);
	
	if (catchdist > *mincatchdist && 
	    *mincatchdist >= MAX(100.0, car->_speed_x*5) &&
	    rldata->avspeed > 1000.0 && getSpeed() > ospeed - (lenient ? 3.0 : 1.5))
	{
		// on a fast section - don't let catchdist be a barrier to overtaking
		catchdist = *mincatchdist * 0.9;
	}

	if ((catchdist <= *mincatchdist) && distance < (speed-ospeed)*2)
	{
		double mradius = MAX(0.0, MIN(1.0, fabs(rInverse) * 70));
		double oradius = MAX(0.0, MIN(1.0, fabs(raceline->getRInverse(distance)) * 80));
		double radius = MAX(1.0, MIN(10.0, MAX(1.0 / mradius, 1.0 / oradius)));
		catchdist *= radius;
		double distance2 = distance * radius;
			
		if (catchdist <= *mincatchdist && distance < fabs(speed-ospeed)*2)
		{
			if (DebugMsg & debug_overtake)
				fprintf(stderr,"%s - %s OVERTAKE: cd=%.1f > %.1f, dist=%.1f > (%.1f-%.1f)*2=%.1f\n",car->_name,o->getCarPtr()->_name,catchdist,*mincatchdist,distance,speed,ospeed,(speed-ospeed)*2);
			*mincatchdist = catchdist+0.01;
			return true;
		}
		else if (DebugMsg & debug_overtake)
			fprintf(stderr,"%s - %s FAIL 2: cd=%.1f > %.1f, dist=%.1f (%.1f/%.1f) > (%.1f-%.1f)*2=%.1f\n",car->_name,o->getCarPtr()->_name,catchdist,*mincatchdist,distance2,distance,o->getDistance(),speed,ospeed,(speed-ospeed)*2);
	}
	else if (DebugMsg & debug_overtake)
		fprintf(stderr,"%s - %s FAIL 1: cd=%.1f > %.1f, dist=%.1f (%.1f) > (%.1f-%.1f)*2=%.1f\n",car->_name,o->getCarPtr()->_name,catchdist,*mincatchdist,distance,o->getDistance(),speed,ospeed,(speed-ospeed)*2);

#endif
	return false;
}

// Compute offset to normal target point for overtaking or let pass an opponent.
float Driver::getOffset()
{
	int i, avoidmovt = 0;
	double catchdist, mincatchdist = MAX(100.0, car->_speed_x * 5), mindist = -1000.0;
	Opponent *o = NULL;
	double lane2left = rldata->lane * car->_trkPos.seg->width;
	double lane2right = car->_trkPos.seg->width-lane2left;
	avoidmode = 0;
	sideratio = 100.0;

	avoidlftoffset = car->_trkPos.seg->width / 2;
	avoidrgtoffset = -car->_trkPos.seg->width / 2;

	// Increment speed dependent.
	//double incspeed = MIN(40.0, MAX(30.0, getSpeed()));
	//double incfactor = (MAX_INC_FACTOR*0.5 - MIN(incspeed/10, MAX_INC_FACTOR*0.5-0.5)) * 60 * IncFactor;
	double incspeed = MIN(60.0, MAX(40.0, getSpeed())) - 10.0;
	double incfactor = (MAX_INC_FACTOR - MIN(fabs(incspeed)/MAX_INC_FACTOR, (MAX_INC_FACTOR - 1.0f))) * (10.0f + MAX(0.0, (CA-1.9)*10));// * MAX(0.4, 1.0 - (fabs(rldata->aInverse)*3));

	//double rgtinc = incfactor * MIN(3.0, MAX(0.6, 1.0 + rldata->mInverse * (rldata->mInverse<0.0?-5:100)));
	//double lftinc = incfactor * MIN(3.0, MAX(0.6, 1.0 - rldata->mInverse * (rldata->mInverse>0.0?-5:100)));
	double rgtinc = incfactor * MIN(3.0, MAX(0.6, 1.0 + (rldata->aInverse < 0.0 ? rldata->aInverse*10 : rldata->aInverse*MAX(10.0, car->_speed_x-25)*3*OutSteerFactor)));
	double lftinc = incfactor * MIN(3.0, MAX(0.6, 1.0 - (rldata->aInverse > 0.0 ? rldata->aInverse*10 : rldata->aInverse*MAX(10.0,car->_speed_x-25)*3*OutSteerFactor)));
	
	//double reduce_movt = MAX(0.01, 1.0 - (MIN(1.0, fabs(laststeer))*2 * fabs(angle-speedangle)*3));
#if 1
	double reduce_movt = MAX(0.1, MIN(1.0, 1.0 - fabs(angle*2 - laststeer)) * MAX(fabs(angle-speedangle), fabs(speedangle-angle))*1);
	lftinc *= reduce_movt;
	rgtinc *= reduce_movt;
#else
	if (rldata->rInverse > 0.0)
		rgtinc *= MAX(0.0, MIN(1.0, 1.0 - (angle*2-car->_yaw_rate/2) * MIN(1.0, rldata->rInverse*50)));
	else if (rldata->rInverse < 0.0)
		lftinc *= MAX(0.0, MIN(1.0, 1.0 + (angle*2-car->_yaw_rate/2) * MIN(1.0, (-rldata->rInverse)*50)));
#endif

	double moffset = car->_trkPos.toMiddle;

	double origoffset = moffset;

	double Width = car->_trkPos.seg->width;
#if 1
	double maxoffset = MAX(moffset-OVERTAKE_OFFSET_INC*rgtinc*2, Width/2 - (car->_dimension_y+SideMargin));
	double minoffset = MIN(moffset+OVERTAKE_OFFSET_INC*lftinc*2, -(Width/2 - (car->_dimension_y+SideMargin)));

	double oldmax = maxoffset, oldmin = minoffset;

	if (rldata->rInverse < 0.0)
	{
		maxoffset = MAX(Width/4, maxoffset - fabs(rldata->rInverse) * (200 - MAX(0.0, MIN(120, (rldata->avspeed-car->_speed_x) * 10))) / 10);
		minoffset = MIN(-(Width/8), minoffset + fabs(rldata->rInverse) * 300);
		tTrackSeg *wseg = car->_wheelSeg(FRNT_RGT);
		if (wseg->surface->kFriction > car->_trkPos.seg->surface->kFriction)
			minoffset += (wseg->surface->kFriction - car->_trkPos.seg->surface->kFriction) * 7;
	}
	else
	{
		maxoffset = MAX(Width/8, maxoffset - fabs(rldata->rInverse) * 300);
		minoffset = MIN(-(Width/4), minoffset + fabs(rldata->rInverse) * (200 - MAX(0.0, MIN(120, (rldata->avspeed-car->_speed_x) * 10))) / 10);
		tTrackSeg *wseg = (car->_wheelSeg(FRNT_LFT));
		if (wseg->surface->kFriction > car->_trkPos.seg->surface->kFriction)
			maxoffset -= (wseg->surface->kFriction - car->_trkPos.seg->surface->kFriction) * 7;
	}
	if (oldmax >= moffset)
		maxoffset = MIN(oldmax, MAX(maxoffset, moffset));
	if (oldmin <= moffset)
		minoffset = MAX(oldmin, MIN(minoffset, moffset));
#else
	double maxoffset = track->width/2 - (car->_dimension_y+SideMargin); // limit to the left
	double minoffset = -(track->width/2 - (car->_dimension_y+SideMargin)); // limit to the right

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
#endif

	
	//myoffset = car->_trkPos.toMiddle;
	if (mode == mode_normal)
	{
		moffset = rldata->offset;
		myoffset = (float) moffset;
	}
	else
	{
		// reduce amount we deviate from the raceline according to how long we've been avoiding, and also
		// how fast we're going.
		double dspeed = MAX(0.0, rldata->speed - getSpeed()) * 4;
		double pspeed = MAX(1.0, 60.0 - (getSpeed() - (30.0 + MAX(0.0, car->_accel_x) + dspeed))) / 10;

		// instead of toMiddle just aiming at where the car currently is, we move it in the direction 
		// the car's travelling ... but less so if the speedangle is different from the car's angle.
		double sa = speedangle;
		//double anglechange = ((sa*0.8) * MAX(0.0, 1.0 - fabs(sa-angle)*(0.6+fabs(sa-angle)))) * 0.7;
		double anglechange = ((sa*1.0) * MAX(0.0, 1.0 - fabs(sa-angle))) * (fabs(getSpeed())/50);
		double toMiddle = car->_trkPos.toMiddle + anglechange*0;
		moffset = toMiddle;
		myoffset = (float) moffset;

		if (mode == mode_correcting && avoidtime < simtime)
			avoidtime += deltaTime*0.8;
		if (simtime - avoidtime > 3.0)
			avoidtime = simtime - 3.0;

		if (0)
		{
			if (toMiddle > rldata->offset)
				moffset = MIN(toMiddle, rldata->offset + (simtime-avoidtime) * 45.0 * incfactor/8);
			else
				moffset = MAX(toMiddle, rldata->offset - (simtime-avoidtime) * 45.0 * incfactor/8);
		}

#if 0
		if (fabs(car->_trkPos.toMiddle) < MAX(fabs(minoffset), fabs(maxoffset)))
		{
			if (anglechange > 0.0)
				minoffset = MIN(maxoffset, MAX(minoffset, car->_trkPos.toMiddle - MAX(0.1, 1.0 - anglechange*2)*2));
			else
				maxoffset = MAX(minoffset, MIN(maxoffset, car->_trkPos.toMiddle + MAX(0.1, 1.0 - fabs(anglechange*2))*2));
		}
#endif
	}

	{
		minoffset = MAX(minoffset, car->_trkPos.toMiddle - OVERTAKE_OFFSET_INC*rgtinc*8);
		maxoffset = MIN(maxoffset, car->_trkPos.toMiddle + OVERTAKE_OFFSET_INC*lftinc*8);
	}
	//myoffset = car->_trkPos.toMiddle;

	// Side Collision.
	for (i = 0; i < opponents->getNOpponents(); i++) 
	{
		tCarElt *ocar = opponent[i].getCarPtr();

		if (ocar->_state & (RM_CAR_STATE_NO_SIMU & ~RM_CAR_STATE_PIT))
			continue;

		if (fabs(ocar->_trkPos.toMiddle) > Width/2 + 3.0 && 
				fabs(car->_trkPos.toMiddle-ocar->_trkPos.toMiddle) >= 5.0)
			continue;

		if ((opponent[i].getState() & OPP_SIDE))
		{
			o = &opponent[i];
if (DebugMsg & debug_overtake)
fprintf(stderr,"%s SIDE %s\n",car->_name,ocar->_name);fflush(stderr);
			
			double sidedist = fabs(ocar->_trkPos.toLeft - car->_trkPos.toLeft);
			double sidemargin = opponent[i].getWidth()/2 + getWidth()/2 + 2.0f + fabs(rldata->rInverse)*100;
			double side = (car->_trkPos.toMiddle-angle) - (ocar->_trkPos.toMiddle-opponent[i].getAngle());
			double sidedist2 = sidedist;
			if (side > 0.0)
			{
				sidedist2 -= (o->getSpeedAngle() - speedangle) * 40;
				sidemargin -= MIN(0.0, rldata->rInverse*100);
			}
			else
			{
				sidedist2 -= (speedangle - o->getSpeedAngle()) * 40;
				sidemargin += MAX(0.0, rldata->rInverse*100);
			}
			int closing = (sidedist2 < sidedist);

			if (sidedist < sidemargin || sidedist2 < sidemargin)
			{
				//double w = Width/WIDTHDIV-BORDER_OVERTAKE_MARGIN;
				double sdiff = 2.0 - MAX(sidemargin-sidedist, sidemargin-sidedist2)/sidemargin;

				if (side > 0.0) {
					myoffset += (float) (OVERTAKE_OFFSET_INC*lftinc*MAX(0.2f, MIN(1.1f, sdiff)));
					//if (rldata->rInverse < 0.0)
					//	myoffset -= rldata->rInverse * 500 * IncFactor;
					avoidmovt = 1;
if (DebugMsg & debug_overtake)
fprintf(stderr,"%s SIDE to Rgt %s, MOVING LEFT by %.3f, sm=%.3f sd=%.3f/%.3f sm-sd=%.3f mInv=%.3f\n",car->_name,ocar->_name,(float) (OVERTAKE_OFFSET_INC*lftinc*MAX(0.2f, MIN(1.5f, sdiff))),sidemargin,sidedist,sidedist2,sdiff,rldata->mInverse);fflush(stderr);
				} else if (side <= 0.0) {
					myoffset -= (float) (OVERTAKE_OFFSET_INC*rgtinc*MAX(0.2f, MIN(1.1f, sdiff)));
					//if (rldata->rInverse > 0.0)
					//	myoffset -= rldata->rInverse * 500 * IncFactor;
					avoidmovt = 1;
if (DebugMsg & debug_overtake)
fprintf(stderr,"%s SIDE to Lft %s, MOVING RIGHT by %.3f sm-sd=%.2f\n",car->_name,ocar->_name,(OVERTAKE_OFFSET_INC*lftinc*MAX(1.0f, MIN(2.0f, sdiff))),sdiff);fflush(stderr);
				}

				if (avoidmovt)
					sideratio = MIN(sidedist,sidedist2)/sidemargin;
else if (DebugMsg & debug_overtake)
fprintf(stderr,"%s SIDE %s, NO MOVE %.1f\n",car->_name,ocar->_name,myoffset);fflush(stderr);
			}
			else if (sidedist > sidemargin+3.0)
			{
				if ((car->_trkPos.toLeft > ocar->_trkPos.toLeft && rldata->rInverse > 0.0) ||
				    (car->_trkPos.toLeft < ocar->_trkPos.toLeft && rldata->rInverse < 0.0))
					avoidtime = MIN(simtime, avoidtime + MIN(deltaTime*0.9, fabs(rldata->rInverse*1.2)));

				if (ocar->_trkPos.toLeft > car->_trkPos.toLeft &&
			            car->_trkPos.toLeft < MIN(lane2left, 4.0 + fabs(nextCRinverse)*1000))
				{
					myoffset -= (float) (OVERTAKE_OFFSET_INC*lftinc/4);
if (DebugMsg & debug_overtake)
fprintf(stderr,"%s SIDE to Rgt %s, MOVING BACK TO RIGHT\n",car->_name,ocar->_name);fflush(stderr);
					if (!avoidmode)
						avoidtime = MIN(simtime, avoidtime+deltaTime*0.5);
				}
				else if (ocar->_trkPos.toLeft < car->_trkPos.toLeft &&
		             	         car->_trkPos.toRight < MIN(lane2right, 4.0 + fabs(nextCRinverse)*1000))
				{
					myoffset += (float) (OVERTAKE_OFFSET_INC*rgtinc/4);
if (DebugMsg & debug_overtake)
fprintf(stderr,"%s SIDE to Lft %s, MOVING BACK TO LEFT\n",car->_name,ocar->_name);fflush(stderr);
					if (!avoidmode)
						avoidtime = MIN(simtime, avoidtime+deltaTime*0.5);
				}
else if (DebugMsg & debug_overtake)
fprintf(stderr,"%s SIDE %s, NO MOVE %.1f\n",car->_name,ocar->_name,myoffset);fflush(stderr);
			}
else if (DebugMsg & debug_overtake)
fprintf(stderr,"%s SIDE %s, NO MOVE AT ALL! %.1f\n",car->_name,ocar->_name,myoffset);fflush(stderr);

			if (ocar->_trkPos.toLeft > car->_trkPos.toLeft)
			{
				avoidrgtoffset = (float) MAX(avoidrgtoffset, ocar->_trkPos.toMiddle + (o->getWidth()+1.0f));
				avoidmode |= avoidright;
				if (avoidmovt)
					avoidmode |= avoidside;
				if (closing)
					avoidmode |= avoidsideclosing;
			}
			else
			{
				avoidlftoffset = (float) MIN(avoidlftoffset, ocar->_trkPos.toMiddle - (o->getWidth()+1.0f));
				avoidmode |= avoidleft;
				if (avoidmovt)
					avoidmode |= avoidside;
				if (closing)
					avoidmode |= avoidsideclosing;
			}

		}
	}

	if (avoidmode)
	{
		if (!avoidmovt)
			avoidtime = MIN(simtime, avoidtime+deltaTime*1.0);
		goto end_getoffset;
	}

#if 0
	// don't try and front-avoid if we're struggling for control!
	if (avgaccel_x - fabs(laststeer) * fabs(angle-speedangle) < -10.0)
		goto end_getoffset;

	if (rldata->lftmargin > 0.0 && minoffset < rldata->offset - rldata->lftmargin)
		minoffset = rldata->offset - rldata->lftmargin;
	if (rldata->rgtmargin > 0.0 && maxoffset > rldata->offset + rldata->rgtmargin)
		maxoffset = rldata->offset + rldata->rgtmargin;
#endif

	//if (fabs(angle) + car->_accel_x/100 < 0.5)
	{
		double caution = rldata->overtakecaution;

		{
			tTrackSeg *wseg = (rldata->rInverse > 0.0 ? car->_wheelSeg(FRNT_LFT) : car->_wheelSeg(FRNT_RGT));
			if (wseg->surface->kFriction > car->_trkPos.seg->surface->kFriction)
				caution += (wseg->surface->kFriction - car->_trkPos.seg->surface->kFriction) * 4;
		}

		//caution += fabs(speedangle - angle)*10;
		int otry_success = 0;
		double rInverse = rldata->rInverse;
		if (fabs(nextCRinverse) > fabs(rInverse))
			rInverse = nextCRinverse;

		for (int otry=0; otry<=1; otry++)
		{
			// Overtake.
			for (i = 0; i < opponents->getNOpponents(); i++) 
			{
				tCarElt *ocar = opponent[i].getCarPtr();
	
				// strategy telling us to follow this car?
				if ((opponent[i].getState() & OPP_FRONT_FOLLOW))
					continue;

				// off track or a long way wide of us?
				if (!(opponent[i].getState() & OPP_COLL) &&
				    fabs(ocar->_trkPos.toMiddle) > Width/2 + 3.0 && 
				    fabs(car->_trkPos.toMiddle-ocar->_trkPos.toMiddle) >= 8.0)
					continue;

				if (ocar->_state & (RM_CAR_STATE_NO_SIMU & ~RM_CAR_STATE_PIT))
					continue;

				if ((opponent[i].getState() & OPP_FRONT) &&
				    !(opponent[i].isTeamMate() && car->race.laps <= opponent[i].getCarPtr()->race.laps))
				{
#if 1
					if (canOvertake(&opponent[i], &mincatchdist, false, (otry == 1)))
						o = &opponent[i];
#else
					int segid = car->_trkPos.seg->id;
					int osegid = opponent[i].getCarPtr()->_trkPos.seg->id;
					double distance = opponent[i].getDistance();
					double truespeed = getTrueSpeed();
					double truespeed2 = truespeed + MAX(0.0, 6.0 - fabs(rInverse)*400);
					double speed = MIN(truespeed2, MIN(rldata->avspeed, truespeed+MAX(1.0, (((car->_speed_x*car->_speed_x)/100)-distance)/3)));
					double ospeed = opponent[i].getTrueSpeed();
					double try_timer = (0.3 + (1.0 - ((simtime-frontavoidtime)/5.0)/2)) * otry;
					double sidedist = fabs(car->_trkPos.toMiddle - opponent[i].getCarPtr()->_trkPos.toMiddle);
					double spddiff = (speed - ospeed)*1.5;
					catchdist = ((double) MIN(speed*distance/(spddiff), distance*CATCH_FACTOR) + (speed * caution)) * (1.0 + try_timer);
					double sdiff = ((distance - MAX(0.0, 5.0-sidedist)) - (spddiff)*2);

					if (catchdist < mincatchdist && speed >= ospeed && sdiff < speed/(3+caution))
					{
						double aspeed = speed+1.0;
						double aspeed2 = aspeed-3.0;
						if (truespeed < getSpeed())
							speed -= getSpeed() - truespeed;
						if (rldata->avspeed < getSpeed() && car->_accel_x < -2.0 &&
								((rInverse > 0.001 && speedangle < 0.0) ||
								 (rInverse < -0.001 && speedangle > 0.0)))
								speed -= fabs(speedangle*4);
	
						if (speed > ospeed)
							catchdist = ((double) MIN(speed*distance/(spddiff), distance*CATCH_FACTOR) + (speed * caution)) * (0.6 + try_timer);
						else
							catchdist = 10000.0;

						if (catchdist < mincatchdist) // && ((catchdist < MAX(20.0, (distance * 3 + (speed-ospeed)))) || distance + caution < 3.0 + MIN(50.0, car->_speed_x)/15))
						{
	
							double mradius = MAX(5.0, 100.0 - fabs(rInverse) * 5000);
							int odiv = (rldata->thisdiv + int(distance/3));
							double oradius = MAX(5.0, 100.0 - fabs(raceline->getRInverse(odiv)) * 5000);

							if (simtime > 1.0 &&
							    (catchdist < MIN(MIN(500.0, aspeed*7.0), MIN(mradius, oradius)*1.5) && 
							    distance < MAX(3.0, MIN(speed, aspeed2)/4 + (aspeed2-ospeed)) &&
							    ospeed < MAX(10.0, aspeed)))
							{
if (DebugMsg & debug_overtake)
fprintf(stderr,"%s -> %s (%.1f < %.1f (ot=%.2f))\n",car->_name,ocar->_name,catchdist,mincatchdist,caution);fflush(stderr);
								mincatchdist = catchdist;
								o = &opponent[i];
								otry_success = otry;
							}
						}
else if (DebugMsg & debug_overtake)
fprintf(stderr,"%s -> %s CANCEL 2 (%.1f > %.1f || %.1f < %.1f || %.3f > %.3f)\n",car->_name,ocar->_name,catchdist,mincatchdist,speed,ospeed,sdiff,speed/(3+caution));
					}
else if (DebugMsg & debug_overtake)
fprintf(stderr,"%s -> %s CANCEL 1 (%.1f > %.1f || %.1f < %.1f || %.3f > %.3f)\n",car->_name,ocar->_name,catchdist,mincatchdist,speed,ospeed,sdiff,speed/(3+caution));
#endif
				}
			}

			if (o || mode != mode_avoiding) break;
		}

		if (o != NULL) 
		{
			tCarElt *ocar = o->getCarPtr();

			// Compute the width around the middle which we can use for overtaking.
			float w = ocar->_trkPos.seg->width/WIDTHDIV-BORDER_OVERTAKE_MARGIN;
			// Compute the opponents distance to the middle.
			float otm = ocar->_trkPos.toMiddle;
			// Define the with of the middle range.
			float wm = ocar->_trkPos.seg->width*CENTERDIV;
			float sidedist = fabs(car->_trkPos.toMiddle-ocar->_trkPos.toMiddle);
			double sdist = (rInverse > 0.0 ? (-sidedist - 3.0) : (sidedist - 3.0));

			//int avoidingside = (otm > wm && myoffset > -w) ? TR_RGT : ((otm < -wm && myoffset < w) ? TR_LFT : TR_STR);
			int avoidingside = (car->_trkPos.toLeft > ocar->_trkPos.toLeft/*+4.0*/ ? TR_LFT : (car->_trkPos.toLeft<ocar->_trkPos.toLeft/*-4.0*/ ? TR_RGT : TR_STR));
			int mustmove = 0;
			int cancelovertake = 0;
			double distance = o->getDistance();

			if (avoidingside != TR_STR)
			{
				int newside = checkSwitch(avoidingside, o, ocar);
if (DebugMsg & debug_overtake)
fprintf(stderr," AVOIDING A %c\n",(avoidingside==TR_LFT?'L':'R'));
				if (newside != avoidingside)
				{
if (DebugMsg & debug_overtake)
fprintf(stderr," SWITCH 1 from %c to %c\n",(avoidingside==TR_LFT?'L':'R'),(newside==TR_LFT?'L':'R'));
					avoidingside = newside;
					mustmove = 1;
				}

				if ((((avoidingside == prefer_side && prefer_side != TR_STR) || (avoidingside == car->_trkPos.seg->type)) && distance > 1.0) ||
				    rldata->braking <= 0.0)
				{
#if 1
					if (!(canOvertake(o, &mincatchdist, true, false)))
						cancelovertake = 1;
#else
					if ((rInverse > 0.0 && angle > 0.0) || (rInverse < 0.0 && angle < 0.0))
						caution += fabs(speedangle-angle) * 20;

					double cdistance = o->getDistance() + (fabs(rInverse)*700);
					double truespeed = getTrueSpeed();
					double truespeed2 = truespeed + MAX(0.0, 6.0 - fabs(rInverse)*700);
					//double speed = MIN(truespeed2, MIN(rldata->avspeed, truespeed+MAX(1.0, cdistance+1.0)));
					double speed = MIN(truespeed2, MIN(rldata->avspeed, truespeed+MAX(1.0, (((car->_speed_x*car->_speed_x)/100)-cdistance)/4)));
					double ospeed = o->getTrueSpeed() * (1.0 + fabs(rInverse)*50);
					catchdist = ((double) MIN(speed*cdistance/(speed-ospeed), cdistance*CATCH_FACTOR) + (speed * caution)) * (1.0 + fabs(rInverse)*120);
					//double sdiff = ((distance + MAX(0.0, 10.0-sdist*2)) - (speed - ospeed)*2);
					double sdiff = (sdist*3+(speed-ospeed)) - distance * (1.0 + fabs(rInverse*80));

					if ((catchdist > MAX(20.0 + sdist*2, (cdistance * 3 + (speed-ospeed)))) || sdiff < 0.0)
					{
						cancelovertake = 1;
						avoidmode = 0;
if (DebugMsg & debug_overtake)
fprintf(stderr,"%s -> %s CANCEL 3 (%.1f < %.1f)\n",car->_name,ocar->_name,catchdist,mincatchdist);fflush(stderr);
					}
#endif
				}
			}

			if (!cancelovertake)
			{
				if (avoidingside == TR_LFT)
				{
					sidedist -= (speedangle - o->getSpeedAngle()) * 20;
					if (mustmove || 
					    sidedist < car->_dimension_y + ocar->_dimension_y + 1.0 ||
					    (o->getState() & OPP_COLL) ||
					    (prefer_side == TR_RGT && car->_trkPos.toRight > MIN(lane2right, 3.0 - nextCRinverse*1000)))
					{
						double rinc = OVERTAKE_OFFSET_INC * rgtinc;
						if (rInverse > 0.0)
							rinc += rInverse * 200 * IncFactor;
			 			myoffset -= (float) rinc;
if (DebugMsg & debug_overtake)
fprintf(stderr,"%s LFT %s, MOVING RIGHT %.3f/%.3f -> %.3f\n",car->_name,ocar->_name,(float) (OVERTAKE_OFFSET_INC*rgtinc),rinc,myoffset);
						avoidmovt = 1;
					}
					else if (sidedist > car->_dimension_y + ocar->_dimension_y + 4.0 &&
							     car->_trkPos.toRight < MIN(lane2right-1.0, 4.0 + fabs(nextCRinverse)*1000))
					{
if (DebugMsg & debug_overtake)
fprintf(stderr,"%s LFT %s, MOVING BACK TO LEFT %.3f\n",car->_name,ocar->_name,(float) (OVERTAKE_OFFSET_INC*lftinc/2));
						myoffset += (float) (OVERTAKE_OFFSET_INC*lftinc)/2;
						if (!avoidmode)
							avoidtime = MIN(simtime, avoidtime+deltaTime);
					}
else if (DebugMsg & debug_overtake)
fprintf(stderr,"%s LFT %s, HOLDING LINE\n",car->_name,ocar->_name);
				}
				else if (avoidingside == TR_RGT)
				{
					sidedist -= (o->getSpeedAngle() - speedangle) * 20;
					if (mustmove || 
					    sidedist < car->_dimension_y + ocar->_dimension_y + 1.0 ||
					    (o->getState() & OPP_COLL) ||
				            (prefer_side == TR_LFT && car->_trkPos.toLeft > MIN(lane2left, 3.0 + nextCRinverse*1000)))
					{
if (DebugMsg & debug_overtake)
fprintf(stderr,"%s RGT %s, MOVING LEFT %.3f\n",car->_name,ocar->_name,(float) (OVERTAKE_OFFSET_INC*lftinc));
						double linc = OVERTAKE_OFFSET_INC * lftinc;
						if (rInverse < 0.0)
							linc -= rInverse * 200 * IncFactor;
						myoffset += (float) linc;
						avoidmovt = 1;
					}
					else if (sidedist > car->_dimension_y + ocar->_dimension_y + 4.0 &&
							     car->_trkPos.toLeft < MIN(lane2left-1.0, 4.0 + fabs(nextCRinverse)*1000))
					{
if (DebugMsg & debug_overtake)
fprintf(stderr,"%s RGT %s, MOVING BACK TO RIGHT %.3f\n",car->_name,ocar->_name,(float) (OVERTAKE_OFFSET_INC*rgtinc/2));
						myoffset -= (float) (OVERTAKE_OFFSET_INC*rgtinc)/2;
						if (!avoidmode)
							avoidtime = MIN(simtime, avoidtime+deltaTime);
					}
else if (DebugMsg & debug_overtake)
fprintf(stderr,"%s RGT %s, HOLDING LINE\n",car->_name,ocar->_name);
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

					if (lenleft > lenright)
						avoidingside = TR_RGT;
					else if (lenright > lenleft)
						avoidingside = TR_LFT;

					// If we are on a straight look for the next turn.
					if (avoidingside == TR_STR)
					{
						while (seg->type == TR_STR) {
							seg = seg->next;
						}
						// Assume: left or right if not straight.
						if (seg->type == TR_LFT) {
							avoidingside = TR_RGT;
						} else {
							avoidingside = TR_LFT;
						}
					}

					// Because we are inside we can go to the border.
					//float maxoff = (float) ((ocar->_trkPos.seg->width - car->_dimension_y)/2.0f-BORDER_OVERTAKE_MARGIN*15);
					int newside = checkSwitch( avoidingside, o, ocar );
if (DebugMsg & debug_overtake)
fprintf(stderr," AVOIDING %c\n",(avoidingside==TR_LFT?'l':'r'));
					if (newside != avoidingside)
					{
if (DebugMsg & debug_overtake)
fprintf(stderr," SWITCH 2 from %c to %c\n",(avoidingside==TR_LFT?'l':'r'),(newside==TR_LFT?'l':'r'));
						avoidingside = newside;
						mustmove = 1;
					}

					if (prefer_side != TR_STR && avoidingside == prefer_side)
					{
#if 1
						if (!(canOvertake(o, &mincatchdist, true, false)))
							cancelovertake = 1;
#else
						double cdistance = o->getDistance() + (fabs(rInverse)*700);
						double truespeed = getTrueSpeed();
						double truespeed2 = truespeed + MAX(0.0, 6.0 - fabs(rInverse)*700);
						//double speed = MIN(truespeed2, MIN(rldata->avspeed, truespeed+MAX(1.0, cdistance+1.0)));
						double speed = MIN(truespeed2, MIN(rldata->avspeed, truespeed+MAX(1.0, (((car->_speed_x*car->_speed_x)/100)-cdistance)/4)));
						double ospeed = o->getTrueSpeed() * (1.0 + fabs(rInverse)*50);
						catchdist = ((double) MIN(speed*cdistance/(speed-ospeed), cdistance*CATCH_FACTOR) + (speed * caution)) * (1.0 + fabs(rInverse)*120);
						double sdiff = (sdist*3+(speed-ospeed)) - distance * (1.0 + fabs(rInverse*80));
	
						if ((catchdist > MAX(20.0 + sdist*2, (cdistance * 3 + (speed-ospeed)))) || sdiff < 0.0)
						{
if (DebugMsg & debug_overtake)
fprintf(stderr,"%s -> %s CANCEL 4 (%.1f < %.1f)\n",car->_name,ocar->_name,catchdist,mincatchdist);fflush(stderr);
							cancelovertake = 1;
						}
#endif
					}

					if (!cancelovertake)
					{
						if (mustmove || (o->getState() & OPP_COLL) || sidedist < car->_dimension_y + ocar->_dimension_y + 1.0)
						{
							if (avoidingside == TR_RGT)
							{
								sidedist -= (o->getSpeedAngle() - speedangle) * 20;
								if (myoffset < maxoffset ||
								    (prefer_side == TR_LFT && car->_trkPos.toLeft > 3.0 + nextCRinverse*1000))
								{
if (DebugMsg & debug_overtake)
fprintf(stderr,"%s RGT %s, MOVING LEFT %.3f\n",car->_name,ocar->_name,(float) (OVERTAKE_OFFSET_INC*lftinc));
									myoffset += (float) (OVERTAKE_OFFSET_INC*lftinc);
									avoidmovt = 1;
								}
								else if (sidedist > car->_dimension_y + ocar->_dimension_y + 4.0 &&
										     car->_trkPos.toLeft < 4.0 + fabs(nextCRinverse)*1000)
								{
if (DebugMsg & debug_overtake)
fprintf(stderr,"%s RGT %s, MOVING BACK TO RIGHT %.3f\n",car->_name,ocar->_name,(float) (OVERTAKE_OFFSET_INC*rgtinc)/2);
									myoffset -= (float) (OVERTAKE_OFFSET_INC*rgtinc)/2;
									if (!avoidmode)
										avoidtime = MIN(simtime, avoidtime+deltaTime);
								}
else if (DebugMsg & debug_overtake)
fprintf(stderr,"%s RGT %s, NO MOVEMENT\n",car->_name,ocar->_name);
							} 
							else 
							{
								sidedist -= (speedangle - o->getSpeedAngle()) * 20;
								if (myoffset > minoffset ||
								    (prefer_side == TR_RGT && car->_trkPos.toRight > 3.0 - nextCRinverse*1000))
								{
if (DebugMsg & debug_overtake)
fprintf(stderr,"%s LFT %s, MOVING RIGHT %.3f -> %.3f\n",car->_name,ocar->_name,(float) (OVERTAKE_OFFSET_INC*rgtinc),myoffset);
									myoffset -= (float) (OVERTAKE_OFFSET_INC*rgtinc);
									avoidmovt = 1;
								}
								else if (sidedist > car->_dimension_y + ocar->_dimension_y + 4.0 &&
										     car->_trkPos.toRight < 4.0 + fabs(nextCRinverse)*1000)
								{
if (DebugMsg & debug_overtake)
fprintf(stderr,"%s LFT %s, MOVING BACK TO LEFT %.3f\n",car->_name,ocar->_name,(float) (OVERTAKE_OFFSET_INC*lftinc)/2);
									myoffset += (float) (OVERTAKE_OFFSET_INC*lftinc)/2;
									if (!avoidmode)
										avoidtime = MIN(simtime, avoidtime+deltaTime);
								}
else if (DebugMsg & debug_overtake)
fprintf(stderr,"%s LFT %s, NO MOVEMENT\n",car->_name,ocar->_name);
							}
						}
					}
				}

				if (!cancelovertake)
				{
				//if (ocar->_trkPos.toLeft > car->_trkPos.toLeft)
					if (avoidingside == TR_RGT)
					{
						avoidrgtoffset = (float) MAX(avoidrgtoffset, ocar->_trkPos.toMiddle + (o->getWidth()+1.0f));
						avoidmode |= avoidright;
					}
					else
					{
						avoidlftoffset = (float) MIN(avoidlftoffset, ocar->_trkPos.toMiddle - (o->getWidth()+1.0f));
						avoidmode |= avoidleft;
					}
	
					if ((avoidingside == TR_LFT && rInverse > 0.0) ||
					    (avoidingside == TR_RGT && rInverse < 0.0))
						avoidtime = MIN(simtime, avoidtime + MIN(deltaTime*0.9, fabs(rInverse*1.2)));

					myoffset = (float) (MAX(minoffset, MIN(maxoffset, myoffset)));

					if (!otry_success)
						frontavoidtime = simtime;
				}
			}
		}

		if (!avoidmode)
		{
			o = NULL;

			// Let overlap or let less damaged team mate pass.
			for (i = 0; i < opponents->getNOpponents(); i++) 
			{
				// Let the teammate with less damage overtake to use slipstreaming.
				// The position change happens when the damage difference is greater than
				// TEAM_DAMAGE_CHANGE_LEAD.
				if ((opponent[i].getState() & OPP_LETPASS))
				{
					// Behind, larger distances are smaller ("more negative").
					if (opponent[i].getDistance() > mindist) {
						mindist = opponent[i].getDistance();
						o = &opponent[i];
					}
				}
			}

			if (o != NULL) 
			{
				tCarElt *ocar = o->getCarPtr();
				float side = car->_trkPos.toMiddle - ocar->_trkPos.toMiddle;
				float w = car->_trkPos.seg->width/WIDTHDIV-BORDER_OVERTAKE_MARGIN;
if (DebugMsg & debug_overtake)
fprintf(stderr,"%s BEHIND %s (%d %d %d %d)\n",car->_name,ocar->_name,((o->getState() & OPP_LETPASS) && !o->isTeamMate()),(o->isTeamMate() && (car->_dammage - o->getDamage() > TEAM_DAMAGE_CHANGE_LEAD)),((o->getDistance() > -TEAM_REAR_DIST) && (o->getDistance() < -car->_dimension_x)),(car->race.laps == o->getCarPtr()->race.laps));
				if (side > 0.0f) {
					if (myoffset < w) {
						myoffset += (float) (OVERTAKE_OFFSET_INC*lftinc);
						avoidmovt = 1;
					}
				} else {
					if (myoffset > -w) {
						myoffset -= (float) (OVERTAKE_OFFSET_INC*rgtinc);
						avoidmovt = 1;
					}
				}

				avoidmode |= avoidback;

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
	
				myoffset = (float) (MAX(minoffset, MIN(maxoffset, myoffset)));
			} 
		}
	}

	if (mode == mode_avoiding && avoidmode == 0)
		setMode(mode_correcting);
	if (mode == mode_normal)
		myoffset = (float) moffset;

#if 1
	// no-one to avoid, work back towards raceline
	if (mode == mode_correcting)
	{
		double factor = (fabs(car->_trkPos.toMiddle) < car->_trkPos.seg->width/2 + 2.0 ? 0.25 : 1.0);
		if (fabs(myoffset) > fabs(rldata->offset))
		{
			double inc = OVERTAKE_OFFSET_INC * MIN(lftinc, rgtinc) * factor;
			if (myoffset < rldata->offset)
				myoffset += (float) (MIN(rldata->offset-myoffset, inc));
			else if (myoffset > rldata->offset)
				myoffset -= (float) (MIN(myoffset-rldata->offset, inc));
		}
	}
#endif

end_getoffset:
	if (avoidmode)
		setMode(mode_avoiding);

	if (mode == mode_avoiding && !avoidmovt)
		avoidtime = MIN(simtime, avoidtime+deltaTime*1.5);

	myoffset = (float) (MAX(minoffset, MIN(maxoffset, myoffset)));
if (DebugMsg & debug_overtake)
	if (mode != mode_normal)
		fprintf(stderr,"mode=%d max=%.1f min=%.1f myoff=%.1f->%.1f->%.1f",mode,maxoffset,minoffset,car->_trkPos.toMiddle,origoffset,myoffset);
if (DebugMsg & debug_overtake)
	if (mode != mode_normal)
	{
		fprintf(stderr,"->%.1f\n",myoffset);
		fflush(stderr);
	}
	return myoffset;
}

int Driver::checkSwitch( int side, Opponent *o, tCarElt *ocar )
{
	double t_impact = MAX(0.0, MIN(10.0, o->getTimeImpact()));
	if (car->_speed_x - ocar->_speed_x < MIN(5.0, o->getDistance()*3))
		t_impact *= (1.0 + (5.0 - (car->_speed_x - ocar->_speed_x)));
	t_impact = MIN(3.0, t_impact);

	double mcatchleft = MAX(1.0, MIN(track->width-1.0, car->_trkPos.toLeft - speedangle * (t_impact * 10)));
	double ocatchleft = MAX(1.0, MIN(track->width-1.0, ocar->_trkPos.toLeft - o->getSpeedAngle() * (t_impact * 10)));
	double xdist = o->getDistance();
	double ydist = mcatchleft-ocatchleft;
	double sdiff = MAX(0.0, getSpeed() - o->getSpeed());
	double radius = MIN(car->_dimension_y*3, fabs(nextCRinverse) * 200);
	double speedchange = 0.0;

	if (prefer_side == side && rldata->speedchange < 0.0 && ocar->_pos > car->_pos)
		speedchange = fabs(rldata->speedchange)*3;

#if 0
	if (fabs(mcatchleft - ocatchleft) < car->_dimension_y + 1.5 + radius &&
			(fabs(mcatchleft - ocatchleft) < fabs(car->_trkPos.toLeft - ocar->_trkPos.toLeft) ||
			 (side == TR_LFT && ocatchleft > car->_trkPos.seg->width - car->_dimension_y + 1.5 + radius) ||
			 (side == TR_RGT && ocatchleft < car->_dimension_y + 1.5 + radius)))
#endif
	{
		switch (side)
		{
			case TR_RGT:
if (DebugMsg & debug_overtake)
fprintf(stderr,"CHECKSWITCH: Rgt - ti=%.2f dm=%.1f o=%.2f->%.2f m=%.2f->%.2f\n",t_impact,deltamult,ocar->_trkPos.toLeft,ocatchleft,car->_trkPos.toLeft,mcatchleft);
				if (nextCRinverse > 0.0)
					radius = 0.0;
				if (side == prefer_side || 
				    xdist > sdiff + ydist + MAX(0.0, angle*10) ||
				    ocatchleft > (car->_dimension_y + 3.0 + radius + speedchange))
				{
					if (ocatchleft < (car->_dimension_y + 1.5 + radius) ||
					    (ocatchleft < car->_trkPos.seg->width-(car->_dimension_y+1.5+radius) && ocatchleft < mcatchleft - 1.5))
					{
						side = TR_LFT;
					}
				}
				break;

			case TR_LFT:
			default:
if (DebugMsg & debug_overtake)
fprintf(stderr,"CHECKSWITCH: Lft - ti=%.2f dm=%.1f o=%.2f->%.2f m=%.2f->%.2f\n",t_impact,deltamult,ocar->_trkPos.toLeft,ocatchleft,car->_trkPos.toLeft,mcatchleft);
				if (nextCRinverse < 0.0)
					radius = 0.0;
				if (side == prefer_side || 
				    xdist > sdiff + (-ydist) + MAX(0.0, -angle*10) ||
				    ocatchleft > (car->_trkPos.seg->width - (car->_dimension_y + 3.0 + radius + speedchange)))
				{
					if (ocatchleft > (car->_trkPos.seg->width - (car->_dimension_y + 1.5 + radius)) ||
					    (ocatchleft > (car->_dimension_y+1.5+radius) && ocatchleft > mcatchleft + 1.5))
					{
						side = TR_RGT;
					}
				}
				break;
		}
	}

	return side;
}

// Update my private data every timestep.
void Driver::update(tSituation *s)
{
	// Update global car data (shared by all instances) just once per timestep.
	if (simtime != s->currentTime) {
		simtime = s->currentTime;
		cardata->update();
	}
	evalTrueSpeed();

	prefer_side = raceline->findNextCorner( &nextCRinverse );

	// Update the local data rest.
	avgaccel_x += (car->_accel_x - avgaccel_x)/2;
	prevspeedangle = speedangle;
	speedangle = (float) -(mycardata->getTrackangle() - atan2(car->_speed_Y, car->_speed_X));
	NORM_PI_PI(speedangle);
	mass = CARMASS + car->_fuel;
	currentspeedsqr = car->_speed_x*car->_speed_x;
	opponents->update(s, this, DebugMsg);
	strategy->update(car, s);

	if (car->_state <= RM_CAR_STATE_PIT && !NoPit)
	{
		if (!pit->getPitstop() && (car->_distFromStartLine < pit->getNPitEntry() || car->_distFromStartLine > pit->getNPitEnd())) 
		{
			pit->setPitstop(strategy->needPitstop(car, s, opponents));
		}

		if (pit->getPitstop() && car->_pit)
		{
			pitpos = PIT_MID;

			for (int i=0; i<opponents->getNOpponents(); i++)
			{
				int idx = opponent[i].getIndex();
				if (opponent[i].getTeam() != TEAM_FRIEND) continue;
				if (opponent[i].getCarPtr() == car) continue;
				if (opponent[i].getCarPtr()->_state > RM_CAR_STATE_PIT)
					continue;

				int opitpos = (int) opponent[i].getCarPtr()->_lightCmd;
 
				if (opitpos != PIT_NONE && car->_fuel > fuelperlap*1.5 && car->_trkPos.toLeft >= 0.0 && car->_trkPos.toLeft <= track->width)
				{
					// pit occupied & we've got enough fuel to go around again
					pit->setPitstop( 0 );
					pitpos = PIT_NONE;
					break;
				}

				if (opponent[i].getCarPtr()->_pit->pos.seg == car->_pit->pos.seg)
				{
					// sharing a pit
					if (opitpos == PIT_FRONT)
					{
						double pitloc = pit->getNPitLoc( PIT_MID );
						double myfrompit = pitloc - car->_distFromStartLine;
						double opfrompit = pitloc - opponent[i].getCarPtr()->_distFromStartLine;
						if (myfrompit < 0.0) myfrompit += track->length;
						if (opfrompit < 0.0) opfrompit += track->length;

						// work out who's closest to the pit & therefore should go in front
						if (opfrompit > myfrompit)
						{
							pitpos = PIT_FRONT;
						}
						else
						{
							pitpos = PIT_BACK; // go in behind other car
						}
					}
					else
					{
						pitpos = PIT_FRONT; // stop at end of pit space to leave room
					}
				}

				break;
			}
		}
		else if (!pit->getInPit())
			pitpos = PIT_NONE;
	}
	else
	{
		pitpos = PIT_NONE;
	}

	car->_lightCmd = (char) pitpos;

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
		if (opponent[i].getTeam() == TEAM_FRIEND)
			continue;

		// not a friend - if we're avoiding then we're obviously not alone
		if (mode == mode_avoiding)
			return 0;

		if ((opponent[i].getState() & (OPP_COLL | OPP_LETPASS)) ||
		    ((opponent[i].getState() & (OPP_FRONT)) && opponent[i].getDistance() < MAX(50.0, car->_speed_x*1.5)) ||
		    (fabs(opponent[i].getDistance()) < 50.0))
		{
			return 0;	// Not alone.
		}
	}
	return 1;	// Alone.
}


float Driver::stuckSteering( float steercmd )
{
	if (stucksteer > 0.0f)
		steercmd = (fabs(steercmd) + stucksteer) / 2;
	else
		steercmd = -(fabs(steercmd) + fabs(stucksteer)) / 2;
	return steercmd;
}

// Check if I'm stuck.
bool Driver::isStuck()
{
	float lftmargin = MAX(4.0f, car->_trkPos.seg->width*0.4);
	float rgtmargin = MAX(4.0f, car->_trkPos.seg->width*0.4);
	float lftwidth = 0.0f, rgtwidth = 0.0f;
	
	if (car->_trkPos.seg->side[TR_SIDE_RGT] != NULL)
	{
		rgtmargin = MIN(car->_trkPos.seg->width * 0.4, 4.0 + MAX(0.0, 5.0 - car->_trkPos.seg->side[TR_SIDE_RGT]->width));
		if (car->_trkPos.seg->side[TR_SIDE_RGT]->style != TR_PLAN)
			rgtmargin = car->_trkPos.seg->width*0.6;
		else
			rgtwidth -= car->_trkPos.seg->side[TR_SIDE_RGT]->width;
	}
	if (car->_trkPos.seg->side[TR_SIDE_LFT] != NULL)
	{
		MIN(car->_trkPos.seg->width * 0.4, 4.0 + MAX(0.0, 5.0 - car->_trkPos.seg->side[TR_SIDE_LFT]->width));
		if (car->_trkPos.seg->side[TR_SIDE_LFT]->style != TR_PLAN)
			lftmargin = car->_trkPos.seg->width*0.6;
		else
			lftwidth -= car->_trkPos.seg->side[TR_SIDE_LFT]->width;
	}

	lftwidth = (lftwidth + car->_trkPos.toLeft);
	rgtwidth = (rgtwidth + car->_trkPos.toRight);

	if (pit->getInPit() || (car->_trkPos.toLeft > lftmargin && car->_trkPos.toRight > rgtmargin))
		allow_stuck = 0;
	//float ss = (-mycardata->getCarAngle() / car->_steerLock);
	float ss = -angle/2; // * (fabs(angle) > 1.8 ? -1 : 1);
#if 0
	if ((angle > 0.0 && lftwidth < 4.0 && ss > 0.0) ||
	    (angle < 0.0 && rgtwidth < 4.0 && ss < 0.0))
		ss = -ss;
#endif

	if (stuckcheck || (fabs(car->_speed_x) < 4.0 && fabs(car->_yaw_rate) < 0.2))
	{
		stucksteer = ss;
	}
	else if (fabs(angle) > 0.6 && stuckcheck)
	{
		if (stucksteer > 0.0f)
			stucksteer = 1.0f;
		else
			stucksteer = -1.0f;
	}
	else
		stucksteer = car->_steerCmd;

	if (fabs(angle) > 1.4 && mode != mode_correcting)
		setMode( mode_correcting );


	// does the car's position indicate we're stuck?
	bool position_stuck = (((fabs(angle) > MAX_UNSTUCK_ANGLE &&
	                        ((car->_gear == -1 && car->_speed_x < MAX_UNSTUCK_SPEED) ||
				 (car->_gear > -1 && fabs(car->_speed_x) < MAX_UNSTUCK_SPEED)) &&
	                        (MIN(car->_trkPos.toLeft, car->_trkPos.toRight)) < MIN_UNSTUCK_DIST)));
	int reversing_ok = 0, force_reverse = 0;
	if (car->_gear == -1 && fabs(angle) > 0.6)
	{
		position_stuck = 1;
		if ((car->_trkPos.toLeft < lftmargin && speedangle < -0.5) ||
		    (car->_trkPos.toRight < rgtmargin && speedangle > 0.5))
		{
			reversing_ok = 1;
		}
if (DebugMsg & debug_steer)
fprintf(stderr,"REVERSING: ok=%d force=%d (%.2f/%.2f -> %.2f)\n",reversing_ok, force_reverse,car->_trkPos.toLeft,car->_trkPos.toRight,speedangle);

	}

	if (car->_gear != -1 && fabs(car->_speed_x) < 7.0 &&
	    ((rgtwidth < 2.0 && angle < -0.7 && angle > -2.5) ||
	     (lftwidth < 2.0 && angle > 0.7 && angle < 2.5)))
	{
		force_reverse = 1;
	}

	if (position_stuck == 1)
	{
		stuckcheck = 1;
		
		if (car->_gear == 1 && car->_speed_x > 3.0 &&
		    ((car->_trkPos.toLeft < 4.0 && speedangle < -0.5) ||
		     (car->_trkPos.toRight < 4.0 && speedangle > 0.5)))
		{
			position_stuck = 0;
		}
	}

	if ((car->_gear == -1 || position_stuck) && allow_stuck)
	{

		// we need to reverse (in theory)...
		if (fabs(car->_speed_x) > 3.0)
		{
			if (simtime-stuck_timer > 3.0 || force_reverse)
			{
				if (reversing_ok)
				{
					// heading towards track, keep reversing
if (DebugMsg & debug_steer)
fprintf(stderr,"STUCK: Reversing, timeup (fast)\n");
					stuckcheck = 1;
					return true;
				}
				else
				{
					// travelling backwards for long enough.  Try forwards
if (DebugMsg & debug_steer)
fprintf(stderr,"STUCK: Cancelling, timeup, (%.2f/%.2f, %.2f)\n",car->_trkPos.toLeft,car->_trkPos.toRight,speedangle);
					stuck_timer = simtime;
					allow_stuck = 0;
					return false;
				}
			}
			else if (!reversing_ok)
			{
				// travelling backwards for long enough.  Try forwards
if (DebugMsg & debug_steer)
fprintf(stderr,"STUCK: Cancelling, !reverse_ok, (%.2f/%.2f, %.2f)\n",car->_trkPos.toLeft,car->_trkPos.toRight,speedangle);
				stuck_timer = simtime;
				allow_stuck = 0;
				return false;
			}

if (DebugMsg & debug_steer)
fprintf(stderr,"STUCK: Reversing, time ok (fast)\n");
			return true;
		}
		else
		{
			if (simtime - stuck_timer > 4.0)
			{
if (DebugMsg & debug_steer)
fprintf(stderr,"STUCK: Cancelling, timeup (slow)\n");
				stuck_timer = simtime;
				allow_stuck = 0;
				return false;
			}

			// yep, we're stuck.  Go backwards.
if (DebugMsg & debug_steer)
fprintf(stderr,"STUCK: Reversing (slow)\n");
			stuckcheck = 1;
			return true;
		}
	}
	else if (allow_stuck && (position_stuck || fabs(car->_speed_x) < 2.0) && (reversing_ok || fabs(car->_speed_x) < 5.0) && !pit->getInPit())
	{
		if (simtime-stuck_timer > MAX(2.0, fabs(car->_speed_x/2)+0.5) || force_reverse)
		{
			// been sitting still for a while even though not stuck.  Try reverse.
if (DebugMsg & debug_steer)
fprintf(stderr,"NOT STUCK: Switch to Reverse (%.3f > %.3f)\n",simtime-stuck_timer,fabs(car->_speed_x)+1.0);
			stuck_timer = simtime;
			allow_stuck = stuckcheck = 1;
			return true;
		}

if (DebugMsg & debug_steer)
fprintf(stderr,"NOT STUCK: Stick with Forwards ps=%d fr=%d (%.3f <= %.3f)\n",position_stuck,force_reverse,simtime-stuck_timer,fabs(car->_speed_x/2)+0.5);
		return false;
	}

	if (car->_gear >= 1 && fabs(angle) < MAX(fabs(rldata->rlangle)*4, fabs(rldata->rInverse)*500) && (MIN(car->_trkPos.toLeft, car->_trkPos.toRight) > 2.0 || fabs(angle) < 0.3))
		stuckcheck = 0;
	else if (mode == mode_normal && fabs(angle) > MAX(fabs(rldata->rlangle)*4, fabs(rldata->rInverse)*500))
		stuckcheck = 1;

	if (car->_gear == -1)
	{
		allow_stuck = 0;
		stuck_timer = simtime;
	}
	else if ((!allow_stuck && simtime-stuck_timer > 3.0) || !stuckcheck)
	{
		allow_stuck = 1;
		stuck_timer = simtime;
	}
	else
	{
if (DebugMsg & debug_steer)
fprintf(stderr," timer=%.3f\n",simtime-stuck_timer);
	}

	return false;
}


// Compute aerodynamic downforce coefficient CA.
void Driver::initWheelPos()
{
 for (int i=0; i<4; i++)
 {
  char *WheelSect[4] = {SECT_FRNTRGTWHEEL, SECT_FRNTLFTWHEEL, SECT_REARRGTWHEEL, SECT_REARLFTWHEEL};
  float rh = 0.0;
  rh = GfParmGetNum(car->_carHandle,WheelSect[i],PRM_RIDEHEIGHT,(char *)NULL, 0.10f);
  wheelz[i] = (-rh / 1.0 + car->info.wheel[i].wheelRadius) - 0.01;
 }
}

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
}


// Compute aerodynamic drag coefficient CW.
void Driver::initCw()
{
	float cx = GfParmGetNum(car->_carHandle, SECT_AERODYNAMICS, PRM_CX, (char*) NULL, 0.0f);
	float frontarea = GfParmGetNum(car->_carHandle, SECT_AERODYNAMICS, PRM_FRNTAREA, (char*) NULL, 0.0f);
	CW = 0.645f*cx*frontarea;
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
			float dist = pit->getNPitLoc(pitpos) - s;
			if (pitpos != PIT_BACK && pit->isTimeout(dist)) {
				pit->setPitstop(false);
				return 0.0f;
			} else {
				if (brakedist(0.0f, mu) > dist) {
					return 2.0f;
				} else if (s > pit->getNPitLoc(pitpos)) {
					// Stop in the pit.
			 		return 2.0f;
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
	collision = 0.0f;

	if (simtime < 1.5)
		return brake;

	float mu = car->_trkPos.seg->surface->kFriction;
	int i;
	float thisbrake = 0.0f;
	for (i = 0; i < opponents->getNOpponents(); i++) 
	{
		if ((opponent[i].getState() & OPP_COLL))
		{
			float accel = 0.0f;//opponent[i].getCarPtr()->_accel_x / MAX(1.0, opponent[i].getTimeImpact()*2);
			float ospeed = opponent[i].getSpeed() + accel;
			float margin = MIN(0.6f, MAX(0.0f, 0.6f - opponent[i].getDistance()));
			if ((opponent[i].getState() & OPP_SIDE_COLL) ||
			    brakedist(ospeed, mu) + MIN(1.0, /*0.5*/ margin + MAX(0.0, (getSpeed()-ospeed)/7)) > opponent[i].getDistance() + accel) 
			{
				accelcmd = 0.0f;
				float thiscollision = MAX(0.01f, MIN(5.0f, opponent[i].getTimeImpact()));
				//thiscollision = MAX(0.01f, MIN(thiscollision, opponent[i].getDistance()/2));
				if (collision)
					collision = MIN(collision, thiscollision);
				else
					collision = thiscollision;
				thisbrake = MAX(thisbrake, 0.3f + (5.0 - collision)/4);
if (DebugMsg & debug_brake)
fprintf(stderr,"%s - %s BRAKE: ti=%.3f\n",car->_name,opponent[i].getCarPtr()->_name,opponent[i].getTimeImpact());
			}
		}
	}
	return MAX(thisbrake, brake);
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
	slip *= 1.0f + MAX(rearskid, MAX(fabs(car->_yaw_rate)/5, fabs(angle)/4));
	slip = car->_speed_x - slip/4.0f;
	if (collision)
		slip *= 0.25f;
	if (origbrake == 2.0f)
		slip *= 0.1f;

	float absslip = (car->_speed_x < 20.0f ? MIN(AbsSlip, 2.0f) : AbsSlip);
	if (slip > absslip) {
		brake = brake - MIN(brake, (slip - absslip)/ABS_RANGE);
	}
	brake = MAX(brake, MIN(origbrake, 0.1f));

	//brake = MAX(MIN(origbrake, collision ? 0.15f :0.05f), brake - MAX(fabs(angle), fabs(car->_yaw_rate) / 2));
	brake = (float) (MAX(MIN(origbrake, (collision ? MAX(0.05f, (5.0-collision)/30) : 0.05f)), brake - fabs(angle-speedangle)*3));

	if (fbrakecmd)
		brake = MAX(brake, fbrakecmd);

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
		tTrackSeg *wseg0 = car->_wheelSeg(REAR_RGT);
		tTrackSeg *wseg1 = car->_wheelSeg(REAR_LFT);
		int count = 0;

#if 0
		double RoughLimit = (wseg0->surface->style == TR_CURB ? 0.06 : 0.02);

		if ((wseg0->surface->kRoughness > MAX(RoughLimit, seg->surface->kRoughness*1.2) ||
		     wseg0->surface->kFriction < seg->surface->kFriction*0.8 ||
 	       wseg0->surface->kRollRes > MAX(0.01, seg->surface->kRollRes*1.2)))
		{
//fprintf(stderr,"wseg0 %d (%.3f) %d (%.3f) %d (%.3f)\n",(wseg0->surface->kRoughness > MAX(0.02, seg->surface->kRoughness*1.2)),wseg0->surface->kRoughness,(wseg0->surface->kFriction < seg->surface->kFriction*0.8), wseg0->surface->kFriction, (wseg0->surface->kRollRes > MAX(0.01, seg->surface->kRollRes*1.2)),wseg0->surface->kRollRes);
			count++;
		}

		RoughLimit = (wseg1->surface->style == TR_CURB ? 0.06 : 0.02);
		if ((wseg1->surface->kRoughness > MAX(RoughLimit, seg->surface->kRoughness*1.2) ||
		     wseg1->surface->kFriction < seg->surface->kFriction*0.8 ||
 	       wseg1->surface->kRollRes > MAX(0.01, seg->surface->kRollRes*1.2)))
		{
//fprintf(stderr,"wseg1 %d (%.3f) %d (%.3f) %d (%.3f)\n",(wseg1->surface->kRoughness > MAX(0.02, seg->surface->kRoughness*1.2)),wseg1->surface->kRoughness,(wseg1->surface->kFriction < seg->surface->kFriction*0.8), wseg1->surface->kFriction, (wseg1->surface->kRollRes > MAX(0.01, seg->surface->kRollRes*1.2)),wseg1->surface->kRollRes);
			count++;
		}
#endif
		if (wseg0->surface->kRoughness > MAX(0.02, seg->surface->kRoughness*1.2))
		{
			accel1 = (float) MAX(0.1f, accel1 - (wseg0->surface->kRoughness-(seg->surface->kRoughness*2.2)) * (wseg0->style == TR_CURB ? 2 : 10));
			if (fabs(car->_steerCmd) > 0.3f)
			{
				if (car->_steerCmd < 0.0)
					car->_steerCmd = MIN(car->_steerCmd*0.3, car->_steerCmd + (wseg0->surface->kRoughness-(seg->surface->kRoughness*2.2)) * (wseg0->style == TR_CURB ? 2 : 10));
				else
					car->_steerCmd = MAX(car->_steerCmd*0.3, car->_steerCmd + (wseg0->surface->kRoughness-(seg->surface->kRoughness*2.2)) * (wseg0->style == TR_CURB ? 2 : 10));
			}
		}
		if (wseg1->surface->kRoughness > MAX(0.02, seg->surface->kRoughness*1.2))
		{
			accel1 = (float) MAX(0.1f, accel1 - (wseg1->surface->kRoughness-(seg->surface->kRoughness*2.2)) * (wseg1->style == TR_CURB ? 2 : 10));
			if (fabs(car->_steerCmd) > 0.3f)
			{
				if (car->_steerCmd < 0.0)
					car->_steerCmd = MIN(car->_steerCmd*0.3, car->_steerCmd - (wseg0->surface->kRoughness-(seg->surface->kRoughness*2.2)) * (wseg0->style == TR_CURB ? 2 : 10));
				else
					car->_steerCmd = MAX(car->_steerCmd*0.3, car->_steerCmd - (wseg0->surface->kRoughness-(seg->surface->kRoughness*2.2)) * (wseg0->style == TR_CURB ? 2 : 10));
			}
		}

#if 0
		if (wseg0->surface->kFriction < seg->surface->kFriction)
			accel1 = (float) MAX(0.0f, accel1 - (seg->surface->kFriction - wseg0->surface->kFriction));
		if (wseg1->surface->kFriction < seg->surface->kFriction)
			accel1 = (float) MAX(0.0f, accel1 - (seg->surface->kFriction - wseg1->surface->kFriction));
fprintf(stderr,"friction %.3f accel=%.3f->%.3f\n",wseg0->surface->kFriction,accel,accel1);
#endif

		if (wseg0->surface->kRollRes > MAX(0.01, seg->surface->kRollRes*1.2))
			accel1 = (float) MAX(0.0f, accel1 - (wseg0->surface->kRollRes - seg->surface->kRollRes*1.2)*4);
		if (wseg1->surface->kRollRes > MAX(0.01, seg->surface->kRollRes*1.2))
			accel1 = (float) MAX(0.0f, accel1 - (wseg1->surface->kRollRes - seg->surface->kRollRes*1.2)*4);

		if (count)
		{
			if (mode != mode_normal &&
			    ((seg->type == TR_RGT && seg->radius <= 200.0f && car->_trkPos.toLeft < 3.0f) ||
			    (seg->type == TR_LFT && seg->radius <= 200.0f && car->_trkPos.toRight < 3.0f)))
				count++;
			accel1 = (float) MAX(0.0f, MIN(accel1, (1.0f-(0.25f*count)) - MAX(0.0f, (getSpeed()-car->_speed_x)/10.0f)));
		}

		if (fabs(angle) > 1.0)
	 		accel1 = (float) MIN(accel1, 1.0f - (fabs(angle)-1.0)*1.3);
	}

	if (fabs(car->_steerCmd) > 0.02 && TurnDecel+rldata->decel > 0.0)
	{
		float decel = (float) ((fabs(car->_steerCmd)-0.02f) * (1.0f+fabs(car->_steerCmd)) * 0.7f);
		decel *= (float) (TurnDecel+rldata->decel);
		if (mode != mode_normal)
		{
			decel *= 1.0 + rldata->adecel;
		}
		accel2 = (float) MIN(accel2, MAX(0.55f, 1.0f-decel));
	}

	float slip = (this->*GET_DRIVEN_WHEEL_SPEED)() - fabs(car->_speed_x);
	if (slip > TclSlip) {
		accel3 = accel3 - MIN(accel3, (slip - TclSlip)/TCL_RANGE);
	}

	accel = MAX(accel/4, MIN(accel1, MIN(accel2, accel3)));
	if (accel > 0.9f)
		accel = 1.0f;

	if (faccelcmd > 0.0f)
		accel = MIN(accel, faccelcmd * 1.2f);

	return accel;
}


// Traction Control (TCL) setup.
void Driver::initTCLfilter()
{
	char *traintype = GfParmGetStr(car->_carHandle, SECT_DRIVETRAIN, PRM_TYPE, VAL_TRANS_RWD);
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
		car->_trkPos.toMiddle*speedangle > 0.0f)	// Speedvector points to the inside of the turn.
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
	return (-log((c + v2sqr*d)/(c + v1sqr*d))/(2.0f*d));
}

