/***************************************************************************
                 isoundengine.h -- Interface for sound engines

    created              : Mon Jul 28 19:48:14 CEST 2012
    copyright            : (C) 2012 by Gaëtan André
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

 #ifndef __ISOUNDENGINE__H__
 #define __ISOUNDENGINE__H__


typedef float sndVec3[3];

struct SoundCam
{
    sndVec3 * Posv;
    sndVec3 * Speedv;
    sndVec3 * Centerv;
    sndVec3 * Upv;
};

class ISoundEngine 
{
public:
 
    virtual void init(struct Situation* s) = 0;
    virtual void shutdown() = 0;
    virtual void refresh(struct Situation *s, SoundCam *camera) = 0;
	virtual void mute(bool bOn = true) = 0;
};
 
 #endif // __ISOUNDENGINE__H__
