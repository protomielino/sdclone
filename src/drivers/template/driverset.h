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

#ifndef _DRIVERSET_H_
#define _DRIVERSET_H_

#include <idriverset.h>


// The driver set class (implements IDriverSet).

class DriverSet : public IDriverSet
{
public:
	
	//! Constructor.
	DriverSet(const std::string& strName, void* hparmSet);
	
	//! Implementation of IDriverset.
	virtual const std::string& getName() const;
	virtual const std::vector<IDriver*>& getDrivers() const;
	virtual const IDriver* getDriver(const std::string strName) const;

	//! Destructor.
	virtual ~DriverSet();
	
protected:

	//! The set name.
	std::string _strName;
	
	//! The drivers of the set.
	std::vector<IDriver*> _vecDrivers;
	
};

#endif /* _DRIVERSET_H_ */ 
