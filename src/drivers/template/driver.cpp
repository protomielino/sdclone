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
    	The driver class of the template robot module	
*/

#include "robotmodule.h"

#include "driver.h"


//! Constructor
Driver::Driver(IDriverSet* pParentSet, void* hparmSet)
: _pDriverSet(pParentSet)
{
	const char* pszDrvListPath = ROB_SECT_ROBOTS"/"ROB_LIST_INDEX;
	
	// Driver "index" in the file (no actual use).
	std::string strInd =
		GfParmListGetCurEltName(hparmSet, pszDrvListPath);

	// Driver name.
	_strName =
		GfParmGetCurStr(hparmSet, pszDrvListPath, ROB_ATTR_NAME, "no name");
	
	// Driver short name.
	_strShortName =
		GfParmGetCurStr(hparmSet, pszDrvListPath, ROB_ATTR_SNAME, "no short name");
	
	// Driver code name.
	_strCode =
		GfParmGetCurStr(hparmSet, pszDrvListPath, ROB_ATTR_CODE, "no code");
	
	// Driver description.
	_strDesc =
		GfParmGetCurStr(hparmSet, pszDrvListPath, ROB_ATTR_DESC, "no description");

	Robot::logger.info("* Ind=%s : Name=%s (SName=%s, Code=%s, Desc=%s)\n",
					   strInd.c_str(), _strName.c_str(), _strShortName.c_str(),
					   _strCode.c_str(), _strDesc.c_str());

	// TODO: Load any remaining data (or done elsewhere) ?
}

//! Destructor (Called before the dll is unloaded)
Driver::~Driver()
{
}

// Implementation of IDriver ===================================================

//! Give the driver the track view. Called for every track change or new race
void Driver::setTrack(tTrack *track,
					  void *carHandle, void **myCarSettings,
					  tSituation *s)
{
}
	
//! Start a new race
void Driver::startRace(tCarElt *car, tSituation *s)
{
}

//! Resume current race (from ESC menu)
void Driver::resumeRace(tCarElt *car, tSituation *s)
{
}

//! Drive during race
void Driver::drive(tCarElt *car, tSituation *s)
{
}

//! End of the current race
void Driver::finishRace(tCarElt *car, tSituation *s)
{
}
	
//! Get the driver's pit commands
int Driver::getPitCommand(tCarElt* car, tSituation *s) const
{
	return -1; // TODO.
}

//! Get the name of the set the driver belongs to (ex: "usr_36GP")
const std::string& Driver::getSetName() const
{
	return _pDriverSet->getName();
}

//! Get the driver name (ex: "Aarne Fisher")
const std::string& Driver::getName() const
{
	return _strName;
}
	
//! Get the driver description (ex: "A well know Indian top fighter")
const std::string& Driver::getDescription() const
{
	return _strDesc;
}
	
