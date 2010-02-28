/**
 * This file contains function which can also be used by raceengineclient.
 * This function is needed in both modules 
 *
 * Copyright (C) 2006, Mart Kelder <mart@kelder31.nl>
 */

#include <tgfclient.h>
#include "racescreens.h"

#include <robot.h>

#define MAX_FEATURE_LENGTH 30
#define NUMBER_OF_FEATURES 2

int RmGetFeaturesList( void* param )
{
	typedef struct Feature
	{
		char string[ MAX_FEATURE_LENGTH ];
		int value;
	} tFeature;

	int nCars;
	char const *cardllname;
	int caridx;
	char const *features;
	int feature_int = 0;
	int result = -1;
	void *robhdle;
	tFeature features_list[ NUMBER_OF_FEATURES ];
	char path[ 256 ];
	char buf[ 1024 ];
	int xx, yy;
	int features_index;
	int buf_index;

	/* Fill features_list */
	strcpy( features_list[0].string, ROB_VAL_FEATURE_PENALTIES );
	features_list[0].value = RM_FEATURE_PENALTIES;
	strcpy( features_list[1].string, ROB_VAL_FEATURE_TIMEDSESSION );
	features_list[1].value = RM_FEATURE_TIMEDSESSION;
	/*strcpy( features_list[0].string, ROB_VAL_FEATURE_SC );
	features_list[0].value = RM_FEATURE_SC | RM_FEATURE_YELLOW | RM_FEATURE_PENALTIES;
	strcpy( features_list[1].string, ROB_VAL_FEATURE_YELLOW );
	features_list[1].value = RM_FEATURE_YELLOW | RM_FEATURE_PENALTIES;
	strcpy( features_list[2].string, ROB_VAL_FEATURE_RED );
	features_list[2].value = RM_FEATURE_RED;
	strcpy( features_list[3].string, ROB_VAL_FEATURE_BLUE );
	features_list[3].value = RM_FEATURE_BLUE;
	strcpy( features_list[4].string, ROB_VAL_FEATURE_PITEXIT );
	features_list[4].value = RM_FEATURE_PITEXIT | RM_FEATURE_PENALTIES;
	strcpy( features_list[5].string, ROB_VAL_FEATURE_TIMEDSESSION );
	features_list[5].value = RM_FEATURE_TIMEDSESSION;
	strcpy( features_list[6].string, ROB_VAL_FEATURE_PENALTIES );
	features_list[6].value = RM_FEATURE_PENALTIES;*/

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

		sprintf( buf, "%s/%s/%d", ROB_SECT_ROBOTS, ROB_LIST_INDEX, caridx );
		if( strcmp( GfParmGetStr( robhdle, buf, ROB_ATTR_TYPE, ROB_VAL_ROBOT ), ROB_VAL_HUMAN ) == 0 )
		{
			if( strcmp( GfParmGetStr( robhdle, buf, ROB_ATTR_LEVEL, ROB_VAL_ROOKIE ), ROB_VAL_ROOKIE ) == 0 )
				feature_int = RM_FEATURE_TIMEDSESSION;
			else if( strcmp( GfParmGetStr( robhdle, buf, ROB_ATTR_LEVEL, ROB_VAL_ROOKIE ), ROB_VAL_AMATEUR ) == 0 )
				feature_int = RM_FEATURE_TIMEDSESSION /* | RM_FEATURE_BLUE */;
			else if( strcmp( GfParmGetStr( robhdle, buf, ROB_ATTR_LEVEL, ROB_VAL_ROOKIE ), ROB_VAL_SEMI_PRO ) == 0 )
				feature_int = /* RM_FEATURE_PENALTIES | RM_FEATURE_SC | RM_FEATURE_YELLOW | RM_FEATURE_RED | */ RM_FEATURE_TIMEDSESSION;
			else if( strcmp( GfParmGetStr( robhdle, buf, ROB_ATTR_LEVEL, ROB_VAL_ROOKIE ), ROB_VAL_PRO ) == 0 )
				feature_int = /*RM_FEATURE_SC | RM_FEATURE_YELLOW | RM_FEATURE_BLUE | RM_FEATURE_RED | RM_FEATURE_PITEXIT
					       |*/ RM_FEATURE_TIMEDSESSION | RM_FEATURE_PENALTIES;
			else
				feature_int = 0;
		} else if( strcmp( GfParmGetStr( robhdle, buf, ROB_ATTR_TYPE, ROB_VAL_ROBOT ), ROB_VAL_ROBOT ) == 0 )
		{
				
			sprintf( buf, "%s/%s/%d", ROB_SECT_ROBOTS, ROB_LIST_INDEX, caridx );
			features = GfParmGetStr( robhdle, buf, ROB_ATTR_FEATURES, "" );
			features_index = 0;
			buf_index = 0;
			while( true )
			{
				if( features[ features_index ] != '\0' && features[ features_index ] != ';' && buf_index < MAX_FEATURE_LENGTH )
				{
					/* Feature name not yet ended */
					buf[ buf_index ] = features[ features_index ];
					++buf_index;
					++features_index;
				} else if( features[ features_index ] == '\0' || features[ features_index ] == ';' )
				{
					/* Feature name ended, check for matched */
					buf[ buf_index ] = '\0';
					for( yy = 0; yy < NUMBER_OF_FEATURES; ++yy )
					{
						if( strcmp( features_list[ yy ].string, buf ) == 0 )
						{
							feature_int |= features_list[ yy ].value;
						}
					}
	
					if( features[ features_index ] == '\0' )
						break; /* Leave */
					++features_index;
					buf_index = 0;
				}
			}	
		}
				
		/* Binary and: the result is only the features all cars have */
		result &= feature_int;
		feature_int = 0;

		GfParmReleaseHandle( robhdle );
	}

	return result;
}

