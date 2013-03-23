/***************************************************************************
                 irobot.h -- Interface for robots (driver types)

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
    	Interface for robots = driver types (sets of driver sets)
    @version	$Id$
*/

#ifndef __IROBOT__H__
#define __IROBOT__H__


#include "robot.h"
#include "idriverset.h"


class IRobot
{
public:

	//! Get the the supported driver sets (ex: { "usr_sc", "usr_trb1" }).
	virtual const std::vector<IDriverSet*>& getDriverSets() = 0;
	
};

#endif // __IROBOT__H__
