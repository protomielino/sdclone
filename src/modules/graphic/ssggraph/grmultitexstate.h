/***************************************************************************

    file                 : grmultitexstate.h
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

#ifndef __GRMULTITEXSTATE
#define __GRMULTITEXSTATE

#include "grsimplestate.h"	//cgrSimpleState


class cgrMultiTexState : public cgrSimpleState
{
 public:

	//! Texturing scheme function.
	typedef void (*tfnTexScheme)(void);

	//! Constructor (use only below static texturing scheme functions for fnTexScheme).
	cgrMultiTexState(tfnTexScheme fnTexScheme = 0);
	
	//! Set texturing scheme (use only below static functions).
	void setTexScheme(tfnTexScheme fnTexScheme);
	
	// Apply the texture state to the given texture unit GL_TEXTURE<nUnit>_ARB
	virtual void apply(GLint nUnit);

	//! Useful texturing schemes.
	static void modulate(); // Multiply color and alpha.
	static void addColorModulateAlpha(); // Self explanatory.
	
	//! Other texturing schemes (tries).
	static void interpolate(); // Interpolate using previous as coefficient.
	static void interpolateConst(); // Interpolate using const RGBA as coefficient.
	static void interpolateReverted(); // Same as interpolate, but prev/tex reverted
	static void duplicate(); // Prev x 2 (ignore tex)
	static void replace(); // Self explanatory.
	static void decal(); // Self explanatory.
	static void add(); // Self explanatory.
	static void blend(); // Self explanatory.

 protected:

	//! Texturing scheme (among above static funtions).
	tfnTexScheme _fnTexScheme;
};

// Car multi-texturing modes, for testing/debugging purpose (see grmain.cpp::initView()).
// extern const int grCarTexturingModes;
// extern int grCarTexturingTrackEnvMode; // Texturing unit 1.
// extern int grCarTexturingSkyShadowsMode; // Texturing unit 2.
// extern int grCarTexturingTrackShadowsMode; // Texturing unit 3.

#endif // __GRMULTITEXSTATE
