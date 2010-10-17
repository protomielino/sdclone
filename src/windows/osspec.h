/***************************************************************************

    file                 : osspec.h
    created              : Sat Mar 18 23:54:47 CET 2000
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

 
#ifndef __OSSPEC__H__
#define __OSSPEC__H__

/* Files Extensions */
#define TRKEXT	"xml"
#define DLLEXT	"dll"
#define PARAMEXT ".xml"

#ifndef _WIN32
#error Hey ! Where is _WIN32 ??
#endif

#include <cstring>
#ifdef WIN32
#	define uint unsigned int
#	define uchar unsigned char
#	define execlp _execlp
#	define strncasecmp strnicmp
#	define strcasecmp stricmp
#endif // WIN32

#define M_PI (3.1415926535897932384626433832795)

#endif /* __OSSPEC__H__ */ 
