/***************************************************************************

    file        : ssggraph.h
    copyright   : (C) 2011 by Jean-Philippe Meuret                        
    email       : pouillot@users.sourceforge.net   
    version     : $Id: ssggraph.h 3741 2011-07-21 22:29:34Z pouillot $

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
    		The "ssggraph" graphics engine module
    @version    $Id: ssggraph.h 3741 2011-07-21 22:29:34Z pouillot $
*/

#ifndef _SNDDEFAULT_H_
#define _SNDDEFAULT_H_

#include <igraphicsengine.h>
#include <isoundengine.h>

#include <tgf.hpp>

class ssgLoaderOptions;


// DLL exported symbols declarator for Windows.
#ifdef WIN32
# ifdef SNDDEFAULTDLL
#  define SNDDEFAULT_API __declspec(dllexport)
# else
#  define SNDDEFAULT_API __declspec(dllimport)
# endif
#else
# define SNDDEFAULT_API
#endif


// The C interface of the module.
extern "C" int SNDDEFAULT_API openGfModule(const char* pszShLibName, void* hShLibHandle);
extern "C" int SNDDEFAULT_API closeGfModule();

// The module main class
// (Singleton, inherits GfModule, and implements IGraphicsEngine and ISoundEngine).
class SNDDEFAULT_API sndDefault : public GfModule, public ISoundEngine
{
 public:

	virtual void initSound(Situation* s);
        virtual void shutdownSound(Situation* s);
        virtual void refreshSound(Situation *s, SoundCam*camera);
        virtual void mute(bool bOn = true);

	// Accessor to the singleton.
	static sndDefault& self();

	// Destructor.
	virtual ~sndDefault();

 protected:

	// Protected constructor to avoid instanciation outside (but friends).
	sndDefault(const std::string& strShLibName, void* hShLibHandle);
	
	// Make the C interface functions nearly member functions.
	friend int openGfModule(const char* pszShLibName, void* hShLibHandle);
	friend int closeGfModule();

 protected:

	// The singleton.
	static sndDefault* _pSelf;

	// The default SSGLoaderOptions instance.
	//ssgLoaderOptions* _pDefaultSSGLoaderOptions;
};

#endif /* _SNDDEFAULT_H_ */ 
