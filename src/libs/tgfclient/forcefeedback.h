/***************************************************************************
                    forcefeedback.h -- Interface file for The Gaming Framework
                             -------------------
    created              : 06/03/2015
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
/*to be checked*/
#include <map>
#include <vector>
#include <string>
#include <ctime>
#include <car.h> //tCarElt
#include <raceman.h> //tSituation


//#include <tgfclient.h>
//#include <portability.h>
/*
//#include "robottools.h"	//Rt*
#include <robot.h>
#include <playerpref.h>
#include <car.h>

#include "humandriver.h"
*/
/*to be checked:end*/

struct forceFeedBackEffect_t {
	std::string name; //a name for the effect
	std::clock_t startTime; //when we have started this effect
	std::clock_t lastExecTime; //the last time the effect was updated/run
	std::clock_t duration; //how mich time the effect should last
};


class ForceFeedbackManager {

	public:
		void readConfiguration(std::string carName);
		void saveConfiguration();
		int updateForce(tCarElt* car, tSituation *s);
		bool initialized;
		int force;
		int reversed;
		std::vector<std::string> effects;
		std::map< std::string, std::map<std::string, int> > effectsConfig;
		std::string carName;

		//constructor
		ForceFeedbackManager();
	
		//destructor
		~ForceFeedbackManager();


	private:
		//std::vector<std::string> msglist;
		void readConfigurationFromFileSection(std::string configFileUrl, std::string effectsSectionPath);

		std::clock_t lastExecTime; //the current time
		
		void* menuXMLDescHdle;

		std::clock_t animationStartTime; //when the animation started
		std::clock_t animationRestStartTime; //when the animation started


		int autocenterEffect(tCarElt* car, tSituation *s);
		int bumpsEffect(tCarElt* car, tSituation *s);
		int globalMultiplier;

};
