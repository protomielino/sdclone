/***************************************************************************
                 idriver.h -- Interface for drivers

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
    	Interface for drivers
    @version	$Id$
*/

#ifndef __IDRIVER__H__
#define __IDRIVER__H__


#include <raceman.h>
#include <string>
#include <vector>

class IDriver
{
public:

	//! Give the driver the track view. Called for every track change or new race
	virtual void setTrack(tTrack *track,
				 		  void *carHandle, void **myCarSettings,
				 		  tSituation *s) = 0;
	
	//! Start a new race
	virtual void startRace(tCarElt *car, tSituation *s) = 0;
	
	//! Resume current race (from ESC menu)
	virtual void resumeRace(tCarElt *car, tSituation *s) = 0;
	
	//! Drive during race
	virtual void drive(tCarElt *car, tSituation *s) = 0;
	
	//! End of the current race
	virtual void finishRace(tCarElt *car, tSituation *s) = 0;
	
	//! Get the driver's pit commands
	virtual int getPitCommand(tCarElt* car, tSituation *s) const = 0;

	//! Get the driver name (ex: "Aarne Fisher")
	virtual const std::string& getName() const = 0;
	
	//! Get the driver description (ex: "A well know Indian top fighter")
	virtual const std::string& getDescription() const = 0;
	
	//! Get the name of the set the driver belongs to (ex: "usr_36GP")
	virtual const std::string& getSetName() const = 0;
};

#endif // __IDRIVER__H__
