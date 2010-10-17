/***************************************************************************

    file        : portability.h
    created     : Fri Jul 8 15:19:34 CET 2005
    copyright   : (C) 2005 Bernhard Wymann
    email       : berniw@bluewin.ch
    version     : $Id$

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _SD_PORTABILITY_H_
#define _SD_PORTABILITY_H_

#include <cstdlib>
#include <cstring>
#ifdef WIN32
#include <direct.h>
#include <process.h>
#endif

#ifdef WIN32
#ifndef HAVE_CONFIG_H
#define HAVE_CONFIG_H
#endif
#endif

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// Missing strndup, define it here (for FreeBSD).
// TODO: Move it into library.
// strndup code provided by Thierry Thomas.
#ifndef HAVE_STRNDUP

static char *strndup(const char *str, int len)
{
	if (!str || len < 0)
		return 0;

	char* ret;
	if (!(ret = (char*)malloc(len + 1)))
		return 0;

	memcpy(ret, str, len);
	ret[len] = '\0';

	return ret;
}

#endif

#ifdef __APPLE__
#define isnan(x) ((x) != (x))
#endif

// Posix functions special names with MS compilers.
#if defined(WIN32)

#define isnan _isnan

#define snprintf _snprintf

//MSVC 2008 already has this
#if _MSC_VER <= 1400
#define vsnprintf _vsnprintf
#define isnan _isnan
#endif

// For MSVC 2005 and newer
#if _MSC_VER >= 1400
#ifdef strdup
#undef strdup
#endif
#define strdup _strdup
#define stricmp _stricmp
#define strnicmp _strnicmp
#define chdir _chdir
#define getcwd _getcwd
#ifdef mkdir
#undef mkdir
#endif
#define mkdir(x) _mkdir(x)
#define execvp _execvp
#endif

#endif

#endif // _SD_PORTABILITY_H_

