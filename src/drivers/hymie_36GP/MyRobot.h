/***************************************************************************

    file        : MyRobot.h
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

// MyRobot.h: interface for the MyRobot class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MYROBOT_H__AA11EF80_E44F_4A64_B4ED_36FDD5FE6C69__INCLUDED_)
#define AFX_MYROBOT_H__AA11EF80_E44F_4A64_B4ED_36FDD5FE6C69__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <track.h>
#include <car.h>
#include <robot.h>

#include "MyTrack.h"
#include "Shared.h"
#include "ClothoidPath.h"
#include "OptimisedPath.h"
#include "PitPath.h"
#include "PitControl.h"
#include "Opponent.h"
#include "PidController.h"
#include "LearnedGraph.h"
#include "AveragedData.h"
#include "LinearRegression.h"
#include "LinearAttractor.h"

const double	SPD_MIN = 0;
const double	SPD_MAX = 100;
const int		SPD_N = 20;
const double	SPD_STEP = (SPD_MAX - SPD_MIN) / SPD_N;
const double	K_MIN = -0.1;
const double	K_MAX = 0.1;
const int		K_N = 100;
const double	K_STEP = (K_MAX - K_MIN) / K_N;


class MyRobot  
{
public:
	enum	// paths
	{
		PATH_NORMAL,
		PATH_LEFT,
		PATH_RIGHT,

		N_PATHS,
	};

	enum
	{
		STEER_SPD_MAX = 20,
		STEER_K_MAX = 41,
		HIST = 20,
	};

public:
	MyRobot();
	~MyRobot();

	void	SetShared( Shared* pShared );
	void	InitTrack( int index, tTrack* track, void* carHandle,
						void** carParmHandle, tSituation* s);
	void	NewRace( int index, tCarElt* car, tSituation* s );

	void	GetPtInfo( int path, double pos, PtInfo& pi ) const;
	void	GetPosInfo( double pos, PtInfo& pi, double u, double v ) const;
	void	GetPosInfo( double pos, PtInfo& pi ) const;
	double	CalcPathTarget( double pos, double offs, double s ) const;
	double	CalcPathTarget( double pos, double offs ) const;
	Vec2d	CalcPathTarget2( double pos, double offs ) const;
	double	CalcPathOffset( double pos, double s, double t ) const;
	void	CalcBestPathUV( double pos, double offs, double& u, double& v ) const;
	double	CalcBestSpeed( double pos, double offs ) const;
	void	GetPathToLeftAndRight( const CarElt* pCar, double& toL, double& toR ) const;

	double	SteerAngle0( tCarElt* car, PtInfo& pi, PtInfo& aheadPi );
	double	SteerAngle1( tCarElt* car, PtInfo& pi, PtInfo& aheadPi );
	double	SteerAngle2( tCarElt* car, PtInfo& pi, PtInfo& aheadPi );
	double	SteerAngle3( tCarElt* car, PtInfo& pi, PtInfo& aheadPi );
	double	SteerAngle4( tCarElt* car, PtInfo& pi, PtInfo& aheadPi );

	void	SpeedControl0( double targetSpd, double spd0, double& acc, double& brk );
	void	SpeedControl1( double targetSpd, double spd0, double& acc, double& brk );
	void	SpeedControl2( double targetSpd, double spd0, double& acc, double& brk );
	void	SpeedControl3( double targetSpd, double spd0, double& acc, double& brk );
	void	SpeedControl4( double targetSpd, double spd0, CarElt* car,
						   double& acc, double& brk );
	void	SpeedControl( int which, double targetSpd, double spd0,
						  CarElt* car, double& acc, double& brk );

	void	Drive( int index, tCarElt* car, tSituation* s );
	int		PitCmd( int index, tCarElt* car, tSituation* s );
	void	EndRace( int index, tCarElt* car, tSituation* s );
	void	Shutdown( int index);

private:
	void	ProcessOtherCars( int index, tCarElt* car, double spd, tSituation* s );
	void	AvoidOtherCars( int index, tCarElt* car, double k,
							double& carTargetSpd, tSituation* s, bool& inTraffic,
							bool& lapper );

	int		CalcGear( tCarElt* car, double& acc );
	double	ApplyAbs( tCarElt* car, double brake );
	double	ApplyTractionControl( tCarElt* car, double acc );
	void	CalcSkill( tSituation *s );
	void	SetRandomSeed( unsigned int seed ) { m_random_seed = (seed ? seed : 0xfded); }
	double	GetRandom() { m_random_seed = 1664525 * m_random_seed + 1013904223; return m_random_seed; }

private:
	enum	// drive types
	{
		cDT_RWD, cDT_FWD, cDT_4WD,
	};

	enum
	{
		cMAX_OPP = 64,
	};

	struct	PathRange
	{
		double	u;
		double	vL;
		double	vR;
		bool	gotL;
		bool	gotR;
		bool	racingLine;

		PathRange() : u(1), vL(-1), vR(1),
					  gotL(false), gotR(false), racingLine(true) {}

		void	AddGreater( double pos, double offs, bool incRL, const MyRobot& me );
		void	AddLesser( double pos, double offs, bool incRL, const MyRobot& me );
	};

private:
	Shared*			m_pShared;
	MyTrack			m_track;
//	ClothoidPath	m_path[N_PATHS];
	OptimisedPath	m_path[N_PATHS];
	PitPath			m_pitPath[N_PATHS];
	PitControl		m_pitControl;

	CarModel		m_cm;
	CarModel		m_cm2;
	CarElt*			m_pCar;

	double			FLY_HEIGHT;
	Array<double>	FACTORS;
	int				BUMP_MOD;
	int				STEER_MOD;
	int				SPDC_NORMAL;
	int				SPDC_TRAFFIC;
	double			STEER_K_ACC;
	double			STEER_K_DEC;
	double			STAY_TOGETHER;			// dist in m.
	double			AVOID_WIDTH;			// in m.
	double			PIT_ENTRY_OFFSET;		// dist in m.
	double			PIT_EXIT_OFFSET;		// dist in m.
	double			ACC_SCALE;
	int			OT_MOD;
	double			BRAKE_PRESSURE;
	double			SKIDSTEER;
	double			AVOIDMOVT;

	int				m_driveType;
	double			m_gearUpRpm;			// for gear changing.
	PidController	m_lineControl;			// controller for line error.
	PidController	m_velAngControl;		// controller for direction of car.
	PidController	m_angControl;			// controller for attack angle error.
	double			m_prevYawError;
	double			m_prevLineError;
	int				m_flying;				// for landing better.
	int				m_nCars;
	int				m_myOppIdx;
	Opponent		m_opp[cMAX_OPP];		// info about other cars.
	double			m_avgAY;
	bool			m_raceStart;
	double			m_avoidS;				// where we are LR->T (0..1).
	double			m_avoidSVel;
	double			m_avoidT;				// where we are L->R (-1..1).
	double			m_avoidTVel;
	double			m_avoidU;
	double			m_avoidV;
	double			m_attractor;			// where we want to be.
	double			m_skill;
	double			m_random_seed;
	double			m_aggression;
	double			m_skill_adjust_timer;
	double			m_skill_adjust_limit;
	double			m_skill_decel_adjust_targ;
	double			m_skill_brake_adjust_targ;
	double			m_skill_decel_adjust_perc;
	double			m_skill_brake_adjust_perc;
	double		m_reverseTime;
	double		m_realSpeed;
	int				m_followPath;			// path we want to follow;
	int			m_verbose;

	LinearAttractor	m_avoidX;
	LinearAttractor	m_avoidY;

	LearnedGraph	m_maxAccel;
//	LearnedGraph	m_maxDecel;
	double			m_angle[SPD_N][K_N];

	Vec2d			m_lastPts[HIST];
	double			m_lastSpd;
	double			m_lastAng;

	LinearRegression	m_accBrkCoeff;		// 
	double			m_brkCoeff[50];
	double			m_steerCoeff[STEER_SPD_MAX][STEER_K_MAX];
	LearnedGraph	m_steerGraph;
	AveragedData	m_steerAvg;
	int				m_lastB;
	double			m_lastBrk;
	double			m_lastTargV;
	double			m_steersum;
	double			m_carangle;
	double			m_speedangle;
	bool			m_toSideL;
	bool			m_toSideR;
	double			m_sideratio;
};

#endif // !defined(AFX_MYROBOT_H__AA11EF80_E44F_4A64_B4ED_36FDD5FE6C69__INCLUDED_)
