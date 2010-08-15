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

#include <stdio.h>
#ifndef WIN32
#include <sys/stat.h>
#endif //!WIN32

#include "tgf.h"
#include "portability.h"

#ifndef TRUE
#define TRUE 1
#endif //TRUE
#ifndef FALSE
#define FALSE 0
#endif //FALSE

static int GfFileSetupCopyFile( const char* dataLocation, const char* localLocation )
{
	FILE *in;
	FILE *out;
	char buf[1024];
	size_t size;

	if( ( in = fopen( dataLocation, "rb" ) ) == NULL )
		return -1;
	if( ( out = fopen( localLocation, "wb" ) ) == NULL )
	{
		fclose( in );
		return -1;
	}

	GfLogTrace("Updating %s\n", localLocation);

	while( !feof( in ) )
	{
		size = fread( buf, 1, 1024, in );
		if( size > 0 )
			fwrite( buf, 1, size, out );
	}

	fclose( in );
	fclose( out );

#ifndef WIN32
	chmod( localLocation, 0640 );
#endif //!WIN32

	return 0;
}

static void GfFileSetupCopy( char* dataLocation, char* localLocation, int major, int minor, void *localHandle, int count )
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
	  GfCreateDir( stringBuf );
	}

	// Copy the source file to its target place.
	if( GfFileSetupCopyFile( dataLocation, localLocation ) != 0 )
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
	char *count;
	int countLength;
	int index;
	int found;
	int major;
	int minor;

	filenameLength = strlen(GetDataDir()) + 12 + 40;
	filename = (char*)malloc( sizeof(char) * filenameLength );
	sprintf( filename, "%sversion.xml", GetDataDir() );
	dataVersionHandle = GfParmReadFile( filename, GFPARM_RMODE_STD );
	if( !dataVersionHandle )
	{
		free( filename );
		return;
	}

	if( filenameLength < strlen(GetLocalDir()) + 12 )
	{
		free( filename );
		filenameLength = strlen(GetLocalDir()) + 12 + 40;
		filename = (char*)malloc( sizeof(char) * filenameLength );
	}

	GfCreateDir( GetLocalDir() );

	sprintf( filename, "%sversion.xml", GetLocalDir() );
	localVersionHandle = GfParmReadFile( filename, GFPARM_RMODE_CREAT );
	GfParmWriteFile( NULL, localVersionHandle, "versions" );
	
	if( !localVersionHandle )
	{
		free( filename );
		GfParmReleaseHandle( dataVersionHandle );
		return;
	}

	if( GfParmListSeekFirst( dataVersionHandle, "versions" ) != 0 )
	{
		free( filename );
		GfParmReleaseHandle( dataVersionHandle );
		GfParmReleaseHandle( localVersionHandle );
		return;
	}

	countLength = GfParmGetEltNb( localVersionHandle, "versions" ) + GfParmGetEltNb( dataVersionHandle, "versions" ) + 2;
	count = (char*)malloc( sizeof(char) * countLength );
	memset( count, 0, sizeof(char) * countLength );
	if( GfParmListSeekFirst( localVersionHandle, "versions" ) == 0 )
	{
		do
		{
			index = atoi( GfParmListGetCurEltName( localVersionHandle, "versions" ) );
			if( 0 <= index && index < countLength )
				count[index] = TRUE;
		} while( GfParmListSeekNext( localVersionHandle, "versions" ) == 0 );
	}

	// For each file referenced in the installation version.xml
	do
	{
		found = FALSE;

		// Get its installation path (data), user settings path (local),
		// and new major and minor version numbers
		dataLocation = strdup( GfParmGetCurStr( dataVersionHandle, "versions", "Data location", "" ) );
		localLocation = strdup( GfParmGetCurStr( dataVersionHandle, "versions", "Local location", "" ) );
		major = (int)GfParmGetCurNum( dataVersionHandle, "versions", "Major version", NULL, 0 );
		minor = (int)GfParmGetCurNum( dataVersionHandle, "versions", "Minor version", NULL, 0 );

		absLocalLocation = (char*)malloc( sizeof(char)*(strlen(GetLocalDir())+strlen(localLocation)+3) );
		sprintf( absLocalLocation, "%s%s", GetLocalDir(), localLocation );

		GfLogTrace("Checking %s : user settings version ", localLocation);

		// Search for its old major and minor version numbers in the user settings.
		if( GfParmListSeekFirst( localVersionHandle, "versions" ) == 0 )
		{
			do
			{
				if( strcmp( absLocalLocation, GfParmGetCurStr( localVersionHandle, "versions", "Local location", "" ) ) == 0 )
				{
					found = TRUE;
					const int locMinor = (int)GfParmGetCurNum( localVersionHandle, "versions", "Minor version", NULL, 0 );
					const int locMajor = (int)GfParmGetCurNum( localVersionHandle, "versions", "Major version", NULL, 0 );

					GfLogTrace("%d.%d is ", locMajor, locMinor);

					if( locMajor != major || locMinor < minor)
					{
						GfLogTrace("obsolete (installed one is %d.%d) => updating ...\n",
								   major, minor);
						GfFileSetupCopy( dataLocation, absLocalLocation, major, minor, localVersionHandle, -1 );
					}
					else
					    GfLogTrace("up-to-date.\n");

					break;
				}
			} while( GfParmListSeekNext( localVersionHandle, "versions" ) == 0 );
		}

		if( !found )
		{
			index = 0;
			while( count[index] )
				++index;
			GfLogTrace("not found => installing ...\n");
			GfFileSetupCopy( dataLocation, absLocalLocation, major, minor, localVersionHandle, index );
			count[index] = TRUE;
		}

		free( dataLocation );
		free( localLocation );
		free( absLocalLocation );

		GfParmWriteFile( NULL, localVersionHandle, "versions" );

	} while( GfParmListSeekNext( dataVersionHandle, "versions" ) == 0 );

	GfParmReleaseHandle( dataVersionHandle );
	GfParmWriteFile( NULL, localVersionHandle, "versions" );
	GfParmReleaseHandle( localVersionHandle );
	free( count );
	free( filename );
}

