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


static void gfFileSetupCopy( char* dataLocation, char* localLocation, int major, int minor, void *localHandle, int count )
{
	// Copy the source file to its target place.
	if( !GfFileCopy( dataLocation, localLocation ) )
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
			char buf[32];
			snprintf( buf, 30, "versions/%d", count );
			GfParmSetStr( localHandle, buf, "Data location", dataLocation );
			GfParmSetStr( localHandle, buf, "Local location", localLocation );
			GfParmSetNum( localHandle, buf, "Major version", NULL, (tdble)major );
			GfParmSetNum( localHandle, buf, "Minor version", NULL, (tdble)minor );
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

