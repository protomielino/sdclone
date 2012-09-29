/***************************************************************************

    file                 : ssggraph.cpp
    created              : Thu Aug 17 23:19:19 CEST 2000
    copyright            : (C) 2000 by Eric Espie
    email                : torcs@free.fr
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

#include "snddefault.h"
#include "grsound.h"

// The SsgGraph: singleton.
sndDefault* sndDefault::_pSelf = 0;

int openGfModule(const char* pszShLibName, void* hShLibHandle)
{
	// Instanciate the (only) module instance.
	sndDefault::_pSelf = new sndDefault(pszShLibName, hShLibHandle);

	// Register it to the GfModule module manager if OK.
	if (sndDefault::_pSelf)
		GfModule::register_(sndDefault::_pSelf);

	// Report about success or error.
	return sndDefault::_pSelf ? 0 : 1;
}

int closeGfModule()
{
	// Unregister it from the GfModule module manager.
	if (sndDefault::_pSelf)
		GfModule::unregister(sndDefault::_pSelf);
	
	// Delete the (only) module instance.
	delete sndDefault::_pSelf;
	sndDefault::_pSelf = 0;

	// Report about success or error.
	return 0;
}

sndDefault& sndDefault::self()
{
	// Pre-condition : 1 successfull openGfModule call.
	return *_pSelf;
}

sndDefault::sndDefault(const std::string& strShLibName, void* hShLibHandle)
: GfModule(strShLibName, hShLibHandle)
{
}

sndDefault::~sndDefault()
{
	// Terminate the PLib SSG layer.
	//delete _pDefaultSSGLoaderOptions;
}

// Implementation of ISoundEngine ****************************************

void sndDefault::initSound(Situation* s){
    grInitSound(s,s->_ncars);
}
void sndDefault::shutdownSound(){
    grShutdownSound();
}
void sndDefault::refreshSound(Situation *s, SoundCam	*camera){
    grRefreshSound(s, camera);
}

void sndDefault::mute(bool bOn)
{
	::grMuteSound(bOn);
}
