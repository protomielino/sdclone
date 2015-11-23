/***************************************************************************

    file        : raceinit.h
    created     : Sat Nov 16 12:24:26 CET 2002
    copyright   : (C) 2002 by Eric Espie
    email       : eric.espie@torcs.org   
    version     : $Id$

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
    		
    @author	<a href=mailto:torcs@free.fr>Eric Espie</a>
    @version	$Id$
*/

#ifndef _RACEINIT_H_
#define _RACEINIT_H_

class GfRaceManager;


extern void ReStartNewRace(void);
extern void ReResumeRace(void);

extern void ReReset(void);
extern void ReCleanup(void);
extern int  ReExit();

extern void ReRaceSelectRaceman(GfRaceManager* pRaceMan, bool bKeepHumans = true);
extern void ReRaceRestore(void* hparmResults);
extern void ReRaceConfigure(bool bInteractive = true);

extern int  ReInitCars(void);

extern void ReRaceCleanup(void);
extern void ReRaceCleanDrivers(void);

extern char *ReGetCurrentRaceName(void);

extern char *ReGetPrevRaceName(bool bLoop);

extern tModList *ReRacingRobotsModList;

// The race situation data structure.
// WIP: Remove this global variable that anyone can wildly change
//      and replace the read/write instruction everywhere
//      by calls to the functions of ReSituation::self().
struct RmInfo;
extern struct RmInfo *ReInfo;

// WEBSERVER class and utilus
struct MemoryStruct {
	char *memory;
	size_t size;
};
 /*
static size_t
WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
	size_t realsize = size * nmemb;
	struct MemoryStruct *mem = (struct MemoryStruct *)userp;
	
	mem->memory =(char*)realloc(mem->memory, mem->size + realsize + 1);
	if(mem->memory == NULL) {
		// out of memory!  
		printf("not enough memory (realloc returned NULL)\n");
		return 0;
	}
	
	memcpy(&(mem->memory[mem->size]), contents, realsize);
	mem->size += realsize;
	mem->memory[mem->size] = 0;
	
	return realsize;
}
*/
static size_t WriteStringCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

class WebServer {

	public:
		int readUserConfig(int userId);

		int sendGenericRequest (std::string data, std::string& serverReply);
		int sendLogin (int userId);
		int sendLap (int race_id, double laptime, double fuel, int position, int wettness);
		int sendRaceStart (int user_skill, const char *track_id, char *car_id, int type, void *setup, int startposition, const char *sdversion);
		int sendRaceEnd (int race_id, int endposition);

		int raceId;
		
		//user info
		int userId;
		int previousLaps;
		const char* username;
		const char* password;
		const char* sessionId;
		
		//config
		const char* url;
		
		//async requests
		int updateAsyncStatus();
		int addAsyncRequest(std::string const data);
		
		//curl
		//CURLM multi_handle; 
		int handle_count;
		std::string curlServerReply;
	
		//constructor
		WebServer();
	
		//destructor
		~WebServer();	
};
/*
 * from raciini.cpp
 * ReInfo->carList[carindex]
 * (elt is a car from ReInfo carlist)
 * elt->_paramsHandle => car setup params?
 * elt->_carHandle
 * line 671
 * */


#endif /* _RACEINIT_H_ */ 



