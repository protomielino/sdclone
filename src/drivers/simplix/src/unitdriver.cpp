//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
// unitdriver.cpp
//--------------------------------------------------------------------------*
// TORCS: "The Open Racing Car Simulator"
// A robot for TORCS-NG-Version 1.4.0
//--------------------------------------------------------------------------*
// Class for driving and driver/robot
// Zentrale Klasse f�r das Fahren bzw. den Fahrer/Roboter
//
// File         : unitdriver.cpp
// Created      : 2007.11.25
// Last changed : 2009.07.26
// Copyright    : � 2007-2009 Wolf-Dieter Beelitz
// eMail        : wdb@wdbee.de
// Version      : 2.00.000
//--------------------------------------------------------------------------*
// Teile dieser Unit basieren auf diversen Header-Dateien von TORCS
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
//#undef SPEED_DREAMS

#include <tmath/v2_t.h>
#include <tgf.h>
#include <robottools.h>
#include <timeanalysis.h>

#include "unitglobal.h"
#include "unitcommon.h"

#include "unitcollision.h"
#include "unitdriver.h"
#include "unitlinalg.h"
#include "unitparabel.h"
#include "unitpit.h"
#include "unitstrategy.h"
#include "unittrack.h"

//==========================================================================*
// Statics
//--------------------------------------------------------------------------*
int TDriver::NBBOTS = MAX_NBBOTS;                  // Nbr of drivers/robots
double TDriver::CurrSimTime = 0;                   // Current simulation time
char* TDriver::MyBotName = "simplix";              // Name of this bot
char* TDriver::ROBOT_DIR = "drivers/simplix";      // Sub path to dll
char* TDriver::SECT_PRIV = "simplix private";      // Private section
char* TDriver::DEFAULTCARTYPE  = "car1-trb1";      // Default car type
bool  TDriver::AdvancedParameters = false;         // Advanced parameters
bool  TDriver::UseOldSkilling = false;             // Use old skilling
bool  TDriver::UseSCSkilling = false;              // Use supercar skilling
bool  TDriver::UseBrakeLimit = false;              // Use brake limit
float TDriver::BrakeLimit = -6;                    // Brake limit
float TDriver::BrakeLimitBase = 0.025f;            // Brake limit base
float TDriver::BrakeLimitScale = 25;               // Brake limit scale
float TDriver::SpeedLimitBase = 0.025f;            // Speed limit base
float TDriver::SpeedLimitScale = 25;               // Speed limit scale

double TDriver::LengthMargin;                      // safety margin long.
bool TDriver::Qualification;                       // Global flag
static char *WheelSect[4] =                        // TORCS defined sections
{SECT_FRNTRGTWHEEL, SECT_FRNTLFTWHEEL, SECT_REARRGTWHEEL, SECT_REARLFTWHEEL};

static double (TDriver::*CalcCrv)(double Crv);

//static double cTimeSum[7] = {0,0,0,0,0,0,0};
//==========================================================================*

//==========================================================================*
// Buffers
//
// Q: qualifying
// F: Free
// L: Avoid to left
// R: Avoid to right
//--------------------------------------------------------------------------*
#define BUFLEN 256
static char PathFilenameBuffer[BUFLEN];          // for path and filename
static char TrackNameBuffer[BUFLEN];             // for track name
static char TrackLoadQualifyBuffer[BUFLEN];      // for track filename Q
static char TrackLoadBuffer[BUFLEN];             // for track filename F
static char TrackLoadLeftBuffer[BUFLEN];         // for track filename L
static char TrackLoadRightBuffer[BUFLEN];        // for track filename R
static char PitLoadBuffer[BUFLEN];               // for pit track filename F
static char PitLoadLeftBuffer[BUFLEN];           // for pit track filename L
static char PitLoadRightBuffer[BUFLEN];          // for pit track filename R
//==========================================================================*

//==========================================================================*
// Skilling: Randomness
//--------------------------------------------------------------------------*
#define RANDOM_SEED 0xfded
#define RANDOM_A    1664525
#define RANDOM_C    1013904223
//==========================================================================*

//==========================================================================*
// Skilling: Initialize Randomness
//--------------------------------------------------------------------------*
void TDriver::SetRandomSeed(unsigned int Seed)
{
 oRandomSeed = Seed ? Seed : RANDOM_SEED;
 return;
}
//==========================================================================*

//==========================================================================*
// Skilling: Get Randomness
//--------------------------------------------------------------------------*
unsigned int TDriver::getRandom()
{
 oRandomSeed = RANDOM_A * oRandomSeed + RANDOM_C;
 return (oRandomSeed >> 16);
}
//==========================================================================*

//==========================================================================*
// Interpolation for point Q between two points P0 and P1 (Result in P0)
//--------------------------------------------------------------------------*
void TDriver::InterpolatePointInfo
  (TLanePoint& P0, const TLanePoint& P1, double Q)
{
  double DeltaOAngle = P1.Angle - P0.Angle;

//  P0.Crv = TUtils::InterpCurvature(P0.Crv, P1.Crv, Q);
  P0.Crv = TUtils::InterpCurvature(P0.Crv, P1.Crv, (1 - Q));
  DOUBLE_NORM_PI_PI(DeltaOAngle);
  P0.Angle = P0.Angle + DeltaOAngle * (1 - Q);
  P0.Offset = Q * P0.Offset + (1 - Q) * P1.Offset;
  P0.Speed = Q * P0.Speed + (1 - Q) * P1.Speed;
}
//==========================================================================*

//==========================================================================*
// Constructor
//--------------------------------------------------------------------------*
TDriver::TDriver(int Index):
  oCommonData(NULL),
  // TrackDesc
  // Racingline
  // oCarParams
  oFlyHeight(0.06f),
  oScaleSteer(1.0),
  oStayTogether(10.0),
  oAvoidScale(8.0),
  oAvoidWidth(1.5),
  oGoToPit(false),

  oDriveTrainType(cDT_RWD),
  // oPIDCLine;
  oFlying(0),
  oNbrCars(0),
  oOwnOppIdx(0),
  // oOpponents
  oAvoidRange(0.9),
  oAvoidRangeDelta(0.0),
  oAvoidOffset(0.0),
  oAvoidOffsetDelta(0.0),
  oMaxAccel(0, 100, 101, 1),
  //oBrakeCoeff
  oLastBrakeCoefIndex(0),
  oLastTargetSpeed(0.0),
  oLastAheadDist(10.0),

  oAccel(0),
  oLastAccel(1.0),
  oBrake(0),
  oLastBrake(0.0),
  oClutch(0.5),
  oGear(0),
  oSteer(0),
  oLastSteer(0),

  oAbsDelta(1.1),
  oAbsScale(0.5),
  oAlone(true),
  oAngle(0),
  oAngleSpeed(0),
  oBotName(NULL),
  // oTeamName
  // oRaceNumber
  oBrakeDiffInitial(2.0),
  oBrakeForceMax(0.5),
  oBrakeScale(INITIAL_BRAKE_SCALE),
  oInitialBrakeCoeff(0.5),
  oCar(NULL),
  oSteerAngle(0.0f),
  oCarType(NULL),
  oClutchMax(0.5),
  oClutchDelta(0.009),
  oClutchRange(0.82),
  oClutchRelease(0.5),
  oCurrSpeed(0),
  // oGearEff
  oIndex(0),
  oLastGear(0),
  oLetPass(false),
  oLookAhead(5.0),
  oLookAheadFactor(0.05),
  oLookScale(0.2),
  oLookBase(5.0),
  oOmegaBase(5.0),
  oOmegaScale(0.2),
  oOmegaAheadFactor(0.1),
  oOmegaAhead(5.0),
  oDistFromStart(0.0),
  // oShift
  oShiftMargin(0),
  oShiftCounter(0),
  oSituation(NULL),
  oStartDistance(50.0),
  oStartRPM(100.0),
  oStuckCounter(0),
  oSysFooStuckX(NULL),
  oSysFooStuckY(NULL),
  oTestPitStop(0),
  oTrackAngle(0.0),
  oTargetSpeed(0.0),
  oTclRange(10.0),
  oTclSlip(1.6),
  oSPEED_DREAMS(true),
  oTrackName(NULL),
  oTrackLoad(NULL),
  oTrackLoadQualify(NULL),
  oTrackLoadLeft(NULL),
  oTrackLoadRight(NULL),
  //oPitLoad
  oTrack(NULL),
  oTolerance(2.0),
  //oLanePoint
  oUnstucking(false),
  oWheelRadius(0),
  oDeltaOffset(0.0),
  oDriftAngle(0.0),
  oLetPassSide(0),
  oOldTarget(0.0),
  oReduced(false),
  oFuelNeeded(0.0),
  oRepairNeeded(0.0),
  oSideReduction(1.0),
  oMinDistLong(FLT_MAX),

  NBRRL(0),
  oRL_FREE(0),
  oRL_LEFT(0),
  oRL_RIGHT(0),

  oCarHandle(NULL),
  oStrategy(NULL),
  oDoAvoid(false),
  oSkilling(true),
  oSkill(0.0),
  oSkillMax(24.0),
  oSkillDriver(0.0),
  oSkillGlobal(0.0),
  oSkillScale(1.0),
  oSkillOffset(0.0),
  oDriverAggression(0.0),
  oSkillAdjustTimer(0.0),
  oSkillAdjustLimit(0.0),
  oBrakeAdjustTarget(1.0),
  oBrakeAdjustPerc(1.0),
  oDecelAdjustTarget(1.0),
  oDecelAdjustPerc(1.0),
  oRandomSeed(0),
  //LengthMargin
  //Qualification
  oStanding(true),
  // Param
  oFuelPer100km(0.0),
  oMaxFuel(0.0),
  oMaxPressure(INITIAL_BRAKE_PRESSURE),
  oBestLapTime(0.0),
  oBestFuelPer100km(0.0),
  oSpeedScale(0.0),
  oTreatTeamMateAsLapper(false),
  oTeamEnabled(true),
  oPitSharing(false),
  oTeamIndex(0),
  oBumpMode(1),
  oTestLane(0),
  oUseFilterAccel(false),
  oUseAccelOut(false),
  oSideScaleMu(0.97f),
  oSideScaleBrake(0.97f),
  oSideBorderOuter(0.2f)
{
//  GfOut("#TDriver::TDriver() >>>\n");
  int I;
  oIndex = Index;                                // Save own index

  // Motion survey
  oSysFooStuckX = new TSysFoo(1,128);            // Ringbuffer for X
  oSysFooStuckY = new TSysFoo(1,128);            // and Y coordinates

  for (I = 0; I <= NBR_BRAKECOEFF; I++)          // Initialize braking
    oBrakeCoeff[I] = 0.5;

  NBRRL = gNBR_RL;                               // Setup number
  oRL_FREE = RL_FREE;                            // and index for
  oRL_LEFT = RL_LEFT;                            // normal races
  oRL_RIGHT = RL_RIGHT;

  TDriver::LengthMargin = LENGTH_MARGIN;         // Initialize safty margin

//  GfOut("#<<< TDriver::TDriver()\n");
}
//==========================================================================*

//==========================================================================*
// destructor
//--------------------------------------------------------------------------*
TDriver::~TDriver()
{
//  GfOut("#TDriver::~TDriver() >>>\n");
//  for (int I = 0; I < oNbrCars; I++)
//    delete oOpponents[I];
  delete [] oOpponents; 

  if (oStrategy != NULL)
    delete oStrategy;
  if (oSysFooStuckX != NULL)
    delete oSysFooStuckX;
  if (oSysFooStuckY != NULL)
    delete oSysFooStuckY;
//  GfOut("#<<< TDriver::~TDriver()\n");
}
//==========================================================================*

//==========================================================================*
// Set name of robot (and other appendant features)
//--------------------------------------------------------------------------*
void TDriver::SetBotName(void* RobotSettings, char* Value)
{
    // At this point TORCS gives us no information
    // about the name of the driver, the team and
    // our own car type!
    // Because we want it to set the name as defined
    // in the teams xml file and to load depending
    // setup files we have to find it out:

    char SectionBuffer[256];                     // Buffer
    snprintf(SectionBuffer,BUFLEN,               // Build name of
        "%s/%s/%d"                               // section from
	    ,ROB_SECT_ROBOTS,ROB_LIST_INDEX,oIndex); // Index of own driver
    char* Section = SectionBuffer;

	oCarType = GfParmGetStr                      // Get pointer to
      (RobotSettings                             // car type
      , Section                                  // defined in corresponding
      , ROB_ATTR_CAR, DEFAULTCARTYPE);           // section, default car type

	oBotName = Value;                            // Get pointer to drivers name

	oTeamName = GfParmGetStr                     // Get pointer to
      (RobotSettings                             // drivers team name
      , Section                                  // defined in corresponding
      , ROB_ATTR_TEAM, (char *) oCarType);       // section, car type as default

	oRaceNumber = (int) GfParmGetNum             // Get pointer to
      (RobotSettings                             // race number
      , Section, ROB_ATTR_RACENUM                // defined in corresponding
      , (char *) NULL, (tdble) oIndex + 1);      // section, index as default

    GfOut("#Bot name    : %s\n",oBotName);
    GfOut("#Team name   : %s\n",oTeamName);
    GfOut("#Car type    : %s\n",oCarType);
    GfOut("#Race number : %d\n",oRaceNumber);
};
//==========================================================================*

//==========================================================================*
// Called for every track change or new race.
//--------------------------------------------------------------------------*
void TDriver::InitTrack
  (PTrack Track, PCarHandle CarHandle,
  PCarSettings *CarSettings, PSituation Situation)
{
  GfOut("#\n\n\n#TDriver::InitTrack >>> \n\n\n");

  oTrack = Track;                                // save pointers
#ifdef SPEED_DREAMS
  if (TrackLength < 2000)
	RtTeamManagerLaps(3);
  else if (TrackLength < 3000)
	RtTeamManagerLaps(2);
#else
#endif

  oSituation = Situation;

  oSkillGlobal = oSkill = oDecelAdjustPerc = oDriverAggression = 0.0;

  // Initialize race type array
  char* RaceType[] =
    {"practice", "qualify", "race"};

  // Initialize the base param path
  char* BaseParamPath = TDriver::ROBOT_DIR;
  char* PathFilename = PathFilenameBuffer;

  // Global skilling from Andrew Sumner ...
  // Check if skilling is enabled
  int SkillEnabled = 0;
  snprintf(PathFilenameBuffer, BUFLEN,           // In default.xml
    "%s/default.xml", BaseParamPath);            // of the robot
  GfOut("#PathFilename: %s\n", PathFilenameBuffer); // itself
  void* SkillHandle = GfParmReadFile
	(PathFilename, GFPARM_RMODE_REREAD);
  if (SkillHandle)
  {
    SkillEnabled = (int) MAX(0,MIN(1,(int) GfParmGetNum(SkillHandle,
	  "skilling", "enable", (char *) NULL, 0.0)));
    GfOut("#SkillEnabled %d\n",SkillEnabled);
    oTeamEnabled = (int)
  	  GfParmGetNum(SkillHandle,"team","enable",0,(float)oTeamEnabled);
    GfOut("#oTeamEnabled %d\n",oTeamEnabled);
  }

  if (SkillEnabled > 0)                          // If skilling is enabled
  {                                              // Get Skill level
	oSkilling = true;                            // of TORCS-Installation
    GfOut("#Skilling: On\n");

	void* SkillHandle = NULL;

    snprintf(PathFilenameBuffer, BUFLEN,
	  "%sconfig/raceman/extra/skill.xml",GetLocalDir());
    GfOut("#skill.xml: %s\n", PathFilename);
    SkillHandle = GfParmReadFile
	  (PathFilename, GFPARM_RMODE_REREAD);
    if (SkillHandle)
    {
      oSkillGlobal = MAX(0.0,MIN(10.0,GfParmGetNum(SkillHandle,
		  "skill", "level", (char *) NULL, 10.0)));
	  GfOut("#LocalDir: SkillGlobal: %g\n", oSkillGlobal);
    }
	else
	{
      snprintf(PathFilenameBuffer, BUFLEN,
	    "%sconfig/raceman/extra/skill.xml",GetDataDir());
      GfOut("#skill.xml: %s\n", PathFilename);
      SkillHandle = GfParmReadFile
	    (PathFilename, GFPARM_RMODE_REREAD);
      if (SkillHandle)
      {
        oSkillGlobal = MAX(0.0,MIN(10.0,GfParmGetNum(SkillHandle,
		  "skill", "level", (char *) NULL, 10.0)));
		GfOut("#DataDir: SkillGlobal: %g\n", oSkillGlobal);
	  }
    }

    // Get individual skilling
    int SkillEnabled = 0;
    snprintf(PathFilenameBuffer,BUFLEN,"%s/%d/skill.xml",
      BaseParamPath,oIndex);
	GfOut("#PathFilename: %s\n", PathFilenameBuffer); // itself
    SkillHandle = GfParmReadFile
	  (PathFilename, GFPARM_RMODE_REREAD);
    if (SkillHandle)
    {
      oSkillDriver = GfParmGetNum(SkillHandle,"skill","level",0,0.0);
      oSkillDriver = MIN(1.0, MAX(0.0, oSkillDriver));
      GfOut("#oSkillDriver: %g\n", oSkillDriver);

      oDriverAggression = 
	    GfParmGetNum(SkillHandle, "skill", "aggression", (char *)NULL, 0.0);
      GfOut("#oDriverAggression: %g\n", oDriverAggression);
    }
  }
  else
  {
	oSkilling = false;
    GfOut("#Skilling: Off\n");
  }
  // ... Global skilling from Andrew Sumner

  // Get the name of the track
  strncpy(TrackNameBuffer,                       // Copy name of track file
    strrchr(oTrack->filename, '/') + 1,          // from path and filename
	sizeof(TrackNameBuffer));                    // regarding length of buffer
  *strrchr(TrackNameBuffer, '.') = '\0';         // Truncate at point
  oTrackName = TrackNameBuffer;                  // Set pointer to buffer

  // Read/merge car parms
  // First all params out of the common files
  oMaxFuel = GfParmGetNum(CarHandle              // Maximal m�glicher
    , SECT_CAR, PRM_TANK                         //   Tankinhalt
    , (char*) NULL, 100.0);
  GfOut("#oMaxFuel (TORCS)   = %.1f\n",oMaxFuel);

  oMaxPressure = GfParmGetNum(CarHandle          // Maximal m�glicher
    , "Brake System", MAXPRESSURE                //   Bremsdruck
    , (char*) NULL, oMaxPressure);
  GfOut("#oMaxPressure       = %.1f\n",oMaxPressure);

  oBrakeScale *= MAX(1.0,INITIAL_BRAKE_PRESSURE / oMaxPressure);
  GfOut("#oBrakeScale       = %.3f\n",oBrakeScale);
  oBrakeForceMax *= oBrakeScale/INITIAL_BRAKE_SCALE;
  GfOut("#oBrakeForceMax    = %.3f\n",oBrakeForceMax);

  // Next Params out of the own files
  PCarHandle Handle = NULL;                      // Start with an "empty file"
  char Buf[1024];                                // Multi purpose buffer

  // Default params for car type (e.g. .../ROBOT_DIR/sc-petrol/default.xml)
  snprintf(Buf,sizeof(Buf),"%s/%s/default.xml",
    BaseParamPath,oCarType);
  GfOut("#Default params for car type: %s\n", Buf);
  Handle = TUtils::MergeParamFile(Handle,Buf);

  snprintf(TrackLoadBuffer,sizeof(TrackLoadBuffer),"%s/tracks/%s.trk",
    BaseParamPath,oTrackName);
  oTrackLoad = TrackLoadBuffer;                  // Set pointer to buffer
  snprintf(TrackLoadQualifyBuffer,sizeof(TrackLoadQualifyBuffer),"%s/tracks/%s.trq",
    BaseParamPath,oTrackName);
  oTrackLoadQualify = TrackLoadQualifyBuffer;    // Set pointer to buffer
  snprintf(TrackLoadLeftBuffer,sizeof(TrackLoadLeftBuffer),"%s/tracks/%s.trl",
    BaseParamPath,oTrackName);
  oTrackLoadLeft = TrackLoadLeftBuffer;          // Set pointer to buffer
  snprintf(TrackLoadRightBuffer,sizeof(TrackLoadRightBuffer),"%s/tracks/%s.trr",
    BaseParamPath,oTrackName);
  oTrackLoadRight = TrackLoadRightBuffer;        // Set pointer to buffer
  snprintf(PitLoadBuffer,sizeof(PitLoadBuffer),"%s/tracks/%s.tpk",
    BaseParamPath,oTrackName);
  oPitLoad[0] = PitLoadBuffer;                   // Set pointer to buffer
  snprintf(PitLoadLeftBuffer,sizeof(PitLoadLeftBuffer),"%s/tracks/%s.tpl",
    BaseParamPath,oTrackName);
  oPitLoad[1] = PitLoadLeftBuffer;               // Set pointer to buffer
  snprintf(PitLoadRightBuffer,sizeof(PitLoadRightBuffer),"%s/tracks/%s.tpr",
    BaseParamPath,oTrackName);
  oPitLoad[2] = PitLoadRightBuffer;              // Set pointer to buffer

  // Override params for track (Pitting) 
  snprintf(Buf,sizeof(Buf),"%s/tracks/%s.xml",
    BaseParamPath,oTrackName);
  Handle = TUtils::MergeParamFile(Handle,Buf);
  double ScaleBrake = GfParmGetNum(Handle,TDriver::SECT_PRIV,PRV_SCALE__BRAKE,NULL,1.0f);
  double ScaleMu = GfParmGetNum(Handle,TDriver::SECT_PRIV,PRV_SCALE__MU,NULL,1.0f);

  // Override params for car type with params of track
  snprintf(Buf,sizeof(Buf),"%s/%s/%s.xml",
    BaseParamPath,oCarType,oTrackName);
  Handle = TUtils::MergeParamFile(Handle,Buf);

  // Override params for car type on track with params of specific race type
  snprintf(Buf,sizeof(Buf),"%s/%s/%s-%s.xml",
    BaseParamPath,oCarType,oTrackName,RaceType[oSituation->_raceType]);
  Handle = TUtils::MergeParamFile(Handle,Buf);

  // Override params for car type on track with driver on track
  snprintf(Buf,sizeof(Buf),"%s/%d/%s.xml",
    BaseParamPath,oIndex,oTrackName);
//  Handle = TUtils::MergeParamFile(Handle,Buf);

  // Override params for driver on track with params of specific race type
  snprintf(Buf,sizeof(Buf),"%s/%d/%s-%s.xml",
    BaseParamPath,oIndex,oTrackName,RaceType[oSituation->_raceType]);
  Handle = TUtils::MergeParamFile(Handle,Buf);

  // Setup the car param handle to be returned
  *CarSettings = Handle;

  // Get the private parameters now.
  TDriver::LengthMargin =
	GfParmGetNum(Handle,TDriver::SECT_PRIV,PRV_LENGTH_MARGIN,0,LENGTH_MARGIN);
  GfOut("#LengthMargin %.2f\n",TDriver::LengthMargin);

  // Check test flag:
  const char* ForceLane = GfParmGetStr(Handle,
	TDriver::SECT_PRIV,PRV_FORCE_LANE,"F");    
  
  if (strcmp(ForceLane,"L") == 0)
	oTestLane = -1;
  else if (strcmp(ForceLane,"R") == 0)
	oTestLane = 1;
  else 
	oTestLane = 0;

  int TestQualification =
	(int) GfParmGetNum(Handle,TDriver::SECT_PRIV,PRV_QUALIFICATION,0,0);
  if ((oSituation->_raceType == RM_TYPE_QUALIF)
	|| (TestQualification > 0))
  {
    if ((oSituation->_raceType == RM_TYPE_PRACTICE)
      || (oSituation->_raceType == RM_TYPE_QUALIF))
	{
	  Qualification = true;
	  GfOut("#Qualification = True\n");
	  NBRRL = 1;
	}
  }

  // Get car's length
  Param.Fix.oLength =
	GfParmGetNum(Handle,SECT_CAR,PRM_LEN,0,4.5);

  if (TDriver::UseBrakeLimit)
  {
    TDriver::BrakeLimit = 
	  GfParmGetNum(Handle,TDriver::SECT_PRIV,PRV_BRAKE_LIMIT,0,(float) TDriver::BrakeLimit);
    GfOut("#BrakeLimit %g\n",TDriver::BrakeLimit);
    TDriver::BrakeLimitBase = 
	  GfParmGetNum(Handle,TDriver::SECT_PRIV,PRV_BRAKE_LIMIT_BASE,0,(float) TDriver::BrakeLimitBase);
    GfOut("#BrakeLimitBase %g\n",TDriver::BrakeLimitBase);
    TDriver::BrakeLimitScale = 
	  GfParmGetNum(Handle,TDriver::SECT_PRIV,PRV_BRAKE_LIMIT_SCALE,0,(float) TDriver::BrakeLimitScale);
    GfOut("#BrakeLimitScale %g\n",TDriver::BrakeLimitScale);
    TDriver::SpeedLimitBase = 
	  GfParmGetNum(Handle,TDriver::SECT_PRIV,PRV_SPEED_LIMIT_BASE,0,(float) TDriver::SpeedLimitBase);
    GfOut("#SpeedLimitBase %g\n",TDriver::SpeedLimitBase);
    TDriver::SpeedLimitScale = 
	  GfParmGetNum(Handle,TDriver::SECT_PRIV,PRV_SPEED_LIMIT_SCALE,0,(float) TDriver::SpeedLimitScale);
    GfOut("#SpeedLimitScale %g\n",TDriver::SpeedLimitScale);
  }

  // Adjust pitting ...
  Param.Pit.oUseFirstPit = (int)
	GfParmGetNum(Handle,TDriver::SECT_PRIV,PRV_PIT_USE_FIRST,0,1);
  GfOut("#oUseFirstPit %d\n",Param.Pit.oUseFirstPit);

  Param.Pit.oUseSmoothPit = (int)
	GfParmGetNum(Handle,TDriver::SECT_PRIV,PRV_PIT_USE_SMOOTH,0,1);
  GfOut("#oUseSmoothPit %d\n",Param.Pit.oUseSmoothPit);

  Param.Pit.oLaneEntryOffset =
	GfParmGetNum(Handle,TDriver::SECT_PRIV,PRV_PITLANE_ENTRY,0,3.0f);
  GfOut("#oLaneEntryOffset %g\n",Param.Pit.oLaneEntryOffset);

  Param.Pit.oLaneExitOffset =
	GfParmGetNum(Handle,TDriver::SECT_PRIV,PRV_PITLANE_EXIT,0,5.0f);
  GfOut("#oLaneExitOffset %g\n",Param.Pit.oLaneExitOffset);

  Param.Pit.oEntryLong =
	GfParmGetNum(Handle,TDriver::SECT_PRIV,PRV_PIT_ENTRY_LONG,0,0);
  GfOut("#oEntryLong %g\n",Param.Pit.oEntryLong);

  Param.Pit.oExitLong =
	GfParmGetNum(Handle,TDriver::SECT_PRIV,PRV_PIT_EXIT_LONG,0,0);
  GfOut("#oExitLong %g\n",Param.Pit.oExitLong);

  Param.Pit.oExitLength =
	GfParmGetNum(Handle,TDriver::SECT_PRIV,PRV_PIT_EXIT_LEN,0,0);
  GfOut("#oExitLength %g\n",Param.Pit.oExitLength);

  Param.Pit.oLatOffset =
	GfParmGetNum(Handle,TDriver::SECT_PRIV,PRV_PIT_LAT_OFFS,0,0.0);
  GfOut("#Lateral Pit Offset %f\n",Param.Pit.oLatOffset);

  Param.Pit.oLongOffset =
	GfParmGetNum(Handle,TDriver::SECT_PRIV,PRV_PIT_LONG_OFFS,0,0.0);
  GfOut("#Longitudinal Pit  Offset %f\n",Param.Pit.oLongOffset);

  Param.oCarParam.oScaleBrakePit =
	GfParmGetNum(Handle,TDriver::SECT_PRIV,PRV_PIT_SCALE_BRAKE,0,
	(float) MIN(1.0,Param.oCarParam.oScaleBrake));
  GfOut("#ScaleBrakePit %g\n",Param.oCarParam.oScaleBrakePit);

  Param.Pit.oStoppingDist =
	GfParmGetNum(Handle,TDriver::SECT_PRIV,PRV_PIT_STOP_DIST,0,1.5);
  GfOut("#oStoppingDist %g\n",Param.Pit.oStoppingDist);

  Param.Fix.oPitBrakeDist =
	GfParmGetNum(Handle,TDriver::SECT_PRIV,PRV_PIT_BRAKE_DIST,0,150.0);
  GfOut("#oPitBrakeDist %g\n",Param.Fix.oPitBrakeDist);

  oTestPitStop = (int)
	GfParmGetNum(Handle,TDriver::SECT_PRIV,PRV_PIT_TEST_STOP,0,0);
  GfOut("#TestPitStop %d\n",oTestPitStop);
  // ... Adjust pitting

  // Adjust driving ...
  Param.oCarParam.oScaleBrake = ScaleBrake *
    GfParmGetNum(Handle,TDriver::SECT_PRIV,PRV_SCALE_BRAKE,NULL,0.85f);
  if(Qualification)
    Param.oCarParam.oScaleBrake = 
	  GfParmGetNum(Handle,TDriver::SECT_PRIV,PRV_SCALE_BRAKE_Q,NULL,Param.oCarParam.oScaleBrake);
  GfOut("#Scale Brake: %g\n",Param.oCarParam.oScaleBrake);

  oBumpMode =
	GfParmGetNum(Handle,TDriver::SECT_PRIV,PRV_BUMP_MODE,NULL,oBumpMode);
  Param.oCarParam.oScaleBump =
	GfParmGetNum(Handle,TDriver::SECT_PRIV,PRV_SCALE_BUMP,NULL,Param.oCarParam.oScaleBump);
  Param.oCarParam.oScaleBumpLeft =
    Param.oCarParam.oScaleBump;
  Param.oCarParam.oScaleBumpRight =
    Param.oCarParam.oScaleBump;
  GfOut("#Scale Bump: %g\n",Param.oCarParam.oScaleBump);

  Param.oCarParam.oScaleBumpOuter =
	GfParmGetNum(Handle,TDriver::SECT_PRIV,PRV_SCALE_BUMPOUTER,NULL,(float) Param.oCarParam.oScaleBump);
  GfOut("#Scale Bump Outer: %g\n",Param.oCarParam.oScaleBumpOuter);

  Param.oCarParam.oScaleMu = ScaleMu *
	GfParmGetNum(Handle,TDriver::SECT_PRIV,PRV_SCALE_MU,NULL,Param.oCarParam.oScaleMu);
  if(Qualification)
    Param.oCarParam.oScaleMu = 
  	  GfParmGetNum(Handle,TDriver::SECT_PRIV,PRV_SCALE_MU_Q,NULL,Param.oCarParam.oScaleMu);
  GfOut("#Scale Mu: %g\n",Param.oCarParam.oScaleMu);

  Param.oCarParam.oScaleMinMu =
	GfParmGetNum(Handle,TDriver::SECT_PRIV,PRV_SCALE_MIN_MU,NULL,Param.oCarParam.oScaleMinMu);
  GfOut("#Scale Min Mu %g\n",Param.oCarParam.oScaleMinMu);

  oSideScaleMu = 
	GfParmGetNum(Handle,TDriver::SECT_PRIV,PRV_SIDE_MU,NULL,oSideScaleMu);
  GfOut("#Side Scale Mu%g\n",oSideScaleMu);

  oSideScaleBrake = 
	GfParmGetNum(Handle,TDriver::SECT_PRIV,PRV_SIDE_BRAKE,NULL,oSideScaleBrake);
  GfOut("#Side Scale Brake%g\n",oSideScaleMu);

  oAvoidScale =
	GfParmGetNum(Handle,TDriver::SECT_PRIV,PRV_AVOID_SCALE,0,oAvoidScale);
  GfOut("#oAvoidScale %g\n",oAvoidScale);

  if (oTrack->width < 11)
    oAvoidWidth = 0.5;
  else if (oTrack->width < 12)
    oAvoidWidth = 0.75;
  else if (oTrack->width < 13)
    oAvoidWidth = 1.0;
  else if (oTrack->width < 14)
    oAvoidWidth = 1.25;

  oAvoidWidth =
	GfParmGetNum(Handle,TDriver::SECT_PRIV,PRV_AVOID_WIDTH,0,oAvoidWidth);
  GfOut("#oAvoidWidth %g\n",oAvoidWidth);

  oLookAhead = Param.Fix.oLength;
  oLookAhead =
	GfParmGetNum(Handle,TDriver::SECT_PRIV,PRV_LOOKAHEAD,0,oLookAhead);
  GfOut("#oLookAhead %g\n",oLookAhead);

  if (GfParmGetNum(Handle,TDriver::SECT_PRIV,PRV_ACCEL_OUT,0,1) != 0)
	  UseAccelOut();

  oOmegaAhead = Param.Fix.oLength;
  oInitialBrakeCoeff = oBrakeCoeff[0];

  if (TDriver::AdvancedParameters)
  {
    Param.Fix.oBorderInner =
	  GfParmGetNum(Handle,TDriver::SECT_PRIV,PRV_BORDER_INNER,0,Param.Fix.oBorderInner);
    GfOut("#Border Inner: %g\n",Param.Fix.oBorderInner);

    Param.Fix.oBorderOuter =
	  GfParmGetNum(Handle,TDriver::SECT_PRIV,PRV_BORDER_OUTER,0,Param.Fix.oBorderOuter);
    GfOut("#Border Outer: %g\n",Param.Fix.oBorderOuter);

    Param.Fix.oMaxBorderInner =
	  GfParmGetNum(Handle,TDriver::SECT_PRIV,PRV_MAX_BORDER_INNER,0,Param.Fix.oMaxBorderInner);
    GfOut("#Max Border Inner: %g\n",Param.Fix.oMaxBorderInner);

    Param.Fix.oBorderScale =
	  GfParmGetNum(Handle,TDriver::SECT_PRIV,PRV_BORDER_SCALE,0,Param.Fix.oBorderScale);
    GfOut("#Border Scale: %g\n",Param.Fix.oBorderScale);

    oFlyHeight =
      GfParmGetNum(Handle,TDriver::SECT_PRIV,PRV_FLY_HEIGHT,"m",oFlyHeight);
    GfOut("#FLY_HEIGHT %g\n",oFlyHeight);

    oLookAhead =
	  GfParmGetNum(Handle,TDriver::SECT_PRIV,PRV_LOOKAHEAD,0,(float) Param.Fix.oLength);
    GfOut("#LookAhead %g\n",oLookAhead);

	oOmegaAhead = Param.Fix.oLength;
	  GfParmGetNum(Handle,TDriver::SECT_PRIV,PRV_OMEGAAHEAD,0,(float) Param.Fix.oLength);
    GfOut("#OmegaAhead %g\n",oOmegaAhead);

    oOmegaAheadFactor =
	  GfParmGetNum(Handle,TDriver::SECT_PRIV,PRV_OMEGAAHEADFACTOR,0,(float) oOmegaAheadFactor);
    GfOut("#OmegaAheadFactor %g\n",oOmegaAheadFactor);

	oInitialBrakeCoeff = oBrakeCoeff[0];
	  GfParmGetNum(Handle,TDriver::SECT_PRIV,PRV_INIT_BRAKE,0,oBrakeCoeff[0]);
    GfOut("#oInitialBrakeCoeff %g\n",oInitialBrakeCoeff);
  }

  oLookAheadFactor =
    GfParmGetNum(Handle,TDriver::SECT_PRIV,PRV_LOOKAHEADFACTOR,0,(float)oLookAheadFactor);
  GfOut("#LookAheadFactor %g\n",oLookAheadFactor);

  oScaleSteer =
	GfParmGetNum(Handle,TDriver::SECT_PRIV,PRV_SCALE_STEER,0,oScaleSteer);
  GfOut("#oScaleSteer %g\n",oScaleSteer);

  oStayTogether =
	GfParmGetNum(Handle,TDriver::SECT_PRIV,PRV_STAY_TOGETHER,0,10);
  GfOut("#oStayTogether %g\n",oStayTogether);

  for (int I = 0; I <= NBR_BRAKECOEFF; I++)      // Initialize braking
    oBrakeCoeff[I] = oInitialBrakeCoeff;

  oTclRange =
	GfParmGetNum(Handle,TDriver::SECT_PRIV,PRV_TCL_RANGE,0,(float)oTclRange);
  GfOut("#oTclRange %g\n",oTclRange);

  oTclSlip =
	GfParmGetNum(Handle,TDriver::SECT_PRIV,PRV_TCL_SLIP,0,(float)oTclSlip);
  GfOut("#oTclSlip %g\n",oTclSlip);

  oAbsDelta =
	GfParmGetNum(Handle,TDriver::SECT_PRIV,PRV_ABS_DELTA,0,(float)oAbsDelta);
  GfOut("#oAbsDelta %g\n",oAbsDelta);

  oAbsScale =
	GfParmGetNum(Handle,TDriver::SECT_PRIV,PRV_ABS_SCALE,0,(float)oAbsScale);
  GfOut("#oAbsScale %g\n",oAbsScale);

  oClutchDelta =
	GfParmGetNum(Handle,TDriver::SECT_PRIV,PRV_CLUTCH_DELTA,0,(float)oClutchDelta);
  GfOut("#oClutchDelta %g\n",oClutchDelta);

  oClutchMax =
	GfParmGetNum(Handle,TDriver::SECT_PRIV,PRV_CLUTCH_MAX,0,(float)oClutchMax);
  GfOut("#oClutchMax %g\n",oClutchMax);

  oClutchRange =
	GfParmGetNum(Handle,TDriver::SECT_PRIV,PRV_CLUTCH_RANGE,0,(float)oClutchRange);
  GfOut("#oClutchRange %g\n",oClutchRange);

  oClutchRelease =
	GfParmGetNum(Handle,TDriver::SECT_PRIV,PRV_CLUTCH_RELEASE,0,(float)oClutchRelease);
  GfOut("#oClutchRelease %g\n",oClutchRelease);

  oTeamEnabled = (int)
    GfParmGetNum(Handle,TDriver::SECT_PRIV,PRV_TEAM_ENABLE,0,(float)oTeamEnabled);
  GfOut("#oTeamEnabled %d\n",oTeamEnabled);
  // ... Adjust driving

  // Adjust skilling ...
  if ((oSkill < 0) || (!oSkilling))
  {
    oSkilling = false;
	oSkill = 1.0;
	GfOut("#No skilling: Skill %g\n",oSkill);
    Param.Tmp.oSkill = 1.0;
  }
  else
  {
    oSkillOffset = MAX(0.0,MIN(10.0,GfParmGetNum(Handle,TDriver::SECT_PRIV,"offset skill", (char *) NULL, oSkillOffset)));
    GfOut("#SkillOffset: %g\n", oSkillOffset);
    oSkillScale = MAX(0.0,MIN(10.0,GfParmGetNum(Handle,TDriver::SECT_PRIV,"scale skill", (char *) NULL, oSkillScale)));
    GfOut("#SkillScale: %g\n", oSkillScale);

    oLookAhead = oLookAhead / (1+oSkillGlobal/24);
    oLookAheadFactor = oLookAheadFactor / (1+oSkillGlobal/24);

	if (UseOldSkilling)
	{
      // Scaling to match hymies
	  oSkill = 
		-0.0000455 * oSkillGlobal*oSkillGlobal*oSkillGlobal*oSkillGlobal*oSkillGlobal
		+0.0014 * oSkillGlobal*oSkillGlobal*oSkillGlobal*oSkillGlobal
		-0.0145 * oSkillGlobal*oSkillGlobal*oSkillGlobal
		+0.0512 * oSkillGlobal*oSkillGlobal
		+0.0978 * oSkillGlobal
		+ oSkillOffset + oSkillDriver;
	}
	else if (UseSCSkilling)
	{
      // Scaling to match usr_sc
	  oSkillScale = oSkillScale/50.0;
	  oSkillDriver = oSkillDriver / ((50.0 - oSkillGlobal)/40.0);
	  oSkill = oSkillScale * (oSkillGlobal + oSkillDriver * 2) * (1.0 + oSkillDriver) + oSkillOffset;
	}
	else
	{
      // Scaling 
	  oSkillScale = oSkillScale/50.0;
	  oSkillDriver = oSkillDriver / (5.0 * ((50.0 - oSkillGlobal)/40.0));
	  oSkill = oSkillScale * (oSkillGlobal + oSkillDriver * 2) * (1.0 + oSkillDriver) + oSkillOffset;
	  oSkillMax = oSkillScale * 24 + oSkillOffset;
	}

	if (Qualification)
	{
  	  oSkill *= 1.5;
	  oSkillMax *= 1.5;
	}

    Param.Tmp.oSkill = 1.0 + oSkill;
	GfOut("\n#>>>Skilling: Skill %g oSkillGlobal %g oSkillDriver %g oLookAhead %g oLookAheadFactor %g effSkill:%g\n\n",
		oSkill,oSkillGlobal,oSkillDriver,oLookAhead,oLookAheadFactor,Param.Tmp.oSkill);
  }
  // ... Adjust skilling

  // Find side and sections of pits ...
  TTrackDescription::PitSideMod PitSideMod;      // Data for track description
  PitSideMod.side = PitSide();                   // Get side of pitlane
  PitSideMod.start =                             // Exclude pits
	int(GfParmGetNum(Handle,                     //   while getting extra width
	TDriver::SECT_PRIV,PRV_TRKPIT_START,0,0));   //   starting here
  PitSideMod.end =                               //   and stopping here
	int(GfParmGetNum(Handle,
	TDriver::SECT_PRIV,PRV_TRKPIT_END,0,0));
  // ... Find side and sections of pits

  // Create track description
  oTrackDesc.InitTrack(oTrack,Param.oCarParam,&PitSideMod);

  // Create pitting strategy
  oStrategy = new TSimpleStrategy();
  oStrategy->oDriver = this;
  oStrategy->oMaxFuel = oMaxFuel;
  Param.Fix.oStrategy = oStrategy; 

  // Setup initial fuel for race ...
  float Fuel = GfParmGetNum(Handle               // Estimate fuel consum
    , TDriver::SECT_PRIV, PRV_FUELPER100KM                // based on kg per 100 km
    , (char*) NULL
	, TSimpleStrategy::cMAX_FUEL_PER_METER * 100000);

  float Reserve = GfParmGetNum(Handle            // Reserve in m
    , TDriver::SECT_PRIV, PRV_RESERVE
    , (char*) NULL, 2000);
  GfOut("#Reserve: %.0f\n",Reserve);
  oStrategy->oReserve = Reserve;
  oFuelNeeded =
    oStrategy->SetFuelAtRaceStart                // Fueling and pitting
	  (oTrack,CarSettings,oSituation,Fuel);      //   strategy
  GfOut("#oFuelNeeded: %.1f\n",oFuelNeeded);
  // ... Setup initial fuel for race

  GfOut("#\n\n\n#<<< TDriver::InitTrack\n\n\n");
}
//==========================================================================*

//==========================================================================*
// Check if pit sharing is activated
//--------------------------------------------------------------------------*
bool TDriver::CheckPitSharing()
{
  const tTrackOwnPit* OwnPit = CarPit;           // Get my pit

  if (OwnPit == NULL)                            // If pit is NULL
  {                                              // nothing to do
	GfOut("\n\nPit = NULL\n\n");                 // here
	return false;
  }

  if (OwnPit->freeCarIndex > 1)
  {
  	  GfOut("\n\nPitSharing = true\n\n");    
	  return true;
  }
  else
  {
  	  GfOut("\n\nPitSharing = false\n\n");    
	  return false;
  }
}
//==========================================================================*

//==========================================================================*
// Start a new race.
//--------------------------------------------------------------------------*
void TDriver::NewRace(PtCarElt Car, PSituation Situation)
{
  //GfOut("#>>> TDriver::NewRace()\n");
  oCar = Car;                                    // Save pointers to TORCS
  oCarHandle = CarCarHandle;                     // data of car, car param
  oSituation = Situation;                        // file and situation
  oLastGear = CarGearNbr - 1;                    // Save index of last gear

  // 
  OwnCarOppIndex();                              // Find own opponent index

  InitCarModells();                              // Initilize Car modells
  oStrategy->Init(this);                         // Init strategy
  oPitSharing = CheckPitSharing();               // Is pitsharing activated?
  FindRacinglines();                             // Find a good racingline
  TeamInfo();                                    // Find team info

  oFlying = 0;                                   // Initialize Flags
  oAvoidRange = 0.9;                             // Relative avoiding offset
  oAvoidRangeDelta = 0.0;                        // Avoiding range change
  oAvoidOffset = CalcPathTarget                  // Get initial offset
	(oTrackDesc.CalcPos(oCar), -CarToMiddle);    // from start grid
  oAvoidOffsetDelta = 0.0;                       // Avoiding speed

  // Skilling from Andrew Sumner ...
  oSkillAdjustTimer = -1;
  oSkillAdjustLimit = 0.0;
  oBrakeAdjustTarget = oDecelAdjustTarget = 1.0f;
  oBrakeAdjustPerc = oDecelAdjustPerc = 1.0f;
//  SetRandomSeed(oIndex);
  SetRandomSeed(0);
  // ... Skilling from Andrew Sumner
  if (RM_TYPE_PRACTICE == oSituation->_raceType)
  {
    oSkill = 1.0;
	Param.Tmp.oSkill = oSkill;
  }
/*
  else if (oSkilling && (RM_TYPE_RACE == oSituation->_raceType))
  {
    Param.oCarParam.oScaleBrake -= 0.1;
    Param.oCarParam.oScaleMu -= 0.1;
  }
*/
/*
 for (int I = 0; I < 2; I++)
	cTimeSum[I] = 0.0;
*/
  //GfOut("#<<< TDriver::NewRace()\n");
}
//==========================================================================*

//==========================================================================*
// Drive
//--------------------------------------------------------------------------*
void TDriver::Drive()
{
/*
  if (CarLaps == 1 + oIndex)
    oTestPitStop = 1;
  else
    oTestPitStop = 0;
	if(!Qualification)
  {
    if (CarLaps > 1)
      oTestPitStop = 1;
  }
*/
  if (oTestPitStop)                              // If defined, try
    oStrategy->TestPitStop();                    //   to stop in pit

  Propagation();                                 // Propagation

  oAlone = true;                                 // Assume free way to race
  bool Close = false;                            // Assume free way to race
  oLetPass = false;                              // Assume no Lappers

  oAccel = 1.0;                                  // Assume full throttle
  oBrake = 0.0;                                  // Assume no braking

  double StartTimeStamp = RtTimeStamp(); 

  double Pos = oTrackDesc.CalcPos(oCar);         // Get current pos on track

  GetPosInfo(Pos,oLanePoint);                    // Get info about pts on track
  oTargetSpeed = oLanePoint.Speed;				 // Target for speed control
  oTargetSpeed = FilterStart(oTargetSpeed);      // Filter Start
  double TrackRollangle = oRacingLine[oRL_FREE].CalcTrackRollangle(Pos);

//  cTimeSum[0] += RtDuration(StartTimeStamp);

  AvoidOtherCars(oLanePoint.Crv,Close,oLetPass); // General avoiding

//  cTimeSum[1] += RtDuration(StartTimeStamp);

  oSteer = Steering();                           // Steering
  oSteer = FilterSteerSpeed(oSteer);             // Steering

//  cTimeSum[2] += RtDuration(StartTimeStamp);

  if(Close)                                      // If opponents are close
  {
	BrakingForceRegulatorTraffic();              // Control breaking force
  }
  else if(oStrategy->GoToPit())                  // Going to pitlane
  {
	BrakingForceRegulatorTraffic();              // Control breaking force
  }
  else
  {
	if (oAvoidRange == 0.0)                      // Still avoiding?
      BrakingForceRegulator();                   // Control breaking force
	else
	  BrakingForceRegulatorAvoid();              // Control breaking force
  }

  Clutching();                                   // Tread/Release clutch
  GearTronic();                                  // Shift if needed
  Turning();                                     // Check driving direction
  FlightControl();                               // Prepare landing while flying
/*
  if (IsFullThrottle)                            // Learning only if untroubled
	oMaxAccel.Measurement                        // get samples
	  (CarSpeedLong,CarAccelLong);
*/
  // Filters for brake
  oBrake = FilterBrake(oBrake);
  oBrake = FilterBrakeSpeed(oBrake);
  oBrake = FilterABS(oBrake);
  if (oBrake == 0.0)
  {
    // Filters for throttle
    oAccel = FilterLetPass(oAccel);
    oAccel = FilterDrifting(oAccel);
    oAccel = FilterTrack(oAccel);
    oAccel = FilterTCL(oAccel);
    if (oUseFilterAccel)
      oAccel = FilterAccel(oAccel);
  }

  oLastSteer = oSteer;
  // Tell TORCS what we want do do
  oLastAccel = oAccel;
  oCar->ctrl.accelCmd = (float) oAccel;
  oCar->ctrl.brakeCmd = (float) oBrake;
  oCar->ctrl.clutchCmd = (float) oClutch;
  oCar->ctrl.gear = oGear;
  oCar->ctrl.steer = (float) oSteer;

  //int Idx = oTrackDesc.IndexFromPos(Pos);
  //GfOut("#%d: %g P:%.0f(%d) A: %g B: %g C: %g G: %d S: %g\n",oIndex,CurrSimTime,Pos,Idx,oAccel,oBrake,oClutch,oGear,oSteer);
/*
  if (oDoAvoid)
    oCar->ctrl.lightCmd = RM_LIGHT_HEAD2;        // Only small lights on
  else
    oCar->ctrl.lightCmd = RM_LIGHT_HEAD1;        // Only big lights on
  oCar->ctrl.lightCmd =                          // All front lights on
    RM_LIGHT_HEAD1 | RM_LIGHT_HEAD2;
*/
  oCar->ctrl.lightCmd =                          // All front lights on
    RM_LIGHT_HEAD1 | RM_LIGHT_HEAD2;             // All rear lights on

  if (!Qualification)                            // Don't use pit while
    oStrategy->CheckPitState(0.6f);              //  qualification

  //if ((oCurrSpeed < 100.0/3.6) && (CurrSimTime > 0) && (CurrSimTime < 10))
  //if (fabs(oLanePoint.Crv) > 1/50.0)
  //  GfOut("t:%.2f s v:(%.1f)%.1f km/h A:%.3f C:%.3f G:%d R:%.1f H:%.3f\n",CurrSimTime,oTargetSpeed*3.6,oCurrSpeed*3.6,oAccel,oClutch,oGear,1/oLanePoint.Crv,CalcHairpin_simplix_36GP(fabs(oLanePoint.Crv)));
  //else
  //  GfOut("t:%.2f s v:(%.1f)%.1f km/h A:%.3f C:%.3f G:%d R:%.1f F:%.3f\n",CurrSimTime,oTargetSpeed*3.6,oCurrSpeed*3.6,oAccel,oClutch,oGear,1/oLanePoint.Crv,CalcCrv_simplix_36GP(fabs(oLanePoint.Crv)));
  GfOut("t:%.2f s v:(%.1f)%.1f km/h Z:%.3f RA:%.3f RAD:%.1f F:%.3f H:%.3f\n",CurrSimTime,oTargetSpeed*3.6,oCurrSpeed*3.6,oLanePoint.Crvz,TrackRollangle,1/oLanePoint.Crv,CalcCrv_simplix_SC(fabs(oLanePoint.Crv)),CalcHairpin_simplix_SC(fabs(oLanePoint.Crv)));
}
//==========================================================================*

//==========================================================================*
// Pitstop callback
//--------------------------------------------------------------------------*
int TDriver::PitCmd()
{
  oStanding = true;                              // Standing, no unstucking!
  oUnstucking = false;                           // Reset pending flag

  // Tell TORCS ...
  oCar->pitcmd.fuel = oStrategy->PitRefuel();    // ... how much to refuel
  oCar->pitcmd.repair = oStrategy->PitRepair();  // and to repair
  oCar->pitcmd.stopType = RM_PIT_REPAIR;         // Set repair flag

  if (oCar->pitcmd.repair > 0)                   // If repairing, show
    GfOut("#%s repairing: %d damage\n",           // who and how much
	oBotName,oCar->pitcmd.repair);
  if (oCar->pitcmd.fuel > 0.0)                   // If refueling
    GfOut("#%s refueling: %.2f\n",                // show who and how much
	oBotName,oCar->pitcmd.fuel);

  oFuelNeeded += oCar->pitcmd.fuel;
  oRepairNeeded += oCar->pitcmd.repair;

  return ROB_PIT_IM;                             // Ready to be serviced
}
//==========================================================================*

//==========================================================================*
// End of the current race.
//--------------------------------------------------------------------------*
void TDriver::EndRace()
{
  GfOut("#TDriver::EndRace() >>>\n");
  oStrategy->PitRelease();                       // Release pit if eliminated
  GfOut("#<<< TDriver::EndRace()\n");
}
//==========================================================================*

//==========================================================================*
// Called before the module is unloaded.
//--------------------------------------------------------------------------*
void TDriver::Shutdown()
{
#ifdef SPEED_DREAMS
	RtTeamManagerDump();
	RtTeamManagerRelease();
#endif
/*
	GfOut("\n\n\n");
	for (int I = 0; I < 7; I++)
		GfOut("cTimeSum[%d]: %g msec\n",I,cTimeSum[I]);
	GfOut("\n\n\n");
*/
}
//==========================================================================*

//==========================================================================*
// Side of pitlane
//--------------------------------------------------------------------------*
int TDriver::PitSide()
{
  return oTrack->pits.side == TR_LFT ? -1 : 1;
}
//==========================================================================*

//==========================================================================*
// Find own car in opponents list
//--------------------------------------------------------------------------*
void TDriver::OwnCarOppIndex()
{
  oOwnOppIdx = -1;                               // Mark as undefined

  if (oNbrCars == 0)
  {
	// First call: Get memory
    oNbrCars = oSituation->_ncars;               // Save Nbr of cars in race
	oOpponents = new TOpponent[oNbrCars];

    for (int I = 0; I < oNbrCars; I++)           // Loop all cars
    {
      oOpponents[I].Initialise                   // Initialize opponents
	    (&oTrackDesc, oSituation, I);            //   situation
	}
  }

  for (int I = 0; I < oNbrCars; I++)             // Loop all cars
  {
    if (oSituation->cars[I] == oCar)             // Check if is own car
      oOwnOppIdx = I;                            //   save index
  }
}
//==========================================================================*

//==========================================================================*
// Init drive train
//--------------------------------------------------------------------------*
void TDriver::InitDriveTrain()
{
  oDriveTrainType = cDT_RWD;                     // Assume rear wheel drive
  const char* TrainType =                        // but check it
	GfParmGetStr(oCarHandle,
	SECT_DRIVETRAIN, PRM_TYPE, VAL_TRANS_RWD);

  if (strcmp(TrainType, VAL_TRANS_FWD) == 0)     // If front wheel drive
    oDriveTrainType = cDT_FWD;                   //   change mode
  else if (strcmp(TrainType, VAL_TRANS_4WD) == 0)// and if all wheel drive
    oDriveTrainType = cDT_4WD;                   //   too
}
//==========================================================================*

//==========================================================================*
// FindRacinglines
//--------------------------------------------------------------------------*
void TDriver::FindRacinglines()
{
  GfOut("#Update car parameters ...\n");
  Param.Update();                                // update car parameters

  GfOut("# ... set track ...\n");
  if(oCommonData->Track != oTrackDesc.Track())   // New track?
  {
    oCommonData->Track = oTrackDesc.Track();     // Save pointer
#ifdef SPEED_DREAMS
#else
    oCommonData->TeamManager.Clear();            // release old informations
#endif
  }

  GfOut("# ... load smooth path ...\n");
  if (oSituation->_raceType == RM_TYPE_PRACTICE)
  {
    GfOut("# ... make smooth path ...\n");
    oRacingLine[oRL_FREE].MakeSmoothPath         // Calculate a smooth path
	  (&oTrackDesc, Param,                       // as main racingline
	  TClothoidLane::TOptions(oBumpMode));
    //oRacingLine[oRL_FREE].SaveToFile("RL_FREE.tk3");
    oRacingLine[oRL_FREE].SavePointsToFile(oTrackLoad);
  }
  else if (oSituation->_raceType == RM_TYPE_QUALIF)
  {
    if (!oRacingLine[oRL_FREE].LoadSmoothPath    // Load a smooth path
	  (oTrackLoadQualify,
	  &oTrackDesc, Param,                        // as main racingline
	  TClothoidLane::TOptions(oBumpMode)))
	{
      GfOut("# ... make smooth path ...\n");
      oRacingLine[oRL_FREE].MakeSmoothPath       // Calculate a smooth path
	    (&oTrackDesc, Param,                     // as main racingline
	    TClothoidLane::TOptions(oBumpMode));
      oRacingLine[oRL_FREE].SavePointsToFile(oTrackLoadQualify);
	}
  }
  else if (!oRacingLine[oRL_FREE].LoadSmoothPath // Load a smooth path
	  (oTrackLoad,
	  &oTrackDesc, Param,                        // as main racingline
	  TClothoidLane::TOptions(oBumpMode)))
  {
    GfOut("# ... make smooth path ...\n");
    oRacingLine[oRL_FREE].MakeSmoothPath         // Calculate a smooth path
	  (&oTrackDesc, Param,                       // as main racingline
	  TClothoidLane::TOptions(oBumpMode));
    //oRacingLine[oRL_FREE].SaveToFile("RL_FREE.tk3");
    oRacingLine[oRL_FREE].SavePointsToFile(oTrackLoad);
  }

  if (Qualification)
  {
	oRL_LEFT = oRL_FREE;
	oRL_RIGHT = oRL_FREE;
  }
  else
  {
    Param.oCarParam2.oScaleBumpRight =           // Adjust outer bump scale
	  Param.oCarParam.oScaleBumpOuter;           //   to be able to avoid
	Param.oCarParam2.oScaleMu =                  // Adjust mu scale
	  oSideScaleMu*Param.oCarParam.oScaleMu;     //   to be able to avoid
	Param.oCarParam2.oScaleBrake =               // Adjust brake scale
	  oSideScaleBrake*Param.oCarParam.oScaleBrake; //   to be able to avoid
    Param.Fix.oBorderOuter += oSideBorderOuter;
	
    if (!oRacingLine[oRL_LEFT].LoadSmoothPath    // Load a smooth path
	  (oTrackLoadLeft,
	  &oTrackDesc, Param,                        // as avoid to left racingline
	    TClothoidLane::TOptions(oBumpMode, FLT_MAX, -oAvoidWidth, true)))
	{
      GfOut("# ... make avoid path left ...\n");

      oRacingLine[oRL_LEFT].MakeSmoothPath       // Avoid to left racingline
	    (&oTrackDesc, Param,
		TClothoidLane::TOptions(oBumpMode, FLT_MAX, -oAvoidWidth, true));
      //oRacingLine[oRL_LEFT].SaveToFile("RL_LEFT.tk3");
      oRacingLine[oRL_LEFT].SavePointsToFile(oTrackLoadLeft);
	}

  	Param.oCarParam2.oScaleBumpLeft =            // Adjust outer bump scale
	  Param.oCarParam.oScaleBumpOuter;           //   to be able to avoid
    Param.oCarParam2.oScaleBumpRight =           // Reset outer bump scale
	  Param.oCarParam.oScaleBump;                //   to keep speed

	if (!oRacingLine[oRL_RIGHT].LoadSmoothPath   // Load a smooth path
	  (oTrackLoadRight,
	  &oTrackDesc, Param,                        // as avoid to right racingline
  	    TClothoidLane::TOptions(oBumpMode, -oAvoidWidth, FLT_MAX, true)))
	{
      GfOut("# ... make avoid path right ...\n");

	  oRacingLine[oRL_RIGHT].MakeSmoothPath      // Avoid to right racingline
	    (&oTrackDesc, Param,
  	    TClothoidLane::TOptions(oBumpMode, -oAvoidWidth, FLT_MAX, true));
      //oRacingLine[oRL_RIGHT].SaveToFile("RL_RIGHT.tk3");
      oRacingLine[oRL_RIGHT].SavePointsToFile(oTrackLoadRight);
	}

    double MaxPitDist = 0.0;
	if (oStrategy->oPit->HasPits())
	{
      for (int I = 0; I < NBRRL; I++)            // Adjust racinglines
      {                                          // using car parameters
	    GfOut("# ... adjust pit path %d ...\n",I);
        oStrategy->oPit->oPitLane[I].MakePath
	      (oPitLoad[I],&oRacingLine[I], Param, I);

	    if (MaxPitDist < oStrategy->oPit->oPitLane[I].PitDist())
          MaxPitDist = oStrategy->oPit->oPitLane[I].PitDist();
	  }
	  //oStrategy->oPit->oPitLane[oRL_FREE].SaveToFile("RL_PIT_FREE.tk3");
	  //oStrategy->oPit->oPitLane[oRL_LEFT].SaveToFile("RL_PIT_LEFT.tk3");
	  //oStrategy->oPit->oPitLane[oRL_RIGHT].SaveToFile("RL_PIT_RIGHT.tk3");
	  oStrategy->oDistToSwitch = MaxPitDist + 100; // Distance to pit entry
	}
  }

  for (int I = 0; I < NBRRL; I++)
  {
    oRacingLine[I].CalcMaxSpeeds(1);
    oRacingLine[I].PropagateBreaking(1);
    oRacingLine[I].PropagateAcceleration(1);
  }
  GfOut("# ... Done\n");
}
//==========================================================================*

//==========================================================================*
// Get Team info
//--------------------------------------------------------------------------*
void TDriver::TeamInfo()
{
#ifdef SPEED_DREAMS
  //RtTeamManagerShowInfo();
  oTeamIndex = RtTeamManagerIndex(oCar,oTrack,oSituation);
  RtTeamManagerDump();
#else
  oTeam = oCommonData->TeamManager.Add(oCar,oSituation);
#endif
  GfOut("#\n# %s Team: %s Teamindex: %d\n#\n",oBotName, oCar->_teamname, oTeamIndex);
}
//==========================================================================*

//==========================================================================*
// Initialize Car modells
//--------------------------------------------------------------------------*
void TDriver::InitCarModells()
{
  oCarParams[0] = &Param.oCarParam;              // Get pointers as array
  oCarParams[1] = &Param.oCarParam2;
  oCarParams[2] = &Param.oCarParam2;

  Param.Initialize(this,oCar);                   // Initialize parameters

  Param.SetEmptyMass(                            // Set car mass
	GfParmGetNum(oCarHandle,
	  SECT_CAR, PRM_MASS, NULL, 1000.0));

  InitCa();                                      // Ca
  InitCw();                                      // Cw
  InitDriveTrain();                              // Drive train
  InitTireMu();                                  // Tyre friction
  InitWheelRadius();                             // Wheel radius
  InitAdaptiveShiftLevels();                     // Gear shifting

  Param.Tmp.oFuel = 0;                           // Still unfueld
  Param.Fix.oWidth = CarWidth;                   // width of car
  Param.oCarParam2 = Param.oCarParam;            // Copy to avoid set
  Param.oCarParam2.oScaleMu =                    // Adjust mu scale to
	MIN(0.95, 0.9 * Param.oCarParam.oScaleMu);   //   be able to avoid
  Param.oCarParam3 = Param.oCarParam;            // Copy to pit set
}
//==========================================================================*

//==========================================================================*
// Update pointers and multiple used values
//--------------------------------------------------------------------------*
void TDriver::Update(tCarElt* Car, tSituation* S)
{
  oCar = Car;                                    // Update pointers
  oSituation = S;

  // Shortcuts
  oCurrSpeed = myhypot(CarSpeedLong, CarSpeedLat); // Save currend speed
  if (fabs(oCurrSpeed) < 1)                      // At slow speeds use
    oAngleSpeed = CarYaw;                        // direction of cars x-axis
  else                                           // else use
    oAngleSpeed = atan2(CarSpeedY, CarSpeedX);   // direction of movement

//  Param.Tmp.oSkill =
//	(1.0 + oSkill/oSkillScale + CarDamage/30000);// Adjust skill to damages
//	(1.0 + oSkill + CarDamage/30000);            // Adjust skill to damages

  oTrackAngle =                                  // Direction of track at the
	 RtTrackSideTgAngleL(&CarTrackPos);          // position of the car
  oDistFromStart = oTrackDesc.CalcPos(oCar, 0.0);// Cars distance from Start
  TVec2d Target =                                // Target to steer to
	CalcPathTarget2(oDistFromStart + 5.0, 0.0);  // while unstucking
  oSteerAngle = (float) atan2                    // Direction to steer
	(Target.y - CarPosY, Target.x - CarPosX);    //   from here to target
  oSteerAngle -= (float) CarYaw;                 // Relative to cars x-axis
//  oSteerAngle -= (float) oAngleSpeed;            // Relative to cars movement
  FLOAT_NORM_PI_PI(oSteerAngle);                 // Normalize to -PI,+PI

  oDriftAngle =                                  // Actual drift angle
	atan2(CarSpeedY, CarSpeedX) - CarYaw;
  DOUBLE_NORM_PI_PI(oDriftAngle);                // normalized to +Pi .. -Pi

  // Get direction of motion
  double MySpd = MAX(0.01,myhypot(CarSpeedX, CarSpeedY));
  double MyDomX = CarSpeedX / MySpd;
  double MyDomY = CarSpeedY / MySpd;
  float MinDistBack = -FLT_MAX;
  double MinTimeSlot = FLT_MAX;

  // Update all opponents data
  for (int I = 0; I < oNbrCars; I++)
  {
	oOpponents[I].Update(oCar,
#ifdef SPEED_DREAMS
#else
	  &oCommonData->TeamManager, 
#endif
	  MyDomX, MyDomY, MinDistBack, MinTimeSlot);
  }

  oStrategy->Update(oCar,MinDistBack,MinTimeSlot);// Update strategic params

  oSideReduction = 1.0;
  if (WheelSeg(REAR_RGT) != WheelSeg(REAR_LFT))
  {
    float MinFriction = MIN(WheelSegFriction(REAR_RGT),WheelSegFriction(REAR_LFT));
	oSideReduction = MIN(1.0,MinFriction / CarFriction);
	//GfOut("SideReduction: %g\n",oSideReduction);
  }
}
//==========================================================================*

//==========================================================================*
// Detect flight
//--------------------------------------------------------------------------*
void TDriver::DetectFlight()
{
  double H[4];
  double HMax = 0.0;

  for (int I = 0; I < 4; I++)
  {
    tTrkLocPos Wp;
    float Wx = oCar->pub.DynGCg.pos.x;
    float Wy = oCar->pub.DynGCg.pos.y;
    RtTrackGlobal2Local(CarSeg, Wx, Wy, &Wp, TR_LPOS_SEGMENT);
    H[I] = CarPosZ - RtTrackHeightL(&Wp) - WheelRad(I);
	if (HMax < H[I])
      HMax = H[I];
  }

  if (HMax > oFlyHeight)
  {
    oFlying = MIN(FLY_COUNT, oFlying + (FLY_COUNT / 2));
  }
  else if (oFlying > 0)
  {
    oFlying--;
  }
}
//==========================================================================*

//==========================================================================*
// Prepare landing
//--------------------------------------------------------------------------*
void TDriver::FlightControl()
{
  if (oFlying)
  {
    // Steer in direction of car movement
    double Angle = oAngleSpeed - CarYaw;
    DOUBLE_NORM_PI_PI(Angle);
    int F = FLY_COUNT - oFlying;
    double T = MAX(0, MIN(1.0 * F / FLY_COUNT, 1));
    oSteer = oSteer * T + (1 - T) * Angle / SteerLock;
  }
}
//==========================================================================*

//==========================================================================*
// Propagation
//--------------------------------------------------------------------------*
void TDriver::Propagation()
{
  if (Param.Tmp.Needed())
  {
	Param.Update();

	for (int I = 0; I < NBRRL; I++)
	{
      oRacingLine[I].CalcMaxSpeeds(1);
	  oRacingLine[I].PropagateBreaking(1);
      oRacingLine[I].PropagateAcceleration(1);
	}
  }
}
//==========================================================================*

//==========================================================================*
// Steering
//--------------------------------------------------------------------------*
double TDriver::Steering()
{
  TLanePoint AheadPointInfo;
  if (oUnstucking)
  {
    double Factor = 4.0 * MAX(0,MIN(1.0,CarSpeedLong));
    double Angle = UnstuckSteerAngle(oLanePoint,AheadPointInfo) * Factor;
    oAngle = SteerAngle(AheadPointInfo);
    double T = MAX(0.0,7 - MAX(0,CarSpeedLong));
	oAngle = Angle * T + oAngle * (1 - T);
  }
  else
    oAngle = SteerAngle(AheadPointInfo);
  oDeltaOffset = oLanePoint.Offset + CarToMiddle;// Delta to planned offset
  return oAngle / SteerLock;
}
//==========================================================================*

//==========================================================================*
// Calculate mean wheel radius
//--------------------------------------------------------------------------*
void TDriver::InitWheelRadius()
{
  int Count = 0;
  oWheelRadius = 0.0;

  if(HasDriveTrainFront)
  {
    oWheelRadius += WheelRad(FRNT_LFT) + WheelRad(FRNT_RGT);
    Count += 2;
  }

  if(HasDriveTrainRear)
  {
    oWheelRadius += WheelRad(REAR_LFT) + WheelRad(REAR_RGT);
    Count += 2;
  }
  oWheelRadius /= Count;
}
//==========================================================================*

//==========================================================================*
// Initialize tire mu
//--------------------------------------------------------------------------*
void TDriver::InitTireMu()
{
  int I;

  Param.Fix.oTyreMuFront = FLT_MAX;
  for (I = 0; I < 2; I++)
	Param.Fix.oTyreMuFront = MIN(Param.Fix.oTyreMuFront,
	  GfParmGetNum(oCarHandle, WheelSect[I],
	  PRM_MU, (char*) NULL, 1.0f));

  Param.Fix.oTyreMuRear = FLT_MAX;
  for (I = 2; I < 4; I++)
	Param.Fix.oTyreMuRear = MIN(Param.Fix.oTyreMuRear,
	  GfParmGetNum(oCarHandle, WheelSect[I],
	  PRM_MU, (char*) NULL, 1.0f));

  Param.Fix.oTyreMu =
	MIN(Param.Fix.oTyreMuFront,Param.Fix.oTyreMuRear);
}
//==========================================================================*

//==========================================================================*
// Initialize Ca
//--------------------------------------------------------------------------*
void TDriver::InitCa()
{
  float FrontWingArea =
	GfParmGetNum(oCarHandle, SECT_FRNTWING,
	  PRM_WINGAREA, (char*) NULL, 0.0);
  float FrontWingAngle =
	GfParmGetNum(oCarHandle, SECT_FRNTWING,
	  PRM_WINGANGLE, (char*) NULL, 0.0);
  float RearWingArea =
	GfParmGetNum(oCarHandle, SECT_REARWING,
	  PRM_WINGAREA, (char*) NULL, 0.0f);
  float RearWingAngle =
	GfParmGetNum(oCarHandle, SECT_REARWING,
	   PRM_WINGANGLE, (char*) NULL, 0.0f);

  FrontWingArea = FrontWingArea * sin(FrontWingAngle);
  RearWingArea = RearWingArea * sin(RearWingAngle);
  float WingCd = (float) (1.23 * (FrontWingArea + RearWingArea));
  Param.Fix.oCdWing = WingCd;

  float CL =
	GfParmGetNum(oCarHandle, SECT_AERODYNAMICS,
	  PRM_FCL, (char*) NULL, 0.0f)
	+ GfParmGetNum(oCarHandle, SECT_AERODYNAMICS,
	  PRM_RCL, (char*) NULL, 0.0f);

  float H = 0.0;
  int I;
  for (I = 0; I < 4; I++)
	H += GfParmGetNum(oCarHandle, WheelSect[I],
	  PRM_RIDEHEIGHT, (char*) NULL, 0.20f);

  H *= 1.5;
  H = H*H;
  H = H*H;
  H = (float) (2.0 * exp(-3.0 * H));
  Param.Fix.oCa = H * CL + 4.0 * WingCd;
  Param.Fix.oCaFrontWing = 4 * 1.23 * FrontWingArea;
  Param.Fix.oCaRearWing = 4 * 1.23 * RearWingArea;
  Param.Fix.oCaGroundEffect = H * CL;

}
//==========================================================================*

//==========================================================================*
// Initialize Cw
//--------------------------------------------------------------------------*
void TDriver::InitCw()
{
  float Cx =
	GfParmGetNum(oCarHandle,
	SECT_AERODYNAMICS, PRM_CX, (char*) NULL, 0.0f);
  float FrontArea =
	GfParmGetNum(oCarHandle,
	SECT_AERODYNAMICS, PRM_FRNTAREA, (char*) NULL, 0.0f);

  Param.Fix.oCdBody = 0.645 * Cx * FrontArea;
}
//==========================================================================*

//==========================================================================*
// Get gear ratio
//--------------------------------------------------------------------------*
double TDriver::GearRatio()
{
  return CarGearRatio[UsedGear + CarGearOffset];
}
//==========================================================================*

//==========================================================================*
// Get gear ratio of previous gear
//--------------------------------------------------------------------------*
double TDriver::PrevGearRatio()
{
  return CarGearRatio[UsedGear + CarGearOffset-1];
}
//==========================================================================*

//==========================================================================*
// Get gear ratio of next gear
//--------------------------------------------------------------------------*
double TDriver::NextGearRatio()
{
  return CarGearRatio[UsedGear + CarGearOffset+1];
}
//==========================================================================*

//==========================================================================*
// Start automatic
//--------------------------------------------------------------------------*
void TDriver::StartAutomatic()
{
  if (oSituation->_raceState & RM_RACE_PRESTART) 
  {
	oClutch = oClutchMax;
	return;
  }

  if ((CarGearCmd == 1) && (TDriver::CurrSimTime < 20))
  {
	if (CarRpm < oStartRPM) 
	  oClutch += oClutchDelta;
	else if (CarRpm > 1.1 * oStartRPM) 
	  oClutch -= oClutchDelta * oClutchRelease;
  }
}
//==========================================================================*

//==========================================================================*
// Simplified clutch controller
//--------------------------------------------------------------------------*
void TDriver::Clutching()
{
  if (TDriver::CurrSimTime < oSituation->deltaTime)
  {
    oClutch = oClutchMax;
	return;
  }

  if(oClutch > 0)
  {
    if (oGear < 2)
      StartAutomatic();

	oClutch = MIN(oClutchMax,oClutch);
	if(oClutch == oClutchMax)
	{
	  if(GearRatio() * CarSpeedLong
		  / (oWheelRadius * CarRpm) > oClutchRange)
	  {
        oClutch = oClutchMax - 0.01;
	  }
	}
	else
	{
	  oClutch -= oClutchDelta;
	  oClutch = MAX(0.0,oClutch);
	}
  }
}
//==========================================================================*

//==========================================================================*
// Turn if driving backwards unexpectedly
//--------------------------------------------------------------------------*
void TDriver::Turning()
{
  if (!oUnstucking && (DistanceRaced > 25))
  {
    double Angle = oLanePoint.Angle - CarYaw;    // Direction moving to
    DOUBLE_NORM_PI_PI(Angle);                    // normalize it

    if((oGear > 0)                               // If a gear is selected
      && (fabs(Angle) > 75 * PI / 180))          // but irregular direction
    {
      if(Angle * CarToMiddle < 0)                // Lets turn
  	  {
        oGear = -1;
        oAccel = 0.5;
        oBrake = 0;
        oSteer = -SGN(Angle);
	  }
    }

    if((oGear > 0) && (CarSpeedLong < -0.01))    // Rolling back?
    {                                            // Shift down and start
      oGear = 1;
      oBrake = CarSpeedLong < -0.5 ? 0.25 : 0;
      oAccel = 0.25;
    }

    if ((oGear == 1)                             // If starting
	  && (CarSpeedLong < 10)                     //   and slow
	  && (fabs(CarSpeedLong) >= 0.01)            //   but moving
	  && (oAccel == 1.0 && oBrake == 0))         //   and acc. (not braking)
    {                                            // use clutch
      double rpm = CarRpm;
      oClutch = (850 - rpm) / 400;
      if(CarSpeedLong < 0.05)
        oClutch = 0.5;

      oClutch = MAX(0, MIN(oClutch, 0.9));       // Normalize
    }
  }
}
//==========================================================================*

//==========================================================================*
// Calculate shift levels
//--------------------------------------------------------------------------*
void TDriver::InitAdaptiveShiftLevels()
{
  //GfOut("#TDriver::InitShiftLevels() >>>\n");

  struct tEdesc
  {
    tdble rpm;
    tdble tq;
  } *Edesc;

  struct TDataPoints
  {
    tdble rads;
    tdble a;
    tdble b;
  } *DataPoints;

  float RevsMax;
  float Tickover;
  float RevsLimiter;
  double RpmFactor = 30 / PI;                    // Unit conversion

  char	idx[64];
  sprintf(idx, "%s/%s", SECT_ENGINE, ARR_DATAPTS);
  int IMax = GfParmGetEltNb(oCarHandle, idx);

  RevsMax = GfParmGetNum(oCarHandle, SECT_ENGINE,
	PRM_REVSMAX, (char*)NULL, 1000);

  Tickover = GfParmGetNum(oCarHandle, SECT_ENGINE,
	  PRM_TICKOVER, (char*)NULL, 150);

  RevsLimiter = GfParmGetNum(oCarHandle, SECT_ENGINE,
	  PRM_REVSLIM, (char*)NULL, 800);

  Edesc = (struct tEdesc*) malloc((IMax + 1) * sizeof(struct tEdesc));

  int I;

  oShiftMargin = 0.9;                            //
  for (I = 0; I < MAX_GEARS; I++)
  {
    oShift[I] = 2000.0;
    if (TDriver::UseBrakeLimit)
	  oGearEff[I] = 0.95;
	else
	  oGearEff[I] = 0.95;
  }

  for (I = 0; I < IMax; I++)
  {
	sprintf(idx, "%s/%s/%d", SECT_ENGINE, ARR_DATAPTS, I+1);
    Edesc[I].rpm = GfParmGetNum(oCarHandle, idx,
	  PRM_RPM, (char*) NULL, RevsMax);
    Edesc[I].tq = GfParmGetNum(oCarHandle, idx,
	  PRM_TQ, (char*) NULL, 0.0f);
  }

  Edesc[IMax].rpm = Edesc[IMax - 1].rpm;
  Edesc[IMax].tq  = Edesc[IMax - 1].tq;

  double maxTq = 0;
  double rpmMaxTq = 0;
  double maxPw = 0;
  double rpmMaxPw = 0;
  double TqAtMaxPw = 0;
  DataPoints = (TDataPoints *) malloc(IMax * sizeof(TDataPoints));
  TDataPoints *Data;
  for (I = 0; I < IMax; I++)
  {
	Data = &DataPoints[I];

	Data->rads = Edesc[I+1].rpm;
	if ((Data->rads >= Tickover)
			&& (Edesc[I+1].tq > maxTq)
			&& (Data->rads < RevsLimiter))
	{
	  maxTq = Edesc[I+1].tq;
	  rpmMaxTq = Data->rads;
	  if (oStartRPM < rpmMaxTq)
	    oStartRPM = rpmMaxTq;
	}
	if ((Data->rads >= Tickover)
			&& (Data->rads * Edesc[I+1].tq > maxPw)
			&& (Data->rads < RevsLimiter))
	{
	  TqAtMaxPw = Edesc[I+1].tq;
	  maxPw = Data->rads * Edesc[I+1].tq;
	  rpmMaxPw = Data->rads;
	}

	Data->a = (Edesc[I+1].tq - Edesc[I].tq)
	  / (Edesc[I+1].rpm - Edesc[I].rpm);
	Data->b = Edesc[I].tq - Data->a * Edesc[I].rpm;
  }

  //GfOut("#\n\n\n#oStartRPM: %g(%g)\n",oStartRPM*RpmFactor,oStartRPM);
  //GfOut("#RevsLimiter: %g(%g)\n",RevsLimiter*RpmFactor,RevsLimiter);
  //GfOut("#RevsMax: %g(%g)\n\n\n",RevsMax*RpmFactor,RevsMax);
  

  for (I = 0; I < CarGearNbr - 1; I++)
  {
	sprintf(idx, "%s/%s/%d", SECT_GEARBOX, ARR_GEARS, I+1);
    oGearEff[I] = GfParmGetNum(oCarHandle, idx,
	  PRM_EFFICIENCY, (char*) NULL, 0.94f);
  }

  int J;
  for (J = 0; J < CarGearNbr; J++)
    if (TDriver::UseBrakeLimit)
      oShift[J] = RevsLimiter * 0.90; //0.87;
	else
      oShift[J] = RevsLimiter * 0.974;

  for (J = 1; J < oLastGear; J++)
  {
      double Rpm = Tickover;
      double RpmNext = Tickover;
      double Tq = 0.0;
      double TqNext = 0.0;
      double GearRatioAct;
      double GearRatioNext;

      while (Rpm <= RevsLimiter)
	  {
		for (I = 0; I < IMax; I++)
		{
			if (Rpm < DataPoints[I].rads)
			{
				Tq = (Rpm * DataPoints[I].a
				  + DataPoints[I].b) * oGearEff[J-1];
				break;
			}
		}

        GearRatioAct = CarGearRatio[J + CarGearOffset];
        GearRatioNext = CarGearRatio[J + 1 + CarGearOffset];
        RpmNext = Rpm * GearRatioNext / GearRatioAct;
		for (I = 0; I < IMax; I++)
		{
			if (RpmNext < DataPoints[I].rads)
			{
				TqNext = (RpmNext * DataPoints[I].a
				  + DataPoints[I].b) * GearRatioNext
				  / GearRatioAct * oGearEff[J];
				break;
			}
		}

        if ((TqNext > Tq ) && (Rpm*RpmFactor > 2000))
		{
		  oShift[J] = Rpm * 0.98;
		  break;
		}
 	    Rpm += 1;
	  }

  }
  
  //for (J = 1; J < oLastGear; J++)
  //  GfOut("#%d: Rpm: %g(%g)\n",J,oShift[J]*RpmFactor,oShift[J]);

  free(DataPoints);
  free(Edesc);
  //GfOut("#<<< TDriver::InitShiftLevels()\n");
}
//==========================================================================*

//==========================================================================*
// EcoShift to reduce fuel consuption
//--------------------------------------------------------------------------*
bool TDriver::EcoShift()
{
  bool ShiftUp = false;

  if (CarRpm > NextRpm)
  {
    oShiftCounter++;
	if (oShiftCounter > 100)
	{
      oShiftCounter = 0;
      ShiftUp = true;
	}
  }
  else
    oShiftCounter = 0;

  return ShiftUp;
}
//==========================================================================*

//==========================================================================*
// S�GCuASL ;D
// = Simplified sequential gear controller using adaptive shift levels
//--------------------------------------------------------------------------*
void TDriver::GearTronic()
{
  if (IsTickover)
  {
    oGear = 1;
  }
  else
  {
    if((UsedGear < oLastGear)
	  && (EcoShift() || (GearRatio() * CarSpeedLong / oWheelRadius > NextRpm)))
	{
      oUnstucking = false;
      TreadClutch;
	  oGear = NextGear;
	}
    else if(UsedGear > 1)
	{
      double PrevRpm =
  	    oShift[UsedGear-1] * oShiftMargin
	    * GearRatio() / PrevGearRatio();

      if(GearRatio() * CarSpeedLong / oWheelRadius < PrevRpm)
	  {
	    TreadClutch;
	    oGear = PrevGear;
	  }
	}
  }
}
//==========================================================================*

//==========================================================================*
// Get info to point
//--------------------------------------------------------------------------*
void TDriver::GetLanePoint(int Path, double Pos, TLanePoint& LanePoint)
{
  if (oStrategy->oPit != NULL 
	&& oStrategy->oPit->HasPits()
	&& !oStrategy->oWasInPit
	&& oStrategy->GoToPit() && oStrategy->oPit->oPitLane[Path].ContainsPos(Pos))
  {
    //GfOut("#+");
    oStrategy->oPit->oPitLane[Path].GetLanePoint(Pos, LanePoint);
	oLookScale = 0.05;
	oOmegaScale = 0.2;
	oLookBase = Param.Fix.oLength / 4;
	oOmegaBase = Param.Fix.oLength / 2;
	oGoToPit = true;
  }
  else if (oStrategy->oPit != NULL 
	&& oStrategy->oPit->HasPits()
	&& oStrategy->oWasInPit
	&& oStrategy->oPit->oPitLane[Path].ContainsPos(Pos))
  {
    //GfOut("#-");
    oStrategy->oPit->oPitLane[Path].GetLanePoint(Pos, LanePoint);
	oLookScale = 0.02;
	oOmegaScale = 0.2;
	oLookBase = Param.Fix.oLength / 10;
	oOmegaBase = Param.Fix.oLength / 2;
	oGoToPit = true;
  }
  else
  {
    //GfOut("#*");
    oRacingLine[Path].GetLanePoint(Pos, LanePoint);
	oLookScale = oLookAheadFactor;
	oOmegaScale = oOmegaAheadFactor;
	oLookBase = oLookAhead;
	oOmegaBase = oOmegaAhead;
	oGoToPit = false;
  }
}
//==========================================================================*

//==========================================================================*
// Get info to position
//--------------------------------------------------------------------------*
void TDriver::GetPosInfo
  (double Pos, TLanePoint& PointInfo, double U, double V )
{
  GetLanePoint(oRL_FREE, Pos, PointInfo);

  if(U != 0.0)
  {
    TLanePoint PointInfoL, PointInfoR;
    GetLanePoint(oRL_LEFT,  Pos, PointInfoL );
    GetLanePoint(oRL_RIGHT, Pos, PointInfoR );

    double T = (1.0 - V) * 0.5;

    InterpolatePointInfo(PointInfoL, PointInfo, U);
    InterpolatePointInfo(PointInfoR, PointInfo, U);

    PointInfo = PointInfoL;

    InterpolatePointInfo(PointInfo, PointInfoR, T);
  }
}
//==========================================================================*

//==========================================================================*
// Get info to position
//--------------------------------------------------------------------------*
void TDriver::GetPosInfo(double Pos, TLanePoint& PointInfo)
{
  GetPosInfo(Pos, PointInfo, oAvoidRange, oAvoidOffset);
}
//==========================================================================*

//==========================================================================*
// Calculate path target
//--------------------------------------------------------------------------*
double TDriver::CalcPathTarget(double Pos, double Offset)
{
  TLanePoint PointInfo, PointInfoL, PointInfoR;

  GetLanePoint(oRL_FREE,Pos,PointInfo);
  GetLanePoint(oRL_LEFT,Pos,PointInfoL);
  GetLanePoint(oRL_RIGHT,Pos,PointInfoR);

  InterpolatePointInfo(PointInfoL,PointInfo,oAvoidRange);
  InterpolatePointInfo(PointInfoR,PointInfo,oAvoidRange);

  double T = (Offset - PointInfoL.Offset) / (PointInfoR.Offset - PointInfoL.Offset);

  return MAX(-1, MIN(T, 1)) * 2 - 1;
}
//==========================================================================*

//==========================================================================*
// Calculate path target
//--------------------------------------------------------------------------*
TVec2d TDriver::CalcPathTarget2(double Pos, double Offset)
{
  TLanePoint PointInfo, PointInfoL, PointInfoR;

  GetLanePoint(oRL_FREE,Pos,PointInfo);
  GetLanePoint(oRL_LEFT,Pos,PointInfoL);
  GetLanePoint(oRL_RIGHT,Pos,PointInfoR);

  InterpolatePointInfo(PointInfoL,PointInfo,oAvoidRange);
  InterpolatePointInfo(PointInfoR,PointInfo,oAvoidRange);

  double T = (Offset - PointInfoL.Offset) / (PointInfoR.Offset - PointInfoL.Offset);

  return TVec2d(MAX(-1, MIN(T, 1)) * 2 - 1, 1);
}
//==========================================================================*

//==========================================================================*
// Calculate path to left and to right
//--------------------------------------------------------------------------*
void TDriver::GetPathToLeftAndRight
  (const PCarElt pCar, double& ToL, double& ToR)
{
  double Pos = pCar->_distFromStartLine;
  double Offset = -pCar->_trkPos.toMiddle;

  TLanePoint PointInfo;
  GetLanePoint(oRL_LEFT,Pos,PointInfo);
  ToL = -(PointInfo.Offset - Offset);
  GetLanePoint(oRL_RIGHT,Pos,PointInfo);
  ToR = PointInfo.Offset - Offset;
}
//==========================================================================*

//==========================================================================*
// Steering angle
//--------------------------------------------------------------------------*
double TDriver::SteerAngle(TLanePoint& AheadPointInfo)
{
  // Look this far ahead.
  double AheadDist = oLookBase + oCurrSpeed * oLookScale;
  if (oDoAvoid)
	AheadDist = 1.5 + oCurrSpeed * 0.04;
  if (oGoToPit)
	AheadDist = 2.0;
  if (AheadDist < oLastAheadDist - 0.05)
    AheadDist = oLastAheadDist - 0.05;
  else if (AheadDist > oLastAheadDist + 0.05)
    AheadDist = oLastAheadDist + 0.05;
  oLastAheadDist = AheadDist;
  double AheadPos = oTrackDesc.CalcPos(oCar, AheadDist);

  // Get info about pts on track.
  GetPosInfo(AheadPos,AheadPointInfo);

  TLanePoint PointInfoOmega;
  double AheadOmega = oOmegaBase + oCurrSpeed * oOmegaScale;
  double AheadOmegaPos = oTrackDesc.CalcPos(oCar, AheadOmega);
  GetPosInfo(AheadOmegaPos,PointInfoOmega);

  // Work out basic steering angle.
  double Angle = AheadPointInfo.Angle - CarYaw;
  DOUBLE_NORM_PI_PI(Angle);

  if (oCurrSpeed < SLOWSPEED)
    return Angle;

  double Delta = oLanePoint.Offset + CarToMiddle;

  // Control rotational velocity.
  double AvgK = (oLanePoint.Crv + PointInfoOmega.Crv) / 2;
  double Omega = CarSpeedLong * AvgK;
  double O2 = (AheadPointInfo.Crv - oLanePoint.Crv) * oCurrSpeed / AheadDist;

  Angle += 0.08 * (Omega - CarYawRate);
  Angle += O2 * 0.08;
  Angle += AvgK * oScaleSteer;

  // control offset from path.
  oPIDCLine.oP = 1.0;
  oPIDCLine.oD = 10;
  Angle -= 0.15 * atan(oPIDCLine.Sample(Delta));

  return Angle;
}
//==========================================================================*

//==========================================================================*
// Unstuck Steering angle
//--------------------------------------------------------------------------*
double TDriver::UnstuckSteerAngle
  (TLanePoint& PointInfo, TLanePoint& AheadPointInfo)
{
  // Get current pos on track.
  double Pos = oTrackDesc.CalcPos(oCar);

  // Look this far ahead.
  double AheadDist = 3.0;
  double AheadPos = oTrackDesc.CalcPos(oCar, AheadDist);

  // Get info about pts on track.
  GetPosInfo(Pos,PointInfo);
  GetPosInfo(AheadPos,AheadPointInfo);

  // Work out basic steering angle.
  double Angle = AheadPointInfo.Angle - CarYaw;
  if (CarToMiddle < 0)
	  Angle += 0.5 * fabs(CarToMiddle/(2*oTrack->width));
  else
    Angle -= 0.5 * fabs(CarToMiddle/(2*oTrack->width));
  DOUBLE_NORM_PI_PI(Angle);
  return Angle;
}
//==========================================================================*

//==========================================================================*
// Control brake press
//--------------------------------------------------------------------------*
void TDriver::BrakingForceRegulator()
{
  double Diff = oCurrSpeed - oTargetSpeed;

  if (Diff > 0.0)
  {
    if (Diff > oBrakeDiffInitial)
	{
	  oAccel = 0;
      oBrake = MIN(oBrakeForceMax,Diff * Diff / oBrakeScale);
	}
	else
	{
	  if (oTargetSpeed > 1)
	  {
	    oAccel = MIN(oAccel, 0.25);
		oBrake = 0.0;
	  }
	  else
	  {
		oAccel = 0;
		oBrake = 0.1;
	  }
	}
  }

  if ((oLastBrake > 0)
	&& (oBrake > 0)
	&& (Diff < 2))
  {
	oBrake = 0;
	oAccel = 0.06;
  }

  oBrake *= (1 + MAX(0.0,(oCurrSpeed - 40.0)/40.0));

  oLastBrake = oBrake;
  oLastTargetSpeed = 0;
}
//==========================================================================*

//==========================================================================*
// Control brake press while avoiding
//--------------------------------------------------------------------------*
void TDriver::BrakingForceRegulatorAvoid()
{
  double Diff = oCurrSpeed - oTargetSpeed;

  if (Diff > 0.0)
  {
    if (Diff > 1.0)
	{
	  oAccel = 0;
      oBrake = MIN(oBrakeForceMax,Diff * Diff / oBrakeScale);
	}
	else
	{
	  if (oTargetSpeed > 1)
	  {
		oAccel = MIN(oAccel, 0.25);
        oBrake = 0.0;
	  }
	  else
	  {
		oAccel = 0;
		oBrake = 0.1;
	  }
	}
  }

  oBrake *= (1 + MAX(0.0,(oCurrSpeed - 40.0)/40.0));

  if (oMinDistLong < 10.0)
	oBrake *= 1.1;

  oLastTargetSpeed = 0;
}
//==========================================================================*

//==========================================================================*
// Control brake press in traffic
//--------------------------------------------------------------------------*
void TDriver::BrakingForceRegulatorTraffic()
{
  double Err = 0.0;
  if(oLastBrake && oLastTargetSpeed)
  {
    Err = oCurrSpeed - oLastTargetSpeed;
    oBrakeCoeff[oLastBrakeCoefIndex] += Err * 0.001;
	oLastBrake = 0;
	oLastTargetSpeed = 0;
  }

  double Diff = oCurrSpeed - oTargetSpeed;

  if (Diff > 0.0)
  {
	int	B = (int) MIN(NBR_BRAKECOEFF,(floor(oCurrSpeed/2)));
	oAccel = 0;
	oBrake = MAX(0, MIN(oBrakeCoeff[B] * Diff * Diff, oBrakeForceMax));
	oLastBrakeCoefIndex = B;
	oLastBrake = oBrake;
	oLastTargetSpeed = 0;

	if ((oBrake > 0) && (oBrake < oBrakeForceMax))
	{
	  if (oTargetSpeed > 0)
		oLastTargetSpeed = oTargetSpeed;
	}
  }

  oBrake *= (1 + MAX(0.0,(oCurrSpeed - 40.0)/40.0));

  if (oMinDistLong < 10.0)
	oBrake *= 1.1;

}
//==========================================================================*

//==========================================================================*
// Evaluate collision flags
//--------------------------------------------------------------------------*
void TDriver::EvaluateCollisionFlags(
  int I,
  TCollision::TCollInfo& Coll,
  double Crv,
  double& MinCatchTime,
  double& MinCatchAccTime,
  double& MinVCatTime,
  bool& IsLapper)
{
  TOpponent::TInfo& OppInfo =                    // Information about opponent
    oOpponents[I].Info();                        // collected at classification

  PCarElt OppCar = oOpponents[I].Car();          // TORCS data of opponents car

  Coll.Flags |= OppInfo.Flags;                   // subsume the collision flags
  for (int I = 0; I < MAXBLOCKED; I++)
    Coll.Blocked[I] |= OppInfo.Blocked[I];       // subsume the blocked flags

  if (OppInfo.GotFlags(F_FRONT))                 // Is opponent in front of us
  {
    if (oMinDistLong > OppInfo.CarDistLong)
	  oMinDistLong = OppInfo.CarDistLong;

	if (OppInfo.GotFlags(F_COLLIDE)
	  && (OppInfo.CatchDecel > 12.5 * CarFriction))
	  Coll.TargetSpeed = MIN(Coll.TargetSpeed, OppInfo.CatchSpeed * 0.9);

	if (OppInfo.Flags & (F_COLLIDE | F_CATCHING))
	  MinCatchTime = MIN(MinCatchTime, OppInfo.CatchTime);

	if (OppInfo.Flags & F_CATCHING_ACC)
	  MinCatchAccTime = MIN(MinCatchAccTime, OppInfo.CatchAccTime);

	if (OppInfo.State.CarDiffVelLong < 0)
	{
	  double VCatTime =
		-(OppInfo.State.CarDistLong - OppInfo.State.MinDistLong) / OppInfo.State.CarDiffVelLong;

	  if (VCatTime > 0)
	    MinVCatTime = MIN(MinVCatTime, VCatTime);
	}

	bool IgnoreTeamMate;
	if (oTeamEnabled) 
	{
	  IgnoreTeamMate =
	    OppInfo.GotFlags(F_TEAMMATE)
	    && (CarLaps < OppCar->_laps
	    || CarDamage + 1000 >= OppInfo.TeamMateDamage);
	}
	else
	{
	  IgnoreTeamMate = false;
	}

	OppInfo.AvoidLatchTime = MAX(0, OppInfo.AvoidLatchTime - oSituation->deltaTime);

	double MaxSpdCrv = Param.Fix.CalcMaxSpeedCrv();
	double ColTime = fabs(Crv) > MaxSpdCrv ? 1.0 : 1.2;
	double CatTime = fabs(Crv) > MaxSpdCrv ? 1.0 : 3.0;
	double CacTime = fabs(Crv) > MaxSpdCrv ? 1.0 : 3.0;
	bool Catching =
	  (OppInfo.CatchTime < ColTime && OppInfo.GotFlags(F_COLLIDE))
	  || (OppInfo.CatchTime < CatTime && OppInfo.GotFlags(F_CATCHING))
	  || (OppInfo.CatchAccTime < CacTime && OppInfo.GotFlags(F_CATCHING_ACC));

	if (!IgnoreTeamMate &&
	  (OppInfo.AvoidLatchTime > 0 || Catching || OppInfo.GotFlags(F_DANGEROUS)))
	{
	  double ToL, ToR;

	  GetPathToLeftAndRight(OppCar, ToL, ToR);
	  ToL += OppInfo.State.TrackVelLat * OppInfo.CatchTime;
	  ToR -= OppInfo.State.TrackVelLat * OppInfo.CatchTime;
	  bool SpaceL = ToL > OppInfo.State.MinDistLat + 0.75;// + 0.25;
	  bool SpaceR = ToR > OppInfo.State.MinDistLat + 0.75;// + 0.25;
	  bool AvoidL = OppInfo.State.CarDistLat < 0 && SpaceR;
	  bool AvoidR = OppInfo.State.CarDistLat > 0 && SpaceL;

	  if (Catching)
	    OppInfo.AvoidLatchTime = fabs(Crv) < MaxSpdCrv ? 0.5 : 0.1;

	  if (fabs(Crv) < MaxSpdCrv)
	  {
	    if (!AvoidL && !AvoidR)
		{
		  AvoidL = !SpaceL && SpaceR;
		  AvoidR = !SpaceR && SpaceL;
		}
	  }

  	  if (AvoidL)
		Coll.OppsAhead |= F_LEFT;

	  if (AvoidR)
		Coll.OppsAhead |= F_RIGHT;

	  if (AvoidL)
		Coll.MinLDist = MIN(OppInfo.State.CarAvgVelLong, Coll.MinLDist);

  	  if (AvoidR)
		Coll.MinRDist = MIN(OppInfo.State.CarAvgVelLong, Coll.MinRDist);
	}
  }

  if (OppInfo.GotFlags(F_AT_SIDE))               // Is Opponent at side of us
  {
	Coll.OppsAtSide |= OppInfo.State.CarDistLat < 0 ? F_LEFT : F_RIGHT;
	if (OppInfo.State.CarDistLat < 0)
	  Coll.MinLSideDist = MIN(Coll.MinLSideDist,
	    -OppInfo.State.CarDistLat - OppInfo.State.MinDistLat);
	else
	  Coll.MinRSideDist = MIN(Coll.MinRSideDist,
	    OppInfo.State.CarDistLat - OppInfo.State.MinDistLat);
  }

  if (oTeamEnabled) 
  {
    oTreatTeamMateAsLapper =
	  OppInfo.GotFlags(F_TEAMMATE | F_REAR)
	  && OppInfo.State.RelPos > -25
	  && CarLaps == OppCar->_laps
	  && CarDamage > OppInfo.TeamMateDamage + 1000;
  }
  else
  {
	oTreatTeamMateAsLapper = false;
  }

  if (oStayTogether > 50
	&& OppInfo.GotFlags(F_TEAMMATE | F_REAR)
	&& OppInfo.State.RelPos < -35
	&& OppInfo.State.RelPos > -oStayTogether
	&& CarDamage + 1000 > OppInfo.TeamMateDamage)
  {
    if (oTeamEnabled) 
  	  IsLapper = true;
  }

  if (OppInfo.GotFlags(F_LAPPER) || oTreatTeamMateAsLapper)
  {
	Coll.LappersBehind |= OppInfo.State.CarDistLat < 0 ? F_LEFT : F_RIGHT;
	IsLapper = true;
  }
}
//==========================================================================*

//==========================================================================*
// Get next curvature to opponent
//--------------------------------------------------------------------------*
void TDriver::NextCurvature(TCollision::TCollInfo& Coll, PtCarElt Car)
{
  int OppPosIndex = oTrackDesc.IndexFromPos(oTrackDesc.CalcPos(Car));
  Coll.NextSide =
	(oRacingLine[oRL_FREE].PathPoints(OppPosIndex).Crv < 0) ? -1 : 0;
}
//==========================================================================*

//==========================================================================*
// Target Reached
//--------------------------------------------------------------------------*
bool TDriver::TargetReached(double Target, double AvoidTarget)
{
  if(((oAvoidRange != 0.0)
	&& (Target == 0))
	|| ((AvoidTarget != oAvoidRange)
	&& (Target != 0)))
    return false;
  else
    return true;
}
//==========================================================================*

//==========================================================================*
// Runaround most urgent obstacle
//--------------------------------------------------------------------------*
void TDriver::Runaround(double Scale, double Target, bool DoAvoid)
{
  // Scale limits of change of lateral movement (accellerations/velocities)
  double RangeAccMax = 0.0005 * Scale;           // Range accelleration and
  double RangeVelMax = 0.005 * Scale;            // velocity per sim.step
  double OffsetAccMax = 0.00015 * Scale;         // Offset accelleration and
  double OffsetVelMax = 0.1 * Scale;             // velocity per sim.step

  double AvoidTarget = 0;                        // Assume come back to RL
  if (DoAvoid)                                   // But if needed
    AvoidTarget = 2.0;                           //   avoid to side

  // Adjusting allowed range of offset ...
  if (!TargetReached(Target,AvoidTarget))        // Target Range reached?
  {
	AvoidTarget = (Target == 0) ? 0 : 1;         // Direction to move to
    double LatAccel = oAvoidRange > AvoidTarget  // Xceleration of changing
      ? RangeAccMax : -RangeAccMax;              // the range
    double Dist =                                // Remaining distance to
	  oAvoidRange - AvoidTarget;                 //   target range

	if (fabs(Dist) < 0.0005)                     // If close to target
	{
      oAvoidRangeDelta = 0.0;                    //   don't change any longer
	}
	else
	{
	  if (fabs(Dist) <=                          // Check wether to decellerate
		XX2Y(oAvoidRangeDelta,RangeAccMax))      // enlargement/contracting
	  {
		LatAccel = -XX2Y(oAvoidRangeDelta,Dist); // Decelerate
	  }
	  oAvoidRangeDelta += LatAccel;              // Accellerate or Decellerate
      oAvoidRangeDelta =                         // Restrict to limits
	    MINMAX(RangeVelMax,oAvoidRangeDelta);
	}
  }
  else                                           // If target is reached
	oAvoidRangeDelta = 0;                        //   stop changing

  double OldAvoidRange = oAvoidRange;            // Save old range
  oAvoidRange -= oAvoidRangeDelta;               // Set new range

  if ((oAvoidRange > 0.9995)                     // If close to Max
	&& (oAvoidRangeDelta < 0))                   // fix range and change
  {
    oAvoidRange = 1.0;                           // Max reached
	oAvoidRangeDelta = 0.0;                      // Stop changing
  }
  else if((oAvoidRange <= 0.0005)                // If close to Min
	&& (oAvoidRangeDelta > 0))                   // fix range and change
  {
	oAvoidRange = 0.0;                           // Min reached
	oAvoidRangeDelta = 0.0;                      // Stop changing
  }
  else if (FixRange)                             // If close to tagret
  {
	oAvoidRange = AvoidTarget;                   // Target reached
	oAvoidRangeDelta = 0.0;                      // Stop changing
  }

  // Adjusting offset ...
  if (Target != oAvoidOffset)                    // Target Offset reached?
  {
 	double LatAccel =                            // Xcelleration of lateral movement
	  OffsetAccMax / MAX(0.2, oAvoidRange);
	LatAccel = Target > oAvoidOffset             // Accelleate or Decellerate?
	  ? LatAccel : -LatAccel;
	double Dist = Target - oAvoidOffset;         // Distance to target offset
	if ((Dist * oAvoidOffsetDelta > 0)           // Check wether to decellerate
      && (fabs(Dist) <=                          //   lateral movement
	    XX2Y(oAvoidOffsetDelta,OffsetAccMax)))
	{
	  LatAccel = -XX2Y(oAvoidOffsetDelta,Dist);  // Decellerate
	}
	LatAccel = MINMAX(OffsetAccMax,LatAccel);    // Restrict to limits
	oAvoidOffsetDelta += LatAccel;               // Accellerate or decellerate
    oAvoidOffsetDelta =                          // Restrict to limits
	    MINMAX(OffsetVelMax,oAvoidOffsetDelta);
  }
  else                                           // If target offset reached
    oAvoidOffsetDelta = 0;                       //   stop changing

  double OldAvoidOffset = oAvoidOffset;          // Save old offset
    oAvoidOffset += oAvoidOffsetDelta;           // Set new offset

  if ((oAvoidOffset < -0.99)                     // If close to Min
	&& (Target < 0))                             // of target dir
  {
	oAvoidOffset = -1;                           // Min reached
	oAvoidOffsetDelta = 0.0;                     // Stop changing
  }
  else if ((oAvoidOffset > 0.99)                 // If close to Max
	&& (Target > 0))                             // of target dir
  {
	oAvoidOffset = 1;                            // Max reached
	oAvoidOffsetDelta = 0.0;                     // Stop changing
  }
  else if (FixOffset)                            // If close to target
  {
	oAvoidOffset = Target;                       // Target reached
 	oAvoidOffsetDelta = 0.0;                     // Stop changing
  }
}
//==========================================================================*

//==========================================================================*
// General avoidance control
//--------------------------------------------------------------------------*
void TDriver::AvoidOtherCars(double K, bool& IsClose, bool& IsLapper)
{
  const TOpponent::TState& MyState =             // Get my own state
	oOpponents[oOwnOppIdx].Info().State;

  int I;
  for (I = 0; I < oNbrCars; I++)                 // All opponents
	for (int J = 0; J < MAXBLOCKED; J++)		 //   all lanes
	  oOpponents[I].Info().Blocked[J] = false;

  for (I = 0; I < oNbrCars; I++)                 // Get info about imminent
  {                                              //   collisions from
      oOpponents[I].Classify(                    //   all opponents depending
	  oCar,                                      //   on TORCS data of own car
	  MyState,                                   //   my own state,
	  /*oStrategy->OutOfPitlane(),*/                 //   In pitlane?
	  oMaxAccel.Estimate(CarSpeedLong));         //   Estimate of accelleration
  }

  // Place to subsume the collision flags from all opponents
  TCollision::TCollInfo Coll;

  double MinCatchTime = FLT_MAX;                 // Initialize limits
  double MinCatchAccTime = FLT_MAX;
  double MinVCatTime = FLT_MAX;

  IsLapper = false;                              // Initialize flags

  TLanePoint PointInfo;                          // Infos to point
  GetLanePoint                                   //   own position
	(oRL_FREE,DistanceFromStartLine,PointInfo);

  for (I = 0; I < oNbrCars; I++)                 // Loop all opponents
  {
    EvaluateCollisionFlags(                      // Evaluate collision flags
      I, Coll, K,
	  MinCatchTime,
	  MinCatchAccTime,
	  MinVCatTime,
	  IsLapper);
  }

  if (((Coll.Flags & F_PREVIEWSLOW) != 0)
    && (DistanceRaced > 1000))
  {
    Coll.AvoidSide = 0;
	int I;
	int K = 0;
	if (Coll.Blocked[0] || Coll.Blocked[1] || Coll.Blocked[2])
	{
      for (I = MAXBLOCKED - 1; I > 1; I--)
      {
		K++;
	    if (Coll.Blocked[I] || Coll.Blocked[I-1] || Coll.Blocked[I-2])
		  continue;
	    else
	    {
	      Coll.AvoidSide = ((I-1) * 2.0/(MAXBLOCKED - 1.0) - 1);
	      break;
	    }
  	  }
	}
	else
	{
      for (I = 0; I < MAXBLOCKED - 2; I++)
      {
		K++;
	    if (Coll.Blocked[I] || Coll.Blocked[I+1] || Coll.Blocked[I+2])
		  continue;
	    else
	    {
	      Coll.AvoidSide = ((I+1) * 2.0/(MAXBLOCKED - 1.0) - 1);
	      break;
	    }
  	  }
	}

	if (K == MAXBLOCKED - 2)
	{
      oSpeedScale += 0.00025;
      Coll.TargetSpeed = MIN(Coll.TargetSpeed, MAX(0.8,(1 - oSpeedScale)) * oTargetSpeed);
	}
  }

  if ((Coll.Flags & F_PREVIEWSLOW) == 0)
    oSpeedScale = 0.0;

  NextCurvature(Coll,oCar);                      // Find side of next curvature

  oDoAvoid = false;                              // Assume not avoiding

  TCollision RunAround;                          // To runaround we have to decide
  double Target = 0.0;                           // which way to take
  float Ratio = 0.0;

  Target = RunAround.AvoidTo                     // Check which way we should take
    (Coll,oCar,*this,oDoAvoid);                  //   depending on opponents

  if (oStrategy->StartPitEntry(Ratio))           // If entrering pit
  {
	if (!oDoAvoid)                               // If no avoiding needed
	{
      Target = Ratio * PitSide();                // Bring us to the correct
	  oDoAvoid = true;                           // side to make pit stop
	}
  }
  else if (oStrategy->StopPitEntry(Param.Pit.oExitLength)) // If coming back to track
  {
	if (!oDoAvoid)                                // If no avoiding needed
	{
      Target = PitSide();                        // Bring us to the correct
	  oDoAvoid = true;                           // side to make pit stop
	}
  }

  if (oTestLane > 0)
  {
    oDoAvoid = true;                             // side to make pit stop
    Target = oTestLane;
  }
  else if (oTestLane < 0)
  {
    oDoAvoid = true;                             // side to make pit stop
    Target = oTestLane;
  }

  double TargetSpeed =                           // Adjust target speed
	MIN(oTargetSpeed, Coll.TargetSpeed * 0.9);

  // Skilling from Andrew Sumner ...
  oTargetSpeed = CalcSkill(TargetSpeed);
  // ... Skilling from Andrew Sumner

  IsClose = (Coll.Flags & F_CLOSE) != 0;         // Set flag, if opponent is close by

  if (oFlying)                                   // Sorry, but we can't do anything
    return;                                      //   right now, we are flying!

  double HalfWidth = oTrackDesc.Width() / 2;     // Half width of track
  double Scale =                                 // Scale reaction
	oAvoidScale /(oAvoidWidth + HalfWidth);

  Runaround(Scale,Target,oDoAvoid);              // runaround most urgent obstacle
}
//==========================================================================*

//==========================================================================*
// Filter Start
//--------------------------------------------------------------------------*
double TDriver::FilterStart(double Speed)
{
  // Decrease target speed with increasing rank
  if ((!Qualification) && (DistanceRaced < 1000))
  {
    double Offset = 0.01;
    Speed *= MAX(0.6,(1.0 - (oCar->race.pos - 1) * Offset));
  }
  return Speed;
}
//==========================================================================*

//==========================================================================*
// Limit steer speed
//--------------------------------------------------------------------------*
double TDriver::FilterSteerSpeed(double Steer)
{
  if (oCurrSpeed < 20)
	return Steer;

  if (!Qualification)
  {
    const float MaxSteerSpeed = 0.1f;
    double Ratio = fabs(oLastSteer - Steer)/MaxSteerSpeed;

    if (Ratio > 1.0)
	{
	  if (Steer > oLastSteer)
	    Steer = oLastSteer + MaxSteerSpeed;
	  else
	    Steer = oLastSteer - MaxSteerSpeed;
	}

    double Range = MIN(1.0,0.3 + 1250.0/(oCurrSpeed*oCurrSpeed));

	if (Steer > 0)
      Steer = MIN(Range,Steer);
	else
      Steer = MAX(-Range,Steer);

  }
  return Steer;
}
//==========================================================================*

//==========================================================================*
// ABS-Filter
//--------------------------------------------------------------------------*
double TDriver::FilterABS(double Brake)
{
  if(CarSpeedLong < 10)
	return Brake;

  double Slip = 0.0;

  for (int I = 0; I < 4; I++)
	Slip += WheelSpinVel(I) * WheelRad(I);

  Slip = 4.0 * CarSpeedLong / Slip;

  if (Slip > oAbsDelta)
  {
	Brake *= oAbsScale;
  }

  return Brake;
}
//==========================================================================*

//==========================================================================*
// Filter Brake while drifting
//--------------------------------------------------------------------------*
double TDriver::FilterBrake(double Brake)
{
  // If braking, decrease braking force while drifting
  if((CarSpeedLong > SLOWSPEED) && (Brake > 0.0))
  {
    double DriftAngle = MAX(MIN(oDriftAngle * 2, PI),-PI);
    Brake *= MAX(0.1, cos(DriftAngle));
  }
  return Brake;
}
//==========================================================================*

//==========================================================================*
// Brake-Skill-Filter
//--------------------------------------------------------------------------*
double TDriver::FilterSkillBrake(double Brake)
{
  //Brake *= oBrakeAdjustPerc;
  return Brake;
}
//==========================================================================*

//==========================================================================*
// Reduces the brake value such that it fits the speed (more downforce -> more braking).
//--------------------------------------------------------------------------*
double TDriver::FilterBrakeSpeed(double Brake)
{
	float WF = Param.Tmp.oMass * G / Param.Fix.oCa;
	float F2 = WF + 10000;
	float F1 = WF + oCurrSpeed * oCurrSpeed;
	return Brake * F1/F2;
}
//==========================================================================*

//==========================================================================*
// Filter acceleration Control
//--------------------------------------------------------------------------*
double TDriver::FilterAccel(double Accel)
{
  if (DistanceRaced < 50)                        // Not at start
	return Accel;

  if(fabs(CarSpeedLong) < 0.001)                 // Only if driving faster
	return Accel;

  //if (fabs(oDriftAngle) > 0.3)
  //  return MIN(Accel, oLastAccel + 0.01);

  return MIN(Accel,oLastAccel + 0.05);
}
//==========================================================================*


//==========================================================================*
// Filter Traction Control
//--------------------------------------------------------------------------*
double TDriver::FilterTCL(double Accel)
{
  if (DistanceRaced < 50)                        // Not at start
	return Accel;

  if(fabs(CarSpeedLong) < 0.001)                 // Only if driving faster
	return Accel;

  double Spin = 0;                               // Initialize spin
  double Wr = 0;                                 // wheel radius
  int Count = 0;                                 // count impellers

  if(HasDriveTrainFront)                         // If front wheels
  {                                              //   are impellers
	Spin += WheelSpinVel(FRNT_LFT);              // Summarize spin
	Spin += WheelSpinVel(FRNT_RGT);              // of both wheels
	Wr += WheelRad(FRNT_LFT)+WheelRad(FRNT_RGT); // measure radius
	Count += 2;                                  // and count both
  }

  if(HasDriveTrainRear)                          // If rear wheels
  {                                              //   are impellers
	Spin += WheelSpinVel(REAR_LFT);              // Summarize spin
	Spin += WheelSpinVel(REAR_RGT);              // of both wheels
	Wr += WheelRad(REAR_LFT)+WheelRad(REAR_RGT); // measure radius
	Count += 2;                                  // and count both
  }
  Spin /= Count;                                 // Calculate spin
  Wr /= Count;                                   // and radius

  double Slip = Spin * Wr - CarSpeedLong;        // Calculate slip
  if (Slip > oTclSlip)                           // Decrease accel if needed
  {
	float MinAccel = 0.2 * Accel;
	Accel -= MIN(Accel, (Slip - oTclSlip)/oTclRange);
	Accel = MAX(MinAccel,Accel);
  }
  return Accel;
}
//==========================================================================*

//==========================================================================*
// Filter Let Pass
//--------------------------------------------------------------------------*
double TDriver::FilterLetPass(double Accel)
{
  // If we should let an other bot pass, decrease accelleration
  if (oLetPass)
  {
    if (oTreatTeamMateAsLapper)
      Accel = MIN(Accel, 0.7);
	else
      Accel = MIN(Accel, 0.5);
  }
  return Accel;
}
//==========================================================================*

//==========================================================================*
// Filter Drifting
//--------------------------------------------------------------------------*
double TDriver::FilterDrifting(double Accel)
{
  // Decrease accelleration while drifting
  if((CarSpeedLong > SLOWSPEED) && (fabs(oDriftAngle) > 0.2))
  {
	Accel *= (float)
	  (0.25 + 0.75 * MAX(0.0,cos(oDriftAngle)));
  }
  return Accel;
}
//==========================================================================*

//==========================================================================*
// Filter Track
//--------------------------------------------------------------------------*
double TDriver::FilterTrack(double Accel)
{
  if (DistanceRaced > oStartDistance)            // Except while starting
  {
	if (fabs(oDeltaOffset) > oTolerance)         // Check offset difference
	  Accel *= (float)                           //   Decrease acceleration
	    (MAX(1.0 - (fabs(oDeltaOffset) - oTolerance) * 0.2, 0.4));

    Accel *= oSideReduction;
  }
  return Accel;
}
//==========================================================================*

//==========================================================================*
// Detect obstacles
//--------------------------------------------------------------------------*
bool TDriver::IsStuck()
{
  TV2D Tmp;                                      // Holds Coordinates
  float Diff;                                    // Distance from old point

  if (!oStrategy->OutOfPitlane())
	return false;

  if ((oStuckCounter > 3) && (oStuckCounter < 6))// Less then six ticks
	oCar->_brakeCmd = 1.0;                       //   left? stop driving back
  else
	oCar->_brakeCmd = 0.0;                       //

  if (oStuckCounter > 0)                         // Driving back?
  {                                              //   If so, clear buffer
    oSysFooStuckX->Reset();
    oSysFooStuckY->Reset();
    oStuckCounter--;                             //   decrement counter
    //GfOut("#Driving back! %d\n",oStuckCounter);
    return true;                                 //   and drive
  }

  TV2D MyPos;                                    // Current position
  MyPos.x = CarPubGlobPosX;                      //
  MyPos.y = CarPubGlobPosY;                      //

  // Check motion
  Tmp.x = oSysFooStuckX->Faltung(float(MyPos.x));// X coordinate
  Tmp.y = oSysFooStuckY->Faltung(float(MyPos.y));// Y coordinate

  Diff = Dist(Tmp,MyPos);                        // Distance from old pos
  if (Diff < 0.3)                                // If distance is to small
//  if (Diff < 5.0)                                // If distance is to small
  {                                              //   assume obstacle
	if (oStuckCounter == 0)
	{
      oStuckCounter = -UNSTUCK_COUNTER;          // Set counter
	  //GfOut("#Set! %d\n",oStuckCounter);
	}

	if (oStanding)                               // But if flag is set
	{                                            //   it is planned!
	  //GfOut("#Standing! %d\n",oStuckCounter);
	  oSysFooStuckX->Reset();                    // Clear buffers
	  oSysFooStuckY->Reset();                    //   of motion survey
      return false;                              //   and signal ok
	}
    else if (oUnstucking)                        // But if flag is set
	{                                            //   it is possible
      if (oStuckCounter < 0)                     //
	  {
        oSysFooStuckX->Reset();
        oSysFooStuckY->Reset();
        oStuckCounter++;                         // Increment counter
		if (oStuckCounter == 0)
	    {
          oStuckCounter = UNSTUCK_COUNTER;       // Set counter
  	      //GfOut("#Stuck1! %d\n",oStuckCounter);
          return true;                           // give signal stuck
	    }
	    //GfOut("#Unstucking! %d\n",oStuckCounter);
        return false;                            //   and signal ok
	  }
	  else                                       // still stuck
	  {
        oStuckCounter = UNSTUCK_COUNTER;         // Set counter
  	    //GfOut("#Stuck1! %d\n",oStuckCounter);
        return true;                             // give signal stuck
	  }
	}
    else                                         // if not
	{
      oStuckCounter = UNSTUCK_COUNTER;           // Set counter
	  //GfOut("#Stuck! %d\n",oStuckCounter);
      return true;                               // give signal stuck
	}
  }
  else
    oStanding = false;                           // Reset flag

  return false;                                  // No obstacle
}
//==========================================================================*

//==========================================================================*
// Unstuck
//--------------------------------------------------------------------------*
void TDriver::Unstuck()
{
  TLanePoint AheadPointInfo;
  oAngle = UnstuckSteerAngle(oLanePoint,AheadPointInfo) / 100.0;

  CarSteerCmd = (float) -(oAngle/CarSteerLock);  // Steering
  CarGearCmd = -1;                               // Reverse gear
  CarBrakeCmd = 0.0;                             // Unlock brake
  CarAccelCmd = 1.0;                             // Open the throttle
  CarClutchCmd = 0.0;                            // Release clutch
  oUnstucking = true;                            // Set flag
}
//==========================================================================*

//==========================================================================*
// Skilling
//--------------------------------------------------------------------------*
double TDriver::CalcSkill(double TargetSpeed)
{
  if (oSkilling
	&& (RM_TYPE_PRACTICE != oSituation->_raceType)
	&& oStrategy->OutOfPitlane())
  {
    if ((oSkillAdjustTimer == -1.0) 
		|| (TDriver::CurrSimTime - oSkillAdjustTimer > oSkillAdjustLimit))
    {
      double Rand1 = (double) getRandom() / 65536.0;
      double Rand2 = (double) getRandom() / 65536.0;
      double Rand3 = (double) getRandom() / 65536.0;

	  //double Rand1 = 0.0;
      //double Rand2 = 0.0;
      //double Rand3 = 0.0;

      // acceleration to use in current time limit
      oDecelAdjustTarget = (oSkill/4 * Rand1);

      // brake to use 
      oBrakeAdjustTarget = MAX(0.7, 1.0 - MAX(0.0, oSkill/10 * (Rand2 - 0.7)));

      // how long this skill mode to last for
      oSkillAdjustLimit = 5.0 + Rand3 * 50.0;
      oSkillAdjustTimer = TDriver::CurrSimTime;
 
      if (oDecelAdjustPerc < oDecelAdjustTarget)
        oDecelAdjustPerc += 
		  MIN(oSituation->deltaTime*4, oDecelAdjustTarget - oDecelAdjustPerc);
      else
        oDecelAdjustPerc -= 
		  MIN(oSituation->deltaTime*4, oDecelAdjustPerc - oDecelAdjustTarget);

      if (oBrakeAdjustPerc < oBrakeAdjustTarget)
        oBrakeAdjustPerc += 
		  MIN(oSituation->deltaTime*2, oBrakeAdjustTarget - oBrakeAdjustPerc);
      else
        oBrakeAdjustPerc -= 
		  MIN(oSituation->deltaTime*2, oBrakeAdjustPerc - oBrakeAdjustTarget);
    }
	//GfOut("TS: %g DAP: %g (%g)",TargetSpeed,oDecelAdjustPerc,(1 - oDecelAdjustPerc/10));
    TargetSpeed *= (1 - oSkill/oSkillMax * oDecelAdjustPerc/20);
	//GfOut("TS: %g\n",TargetSpeed);
  }
  //GfOut("%g %g\n",oDecelAdjustPerc,(1 - oDecelAdjustPerc/10));
  return TargetSpeed;
}
//==========================================================================*

//==========================================================================*
// Set scaling factor for avoiding racinglines
//--------------------------------------------------------------------------*
void TDriver::ScaleSide(float FactorMu, float FactorBrake)
{
  oSideScaleMu = FactorMu;
  oSideScaleBrake = FactorBrake;
}
//==========================================================================*

//==========================================================================*
// Set additional border to outer side
//--------------------------------------------------------------------------*
void TDriver::SideBorderOuter(float Factor)
{
  oSideBorderOuter = Factor;
}
//==========================================================================*

//==========================================================================*
// Calculate the crv
//--------------------------------------------------------------------------*
double TDriver::CalcCrv(double Crv)
{
  return (this->*CalcCrvFoo)(Crv);
}
//==========================================================================*

//==========================================================================*
// Calculate the hairpin
//--------------------------------------------------------------------------*
double TDriver::CalcHairpin(double Crv)
{
  return (this->*CalcHairpinFoo)(Crv);
}
//==========================================================================*

//==========================================================================*
// simplix
//--------------------------------------------------------------------------*
double TDriver::CalcCrv_simplix(double Crv)
{
  return MAX(0.75,MIN(3.0,600000.0 * Crv * Crv * Crv));
}
//==========================================================================*

//==========================================================================*
// simplix_TRB1
//--------------------------------------------------------------------------*
double TDriver::CalcCrv_simplix_TRB1(double Crv)
{
  return MAX(0.75,MIN(3.0,350000.0 * Crv * Crv * Crv));
  //return MAX(0.75,MIN(3.0,600000.0 * Crv * Crv * Crv));
}
//==========================================================================*

//==========================================================================*
// simplix_sc
//--------------------------------------------------------------------------*
double TDriver::CalcCrv_simplix_SC(double Crv)
{
  return MAX(0.75,MIN(3.0,75000.0 * Crv * Crv * Crv));
  //return MAX(0.75,MIN(3.0,600000.0 * Crv * Crv * Crv));
}
//==========================================================================*

//==========================================================================*
// simplix_36GP
//--------------------------------------------------------------------------*
double TDriver::CalcCrv_simplix_36GP(double Crv)
{
  if (Qualification)
    return MAX(1.00,MIN(2.8,5200.0 * Crv * Crv)); 
  else
    return MAX(1.00,MIN(3.2,7500.0 * Crv * Crv)); 
}
//==========================================================================*

//==========================================================================*
// simplix
//--------------------------------------------------------------------------*
double TDriver::CalcHairpin_simplix(double Crv)
{
  return MAX(0.75,MIN(5.0,600000.0 * Crv * Crv * Crv));
}
//==========================================================================*

//==========================================================================*
// simplix_TRB1
//--------------------------------------------------------------------------*
double TDriver::CalcHairpin_simplix_TRB1(double Crv)
{
  return MAX(0.75,MIN(5.0,300000.0 * Crv * Crv * Crv));
  //return MAX(0.75,MIN(5.0,600000.0 * Crv * Crv * Crv));
}
//==========================================================================*

//==========================================================================*
// simplix_sc
//--------------------------------------------------------------------------*
double TDriver::CalcHairpin_simplix_SC(double Crv)
{
  return MAX(1.0,MIN(2.5,140000.0 * Crv * Crv * Crv));
  //return MAX(0.75,MIN(5.0,600000.0 * Crv * Crv * Crv));
}
//==========================================================================*

//==========================================================================*
// simplix_36GP
//--------------------------------------------------------------------------*
double TDriver::CalcHairpin_simplix_36GP(double Crv)
{
  //GfOut("HP\n");
  if (Qualification)
    return MAX(1.00,MIN(3.0,2300.0  * Crv * Crv)); 
  else
    return MAX(1.00,MIN(3.2,6000.0  * Crv * Crv));
}
//==========================================================================*

//--------------------------------------------------------------------------*
// end of file unitdriver.cpp
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
