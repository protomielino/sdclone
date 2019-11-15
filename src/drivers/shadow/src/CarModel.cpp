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

#include <math.h>

#include "CarModel.h"
#include "Quadratic.h"
#include "Utils.h"

// The "SHADOW" logger instance.
extern GfLogger* PLogSHADOW;
#define LogSHADOW (*PLogSHADOW)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CarModel::CarModel():
    FLAGS(F_SEPARATE_FRONT_REAR),
    AERO(0),
    EMPTYMASS(0),
    MASS(0),
    LENGTH(0),
    FUEL(0),
    DAMAGE(0),
    NEEDSINLONG(false),
    USEDACCEXIT(false),
    USECONFIG(false),
    USECONFIGWHEEL(false),
    SKILL(0),

    TYRE_MU(0),
    TYRE_MU_F(0),
    TYRE_MU_R(0),
    MU_SCALE(0.9),
    MIN_MU_SCALE(0),
    AVOID_MU_SCALE(0.9),
    GRIP_SCALE_F(1),
    GRIP_SCALE_R(1),
    WING_ANGLE_F(0),
    WING_ANGLE_R(0),
    KZ_SCALE(0.43f),
    OFFLINE_KZ_SCALE(0.43f),
    AVOID_KZ_SCALE(0.43f),
    KV_SCALE(1),

    BRAKESCALE(0),
    BRAKEFORCE(0),
    BRAKELIMIT(0.0),

    CA(0),
    CA_FW(0),
    CA_RW(0),
    CA_GE(0),
    CA_GE_F(0),
    CA_GE_R(0),

    CD_BODY(0),
    CD_WING(0),
    CD_CX(0),

    lftOH(0),
    rgtOH(0),

    WIDTH(2),
    BRAKE_FACTOR(1),
    CT_FACTOR(1),

    HASTYC(false),
    TYRECONDITIONFRONT(0),
    TYRECONDITIONREAR(0)
{
}

CarModel::~CarModel()
{
}

double	CarModel::CalcMaxSpeed(double k, double kz, double kv, double trackMu, double trackRollAngle , double trackPitchAngle) const
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

    if( FLAGS & F_OLD_AERO_1 )
    {
        double	MU_F = trackMu * TYRE_MU_F;
        double	MU_R = trackMu * TYRE_MU_R;

        muf = MU_F * MU_SCALE;
        mur = MU_R * MU_SCALE;
        mua = (MU_F + MU_R) * 0.5;
    }
    else
    {
        double	MU = trackMu * TYRE_MU;

        mua   = MU * MU_SCALE;// * 0.975;
    }

    mua *= MN(GRIP_SCALE_F, GRIP_SCALE_R);

    double	cs = cos(trackRollAngle) * cos(trackRollAngle);
    double	sn = sin(trackRollAngle);

    double	absK = MX(0.001, fabs(k));
    double	sgnK = SGN(k);

    double	num, den;

    if( FLAGS & F_OLD_AERO_1 )
    {
        num = M * (cs * G * mua + sn * G * sgnK);
//		den = M * (absK - 0.1 * kz) -
        if( FLAGS & F_USE_KV )
            den = M * (absK - KV_SCALE * kv) -
                        (CA_FW * muf + CA_RW * mur + CA_GE * mua);
        else
            den = M * (absK - KZ_SCALE * kv) -
                        (CA_FW * muf + CA_RW * mur + CA_GE * mua);
    }
    else
    {
        num = M * (cs * G * mua + sn * G * sgnK);

        if( FLAGS & F_USE_KV )
            den = M * (absK - KV_SCALE * kv) - CA * mua;
        else
            den = M * (absK - KZ_SCALE * kv) - CA * mua;
    }

    if( den < 0.00001 )
        den = 0.00001;

    double	spd = sqrt(num / den);

    if( spd > 200 )
        spd = 200;

    LogSHADOW.debug( "Max Speed in CarModel = %.3f\n", spd);

    return spd;
}

/*double	CarModel::CalcMaxSpeed(double k, double k1, double kz, double kFriction, double RollAngle, double TiltAngle ) const
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
    double Mu;

    double Cos = cos(RollAngle)*cos(TiltAngle);
    double SinLat = sin(RollAngle);
    double SinLong = sin(TiltAngle);
    double Sin = SinLat;

    if (NEEDSINLONG)
    {
      if (SinLat < SinLong)
            Sin = SinLong;
    }

    double AbsCrv0 = MAX(0.001, fabs(k));
    double AbsCrv1 = MAX(0.001, fabs(k1));
    double AbsCrv = AbsCrv0;
    double factor = 1.0;

    if (AbsCrv < 1/200.0)
      kz *= KZ_SCALE;

    if (AbsCrv > AbsCrv1)
    {
      if (USEDACCEXIT)
        factor = 1.015;

    }
    else
    {
      factor = 0.985;
    }

    double Den;

    double ScaleBump  = BUMP_FACTOR;

    if (k > 0)
      ScaleBump = BUMP_FACTORLEFT;
    else
      ScaleBump = BUMP_FACTORRIGHT;

    double MuF = kFriction * TYRE_MU_F; /* MU_SCALE;*/
/*    double MuR = kFriction * TYRE_MU_R; /* MU_SCALE;*/

/*    if (HASTYC)
    {
      double TcF = TYRECONDITIONFRONT;
      double TcR = TYRECONDITIONREAR;
      MuF = TcF * MuF;
      MuR = TcR * MuR;
      Mu = MIN(MuF, MuR); // SKILL;
      LogSHADOW.debug("TYRE MUF = %.f - TYRE MUR = %.f - MU = %.f\n", MuF, MuR, Mu);
    }
    else
    {
        Mu = MIN(MuF, MuR);
        LogSHADOW.debug("MU = %.f\n", Mu);
    }

    Den = (AbsCrv - ScaleBump * kz) - (CA_FW * MuF + CA_RW * MuR + CA_GE_F * MuF + CA_GE_R * MuR) / MASS;

    if (Den < 0.00001)
     Den = 0.00001;

    if (AbsCrv > 0.002)
    {
        if (Sin * SGN(k) < 0)
        {
            Sin *= 8.0;
            Sin = SGN(Sin) * MIN(0.05,fabs(Sin));
        }
    }

    double Speed = factor * sqrt((Cos * GRAVITY * Mu + Sin * GRAVITY * SGN(k) + kz) / Den);
    LogSHADOW.debug("CarModel CalcMaxSpeed = %.f\n", Speed);

    return Speed;
}*/

double	CarModel::CalcBreaking(double k0, double kz0, double k1, double kz1, double spd1, double dist, double kFriction, double RollAngle , double TiltAngle) const
{
    // when under braking we keep some grip in reserve.
    if (spd1 > 180/3.6)
      kFriction *= 0.90;
    else
      kFriction *= 0.95;

    double	cs  = cos(RollAngle);
    double  cs2 = cos(TiltAngle);
    double	sn  = sin(RollAngle);
    double  sn2 = sin(TiltAngle);

    double	K  = (0.3 * k0  + 0.9 * k1);
    double	Kz = (0.25 * kz0 + 0.75 * kz1);// * KZ_SCALE;
    if( Kz > 0 )
        Kz = 0;

    double	M  = MASS + FUEL;

    double	MU = kFriction * TYRE_MU;
    double	MU_F = MU;
    double	MU_R = MU;

    if( AERO == 1 )
    {
        MU_F = kFriction * TYRE_MU_F;
        MU_R = kFriction * TYRE_MU_R;
        MU   = (MU_F + MU_R) * 0.5;
        LogSHADOW.debug("CalcBreaking TYRE MUF = %.f - TYRE MUR = %.f - MU = %.f\n", MU_F, MU_R, MU);
    }

    if (HASTYC)
    {
      double TcF = TYRECONDITIONFRONT;
      double TcR = TYRECONDITIONREAR;
      MU_F = TcF * MU_F;
      MU_R = TcR * MU_R;
      MU = MIN(MU_F, MU_R); // SKILL;
      LogSHADOW.debug("CalcBreaking HASTYC TYRE MUF = %.f - TYRE MUR = %.f - MU = %.f\n", MU_F, MU_R, MU);
    }
    else
      MU = MIN(MU_F, MU_R); // oTmpCarParam->oSkill;

    double	CD = CD_BODY * (1.0 + DAMAGE / 10000.0) + CD_WING;

    double	Gdown = GRAVITY * cs * cs2;
    double	Glat  = fabs(sn * GRAVITY);
    double	Gtan  = - GRAVITY * sn2;

    double	v = spd1;
    double	u = v;

    for( int count = 0; count < 100; count++ )
    {
        double	avgV = (u + v) * 0.5;
        double	avgVV = avgV * avgV;

        double	Froad;

        if( AERO == 1 )
        {
            double	Fdown = M * Gdown + M * Kz * avgVV + CA_GE * avgVV;
            double	Ffrnt = CA_FW * avgVV;
            double	Frear = CA_RW * avgVV;

            Froad = Fdown * MU + Ffrnt * MU_F + Frear * MU_R; // maybe * 0.95
        }
        else
        {
            double	Fdown = M * Gdown + M * Kz * avgVV + CA * avgVV; // idem

            Froad = Fdown * MU;
        }
        double	Flat  = M * Glat;
        double	Ftan  = M * Gtan - CD * avgVV;

        double	Flatroad = fabs(M * avgVV * K - Flat);
        if( Flatroad > Froad )
            Flatroad = Froad;
        double	Ftanroad = -sqrt(Froad * Froad - Flatroad * Flatroad) + Ftan;

        double	acc = Ftanroad / M;

        acc = BRAKESCALE * Ftanroad / (MASS * ( 3 + SKILL) / 4);

        if (BRAKELIMIT > 0.0)
        {
            double Radius = 1.0 / fabs(Kz);
            double factor = MIN(1.0,MAX(0.39, (Radius - 190.0) / 100.0));
            acc = MAX(acc, BRAKELIMIT * factor);
        }

        double	inner = MX(0, v * v - 2 * acc * dist );
        double	oldU = u;
        u = sqrt(inner);

        if( fabs(u - oldU) < 0.001 )
            break;
    }

    double midspd = (u + spd1)/2;

    // Check brake
    double brakedecel = BRAKESCALE * BRAKEFORCE / MASS;
    double braketargetspd = sqrt(midspd * midspd + 2 * brakedecel * dist);
    double resulttargetspd = MIN(u, braketargetspd);

    LogSHADOW.debug("CalcBreaking resulttargetspd = %.f\n", MAX(resulttargetspd, spd1));

    return MAX(resulttargetspd, spd1);
}

double	CarModel::CalcAcceleration(double k0, double kz0, double k1, double kz1, double spd0, double dist, double kFriction, double RollAngle , double TiltAngle) const
{
    double	M  = MASS + FUEL;
    double	MU = kFriction * TYRE_MU;
    double	CD = CD_BODY * (1.0 + DAMAGE / 10000.0) + CD_WING;

    // when under braking we keep some grip in reserve.
    MU *= 0.95;

    if (HASTYC)
    {
        double TcF = TYRECONDITIONFRONT;
        double TcR = TYRECONDITIONREAR;
        double MU_F = TcF * TYRE_MU_F;
        double MU_R = TcR * TYRE_MU_R;
        MU = MIN(MU_F, MU_R); // SKILL;
        LogSHADOW.debug("CalcAcceleration TYRE MUF = %.f - TYRE MUR = %.f - MU = %.f\n", MU_F, MU_R, MU);
    }

    double	cs = cos(RollAngle);
    double	sn = sin(RollAngle);
    double  sn2 = sin(TiltAngle);

    double	K  = (0.25 * k0  + 0.75 * k1);
    double	Kz = (0.25 * kz0 + 0.75 * kz1);// * KZ_SCALE;
    if( Kz > 0 )
        Kz = 0;

    double	Gdown = cs * GRAVITY;
    double	Glat  = sn * GRAVITY;
    double	Gtan  = - GRAVITY * sn2;

    double	u = spd0;
    double	v = u;

    // 30m/ss @ 0m/s
    //  3m/ss @ 60m/s
    //	1m/ss @ 75m/s
    //	0m/ss @ 85m/s
    //Quadratic	accFromSpd(0.001852, -0.35, 17.7);		// approx. clkdtm
    Quadratic	accFromSpd(21.0/5400, -43.0/60, 30);	// approx. clkdtm
    double OldV = 0.0;
    // Power (kW) = Torque (Nm) x Speed (RPM) / 9.5488

    for( int count = 0; count < 100; count++ )
    {
        double	avgV = (u + v) * 0.5;
        double	vv = avgV * avgV;

        double	Fdown = M * Gdown + M * Kz * vv + CA * vv;
        double	Froad = Fdown * MU;
        double	Flat  = M * Glat;
        double	Ftan  = M * Gtan - CD * vv;

        double	Flatroad = fabs(M * vv * K - Flat);

        if( Flatroad > Froad )
            Flatroad = Froad;

        double	Ftanroad = sqrt(Froad * Froad - Flatroad * Flatroad) + Ftan;
        double	acc = Ftanroad / M;
        double	maxAcc = MIN(11.5, accFromSpd.CalcY(avgV));

        if( acc > maxAcc )
            acc = maxAcc;

        double	inner = MX(0, u * u + 2 * acc * dist );
        double	oldV = v;
        v = sqrt(inner);

        if( fabs(v - oldV) < 0.001 )
            break;

        OldV = v;
    }

    return v;
}

double CarModel::CalcMaxSpdK() const
{
    const double	MAX_SPD = 112;	// ~400 kph

    return GRAVITY * TYRE_MU / (MAX_SPD * MAX_SPD);
}

double CarModel::CalcMaxLateralF( double spd, double kFriction, double kz ) const
{
    double Fdown = (MASS + FUEL) * GRAVITY + (MASS * kz + CA) * spd * spd;

    return Fdown * kFriction * TYRE_MU;
}

double CarModel::CalcMaxSpeedCrv() const
{
  const double MAX_SPD = 112; // 400 km/h

  return GRAVITY * TYRE_MU / (MAX_SPD * MAX_SPD);
}

double CarModel::calcPredictedLoad(
    double speed,
    double weight_fraction,
    double downforce_constant,
    double k,
    double kz,
    double kv,
    double sin_roll,
    double cos_roll,
    double cos_pitch ) const
{
    double	load_g = (MASS + FUEL) * weight_fraction * G * cos_roll * cos_pitch;
    double	load_a = downforce_constant * speed * speed;
    double	load_v;

    if( FLAGS & F_USE_KV )
        load_v = (MASS + FUEL) * weight_fraction * kv * KV_SCALE * speed * speed;
    else
        load_v = (MASS + FUEL) * weight_fraction * cos_roll * kz * KZ_SCALE * speed * speed;

    return	load_g + load_a + /*load_h*/ + load_v;
}

void CarModel::CalcSimuSpeeds( double spd0, double dy, double dist, double kFriction, double& minSpd, double& maxSpd ) const
{
    // simple speed calc for use in simulation for path optimisation... the
    //	overriding pre-requisite of which is speed of calculation.

    double	MU = kFriction * TYRE_MU;

    double	max_acc = GRAVITY * MU;
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
    static const Quadratic	accFromSpd(0.001852, -0.35, 17.7);
    double	eng_acc = accFromSpd.CalcY(spd0) * kFriction;
    if( eng_acc > lin_acc )
        eng_acc = lin_acc;

    maxSpd = sqrt(spd0 * spd0 + 2 * eng_acc * dist);
    minSpd = sqrt(spd0 * spd0 - 2 * lin_acc * dist);
}

void CarModel::CalcSimuSpeedRanges( double spd0,	double dist, double	kFriction, double& minSpd, double& maxSpd, double& maxDY ) const
{
    // simple speed calc for use in simulation for path optimisation... the
    //	overriding pre-requisite of which is speed of calculation.

    double	MU = kFriction * TYRE_MU;
    double	max_acc = GRAVITY * MU;

    //
    // accelerate
    //

    // acceleration is limited by engine power... and this quadratic
    //	is an estimation (poor, but hopefully good enough for our purposes).
    static const Quadratic	accFromSpd(0.001852, -0.35, 17.7);
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

    double	turnT = dist / spd0;
    maxDY = 0.5 * max_acc * turnT * turnT;
}

//===========================================================================
void CarModel::config( const tCarElt* car )
{
    configWheels( car );
    configCar( car->_carHandle );
}

void CarModel::config( void* hCar )
{
    configWheels( hCar );
    configCar( hCar );
}

void CarModel::setupDefaultGearbox()
{
    GEAR_RATIOS.clear();
    GEAR_EFFS.clear();

    GEAR_RATIOS.push_back( 2.66 );
    GEAR_EFFS.push_back( 0.955 );
    GEAR_RATIOS.push_back( 1.78 );
    GEAR_EFFS.push_back( 0.957 );
    GEAR_RATIOS.push_back( 1.3 );
    GEAR_EFFS.push_back( 0.95 );
    GEAR_RATIOS.push_back( 1.0 );
    GEAR_EFFS.push_back( 0.983 );
    GEAR_RATIOS.push_back( 0.84 );
    GEAR_EFFS.push_back( 0.948 );
    GEAR_RATIOS.push_back( 0.74 );
    GEAR_EFFS.push_back( 0.94 );
}

void CarModel::setupDefaultEngine()
{
    ENGINE_REVS.clear();
    ENGINE_TORQUES.clear();

    ENGINE_REVS.push_back(     0 );
    ENGINE_REVS.push_back(  1000 * 2 * PI / 60 );
    ENGINE_REVS.push_back(  2000 * 2 * PI / 60 );
    ENGINE_REVS.push_back(  3000 * 2 * PI / 60 );
    ENGINE_REVS.push_back(  4000 * 2 * PI / 60 );
    ENGINE_REVS.push_back(  5000 * 2 * PI / 60 );
    ENGINE_REVS.push_back(  6000 * 2 * PI / 60 );
    ENGINE_REVS.push_back(  7000 * 2 * PI / 60 );
    ENGINE_REVS.push_back(  8000 * 2 * PI / 60 );
    ENGINE_REVS.push_back(  9000 * 2 * PI / 60 );
    ENGINE_REVS.push_back( 10000 * 2 * PI / 60 );

    ENGINE_TORQUES.push_back( 97 );
    ENGINE_TORQUES.push_back( 222 );
    ENGINE_TORQUES.push_back( 325 );
    ENGINE_TORQUES.push_back( 470 );
    ENGINE_TORQUES.push_back( 560 );
    ENGINE_TORQUES.push_back( 555 );
    ENGINE_TORQUES.push_back( 545 );
    ENGINE_TORQUES.push_back( 511 );
    ENGINE_TORQUES.push_back( 471 );
    ENGINE_TORQUES.push_back( 410 );
    ENGINE_TORQUES.push_back( 320 );
}

//===========================================================================
const WheelModel& CarModel::wheel( int wl ) const
{
    return _wheel[wl];
}

//===========================================================================
void	CarModel::configWheels( const tCarElt* car )
{
    for( int w = 0; w < 4; w++ )
        _wheel[w].config( car );
}

//===========================================================================
void	CarModel::configWheels( void* hCar )
{
    for( int w = 0; w < 4; w++ )
        _wheel[w].config( hCar );
}

//===========================================================================
void	CarModel::updateWheels( const tCarElt* car, const tSituation* s )
{
    for( int w = 0; w < 4; w++ )
        _wheel[w].update( car, s, *this );
}

