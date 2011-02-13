//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
// unitfixcarparam.cpp
//--------------------------------------------------------------------------*
// TORCS: "The Open Racing Car Simulator"
// A robot for Speed Dreams Version 1.4.0
//--------------------------------------------------------------------------*
// Constant parameters of the car and calculations with it
// Unver�nderliche Parameter des Fahrzeugs und Nebenrechnungen
//
// File         : unitfixcarparam.cpp
// Created      : 2007.11.25
// Last changed : 2011.02.12
// Copyright    : � 2007-2010 Wolf-Dieter Beelitz
// eMail        : wdb@wdbee.de
// Version      : 3.00.000
//--------------------------------------------------------------------------*
// Ein erweiterter TORCS-Roboters
//--------------------------------------------------------------------------*
// Diese Unit basiert auf dem Roboter mouse_2006
//
//    Copyright: (C) 2006-2007 Tim Foden
//
//--------------------------------------------------------------------------*
// This program was developed and tested on windows XP
// There are no known Bugs, but:
// Who uses the files accepts, that no responsibility is adopted
// for bugs, dammages, aftereffects or consequential losses.
//
// Das Programm wurde unter Windows XP entwickelt und getestet.
// Fehler sind nicht bekannt, dennoch gilt:
// Wer die Dateien verwendet erkennt an, dass f�r Fehler, Sch�den,
// Folgefehler oder Folgesch�den keine Haftung �bernommen wird.
//--------------------------------------------------------------------------*
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// Im �brigen gilt f�r die Nutzung und/oder Weitergabe die
// GNU GPL (General Public License)
// Version 2 oder nach eigener Wahl eine sp�tere Version.
//--------------------------------------------------------------------------*
#include <math.h>

#include "unitglobal.h"
#include "unitcommon.h"

#include "unitdriver.h"
#include "unitparabel.h"
#include "unitfixcarparam.h"

//==========================================================================*
// Default constructor
//--------------------------------------------------------------------------*
TFixCarParam::TFixCarParam():
  oCar(NULL),
  oTmpCarParam(NULL),
  oBorderInner(0.5),
  oBorderOuter(0.5),
  oMaxBorderInner(1.00),
  oBorderScale(50.0),
  oCa(0),
  oCaFrontWing(0),
  oCaGroundEffect(0),
  oCaRearWing(0),
  oCdBody(0),
  oCdWing(0),
  oEmptyMass(0),
  oLength(4.5),
  oTyreMu(0),
  oTyreMuFront(0),
  oTyreMuRear(0),
  oWidth(2.0),
  oPitBrakeDist(150.0),
  oPitMinEntrySpeed(24.5f),
  oPitMinExitSpeed(24.5f),
  oStrategy(NULL)
{
}
//==========================================================================*

//==========================================================================*
// Destructor
//--------------------------------------------------------------------------*
TFixCarParam::~TFixCarParam()
{
}
//===========================================================================

//==========================================================================*
// Initialize
//--------------------------------------------------------------------------*
void TFixCarParam::Initialize(PDriver Driver, PtCarElt Car)
{
  oDriver = Driver;
  oCar = Car;
}
//==========================================================================*

//==========================================================================*
// Calculate accelleration
//--------------------------------------------------------------------------*
double TFixCarParam::CalcAcceleration(
  double Crv0,                                   // Curvature in xy at P0
  double Crvz0,                                  // Curvature in z at P0
  double Crv1,                                   // Curvature in xy at P1
  double Crvz1,                                  // Curvature in z at P0
  double Speed,                                  // Speed
  double Dist,                                   // Distance P0 P1
  double Friction,                               // Friction
  double TrackRollAngle) const                   // Track roll angle
{
  double MU = Friction * oTyreMu;
  double CD = oCdBody * 
	(1.0 + oTmpCarParam->oDamage / 10000.0) + oCdWing;

  double Crv = (0.25*Crv0  + 0.75*Crv1);
  double Crvz = (0.25*Crvz0 + 0.75*Crvz1); 
  if (Crvz > 0)
	Crvz = 0;

  double Gdown = G * cos(TrackRollAngle);
  double Glat = G * sin(TrackRollAngle);
  double Gtan = 0;	// TODO: track pitch angle.

  double U = Speed;
  double V = U;

  TParabel AccFromSpd(PAR_A, PAR_B, PAR_C);      // approx. carX-trb1
  double OldV = 0.0;
  for (int Count = 0; Count < 10; Count++)
  {
    double AvgV = (U + V) * 0.5;
    double AvgV2 = AvgV * AvgV;

    double Fdown = oTmpCarParam->oMass * Gdown 
	  + (oTmpCarParam->oMass * Crvz + oCa) * AvgV2;
    double Froad = Fdown * MU;
    double Flat = oTmpCarParam->oMass * Glat;
    double Ftan = oTmpCarParam->oMass * Gtan - CD * AvgV2;

    double Flatroad = fabs(oTmpCarParam->oMass * AvgV2 * Crv - Flat);
    if (Flatroad > Froad)
  	  Flatroad = Froad;

	double Ftanroad = sqrt(Froad * Froad - Flatroad * Flatroad) + Ftan;

	double Acc = Ftanroad / oTmpCarParam->oMass;
	double MaxAcc = MIN(11.5,AccFromSpd.CalcY(AvgV));
    if (Acc > MaxAcc)
	  Acc = MaxAcc;

	double Inner = MAX(0, U * U + 2 * Acc * Dist);
	V = sqrt(Inner);
	if (fabs(V - OldV) < 0.001)
	  break;
	OldV = V;
  }
  return V;
}
//==========================================================================*

//==========================================================================*
// Calculate decceleration
//--------------------------------------------------------------------------*
double TFixCarParam::CalcBraking
  (TCarParam& CarParam,                          // Lane specific parameters
  double Crv0,                                   // Curvature in xy at P0
  double Crvz0,                                  // Curvature in z at P0
  double Crv1,                                   // Curvature in xy at P1
  double Crvz1,                                  // Curvature in z at P0
  double Speed,                                  // Speed
  double Dist,                                   // Distance P0 P1
  double Friction,                               // Friction
  double TrackRollAngle,                         // Track roll angle
  double TrackTiltAngle) const                   // Track tilt angle
{
  if (Speed > 180/3.6)
    Friction *= 0.90;
  else
    Friction *= 0.95;

  double Crv = (0.3*Crv0 + 0.9*Crv1);
  double Crvz = -(0.25*Crvz0 + 0.75*Crvz1);
  //Friction *= AdjustFriction(Crv);
  Friction *= oDriver->CalcFriction(Crv);

  double Mu = Friction * oTyreMu;
  double Mu_F = Mu;
  double Mu_R = Mu;

  Mu_F = Friction * oTyreMuFront;
  Mu_R = Friction * oTyreMuRear;
  Mu = MIN(Mu_F,Mu_R);

  // From TORCS:
  double Cd = oCdBody * 
	(1.0 + oTmpCarParam->oDamage / 10000.0) + oCdWing;

  Crv *= oDriver->CalcCrv(fabs(Crv));

  if (Crvz > 0)
	Crvz = 0; 

  double Gdown = G * cos(TrackRollAngle) * cos(TrackTiltAngle);
  double Glat  = fabs(G * sin(TrackRollAngle));
  double Gtan  = G * sin(TrackTiltAngle);

  double V = Speed;
  double U = V;
  double Acc;

  for (int I = 0; I < 10; I++)
  {
	double AvgV = (U + V) * 0.5;
	double AvgV2 = AvgV * AvgV;

	double Froad;
	double Fdown = oTmpCarParam->oMass * Gdown + (oTmpCarParam->oMass * Crvz + oCaGroundEffect) * AvgV2;
	double Ffrnt = oCaFrontWing * AvgV2;
	double Frear = oCaRearWing * AvgV2;

	Froad = 0.95 * Fdown * Mu + Ffrnt * Mu_F + Frear * Mu_R;

	double Flat  = oTmpCarParam->oMass * Glat;
	double Ftan  = oTmpCarParam->oMass * Gtan - Cd * AvgV2;

	double Flatroad = MAX(0.0,oTmpCarParam->oMass * AvgV2 * fabs(Crv) - Flat);
	if (Flatroad > Froad)
	  Flatroad = Froad;

	double Ftanroad = -sqrt(Froad * Froad - Flatroad * Flatroad) + Ftan;

	Acc = CarParam.oScaleBrake * Ftanroad 
	  / (oTmpCarParam->oMass * ( 3 + oTmpCarParam->oSkill) / 4);
    
	if (TDriver::UseBrakeLimit)
	{
      double factor = 1.0 - MAX(0.0,TDriver::BrakeLimitScale * (fabs(Crv) - TDriver::BrakeLimitBase));
	  Acc = MAX(Acc,TDriver::BrakeLimit * factor);
	}

	double Inner = MAX(0, V * V - 2 * Acc * Dist);
	double OldU = U;
	U = sqrt(Inner);
	if (fabs(U - OldU) < 0.001)
	  break;
  }

  return U;
}
//==========================================================================*

//==========================================================================*
// Calculate decceleration in pitlane
//--------------------------------------------------------------------------*
double	TFixCarParam::CalcBrakingPit
  (TCarParam& CarParam,                          // Lane specific parameters
  double Crv0,                                   // Curvature in xy at P0
  double Crvz0,                                  // Curvature in z at P0
  double Crv1,                                   // Curvature in xy at P1
  double Crvz1,                                  // Curvature in z at P0
  double Speed,                                  // Speed
  double Dist,                                   // Distance P0 P1
  double Friction,                               // Friction
  double TrackRollAngle) const                   // Track roll angle
{
  if (Speed > 180/3.6)
    Friction *= 0.90;
  else
    Friction *= 0.95;

  double Crv = (0.3*Crv0 + 0.9*Crv1);
  double Crvz = (0.25*Crvz0 + 0.75*Crvz1);
  //Friction *= AdjustFriction(Crv);
  Friction *= oDriver->CalcFriction(Crv);

  double Mu = Friction * oTyreMu;
  double Mu_F = Mu;
  double Mu_R = Mu;

  Mu_F = Friction * oTyreMuFront;
  Mu_R = Friction * oTyreMuRear;
  Mu = MIN(Mu_F,Mu_R);

  // From TORCS:
  double Cd = oCdBody * 
	(1.0 + oTmpCarParam->oDamage / 10000.0) + oCdWing;

  Crv *= oDriver->CalcCrv(fabs(Crv));

  double Gdown = G * cos(TrackRollAngle);
  double Glat  = G * sin(TrackRollAngle);
  double Gtan  = 0;	

  double V = Speed;
  double U = V;

  for (int I = 0; I < 10; I++)
  {
	double AvgV = (U + V) * 0.5;
	double AvgV2 = AvgV * AvgV;

	double Froad;
	double Fdown = oTmpCarParam->oMass * Gdown + (oTmpCarParam->oMass * Crvz + oCaGroundEffect) * AvgV2;
	double Ffrnt = oCaFrontWing * AvgV2;
	double Frear = oCaRearWing * AvgV2;

	Froad = Fdown * Mu + Ffrnt * Mu_F + Frear * Mu_R;

	double Flat  = oTmpCarParam->oMass * Glat;
	double Ftan  = oTmpCarParam->oMass * Gtan - Cd * AvgV2;

	double Flatroad = fabs(oTmpCarParam->oMass * AvgV2 * Crv - Flat);
	if (Flatroad > Froad)
	  Flatroad = Froad;

	double Ftanroad = -sqrt(Froad * Froad - Flatroad * Flatroad) + Ftan;

	double Acc = CarParam.oScaleBrakePit * Ftanroad 
	  / oTmpCarParam->oMass;

	if (TDriver::UseGPBrakeLimit)
	  Acc = MAX(Acc,TDriver::BrakeLimit/2);

	double Inner = MAX(0, V * V - 2 * Acc * Dist);
	double OldU = U;
	U = sqrt(Inner);
	if (fabs(U - OldU) < 0.001)
	  break;
  }

  return U;
}
//==========================================================================*

//==========================================================================*
// Calculate maximum of speed
//--------------------------------------------------------------------------*
double TFixCarParam::CalcMaxSpeed
  (TCarParam& CarParam,                          // Lane specific parameters
  double Crv0,                                   // Curvature in xy at P
  double Crv1,                                   // Curvature in xy at P
  double CrvZ,                                   // Curvature in z at P
  double Friction,                               // Friction
  double TrackRollAngle) const                   // Track roll angle
{
  // Here we calculate the theoretical maximum speed at a point on the
  // path. This takes into account the curvature of the path (crv), the
  // grip on the road (mu), the downforce from the wings and the ground
  // effect (CA), the tilt of the road (left to right slope) (sin)
  // and the curvature of the road in z (crvz).
  //
  // There are still a few silly fudge factors to make the theory match
  // with the reality (the car goes too slowly otherwise, aarrgh!).

  double Mu;

  double Cos = cos(TrackRollAngle);
  double Sin = sin(TrackRollAngle);

  double AbsCrv0 = MAX(0.001, fabs(Crv0));
  double AbsCrv1 = MAX(0.001, fabs(Crv1));
  double AbsCrv = AbsCrv0;
  double factor = 1.0;
  if (AbsCrv > AbsCrv1)
  {
	if (oDriver->oUseAccelOut)
	  factor = 1.015; 
    AbsCrv *= oDriver->CalcCrv(AbsCrv);
  }
  else
  {
	factor = 0.985;
    AbsCrv *= oDriver->CalcCrv(AbsCrv);
  }
  //Friction *= AdjustFriction(AbsCrv);
  Friction *= oDriver->CalcFriction(AbsCrv);

  double Den;

  double ScaleBump;
  if (Crv0 > 0)
    ScaleBump = CarParam.oScaleBumpLeft;
  else
    ScaleBump = CarParam.oScaleBumpRight;

  double MuF = Friction * oTyreMuFront * CarParam.oScaleMu;
  double MuR = Friction * oTyreMuRear * CarParam.oScaleMu;
  Mu = MIN(MuF,MuR) / oTmpCarParam->oSkill;

  Den = (AbsCrv - ScaleBump * CrvZ)
    - (oCaFrontWing * MuF + oCaRearWing * MuR + oCaGroundEffect * Mu) / oTmpCarParam->oMass;

  if (Den < 0.00001)
   Den = 0.00001;

  double Speed = factor * sqrt((Cos * G * Mu + Sin * G * SGN(Crv0)) / Den);

//  if (fabs(AbsCrv) > 1/45.0)
//    Speed *= 0.89;                               // Filter hairpins

  if (fabs(AbsCrv) > 1/40.0)
    Speed *= 0.70;                               // Filter hairpins
  else if (fabs(AbsCrv) > 1/45.0)
    Speed *= 0.84;                               // Filter hairpins
  else if (Speed > 112)                          // (111,11 m/s = 400 km/h)
    Speed = 112;                                 

  if (Speed < 11.0)
	  Speed =  11.0;

  return Speed;
}
//==========================================================================*

//==========================================================================*
// Calculate maximum of lateral force
//--------------------------------------------------------------------------*
double TFixCarParam::CalcMaxLateralF
  (double Speed, double Friction, double Crvz) const
{
  double Fdown = oTmpCarParam->oMass * G 
	+ (oTmpCarParam->oMass * Crvz + oCa) * Speed * Speed;
  return Fdown * Friction * oTyreMu;
}
//==========================================================================*

//==========================================================================*
// Calculate curve at maximum of speed
//--------------------------------------------------------------------------*
double TFixCarParam::CalcMaxSpeedCrv() const
{
  const double MAX_SPD = 112; // 400 km/h
  return G * oTyreMu / (MAX_SPD * MAX_SPD);
}
//==========================================================================*

//--------------------------------------------------------------------------*
// end of file unitfixcarparam.cpp
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
