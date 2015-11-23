/***************************************************************************
                    webserver.h -- Interface file for The Gaming Framework
                             -------------------
    created              : 04/11/2015
    copyright            : (C) 2015 by MadBad
    email                : madbad82@gmail.com
    version              : $Id$
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
        The Gaming Framework API (client part).
    @author     <a href=mailto:madbad82@gmail.com>MadBad</a>
    @version    $Id$
*/
#include <vector>
#include <string>
#include <sstream>
#include <ctime>
#include <tgf.h>
#include "tgfclient.h"

class NotificationManager {

	public:
		//a list of notification messages
		std::vector<std::string> msglist;	
	
		//constructor
		NotificationManager();
	
		//destructor
		~NotificationManager();

		void updateStatus();


	private:
		void startNewNotification();
		void runAnimation();
		void removeOldUi();
		void createUi();
		void updateWebserverStatusUi();
		
		void* screenHandle;
		void* prevScreenHandle;	
		void* menuXMLDescHdle;
		int	notifyUiIdBg;//the bg image uiid
		int notifyUiIdBusyIcon; //the webserver busy icon
		std::vector<int> notifyUiId;//the text lines uiid
		bool busy;
		int textPadding;
		std::clock_t animationStartTime; //when the animation started
		std::clock_t animationRestStartTime; //when the animation started
		std::clock_t animationLastExecTime; //the current time
		float totalAnimationDuration;//how much the animation should take to fully run in one direction
		float animationRestTime; //how much wes should wait when we a re fully displayed
		int animationDirection;
		int propertyFinalValue;
		std::vector<std::string> messageLines;
		int propertyChangeNeeded;
		
};





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


