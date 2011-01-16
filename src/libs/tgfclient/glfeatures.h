/***************************************************************************

    file                 : glfeatures.h
    created              : Wed Jun 1 14:56:31 CET 2005
    copyright            : (C) 2005 by Bernhard Wymann
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

#ifndef _GLFEATURES_H_
#define _GLFEATURES_H_

#ifdef WIN32
#include <windows.h>
#include <GL/gl.h>
#include <GL/glext.h>
#endif //WIN32

#ifdef __APPLE__
#include <OpenGL/gl.h>
#endif //__APPLE__

#ifdef LINUX
#include <GL/gl.h>
#endif 

#include <graphic.h>

#include "tgfclient.h"

#endif // _GLFEATURES_H_

