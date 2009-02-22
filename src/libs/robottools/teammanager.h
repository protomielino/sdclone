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
#include <track.h> // TR_PIT_MAXCARPERPIT = 4

// Teammanager defines

#define PIT_IS_FREE NULL

// Teammanager Utilities

//
// Data of a teammate
//
typedef struct tTeammate
{
	CarElt*	Car;		                         // The car of this team member
	tTeammate* Next;	                         // The next team member
} tTeammate;

//
// Data of a team
//
typedef struct tTeam
{
	int Count;                                   // Nbr of Teammates
	CarElt*	PitState;                            // Request for shared pit
	char* TeamName;	                             // Name of team
	tTeam* Teams;                                // Linked list of teams
	tTeammate* Teammates;                        // Linked list of teammates
	CarElt* Cars[TR_PIT_MAXCARPERPIT];           // Cars
	int FuelForLaps[TR_PIT_MAXCARPERPIT];        // Fuel for laps 
} tTeam;

//
// Data of a team manager
//
typedef struct 
{
	int Count;                                   // Nbr of Teammates
	tTeam* Teams;                                // Linked list of teams
} tTeamManager;

//
// Utility functions
//
bool RtIsTeamMate(const CarElt* Car0, const CarElt* Car1);

//
// Teammanager related functions
//
extern tTeamManager* RtTeamManager();           
extern void RtTeamManagerFree(tTeamManager* Teammanager);  
extern tTeam* RtTeamManagerAdd(tTeamManager *TeamManager, CarElt* Car);
extern tTeamManager* RtGetGlobalTeamManager();   
extern void RtFreeGlobalTeamManager();


//
// Team related functions
//
extern tTeam* RtTeam();                    
extern void RtTeamFree(tTeam* Team);
extern int RtTeamGetMinLaps(tTeam* Team, CarElt* Car);
extern void RtTeamSetMinLaps(tTeam *Team, CarElt* Car, int FuelForLaps); 
extern void RtTeamAdd(tTeam *Team, tTeammate* Teammate);

//
// Teammate related functions
//
extern tTeammate* RtTeammate();                    
extern void RtTeammateFree(tTeammate* Teammate);


#endif /* _TEAMMANAGER_H_ */ 



