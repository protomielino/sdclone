/***************************************************************************

    file        : raceinit.cpp
    created     : Sat Nov 16 10:34:35 CET 2002
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
        Race initialization routines
    @author <a href=mailto:eric.espie@torcs.org>Eric Espie</a>
    @version  $Id$
*/

#include <cstdlib>
#include <cstdio>
#include <string>
#include <sstream>
#include <map>
#include <ctime>
#include <iostream>

#ifdef THIRD_PARTY_SQLITE3
#include <sqlite3.h>
#endif

#include <raceman.h>
#include <robot.h>
#include <teammanager.h>
#include <robottools.h>
#include <replay.h>

#include <portability.h>
#include <tgf.hpp>

#include <racemanagers.h>
#include <race.h>

#include "standardgame.h"

#include "racesituation.h"
#include "racemain.h"
#include "raceupdate.h"
#include "racestate.h"
#include "raceresults.h"
#include "racecareer.h"

#include "raceinit.h"

//webserver requirements
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sstream>
//#include <curl/curl.h>
#include <curl/multi.h>
#include <playerpref.h>

//webserver utility
//replace
void replaceAll(std::string& str, const std::string& from, const std::string& to) {
    if(from.empty())
        return;
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
    }
}
//to string (from c++11)
template <typename T>
std::string to_string(T value)
{
	std::ostringstream os ;
	os << value ;
	return os.str() ;
}

CURLM* multi_handle; 
extern std::vector<std::string> msglist;
extern int webserverState;

/* START webserver*/
WebServer::WebServer(){
	void *configHandle;
	char configFileUrl[256];
	
	//initialize some curl var
	multi_handle = curl_multi_init();
	this->handle_count = 0;

	//get the preferencies file location
	sprintf(configFileUrl, "%s%s", GfLocalDir(), "config/webserver.xml");

	//read the preferencies file
	configHandle = GfParmReadFile(configFileUrl, GFPARM_RMODE_REREAD);

	//get webServer url from the config
	this->url = GfParmGetStr(configHandle, "WebServer Settings", "url","val");

	GfLogInfo("WebServer - webserver url is: %s\n", this->url);

}
WebServer::~WebServer(){
	//cleanup curl
	curl_multi_cleanup(multi_handle);
}
int WebServer::updateAsyncStatus(){

	//perform the pending requests
	curl_multi_perform(multi_handle, &this->handle_count);

	if( this->handle_count>0){
		GfLogInfo("############################# ASYNC WAITING UPDATES: %i\n", this->handle_count);
		//display some UI to the user to inform him we are waiting a reply from the server
webserverState=2;
	}else{
webserverState=0;
	}
	
	CURLMsg *msg;
	CURL *eh=NULL;
	CURLcode return_code;
	int http_status_code;
	const char *szUrl;
	
	while ((msg = curl_multi_info_read(multi_handle, &this->handle_count))) {
		if (msg->msg == CURLMSG_DONE) {
			eh = msg->easy_handle;

			return_code = msg->data.result;
			if(return_code!=CURLE_OK) {
				fprintf(stderr, "CURL error code: %d\n", msg->data.result);
				
				//something went wrong. anyway we are no more busy
				webserverState=0;

				continue;
			}

			// Get HTTP status code
			http_status_code=0;
			szUrl = NULL;

			curl_easy_getinfo(eh, CURLINFO_RESPONSE_CODE, &http_status_code);
			//curl_easy_getinfo(eh, CURLINFO_PRIVATE, &szUrl);
			curl_easy_getinfo(eh, CURLINFO_EFFECTIVE_URL, &szUrl);

			//the server replyed
			if(http_status_code==200) {
				printf("200 OK for %s\n", szUrl);

				GfLogInfo("############################# ASYNC SERVER REPLY:\n%s\n", this->curlServerReply.c_str());
				//manage server replyes...
				
				//read the xml reply of the server
				void *xmlReply;
				xmlReply = GfParmReadBuf(const_cast<char*>(this->curlServerReply.c_str()));

				//the server want we display something?
				//todo: if more than one messagge only the last one is show to the user...
//GfParmListSeekFirst
//GfParmListSeekNext
				if(GfParmExistsSection(xmlReply, "content/reply/messages")){
					int msgsCount = GfParmGetNum(xmlReply, "content/reply/messages", "number", "null",0);
					if( msgsCount > 0 ){
						for( int dispatchedMsgs = 0; dispatchedMsgs < msgsCount; dispatchedMsgs = dispatchedMsgs + 1 )
						   {
							std::string msgTag = "message";
							msgTag.append(to_string(dispatchedMsgs));
							GfLogInfo("n\%s\n", msgTag.c_str());
							
							//ReSituation::self().setRaceMessage(GfParmGetStr(xmlReply, "content/reply/messages", msgTag.c_str(), "null"),5, false);
							msglist.push_back(GfParmGetStr(xmlReply, "content/reply/messages", msgTag.c_str(), "null"));
						   }
					}
				}

				//race reply
				//store the webServer assigned race id
				if(GfParmExistsSection(xmlReply, "content/reply/races")){
					if(GfParmGetNum(xmlReply, "content/reply/races", "id", "null", 0) != 0){
						this->raceId = (int)GfParmGetNum(xmlReply, "content/reply/races", "id", "null", 0);
						GfLogInfo("WebServer - assigned race id is: %i\n", this->raceId);
						msglist.push_back("Webserver assigned a raceid");
					}
				}

				//empty the string
				this->curlServerReply.clear();
			} else {
				fprintf(stderr, "GET of %s returned http status code %d\n", szUrl, http_status_code);
			}
			curl_multi_remove_handle(multi_handle, eh);
			curl_easy_cleanup(eh);
			/* then cleanup the formpost chain */ 
			//curl_formfree(formpost);
		}
		else {
			fprintf(stderr, "error: after curl_multi_info_read(), CURLMsg=%d\n", msg->msg);
		}
	} 	

	return 0;
}

int WebServer::addAsyncRequest(std::string const data){
	GfLogInfo("############################# ADD ASYNC REQUEST:\n %s \n", data.c_str());

	CURL* curl = NULL;
	struct curl_httppost *formpost=NULL;
	struct curl_httppost *lastptr=NULL;
	//struct MemoryStruct chunk;
	//
	curl = curl_easy_init();
	if(curl) {
		
		curl_easy_setopt(curl, CURLOPT_URL, this->url);
		//curl_easy_setopt(curl, CURLOPT_URL, testurl);
		
		// send all data to this function
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteStringCallback);
		
		//pass our std::string to the WriteStringCallback functions so it can write into it
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &this->curlServerReply);
		
		// some servers don't like requests that are made without a user-agent
		// field, so we provide one 
		curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");
		
		//prepare the form-post to be sent to the server
		curl_formadd(&formpost,
				   &lastptr,
				   CURLFORM_COPYNAME, "data", //the field name where the data will be stored
				   CURLFORM_COPYCONTENTS, data.c_str(), //the actual data
				   CURLFORM_END);
				   
		//inform curl to send the form-post
		curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);	

	}
	
	//add the request to the queque
	curl_multi_add_handle(multi_handle, curl);

	//pending request
	//webserverBusy=true;
webserverState=1;
	return 0;	
}

int WebServer::sendGenericRequest (std::string data, std::string& serverReply){
	CURL *curl;
	CURLcode res;

	GfLogInfo("WebServer - SENDING data to server:\n %s \n", data.c_str()); 
webserverState=1;
	//insert "data=" before the actual data
	data.insert(0,"data=");
	const char *postthis=data.c_str();
 
	curl_global_init(CURL_GLOBAL_ALL);
	curl = curl_easy_init();
	if(curl) {
		
		curl_easy_setopt(curl, CURLOPT_URL, this->url);
		
		// send all data to this function
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteStringCallback);
		
		//pass our std::string to the WriteStringCallback functions so it can write into it
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &this->curlServerReply);
		
		// some servers don't like requests that are made without a user-agent
		// field, so we provide one 
		curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");
	
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postthis);
		
		// if we don't provide POSTFIELDSIZE, libcurl will strlen() by
		// itself 
		curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)strlen(postthis));
		
		// Perform the request, res will get the return code 
		res = curl_easy_perform(curl);

		// Check for errors 
		if(res != CURLE_OK) {
			msglist.push_back("WebServer: failed to connect!");
			fprintf(stderr, "curl_easy_perform() failed: %s\n",
					curl_easy_strerror(res));
		}
		else {
			//
			// Now, our this->curlServerReply
			// contains the remote file (aka serverReply).
			//
			// Do something nice with it!
			// 

			GfLogInfo("WebServer - RECEIVING data from server:\n %s\n", this->curlServerReply.c_str());
webserverState=2;
			serverReply = this->curlServerReply;
			//empty the string
			this->curlServerReply.clear();			
		}
	 
		// always cleanup 
		curl_easy_cleanup(curl);
	 
		// we're done with libcurl, so clean it up 
		curl_global_cleanup();
	}
	return 0;
}

int WebServer::readUserConfig (int userId){
	void *prHandle;
	char prefFileUrl[256];
	char xmlPath[256];

	// find the xmlPath to our specific user in the preferencies xml file
	sprintf(xmlPath, "%s%i", "Preferences/Drivers/", userId);

	//get the preferencies file location
	sprintf(prefFileUrl, "%s%s", GfLocalDir(), HM_PREF_FILE);

	//read the preferencies file
	prHandle = GfParmReadFile(prefFileUrl, GFPARM_RMODE_REREAD);

	//get webServer user id for current user
	this->username = GfParmGetStr(prHandle, xmlPath, "WebServerUsername","val");

	//get webServer user password for current user
	this->password = GfParmGetStr(prHandle, xmlPath, "WebServerPassword","val");

	return 0;
}
int WebServer::sendLogin (int userId){
	std::string serverReply;

	//read username and password and save it in as webserver properties 
	this->readUserConfig(userId);
	
	std::string username="username";
	std::string password="password";
	
	//if the user has not setup the webserver login info abort the login
	if(username==this->username && password==this->password){
		GfLogInfo("WebServer - send of login info aborted (the user is not correctly setup).\n");
		return 1;	
	}

	//prepare the string to send
	std::string dataToSend ("");
	dataToSend.append(	"<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
						"<content>"
						"<request>"
						"<login>"
						"<username>{{username}}</username>"
						"<password>{{password}}</password>"
						"</login>"
						"</request>"
						"</content>");
						
	//replace the {{tags}} with the respecting values
	replaceAll(dataToSend, "{{username}}", this->username);
	replaceAll(dataToSend, "{{password}}", this->password);
	
	GfLogInfo("WebServer - sending LOGIN info\n");

	this->sendGenericRequest(dataToSend, serverReply);

	GfLogInfo("WebServer - PROCESSING SERVER REPLY:\n%s\n", serverReply.c_str());

	//read the xml reply of the server
	void *xmlReply;
	xmlReply = GfParmReadBuf(const_cast<char*>(serverReply.c_str()));

	//login reply
	//store the webServer session and id assigned if available
	if(GfParmExistsSection(xmlReply, "content/reply/login")){
		if(GfParmGetNum(xmlReply, "content/reply/login", "id", "null",0) != 0){
			GfLogInfo("WebServer - loggin succeded.\n");
			msglist.push_back("WebServer: LOGGED IN!");
			//store the webServer session and id assigned
			this->sessionId = GfParmGetStr(xmlReply, "content/reply/login", "sessionid", "null");
			this->userId = GfParmGetNum(xmlReply, "content/reply/login", "id", "null",0);
		}else{
			GfLogInfo("WebServer - Login Failed: probably wrong username or password.\n");
			msglist.push_back("WebServer: Login Failed:\nwrong username or password.");
			return 1;				
		}
	}else{
		GfLogInfo("WebServer - Login Failed: bad reply from the server.\n");
		msglist.push_back("WebServer: Login Failed:\nbad reply from the server.");
		return 1;		
	}


	GfLogInfo("WebServer - assigned session id is: %s\n", this->sessionId);

	return 0;
}
int WebServer::sendLap (int race_id, double laptime, double fuel, int position, int wettness){

	//are we logged in?
	if(this->sessionId=='\0'){
		GfLogInfo("WebServer - send of lap info aborted. No session ID assigned (we are not logged-in).\n");			
		return 1;
	}

	//prepare the string to send
	std::string dataToSend ("");
	dataToSend.append(	"<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
						"<content>"
						"<request>"
						"<laps>"
						"<race_id>{{race_id}}</race_id>"
						"<laptime>{{laptime}}</laptime>"
						"<fuel>{{fuel}}</fuel>"
						"<position>{{position}}</position>"
						"<wettness>{{wettness}}</wettness>"						
						"</laps>"
						"</request>"
						"</content>");
						
	//replace the {{tags}} with the respecting values						
	replaceAll(dataToSend, "{{race_id}}", to_string(race_id));
	replaceAll(dataToSend, "{{laptime}}", to_string(laptime));
	replaceAll(dataToSend, "{{fuel}}", to_string(fuel));
	replaceAll(dataToSend, "{{position}}", to_string(position));
	replaceAll(dataToSend, "{{wettness}}", to_string(wettness));	
	
	GfLogInfo("WebServer - sending LAP info\n");

	this->addAsyncRequest(dataToSend);

	return 0;
}
int WebServer::sendRaceStart (int user_skill, const char *track_id, char *car_id, int type, void *setup, int startposition, const char *sdversion){
	std::string serverReply;
	std::string mysetup;
	std::string dataToSend;

	//are we logged in?
	if(this->sessionId=='\0'){
		GfLogInfo("WebServer - send of racestart info aborted. No session ID assigned (we are not logged in)");			
		return 1;
	}

	//read the setup
	GfParmWriteString(setup, mysetup);

	//prepare the string to send
	dataToSend.append(	"<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
						"<content>"
						"<request>"
						"<races>"
						"<user_id>{{user_id}}</user_id>"
						"<user_skill>{{user_skill}}</user_skill>"
						"<track_id>{{track_id}}</track_id>"
						"<car_id>{{car_id}}</car_id>"
						"<type>{{type}}</type>"
						"<setup><![CDATA[{{setup}}]]></setup>"
						"<startposition>{{startposition}}</startposition>"
						"<sdversion>{{sdversion}}</sdversion>"
						"</races>"
						"</request>"
						"</content>");
						
	//replace the {{tags}} with the respecting values						
	replaceAll(dataToSend, "{{user_id}}", to_string(this->userId));						
	replaceAll(dataToSend, "{{user_skill}}", to_string(user_skill));						
	replaceAll(dataToSend, "{{track_id}}", to_string(track_id));						
	replaceAll(dataToSend, "{{car_id}}", to_string(car_id));						
	replaceAll(dataToSend, "{{type}}", to_string(type));						
	replaceAll(dataToSend, "{{setup}}", mysetup);						
	replaceAll(dataToSend, "{{startposition}}", to_string(startposition));						
	replaceAll(dataToSend, "{{sdversion}}", to_string(sdversion));						
						
	GfLogInfo("WebServer - sending RACE START info\n");	

	this->addAsyncRequest(dataToSend);

	return 0;
}
int WebServer::sendRaceEnd (int race_id, int endposition){
	std::string serverReply;

	//are we logged in?
	if(this->sessionId=='\0'){
		GfLogInfo("WebServer - send of raceend info aborted. No session ID assigned (we are not logged in)");			
		return 1;
	}

	//prepare the string to send
	std::string dataToSend ("");
	dataToSend.append(	"<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
						"<content>"
						"<request>"
						"<races>"
						"<id>{{race_id}}</id>"
						"<endposition>{{endposition}}</endposition>"
						"</races>"
						"</request>"
						"</content>");
						
	//replace the {{tags}} with the respecting values						
	replaceAll(dataToSend, "{{race_id}}", to_string(race_id));
	replaceAll(dataToSend, "{{endposition}}", to_string(endposition));

	GfLogInfo("WebServer - sending RACE END information \n");	

	this->sendGenericRequest(dataToSend, serverReply);

	return 0;
}
//initialize the web server
WebServer webServer;

/* END webserver*/

static const char *aPszSkillLevelNames[] =
	{ ROB_VAL_ARCADE, ROB_VAL_SEMI_ROOKIE, ROB_VAL_ROOKIE, ROB_VAL_AMATEUR, ROB_VAL_SEMI_PRO, ROB_VAL_PRO };
static const int NSkillLevels = (int)(sizeof(aPszSkillLevelNames)/sizeof(char*));

// The list of robot modules loaded for the race.
tModList *ReRacingRobotsModList = 0;

// The race situation
tRmInfo	*ReInfo = 0;

int replayRecord;
double replayTimestamp;
#ifdef THIRD_PARTY_SQLITE3
sqlite3 *replayDB;
sqlite3_stmt *replayBlobs[50];
#endif

// Race Engine reset
void
ReReset(void)
{
	// Allocate race engine info structures if not already done.
	ReInfo = ReSituation::self().data();
	ReInfo->robModList = &ReRacingRobotsModList;

	// Load Race engine params.
	char buf[256];
	snprintf(buf, sizeof(buf), "%s%s", GfLocalDir(), RACE_ENG_CFG);
	ReInfo->_reParam = GfParmReadFile(buf, GFPARM_RMODE_REREAD | GFPARM_RMODE_CREAT);
}


// Race Engine cleanup
void ReCleanup(void)
{
	ReSituation::terminate();

    if (!ReInfo)
        return;

	// Free ReInfo memory.
	// ReSituation::terminate();
	ReInfo = 0;
}

// Race Engine Exit
int
ReExit(void)
{
	// Stop and cleanup the race engine.
	ReStop();
	StandardGame::self().cleanup();
	
	// Notify the user interface.
	ReUI().quit();

	return RM_QUIT;
}

// Select the given manager for the race.
void
ReRaceSelectRaceman(GfRaceManager* pRaceMan, bool bKeepHumans)
{
	// Trace the chosen raceman full type.
	std::string strFullType(pRaceMan->getType());
	if (!pRaceMan->getSubType().empty())
	{
		strFullType += " / ";
		strFullType += pRaceMan->getSubType();
	}
	GfLogTrace("'%s' race mode selected\n", strFullType.c_str());
	
	// Re-init. race engine info about the race manager (= the race mode / type / class).
	ReInfo->_reName = pRaceMan->getName().c_str();
	ReInfo->_reFilename = pRaceMan->getId().c_str();

	// (Re-)initialize the currrent race configuration from the selected race manager.
	StandardGame::self().race()->load(pRaceMan, bKeepHumans);
}

// Start configuring the race
void
ReRaceConfigure(bool bInteractive)
{
	// Update race engine info.
	ReInfo->mainParams = ReInfo->params =
		StandardGame::self().race()->getManager()->getDescriptorHandle();
	
	GfParmRemoveVariable(ReInfo->params, "/", "humanInGroup");
	GfParmSetVariable(ReInfo->params, "/", "humanInGroup", ReHumanInGroup() ? 1.0f : 0.0f);
	
	// Enter CONFIG state and return to the race engine automaton if interactive mode.
	if (bInteractive)
		ReStateApply((void*)RE_STATE_CONFIG);
}

// Restore the race from the given results file
void
ReRaceRestore(void* hparmResults)
{
	// Update race engine info in order to set it in the exact state
	// it was in when the race mode was saved.
	GfRace* pRace = StandardGame::self().race();
	ReInfo->mainParams = pRace->getManager()->getDescriptorHandle();
	ReInfo->mainResults = pRace->getResultsDescriptorHandle();
	if (!pRace->getManager()->hasSubFiles())
	{
		// Non-Career mode.
		ReInfo->params = ReInfo->mainParams;
		ReInfo->results = ReInfo->mainResults;
		ReInfo->_reRaceName = pRace->getSessionName().c_str(); //ReInfo->_reName;
	}
	else
	{
		// Career mode : More complicated, as everything is not in one params/results file
		// (the target state is right after the end of the previous event,
		//  which was from the previous group).
		const char* pszPrevParamsFile =
			GfParmGetStr(ReInfo->mainResults, RE_SECT_CURRENT, RE_ATTR_PREV_FILE, 0);
		if (!pszPrevParamsFile)
			GfLogWarning("Career : No previous file in MainResults\n");
		ReInfo->params =
			pszPrevParamsFile ? GfParmReadFile(pszPrevParamsFile, GFPARM_RMODE_STD) : ReInfo->mainParams;
		const char* pszPrevResultsFile =
			GfParmGetStr(ReInfo->params, RM_SECT_SUBFILES, RM_ATTR_RESULTSUBFILE, 0);
		if (!pszPrevResultsFile)
			GfLogWarning("Career : Failed to load previous results from previous params\n");
		ReInfo->results = 
			pszPrevResultsFile ? GfParmReadFile(pszPrevResultsFile, GFPARM_RMODE_STD) : ReInfo->mainResults;
		ReInfo->_reRaceName = ReGetPrevRaceName(/* bLoop = */true);
	}

	GfParmRemoveVariable(ReInfo->params, "/", "humanInGroup");
	GfParmSetVariable(ReInfo->params, "/", "humanInGroup", ReHumanInGroup() ? 1.0f : 0.0f);
}

// Start a new race for the previously configured race manager
void
ReStartNewRace()
{
	// Save the race settings to the race manager file is anything changed.
	GfRace* pRace = StandardGame::self().race();
	if (pRace->isDirty())
	{
		pRace->store(); // Save data to params.
		GfParmWriteFile(NULL, ReInfo->params, ReInfo->_reName); // Save params to disk.
	}

	// Initialize the result system (different way for the Career mode).
	if (pRace->getManager()->hasSubFiles())
		ReCareerNew();
	else
		ReInitResults();

	// Enter EVENT_INIT state and return to the race engine automaton.
	ReStateApply((void*)RE_STATE_EVENT_INIT);
}

// Resume the previously restored race from a results file
void
ReResumeRace()
{
	ReUI().onRaceResuming();
}


/*
 * Function
 *  initStartingGrid
 *
 * Description
 *  Place the cars on the starting grid
 *
 * Parameters
 *  Race Information structure initialized
 *
 * Return
 *  none
 */
static void
initStartingGrid(void)
{
  char path[64];
  int i;
  tTrackSeg *curseg;
  int rows;
  tdble a, b;
  //tdble wi2; // Never used.
  tdble d1, d2,d3;
  tdble startpos, tr, ts;
  tdble speedInit;
  tdble heightInit;
  tCarElt *car;
  const char *pole;
  void *trHdle = ReInfo->track->params;
  void *params = ReInfo->params;

  snprintf(path, sizeof(path), "%s/%s", ReInfo->_reRaceName, RM_SECT_STARTINGGRID);

  /* Search for the first turn for find the pole side */
  curseg = ReInfo->track->seg->next;
  while (curseg->type == TR_STR) {
    /* skip the straight segments */
    curseg = curseg->next;
  }
  /* Set the pole for the inside of the first turn */
  if (curseg->type == TR_LFT) {
    pole = GfParmGetStr(params, path, RM_ATTR_POLE, "left");
  } else {
    pole = GfParmGetStr(params, path, RM_ATTR_POLE, "right");
  }
  /* Tracks definitions can force the pole side */
  pole = GfParmGetStr(trHdle, RM_SECT_STARTINGGRID, RM_ATTR_POLE, pole);

  if (strcmp(pole, "left") == 0) {
    a = ReInfo->track->width;
    b = -a;
  } else {
    a = 0;
    b = ReInfo->track->width;
  }
  //wi2 = ReInfo->track->width * 0.5f; // Never used.

  rows = (int)GfParmGetNum(params, path, RM_ATTR_ROWS, (char*)NULL, 2);
  rows = (int)GfParmGetNum(trHdle, RM_SECT_STARTINGGRID, RM_ATTR_ROWS, (char*)NULL, (tdble)rows);
  d1 = GfParmGetNum(params, path, RM_ATTR_TOSTART, (char*)NULL, 10);
  d1 = GfParmGetNum(trHdle, RM_SECT_STARTINGGRID, RM_ATTR_TOSTART, (char*)NULL, d1);
  d2 = GfParmGetNum(params, path, RM_ATTR_COLDIST, (char*)NULL, 10);
  d2 = GfParmGetNum(trHdle, RM_SECT_STARTINGGRID, RM_ATTR_COLDIST, (char*)NULL, d2);
  d3 = GfParmGetNum(params, path, RM_ATTR_COLOFFSET, (char*)NULL, 5);
  d3 = GfParmGetNum(trHdle, RM_SECT_STARTINGGRID, RM_ATTR_COLOFFSET, (char*)NULL, d3);
  speedInit = GfParmGetNum(params, path, RM_ATTR_INITSPEED, (char*)NULL, 0.0);
  heightInit = GfParmGetNum(params, path, RM_ATTR_INITHEIGHT, (char*)NULL, 0.3f);
  heightInit = GfParmGetNum(trHdle, RM_SECT_STARTINGGRID, RM_ATTR_INITHEIGHT, (char*)NULL, heightInit);

  if (rows < 1) {
    rows = 1;
  }
  for (i = 0; i < ReInfo->s->_ncars; i++) {
    car = &(ReInfo->carList[i]);
    car->_speed_x = speedInit;
    startpos = ReInfo->track->length - (d1 + (i / rows) * d2 + (i % rows) * d3);
    tr = a + b * ((i % rows) + 1) / (rows + 1);
    curseg = ReInfo->track->seg;  /* last segment */
    while (startpos < curseg->lgfromstart) {
      curseg = curseg->prev;
    }
    ts = startpos - curseg->lgfromstart;
    car->_trkPos.seg = curseg;
    car->_trkPos.toRight = tr;
    switch (curseg->type) {
      case TR_STR:
        car->_trkPos.toStart = ts;
        RtTrackLocal2Global(&(car->_trkPos), &(car->_pos_X), &(car->_pos_Y), TR_TORIGHT);
        car->_yaw = curseg->angle[TR_ZS];
        break;
      case TR_RGT:
        car->_trkPos.toStart = ts / curseg->radius;
        RtTrackLocal2Global(&(car->_trkPos), &(car->_pos_X), &(car->_pos_Y), TR_TORIGHT);
        car->_yaw = curseg->angle[TR_ZS] - car->_trkPos.toStart;
        break;
      case TR_LFT:
        car->_trkPos.toStart = ts / curseg->radius;
        RtTrackLocal2Global(&(car->_trkPos), &(car->_pos_X), &(car->_pos_Y), TR_TORIGHT);
        car->_yaw = curseg->angle[TR_ZS] + car->_trkPos.toStart;
        break;
    }
    car->_pos_Z = RtTrackHeightL(&(car->_trkPos)) + heightInit;

    FLOAT_NORM0_2PI(car->_yaw);

	RePhysicsEngine().configureCar(car);
  }
}


static void
initPits(void)
{
  tTrackPitInfo *pits;
  int i, j;

  /*
  typedef std::map<std::string, int> tTeamsMap;
  typedef tTeamsMap::const_iterator tTeamsMapIterator;
  tTeamsMap teams;
  tTeamsMapIterator teamsIterator;

  // create a list with the teams, a pit can just be used by one team.
  for (i = 0; i < ReInfo->s->_ncars; i++) {
    tCarElt *car = &(ReInfo->carList[i]);
    teams[car->_teamname] = teams[car->_teamname] + 1; 
  } 

  for (teamsIterator = teams.begin(); teamsIterator != teams.end(); ++teamsIterator) {
    GfLogDebug("----------------- %s\t%d\n", (teamsIterator->first).c_str(), teamsIterator->second); 
  }
  */

  // How many cars are sharing a pit?
  int carsPerPit = (int) GfParmGetNum(ReInfo->params, ReInfo->_reRaceName, RM_ATTR_CARSPERPIT, NULL, 1.0f);
  if (carsPerPit < 1) {
    carsPerPit = 1;
  } else if (carsPerPit > TR_PIT_MAXCARPERPIT) {
    carsPerPit = TR_PIT_MAXCARPERPIT;
  }
  GfLogInfo("Cars per pit: %d\n", carsPerPit);

  switch (ReInfo->track->pits.type) {
    case TR_PIT_ON_TRACK_SIDE:
    case TR_PIT_NO_BUILDING:
      pits = &(ReInfo->track->pits);
      pits->driversPitsNb = ReInfo->s->_ncars;
      pits->carsPerPit = carsPerPit;

      // Initialize pit data for every pit, necessary because of restarts
      // (track gets not reloaded, no calloc).
      for (i = 0; i < pits->nMaxPits; i++) {
        tTrackOwnPit *pit = &(pits->driversPits[i]);
        pit->freeCarIndex = 0;
        pit->pitCarIndex = TR_PIT_STATE_FREE;
        for (j = 0; j < TR_PIT_MAXCARPERPIT; j++) {
          pit->car[j] = NULL;
        }
      }

      // Assign cars to pits. Inefficient (O(n*n)), but just at initialization, so do not care.
      // One pit can just host cars of one team (this matches with the reality)
      for (i = 0; i < ReInfo->s->_ncars; i++) {
        // Find pit for the cars team.
        tCarElt *car = &(ReInfo->carList[i]);
        for (j = 0; j < pits->nMaxPits; j++) {
          tTrackOwnPit *pit = &(pits->driversPits[j]);
          // Put car in this pit if the pit is unused or used by a teammate and there is
          // space left.
          if (pit->freeCarIndex <= 0 ||
            (strcmp(pit->car[0]->_teamname, car->_teamname) == 0 && pit->freeCarIndex < carsPerPit))
          {
            // Assign car to pit.
            pit->car[pit->freeCarIndex] = car;
            // If this is the first car, set up more pit values ; assumption: the whole team
            // uses the same car. If not met, it does not matter much, but the car might be
            // captured a bit too easy or too hard.
            if (pit->freeCarIndex == 0) {
              pit->pitCarIndex = TR_PIT_STATE_FREE;
              pit->lmin = pit->pos.seg->lgfromstart + pit->pos.toStart - pits->len / 2.0f + car->_dimension_x / 2.0f;
              if (pit->lmin > ReInfo->track->length) {
                pit->lmin -= ReInfo->track->length;
              }
              pit->lmax = pit->pos.seg->lgfromstart + pit->pos.toStart + pits->len / 2.0f - car->_dimension_x / 2.0f;
              if (pit->lmax > ReInfo->track->length) {
                pit->lmax -= ReInfo->track->length;
              }
            }
            (pit->freeCarIndex)++;
            ReInfo->carList[i]._pit = pit;
            // Assigned, continue with next car.
            break;
          }
        }
      }

      // Print out assignments.
      for (i = 0; i < pits->nMaxPits; i++) {
        tTrackOwnPit *pit = &(pits->driversPits[i]);
        for (j = 0; j < pit->freeCarIndex; j++) {
          if (j == 0) {
            GfLogTrace("Pit %d, Team: %s, ", i, pit->car[j]->_teamname); 
          }
          GfLogTrace("%d: %s ", j, pit->car[j]->_name);
        }
        if (j > 0) {
          GfOut("\n");
        }
      }

      break;
    case TR_PIT_ON_SEPARATE_PATH:
      break;
    case TR_PIT_NONE:
      break;
  }
}

/**
 * Function to load a car.
 *
 * @param carindex The index whichs will be used as car->index for the car.
 * @param listindex The listindex in RM_SECT_DRIVERS_RACING
 * @param modindex The index of the mod; must be MAX_MOD_ITF if normal_carname is FALSE.
 * @param robotIdx The index of the robot.
 * @param normal_carname If this member is TRUE, the car is treated as an ordinary car;
 *                       if this member is FALSE, then the car used is the one given
 *                       in the xml-file, and there is no restriction on the number of instances.
 * @param cardllname The dllname of the driver
 * @return A pointer to the newly created car if successfull; NULL otherwise
 */
static tCarElt* reLoadSingleCar( int carindex, int listindex, int modindex, int relativeRobotIdx, char normal_carname, char const *cardllname )
{
  tCarElt *elt;
  tMemoryPool oldPool;
  char path[256];
  char path2[256];
  char buf[256];
  char buf2[256];
  char const *str;
  char const *category;
  char const *subcategory;
  char const *teamname;
  std::string carname;
  tModInfoNC *curModInfo;
  tRobotItf *curRobot;
  void *robhdle;
  void *cathdle;
  void *carhdle;
  void *handle;
  int k;
  int xx;
  char isHuman;
  int robotIdx = relativeRobotIdx;

  /* good robot found */
  curModInfo = &((*(ReInfo->robModList))->modInfo[modindex]);

  subcategory = ReInfo->track->subcategory;

#if 0 //SDW
  if (replayReplay)
    GfLogInfo("Driver in car %d being driven by replay\n", carindex);
  else
#endif
    GfLogInfo("Driver's name: %s\n", curModInfo->name);

  isHuman = strcmp( cardllname, "human" ) == 0 || strcmp( cardllname, "networkhuman" ) == 0;

  /* Extended is forced for humans, so no need to increase robotIdx */
  if (!normal_carname && !isHuman) 
    robotIdx += curModInfo->index;

  /* Retrieve the driver interface (function pointers) */
  curRobot = (tRobotItf*)calloc(1, sizeof(tRobotItf));

  /* ... and initialize the driver */
#if 0 // SDW
  if (replayReplay) {
    // Register against the Replay driver (which does nothing)
    curModInfo->fctInit(carindex, (void*)(curRobot));
  } else if (!(ReInfo->_displayMode & RM_DISP_MODE_SIMU_SIMU)) {
#else
  if (!(ReInfo->_displayMode & RM_DISP_MODE_SIMU_SIMU)) {
#endif
    curModInfo->fctInit(robotIdx, (void*)(curRobot));
  } else {
    curRobot->rbNewTrack = NULL;
    curRobot->rbNewRace  = NULL;
    curRobot->rbResumeRace  = NULL;
    curRobot->rbDrive    = NULL;
    curRobot->rbPitCmd   = NULL;
    curRobot->rbEndRace  = NULL;
    curRobot->rbShutdown = NULL;
    curRobot->index      = 0;
  }

  /* Retrieve and load the robotXML file :
     1) from user settings dir (local dir)
     2) from installed data dir */
  snprintf(buf, sizeof(buf), "%sdrivers/%s/%s.xml", GfLocalDir(), cardllname, cardllname);
  robhdle = GfParmReadFile(buf, GFPARM_RMODE_STD);
  if (!robhdle) {
    snprintf(buf, sizeof(buf), "drivers/%s/%s.xml", cardllname, cardllname);
    robhdle = GfParmReadFile(buf, GFPARM_RMODE_STD);
  }

  if (normal_carname || isHuman)
    snprintf(path, sizeof(path), "%s/%s/%d", ROB_SECT_ROBOTS, ROB_LIST_INDEX, robotIdx);
  else
    snprintf(path, sizeof(path), "%s", ROB_SECT_ARBITRARY);
  
  /* Load car/driver info (in race engine data structure) */
  if (robhdle)
  {
    elt = &(ReInfo->carList[carindex]);
    GF_TAILQ_INIT(&(elt->_penaltyList));

    const std::string strDType = GfParmGetStr(robhdle, path, ROB_ATTR_TYPE, ROB_VAL_ROBOT);
    if (strDType == ROB_VAL_ROBOT){
      elt->_driverType = RM_DRV_ROBOT;
      elt->_networkPlayer = 0;
    } 
    else if (strDType == ROB_VAL_HUMAN)
    {
      elt->_driverType = RM_DRV_HUMAN;
      std::string strNetPlayer = GfParmGetStr(robhdle, path, "networkrace", "no");
      elt->_networkPlayer = (strNetPlayer == "yes") ? 1 : 0;
    }

    elt->index = carindex;
    elt->robot = curRobot;
    elt->_paramsHandle = robhdle;
    elt->_driverIndex = robotIdx;
    elt->_moduleIndex = relativeRobotIdx;
    strncpy(elt->_modName, cardllname, MAX_NAME_LEN - 1);
    elt->_modName[MAX_NAME_LEN - 1] = 0;

    //snprintf(path, sizeof(path), "%s/%s/%d", ROB_SECT_ROBOTS, ROB_LIST_INDEX, robotIdx);
    snprintf( path2, sizeof(path2), "%s/%s/%d/%d", RM_SECT_DRIVERINFO, elt->_modName, normal_carname ? 0 : 1, elt->_moduleIndex );
    if (normal_carname || elt->_driverType == RM_DRV_HUMAN) {
      strncpy(elt->_name, GfParmGetStr(robhdle, path, ROB_ATTR_NAME, "none"), MAX_NAME_LEN - 1);
      strncpy(elt->_sname, GfParmGetStr(robhdle, path, ROB_ATTR_SNAME, "none"), MAX_NAME_LEN - 1);
      strncpy(elt->_cname, GfParmGetStr(robhdle, path, ROB_ATTR_CODE, "---"), 3);
    } else {
      strncpy(elt->_name, GfParmGetStr(ReInfo->params, path2, ROB_ATTR_NAME, "none"), MAX_NAME_LEN - 1);
      strncpy(elt->_sname, GfParmGetStr(ReInfo->params, path2, ROB_ATTR_SNAME, "none"), MAX_NAME_LEN - 1);
      strncpy(elt->_cname, GfParmGetStr(ReInfo->params, path2, ROB_ATTR_CODE, "---"), 3);
    }
    elt->_name[MAX_NAME_LEN - 1] = 0;
    elt->_sname[MAX_NAME_LEN - 1] = 0;
    elt->_cname[3] = 0;

    teamname = GfParmGetStr(robhdle, path, ROB_ATTR_TEAM, "none");
    teamname = GfParmGetStr(ReInfo->params, path2, ROB_ATTR_TEAM, teamname ); //Use the name in params if it has a team name
    strncpy(elt->_teamname, teamname, MAX_NAME_LEN - 1);
    elt->_teamname[MAX_NAME_LEN - 1] = 0;

    elt->_driveSkill = GfParmGetNum(ReInfo->params, path2, RM_ATTR_SKILLLEVEL, NULL, -1.0f);

    if (normal_carname) /* Even if we get a normal_carname for humans we use it despite of forced extended mode*/
      strncpy(elt->_carName, GfParmGetStr(robhdle, path, ROB_ATTR_CAR, ""), MAX_NAME_LEN - 1);
    else
      strncpy(elt->_carName, GfParmGetStr(ReInfo->params, path2, RM_ATTR_CARNAME, ""), MAX_NAME_LEN - 1);
    elt->_carName[MAX_NAME_LEN - 1] = 0; /* XML file name */

    // Load custom skin name and targets from race info (if specified).
    snprintf(path2, sizeof(path2), "%s/%d", RM_SECT_DRIVERS_RACING, listindex);
    if (GfParmGetStr(ReInfo->params, path2, RM_ATTR_SKINNAME, 0))
    {
      strncpy(elt->_skinName, GfParmGetStr(ReInfo->params, path2, RM_ATTR_SKINNAME, ""), MAX_NAME_LEN - 1);
      elt->_skinName[MAX_NAME_LEN - 1] = 0; // Texture name
    }
	elt->_skinTargets = (int)GfParmGetNum(ReInfo->params, path2, RM_ATTR_SKINTARGETS, (char*)NULL, 0);
	
    // Load other data from robot descriptor.
    elt->_raceNumber = (int)GfParmGetNum(robhdle, path, ROB_ATTR_RACENUM, (char*)NULL, 0);
    if (!normal_carname && elt->_driverType != RM_DRV_HUMAN) // Increase racenumber if needed
      elt->_raceNumber += elt->_moduleIndex;
    elt->_skillLevel = 0;
    str = GfParmGetStr(robhdle, path, ROB_ATTR_LEVEL, ROB_VAL_SEMI_PRO);
    for(k = 0; k < NSkillLevels; k++) {
      if (strcmp(aPszSkillLevelNames[k], str) == 0) {
        elt->_skillLevel = k;
        break;
      }
    }
    elt->_startRank  = carindex;
    elt->_pos        = carindex+1;
    elt->_remainingLaps = ReInfo->s->_totLaps;

    elt->_newTrackMemPool = NULL;
    elt->_newRaceMemPool = NULL;
    elt->_endRaceMemPool = NULL;
    elt->_shutdownMemPool = NULL;

	carname = elt->_carName;

    GfLogTrace("Driver #%d(%d) : module='%s', name='%s', car='%s', cat='%s', skin='%s' on %x\n",
			   carindex, listindex, elt->_modName, elt->_name, elt->_carName,
			   elt->_category, elt->_skinName, elt->_skinTargets);

    if ((strncmp(carname.c_str(), "mpa1", 4) == 0))
	{
		if (strcmp(subcategory, "long") == 0)
			carname = carname+"-long";
		else if (strcmp(subcategory, "short") == 0)
			carname = carname+"-short";
		else 
			carname = carname+"-road";

		GfLogTrace("MPA... Category car = %s \n", carname.c_str());

		/* Retrieve and load car specs : merge car default specs,
		category specs and driver modifications (=> handle) */
		/* Read Car model specifications */
		snprintf(buf, sizeof(buf), "cars/models/%s/%s.xml", elt->_carName, carname.c_str());
		carhdle = GfParmReadFile(buf, GFPARM_RMODE_STD | GFPARM_RMODE_CREAT);

	}
	else
	{  
		/* Retrieve and load car specs : merge car default specs,
		category specs and driver modifications (=> handle) */
		/* Read Car model specifications */
		snprintf(buf, sizeof(buf), "cars/models/%s/%s.xml", elt->_carName, elt->_carName);
		carhdle = GfParmReadFile(buf, GFPARM_RMODE_STD | GFPARM_RMODE_CREAT);
	}

    category = GfParmGetStr(carhdle, SECT_CAR, PRM_CATEGORY, NULL);
    if (category)
    {
	  GfLogTrace("Checking/Merging %s specs into %s base setup for %s ...\n",
				 category, elt->_carName, curModInfo->name);
      strncpy(elt->_category, category, MAX_NAME_LEN - 1);
      elt->_category[MAX_NAME_LEN - 1] = 0;
      /* Read Car Category specifications */
      snprintf(buf2, sizeof(buf2), "cars/categories/%s.xml", elt->_category);
      cathdle = GfParmReadFile(buf2, GFPARM_RMODE_STD | GFPARM_RMODE_CREAT);
	  int errorcode = 0;

      if ((errorcode = GfParmCheckHandle(cathdle, carhdle))) 
	  {
        switch (errorcode)
		{
		  case -1:
            GfLogError("Car %s NOT in category %s (driver %s) !!!\n", elt->_carName, category, elt->_name);
		    break;

		  case -2:
            GfLogError("Parameters out of bound for car %s (driver %s)!!!\n",elt->_carName, elt->_name);
		    break;

		  case -3:
            GfLogError("Parameter not allowed for car %s (driver %s)!!!\n",elt->_carName, elt->_name);
		    break;

		  default:
            GfLogError("Unknown error for %s (driver %s)!!!\n",elt->_carName, elt->_name);
		    break;
	    } 
        return NULL;
      }

      carhdle = GfParmMergeHandles(cathdle, carhdle,
                                   GFPARM_MMODE_SRC | GFPARM_MMODE_DST | GFPARM_MMODE_RELSRC | GFPARM_MMODE_RELDST);
	  
      /* The code below stores the carnames to a separate xml-file
		 such that at newTrack it is known which car is used.
		 TODO: find a better method for this */
      snprintf (buf, sizeof(buf), "%sdrivers/curcarnames.xml", GfLocalDir());
      handle = GfParmReadFile(buf, GFPARM_RMODE_CREAT);
      if (handle) {
        snprintf(path, sizeof(path), "drivers/%s/%d", cardllname, elt->_driverIndex);
        GfParmSetStr (handle, path, RM_ATTR_CARNAME, elt->_carName);
        GfParmWriteFile (0, handle, "Car names");
        GfParmReleaseHandle (handle);
      }
      if (!(ReInfo->_displayMode & RM_DISP_MODE_SIMU_SIMU))
      {
        GfPoolMove(&elt->_newTrackMemPool, &oldPool);
        curRobot->rbNewTrack(elt->_driverIndex, ReInfo->track, carhdle, &handle, ReInfo->s);
        GfPoolFreePool( &oldPool );
      }
      else
        handle = NULL;
      if (handle && !replayReplay) {
		GfLogTrace("Checking/Merging %s specific setup into %s setup.\n",
				   curModInfo->name, elt->_carName);
        if (GfParmCheckHandle(carhdle, handle)) {
          GfLogError("Bad Car parameters for driver %s\n", elt->_name);
          return NULL;
        }
        handle = GfParmMergeHandles(carhdle, handle,
                                    GFPARM_MMODE_SRC | GFPARM_MMODE_DST | GFPARM_MMODE_RELSRC | GFPARM_MMODE_RELDST);
      } else {
		GfLogTrace("Keeping %s setup as is for %s (no specific setup).\n",
				   elt->_carName, curModInfo->name);
		handle = carhdle;
      }
      elt->_carHandle = handle;

      /* Initialize sectors */
      elt->_currentSector = 0;
      elt->_curSplitTime = (double*)malloc( sizeof(double) * ( ReInfo->track->numberOfSectors - 1 ) );
      elt->_bestSplitTime = (double*)malloc( sizeof(double) * ( ReInfo->track->numberOfSectors - 1 ) );
      for (xx = 0; xx < ReInfo->track->numberOfSectors - 1; ++xx)
      {
        elt->_curSplitTime[xx] = -1.0f;
        elt->_bestSplitTime[xx] = -1.0f;
      }
    } else {
      elt->_category[ 0 ] = '\0';
      GfLogError("Bad Car category for driver %s\n", elt->_name);
      return NULL;
    }

    return elt;
  } else {
    GfLogError("No description file for robot %s\n", cardllname);
  }
  return NULL;
}


/** Initialize the cars for a race.
    The cars are positionned on the starting grid.
    @return 0 Ok, -1 Error
 */
int
ReInitCars(void)
{
  char buf[512];
  char path[512];
  int nCars;
  int index;
  int i, j;
  const char *robotModuleName;
  int robotIdx;
  void *robhdle;
  tCarElt *elt;
  //const char *focused; // Never used.
  //int focusedIdx; // Never used.
  void *params = ReInfo->params;

  /* Get the number of cars (= drivers) racing */
  nCars = GfParmGetEltNb(params, RM_SECT_DRIVERS_RACING);
  GfLogTrace("Loading %d car(s)\n", nCars);

  FREEZ(ReInfo->carList);
  ReInfo->carList = (tCarElt*)calloc(nCars, sizeof(tCarElt));
  FREEZ(ReInfo->rules);
  ReInfo->rules = (tRmCarRules*)calloc(nCars, sizeof(tRmCarRules));
  //focused = GfParmGetStr(ReInfo->params, RM_SECT_DRIVERS, RM_ATTR_FOCUSED, "");
  //focusedIdx = (int)GfParmGetNum(ReInfo->params, RM_SECT_DRIVERS, RM_ATTR_FOCUSEDIDX, NULL, 0);
  index = 0;

  /* For each car/driver : */
  for (i = 1; i < nCars + 1; i++) 
  {
    /* Get the name of the module (= shared library) of the robot */
    snprintf(path, sizeof(path), "%s/%d", RM_SECT_DRIVERS_RACING, i);
    robotModuleName = GfParmGetStr(ReInfo->params, path, RM_ATTR_MODULE, "");
    robotIdx = (int)GfParmGetNum(ReInfo->params, path, RM_ATTR_IDX, NULL, 0);

#if 0 // SDW
    if (replayReplay)
      // Register against the Replay driver
      snprintf(path, sizeof(path), "%sdrivers/replay/replay.%s", GfLibDir(), DLLEXT);
    else
#endif
      snprintf(path, sizeof(path), "%sdrivers/%s/%s.%s", GfLibDir(), robotModuleName, robotModuleName, DLLEXT);

    /* Load the robot shared library */
    if (GfModLoad(CAR_IDENT, path, ReInfo->robModList)) 
    {
      GfLogError("Failed to load robot module %s\n", path);
      continue;
    }

    /* Load the racing driver info in the race data structure */
    elt = NULL;
    snprintf(path, sizeof(path), "%s/%d", RM_SECT_DRIVERS_RACING, i);
    if ((int)GfParmGetNum(ReInfo->params, path, RM_ATTR_EXTENDED, NULL, 0) == 0) 
    {
      /* Search for the index of the racing driver in the list of interfaces
         of the module */
      for (j = 0; j < (*(ReInfo->robModList))->modInfoSize; j++) 
      {
        if ((*(ReInfo->robModList))->modInfo[j].name && (*(ReInfo->robModList))->modInfo[j].index == robotIdx) 
        {
          /* We have the right driver : load it */
          elt = reLoadSingleCar( index, i, j, robotIdx, TRUE, robotModuleName );
          if (!elt)
		  {
            GfLogError("No descriptor file for robot %s or parameter errors (1)\n", robotModuleName);
			snprintf(buf, sizeof(buf), "Error: May be no driver, or some parameters are out of bound");
	        ReUI().addLoadingMessage(buf);
			snprintf(buf, sizeof(buf), "       Have a look at the console window for mode details about the error");
	        ReUI().addLoadingMessage(buf);
			snprintf(buf, sizeof(buf), "       Back to the config menu in 10 s ...");
	        ReUI().addLoadingMessage(buf);
			
			// Wait some time to allow the user to read the message!
            GfSleep(10.0); // 10 seconds
		  }
        }
      }
    }
    else 
    {
      GfLogTrace("Loading robot %s descriptor file\n", robotModuleName );
      snprintf(buf, sizeof(buf), "%sdrivers/%s/%s.xml", GfLocalDir(), robotModuleName, robotModuleName);
      robhdle = GfParmReadFile(buf, GFPARM_RMODE_STD);
      if (!robhdle) 
      {
        snprintf(buf, sizeof(buf), "drivers/%s/%s.xml", robotModuleName, robotModuleName);
        robhdle = GfParmReadFile(buf, GFPARM_RMODE_STD);
      }
      if (robhdle && ( strcmp( robotModuleName, "human" ) == 0 || strcmp( robotModuleName, "networkhuman" ) == 0 ) )
      {
        /* Human driver */
        elt = reLoadSingleCar( index, i, robotIdx - (*(ReInfo->robModList))->modInfo[0].index, robotIdx, FALSE, robotModuleName );
      }
      else if (robhdle && ( strcmp( GfParmGetStr( robhdle, ROB_SECT_ARBITRARY, ROB_ATTR_TEAM, "foo" ),
                              GfParmGetStr( robhdle, ROB_SECT_ARBITRARY, ROB_ATTR_TEAM, "bar" ) ) == 0 ) )
      {
        elt = reLoadSingleCar( index, i, (*(ReInfo->robModList))->modInfoSize, robotIdx, FALSE, robotModuleName );
      }
      else
        GfLogError("No descriptor for robot %s (2)\n", robotModuleName );
    }

    if (elt)
      ++index;
  }

  nCars = index; /* real number of cars */
  if (nCars == 0) 
  {
    GfLogError("No driver for that race ; exiting ...\n");
    return -1;
  }
  else 
  {
    GfLogInfo("%d driver(s) ready to race\n", nCars);
  }

  if (replayReplay)
    replayRecord = 0;
  else {
        char buf[1024];
	const char *replayRateSchemeName;
        snprintf(buf, sizeof(buf), "%s%s", GfLocalDir(), RACE_ENG_CFG);

        void *paramHandle = GfParmReadFile(buf, GFPARM_RMODE_REREAD | GFPARM_RMODE_CREAT);
        replayRateSchemeName = GfParmGetStr(paramHandle, RM_SECT_RACE_ENGINE, RM_ATTR_REPLAY_RATE, "0");
        GfParmReleaseHandle(paramHandle);

	replayRecord = atoi(replayRateSchemeName);
  }

  if (replayRecord || replayReplay) {
#ifdef THIRD_PARTY_SQLITE3
    int result;

    result = sqlite3_open("/tmp/race.sqlite", &replayDB);
    if (result) {
      GfLogError("Replay: Unable to open Database: %s\n", sqlite3_errmsg(replayDB));
      sqlite3_close(replayDB);
      replayDB = NULL;
    } else {
      GfLogInfo("Replay: Database Opened 0x8%8.8X\n", replayDB);

      if (replayRecord)
        GfLogInfo("Replay: Record Timestep = %f\n", 1/(float)replayRecord);

      if (replayReplay)
        GfLogInfo("Replay: Playback from file\n");

      /* speed up database by turning of synchronous behaviour/etc */
      sqlite3_exec(replayDB, "PRAGMA synchronous = OFF", NULL, NULL, NULL);
      sqlite3_exec(replayDB, "PRAGMA journal_mode = OFF", NULL, NULL, NULL);
      sqlite3_exec(replayDB, "PRAGMA count_changes = OFF", NULL, NULL, NULL);
#if 0 // This pragma seems to prevent re-opening the sqlite3 database
      sqlite3_exec(replayDB, "PRAGMA locking_mode = EXCLUSIVE", NULL, NULL, NULL);
#endif
      sqlite3_exec(replayDB, "PRAGMA default_temp_store = MEMORY", NULL, NULL, NULL);

      //replayBlobs = (sqlite3_stmt *) calloc(nCars, sizeof(void *)); //sqlite3_stmt));

      replayTimestamp = -5;
      ghostcarActive = 0;
    }
#endif
  }

  ReInfo->s->_ncars = nCars;
  FREEZ(ReInfo->s->cars);
  ReInfo->s->cars = (tCarElt **)calloc(nCars, sizeof(tCarElt *));
  for (i = 0; i < nCars; i++)
  {
    ReInfo->s->cars[i] = &(ReInfo->carList[i]);

#ifdef THIRD_PARTY_SQLITE3
    //open a table for each car
    if (replayDB) {
      char command[200];
      int result;

      if (replayRecord) {
        sprintf(command, "DROP TABLE IF EXISTS car%d", i);
        result = sqlite3_exec(replayDB, command, 0, 0, 0);
        if (result) GfLogInfo("Replay: Unable to drop table car%d: %s\n", i, sqlite3_errmsg(replayDB));
      }

      sprintf(command, "CREATE TABLE IF NOT EXISTS car%d (timestamp, lap, datablob BLOB)", i);
      result = sqlite3_exec(replayDB, command, 0, 0, 0);
      if (result) {
         GfLogInfo("Replay: Unable to create table car%d: %s\n", i, sqlite3_errmsg(replayDB));
         exit(0);
      }

      if (replayReplay) {
        // Build index to allow faster read access
        sprintf(command, "CREATE UNIQUE INDEX IF NOT EXISTS index%d ON car%d (timestamp)", i, i);
        result = sqlite3_exec(replayDB, command, 0, 0, 0);
        if (result) GfLogInfo("Replay: Unable to create index car%d: %s\n", i, sqlite3_errmsg(replayDB));
      }
    }
#endif
  }

	// webServer lap logger.
	//Find human cars
	for (int i = 0; i < ReInfo->s->_ncars; i++) {
		if(ReInfo->s->cars[i]->_driverType == RM_DRV_HUMAN){
			
			//login
			webServer.sendLogin(ReInfo->s->cars[i]->_driverIndex);
			
			//send race data
			webServer.sendRaceStart (
				ReInfo->s->cars[i]->_skillLevel,	//user_skill,
				ReInfo->track->internalname,		//track_id,
				ReInfo->s->cars[i]->_carName,		//car_id
				ReInfo->s->_raceType,				//type of race: 0 practice/ 1 qualify/ 2 race
				ReInfo->s->cars[i]->_carHandle,		//car setup file,
				ReInfo->s->cars[i]->_pos,			//car starting position,
				VERSION_LONG 						//speed dreams version
			);
		}
	}



  ReInfo->_rePitRequester = 0;

  // TODO: reconsider splitting the call into one for cars, track and maybe other objects.
  // I stuff for now anything into one call because collision detection works with the same
  // library on all objects, so it is a bit dangerous to distribute the handling to various
  // locations (because the library maintains global state like a default collision handler etc.).
  RePhysicsEngine().initialize(nCars, ReInfo->track);

  initStartingGrid();

  initPits();

  return 0;
}

void
ReRaceCleanup(void)
{
  RePhysicsEngine().shutdown();
  StandardGame::self().unloadPhysicsEngine();

  ReStoreRaceResults(ReInfo->_reRaceName);

  ReRaceCleanDrivers();

#ifdef THIRD_PARTY_SQLITE3
  GfLogInfo("Replay: Database closed\n");
  if (replayDB)
    sqlite3_close(replayDB);

  replayDB = NULL;
#endif
  replayRecord = 0;
}


void
ReRaceCleanDrivers(void)
{
  int i;
  tRobotItf *robot;
  int nCars;
  tMemoryPool oldPool = NULL;

  nCars = ReInfo->s->_ncars;
  for (i = 0; i < nCars; i++) 
  {
    robot = ReInfo->s->cars[i]->robot;
    GfPoolMove( &ReInfo->s->cars[i]->_shutdownMemPool, &oldPool );
    if (robot->rbShutdown && !(ReInfo->_displayMode & RM_DISP_MODE_SIMU_SIMU))
    {
      robot->rbShutdown(robot->index);
    }
    GfPoolFreePool( &oldPool );
    GfParmReleaseHandle(ReInfo->s->cars[i]->_paramsHandle);
    free(robot);
    free(ReInfo->s->cars[i]->_curSplitTime);
    free(ReInfo->s->cars[i]->_bestSplitTime);
  }
  RtTeamManagerRelease();

  FREEZ(ReInfo->s->cars);
  ReInfo->s->cars = 0;
  ReInfo->s->_ncars = 0;
  GfModUnloadList(&ReRacingRobotsModList);
}

// Get the name of the current "race"
// (actually the current "race session", like quali.1, quali2, ... 1st race, ...).
char *
ReGetCurrentRaceName(void)
{
	char path[64];
    int   curRaceIdx;
    void  *params = ReInfo->params;
    void  *results = ReInfo->results;

    curRaceIdx = (int)GfParmGetNum(results, RE_SECT_CURRENT, RE_ATTR_CUR_RACE, NULL, 1);
    snprintf(path, sizeof(path), "%s/%d", RM_SECT_RACES, curRaceIdx);

    return GfParmGetStrNC(params, path, RM_ATTR_NAME, 0);
}

// Get the previous "race" (actually the previous "race session").
char *
ReGetPrevRaceName(bool bLoop)
{
	char path[64];
    int   curRaceIdx;
    void  *params = ReInfo->params;
    void  *results = ReInfo->results;

    curRaceIdx = (int)GfParmGetNum(results, RE_SECT_CURRENT, RE_ATTR_CUR_RACE, NULL, 1) - 1;
	if (bLoop && curRaceIdx <= 0)
		curRaceIdx = (int)GfParmGetEltNb(params, RM_SECT_RACES);
    snprintf(path, sizeof(path), "%s/%d", RM_SECT_RACES, curRaceIdx);

    return GfParmGetStrNC(params, path, RM_ATTR_NAME, 0);
}
