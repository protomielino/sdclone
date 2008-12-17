/***************************************************************************

    file        : portability.h
    created     : Fri Jul 8 15:19:34 CET 2005
    copyright   : (C) 2005 Bernhard Wymann
    email       : berniw@bluewin.ch
    version     : $Id: portability.h,v 1.2 2005/08/05 09:05:59 berniw Exp $

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _TORCS_PORTABILITY_H_
#define _TORCS_PORTABILITY_H_

#include <stdlib.h>
#include <cstring>

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
	char *ret;

	if ((str == NULL || len < 0)) {
		return (NULL);
	}

	ret = (char *) malloc(len + 1);
	if (ret == NULL) {
		return (NULL);
	}

	memcpy(ret, str, len);
	ret[len] = '\0';
	return (ret);
}

#endif

// Posix functions special names with MS compilers.
#if defined(WIN32)

#define snprintf _snprintf
#define vsnprintf _vsnprintf

// For MSVC 2005 and newer
#if _MSC_VER >= 1400
#ifdef strdup
#undef strdup
#endif
#define strdup _strdup
#define stricmp _stricmp
#define strnicmp _strnicmp
#define chdir _chdir
#endif

#endif

#endif // _TORCS_PORTABILITY_H_

