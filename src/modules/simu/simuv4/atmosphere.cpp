/***************************************************************************

    file                 : atmosphere.cpp
    created              : Sun Feb 12 17:37:50 CET 2017
    copyright            : (C) 2017-2017 by Bernhard Wymann
    email                : berniw@bluewin.ch
    version              : $Id: atmosphere.cpp,v 1.1.2.1 2017/02/12 17:52:38 berniw Exp $

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <ctime>

#include "sim.h"

//static int SimClouds = 0;
static double simDegree = 0.000000023;  // Value 1 degree celsius / 24 hours
static int simuMonth = 1;

void SimAtmospherePreConfig(tTrack *track)
{
    time_t simuTime = time(0);
    tm *g = gmtime(&simuTime);
    simuMonth = 1 + g->tm_mon;

    SimRain = track->local.rain;
    SimTimeOfDay = track->local.timeofday;
    SimClouds = track->local.clouds;
    Tair = track->local.airtemperature + 273.15f;
    SimAirPressure = track->local.airpressure;
    SimAirDensity = track->local.airdensity;

    if (SimAirPressure == 0.0f)
        SimAirPressure = 101300.0f;

    if (SimAirDensity == 0.0f)
        SimAirDensity = 1.290f;

    if (track->local.config > 0)
        SimAtmosphereConfig(track);

    switch(simuMonth)
    {
    case 1:
        simDegree *= 2.0;
        break;
    case 2:
        simDegree *= 3.5;
        break;
    case 3:
        simDegree *= 5.0;
        break;
    case 4:
        simDegree *= 6.0;
        break;
    case 5:
        simDegree *= 8.0;
        break;
    case 6:
        simDegree *= 9.0;
        break;
    case 7:
        simDegree *= 10.0;
        break;
    case 8:
        simDegree *= 12.0;
        break;
    case 9:
        simDegree *= 11.0;
        break;
    case 10:
        simDegree *= 8.5;
        break;
    case 11:
        simDegree *= 5.5;
        break;
    case 12:
        simDegree *= 3.0;
        break;
    default:
        simDegree *= 1.5;
        break;
    }

    GfLogDebug("SimAirPressure = %3f - SimAirDensity = %3f\n", SimAirPressure, SimAirDensity);
}

void SimAtmosphereConfig(tTrack *track)
{
    if (SimTimeOfDay < 21000 && SimTimeOfDay > 68400)
        Tair -= 6.75;
    else if (SimTimeOfDay > 6.00 * 60 * 60 && SimTimeOfDay < 10 * 60 * 60)
        Tair += 5.75;
    else if (SimTimeOfDay > 10.00 * 60 * 60 && SimTimeOfDay < 14.00 * 60 *60)
        Tair += 10.25;
    else if (SimTimeOfDay > 14.00 * 60 * 60 && SimTimeOfDay < 16.00 * 60 * 60)
        Tair += 15.75;
    else if (SimTimeOfDay > 16.00 * 60 * 60 && SimTimeOfDay < 18.00 * 60 * 60)
        Tair += 20.75;
    else Tair +=5.75;

    if (SimClouds == 0 || SimClouds == 1)
        Tair = Tair;
    else if (SimClouds == 2 || SimClouds == 3 )
        Tair -= 1.55;
    else if (SimClouds == 4 )
        Tair -= 3.05;
    else
        Tair -= 5.75;
}

void SimAtmosphereUpdate(tSituation *s)
{
    double timeofday = SimTimeOfDay + s->currentTime;

    if ((timeofday > 6.00 * 60 * 60) && (timeofday < 18.00 * 60 * 60))
    {
        Tair = Tair + simDegree;
        GfLogDebug("Tair update = %.7f\n", Tair - 273.15);
    }
    else
    {
        Tair = Tair - simDegree;
        GfLogDebug("Tair update = %.7f\n", Tair - 273.15);
    }
    // TODO: get this later form the situation, weather simulation.
    //car->localTemperature = 273.15f + 20.0f;
    //car->localPressure = 101300.0f;
}
