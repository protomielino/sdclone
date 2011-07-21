/***************************************************************************
                 isoundengine.h -- Interface for sound engines

    created              : Mon Mar 28 19:48:14 CEST 2011
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
    	Interface for sound engines
		Only embryonic, as long as we don't have a real separate sound engine
		(for the moment, it is inside the graphics engine)
    @version	$Id$
*/

#ifndef __ISOUNDENGINE__H__
#define __ISOUNDENGINE__H__

class ISoundEngine
{
public:

	virtual void mute(bool bOn = true) = 0;
};

#endif // __ISOUNDENGINE__H__
