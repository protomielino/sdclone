/***************************************************************************

    file        : grSphere.h
    copyright   : (C) 2009 by Xavier Bertaux (based on ssgasky plib code)
    web         : http://www.speed-dreams.org

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <plib/sg.h>
#include <plib/ssg.h>

namespace ssggraph {

// return a sphere object as an ssgBranch (connected to the specified ssgSimpleState)
extern ssgBranch *grMakeSphere(ssgSimpleState *state, ssgColourArray *cl,
							   float radius, int slices, int stacks,
							   ssgCallback predraw, ssgCallback postdraw );

} // namespace ssggraph
