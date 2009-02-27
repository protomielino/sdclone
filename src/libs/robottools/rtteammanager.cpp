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
	teams build of different robots.
	It can handle teams with more drivers than cars per pit.
	You can see how to use in the simplix robots. 

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

// Private variable

static tTeamManager* RtTM = NULL;                // The one and only team manager!
static bool RtTMShowInfo = false;                // Flag: Show team manager infos at console
static int RtTMLaps = 1;                         // Nbr of laps to add for MinLaps (Increase for short tracks regarding the possibility of high damages to repair)



// Private functions

//
// Set Header
//
tDataStructVersionHeader RtSetHeader(const int Size)
{
  tDataStructVersionHeader Header;
  Header.MajorVersion = RT_TM_CURRENT_MAJOR_VERSION;
  Header.MinorVersion = RT_TM_CURRENT_MINOR_VERSION;
  Header.Size = Size;
  Header.Next = RtTM->GarbageCollection;

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
    RtTM->GarbageCollection = (tDataStructVersionHeader*) TeamDriver;

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
// Get TeamDriver from index
//
extern tTeamDriver* RtTeamDriverGet(const int TeamIndex)
{
	return RtTM->Drivers[TeamIndex - 1];
};

//
// Add a teammate's driver to the global team manager
//
int RtTeamDriverAdd(tTeam* const Team, tTeammate* const Teammate, tTeamPit* const TeamPit)
{
	tTeamDriver* TeamDriver = RtTeamDriver();
	if (RtTM->TeamDrivers)
	{
		TeamDriver->Next = RtTM->TeamDrivers;
		TeamDriver->Count = 1 + RtTM->TeamDrivers->Count;
	}
	else
		TeamDriver->Count = 1;

	TeamDriver->Car = Teammate->Car;
	TeamDriver->Team = Team;
	TeamDriver->TeamPit = TeamPit;
	TeamDriver->MinLaps = TeamPit->Teammates->Count + 1;

	RtTM->TeamDrivers = TeamDriver;
	RtTM->Drivers[TeamDriver->Count - 1] = TeamDriver;

	return TeamDriver->Count; // For use as handle for the driver
}

//
// Create a global team manager (allocate memory for data)
//
tTeamManager* RtTeamManager()                   
{
    tTeamManager* TeamManager = (tTeamManager*)  
		malloc(sizeof(tTeamManager));            

	TeamManager->Header.MajorVersion = RT_TM_CURRENT_MAJOR_VERSION;
	TeamManager->Header.MinorVersion = RT_TM_CURRENT_MINOR_VERSION;
	TeamManager->Header.Size = sizeof(tTeamManager);	
	TeamManager->Header.Next = NULL;

	TeamManager->GarbageCollection = (tDataStructVersionHeader*) TeamManager;

	TeamManager->Teams = NULL;                   
	TeamManager->TeamDrivers = NULL;                   
	TeamManager->Drivers = NULL;                   
	TeamManager->Track = NULL;                   
	TeamManager->Count = 0;     
	TeamManager->PitSharing = false;
	TeamManager->RaceDistance = 500000;

	return TeamManager;
}

//
// Destroy a global team manager's allocated data
//
void RtTeamManagerFree()
{
    if (RtTM != NULL)                           
	{
		free(RtTM->Drivers);

		tDataStructVersionHeader* Block = RtTM->GarbageCollection;
		while (Block)
		{
			tDataStructVersionHeader* ToFree = Block;
			Block = Block->Next;
			free(ToFree);
		}
		RtTM = NULL;                           
	}
}

//
// Initialize the global teammanager
//
bool RtTeamManagerInit()
{
	if (RtTM == NULL)
	{
		RtTM = RtTeamManager();
		return true;
	}
	return false;
}

//
// Create a team (allocate memory for data)
//
tTeam* RtTeam()                                  
{
    tTeam* Team = (tTeam*) malloc(sizeof(tTeam));
	Team->Header = RtSetHeader(sizeof(tTeam));   
    RtTM->GarbageCollection = (tDataStructVersionHeader*) Team;

	Team->TeamName = NULL;	                     
	Team->Next = NULL;                          
	Team->Count = 0;        
	Team->TeamPits = NULL;                        
	Team->MinMajorVersion = INT_MAX;

	return Team;
}

//
// Get nbr of laps all other teammates using the same pit can race with current fuel
//
int RtTeamDriverUpdate(tTeamDriver* TeamDriver, const int FuelForLaps) 
{                                                
	TeamDriver->FuelForLaps = FuelForLaps;             
	tTeamPit* TeamPit = TeamDriver->TeamPit;

	int MinLaps = INT_MAX;                       // Assume unlimited
	float MinFuel = FLT_MAX;

	tTeamDriver* OtherTeamDriver = RtTM->TeamDrivers;

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
// Create a pit of a team (allocate memory for data)
//
tTeamPit* RtTeamPit()                                  
{
    tTeamPit* TeamPit = (tTeamPit*) malloc(sizeof(tTeamPit));
	TeamPit->Header = RtSetHeader(sizeof(tTeamPit));   
    RtTM->GarbageCollection = (tDataStructVersionHeader*) TeamPit;

	TeamPit->Teammates = NULL;                      
	TeamPit->Next = NULL;                        
	TeamPit->PitState = RT_TM_PIT_IS_FREE;	     // Request for this shared pit
	TeamPit->Count = 0;        
	TeamPit->Pit = NULL;
	TeamPit->Name = NULL;

	return TeamPit;
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
    RtTM->GarbageCollection = (tDataStructVersionHeader*) Teammate;

	Teammate->Count = 0;
	Teammate->Car = NULL;                           
	Teammate->Next = NULL;                          

	return Teammate;
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
// Add a car to it's teams resources
//
tTeam* RtTeamManagerAdd(CarElt* const Car, tTeammate* Teammate, tTeamPit** TeamPit)
{
	tTeam* Team = RtTM->Teams;            
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
	if (RtTM->Teams)                
	{
		NewTeam->Next = RtTM->Teams;
		NewTeam->Count = 1 + RtTM->Teams->Count;      
	}
	else                                         
	{                                     
		NewTeam->Count = 1;      
	}
	NewTeam->TeamName = Car->_teamname;          
	RtTM->Teams = NewTeam;

	// Add new teammate to team
	*TeamPit = RtTeamAdd(NewTeam, Teammate);     

	return NewTeam;
}




// Published functions:

//
// Get MajorVersion of global teammanager
//
short int RtTeamManagerGetMajorVersion()
{
  return RT_TM_CURRENT_MAJOR_VERSION;
}

//
// Get MinorVersion of global teammanager
//
short int RtTeamManagerGetMinorVersion()
{
  return RT_TM_CURRENT_MINOR_VERSION;
}

//
// Check if Car0 is teammate of Car1
//
bool RtIsTeamMate(const CarElt* Car0, const CarElt* Car1)
{
  return strcmp(Car0->_teamname, Car1->_teamname) == 0;
}

//
// Nbr of laps to add for MinLaps 
// (Increase for short tracks regarding the possibility of high damages to repair)
//
void RtTeamManagerLaps(const int Laps)
{
	RtTMLaps = Laps;
}

//
// Add a car to it's team and get it's TeamIndex as handle for subsequent calls
//
int RtTeamManagerIndex(CarElt* const Car, tTrack* const Track, tSituation* Situation)
{
    RtTeamManagerInit(); // Initializes the global team manager if needed
	if (RtTM->Drivers == NULL)
	{
		RtTM->Count = Situation->_ncars;
		RtTM->Drivers = 
			(tTeamDriver**) malloc(Situation->_ncars * sizeof(tTeamDriver*));
	}

 	RtTM->Track = Track;            
	RtTM->RaceDistance = Track->length * Situation->_totLaps;   

	tTeammate* Teammate = RtTeammate();
	Teammate->Car = Car;                     

	tTeamPit* TeamPit = NULL;
	tTeam* Team = RtTeamManagerAdd(Car,Teammate,&TeamPit);

	int TeamIndex = RtTeamDriverAdd(Team, Teammate, TeamPit);

	tTeamDriver* TeamDriver = RtTM->TeamDrivers;
	if (TeamDriver->Count < RtTM->Count)
		return TeamIndex;

	// Last driver is added, let's update the number of teammates
	while(TeamDriver)
	{
		TeamDriver->MinLaps = TeamDriver->TeamPit->Teammates->Count + RtTMLaps;
		TeamDriver = TeamDriver->Next;
	}
	return TeamIndex;
}

//
// Switch on teammanager info output
//
void RtTeamManagerShowInfo()
{
	RtTMShowInfo = true;
}

//
// Release the global teammanager
//
void RtTeamManagerRelease()
{
	RtTeamManagerFree();
}

//
// Get nbr of laps all other teammates using the same pit can race with current fuel
//
int RtTeamUpdate(const int TeamIndex, const int FuelForLaps) 
{                                                
	tTeamDriver* TeamDriver = RtTeamDriverGet(TeamIndex);
	return RtTeamDriverUpdate(TeamDriver,FuelForLaps);
}

//
// Allocate pit
//
bool RtTeamAllocatePit(const int TeamIndex)
{
	tTeamDriver* TeamDriver = RtTeamDriverGet(TeamIndex);

	if (TeamDriver->TeamPit->PitState == RT_TM_PIT_IS_FREE)
		TeamDriver->TeamPit->PitState = TeamDriver->Car;

    return (TeamDriver->TeamPit->PitState == TeamDriver->Car);
}

//
// Is pit free?
//
bool RtTeamIsPitFree(const int TeamIndex)
{
	tTeamDriver* TeamDriver = RtTeamDriverGet(TeamIndex);

	if ((TeamDriver->Car->_pit->pitCarIndex == TR_PIT_STATE_FREE)
      && ((TeamDriver->TeamPit->PitState == TeamDriver->Car) || (TeamDriver->TeamPit->PitState == RT_TM_PIT_IS_FREE)))
	  return true;
	else
	  return false;
}

//
// Release pit
//
void RtTeamReleasePit(const int TeamIndex)
{
	tTeamDriver* TeamDriver = RtTeamDriverGet(TeamIndex);

	if (TeamDriver->TeamPit->PitState == TeamDriver->Car)
		TeamDriver->TeamPit->PitState = RT_TM_PIT_IS_FREE;
}

//
// Simple check: Is pit stop needed?
//
bool RtTeamNeedPitStop(const int TeamIndex, const float FuelPerM, const int RepairWanted)
{
	tTeamDriver* TeamDriver = RtTeamDriverGet(TeamIndex);

	CarElt* Car = TeamDriver->Car; 
	if (Car->_pit == NULL)  
		return false;	

	bool PitSharing = RtIsPitSharing(Car);
	if (PitSharing)                        
	{                             
		if (!((Car->_pit->pitCarIndex == TR_PIT_STATE_FREE)
			&& ((TeamDriver->TeamPit->PitState == Car) || (TeamDriver->TeamPit->PitState == RT_TM_PIT_IS_FREE))))
		{
			if (RtTMShowInfo) GfOut("TM: %s pit is locked(%d)\n",Car->info.name,TeamIndex);
			return false;              
		}
	}

	float FuelConsum;     
	float FuelNeeded;     
	float TrackLength = RtTM->Track->length;
	bool GotoPit = false;

	TeamDriver->RemainingDistance = RtTM->RaceDistance 
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
			if (RtTMShowInfo) GfOut("TM: %s pitstop by fuel (%d) (%g<%g)\n",Car->info.name,TeamIndex,Car->_fuel,FuelNeeded);
			GotoPit = true;    
		}
		else if (!PitSharing)           
		{
			if (RtTMShowInfo) GfOut("TM: %s !PitSharing (%d)\n",Car->info.name,TeamIndex);
  			GotoPit = false; 
		}
		else                            
		{                               
			FuelNeeded = FuelConsum * TrackLength ;
			int FuelForLaps = (int) (Car->_fuel / FuelNeeded - 1);        
			int MinLaps = RtTeamDriverUpdate(TeamDriver,FuelForLaps);

			if (FuelForLaps < MinLaps) 
			{                                      
				if ((MinLaps < TeamDriver->MinLaps)
					&& (TeamDriver->LapsRemaining > FuelForLaps))
				{                                      
					if (RtTMShowInfo) GfOut("TM: %s pitstop by teammate (%d) (L:%d<%d<%d)\n",Car->info.name,TeamIndex,FuelForLaps,MinLaps,TeamDriver->MinLaps);
					GotoPit = true;    
				}
				else if ((MinLaps == TeamDriver->MinLaps)
					&& (Car->_fuel < TeamDriver->MinFuel)
					&& (TeamDriver->LapsRemaining > FuelForLaps))
				{                                      
					if (RtTMShowInfo) GfOut("TM: %s pitstop by teammate (%d) (L:%d(%d=%d)(F:%g<%g)\n",Car->info.name,TeamIndex,FuelForLaps,MinLaps,TeamDriver->MinLaps,Car->_fuel,TeamDriver->MinFuel);
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
				if (RtTMShowInfo) GfOut("TM: %s pitstop by damage (%d)(D:%d)\n",Car->info.name,TeamIndex,RepairWanted);
				GotoPit = true;
			}
		}
	}

	if(GotoPit)
	{
		if (TeamDriver->TeamPit->PitState == RT_TM_PIT_IS_FREE)
			TeamDriver->TeamPit->PitState = TeamDriver->Car;

		GotoPit = (TeamDriver->TeamPit->PitState == TeamDriver->Car);
	}
	return GotoPit; 
};

//
// Get remaining distance to race
//
float RtTeamDriverRemainingDistance(const int TeamIndex)
{
	tTeamDriver* TeamDriver = RtTeamDriverGet(TeamIndex);
	return TeamDriver->RemainingDistance;
}

//
// For tests: Dump all Info
//
void RtTeamManagerDump(int DumpMode)
{
	if (!RtTMShowInfo)
		return;

	if (!RtTM)
		return;

	if ((DumpMode < 2)
		&& (RtTM->TeamDrivers->Count != RtTM->Count))
		return;

	if ((DumpMode == 0)
		&& (RtTM->Count == 1))
		return;

	GfOut("\n\nTM: RtTeamManagerDump(%d) >>>\n",DumpMode);

	if (RtTeamManagerInit())
		GfOut("TM: Initialized\n");

	tTeamDriver* TeamDriver = RtTM->TeamDrivers;
	if (TeamDriver) 
		GfOut("\nTM: TeamDriver->Count: %d\n",TeamDriver->Count);
	while (TeamDriver)
	{
		GfOut("\nTM: TeamDriver %d:\n",TeamDriver->Count);
//		GfOut("TM: Header           : V%d.%d (%d Bytes): x%p\n",TeamDriver->Header.MajorVersion,TeamDriver->Header.MinorVersion,TeamDriver->Header.Size,TeamDriver);
		GfOut("TM: Name             : %s\n",TeamDriver->Car->info.name);
		GfOut("TM: FuelForLaps      : %d\n",TeamDriver->FuelForLaps);
		GfOut("TM: MinLaps          : %d\n",TeamDriver->MinLaps);
		GfOut("TM: LapsRemaining    : %d\n",TeamDriver->LapsRemaining);
		GfOut("TM: RemainingDistance: %g m\n",TeamDriver->RemainingDistance);
		GfOut("TM: Reserve          : %g m\n",TeamDriver->Reserve);
		GfOut("TM: Team->TeamName   : %s\n",TeamDriver->Team->TeamName);
//		GfOut("TM: Next             : x%p\n",TeamDriver->Next);

		TeamDriver = TeamDriver->Next;

	}

	tTeam* Team = RtTM->Teams;
	if (Team)
		GfOut("\n\nTM: Team->Count: %d\n",Team->Count);
	while (Team)
	{
		GfOut("\nTM: Team %d:\n",Team->Count);
//		GfOut("TM: Header           : V%d.%d (%d Bytes): x%p\n",Team->Header.MajorVersion,Team->Header.MinorVersion,Team->Header.Size,Team);
		GfOut("TM: Name             : %s\n",Team->TeamName);
		GfOut("TM: MinMajorVersion  : %d\n",Team->MinMajorVersion);
//		GfOut("TM: Next             : x%p\n",Team->Next);

		tTeamPit* TeamPit = Team->TeamPits;
		if (TeamPit)
			GfOut("\nTM: TeamPit.Count    : %d\n\n",TeamPit->Count);
		while (TeamPit)
		{
			GfOut("TM: TeamPit %d:\n",TeamPit->Count);
//			GfOut("TM: Header           : V%d.%d (%d Bytes): x%p\n",TeamPit->Header.MajorVersion,TeamPit->Header.MinorVersion,TeamPit->Header.Size,TeamPit);
			GfOut("TM: Name             : %s\n",TeamPit->Name);
			GfOut("TM: PitState         : %d\n",TeamPit->PitState);
			GfOut("TM: Pit              : x%p\n",TeamPit->Pit);
//			GfOut("TM: Teammates        : x%p\n",TeamPit->Teammates);
//			GfOut("TM: Next             : x%p\n",TeamPit->Next);

			tTeammate* Teammate = TeamPit->Teammates;
			if (Teammate)
				GfOut("\nTM: Teammate.Count   : %d\n\n",Teammate->Count);
			while (Teammate)
			{
				GfOut("TM: Teammate %d:\n",Teammate->Count);
//				GfOut("TM: Header           : V%d.%d (%d Bytes): x%p\n",Teammate->Header.MajorVersion,Teammate->Header.MinorVersion,Teammate->Header.Size,Teammate);
				GfOut("TM: Name             : %s\n",Teammate->Car->info.name);
//				GfOut("TM: Next             : x%p\n\n",Teammate->Next);

  				Teammate = Teammate->Next;
			}
			TeamPit = TeamPit->Next;
		}
		Team = Team->Next;

	}

	GfOut("\n\nTM: <<< RtTeamManagerDump\n\n");
}
