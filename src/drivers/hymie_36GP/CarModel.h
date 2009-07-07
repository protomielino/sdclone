/***************************************************************************

    file        : CarModel.h
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

// CarModel.h: interface for the CarModel class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CARMODEL_H__60AA9DE0_703F_4684_BCBE_35873DC5C9A6__INCLUDED_)
#define AFX_CARMODEL_H__60AA9DE0_703F_4684_BCBE_35873DC5C9A6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

struct PathMod {
	int bgn_div;
	int end_div;
	int AERO;
	int BRAKE_MOD;
	double MU_SCALE;
	double KZ_SCALE;
	double BRK_SCALE;
	double BRK_MU_SCALE;
	double AHEAD_FACTOR;
	double  LFT_MARGIN;
	double  RGT_MARGIN;
};

class CarModel  
{
public:
	CarModel();
	~CarModel();

	double	CalcMaxSpeed( int div, double k, double kz, double kFriction,
						  double rollAngle ) const;

	double	CalcBreaking( int div, double k0, double kz0, double k1, double kz1,
						  double spd1, double dist, double kFriction,
						  double trackRollAngle ) const;

	double	CalcAcceleration( double k0, double kz0, double k1, double kz1,
						  double spd0, double dist, double kFriction,
						  double trackRollAngle ) const;
	double	CalcMaxSpdK() const;
	double	CalcMaxLateralF( double spd, double kFriction ) const;

	void	CalcSimuSpeeds( double spd0, double dy, double dist, double kFriction,
							double& minSpd, double& maxSpd ) const;
	void	CalcSimuSpeedRanges( double spd0, double dist, double kFriction,
								 double& minSpd, double& maxSpd, double& maxDY ) const;

public:
	int		AERO;		// which aero calc to use.
	int     BRAKE_MOD ;	// which brake calc to use.
	double	AHEAD_FACTOR;	// how far ahead to look in steering calcs
	double	MASS;		// fixed mass of car.
	double	FUEL;		// mass of fuel in car.
	double	DAMAGE;		// damage of this car.
	double	TYRE_MU;	// mu value of tyres (min of those avail).
	double	TYRE_MU_F;	// mu value of front tyres.
	double	TYRE_MU_R;	// mu value of rear  tyres.
	double	MU_SCALE;	// scaling of MU to use for this car.
	double	CA;			// aerodynamic downforce constant -- total.
	double	CA_FW;		// aerodynamic downforce constant -- front wing.
	double	CA_RW;		// aerodynamic downforce constant -- rear wing.
	double	CA_GE;		// aerodynamic downforce constant -- ground effect.
	double	CD_BODY;	// aerodynamic drag constant -- car body.
	double	CD_WING;	// aerodynamic drag constant -- wings.
	double	KZ_SCALE;	// bump sensitivity.
	double	WIDTH;		// width of car (m).
	double  BRK_SCALE;      // affect braking distances
	double  BRK_MU_SCALE;      // affect braking distances
	double  LFT_MARGIN;
	double  RGT_MARGIN;
	double  RI_CUTOFF;
	struct PathMod m_pm[64];
	int	m_pmused;
};

#endif // !defined(AFX_CARMODEL_H__60AA9DE0_703F_4684_BCBE_35873DC5C9A6__INCLUDED_)
