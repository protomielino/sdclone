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

#include <playerpref.h>
#include <tgf.h>
#include "tgfclient.h"
#include "forcefeedback.h"

int filterPositiveNumbers (int number){
	if (number > 0){
		return number;
	}else{
		return 0;
	}
}


ForceFeedbackManager::ForceFeedbackManager(){
	this->initialized = false;


	// ??log the initialization time ??
}
ForceFeedbackManager::~ForceFeedbackManager(){

}
void ForceFeedbackManager::readConfiguration(tCarElt* car){
	//use the user specified configuration for the specified car
	//or use the user specified global configuration
	//or use the default configuration for the car
	//or use the default global configuration

	std::string configFileUrl = GfLocalDir();
	configFileUrl.append("/drivers/human/preferences.xml");
	
	std::string effectsSectionPathDefault = "forceFeedback/default/effectsConfig";
	
	std::string effectsSectionPathSpecific = "forceFeedback/";
	effectsSectionPathSpecific.append(car->_carName);
	effectsSectionPathSpecific.append("/effectsConfig");
	
	//remove the previous stored configuration
	this->effectsConfig.clear();
	
	//add some needed default configuration
	this->effectsConfig["autocenterEffect"]["previousValue"] = 1;
	this->effectsConfig["bumpsEffect"]["initialized"] = 0;

	//read the default configuration (this should always exist)
	this->readConfigurationFromFileSection(configFileUrl, effectsSectionPathDefault);

	//merge the current configuration with the read the car specific configuration
	//if there is one
	void *paramHandle = GfParmReadFile(configFileUrl.c_str(), GFPARM_RMODE_STD);
	if(GfParmExistsSection(paramHandle, effectsSectionPathSpecific.c_str())){
		this->readConfigurationFromFileSection(configFileUrl, effectsSectionPathSpecific);
	}

	//now we are correctly initialized
	this->initialized = true;
}
void ForceFeedbackManager::readConfigurationFromFileSection(std::string configFileUrl, std::string effectsSectionPath){

	std::vector<std::string> effectSectionsPath;
	std::string subSectionPath = "";
	std::string subSectionName = "";
	int paramValue = 0;

	//open the file
	void *paramHandle = GfParmReadFile(configFileUrl.c_str(), GFPARM_RMODE_STD);
	
	//for each section on the effectConfig section
	if (GfParmListSeekFirst(paramHandle, effectsSectionPath.c_str()) == 0) {
		do {
			subSectionName = GfParmListGetCurEltName(paramHandle, effectsSectionPath.c_str());
			subSectionPath = effectsSectionPath + "/" + subSectionName;
					
			//get a list of the params in this section
			 std::vector<std::string> paramsInSection = GfParmListGetParamsNamesList(paramHandle, subSectionPath.c_str());		

			GfLogInfo ("=== (%s) ===\n", subSectionPath.c_str());
			
			//for each param
			for (int i = 0; i < paramsInSection.size(); i++) {

				paramValue = (int)GfParmGetNum(paramHandle, subSectionPath.c_str(), paramsInSection[i].c_str(), "null", 0);
				GfLogInfo ("(%s): (%i)\n", paramsInSection[i].c_str(), paramValue);
				this->effectsConfig[subSectionName.c_str()][paramsInSection[i]] = paramValue;
			 }
			
		} while (GfParmListSeekNext(paramHandle, effectsSectionPath.c_str()) == 0);
	}

}
int ForceFeedbackManager::updateForce(tCarElt* car, tSituation *s){

	//calculate autocenter
	this->force = this->autocenterEffect(car, s);
	GfLogInfo("After autocenter: (%i)\n", this->force);

	//calculate bump
	//this->force += this->bumpsEffect(car, s);
	//GfLogInfo("After bump: (%i)\n", this->force);


	//apply global effect
	//multiply
	this->force = this->force * this->effectsConfig["globalEffect"]["multiplier"] / 1000;
//	GfLogInfo("Final multiply: (%i)\n", this->force);
	
	//reverse if needed
	if(this->effectsConfig["globalEffect"]["reverse"] == 1){
		this->force = -1 * this->force;
	}
//	GfLogInfo("Final reverse: (%i)\n", this->force);


//	GfLogInfo("Final force: (%i)\n", this->force);

	return this->force;

}

int ForceFeedbackManager::autocenterEffect(tCarElt* car, tSituation *s){
	
	/*
	 * car->steerLock
	 * */
	
	
	int effectForce;
	
	//force acting on the front wheels
	effectForce = car->_steerTq; 

	//force action on the back wheels
	//effectForce += car->_wheelFy(REAR_RGT);
	//effectForce += car->_wheelFy(REAR_LFT);

	
//	GfLogInfo("Autocenter tq: (%i)\n", effectForce);

	//multiply
	effectForce = effectForce * this->effectsConfig["autocenterEffect"]["multiplier"] / 1000;
//	GfLogInfo("Autocenter multipli: (%i)\n", effectForce);
	
	//smooth
	effectForce = (effectForce + (this->effectsConfig["autocenterEffect"]["previousValue"] * this->effectsConfig["autocenterEffect"]["multiplier"] / 1000)) / ((this->effectsConfig["autocenterEffect"]["multiplier"]/1000)+1);
//	GfLogInfo("Autocenter smooth: (%i)\n", effectForce);

	//remember the current value for the next run
	this->effectsConfig["autocenterEffect"]["previousValue"] = effectForce;
	
//	GfLogInfo("Autocenter: (%i)\n", effectForce);
	
	return effectForce;

}

int ForceFeedbackManager::bumpsEffect(tCarElt* car, tSituation *s){
	/*
	 * wheel
	 * 
	 * */
	int effectForce;
/*
	GfLogInfo("\n\n");
	GfLogInfo("(%f)\n",car->_wheelFz(0));
	GfLogInfo("(%f)\n",car->_wheelFz(1));
	GfLogInfo("(%f)\n",car->_wheelFz(2));
	GfLogInfo("(%f)\n",car->_wheelFz(3));
*/
	if(this->effectsConfig["bumpsEffect"]["initialized"] == 0){
		this->effectsConfig["bumpsEffect"]["previousWheelZForce0"] = car->_wheelFz(0);
		this->effectsConfig["bumpsEffect"]["previousWheelZForce1"] = car->_wheelFz(1);
		this->effectsConfig["bumpsEffect"]["previousWheelZForce2"] = car->_wheelFz(2);
		this->effectsConfig["bumpsEffect"]["previousWheelZForce3"] = car->_wheelFz(3);
		this->effectsConfig["bumpsEffect"]["initialized"] = 1;
	}

	//fl/fr/rl/rr
/*
	int left = filterPositiveNumbers(this->effectsConfig["bumpsEffect"]["previousWheelZForce0"] - car->_wheelFz(0)) +
				filterPositiveNumbers(this->effectsConfig["bumpsEffect"]["previousWheelZForce2"] - car->_wheelFz(2));
			
	int right = filterPositiveNumbers(this->effectsConfig["bumpsEffect"]["previousWheelZForce1"] - car->_wheelFz(1)) +
				filterPositiveNumbers(this->effectsConfig["bumpsEffect"]["previousWheelZForce3"] - car->_wheelFz(3));
*/

	int left = filterPositiveNumbers(this->effectsConfig["bumpsEffect"]["previousWheelZForce0"] - car->_wheelFz(0));			
	int right = filterPositiveNumbers(this->effectsConfig["bumpsEffect"]["previousWheelZForce1"] - car->_wheelFz(1));



	this->effectsConfig["bumpsEffect"]["previousWheelZForce0"] = car->_wheelFz(0);
	this->effectsConfig["bumpsEffect"]["previousWheelZForce1"] = car->_wheelFz(1);
	this->effectsConfig["bumpsEffect"]["previousWheelZForce2"] = car->_wheelFz(2);
	this->effectsConfig["bumpsEffect"]["previousWheelZForce3"] = car->_wheelFz(3);


	GfLogInfo("\n\n");
	GfLogInfo("(%i)\n",left);
	GfLogInfo("(%i)\n",right);
	
	if( left > 4000){
		effectForce = -10000;
	}else if (right > 4000){
		effectForce = 10000;
	}


	
/*
	GfLogInfo("\n\n");
	GfLogInfo("(%f)\n",car->_ride(0));
	GfLogInfo("(%f)\n",car->_ride(1));
	GfLogInfo("(%f)\n",car->_ride(2));
	GfLogInfo("(%f)\n",car->_ride(3));
*/
/*
	if(this->effectsConfig["bumpsEffect"]["initialized"] == 0){
		this->effectsConfig["bumpsEffect"]["previousWheelRide0"] = (int) (car->_ride(0)*1000);
		this->effectsConfig["bumpsEffect"]["previousWheelRide1"] = (int) (car->_ride(1)*1000);
		this->effectsConfig["bumpsEffect"]["previousWheelRide2"] = (int) (car->_ride(2)*1000);
		this->effectsConfig["bumpsEffect"]["previousWheelRide3"] = (int) (car->_ride(3)*1000);
		this->effectsConfig["bumpsEffect"]["initialized"] = 1;
	}

	//fl/fr/rl/rr

	int left = this->effectsConfig["bumpsEffect"]["previousWheelRide0"] - (car->_ride(0)*1000) +
				this->effectsConfig["bumpsEffect"]["previousWheelRide2"] - (car->_ride(2)*1000);
			
	int right = this->effectsConfig["bumpsEffect"]["previousWheelRide1"] - (car->_ride(1)*1000) +
				this->effectsConfig["bumpsEffect"]["previousWheelRide3"] - (car->_ride(3)*1000);

	this->effectsConfig["bumpsEffect"]["previousWheelRide0"] = (int) (car->_ride(0)*1000);
	this->effectsConfig["bumpsEffect"]["previousWheelRide1"] = (int) (car->_ride(1)*1000);
	this->effectsConfig["bumpsEffect"]["previousWheelRide2"] = (int) (car->_ride(2)*1000);
	this->effectsConfig["bumpsEffect"]["previousWheelRide3"] = (int) (car->_ride(3)*1000);


	GfLogInfo("\n\n");
	GfLogInfo("(%i)\n",left);
	GfLogInfo("(%i)\n",right);
*/
	return effectForce;

}
//initialize the force feedback
ForceFeedbackManager forceFeedback;
