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
//static bool PitSharing = false;                  // Is pitsharing activated?

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
  Header.Next = GlobalTeamManager->GarbageCollection;

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
    GlobalTeamManager->GarbageCollection = (tDataStructVersionHeader*) TeamDriver;

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
	GfOut("# %s (%s) pit: %p\n",TeamDriver->Car->info.name,TeamDriver->Team->TeamName,&(TeamDriver->TeamPit));
	free(TeamDriver);
};

//
// Get TeamDriver from index
//
extern tTeamDriver* RtGetTeamDriver(const int TeamIndex)
{
	tTeamDriver* TeamDriver = GlobalTeamManager->TeamDrivers;
	while (TeamDriver)
	{
		if (TeamDriver->Count == TeamIndex)
			return TeamDriver;
		TeamDriver = TeamDriver->Next;
	}
	return NULL;
};

//
// Add a teammate's driver to the global team manager
//
int RtTeamDriverAdd(tTeam* const Team, tTeammate* const Teammate, tTeamPit* const TeamPit)
{
	tTeamDriver* TeamDriver = RtTeamDriver();
	if (GlobalTeamManager->TeamDrivers)
	{
		TeamDriver->Next = GlobalTeamManager->TeamDrivers;
		TeamDriver->Count = 1 + GlobalTeamManager->TeamDrivers->Count;
	}
	else
		TeamDriver->Count = 1;

	TeamDriver->Car = Teammate->Car;
	TeamDriver->Team = Team;
	TeamDriver->TeamPit = TeamPit;
	TeamDriver->MinLaps = TeamPit->Teammates->Count + 1;

	GlobalTeamManager->TeamDrivers = TeamDriver;
	return TeamDriver->Count; // For use as handle for the driver
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

	TeamManager->Header.MajorVersion = CURRENT_MAJOR_VERSION;
	TeamManager->Header.MinorVersion = CURRENT_MINOR_VERSION;
	TeamManager->Header.Size = sizeof(tTeamManager);	
	TeamManager->Header.Next = NULL;

	TeamManager->GarbageCollection = (tDataStructVersionHeader*) TeamManager;

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
		tDataStructVersionHeader* Block = GlobalTeamManager->GarbageCollection;
		while (Block)
		{
			tDataStructVersionHeader* ToFree = Block;
			Block = Block->Next;
			free(ToFree);
		}
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
			Team = Team->Next;
	}

	// If the team doesn't exists yet
	tTeam* NewTeam = RtTeam();                   

	// Add new team to linked list of teams
	if (GlobalTeamManager->Teams)                
	{
		NewTeam->Next = GlobalTeamManager->Teams;
		NewTeam->Count = 1 + GlobalTeamManager->Teams->Count;      
	}
	else                                         
	{                                     
		NewTeam->Count = 1;      
	}
	NewTeam->TeamName = Car->_teamname;          
	GlobalTeamManager->Teams = NewTeam;

	// Add new teammate to team
	*TeamPit = RtTeamAdd(NewTeam, Teammate);     

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
	Teammate->Count = 0;
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
    GlobalTeamManager->GarbageCollection = (tDataStructVersionHeader*) Team;

	Team->TeamName = NULL;	                     
	Team->Next = NULL;                          
	Team->Count = 0;        
	Team->TeamPits = NULL;                        
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
int RtMinLaps(tTeamDriver* TeamDriver, const int FuelForLaps) 
{                                                
	TeamDriver->FuelForLaps = FuelForLaps;             
	tTeamPit* TeamPit = TeamDriver->TeamPit;

	int MinLaps = INT_MAX;                            // Assume unlimited
	float MinFuel = FLT_MAX;

	tTeamDriver* OtherTeamDriver = GlobalTeamManager->TeamDrivers;

	while (OtherTeamDriver)
	{
		if ((OtherTeamDriver != TeamDriver) && (OtherTeamDriver->TeamPit == TeamPit))
		{
			MinLaps = MIN(MinLaps,OtherTeamDriver->FuelForLaps);
			MinFuel = MIN(MinFuel,OtherTeamDriver->Car->_fuel);
		}
		OtherTeamDriver = OtherTeamDriver->Next;
	}
	TeamDriver->MinFuel = MinFuel;
	return MinLaps;
}

//
// Get nbr of laps all other teammates using the same pit can race with current fuel
//
int RtGetMinLaps(const int TeamIndex, const int FuelForLaps) 
{                                                
	tTeamDriver* TeamDriver = RtGetTeamDriver(TeamIndex);
	return RtMinLaps(TeamDriver,FuelForLaps);
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

	while (TeamPit)
	{
		if (TeamPit->Pit == Pit)
		{
			RtTeamPitAdd(TeamPit,Teammate);
			return TeamPit;
		}
		TeamPit = TeamPit->Next;
	}

	// If there is still not a TeamPit using this pit
	TeamPit = RtTeamPit();
	if (Team->TeamPits)
	{
		TeamPit->Next = Team->TeamPits;
		TeamPit->Count = 1 + Team->TeamPits->Count;
	}
	else
	{
		TeamPit->Count = 1;
	}
	TeamPit->Pit = Pit;
	TeamPit->Name = Team->TeamName;
	Team->TeamPits = TeamPit;

	RtTeamPitAdd(TeamPit,Teammate);

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
    GlobalTeamManager->GarbageCollection = (tDataStructVersionHeader*) TeamPit;

	TeamPit->Teammates = NULL;                      
	TeamPit->Next = NULL;                        
	TeamPit->PitState = PIT_IS_FREE;	             // Request for this shared pit
	TeamPit->Count = 0;        
	TeamPit->Pit = NULL;
	TeamPit->Name = NULL;

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
	if (TeamPit->Teammates)
	{
		NewTeammate->Next = TeamPit->Teammates;
		NewTeammate->Count = 1 + TeamPit->Teammates->Count;
	}
	else
		NewTeammate->Count = 1;

	TeamPit->Teammates = NewTeammate;

	return NewTeammate->Count;
}


//
// Create a teammate (allocate memory for data)
//
tTeammate* RtTeammate()                          
{
    tTeammate* Teammate = (tTeammate*) malloc(sizeof(tTeammate));
	Teammate->Header = RtSetHeader(sizeof(tTeammate));
    GlobalTeamManager->GarbageCollection = (tDataStructVersionHeader*) Teammate;

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
bool RtNeedPitStop(const int TeamIndex, const float FuelPerM, const int RepairWanted)
{
	tTeamDriver* TeamDriver = RtGetTeamDriver(TeamIndex);

	CarElt* Car = TeamDriver->Car; 
	if (Car->_pit == NULL)  
		return false;	

	bool PitSharing = RtIsPitSharing(Car);
	if (PitSharing)                        
	{                             
//		if (!RtIsPitFree(TeamIndex))   
		if (!((Car->_pit->pitCarIndex == TR_PIT_STATE_FREE)
			&& ((TeamDriver->TeamPit->PitState == Car) || (TeamDriver->TeamPit->PitState == PIT_IS_FREE))))
		{
			GfOut("%s pit is locked(%d)\n",Car->info.name,TeamIndex);
			return false;              
		}
	}

	float FuelConsum;     
	float FuelNeeded;     
	float TrackLength = GlobalTeamManager->Track->length;
	bool GotoPit = false;

	TeamDriver->RemainingDistance = GlobalTeamManager->RaceDistance 
		+ TeamDriver->Reserve
		- Car->_distRaced
		- TrackLength * Car->_lapsBehindLeader; 

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
		{
			GfOut("%s pitstop by fuel (%d) (%g<%g)\n",Car->info.name,TeamIndex,Car->_fuel,FuelNeeded);
			GotoPit = true;    
		}
		else if (!PitSharing)           
		{
			GfOut("%s !PitSharing (%d)\n",Car->info.name,TeamIndex);
  			GotoPit = false; 
		}
		else                            
		{                               
			FuelNeeded = FuelConsum * TrackLength ;
			int FuelForLaps = (int) (Car->_fuel / FuelNeeded - 1);        
//			RtSetMinLaps(TeamIndex,FuelForLaps);  
//			int MinLaps = RtGetMinLaps(TeamIndex);
			int MinLaps = RtMinLaps(TeamDriver,FuelForLaps);

			if (FuelForLaps < MinLaps) 
			{                                      
				if ((MinLaps < TeamDriver->MinLaps)
					&& (TeamDriver->LapsRemaining > FuelForLaps))
				{                                      
					GfOut("%s pitstop by teammate (%d) (L:%d<%d<%d)\n",Car->info.name,TeamIndex,FuelForLaps,MinLaps,TeamDriver->MinLaps);
					GotoPit = true;    
				}
				else if ((MinLaps == TeamDriver->MinLaps)
					&& (Car->_fuel < TeamDriver->MinFuel)
					&& (TeamDriver->LapsRemaining > FuelForLaps))
				{                                      
					GfOut("%s pitstop by teammate (%d) (L:%d(%d=%d)(F:%g<%g)\n",Car->info.name,TeamIndex,FuelForLaps,MinLaps,TeamDriver->MinLaps,Car->_fuel,TeamDriver->MinFuel);
					GotoPit = true;    
				}
			}
		}
	}

	if (!GotoPit)
	{
		if (TeamDriver->RemainingDistance > TrackLength + 100)
		{
			if (RepairWanted > 0)          
			{
				GfOut("%s pitstop by damage (%d)(D:%d)\n",Car->info.name,TeamIndex,RepairWanted);
				GotoPit = true;
			}
		}
	}

	if(GotoPit)
	{
//		GotoPit = RtAllocatePit(TeamIndex);
		if (TeamDriver->TeamPit->PitState == PIT_IS_FREE)
			TeamDriver->TeamPit->PitState = TeamDriver->Car;

		GotoPit = (TeamDriver->TeamPit->PitState == TeamDriver->Car);
	}
	return GotoPit; 
};

//
// For tests: Dump all Info
//
void RtDumpTeammanager()
{
	GfOut("\n\n\nRtDumpTeammanager >>>\n");

	RtInitGlobalTeamManager();

	GfOut("Initialized\n");

	int I = 0;

	tTeamDriver* TeamDriver = GlobalTeamManager->TeamDrivers;
	if (TeamDriver) 
		GfOut("\nTeamDriver->Count: %d\n\n",TeamDriver->Count);
	while (TeamDriver)
	{
		GfOut("TeamDriver %d:\n",TeamDriver->Count);
		GfOut("Header           : V%d.%d (%d Bytes): x%p\n",TeamDriver->Header.MajorVersion,TeamDriver->Header.MinorVersion,TeamDriver->Header.Size,TeamDriver);
		GfOut("Name             : %s\n",TeamDriver->Car->info.name);
		GfOut("FuelForLaps      : %d\n",TeamDriver->FuelForLaps);
		GfOut("MinLaps          : %d\n",TeamDriver->MinLaps);
		GfOut("LapsRemaining    : %d\n",TeamDriver->LapsRemaining);
		GfOut("RemainingDistance: %g m\n",TeamDriver->RemainingDistance);
		GfOut("Reserve          : %g m\n",TeamDriver->Reserve);
		GfOut("Team->TeamName   : %s\n",TeamDriver->Team->TeamName);
		GfOut("Next             : x%p\n",TeamDriver->Next);

		TeamDriver = TeamDriver->Next;

	}

	I = 0;
	tTeam* Team = GlobalTeamManager->Teams;
	if (Team)
		GfOut("\nTeam->Count: %d\n\n",Team->Count);
	while (Team)
	{
		GfOut("Team %d:\n",Team->Count);
		GfOut("Header           : V%d.%d (%d Bytes): x%p\n",Team->Header.MajorVersion,Team->Header.MinorVersion,Team->Header.Size,Team);
		GfOut("Name             : %s\n",Team->TeamName);
		GfOut("MinMajorVersion  : %d\n",Team->MinMajorVersion);
		GfOut("Next             : x%p\n",Team->Next);

		tTeamPit* TeamPit = Team->TeamPits;
		if (TeamPit)
			GfOut("\nTeamPit.Count    : %d\n\n",TeamPit->Count);
		int J = 0;
		while (TeamPit)
		{
			GfOut("TeamPit %d:\n",TeamPit->Count);
			GfOut("Header           : V%d.%d (%d Bytes): x%p\n",TeamPit->Header.MajorVersion,TeamPit->Header.MinorVersion,TeamPit->Header.Size,TeamPit);
			GfOut("Name             : %s\n",TeamPit->Name);
			GfOut("PitState         : %d\n",TeamPit->PitState);
			GfOut("Pit              : x%p\n",TeamPit->Pit);
			GfOut("Teammates        : x%p\n",TeamPit->Teammates);
			GfOut("Next             : x%p\n",TeamPit->Next);

			tTeammate* Teammate = TeamPit->Teammates;
			if (Teammate)
				GfOut("\nTeammate.Count   : %d\n\n",Teammate->Count);
			int K = 0;
			while (Teammate)
			{
				GfOut("Teammate %d:\n",Teammate->Count);
				GfOut("Header           : V%d.%d (%d Bytes): x%p\n",Teammate->Header.MajorVersion,Teammate->Header.MinorVersion,Teammate->Header.Size,Teammate);
				GfOut("Name             : %s\n",Teammate->Car->info.name);
				GfOut("Next             : x%p\n\n",Teammate->Next);

  				Teammate = Teammate->Next;
			}
			TeamPit = TeamPit->Next;
		}
		Team = Team->Next;

	}

	GfOut("\n\n<<< RtDumpTeammanager\n\n\n");
}
