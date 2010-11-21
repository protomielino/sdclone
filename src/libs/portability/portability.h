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
#ifdef _MSC_VER
#include <direct.h>
#include <process.h>
#define _WINSOCKAPI_   /* Prevent inclusion of winsock.h in windows.h */
#endif

#ifdef _MSC_VER
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

// Apple platform ---------------------------------------------------
#ifdef __APPLE__

#define isnan(x) ((x) != (x))

#endif

// Windows platform -------------------------------------------------
// Posix functions special names with MS compilers.
// Notes on MSVC compilers detection :
// * _MSC_VER define should be prefered to WIN32/_WIN32
// * MSVC    6 : 1200 <= _MSC_VER < 1300
// * MSVC 2003 : 1300 <= _MSC_VER < 1400
// * MSVC 2005 : 1400 <= _MSC_VER < 1500
// * MSVC 2008 : 1500 <= _MSC_VER
#ifdef _MSC_VER

#define isnan _isnan

#define snprintf _snprintf

// For MSVC 2005 and older (2008 already defines these)
#if _MSC_VER < 1500

#define vsnprintf _vsnprintf

#endif // _MSC_VER < 1500

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
#define chmod _chmod

#ifdef mkdir
#undef mkdir
#endif
#define mkdir(x) _mkdir(x)

#define execvp _execvp

#endif // _MSC_VER >= 1400

#endif // _MSC_VER

#endif // _SD_PORTABILITY_H_

