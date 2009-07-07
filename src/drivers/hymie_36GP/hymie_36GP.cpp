/***************************************************************************

    file        : hymie_36GP.cpp
    created     : 2009
    copyright   : (C) 2009 Andrew Sumner

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
#include <portability.h>

#include "Vec3d.h"
#include "MyRobot.h"
#include "Shared.h"

static const int	NBOTS = 20;//2;
static int NBBOTS = 0;
static MyRobot		s_robot[NBOTS];
static Shared		s_shared;
static const int BUFSIZE = 256;
static const int MAXNBBOTS = 20;
static const int DRIVERLEN = 32;
static char DriverNames[DRIVERLEN * MAXNBBOTS];
static char DriverDescs[DRIVERLEN * MAXNBBOTS];  
static char nameBuffer[BUFSIZE];
static const char* robotName = nameBuffer;
static char pathBuffer[BUFSIZE]; 
static const char* pathXml = pathBuffer;
static char* defaultBotDesc[MAXNBBOTS] = {
	"driver 1",  "driver 2",  "driver 3",  "driver 4",  "driver 5",
	"driver 6",  "driver 7",  "driver 8",  "driver 9",  "driver 10", 
	"driver 11", "driver 12", "driver 13", "driver 14", "driver 15",
	"driver 16", "driver 17", "driver 18", "driver 19", "driver 20" 
};
static int indexOffset = 0;
char undefined[] = "undefined";    

static void initTrack( int index, tTrack* track, void* carHandle,
						void** carParmHandle, tSituation* s);
static void newRace( int index, tCarElt* car, tSituation* s );
static void drive( int index, tCarElt* car, tSituation* s );
static int pitcmd( int index, tCarElt* car, tSituation* s );
static void endRace( int index, tCarElt* car, tSituation* s );
static void shutdown( int index);
static int InitFuncPt( int index, void* pt );

// Set robots's name and xml file pathname
static void setRobotName(const char *name)
{
	char* c;

	strcpy(nameBuffer, name);
	snprintf(pathBuffer, BUFSIZE, "drivers/%s/%s.xml", name, name);

	GfOut("Robot Name: >%s<\n",robotName);
}

// Module entry point (new fixed name scheme).
// Extended for use with schismatic robots
extern "C" int moduleWelcome(const tModWelcomeIn* welcomeIn, tModWelcomeOut* welcomeOut)
{
	int i;

	// Save module name and loadDir, and determine module XML file pathname.
	setRobotName(welcomeIn->name);

	GfOut("Robot XML-Path: %s\n\n",pathXml);

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
            indexOffset = 0; 
		}
		else
		{
			// Teams xml file uses index 1, 2, ..., N
            indexOffset = 1; 
		}

		// Loop over all possible drivers, clear all buffers, save defined driver names and desc.
		for (i = 0; i < MAXNBBOTS; i++)
		{
			memset(&DriverNames[i*DRIVERLEN], 0, DRIVERLEN); // Clear buffer
			memset(&DriverDescs[i*DRIVERLEN], 0, DRIVERLEN); // Clear buffer
			snprintf( SectionBuf, BUFSIZE, "%s/%s/%d", ROB_SECT_ROBOTS, ROB_LIST_INDEX, i + indexOffset );
			const char *DriverName = GfParmGetStr( RobotSettings, Section, (char *) ROB_ATTR_NAME,undefined);

	        if (strncmp(DriverName,undefined,strlen(undefined)) != 0)
			{   // This driver is defined in robot's xml-file
				snprintf( &DriverNames[i*DRIVERLEN], DRIVERLEN-1, DriverName );
			    const char *DriverDesc = GfParmGetStr( RobotSettings, Section, (char *) ROB_ATTR_DESC, defaultBotDesc[i]);
				snprintf( &DriverDescs[i*DRIVERLEN], DRIVERLEN-1, DriverDesc );
				NBBOTS = i + 1;
			}
		}
	}
	else
	{
		// For schismatic robots NBBOTS is unknown!
		// Handle error here
		NBBOTS = 1;
	}
	GfOut("NBBOTS: %d (of %d)\n",NBBOTS,MAXNBBOTS);

	// SetUpHymie();

	// Set max nb of interfaces to return.
	welcomeOut->maxNbItf = NBBOTS;

	return 0;
}

// Module entry point (new fixed name scheme).
extern "C" int moduleInitialize(tModInfo *modInfo)
{
	GfOut("\n\n\nInitialize from %s ...\n",pathXml);
	GfOut("NBBOTS: %d (of %d)\n",NBBOTS,MAXNBBOTS);
	int i;

	// Clear all structures.
	memset(modInfo, 0, NBBOTS*sizeof(tModInfo));
	for (i = 0; i < NBBOTS; i++) {
		modInfo[i].name = &DriverNames[i*DRIVERLEN];
		modInfo[i].desc = &DriverDescs[i*DRIVERLEN];
		modInfo[i].fctInit = InitFuncPt;	 // Init function.
		modInfo[i].gfId    = ROB_IDENT;		 // Supported framework version.
		modInfo[i].index   = i + indexOffset;	 // Indices depending on robot's xml-file
	}
	
	GfOut("... Initialized from %s\n\n\n",pathXml);
	    
	return 0;
}

// Module entry point.
extern "C" int hymie_36GP( tModInfo* modInfo )
{
	return moduleInitialize( modInfo );
}


// Module exit point (new fixed name scheme).
extern "C" int moduleTerminate()
{
    GfOut("Terminated Hymie\n");
	
    return 0;
}

extern "C" int hymie_36GPshutdown()
{
	return moduleTerminate();
}


// Module interface initialization.
static int InitFuncPt( int index, void* pt )
{
//	GfOut( "hymie_36GP:InitFuncPt()\n" );

	tRobotItf* itf = (tRobotItf*)pt;

	// Create robot instance for index.
	itf->rbNewTrack = initTrack;	// Give the robot the track view called.
	itf->rbNewRace  = newRace;		// Start a new race.
	itf->rbDrive    = drive;		// Drive during race.
	itf->rbPitCmd   = pitcmd;		// Pit commands.
	itf->rbEndRace  = endRace;		// End of the current race.
	itf->rbShutdown = shutdown;		// Called before the module is unloaded.
	itf->index      = index;		// Index used if multiple interfaces.

//	GfOut( "  itf->rbNewTrack    %p\n", itf->rbNewTrack );
//	GfOut( "  itf->rbNewRace     %p\n", itf->rbNewRace );
//	GfOut( "  itf->rbDrive       %p\n", itf->rbDrive );
//	GfOut( "  itf->rbPitCmd      %p\n", itf->rbPitCmd );
//	GfOut( "  itf->rbEndRace     %p\n", itf->rbEndRace );
//	GfOut( "  itf->rbShutdown    %p\n", itf->rbShutdown );
//	GfOut( "  itf->index         %d\n", itf->index );

	return 0;
}


// Called for every track change or new race.
static void initTrack( int index, tTrack* track, void* carHandle,
						void** carParmHandle, tSituation* s )
{
	s_robot[index].SetShared( &s_shared );
	s_robot[index].InitTrack(index, track, carHandle, carParmHandle, s);
}


// Start a new race.
static void newRace( int index, tCarElt* car, tSituation* s )
{
	s_robot[index].NewRace( index, car, s );
}

// Drive during race.
static void drive( int index, tCarElt* car, tSituation* s )
{
	s_robot[index].Drive( index, car, s );
}


// Pitstop callback.
static int pitcmd( int index, tCarElt* car, tSituation* s )
{
	return s_robot[index].PitCmd(index, car, s);
}


// End of the current race.
static void endRace( int index, tCarElt* car, tSituation* s )
{
	s_robot[index].EndRace( index, car, s );
}


// Called before the module is unloaded.
static void shutdown( int index )
{
	s_robot[index].Shutdown( index );
}

