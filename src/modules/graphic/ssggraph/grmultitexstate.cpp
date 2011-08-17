/***************************************************************************

    file                 : grmultitexstate.cpp
    created              : Fri Mar 22 23:16:44 CET 2002
    copyright            : (C) 2001 by Christophe Guionneau
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

#include <plib/ssg.h>

#ifdef WIN32
#include <GL/gl.h>
#include <GL/glext.h>
////// Multitexturing Info
extern PFNGLACTIVETEXTUREARBPROC glActiveTextureARB ;
#endif

#include "grmultitexstate.h"

#ifdef DMALLOC
#include "dmalloc.h"
#endif

// Apply the state to the give texture unit GL_TEXTURE<unit>_ARB
// Temporary bFlag argument for testing (see calls in grvtxtable.cpp).
void grMultiTexState::apply(int unit, bool bFlag)
{
	switch(unit)
	{
		// Tracks & cars : "base" texture from .ac/.acc
		case 0:
			glActiveTextureARB(GL_TEXTURE0_ARB);
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, ssgSimpleState::getTextureHandle());
			break;

		// Tracks : "tiled" texture from .ac/.acc
		// Cars : horizontal reflexion = projection of track objects = grEnvState
		case 1:
			glActiveTextureARB(GL_TEXTURE1_ARB);
			glEnable(GL_TEXTURE_2D);
			// glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
			// glBlendFunc(GL_ZERO, GL_SRC_COLOR);
			glBindTexture(GL_TEXTURE_2D, ssgSimpleState::getTextureHandle());
			if (bFlag)
			{
				//glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE); // OK, = default.
				
				//glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE); // Bad

				//glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_BLEND); // Bad
				//static const float aColor[4] = { 1.0, 1.0, 1.0, 1.0 };
				//glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, aColor);

				//glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL); // Bad

				//glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_ADD); // Bad

				
				//glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE); // Needs more params
			}
			break;

		// Tracks : "skids" texture from .ac/.acc
		// Cars : vertical reflexion = projection of the clouds = grEnvStateShadowState
		case 2:
			glActiveTextureARB(GL_TEXTURE2_ARB);
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, ssgSimpleState::getTextureHandle());
			if (bFlag)
			{
			}
			break;

		// Tracks : "shad" texture from .ac/.acc
		// Cars : track objects shadows = vertical projection = grEnvShadowStateOnCars
		case 3:
			glActiveTextureARB(GL_TEXTURE3_ARB);
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, ssgSimpleState::getTextureHandle());
			break;

		// Should never be used
		default:
			GfLogWarning("grMultiTexState@%p::apply(unit=%d) : "
						 "No support for this texture unit ; redirecting to current\n");
			// glActiveTextureARB(GL_TEXTURE0_ARB);
			glBindTexture(GL_TEXTURE_2D, getTextureHandle());
			_ssgCurrentContext->getState()->setTexture(getTexture());  
	}
}
