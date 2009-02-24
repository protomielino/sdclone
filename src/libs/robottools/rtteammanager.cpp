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

static tTeamManager* GlobalTeamManager = NULL;   // the one and only!
static bool PitSharing = false;                  // Is pitsharing activated?

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

//
// Check if pit sharing is activated
//
bool RtIsPitSharing(const CarElt* Car)
{
    const tTrackOwnPit* OwnPit = Car->_pit;       

	if (OwnPit == NULL)   
	{
		return false;
	}
	else
	{
		if (OwnPit->freeCarIndex > 1)
			return true;
		else
			return false;
	}
}




//
// Create a team driver (allocate memory for data)
//
tTeamDriver* RtTeamDriver()
{
	tTeamDriver* TeamDriver = (tTeamDriver*) malloc(sizeof(tTeamDriver));
	TeamDriver->Header = RtSetHeader(sizeof(tTeamDriver));   
	TeamDriver->Next = NULL;   
	TeamDriver->Count = 0;   
	TeamDriver->Car = NULL;   
	TeamDriver->Team = NULL;   
	TeamDriver->TeamPit = NULL;   

	TeamDriver->RemainingDistance = 500000;
	TeamDriver->Reserve = 2000;
	TeamDriver->FuelForLaps = 99;
	TeamDriver->MinLaps = 1;
	TeamDriver->LapsRemaining = 99;


	return TeamDriver;
};

//
// Destroy a team driver (free allocated memory)
//
void RtTeamDriverFree(tTeamDriver* TeamDriver)
{
	free(TeamDriver);
};

//
// Get TeamDriver from index
//
extern tTeamDriver* RtGetTeamDriver(const int TeamIndex)
{
	tTeamDriver* TeamDriver = GlobalTeamManager->TeamDrivers;
	for (int I = 0; I < TeamIndex; I++)
		TeamDriver = TeamDriver->Next;
	return TeamDriver;
};

//
// Add a teammate's driver to the global team manager
//
int RtTeamDriverAdd(tTeam* const Team, tTeammate* const Teammate, tTeamPit* const TeamPit)
{
	tTeamDriver* TeamDriver = GlobalTeamManager->TeamDrivers;       
	if (TeamDriver == NULL)
	{
		TeamDriver = RtTeamDriver();
		GlobalTeamManager->TeamDrivers = TeamDriver;
		TeamDriver->Car = Teammate->Car;
		TeamDriver->Team = Team;
		TeamDriver->TeamPit = TeamPit;
		TeamDriver->MinLaps = 1;
	}
	else
	{
		GlobalTeamManager->Count++;
		TeamDriver->MinLaps = GlobalTeamManager->Count;
		tTeamDriver* NewTeamDriver = RtTeamDriver();
		NewTeamDriver->Car = Teammate->Car; 
		NewTeamDriver->Team = Team;
		NewTeamDriver->TeamPit = TeamPit;
		NewTeamDriver->MinLaps = GlobalTeamManager->Count;
		while (TeamDriver->Next)
		{
			TeamDriver = TeamDriver->Next;
			TeamDriver->MinLaps = GlobalTeamManager->Count;
		}
		TeamDriver->Next = NewTeamDriver;
	}

	return GlobalTeamManager->Count; // For use as handle for the driver
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
	TeamManager->TeamDrivers = NULL;                   
	TeamManager->Track = NULL;                   
	TeamManager->RaceDistance = 500000;
	TeamManager->PitSharing = false;

	return TeamManager;
}

//
// Destroy a global team manager's allocated data
//
void RtTeamManagerFree()
{
    if (GlobalTeamManager != NULL)                           
	{
		tTeam* Team = GlobalTeamManager->Teams;
		while (Team != NULL)
		{
			tTeam* NextTeam = Team->Teams;
			RtTeamFree(Team);
			Team = NextTeam;
		}
		free(GlobalTeamManager);                           
		GlobalTeamManager = NULL;                           
	}
}

//
// Add a car to it's teams resources
//
tTeam* RtTeamManagerAddToTeam(CarElt* const Car, tTeammate* Teammate, tTeamPit** TeamPit)
{
	tTeam* Team = GlobalTeamManager->Teams;            
	while (Team != NULL)                         
	{
		if (strcmp(Car->_teamname,Team->TeamName) == 0) 
		{   // If Team is cars team add car to team
			*TeamPit = RtTeamAdd(Team, Teammate); 
			return Team;
		}
		else
			Team = Team->Teams;
	}

	// If the team doesn't exists yet
	tTeam* NewTeam = RtTeam();                   
	NewTeam->TeamName = Car->_teamname;          
	*TeamPit = RtTeamAdd(NewTeam, Teammate);      // Add new teammate

	if (GlobalTeamManager->Teams == NULL)        // Add new team to
		GlobalTeamManager->Teams = NewTeam;      // linked list of
	else                                         // teams
	{                                     
		Team = GlobalTeamManager->Teams;
		while (Team->Teams != NULL)       
			Team = Team->Teams;
		Team->Teams = NewTeam;
	}

	return NewTeam;
}


//
// Add a car to it's team and get it's TeamIndex as handle for subsequent calls
//
int RtGetTeamIndex(CarElt* const Car, tTrack* const Track, tSituation* Situation)
{
    RtInitGlobalTeamManager(); // Initializes the global team manager if needed

 	GlobalTeamManager->Track = Track;            
	GlobalTeamManager->RaceDistance = Track->length * Situation->_totLaps;            

	tTeammate* Teammate = RtTeammate();
	Teammate->Car = Car;                     

	tTeamPit* TeamPit = NULL;
	tTeam* Team = RtTeamManagerAddToTeam(Car,Teammate,&TeamPit);

	return RtTeamDriverAdd(Team, Teammate, TeamPit);
}

//
// Initialize the global teammanager
//
void RtInitGlobalTeamManager()
{
	if (GlobalTeamManager == NULL)
		GlobalTeamManager = RtTeamManager();
}

//
// Release the global teammanager
//
void RtFreeGlobalTeamManager()
{
	RtTeamManagerFree();
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
	Team->Count = 0;        
	Team->TeamPits = NULL;                        
	Team->PitCount = 0;        
	Team->MinMajorVersion = INT_MAX;

	return Team;
}

//
// Destroy a team's allocated data
//
void RtTeamFree(tTeam* const Team)               
{
	tTeamPit* TeamPit = Team->TeamPits;
	while (TeamPit != NULL)
	{
		tTeamPit* NextTeamPit = TeamPit->Next;
		RtTeamPitFree(TeamPit);
		TeamPit = NextTeamPit;
	}
    free(Team);                                  
}

//
// Get nbr of laps all other teammates using the same pit can race with current fuel
//
int RtGetMinLaps(const int TeamIndex) 
{                                                
	tTeamDriver* TeamDriver = RtGetTeamDriver(TeamIndex);
	tTeamPit* TeamPit = TeamDriver->TeamPit;

	int L = 99;                                  // Assume unlimited
	int I = 0;

	TeamDriver = GlobalTeamManager->TeamDrivers;

	while (TeamDriver)
	{
		if ((I++ != TeamIndex) && (TeamPit == TeamDriver->TeamPit))                 
			L = MIN(L,TeamDriver->FuelForLaps);    
		TeamDriver = TeamDriver->Next;
	}
	return L;                                    
}

//
// Set nbr of laps this teammate can race with current fuel
//
void RtSetMinLaps(const int TeamIndex, const int FuelForLaps) 
{                                    
	tTeamDriver* TeamDriver = RtGetTeamDriver(TeamIndex);
	TeamDriver->FuelForLaps = FuelForLaps;             
}

//
// Add a teammate to the team
//
tTeamPit* RtTeamAdd(tTeam* const Team, tTeammate* const Teammate)
{
	tTrackOwnPit* Pit = Teammate->Car->_pit;
	tTeamPit* TeamPit = Team->TeamPits;       
	if (TeamPit == NULL)
	{
		TeamPit = RtTeamPit();
		Team->TeamPits = TeamPit;
		TeamPit->Pit = Pit;
		RtTeamPitAdd(TeamPit,Teammate);
	}
	else
	{
		do
		{
			if (TeamPit->Pit == Pit)
			{
				RtTeamPitAdd(TeamPit,Teammate);
				break;
			}
			if (!TeamPit->Next)
			{
				TeamPit->Next = RtTeamPit();
				TeamPit = TeamPit->Next;
				RtTeamPitAdd(TeamPit,Teammate);
				break;
			}
		} while (TeamPit = TeamPit->Next);  
	}

	return TeamPit;
}

//
// Allocate pit
//
bool RtAllocatePit(const int TeamIndex)
{
	tTeamDriver* TeamDriver = RtGetTeamDriver(TeamIndex);

	if (TeamDriver->TeamPit->PitState == PIT_IS_FREE)
		TeamDriver->TeamPit->PitState = TeamDriver->Car;

    return (TeamDriver->TeamPit->PitState == TeamDriver->Car);
}

//
// Is pit free?
//
bool RtIsPitFree(const int TeamIndex)
{
	tTeamDriver* TeamDriver = RtGetTeamDriver(TeamIndex);

	if ((TeamDriver->Car->_pit->pitCarIndex == TR_PIT_STATE_FREE)
      && ((TeamDriver->TeamPit->PitState == TeamDriver->Car) || (TeamDriver->TeamPit->PitState == PIT_IS_FREE)))
	  return true;
	else
	  return false;

	return false;
}

//
// Release pit
//
void RtReleasePit(const int TeamIndex)
{
	tTeamDriver* TeamDriver = RtGetTeamDriver(TeamIndex);

	if (TeamDriver->TeamPit->PitState == TeamDriver->Car)
		TeamDriver->TeamPit->PitState = PIT_IS_FREE;
}


//
// Create a pit of a team (allocate memory for data)
//
tTeamPit* RtTeamPit()                                  
{
    tTeamPit* TeamPit = (tTeamPit*) malloc(sizeof(tTeamPit));
	TeamPit->Header = RtSetHeader(sizeof(tTeamPit));   
	TeamPit->Teammates = NULL;                      
	TeamPit->Next = NULL;                        
	TeamPit->PitState = PIT_IS_FREE;	             // Request for this shared pit
	TeamPit->Count = 0;        
	TeamPit->Pit = NULL;

	return TeamPit;
}

//
// Destroy a pit's data
//
void RtTeamPitFree(tTeamPit* TeamPit)
{
	tTeammate* Teammate = TeamPit->Teammates;
	while (Teammate != NULL)
	{
		tTeammate* NextTeammate = Teammate->Next;
		RtTeammateFree(Teammate);
		Teammate = NextTeammate;
	}
	free(TeamPit);
}


//
// Add a teammate to a pit of the team
//
int RtTeamPitAdd(tTeamPit* const TeamPit, tTeammate* const NewTeammate)
{
	tTeammate* Teammate = TeamPit->Teammates;       
	if (Teammate == NULL)
		TeamPit->Teammates = NewTeammate;
	else
	{
        while (Teammate->Next)                   
	        Teammate = Teammate->Next;
		Teammate->Next = NewTeammate;            
	}
    //TeamPit->Cars[TeamPit->Count] = NewTeammate->Car;

	return TeamPit->Count++;
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

//
// Get remaining distance to race
//
float RtGetRemainingDistance(const int TeamIndex)
{
	tTeamDriver* TeamDriver = RtGetTeamDriver(TeamIndex);
	return TeamDriver->RemainingDistance;
}

//
// Simple check: Is pit stop needed?
//
bool RtNeedPitStop(const int TeamIndex, float FuelPerM)
{
	tTeamDriver* TeamDriver = RtGetTeamDriver(TeamIndex);

	CarElt* Car = TeamDriver->Car; 
	if (Car->_pit == NULL)  
		return false;         
	PitSharing = RtIsPitSharing(Car);
	if (PitSharing)                        
	{                                      
		if (!RtIsPitFree(TeamIndex))   
			return false;              
	}

	float FuelConsum;     
	float FuelNeeded;     
	float TrackLength = GlobalTeamManager->Track->length;

	TeamDriver->RemainingDistance = GlobalTeamManager->RaceDistance 
		+ TeamDriver->Reserve
		- TeamDriver->Car->_distRaced
		- TrackLength * TeamDriver->Car->_lapsBehindLeader; 

	if (TeamDriver->RemainingDistance > TrackLength)
	{                                              
		if (FuelPerM == 0.0)  
			FuelConsum = 0.0008f;                      
		else                       
		FuelConsum = FuelPerM;   

		FuelNeeded =                        
			MIN(TrackLength + TeamDriver->Reserve,     
				TeamDriver->RemainingDistance + TeamDriver->Reserve) * FuelConsum;

		if (Car->_fuel < FuelNeeded)   
			return true;    
		else if (!PitSharing)           
  			return false; 
		else                            
		{                               
			FuelNeeded = FuelConsum * TrackLength ;
			int FuelForLaps = (int) (Car->_fuel / FuelNeeded - 1);        
			RtSetMinLaps(TeamIndex,FuelForLaps);  
			int MinLaps = RtGetMinLaps(TeamIndex);

			if (FuelForLaps <= MinLaps) 
			{                                      
				if ((MinLaps < TeamDriver->MinLaps)
					&& (TeamDriver->LapsRemaining > FuelForLaps))
				{                                      
					FuelNeeded = (TeamDriver->RemainingDistance + TeamDriver->Reserve) * FuelConsum;
					if (Car->_fuel < FuelNeeded) 
					return true;
				}
			}
		}
	}

	return false; 
};
