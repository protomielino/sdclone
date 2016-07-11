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

bool timeLogged = false;
clock_t effectStart = std::clock();
clock_t effectCurTime = std::clock();

float prevSteerCmd;
float prevSteerCmdDiff;
int prevDirection = 1;

int filterPositiveNumbers (int number){
	if (number > 0){
		return number;
	}else{
		return 0;
	}
}


ForceFeedbackManager::ForceFeedbackManager(){

	this->initialized = false;

}
ForceFeedbackManager::~ForceFeedbackManager(){
	// iterate on the first map
	typedef std::map<std::string, std::map<std::string, int> >::iterator it_type;
	for(it_type iterator = this->effectsConfig.begin(); iterator != this->effectsConfig.end(); iterator++) {
		// iterator->first = key (effect type name)
		// iterator->second = value (second map)

		//clear the sub-map
		iterator->second.clear();
	}
	
	//clear the map
	effectsConfig.clear();

}
void ForceFeedbackManager::readConfiguration(std::string carName){
	//use the user specified configuration for the specified car
	//or use the user specified global configuration
	//or use the default configuration for the car
	//or use the default global configuration
	this->carName = carName;

	std::string configFileUrl = GfLocalDir();
	configFileUrl.append("/drivers/human/preferences.xml");
	
	std::string effectsSectionPathDefault = "forceFeedback/default/effectsConfig";
	
	std::string effectsSectionPathSpecific = "forceFeedback/";
	effectsSectionPathSpecific.append(carName);
	effectsSectionPathSpecific.append("/effectsConfig");
	
	//remove the previous stored configuration (if any)
	this->effectsConfig.clear();
	
	//add some needed default configuration
	this->effectsConfig["autocenterEffect"]["previousValue"] = 1;
	this->effectsConfig["bumpsEffect"]["initialized"] = 0;

	//read the default configuration (this should always exist)
	this->readConfigurationFromFileSection(configFileUrl, effectsSectionPathDefault);

	//merge the current configuration with the read car specific configuration
	//if there is one
	void *paramHandle = GfParmReadFile(configFileUrl.c_str(), GFPARM_RMODE_STD);
	if(GfParmExistsSection(paramHandle, effectsSectionPathSpecific.c_str())){
		this->readConfigurationFromFileSection(configFileUrl, effectsSectionPathSpecific);
	}
	GfParmReleaseHandle(paramHandle);

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
	GfParmReleaseHandle(paramHandle);


}
void ForceFeedbackManager::saveConfiguration(){

	std::string configFileUrl = GfLocalDir();
	configFileUrl.append("/drivers/human/preferences.xml");
	
	std::string effectsSectionPathSpecific = "/forceFeedback/";
	effectsSectionPathSpecific.append(carName);
	
	//open the file
	void *paramHandle = GfParmReadFile(configFileUrl.c_str(), GFPARM_RMODE_STD);
	
	//delette the current car specific section if it exist
	if(GfParmExistsSection(paramHandle, effectsSectionPathSpecific.c_str())){
		//delette the section
		//GfParmListRemoveElt (void *handle, const char *path, const char *key)
		GfParmListClean(paramHandle, effectsSectionPathSpecific.c_str());
	}
	
	effectsSectionPathSpecific.append("/effectsConfig");
	//now recreate the whole car section
	
	// iterate on the first map
	typedef std::map<std::string, std::map<std::string, int> >::iterator it_type;
	for(it_type iterator = this->effectsConfig.begin(); iterator != this->effectsConfig.end(); iterator++) {
		// iterator->first = key (effect type name)
		// iterator->second = value (second map)
		
		// now iterate on the second map
		typedef std::map<std::string, int>::iterator it_type2;
		for(it_type2 iterator2 = iterator->second.begin(); iterator2 != iterator->second.end(); iterator2++) {
			// iterator2->first = key (effect parameter name)
			// iterator2->second = value (effect value)
			
			std::string effectPath = "";
			
			if ( iterator->first.compare("globalEffect") == 0){
				//save global setting in the default section
				effectPath.append("/forceFeedback/default/effectsConfig/");
				effectPath.append(iterator->first.c_str());
			}else{
				//save other settings in car specific section
				effectPath.append(effectsSectionPathSpecific);
				effectPath.append("/");
				effectPath.append(iterator->first.c_str());
			}


			//remove the first slash
			effectPath.erase(0,1);
			
			GfParmSetNum(paramHandle, effectPath.c_str(), iterator2->first.c_str(), "", iterator2->second);

		}
	}
	
	//write changes
	GfParmWriteFile(NULL,paramHandle,"preferences");
	GfParmReleaseHandle(paramHandle);

}
int ForceFeedbackManager::updateForce(tCarElt* car, tSituation *s){

	this->force = 0;

	//calculate autocenter if enabled
	if (this->effectsConfig["autocenterEffect"]["enabled"]){
		this->force = this->autocenterEffect(car, s);
		GfLogInfo("After autocenter: (%i)\n", this->force);
	}
	
	//calculate engine revving if enabled
	if (this->effectsConfig["engineRevvingEffect"]["enabled"]){
		this->force += this->engineRevvingEffect(car, s);
		GfLogInfo("After engineRevving: (%i)\n", this->force);
	}

	//calculate engine revving if enabled
	//if (this->effectsConfig["engineRevvingEffect"]["enabled"]){
		this->force += this->lowSpeedCostantForceEffect(car, s);
	//	GfLogInfo("After engineRevving: (%i)\n", this->force);
	//}
	
	//calculate bump
	//this->force += this->bumpsEffect(car, s);
	//GfLogInfo("After bump: (%i)\n", this->force);

	//apply global effect
	//multiply
	this->force = this->force * this->effectsConfig["globalEffect"]["multiplier"] / 1000;
	
	//reverse if needed
	if(this->effectsConfig["globalEffect"]["reverse"] == 1){
		this->force = -1 * this->force;
	}
	
	GfLogInfo("Final force: (%i)\n", this->force);

	return this->force;

}

int ForceFeedbackManager::autocenterEffect(tCarElt* car, tSituation *s){
	
	
	if(car->_speed_xy < 4){
		return 0;
	}
	/*
	 * car->steerLock
	 * */
	
	int effectForce;
	int sign;
	tdble TqAlign;
	
	//force acting on the front wheels
	tdble H = 450.0;
	TqAlign = car->_steerTqAlign;
	TqAlign = H * TqAlign / (fabs(TqAlign) + H);
	effectForce = TqAlign * this->effectsConfig["autocenterEffect"]["frontwheelsmultiplier"] / 1000; 

	//force action on the back wheels
	effectForce += car->_wheelFy(REAR_RGT) * this->effectsConfig["autocenterEffect"]["rearwheelsmultiplier"] / 1000;
	effectForce += car->_wheelFy(REAR_LFT) * this->effectsConfig["autocenterEffect"]["rearwheelsmultiplier"] / 1000;
	
	//smooth
	effectForce = (effectForce + (this->effectsConfig["autocenterEffect"]["previousValue"] * this->effectsConfig["autocenterEffect"]["smoothing"] / 1000)) / ((this->effectsConfig["autocenterEffect"]["smoothing"]/1000)+1);

	//remember the current value for the next run
	this->effectsConfig["autocenterEffect"]["previousValue"] = effectForce;
	
	//we need to store the sign of the force
	sign = (effectForce > 0) - (effectForce < 0);
	
	//be sure this is a positive number
	//effectForce = effectForce * sign;

	//we use an inverse exponential function to have a stronger force at low values and a lighetr one a bigger values
	//effectForce = (int)((pow((double) effectForce, (double) 1/2) * 120) * sign);

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

int ForceFeedbackManager::engineRevvingEffect(tCarElt* car, tSituation *s){

	int effectForce;
	double changeTimeInterval = 20;

	if(timeLogged != true){
		effectStart = std::clock();
		timeLogged = true;
	GfLogInfo("StartTime: (%f)\n",(double)effectStart);
		GfLogInfo("###############new time\n");
	GfLogInfo("StartTime: (%f)\n",(double)effectStart);

	}
	effectCurTime = std::clock();
	
	double timeDiff = (((double)effectCurTime - (double)effectStart )) / CLOCKS_PER_SEC * 1000;
	
	GfLogInfo("CurTime: (%f)\n", (double)effectCurTime);
	GfLogInfo("StartTime: (%f)\n",(double)effectStart);
	GfLogInfo("TimeDiff: (%f)\n", timeDiff);
	
	if (timeDiff > 40){
		if( this->effectsConfig["engineRevvingEffect"]["previousSign"] > 0 ){
			this->effectsConfig["engineRevvingEffect"]["previousSign"] = -1;
		}else{
			this->effectsConfig["engineRevvingEffect"]["previousSign"] = 1;		
		}
		
		effectStart = std::clock();
	}

	GfLogInfo("Sign: (%i)\n", this->effectsConfig["engineRevvingEffect"]["previousSign"]);

	//force acting on the front wheels
	effectForce = 50000 / (int)car->_enginerpm * 2 * this->effectsConfig["engineRevvingEffect"]["previousSign"] * this->effectsConfig["engineRevvingEffect"]["multiplier"] / 1000; 
	
	GfLogInfo("RPM: (%i)\n", (int)car->_enginerpm);
	GfLogInfo("Efect: (%i)\n", effectForce);
	
	return effectForce;

}

int ForceFeedbackManager::lowSpeedCostantForceEffect(tCarElt* car, tSituation *s){

	int effectForce;
	int sign;

	//we need to store the sign of the force
	sign = ((car->_steerTqCenter - prevSteerCmd) > 0) - ((car->_steerTqCenter - prevSteerCmd) < 0);

	GfLogInfo("test: (%f)\n", car->_steerTqCenter);
	GfLogInfo("test: (%f)\n", prevSteerCmd );

	int prevDirectionSign = (prevDirection > 0) - (prevDirection < 0);

	GfLogInfo("Sign: (%d)\n", sign);
	GfLogInfo("Direction sign: (%d)\n", prevDirectionSign);	


/*	
	if(prevDirectionSign == sign || sign == 0){
		
		prevDirection = prevDirection + sign;

	}else{

		prevDirection = sign;

	}
*/

	prevDirection = prevDirection + sign;
	if (prevDirection > 7) prevDirection =7;
	if (prevDirection < -7) prevDirection =-7;
	



	GfLogInfo("Direction score: (%d)\n", prevDirection);


	//force calculation
	if (car->_speed_xy < this->effectsConfig["lowSpeedCostantForceEffect"]["maxSpeedAtWithcForceIsApplied"]
//		&& abs(prevDirection) > 8
	){

		effectForce =
				this->effectsConfig["lowSpeedCostantForceEffect"]["maxForce"] / 8 * abs(prevDirection) /
				//(car->_speed_xy  + 1) * 
				(pow(car->_speed_xy, (float) 1/2) + 1) *
				//sign;
				prevDirectionSign;
		
	}else{

		effectForce = 1;

	}

	prevSteerCmdDiff = car->_steerTqCenter - prevSteerCmd;
	prevSteerCmd = car->_steerTqCenter;

	GfLogInfo("SPEED: (%i)\n", (int)car->_speed_xy);
	GfLogInfo("Efect: (%i)\n", effectForce);

	return effectForce;

}
//initialize the force feedback
TGFCLIENT_API ForceFeedbackManager forceFeedback;
