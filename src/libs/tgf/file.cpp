/***************************************************************************
                        file.cpp -- directory management                       
                             -------------------                                         
    created              : Thu Oct 12 21:58:55 CEST 2010
    copyright            : (C) 2010 by Mart Kelder, Jean-Philippe Meuret
    web                  : http://www.speed-reams.org
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

/** @file
    		This is used for file manipulations.
    @author	Mart Kelder, Jean-Philippe Meuret
    @version	$Id$
    @ingroup	file
*/

#include <cstdio>
#include <cerrno>
#include <sys/stat.h>

#ifdef WIN32
#include <io.h>
#endif

#include <portability.h>

#include "tgf.h"


bool GfFileExists(const char* pszName)
{
	struct stat st;
	return stat(pszName, &st) ? false : true;
}

bool GfFileCopy(const char* pszSrcName, const char* pszTgtName)
{
	static const size_t maxBufSize = 1024;
	char buf[maxBufSize];
	FILE *in;
	FILE *out;
	size_t size;
	int errnum;
	bool res = true;
	
	// Create the target local directory (and parents) if not already done
	// (first, we have to deduce its path from the target file path-name).
	strncpy(buf, pszTgtName, strlen(pszTgtName)+1);
#ifdef WIN32
	for (int i = 0; i < maxBufSize && buf[i] != '\0'; i++)
		if (buf[i] == '\\')
			buf[i] = '/';
#endif
	char *lastSlash = strrchr(buf, '/');
	if (lastSlash)
	{
	  *lastSlash = '\0';
	  GfDirCreate( buf );
	}

	// Set target file access attributes to "read/write for the current user",
	// if the target file exists (quite paranoid, but sometimes needed).
	struct stat st;
	if (! stat(pszTgtName, &st) && chmod( pszTgtName, 0640 ))
	{
		const int errnum = errno; // Get errno before it is overwritten by some system call.
		GfLogWarning("Failed to set 0640 attributes to %s (%s)\n",
					 pszTgtName, strerror(errnum));
	}

	// Open the source and the target file.
	if( ( in = fopen( pszSrcName, "rb" ) ) == NULL )
	{
		errnum = errno; // Get errno before it is overwritten by some system call.
		GfLogError("Could not open %s in 'rb' mode when copying it to %s (%s).\n",
				   pszSrcName, pszTgtName, strerror(errnum));
		return false;
	}
	if( ( out = fopen( pszTgtName, "wb" ) ) == NULL )
	{
		errnum = errno; // Get errno before it is overwritten by some system call.
		GfLogError("Could not open %s in 'wb' mode when creating it from %s (%s).\n",
				   pszTgtName, pszSrcName, strerror(errnum));
		fclose( in );
		return false;
	}

	// Do the byte to byte copy.
	GfLogDebug("Copying %s to %s\n", pszSrcName, pszTgtName);

	while( !feof( in ) )
	{
		size = fread( buf, 1, 1024, in );
		if( size > 0 )
		{
			fwrite( buf, 1, size, out );
			if( ferror( out ) )
			{
				errnum = errno; // Get errno before it is overwritten by some system call.
				GfLogError("Failed to write data to %s when creating it from %s (%s).\n",
						   pszTgtName, pszSrcName, strerror(errnum));
				res = false;
				break;
			}
		}
		else if( ferror( in ) )
		{
			errnum = errno; // Get errno before it is overwritten by some system call.
			GfLogError("Failed to read data from %s when copying it to %s (%s).\n",
					   pszSrcName, pszTgtName, strerror(errnum));
			res = false;
			break;
		}
	}

	fclose( in );
	fclose( out );

	// Set target file access attributes to "read/write for the current user".
	if (chmod( pszTgtName, 0640 ))
	{
		const int errnum = errno; // Get errno before it is overwritten by some system call.
		GfLogWarning("Failed to set 0640 attributes to %s (%s)\n",
					 pszTgtName, strerror(errnum));
	}

	return true;
}

