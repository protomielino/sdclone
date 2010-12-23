/**
 * This file contains function which can also be used by raceengineclient.
 * This function is needed in both modules 
 *
 * Copyright (C) 2006, Mart Kelder <mart@kelder31.nl>
 */

#include <tgfclient.h>
#include "racescreens.h"

#include <robot.h>

static const int nMaxFeatureNameLength = 30;

typedef struct Feature
{
	char name[ nMaxFeatureNameLength ];
	int value;
} tFeature;

static tFeature features_list[] =
{
	{ ROB_VAL_FEATURE_PENALTIES, RM_FEATURE_PENALTIES },
	{ ROB_VAL_FEATURE_TIMEDSESSION, RM_FEATURE_TIMEDSESSION },
	{ ROB_VAL_FEATURE_WETTRACK, RM_FEATURE_WETTRACK },
	
	/* Career mode features not yet resurrected (robots need work to support them).
	   { ROB_VAL_FEATURE_SC, RM_FEATURE_SC | RM_FEATURE_YELLOW | RM_FEATURE_PENALTIES },
	   { ROB_VAL_FEATURE_YELLOW, RM_FEATURE_YELLOW | RM_FEATURE_PENALTIES },
	   { ROB_VAL_FEATURE_RED, RM_FEATURE_RED },
	   { ROB_VAL_FEATURE_BLUE, RM_FEATURE_BLUE },
	   { ROB_VAL_FEATURE_PITEXIT, RM_FEATURE_PITEXIT | RM_FEATURE_PENALTIES },
	   { ROB_VAL_FEATURE_TIMEDSESSION, RM_FEATURE_TIMEDSESSION },
	   { ROB_VAL_FEATURE_PENALTIES, RM_FEATURE_PENALTIES }
	*/
};
static const int nFeatures = sizeof(features_list) / sizeof(tFeature);


int RmGetFeaturesList( void* param )
{
	int nCars;
	char const *cardllname;
	int caridx;
	char const *features;
	int driverFeatureMask;
	int raceFeatureMask = -1; // All bits set to 1.
	void *robhdle;
	
	char path[ 256 ];
	char buf[ 1024 ];
	int xx, yy;
	int features_index;
	int buf_index;

	nCars = GfParmGetEltNb( param, RM_SECT_DRIVERS );
	for( xx = 1; xx < nCars + 1; ++xx )
	{
		/* Open robot */
		sprintf( path, "%s/%d", RM_SECT_DRIVERS, xx );
		cardllname = GfParmGetStr( param, path, RM_ATTR_MODULE, "" );
		caridx = (int)GfParmGetNum( param, path, RM_ATTR_IDX, NULL, 0 );
		sprintf( buf, "%s/drivers/%s/%s.xml", GetLocalDir(), cardllname, cardllname );
		robhdle = GfParmReadFile( buf, GFPARM_RMODE_STD );

		if( !robhdle )
		{
			sprintf( buf, "drivers/%s/%s.xml", cardllname, cardllname );
			robhdle = GfParmReadFile( buf, GFPARM_RMODE_STD );
		}
		if( !robhdle )
			continue;

		driverFeatureMask = 0;

		sprintf( buf, "%s/%s/%d", ROB_SECT_ROBOTS, ROB_LIST_INDEX, caridx );
		if( strcmp( GfParmGetStr( robhdle, buf, ROB_ATTR_TYPE, ROB_VAL_ROBOT ), ROB_VAL_HUMAN ) == 0 )
		{
			if( strcmp( GfParmGetStr( robhdle, buf, ROB_ATTR_LEVEL, ROB_VAL_ROOKIE ), ROB_VAL_ROOKIE ) == 0 )
				driverFeatureMask |= RM_FEATURE_TIMEDSESSION | RM_FEATURE_WETTRACK;
			else if( strcmp( GfParmGetStr( robhdle, buf, ROB_ATTR_LEVEL, ROB_VAL_ROOKIE ), ROB_VAL_AMATEUR ) == 0 )
				driverFeatureMask |= RM_FEATURE_TIMEDSESSION | RM_FEATURE_WETTRACK;
			      /* | RM_FEATURE_BLUE */
			else if( strcmp( GfParmGetStr( robhdle, buf, ROB_ATTR_LEVEL, ROB_VAL_ROOKIE ), ROB_VAL_SEMI_PRO ) == 0 )
				driverFeatureMask |= RM_FEATURE_TIMEDSESSION | RM_FEATURE_WETTRACK;
			      /* | RM_FEATURE_PENALTIES | RM_FEATURE_SC | RM_FEATURE_YELLOW | RM_FEATURE_RED | */
			else if( strcmp( GfParmGetStr( robhdle, buf, ROB_ATTR_LEVEL, ROB_VAL_ROOKIE ), ROB_VAL_PRO ) == 0 )
				driverFeatureMask |= RM_FEATURE_TIMEDSESSION | RM_FEATURE_PENALTIES | RM_FEATURE_WETTRACK;
			      /*RM_FEATURE_SC | RM_FEATURE_YELLOW | RM_FEATURE_BLUE | RM_FEATURE_RED | RM_FEATURE_PITEXIT |*/		      
		} else if( strcmp( GfParmGetStr( robhdle, buf, ROB_ATTR_TYPE, ROB_VAL_ROBOT ), ROB_VAL_ROBOT ) == 0 )
		{
			sprintf( buf, "%s/%s/%d", ROB_SECT_ROBOTS, ROB_LIST_INDEX, caridx );
			features = GfParmGetStr( robhdle, buf, ROB_ATTR_FEATURES, "" );
			features_index = 0;
			buf_index = 0;
			while( true )
			{
				if( features[ features_index ] != '\0' && features[ features_index ] != ';'
					&& buf_index < nMaxFeatureNameLength )
				{
					/* Feature name not yet ended */
					buf[ buf_index ] = features[ features_index ];
					++buf_index;
					++features_index;
				} else if( features[ features_index ] == '\0' || features[ features_index ] == ';' )
				{
					/* Feature name ended, check for matched */
					buf[ buf_index ] = '\0';
					for( yy = 0; yy < nFeatures; ++yy )
					{
						if( strcmp( features_list[ yy ].name, buf ) == 0 )
						{
							driverFeatureMask |= features_list[ yy ].value;
						}
					}
	
					if( features[ features_index ] == '\0' )
						break; /* Leave */
					++features_index;
					buf_index = 0;
				}
			}	
		}
		GfLogDebug("Driver %s#%d supported-feature mask : 0x%02X\n",
				   cardllname, caridx, driverFeatureMask);

		/* Binary and: the raceFeatureMask is only the features all cars have */
		raceFeatureMask &= driverFeatureMask;

		GfParmReleaseHandle( robhdle );
	}

	GfLogTrace("Race supported-feature mask : 0x%02X\n", raceFeatureMask);
	
	return raceFeatureMask;
}

