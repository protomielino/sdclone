/***************************************************************************

    file                 : raceparamsmenu.cpp
    created              : Thu May  2 22:02:51 CEST 2002
    copyright            : (C) 2001 by Eric Espie
    email                : eric.espie@torcs.org
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
    		This is the race options menu.
    @ingroup	racemantools
    @author	<a href=mailto:eric.espie@torcs.org>Eric Espie</a>
    @version	$Id$
*/

#include <cstdlib>
#include <cstdio>
#include <cstring>
#ifdef WIN32
#include <windows.h>
#endif

#include <portability.h>
#include <tgfclient.h>
#include <track.h>
#include <car.h>
#include <raceman.h>
#include <robot.h>
#include <graphic.h>

#include "racescreens.h"


// Constants.
static const char *DispModeValues[] = { RM_VAL_VISIBLE, RM_VAL_INVISIBLE};

static const char *TimeOfDayValues[] = RM_VALS_TIME;
static const int NTimeOfDayValues = sizeof( TimeOfDayValues ) / sizeof( const char* );

static const char* CloudsValues[] = RM_VALS_CLOUDS;
static const int NCloudsValues = sizeof( CloudsValues ) / sizeof( const char* );

static const char *RainValues[] = RM_VALS_RAIN;
static const int NRainValues = sizeof( RainValues ) / sizeof( const char* );

// Global variables.
static void		*scrHandle;
static tRmRaceParam	*rp;

// Menu control ids
static int		rmrpDistEditId;
static int		rmrpLapsEditId;
static int		rmrpSessionTimeEditId;
static int		rmrpDispModeEditId;
static int		rmrpCloudsEditId, rmrpCloudsLeftArrowId, rmrpCloudsRightArrowId;
static int		rmrpTimeOfDayEditId;
static int		rmrpRainEditId;

// Race params
static int		rmrpDistance;
static int		rmrpLaps;
static int		rmrpSessionTime;
static int		rmrpDispMode;
static int		rmrpClouds;
static int		rmrpTimeOfDay;
static int		rmrpRain;

static int		rmrpFeatures;
static bool		rmrpIsSkyDomeEnabled;


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

    val = GfuiEditboxGetString(scrHandle, rmrpDistEditId);
    rmrpDistance = strtol(val, (char **)NULL, 0);
    if (rmrpDistance == 0) {
		strcpy(buf, "---");
    } else {
		sprintf(buf, "%d", rmrpDistance);
		rmrpLaps = 0;
		GfuiEditboxSetString(scrHandle, rmrpLapsEditId, "---");
    }
    GfuiEditboxSetString(scrHandle, rmrpDistEditId, buf);
}

static void
rmrpUpdLaps(void * /* dummy */)
{
    char	buf[32];
    char	*val;

    val = GfuiEditboxGetString(scrHandle, rmrpLapsEditId);
    rmrpLaps = strtol(val, (char **)NULL, 0);
    if (rmrpLaps == 0) {
		strcpy(buf, "---");
    } else {
		sprintf(buf, "%d", rmrpLaps);
		rmrpDistance = 0;
		GfuiEditboxSetString(scrHandle, rmrpDistEditId, "---");
    }
    GfuiEditboxSetString(scrHandle, rmrpLapsEditId, buf);
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

    val = GfuiEditboxGetString(scrHandle, rmrpSessionTimeEditId);
    
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
    GfuiEditboxSetString(scrHandle, rmrpSessionTimeEditId, buf);
}

static void
rmChangeDisplayMode(void * /* dummy */)
{
    rmrpDispMode = 1 - rmrpDispMode;
    GfuiLabelSetText(scrHandle, rmrpDispModeEditId, DispModeValues[rmrpDispMode]);
}

static void
rmChangeTimeOfDay(void *vp)
{
	const long delta = (long)vp;
	rmrpTimeOfDay = (rmrpTimeOfDay + NTimeOfDayValues + delta) % NTimeOfDayValues;
	GfuiLabelSetText(scrHandle, rmrpTimeOfDayEditId, TimeOfDayValues[rmrpTimeOfDay]);
}

static void rmChangeRain(void *vp);

static void
rmChangeClouds(void *vp)
{
    const long delta = (long)vp;
    rmrpClouds = (rmrpClouds + NCloudsValues + delta) % NCloudsValues;
    GfuiLabelSetText(scrHandle, rmrpCloudsEditId, CloudsValues[rmrpClouds]);

    if ((rp->confMask & RM_CONF_RAIN_FALL) && (rmrpFeatures & RM_FEATURE_WETTRACK))
	{
		// Make rain level compatible if needed.
		if (rmrpClouds < NCloudsValues - 1) // No heavy clouds => no rain
		{
			rmrpRain = TR_RAIN_NONE;
			GfuiLabelSetText(scrHandle, rmrpRainEditId, RainValues[rmrpRain]);
		}
	}
}

static void
rmChangeRain(void *vp)
{
	const long delta = (long)vp;
	rmrpRain = (rmrpRain + NRainValues + delta) % NRainValues;
	GfuiLabelSetText(scrHandle, rmrpRainEditId, RainValues[rmrpRain]);

    if ((rp->confMask & RM_CONF_CLOUD_COVER) && rmrpIsSkyDomeEnabled)
	{
		// Make clouds state compatible if needed.
		int cloudsComboVisibility;
		if (rmrpRain == TR_RAIN_RANDOM) // Random rain => Random clouds.
		{
			cloudsComboVisibility = GFUI_INVISIBLE;
			GfuiLabelSetText(scrHandle, rmrpCloudsEditId, "random");
		}
		else
		{
			cloudsComboVisibility = GFUI_VISIBLE;
			if (rmrpRain == TR_RAIN_NONE)
				rmrpClouds = 0; // No rain => no clouds by default.
			else
				rmrpClouds = NCloudsValues - 1; // Rain => Heavy clouds.
			GfuiLabelSetText(scrHandle, rmrpCloudsEditId, CloudsValues[rmrpClouds]);
		}

		// Show / hide clouds combo arrow buttons (random rain => no sky choice).
		GfuiVisibilitySet(scrHandle, rmrpCloudsLeftArrowId, cloudsComboVisibility);
		GfuiVisibilitySet(scrHandle, rmrpCloudsRightArrowId, cloudsComboVisibility);
	}
}

static void
rmrpValidate(void * /* dummy */)
{
    // Force current edit to loose focus (if one has it) and update associated variable.
    GfuiUnSelectCurrent();

    if (rp->confMask & RM_CONF_RACE_LEN)
	{
		GfParmSetNum(rp->param, rp->title, RM_ATTR_DISTANCE, "km", rmrpDistance);
		GfParmSetNum(rp->param, rp->title, RM_ATTR_LAPS, (char*)NULL, rmrpLaps);
		GfParmSetNum(rp->param, rp->title, RM_ATTR_SESSIONTIME, "s", (tdble)rmrpSessionTime);
    }
	
    if (rp->confMask & RM_CONF_TIME_OF_DAY)
	{
		GfParmSetStr(rp->param, rp->title, RM_ATTR_TIME_OF_DAY, TimeOfDayValues[rmrpTimeOfDay]);
	}
	
    if (rp->confMask & RM_CONF_CLOUD_COVER)
	{
		GfParmSetStr(rp->param, rp->title, RM_ATTR_CLOUDS, CloudsValues[rmrpClouds]);
	}
	
    if ((rp->confMask & RM_CONF_RAIN_FALL) && (rmrpFeatures & RM_FEATURE_WETTRACK))
	{
		GfParmSetStr(rp->param, rp->title, RM_ATTR_RAIN, RainValues[rmrpRain]);
	}

	if (rp->confMask & RM_CONF_DISP_MODE)
	{
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
RmRaceParamsMenu(void *vrp)
{
    char buf[256];

	GfLogTrace("Entering Race Params menu\n");

	rp = (tRmRaceParam*)vrp;

	// Get race features.
    rmrpFeatures = RmGetFeaturesList(rp->param);

	// Check if SkyDome is enabled
	snprintf(buf, sizeof(buf), "%s%s", GetLocalDir(), GR_PARAM_FILE);
	void *grHandle = GfParmReadFile(buf, GFPARM_RMODE_STD);
	rmrpIsSkyDomeEnabled =
		(int)GfParmGetNum(grHandle, GR_SCT_GRAPHIC, GR_ATT_SKYDOMEDISTANCE, NULL, 0) != 0;
	GfParmReleaseHandle(grHandle);
	
    // Create screen, load menu XML descriptor and create static controls.
    scrHandle = GfuiScreenCreateEx((float*)NULL, NULL, NULL, NULL, (tfuiCallback)NULL, 1);   
    void *menuXMLDescHdle = LoadMenuXML("raceparamsmenu.xml");
    CreateStaticControls(menuXMLDescHdle,scrHandle);

    // Create variable title label.
    int titleId = CreateLabelControl(scrHandle,menuXMLDescHdle,"title");
    sprintf(buf, "%s Options", rp->title);
    GfuiLabelSetText(scrHandle,titleId,buf);
    
    if (rp->confMask & RM_CONF_RACE_LEN) 
    {
		// Create Race distance label.
		CreateLabelControl(scrHandle,menuXMLDescHdle,"racedistancelabel");
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
		
		rmrpDistEditId = CreateEditControl(scrHandle,menuXMLDescHdle,"racedistanceedit",NULL,NULL,rmrpUpdDist);
		GfuiEditboxSetString(scrHandle,rmrpDistEditId,buf);
		
		// Create Laps label.
		CreateLabelControl(scrHandle,menuXMLDescHdle,"lapslabel");
		
		// Create and initialize Laps edit.
		if (rmrpLaps == 0) 
		{
			strcpy(buf, "---");
		} 
		else 
		{
			sprintf(buf, "%d", rmrpLaps);
		}
		
		rmrpLapsEditId = CreateEditControl(scrHandle,menuXMLDescHdle,"lapsedit",NULL,NULL,rmrpUpdLaps);
		GfuiEditboxSetString(scrHandle,rmrpLapsEditId,buf);
		
		if (rmrpFeatures & RM_FEATURE_TIMEDSESSION)
		{
			// Create Session time label.
			CreateLabelControl(scrHandle,menuXMLDescHdle,"sessiontimelabel");
			
			// Create and initialize Session time edit.
			if (rmrpSessionTime <= 0) 
			{
				strcpy(buf, "---");
			}
			else 
			{
				sprintf(buf, "%d:%02d:%02d", (int)floor((float) rmrpSessionTime / 3600.0f ),
						(int)floor( (float)rmrpSessionTime / 60.0f ) % 60,
						(int)floor( (float)rmrpSessionTime ) % 60 );
			}
			
			rmrpSessionTimeEditId =
				CreateEditControl(scrHandle,menuXMLDescHdle,"sessiontimeedit",NULL,NULL,rmrpUpdSessionTime);
			GfuiEditboxSetString(scrHandle,rmrpSessionTimeEditId,buf);
		}
		else
		{
			rmrpSessionTime = 0;
		}
    }

    // Create and initialize Time of day combo box (2 arrow buttons and a variable label).
	if (rp->confMask & RM_CONF_TIME_OF_DAY)
	{
		if (rmrpIsSkyDomeEnabled)
		{
			rmrpTimeOfDay = 0;
			const char* pszTimeOfDay =
				GfParmGetStr(rp->param, rp->title, RM_ATTR_TIME_OF_DAY, RM_VAL_TIME_AFTERNOON);
			for (int i = 0; i < NTimeOfDayValues; i++)
				if (!strcmp(pszTimeOfDay, TimeOfDayValues[i]))
				{
					rmrpTimeOfDay = i;
					break;
				}
		
			// Create Time of day label.
			CreateLabelControl(scrHandle,menuXMLDescHdle,"timeofdaylabel");

			// Create and initialize Time of day combo-box-like control.
			CreateButtonControl(scrHandle,menuXMLDescHdle,"timeofdayleftarrow",(void*)-1, rmChangeTimeOfDay);
			CreateButtonControl(scrHandle,menuXMLDescHdle,"timeofdayrightarrow",(void*)1, rmChangeTimeOfDay);
			
			rmrpTimeOfDayEditId = CreateLabelControl(scrHandle,menuXMLDescHdle,"timeofdayedit");
			GfuiLabelSetText(scrHandle,rmrpTimeOfDayEditId,TimeOfDayValues[rmrpTimeOfDay]);
		}
		else
		{
			rmrpTimeOfDay = 3; // Afternoon (but normally not taken into account).
		}
    }
	
    if (rp->confMask & RM_CONF_CLOUD_COVER)
	{
		if (rmrpIsSkyDomeEnabled)
		{
			// Create and initialize Clouds combo box (2 arrow buttons and a variable label).
			rmrpClouds = 0;
			const char* pszClouds =
				GfParmGetStr(rp->param, rp->title, RM_ATTR_CLOUDS, RM_VAL_CLOUDS_NONE);
			for (int i = 0; i < NCloudsValues; i++)
				if (!strcmp(pszClouds, CloudsValues[i]))
				{
					rmrpClouds = i;
					break;
				}
			
			// Create Cloud cover label.
			CreateLabelControl(scrHandle,menuXMLDescHdle,"cloudslabel");

			// Create and initialize Cloud cover combo-box-like control.
			rmrpCloudsLeftArrowId =
				CreateButtonControl(scrHandle,menuXMLDescHdle,"cloudsleftarrow",(void*)-1, rmChangeClouds);
			rmrpCloudsRightArrowId =
				CreateButtonControl(scrHandle,menuXMLDescHdle,"cloudsrightarrow",(void*)1, rmChangeClouds);
			
			rmrpCloudsEditId = CreateLabelControl(scrHandle,menuXMLDescHdle,"cloudsedit");
			GfuiLabelSetText(scrHandle,rmrpCloudsEditId,CloudsValues[rmrpClouds]);
		}
		else
		{
			rmrpClouds = 0;
		}
	}
	
	if ((rp->confMask & RM_CONF_RAIN_FALL) && (rmrpFeatures & RM_FEATURE_WETTRACK))
	{
		// Create and initialize Rain combo box (2 arrow buttons and a variable label).
		rmrpRain = 0;
		const char* pszRain =
			GfParmGetStr(rp->param, rp->title, RM_ATTR_RAIN, RM_VAL_RAIN_NONE);
		for (int i = 0; i < NRainValues; i++)
			if (!strcmp(pszRain, RainValues[i]))
			{
				rmrpRain = i;
				break;
			}
			
		// Create Rain label.
		CreateLabelControl(scrHandle,menuXMLDescHdle,"rainlabel");

		// Create and initialize Rain combo-box-like control.
		CreateButtonControl(scrHandle,menuXMLDescHdle,"rainleftarrow",(void*)-1, rmChangeRain);
		CreateButtonControl(scrHandle,menuXMLDescHdle,"rainrightarrow",(void*)1, rmChangeRain);
			
		rmrpRainEditId = CreateLabelControl(scrHandle,menuXMLDescHdle,"rainedit");
		GfuiLabelSetText(scrHandle,rmrpRainEditId,RainValues[rmrpRain]);
			
		rmChangeRain(0); // Make cloud cover settings compatible if needed.
	}
	
    if (rp->confMask & RM_CONF_DISP_MODE) 
    {
		if (!strcmp(GfParmGetStr(rp->param, rp->title, RM_ATTR_DISPMODE, RM_VAL_VISIBLE), RM_VAL_INVISIBLE)) 
			rmrpDispMode = 1;
		else 
			rmrpDispMode = 0;

		// Create Display mode label.
		CreateLabelControl(scrHandle,menuXMLDescHdle,"displaylabel");

		// Create and initialize Display mode combo-box-like control.
		CreateButtonControl(scrHandle,menuXMLDescHdle,"displayleftarrow",(void*)0, rmChangeDisplayMode);
		CreateButtonControl(scrHandle,menuXMLDescHdle,"displayrightarrow",(void*)1, rmChangeDisplayMode);
		rmrpDispModeEditId = CreateLabelControl(scrHandle,menuXMLDescHdle,"displayedit");
		GfuiLabelSetText(scrHandle,rmrpDispModeEditId,DispModeValues[rmrpDispMode]);
    }
	
    // Create Accept and Cancel buttons
    CreateButtonControl(scrHandle,menuXMLDescHdle,"nextbutton",NULL,rmrpValidate);
    CreateButtonControl(scrHandle,menuXMLDescHdle,"previousbutton",rp->prevScreen,rmrpDeactivate);
    
    // Close menu XML descriptor.
    GfParmReleaseHandle(menuXMLDescHdle);
    
    // Register keyboard shortcuts.
    rmrpAddKeys();
    
    GfuiScreenActivate(scrHandle);
}
