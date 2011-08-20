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
			glBindTexture(GL_TEXTURE_2D, ssgSimpleState::getTextureHandle());
			if (!bFlag)
				break;

			extern const int grCarTexturingModes;
			extern int grCarTexturingMode;
			if (grCarTexturingMode % grCarTexturingModes == 0)
			{
				// Legacy "multiply" = Open GL modulate texturing mode
				glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			}

			// Interpolate between skin and env.png, using env.png color as interpolation coef.
			else if (grCarTexturingMode % grCarTexturingModes == 1) // Not very good :-(
			{
				glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
				glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_INTERPOLATE);   //Interpolate RGB with RGB
				glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_PREVIOUS);
				glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_TEXTURE);
				glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_RGB, GL_TEXTURE);
				glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
				glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);
				glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_RGB, GL_SRC_COLOR);
				//------------------------
				glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_INTERPOLATE);   //Interpolate ALPHA with ALPHA (normally no use, as there's no transparency in env.png => keep PREVIOUS alpha)
				glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_PREVIOUS);
				glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA, GL_TEXTURE);
				glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_ALPHA, GL_TEXTURE);
				glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA);
				glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA, GL_SRC_ALPHA);
				glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_ALPHA, GL_SRC_ALPHA);
			}
			
			else if (grCarTexturingMode % grCarTexturingModes == 2) // Too whitish
			{
				glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
				glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_INTERPOLATE);   //Interpolate RGB with RGB
				glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_PREVIOUS);
				glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_TEXTURE);
				glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_RGB, GL_CONSTANT);
				glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
				glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);
				glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_RGB, GL_SRC_COLOR);
				//------------------------
				glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_INTERPOLATE);   //Interpolate ALPHA with ALPHA
				glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_PREVIOUS);
				glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA, GL_TEXTURE);
				glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_ALPHA, GL_CONSTANT);
				glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA);
				glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA, GL_SRC_ALPHA);
				glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_ALPHA, GL_SRC_ALPHA);
				//------------------------
				static const float aColor[4] = // The GL_CONSTANT use above.
					//{ 1.0, 1.0, 1.0, 1.0 }; // Screenshot 124548 : No change.
					//{ 0.5, 0.5, 0.5, 1.0 }; // Screenshot 140941 : Very whitish
					{ 0.75, 0.75, 0.75, 1.0 }; // Screenshot 144203 : Whitish
				glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, aColor);
			}
			else if (grCarTexturingMode % grCarTexturingModes == 3) // In test ... 
			{
				glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
				glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_INTERPOLATE);   //Interpolate RGB with RGB
				glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_TEXTURE);
				glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_PREVIOUS);
				glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_RGB, GL_TEXTURE);
				glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
				glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);
				glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_RGB, GL_SRC_COLOR);
				//------------------------
				glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_INTERPOLATE);   //Interpolate ALPHA with ALPHA
				glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_TEXTURE);
				glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA, GL_PREVIOUS);
				glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_ALPHA, GL_TEXTURE);
				glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA);
				glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA, GL_SRC_ALPHA);
				glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_ALPHA, GL_SRC_ALPHA);
			}
			
			// Other tests of alterate blending modes.
			//glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE); // Bad

			//glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_BLEND); // Bad
			//static const float aColor[4] = { 1.0, 1.0, 1.0, 1.0 };
			//glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, aColor);

			//glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL); // Bad

			//glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_ADD); // Bad
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
