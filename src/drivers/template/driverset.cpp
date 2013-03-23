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
    	The driver set class of the template robot module	
*/

#include "robotmodule.h"

#include "driver.h"
#include "driverset.h"


DriverSet::DriverSet(const std::string& strName, void* hparmSet)
: _strName(strName)
{
	const char* pszDrvListPath = ROB_SECT_ROBOTS"/"ROB_LIST_INDEX;
	Robot::logger.info("Driver set '%s' : %d drivers\n",
					   _strName.c_str(), GfParmGetEltNb(hparmSet, pszDrvListPath));

	if (GfParmListSeekFirst(hparmSet, pszDrvListPath))
		return;

	// Set up drivers from settings in file.
	do
	{
		Driver* pDriver = new Driver(this, hparmSet);
		_vecDrivers.push_back(pDriver);
	}
	while(!GfParmListSeekNext(hparmSet, pszDrvListPath));
}

DriverSet::~DriverSet()
{
	// Delete drivers.
	std::vector<IDriver*>::iterator itDrv;
	for (itDrv = _vecDrivers.begin(); itDrv != _vecDrivers.end(); itDrv++)
		delete dynamic_cast<Driver*>(*itDrv);
	
	// All done.
	Robot::logger.info("'%s' driver set destroyed.\n", _strName.c_str());
}

// void DriverSet::setupDriverSet(const std::string strSetName)
// {
// }

// Implementation of IDriverSet.
const std::string& DriverSet::getName() const
{
	return _strName;
}

const std::vector<IDriver*>& DriverSet::getDrivers() const
{
	return _vecDrivers;
}
	
const IDriver* DriverSet::getDriver(const std::string strName) const
{
	return 0; // TODO.
}
