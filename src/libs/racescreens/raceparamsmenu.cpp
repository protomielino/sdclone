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

#include <race.h>

#include "racescreens.h"


// Constants.
static const char *DispModeValues[GfRace::nDisplayModeNumber] =
	{ RM_VAL_VISIBLE, RM_VAL_INVISIBLE};
static const char *TimeOfDayValues[GfRace::nTimeSpecNumber] = RM_VALS_TIME;
static const char* CloudsValues[GfRace::nCloudsSpecNumber] = RM_VALS_CLOUDS;
static const char *RainValues[GfRace::nRainSpecNumber] = RM_VALS_RAIN;

// Global variables.
static void		*ScrHandle;
static tRmRaceParam	*MenuData;

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
static GfRace::EDisplayMode		rmrpDispMode;
static GfRace::ECloudsSpec		rmrpClouds;
static GfRace::ETimeOfDaySpec	rmrpTimeOfDay;
static GfRace::ERainSpec		rmrpRain;

static int		rmrpFeatures;
static bool		rmrpIsSkyDomeEnabled;


static void
rmrpDeactivate(void *screen)
{
    GfuiScreenRelease(ScrHandle);
    
    if (screen) {
		GfuiScreenActivate(screen);
    }
}

static void
rmrpUpdDist(void * /* dummy */)
{
    char	buf[32];
    char	*val;

    val = GfuiEditboxGetString(ScrHandle, rmrpDistEditId);
    rmrpDistance = strtol(val, (char **)NULL, 0);
    if (rmrpDistance == 0) {
		strcpy(buf, "---");
    } else {
		sprintf(buf, "%d", rmrpDistance);
		rmrpLaps = 0;
		GfuiEditboxSetString(ScrHandle, rmrpLapsEditId, "---");
    }
    GfuiEditboxSetString(ScrHandle, rmrpDistEditId, buf);
}

static void
rmrpUpdLaps(void * /* dummy */)
{
    char	buf[32];
    char	*val;

    val = GfuiEditboxGetString(ScrHandle, rmrpLapsEditId);
    rmrpLaps = strtol(val, (char **)NULL, 0);
    if (rmrpLaps == 0) {
		strcpy(buf, "---");
    } else {
		sprintf(buf, "%d", rmrpLaps);
		rmrpDistance = 0;
		GfuiEditboxSetString(ScrHandle, rmrpDistEditId, "---");
    }
    GfuiEditboxSetString(ScrHandle, rmrpLapsEditId, buf);
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

    val = GfuiEditboxGetString(ScrHandle, rmrpSessionTimeEditId);
    
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
    GfuiEditboxSetString(ScrHandle, rmrpSessionTimeEditId, buf);
}

static void
rmChangeDisplayMode(void *vp)
{
    const long delta = (int)(long)vp;
    rmrpDispMode = 
		(GfRace::EDisplayMode)
		((rmrpDispMode + GfRace::nDisplayModeNumber + delta) % GfRace::nDisplayModeNumber);
    GfuiLabelSetText(ScrHandle, rmrpDispModeEditId, DispModeValues[rmrpDispMode]);
}

static void
rmChangeTimeOfDay(void *vp)
{
	const long delta = (int)(long)vp;
	rmrpTimeOfDay =
		(GfRace::ETimeOfDaySpec)
		((rmrpTimeOfDay + GfRace::nTimeSpecNumber + delta) % GfRace::nTimeSpecNumber);
	GfuiLabelSetText(ScrHandle, rmrpTimeOfDayEditId, TimeOfDayValues[rmrpTimeOfDay]);
}

static void rmChangeRain(void *vp);

static void
rmChangeClouds(void *vp)
{
    const long delta = (int)(long)vp;
    rmrpClouds =
		(GfRace::ECloudsSpec)
		((rmrpClouds + GfRace::nCloudsSpecNumber + delta) % GfRace::nCloudsSpecNumber);
    GfuiLabelSetText(ScrHandle, rmrpCloudsEditId, CloudsValues[rmrpClouds]);

    if ((MenuData->confMask & RM_CONF_RAIN_FALL) && (rmrpFeatures & RM_FEATURE_WETTRACK))
	{
		// Make rain level compatible if needed.
		if (rmrpClouds != GfRace::eCloudsFull) // No heavy clouds => no rain
		{
			rmrpRain = GfRace::eRainNone;
			GfuiLabelSetText(ScrHandle, rmrpRainEditId, RainValues[rmrpRain]);
		}
	}
}

static void
rmChangeRain(void *vp)
{
	const long delta = (int)(long)vp;
	rmrpRain =
		(GfRace::ERainSpec)
		((rmrpRain + GfRace::nRainSpecNumber + delta) % GfRace::nRainSpecNumber);
	GfuiLabelSetText(ScrHandle, rmrpRainEditId, RainValues[rmrpRain]);

    if ((MenuData->confMask & RM_CONF_CLOUD_COVER) && rmrpIsSkyDomeEnabled)
	{
		// Make clouds state compatible if needed.
		int cloudsComboVisibility;
		if (rmrpRain == GfRace::eRainRandom) // Random rain => Random clouds.
		{
			cloudsComboVisibility = GFUI_INVISIBLE;
			GfuiLabelSetText(ScrHandle, rmrpCloudsEditId, "random");
		}
		else
		{
			cloudsComboVisibility = GFUI_VISIBLE;
			if (rmrpRain == GfRace::eRainNone)
				rmrpClouds = GfRace::eCloudsNone; // No rain => no clouds by default.
			else
				rmrpClouds = GfRace::eCloudsFull; // Rain => Heavy clouds.
			GfuiLabelSetText(ScrHandle, rmrpCloudsEditId, CloudsValues[rmrpClouds]);
		}

		// Show / hide clouds combo arrow buttons (random rain => no sky choice).
		GfuiVisibilitySet(ScrHandle, rmrpCloudsLeftArrowId, cloudsComboVisibility);
		GfuiVisibilitySet(ScrHandle, rmrpCloudsRightArrowId, cloudsComboVisibility);
	}
}

static void
rmrpValidate(void * /* dummy */)
{
    // Force current edit to loose focus (if one has it) and update associated variable.
    GfuiUnSelectCurrent();

	GfRace::Parameters* pRaceParams = MenuData->pRace->getParameters();
	
    if (MenuData->confMask & RM_CONF_RACE_LEN)
	{
		pRaceParams->nDistance = rmrpDistance;
		pRaceParams->nLaps = rmrpLaps;
		pRaceParams->nDuration = rmrpSessionTime;
    }
	
    if (MenuData->confMask & RM_CONF_TIME_OF_DAY)
	{
		pRaceParams->eTimeOfDaySpec = (GfRace::ETimeOfDaySpec)rmrpTimeOfDay;
	}
	
    if (MenuData->confMask & RM_CONF_CLOUD_COVER)
	{
		pRaceParams->eCloudsSpec = (GfRace::ECloudsSpec)rmrpClouds;
	}
	
    if ((MenuData->confMask & RM_CONF_RAIN_FALL) && (rmrpFeatures & RM_FEATURE_WETTRACK))
	{
		pRaceParams->eRainSpec = (GfRace::ERainSpec)rmrpRain;
	}

	if (MenuData->confMask & RM_CONF_DISP_MODE)
	{
		pRaceParams->eDisplayMode = (GfRace::EDisplayMode)rmrpDispMode;
	}

    rmrpDeactivate(MenuData->nextScreen);
}

static void
rmrpAddKeys(void)
{
    GfuiAddKey(ScrHandle, GFUIK_RETURN, "Accept", NULL, rmrpValidate, NULL);
    GfuiAddKey(ScrHandle, GFUIK_ESCAPE, "Cancel", MenuData->prevScreen, rmrpDeactivate, NULL);
    GfuiAddKey(ScrHandle, GFUIK_F1, "Help", ScrHandle, GfuiHelpScreen, NULL);
    GfuiAddKey(ScrHandle, GFUIK_F12, "Screen-Shot", NULL, GfuiScreenShot, NULL);
}

void
RmRaceParamsMenu(void *vrp)
{
    char buf[256];

	GfLogTrace("Entering Race Params menu\n");

	MenuData = (tRmRaceParam*)vrp;

	GfRace::Parameters* pRaceParams = MenuData->pRace->getParameters();
	if (!pRaceParams)
		return;
	
	// Get race features.
    rmrpFeatures = MenuData->pRace->getSupportedFeatures();

	// Check if SkyDome is enabled
	snprintf(buf, sizeof(buf), "%s%s", GfLocalDir(), GR_PARAM_FILE);
	void *grHandle = GfParmReadFile(buf, GFPARM_RMODE_STD);
	rmrpIsSkyDomeEnabled =
		(int)GfParmGetNum(grHandle, GR_SCT_GRAPHIC, GR_ATT_SKYDOMEDISTANCE, NULL, 0) != 0;
	GfParmReleaseHandle(grHandle);
	
    // Create screen, load menu XML descriptor and create static controls.
    ScrHandle = GfuiScreenCreateEx((float*)NULL, NULL, NULL, NULL, (tfuiCallback)NULL, 1);   
    void *menuXMLDescHdle = LoadMenuXML("raceparamsmenu.xml");
    CreateStaticControls(menuXMLDescHdle,ScrHandle);

    // Create variable title label.
    int titleId = CreateLabelControl(ScrHandle,menuXMLDescHdle,"title");
	sprintf(buf, "%s Options", MenuData->pRace->getSessionName().c_str());
    GfuiLabelSetText(ScrHandle,titleId,buf);
    
    if (MenuData->confMask & RM_CONF_RACE_LEN) 
    {
		// Create Race distance label.
		CreateLabelControl(ScrHandle,menuXMLDescHdle,"racedistancelabel");
		rmrpDistance = pRaceParams->nDistance;
		
		// Create and initialize Race distance edit.
		rmrpSessionTime = pRaceParams->nDuration;
		if (rmrpSessionTime > 0 && ( rmrpFeatures & RM_FEATURE_TIMEDSESSION ) == 0 )
			rmrpDistance += rmrpSessionTime / 30;
		if (rmrpDistance == 0) 
		{
			strcpy(buf, "---");
			rmrpLaps = pRaceParams->nLaps;
			if (rmrpSessionTime > 0 && ( rmrpFeatures & RM_FEATURE_TIMEDSESSION ) == 0 )
				rmrpLaps += (int)floor( (tdble)rmrpSessionTime / 1.5f + 0.5f );
		} 
		else 
		{
			sprintf(buf, "%d", rmrpDistance);
			rmrpLaps = 0;
		}
		
		rmrpDistEditId = CreateEditControl(ScrHandle, menuXMLDescHdle, "racedistanceedit",
										   NULL, NULL, rmrpUpdDist);
		GfuiEditboxSetString(ScrHandle,rmrpDistEditId,buf);
		
		// Create Laps label.
		CreateLabelControl(ScrHandle,menuXMLDescHdle,"lapslabel");
		
		// Create and initialize Laps edit.
		if (rmrpLaps == 0) 
		{
			strcpy(buf, "---");
		} 
		else 
		{
			sprintf(buf, "%d", rmrpLaps);
		}
		
		rmrpLapsEditId = CreateEditControl(ScrHandle, menuXMLDescHdle, "lapsedit",
										   NULL, NULL, rmrpUpdLaps);
		GfuiEditboxSetString(ScrHandle,rmrpLapsEditId,buf);
		
		if (rmrpFeatures & RM_FEATURE_TIMEDSESSION)
		{
			// Create Session time label.
			CreateLabelControl(ScrHandle,menuXMLDescHdle,"sessiontimelabel");
			
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
				CreateEditControl(ScrHandle, menuXMLDescHdle, "sessiontimeedit",
								  NULL, NULL, rmrpUpdSessionTime);
			GfuiEditboxSetString(ScrHandle,rmrpSessionTimeEditId,buf);
		}
		else
		{
			rmrpSessionTime = 0;
		}
    }

    // Create and initialize Time of day combo box (2 arrow buttons and a variable label).
	if (MenuData->confMask & RM_CONF_TIME_OF_DAY)
	{
		if (rmrpIsSkyDomeEnabled)
		{
			rmrpTimeOfDay = pRaceParams->eTimeOfDaySpec;
		
			// Create Time of day label.
			CreateLabelControl(ScrHandle,menuXMLDescHdle,"timeofdaylabel");

			// Create and initialize Time of day combo-box-like control.
			CreateButtonControl(ScrHandle, menuXMLDescHdle, "timeofdayleftarrow",
								(void*)-1, rmChangeTimeOfDay);
			CreateButtonControl(ScrHandle, menuXMLDescHdle, "timeofdayrightarrow",
								(void*)1, rmChangeTimeOfDay);
			
			rmrpTimeOfDayEditId = CreateLabelControl(ScrHandle,menuXMLDescHdle,"timeofdayedit");
			GfuiLabelSetText(ScrHandle, rmrpTimeOfDayEditId, TimeOfDayValues[rmrpTimeOfDay]);
		}
		else
		{
			rmrpTimeOfDay = GfRace::eTimeAfternoon; // Normally not taken into account.
		}
    }
	
    if (MenuData->confMask & RM_CONF_CLOUD_COVER)
	{
		if (rmrpIsSkyDomeEnabled)
		{
			// Create and initialize Clouds combo box (2 arrow buttons and a variable label).
			rmrpClouds = pRaceParams->eCloudsSpec;
			
			// Create Cloud cover label.
			CreateLabelControl(ScrHandle,menuXMLDescHdle,"cloudslabel");

			// Create and initialize Cloud cover combo-box-like control.
			rmrpCloudsLeftArrowId =
				CreateButtonControl(ScrHandle, menuXMLDescHdle, "cloudsleftarrow",
									(void*)-1, rmChangeClouds);
			rmrpCloudsRightArrowId =
				CreateButtonControl(ScrHandle, menuXMLDescHdle, "cloudsrightarrow",
									(void*)+1, rmChangeClouds);
			
			rmrpCloudsEditId = CreateLabelControl(ScrHandle,menuXMLDescHdle,"cloudsedit");
			GfuiLabelSetText(ScrHandle,rmrpCloudsEditId,CloudsValues[rmrpClouds]);
		}
		else
		{
			rmrpClouds = GfRace::eCloudsNone;
		}
	}
	
	if ((MenuData->confMask & RM_CONF_RAIN_FALL) && (rmrpFeatures & RM_FEATURE_WETTRACK))
	{
		// Create and initialize Rain combo box (2 arrow buttons and a variable label).
		rmrpRain = pRaceParams->eRainSpec;
			
		// Create Rain label.
		CreateLabelControl(ScrHandle,menuXMLDescHdle,"rainlabel");

		// Create and initialize Rain combo-box-like control.
		CreateButtonControl(ScrHandle, menuXMLDescHdle, "rainleftarrow",
							(void*)-1, rmChangeRain);
		CreateButtonControl(ScrHandle, menuXMLDescHdle, "rainrightarrow",
							(void*)1, rmChangeRain);
			
		rmrpRainEditId = CreateLabelControl(ScrHandle,menuXMLDescHdle,"rainedit");
		GfuiLabelSetText(ScrHandle,rmrpRainEditId,RainValues[rmrpRain]);
			
		rmChangeRain(0); // Make cloud cover settings compatible if needed.
	}
	
    if (MenuData->confMask & RM_CONF_DISP_MODE) 
    {
		rmrpDispMode = pRaceParams->eDisplayMode;

		// Create Display mode label.
		CreateLabelControl(ScrHandle, menuXMLDescHdle, "displaylabel");

		// Create and initialize Display mode combo-box-like control.
		CreateButtonControl(ScrHandle, menuXMLDescHdle, "displayleftarrow",
							(void*)-1, rmChangeDisplayMode);
		CreateButtonControl(ScrHandle, menuXMLDescHdle, "displayrightarrow",
							(void*)+1, rmChangeDisplayMode);
		rmrpDispModeEditId = CreateLabelControl(ScrHandle, menuXMLDescHdle, "displayedit");
		GfuiLabelSetText(ScrHandle, rmrpDispModeEditId, DispModeValues[rmrpDispMode]);
    }
	
    // Create Accept and Cancel buttons
    CreateButtonControl(ScrHandle, menuXMLDescHdle, "nextbutton", NULL, rmrpValidate);
    CreateButtonControl(ScrHandle, menuXMLDescHdle, "previousbutton",
						MenuData->prevScreen, rmrpDeactivate);
    
    // Close menu XML descriptor.
    GfParmReleaseHandle(menuXMLDescHdle);
    
    // Register keyboard shortcuts.
    rmrpAddKeys();
    
    GfuiScreenActivate(ScrHandle);
}
