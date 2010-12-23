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

#include <raceman.h>
#include <track.h>

#include "racesituation.h"
#include "raceweather.h"

// Start Weather
void ReStartWeather(void)
{
	int timeofday;
	int	rain;
	int	clouds;
	int	rainprob;
	int problrain;
	int probrain;
	int	resul;
	int resul2;
	tTrack *track = ReInfo->track;

	timeofday = track->timeofday;
	clouds = track->clouds;
	rain = track->rain;
	
	if (rain != TR_RAIN_RANDOM)
	{
		GfLogInfo("Using loaded rain (%d) and clouds (%d) settings\n",
				  rain, (rain != TR_RAIN_NONE) ? TR_CLOUDS_FULL : clouds);
	}
	else
	{
		// Randow clouds (if no rain).
		clouds = 1 + (int)(rand()/(float)RAND_MAX * 7); //random clouds on Track when championship or career
		resul = 1 + (int)(rand()/(float)RAND_MAX * 99); // rain probability, if result < rainprob, then it rains

		// Random rain.
		rainprob = track->rainprob;
		GfLogDebug("Result =  %d - RainProb = %d\n", resul, rainprob);
		if (resul < rainprob)
		{
			problrain = track->rainlprob;
			probrain = track->probrain;
			resul2 = 1 + (int)(rand()/(float)RAND_MAX * 99); 
			if (resul2 < (problrain + 1)) // if result2 < probability little rain, so rain = little rain
				rain = TR_RAIN_LITTLE;
			else if (resul2 < (probrain +1)) // if result2 < probability normal rain, so rain = normal rain
				rain = TR_RAIN_MEDIUM;
			else // result2 > probability normal rain so rain = Heavy rain
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
	track->water = track->rain; // ground water = rain ; should change in the future
	
	ReTrackUpdate();
}

// Update Track Physic
void ReTrackUpdate(void)
{
	tTrack *track = ReInfo->track;
	int rain = track->rain;

	GfLogDebug("ReTrackUpdate : Track timeofday=%d, clouds=%d, rain=%d, water=%d, rainp=%d, rainlp=%d\n",
			   track->timeofday, track->clouds, track->rain, track->water, track->rainprob, track->rainlprob);
	GfLogDebug("ReTrackUpdate : kFriction | kRollRes | Surface :\n");

	tTrackSurface *curSurf;
	curSurf = track->surfaces;
	do
	{
		curSurf->kFrictionDry = curSurf->kFriction; // Store initial value!
		switch (rain)
		{
			case 1:
			{
				curSurf->kFriction     = curSurf->kFriction2;
				curSurf->kRollRes      = curSurf->kRollRes2;
				break;
			}
			case 2:
			{
				curSurf->kFriction     = curSurf->kFriction2 * 0.9f;
				curSurf->kRollRes      = curSurf->kRollRes2;
				break;
			}
			case 3:
			{
				curSurf->kFriction     = curSurf->kFriction2 * 0.7f;
				curSurf->kRollRes      = curSurf->kRollRes2;
				break;
			}
			default:
			{
				curSurf->kFriction     = curSurf->kFriction;
				curSurf->kRollRes      = curSurf->kRollRes;
				break;
			}
		}							
		
		GfLogDebug("                   %.4f |   %.4f | %s\n",
				   curSurf->kFriction, curSurf->kRollRes, curSurf->material);

		curSurf = curSurf->next;

	} while ( curSurf );
}
