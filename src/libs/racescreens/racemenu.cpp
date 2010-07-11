/***************************************************************************

    file                 : racemenu.cpp
    created              : Thu May  2 22:02:51 CEST 2002
    copyright            : (C) 2001 by Eric Espie
    email                : eric.espie@torcs.org
    version              : $Id: racemenu.cpp,v 1.2 20 Mar 2006 04:31:12 torcs Exp $

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
    		This is the race options menu.
    @ingroup	racemantools
    @author	<a href=mailto:eric.espie@torcs.org>Eric Espie</a>
    @version	$Id: racemenu.cpp,v 1.2 2003/06/24 21:02:24 torcs Exp $
*/

#include <cstdlib>
#include <cstdio>
#include <cstring>
#ifdef WIN32
#include <windows.h>
#endif


#include <tgfclient.h>
#include <track.h>
#include <car.h>
#include <raceman.h>
#include <robot.h>
#include <graphic.h>

#include "racescreens.h"

#define CLOUD_VAL_NO 0
#define CLOUD_VAL_SCARCE 1
#define CLOUD_VAL_MORE 2
#define CLOUD_VAL_OVERCAST 3
 
typedef struct WeatherOption
{
	const char* name;
	float rain; // l/m2/h
	int clouds;
} tWeatherOption;

// Constants.
static const char *DispModeValues[] = { RM_VAL_VISIBLE, RM_VAL_INVISIBLE};

static const tWeatherOption WeatherValues[] = { { "Clear sky", 0.0f, CLOUD_VAL_NO },
                                                { "Scarce Clouds", 0.0f, CLOUD_VAL_SCARCE },
						{ "More Clouds", 0.0f, CLOUD_VAL_MORE },
						{ "Overcast Sky", 0.0f, CLOUD_VAL_OVERCAST },
						{ "Drizzle", 1.0f, CLOUD_VAL_OVERCAST },
						{ "Rain", 5.0f, CLOUD_VAL_OVERCAST },
						{ "Heavy Rain", 20.0f, CLOUD_VAL_OVERCAST } };

static const int NWeatherValues = sizeof( WeatherValues ) / sizeof( tWeatherOption );

static const int NTimeOfDayValues = 4;
static const char *TimeOfDayValues[NTimeOfDayValues] = {"Night", "Dawn", "Morning", "Noon"};

// Global variables.
static void		*scrHandle;
static tRmRaceParam	*rp;

static int		rmrpDistId;
static int		rmrpLapsId;
static int		rmrpSessionTimeId;
static int		rmrpDispModeId;
static int		rmrpWeatherId;
static int		rmrpTimeOfDayId;

static int		rmrpDistance;
static int		rmrpLaps;
static int		rmrpSessionTime;
static int		rmrpDispMode;
static int		rmrpWeather;
static int		rmrpTimeOfDay;

static int		rmrpFeatures;



static void
rmrpDeactivate(void *screen)
{
    GfuiScreenRelease(scrHandle);
    
    if (screen) {
	GfuiScreenActivate(screen);
    }
}

static void
rmrpUpdDist(void * /* dummy */)
{
    char	buf[32];
    char	*val;

    val = GfuiEditboxGetString(scrHandle, rmrpDistId);
    rmrpDistance = strtol(val, (char **)NULL, 0);
    if (rmrpDistance == 0) {
	strcpy(buf, "---");
    } else {
	sprintf(buf, "%d", rmrpDistance);
	rmrpLaps = 0;
	GfuiEditboxSetString(scrHandle, rmrpLapsId, "---");
    }
    GfuiEditboxSetString(scrHandle, rmrpDistId, buf);
}

static void
rmrpUpdLaps(void * /* dummy */)
{
    char	buf[32];
    char	*val;

    val = GfuiEditboxGetString(scrHandle, rmrpLapsId);
    rmrpLaps = strtol(val, (char **)NULL, 0);
    if (rmrpLaps == 0) {
	strcpy(buf, "---");
    } else {
	sprintf(buf, "%d", rmrpLaps);
	rmrpDistance = 0;
	GfuiEditboxSetString(scrHandle, rmrpDistId, "---");
    }
    GfuiEditboxSetString(scrHandle, rmrpLapsId, buf);
}

static void
rmrpUpdSessionTime(void * /*dummy*/)
{
    char buf[64];
    char *val;
    int nbSep = 0;
    int subresult = 0;
    int result = 0;

    if ((rmrpFeatures & RM_FEATURE_TIMEDSESSION) == 0)
    	return;	/* No timed session feature => nothing to do here */

    val = GfuiEditboxGetString(scrHandle, rmrpSessionTimeId);
    
    while( true )
    {
	if (val[0] >= '0' && val[0] <= '9')
	{
	    subresult *= 10;
	    subresult += val[0] - '0';
	}
	else if (val[0] == ':')
	{
	    if (nbSep == 0 || subresult < 60)
	    {
		result *= 60;
		result += subresult;
		subresult = 0;
		++nbSep;
	    }
	    else
	    {
		result = 0;
		break;
	    }
	}
	else
	{
	    break;
	}

	++val;
    }

    if (nbSep == 0 || subresult < 60)
    {
	result *= 60;
	result += subresult;
    }
    else
    {
	result = 0;
    }
    
    rmrpSessionTime = result;
    
    if (rmrpSessionTime <= 0)
	strcpy( buf, "---");
    else
	sprintf(buf, "%d:%02d:%02d", (int)floor( (float)rmrpSessionTime / 3600.0f ), (int)floor( (float)rmrpSessionTime / 60.0f ) % 60, (int)floor( (float)rmrpSessionTime ) % 60 );
    GfuiEditboxSetString(scrHandle, rmrpSessionTimeId, buf);
}

void
rmChangeDisplayMode(void * /* dummy */)
{
    rmrpDispMode = 1 - rmrpDispMode;
    GfuiLabelSetText(scrHandle, rmrpDispModeId, DispModeValues[rmrpDispMode]);
}

static void rmChangeTime(void *vp)
{
    const long delta = (long)vp;
    rmrpTimeOfDay = (rmrpTimeOfDay + NTimeOfDayValues + delta) % NTimeOfDayValues;
    GfuiLabelSetText(scrHandle, rmrpTimeOfDayId, TimeOfDayValues[rmrpTimeOfDay]);;
}

static void rmChangeWeather(void *vp)
{
    const long delta = (long)vp;
    rmrpWeather = (rmrpWeather + NWeatherValues + delta) % NWeatherValues;
    GfuiLabelSetText(scrHandle, rmrpWeatherId, WeatherValues[rmrpWeather].name);
}


static void
rmrpValidate(void * /* dummy */)
{
    // Force current edit to loose focus (if one has it) and update associated variable.
    GfuiUnSelectCurrent();

    if (rp->confMask & RM_CONF_RACE_LEN) {
	GfParmSetNum(rp->param, rp->title, RM_ATTR_DISTANCE, "km", rmrpDistance);
	GfParmSetNum(rp->param, rp->title, RM_ATTR_LAPS, (char*)NULL, rmrpLaps);
	GfParmSetNum(rp->param, rp->title, RM_ATTR_SESSIONTIME, "s", (tdble)rmrpSessionTime);
	GfParmSetNum(rp->param, rp->title, RM_ATTR_WEATHER, NULL, rmrpWeather + 1); //TODO: delete this line
	GfParmSetNum(rp->param, rp->title, RM_ATTR_WEATHER_RAIN, "l/m2/h", WeatherValues[rmrpWeather].rain);
	switch( WeatherValues[rmrpWeather ].clouds )
	{
	case CLOUD_VAL_SCARCE:
		GfParmSetStr(rp->param, rp->title, RM_ATTR_WEATHER_CLOUDS, RM_VAL_SCARCE_CLOUDS );
		break;
	case CLOUD_VAL_MORE:
		GfParmSetStr(rp->param, rp->title, RM_ATTR_WEATHER_CLOUDS, RM_VAL_MORE_CLOUDS );
		break;
	case CLOUD_VAL_OVERCAST:
		GfParmSetStr(rp->param, rp->title, RM_ATTR_WEATHER_CLOUDS, RM_VAL_OVERCAST_CLOUDS );
		break;
	case CLOUD_VAL_NO:
	default:
		GfParmSetStr(rp->param, rp->title, RM_ATTR_WEATHER_CLOUDS, RM_VAL_NO_CLOUDS );
		break;
	}
	GfParmSetNum(rp->param, rp->title, RM_ATTR_TIME, NULL, rmrpTimeOfDay + 1);
    }

    if (rp->confMask & RM_CONF_DISP_MODE) {
	GfParmSetStr(rp->param, rp->title, RM_ATTR_DISPMODE, DispModeValues[rmrpDispMode]);
    }

    rmrpDeactivate(rp->nextScreen);
}

static void
rmrpAddKeys(void)
{
    GfuiAddKey(scrHandle, GFUIK_RETURN, "Accept", NULL, rmrpValidate, NULL);
    GfuiAddKey(scrHandle, GFUIK_ESCAPE, "Cancel", rp->prevScreen, rmrpDeactivate, NULL);
    GfuiAddKey(scrHandle, GFUIK_F1, "Help", scrHandle, GfuiHelpScreen, NULL);
    GfuiAddKey(scrHandle, GFUIK_F12, "Screen-Shot", NULL, GfuiScreenShot, NULL);
}

void
RmRaceParamMenu(void *vrp)
{
    char	buf[64];
    int         xx;

    rp = (tRmRaceParam*)vrp;

    rmrpFeatures = RmGetFeaturesList (rp->param);

    // Create screen, load menu XML descriptor and create static controls.
    scrHandle = GfuiScreenCreateEx((float*)NULL, NULL, NULL, NULL, (tfuiCallback)NULL, 1);   
    void *menuXMLDescHdle = LoadMenuXML("racemenu.xml");
    CreateStaticControls(menuXMLDescHdle,scrHandle);

    // Create variable title label.
    int titleId = CreateLabelControl(scrHandle,menuXMLDescHdle,"title");
    sprintf(buf, "%s Options", rp->title);
    GfuiLabelSetText(scrHandle,titleId,buf);
    
    if (rp->confMask & RM_CONF_RACE_LEN) 
    {
	// Create Race distance label.
	CreateLabelControl(scrHandle,menuXMLDescHdle,"racedistance");
	rmrpDistance = (int)GfParmGetNum(rp->param, rp->title, RM_ATTR_DISTANCE, "km", 0);

	// Create and initialize Race distance edit.
	rmrpSessionTime = (int)GfParmGetNum(rp->param, rp->title, RM_ATTR_SESSIONTIME, NULL, 0);
	if (rmrpSessionTime > 0 && ( rmrpFeatures & RM_FEATURE_TIMEDSESSION ) == 0 )
	    rmrpDistance += rmrpSessionTime / 30;
	if (rmrpDistance == 0) 
	{
	    strcpy(buf, "---");
	    rmrpLaps = (int)GfParmGetNum(rp->param, rp->title, RM_ATTR_LAPS, NULL, 25);
	    if (rmrpSessionTime > 0 && ( rmrpFeatures & RM_FEATURE_TIMEDSESSION ) == 0 )
		rmrpLaps += (int)floor( (tdble)rmrpSessionTime / 1.5f + 0.5f );
	} 
	else 
	{
	    sprintf(buf, "%d", rmrpDistance);
	    rmrpLaps = 0;
	}

	rmrpDistId = CreateEditControl(scrHandle,menuXMLDescHdle,"racedistanceedit",NULL,NULL,rmrpUpdDist);
	GfuiEditboxSetString(scrHandle,rmrpDistId,buf);

	// Create Laps label.
	CreateLabelControl(scrHandle,menuXMLDescHdle,"laps");
	
	// Create and initialize Laps edit.
	if (rmrpLaps == 0) 
	{
	    strcpy(buf, "---");
	} 
	else 
	{
	    sprintf(buf, "%d", rmrpLaps);
	}

	rmrpLapsId = CreateEditControl(scrHandle,menuXMLDescHdle,"lapsedit",NULL,NULL,rmrpUpdLaps);
	GfuiEditboxSetString(scrHandle,rmrpLapsId,buf);

	if (rmrpFeatures & RM_FEATURE_TIMEDSESSION)
	{
	    // Create Session time label.
	    CreateLabelControl(scrHandle,menuXMLDescHdle,"sessiontime");

	    // Create and initialize Session time edit.
	    if (rmrpSessionTime <= 0) 
	    {
		strcpy(buf, "---");
	    }
	    else 
	    {
		sprintf(buf, "%d:%02d:%02d", (int)floor((float) rmrpSessionTime / 3600.0f ), (int)floor( (float)rmrpSessionTime / 60.0f ) % 60, (int)floor( (float)rmrpSessionTime ) % 60 );
	    }
	    
	    rmrpSessionTimeId = CreateEditControl(scrHandle,menuXMLDescHdle,"sessiontimeedit",NULL,NULL,rmrpUpdSessionTime);
	    GfuiEditboxSetString(scrHandle,rmrpSessionTimeId,buf);
	}
	else
	{
	    rmrpSessionTime = 0;
	}
    }

    // Create and initialize Time of day combo box (2 arrow buttons and a variable label).
    rmrpTimeOfDay = ((int)GfParmGetNum(rp->param, rp->title, RM_ATTR_TIME, NULL, 0) - 1);

    CreateButtonControl(scrHandle,menuXMLDescHdle,"timeofdayleftarrow",(void*)-1, rmChangeTime);
    CreateButtonControl(scrHandle,menuXMLDescHdle,"timeofdayrightarrow",(void*)1, rmChangeTime);

    rmrpTimeOfDayId = CreateLabelControl(scrHandle,menuXMLDescHdle,"timeofdayedit");
    GfuiLabelSetText(scrHandle,rmrpTimeOfDayId,TimeOfDayValues[rmrpTimeOfDay]);
    
    //Read rmrpWeather from file. We first look to the value nearest to the rain
    rmrpWeather = 0;
    for( xx = 1; xx < NWeatherValues; ++xx )
    {
    	//First check if rain is nearer then the current value
    	if( fabs( WeatherValues[ xx          ].rain - GfParmGetNum(rp->param, rp->title, RM_ATTR_WEATHER_RAIN, "l/m2/h", 0.0f ) ) <
	    fabs( WeatherValues[ rmrpWeather ].rain - GfParmGetNum(rp->param, rp->title, RM_ATTR_WEATHER_RAIN, "l/m2/h", 0.0f ) ) )
	{
		rmrpWeather = xx;
	}
    	else if( fabs( WeatherValues[ xx          ].rain - GfParmGetNum(rp->param, rp->title, RM_ATTR_WEATHER_RAIN, "l/m2/h", 0.0f ) ) <
	         fabs( WeatherValues[ rmrpWeather ].rain - GfParmGetNum(rp->param, rp->title, RM_ATTR_WEATHER_RAIN, "l/m2/h", 0.0f ) ) )
	{
		//Rain is equal far off, look if we have a match in the cloud type
		switch( WeatherValues[ xx ].clouds )
		{
		case CLOUD_VAL_SCARCE:
			if( strcmp( RM_VAL_SCARCE_CLOUDS, GfParmGetStr(rp->param, rp->title, RM_ATTR_WEATHER_CLOUDS, RM_VAL_NO_CLOUDS ) ) == 0 )
				rmrpWeather = xx;
			break;
		case CLOUD_VAL_MORE:
			if( strcmp( RM_VAL_MORE_CLOUDS, GfParmGetStr(rp->param, rp->title, RM_ATTR_WEATHER_CLOUDS, RM_VAL_NO_CLOUDS ) ) == 0 )
				rmrpWeather = xx;
			break;
		case CLOUD_VAL_OVERCAST:
			if( strcmp( RM_VAL_OVERCAST_CLOUDS, GfParmGetStr(rp->param, rp->title, RM_ATTR_WEATHER_CLOUDS, RM_VAL_NO_CLOUDS ) ) == 0 )
				rmrpWeather = xx;
			break;
		case CLOUD_VAL_NO:
		default:
			if( strcmp( RM_VAL_NO_CLOUDS, GfParmGetStr(rp->param, rp->title, RM_ATTR_WEATHER_CLOUDS, RM_VAL_NO_CLOUDS ) ) == 0 )
				rmrpWeather = xx;
			break;
		}
	}
	
    }
    
    CreateButtonControl(scrHandle,menuXMLDescHdle,"weatherleftarrow",(void*)-1, rmChangeWeather);
    CreateButtonControl(scrHandle,menuXMLDescHdle,"weatherrightarrow",(void*)1, rmChangeWeather);

    rmrpWeatherId = CreateLabelControl(scrHandle,menuXMLDescHdle,"weatheredit");
    GfuiLabelSetText(scrHandle,rmrpWeatherId,WeatherValues[rmrpWeather].name);
    
    if (rp->confMask & RM_CONF_DISP_MODE) 
    {
	if (!strcmp(GfParmGetStr(rp->param, rp->title, RM_ATTR_DISPMODE, RM_VAL_VISIBLE), RM_VAL_INVISIBLE)) 
	{
	    rmrpDispMode = 1;
	}
	else 
	{
	    rmrpDispMode = 0;
	}

	// Create Display mode label.
	CreateLabelControl(scrHandle,menuXMLDescHdle,"display");

	// Create and initialize Display mode combo-box-like control.
	CreateButtonControl(scrHandle,menuXMLDescHdle,"displayleftarrow",(void*)0, rmChangeDisplayMode);
	CreateButtonControl(scrHandle,menuXMLDescHdle,"displayrightarrow",(void*)1, rmChangeDisplayMode);
	rmrpDispModeId = CreateLabelControl(scrHandle,menuXMLDescHdle,"displayedit");
	GfuiLabelSetText(scrHandle,rmrpDispModeId,DispModeValues[rmrpDispMode]);
    }
	
    // Create Accept and Cancel buttons
    CreateButtonControl(scrHandle,menuXMLDescHdle,"accept",NULL,rmrpValidate);
    CreateButtonControl(scrHandle,menuXMLDescHdle,"cancel",rp->prevScreen,rmrpDeactivate);
    
    // Close menu XML descriptor.
    GfParmReleaseHandle(menuXMLDescHdle);
    
    // Register keyboard shortcuts.
    rmrpAddKeys();
    
    GfuiScreenActivate(scrHandle);
}
