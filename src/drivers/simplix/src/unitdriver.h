//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
// unitdriver.h
//--------------------------------------------------------------------------*
// TORCS: "The Open Racing Car Simulator"
// A robot for TORCS-NG-Version 1.4.0
//--------------------------------------------------------------------------*
// Class for driving and driver/robot
// Zentrale Klasse für das Fahren bzw. den Fahrer/Roboter
//
// File         : unitdriver.h
// Created      : 2007.11.25
// Last changed : 2009.01.11
// Copyright    : © 2007-2009 Wolf-Dieter Beelitz
// eMail        : wdb@wdbee.de
// Version      : 2.00.000
//--------------------------------------------------------------------------*
// Teile diese Unit basieren auf diversen Header-Dateien von TORCS
//
//    Copyright: (C) 2000 by Eric Espie
//    eMail    : torcs@free.fr
//
// dem erweiterten Robot-Tutorial bt
//
//    Copyright: (C) 2002-2004 Bernhard Wymann
//    eMail    : berniw@bluewin.ch
//
// dem Roboter delphin
//
//    Copyright: (C) 2006-2007 Wolf-Dieter Beelitz
//    eMail    : wdb@wdbee.de
//
// dem Roboter wdbee_2007
//
//    Copyright: (C) 2006-2007 Wolf-Dieter Beelitz
//    eMail    : wdb@wdbee.de
//
// und dem Roboter mouse_2006
//
//    Copyright: (C) 2006-2007 Tim Foden.
//
//--------------------------------------------------------------------------*
// Das Programm wurde unter Windows XP entwickelt und getestet.
// Fehler sind nicht bekannt, dennoch gilt:
// Wer die Dateien verwendet erkennt an, dass für Fehler, Schäden,
// Folgefehler oder Folgeschäden keine Haftung übernommen wird.
//
// Im übrigen gilt für die Nutzung und/oder Weitergabe die
// GNU GPL (General Public License)
// Version 2 oder nach eigener Wahl eine spätere Version.
//--------------------------------------------------------------------------*
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//--------------------------------------------------------------------------*
#ifndef _UNITDRIVER_H_
#define _UNITDRIVER_H_

#include <track.h>
#include <car.h>
#include <robot.h>

#include "unitglobal.h"
#include "unitcommon.h"
#include "unitcommondata.h"

#include "unitcharacteristic.h"
#include "unitcollision.h"
#include "unitclothoid.h"
#include "unitpit.h"
#include "unitstrategy.h"
#include "unittrack.h"
#include "unitopponent.h"
#include "unitparam.h"                           // TParam
#include "unitpidctrl.h"
#include "unitsysfoo.h"
#include "unitteammanager.h"

//==========================================================================*
// Deklaration der Klasse TDriver
//--------------------------------------------------------------------------*
class TDriver  
{
  public:
	TDriver(int Index);                          // Constructor
	~TDriver();                                  // Destructor

	// TORCS-Interface:
	void InitTrack                               // Initialize Track
	  (PTrack track, PCarHandle CarHandle, 
	  PCarSettings *CarParmHandle, 
	  PSituation Situation);
	void NewRace                                 // Start new Race 
	  (PtCarElt Car, PSituation Situation);
	void Drive();                                // Drive while racing
	int	PitCmd();                                // Handle pitstop
	void EndRace();                              // Stop race
	void Shutdown();                             // Cleanup

    void Update                                  // Update data
	  (PtCarElt Car, PSituation Situation); 
    void InitAdaptiveShiftLevels();              // Calculate shifting levels
    float BetterSteering();                      // Steering while unstucking
	void Clutching();                            // Clutch controller
	void StartAutomatic();                       // Clutch controller
	void DetectFlight();                         // Detect flight
    void FindRacinglines();                      // Find racing lines   
	void FlightControl();                        // Prepare landing
	double GearRatio();                          // Get gear ratio
    void GearTronic();                           // GearTronic
    void InitCarModells();                       // Initialize Car modells
	void InitCa();                               // Initialize Ca
	void InitCw();                               // Initialize Cw
    void InitDriveTrain();                       // Initialize drive train
	void InitTireMu();                           // Initailize tire mu
    void InitWheelRadius();                      // Calculate mean wheel radius
    bool IsStuck();                              // Stehen wir vor einem Hindernis
	double NextGearRatio();                      // Get next gear ratio
    bool TargetReached                           // Target reached
      (double Target, double AvoidTarget); 
	int PitSide();                               // Side of pitlane
	double PrevGearRatio();                      // Get prev gear ratio
	void Propagation();                          // Propagation
	double Steering();                           // Steering
	void TeamInfo();                             // Get team infos
	void Turning();                              // Turn if needed
    void Unstuck();                              // Rangieren
    void YawControl();                           // Controll yawing

    inline PTrack Track();                       // Get Pointer to TORCS track data
    inline PtCarElt Car();                       // Get Pointer to TORCS car data

	void GetLanePoint                            // Interpolate Lanepoint
      (int Path, double Pos, TLanePoint& LanePoint);
	void GetPosInfo                              // Get Info to position
	  (double pos, TLanePoint& pi, double u, double v);
	void GetPosInfo                              // Get Info to position
	  (double pos, TLanePoint& pi);
	double CalcPathTarget                        // Get target
	  (double pos, double offs);
	TVec2d CalcPathTarget2                       // Get target
	  (double pos, double offs);
	void GetPathToLeftAndRight                   // Get width to sides
	  (const PCarElt Car, double& toL, double& toR);
    void OwnCarOppIndex();                       // Get own index

	double SteerAngle                            // Get steer angle
	  (TLanePoint& AheadLanePoint);

	void BrakingForceRegulator();                // Regulate braking
    void BrakingForceRegulatorAvoid();           // R. b. while avoiding
    void BrakingForceRegulatorTraffic();         // R. b. in trafic

	void SetBotName                              // Set name of bot
	  (void* RobotSettings, char* Value);
	inline void	SetCommonData                    // Set pointer to common data
	  (TCommonData* CommonData);
#ifdef _USE_RTTEAMMANAGER_
	inline tTeam* GetTeam();
#else
	inline TTeamManager::TTeam* GetTeam();
#endif
    inline char* GetBotName();
    inline float CurrSpeed();

private:
	void AvoidOtherCars                          // Avoiding
	  (double K, bool& IsClose, bool& IsLapper);
    bool EcoShift();                             // Reduce fuel consumption
    void EvaluateCollisionFlags(                 // Check flags
      int I, 
      TCollision::TCollInfo& Coll,
      double Crv,
      double& MinCatchTime,
      double& MinCatchAccTime,
      double& MinVCatTime, 
	  bool& IsLapper);

	double FilterABS(double Brake);              // ABS filter
    double FilterBrake(double Brake);            // 
    double FilterSkillBrake(double Brake);

	double FilterDrifting(double Accel);         // Drifting
	double FilterLetPass(double Accel);          // Reduce accel
	double FilterTCL(double Accel);              // Tracktion control
    double FilterTrack(double Accel);            // Keep on track

    double FilterStart(double Speed);            // Filter Start

    void NextCurvature                           // Get next crv
	  (TCollision::TCollInfo& Coll, PtCarElt Car);
    void Runaround                               // run around obstacles
	  (double Scale, double Target, bool DoAvoid);
    double UnstuckSteerAngle                     // steer while unstucking
      (TLanePoint& PointInfo, TLanePoint& AheadPointInfo);

    void InterpolatePointInfo                    // Interpolation
	  (TLanePoint& P0, const TLanePoint& P1, double Q);

    void SetRandomSeed(unsigned int Seed);
    unsigned int getRandom();
	double CalcSkill(double TargetSpeed);


private:
	enum // drive types
	{
		cDT_RWD, cDT_FWD, cDT_4WD,
	};

	TCommonData* oCommonData;                    // Pointer to common data
	TTrackDescription oTrackDesc;                // Track description
	TClothoidLane oRacingLine[gNBR_RL];          // Racinglines

	TCarParam* oCarParams[3];                    // Array of pointers to parameter sets

	double oFlyHeight;                           // fly height
	double oScaleSteer;                          // scale steering
	double oStayTogether;			             // Dist in m.
	double oAvoidScale;			                 // scale avoiding 
	double oAvoidWidth;			                 // In m.
	bool oGoToPit;                               // Enter pit flag
#ifdef _USE_RTTEAMMANAGER_
	tTeam* oTeam;
#else
	TTeamManager::TTeam* oTeam;                  // Team
#endif

	int	oDriveTrainType;                         // Drive train type
	TPidController oPIDCLine;      	             // Controller for line error.
	int	oFlying;				                 // Flag prepare landing 
	int oNbrCars;                                // Nbr of cars in race
	int	oOwnOppIdx;                              // Index of own car in list of opponents
	TOpponent oOpponents[cMAX_OPP];		         // Infos about other cars.

	double oAvoidRange;				             // Where we are T->LR (0..1).
	double oAvoidRangeDelta;                     // Delta to change range
	double oAvoidOffset;				         // Where we are L->R (-1..1).
	double oAvoidOffsetDelta;                    // Delta to change offset

	TCharacteristic oMaxAccel;                   // Cars accelleration characteristic

	double oBrakeCoeff[NBR_BRAKECOEFF];          // Brake coefficients 
	int	oLastBrakeCoefIndex;                     // Index of last brake coef.
	double oLastBrake;                           // Last brake command
	double oLastTargetSpeed;                     // Last target speed

    // State values to update commands 
	double oAccel;                               // Accelleration
	double oBrake;                               // Braking
	double oClutch;                              // Clutching
	int oGear;                                   // Gear
	double oSteer;                               // Steering

	double oAbsDelta;
	double oAbsScale;
    bool oAlone;                                 // No opponent near
	double oAngle;                               // Actual Angle
    double oAngleSpeed;                          // Angle of speed
	char* oBotName;                              // Name of driver
	const char* oTeamName;                       // Name of team
	int oRaceNumber;                             // Race number
	double oBrakeDiffInitial;                    // Initial difference to brake
	double oBrakeForceMax;                       // Maximum braking force
	double oBrakeScale;                          // Brake force scaling
	double oInitialBrakeCoeff;
	PtCarElt oCar;                               // TORCS data for own car
    float oSteerAngle;                           // Angle to steer
    const char* oCarType;                        // Type name of own car
	double oClutchMax;
	double oClutchDelta;
	double oClutchRange;
	double oClutchRelease;
	double oCurrSpeed;                           // Currend speed
	double oGearEff[MAX_GEARS];                  // Efficiency of gears
	int oIndex;                                  // index of own driver
	int oLastGear;                               // Last gear
    bool oLetPass;                               // Let opoonent pass
	double oLookAhead;                           // Look ahead base value
	double oLookAheadFactor;                     // Look ahead factor
	double oLookScale;                           // Actual scale
	double oLookBase;                            // Actual base
	double oOmegaBase;                           // Actual base
	double oOmegaScale;                          // Actual scale
	double oOmegaAheadFactor;                    // Omega ahead factor
	double oOmegaAhead;                          // Omega ahead base value
    double oDistFromStart;                       // Position along Track
    double oShift[MAX_GEARS];                    // Shift levels
    double oShiftMargin;                         // Shift back margin
    int oShiftCounter;                           // Shift timer
    PSituation oSituation;                       // TORCS data fpr situation
	double oStartDistance;                       // max Dist. raced while starting
	double oStartRPM;
    int oStuckCounter;                           // Tick counter
    PSysFoo oSysFooStuckX;                       // Positionsüberwachung in X
    PSysFoo oSysFooStuckY;                       // und Y
    int oTestPitStop;                            // Test pit stop
	float oTrackAngle;                           // Direction of track
    double oTargetSpeed;                         // Target speed for speed controller
	double oTclRange;                            // TCL range
	double oTclSlip;                             // Max TCL slip
	bool oTORCS_NG;
	char* oTrackName;                            // Name of track to drive on
	char* oTrackLoad;                            // Name of track to drive on
	char* oTrackLoadQualify;                     // Name of track to drive on
	char* oTrackLoadLeft;                        // Name of track to drive on
	char* oTrackLoadRight;                       // Name of track to drive on
	char* oPitLoad[3];                           // Name of track to drive on
	PTrack oTrack;                               // TORCS data fpr track
	double oTolerance;                           // Tolerable offset difference
    TLanePoint oLanePoint;                       // Information to Point
	bool oUnstucking;                            // Surmounting himdrance 
	double oWheelRadius;                         // Mean wheel radius
    double oDeltaOffset;                         // Delta to planned
    double oDriftAngle;                          // Drifting angle
	int oLetPassSide;                            // Go to side to let pass
	double oOldTarget;
    bool oReduced;
    double oFuelNeeded;
	double oRepairNeeded;

	int NBRRL;
	int oRL_FREE;
	int oRL_LEFT;
	int oRL_RIGHT;

	PCarHandle oCarHandle;                       // Handle of car parameter file 
    PSimpleStrategy oStrategy;                   // Pit strategy

    bool oDoAvoid;                               // Do avoid

    bool oSkilling;                              // Skilling on/off
	double oSkill;                               // Skilling
	double oSkillDriver;                         // Individual skilling level
	double oSkillGlobal;                         // Global skilling level
	double oSkillScale;                          // Track skilling level
	double oSkillOffset;                         // Hymie skilling level
    double oDriverAggression;
	double oSkillAdjustTimer;                    // Timer
	double oSkillAdjustLimit;                    // Limit
    double oBrakeAdjustTarget;                   //
    double oBrakeAdjustPerc;                     // 
    double oDecelAdjustTarget;                   //
    double oDecelAdjustPerc;                     // 
    unsigned int oRandomSeed;                    // seed of generator
	
  public:
	static double LengthMargin;                  // Length margin
	static bool Qualification;                   // Flag qualifying
	bool oStanding;                              // Fahrzeug steht#
	TParam Param;                                // Parameters
	double oFuelPer100km;                        //
	double oMaxFuel;                             // tank capacity
	double oBestLapTime;
	double oBestFuelPer100km;                    //
	double oSpeedScale;                          //
    bool oTreatTeamMateAsLapper;
	bool oTeamEnabled;
	int oCarsPerPit;                             // Pit sharing
	float oBumpMode;                             //

	static int NBBOTS;                           // Nbr of cars
    static double CurrSimTime;                   // Current simulation time
	static char* MyBotName;                      // Name of this bot 
	static char* ROBOT_DIR;                      // Sub path to dll
	static char* SECT_PRIV;                      // Private section
	static char* DEFAULTCARTYPE;                 // Default car type

	static bool AdvancedParameters;
    static bool UseOldSkilling;
	static bool UseBrakeLimit;
	static float BrakeLimit;
	static float BrakeLimitScale;
	static float BrakeLimitBase;
	static float SpeedLimitScale;
	static float SpeedLimitBase;
	
};
//==========================================================================*

//==========================================================================*
// Get name of robot
//--------------------------------------------------------------------------*
char* TDriver::GetBotName()
  {return oBotName;};
//==========================================================================*

//==========================================================================*
// Set pointer to common data
//--------------------------------------------------------------------------*
void TDriver::SetCommonData
  (TCommonData* CommonData){oCommonData = CommonData;};
//==========================================================================*

//==========================================================================*
// Get Team
//--------------------------------------------------------------------------*
#ifdef _USE_RTTEAMMANAGER_
tTeam* TDriver::GetTeam()
#else
TTeamManager::TTeam* TDriver::GetTeam()
#endif
{
  return oTeam;
}
//==========================================================================*

//==========================================================================*
// Get Pointer to TORCS data of track
//--------------------------------------------------------------------------*
PTrack TDriver::Track()
  {return oTrack;};
//==========================================================================*

//==========================================================================*
// Get Pointer to TORCS data of car
//--------------------------------------------------------------------------*
PtCarElt TDriver::Car()
  {return oCar;};
//==========================================================================*

//==========================================================================*
// Get Pointer to TORCS data of car
//--------------------------------------------------------------------------*
float TDriver::CurrSpeed()
  {return oCurrSpeed;};
//==========================================================================*
#endif // _UNITDRIVER_H_
//--------------------------------------------------------------------------*
// end of file unitdriver.h
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*

