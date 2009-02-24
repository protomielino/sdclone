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
    		Teammanager
    @author	<a href=mailto:wdb@wdbee.de>Wolf-Dieter Beelitz</a>
    @version	
*/

#ifndef _TEAMMANAGER_H_
#define _TEAMMANAGER_H_

#include <car.h>
#include <track.h>                               // TR_PIT_MAXCARPERPIT = 4
#include <raceman.h>                             // tSituation

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
// Robot developer API:
//

//
// Teammanager related functions for use by robots
//
extern int RtGetTeamIndex(                       // Add a Teammate to it's team
	CarElt* const Car,                           // -> teammate's car 
	tTrack* const Track,                         // -> track
	tSituation* Situation);                      // -> situaion
                                                 // <- TeamIndex as handle for the subsequent calls

extern void RtFreeGlobalTeamManager();           // Free at Shutdown


//
// Team related functions for use by robots
//
extern int RtGetMinLaps(                         // Get nbr of laps all other teammates using the same pit can still race 
	const int TeamIndex);

extern void RtSetMinLaps(                        // Set the nbr of laps this teammate can still drive without refueling
	const int TeamIndex, const int FuelForLaps); 

extern float RtGetRemainingDistance(             // Get the remaining distance to race for this teammate
	const int TeamIndex);                        // (depends from being overlapped or not)

extern bool RtNeedPitStop(                       // Check wether this teammate should got to pit for refueling 
	const int TeamIndex, float FuelPerM);        // (depends from the fuel of all other teammates using the same pit)

extern bool RtAllocatePit(                       // Try to allocate the pit for use of this teammate 
	const int TeamIndex);

extern bool RtIsPitFree(                         // Check wether the pit to use is available
	const int TeamIndex);

extern void RtReleasePit(                        // Release the pit
	const int TeamIndex);

//
// End of robot developer API
//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
// Implementation:
//

// Teammanager defines

#define CURRENT_MAJOR_VERSION 1                  // First version of teammanager
#define CURRENT_MINOR_VERSION 0                  // Without changes solong

#define PIT_IS_FREE NULL                         // = *Car if reserved/used


// Teammanager Utilities

//
// Version header
//
typedef struct 
{                                                // NEVER CHANGE THIS >>> 
    short int MajorVersion;                      // Changed if struct is extended 
    short int MinorVersion;                      // Changed for changes without extending the struct 
	int Size;                                    // sizeof the struct including this header
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
	int Count;                                   // Nbr of Teammates using this pit
	CarElt*	PitState;                            // Request for shared pit
	tTrackOwnPit* Pit;                           // TORCS pit
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
	tTeam* Teams;                                // Linked list of teams
	tTeamPit* TeamPits;                          // Linked list of pits of this team
	int Count;                                   // Nbr of teammates
	int PitCount;                                // Nbr of pits of the team
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
	int FuelForLaps;                             // Driver has still fuel for this nbr of laps
	int MinLaps;                                 // All Teammates using this pit have to be able to drive this nbr of laps 
	int LapsRemaining;                           // Nbr of laps still to race
	                                             // <<< NEVER CHANGE THIS V1.X
} tTeamDriver; 

//
// Data of the one and only team manager
//
typedef struct 
{                                                // NEVER CHANGE THIS >>> V1.X
    tDataStructVersionHeader Header;             // Version and size of this struct
	int Count;                                   // Nbr of Teammates/Drivers
	tTeam* Teams;                                // Linked list of teams
	tTeamDriver* TeamDrivers;                    // Linked list of drivers belonging to a team
	tTrack* Track;                               // Track
	float RaceDistance;							 // Distance to race
	bool PitSharing;                             // Pit sharing activated? 
	                                             // <<< NEVER CHANGE THIS V1.X
	                                             // Extend it here if needed but increment MAJOR VERSION!
} tTeamManager;


//
// Utility functions
//
short int RtGetMajorVersion();
short int RtGetMinorVersion();
bool RtIsTeamMate(const CarElt* Car0, const CarElt* Car1);


//
// Teammanager related internal functions
//
extern tTeamManager* RtTeamManager();           
extern void RtTeamManagerFree();  
extern void RtInitGlobalTeamManager();           


//
// TeamDriver related internal functions
//
extern tTeamDriver* RtTeamDriver();
extern void RtTeamDriverFree(tTeamDriver* TeamDriver);
extern tTeamDriver* RtGetTeamDriver(const int TeamIndex);


//
// TeamPit related internal functions
//
extern tTeamPit* RtTeamPit();     
extern void RtTeamPitFree(tTeamPit* TeamPit);
extern int RtTeamPitAdd(tTeamPit* const TeamPit, tTeammate* const Teammate);


//
// Team related internal functions
//
extern tTeam* RtTeam();                    
extern void RtTeamFree(tTeam* const Team);
extern tTeamPit* RtTeamAdd(tTeam* const Team, tTeammate* const Teammate);


//
// Teammate related internal functions
//
extern tTeammate* RtTeammate();                    
extern void RtTeammateFree(tTeammate* const Teammate);

#endif /* _TEAMMANAGER_H_ */ 



