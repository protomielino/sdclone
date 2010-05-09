#include <raceman.h>

#include <time.h>
#include "raceengine.h"

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

	srand((unsigned)time(NULL));
	cloud = track->weather;
	Timeday = track->Timeday;

	if (Timeday > 0)
	{
		rainbool = track->weather;
		rainbool = rainbool - 4;
	}
	else
	{
		rain = track->rainprob;
		cloud = 1 + (int)(rand()/(float)RAND_MAX * 7); //random cloud on Track when championship or career
		track->weather = cloud; // cloud = random cloud

		resul = 1 + (int)(rand()/(float)RAND_MAX * 99); // probability rain, if result < rain, so it rain
		printf("Result =  %d - Rain = %d\n", resul, rain);
		if (resul < rain)
		{
			problrain = track->rainlprob;
			probrain = track->probrain;
			track->weather = 8; // it rain so cloud coverage selected
			resul2 = 1 + (int)(rand()/(float)RAND_MAX * 99); 
			if (resul2 < (problrain + 1)) // if result2 < probability little rain, so rain = little rain
			{
				rainbool = RAIN_VAL_LITTLE;
				printf("RainBool = %d\n", rainbool);
			}
			else if (resul2 < (probrain +1)) // if result2 < probability normal rain, so rain = normal rain
			{
				rainbool = RAIN_VAL_NORMAL;
				printf("RainBool = %d\n", rainbool);
			}
			else // result2 > probability normal rain so rain = Heavy rain
			{
				rainbool = RAIN_VAL_HEAVY;
				printf("RainBool = %d\n", rainbool);
			}
		}
		else
			rainbool = 0;
			
	}

	if (rainbool > 0)
		{
			track->Rain = rainbool;
	    	ReTrackUpdate();
		}
	else
		track->Rain = 0;
}

// Update Track Physic
void ReTrackUpdate(void)
{
	int rain;
	tTrack *track = ReInfo->track;
	tTrackSurface *curSurf;

	rain = track->Rain;
	curSurf = track->surfaces;
	do
	{
		switch (rain)
		{
			case 1:
			{
				curSurf->kFriction     = curSurf->kFriction2;
    				curSurf->kRollRes      = curSurf->kRollRes2;
				printf("Friction = %f - RollRes = %f\n", curSurf->kFriction, curSurf->kRollRes);
				break;
			}
			case 2:
			{
				curSurf->kFriction     = curSurf->kFriction2 * 0.8f;
    				curSurf->kRollRes      = curSurf->kRollRes2;;
				printf("Friction = %f - RollRes = %f\n", curSurf->kFriction, curSurf->kRollRes);
				break;
			}
			case 3:
			{
				curSurf->kFriction     = curSurf->kFriction2 * 0.6f;
    				curSurf->kRollRes      = curSurf->kRollRes2;
				printf("Friction = %f - RollRes = %f\n", curSurf->kFriction, curSurf->kRollRes);
				break;
			}
			default:
			{
				curSurf->kFriction     = curSurf->kFriction;
    				curSurf->kRollRes      = curSurf->kRollRes;
				printf("Friction = %f - RollRes = %f\n", curSurf->kFriction, curSurf->kRollRes);
				break;
			}
		}							
		
		curSurf = curSurf->next;
	} while ( curSurf->next != 0);
}
