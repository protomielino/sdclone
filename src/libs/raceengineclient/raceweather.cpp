/***************************************************************************

    file        : raceweather.cpp
    copyright   : (C) 2010 by Xavier Bertaux
    web         : www.speed-dreams.org 
    version     : $Id$

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
    		
    @author	    Xavier Bertaux
    @version	$Id$
*/

/* Changes by wdbee :
   - #define RAIN_VAL_LITTLE, RAIN_VAL_NORMAL, RAIN_VAL_HEAVY in raceweather.h
     to make it usable by robots (moved to interfaces/track by Jean-Philippe).
   - curSurf->kFrictionDry = curSurf->kFriction; 
     Store initial value for calculation of the rain intensity.
*/

#include <tgf.h>

#include <raceman.h>
#include <track.h>

#include "racesituation.h"
#include "raceweather.h"


// Start Weather
void ReStartWeather(void)
{
	tTrack *track = ReInfo->track;

	int clouds = track->clouds;
	int rain = track->rain;
	
	if (rain != TR_RAIN_RANDOM)
	{
		GfLogInfo("Using loaded rain (%d) and clouds (%d) settings\n",
				  rain, (rain != TR_RAIN_NONE) ? TR_CLOUDS_FULL : clouds);
	}
	else
	{
		// Randow clouds (if no rain).
		clouds = 1 + (int)(rand()/(float)RAND_MAX * 7); //random clouds on Track when championship or career
		int result = 1 + (int)(rand()/(float)RAND_MAX * 99); // rain likelyhood, if result < track->rainprob, then it rains

		// Random rain.
		GfLogDebug("Result =  %d - RainProb = %d\n", result, track->rainprob);
		if (result < track->rainprob)
		{
			int resul2 = 1 + (int)(rand()/(float)RAND_MAX * 99); 
			if (resul2 < (track->rainlprob + 1)) // if result2 < little rain likelyhood, rain = little rain
				rain = TR_RAIN_LITTLE;
			else if (resul2 < (track->probrain +1)) // if result2 < medium rain likelyhood, rain = medium rain
				rain = TR_RAIN_MEDIUM;
			else // otherwise, result2 >= medium rain likelyhood, rain = Heavy rain
				rain = TR_RAIN_HEAVY;
		}
		else
			rain = TR_RAIN_NONE;
		
		GfLogInfo("ReStartWeather : Using random rain (%d) and clouds (%d) settings\n",
				  rain, (rain != TR_RAIN_NONE) ? TR_CLOUDS_FULL : clouds);
	}

//rain = TR_RAIN_NONE;
//rain = TR_RAIN_LITTLE;
//rain = TR_RAIN_MEDIUM;
//rain = TR_RAIN_HEAVY;

	track->rain = rain;
	track->clouds = (rain != TR_RAIN_NONE) ? TR_CLOUDS_FULL : clouds; // rain => heavy clouds
	track->water = rain; // ground water = rain ; may be disconnected in the future
	
	ReTrackUpdate();
}

// Update Track Physics (compute kFriction from current "water level" on ground).
void ReTrackUpdate(void)
{
	tTrack *track = ReInfo->track;

	// Get the wet / dry friction coefficients ratio.
	void* hparmTrackConsts =
		GfParmReadFile(TRK_PHYSICS_FILE, GFPARM_RMODE_STD | GFPARM_RMODE_CREAT);
	const tdble kFrictionWetDryRatio = 
		GfParmGetNum(hparmTrackConsts, TRKP_SECT_SURFACES, TRKP_VAL_FRICTIONWDRATIO, (char*)NULL, 0.5f);
	GfParmReleaseHandle(hparmTrackConsts);
	
	// Determine the "wetness" (inside  [0, 1]).
	const tdble wetness = (tdble)track->water / TR_WATER_MUCH;

	GfLogDebug("ReTrackUpdate : water = %d, wetness = %.2f, wet/dry mu = %.4f\n",
			   track->water, wetness, kFrictionWetDryRatio);

	// Set the actual friction for each track surface (only those on the ground).
	GfLogDebug("ReTrackUpdate : kFriction | kRollRes | Surface :\n");
	tTrackSurface *curSurf;
	curSurf = track->surfaces;
	do
	{
		// Linear interpolation of kFriction from dry to wet according to wetness.
		curSurf->kFriction =
			curSurf->kFrictionDry * (1 - wetness)
			+ curSurf->kFrictionDry * kFrictionWetDryRatio * wetness;

		// For the moment, we don't change curSurf->kRollRes (will change in the future).
			
		GfLogDebug("                   %.4f |   %.4f | %s\n",
				   curSurf->kFriction, curSurf->kRollRes, curSurf->material);

		curSurf = curSurf->next;

	} while ( curSurf );
}
