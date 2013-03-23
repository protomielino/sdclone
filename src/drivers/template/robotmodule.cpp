/***************************************************************************

    created     : Sat March 16 19:12:46 CEST 2003
    copyright   : (C) 2013 by Jean-Philippe Meuret
    web         : www.speed-dreams.org
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
    	The template robot module interface to Speed Dreams	
*/

#include <sstream>

#include <irobot.h>
#include <robottools.h>

#include "driverset.h"
#include "robotmodule.h"


// The singleton.
Robot* Robot::_pSelf = 0;

// Initialise the robot-dedicated logger instance.
GfLogger& Robot::logger = *GfLogger::instance("TmplRobot");

// Open the module (mandatory C interface of the module DLL).
int openGfModule(const char* pszShLibName, void* hShLibHandle)
{
	// Instanciate the (only) module instance.
	Robot::_pSelf = new Robot(pszShLibName, hShLibHandle);

	// Register it to the GfModule module manager if OK.
	if (Robot::_pSelf)
		GfModule::register_(Robot::_pSelf);

	// Report about success or error.
	return Robot::_pSelf ? 0 : 1;
}

// Close the module (mandatory C interface of the module DLL).
int closeGfModule()
{
	// Unregister it from the GfModule module manager.
	if (Robot::_pSelf)
		GfModule::unregister(Robot::_pSelf);

	// Delete the (only) module instance.
	delete Robot::_pSelf;
	Robot::_pSelf = 0;

	// Report about success or error.
	return 0;
}

// Robot& Robot::self()
// {
// 	// Pre-condition : 1 successfull openGfModule call.
// 	return *_pSelf;
// }

Robot::Robot(const std::string& strShLibName, void* hShLibHandle)
: GfModule(strShLibName, hShLibHandle)
{
	// Set default name.
	_strName = "template";
	
	// All done.
	logger.info("'%s' robot module created.\n", _strName.c_str());
}

Robot::~Robot()
{
	// Delete driver sets.
	std::vector<IDriverSet*>::iterator itDrvSet;
	for (itDrvSet = _vecDriverSets.begin(); itDrvSet != _vecDriverSets.end(); itDrvSet++)
		delete dynamic_cast<DriverSet*>(*itDrvSet);
	
	// All done.
	logger.info("'%s' robot module destroyed.\n", _strName.c_str());
}

// Implementation of IRobot.
const std::vector<IDriverSet*>& Robot::getDriverSets()
{
	// Don't re-read : if already done, simply return it.
	if (!_vecDriverSets.empty())
		return _vecDriverSets;
	
	// Get the list of sub-dirs in the data folder of the robot :
	// apart from . and .., there should be 1 folder for each available driver set.
	std::ostringstream ossDataDir;
	ossDataDir << GfDataDir() << "drivers";
	tFList* lstFolders = GfDirGetListFiltered(ossDataDir.str().c_str(), _strName.c_str(), 0);
	if (!lstFolders)
	{
		logger.warning("No '%s' driver set data folder in %s ; will ignore this bot\n",
					   _strName.c_str(), ossDataDir.str().c_str());
		return _vecDriverSets; // It is empty.
	}

	// For each sub-folder, load driver set data if possible.
	tFList* pFolder = lstFolders;
	do 
	{
		// Try and open the driver set XML file (named as the folder itself,
		// or simply "driverset", not talking about the file extension, which must be PARAMEXT).
		std::ostringstream ossSetDescFile;
		ossSetDescFile << ossDataDir.str() << "/" << pFolder->name
					   << "/" << pFolder->name << PARAMEXT;
		if (!GfFileExists(ossSetDescFile.str().c_str()))
		{
			ossSetDescFile.str("");
			ossSetDescFile << ossDataDir.str() << "/" << pFolder->name << "/driverset" << PARAMEXT;
			if (!GfFileExists(ossSetDescFile.str().c_str()))
			{
				logger.warning("No driver set %s descriptor file in data folder '%s/%s' ; will ignore this driver set\n",
								 PARAMEXT, ossDataDir.str().c_str(), pFolder->name);
				continue;
			}
		}

		void* hparmSet = GfParmReadFile(ossSetDescFile.str().c_str(), GFPARM_RMODE_STD);
		if (!hparmSet)
		{
			logger.warning("Ignoring driver set %s (file %s not readable)\n",
							 pFolder->name, ossSetDescFile.str().c_str());
			continue;
		}

		// Create the driver set instance and set its data from the descriptor file.
		DriverSet* pDriverSet = new DriverSet(pFolder->name, hparmSet);

		// Register the set.
		_vecDriverSets.push_back(pDriverSet);

		// Close the descriptor file.
		GfParmReleaseHandle(hparmSet);

	} // Next sub-folder.
	while ((pFolder = pFolder->next) != lstFolders);
	
	// All done.
	return _vecDriverSets;
}

