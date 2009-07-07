/***************************************************************************

    file        : CarModel.cpp
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

// CarModel.cpp: implementation of the CarModel class.
//
//////////////////////////////////////////////////////////////////////

#include "CarModel.h"
#include "Quadratic.h"

#include <math.h>
#include "Utils.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CarModel::CarModel()
:	AERO(0),
 	BRAKE_MOD(0),
	MASS(0),
	FUEL(0),
	DAMAGE(0),
	TYRE_MU(0),
	TYRE_MU_F(0),
	TYRE_MU_R(0),
	MU_SCALE(0),
	CA(0),
	CA_FW(0),
	CA_RW(0),
	CA_GE(0),
	CD_BODY(0),
	CD_WING(0),
	KZ_SCALE(0),
	WIDTH(2),
	BRK_SCALE(1),
	BRK_MU_SCALE(1),
	LFT_MARGIN(0),
	RGT_MARGIN(0),
	RI_CUTOFF(0)
{
}

//===========================================================================

CarModel::~CarModel()
{
}

//===========================================================================

double	CarModel::CalcMaxSpeed(
	int div,
	double k,
	double kz,
	double kFriction,
	double trackRollAngle ) const
{
	//
	//	Here we calculate the theoretical maximum speed at a point on the
	//	path.  This takes into account the curvature of the path (k), the
	//	grip on the road (mu), the downforce from the wings and the ground
	//	effect (CA), the tilt of the road (left to right slope) (sn)
	//	and the curvature of the road in z (kz).
	//
	//	There are still a few silly fudge factors to make the theory match
	//	with the reality (the car goes too slowly otherwise, aarrgh!).
	//

	double	M  = MASS + FUEL;

	double	mua, muf, mur;

	int aero = AERO;
	double mu_scale = MU_SCALE;
	double kz_scale = KZ_SCALE;
	double speed_limit = -1.0;

	k = MAX(0.0, fabs(k) - RI_CUTOFF);

	for (int i=0; i<m_pmused; i++)
	{
		if (div >= m_pm[i].bgn_div && div <= m_pm[i].end_div)
		{
			aero = m_pm[i].AERO;
			mu_scale = m_pm[i].MU_SCALE;
			kz_scale = m_pm[i].KZ_SCALE;
			speed_limit = m_pm[i].SPEED_LIMIT;
			break;
		}
	}

	if( aero == 1 )
	{
		double	MU_F = kFriction * TYRE_MU_F;
		double	MU_R = kFriction * TYRE_MU_R;

		muf = MU_F * mu_scale;
		mur = MU_R * mu_scale;
		mua = (MU_F + MU_R) * 0.5;
	}
	else
	{
		double	MU = kFriction * TYRE_MU;

		mua   = MU * mu_scale;// * 0.975;
	}

	double	cs = cos(trackRollAngle);
	double	sn = sin(trackRollAngle);

	double	absK = MX(0.001, fabs(k));
	double	sgnK = SGN(k);

	double	num, den;

	if( aero == 1 )
	{
		num = M * (cs * G * mua + sn * G * sgnK);
//		den = M * (absK - 0.1 * kz) -
		den = M * (absK - kz_scale * kz) -
					(CA_FW * muf + CA_RW * mur + CA_GE * mua);
	}
	else
	{
//		num = M * (G * mu + sn * G * sgnK);
		num = M * (cs * G * mua + sn * G * sgnK);
//		den = M * (absK - 0.00 * kz) - CA * mu_df;
//		den = M * (absK - 0.10 * kz) - CA * mu_df;
//		den = M * (absK - 0.20 * kz) - CA * mu_df;
//		den = M * (absK - 0.25 * kz) - CA * mu_df;
//		den = M * (absK - 0.29 * kz) - CA * mu_df;
//		den = M * (absK - 0.33 * kz) - CA * mu_df;
//		den = M * (absK - 0.42 * kz) - CA * mu_df;
		den = M * (absK - kz_scale * kz) - CA * mua; //mu_df;
	}

	if( den < 0.00001 )
		den = 0.00001;

	double	spd = sqrt(num / den);

	if( spd > 200 )
		spd = 200;
	if (speed_limit > 0)
		spd = MIN(spd, speed_limit);

	return spd;
}

#if 0
double	CarModel::CalcMaxSpeed(
	double k,
	double kz,
	double kFriction,
	double trackRollAngle ) const
{
	//
	//	Here we calculate the theoretical maximum speed at a point on the
	//	path.  This takes into account the curvature of the path (k), the
	//	grip on the road (mu), the downforce from the wings and the ground
	//	effect (CA), the tilt of the road (left to right slope) (sn)
	//	and the curvature of the road in z (kz).
	//
	//	There are still a few silly fudge factors to make the theory match
	//	with the reality (the car goes too slowly otherwise, aarrgh!).
	//

	double	M  = MASS + FUEL;

	double	MU = kFriction * TYRE_MU;
//	MU = MU * (1 - m_pPath[i].pSeg->pSeg->surface->kRoughness * 5);

	double	cs = cos(trackRollAngle);
	double	sn = sin(trackRollAngle);

	double	absK = MX(0.001, fabs(k));
	double	sgnK = SGN(k);

	double	mu    = MU * MU_SCALE;// * 0.975;
	double	mu_df = MU * MU_SCALE;//MU_DF_SCALE;

//	double	beta = 0.5 * 3 * absK;	// estimate of turning angle of front wheels.
//	mu = mu_df = 0.5 * mu * (1 + fabs(cos(beta)));
//	double	beta = 3 * absK;	// estimate of turning angle of front wheels.
//	mu = mu * fabs(cos(beta));

//	double	num = M * (G * mu + sn * G * sgnK);
	double	num = M * (cs * G * mu + sn * G * sgnK);
//	double	den = M * (absK - 0.00 * kz) - CA * mu_df;
//	double	den = M * (absK - 0.10 * kz) - CA * mu_df;
//	double	den = M * (absK - 0.20 * kz) - CA * mu_df;
//	double	den = M * (absK - 0.25 * kz) - CA * mu_df;
//	double	den = M * (absK - 0.29 * kz) - CA * mu_df;
//	double	den = M * (absK - 0.33 * kz) - CA * mu_df;
//	double	den = M * (absK - 0.42 * kz) - CA * mu_df;
	double	den = M * (absK - KZ_SCALE * kz) - CA * mu_df;

	if( den < 0.00001 )
		den = 0.00001;

	double	spd = sqrt(num / den);

	if( spd > 200 )
		spd = 200;

	return spd;
}
#endif
//===========================================================================

double	CarModel::CalcBreaking(
	int div,
	double k0, double kz0, double k1, double kz1,
	double spd1, double dist, double kFriction,
	double trackRollAngle ) const
{
	double brk_mu_scale = BRK_MU_SCALE;
	double brk_scale = BRK_SCALE;
	int aero = AERO;

	for (int i=0; i<m_pmused; i++)
	{
		if (div >= m_pm[i].bgn_div && div <= m_pm[i].end_div)
		{
			aero = m_pm[i].AERO;
			brk_scale = m_pm[i].BRK_SCALE;
			brk_mu_scale = m_pm[i].BRK_MU_SCALE;
			break;
		}
	}

	// when under braking we keep some grip in reserve.
	kFriction *= brk_mu_scale;

	double	M  = MASS + FUEL;

	double	MU = kFriction * TYRE_MU;
	double	MU_F = MU;
	double	MU_R = MU;
	if( AERO == 1 )
	{
		MU_F = kFriction * TYRE_MU_F;
		MU_R = kFriction * TYRE_MU_R;
		MU   = (MU_F + MU_R) * 0.5;
	}

	double	CD = CD_BODY * (1.0 + DAMAGE / 10000.0) + CD_WING;

	double	cs = cos(trackRollAngle);
	double	sn = sin(trackRollAngle);

	double	K  = (k0  + k1)  * 0.5;
	double	Kz = (kz0 + kz1) * 0.5;// * KZ_SCALE;
	if( Kz > 0 )
		Kz = 0;

	double	Gdown = cs * G;
	double	Glat  = sn * G;
	double	Gtan  = 0;	// TODO: track pitch angle.

	double	v = spd1;
	double	u = v;

//	double	dist = Utils::VecLenXY(m_pPath[i].CalcPt() -
//								   m_pPath[j].CalcPt());

	for( int count = 0; count < 100; count++ )
	{
		double	avgV = (u + v) * 0.5;
		double	avgVV = avgV * avgV;
		double  brk_scale2 = brk_scale * MAX(0.5, 1.0 - fabs(Kz)*10);

		double	Froad;
		if( aero == 1 )
		{
			double	Fdown = M * Gdown + (M * Kz * avgVV + CA_GE) * avgVV;
			double	Ffrnt = CA_FW * avgVV;
			double	Frear = CA_RW * avgVV;

			Froad = Fdown * MU + Ffrnt * MU_F + Frear * MU_R;
		}
		else
		{
			double	Fdown = M * Gdown + (M * Kz * avgVV + CA) * avgVV;

			Froad = Fdown * MU;
		}
		double	Flat  = M * Glat;
		double	Ftan  = M * Gtan - CD * avgVV;

		double	Flatroad = fabs(M * avgVV * K - Flat);
		if( Flatroad > Froad )
			Flatroad = Froad;
		double	Ftanroad = -sqrt(Froad * Froad - Flatroad * Flatroad) + Ftan;

		double	acc = brk_scale2 * Ftanroad / M;// * 0.95;

//		GfOut( "%4d K %7.4f  Glat %.3f\n",
//				i, K, Glat );
//		GfOut( "%4d K %7.4f  Fr %.3f  Fl %.3f  Ft %.3f  u %.1f  acc %.1f\n",
//				i, K, Froad, Flat, Ftan, u, acc );
//		GfOut( "%4d K %7.4f  Flr %.3f  Ftr %.3f  u %.1f  acc %.1f\n",
//				i, K, Flatroad, Ftanroad, u, acc );

		double	inner = MX(0, v * v - 2 * acc * dist );
		double	oldU = u;
		u = sqrt(inner);
		if( fabs(u - oldU) < 0.001 )
			break;
	}

	return u;
}

//===========================================================================

double	CarModel::CalcAcceleration(
	double k0, double kz0, double k1, double kz1,
	double spd0, double dist, double kFriction,
	double trackRollAngle ) const
{
	double	M  = MASS + FUEL;
	double	MU = kFriction * TYRE_MU;
	double	CD = CD_BODY * (1.0 + DAMAGE / 10000.0) + CD_WING;

	double	cs = cos(trackRollAngle);
	double	sn = sin(trackRollAngle);

	double	K  = (k0  + k1)  * 0.5;
	double	Kz = (kz0 + kz1) * 0.5;// * KZ_SCALE;
	if( Kz > 0 )
		Kz = 0;

	double	Gdown = cs * G;
	double	Glat  = sn * G;
	double	Gtan  = 0;	// TODO: track pitch angle.

	double	u = spd0;
	double	v = u;

	// 30m/ss @ 0m/s
	//  3m/ss @ 60m/s
	//	1m/ss @ 75m/s
	//	0m/ss @ 85m/s
	//Quadratic	accFromSpd(21.0/5400, -43.0/60, 30);	// approx. clkdtm
	Quadratic	accFromSpd(0.001852,  -0.35,  17.7);	// approx. car4-trb1

	// Power (kW) = Torque (Nm) x Speed (RPM) / 9.5488

	for( int count = 0; count < 100; count++ )
	{
		double	avgV = (u + v) * 0.5;
		double	vv = avgV * avgV;

		double	Fdown = M * Gdown + (M * Kz * vv + CA) * vv;
		double	Froad = Fdown * MU;
		double	Flat  = M * Glat;
		double	Ftan  = M * Gtan - CD * vv;

		double	Flatroad = fabs(M * vv * K - Flat);
		if( Flatroad > Froad )
			Flatroad = Froad;
		double	Ftanroad = sqrt(Froad * Froad - Flatroad * Flatroad) + Ftan;

		double	acc = Ftanroad / M;
		double	maxAcc = accFromSpd.CalcY(avgV);
		if( acc > maxAcc )
			acc = maxAcc;

		double	inner = MX(0, u * u + 2 * acc * dist );
		double oldV = v;
		v = sqrt(inner);
		if( fabs(v - oldV) < 0.001 )
			break;
	}

	return v;
}

//===========================================================================

double	CarModel::CalcMaxSpdK() const
{
	const double	MAX_SPD = 110;	// ~400 kph
	double	maxSpdK = G * TYRE_MU / (MAX_SPD * MAX_SPD);
	return maxSpdK;
}

//===========================================================================

double	CarModel::CalcMaxLateralF( double spd, double kFriction ) const
{
	double	M  = MASS + FUEL;
	double	MU = kFriction * TYRE_MU;

	double	vv = spd * spd;

	double	Fdown = M * G + /*M * Kz * vv*/ + CA * vv;
	double	Flat  = Fdown * MU;

	return Flat;
}

//===========================================================================

void	CarModel::CalcSimuSpeeds(
	double	spd0,
	double	dy,
	double	dist,
	double	kFriction,
	double&	minSpd,
	double&	maxSpd ) const
{
	// simple speed calc for use in simulation for path optimisation... the
	//	overriding pre-requisite of which is speed of calculation.
	//
	// a = v*v/r
	// max_a = M * G * MU;
	// max_spd = sqrt(max_a r) = sqrt(M * G * MU / k)

	double	M  = MASS + FUEL;
	double	MU = kFriction * TYRE_MU;

	double	max_acc = G * MU;
//	double	max_spd = k == 0 ? 200 : MN(200, sqrt(max_acc / k));

	//	s = ut + 0.5 att = dy
	//	a = 2(dy - ut) / tt      ... but lateral u = 0
	double	estT = dist / spd0;

	double	lat_acc = 2 * dy / (estT * estT);
	if( lat_acc > max_acc )
		lat_acc = max_acc;
	double	lin_acc = sqrt(max_acc * max_acc - lat_acc * lat_acc);

	//
	// accelerate
	//

	// acceleration is limited by engine power... and this quadratic
	//	is an estimation (poor, but hopefully good enough for our purposes).
	static const Quadratic	accFromSpd(21.0/5400, -43.0/60, 30);
	double	eng_acc = accFromSpd.CalcY(spd0) * kFriction;
	if( eng_acc > lin_acc )
		eng_acc = lin_acc;

	maxSpd = sqrt(spd0 * spd0 + 2 * eng_acc * dist);
//	if( maxSpd > max_spd )
//		maxSpd = max_spd;

	//
	// brake
	//

	minSpd = sqrt(spd0 * spd0 - 2 * lin_acc * dist);
}

//===========================================================================

void	CarModel::CalcSimuSpeedRanges(
	double	spd0,
	double	dist,
	double	kFriction,
	double&	minSpd,
	double&	maxSpd,
	double&	maxDY ) const
{
	// simple speed calc for use in simulation for path optimisation... the
	//	overriding pre-requisite of which is speed of calculation.
	//
	// a = v*v/r
	// max_a = M * G * MU;
	// max_spd = sqrt(max_a r) = sqrt(M * G * MU / k)

	double	M  = MASS + FUEL;
	double	MU = kFriction * TYRE_MU;

	double	max_acc = G * MU;

	//
	// accelerate
	//

	// acceleration is limited by engine power... and this quadratic
	//	is an estimation (poor, but hopefully good enough for our purposes).
	static const Quadratic	accFromSpd(21.0/5400, -43.0/60, 30);
	double	eng_acc = accFromSpd.CalcY(spd0) * kFriction;
	if( eng_acc > max_acc )
		eng_acc = max_acc;

	maxSpd = sqrt(spd0 * spd0 + 2 * eng_acc * dist);

	//
	// brake
	//

	minSpd = sqrt(spd0 * spd0 - 2 * max_acc * dist);

	//
	// turn (turning is symmetrical)
	//

	// s = ut + 1/2 att    u = 0, as we're looking along vel vector.
	// t = dist / spd0;
	double	turnT = dist / spd0;
	maxDY = 0.5 * max_acc * turnT * turnT;
}

//===========================================================================
