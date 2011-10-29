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

// Question: What for ?
#ifdef DMALLOC
#include "dmalloc.h"
#endif


cgrMultiTexState::cgrMultiTexState(tfnTexScheme fnTexScheme)
: _fnTexScheme(fnTexScheme)
{
}
	
void cgrMultiTexState::setTexScheme(tfnTexScheme fnTexScheme)
{
	_fnTexScheme = fnTexScheme;
}

// Apply the texture state to the given texture unit GL_TEXTURE<nUnit>_ARB
void cgrMultiTexState::apply(GLint nUnit)
{
	glActiveTextureARB(nUnit);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, ssgSimpleState::getTextureHandle());
	if (_fnTexScheme)
		_fnTexScheme();
}

// Standard "multiply" scheme.
void cgrMultiTexState::modulate()
{
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
}

// Name is self-explanatory.
// =>Possible saturation
void cgrMultiTexState::addColorModulateAlpha()
{
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
	
	// Add Color.
	glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_ADD);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_PREVIOUS);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_TEXTURE);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);

	// Modulate Alpha.
	glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_MODULATE); 
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_PREVIOUS);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA, GL_TEXTURE);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA, GL_SRC_ALPHA);
}

// Interpolate between PREV and TEX, using TEX color as interpolation coef.
// => Not very good :-(
void cgrMultiTexState::interpolate()
{
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);

	//Interpolate RGB with RGB
	glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_INTERPOLATE);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_PREVIOUS);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_TEXTURE);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_RGB, GL_TEXTURE);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_RGB, GL_SRC_COLOR);

	//Interpolate ALPHA with ALPHA
	// (no use i there's no transparency in texture => keep PREVIOUS alpha)
	glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_INTERPOLATE);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_PREVIOUS);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA, GL_TEXTURE);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_ALPHA, GL_TEXTURE);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA, GL_SRC_ALPHA);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_ALPHA, GL_SRC_ALPHA);
}

// Interpolate between PREV and TEX, using constant color as interpolation coef.
// => Too whitish
void cgrMultiTexState::interpolateConst()
{
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);

	//Interpolate RGB / RGB
	glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_INTERPOLATE);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_PREVIOUS);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_TEXTURE);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_RGB, GL_CONSTANT);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_RGB, GL_SRC_COLOR);

	//Interpolate ALPHA / ALPHA
	glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_INTERPOLATE);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_PREVIOUS);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA, GL_TEXTURE);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_ALPHA, GL_CONSTANT);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA, GL_SRC_ALPHA);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_ALPHA, GL_SRC_ALPHA);

	// The GL_CONSTANT used above.
	static const float aColor[4] =
	//{ 1.0, 1.0, 1.0, 1.0 }; // No change.
	//{ 0.5, 0.5, 0.5, 1.0 }; // Very whitish
		{ 0.75, 0.75, 0.75, 1.0 }; // Whitish
	glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, aColor);
}

// Interpolate between TEX and PREV, using constant color as interpolation coef.
// => Full metal jacket !
void cgrMultiTexState::interpolateReverted()
{
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
			
	glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_INTERPOLATE); //Interpolate RGB / RGB
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_TEXTURE);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_PREVIOUS);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_RGB, GL_TEXTURE);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_RGB, GL_SRC_COLOR);

	glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_INTERPOLATE); //Interpolate ALPHA / ALPHA
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_TEXTURE);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA, GL_PREVIOUS);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_ALPHA, GL_TEXTURE);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA, GL_SRC_ALPHA);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_ALPHA, GL_SRC_ALPHA);
}

// 2 x PREV, no TEX
// => Too bright, and not what we want, 'cause inhibits TEX
void cgrMultiTexState::duplicate()
{
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);

	// Add PREV to PREV
	glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_ADD);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_PREVIOUS);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_PREVIOUS);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_RGB, GL_PREVIOUS); // no use
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_RGB, GL_SRC_COLOR); // no use

	//Interpolate ALPHA / ALPHA
	// (no use when TEX = env.png, as there's no transparency in it => keeps PREVIOUS alpha)
	glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_INTERPOLATE);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_PREVIOUS);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA, GL_TEXTURE);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_ALPHA, GL_TEXTURE); // no use
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA, GL_SRC_ALPHA);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_ALPHA, GL_SRC_ALPHA); // no use
}

// Name is self-explanatory.
// => Bad
void cgrMultiTexState::replace()
{
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
}

// Name is self-explanatory.
// => Bad
void cgrMultiTexState::decal()
{
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
}

// Name is self-explanatory.
// => Bad
void cgrMultiTexState::add()
{
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_ADD);
}
			
// Name is self-explanatory.
// => Bad
void cgrMultiTexState::blend()
{
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_BLEND);
	static const float aColor[4] = { 1.0, 1.0, 1.0, 1.0 };
	glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, aColor);
}
			
// Archives (TODO: remove) =========================================================
#if 0

// Tool for debugging the car texturing mode (see grmultitexstate.cpp)
const int grCarTexturingModes = 6;
int grCarTexturingTrackEnvMode = 0;
int grCarTexturingSkyShadowsMode = 5;
int grCarTexturingTrackShadowsMode = 0;

static void applyAltTexEnv(int nModeNum)
{
	switch (nModeNum)
	{
		case 0:
			cgrMultiTexState::schemeModulate();
			break;

		case 1:
			cgrMultiTexState::interpolate();
			break;
			
		case 2:
			cgrMultiTexState::interpolateConst();
			break;
		
		case 3:
			cgrMultiTexState::interpolateReverted();
			break;
			
		case 4:
			cgrMultiTexState::duplicate();
			break;
			
		case 5:
			
			cgrMultiTexState::schemeAddColorModulateAlpha();
			break;
			
		default:
			
			GfLogError("Unsupported car multi-texturing mode %d\n", nModeNum);
			
			break;
	}
}


// Apply the state to the given texture unit GL_TEXTURE<unit>_ARB
// (use alternate texture env. mode if bAltEnv)
void cgrMultiTexState::apply(int unit, bool bAltEnv)
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
			if (bAltEnv && grCarTexturingTrackEnvMode)
				applyAltTexEnv(grCarTexturingTrackEnvMode);
			break;

		// Tracks : "skids" texture from .ac/.acc
		// Cars : vertical reflexion = projection of the clouds = grEnvShadowState
		case 2:
			glActiveTextureARB(GL_TEXTURE2_ARB);
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, ssgSimpleState::getTextureHandle());
			if (bAltEnv && grCarTexturingSkyShadowsMode)
				applyAltTexEnv(grCarTexturingSkyShadowsMode);
			break;

		// Tracks : "shad" texture from .ac/.acc
		// Cars : track objects shadows = vertical projection = grEnvShadowStateOnCars
		case 3:
			glActiveTextureARB(GL_TEXTURE3_ARB);
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, ssgSimpleState::getTextureHandle());
			if (bAltEnv && grCarTexturingTrackShadowsMode)
				applyAltTexEnv(grCarTexturingTrackShadowsMode);
			break;

		// Should never be used
		default:
			GfLogWarning("cgrMultiTexState@%p::apply(unit=%d) : "
						 "No support for this texture unit ; redirecting to current\n");
			// glActiveTextureARB(GL_TEXTURE0_ARB);
			glBindTexture(GL_TEXTURE_2D, getTextureHandle());
			_ssgCurrentContext->getState()->setTexture(getTexture());  
	}
}
#endif

