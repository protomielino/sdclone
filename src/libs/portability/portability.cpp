/***************************************************************************

    file                 : portability.cpp
    created              : August 2012
    copyright            : (C) 2012 Jean-Philippe Meuret
    web                  : speed-dreams.sourceforge.net

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "portability.h"


// Missing strndup, define it here (for FreeBSD).
// Code provided by Thierry Thomas.
#ifndef HAVE_STRNDUP

char *strndup(const char *str, size_t len)
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

#endif // HAVE_STRNDUP

// Missing strtok_r, define it here (for MinGW).
// Code provided by Charlie Gordon http://bytes.com/topic/c/answers/708293-strtok-strtok_r.
#ifndef HAVE_STRTOK_R

#ifdef _MSC_VER
#undef strtok_r // Avoid warning C4273: 'strtok_s' : inconsistent dll linkage (yes, 'strtok_s', not '_r' !)
#endif

char *strtok_r(char *str, const char *delim, char **nextp)
{
	char *ret;

	if (!str)
		str = *nextp;
	str += strspn(str, delim);
	if (*str == '\0')
		return 0;
	ret = str;
	str += strcspn(str, delim);
	if (*str)
		*str++ = '\0';
	*nextp = str;

	return ret;
}

#endif // HAVE_STRTOK_R


// Ticket #663 - MSVC implementation of snprintf prior to VS 2015 is not safe
// We provide our own version of the function,
// that ensures 0 ending for the string.
// and returns the number of bytes written to the str
// or if the str is not large enough, the function writes as much
// as possible and returns the number of bytes that WOULD have
// been written had str been large enough. ie: the needed buffer size

#if _MSC_VER && _MSC_VER < 1900
int SD_snprintf(char *str, size_t size, const char *format, ...)
{
	va_list vaArgs;
	va_start(vaArgs, format);
	int len = vsnprintf(str, size, format, vaArgs);
	va_end(vaArgs);
	if (size > 0)
		str[size - 1] = 0;
	return len;
}
#endif // _MSC_VER && _MSC_VER < 1900
