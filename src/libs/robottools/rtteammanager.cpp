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

static tTeamManager* GlobalTeamManager; // the one and only!


// Private functions

//
// Set Header
//
tDataStructVersionHeader RtSetHeader(const int Size)
{
  tDataStructVersionHeader Header;
  Header.MajorVersion = CURRENT_MAJOR_VERSION;
  Header.MinorVersion = CURRENT_MINOR_VERSION;
  Header.Size = Size;
  return Header;
}


// Published functions:

//
// Check if Car0 is teammate of Car1
//
bool RtIsTeamMate(const CarElt* Car0, const CarElt* Car1)
{
  return strcmp(Car0->_teamname, Car1->_teamname) == 0;
}

//
// Get MajorVersion of global teammanager
//
short int RtGetMajorVersion()
{
  return CURRENT_MAJOR_VERSION;
}

//
// Get MinorVersion of global teammanager
//
short int RtGetMinorVersion()
{
  return CURRENT_MINOR_VERSION;
}

//
// Create a global team manager (allocate memory for data)
//
tTeamManager* RtTeamManager()                    
{
    tTeamManager* TeamManager = (tTeamManager*)  
		malloc(sizeof(tTeamManager));            

	TeamManager->Header = RtSetHeader(sizeof(tTeamManager));

	TeamManager->Count = 0;                      
	TeamManager->Teams = NULL;                   

	return TeamManager;
}

//
// Destroy a global team manager 's allocated data
//
void RtTeamManagerFree(tTeamManager* const TeamManager)
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
    free(TeamManager);                           
}

//
// Add a car to it's team
//
tTeam* RtTeamManagerAdd(tTeamManager* const TeamManager, CarElt* const Car, int& TeamIndex)
{
	tTeammate* NewTeammate = RtTeammate();       
	NewTeammate->Car = Car ;                     
	NewTeammate->Next = NULL;                    

	tTeam* Team = TeamManager->Teams;            
	while (Team != NULL)                         
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
	TeamManager->Count++;                        // We need a new team 
	tTeam* NewTeam = RtTeam();                   
	NewTeam->TeamName = Car->_teamname;          
	RtTeamAdd(NewTeam, NewTeammate);             // Add new teammate

	if (TeamManager->Teams == NULL)              // Add new team to
		TeamManager->Teams = NewTeam;            // linked list of
	else                                         // teams
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
// Create a team (allocate memory for data)
//
tTeam* RtTeam()                                  
{
    tTeam* Team = (tTeam*) malloc(sizeof(tTeam));
	Team->Header = RtSetHeader(sizeof(tTeam));   
	Team->TeamName = NULL;	                     
	Team->Teams = NULL;                          
	Team->Teammates = NULL;                      
	Team->Teampit = NULL;                        
	Team->PitState = PIT_IS_FREE;	             // Request for first shared pit
	Team->Count = 0;        
	Team->MinMajorVersion = INT_MAX;
    
	// Initialize all possible teammates data
	for (int I = 0; I < MAXTEAMMATES; I++)       // Loop over all
	{                                            // possible teammates 
	  Team->Cars[I] = NULL;                      
	  Team->Data[I].Size = sizeof(tTeammateData);            
	  Team->Data[I].FuelForLaps = 99;            
  	  Team->Data[I].Fuel = 99;                   
	  Team->Data[I].MinLapTime = 0;              
	  Team->Data[I].TimeBeforeNextTeammate = 0;  
      // Add additional initializations here
	}

	return Team;
}

//
// Destroy a team's allocated data
//
void RtTeamFree(tTeam* const Team)               
{
    free(Team);                                  
}

//
// Get nbr of laps all other teammates can race with current fuel
//
int RtTeamGetMinLaps(tTeam* const Team, const int TeamIndex) 
{                                                
	int L = 99;                                  // Assume unlimited
	for (int I = 0; I < MAXTEAMMATES; I++)       
	  if (I != TeamIndex)                  
		L = MIN(L,Team->Data[I].FuelForLaps);    

	return L;                                    
}

//
// Set nbr of laps this teammate can race with current fuel (old way)
//
void RtTeamSetMinLaps(tTeam* const Team, const int TeamIndex, const int FuelForLaps) 
{                                                
	Team->Data[TeamIndex].FuelForLaps = FuelForLaps;             
}

//
// Update teammates current data (new way)
//
void RtTeamUpdate(tTeam* const Team, const int TeamIndex, tTeammateData& Data) 
{
	Team->MinMajorVersion = MIN(Team->MinMajorVersion,Data.MajorVersion);
	Team->Data[TeamIndex].FuelForLaps = Data.FuelForLaps;  
	if (Data.MajorVersion < 1) // If an old robot uses it
		return;
	Team->Data[TeamIndex].Fuel = Data.Fuel;  
	Team->Data[TeamIndex].MinLapTime = Data.MinLapTime;  
	Team->Data[TeamIndex].Damages = Data.Damages;  
	Team->Data[TeamIndex].RemainingDistance = Data.RemainingDistance;
	Team->Data[TeamIndex].TimeBeforeNextTeammate = Data.TimeBeforeNextTeammate;
	if ((Team->Header.MajorVersion < 2) || (Data.MajorVersion < 2))
		return;
	// ... Add additional values here
	//if ((Team->Header.MajorVersion < 3) || (Data.MajorVersion < 3))
	//	return;
}

//
// Add a teammate to the team
//
int RtTeamAdd(tTeam* const Team, tTeammate* const NewTeammate)
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
    Team->Cars[Team->Count] = NewTeammate->Car;

	return Team->Count++;
}

//
// Allocate pit
//
bool RtTeamAllocatePit(tTeam* const Team, const int TeamIndex)
{
    if (Team->PitState == PIT_IS_FREE)
      Team->PitState = Team->Cars[TeamIndex];

    return Team->PitState == Team->Cars[TeamIndex];
}

//
// Is pit free?
//
bool RtTeamIsPitFree(tTeam* const Team, const int TeamIndex)
{
	if ((Team->Cars[TeamIndex]->_pit->pitCarIndex == TR_PIT_STATE_FREE)
      && ((Team->PitState == Team->Cars[TeamIndex]) || (Team->PitState == PIT_IS_FREE)))
	  return true;
	else
	  return false;
}

//
// Release pit
//
void RtTeamReleasePit(tTeam* const Team, const int TeamIndex)
{
    if (Team->PitState == Team->Cars[TeamIndex])
		Team->PitState = PIT_IS_FREE;
}

//
// Create a teammate (allocate memory for data)
//
tTeammate* RtTeammate()                          
{
    tTeammate* Teammate = (tTeammate*) malloc(sizeof(tTeammate));
	Teammate->Header = RtSetHeader(sizeof(tTeammate));
	Teammate->Car = NULL;                           
	Teammate->Next = NULL;                          

	return Teammate;
}

//
// Destroy a teammate's allocated data
//
void RtTeammateFree(tTeammate* const Teammate)        
{
    free(Teammate);                             
}
