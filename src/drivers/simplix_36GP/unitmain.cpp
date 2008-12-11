//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
// unitmain.cpp
//--------------------------------------------------------------------------*
// TORCS: "The Open Racing Car Simulator"
// A schismatic robot for TORCS-Version 1.3.0 and 1.3.1
//--------------------------------------------------------------------------*
// Interface to TORCS
//
// File         : unitmain.cpp
// Created      : 2008.01.27
// Stand        : 2008.12.06
// Copyright    : © 2007-2008 Wolf-Dieter Beelitz
// eMail        : wdb@wdbee.de
// Version      : 2.00.000
//--------------------------------------------------------------------------*
// V2.00:
// How to use the schismatic robot:
// This robot exports more than one modul entry point routines.
// There are alias names for:
//   simplix_trb1a(tModInfo *ModInfo) (for use with Trb1-Car-Set car1-car5)
//   simplix_trb1b(tModInfo *ModInfo) (for use with Trb1-Car-Set car6-car7)
//   simplix_sca(tModInfo *ModInfo) (for use with Supecar-Set car1-car5)
//   simplix_scb(tModInfo *ModInfo) (for use with Supecar-Set car6-car10)
//   simplix_36GP(tModInfo *ModInfo) (for use with 36GP set)
//   my_simplix_1(tModInfo *ModInfo) (for user defined use)
//   ...
//   my_simplix_9(tModInfo *ModInfo) (for user defined use)
//
// Zur Nutzung der Abspaltung des Robots:
// Der Roboter exportiert mehr als eine Modul-Entry-Point routine.
// Folgende Aliasnamen sind vorhanden:
//   simplix_trb1a(tModInfo *ModInfo) (für das Trb1-Car-Set car1-car5)
//   simplix_trb1b(tModInfo *ModInfo) (für das Trb1-Car-Set car6-car7)
//   simplix_sca(tModInfo *ModInfo) (für das Supecar-Set car1-car5)
//   simplix_scb(tModInfo *ModInfo) (für das Supecar-Set car6-car10)
//   simplix_36GP(tModInfo *ModInfo) (für das 36GP-Set)
//   my_simplix_1(tModInfo *ModInfo) (Zur Anwender definierten Nutzung)
//   ...
//   my_simplix_9(tModInfo *ModInfo) (Zur Anwender definierten Nutzung)
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
// Consts, types and variables, visible outside of this unit
// Konstanten, Typen und Variablen, die außerhalb dieser Unit sichtbar sind
//--------------------------------------------------------------------------*
// Default names of the drivers/roboters
// Namen und Beschreibung der Roboter
static const char* Default_BotName[MAX_NBBOTS] = {
  "car1-trb1_0",
  "car1-trb1_1",
  "car2-trb1_0",
  "car2-trb1_1",
  "car3-trb1_0",
  "car3-trb1_1",
  "car4-trb1_0",
  "car4-trb1_1",
  "car5-trb1_0",
  "car5-trb1_1"
};
static const char* Default_BotDesc[MAX_NBBOTS] = {
  "car1-trb1_0",
  "car1-trb1_1",
  "car2-trb1_0",
  "car2-trb1_1",
  "car3-trb1_0",
  "car3-trb1_1",
  "car4-trb1_0",
  "car4-trb1_1",
  "car5-trb1_0",
  "car5-trb1_1"
};

//  Robot of this modul
//  Roboter des Moduls
static const char** BotName = Default_BotName;
static const char** BotDesc = Default_BotDesc;

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
#define BIGBUFLEN 256
static char FilenameBuffer[BIGBUFLEN];           // for path and filename
#define BUFLEN 32
static char Drivers[BUFLEN * MAX_NBBOTS];        // Driver names
static bool Footprint = false;                   // Never called yet
static int InitDriver = -1;                      // None initialized yet
//==========================================================================*

//==========================================================================*
// Prototypes of routines(functions/procedures), provided
// for communication with TORCS
// Prototypen der Routinen(Funktionen/Prozeduren), die wir für die
// Kommunikation mit TORCS bereitstellen
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
// Prepare names
// Aufbereitung der Namen
//--------------------------------------------------------------------------*
bool Prepare()
{
  InitDriver++;       // Count initialized drivers
  if (Footprint)      // Check wether we have done it before
    return false;     //   If so, return false,
  else
    Footprint = true; // else set flag

  // Initialize the base param path.
  //char* BaseParamPath = TDriver::ROBOT_DIR;      // Depends on robots name
  char* PathFilename = FilenameBuffer;           // Pointer to buffer

  memset(&Drivers[0],0,BUFLEN * TDriver::NBBOTS);// Clear buffer

  snprintf(FilenameBuffer,BIGBUFLEN,             // Build path to
    "drivers/%s/%s.xml"                          // own robot from
	,TDriver::MyBotName,TDriver::MyBotName);     // name of robot

  void* RobotSettings = GfParmReadFile           // Open team setup file
    (PathFilename,GFPARM_RMODE_STD);

  if (RobotSettings)                             // If file opened
  {
    char SectionBuffer[256];                     // Buffer for section name
    char* Section = SectionBuffer;               // Adjust Pointer

    for (int I = 0; I < TDriver::NBBOTS; I++)    // Loop all drivers
    {
      snprintf(SectionBuffer,BUFLEN,             // Build name of
        "%s/%s/%d"                               // section from
	    ,ROB_SECT_ROBOTS,ROB_LIST_INDEX,I);      // Index of driver

	  const char* DriverName = GfParmGetStr      // Get pointer to
        (RobotSettings                           // drivers name
        , Section                                // defined in corresponding
        , (char *) ROB_ATTR_NAME                 // section,
        , (char *) BotName[I]);                  // BotName[I] as default

	  snprintf(&Drivers[I*BUFLEN],BUFLEN-1,DriverName);
    }
  }
  else
  { // This should never happen! But give user a chance to read it!
	GfOut("\n\n\n FATAL ERROR: File '%s' not found\n\n",PathFilename);
	for (int I = 0; I < TDriver::NBBOTS; I++)
      snprintf(&Drivers[I*BUFLEN],BUFLEN-1,BotName[I]);
  }
  return Footprint;
};
//==========================================================================*

//==========================================================================*
// Get name of driver/robot
// Namen des Fahrers/Roboters holen
//--------------------------------------------------------------------------*
char* GetBotName(int Index)
{
  return &Drivers[Index*BUFLEN];
};
//==========================================================================*

//==========================================================================*
// Tells TORCS, who we are, how we want to be called and
// what we are able to do.
// Teilt TORCS mit, wer wir sind, wie wir angesprochen werden wollen und
// was wir können.
//--------------------------------------------------------------------------*
extern "C" int simplix(tModInfo *ModInfo)
{
  if (Prepare()) // Check Footprint and prepare names
  { // Run once only: Clear memory provided
    memset(ModInfo, 0, 10 * sizeof(tModInfo));
  }

  int I;
  for (I = 0; I < TDriver::NBBOTS; I++)
  {
    ModInfo[I].name    = GetBotName(I);          // Tell customisable name
    ModInfo[I].desc    = (char *) BotDesc[I];    // Tell customisable desc.
    ModInfo[I].fctInit = InitFuncPt;             // Common used functions
    ModInfo[I].gfId    = ROB_IDENT;              // Robot identity
    ModInfo[I].index   = I;                      // Drivers index
  }
  return 0;
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

  cRobot[Index] = new TDriver(Index);            // Create a driver
  cRobot[Index]->SetBotName(GetBotName(Index));  // Store customized name

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
  cRobot[Index]->SetCommonData(&gCommonData);    // Init common used data
  cRobot[Index]->InitTrack(Track,CarHandle,CarParmHandle, S);
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

  cRobot[Index]->NewRace(Car, S);
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
  if (cRobot[Index]->CurrSimTime != S->currentTime)
  {
    clock_t StartTicks = clock();                // Start used time

    cRobot[Index]->CurrSimTime = S->currentTime; // Update current time
    cRobot[Index]->Update(Car,S);                // Update info about opp.
    if (cRobot[Index]->IsStuck())                // Check if we are stuck
  	  cRobot[Index]->Unstuck();                  //   Unstuck
	else                                         // or
	  cRobot[Index]->Drive();                    //   Drive

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
  return cRobot[Index]->PitCmd();
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
  cRobot[Index]->EndRace();
}
//==========================================================================*

//==========================================================================*
// TORCS: Cleanup
// TOCRS: Aufräumen
//--------------------------------------------------------------------------*
static void Shutdown(int Index)
{
  cRobot[Index]->Shutdown();
  delete cRobot[Index];

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

//==========================================================================*
// Schismatic entry point for simplix_trb1a
//--------------------------------------------------------------------------*
extern "C" int simplix_trb1a(tModInfo *ModInfo)
{
  TDriver::NBBOTS = 10;                                   // use 2*5 cars
  TDriver::MyBotName = "simplix_trb1a";                   // Name of this bot
  TDriver::ROBOT_DIR = "drivers/simplix_trb1a";           // Sub path to dll
  TDriver::SECT_PRIV = "simplix private";                 // Private section
  TDriver::DEFAULTCARTYPE  = "car1-trb1";                 // Default car type

  return simplix(ModInfo);
};
//==========================================================================*

//==========================================================================*
// Schismatic entry point for simplix_trb1b
//--------------------------------------------------------------------------*
extern "C" int simplix_trb1b(tModInfo *ModInfo)
{
  TDriver::NBBOTS = 4;                                    // use 2*2 cars
  TDriver::MyBotName = "simplix_trb1b";                   // Name of this bot
  TDriver::ROBOT_DIR = "drivers/simplix_trb1b";           // Sub path to dll
  TDriver::SECT_PRIV = "simplix private";                 // Private section
  TDriver::DEFAULTCARTYPE  = "car1-trb1";                 // Default car type
  return simplix(ModInfo);
};
//==========================================================================*

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

  return simplix(ModInfo);
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
  return simplix(ModInfo);
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
  return simplix(ModInfo);
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
  return simplix(ModInfo);
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
  return simplix(ModInfo);
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
  return simplix(ModInfo);
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
  return simplix(ModInfo);
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
  return simplix(ModInfo);
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
  return simplix(ModInfo);
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
  return simplix(ModInfo);
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
  return simplix(ModInfo);
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
  return simplix(ModInfo);
};
//==========================================================================*

//--------------------------------------------------------------------------*
// end of file unitmain.cpp
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
