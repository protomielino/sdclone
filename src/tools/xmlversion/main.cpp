#include <tgf.h>

#ifndef TRUE
#define TRUE 1
#endif //TRUE

#ifndef FALSE
#define FALSE 0
#endif //FALSE

#ifdef WIN32
#define snprintf _snprintf
#endif

static char* strip_destdir( char *filename, char const *destdir )
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

// Uncomment to get debug traces in findIndex.
#define DEBUG_FINDINDEX
#ifdef DEBUG_FINDINDEX
#define trace printf
#else
#define trace
#endif

static int findIndex( void *versionHandle, const char* dataLocation,
					  const char* userLocation, char* path, bool dataOnly )
{
	int nbIndices = GfParmGetEltNb( versionHandle, path ) + 1;
	char *indices = (char*)malloc( sizeof(char) * nbIndices );
	int curIndex;

	trace("findIndex(h=%p, d=%s, u=%s, p=%s, dataonly=%d) : n=%d\n",
		  versionHandle, dataLocation, userLocation, path, dataOnly, nbIndices-1);
	
	memset( indices, FALSE, nbIndices );

	if( GfParmListSeekFirst( versionHandle, path ) == 0 )
	{
		do
		{
			curIndex = atoi( GfParmListGetCurEltName( versionHandle, path ) );
			trace("  Examining index %d : ", curIndex);
			
			if( curIndex >= 0 && curIndex < nbIndices )
				indices[ curIndex ] = TRUE;

			if( strcmp( GfParmGetCurStr( versionHandle, path, "Data location", "" ), dataLocation ) == 0 &&
			    ( dataOnly || strcmp( GfParmGetCurStr( versionHandle, path, "Local location", "" ), userLocation ) == 0 ) )
			{
				trace("yes.\n");
				free( indices );
				return curIndex;
			}
			trace("no.\n");
		} while( GfParmListSeekNext( versionHandle, path ) == 0 );
	}

	curIndex = 0;
	while( indices[ curIndex ] )
		++curIndex;
	trace("  New = %d.\n", curIndex);

	free( indices );

	return curIndex;
}

static int process( const char* versionFile, char* dataLocation, char* userLocation, char const* destdir )
{
	void *versionHandle;
	void *xmlHandle;
	int index;
	char *path;
	char *pathStart = strdup( "versions" );
	char* actualDataLoc;
	int majorVer, minorVer;

	xmlHandle = GfParmReadFile( dataLocation, GFPARM_RMODE_STD );
	if( !xmlHandle )
	{
		fprintf( stderr, "xmlversion: Cannot open xml-file \"%s\".\n", dataLocation );
		return 1;
	}

	versionHandle = GfParmReadFile( versionFile, GFPARM_RMODE_CREAT );
	if( !versionHandle )
	{
		fprintf( stderr, "xmlversion: Cannot open or create xml-file \"%s\".\n", versionFile );
		return 1;
	}

	index = findIndex( versionHandle, dataLocation, userLocation, pathStart, false );
	actualDataLoc = strip_destdir( dataLocation, destdir );
	majorVer = GfParmGetMajorVersion( xmlHandle );
	minorVer = GfParmGetMinorVersion( xmlHandle );
	
	path = (char*)malloc( sizeof(char) * 31 );
	snprintf( path, 30, "versions/%d", index );

	GfParmSetStr( versionHandle, path, "Data location", actualDataLoc);
	GfParmSetStr( versionHandle, path, "Local location", userLocation );
	GfParmSetNum( versionHandle, path, "Major version", NULL, (tdble)majorVer);
	GfParmSetNum( versionHandle, path, "Minor version", NULL, (tdble)minorVer);

	free( pathStart );
	free( path );

	GfParmWriteFile( NULL, versionHandle, "versions" );
	
	GfParmReleaseHandle( versionHandle );
	GfParmReleaseHandle( xmlHandle );

	fprintf(stderr, "xmlversion: updated %s (file #%d %s (version %d.%d) => %s).\n",
			versionFile, index, actualDataLoc, majorVer, minorVer, userLocation);

	return 0;
}

static int add_directory( const char* versionFile, char* directoryName, char const* destdir )
{
	void *versionHandle;
	char *path;
	char *pathStart = strdup( "directories" );
	int index;
	char* actualDataLoc;

	versionHandle = GfParmReadFile( versionFile, GFPARM_RMODE_STD );
	if( !versionHandle )
	{
		GfParmReleaseHandle( versionHandle );
		fprintf( stderr, "xmlversion: Cannot open or create xml-file \"%s\".\n", versionFile );
		return 1;
	}

	index = findIndex( versionHandle, directoryName, "", pathStart, true );
	actualDataLoc = strip_destdir( directoryName, destdir );

	path = (char*)malloc( sizeof(char) * 31 );
	snprintf( path, 30, "directories/%d", index );

	GfParmSetStr( versionHandle, path, "Data location", actualDataLoc);

	free( pathStart );
	free( path );

	GfParmWriteFile( NULL, versionHandle, "versions" );
	
	GfParmReleaseHandle( versionHandle );

	fprintf(stderr, "xmlversion: updated %s (directory %s).\n", versionFile, actualDataLoc);

	return 0;
}

int main( int argc, char **argv )
{
	char *versionfile;
	char *dataLocation;
	char *userLocation;
	char const *destdir;
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

	if( argc > 4 )
		fprintf( stderr, "Warning: too many arguments in xmlversion. Ignoring extra arguments\n" );

	versionfile = strdup( argv[1] );
	dataLocation = strdup( argv[2] );
	userLocation = strdup( argv[3] );
	destdir = getenv( "DESTDIR" );
	fprintf(stderr, "xmlversion: DESTDIR=%s\n", destdir ? destdir : "<undefined>");
	
	if( strcmp( versionfile, "-d" ) == 0 )
		ret = add_directory( dataLocation, userLocation, destdir );
	else if( strcmp( dataLocation, "-d" ) == 0 )
		ret = add_directory( versionfile, userLocation, destdir );
	else if( strcmp( userLocation, "-d" ) == 0 )
		ret = add_directory( versionfile, dataLocation, destdir );
	else
		ret = process( versionfile, dataLocation, userLocation, destdir );

	free( versionfile );
	free( dataLocation );
	free( userLocation );

	exit( ret );
}

