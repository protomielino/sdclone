/***************************************************************************
                 filesetup.cpp -- Versioned settings XML files installation
                             -------------------                                         
    created              : 2009
    author               : Mart Kelder
    web                  : http://speed-dreams.sourceforge.net   
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
    		Versioned settings XML files installation at run-time
    @author	Mart Kelder
    @ingroup	tgf
*/

#include <cstdio>
#include <cerrno>
#include <sys/stat.h>

#include "tgf.h"
#include "portability.h"


static int gfFileSetupCopyFile( const char* dataLocation, const char* localLocation )
{
	FILE *in;
	FILE *out;
	char buf[1024];
	size_t size;
	int errnum;
	int res = 0;
	
	if( ( in = fopen( dataLocation, "rb" ) ) == NULL )
	{
		errnum = errno; // Get errno before it is overwritten by some system call.
		GfLogError("Could not open %s in 'rb' mode when copying it to %s (%s).\n",
				   dataLocation, localLocation, strerror(errnum));
		return -1;
	}
	if( ( out = fopen( localLocation, "wb" ) ) == NULL )
	{
		errnum = errno; // Get errno before it is overwritten by some system call.
		GfLogError("Could not open %s in 'w' mode when creating it from %s (%s).\n",
				   localLocation, dataLocation, strerror(errnum));
		fclose( in );
		return -1;
	}

	GfLogDebug("Updating %s\n", localLocation);

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
						   localLocation, dataLocation, strerror(errnum));
				res = -1;
				break;
			}
		}
		else if( ferror( in ) )
		{
			errnum = errno; // Get errno before it is overwritten by some system call.
			GfLogError("Failed to read data from %s when copying it to %s (%s).\n",
					   dataLocation, localLocation, strerror(errnum));
			res = -1;
			break;
		}
	}

	fclose( in );
	fclose( out );

#ifndef WIN32
	chmod( localLocation, 0640 );
#endif //!WIN32

	return res;
}

static void gfFileSetupCopy( char* dataLocation, char* localLocation, int major, int minor, void *localHandle, int count )
{
	static const size_t maxBufSizeSize = 1024;
	char stringBuf[maxBufSizeSize];

	// Create the target local directory (and parents) if not already done
	// (first, we have to deduce its path from the target file path-name).
	strncpy(stringBuf, localLocation, strlen(localLocation)+1);
#ifdef WIN32
	int i;
	for (i = 0; i < maxBufSizeSize && stringBuf[i] != '\0'; i++) {
		if (stringBuf[i] == '\\') {
			stringBuf[i] = '/';
		}
	}
#endif
	char *lastSlash = strrchr(stringBuf, '/');
	if (lastSlash)
	{
	  *lastSlash = '\0';
	  GfDirCreate( stringBuf );
	}

	// Copy the source file to its target place.
	if( gfFileSetupCopyFile( dataLocation, localLocation ) != 0 )
		return;

	// Update local version.xml file.
	if( localHandle )
	{
		if( count < 0 )
		{
			GfParmSetCurStr( localHandle, "versions", "Data location", dataLocation );
			GfParmSetCurStr( localHandle, "versions", "Local location", localLocation );
			GfParmSetCurNum( localHandle, "versions", "Major version", NULL, (tdble)major );
			GfParmSetCurNum( localHandle, "versions", "Minor version", NULL, (tdble)minor );
		}
		else
		{
			snprintf( stringBuf, 30, "versions/%d", count );
			GfParmSetStr( localHandle, stringBuf, "Data location", dataLocation );
			GfParmSetStr( localHandle, stringBuf, "Local location", localLocation );
			GfParmSetNum( localHandle, stringBuf, "Major version", NULL, (tdble)major );
			GfParmSetNum( localHandle, stringBuf, "Minor version", NULL, (tdble)minor );
		}
	}
}

void GfFileSetup()
{
	void *dataVersionHandle;
	void *localVersionHandle;
	char *filename;
	size_t filenameLength;
	char *dataLocation;
	char *localLocation;
	char *absLocalLocation;
	char *absDataLocation;
	bool *count;
	int countLength;
	int index;
	bool found;
	int major;
	int minor;
	struct stat st;

	// Open data (installation) version.xml.
	filenameLength = strlen(GetDataDir()) + 12 + 40;
	filename = (char*)malloc( sizeof(char) * filenameLength );
	sprintf( filename, "%sversion.xml", GetDataDir() );
	dataVersionHandle = GfParmReadFile( filename, GFPARM_RMODE_STD );
	if( !dataVersionHandle )
	{
		free( filename );
		return;
	}

	// Exit if nothing inside.
	if( GfParmListSeekFirst( dataVersionHandle, "versions" ) != 0 )
	{
		free( filename );
		GfParmReleaseHandle( dataVersionHandle );
		return;
	}

	// Create LocalDir (user settings root) if not already done.
	GfDirCreate( GetLocalDir() );

	// Open local (user settings) version.xml (create it if not there).
	if( filenameLength < strlen(GetLocalDir()) + 12 )
	{
		free( filename );
		filenameLength = strlen(GetLocalDir()) + 12 + 40;
		filename = (char*)malloc( sizeof(char) * filenameLength );
	}

	sprintf( filename, "%sversion.xml", GetLocalDir() );
	localVersionHandle = GfParmReadFile( filename, GFPARM_RMODE_CREAT );
	GfParmWriteFile( NULL, localVersionHandle, "versions" );

	// Exit if open/creation failed.
	if( !localVersionHandle )
	{
		free( filename );
		GfParmReleaseHandle( dataVersionHandle );
		return;
	}

	// Setup the index of the XML files referenced in the local version.xml.
	countLength = GfParmGetEltNb( localVersionHandle, "versions" )
		          + GfParmGetEltNb( dataVersionHandle, "versions" ) + 2;
	count = (bool*)malloc( sizeof(bool) * countLength );
	for( index = 0; index < countLength; index++ )
		count[index] = false;
	if( GfParmListSeekFirst( localVersionHandle, "versions" ) == 0 )
	{
		do
		{
			index = atoi( GfParmListGetCurEltName( localVersionHandle, "versions" ) );
			if( 0 <= index && index < countLength )
				count[index] = true;
		} while( GfParmListSeekNext( localVersionHandle, "versions" ) == 0 );
	}

	// For each file referenced in the installation version.xml
	do
	{
		found = false;

		// Get its installation path (data), user settings path (local),
		// and new major and minor version numbers
		dataLocation = strdup( GfParmGetCurStr( dataVersionHandle, "versions", "Data location", "" ) );
		localLocation = strdup( GfParmGetCurStr( dataVersionHandle, "versions", "Local location", "" ) );
		major = (int)GfParmGetCurNum( dataVersionHandle, "versions", "Major version", NULL, 0 );
		minor = (int)GfParmGetCurNum( dataVersionHandle, "versions", "Minor version", NULL, 0 );

		absLocalLocation = (char*)malloc( sizeof(char)*(strlen(GetLocalDir())+strlen(localLocation)+3) );
		sprintf( absLocalLocation, "%s%s", GetLocalDir(), localLocation );

		absDataLocation = (char*)malloc( sizeof(char)*(strlen(GetDataDir())+strlen(dataLocation)+3) );
		sprintf( absDataLocation, "%s%s", GetDataDir(), dataLocation );

		GfLogTrace("Checking %s : user settings version ", localLocation);

		// Search for its old major and minor version numbers in the user settings.
		if( GfParmListSeekFirst( localVersionHandle, "versions" ) == 0 )
		{
			do
			{
				if( strcmp( absLocalLocation, GfParmGetCurStr( localVersionHandle, "versions", "Local location", "" ) ) == 0 )
				{
					found = true;
					const int locMinor = (int)GfParmGetCurNum( localVersionHandle, "versions", "Minor version", NULL, 0 );
					const int locMajor = (int)GfParmGetCurNum( localVersionHandle, "versions", "Major version", NULL, 0 );

					GfLogTrace("%d.%d is ", locMajor, locMinor);

					if( locMajor != major || locMinor < minor)
					{
						GfLogTrace("obsolete (installed one is %d.%d) => updating ...\n",
								   major, minor);
						gfFileSetupCopy( absDataLocation, absLocalLocation, major, minor, localVersionHandle, -1 );
					}
					else
					{
					    GfLogTrace("up-to-date");
						if (stat(absLocalLocation, &st))
						{
							GfLogTrace(", but the file is not there => installing ...\n");
							gfFileSetupCopy( absDataLocation, absLocalLocation, major, minor, localVersionHandle, -1 );
						}
						else
							GfLogTrace(".\n");
					}
					
					break;
				}
			} while( GfParmListSeekNext( localVersionHandle, "versions" ) == 0 );
		}

		if( !found)
		{
			index = 0;
			while( count[index] )
				++index;
			GfLogTrace("not found => installing ...\n");
			gfFileSetupCopy( absDataLocation, absLocalLocation, major, minor, localVersionHandle, index );
			count[index] = true;
		}

		free( dataLocation );
		free( localLocation );
		free( absDataLocation );
		free( absLocalLocation );

		GfParmWriteFile( NULL, localVersionHandle, "versions" );

	} while( GfParmListSeekNext( dataVersionHandle, "versions" ) == 0 );

	GfParmReleaseHandle( dataVersionHandle );
	GfParmWriteFile( NULL, localVersionHandle, "versions" );
	GfParmReleaseHandle( localVersionHandle );
	free( count );
	free( filename );
}

