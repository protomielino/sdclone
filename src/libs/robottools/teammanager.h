/***************************************************************************

    file                 : teammanager.h
    created              : Sun Feb 22 28:43:00 CET 2009
    copyright            : (C) 2009 by Wolf-Dieter Beelitz
    email                : wdb@wdbee.de
    version              : 

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/** @file   
    This is a collection of useful functions for using a teammanager with
	teams build of different robots.
	It can handle teams with more drivers than cars per pit.
	You can see how to use in the simplix robots. 

    @author	<a href=mailto:wdb@wdbee.de>Wolf-Dieter Beelitz</a>
    @version	
    @ingroup	robottools
*/

#ifndef _TEAMMANAGER_H_
#define _TEAMMANAGER_H_

#include <car.h>
#include <track.h>                               // TR_PIT_MAXCARPERPIT = 4
#include <raceman.h>                             // tSituation

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
// Teammanager
// Robot developer API:
//

//
// Utility functions
//
bool RtIsTeamMate                                // Check wether Car0 is Teammate of Car1
	(const CarElt* Car0, const CarElt* Car1);

//
// Teammanager related functions for use by robots
//
short int RtTeamManagerGetMajorVersion();        // Get major version of used team manager data blocks
short int RtTeamManagerGetMinorVersion();        // Get minor version of used team manager data blocks

extern void RtTeamManagerShowInfo();             // Switch on team manager info output 
extern void RtTeamManagerLaps(int Laps);         // Nbr of laps to add for MinLaps 

extern bool RtTeamManagerInit();                 // Initialize team manager (is called by RtTeamManagerIndex
                                                 // and RtTeamManagerDump implicitly)

extern int RtTeamManagerIndex(                   // Add a Teammate to it's team (at NewRace)
	CarElt* const Car,                           // -> teammate's car 
	tTrack* const Track,                         // -> track
	tSituation* Situation);                      // -> situaion
                                                 // <- TeamIndex as handle for the subsequent calls

extern void RtTeamManagerRelease();              // Release team manager at Shutdown

extern void RtTeamManagerDump(int DumpMode = 0); // For tests: Dump content to console
                                                 // -> DumpMode = 2, dump allways
                                                 // -> DumpMode = 1, dump only after last driver has been added
                                                 // -> DumpMode = 0, dump only after last driver has been added if more than 1 driver is used

extern void RtTeamManagerStart();                // Start team manager, needed to start if not all robots use it 

//
// Team related functions for use by robots
//
extern bool RtTeamAllocatePit(                   // Try to allocate the pit for use of this teammate 
	const int TeamIndex);

extern bool RtTeamIsPitFree(                     // Check wether the pit to use is available
	const int TeamIndex);

extern bool RtTeamNeedPitStop(                   // Check wether this teammate should got to pit for refueling 
	const int TeamIndex,                         // (depends from the fuel of all other teammates using the same pit)
	float FuelPerM,                              // Fuel consumption per m
	int RepairWanted);                           // Damage to repair at next pitstop

extern void RtTeamReleasePit(                    // Release the pit
	const int TeamIndex);

extern int RtTeamUpdate(                         // Get nbr of laps all other teammates using the same pit can still race 
	const int TeamIndex, const int FuelForLaps); // -> Nbr of laps the driver has fuel for
                                                 // <- Min nbr of laps all other teammates using the same pit have fuel for

//
// Team driver related functions for use by robots
//
extern float RtTeamDriverRemainingDistance       // Get the remaining distance to race
	(const int TeamIndex);						 // Depends on beeing overlapped or not

//
// End of robot developer API
//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
// Implementation:
//

// Teammanager defines

#define RT_TM_CURRENT_MAJOR_VERSION 1            // First version of teammanager
#define RT_TM_CURRENT_MINOR_VERSION 0            // Without changes solong

#define RT_TM_PIT_IS_FREE NULL                   // = *Car if reserved/used

#define RT_TM_STATE_NULL 0                       // Team manager was created
#define RT_TM_STATE_INIT 1                       // Team manager was initialized
#define RT_TM_STATE_USED 2                       // Team manager was used


// Teammanager Utilities

//
// Version header
//
typedef struct tDataStructVersionHeader
{                                                // NEVER CHANGE THIS >>> 
    short int MajorVersion;                      // Changed if struct is extended 
    short int MinorVersion;                      // Changed for changes without extending the struct 
	int Size;                                    // sizeof the struct including this header
	tDataStructVersionHeader* Next;              // Linked list for garbage collection
                                                 // NEVER CHANGE THIS <<< 
} tDataStructVersionHeader;


//
// Data of a teammate
//
typedef struct tTeammate
{                                                // NEVER CHANGE THIS >>> V1.X
    tDataStructVersionHeader Header;             // Version and size of this struct
	CarElt*	Car;		                         // The car of this team member
	tTeammate* Next;	                         // The next team member
	int Count;                                   // Nbr of Teammates in this list
	                                             // <<< NEVER CHANGE THIS V1.X
	                                             // Extend it here if needed but increment MAJOR VERSION!
} tTeammate;


//
// Data of a teams pit (For later use with multiple pits per team)
//
typedef struct tTeamPit
{                                                // NEVER CHANGE THIS >>> V1.X
    tDataStructVersionHeader Header;             // Version and size of this struct
	tTeamPit* Next;                              // Linked list of pits of this team for later use
	tTeammate* Teammates;						 // Linked list of teammates of this pit
	CarElt*	PitState;                            // Request for shared pit
	tTrackOwnPit* Pit;                           // Game pit
	int Count;                                   // Nbr of TeamPits in this list
	char* Name;                                  // Name of the Teampit
	                                             // <<< NEVER CHANGE THIS V1.X
	                                             // Extend it here if needed but increment MAJOR VERSION!
} tTeamPit;

//
// Data of a team
//
typedef struct tTeam
{                                                // NEVER CHANGE THIS >>> V1.X
    tDataStructVersionHeader Header;             // Version and size of this struct
	char* TeamName;	                             // Name of team
	tTeam* Next;                                 // Linked list of teams
	tTeamPit* TeamPits;                          // Linked list of pits of this team
	int Count;                                   // Nbr of teammates
	int MinMajorVersion;                         // Min MajorVersion used by an teammates robot
	                                             // <<< NEVER CHANGE THIS V1.X
} tTeam;                                         // and add the additional values to function RtTeamUpdate
												 // and initialization in function RtTeam

//
// Data of a driver beeing in the race
//
typedef struct tTeamDriver 
{                                                // NEVER CHANGE THIS >>> V1.X
    tDataStructVersionHeader Header;             // Version and size of this struct
	tTeamDriver* Next;                           // Linked list of drivers (containig all drivers of the race)
	int Count;                                   // Nbr of drivers
	tCarElt* Car;                                // Drivers car
	tTeam* Team;                                 // Drivers Team
	tTeamPit* TeamPit;                           // Drivers Pit

	float RemainingDistance;                     // Distance still to race
	float Reserve;                               // Reserve in [m] to keep fuel for
	float MinFuel;                               // Min fuel of all other teammates using this pit
	int MinLaps;                                 // All Teammates using this pit have to be able to drive this nbr of laps 
	int FuelForLaps;                             // Driver has still fuel for this nbr of laps
	int LapsRemaining;                           // Nbr of laps still to race
	                                             // <<< NEVER CHANGE THIS V1.X
} tTeamDriver; 

//
// Data of the one and only team manager
//
typedef struct 
{                                                // NEVER CHANGE THIS >>> V1.X
    tDataStructVersionHeader Header;             // Version and size of this struct
    tDataStructVersionHeader* GarbageCollection; // Linked List of allocated memory blocks used for destruction
	tTeam* Teams;                                // Linked list of teams
	tTeamDriver* TeamDrivers;                    // Linked list of drivers belonging to a team
	tTrack* Track;                               // Track
	tTeamDriver** Drivers;                       // Array of pointers to TeamDrivers 
	int State;                                   // State of team manager
	int Count;                                   // Nbr of drivers in race
	bool PitSharing;                             // Pit sharing activated? 
	float RaceDistance;							 // Distance to race
	                                             // <<< NEVER CHANGE THIS V1.X
	                                             // Extend it here if needed but increment MAJOR VERSION!
} tTeamManager;

#endif /* _TEAMMANAGER_H_ */ 
