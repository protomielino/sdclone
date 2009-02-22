/***************************************************************************

    file                 : rtteammanager.cpp
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
	teams being build of different robots.
	You can see an example of teammanager usage in the robots of GP36: 
	  simplix_36GP and usr_36GP.

    @author	<a href=mailto:wdb@wdbee.de>Wolf-Dieter Beelitz</a>
    @version	
    @ingroup	robottools
*/

/** @defgroup teammanager		Teammanager for robots.
    @ingroup	robottools
*/
    
#include <stdlib.h>
#include <stdio.h>
#include <cstring>
#ifdef WIN32
#include <windows.h>
#endif
#include <car.h>
#include "robottools.h"
#include "teammanager.h"

static tTeamManager* GlobalTeamManager;

//
// Check if Car0 is teammate of Car1
//
bool RtIsTeamMate(const CarElt* Car0, const CarElt* Car1)
{
  return strcmp(Car0->_teamname, Car1->_teamname) == 0;
}

//
// Create a team manager
//
tTeamManager* RtTeamManager()                    // Default constructor
{
    tTeamManager* TeamManager = (tTeamManager*)  // get memory 
		malloc(sizeof(tTeamManager));            
	TeamManager->Count = 0;                      // Nbr of Teams
	TeamManager->Teams = NULL;                   // The next team member

	return TeamManager;
}

//
// Destroy a team manager
//
void RtTeamManagerFree(tTeamManager *TeamManager)// Destructor
{
	tTeam* Team = TeamManager->Teams;
	while (Team != NULL)
	{
		tTeam* NextTeam = Team->Teams;
		tTeammate* Teammate = Team->Teammates;
		while (Teammate != NULL)
		{
			tTeammate* NextTeammate = Teammate->Next;
			RtTeammateFree(Teammate);
			Teammate = NextTeammate;
		}
        RtTeamFree(Team);
		Team = NextTeam;
	}
    free(TeamManager);                           // Release memory 
}

//
// Add a car to his team
//
tTeam* RtTeamManagerAdd(tTeamManager *TeamManager, CarElt* Car)
{
	tTeammate* NewTeammate = RtTeammate();       // Add car: new teammate 
	NewTeammate->Car = Car ;                     // Set car pointer
	NewTeammate->Next = NULL;                    // Empty list

	tTeam* Team = TeamManager->Teams;            // Get first Team;
	while (Team != NULL)                         // Loop over all teams
	{
		if (strcmp(Car->_teamname,Team->TeamName) == 0) 
		{   // If Team is cars team add car to team
			RtTeamAdd(Team, NewTeammate); 
			return Team;
		}
		else
			Team = Team->Teams;
	}

	// If the team doesn't exists yet
	tTeam* NewTeam = RtTeam();                     // Create a new team
	NewTeam->TeamName = Car->_teamname;            // Set its teamname
	RtTeamAdd(NewTeam, NewTeammate);               // Add new teammate

	if (TeamManager->Teams == NULL)                // Add new team to
		TeamManager->Teams = NewTeam;              // linked list of
	else                                           // teams
	{                                     
		Team = TeamManager->Teams;
		while (Team->Teams != NULL)       
			Team = Team->Teams;
		Team->Teams = NewTeam;
	}
	return NewTeam;
}

//
// Get a pointer to the global teammanager
//
tTeamManager* RtGetGlobalTeamManager()
{
	if (GlobalTeamManager == NULL)
		GlobalTeamManager = RtTeamManager();
	return GlobalTeamManager;
}

//
// Release the global teammanager
//
void RtFreeGlobalTeamManager()
{
	if (GlobalTeamManager != NULL)
		RtTeamManagerFree(GlobalTeamManager);

	GlobalTeamManager = NULL;
}

//
// Create a team
//
tTeam* RtTeam()                                  // Default constructor
{
    tTeam* Team = (tTeam*) malloc(sizeof(tTeam));// get memory 
	Team->Count = 0;                             // Nbr of Teammates
	Team->PitState = PIT_IS_FREE;	             // Request for shared pit
	Team->TeamName = NULL;	                     // Name of team
	Team->Teams = NULL;                          // Empty list
	Team->Teammates = NULL;                      // Empty list
	for (int I = 0; I < TR_PIT_MAXCARPERPIT; I++)// Loop over all
	{                                            // possible members 
	  Team->Cars[I] = NULL;                      // No Cars
	  Team->FuelForLaps[I] = 99;                 // Fuel still unlimited
	}

	return Team;
}

//
// Destroy a team
//
void RtTeamFree(tTeam *Team)                     // Destructor
{
    free(Team);                                  // Release memory 
}

//
// Get nbr of laps all other teammates can race with current fuel
//
int RtTeamGetMinLaps(tTeam *Team, CarElt* Car)   // Get Nbr of laps, all
{                                                //  teammates has fuel for 
	int MinL = 99;                               // Assume unlimited
	for (int I = 0; I < TR_PIT_MAXCARPERPIT; I++)// Loop over all possible
	  if (Team->Cars[I] != Car)                  // entries!
		MinL = MIN(MinL,Team->FuelForLaps[I]);   // If not self, calculate 

	return MinL;                                 // Nbr of laps all can race 
}

//
// Set nbr of laps this teammate can race with current fuel
//
void RtTeamSetMinLaps(tTeam *Team, CarElt* Car, int FuelForLaps) 
{                                                
	for (int I = 0; I < TR_PIT_MAXCARPERPIT; I++)// Loop over all teammates
	{                                            
		if (Team->Cars[I] == Car)                // If self, set value
		{
			Team->FuelForLaps[I] = FuelForLaps;             
			break;
		}
	}
}

//
// Add a teammate to the team
//
void RtTeamAdd(tTeam *Team, tTeammate* NewTeammate)
{
	tTeammate* Teammate = Team->Teammates;       
	if (Teammate == NULL)
		Team->Teammates = NewTeammate;
	else
	{
        while (Teammate->Next)                   
	        Teammate = Teammate->Next;
		Teammate->Next = NewTeammate;            
	}
    Team->Cars[Team->Count++] = NewTeammate->Car;
}

//
// Create a teammate
//
tTeammate* RtTeammate()                          // Default constructor
{
    tTeammate* Teammate = (tTeammate*) malloc(sizeof(tTeammate));
	Teammate->Car = NULL;                       // Teammates car
	Teammate->Next = NULL;                      // Empty list

	return Teammate;
}

//
// Destroy a teammate
//
void RtTeammateFree(tTeammate* Teammate)        
{
    free(Teammate);                             // Release memory 
}
