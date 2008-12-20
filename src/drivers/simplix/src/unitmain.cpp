//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
// unitmain.cpp
//--------------------------------------------------------------------------*
// TORCS: "The Open Racing Car Simulator"
// A robot for TORCS-NG-Version 1.4.0
//--------------------------------------------------------------------------*
// Interface to TORCS
// 
// File         : unitmain.cpp 
// Created      : 2008.01.27
// Last changed : 2008.12.19
// Copyright    : © 2007-2008 Wolf-Dieter Beelitz
// eMail        : wdb@wdbee.de
// Version      : 2.00.000 
//--------------------------------------------------------------------------*
// V2.00:
// 
//--------------------------------------------------------------------------*
// V1.10:
// Features of the advanced TORCS Interface:
// Initialization runs once only, see "simplix(tModInfo *ModInfo)"
// Allways gives back the names of drivers as defined in teams xml file!
// Checks and handles pitsharing state enabled/disabled for endurance races.
// 
// Eigenschaften des erweiterten TORCS Interfaces:
// Die Initialisierung wird nur einmal ausgeführt, siehe dazu 
// "simplix(tModInfo *ModInfo)"
// Die DLL gibt die Namen der Fahrer immer so an TORCS zurück, wie sie in 
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
// Wer die Dateien verwendet erkennt an, dass für Fehler, Schäden,
// Folgefehler oder Folgeschäden keine Haftung übernommen wird.
//--------------------------------------------------------------------------*
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// Im übrigen gilt für die Nutzung und/oder Weitergabe die
// GNU GPL (General Public License)
// Version 2 oder nach eigener Wahl eine spätere Version.
//--------------------------------------------------------------------------*
#ifdef _WIN32
#include <windows.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include <tgf.h>
#include <track.h>
#include <car.h>
#include <raceman.h>
#include <robottools.h>
#include <robot.h>

#include "unitglobal.h"
#include "unitcommon.h"

#include "unitdriver.h"

//==========================================================================*
// Prototypes of routines(functions/procedures), provided
// for communication with TORCS using the traditional Interface
// Prototypen der Routinen(Funktionen/Prozeduren), die wir für die
// Kommunikation mit TORCS über das traditionale Interface bereitstellen
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
// TORCS-NG-Interface
//--------------------------------------------------------------------------*
//static const int MAX_NBBOTS = 20;                // Number of drivers/robots
static const int BUFSIZE = 256;

// Default driver names
static char* defaultBotName[MAX_NBBOTS] = {
	"driver 1",  "driver 2",  "driver 3",  "driver 4",  "driver 5",
	"driver 6",  "driver 7",  "driver 8",  "driver 9",  "driver 10", 
	"driver 11", "driver 12", "driver 13", "driver 14", "driver 15",
	"driver 16", "driver 17", "driver 18", "driver 19", "driver 20" 
};

// Default driver descriptions
static char* defaultBotDesc[MAX_NBBOTS] = {
	"driver 1",  "driver 2",  "driver 3",  "driver 4",  "driver 5",
	"driver 6",  "driver 7",  "driver 8",  "driver 9",  "driver 10", 
	"driver 11", "driver 12", "driver 13", "driver 14", "driver 15",
	"driver 16", "driver 17", "driver 18", "driver 19", "driver 20" 
};

static const int DRIVERLEN = 32;                 // Max length of a drivers name
static char DriverNames[DRIVERLEN * MAX_NBBOTS]; // Buffer for driver's names defined in robot's xml-file
static char DriverDescs[DRIVERLEN * MAX_NBBOTS]; // Buffer for driver's descriptions defined in robot's xml-file

// Number of drivers defined in robot's xml-file
static int NBBOTS = 0;                           // Still unknown
// Robot's name
char nameBuffer[BUFSIZE];                        // Buffer for robot's name
static const char* robotName = nameBuffer;       // Pointer to robot's name
// Robot's xml-filename
char pathBuffer[BUFSIZE];                        // Buffer for robot's xml-filename
static const char* pathXml = pathBuffer;         // Pointer to robot's xml-filename
// Robot's dir
char dirBuffer[BUFSIZE];

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
static char** BotDesc = defaultBotDesc;

static TDriver *cRobot[MAX_NBBOTS];
static TCommonData gCommonData;
static double cTicks;
static double cMinTicks;
static double cMaxTicks;
static int cTickCount;
static int cLongSteps;
static int cCriticalSteps;
static int cUnusedCount;
//==========================================================================*

//==========================================================================*
// Buffers
// Puffer
//--------------------------------------------------------------------------*
static char FilenameBuffer[BUFSIZE];             // for path and filename
static bool Footprint = false;                   // Never called yet
static int InitDriver = -1;                      // None initialized yet
//==========================================================================*


//==========================================================================*
// Schismatic entry point for simplix
//--------------------------------------------------------------------------*
void SetUpSimplix()
{
  TDriver::NBBOTS = NBBOTS;                               // use defined nbr of cars
  TDriver::MyBotName = (char *) robotName;                // Name of this bot 
  snprintf(dirBuffer,BUFSIZE,"drivers/%s",robotName);
  TDriver::ROBOT_DIR = dirBuffer;                         // Sub path to dll
  TDriver::SECT_PRIV = "simplix private";                 // Private section
  TDriver::DEFAULTCARTYPE  = "car1-trb1";                 // Default car type
  TDriver::AdvancedParameters = true;
};
//==========================================================================*

//==========================================================================*
// Schismatic entry point for simplix_trb1
//--------------------------------------------------------------------------*
void SetUpSimplix_trb1()
{
  TDriver::NBBOTS = 14;                                   // use 2*7 cars
  TDriver::MyBotName = "simplix_trb1";                    // Name of this bot 
  TDriver::ROBOT_DIR = "drivers/simplix_trb1";            // Sub path to dll
  TDriver::SECT_PRIV = "simplix private";                 // Private section
  TDriver::DEFAULTCARTYPE  = "car1-trb1";                 // Default car type
};
//==========================================================================*

//==========================================================================*
// Schismatic entry point for simplix_36GP
//--------------------------------------------------------------------------*
void SetUpSimplix_36GP()
{
  TDriver::NBBOTS = 10;                                   // use 2*5 cars
  TDriver::MyBotName = "simplix_36GP";                    // Name of this bot 
  TDriver::ROBOT_DIR = "drivers/simplix_36GP";            // Sub path to dll
  TDriver::SECT_PRIV = "simplix private";                 // Private section
  TDriver::DEFAULTCARTYPE  = "36GP-alfa12c";              // Default car type
  TDriver::AdvancedParameters = true;
  TDriver::UseBrakeLimit = true;
};
//==========================================================================*

//==========================================================================*
// Get robots's name and xml-filename
//--------------------------------------------------------------------------*
const char* getRobotNames(const char * libPath)
{
	char* c;

	int Len = MIN(BUFSIZE-2,strlen(libPath));    // Get length of path/filename of robots lib
	snprintf(pathBuffer, Len, libPath);          // Save path/filename 
	if (pathBuffer[Len-4] == '.')                // Check wether it is a windows .dll or a linux .so 
	{
      // Windows version .dll -> .xml
      pathBuffer[Len-3] = 'x';
      pathBuffer[Len-2] = 'm';
      pathBuffer[Len-1] = 'l';
      pathBuffer[Len] = 0;

	  c = &pathBuffer[Len-4];                    // Start for search of robot name 
	}
	else
	{
      // Linux version .so -> .xml
      pathBuffer[Len-2] = 'x';
      pathBuffer[Len-1] = 'm';
      pathBuffer[Len] = 'l';
      pathBuffer[Len+1] = 0;

	  c = &pathBuffer[Len-3];                    // Start for search of robot name 
	}

    // Get robot name
	int LenName = -1;
	while (*c != '/')
	{
	    LenName++;
        c--;
	}
	snprintf(nameBuffer, LenName, ++c);          // Save robot name
	GfOut("Robot Name: >%s<\n",robotName);

	return pathXml;                              
}
//==========================================================================*

//==========================================================================*
// Module entry point (new fixed name scheme).
// Extended for use with schismatic robots
//--------------------------------------------------------------------------*
extern "C" int moduleMaxInterfaces(const char * libPath)
{
	int i;

	GfOut("\n\n\nRobot LIB-Path: %s\n",libPath);
	GfOut("Robot XML-Path: %s\n\n",getRobotNames(libPath));

	// Filehandle for robot's xml-file
	void *RobotSettings = GfParmReadFile( pathXml, GFPARM_RMODE_STD );

	// Let's look what we have to provide here
	if (RobotSettings)
	{
		char SectionBuf[BUFSIZE];
		char *Section = SectionBuf;

		snprintf( SectionBuf, BUFSIZE, "%s/%s/%d", ROB_SECT_ROBOTS, ROB_LIST_INDEX, 0);

		// Try to get first driver from index 0
		const char *DriverName = GfParmGetStrNC( RobotSettings, 
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

		// Loop over all possible drivers, clear all buffers, save defined driver names and desc.
		for (i = 0; i < MAX_NBBOTS; i++)
		{
			memset(&DriverNames[i*DRIVERLEN], 0, DRIVERLEN); // Clear buffer
			memset(&DriverDescs[i*DRIVERLEN], 0, DRIVERLEN); // Clear buffer
			snprintf( SectionBuf, BUFSIZE, "%s/%s/%d", ROB_SECT_ROBOTS, ROB_LIST_INDEX, i + IndexOffset );
			const char *DriverName = GfParmGetStr( RobotSettings, Section, (char *) ROB_ATTR_NAME,undefined);

	        if (strncmp(DriverName,undefined,strlen(undefined)) != 0)
			{   // This driver is defined in robot's xml-file
				snprintf( &DriverNames[i*DRIVERLEN], DRIVERLEN-1, DriverName );
			    const char *DriverDesc = GfParmGetStr( RobotSettings, Section, (char *) ROB_ATTR_DESC, defaultBotDesc[i]);
				snprintf( &DriverDescs[i*DRIVERLEN], DRIVERLEN-1, DriverDesc );
				NBBOTS = i + 1 - IndexOffset;
			}
		}
	}
	else
	{
		// For schismatic robots NBBOTS is unknown!
		// Handle error here
		NBBOTS = 1;
	}
	GfOut("NBBOTS: %d (of %d)\n",NBBOTS,MAX_NBBOTS);

    if (strncmp(robotName,"simplix_trb1",strlen("simplix_trb1")) == 0)
		SetUpSimplix_trb1();
	else if (strncmp(robotName,"simplix_36GP",strlen("simplix_36GP")) == 0)
		SetUpSimplix_36GP();
	else 
		SetUpSimplix();

	return NBBOTS;
}
//==========================================================================*

//==========================================================================*
// Module entry point (new fixed name scheme).
// Tells TORCS, who we are, how we want to be called and 
// what we are able to do.
// Teilt TORCS mit, wer wir sind, wie wir angesprochen werden wollen und
// was wir können.
//--------------------------------------------------------------------------*
extern "C" int moduleInitialize(tModInfo *ModInfo)
{
  GfOut("\n\n\nInitialize from %s ...\n",pathXml);
  GfOut("NBBOTS: %d (of %d)\n",NBBOTS,MAX_NBBOTS);

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

  GfOut("... Initialized from %s\n\n\n",pathXml);

  return 0;
}
//==========================================================================*

//==========================================================================*
// Module exit point (new fixed name scheme).
//--------------------------------------------------------------------------*
extern "C" int moduleTerminate()
{
  GfOut("Terminated %s\n",robotName);
	
  return 0;
}
//==========================================================================*

//==========================================================================*
// Module entry point (Torcs backward compatibility scheme).
//--------------------------------------------------------------------------*
extern "C" int simplix(tModInfo *ModInfo)
{
  NBBOTS = 10;
  memset(ModInfo, 0, NBBOTS*sizeof(tModInfo));
  memset(DriverNames, 0, NBBOTS*DRIVERLEN);
  memset(DriverDescs, 0, NBBOTS*DRIVERLEN);

  snprintf(pathBuffer, BUFSIZE, "drivers/simplix/simplix.xml");
  snprintf(dirBuffer, BUFSIZE, "drivers/simplix");
  snprintf(nameBuffer, BUFSIZE, "simplix");

  // Filehandle for robot's xml-file
  void *RobotSettings = GfParmReadFile(pathXml, GFPARM_RMODE_STD );

  // Let's look what we have to provide here
  if (RobotSettings)
  {
	char SectionBuf[BUFSIZE];
	char *Section = SectionBuf;

	snprintf( SectionBuf, BUFSIZE, "%s/%s/%d", ROB_SECT_ROBOTS, ROB_LIST_INDEX, 0);

    int I;
    for (I = 0; I < NBBOTS; I++) 
    {
	  const char *DriverName = GfParmGetStr( RobotSettings, 
		  Section, (char *) ROB_ATTR_NAME, defaultBotName[I]);
	  snprintf(&DriverNames[I*DRIVERLEN], DRIVERLEN-1, DriverName);
      const char *DriverDesc = GfParmGetStr( RobotSettings, 
		  Section, (char *) ROB_ATTR_DESC, defaultBotDesc[I]);
	  snprintf(&DriverDescs[I*DRIVERLEN], DRIVERLEN-1, DriverDesc);
    }
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
// Nach Klärung der generellen Ansprache (Aufruf dieser Fkt), teilen wir
// TORCS nun noch mit, mit welchen Funktionen wir die angeforderten
// Leistungen erbringen werden:
//
// Die geforderten Leistungen müssen erbracht werden ...
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

  cRobot[Index-IndexOffset] =                    // Create a driver
	  new TDriver(Index-IndexOffset);
  cRobot[Index-IndexOffset]->SetBotName          // Store customized name
	  (&DriverNames[Index-IndexOffset*DRIVERLEN]);  

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
  cRobot[Index-IndexOffset]->SetCommonData(&gCommonData);    // Init common used data
  cRobot[Index-IndexOffset]->InitTrack(Track,CarHandle,CarParmHandle, S);
}
//==========================================================================*

//==========================================================================*
// TORCS: New Race starts
// TOCRS: Neues Rennen beginnt
//--------------------------------------------------------------------------*
static void NewRace(int Index, tCarElt* Car, tSituation *S)
{
  cTicks = 0.0;                                  // Initialize counters
  cMinTicks = FLT_MAX;                           // and time data 
  cMaxTicks = 0.0;
  cTickCount = 0;
  cLongSteps = 0;
  cCriticalSteps = 0;
  cUnusedCount = 0;
  
  cRobot[Index-IndexOffset]->NewRace(Car, S);
}
//==========================================================================*

//==========================================================================*
// TORCS-Callback: Drive
// TOCRS-Callback: Rennen fahren
//
// Attention: This procedure is called very frequent and fast in succession!
// Therefore we don't throw debug messages here!
// To find basic bugs, it may be usefull to do it anyhow!

// Achtung: Diese Prozedur wird sehr häufig und schnell nacheinander
// aufgerufen. Deshalb geben wir hier in der Regel keine Debug-Texte aus!
// Zur Fehlersuche kann das aber mal sinnvoll sein.
//--------------------------------------------------------------------------*
static void Drive(int Index, tCarElt* Car, tSituation *S)
{
  //GfOut(">>> TDriver::Drive\n");
  if (cRobot[Index-IndexOffset]->CurrSimTime != S->currentTime)
  {
    clock_t StartTicks = clock();                // Start used time 

    cRobot[Index-IndexOffset]->CurrSimTime =     // Update current time
		S->currentTime; 
    cRobot[Index-IndexOffset]->Update(Car,S);    // Update info about opp.
    if (cRobot[Index-IndexOffset]->IsStuck())    // Check if we are stuck  
  	  cRobot[Index-IndexOffset]->Unstuck();      //   Unstuck 
	else                                         // or
	  cRobot[Index-IndexOffset]->Drive();        //   Drive

	clock_t StopTicks = clock();                 // Calculate used time 
    double Duration = 1000.0 * (StopTicks - StartTicks)/CLOCKS_PER_SEC;

	if (cTickCount > 0)                          // Collect used time 
	{
	  if (Duration > 1.0)
        cLongSteps++;
	  if (Duration > 2.0)
        cCriticalSteps++;
	  if (cMinTicks > Duration)
	    cMinTicks = Duration;
	  if (cMaxTicks < Duration)
	    cMaxTicks = Duration;
	}
	cTickCount++;
  	cTicks += Duration;
  }
  else
    cUnusedCount++;
  //GfOut("<<< TDriver::Drive\n");
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
  return cRobot[Index-IndexOffset]->PitCmd();
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
    printf("EndRace\n");
  cRobot[Index-IndexOffset]->EndRace();
}
//==========================================================================*

//==========================================================================*
// TORCS: Cleanup
// TOCRS: Aufräumen
//--------------------------------------------------------------------------*
static void Shutdown(int Index)
{
  cRobot[Index-IndexOffset]->Shutdown();
  delete cRobot[Index-IndexOffset];

  GfOut("\n\nClock\n");
  GfOut("Total Time used: %g sec\n",cTicks/1000.0);
  //GfOut("Min   Time used: %g msec\n",cMinTicks);
  //GfOut("Max   Time used: %g msec\n",cMaxTicks);
  GfOut("Mean  Time used: %g msec\n",cTicks/cTickCount);
  //GfOut("Long Time Steps: %d\n",cLongSteps);
  //GfOut("Critical Steps : %d\n",cCriticalSteps);
  //GfOut("Unused Steps   : %d\n",cUnusedCount);
  
  GfOut("\n\n");
}
//==========================================================================*
/*
//==========================================================================*
// Schismatic entry point for simplix_sca
//--------------------------------------------------------------------------*
extern "C" int simplix_sca(tModInfo *ModInfo)
{
  TDriver::NBBOTS = 10;                                   // use 2*5 cars
  TDriver::MyBotName = "simplix_sca";                     // Name of this bot 
  TDriver::ROBOT_DIR = "drivers/simplix_sca";             // Sub path to dll
  TDriver::SECT_PRIV = "simplix private";                 // Private section
  TDriver::DEFAULTCARTYPE  = "sc-996";                    // Default car type

  return simplix_internal(ModInfo);
};
//==========================================================================*

//==========================================================================*
// Schismatic entry point for simplix_scb
//--------------------------------------------------------------------------*
extern "C" int simplix_scb(tModInfo *ModInfo)
{
  TDriver::NBBOTS = 10;                                   // use 2*5 cars
  TDriver::MyBotName = "simplix_scb";                     // Name of this bot 
  TDriver::ROBOT_DIR = "drivers/simplix_scb";             // Sub path to dll
  TDriver::SECT_PRIV = "simplix private";                 // Private section
  TDriver::DEFAULTCARTYPE  = "sc-996";                    // Default car type
  return simplix_internal(ModInfo);
};
//==========================================================================*

//==========================================================================*
// Schismatic entry point for simplix_36GP
//--------------------------------------------------------------------------*
extern "C" int simplix_36GP(tModInfo *ModInfo)
{
  TDriver::NBBOTS = 10;                                   // use 10 cars
  TDriver::MyBotName = "simplix_36GP";                    // Name of this bot 
  TDriver::ROBOT_DIR = "drivers/simplix_36GP";            // Sub path to dll
  TDriver::SECT_PRIV = "simplix private";                 // Private section
  TDriver::DEFAULTCARTYPE  = "36GP-alfa12c";              // Default car type
  TDriver::AdvancedParameters = true;
  TDriver::UseBrakeLimit = true;
  return simplix_internal(ModInfo);
};
//==========================================================================*

//==========================================================================*
// Schismatic entry point for my_simplix_0
//--------------------------------------------------------------------------*
extern "C" int my_simplix_0(tModInfo *ModInfo)
{
  TDriver::NBBOTS = 10;                                   // use 10 cars
  TDriver::MyBotName = "my_simplix_0";                    // Name of this bot 
  TDriver::ROBOT_DIR = "drivers/my_simplix_0";            // Sub path to dll
  TDriver::SECT_PRIV = "simplix private";                 // Private section
  TDriver::DEFAULTCARTYPE  = "car1-trb1";                 // Default car type
  TDriver::AdvancedParameters = true;
  return simplix_internal(ModInfo);
};
//==========================================================================*

//==========================================================================*
// Schismatic entry point for my_simplix_1
//--------------------------------------------------------------------------*
extern "C" int my_simplix_1(tModInfo *ModInfo)
{
  TDriver::NBBOTS = 10;                                   // use 10 cars
  TDriver::MyBotName = "my_simplix_1";                    // Name of this bot 
  TDriver::ROBOT_DIR = "drivers/my_simplix_1";            // Sub path to dll
  TDriver::SECT_PRIV = "simplix private";                 // Private section
  TDriver::DEFAULTCARTYPE  = "car1-trb1";                 // Default car type
  TDriver::AdvancedParameters = true;
  return simplix_internal(ModInfo);
};
//==========================================================================*

//==========================================================================*
// Schismatic entry point for my_simplix_2
//--------------------------------------------------------------------------*
extern "C" int my_simplix_2(tModInfo *ModInfo)
{
  TDriver::NBBOTS = 10;                                   // use 10 cars
  TDriver::MyBotName = "my_simplix_2";                    // Name of this bot 
  TDriver::ROBOT_DIR = "drivers/my_simplix_2";            // Sub path to dll
  TDriver::SECT_PRIV = "simplix private";                 // Private section
  TDriver::DEFAULTCARTYPE  = "car1-trb1";                 // Default car type
  TDriver::AdvancedParameters = true;
  return simplix_internal(ModInfo);
};
//==========================================================================*

//==========================================================================*
// Schismatic entry point for my_simplix_3
//--------------------------------------------------------------------------*
extern "C" int my_simplix_3(tModInfo *ModInfo)
{
  TDriver::NBBOTS = 10;                                   // use 10 cars
  TDriver::MyBotName = "my_simplix_3";                    // Name of this bot 
  TDriver::ROBOT_DIR = "drivers/my_simplix_3";            // Sub path to dll
  TDriver::SECT_PRIV = "simplix private";                 // Private section
  TDriver::DEFAULTCARTYPE  = "car1-trb1";                 // Default car type
  TDriver::AdvancedParameters = true;
  return simplix_internal(ModInfo);
};
//==========================================================================*

//==========================================================================*
// Schismatic entry point for my_simplix_4
//--------------------------------------------------------------------------*
extern "C" int my_simplix_4(tModInfo *ModInfo)
{
  TDriver::NBBOTS = 10;                                   // use 10 cars
  TDriver::MyBotName = "my_simplix_4";                    // Name of this bot 
  TDriver::ROBOT_DIR = "drivers/my_simplix_4";            // Sub path to dll
  TDriver::SECT_PRIV = "simplix private";                 // Private section
  TDriver::DEFAULTCARTYPE  = "car1-trb1";                 // Default car type
  TDriver::AdvancedParameters = true;
  return simplix_internal(ModInfo);
};
//==========================================================================*

//==========================================================================*
// Schismatic entry point for my_simplix_5
//--------------------------------------------------------------------------*
extern "C" int my_simplix_5(tModInfo *ModInfo)
{
  TDriver::NBBOTS = 10;                                   // use 10 cars
  TDriver::MyBotName = "my_simplix_5";                    // Name of this bot 
  TDriver::ROBOT_DIR = "drivers/my_simplix_5";            // Sub path to dll
  TDriver::SECT_PRIV = "simplix private";                 // Private section
  TDriver::DEFAULTCARTYPE  = "car1-trb1";                 // Default car type
  TDriver::AdvancedParameters = true;
  return simplix_internal(ModInfo);
};
//==========================================================================*

//==========================================================================*
// Schismatic entry point for my_simplix_6
//--------------------------------------------------------------------------*
extern "C" int my_simplix_6(tModInfo *ModInfo)
{
  TDriver::NBBOTS = 10;                                   // use 10 cars
  TDriver::MyBotName = "my_simplix_6";                    // Name of this bot 
  TDriver::ROBOT_DIR = "drivers/my_simplix_6";            // Sub path to dll
  TDriver::SECT_PRIV = "simplix private";                 // Private section
  TDriver::DEFAULTCARTYPE  = "car1-trb1";                 // Default car type
  TDriver::AdvancedParameters = true;
  return simplix_internal(ModInfo);
};
//==========================================================================*

//==========================================================================*
// Schismatic entry point for my_simplix_7
//--------------------------------------------------------------------------*
extern "C" int my_simplix_7(tModInfo *ModInfo)
{
  TDriver::NBBOTS = 10;                                   // use 10 cars
  TDriver::MyBotName = "my_simplix_7";                    // Name of this bot 
  TDriver::ROBOT_DIR = "drivers/my_simplix_7";            // Sub path to dll
  TDriver::SECT_PRIV = "simplix private";                 // Private section
  TDriver::DEFAULTCARTYPE  = "car1-trb1";                 // Default car type
  TDriver::AdvancedParameters = true;
  return simplix_internal(ModInfo);
};
//==========================================================================*

//==========================================================================*
// Schismatic entry point for my_simplix_8
//--------------------------------------------------------------------------*
extern "C" int my_simplix_8(tModInfo *ModInfo)
{
  TDriver::NBBOTS = 10;                                   // use 10 cars
  TDriver::MyBotName = "my_simplix_8";                    // Name of this bot 
  TDriver::ROBOT_DIR = "drivers/my_simplix_8";            // Sub path to dll
  TDriver::SECT_PRIV = "simplix private";                 // Private section
  TDriver::DEFAULTCARTYPE  = "car1-trb1";                 // Default car type
  TDriver::AdvancedParameters = true;
  return simplix_internal(ModInfo);
};
//==========================================================================*

//==========================================================================*
// Schismatic entry point for my_simplix_9
//--------------------------------------------------------------------------*
extern "C" int my_simplix_9(tModInfo *ModInfo)
{
  TDriver::NBBOTS = 10;                                   // use 10 cars
  TDriver::MyBotName = "my_simplix_9";                    // Name of this bot 
  TDriver::ROBOT_DIR = "drivers/my_simplix_9";            // Sub path to dll
  TDriver::SECT_PRIV = "simplix private";                 // Private section
  TDriver::DEFAULTCARTYPE  = "car1-trb1";                 // Default car type
  TDriver::AdvancedParameters = true;
  return simplix_internal(ModInfo);
};
//==========================================================================*
*/
//--------------------------------------------------------------------------*
// end of file unitmain.cpp
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
