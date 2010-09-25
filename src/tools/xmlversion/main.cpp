/***************************************************************************
                 main.cpp -- Versionned settings XML files installation
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

#include <tgf.h>
#include <portability.h>


static const char* strip_destdir(const char *filename, const char *destdir )
{
	int xx;
	int destdir_length;

	if( !destdir )
		return filename;
	if( !filename )
		return NULL;

	destdir_length = strlen( destdir );

	for( xx = 0; xx < destdir_length; ++xx )
		if( destdir[ xx ] != filename[ xx ] )
			return filename;

	return &filename[ destdir_length ];
}

static int findIndex( void *versionHandle, const char* dataLocation,
					  const char* userLocation, const char* path, bool dataOnly )
{
	int nbIndices = GfParmGetEltNb( versionHandle, path ) + 1;
	bool *indices = (bool*)malloc( sizeof(bool) * nbIndices );
	int curIndex;

	GfLogDebug("findIndex(h=%p, d=%s, u=%s, p=%s, dataonly=%d) : n=%d\n",
			   versionHandle, dataLocation, userLocation, path, dataOnly, nbIndices-1);
	
	memset( indices, false, nbIndices );

	if( GfParmListSeekFirst( versionHandle, path ) == 0 )
	{
		do
		{
			curIndex = atoi( GfParmListGetCurEltName( versionHandle, path ) );
			GfLogDebug("  Examining index %d : ", curIndex);
			
			if( curIndex >= 0 && curIndex < nbIndices )
				indices[ curIndex ] = true;

			if( strcmp( GfParmGetCurStr( versionHandle, path, "Data location", "" ), dataLocation ) == 0 &&
			    ( dataOnly || strcmp( GfParmGetCurStr( versionHandle, path, "Local location", "" ), userLocation ) == 0 ) )
			{
				GfLogDebug("yes.\n");
				free( indices );
				return curIndex;
			}
			GfLogDebug("no.\n");
		} while( GfParmListSeekNext( versionHandle, path ) == 0 );
	}

	curIndex = 0;
	while( indices[ curIndex ] )
		++curIndex;
	GfLogDebug("  New = %d.\n", curIndex);

	free( indices );

	return curIndex;
}

static int process( const char* versionFile, const char* dataLocation,
					const char* userLocation, const char * destdir )
{
	void *versionHandle;
	void *xmlHandle;
	int index;
	char *path;
	const char* actualDataLoc;
	int majorVer, minorVer;

	xmlHandle = GfParmReadFile( dataLocation, GFPARM_RMODE_STD );
	if( !xmlHandle )
	{
		fprintf( stderr, "xmlversion: Can't open \"%s\".\n", dataLocation );
		return 1;
	}

	versionHandle = GfParmReadFile( versionFile, GFPARM_RMODE_CREAT );
	if( !versionHandle )
	{
		fprintf( stderr, "xmlversion: Can't open or create \"%s\".\n", versionFile );
		return 1;
	}

	index = findIndex( versionHandle, dataLocation, userLocation,  "versions", false );
	actualDataLoc = strip_destdir( dataLocation, destdir );
	majorVer = GfParmGetMajorVersion( xmlHandle );
	minorVer = GfParmGetMinorVersion( xmlHandle );
	
	path = (char*)malloc( sizeof(char) * 31 );
	snprintf( path, 30, "versions/%d", index );

	GfParmSetStr( versionHandle, path, "Data location", actualDataLoc);
	GfParmSetStr( versionHandle, path, "Local location", userLocation );
	GfParmSetNum( versionHandle, path, "Major version", NULL, (tdble)majorVer);
	GfParmSetNum( versionHandle, path, "Minor version", NULL, (tdble)minorVer);

	free( path );

	GfParmWriteFile( NULL, versionHandle, "versions" );
	
	GfParmReleaseHandle( versionHandle );
	GfParmReleaseHandle( xmlHandle );

	fprintf(stderr, "xmlversion: Updated %s (file #%d %s (version %d.%d) => %s).\n",
			versionFile, index, actualDataLoc, majorVer, minorVer, userLocation);

	return 0;
}

static int add_directory( const char* versionFile, const char* directoryName, const char *destdir )
{
	void *versionHandle;
	char *path;
	int index;
	const char* actualDataLoc;

	versionHandle = GfParmReadFile( versionFile, GFPARM_RMODE_STD );
	if( !versionHandle )
	{
		GfParmReleaseHandle( versionHandle );
		fprintf( stderr, "xmlversion: Can't open or create \"%s\".\n", versionFile );
		return 1;
	}

	index = findIndex( versionHandle, directoryName, "", "directories", true );
	actualDataLoc = strip_destdir( directoryName, destdir );

	path = (char*)malloc( sizeof(char) * 31 );
	snprintf( path, 30, "directories/%d", index );

	GfParmSetStr( versionHandle, path, "Data location", actualDataLoc);

	free( path );

	GfParmWriteFile( NULL, versionHandle, "versions" );
	
	GfParmReleaseHandle( versionHandle );

	fprintf(stderr, "xmlversion: Updated %s (directory %s).\n", versionFile, actualDataLoc);

	return 0;
}

int main( int argc, char **argv )
{
	const char *versionfile;
	const char *dataLocation;
	const char *userLocation;
	const char *destdir;
	int ret;

	if( argc <= 3 )
	{
		fprintf( stderr, "Usage: xmlversion version-file data-location local-location\n\n" );
		fprintf( stderr, "   version-file: The location of the version.xml file\n" );
		fprintf( stderr, "   data-location: Full path and filename to the location of installed xml-file\n" );
		fprintf( stderr, "   local-location: path and filename to the location of the local xml-file\n" );
		fprintf( stderr, "                   (relative to the users local directory)\n\n" );
		fprintf( stderr, "Usage: xmlversion -d version-file local-location\n\n" );
		fprintf( stderr, "   This command causes a directory local-location to be made at startup\n");
		fprintf( stderr, "   version-file: The location of the version.xml file\n" );
		fprintf( stderr, "   local-location: path and filename to the location of the local xml-file\n" );
		fprintf( stderr, "                   (relative to the users local directory)\n\n" );
		fprintf( stderr, "NOTE: this program should normally only be used automatically during installation\n" );
		return 1; //Not enough arguments
	}

	// Uncomment to get debug traces.
    //GfInit(); 

	if( argc > 4 )
		fprintf( stderr, "Warning: Too many arguments (should be 3). Ignoring extra ones.\n" );

	versionfile = argv[1];
	dataLocation = argv[2];
	userLocation = argv[3];
	destdir = getenv( "DESTDIR" );
	GfLogDebug("xmlversion: DESTDIR='%s'\n", destdir ? destdir : "<undefined>");
	GfLogDebug("xmlversion: versionfile='%s'\n", versionfile);
	GfLogDebug("xmlversion: dataLocation='%s'\n", dataLocation);
	GfLogDebug("xmlversion: userLocation='%s'\n", userLocation);
	
	if( strcmp( versionfile, "-d" ) == 0 )
		ret = add_directory( dataLocation, userLocation, destdir );
	else if( strcmp( dataLocation, "-d" ) == 0 )
		ret = add_directory( versionfile, userLocation, destdir );
	else if( strcmp( userLocation, "-d" ) == 0 )
		ret = add_directory( versionfile, dataLocation, destdir );
	else
		ret = process( versionfile, dataLocation, userLocation, destdir );

	exit( ret );
}

