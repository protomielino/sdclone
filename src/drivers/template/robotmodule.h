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

#ifndef _ROBOTMODULE_H_
#define _ROBOTMODULE_H_

#include <irobot.h>

#include <tgf.hpp>


// DLL exported symbols declarator for Windows.
#ifdef WIN32
# ifdef ROBOT_DLL
#  define ROBOT_API __declspec(dllexport)
# else
#  define ROBOT_API __declspec(dllimport)
# endif
#else
# define ROBOT_API
#endif

// The C interface of the module.
extern "C" int ROBOT_API openGfModule(const char* pszShLibName, void* hShLibHandle);
extern "C" int ROBOT_API closeGfModule();

// The robot module main class
// (Singleton, inherits GfModule, and implements IRobot).

class ROBOT_API Robot : public GfModule, public IRobot
{
public:
	
	//! Implementation of IRobot.
	virtual const std::vector<IDriverSet*>& getDriverSets();

	// Accessor to the singleton.
	//static Robot& self();

	//! The robot dedicated logger.
	static GfLogger& logger;

	//! Destructor.
	virtual ~Robot();
	
protected:

	//! Protected constructor to avoid instanciation outside (but friends).
	Robot(const std::string& strShLibName, void* hShLibHandle);
	
	//! Make the C interface functions nearly member functions.
	friend int openGfModule(const char* pszShLibName, void* hShLibHandle);
	friend int closeGfModule();
	
protected:

	//! The robot singleton.
	static Robot* _pSelf;

	//! The robot logger.
	GfLogger* _pLogger;

	//! The robot name.
	std::string _strName;
	
	//! The robot driver sets.
	std::vector<IDriverSet*> _vecDriverSets;
};

#endif /* _ROBOTMODULE_H_ */ 
