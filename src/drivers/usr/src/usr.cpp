/***************************************************************************

    file                 : usr.cpp
    created              : Wed Jan 8 18:31:16 CET 2003
    copyright            : (C) 2002-2004 Bernhard Wymann
    email                : berniw@bluewin.ch
    version              : $Id$

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifdef _WIN32
#include <windows.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <tgf.h>
#include <track.h>
#include <car.h>
#include <raceman.h>
#include <robottools.h>
#include <robot.h>

#include "src/drivers/usr/src/driver.h"

// Traditional TORCS Interface
static void initTrack(int index, tTrack* track, void *carHandle,
                      void **carParmHandle, tSituation *s);
static void newRace(int index, tCarElt* car, tSituation *s);
static void drive(int index, tCarElt* car, tSituation *s);
static int pitcmd(int index, tCarElt* car, tSituation *s);
static void shutdown(int index);
static int InitFuncPt(int index, void *pt);
static void endRace(int index, tCarElt *car, tSituation *s);

// Speed Dreams Interface
static const int BUFSIZE = 256;   // Buffer size for path/filename buffers
static const int MAXNBBOTS = 20;  // Set value to max capacity
static char const* defaultBotName[MAXNBBOTS] = {
  "driver 1",  "driver 2",  "driver 3",  "driver 4",  "driver 5",
  "driver 6",  "driver 7",  "driver 8",  "driver 9",  "driver 10",
  "driver 11", "driver 12", "driver 13", "driver 14", "driver 15",
  "driver 16", "driver 17", "driver 18", "driver 19", "driver 20"
};

static char const* defaultBotDesc[MAXNBBOTS] = {
  "driver 1",  "driver 2",  "driver 3",  "driver 4",  "driver 5",
  "driver 6",  "driver 7",  "driver 8",  "driver 9",  "driver 10",
  "driver 11", "driver 12", "driver 13", "driver 14", "driver 15",
  "driver 16", "driver 17", "driver 18", "driver 19", "driver 20"
};

// Max length of a drivername
static const int DRIVERLEN = 32;
// Buffer for driver's names defined in robot's xml-file
static char DriverNames[DRIVERLEN * MAXNBBOTS];
// Buffer for driver's descriptions defined in robot's xml-file
static char DriverDescs[DRIVERLEN * MAXNBBOTS];
// Array of drivers
static Driver *driver[MAXNBBOTS];

// Number of drivers defined in robot's xml-file
static int NBBOTS = 0;                           // Still unknown
// Robot's name
static char nameBuffer[BUFSIZE];                 // Buffer for robot's name
static const char* robotName = nameBuffer;       // Pointer to robot's name
// Robot's xml-filename
static char pathBuffer[BUFSIZE];                 // Buffer for xml-filename
static const char* pathXml = pathBuffer;         // Pointer to xml-filename

// Save start index offset from robot's xml file
static int indexOffset = 0;
// Marker for undefined drivers to be able to comment out drivers
// in the robot's xml-file between others, not only at the end of the list
char undefined[] = "undefined";

// Schismatic init for usr
void SetUpUSR() {
  // Add usr specific initialization here
};

// Schismatic init for usr_trb1
void SetUpUSR_trb1() {
  // Add usr_trb1 specific initialization here
};

// Schismatic init for usr_36GP
void SetUpUSR_36GP() {
  // Add usr_36GP specific initialization here
};

// Schismatic init for usr_ls1
void SetUpUSR_ls1() {
  // Add usr_ls1 specific initialization here
};

// Schismatic init for usr_sc
void SetUpUSR_sc() {
  // Add usr_sc specific initialization here
};


// Set robots's name and xml file pathname
static void setRobotName(const char *name) {
  strncpy(nameBuffer, name, BUFSIZE);
  snprintf(pathBuffer, BUFSIZE, "drivers/%s/%s.xml", name, name);

  GfOut("Robot Name: >%s<\n", robotName);
}


// Module entry point (new fixed name scheme).
// Extended for use with schismatic robots
extern "C" int moduleWelcome(const tModWelcomeIn* welcomeIn,
                              tModWelcomeOut* welcomeOut) {
  // Save module name and loadDir, and determine module XML file pathname.
  setRobotName(welcomeIn->name);

  GfOut("Robot XML-Path: %s\n\n", pathXml);

  // Filehandle for robot's xml-file
  void *RobotSettings = GfParmReadFile(pathXml, GFPARM_RMODE_STD);

  // Let's look what we have to provide here
  if (RobotSettings) {
    char SectionBuf[BUFSIZE];
    char *Section = SectionBuf;

    snprintf(SectionBuf, BUFSIZE, "%s/%s/%d",
              ROB_SECT_ROBOTS, ROB_LIST_INDEX, 0);

    // Try to get first driver from index 0
    const char *DriverName = GfParmGetStrNC(RobotSettings, Section,
              ROB_ATTR_NAME, undefined);

    // Check whether index 0 is used as start index
    if (strncmp(DriverName, undefined, strlen(undefined)) != 0) {
      // Teams xml file uses index 0, 1, ..., N - 1
      indexOffset = 0;
    } else {
      // Teams xml file uses index 1, 2, ..., N
      indexOffset = 1;
    }

    // Loop over all possible drivers, clear all buffers,
    // save defined driver names and desc.
    for (int i = 0; i < MAXNBBOTS; ++i) {
      // Clear buffers
      memset(&DriverNames[i * DRIVERLEN], 0, DRIVERLEN);
      memset(&DriverDescs[i * DRIVERLEN], 0, DRIVERLEN);

      snprintf(SectionBuf, BUFSIZE, "%s/%s/%d",
                ROB_SECT_ROBOTS, ROB_LIST_INDEX, i + indexOffset);
      const char *DriverName = GfParmGetStr(RobotSettings, Section,
                                            ROB_ATTR_NAME, undefined);

      if (strncmp(DriverName, undefined, strlen(undefined)) != 0) {
        // This driver is defined in robot's xml-file
        strncpy(&DriverNames[i * DRIVERLEN], DriverName, DRIVERLEN - 1);
        const char *DriverDesc = GfParmGetStr(RobotSettings, Section,
                                            ROB_ATTR_DESC, defaultBotDesc[i]);
        strncpy(&DriverDescs[i * DRIVERLEN], DriverDesc, DRIVERLEN - 1);
        NBBOTS = i + 1;
      }
    }
  } else {
    // For schismatic robots NBBOTS is unknown!
    // Handle error here
    NBBOTS = 1;
  }
  GfOut("NBBOTS: %d (of %d)\n", NBBOTS, MAXNBBOTS);

  SetUpUSR();

  // Set max nb of interfaces to return.
  welcomeOut->maxNbItf = NBBOTS;

  return 0;
}


// Module entry point (new fixed name scheme).
extern "C" int moduleInitialize(tModInfo *modInfo) {
  GfOut("\n\n");
  GfOut("Initialize from %s ...\n", pathXml);
  GfOut("NBBOTS: %d (of %d)\n", NBBOTS, MAXNBBOTS);

  // Clear all structures.
  memset(modInfo, 0, NBBOTS*sizeof(tModInfo));
  for (int i = 0; i < NBBOTS; ++i) {
    modInfo[i].name = &DriverNames[i * DRIVERLEN];
    modInfo[i].desc = &DriverDescs[i * DRIVERLEN];
    modInfo[i].fctInit = InitFuncPt;   // Init function.
    modInfo[i].gfId    = ROB_IDENT;    // Supported framework version.
    modInfo[i].index   = i + indexOffset;  // Indices depend on xml-file
  }

  GfOut("... Initialized from %s\n\n\n", pathXml);

  return 0;
}


// Module entry point (Torcs backward compatibility scheme).
extern "C" int usr(tModInfo *modInfo) {
  NBBOTS = 10;
  memset(DriverNames, 0, NBBOTS * DRIVERLEN);
  memset(DriverDescs, 0, NBBOTS * DRIVERLEN);

  snprintf(pathBuffer, BUFSIZE, "drivers/usr/usr.xml");
  snprintf(nameBuffer, BUFSIZE, "usr");

  // Filehandle for robot's xml-file
  void *RobotSettings = GfParmReadFile(pathXml, GFPARM_RMODE_STD);

  // Let's look what we have to provide here
  if (RobotSettings) {
    char SectionBuf[BUFSIZE];
    char *Section = SectionBuf;

    snprintf(SectionBuf, BUFSIZE, "%s/%s/%d",
              ROB_SECT_ROBOTS, ROB_LIST_INDEX, 0);

    for (int i = 0; i < NBBOTS; ++i) {
      const char *DriverName = GfParmGetStr(RobotSettings, Section,
                              ROB_ATTR_NAME, defaultBotName[i]);
      strncpy(&DriverNames[i * DRIVERLEN], DriverName, DRIVERLEN - 1);
      const char *DriverDesc = GfParmGetStr(RobotSettings, Section,
                              ROB_ATTR_DESC, defaultBotDesc[i]);
      strncpy(&DriverDescs[i * DRIVERLEN], DriverDesc, DRIVERLEN - 1);
    }
  }
  return moduleInitialize(modInfo);
}

// Module exit point (new fixed name scheme).
extern "C" int moduleTerminate() {
    GfOut("Terminated usr\n");
    return 0;
}


// Module exit point (Torcs backward compatibility scheme).
extern "C" int usr_Shut() {
  return moduleTerminate();
}


// Module interface initialization.
static int InitFuncPt(int index, void *pt) {
  tRobotItf *itf = reinterpret_cast<tRobotItf*>(pt);

  // Create robot instance for index.
  driver[index-indexOffset] = new Driver(index);
  itf->rbNewTrack = initTrack;  // Give the robot the track view called.
  itf->rbNewRace  = newRace;    // Start a new race.
  itf->rbDrive    = drive;    // Drive during race.
  itf->rbPitCmd   = pitcmd;   // Pit commands.
  itf->rbEndRace  = endRace;    // End of the current race.
  itf->rbShutdown = shutdown;   // Called before the module is unloaded.
  itf->index      = index;    // Index used if multiple interfaces.
  return 0;
}


// Called for every track change or new race.
static void initTrack(int index, tTrack* track, void *carHandle,
                      void **carParmHandle, tSituation *s) {
  driver[index-indexOffset]->initTrack(track, carHandle, carParmHandle, s);
}


// Start a new race.
static void newRace(int index, tCarElt* car, tSituation *s) {
  driver[index-indexOffset]->newRace(car, s);
}


// Drive during race.
static void drive(int index, tCarElt* car, tSituation *s) {
  driver[index-indexOffset]->drive(s);
}


// Pitstop callback.
static int pitcmd(int index, tCarElt* car, tSituation *s) {
  return driver[index-indexOffset]->pitCommand(s);
}


// End of the current race.
static void endRace(int index, tCarElt *car, tSituation *s) {
  driver[index-indexOffset]->endRace(s);
}


// Called before the module is unloaded.
static void shutdown(int index) {
  driver[index-indexOffset]->shutdown();
  delete driver[index-indexOffset];
}
