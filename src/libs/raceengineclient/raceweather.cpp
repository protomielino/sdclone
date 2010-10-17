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

#include <ctime>

#include <raceman.h>

#include "racesituation.h"
#include "raceweather.h"


#define RAIN_VAL_LITTLE 1
#define RAIN_VAL_NORMAL 2
#define RAIN_VAL_HEAVY 3


// Start Weather
void ReStartWeather(void)
{
	int	cloud;
	int Timeday;
	int	rain;
	int problrain;
	int probrain;
	int	rainbool;
	int	resul;
	int resul2;
	tTrack *track = ReInfo->track;

	 // TODO: Move this inside TGF initialization, or so (in order to have it done only once).
	srand((unsigned)time(NULL));
	
	cloud = track->weather;
	Timeday = track->Timeday;

	if (Timeday > 0)
	{
		GfLogDebug("ReStartWeather : Using loaded rain params\n");
		rainbool = track->weather;
		rainbool = rainbool - 4;
	}
	else
	{
		GfLogDebug("ReStartWeather : Using random rain params\n");
		rain = track->rainprob;
		cloud = 1 + (int)(rand()/(float)RAND_MAX * 7); //random cloud on Track when championship or career
		track->weather = cloud; // cloud = random cloud

		resul = 1 + (int)(rand()/(float)RAND_MAX * 99); // probability rain, if result < rain, so it rain
		//GfLogDebug("Result =  %d - Rain = %d\n", resul, rain);
		if (resul < rain)
		{
			problrain = track->rainlprob;
			probrain = track->probrain;
			track->weather = 8; // it rain so cloud coverage selected
			resul2 = 1 + (int)(rand()/(float)RAND_MAX * 99); 
			if (resul2 < (problrain + 1)) // if result2 < probability little rain, so rain = little rain
			{
				rainbool = RAIN_VAL_LITTLE;
				//GfLogDebug("RainBool = %d\n", rainbool);
			}
			else if (resul2 < (probrain +1)) // if result2 < probability normal rain, so rain = normal rain
			{
				rainbool = RAIN_VAL_NORMAL;
				//GfLogDebug("RainBool = %d\n", rainbool);
			}
			else // result2 > probability normal rain so rain = Heavy rain
			{
				rainbool = RAIN_VAL_HEAVY;
   			//GfLogDebug("RainBool = %d\n", rainbool);
			}
		}
		else
			rainbool = 0;
			
	}

	if (rainbool > 0)
		track->Rain = rainbool;
	else
		track->Rain = 0;
	
	ReTrackUpdate();
}

// Update Track Physic
void ReTrackUpdate(void)
{
	tTrack *track = ReInfo->track;
	int rain = track->Rain;

	GfLogDebug("ReTrackUpdate : Track timeday=%d, weather=%d, rain=%d, rainp=%d, rainlp=%d\n",
			   track->Timeday, track->weather, track->Rain, track->rainprob, track->rainlprob);
	GfLogDebug("ReTrackUpdate : kFriction, kRollRes for each track surface :\n");

	tTrackSurface *curSurf;
	curSurf = track->surfaces;
	do
	{
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
		
		GfLogDebug("                   %.4f, %.4f   %s\n",
				   curSurf->kFriction, curSurf->kRollRes, curSurf->material);

		curSurf = curSurf->next;

	} while ( curSurf );
}
