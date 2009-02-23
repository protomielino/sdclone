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

// Teammanager defines

#define CURRENT_MAJOR_VERSION 1                  // First version of teammanager
#define CURRENT_MINOR_VERSION 0                  // Without changes solong

#define PIT_IS_FREE NULL                         // = *Car if reserved/used
#define MAXTEAMMATES TR_PIT_MAXCARPERPIT

// Teammanager Utilities
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
typedef struct tTeampit
{                                                // NEVER CHANGE THIS >>> V1.X
    tDataStructVersionHeader Header;             // Version and size of this struct
	tTeampit* Teampit;                           // Linked list of pits of this team for later use
	CarElt*	PitState;                            // Request for shared pit
	                                             // <<< NEVER CHANGE THIS V1.X
	                                             // Extend it here if needed but increment MAJOR VERSION!
} tTeampit;


//
// Data of a teammate
//
typedef struct
{                                                // NEVER CHANGE THIS >>> V1.X
	short int MajorVersion;                      // Set by Robot to define updated set
	short int MinorVersion;                      // Set by Robot
	int Size;                                    // sizeof the struct including this header
	int FuelForLaps;                             // Fuel for laps 
	int Fuel;                                    // current fuel 
	int MinLapTime;                              // Lap time of fastest lap
	int Damages;                                 // Dammages of teammate
	float RemainingDistance;                     // Remaining Distance to race
	double TimeBeforeNextTeammate;               // Time in s
	                                             // <<< NEVER CHANGE THIS V1.X
	                                             // Extend it here if needed but increment MAJOR VERSION!
} tTeammateData;

//
// Data of a team
//
typedef struct tTeam
{                                                // NEVER CHANGE THIS >>> V1.X
    tDataStructVersionHeader Header;             // Version and size of this struct
	char* TeamName;	                             // Name of team
	tTeam* Teams;                                // Linked list of teams
	tTeammate* Teammates;						 // Linked list of teammates
	tTeampit* Teampit;                           // Linked list of additional pits of this team for later use
	CarElt*	PitState;                            // Request for shared pit for teams frist pit
	CarElt* Cars[MAXTEAMMATES];                  // Cars (used to identify the own team by robots)
	int Count;                                   // Nbr of Teammates
	int MinMajorVersion;                         // Min MajorVersion used by an teammates robot
	                                             // <<< NEVER CHANGE THIS V1.X
    tTeammateData Data[MAXTEAMMATES];            // Extend it here if needed but increment MAJOR VERSION!
} tTeam;                                         // and add the additional values to function RtTeamUpdate
                                                 // and initialization in function RtTeam
//
// Data of a team manager
//
typedef struct 
{                                                // NEVER CHANGE THIS >>> V1.X
    tDataStructVersionHeader Header;             // Version and size of this struct
	int Count;                                   // Nbr of Teammates
	tTeam* Teams;                                // Linked list of teams
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
// Teammanager related functions
//
extern tTeamManager* RtTeamManager();           
extern void RtTeamManagerFree(tTeamManager* const Teammanager);  
extern tTeam* RtTeamManagerAdd(tTeamManager* const TeamManager, CarElt* const Car, int& TeamIndex);
extern tTeamManager* RtGetGlobalTeamManager();   
extern void RtFreeGlobalTeamManager();


//
// Team related functions
//
extern tTeam* RtTeam();                    
extern void RtTeamFree(tTeam* const Team);
extern int RtTeamGetMinLaps(tTeam* const Team, const int TeamIndex);
extern void RtTeamSetMinLaps(tTeam* const Team, const int TeamIndex, const int FuelForLaps); 
extern void RtTeamUpdate(tTeam* const Team, const int TeamIndex, tTeammateData& Data); 
extern int RtTeamAdd(tTeam* const Team, tTeammate* const Teammate);
extern bool RtTeamAllocatePit(tTeam* const Team, const int TeamIndex);
extern bool RtTeamIsPitFree(tTeam* const Team, const int TeamIndex);
extern void RtTeamReleasePit(tTeam* const Team, const int TeamIndex);


//
// Teammate related functions
//
extern tTeammate* RtTeammate();                    
extern void RtTeammateFree(tTeammate* const Teammate);

#endif /* _TEAMMANAGER_H_ */ 



