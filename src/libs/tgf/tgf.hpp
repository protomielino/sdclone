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
// New dynamicaly loadable modules system
// * 1 module per shared library
// * the shared library exports 2 extern "C" functions :
//   - bool initializeModule(); // TODO: find a different name (legacy welcome modules)
//   - bool terminateModule(); // Idem

class TGF_API GfModule
{
 public: // Services for the code that is client of the module.

	//! Load a module from the given module library file.
	static GfModule* load(const std::string& strShLibName);

	//! Unload a module and the associated library (supposed to contain no other module).
	virtual bool unload();

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
		
 protected:

	//! The table of loaded modules and their associated shared library (key = file name).
	static std::map<std::string, GfModule*> _mapModulesByLibName;

	//! The associated shared library file path-name.
	std::string _strShLibName;

	//! The OS-level handle of the associated shared library.
	void* _hShLibHandle;
};

#endif // __TGF__HPP__


