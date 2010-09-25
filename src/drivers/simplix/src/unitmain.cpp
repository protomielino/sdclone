//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
// unitmain.cpp
//--------------------------------------------------------------------------*
// TORCS: "The Open Racing Car Simulator"
// A robot for Speed Dreams-Version 1.4.0/2.0
//--------------------------------------------------------------------------*
// Interface to TORCS
// 
// File         : unitmain.cpp 
// Created      : 2008.01.27
// Last changed : 2010.09.25
// Copyright    : � 2007-2010 Wolf-Dieter Beelitz
// eMail        : wdb@wdbee.de
// Version      : 2.00.001 
//--------------------------------------------------------------------------*
// V2.00.01 (Speed Dreams - Career mode):
// Uses new Speed Dreams Interfaces and was extended to use career mode
// - Still work in progress
//--------------------------------------------------------------------------*
// V2.00 (Speed Dreams):
// Uses new Speed Dreams Interfaces
//--------------------------------------------------------------------------*
// V1.10:
// Features of the advanced TORCS Interface:
// Initialization runs once only, see "simplix(tModInfo *ModInfo)"
// Allways gives back the names of drivers as defined in teams xml file!
// Checks and handles pitsharing state enabled/disabled for endurance races.
// 
// Eigenschaften des erweiterten TORCS Interfaces:
// Die Initialisierung wird nur einmal ausgef�hrt, siehe dazu 
// "simplix(tModInfo *ModInfo)"
// Die DLL gibt die Namen der Fahrer immer so an TORCS zur�ck, wie sie in 
// der XML-Datei des Teams angegeben sind!
// Wertet den Pitsharing-Status aus und handelt danach bei Endurance Rennen
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

#include <tgf.h>
#include <track.h>
#include <car.h>
#include <raceman.h>
#include <robottools.h>
#include <timeanalysis.h>
#include <robot.h>

#include "unitglobal.h"
#include "unitcommon.h"

#include "unitdriver.h"

//==========================================================================*
// Prototypes of routines(functions/procedures), provided
// for communication with TORCS using the traditional Interface
// Prototypen der Routinen(Funktionen/Prozeduren), die wir f�r die
// Kommunikation mit TORCS �ber das traditionale Interface bereitstellen
//--------------------------------------------------------------------------*
static void InitTrack
  (int index,
  tTrack* track,
  void *carHandle,
  void **carParmHandle,
  tSituation *s);
static void NewRace
  (int index,
  tCarElt* car,
  tSituation *s);
static void Drive
  (int index,
  tCarElt* car,
  tSituation *s);
static int PitCmd
  (int index,
  tCarElt* car,
  tSituation *s);
static void Shutdown
  (int index);
static int InitFuncPt
  (int index,
  void *pt);
static void EndRace
  (int index,
  tCarElt *car,
  tSituation *s);
static void Shutdown
  (int index);
//==========================================================================*

//==========================================================================*
// Speed Dreams-Interface
//--------------------------------------------------------------------------*
static const int MAXNBBOTS = MAX_NBBOTS;         // Number of drivers/robots
static const int BUFSIZE = 256;

// Default driver names
static char const* defaultBotName[MAXNBBOTS] = {
	"driver 1",  "driver 2",  "driver 3",  "driver 4",  "driver 5",
	"driver 6",  "driver 7",  "driver 8",  "driver 9",  "driver 10", 
	"driver 11", "driver 12", "driver 13", "driver 14", "driver 15",
	"driver 16", "driver 17", "driver 18", "driver 19", "driver 20" 
};

// Default driver descriptions
static char const* defaultBotDesc[MAXNBBOTS] = {
	"driver 1",  "driver 2",  "driver 3",  "driver 4",  "driver 5",
	"driver 6",  "driver 7",  "driver 8",  "driver 9",  "driver 10", 
	"driver 11", "driver 12", "driver 13", "driver 14", "driver 15",
	"driver 16", "driver 17", "driver 18", "driver 19", "driver 20" 
};

// Max length of a drivers name
static const int DRIVERLEN = 32;                 
// Buffer for driver's names defined in robot's xml-file
static char DriverNames[DRIVERLEN * MAXNBBOTS]; 
// Buffer for driver's descriptions defined in robot's xml-file
static char DriverDescs[DRIVERLEN * MAXNBBOTS]; 

// Number of drivers defined in robot's xml-file
static int NBBOTS = 0;                           // Still unknown
// Robot's name
static char BufName[BUFSIZE];                    // Buffer for robot's name
static const char* RobName = BufName;            // Pointer to robot's name
// Robot's relative dir
static char BufPathDirRel[BUFSIZE];              // Robot's dir relative
static const char* RobPathDirRel = BufPathDirRel;// to installation dir
// Robot's relative xml-filename
static char BufPathXMLRel[BUFSIZE];              // Robot's xml-filename
static const char* RobPathXMLRel = BufPathXMLRel;// relative to install. dir
// Robot's absolute dir
static char BufPathDir[BUFSIZE];                 // Robot's dir 
static const char* RobPathDir = BufPathDir;      // Pointer to robot's dir
// Robot's absolute xml-filename
static char BufPathXML[BUFSIZE];                 // Robot's xml-filename
static const char* RobPathXML = BufPathXML;      // Pointer to xml-filename

static void *RobotSettings;                      // Filehandle

// Save start index offset from robot's xml file
static int IndexOffset = 0;

// Marker for undefined drivers to be able to comment out drivers 
// in the robot's xml-file between others, not only at the end of the list
char undefined[] = "undefined";                  
//==========================================================================*

//==========================================================================*
//  Robot of this modul
//  Roboter des Moduls
//--------------------------------------------------------------------------*
//static char const** BotDesc = defaultBotDesc;

static TCommonData gCommonData;
static int cRobotType;

typedef struct stInstanceInfo
{
	TDriver *cRobot;
	double cTicks;
	double cMinTicks;
	double cMaxTicks;
	int cTickCount;
	int cLongSteps;
	int cCriticalSteps;
	int cUnusedCount;
} tInstanceInfo;

//#undef ROB_SECT_ARBITRARY
#ifdef ROB_SECT_ARBITRARY
static tInstanceInfo *cInstances;
static int cInstancesCount;
#else //ROB_SECT_ARBITRARY
static tInstanceInfo cInstances[MAXNBBOTS];
#endif //ROB_SECT_ARBITRARY

//==========================================================================*

//==========================================================================*
// Get filehandle for robot's xml-file
//--------------------------------------------------------------------------*
void* GetFileHandle(const char* RobotName)
{
    strncpy(BufName, RobotName, BUFSIZE);       // Save robot's name
    snprintf(BufPathDirRel, BUFSIZE,             // Robot's directory  
		"drivers/%s",RobotName);                 // relative to installation
    snprintf(BufPathXMLRel, BUFSIZE,             // Robot's xml-filename
		"drivers/%s/%s.xml",RobotName,RobotName);// relative to installation

	// Test local installation path
    snprintf(BufPathXML, BUFSIZE, "%s%s",         
		GetLocalDir(), RobPathXMLRel);
	snprintf(BufPathDir, BUFSIZE, "%s%s", 
		GetLocalDir(), RobPathDirRel);
	RobotSettings = GfParmReadFile
		(RobPathXML, GFPARM_RMODE_STD );

	if (!RobotSettings)
	{
	  // If not found, use global installation path
	  snprintf(BufPathXML, BUFSIZE, "%s%s", 
		  GetDataDir(), RobPathXMLRel);
  	  snprintf(BufPathDir, BUFSIZE, "%s%s", 
		  GetDataDir(), RobPathDirRel);
	  RobotSettings = GfParmReadFile
		  (RobPathXML, GFPARM_RMODE_STD );
	}
	return RobotSettings;
}
//==========================================================================*

//==========================================================================*
// Set parameters 
//--------------------------------------------------------------------------*
void SetParameters(int N, char const* DefaultCarType)
{
  NBBOTS = N;
  TDriver::NBBOTS = N;                                    // Used nbr of cars
  TDriver::MyBotName = BufName;                           // Name of this bot 
  TDriver::ROBOT_DIR = BufPathDir;                        // Path to dll
  TDriver::SECT_PRIV = "simplix private";                 // Private section
  TDriver::DEFAULTCARTYPE  = DefaultCarType;              // Default car type
};
//==========================================================================*

//==========================================================================*
// Schismatic entry point for simplix
//--------------------------------------------------------------------------*
void SetUpSimplix()
{
  cRobotType = RTYPE_SIMPLIX;
  SetParameters(NBBOTS, "car1-trb1");
  TDriver::AdvancedParameters = true;
};
//==========================================================================*

//==========================================================================*
// Schismatic entry point for simplix_indy
//--------------------------------------------------------------------------*
void SetUpSimplix_indy()
{
	cRobotType = RTYPE_SIMPLIX_INDY;
	SetParameters(NBBOTS, "indycar01");
};
//==========================================================================*

//==========================================================================*
// Schismatic entry point for simplix_trb1
//--------------------------------------------------------------------------*
void SetUpSimplix_trb1()
{
  cRobotType = RTYPE_SIMPLIX_TRB1;
  SetParameters(NBBOTS, "car1-trb1");
  TDriver::Learning = true;
};
//==========================================================================*

//==========================================================================*
// Schismatic entry point for simplix_sc
//--------------------------------------------------------------------------*
void SetUpSimplix_sc()
{
  cRobotType = RTYPE_SIMPLIX_SC;
  SetParameters(NBBOTS, "sc996");
  TDriver::UseSCSkilling = true;                 // Use supercar skilling
};
//==========================================================================*

//==========================================================================*
// Schismatic entry point for simplix_36GP
//--------------------------------------------------------------------------*
void SetUpSimplix_36GP()
{
  cRobotType = RTYPE_SIMPLIX_36GP;
  SetParameters(NBBOTS, "36GP-alfa12c");
  TDriver::AdvancedParameters = true;
  TDriver::UseBrakeLimit = true;
  TDriver::Learning = true;
};
//==========================================================================*

//==========================================================================*
// Schismatic entry point for simplix_ls1
//--------------------------------------------------------------------------*
void SetUpSimplix_ls1()
{
	cRobotType = RTYPE_SIMPLIX_LS1;
	SetParameters(NBBOTS, "ls1-ciclon-rgt");
	//TDriver::UseSCSkilling = true; 
};
//==========================================================================*

//==========================================================================*
// Schismatic entry point for simplix_gp1600
//--------------------------------------------------------------------------*
void SetUpSimplix_gp1600()
{
	cRobotType = RTYPE_SIMPLIX_GP1600;
	SetParameters(NBBOTS, "gp1600");
	//TDriver::UseSCSkilling = true; 
};
//==========================================================================*

//==========================================================================*
// Handle module entry for Speed Dreams Interface V1.00 (new fixed name scheme)
//--------------------------------------------------------------------------*
int moduleWelcomeV1_00
  (const tModWelcomeIn* welcomeIn, tModWelcomeOut* welcomeOut)
{
    memset(DriverNames, 0, MAXNBBOTS*DRIVERLEN);
    memset(DriverDescs, 0, MAXNBBOTS*DRIVERLEN);

	GfOut("\n");
	GfOut("#Interface Version: %d.%d\n",
		welcomeIn->itfVerMajor,welcomeIn->itfVerMinor);

	// Get filehandle for robot's xml-file
	void *RobotSettings = GetFileHandle(welcomeIn->name);
	// Let's look what we have to provide here
	if (RobotSettings)
	{
		GfOut("#Robot name      : %s\n",RobName);
 		GfOut("#Robot directory : %s\n",RobPathDir);
		GfOut("#Robot XML-file  : %s\n",RobPathXML);

		char Buffer[BUFSIZE];
		char *Section = Buffer;

		snprintf(Buffer, BUFSIZE, "%s/%s/%d", 
			ROB_SECT_ROBOTS, ROB_LIST_INDEX, 0);

		// Try to get first driver from index 0
		const char *DriverName = GfParmGetStr( RobotSettings, 
			Section, (char *) ROB_ATTR_NAME, undefined);

		// Check wether index 0 is used as start index
		if (strncmp(DriverName,undefined,strlen(undefined)) != 0)
		{
			// Teams xml file uses index 0, 1, ..., N - 1
            IndexOffset = 0; 
		}
		else
		{
			// Teams xml file uses index 1, 2, ..., N
            IndexOffset = 1; 
		}

		// Loop over all possible drivers, clear all buffers, 
		// save defined driver names and desc.
	    int I;
		for (I = 0; I < MAXNBBOTS; I++)
		{
			snprintf(Section, BUFSIZE, "%s/%s/%d", 
				ROB_SECT_ROBOTS, ROB_LIST_INDEX, I + IndexOffset );
			const char *DriverName = GfParmGetStr( RobotSettings, Section, 
				(char *) ROB_ATTR_NAME,undefined);

	        if (strncmp(DriverName,undefined,strlen(undefined)) != 0)
			{   // This driver is defined in robot's xml-file
				strncpy(&DriverNames[I*DRIVERLEN], DriverName, DRIVERLEN-1);
			    const char *DriverDesc = GfParmGetStr(RobotSettings, Section, 
					(char *) ROB_ATTR_DESC, defaultBotDesc[I]);
				strncpy(&DriverDescs[I*DRIVERLEN], DriverDesc, DRIVERLEN-1);
				NBBOTS = I + 1;
			}
		}
	}
	else
	{
		// Handle error here
 	    GfOut("#Robot XML-Path not found: (%s) or (%s) %s\n\n",
			GetLocalDir(),GetDataDir(),RobPathXMLRel);

		NBBOTS = 0;
	    welcomeOut->maxNbItf = NBBOTS;
	    return -1;
	}

	// Handle additional settings for wellknown identities
	if (strncmp(RobName,"simplix_trb1",strlen("simplix_trb1")) == 0)
		SetUpSimplix_trb1();
	else if (strncmp(RobName,"simplix_sc",strlen("simplix_sc")) == 0)
		SetUpSimplix_sc();
	else if (strncmp(RobName,"simplix_36GP",strlen("simplix_36GP")) == 0)
		SetUpSimplix_36GP();
	else if (strncmp(RobName,"simplix_INDY",strlen("simplix_INDY")) == 0)
		SetUpSimplix_indy();
	else if (strncmp(RobName,"simplix_LS1",strlen("simplix_LS1")) == 0)
		SetUpSimplix_ls1();
	else if (strncmp(RobName,"simplix_gp1600",strlen("simplix_gp1600")) == 0)
		SetUpSimplix_ls1();
	else 
		SetUpSimplix();

	// Set max nb of interfaces to return.
	welcomeOut->maxNbItf = NBBOTS;

	return 0;
}
//==========================================================================*

//==========================================================================*
// Module entry point (new fixed name scheme).
// Extended for use with schismatic robots and checked interface versions
//--------------------------------------------------------------------------*
extern "C" int moduleWelcome
  (const tModWelcomeIn* welcomeIn, tModWelcomeOut* welcomeOut)
{
	if (welcomeIn->itfVerMajor >= 1)
	{
		if (welcomeIn->itfVerMinor >= 0)
          return moduleWelcomeV1_00(welcomeIn, welcomeOut);
	}

	GfOut("\n");
	GfOut("#Unhandled Interface Version: %d.%d\n",
  	      welcomeIn->itfVerMajor,welcomeIn->itfVerMinor);
	welcomeOut->maxNbItf = 0;
	return -1;
}
//==========================================================================*

//==========================================================================*
// Module entry point (new fixed name scheme).
// Tells TORCS, who we are, how we want to be called and 
// what we are able to do.
// Teilt TORCS mit, wer wir sind, wie wir angesprochen werden wollen und
// was wir k�nnen.
//--------------------------------------------------------------------------*
extern "C" int moduleInitialize(tModInfo *ModInfo)
{
  GfOut("\n");
  GfOut("#Initialize from %s ...\n",RobPathXML);
  GfOut("#NBBOTS: %d (of %d)\n",NBBOTS,MAXNBBOTS);

#ifdef ROB_SECT_ARBITRARY
  // Clear all structures.
  memset(ModInfo, 0, (NBBOTS+1)*sizeof(tModInfo));

  int I;
  for (I = 0; I < TDriver::NBBOTS; I++) 
  {
    ModInfo[I].name = &DriverNames[I*DRIVERLEN]; // Tell customisable name
    ModInfo[I].desc = &DriverDescs[I*DRIVERLEN]; // Tell customisable desc.
    ModInfo[I].fctInit = InitFuncPt;             // Common used functions
    ModInfo[I].gfId = ROB_IDENT;                 // Robot identity
    ModInfo[I].index = I+IndexOffset;            // Drivers index
  }
  ModInfo[NBBOTS].name = RobName;
  ModInfo[NBBOTS].desc = RobName;
  ModInfo[NBBOTS].fctInit = InitFuncPt;
  ModInfo[NBBOTS].gfId = ROB_IDENT;
  ModInfo[NBBOTS].index = NBBOTS+IndexOffset;
#else //ROB_SECT_ARBITRARY
  // Clear all structures.
  memset(ModInfo, 0, NBBOTS*sizeof(tModInfo));

  int I;
  for (I = 0; I < TDriver::NBBOTS; I++) 
  {
    ModInfo[I].name = &DriverNames[I*DRIVERLEN]; // Tell customisable name
    ModInfo[I].desc = &DriverDescs[I*DRIVERLEN]; // Tell customisable desc.
    ModInfo[I].fctInit = InitFuncPt;             // Common used functions
    ModInfo[I].gfId = ROB_IDENT;                 // Robot identity
    ModInfo[I].index = I+IndexOffset;            // Drivers index
  }
#endif //ROB_SECT_ARBITRARY

  GfOut("# ... Initialized\n\n");

  return 0;
}
//==========================================================================*

//==========================================================================*
// Module exit point (new fixed name scheme).
//--------------------------------------------------------------------------*
extern "C" int moduleTerminate()
{
  GfOut("\n");
  GfOut("#Terminated %s\n",RobName);
	
  return 0;
}
//==========================================================================*

//==========================================================================*
// Module entry point (Torcs backward compatibility scheme).
//--------------------------------------------------------------------------*
int simplixEntryPoint(tModInfo *ModInfo, void *RobotSettings)
{
    GfOut("\n");
    GfOut("#Torcs backward compatibility scheme used\n");
    NBBOTS = MIN(10,NBBOTS);

    memset(ModInfo, 0, NBBOTS*sizeof(tModInfo));
    memset(DriverNames, 0, MAXNBBOTS*DRIVERLEN);
    memset(DriverDescs, 0, MAXNBBOTS*DRIVERLEN);

    char SectionBuf[BUFSIZE];
	char *Section = SectionBuf;

	snprintf( SectionBuf, BUFSIZE, "%s/%s/%d", 
		ROB_SECT_ROBOTS, ROB_LIST_INDEX, 0);

    int I;
    for (I = 0; I < NBBOTS; I++) 
    {
	  snprintf( SectionBuf, BUFSIZE, "%s/%s/%d", 
		  ROB_SECT_ROBOTS, ROB_LIST_INDEX, I + IndexOffset );
	  const char *DriverName = GfParmGetStr( RobotSettings, 
		  Section, (char *) ROB_ATTR_NAME, defaultBotName[I]);
	  strncpy(&DriverNames[I*DRIVERLEN], DriverName, DRIVERLEN-1);
      const char *DriverDesc = GfParmGetStr( RobotSettings, 
		  Section, (char *) ROB_ATTR_DESC, defaultBotDesc[I]);
	  strncpy(&DriverDescs[I*DRIVERLEN], DriverDesc, DRIVERLEN-1);
    }

    return moduleInitialize(ModInfo);
}
//==========================================================================*

//==========================================================================*
// Module exit point (Torcs backward compatibility scheme).
//--------------------------------------------------------------------------*
extern "C" int simplixShut()
{
    return moduleTerminate();
}
//==========================================================================*

//==========================================================================*
// TORCS: Initialization
// TOCRS: Initialisierung
//
// After clarification of the general calling (calling this func.), 
// we tell TORCS our functions to provide the requested services:
//
// Nach Kl�rung der generellen Ansprache (Aufruf dieser Fkt), teilen wir
// TORCS nun noch mit, mit welchen Funktionen wir die angeforderten
// Leistungen erbringen werden:
//
// Die geforderten Leistungen m�ssen erbracht werden ...
// RbNewTrack: ... wenn Torcs eine neue Rennstrecke bereitstellt
// RbNewRace:  ... wenn Torcs ein neues Rennen startet
// RbDrive:    ... wenn das Rennen gefahren wird
// RbPitCmd:   ... wenn wir einen Boxenstop machen
// RbEndRace:  ... wenn das Rennen ist beendet
// RbShutDown: ... wenn der ggf. angefallene Schrott beseitigt werden muss
//--------------------------------------------------------------------------*
static int InitFuncPt(int Index, void *Pt)
{
  tRobotItf *Itf = (tRobotItf *)Pt;              // Get typed pointer

  Itf->rbNewTrack = InitTrack;                   // Store function pointers 
  Itf->rbNewRace  = NewRace;
  Itf->rbDrive    = Drive;
  Itf->rbPitCmd   = PitCmd;
  Itf->rbEndRace  = EndRace;
  Itf->rbShutdown = Shutdown;
  Itf->index      = Index;                       // Store index

#ifdef ROB_SECT_ARBITRARY
  int xx;
  tInstanceInfo *copy;

  //Make sure enough data is allocated
  if (cInstancesCount <= Index-IndexOffset) {
    copy = new tInstanceInfo[Index-IndexOffset+1];
    for (xx = 0; xx < cInstancesCount; ++xx)
      copy[xx] = cInstances[xx];
    for (xx = cInstancesCount; xx < Index-IndexOffset+1; ++xx)
      copy[xx].cRobot = NULL;
    if (cInstancesCount > 0)
      delete []cInstances;
    cInstances = copy;
    cInstancesCount = Index-IndexOffset+1;
  }
#endif

  cInstances[Index-IndexOffset].cRobot =                    // Create a driver
	  new TDriver(Index-IndexOffset);
  cInstances[Index-IndexOffset].cRobot->SetBotName          // Store customized name
	  (RobotSettings,                            // Robot's xml-file
	  &DriverNames[(Index-IndexOffset)*DRIVERLEN]);// not drivers xml-file!  

  if (cRobotType == RTYPE_SIMPLIX)
  {
	  GfOut("#cRobotType == RTYPE_SIMPLIX\n");
    cInstances[Index-IndexOffset].cRobot->CalcCrvFoo = &TDriver::CalcCrv_simplix;
    cInstances[Index-IndexOffset].cRobot->CalcHairpinFoo = &TDriver::CalcHairpin_simplix;
    cInstances[Index-IndexOffset].cRobot->ScaleSide(0.95f,0.95f);
    cInstances[Index-IndexOffset].cRobot->SideBorderOuter(0.20f);
  }
  else if (cRobotType == RTYPE_SIMPLIX_TRB1)
  {
    GfOut("#cRobotType == RTYPE_SIMPLIX_TRB1\n");
    cInstances[Index-IndexOffset].cRobot->CalcCrvFoo = &TDriver::CalcCrv_simplix_TRB1;
    cInstances[Index-IndexOffset].cRobot->CalcHairpinFoo = &TDriver::CalcHairpin_simplix_TRB1;
    cInstances[Index-IndexOffset].cRobot->ScaleSide(0.95f,0.95f);
    cInstances[Index-IndexOffset].cRobot->SideBorderOuter(0.20f);
  }
  else if (cRobotType == RTYPE_SIMPLIX_SC)
  {
    GfOut("#cRobotType == RTYPE_SIMPLIX_SC\n");
    cInstances[Index-IndexOffset].cRobot->CalcCrvFoo = &TDriver::CalcCrv_simplix_SC;
    cInstances[Index-IndexOffset].cRobot->CalcHairpinFoo = &TDriver::CalcHairpin_simplix_SC;
    cInstances[Index-IndexOffset].cRobot->ScaleSide(0.90f,0.95f);
    cInstances[Index-IndexOffset].cRobot->SideBorderOuter(0.30f);
  }
  else if (cRobotType == RTYPE_SIMPLIX_36GP)
  {
    GfOut("#cRobotType == RTYPE_SIMPLIX_36GP\n");
    cInstances[Index-IndexOffset].cRobot->CalcCrvFoo = &TDriver::CalcCrv_simplix_36GP;
    cInstances[Index-IndexOffset].cRobot->CalcHairpinFoo = &TDriver::CalcHairpin_simplix_36GP;
    cInstances[Index-IndexOffset].cRobot->ScaleSide(0.85f,0.85f);
    cInstances[Index-IndexOffset].cRobot->SideBorderOuter(0.75f);
    //cRobot[Index-IndexOffset]->UseFilterAccel();
  }
  else if (cRobotType == RTYPE_SIMPLIX_INDY)
  {
    GfOut("#cRobotType == RTYPE_SIMPLIX_INDY\n");
    cInstances[Index-IndexOffset].cRobot->CalcCrvFoo = &TDriver::CalcCrv_simplix_INDY;
    cInstances[Index-IndexOffset].cRobot->CalcHairpinFoo = &TDriver::CalcHairpin_simplix_INDY;
    cInstances[Index-IndexOffset].cRobot->ScaleSide(0.95f,0.95f);
    cInstances[Index-IndexOffset].cRobot->SideBorderOuter(0.20f);
  }
  else if (cRobotType == RTYPE_SIMPLIX_LS1)
  {
    GfOut("#cRobotType == RTYPE_SIMPLIX_LS1\n");
    cInstances[Index-IndexOffset].cRobot->CalcCrvFoo = &TDriver::CalcCrv_simplix_LS1;
    cInstances[Index-IndexOffset].cRobot->CalcHairpinFoo = &TDriver::CalcHairpin_simplix_LS1;
    cInstances[Index-IndexOffset].cRobot->ScaleSide(0.95f,0.95f);
    cInstances[Index-IndexOffset].cRobot->SideBorderOuter(0.20f);
  }
  else if (cRobotType == RTYPE_SIMPLIX_GP1600)
  {
    GfOut("#cRobotType == RTYPE_SIMPLIX_GP1600\n");
    cInstances[Index-IndexOffset].cRobot->CalcCrvFoo = &TDriver::CalcCrv_simplix_GP1600;
    cInstances[Index-IndexOffset].cRobot->CalcHairpinFoo = &TDriver::CalcHairpin_simplix_GP1600;
    cInstances[Index-IndexOffset].cRobot->ScaleSide(0.85f,0.85f);
    cInstances[Index-IndexOffset].cRobot->SideBorderOuter(0.75f);
  }

  return 0;
}
//==========================================================================*

//==========================================================================*
// TORCS: New track
// TOCRS: Neue Rennstrecke
//--------------------------------------------------------------------------*
static void InitTrack(int Index,
  tTrack* Track,void *CarHandle,void **CarParmHandle, tSituation *S)
{
  // Init common used data
  cInstances[Index-IndexOffset].cRobot->SetCommonData(&gCommonData);    
  cInstances[Index-IndexOffset].cRobot->InitTrack(Track,CarHandle,CarParmHandle, S);
}
//==========================================================================*

//==========================================================================*
// TORCS: New Race starts
// TOCRS: Neues Rennen beginnt
//--------------------------------------------------------------------------*
static void NewRace(int Index, tCarElt* Car, tSituation *S)
{
#ifdef SPEED_DREAMS
  RtInitTimer(); // Check existance of Performance Counter Hardware
#endif
  cInstances[Index-IndexOffset].cTicks = 0.0;               // Initialize counters
  cInstances[Index-IndexOffset].cMinTicks = FLT_MAX;        // and time data 
  cInstances[Index-IndexOffset].cMaxTicks = 0.0;
  cInstances[Index-IndexOffset].cTickCount = 0;
  cInstances[Index-IndexOffset].cLongSteps = 0;
  cInstances[Index-IndexOffset].cCriticalSteps = 0;
  cInstances[Index-IndexOffset].cUnusedCount = 0;
  
  cInstances[Index-IndexOffset].cRobot->NewRace(Car, S);
  cInstances[Index-IndexOffset].cRobot->CurrSimTime = -10.0;
}
//==========================================================================*

//==========================================================================*
// TORCS-Callback: Drive
// TOCRS-Callback: Rennen fahren
//
// Attention: This procedure is called very frequent and fast in succession!
// Therefore we don't throw debug messages here!
// To find basic bugs, it may be usefull to do it anyhow!

// Achtung: Diese Prozedur wird sehr h�ufig und schnell nacheinander
// aufgerufen. Deshalb geben wir hier in der Regel keine Debug-Texte aus!
// Zur Fehlersuche kann das aber mal sinnvoll sein.
//--------------------------------------------------------------------------*
static void Drive(int Index, tCarElt* Car, tSituation *S)
{
  //GfOut("#>>> TDriver::Drive\n");
  if (cInstances[Index-IndexOffset].cRobot->CurrSimTime < S->currentTime)
//  if (cInstances[Index-IndexOffset].cRobot->CurrSimTime + 0.03 < S->currentTime)
  {
    //GfOut("#Drive\n");
	double StartTimeStamp = RtTimeStamp(); 

    cInstances[Index-IndexOffset].cRobot->CurrSimTime =     // Update current time
		S->currentTime; 
    cInstances[Index-IndexOffset].cRobot->Update(Car,S);    // Update info about opp.
    if (cInstances[Index-IndexOffset].cRobot->IsStuck())    // Check if we are stuck  
  	  cInstances[Index-IndexOffset].cRobot->Unstuck();      //   Unstuck 
	else                                         // or
	  cInstances[Index-IndexOffset].cRobot->Drive();        //   Drive

	double Duration = RtDuration(StartTimeStamp);

	if (cInstances[Index-IndexOffset].cTickCount > 0)       // Collect used time 
	{
	  if (Duration > 1.0)
        cInstances[Index-IndexOffset].cLongSteps++;
	  if (Duration > 2.0)
        cInstances[Index-IndexOffset].cCriticalSteps++;
	  if (cInstances[Index-IndexOffset].cMinTicks > Duration)
	    cInstances[Index-IndexOffset].cMinTicks = Duration;
	  if (cInstances[Index-IndexOffset].cMaxTicks < Duration)
	    cInstances[Index-IndexOffset].cMaxTicks = Duration;
	}
	cInstances[Index-IndexOffset].cTickCount++;
  	cInstances[Index-IndexOffset].cTicks += Duration;
  }
  else
  {
    //GfOut("#DriveLast\n");
    cInstances[Index-IndexOffset].cUnusedCount++;
    cInstances[Index-IndexOffset].cRobot->DriveLast();      // Use last drive commands
  }
  //GfOut("#<<< TDriver::Drive\n");
}
//==========================================================================*

//==========================================================================*
// TORCS: Pitstop (Car is in pit!)
// TOCRS: Boxenstop (Wagen steht in der Box!)
//--------------------------------------------------------------------------*
static int PitCmd(int Index, tCarElt* Car, tSituation *S)
{
  // Dummy: use parameters
  if ((Index < 0) || (Car == NULL) || (S == NULL))
    printf("PitCmd\n");
  return cInstances[Index-IndexOffset].cRobot->PitCmd();
}
//==========================================================================*

//==========================================================================*
// TORCS: Race ended
// TOCRS: Rennen ist beendet
//--------------------------------------------------------------------------*
static void EndRace(int Index, tCarElt *Car, tSituation *S)
{
  // Dummy: use parameters
  if ((Index < 0) || (Car == NULL) || (S == NULL))
	  Index = 0;

  printf("EndRace\n");
  cInstances[Index-IndexOffset].cRobot->EndRace();
}
//==========================================================================*

//==========================================================================*
// TORCS: Cleanup
// TOCRS: Aufr�umen
//--------------------------------------------------------------------------*
static void Shutdown(int Index)
{
#ifdef ROB_SECT_ARBITRARY
  int count;
  int xx;
  tInstanceInfo *copy;
#endif //ROB_SECT_ARBITRARY

  GfOut("\n");
  GfOut("\n");
  GfOut("#Clock\n");
  GfOut("#Total Time used: %g sec\n",cInstances[Index-IndexOffset].cTicks/1000.0);
  GfOut("#Min   Time used: %g msec\n",cInstances[Index-IndexOffset].cMinTicks);
  GfOut("#Max   Time used: %g msec\n",cInstances[Index-IndexOffset].cMaxTicks);
  GfOut("#Mean  Time used: %g msec\n",cInstances[Index-IndexOffset].cTicks/cInstances[Index-IndexOffset].cTickCount);
  GfOut("#Long Time Steps: %d\n",cInstances[Index-IndexOffset].cLongSteps);
  GfOut("#Critical Steps : %d\n",cInstances[Index-IndexOffset].cCriticalSteps);
  GfOut("#Unused Steps   : %d\n",cInstances[Index-IndexOffset].cUnusedCount);
  GfOut("\n");
  GfOut("\n");

  cInstances[Index-IndexOffset].cRobot->Shutdown();
  delete cInstances[Index-IndexOffset].cRobot;
  cInstances[Index-IndexOffset].cRobot = NULL;

#ifdef ROB_SECT_ARBITRARY
  //Check if this was the highest index
  if (cInstancesCount == Index-IndexOffset+1)
  {
    //Now make the cInstances array smaller
    //Count the number of robots which are still in the array
    count = 0;
    for (xx = 0; xx < Index-IndexOffset+1; ++xx)
    {
      if (cInstances[xx].cRobot)
        count = xx+1;
    }

    if (count>0)
    {
      //We have robots left: make a new array
      copy = new tInstanceInfo[count];
      for (xx = 0; xx < count; ++xx)
        copy[xx] = cInstances[xx];
    }
    else
    {
      copy = NULL;
    }
    delete []cInstances;
    cInstances = copy;
    cInstancesCount = count;
  }
#endif //ROB_SECT_ARBITRARY
}
//==========================================================================*

//==========================================================================*
// Module entry point (Torcs backward compatibility scheme).
//--------------------------------------------------------------------------*
extern "C" int simplix(tModInfo *ModInfo)
{
  void *RobotSettings = GetFileHandle("simplix");
  if (!RobotSettings)
	  return -1;
  
  SetParameters(1, "car1-trb1");
  return simplixEntryPoint(ModInfo,RobotSettings);
}
//==========================================================================*

//==========================================================================*
// Schismatic entry point for simplix_indy
//--------------------------------------------------------------------------*
extern "C" int simplix_indy(tModInfo *ModInfo)
{
  void *RobotSettings = GetFileHandle("simplix_indy");
  if (!RobotSettings)
	  return -1;

  SetParameters(10, "indycar01");
  return simplixEntryPoint(ModInfo,RobotSettings);
};
//==========================================================================*

//==========================================================================*
// Schismatic entry point for simplix_trb1a
//--------------------------------------------------------------------------*
extern "C" int simplix_trb1a(tModInfo *ModInfo)
{
  void *RobotSettings = GetFileHandle("simplix_trb1a");
  if (!RobotSettings)
	  return -1;

  SetParameters(10, "car1-trb1");
  return simplixEntryPoint(ModInfo,RobotSettings);
};
//==========================================================================*

//==========================================================================*
// Schismatic entry point for simplix_trb1b
//--------------------------------------------------------------------------*
extern "C" int simplix_trb1b(tModInfo *ModInfo)
{
  void *RobotSettings = GetFileHandle("simplix_trb1b");
  if (!RobotSettings)
	  return -1;

  SetParameters(10, "car6-trb1");
  return simplixEntryPoint(ModInfo,RobotSettings);
};
//==========================================================================*

//==========================================================================*
// Schismatic entry point for simplix_sca
//--------------------------------------------------------------------------*
extern "C" int simplix_sca(tModInfo *ModInfo)
{
  void *RobotSettings = GetFileHandle("simplix_sca");
  if (!RobotSettings)
	  return -1;

  SetParameters(10, "sc-996");
  return simplixEntryPoint(ModInfo,RobotSettings);
};
//==========================================================================*

//==========================================================================*
// Schismatic entry point for simplix_scb
//--------------------------------------------------------------------------*
extern "C" int simplix_scb(tModInfo *ModInfo)
{
  void *RobotSettings = GetFileHandle("simplix_scb");
  if (!RobotSettings)
	  return -1;

  SetParameters(10, "sc-996");
  return simplixEntryPoint(ModInfo,RobotSettings);
};
//==========================================================================*

//==========================================================================*
// Schismatic entry point for simplix_36GP
//--------------------------------------------------------------------------*
extern "C" int simplix_36GP(tModInfo *ModInfo)
{
  void *RobotSettings = GetFileHandle("simplix_36GP");
  if (!RobotSettings)
	  return -1;

  SetParameters(10, "36GP-alfa12c");
  TDriver::AdvancedParameters = true;
  TDriver::UseBrakeLimit = true;
  return simplixEntryPoint(ModInfo,RobotSettings);
};
//==========================================================================*

//==========================================================================*
// Schismatic entry point for simplix_ls1
//--------------------------------------------------------------------------*
extern "C" int simplix_ls1(tModInfo *ModInfo)
{
  void *RobotSettings = GetFileHandle("simplix_ls1");
  if (!RobotSettings)
	  return -1;

  SetParameters(10, "ls1-ciclon-rgt");
  return simplixEntryPoint(ModInfo,RobotSettings);
};
//==========================================================================*

//==========================================================================*
// Schismatic entry point for simplix_gp1600
//--------------------------------------------------------------------------*
extern "C" int simplix_gp1600(tModInfo *ModInfo)
{
  void *RobotSettings = GetFileHandle("simplix_gp1600");
  if (!RobotSettings)
	  return -1;

  SetParameters(10, "gp1600");
  return simplixEntryPoint(ModInfo,RobotSettings);
};
//==========================================================================*

//==========================================================================*
// Schismatic entry point for my_simplix_0
//--------------------------------------------------------------------------*
extern "C" int my_simplix_0(tModInfo *ModInfo)
{
  void *RobotSettings = GetFileHandle("my_simplix_0");
  if (!RobotSettings)
	  return -1;

  SetParameters(10, "car1-trb1");
  TDriver::AdvancedParameters = true;
  return simplixEntryPoint(ModInfo,RobotSettings);
};
//==========================================================================*

//==========================================================================*
// Schismatic entry point for my_simplix_1
//--------------------------------------------------------------------------*
extern "C" int my_simplix_1(tModInfo *ModInfo)
{
  void *RobotSettings = GetFileHandle("my_simplix_1");
  if (!RobotSettings)
	  return -1;

  SetParameters(10, "car1-trb1");
  TDriver::AdvancedParameters = true;
  return simplixEntryPoint(ModInfo,RobotSettings);
};
//==========================================================================*

//==========================================================================*
// Schismatic entry point for my_simplix_2
//--------------------------------------------------------------------------*
extern "C" int my_simplix_2(tModInfo *ModInfo)
{
  void *RobotSettings = GetFileHandle("my_simplix_2");
  if (!RobotSettings)
	  return -1;

  SetParameters(10, "car1-trb1");
  TDriver::AdvancedParameters = true;
  return simplixEntryPoint(ModInfo,RobotSettings);
};
//==========================================================================*

//==========================================================================*
// Schismatic entry point for my_simplix_3
//--------------------------------------------------------------------------*
extern "C" int my_simplix_3(tModInfo *ModInfo)
{
  void *RobotSettings = GetFileHandle("my_simplix_3");
  if (!RobotSettings)
	  return -1;

  SetParameters(10, "car1-trb1");
  TDriver::AdvancedParameters = true;
  return simplixEntryPoint(ModInfo,RobotSettings);
};
//==========================================================================*

//==========================================================================*
// Schismatic entry point for my_simplix_4
//--------------------------------------------------------------------------*
extern "C" int my_simplix_4(tModInfo *ModInfo)
{
  void *RobotSettings = GetFileHandle("my_simplix_4");
  if (!RobotSettings)
	  return -1;

  SetParameters(10, "car1-trb1");
  TDriver::AdvancedParameters = true;
  return simplixEntryPoint(ModInfo,RobotSettings);
};
//==========================================================================*

//==========================================================================*
// Schismatic entry point for my_simplix_5
//--------------------------------------------------------------------------*
extern "C" int my_simplix_5(tModInfo *ModInfo)
{
  void *RobotSettings = GetFileHandle("my_simplix_5");
  if (!RobotSettings)
	  return -1;

  SetParameters(10, "car1-trb1");
  TDriver::AdvancedParameters = true;
  return simplixEntryPoint(ModInfo,RobotSettings);
};
//==========================================================================*

//==========================================================================*
// Schismatic entry point for my_simplix_6
//--------------------------------------------------------------------------*
extern "C" int my_simplix_6(tModInfo *ModInfo)
{
  void *RobotSettings = GetFileHandle("my_simplix_6");
  if (!RobotSettings)
	  return -1;

  SetParameters(10, "car1-trb1");
  TDriver::AdvancedParameters = true;
  return simplixEntryPoint(ModInfo,RobotSettings);
};
//==========================================================================*

//==========================================================================*
// Schismatic entry point for my_simplix_7
//--------------------------------------------------------------------------*
extern "C" int my_simplix_7(tModInfo *ModInfo)
{
  void *RobotSettings = GetFileHandle("my_simplix_7");
  if (!RobotSettings)
	  return -1;

  SetParameters(10, "car1-trb1");
  TDriver::AdvancedParameters = true;
  return simplixEntryPoint(ModInfo,RobotSettings);
};
//==========================================================================*

//==========================================================================*
// Schismatic entry point for my_simplix_8
//--------------------------------------------------------------------------*
extern "C" int my_simplix_8(tModInfo *ModInfo)
{
  void *RobotSettings = GetFileHandle("my_simplix_8");
  if (!RobotSettings)
	  return -1;

  SetParameters(10, "car1-trb1");
  TDriver::AdvancedParameters = true;
  return simplixEntryPoint(ModInfo,RobotSettings);
};
//==========================================================================*

//==========================================================================*
// Schismatic entry point for my_simplix_9
//--------------------------------------------------------------------------*
extern "C" int my_simplix_9(tModInfo *ModInfo)
{
  void *RobotSettings = GetFileHandle("my_simplix_9");
  if (!RobotSettings)
	  return -1;

  SetParameters(10, "car1-trb1");
  TDriver::AdvancedParameters = true;
  return simplixEntryPoint(ModInfo,RobotSettings);
};
//==========================================================================*

//--------------------------------------------------------------------------*
// end of file unitmain.cpp
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
