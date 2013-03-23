/***************************************************************************
                 idriverset.h -- Interface for driver sets

    created              : Thu Mar 14 19:48:14 CEST 2013
    copyright            : (C) 2013 by Jean-Philippe Meuret                         
    web                  : http://www.speed-dreams.org
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
    	Interface for driver sets (a set of drivers)
    @version	$Id$
*/

#ifndef __IDRIVERSET__H__
#define __IDRIVERSET__H__

#include <string>
#include <vector>

#include "idriver.h"


class IDriverSet
{
public:

	//! Get the set driver name.
	virtual const std::string& getName() const = 0;
	
	//! Get all the drivers.
	virtual const std::vector<IDriver*>& getDrivers() const = 0;
	
	//! Get the driver of given name..
	virtual const IDriver* getDriver(const std::string strName) const = 0;
	
};

#endif // __IDRIVERSET__H__
