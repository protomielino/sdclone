/***************************************************************************
                    tgf.hpp -- Interface file for The Gaming Framework
                             -------------------                                         
    created              : Mod Mar 14 20:32:14 CEST 2011
    copyright            : (C) 2011 by Jean-Philippe Meuret
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
    	The Gaming Framework API, with C++-specific extensions.
    @version	$Id$
*/

#ifndef __TGF__HPP__
#define __TGF__HPP__

#ifdef _MSC_VER
// Disable useless MSVC warnings
#  pragma warning (disable:4251) // class XXX needs a DLL interface ...
#endif

#include <string>
#include <map>

#include "tgf.h"


//****************************************
// New dynamically loadable modules system
// - 1 and only 1 module per shared library,
// - a module is a GfModule-derived class instance (and even a singleton),
// - the shared library exports 2 extern 'C' functions :
//   - int openGfModule(const char* pszShLibName, void* hShLibHandle) :
//     it must instanciate the module class (new), register the module instance (register_),
//     initialize / allocate any needed internal resource and finally return 0
//     if everything is OK, non-0 otherwise;
//   - int closeGfModule() :
//     it must release any allocated resource, unregister the module instance (unregister),
//     and finally return 0, non-0 otherwise.

class TGF_API GfModule
{
 public: // Services for the code that is client of the module.

	//! Load a module from the given module library file.
	static GfModule* load(const std::string& strShLibName);

	//! Delete a module and unload the associated library (supposed to contain no other module).
	static bool unload(GfModule*& pModule);

	//! Get the module as a pointer to the given interface (aka "facet").
	template <class Interface>
	Interface* getInterface()
	{
		return dynamic_cast<Interface*>(this);
	}
	
 public: // Services for the module implementation.

	//! Register a new module instance (aimed at being called by the GfModuleOpen function).
	static bool register_(GfModule* pModule);
	
	//! Constructor.
	GfModule(const std::string& strShLibName, void* hShLibHandle);

	//! Destructor.
	virtual ~GfModule();

	//! Get the asssociated shared library path-name.
	const std::string& getSharedLibName() const;
		
	//! Get the asssociated shared library handle.
	void* getSharedLibHandle() const;
		
 protected:

	//! Unregister a module instance.
	static bool unregister(GfModule* pModule);
	
 protected:

	//! The table of loaded modules and their associated shared library (key = file name).
	static std::map<std::string, GfModule*> _mapModulesByLibName;

	//! The associated shared library file path-name.
	std::string _strShLibName;

	//! The OS-level handle of the associated shared library.
	void* _hShLibHandle;
};

#endif // __TGF__HPP__


