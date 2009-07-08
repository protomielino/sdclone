/***************************************************************************

    file        : MyRobot.cpp
    created     : 9 Apr 2006
    copyright   : (C) 2006 Tim Foden

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// MyRobot.cpp: implementation of the MyRobot class.
//
//////////////////////////////////////////////////////////////////////

#include "MyRobot.h"
#include "Quadratic.h"
#include "Utils.h"
#include "Avoidance.h"
#include "GenericAvoidance.h"
#include "InsideLineAvoidance.h"
#include "OutsideLineAvoidance.h"
#include "BendAnalysis.h"
#include "Linalg.h"

#include <portability.h>
#include <robottools.h>

#define ROBOT_NAME "hymie_36GP"

#define	USE_NEW_AVOIDANCE
//#define	USE_NEW_AVOIDANCE_GFOUT
#if defined(USE_NEW_AVOIDANCE_GFOUT)
#define	NAOUT	GfOut
#else
#define	NAOUT
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#define SECT_PRIV			"hymie private"
#define PRV_GEAR_UP_RPM		"gear up rpm"
#define PRV_MU_SCALE		"mu scale"
#define PRV_AVOID_MU_SCALE	"avoid mu scale"
#define PRV_OT_SCALE		"overtake scale"
#define PRV_DIV_BEGIN		"begin div"
#define PRV_DIV_END		"end div"
#define PRV_OT_MOD		"overtake mod"
#define PRV_BRK_MU_SCALE	"brk mu scale"
#define PRV_AVOID_SCALE 	"avoid scale"
#define PRV_SKID_STEER	 	"skidsteer"
#define PRV_BRK_SCALE		"brk scale"
#define PRV_BRK_FACTOR		"brk factor"
#define PRV_MU_FACTOR		"mu factor"
#define PRV_KZ_FACTOR		"kz factor"
#define PRV_AVOID_BRK_SCALE	"avoid brk scale"
#define PRV_BRAKE_PRESSURE	"brk pressure"
#define PRV_ACC_SCALE		"acc scale"
#define PRV_FLY_HEIGHT		"fly height"
#define PRV_FACTOR		"factor"
#define PRV_AERO_MOD		"aero mod"
#define PRV_BRAKE_MOD		"brake mod"
#define PRV_BUMP_MOD		"bump mod"
#define PRV_SIDE_MOD		"side mod"
#define PRV_STEER_MOD		"steer mod"
#define PRV_KZ_SCALE		"kz scale"
#define PRV_STEER_K_ACC		"steer k acc"
#define PRV_STEER_K_DEC		"steer k dec"
#define PRV_AVOID_WIDTH		"avoid width"
#define PRV_SPDC_NORMAL		"spd ctrl normal"
#define PRV_SPDC_TRAFFIC	"spd ctrl traffic"
#define PRV_STAY_TOGETHER	"stay together"
#define PRV_PIT_ENTRY_OFFS	"pit entry offset"
#define PRV_PIT_EXIT_OFFS	"pit exit offset"
#define PRV_AHEAD_FACTOR	"ahead factor"
#define PRV_LEFT_MARGIN		"left margin"
#define PRV_RIGHT_MARGIN	"right margin"
#define PRV_RI_CUTOFF		"curv cutoff"
#define PRV_SPEED_LIMIT		"speed limit"
#define PRV_VERBOSE		"verbose"
#define PRV_MAX_FUEL		"max fuel"
#define PRV_YR_ACCEL		"yr accel"

#define FLY_COUNT		20

#define	STEER_SPD_IDX(x)	(int(floor((x) / 5)))
#define	STEER_K_IDX(k)		(MX(0, MN(int(20 + floor((k) * 500 + 0.5)), 40)))

#define SETSPEED0  \
	double	spd0 = hypot(car->_speed_x, car->_speed_y);\
	if( !m_pitControl.WantToPit() ) { \
		if (STEER_MOD == 1) \
			spd0 = (m_realSpeed+car->_speed_x)/2; \
		else if (STEER_MOD == 2) \
			spd0 = car->_speed_x;  \
	}

static const double	s_sgMin[] = { 0.00,  10 };
static const double	s_sgMax[] = { 0.03, 100 };
static const int	s_sgSteps[] = { 10,  18 };

void	MyRobot::PathRange::AddGreater(
	double pos,
	double offs,
	bool incRL,
	const MyRobot& me )
{
/*	double	pu, pv;
	me.CalcBestPathUV( pos, offs, pu, pv );
	if( !gotL )
	{
		u  = pu;
		vL = pv;
	}
	else
	{
		if( u != pu )
		{
			if( u < pu )
			{
				// need to re-calc pv using u;
				pv = rgtT = me.CalcPathTarget(pos, offs, u);
				if( vL < pv )
				{
					vL = pv;
				}
			}
			else
			{
				// need to re-calc vL using pu;
				double vL2 = me.CalcPathTarget(pos, offs, pu);
				if( vL2 < pv )
				{
					u  = pu;
					vL = pv;
				}
			}
		}
		else if( vL < pv )
		{
			vL = pv;
		}
	}*/
}

void	MyRobot::PathRange::AddLesser(
	double pos,
	double offs,
	bool incRL,
	const MyRobot& me )
{
}

MyRobot::MyRobot()
:	m_pitControl(m_track, m_pitPath[PATH_NORMAL]),
	m_driveType(cDT_RWD),
	m_gearUpRpm(8000),
	m_prevYawError(0),
	m_prevLineError(0),
	m_flying(0),
	m_avgAY(0),
	m_raceStart(false),
	m_avoidS(1),
	m_avoidT(0),
	m_followPath(PATH_NORMAL),
	m_verbose(0),
	m_lastB(0),
	m_lastBrk(0),
	m_lastTargV(0),
	m_steersum(0),
	m_maxAccel(0, 150, 30, 1),
	m_steerGraph(2, s_sgMin, s_sgMax, s_sgSteps, 0),
	m_steerAvg(19, 0.001, 0.02, 15, 20, 95)
{
	{for( int i = 0; i < 50; i++ )
	{
		m_brkCoeff[i] = 0.5;//0.12;
	}}

	{for( int i = 0; i < STEER_SPD_MAX; i++ )
	{
		for( int j = 0; j < STEER_K_MAX; j++ )
		{
			m_steerCoeff[i][j] = 0;
		}
	}}

	memset( m_angle, 0, sizeof(m_angle) );
}

MyRobot::~MyRobot()
{
}

static void*	MergeParamFile( void* hParams, const char* fileName )
{
//	GfOut( "merging: '%s'\n", fileName );
	void*	hNewParams = GfParmReadFile(fileName, GFPARM_RMODE_STD);
	if( hNewParams == NULL )
		return hParams;

	if( hParams == NULL )
		return hNewParams;

	return GfParmMergeHandles(hParams, hNewParams,
				GFPARM_MMODE_SRC    | GFPARM_MMODE_DST |
				GFPARM_MMODE_RELSRC | GFPARM_MMODE_RELDST);
}

void	MyRobot::SetShared( Shared* pShared )
{
	m_pShared = pShared;
}

// Called for every track change or new race.
void	MyRobot::InitTrack(
	int			index,
	tTrack*		pTrack,
	void*		pCarHandle,
	void**		ppCarParmHandle,
	tSituation*	pS )
{
//	GfOut( "hymie:initTrack()\n" );

	//
	//	get the name of the car (e.g. "clkdtm").
	//
	//	this is a sucky way to get this, but it's the only way I've managed
	//	to come up with so far.  it basically gets the name of the car .acc
	//	file, and strips off the ".acc" part, to get the name of the car.  yuk.
	//

	char*	path = SECT_GROBJECTS "/" LST_RANGES "/" "1";
	char*	key  = PRM_CAR;
	char	carName[256];
	strncpy( carName, GfParmGetStr(pCarHandle, path, key, ""), sizeof(carName) );
	char*	p = strrchr(carName, '.');
	if( p )
		*p = '\0';
//	GfOut( "   carName: '%s'\n", carName );

	//
	//	get the name of the track (e.g. "e-track-1")
	//

	char	trackName[256];
	strncpy( trackName, strrchr(pTrack->filename, '/') + 1, sizeof(trackName) );
	*strrchr(trackName, '.') = '\0';

	//
	//	set up race type array.
	//

	char*	raceType[] = { "practice", "qualify", "race" };

	//
	//	set up the base param path.
	//

	char	baseParamPath[32];
	snprintf(baseParamPath, 32-1, "drivers/%s", ROBOT_NAME);

	//
	//	ok, lets read/merge the car parms.
	//

	void*	hCarParm = 0;
	char	buf[1024];

	// default params for car type (e.g. clkdtm)
    snprintf( buf, sizeof(buf), "%s/cars/%s/default.xml",
				baseParamPath, carName );
	hCarParm = MergeParamFile(hCarParm, buf);

	// override params for car type on track.
    snprintf( buf, sizeof(buf), "%s/cars/%s/%s.xml",
				baseParamPath, carName, trackName );
	hCarParm = MergeParamFile(hCarParm, buf);

	// override params for car type on track of specific race type.
    snprintf( buf, sizeof(buf), "%s/cars/%s/%s/%s.xml",
				baseParamPath, carName, raceType[pS->_raceType], trackName );
	hCarParm = MergeParamFile(hCarParm, buf);

	// setup the car param handle to be returned.

	*ppCarParmHandle = hCarParm;

	m_skill = -1.0;
	m_aggression = 0.0;
	{
		// read the skill parameters
		snprintf(buf, 1024-1, "config/raceman/extra/skill.xml");
		void *skillHandle = GfParmReadFile(buf, GFPARM_RMODE_REREAD);

		if (skillHandle)
		{
			double driver_skill = 0.0;
			double global_skill = GfParmGetNum(skillHandle, (char *)"skill", (char *)"level", (char *)NULL, -1.0);
			global_skill = MAX(0.0f, MIN(10.0f, global_skill));

			snprintf(buf, 1024-1, "drivers/%s/%d/skill.xml", ROBOT_NAME, index);
			skillHandle = GfParmReadFile(buf, GFPARM_RMODE_STD);

			if (skillHandle)
			{
				driver_skill = GfParmGetNum(skillHandle, (char *)"skill", (char *)"level", (char *)NULL, 0.0);
				m_aggression = GfParmGetNum(skillHandle, (char *)"skill", (char *)"aggression", (char *)NULL, 0.0);
				driver_skill = MAX(0.0f, MIN(1.0f, driver_skill));
			}

			m_skill = (global_skill + driver_skill*2) * (1.0 + driver_skill);
		}
	}

	// get the private parameters now.

	double rpm = GfParmGetNum(hCarParm, SECT_PRIV, PRV_GEAR_UP_RPM, NULL, 7000);
	m_gearUpRpm = rpm / 10.0;
//	GfOut( "*** Mid rpm: %.0f\n", rpm );

	double kz_factor = GfParmGetNum(hCarParm, SECT_PRIV, PRV_KZ_FACTOR, NULL, 1.00f);
	double mu_factor = GfParmGetNum(hCarParm, SECT_PRIV, PRV_MU_FACTOR, NULL, 1.00f);
	double brk_factor = GfParmGetNum(hCarParm, SECT_PRIV, PRV_BRK_FACTOR, NULL, 1.00f);

	m_cm.AERO = (int)GfParmGetNum(hCarParm, SECT_PRIV, PRV_AERO_MOD, 0, 0);
	m_cm.BRAKE_MOD = (int)GfParmGetNum(hCarParm, SECT_PRIV, PRV_BRAKE_MOD, 0, 0);
	m_cm.MU_SCALE = GfParmGetNum(hCarParm, SECT_PRIV, PRV_MU_SCALE, NULL, 0.9f);
	m_cm.BRK_MU_SCALE = GfParmGetNum(hCarParm, SECT_PRIV, PRV_BRK_MU_SCALE, NULL, 0.9f);
	m_cm.KZ_SCALE = GfParmGetNum(hCarParm, SECT_PRIV, PRV_KZ_SCALE, NULL, 0.43f);
	m_cm.BRK_SCALE = GfParmGetNum(hCarParm, SECT_PRIV, PRV_BRK_SCALE, NULL, 1.00f);
	m_cm.AHEAD_FACTOR = GfParmGetNum(hCarParm, SECT_PRIV, PRV_AHEAD_FACTOR, NULL, 1.00f);
	m_cm.LFT_MARGIN = GfParmGetNum(hCarParm, SECT_PRIV, PRV_LEFT_MARGIN, NULL, 0.00f);
	m_cm.RGT_MARGIN = GfParmGetNum(hCarParm, SECT_PRIV, PRV_RIGHT_MARGIN, NULL, 0.00f);
	m_cm.RI_CUTOFF = GfParmGetNum(hCarParm, SECT_PRIV, PRV_RI_CUTOFF, NULL, 0.0f);
	m_cm.YR_ACCEL = GfParmGetNum(hCarParm, SECT_PRIV, PRV_YR_ACCEL, NULL, 0.0f);

	if (m_skill > 0.0)
	{
		m_cm.LFT_MARGIN = MAX(m_cm.LFT_MARGIN, m_skill/12);
		m_cm.RGT_MARGIN = MAX(m_cm.RGT_MARGIN, m_skill/12);
		m_cm.MU_SCALE -= m_skill/90;
	}

	FACTORS.RemoveAll();
	{for( int i = 0; ; i++ )
	{
		snprintf( buf, sizeof(buf), "%s %d", PRV_FACTOR, i );
		double	factor = GfParmGetNum(hCarParm, SECT_PRIV, buf, 0, -1);
		GfOut( "FACTOR %g\n", factor );
		if( factor == -1 )
			break;
		FACTORS.Add( factor );
	}}

	if( FACTORS.GetSize() == 0 )
		FACTORS.Add( 1.005 );


	FLY_HEIGHT = GfParmGetNum(hCarParm, SECT_PRIV, PRV_FLY_HEIGHT, "m", 0.15f);
	BUMP_MOD = int(GfParmGetNum(hCarParm, SECT_PRIV, PRV_BUMP_MOD, 0, 0));
	STEER_MOD = int(GfParmGetNum(hCarParm, SECT_PRIV, PRV_STEER_MOD, 0, 0));
	SPDC_NORMAL = int(GfParmGetNum(hCarParm, SECT_PRIV, PRV_SPDC_NORMAL, 0, 2));
//	SPDC_NORMAL = 4;
	SPDC_TRAFFIC = int(GfParmGetNum(hCarParm, SECT_PRIV, PRV_SPDC_TRAFFIC, 0, 2));
	AVOID_WIDTH = GfParmGetNum(hCarParm, SECT_PRIV, PRV_AVOID_WIDTH, 0, 0.5);
	STAY_TOGETHER = GfParmGetNum(hCarParm, SECT_PRIV, PRV_STAY_TOGETHER, 0, 0);
	STEER_K_ACC = GfParmGetNum(hCarParm, SECT_PRIV, PRV_STEER_K_ACC, 0, 0);
	STEER_K_DEC = GfParmGetNum(hCarParm, SECT_PRIV, PRV_STEER_K_DEC, 0, 0);
	PIT_ENTRY_OFFSET = GfParmGetNum(hCarParm, SECT_PRIV, PRV_PIT_ENTRY_OFFS, 0, 0);
	PIT_EXIT_OFFSET = GfParmGetNum(hCarParm, SECT_PRIV, PRV_PIT_EXIT_OFFS, 0, 0);
	ACC_SCALE = GfParmGetNum(hCarParm, SECT_PRIV, PRV_ACC_SCALE, 0, 1);
	OT_MOD = int(GfParmGetNum(hCarParm, SECT_PRIV, PRV_OT_MOD, 0, 0));
	m_verbose = int(GfParmGetNum(hCarParm, SECT_PRIV, PRV_VERBOSE, 0, 0));

	BRAKE_PRESSURE = GfParmGetNum(hCarParm, SECT_PRIV, PRV_BRAKE_PRESSURE, 0, 1.0);
	SKIDSTEER = GfParmGetNum(hCarParm, SECT_PRIV, PRV_SKID_STEER, 0, 0.7);
	AVOIDMOVT = GfParmGetNum(hCarParm, SECT_PRIV, PRV_AVOID_SCALE, 0, 0.5);

	m_cm.m_pmused = 0;
	{for (int i=0; i<64; i++)
	{
		char tmpstr[64];
		sprintf(tmpstr, "%d %s", i, PRV_DIV_BEGIN);
		int bgn_div = int(GfParmGetNum(hCarParm, SECT_PRIV, tmpstr, 0, -1.0));

		if (bgn_div >= 0)
		{
			m_cm.m_pm[m_cm.m_pmused].bgn_div = bgn_div;
			sprintf(tmpstr, "%d %s", i, PRV_DIV_END);
			m_cm.m_pm[m_cm.m_pmused].end_div = int(GfParmGetNum(hCarParm, SECT_PRIV, tmpstr, 0, -1.0));
			sprintf(tmpstr, "%d %s", i, PRV_AERO_MOD);
			m_cm.m_pm[m_cm.m_pmused].AERO = int(GfParmGetNum(hCarParm, SECT_PRIV, tmpstr, 0, (double)m_cm.AERO));
			sprintf(tmpstr, "%d %s", i, PRV_AHEAD_FACTOR);
			m_cm.m_pm[m_cm.m_pmused].AHEAD_FACTOR = int(GfParmGetNum(hCarParm, SECT_PRIV, tmpstr, 0, (double)m_cm.AHEAD_FACTOR));
			sprintf(tmpstr, "%d %s", i, PRV_BRAKE_MOD);
			m_cm.m_pm[m_cm.m_pmused].BRAKE_MOD = int(GfParmGetNum(hCarParm, SECT_PRIV, tmpstr, 0, (double)m_cm.BRAKE_MOD));
			sprintf(tmpstr, "%d %s", i, PRV_MU_SCALE);
			m_cm.m_pm[m_cm.m_pmused].MU_SCALE = GfParmGetNum(hCarParm, SECT_PRIV, tmpstr, 0, m_cm.MU_SCALE) * mu_factor;
			sprintf(tmpstr, "%d %s", i, PRV_BRK_MU_SCALE);
			m_cm.m_pm[m_cm.m_pmused].BRK_MU_SCALE = GfParmGetNum(hCarParm, SECT_PRIV, tmpstr, 0, m_cm.BRK_MU_SCALE) * brk_factor;
			sprintf(tmpstr, "%d %s", i, PRV_KZ_SCALE);
			m_cm.m_pm[m_cm.m_pmused].KZ_SCALE = GfParmGetNum(hCarParm, SECT_PRIV, tmpstr, 0, m_cm.KZ_SCALE) * kz_factor;
			sprintf(tmpstr, "%d %s", i, PRV_BRK_SCALE);
			m_cm.m_pm[m_cm.m_pmused].BRK_SCALE = GfParmGetNum(hCarParm, SECT_PRIV, tmpstr, 0, m_cm.BRK_SCALE);
			sprintf(tmpstr, "%d %s", i, PRV_LEFT_MARGIN);
			m_cm.m_pm[m_cm.m_pmused].LFT_MARGIN = GfParmGetNum(hCarParm, SECT_PRIV, tmpstr, 0, m_cm.LFT_MARGIN);
			sprintf(tmpstr, "%d %s", i, PRV_RIGHT_MARGIN);
			m_cm.m_pm[m_cm.m_pmused].RGT_MARGIN = GfParmGetNum(hCarParm, SECT_PRIV, tmpstr, 0, m_cm.RGT_MARGIN);
			sprintf(tmpstr, "%d %s", i, PRV_SPEED_LIMIT);
			m_cm.m_pm[m_cm.m_pmused].SPEED_LIMIT = GfParmGetNum(hCarParm, SECT_PRIV, tmpstr, 0, -1.0f);
			sprintf(tmpstr, "%d %s", i, PRV_YR_ACCEL);
			m_cm.m_pm[m_cm.m_pmused].YR_ACCEL = GfParmGetNum(hCarParm, SECT_PRIV, tmpstr, 0, -1.0f);

			m_cm.m_pmused++;
		}
	}}

	m_cm.KZ_SCALE *= kz_factor;
	m_cm.MU_SCALE *= mu_factor;
	m_cm.BRK_SCALE *= brk_factor;

	GfOut( "FLY_HEIGHT %g\n", FLY_HEIGHT );
	GfOut( "BUMP_MOD %d\n", BUMP_MOD );

	MyTrack::SideMod	sideMod;
	sideMod.side = -1;
	const char*	pStr = GfParmGetStr(hCarParm, SECT_PRIV, PRV_SIDE_MOD, "");
	if( pStr == 0 ||
		sscanf(pStr, "%d , %d , %d",
				&sideMod.side, &sideMod.start, &sideMod.end) != 3 )
	{
		sideMod.side = -1;
	}

	GfOut( "SIDE MOD %d %d %d\n", sideMod.side, sideMod.start, sideMod.end );
	GfOut( "STAY_TOGETHER %g\n", STAY_TOGETHER );

	m_track.NewTrack( pTrack, &m_cm, false, &sideMod );

	// setup initial fuel for race.
	double	fuelPerM = 0.001;
	double	maxFuel = GfParmGetNum(hCarParm, SECT_CAR, PRM_TANK, (char*) NULL, 100.0);
	double	fuel = MN(pS->_totLaps * pTrack->length * fuelPerM, maxFuel);
	double  max_set_fuel = GfParmGetNum(hCarParm, SECT_PRIV, PRV_MAX_FUEL, (char *) NULL, fuel);
	fuel = MN(fuel, max_set_fuel);
//	if( pS->_raceType != RM_TYPE_PRACTICE )
		GfParmSetNum( hCarParm, SECT_CAR, PRM_FUEL, (char*) NULL, fuel );
}

// Start a new race.
void	MyRobot::NewRace( int index, tCarElt* pCar, tSituation* pS )
{
//	GfOut( "hymie:newRace()\n" );

//	if( pS == 0 )
//		GfOut( "!!!!! situation ptr is null !!!!!\n" );
	m_nCars = pS->_ncars;
	m_myOppIdx = -1;
    double otscale = GfParmGetNum(pCar->_carHandle, SECT_PRIV, PRV_OT_SCALE, 0, 1.0);
	{for( int i = 0; i < m_nCars; i++ )
	{
//		if( pS->cars[i] == 0 )
//			GfOut( "!!!!! car ptr is null !!!!!\n" );
//		m_oppPath[i].Initialise( &m_track, pS->cars[i] );
//		m_opp[i].Initialise( &m_track, pS->cars[i] );
		m_opp[i].Initialise( &m_track, pS->cars[i] );
		if( pS->cars[i] == pCar )
			m_myOppIdx = i;
		m_opp[i].SetOTScale( otscale );
	}}

	SetRandomSeed((index + 1));

	m_cm.MASS = GfParmGetNum(pCar->_carHandle, SECT_CAR, PRM_MASS, NULL, 1000.0);

    float fwingarea = GfParmGetNum(pCar->_carHandle, SECT_FRNTWING,
                                       PRM_WINGAREA, (char*) NULL, 0.0);
    float fwingangle = GfParmGetNum(pCar->_carHandle, SECT_FRNTWING,
                                        PRM_WINGANGLE, (char*) NULL, 0.0);
    float rwingarea = GfParmGetNum(pCar->_carHandle, SECT_REARWING,
                                       PRM_WINGAREA, (char*) NULL, 0.0);
    float rwingangle = GfParmGetNum(pCar->_carHandle, SECT_REARWING,
                                        PRM_WINGANGLE, (char*) NULL, 0.0);
	float fwingArea = rwingarea * sin(rwingangle);
	float rwingArea = fwingarea * sin(fwingangle);
    float wingca = 1.23 * (fwingArea + rwingArea);

    float cl = GfParmGetNum(pCar->_carHandle, SECT_AERODYNAMICS,
                             PRM_FCL, (char*) NULL, 0.0) +
                GfParmGetNum(pCar->_carHandle, SECT_AERODYNAMICS,
                             PRM_RCL, (char*) NULL, 0.0);
    float h = 0.0;
    char *WheelSect[4] = {SECT_FRNTRGTWHEEL, SECT_FRNTLFTWHEEL,
                          SECT_REARRGTWHEEL, SECT_REARLFTWHEEL};
    {for( int i = 0; i < 4; i++ )
	{
        h += GfParmGetNum(pCar->_carHandle, WheelSect[i],
                          PRM_RIDEHEIGHT, (char*) NULL, 0.20f);
	}}
    h*= 1.5; h = h*h; h = h*h; h = 2.0 * exp(-3.0*h);
    m_cm.CA = h*cl + 4.0*wingca;
	m_cm.CA_FW = 4 * 1.23 * fwingArea;
	m_cm.CA_RW = 4 * 1.23 * rwingArea;
	m_cm.CA_GE = h * cl;

	GfOut( "CA %g   CA_FW %g   CA_RW %g   CA_GE %g\n",
			m_cm.CA, m_cm.CA_FW, m_cm.CA_RW, m_cm.CA_GE );

	double	cx = GfParmGetNum(pCar->_carHandle, SECT_AERODYNAMICS, PRM_CX, (char*)NULL, 0.0);
	double	frontArea = GfParmGetNum(pCar->_carHandle, SECT_AERODYNAMICS, PRM_FRNTAREA, (char*)NULL, 0.0);

	m_cm.TYRE_MU   = 9999;
	m_cm.TYRE_MU_F = 9999;
	m_cm.TYRE_MU_R = 9999;
	{for( int i = 0; i < 4; i++ )
	{
		double	mu = GfParmGetNum(pCar->_carHandle, WheelSect[i],
								  PRM_MU, (char*)NULL, 1.0);
		m_cm.TYRE_MU = MN(m_cm.TYRE_MU, mu);
		if( i < 2 )
			m_cm.TYRE_MU_F = MN(m_cm.TYRE_MU_F, mu);
		else
			m_cm.TYRE_MU_R = MN(m_cm.TYRE_MU_R, mu);
	}}

	GfOut( "CARMASS %g   TYRE_MU %g   TYRE_MU_F %g   TYRE_MU_R %g \n",
			m_cm.MASS, m_cm.TYRE_MU, m_cm.TYRE_MU_F, m_cm.TYRE_MU_R );
	GfOut( "MU_SC %g   KZ_SCALE %g   FLY_HEIGHT %g\n",
			m_cm.MU_SCALE, m_cm.KZ_SCALE, FLY_HEIGHT );

	char*	pTrackName = strrchr(m_track.GetTrack()->filename, '/') + 1;
	char	buf[1024];
//	sprintf( buf, "drivers/hymie/%s.path", pTrackName );
//	sprintf( buf, "drivers/hymie/%s.path", m_track.GetTrack()->name );
	sprintf( buf, "drivers/%s/%s.spr", ROBOT_NAME, pTrackName );

//	m_track.CalcMaxSpeeds( CARMASS, CA, TYRE_MU, MU_SCALE, MU_DF_SCALE, pCar->_fuel );
//	m_track.PropagateBreaking();

	m_cm.FUEL = 0;//pCar->_fuel;
	m_cm.CD_BODY = 0.645 * cx * frontArea;
	m_cm.CD_WING = wingca;
	m_cm.WIDTH = pCar->_dimension_y;
//	cm.CD = cx * (frontArea + wingArea);
//	cm.CD = cx * frontArea + wingca;
//	m_cm.CD = 0.645 * cx * frontArea;
	m_cm2 = m_cm;
	m_cm2.BRK_SCALE = GfParmGetNum(pCar->_carHandle, SECT_PRIV, PRV_AVOID_BRK_SCALE, NULL, 0.9f);
	m_cm2.MU_SCALE = MN(1.0, m_cm.MU_SCALE);
	{ for (int i=0; i<m_cm2.m_pmused; i++)
	{
		char tmpstr[64];
		sprintf(tmpstr, "%d %s", i, PRV_AVOID_MU_SCALE);
		double avoid_mu_scale = GfParmGetNum(pCar->_carHandle, SECT_PRIV, tmpstr, 0, 0.0);
		sprintf(tmpstr, "%d %s", i, PRV_AVOID_BRK_SCALE);
		double avoid_brk_scale = GfParmGetNum(pCar->_carHandle, SECT_PRIV, tmpstr, 0, 0.0);

		if (avoid_mu_scale > 0.1)
			m_cm2.m_pm[i].MU_SCALE = avoid_mu_scale;
		else
			m_cm2.m_pm[i].MU_SCALE = MN(1.0, m_cm.m_pm[i].MU_SCALE);

		if (avoid_brk_scale > 0.1)
			m_cm2.m_pm[i].BRK_SCALE = avoid_brk_scale;
		else
			m_cm2.m_pm[i].BRK_SCALE = m_cm2.BRK_SCALE;

		m_cm2.m_pm[i].AHEAD_FACTOR = MN(10, m_cm.m_pm[i].AHEAD_FACTOR/2);
	}}

//	GfOut( "CD %g   wing CA %g\n", m_cm.CD_BODY, wingca );
	GfOut( "WING F %g    WING R %g   SPDC N %d    SPDC T %d\n",
			fwingangle * 180 / PI, rwingangle * 180 / PI, SPDC_NORMAL, SPDC_TRAFFIC );

#if 0
	bool loadpath = (bool) GfParmGetNum(pCar->_carHandle, SECT_PRIV, "loadpath", 0, 0);
	bool savepath = (bool) GfParmGetNum(pCar->_carHandle, SECT_PRIV, "savepath", 0, 0);

	if (loadpath)
	{
		if( m_pShared->m_path[PATH_NORMAL].GetFactors() != FACTORS ||
			m_pShared->m_pTrack != m_track.GetTrack() )
		{
			if( m_pShared->m_pTrack != m_track.GetTrack() )
			{
				m_pShared->m_pTrack = m_track.GetTrack();
				m_pShared->m_teamInfo.Empty();
			}
		}

		m_pShared->m_path[PATH_NORMAL].SetFactors( FACTORS );
		m_pShared->m_path[PATH_LEFT].SetFactors( FACTORS );
		m_pShared->m_path[PATH_RIGHT].SetFactors( FACTORS );

		double w = AVOID_WIDTH;

		// load the path from file(s)
		if (!(m_pShared->m_path[PATH_NORMAL].LoadPath( &m_track, pCar->_carHandle, 0 )))
		{
			loadpath = false;
			goto loadpathend;
		}

		if (!(m_pShared->m_path[PATH_LEFT].LoadPath( &m_track, pCar->_carHandle, 1 )))
		{
			loadpath = false;
			goto loadpathend;
		}

		if (!(m_pShared->m_path[PATH_RIGHT].LoadPath( &m_track, pCar->_carHandle, 2 )))
		{
			loadpath = false;
			goto loadpathend;
		}
	}

loadpathend:
	if (!loadpath)
#endif
	{
		// generate the path

		if( m_pShared->m_path[PATH_NORMAL].GetFactors() != FACTORS ||
			m_pShared->m_pTrack != m_track.GetTrack() )
		{
			if( m_pShared->m_pTrack != m_track.GetTrack() )
			{
				m_pShared->m_pTrack = m_track.GetTrack();
				m_pShared->m_teamInfo.Empty();
			}

//			GfOut( "Generating smooth paths...\n" );
//			double	w = 0.5;//m_track.GetWidth() / 6;
//			w = MX(0.5, m_track.GetWidth() / 2 - 3);
			double	w = AVOID_WIDTH;
#if defined(USE_NEW_AVOIDANCE)
//			w = (m_track.GetWidth() / 2 - 2) * 0.8;
#endif
	
			m_pShared->m_path[PATH_NORMAL].SetFactors( FACTORS );
			m_pShared->m_path[PATH_NORMAL].MakeSmoothPath( &m_track, m_cm,
			                                               ClothoidPath::Options(BUMP_MOD) );
			m_pShared->m_path[PATH_LEFT].SetFactors( FACTORS );
			m_pShared->m_path[PATH_LEFT].MakeSmoothPath( &m_track, m_cm2,
			                                               ClothoidPath::Options(BUMP_MOD, 999, -w),
								       (pS->_raceType != RM_TYPE_RACE ? 2 : 32) );
			m_pShared->m_path[PATH_RIGHT].SetFactors( FACTORS );
			m_pShared->m_path[PATH_RIGHT].MakeSmoothPath( &m_track, m_cm2,
			                                               ClothoidPath::Options(BUMP_MOD, -w, 999),
								       (pS->_raceType != RM_TYPE_RACE ? 2 : 32) );
		}
	}

//	BendAnalysis	ba(m_pShared->m_path[PATH_NORMAL], m_track);

//	m_path[PATH_NORMAL].MakeSmoothPath( &m_track );
	m_path[PATH_NORMAL] = m_pShared->m_path[PATH_NORMAL];
//	m_path[PATH_NORMAL].Initialise( &m_track, m_cm, 999, 999 );
//	if( index == 0 )
//		m_path[PATH_NORMAL].LoadPath( buf );
	m_path[PATH_NORMAL].CalcMaxSpeeds( m_cm );
	m_path[PATH_NORMAL].PropagateBreaking( m_cm );

//	m_path[PATH_LEFT].MakeSmoothPath( &m_track, 999, -0.5 );
	m_path[PATH_LEFT] = m_pShared->m_path[PATH_LEFT];
	m_path[PATH_LEFT].CalcMaxSpeeds( m_cm2, 2 );
	m_path[PATH_LEFT].PropagateBreaking( m_cm2, 2 );

//	m_path[PATH_RIGHT].MakeSmoothPath( &m_track, -0.5, 999 );
	m_path[PATH_RIGHT] = m_pShared->m_path[PATH_RIGHT];
	m_path[PATH_RIGHT].CalcMaxSpeeds( m_cm2, 2 );
	m_path[PATH_RIGHT].PropagateBreaking( m_cm2, 2 );

#if 0
	if (!loadpath && savepath)
	{
		m_path[PATH_NORMAL].SavePath( pCar->_carHandle, 0 );
		m_path[PATH_LEFT].SavePath( pCar->_carHandle, 1 );
		m_path[PATH_RIGHT].SavePath( pCar->_carHandle, 2 );
	}
#endif

	m_pitPath[PATH_NORMAL].MakePath( pCar, &m_path[PATH_NORMAL], m_cm,
										PIT_ENTRY_OFFSET, PIT_EXIT_OFFSET );
	m_pitPath[PATH_LEFT].  MakePath( pCar, &m_path[PATH_LEFT],   m_cm2,
										PIT_ENTRY_OFFSET, PIT_EXIT_OFFSET );
	m_pitPath[PATH_RIGHT]. MakePath( pCar, &m_path[PATH_RIGHT],  m_cm2,
										PIT_ENTRY_OFFSET, PIT_EXIT_OFFSET );
/*
	{
		bool	same = true;
		int		via  = -1;
		const int	NSEG = m_track.GetSize();
		{for( int i = 0; i < NSEG; i++ )
		{
			const LinePath::PathPt&	n = m_path[PATH_NORMAL].GetAt(i);
			const LinePath::PathPt&	l = m_path[PATH_LEFT].GetAt(i);
			bool	newSame = fabs(n.offs - l.offs) < 0.25;
			if( same & !newSame )
			{
				via = i;
				GfOut( "%4d  VIA L\n", i );
			}
			else if( !same && newSame )
			{
				GfOut( "%4d  JOIN L     %.4f    %.4f\n", i,
						m_path[PATH_NORMAL].CalcEstimatedTime(via, i - via),
						m_path[PATH_LEFT].CalcEstimatedTime(via, i - via) );
			}
			same = newSame;
		}}
	}
*/
//	double	lapTime = m_path[PATH_NORMAL].CalcEstimatedLapTime();
//	GfOut( "Estimated lap time (%g): %02d:%02d.%03d\n",
//		lapTime, int(floor(lapTime / 60)), int(floor(lapTime)) % 60,
//		int((lapTime - floor(lapTime)) * 1000) );

    char* traintype = (char *)GfParmGetStr(pCar->_carHandle,
							SECT_DRIVETRAIN, PRM_TYPE, VAL_TRANS_RWD);
	m_driveType = cDT_RWD;
    if( strcmp(traintype, VAL_TRANS_RWD) == 0 )
        m_driveType = cDT_RWD;
    else if( strcmp(traintype, VAL_TRANS_FWD) == 0 )
        m_driveType = cDT_FWD;
    else if( strcmp(traintype, VAL_TRANS_4WD) == 0 )
        m_driveType = cDT_4WD;

	m_flying = 0;
	m_raceStart = true;
//	GfOut( "m_avoidS = %g (race start)\n", 0.0 );
	m_avoidS = 0;
	m_avoidT = 0;

	m_accBrkCoeff.Clear();
	m_accBrkCoeff.Sample( 0, 0 );
	m_accBrkCoeff.Sample( 1, 0.5 );

	TeamInfo::Item*	pItem = new TeamInfo::Item();
	pItem->index = pCar->index;
//	pItem->team = pCar->index / 2;
//	pItem->team = -1;
	pItem->teamName = pCar->_teamname;
	pItem->damage = pCar->_dammage;
	pItem->pitState = TeamInfo::PIT_NOT_SHARED;
	pItem->pOther = 0;
	pItem->pCar = pCar;
	m_pShared->m_teamInfo.Add( pCar->index, pItem );

	m_reverseTime = 0.0;
	m_realSpeed = 0.0;
}

void	MyRobot::GetPtInfo( int path, double pos, PtInfo& pi ) const
{

	if( m_pitControl.WantToPit() && m_pitPath[path].ContainsPos(pos) )
		m_pitPath[path].GetPtInfo( pos, pi );
	else
		m_path[path].GetPtInfo( pos, pi );
}

void	InterpPtInfo( PtInfo& pi0, const PtInfo& pi1, double t )
{
//		pi.k	= piL.k    * (1 - t) + piR.k    * t;
		pi0.k	= Utils::InterpCurvature(pi0.k, pi1.k, t);
		double	deltaOAng = pi1.oang - pi0.oang;
		NORM_PI_PI(deltaOAng);
		pi0.oang = pi0.oang + deltaOAng * t;
		pi0.offs = pi0.offs * (1 - t) + pi1.offs * t;
		pi0.spd	 = pi0.spd  * (1 - t) + pi1.spd  * t;
//		pi.toL
}

void	MyRobot::GetPosInfo(
	double		pos,
	PtInfo&		pi,
	double		u,
	double		v ) const
{
	GetPtInfo( PATH_NORMAL, pos, pi );

	PtInfo	piL, piR;

	if( u != 1 )
	{
		GetPtInfo( PATH_LEFT,  pos, piL );
		GetPtInfo( PATH_RIGHT, pos, piR );

		double	s = u;
		double	t = (v + 1) * 0.5;

		InterpPtInfo( piL, pi, s );
		InterpPtInfo( piR, pi, s );

		pi = piL;

		InterpPtInfo( pi, piR, t );
	}
}

void	MyRobot::GetPosInfo(
	double		pos,
	PtInfo&		pi ) const
{
	GetPosInfo(pos, pi, m_avoidS, m_avoidT);
}

double	MyRobot::CalcPathTarget( double pos, double offs, double s ) const
{
	PtInfo	pi, piL, piR;
	GetPtInfo( PATH_NORMAL, pos, pi );
	GetPtInfo( PATH_LEFT, pos, piL );
	GetPtInfo( PATH_RIGHT, pos, piR );

	InterpPtInfo( piL, pi, s );
	InterpPtInfo( piR, pi, s );

	double	t = (offs - piL.offs) / (piR.offs - piL.offs);

	return MX(-1, MN(t, 1)) * 2 - 1;
}

double	MyRobot::CalcPathTarget( double pos, double offs ) const
{
	return CalcPathTarget(pos, offs, m_avoidS);
}

Vec2d	MyRobot::CalcPathTarget2( double pos, double offs ) const
{
	PtInfo	pi, piL, piR;
	GetPtInfo( PATH_NORMAL, pos, pi );
	GetPtInfo( PATH_LEFT, pos, piL );
	GetPtInfo( PATH_RIGHT, pos, piR );

	double	s = m_avoidS;

	InterpPtInfo( piL, pi, s );
	InterpPtInfo( piR, pi, s );

	double	t = (offs - piL.offs) / (piR.offs - piL.offs);

	return Vec2d(MX(-1, MN(t, 1)) * 2 - 1, 1);
}

double	MyRobot::CalcPathOffset( double pos, double s, double t ) const
{
	PtInfo	pi, piL, piR;
	GetPtInfo( PATH_NORMAL, pos, pi );
	GetPtInfo( PATH_LEFT, pos, piL );
	GetPtInfo( PATH_RIGHT, pos, piR );

	InterpPtInfo( piL, pi, s );
	InterpPtInfo( piR, pi, s );

	InterpPtInfo( piL, piR, (t + 1) * 0.5 );

	return piL.offs;
}

void	MyRobot::CalcBestPathUV( double pos, double offs, double& u, double& v ) const
{
	PtInfo	pi, piL, piR;
	GetPtInfo( PATH_NORMAL, pos, pi );

	if( fabs(offs - pi.offs) < 0.01 )
	{
		u = 1;
		v = 0;
		return;
	}

	GetPtInfo( PATH_LEFT,  pos, piL );
	GetPtInfo( PATH_RIGHT, pos, piR );

	double	doffs = offs - pi.offs;
	if( doffs < 0 )
	{
		double	den = piL.offs - pi.offs;
		if( fabs(den) > 0.001 )
			u = 1 - MN(1, doffs / den);
		else
			u = 0;
		v = -1;
	}
	else
	{
		double	den = piR.offs - pi.offs;
		if( fabs(den) > 0.001 )
			u = 1 - MN(1, doffs / den);
		else
			u = 0;
		v = 1;
	}
//	double	out = CalcPathOffset(pos, u, v);
//	GfOut( "CalcBestPathUV %5.1f:%4.1f  doffs %5.1f  %5.1f/%5.1f/%5.1f  %5.3f %6.3f : %6.3f\n",
//			pos, offs, doffs, piL.offs, pi.offs, piR.offs, u, v, out );
}

double	MyRobot::CalcBestSpeed( double pos, double offs ) const
{
	double	u, v;
	CalcBestPathUV( pos, offs, u, v );

	PtInfo	pi;
	GetPosInfo( pos, pi, u, v );

	return pi.spd;
}

void	MyRobot::GetPathToLeftAndRight( const CarElt* pCar, double& toL, double& toR ) const
{
	double	pos = pCar->_distFromStartLine;
	double	offs = -pCar->_trkPos.toMiddle;

	PtInfo	pi;
	GetPtInfo( PATH_LEFT, pos, pi );
	toL = -(pi.offs - offs);
	GetPtInfo( PATH_RIGHT, pos, pi );
	toR = pi.offs - offs;
}

double	CalcMaxSlip( double fRatio )
{
//	double	stmp = MIN(slip, 1.5f);
//	Bx = wheel->mfB * stmp;
//	F = sin(wheel->mfC * atan(Bx * (1.0f - wheel->mfE) +
//								wheel->mfE * atan(Bx))) *
//			(1.0f + stmp * simSkidFactor[car->carElt->_skillLevel]);
//
//	f = sin(C + atan(Bx * (1 - E) + E * atan(Bx)))
//
//	s = 0, f = 0
//	s = 0.75, f = 0.75
//	s = 1.25, f = 1
//	s = 10, f = 1
//
//	sa = yaw - velAng;
//	slip = tan(sa) * vel;
//	sr = slip / vel == tan(sa) * vel / vel == tan(sa)
	return 0;
}

double	MyRobot::SteerAngle0( tCarElt* car, PtInfo& pi, PtInfo& aheadPi )
{
	// work out current car speed.
	SETSPEED0;

	// get current pos on track.
	double	pos = m_track.CalcPos(car);//, car->_dimension_x * 0.025);
	GetPosInfo( pos, pi );

	// look this far ahead.
	double  aheadFactor = m_cm.AHEAD_FACTOR;


	{for (int i=0; i<m_cm.m_pmused; i++)
	{
		if (pi.idx >= m_cm.m_pm[i].bgn_div && pi.idx <= m_cm.m_pm[i].end_div)
		{
			aheadFactor = m_cm.m_pm[i].AHEAD_FACTOR;
			break;
		}
	}}

	if( m_pitControl.WantToPit() && m_pitPath[PATH_NORMAL].ContainsPos(pos) && car->_speed_x < 15 )
		aheadFactor *= 0.5;

	double	aheadDist = car->_dimension_x * 0.5 + spd0 * 0.02 * aheadFactor;
//	if( m_flying )
//		aheadDist += 10;
	double	aheadPos = m_track.CalcPos(car, aheadDist);

	// get info about pts on track.
	GetPosInfo( aheadPos, aheadPi );

	PtInfo	piOmega;
	double	aheadOmega = car->_dimension_x * 0.5 + spd0 * 0.02;// * 10;
	double	aheadOmegaPos = m_track.CalcPos(car, aheadOmega);
	GetPosInfo( aheadOmegaPos, piOmega );
//	static double avgDelta = 0;

	// work out basic steering angle.
	double	velAng = atan2(car->_speed_Y, car->_speed_X);
	double	angle = aheadPi.oang - car->_yaw;
	double	delta = pi.offs + car->_trkPos.toMiddle;
//	avgDelta = avgDelta * 0.5 + delta * 0.5;
//	delta = avgDelta;
	NORM_PI_PI(angle);
//	angle *= 0.75;
//	if( car->_speed_y * delta > 0 )
//		angle *= 1.01;
//	angle = tanh(2 * angle) * 0.5;

	// control rotational velocity.
//	double	avgK = (pi.k + aheadPi.k) * 0.5;
	double	avgK = (pi.k + piOmega.k) * 0.5;
//	double	avgK = pi.k;
//	double	avgK = pi.k;//(pi.k + aheadPi.k) * 0.5;
//	double	avgK = Utils::InterpCurvatureLin(pi.k, aheadPi.k, 0.5);
//	double	avgK = Utils::InterpCurvatureRad(pi.k, aheadPi.k, 0.5);
	double omega = car->_speed_x * avgK;//aheadPi.k;
	double	o2 = (aheadPi.k - pi.k) * spd0 / aheadDist;
//	if( fabs(pi.k - aheadPi.k) < 0.0025 )
	{
//		double omega = spd0 * avgK;//aheadPi.k;
//		angle += 0.02 * (omega - car->_yaw_rate);
//		angle += 0.04 * (omega - car->_yaw_rate);
//		angle += 0.05 * (omega - car->_yaw_rate);
		angle += 0.08 * (omega - car->_yaw_rate);
//		angle += 0.10 * (omega - car->_yaw_rate);
//		angle += 0.12 * (omega - car->_yaw_rate);
	}

	angle += o2 * 0.08;

	if( car->_accel_x > 0 )
		angle += avgK * STEER_K_ACC;
	else
		angle += avgK * STEER_K_DEC;

	{
		double	velAng = atan2(car->_speed_Y, car->_speed_X);
		double	ang = car->_yaw - velAng;
		NORM_PI_PI(ang);

		int	k = int(floor((pi.k - K_MIN) / K_STEP));
		int	s = int(floor((spd0 - SPD_MIN) / SPD_STEP));
		double	ae = 0;
		if( k >= 0 && k < K_N && s >= 0 && s < SPD_N )
		{
			ae = m_angle[s][k] - ang;
//			NORM_PI_PI(ae);
		}

//		GfOut( "k %g  delta %g\n", pi.k, delta );
//		if( fabs(delta) > 0.2 && pi.k * delta < 0 )
//		{
//			double	ang = MX(0, MN((fabs(delta) - 0.2) * 0.2, 0.05));
//			angle += ang * SGN(pi.k);
//		}

//		m_angControl.m_p = 1;
//		m_angControl.m_d = 0;//10;
//		angle -= atan(m_angControl.Sample(ae));
//		angle += (ae + ang) * 0.5;
	}

//	GfOut( "k change %7.4f  omega %7.4f  o2 %7.4f   yaw_rate %7.4f\n",
//			pi.k - aheadPi.k, omega, o2, car->_yaw_rate );

	// control offset from path.
	m_lineControl.m_p = 1.0;
	m_lineControl.m_d = 10;
	const double SC = 0.15;//0.15;//0.15;//0.2;//0.15;
	angle -= SC * atan(m_lineControl.Sample(delta));

	return angle;
}

double	MyRobot::SteerAngle1( tCarElt* car, PtInfo& pi, PtInfo& aheadPi )
{
	// work out current car speed.
	SETSPEED0;

	// calc x,y coords of mid point on frt axle.
	double	midPt = 1.37;//1;//car->_dimension_x * 0.5;
//	double	midPt = car->_dimension_x * 0.5;
	double	x = car->pub.DynGCg.pos.x + midPt * cos(car->_yaw);
	double	y = car->pub.DynGCg.pos.y + midPt * sin(car->_yaw);

	tTrkLocPos	trkPos;
	RtTrackGlobal2Local(car->_trkPos.seg, x, y, &trkPos, 0);
	double	toMiddle = trkPos.toMiddle;

	// get curret pos on track.
	double	pos = m_track.CalcPos(trkPos);

	// look this far ahead.
//	double	aheadDist = 2 * spd0 * 0.02;
	double	aheadDist = spd0 * (spd0/1000);//0.04;
	double	aheadPos = m_track.CalcPos(trkPos, aheadDist);

	// get info about pts on track.
	GetPosInfo( pos, pi );
	GetPosInfo( aheadPos, aheadPi );

	double	angle = aheadPi.oang - car->_yaw;
	NORM_PI_PI(angle);

//	GfOut( "**** offs %6.3f  delta %6.3f   avoidT %5.2f\n",
//			pi.offs, delta, m_avoidT );

//	double	avgK = (pi.k + aheadPi.k) * 0.5;
//	double	avgK = pi.k;//(pi.k + aheadPi.k) * 0.5;
	double	avgK = aheadPi.k;
//	double	avgK = Utils::InterpCurvatureLin(pi.k, aheadPi.k, 0.5);
//	double	avgK = Utils::InterpCurvatureRad(pi.k, aheadPi.k, 0.5);
	double omega = car->_speed_x * avgK;//aheadPi.k;
//	double omega = spd0 * avgK;//aheadPi.k;
//	angle += 0.02 * (omega - car->_yaw_rate);
	angle += 0.08 * (omega - car->_yaw_rate);
/*
	omega = spd0 * avgK - car->_yaw_rate;//aheadPi.k;
	double	dist1s = spd0;
//	omega = car->_speed_x * avgK - car->_yaw_rate;//aheadPi.k;
//	double	dist1s = car->_speed_x;
	double	len = 2.63;	// dist between front/back wheel centres.
	double	radiusRear = dist1s / omega;
	angle += atan(len / radiusRear);
*/
	// control offset from path.
	m_lineControl.m_p = 1.0;
	m_lineControl.m_d = 10;
	m_lineControl.m_i = 0;//0.02;
	m_lineControl.m_totalRate = 0;
	m_lineControl.m_maxTotal = 2;
//	const double SC = 0.15;//0.15;//0.15;//0.2;//0.15;
	const double SC = MN(1, 8.5 / spd0);
	double	delta = pi.offs + toMiddle;
//	angle -= SC * atan(m_lineControl.Sample(delta));
	angle -= SC * tanh(m_lineControl.Sample(delta));

//	GfOut( "offset %6.2f tot %g\n", delta, m_lineControl.m_total );

	return angle;
}

double	MyRobot::SteerAngle2( tCarElt* car, PtInfo& pi, PtInfo& aheadPi )
{
	// work out current car speed.
	SETSPEED0;

	// calc x,y coords of mid point on frt axle.
	double	midPt = 1.37;//1;//car->_dimension_x * 0.5;
//	double	midPt = car->_dimension_x * 0.5;
	double	x = car->pub.DynGCg.pos.x + midPt * cos(car->_yaw);
	double	y = car->pub.DynGCg.pos.y + midPt * sin(car->_yaw);

	static double	oldX = x;
	static double	oldY = y;
	double	velX = (x - oldX) / 0.02;
	double	velY = (y - oldY) / 0.02;
	oldX = x;
	oldY = y;

//	GfOut( "%g %g    %g %g    yaw %g\n",
//			car->pub.DynGCg.pos.x, car->pub.DynGCg.pos.y, x, y, car->_yaw );

	tTrkLocPos	trkPos;
	RtTrackGlobal2Local(car->_trkPos.seg, x, y, &trkPos, 0);
	double	toMiddle = trkPos.toMiddle;


	// get curret pos on track.
	double	pos = m_track.CalcPos(trkPos);

	// look this far ahead.
//	double	aheadDist = car->_dimension_x * 0.5 + spd0 * 0.02;
//	double	aheadDist = (car->_dimension_x - 1.37) * 0.5 + spd0 * 0.02;
//	double	aheadDist = 1.37 + spd0 * 0.02;
//	double	aheadDist = 2 * spd0 * 0.02;
	double	aheadDist = spd0 * 0.02;
//	double	aheadDist = 2.0 + spd0 * 0.02;
	double	aheadPos = m_track.CalcPos(trkPos, aheadDist);

	// get info about pts on track.
	GetPosInfo( pos, pi );
	GetPosInfo( aheadPos, aheadPi );

	double	angle = aheadPi.oang - car->_yaw;
	NORM_PI_PI(angle);

//	double	velAng = atan2(velY, velX);
	double	velAng = atan2(car->_speed_Y, car->_speed_X);
	double	velAngCtrl = aheadPi.oang - velAng;
//	double	yawAng = car->_yaw - velAng;
	NORM_PI_PI(velAngCtrl);
	m_velAngControl.m_p = 1;//0.5;//1;
	m_velAngControl.m_d = 10;//25;
	velAngCtrl = m_velAngControl.Sample(velAngCtrl);
//	angle += velAngCtrl;
	angle += tanh(velAngCtrl);
//	angle += 0.5 * velAngCtrl;
//	angle += 0.05 * tanh(20 * velAngCtrl);

//	GfOut( "**** offs %6.3f  delta %6.3f   avoidT %5.2f\n",
//			pi.offs, delta, m_avoidT );

//	double	avgK = (pi.k + aheadPi.k) * 0.5;
//	double	avgK = pi.k;//(pi.k + aheadPi.k) * 0.5;
	double	avgK = aheadPi.k;
//	double	avgK = Utils::InterpCurvatureLin(pi.k, aheadPi.k, 0.5);
//	double	avgK = Utils::InterpCurvatureRad(pi.k, aheadPi.k, 0.5);
	double omega = car->_speed_x * avgK;//aheadPi.k;
//	double omega = spd0 * avgK;//aheadPi.k;
	angle += 0.02 * (omega - car->_yaw_rate);

	// control offset from path.
	m_lineControl.m_p = 1.0;
	m_lineControl.m_d = 10;
	const double SC = 0.15;//0.15;//0.15;//0.2;//0.15;
	double	delta = pi.offs + toMiddle;
//	angle -= SC * atan(m_lineControl.Sample(delta));
	angle -= SC * tanh(m_lineControl.Sample(delta));

	return angle;
}

double	MyRobot::SteerAngle3( tCarElt* car, PtInfo& pi, PtInfo& aheadPi )
{
	// work out current car speed.
	SETSPEED0;

	// get curret pos on track.
	double	pos = m_track.CalcPos(car);

	// look this far ahead.
	double	aheadTime = 0.25;
	double	aheadDist = spd0 * aheadTime;

	double	aheadPos = m_track.CalcPos(car, aheadDist);

	// get info about pts on track.
	GetPosInfo( pos, pi );
	GetPosInfo( aheadPos, aheadPi );

	// we are trying to control 4 things with the steering...
	//	1. the distance of the car from the driving line it is to follow.
	//	2. the gradient of the distance of car from driving line.
	//	3. the angle of the car (yaw).
	//	4. the rotation speed (omega) of the car.

	// we want the angle of the car to be aheadPi.oang in aheadTime seconds.
	//	our current rotation speed is car->_yaw_rate.  our current rotation
	//	angle is car->_yaw.

//	GfOut( "*****    yaw %g    g_yaw %g\n", car->_yaw, car->pub.DynGCg.pos.az );
//	GfOut( "*****    yaw %g    yaw_rate %g\n", car->_yaw, car->_yaw_rate );

//	static double	avgYawU = 0;
//	static double	avgAhdK = 0;
//	static double	avgYawS = 0;
//	avgYawU = avgYawU * 0.5 + car->_yaw_rate * 0.5;
//	avgAhdK = avgAhdK * 0.5 + aheadPi.k * 0.5;
//	double	ys = aheadPi.oang - car->_yaw;
//	NORM_PI_PI(ys);
//	avgYawS = avgYawS * 0.5 + ys * 0.5;

	// current yaw rate.
	double	yawU = car->_yaw_rate;
//	double	yawU = avgYawU;

	// future yaw rate required ahead.
	double	yawV = aheadPi.k * spd0;
//	double	yawV = avgAhdK * spd0;

	// future yaw required ahead (assuming current yaw to be 0).
	double	yawS = aheadPi.oang - car->_yaw;
//	double	yawS = avgYawS;
	NORM_PI_PI(yawS);

	// acceleration to apply.
	double	yawA = 2 * (yawS - yawU * aheadTime) / (aheadTime * aheadTime);
//	double	yawA = (yawS / aheadTime) - yawU;

	double	yaw1s = yawU + 0.5 * yawA;

//	GfOut( "**** yaw %.3f  yawU %.3f  yawV %.3f  yawS %.3f  yawA %.3f\n",
//			car->_yaw, yawU, yawV, yawS, yawA );

	// angle to steer to get desired yaw angle after timestep.
	double	dist1s = spd0;
	double	len = 2.63;	// dist between front/back wheel centres.
	double	radiusRear = dist1s / yaw1s;
//	double	angle = MX(-0.5, MN(atan(len / radiusRear), 0.5));
	double	angle = atan(len / radiusRear);

	if( spd0 < 1 )
		angle = 0;

//	GfOut( "**** yaw1s %g  distTs %.3f  radiusRear %.3f  angle %.3f\n",
//			yaw1s, dist1s, radiusRear, angle );

//	double omega = spd0 * avgK;//aheadPi.k;
//	angle += 0.02 * (omega - car->_yaw_rate);

	// control offset from path.
	const double SC1 = 1;
	m_lineControl.m_p = SC1 * 0.25;	// 1.0 == oscillates
	m_lineControl.m_d = SC1 * 2.5;	// 9.5 == oscillates
	double	delta = pi.offs + car->_trkPos.toMiddle;
	const double SC2 = 1.0 / SC1;
	angle -= SC2 * atan(m_lineControl.Sample(delta));

	return angle;
}

double	MyRobot::SteerAngle4( tCarElt* car, PtInfo& pi, PtInfo& aheadPi )
{
	// work out current car speed.
	SETSPEED0;

	// get curret pos on track.
	double	pos = m_track.CalcPos(car);

	// look this far ahead.
	double	aheadDist = car->_dimension_x * 0.5 + spd0 * 0.02;
//	double	aheadDist = 1 + spd0 * 0.04;
	double	aheadPos = m_track.CalcPos(car, aheadDist);

	// get info about pts on track.
	GetPosInfo( pos, pi );
	GetPosInfo( aheadPos, aheadPi );

	//
	//	deal with yaw.
	//

    double	yawError = aheadPi.oang - car->_yaw;
	NORM_PI_PI(yawError);
	double	yawERate = yawError - m_prevYawError;

	// PID with no integration term.
	const double Y_PROP = 0.1;//1;//2;//1.0;
	const double Y_DIFF = 2.5;//1;//10;
    const double Y_SC = 1;
	double angle = Y_SC * atan((Y_PROP * yawError + Y_DIFF * yawERate) / Y_SC);

	//
	//	deal with line.
	//

	double	lineError = -(pi.offs + car->_trkPos.toMiddle);
	double	lineERate = lineError - m_prevLineError;
	m_prevLineError = lineError;

//	GfOut( "**** offs %6.3f  delta %6.3f   avoidT %5.2f\n",
//			pi.offs, delta, m_avoidT );

	// PID with no integration term.
//	const double L_PROP = 0.025;//005;
//	const double L_DIFF = 0.5;	// 2 -- osc;
	const double L_PROP = 0;//0.15;//005;
	const double L_DIFF = 0;//1.5;	// 2 -- osc;
    const double L_SC = 0.15;
	angle += L_SC * atan((L_PROP * lineError + L_DIFF * lineERate) / L_SC);

	return angle;
}

void	MyRobot::SpeedControl0(
	double	targetSpd,
	double	spd0,
	double&	acc,
	double&	brk )
{
	if( spd0 > targetSpd )
	{
		if( spd0 - 2 > targetSpd )
//		if( spd0 * 0.9 > targetSpd )
//		if( spd0 - 1 > targetSpd )
		{
			if( !m_pitControl.WantToPit() )
				brk = ((spd0-2) - targetSpd);
#if 1
			if( spd0 - 2 < targetSpd )
				brk = 0.07;
			else if( spd0 - 3 < targetSpd )
				brk = 0.14;
			else if( spd0 - 4 < targetSpd )
				brk = 0.20;
			else if( spd0 - 5 < targetSpd )
				brk = 0.35;
			else if( spd0 - 6 < targetSpd )
				brk = 0.5;
			else
				brk = 0.8;//targetSpd == 0 ? 0.5 : 0.75;
#endif
//			brk = spd0 - 3 < targetSpd ? 0.14 : spd0 - 5 < targetSpd ? 0.25 : 0.5;
			acc = 0;
		}
		else
		{
			if( targetSpd > 1 )
				// slow naturally.
				acc = MN(acc, 0.25);
			else
			{
				acc = 0;
				brk = 0.1;
			}
		}
	}

	if( m_lastBrk > 0 && brk > 0 && spd0 - 6 < targetSpd )
	{
//			brk -= 0.3;
//			if( brk < 0 )
		{
//				acc = MN(0.25, -brk);
			brk = 0;
			acc = 0.25;
		}
	}

	m_lastBrk = brk;
	m_lastTargV = 0;
}

void	MyRobot::SpeedControl1(
	double	targetSpd,
	double	spd0,
	double&	acc,
	double&	brk )
{
	if( spd0 > targetSpd )
	{
		if( spd0 - 1 > targetSpd )
		{
			if( !m_pitControl.WantToPit() )
				brk = ((spd0-1) - targetSpd);
#if 1
			if( spd0 - 2 < targetSpd )
				brk = 0.07;
			else if( spd0 - 3 < targetSpd )
				brk = 0.14;
			else if( spd0 - 4 < targetSpd )
				brk = 0.20;
			else if( spd0 - 5 < targetSpd )
				brk = 0.25;
			else if( spd0 - 5 < targetSpd )
				brk = 0.5;
			else
				brk = 0.8;//targetSpd == 0 ? 0.5 : 0.75;
#endif
//			brk = spd0 - 3 < targetSpd ? 0.14 : spd0 - 5 < targetSpd ? 0.25 : 0.5;
			acc = 0;
		}
		else
		{
			if( targetSpd > 1 )
				// slow naturally.
				acc = MN(acc, 0.25);
			else
			{
				acc = 0;
				brk = 0.1;
			}
		}
	}

	m_lastTargV = 0;
}

void	MyRobot::SpeedControl2(
	double	targetSpd,
	double	spd0,
	double&	acc,
	double&	brk )
{
	if( m_lastBrk && m_lastTargV )
	{
		if( m_lastBrk > 0 )//|| (car->ctrl.accelCmd == -m_lastBrk) )
		{
			double	err = m_lastTargV - spd0;
			m_accBrkCoeff.Sample( err, m_lastBrk );
//			GfOut( "accbrk sample %g -> %g\n", err, m_lastBrk );
		}
		m_lastBrk = 0;
		m_lastTargV = 0;
	}

	if( spd0 > targetSpd )
//	if( targetSpd < 100 )
	{
//		if( spd0 - 1 > targetSpd )
		{
			double	MAX_BRK = 0.5;
//			double	MAX_BRK = 0.75;
			double	err = spd0 - targetSpd;
			brk = MX(0, MN(m_accBrkCoeff.CalcY(err), MAX_BRK));
//			GfOut( "accbrk calcy  %g -> %g\n", err, t );
			acc = 0;

			m_lastBrk = brk;
			m_lastTargV = 0;

			if( brk > 0 )
			{
				if( targetSpd > 0 )
					m_lastTargV = spd0;
			}

//			GfOut( "*** brake ***  spd0 %g   targ %g  brk %g   acc %g\n",
//					spd0, targetSpd, brk, acc );
		}
//		else
//			acc = MN(acc, 0.1);
	}
}

void	MyRobot::SpeedControl3(
	double	targetSpd,
	double	spd0,
	double&	acc,
	double&	brk )
{
	if( m_lastBrk && m_lastTargV )
	{
		double	err = spd0 - m_lastTargV;
		m_brkCoeff[m_lastB] += err * 0.001;
//		GfOut( "*** bfk err %g   coeff %g\n", err, m_brkCoeff[m_lastB] );
		m_lastBrk = 0;
		m_lastTargV = 0;
	}

	if( spd0 > targetSpd )
	{
//		if( spd0 - 1 > targetSpd )
		{
			int		b = int(floor(spd0 / 2));
			double	MAX_BRK = 0.5;
			brk = MX(0, MN(m_brkCoeff[b] * (spd0 - targetSpd), MAX_BRK));
			acc = 0;
			m_lastB = b;
			m_lastBrk = brk;
			m_lastTargV = 0;

			if( brk > 0 && brk < MAX_BRK)
			{
				if( targetSpd > 0 )
					m_lastTargV = targetSpd;
			}

//			GfOut( "*** brake ***  spd0 %g   targ %g  brk %g   acc %g\n",
//					spd0, targetSpd, brk, acc );
		}
//		else
//			acc = 0.1;
	}
}

void	MyRobot::SpeedControl4(
	double	targetSpd,
	double	spd0,
	CarElt*	car,
	double&	acc,
	double&	brk )
{
	if( m_lastBrk && m_lastTargV )
	{
		if( m_lastBrk > 0 || (car->ctrl.accelCmd == -m_lastBrk) )
		{
			double	err = m_lastTargV - spd0;
			m_accBrkCoeff.Sample( err, m_lastBrk );
//			GfOut( "accbrk sample %g -> %g\n", err, m_lastBrk );
		}
		m_lastBrk = 0;
		m_lastTargV = 0;
	}

//	if( spd0 > targetSpd )
	if( targetSpd < 100 )
	{
//		if( spd0 - 1 > targetSpd )
		{
			double	MAX_BRK = 0.5;
//			double	MAX_BRK = 0.75;
			double	err = spd0 - targetSpd;
			double	t = m_accBrkCoeff.CalcY(err);
//			GfOut( "accbrk calcy  %g -> %g\n", err, t );
			if( t > 0 )
			{
				brk = MN(t, MAX_BRK);
				acc = 0;
			}
			else
			{
				brk = 0;
				acc = MN(-t, 1);
			}
			m_lastBrk = t;
			m_lastTargV = 0;

//			if( brk > 0 && brk < MAX_BRK )
			if( t > -1 && t < MAX_BRK )
			{
				if( targetSpd > 0 )
					m_lastTargV = spd0;
			}

//			GfOut( "*** brake ***  spd0 %g   targ %g  brk %g   acc %g\n",
//					spd0, targetSpd, brk, acc );
		}
//		else
//			acc = 0.1;
	}
}

void	MyRobot::SpeedControl(
	int		which,
	double	targetSpd,
	double	spd0,
	CarElt*	car,
	double&	acc,
	double&	brk )
{
//	SpeedControl2(targetSpd, spd0, acc, brk);
//	SpeedControl4(targetSpd, spd0, car, acc, brk);
	switch( which )
	{
		case 0:		SpeedControl0(targetSpd, spd0, acc, brk);		break;
		case 1:		SpeedControl1(targetSpd, spd0, acc, brk);		break;
		case 2:		SpeedControl2(targetSpd, spd0, acc, brk);		break;
		case 3:		SpeedControl3(targetSpd, spd0, acc, brk);		break;
		case 4:		SpeedControl4(targetSpd, spd0, car, acc, brk);	break;
		default:	SpeedControl3(targetSpd, spd0, acc, brk);		break;
	}
}

void	MyRobot::CalcSkill( tSituation *s )
{
	if (m_skill > 0.0 && s->_raceType != RM_TYPE_PRACTICE)
	{
		if (s->currentTime < 1.0)
		{
			m_skill_adjust_timer = -1.0;
			m_skill_adjust_limit = 0.0;
			m_skill_decel_adjust_targ = 0.0;
			m_skill_brake_adjust_targ = 0.0;
			m_skill_decel_adjust_perc = 0.0;
			m_skill_brake_adjust_perc = 0.0;
		}
		else
		{
			if (m_skill_adjust_timer == -1.0 || 
			         s->currentTime - m_skill_adjust_timer > m_skill_adjust_limit)
			{
				double rand1 = GetRandom() / 65536.0; // how long we'll change speed for
				double rand2 = GetRandom() / 65536.0; // the actual speed change
				double rand3 = GetRandom() / 65536.0; // whether change is positive or negative

				// acceleration to use in current time limit
				m_skill_decel_adjust_targ = (m_skill/4 * rand1);

				// brake to use - usually 1.0, sometimes less (more rarely on higher skill)
				m_skill_brake_adjust_targ = MAX(0.7, 1.0 - MAX(0.0, m_skill/10 * (rand2-0.7)));

				// how long this skill mode will last
				m_skill_adjust_limit = 5.0 + rand3 * 50.0;
				m_skill_adjust_timer = s->currentTime;
			}

			// adjust skill changes so they're not abrupt
			if (m_skill_decel_adjust_perc < m_skill_decel_adjust_targ)
				m_skill_decel_adjust_perc += MIN(s->deltaTime*4, m_skill_decel_adjust_targ - m_skill_decel_adjust_perc);
			else
				m_skill_decel_adjust_perc -= MIN(s->deltaTime*4, m_skill_decel_adjust_perc - m_skill_decel_adjust_targ);
			if (m_skill_brake_adjust_perc < m_skill_brake_adjust_targ)
				m_skill_brake_adjust_perc += MIN(s->deltaTime*4, m_skill_brake_adjust_targ - m_skill_brake_adjust_perc);
			else
				m_skill_brake_adjust_perc -= MIN(s->deltaTime*4, m_skill_brake_adjust_perc - m_skill_brake_adjust_targ);
		}
	}
}

void	MyRobot::Drive( int index, tCarElt* car, tSituation* s )
{
	CalcSkill( s );

	m_realSpeed = 0.0;
	{
		double trackangle = RtTrackSideTgAngleL(&(car->_trkPos));
		v2d speed, dir;
		speed.x = car->_speed_X;
		speed.y = car->_speed_Y;
		dir.x = cos(trackangle);
		dir.y = sin(trackangle);
		m_realSpeed = speed * dir;
	}

	double	h[4];
	{for( int i = 0; i < 4; i++ )
	{
		tTrkLocPos	wp;
//		double		wx = car->_pos_X;
//		double		wy = car->_pos_Y;
		double		wx = car->pub.DynGCg.pos.x;
		double		wy = car->pub.DynGCg.pos.y;
		RtTrackGlobal2Local(car->_trkPos.seg, wx, wy, &wp, TR_LPOS_SEGMENT);
		h[i] = car->_pos_Z - RtTrackHeightL(&wp) - car->_wheelRadius(i);
	}}

	if( m_raceStart || s->currentTime <= 0.5 )
	{
		if( m_raceStart )
		{
			Vec2d	thisPt(car->_pos_X, car->_pos_Y);
			for( int i = 0; i + 1 < HIST; i++ )
				m_lastPts[i] = m_lastPts[i + 1];
			m_lastSpd = 0;
			m_lastAng = 0;
			m_steerGraph.SetBeta( 0.1 );
		}

		m_raceStart = false;
//		GfOut( "m_avoidS = %g (race start 2)\n", 0.0 );
		m_avoidS = 0;
		m_avoidSVel = 0;
		m_avoidT = CalcPathTarget(m_track.CalcPos(car), -car->_trkPos.toMiddle);
		m_avoidTVel = 0;
	}

	double	carFuel = car->_fuel;
//	double	carFuel = MN(car->_fuel, 50);
//	double	carFuel = 0;
//	if( fabs(m_cm.FUEL   - car->_fuel)    > 5	||
	if( fabs(m_cm.FUEL   - carFuel)    > 5	||
		fabs(m_cm.DAMAGE - car->_dammage) > 250 )
	{
//		m_cm.FUEL	= 5 * floor(car->_fuel / 5);
		m_cm.FUEL	= 5 * floor(carFuel / 5);
		m_cm.DAMAGE	= car->_dammage;
		m_cm2.FUEL = m_cm.FUEL;
		m_cm2.DAMAGE = m_cm.DAMAGE;
/*		for( int path = PATH_NORMAL; path <= PATH_RIGHT; path++ )
		{
			m_path[path].CalcMaxSpeeds( m_cm );
			m_path[path].PropagateBreaking( m_cm );
			m_path[path].PropagateAcceleration( m_cm );
//			m_pitPath[path].CalcMaxSpeeds( m_cm );
//			m_pitPath[path].PropagateBreaking( m_cm );
//			m_pitPath[path].PropagateAcceleration( m_cm );
		}*/

		m_path[PATH_NORMAL].CalcMaxSpeeds( m_cm );
		m_path[PATH_NORMAL].PropagateBreaking( m_cm );
		m_path[PATH_NORMAL].PropagateAcceleration( m_cm );
		m_path[PATH_LEFT].CalcMaxSpeeds( m_cm2 );
		m_path[PATH_LEFT].PropagateBreaking( m_cm2 );
		m_path[PATH_LEFT].PropagateAcceleration( m_cm2 );
		m_path[PATH_RIGHT].CalcMaxSpeeds( m_cm2 );
		m_path[PATH_RIGHT].PropagateBreaking( m_cm2 );
		m_path[PATH_RIGHT].PropagateAcceleration( m_cm2 );
	}

//	GfOut( "**** wheel fz %g %g %g %g\n", h[0], h[1], h[2], h[3] );

	if( h[0] > FLY_HEIGHT )
	{
//		m_flying = FLY_COUNT;
		m_flying = MN(FLY_COUNT, m_flying + (FLY_COUNT / 2));
	}
	else if( m_flying > 0 )
	{
		m_flying--;
	}

	// get curret pos on track.

	PtInfo	pi, aheadPi;
	double angle;
	double	pos = m_track.CalcPos(car);
	if( m_pitControl.WantToPit() && m_pitPath[PATH_NORMAL].ContainsPos(pos) && car->_speed_x < 15 )
		angle = SteerAngle1(car, pi, aheadPi);
	else
		angle = SteerAngle0(car, pi, aheadPi);
//	double	angle = SteerAngle1(car, pi, aheadPi);
//	double	angle = SteerAngle2(car, pi, aheadPi);
//	double	angle = SteerAngle3(car, pi, aheadPi);

	double	steer = angle / car->_steerLock;

	// steer into skid
#if 1
	double trackangle = RtTrackSideTgAngleL( &(car->_trkPos) );
	double carangle = trackangle - car->_yaw;
	NORM_PI_PI(carangle);
	carangle = -carangle;
	double speedangle = -(trackangle - atan2(car->_speed_Y, car->_speed_X));
	NORM_PI_PI(speedangle);
	double nextangle = carangle + car->_yaw_rate/3;
//fprintf(stderr,"ta=%.3f ca=%.3f sa=%.3f na=%.3f steer=%.3f ",trackangle,carangle,speedangle,nextangle,steer);
	if (fabs(nextangle) > fabs(speedangle))
	{
		double sa = MAX(-0.3, MIN(0.3, speedangle/3));
		double anglediff = (speedangle - nextangle) * (0.1 + fabs(nextangle)/6);
		steer += anglediff * (SKIDSTEER + MAX(1.0, 1.0 - car->_accel_x/5));
//fprintf(stderr," + %.3f ",anglediff*SKIDSTEER);
	}

	{
		// smooth small steering changes
		double newsteer = steer;
		if (fabs(steer - m_steersum / 2) < 0.04)
			newsteer = (m_steersum*2+steer) / 3;

		m_steersum = m_steersum/2 + steer;
		steer = newsteer;
	}


//fprintf(stderr,"= %.3f\n",steer);
#endif

//	GfOut( "   *** pos %7.2f  angle %6.3f\n", pos, angle );

	// work out current car speed.
	//double	spd0 = hypot(car->_speed_x, car->_speed_y);
	double	spd0 = m_realSpeed;

	{
		Vec2d	thisPt(car->_pos_X, car->_pos_Y);
		if( (thisPt - m_lastPts[0]).len() > 0.1 &&
			car->ctrl.accelCmd == 1.0 )
		{
			double	x[2];
			x[0] = Utils::CalcCurvature(m_lastPts[0], m_lastPts[HIST / 2], thisPt);
			x[0] = fabs(x[0]);
			x[1] = m_lastSpd;
			m_steerGraph.Learn( x, fabs(m_lastAng) );
//			GfOut( "*** Learn %8.6f, %5.2f -> %8.6f  \n",
//					x[0], m_lastSpd, fabs(m_lastAng) );
			m_steerAvg.AddValue( x[0], x[1], fabs(m_lastAng) );
		}

		for( int i = 0; i + 1 < HIST; i++ )
		{
			m_lastPts[i] = m_lastPts[i + 1];
		}

		m_lastPts[HIST - 1] = thisPt;
		m_lastSpd = spd0;
		m_lastAng = m_lastAng * 0.75 + angle * 0.25;
	}

	const double G = 9.81;

	double	acc = (!m_pitControl.WantToPit() ? 0.5 : 1.0);
	double	brk = 0;

	double	targetSpd = pi.spd;
	double	skidAng = atan2(car->_speed_Y, car->_speed_X) - car->_yaw;
	NORM_PI_PI(skidAng);

	if( !m_pitControl.WantToPit() )
	{
		// Andrew's Super Throttle Control System (TM)
		double avgk = (pi.k + aheadPi.k) / 2;
		double k = (avgk > 0.0 ? MAX(0.0, avgk-0.002) : MIN(0.0, avgk+0.002)) * 24;

		// x is < 0 for decel, > 0 for accel
		double x = MIN(1.0, (10 + car->_speed_x) * (targetSpd - car->_speed_x) / 200);
		
		// if over/understeering, decrease speed
		if (k > 0.0)
		{
			double mod = ((steer-k) * fabs(k*15));
			if (mod < 0.0) // oversteer, will increase x but by less if skidding outwards
				mod /= 7 - MIN(0.0, skidAng * 20);
			x = MIN(x, 1.0) - mod;
		}
		else
		{
			double mod = ((steer-k) * fabs(k*15));
			if (mod > 0.0) // oversteer
				mod /= 7 + MAX(0.0, skidAng * 20);
			x = MIN(x, 1.0) + mod;
		}
	
		acc = MAX(0.25, MN(1.0, x * ACC_SCALE));

		if (x > 0)
			targetSpd = MAX(targetSpd, spd0);
	}

//	if( pi.idx >= 461 && pi.idx <= 484 )
//		targetSpd = 100;

	bool	close = false;
	bool	lapper = false;
	AvoidOtherCars( index, car, pi.k, targetSpd, s, close, lapper );
	if( close )
	{
		SpeedControl( SPDC_TRAFFIC, targetSpd, spd0, car, acc, brk );
	}
	else if( m_pitControl.WantToPit() )
	{
		SpeedControl( SPDC_TRAFFIC, targetSpd, spd0, car, acc, brk );
	}
	else
	{
		if( m_avoidS == 1 )
			SpeedControl( SPDC_NORMAL, targetSpd, spd0, car, acc, brk );
		else
			SpeedControl1( targetSpd, spd0, acc, brk );
	}

	acc = MAX(0.0, acc - fabs(car->_yaw_rate - steer) * m_cm.GetYRAccel(pi.idx));

	brk *= BRAKE_PRESSURE;

	if (m_skill > 0.0)
	{
		// apply random skill changes
		double turndecel = m_skill_decel_adjust_perc * 2;
		if (fabs(steer) > 0.02)
		{
			double decel = ((fabs(steer)-0.02) * (1.0+fabs(steer)) * turndecel);
			acc = MAX(acc/3, 1.0 - decel);
		}
		brk *= m_skill_brake_adjust_perc;
	}

	if( lapper )
		acc = MN(acc, 0.9);

	double	delta = pi.offs + car->_trkPos.toMiddle;
//	if( acc == 1.0 && car->_speed_y * delta > 0 &&
//		car->_speed_x > 0 && fabs(car->_speed_y / car->_speed_x) > 0.1 )
//		acc = 0.95;

//	acc = 1.0;
//	brk = 0;
	// out of control??
//	if( car->_speed_x > 5 && fabs(car->_speed_y / car->_speed_x) > 0.15 )
	if( car->_speed_x > 5 && fabs(skidAng) > 0.2 )
	{
//		GfOut( "******* skidAng %g    x_spd %g    ratio %g\n",
//				skidAng, car->_speed_x, car->_speed_y / car->_speed_x );
		acc = MN(acc, 0.25 + 0.75 * cos(skidAng));
	}

	if( car->_speed_x > 5 )
	{
		skidAng = MN(skidAng * 2, PI);
		brk *= MX(0, fabs(cos(skidAng)));
	}

	// too far off line?
	double	fDelta = fabs(delta);
	const double	offDist = 2;//0.5;
	if( fDelta > offDist && car->_distRaced > 50 )
	{
		double	mx = MX(1.0 - (fDelta - offDist) * 0.2, 0.4);
		acc = MN(mx, acc);
	}

//	GfOut( "****** slip %6.3f m/s    sy %6.3f\n", spd0 * sin(skidAng),
//				sin(skidAng) );
//	if( fabs(sin(skidAng)) > 0.125 )
//		acc = MN(acc, 0.2);

/*
	// stuck?
	double	carRelAng = car->_yaw
	if( car->_speed_x < 5 && (pi.toL < 3 || pi.toR < 3) &&
		car->
	{
		m_stuck = true;
	}

	// get unstuck if stuck.
	if( m_stuck )
	{
	}
*/

//	GfOut( "acc %g  brk %g\n", acc, brk );

	if( car->ctrl.clutchCmd > 0 )
	{
/*		car->ctrl.clutchCmd -= 0.085f;
		if( car->ctrl.clutchCmd < 0 )
			car->ctrl.clutchCmd = 0;
*/
		double	wr = 0;
		int		count = 0;

		if( m_driveType == cDT_FWD || m_driveType == cDT_4WD )
		{
			wr += car->_wheelRadius(FRNT_LFT) + car->_wheelRadius(FRNT_RGT);
			count += 2;
		}

		if( m_driveType == cDT_RWD || m_driveType == cDT_4WD )
		{
			wr += car->_wheelRadius(REAR_LFT) + car->_wheelRadius(REAR_RGT);
			count += 2;
		}
		wr /= count;
		double	gr = car->_gearRatio[car->_gear + car->_gearOffset];
		double	rpmForSpd = gr * car->_speed_x / wr;
		double	rpm = car->_enginerpm;
//		GfOut( "RPM %3.0f  RPM2 %3.0f  %g\n", rpm, rpmForSpd, rpmForSpd / rpm );

		if( car->ctrl.clutchCmd > 0.5 )
		{
			car->ctrl.clutchCmd = 0.5;
		}
		else if( car->ctrl.clutchCmd == 0.5 )
		{
			if( rpmForSpd / rpm > 0.82 )
				car->ctrl.clutchCmd = 0.49f;
		}
		else
		{
			car->ctrl.clutchCmd -= 0.04f;
			if( car->ctrl.clutchCmd < 0 )
				car->ctrl.clutchCmd = 0;
		}
	}

	int gear = CalcGear(car, acc);

	// facing the wrong way on the track??
	double	dirAng = pi.oang - car->_yaw;
	NORM_PI_PI(dirAng);
	if( gear > 0 && s->currentTime - m_reverseTime < 5.0 && m_reverseTime <= s->currentTime &&
		(fabs(dirAng) > 75 * PI / 180 || fabs(carangle) > 1.4))//car->_speed_x < 0 )
	{
//		GfOut( "Backwards (dirAng %lg) (offs %lg)\n", dirAng, pi.offs );
//		if( dirAng * pi.offs < 0 )
		if( dirAng * car->_trkPos.toMiddle < 0 || 
		    (spd0 < 1 && fabs(carangle) > 0.8 + 
		      MN(0.7, MX(0.0, car->_trkPos.seg->width -
		       MN(car->_trkPos.toLeft, car->_trkPos.toRight))/10)) )
		{
//			GfOut( "Reversing a bit\n", dirAng );
			gear = -1;
			acc = 0.8;
			brk = 0;
			steer = -SGN(dirAng);
		}
	}
	if (car->_gear == -1 && 
		gear > 0 && 
		fabs(carangle) > 1.3 && 
		s->currentTime - m_reverseTime < 4.0 &&
		m_reverseTime <= s->currentTime)
	{
		gear = -1;
		acc = 0.8;
		brk = 0;
		steer = -SGN(dirAng);
	}
	else if (s->currentTime - m_reverseTime >= 2.0)
	{
		m_reverseTime = s->currentTime + 4.0;
	}
	else if (gear >= 1)
	{
		m_reverseTime = MAX(m_reverseTime, s->currentTime);
	}

	if( gear > 0 && car->_speed_x < -0.01 )
	{
		gear = 1;
		brk = car->_speed_x < -0.5 ? 0.25 : 0;
		acc = 0.25;
	}

	if (gear == -1 && car->_speed_x < -5.0 && 
	    ((car->_trkPos.toMiddle > 2.0 && speedangle > 0.4 && speedangle < 2.8) ||
	     (car->_trkPos.toMiddle < -2.0 && speedangle < -0.4 && speedangle > -2.8)))
	{
		// reversing out of control - break!
		gear = 1;
		brk = 0.5;
		acc = 0.0;
	}


	if( gear == 1 && car->_speed_x >= -0.01 && car->_speed_x < 10 &&
		acc == 1.0 && brk == 0 )
	{
		double	rpm = car->_enginerpm;
//		double	clutch = (800 - rpm) / 850;
		double	clutch = (850 - rpm) / 400;
		if( car->_speed_x < 0.05 )
			clutch = 0.5;

//		if( car->race.curTime <= 0.01 )
//			clutch = 1.0;
		car->ctrl.clutchCmd = MX(0, MN(clutch, 0.9));
	}

//	if( car->_speed_x > 10 )
//		acc = 0.2;

/*
	if( m_flying )
	{
//		m_flying--;

		// steer in direction of car movement.
		double	velAng = atan2(car->_speed_Y, car->_speed_X);
		double	angle = velAng - car->_yaw;
		NORM_PI_PI(angle);

//		GfOut( "*** flying %d   h %.3f  ang%g\n", m_flying, h[0], angle );

		int		f = FLY_COUNT - m_flying;
		double	t = MX(0, MN(1.0 * f / FLY_COUNT, 1));
//		t = 0.25;
		steer = steer * t + (1 - t) * angle / car->_steerLock;//0.25;//angle *= 0.5;//0.1;//5;
//		steer = steer * t;
		acc = MN(acc, MX(t, 0.8 + 0.2 * t));
	}
*/
	if( fabs(pi.offs) > 5 )
	{
//		acc = MN(acc, 0.25);
	}

	double	toL = pi.toL - car->_trkPos.toMiddle;
	double	toR = pi.toR + car->_trkPos.toMiddle;
//	GfOut( "toL %g   toR %g\n", toL, toR );
	if( toL < -1 || toR < -1 )
	{
		// off track.
//		acc = MN(acc, 0.2);
	}

	if( car->ctrl.accelCmd == 1 && car->ctrl.brakeCmd == 0 )
	{
		m_maxAccel.Learn( car->_speed_x, car->_accel_x );
//		GfOut( "%4.1f %4.1f ", car->_speed_x, car->_accel_x );
//		for( int i = 0; i < 12; i++ )
//			GfOut( " %4.1f", m_maxAccel.GetY(i) );
//		GfOut( "\n" );
	}

	if( fabs(pi.k * spd0 - car->_yaw_rate) < 0.02 )
	{
		double	velAng = atan2(car->_speed_Y, car->_speed_X);
		double	ang = car->_yaw - velAng;
		NORM_PI_PI(ang);

		int	k = int(floor((pi.k - K_MIN) / K_STEP));
		int	s = int(floor((spd0 - SPD_MIN) / SPD_STEP));
		double	ae = 0;
		if( k >= 0 && k < K_N && s >= 0 && s < SPD_N )
		{
			ae = ang - m_angle[s][k];
			m_angle[s][k] += ae * 0.1;
			ae = m_angle[s][k] - ang;
		}

//		GfOut( "Steady state  s %.1f, k %.6f, a %.4f  ae %.4f  o %g\n",
//				spd0, pi.k, ang, ae, pi.offs + car->_trkPos.toMiddle );
	}

//	if( acc > 0.25 )
//		acc = 0.25;
//	if( car->ctrl.accelCmd + 0.1 < acc )
//	{
//		acc = car->ctrl.accelCmd + 0.1;
//	}

	brk = ApplyAbs(car, brk);
//	acc = ApplyTractionControl(car, acc);

//	if( car->_speed_x > 10 )
//		acc = 0.0;

//	GfOut( "accel cmd: %.2f    steer ang: %.3f\n", acc, steer * car->_steerLock * 180 / PI );

//	if( pi.idx >= 461 && pi.idx <= 484 )
//		acc = 1.0;

        // set up the values to return
        car->ctrl.steer = steer;
        car->ctrl.gear = gear;
        car->ctrl.accelCmd = acc;
        car->ctrl.brakeCmd = brk;
	if (m_verbose)
	{
		fprintf(stderr,"%s %d: speed=%.3f steer=%.3f gear=%d accel=%.3f brake=%.3f\n",car->_carName,pi.idx,car->_speed_x,steer,gear,acc,brk);
		fflush(stderr);
	}

	// see if team-mates are pitting...
	if (s->_raceType != RM_TYPE_QUALIF)
	{
		int pitInUse = 0;
		CarElt *pOCar = NULL;

		for( int i = 0; i < m_nCars; i++ )
		{
			Opponent::Info&	oi = m_opp[i].GetInfo();
			CarElt*	oCar = m_opp[i].GetCar();
			if (oCar == car)
				continue;
			if (oCar->_state > RM_CAR_STATE_PIT)
				continue;
	
			if (oi.GotFlags(Opponent::F_TEAMMATE))
			{
				pitInUse = oCar->_lightCmd;
				pOCar = oCar;
				break;
			}
		}

		car->_lightCmd = m_pitControl.Process( car, pitInUse, pOCar );
	}

//	GfOut( "%g %g %g\n", car->priv.wheel[0].Fx, car->priv.wheel[0].Fy, car->priv.wheel[0].Fz );
//	GfOut( "%g %g %08x\n", car->priv.wheel[0].slipSide, car->priv.wheel[0].slipAccel, car->priv.wheel[0].seg );
//	GfOut( "%d %g %g --  \r", pi.idx, car->_pos_X, car->_pos_Y );
}

// Pitstop callback.
int		MyRobot::PitCmd( int index, tCarElt* car, tSituation* s )
{
	return false;
}


// End of the current race.
void	MyRobot::EndRace( int index, tCarElt* car, tSituation* s )
{
}


// Called before the module is unloaded.
void	MyRobot::Shutdown( int index )
{
	// dump out some interesting info....
//	const int	NSEG = m_track.GetSize();
//	{for( int i = 0; i < NSEG; i++ )
//	{
//		GfOut( "dist %7.1f  spd %7.1f  rec %7.1f\n",
//				m_track[i].segDist,
//				m_path[PATH_NORMAL].GetAt(i).spd,
//				m_oppPath[m_myOppIdx].GetAt(i).avgV );
//	}}
/*
	GfOut( "*** STEER GRAPH ***\n" );
	FILE*	pFile = fopen("steer.csv", "w");
	fprintf( pFile, "," );
	for( int j = 0; j < m_steerAvg.GetYSize(); j++ )
		fprintf( pFile, "%g, ", m_steerAvg.GetAxisValue(1, j) );
	fprintf( pFile, "\n" );
	for( int i = 0; i < m_steerAvg.GetXSize(); i++ )
	{
		fprintf( pFile, "%g, ", m_steerAvg.GetAxisValue(0, i) );
		for( int j = 0; j < m_steerAvg.GetYSize(); j++ )
		{
			double	value = m_steerAvg.GetValueAt(i, j);
			if( value == 0 )
				fprintf( pFile, ", " );
			else
				fprintf( pFile, "%0.6f, ", value );
		}
		fprintf( pFile, "\n" );
	}
	fclose( pFile );*/
}

void	MyRobot::AvoidOtherCars(
	int			index,
	tCarElt*	car,
	double		k,
	double&		carTargetSpd,
	tSituation*	s,
	bool&		close,
	bool&		lapper )
{
	m_pShared->m_teamInfo.GetAt(car->index)->damage = car->_dammage;

	double	trackLen = m_track.GetLength();
	double	myPos = RtGetDistFromStart(car);
	double	mySpd = hypot(car->_speed_X, car->_speed_Y);
	if( fabs(mySpd) < 0.01 )
		mySpd = 0.01;

	double	myDirX = car->_speed_X / mySpd;
	double	myDirY = car->_speed_Y / mySpd;
	int		myIdx = 0;

	{for( int i = 0; i < m_nCars; i++ )
	{
		m_opp[i].UpdatePath();
/*		double	conf = m_oppPath[i].CalcConfidence();
		if( conf > 0.95 )
		{
			double	dist = RtGetDistFromStart(m_oppPath[i].GetCar());
			GfOut( "***** %2d   dist %.2f   conf %g\n",
					m_oppPath[i].GetCar()->_laps, dist, conf );
		}*/

		m_opp[i].UpdateSit( car, &m_pShared->m_teamInfo, myDirX, myDirY );
	}}

	const Opponent::Sit&	mySit = m_opp[m_myOppIdx].GetInfo().sit;

	{for( int i = 0; i < m_nCars; i++ )
	{
		m_opp[i].ProcessMyCar( s, &m_pShared->m_teamInfo, car, mySit, *this,
					m_maxAccel.CalcY(car->_speed_x), m_aggression, i );
	}}

#if defined(USE_NEW_AVOIDANCE)
	double	newAvoidS = 1;
	double	newAvoidT = 0;
	if (OT_MOD)
	{

	{
		int	sort[cMAX_OPP];
		int	cnt = 0;
		double  closest = 100.0;

		for( int i = 0; i < m_nCars; i++ )
		{
			Opponent::Info&	oi = m_opp[i].GetInfo();
			CarElt*			oCar = m_opp[i].GetCar();
			if( oCar == car || oi.flags == 0 )
				continue;

			bool	ignoreTeamMate =
						oi.GotFlags(Opponent::F_TEAMMATE | Opponent::F_AHEAD) &&
						(car->_laps < oCar->_laps ||
						 car->_dammage + 200 >= oi.tmDamage);
			if( ignoreTeamMate )
				continue;

			if( oi.newCatching )
			{
				int	j;
				for( j = cnt; j > 0; j-- )
				{
					if( m_opp[sort[j - 1]].GetInfo().newCatchTime < oi.newCatchTime )
						break;
					sort[j] = sort[j - 1];
				}
				sort[j] = i;
				cnt++;
			}
		}

//		double	toL, toR;
//		GetPathToLeftAndRight( car, toL, toR );
//		Span	rootSpan(-car->_trkPos.toMiddle - toL, -car->_trkPos.toMiddle + toR);

		PtInfo	pi;
		GetPtInfo( PATH_NORMAL, car->_distFromStartLine, pi );
		double	targ = pi.offs;

		double	width = m_track.GetWidth() * 0.5 - 1;
		Span	rootSpan(-width, width);
		rootSpan.Extend( pi.offs );

		double	myOffs = -car->_trkPos.toMiddle;
		bool	toSideL = false;
		bool	toSideR = false;
		double	repulsion = 0;

		//
		//	deal with cars to side.
		//

		int		which = 0;
		for( ;	which < cnt &&
				m_opp[sort[which]].GetInfo().newCatchTime == 0; which++ )
		{
			Opponent::Info&	oi = m_opp[sort[which]].GetInfo();

			// to side.
			if( oi.sit.rdPY > 0 )
			{
				toSideL = true;

				double	offs;
				if( oi.newPiL.isSpace )
					offs = oi.newPiL.myOffset;
				else
					offs = oi.sit.offs - oi.sit.minDY;
				rootSpan.b = MN(rootSpan.b, offs);

				double	dist = fabs(myOffs - offs);
				if( dist < 3 )
					repulsion -= 0.2;//0.5 / MX(1, dist);
			}
			else
			{
				toSideR = true;

				double	offs;
				if( oi.newPiR.isSpace )
					offs = oi.newPiR.myOffset;
				else
					offs = oi.sit.offs + oi.sit.minDY;
				rootSpan.a = MX(rootSpan.a, offs);

				double	dist = fabs(myOffs - offs);
				if( dist < 3 )
					repulsion += 0.2;//0.5 / MX(1, dist);
			}

			targ = myOffs * 0.95 + pi.offs * 0.05 + repulsion;
		}

		//
		//	setup path coords for cars to side.
		//

		PathRange	pr;
		if( toSideL || toSideR )
		{
			if( toSideL )
				pr.AddGreater( myPos, rootSpan.a, false, *this );

			if( toSideR )
				pr.AddLesser( myPos, rootSpan.b, false, *this );
		}

		//
		//	deal with cars ahead.
		//

		{for( ; which < cnt; which++ )
		{
			Opponent::Info&	oi = m_opp[sort[which]].GetInfo();

			if( rootSpan.IsNull() )
			{
				rootSpan.a = rootSpan.b = (rootSpan.a + rootSpan.b) * 0.5;
				targ = rootSpan.a;
				break;
			}

			Span	temp(rootSpan);

			// ahead.
			if( oi.newPiL.isSpace && oi.newPiL.goodPath )
			{
				if( oi.newPiR.isSpace && oi.newPiR.goodPath )
				{
					// space on both sides... damn, need to make a
					//	decision!  :)
					double	mid = (oi.newPiL.myOffset + oi.newPiR.myOffset) * 0.5;
					if( pi.offs < mid )
					{
						// overtake to left.
						double	offs = oi.newPiL.myOffset;
						rootSpan.b = MN(rootSpan.b, offs);
					}
					else //if( pi.offs > oi.newPiR.myOffset )
					{
						// overtake to right.
						double	offs = oi.newPiR.myOffset;
						rootSpan.a = MX(rootSpan.a, offs);
					}
				}
				else
				{
					// space only to left.
					double	offs = oi.newPiL.myOffset;
					rootSpan.b = MN(rootSpan.b, offs);
				}
			}
			else if( oi.newPiR.isSpace && oi.newPiR.goodPath )
			{
				// space only to right.
				double	offs = oi.newPiR.myOffset;
				rootSpan.a = MX(rootSpan.a, offs);
			}
			else
			{
				// no space to overtake at all! (thin track?)
			}

			if( rootSpan.IsNull() )
			{
				rootSpan = temp;
				break;
			}
		}}

#ifdef USE_NEW_AVOIDANCE_GFOUT
		if( cnt > 0 )
		{
			double	offs = CalcPathOffset(car->_distFromStartLine, m_avoidS, m_avoidT);
			GfOut( "---- %.1f %.1f -------- targ %.1f -------------------\n",
					0.0, 0.0, offs );//span.a, span.b );
		}
#endif

		int	nonZ = -1;
		double	minSide = -99;
		double	maxSide = 99;
		double	curOffs = -car->_trkPos.toMiddle;
		{for( int i = cnt - 1; i >= 0; i-- )
		{
			int	j = sort[i];
			Opponent::Info&	oi = m_opp[j].GetInfo();

			NAOUT( "%2d (%6.2f, %5.1f)  t %5.1f %5.1f  mp %6.1f  spd %5.1f rtv(%6.2f)  bo %5.1f\n",
				j, oi.sit.rdPX, oi.sit.rdPY,
				oi.newCatchTime, oi.newAheadTime, oi.newMidPos,
				oi.sit.spd, oi.newCatchSpd, oi.newBestOffset );

#ifdef USE_NEW_AVOIDANCE_GFOUT
			if( oi.newPiL.isSpace )
			{
				GfOut( "    L %5.2f  mySpd %5.1f  goodPath %d  myOffset %5.1f (%6.3f %6.3f)\n",
						oi.newPiL.offset, oi.newPiL.mySpeed, oi.newPiL.goodPath,
						oi.newPiL.myOffset, oi.newPiL.bestU, oi.newPiL.bestV );
			}
			if( oi.newPiR.isSpace )
			{
				GfOut( "    R %5.2f  mySpd %5.1f  goodPath %d  myOffset %5.1f (%6.3f %6.3f)\n",
						oi.newPiR.offset, oi.newPiR.mySpeed, oi.newPiR.goodPath,
						oi.newPiR.myOffset, oi.newPiR.bestU, oi.newPiR.bestV );
			}
#endif

			if( oi.newCatchTime > 0 )
				nonZ = j;
			if( oi.newCatchTime == 0 )
			{
//				if( curOffs < oi.newSpan.b )
//					maxSide = MN(maxSide, oi.newSpan.a);
//				if( curOffs > oi.newSpan.a )
//					minSide = MX(minSide, oi.newSpan.b);
			}
		}}

/*
		if( nonZ >= 0 )
		{
			double	oppTarg;
			Opponent::Info&	oi = m_opp[nonZ].GetInfo();
			if( oi.newBestOffset < oi.newSpan.a ||
				oi.newBestOffset > oi.newSpan.b )
				oppTarg = oi.newBestOffset;
			else if( oi.newBestOffset - oi.newSpan.a <
					 oi.newSpan.b - oi.newBestOffset )
				oppTarg = oi.newSpan.a;
			else
				oppTarg = oi.newSpan.b;

			CarElt*			oCar = m_opp[nonZ].GetCar();
			double	u, v;
			CalcBestPathUV( oCar->_distFromStartLine, oppTarg, u, v );
			targ = CalcPathOffset(car->_distFromStartLine, newAvoidS, newAvoidT );
		}
*/
		if( targ < rootSpan.a )
			targ = rootSpan.a;
		else if( targ > rootSpan.b )
			targ = rootSpan.b;

		NAOUT( "[%.1f %.1f]  nonZ %d\n", rootSpan.a, rootSpan.b, nonZ );

		CalcBestPathUV( car->_distFromStartLine, targ, newAvoidS, newAvoidT );
//		newAvoidT = CalcPathTarget( car->_distFromStartLine, targ );
//		if( newAvoidT != 0 )
//			newAvoidS = 0;
//		else
//			newAvoidS = 1;

		if( toSideL || toSideR )
		{
//			if( newAvoidS != 1 )
			{
				newAvoidS = 0;
				newAvoidT = CalcPathTarget(car->_distFromStartLine, targ);
			}
		}

		NAOUT( "pos %.1f  targ %.1f  newA %5.2f %5.2f (av %5.2f %5.2f)\n",
				car->_distFromStartLine, targ, newAvoidS, newAvoidT,
				m_avoidS, m_avoidT );
	}
	} // OT_MOD
#endif

	// accumulate all the collision the flags...
	Avoidance::Info	ai;
	double	minCatchTime = 99;
	double	minCatchAccTime = 99;
	double	minVCatTime = 99;
	lapper = false;

	double	width = m_track.GetWidth();
	ai.aheadSpan = Span(-width / 2, width / 2);
	ai.sideSpan = ai.aheadSpan;

	PtInfo	pi;
	GetPtInfo( PATH_NORMAL, car->_distFromStartLine, pi );
	ai.bestPathOffs = pi.offs;
	CarElt *closestRCar = NULL;

	{
		double closedist = 10000.0;

		for( int i = 0; i < m_nCars; i++ )
		{
			Opponent::Info&	oi = m_opp[i].GetInfo();
			double dist = car->_distFromStartLine - m_opp[i].GetCar()->_distFromStartLine;
	
			if (m_opp[i].GetCar() == car)
				continue;

			if (dist > -(car->_dimension_x) && dist < closedist)
			{
				closestRCar = m_opp[i].GetCar();
				closedist = dist;
			}
		}
	}

	{for( int i = 0; i < m_nCars; i++ )
	{
		Opponent::Info&	oi = m_opp[i].GetInfo();
		CarElt*			oCar = m_opp[i].GetCar();

		ai.flags |= oi.flags;

		if( oi.GotFlags(Opponent::F_FRONT) )
		{
			if( oi.flags & (Opponent::F_COLLIDE | Opponent::F_CATCHING |
							Opponent::F_CATCHING_ACC) )
			{
//				GfOut( "*** flags " );
//				if( oi.GotFlags(Opponent::F_COLLIDE) )		GfOut( "COLLIDE " );
//				if( oi.GotFlags(Opponent::F_CATCHING) )		GfOut( "CATCH " );
//				if( oi.GotFlags(Opponent::F_CATCHING_ACC) )	GfOut( "CATCH_A " );
//				if( oi.GotFlags(Opponent::F_CLOSE) )		GfOut( "CLOSE " );
//				GfOut( "\n" );
//				GfOut( "t %g   catchY %g  catchSpd %g  catchDec %g\n",
//						oi.catchTime, oi.catchY, oi.catchSpd, oi.catchDecel );
			}

			if( oi.GotFlags(Opponent::F_COLLIDE) &&
//				oi.catchTime < 3 && fabs(oi.catchY) < oi.sit.minDY )
//				oi.catchDecel > 5 & fabs(oi.catchY) < oi.sit.minDY )
				oi.catchDecel > 12.5 * car->_trkPos.seg->surface->kFriction )
			{
				ai.spdF = MN(ai.spdF, oi.catchSpd);
			}

			if( oi.flags & (Opponent::F_COLLIDE | Opponent::F_CATCHING) )
				minCatchTime = MN(minCatchTime, oi.catchTime);
			if( oi.flags & Opponent::F_CATCHING_ACC )
				minCatchAccTime = MN(minCatchAccTime, oi.catchAccTime);
			if( oi.sit.rdVX < 0 )
			{
				double	vCatTime = -(oi.sit.rdPX - oi.sit.minDX) / oi.sit.rdVX;
				if( vCatTime > 0 )
					minVCatTime = MN(minVCatTime, vCatTime);
			}

			bool	ignoreTeamMate = oi.GotFlags(Opponent::F_TEAMMATE) &&
										(car->_laps < oCar->_laps ||
										 car->_dammage + 200 >= oi.tmDamage);

			oi.avoidLatchTime = MX(0, oi.avoidLatchTime - s->deltaTime);

			double	maxSpdK = 15.0 / (110 * 110);
			double	colTime = fabs(k) > maxSpdK ? 0.5 : 0.7;
			double	catTime = fabs(k) > maxSpdK ? 0.5 : 2.5;
			double	cacTime = fabs(k) > maxSpdK ? 0.5 : 2.5;
			bool	catching = (oi.newPiR.goodPath || oi.newPiL.goodPath) &&
				 (oi.catchTime    < colTime && oi.GotFlags(Opponent::F_COLLIDE)  ||
				  oi.catchTime    < catTime && oi.GotFlags(Opponent::F_CATCHING) ||
				  oi.catchAccTime < cacTime && oi.GotFlags(Opponent::F_CATCHING_ACC));

			if( !ignoreTeamMate &&
//				oi.catchTime < 10 &&
//				(oi.GotFlags(Opponent::F_CATCHING) ||
//				 oi.GotFlags(Opponent::F_CATCHING_ACC)) )
				(//oi.GotFlags(Opponent::F_CLOSE) ||
				 oi.avoidLatchTime > 0 || catching ||
				 oi.GotFlags(Opponent::F_DANGEROUS)) )
			{
				double	toL, toR;
				GetPathToLeftAndRight( oCar, toL, toR );
				toL += oi.sit.tVY * oi.catchTime;
				toR -= oi.sit.tVY * oi.catchTime;
				bool	spaceL = toL > oi.sit.minDY;// + 0.25;
				bool	spaceR = toR > oi.sit.minDY;// + 0.25;
				bool	avoidL = oi.sit.rdPY < 0 && spaceR;
				bool	avoidR = oi.sit.rdPY > 0 && spaceL;

				if( catching )
					oi.avoidLatchTime = fabs(k) < maxSpdK ? 0.5 : 0.1;

//				bool	avoidL = oi.sit.rdPY < -oi.sit.minDY && spaceR;
//				bool	avoidR = oi.sit.rdPY >  oi.sit.minDY && spaceL;

				if( fabs(k) < maxSpdK )
				{
					if( !avoidL && !avoidR )
					{
						avoidL = !spaceL && spaceR;
						avoidR = !spaceR && spaceL;
					}
/*					if( spaceL && spaceR )
					{
						if( oi.sit.rdPY < -oi.sit.minDY )
							avoidR = true;
						else if( oi.sit.rdPY >  oi.sit.minDY )
							avoidL = true;
						else if( spaceL && spaceR )
						{
							avoidL = toL < toR;
							avoidR = !avoidL;
						}
					}*/
				}

//				bool	avoidL = oi.sit.rdPY < 0 && oCar->pub.trkPos.toRight > 4.0 ||
//								 oi.sit.rdPY > 0 && oCar->pub.trkPos.toLeft  < 4.0;
//				bool	avoidL = oi.sit.offs < 0;

				if( avoidL )
				{
					ai.avoidAhead |= Opponent::F_LEFT;
					ai.aheadSpan.ExcludeLeftOf(-oCar->_trkPos.toMiddle + oi.sit.minDY);
				}
				if( avoidR )
				{
					ai.avoidAhead |= Opponent::F_RIGHT;
					ai.aheadSpan.ExcludeRightOf(-oCar->_trkPos.toMiddle - oi.sit.minDY);
				}

				if( avoidL )
//					minLDist = MN(oi.sit.rdPX, minLDist);
					ai.minLDist = MN(oi.sit.ragVX, ai.minLDist);
				if( avoidR )
//					minRDist = MN(oi.sit.rdPX, minRDist);
					ai.minRDist = MN(oi.sit.ragVX, ai.minRDist);
			}
		}

		if( oi.GotFlags(Opponent::F_TO_SIDE) )
		{
			int	av = oi.sit.rdPY < 0 ? Opponent::F_LEFT : Opponent::F_RIGHT;

			ai.avoidToSide |= av;

			if( oi.sit.rdPY < 0 )
			{
				ai.minLSideDist = MN(ai.minLSideDist, -oi.sit.rdPY - oi.sit.minDY);
				ai.sideSpan.ExcludeLeftOf(-oCar->_trkPos.toMiddle + oi.sit.minDY);
			}
			else
			{
				ai.minRSideDist = MN(ai.minRSideDist,  oi.sit.rdPY - oi.sit.minDY);
				ai.sideSpan.ExcludeRightOf(-oCar->_trkPos.toMiddle - oi.sit.minDY);
			}

		}

		if( oi.GotFlags(Opponent::F_AHEAD) )
		{
			if( ai.pClosestAhead == 0 ||
				ai.pClosestAhead->sit.rdPX > oi.sit.rdPX )
			{
				ai.pClosestAhead = &oi;
			}
		}

		bool	treatTeamMateAsLapper =
				       (oi.GotFlags(Opponent::F_TEAMMATE | Opponent::F_REAR) &&
					oi.sit.relPos > -25 &&
					car->_laps == oCar->_laps &&
					oCar == closestRCar &&
					car->_dammage > oi.tmDamage + 300);

		if( STAY_TOGETHER > 50 &&
		    oi.GotFlags(Opponent::F_TEAMMATE | Opponent::F_REAR) &&
		    oi.sit.relPos < -25 && oi.sit.relPos > -STAY_TOGETHER &&
		    closestRCar == oCar &&
		    car->_dammage + 2000 > oi.tmDamage )
		{
//			treatTeamMateAsLapper = true;
			lapper = true; 
		}

		if( oi.GotFlags(Opponent::F_LAPPER) || treatTeamMateAsLapper )
		{
			int	av = oi.sit.rdPY < 0 ? Opponent::F_LEFT : Opponent::F_RIGHT;
			ai.avoidLapping |= av;
			lapper = true;
		}
	}}

	ai.k = k;
	ai.nextK = k;

	NAOUT( "ss %5.1f %5.1f  as %5.1f %5.1f  bpo %5.1f\r",
			ai.sideSpan.a, ai.sideSpan.b, ai.aheadSpan.a, ai.aheadSpan.b,
			ai.bestPathOffs );

	double	pos = car->_distFromStartLine;
	int		carIdx = m_track.IndexFromPos(m_track.CalcPos(car));
	ai.k = 	m_path[PATH_NORMAL].GetAt(carIdx).k;
	int		NSEG = m_track.GetSize();
	for( int i = 1; i < NSEG; i++ )
	{
		int	idx = (carIdx + i) % NSEG;
		double	thisK = m_path[PATH_NORMAL].GetAt(idx).k;
		if( fabs(thisK) > 0.01 )
		{
			ai.nextK = thisK;
			break;
		}
	}

	GenericAvoidance		ga;
	InsideLineAvoidance		ila;
	OutsideLineAvoidance	ola;

	int		priority = ga.priority(ai, car);
	Vec2d	target = ga.calcTarget(ai, car, *this);

#if defined(USE_NEW_AVOIDANCE)
	if (OT_MOD)
	{
	  target.x = newAvoidT;
	  target.y = 1 - newAvoidS;
	}
#endif

//	GfOut( "tx %g  ty %g\n", target.x, target.y );
/*
	if( priority < ila.priority(ai, car) )
	{
		priority = ila.priority(ai, car);
		target = ila.calcTarget(ai, car, *this);
	}

	if( priority < ola.priority(ai, car) )
	{
		priority = ola.priority(ai, car);
		target = ola.calcTarget(ai, car, *this);
	}
*/
//	GfOut( "****** avoid: %d%d%d  k %6.3f  nextK %6.4f  pri %d  target: %6.3f\n",
//			ai.avoidToSide, ai.avoidLapping, ai.avoidAhead, ai.k, ai.nextK,
//			priority, target );

	carTargetSpd = MN(carTargetSpd, ai.spdF);
	close = (ai.flags & Opponent::F_CLOSE) != 0;
//	lapper = ai.avoidLapping != 0;

	if( m_flying )
		return;

//	double	w = width;
	double	w = AVOID_WIDTH * 2 + width;
	double	scale = 25.0 / w;
//	scale *= (w * 0.5 + 1) / (AVOID_WIDTH * 2 + w * 0.5);
	double	avoidSMaxA = 0.00075 * scale;
	double	avoidSMaxV = 0.005 * scale;
//	double	avoidSMaxA = 0.00025 * scale;
//	double	avoidSMaxV = 0.0015 * scale;

//	double	avoidTMaxA = 0.0002;
	double	avoidTMaxA = 0.0003 * scale;
//	double	avoidTMaxA = 0.001 * scale;
	double	avoidTMaxV = 0.2 * scale;

//	avoidSMaxA *= 0.5;
//	avoidSMaxV *= 0.5;
//	avoidTMaxA *= 0.66;
//	avoidTMaxV *= 0.66;

//	if( avoidToSide && fabs(minSideDist) < 4 )
//	{
//		avoidMaxA    *= 2;
//		avoidMaxVNeg *= 2;
//		avoidMaxVPos *= 2;
//	}

/*
	m_attractor = 0.0;
	if( m_followPath == PATH_LEFT && m_avoidT <= 0 )
		m_attractor = -1.0;
	else if( m_followPath == PATH_RIGHT && m_avoidT >= 0 )
		m_attractor = 1.0;
*/
//	if( m_avoidT * target >= 0 )
		m_attractor = target.x;

//	if( m_avoidS == 1 && m_attractor != 0 )
//		m_avoidT = m_attractor;

	double	targetS = 1 - target.y;
	if( m_avoidS != 1 && m_attractor == 0 ||
		m_avoidS != targetS && m_attractor != 0 )
	{
		targetS = (m_attractor == 0) ? 1 : 0;//0.35;
		double	avoidA = targetS > m_avoidS ? avoidSMaxA : -avoidSMaxA;

		double	dist = targetS - m_avoidS;
		if( fabs(dist) < 0.0005 )
			m_avoidSVel = 0;
		else
		{
			double	slowS = (m_avoidSVel * m_avoidSVel) / (2 * avoidSMaxA);
			if( fabs(dist) <= slowS )
			{
				avoidA = -(m_avoidSVel * m_avoidSVel) / (2 * dist);
			}

			m_avoidSVel += avoidA;
		}
	}
	else
		m_avoidSVel = 0;

	if( m_avoidSVel > avoidSMaxV )
		m_avoidSVel = avoidSMaxV;
	else if( m_avoidSVel < -avoidSMaxV )
		m_avoidSVel = -avoidSMaxV;

	double	oldAvoidS = m_avoidS;
	m_avoidS += m_avoidSVel;
	if( m_avoidS < 0.0005 && m_avoidSVel < 0 )
	{
//		GfOut( "m_avoidS %g (v %g)\n", m_avoidS, m_avoidSVel );
//		GfOut( "m_avoidS = %g (min limit)\n", 0.0 );
		m_avoidS = 0;
		m_avoidSVel = 0;
	}
	else if( m_avoidS >= 0.9995 && m_avoidSVel > 0 )
	{
//		GfOut( "m_avoidS = %g (max limit)\n", 1.0 );
		m_avoidS = 1;
//		m_avoidT = 0;
		m_avoidSVel = 0;
	}
	else if( (oldAvoidS < targetS && m_avoidS >= targetS) ||
	         (oldAvoidS > targetS && m_avoidS <= targetS) ||
	         fabs(targetS - m_avoidS) < 0.0005 )
	{
//		GfOut( "m_avoidS = %g (hit target)\n", targetS );
		double targS = (m_avoidS < targetS ? 
		                (MIN(targetS, m_avoidS + ((100.0 - MIN(70.0, MAX(25.0, car->_speed_x))) / 500) * AVOIDMOVT)) :
		                (MAX(targetS, m_avoidS - ((100.0 - MIN(70.0, MAX(25.0, car->_speed_x))) / 500) * AVOIDMOVT)));

		m_avoidS = targS;
		m_avoidSVel = 0;
	}

//	avoidMaxVNeg *= 2;
//	avoidMaxVPos *= 2;

//	m_avoidS = 1;
//	GfOut( "**** avoidS %g\n", m_avoidS );

//	GfOut( "at %8.5f ts %3.1f as %8.5f asv %8.5f att %5.2f minCat %5.2f/%5.2f/%5.2f\r",
//			m_avoidT, targetS, m_avoidS, m_avoidSVel, m_attractor,
//			minCatchTime, minCatchAccTime, minVCatTime );

	double	attractT = m_attractor;
//	if( attractT == 0 )
//		attractT = m_avoidT;
	double	avoidA = 0;
	if( attractT != m_avoidT )
	{
		double	tMaxA = avoidTMaxA / MX(0.2, 1 - m_avoidS);
//		avoidA = attractT > m_avoidT ? avoidTMaxA : -avoidTMaxA;
		avoidA = attractT > m_avoidT ? tMaxA : -tMaxA;
//		double	slowA  = -(m_avoidTVel * m_avoidTVel) / (2 * (attractor - m_avoidT));
//		if( fabs(slowA) >= avoidMaxA )
//			avoidA = SGN(slowA) * avoidMaxA;

		double	dist = attractT - m_avoidT;
		double	slowS = (m_avoidTVel * m_avoidTVel) / (2 * avoidTMaxA);
		if( dist * m_avoidTVel > 0 && fabs(dist) <= slowS )
		{
			avoidA = -(m_avoidTVel * m_avoidTVel) / (2 * dist);
		}

//		GfOut( "##### avoidA %7.4f   attractor %g\n", avoidA, attractor );
		if( avoidA > avoidTMaxA )
			avoidA = avoidTMaxA;
		else if( avoidA < -avoidTMaxA )
			avoidA = -avoidTMaxA;

		m_avoidTVel += avoidA;
	}
	else
		m_avoidTVel = 0;

	double	tMaxV = avoidTMaxV / MX(0.2, 1 - m_avoidS);
	if( m_avoidTVel > tMaxV )
		m_avoidTVel = tMaxV;
	else if( m_avoidTVel < -tMaxV )
		m_avoidTVel = -tMaxV;

	double	oldAvoidT = m_avoidT;
	m_avoidT += m_avoidTVel;
//	if( fabs(m_avoidT - attractor) <= 0.005 )
/*	if( fabs(m_avoidT - attractT) <= avoidTMaxA * 1.01 ||
		m_avoidS >= 0.99 )
	{
		m_avoidT = attractT;
		m_avoidTVel = 0;
	}
*/
	if( m_avoidT < -1 )
	{
		m_avoidT = -1;
		m_avoidTVel = 0;
	}
	else if( m_avoidT > 1 )
	{
		m_avoidT = 1;
		m_avoidTVel = 0;
	}
	else if( oldAvoidT < attractT && m_avoidT >= attractT ||
			 oldAvoidT > attractT && m_avoidT <= attractT )
	{
		m_avoidT = attractT;
		m_avoidTVel = 0;
	}

#if defined(USE_NEW_AVOIDANCE)
	if (OT_MOD)
	{
		double	newScale = 0.5;
		m_avoidY.SetMaxAccel(    0.001 * newScale );
		m_avoidY.SetMaxVelocity( 0.010 * newScale );
		m_avoidX.SetMaxAccel(    0.002 * newScale );
		m_avoidX.SetMaxVelocity( 0.040 * newScale );
/*
	if( newAvoidS != 1 && m_avoidY.GetValue() == 1 )
	{
		m_avoidX.SetValue( newAvoidT );
	}
*/
		m_avoidY.Update( newAvoidS );
		m_avoidX.Update( newAvoidT );

		NAOUT( "linAtt %6.3f %6.3f -- targ %6.3f %6.3f\n",
				m_avoidY.GetValue(), m_avoidX.GetValue(), targetS, attractT );

		m_avoidS = m_avoidY.GetValue();
		m_avoidT = m_avoidX.GetValue();
	}
#endif

//	double	offs = CalcPathOffset(pos, m_avoidS, m_avoidT);
//	CalcBestPathUV(pos, offs, m_avoidU, m_avoidV);

//	GfOut( "***** target %5.2f  avS %.3f avT %.4f  avTV %.5f  avTA %.5f\n",
//			target, m_avoidS, m_avoidT, m_avoidTVel, avoidA );
//	GfOut( "***** targ %5.2f  offs %5.2f  s %.3f  t %.3f  u %.3f  v %.3f\n",
//			target, offs, m_avoidS, m_avoidT, m_avoidU, m_avoidV );
}

int		MyRobot::CalcGear( tCarElt* car, double& acc )
{
    if( car->_gear <= 0 )
		return 1;

//	const int	MAX_GEAR = 7;
	const int	MAX_GEAR = car->_gearNb - 1;

	double	gr_dn = car->_gear > 1 ?
				car->_gearRatio[car->_gear + car->_gearOffset - 1] :
				1e5;
	double	gr_this = car->_gearRatio[car->_gear + car->_gearOffset];

	double	wr = (car->_wheelRadius(2) + car->_wheelRadius(3)) / 2;
	double	rpm = gr_this * car->_speed_x / wr;
//	double	rpm = car->_enginerpm;

	double	rpmUp = m_gearUpRpm;
	double	rpmDn = rpmUp * gr_this * 0.9 / gr_dn;

//	GfOut( "gear %d    rpm %6.1f %6.1f    dist %6.1f  up dist %6.1f   down dist %6.1f\n",
//		car->_gear, rpm, car->_enginerpm, grDist, upDist, dnDist );

    if( car->_gear < MAX_GEAR && rpm > rpmUp )
	{
		car->ctrl.clutchCmd = 1.0;
//		acc = 0.5;
        return car->_gear + 1;
    }
	else if( car->_gear > 1 && rpm < rpmDn )
	{
//		car->ctrl.clutchCmd = 1.0;
//		acc = 1.0;
		return car->_gear - 1;
	}

    return car->_gear;
}

double	MyRobot::ApplyAbs( tCarElt* car, double brake )
{
	if( car->_speed_x < 10 )
		return brake;

	double	slip = 0.0;
	for( int i = 0; i < 4; i++ )
		slip += car->_wheelSpinVel(i) * car->_wheelRadius(i);
	slip = car->_speed_x - slip/4.0;

#if 1
	if (slip > 2.5)
		brake = brake * (1.0 - MIN(0.8, (slip - 2.5) / 4.5));
#else
	if( slip < 0.9 )//ABS_SLIP )
	{
//		GfOut( "ABS ABS ABS ABS ABS ABS ABS  slip %g\n", slip );
//		brake *= slip;
		brake *= 0.5;
	}
#endif

	return brake;
}

double	MyRobot::ApplyTractionControl( tCarElt* car, double acc )
{
	double	spin = 0;
	double	wr = 0;
	int		count = 0;

	if( m_driveType == cDT_FWD || m_driveType == cDT_4WD )
	{
		spin += car->_wheelSpinVel(FRNT_LFT) * car->_wheelRadius(FRNT_LFT);
		spin += car->_wheelSpinVel(FRNT_RGT) * car->_wheelRadius(FRNT_RGT);
		wr += car->_wheelRadius(FRNT_LFT) + car->_wheelRadius(FRNT_RGT);
		count += 2;
	}

	if( m_driveType == cDT_RWD || m_driveType == cDT_4WD )
	{
		spin += car->_wheelSpinVel(REAR_LFT) * car->_wheelRadius(REAR_LFT);
		spin += car->_wheelSpinVel(REAR_RGT) * car->_wheelRadius(REAR_RGT);
		wr += car->_wheelRadius(REAR_LFT) + car->_wheelRadius(REAR_RGT);
		count += 2;
	}

	static double	tract = 1.0;

	spin /= count;

	if( car->_speed_x < 0.01 )
		return acc;

	double	slip = car->_speed_x / spin;

    if( slip > 1.1 )
	{
		tract = 0.1;

		wr /= count;
		double	gr = car->_gearRatio[car->_gear + car->_gearOffset];
		double	rpmForSpd = gr * car->_speed_x / wr;
//		GfOut( "TC:  slip %g   rpmForSpd %g\n",
//					slip, rpmForSpd );
//		acc = 0.5 * rpmForSpd / car->_enginerpmRedLine;
		acc = 0;
	}
	else
	{
		tract = MN(1.0, tract + 0.1);
	}

	acc = MN(acc, tract);

	return acc;
}

