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

#ifndef _DRIVER_H_
#define _DRIVER_H_

#include <idriver.h>

class IDriverSet;


// The driver class (implements IDriver).

class Driver : public IDriver
{
public: // Implementation of IDriver.
	
	//! Give the driver the track view. Called for every track change or new race
	virtual void setTrack(tTrack *track,
				 		  void *carHandle, void **myCarSettings,
				 		  tSituation *s);
	
	//! Start a new race
	virtual void startRace(tCarElt *car, tSituation *s);
	
	//! Resume current race (from ESC menu)
	virtual void resumeRace(tCarElt *car, tSituation *s);
	
	//! Drive during race
	virtual void drive(tCarElt *car, tSituation *s);
	
	//! End of the current race
	virtual void finishRace(tCarElt *car, tSituation *s);
	
	//! Get the driver's pit commands
	virtual int getPitCommand(tCarElt* car, tSituation *s) const;

	//! Get the driver name (ex: "Aarne Fisher")
	virtual const std::string& getName() const;
	
	//! Get the driver description (ex: "A well know Indian top fighter")
	virtual const std::string& getDescription() const;
	
	//! Get the name of the set the driver belongs to (ex: "usr_36GP")
	virtual const std::string& getSetName() const;

public:
	
	//! Constructor.
	Driver(IDriverSet* pParentSet, void* hparmSet);
	
	//! Destructor.
	virtual ~Driver();
	
protected:

	//! The set to which it belongs.
	IDriverSet* _pDriverSet;

	//! The driver name.
	std::string _strName;
	
	//! The driver short name.
	std::string _strShortName;
	
	//! The driver code name.
	std::string _strCode;
	
	//! The driver description.
	std::string _strDesc;
	
	
};

#endif /* _DRIVER_H_ */ 
