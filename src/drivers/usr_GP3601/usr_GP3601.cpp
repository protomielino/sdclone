/***************************************************************************

    file                 : usr_GP3601.cpp
    created              : Wed Jan 8 18:31:16 CET 2003
    copyright            : (C) 2002-2004 Bernhard Wymann
    email                : berniw@bluewin.ch
    version              : $Id: usr_GP3601.cpp,v 1.1 2008/02/11 00:53:10 andrew Exp $

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

#include "driver.h"

#define NBBOTS 10

static char* Default_botname[NBBOTS] = {"usr_GP3601 1", "usr_GP3601 2", "usr_GP3601 3", "usr_GP3601 4", "usr_GP3601 5",
								"usr_GP3601 6", "usr_GP3601 7", "usr_GP3601 8", "usr_GP3601 9", "usr_GP3601 10"};
#define DRIVERLEN 32
static char DriverNames[DRIVERLEN * NBBOTS];
static char* botdesc[NBBOTS] = {"usr_GP3601 1", "usr_GP3601 2", "usr_GP3601 3", "usr_GP3601 4", "usr_GP3601 5",
								"usr_GP3601 6", "usr_GP3601 7", "usr_GP3601 8", "usr_GP3601 9", "usr_GP3601 10"};

static Driver *driver[NBBOTS];

static void initTrack(int index, tTrack* track, void *carHandle, void **carParmHandle, tSituation *s);
static void newRace(int index, tCarElt* car, tSituation *s);
static void drive(int index, tCarElt* car, tSituation *s);
static int pitcmd(int index, tCarElt* car, tSituation *s);
static void shutdown(int index);
static int InitFuncPt(int index, void *pt);
static void endRace(int index, tCarElt *car, tSituation *s);


// Module entry point.
extern "C" int usr_GP3601(tModInfo *modInfo)
{
	int i;
	
	// Clear all structures.
	memset(modInfo, 0, 10*sizeof(tModInfo));
	memset(&DriverNames[0], 0, DRIVERLEN * NBBOTS);

	for (i = 0; i < NBBOTS; i++) {
		modInfo[i].name    = Default_botname[i];  			// name of the module (short).
		modInfo[i].desc    = botdesc[i];			// Description of the module (can be long).
		modInfo[i].fctInit = InitFuncPt;			// Init function.
		modInfo[i].gfId    = ROB_IDENT;				// Supported framework version.
		modInfo[i].index   = i+1;						// Indices from 0 to 9.
	}

	char path[256];
	snprintf(path, 255, "drivers/usr_GP3601/usr_GP3601.xml");

	void *RobotSettings = GfParmReadFile( path, GFPARM_RMODE_STD );

	if (RobotSettings)
	{
		char SectionBuf[256];
		char *Section = SectionBuf;

		for (i = 0; i < NBBOTS; i++)
		{
			snprintf( SectionBuf, DRIVERLEN, "%s/%s/%d", ROB_SECT_ROBOTS, ROB_LIST_INDEX, i+1 );
			const char *DriverName = GfParmGetStr( RobotSettings, Section, (char *) ROB_ATTR_NAME, (char *) Default_botname[i] );
			snprintf( &DriverNames[i*DRIVERLEN], DRIVERLEN-1, DriverName );

			modInfo[i].name = &DriverNames[i*DRIVERLEN];
		}
	}

	return 0;
}


// Module interface initialization.
static int InitFuncPt(int index, void *pt)
{
	tRobotItf *itf = (tRobotItf *)pt;

	// Create robot instance for index.
	driver[index-1] = new Driver(index);
	itf->rbNewTrack = initTrack;	// Give the robot the track view called.
	itf->rbNewRace  = newRace;		// Start a new race.
	itf->rbDrive    = drive;		// Drive during race.
	itf->rbPitCmd   = pitcmd;		// Pit commands.
	itf->rbEndRace  = endRace;		// End of the current race.
	itf->rbShutdown = shutdown;		// Called before the module is unloaded.
	itf->index      = index;		// Index used if multiple interfaces.
	return 0;
}


// Called for every track change or new race.
static void initTrack(int index, tTrack* track, void *carHandle, void **carParmHandle, tSituation *s)
{
	driver[index-1]->initTrack(track, carHandle, carParmHandle, s);
}


// Start a new race.
static void newRace(int index, tCarElt* car, tSituation *s)
{
	driver[index-1]->newRace(car, s);
}


// Drive during race.
static void drive(int index, tCarElt* car, tSituation *s)
{
	driver[index-1]->drive(s);
}


// Pitstop callback.
static int pitcmd(int index, tCarElt* car, tSituation *s)
{
	return driver[index-1]->pitCommand(s);
}


// End of the current race.
static void endRace(int index, tCarElt *car, tSituation *s)
{
	driver[index-1]->endRace(s);
}


// Called before the module is unloaded.
static void shutdown(int index)
{
	delete driver[index-1];
}

