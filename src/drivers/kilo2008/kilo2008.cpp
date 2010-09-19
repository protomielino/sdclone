/*
 *      kilo2008.cpp
 *      
 *      Copyright 2009 kilo aka Gabor Kmetyko <kg.kilo@gmail.com>
 *      Based on work by Bernhard Wymann and Andrew Sumner.
 *      
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *      
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *      
 *      You should have received a copy of the GNU General Public License
 *      along with this program; if not, write to the Free Software
 *      Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *      MA 02110-1301, USA.
 * 
 *      $Id$
 * 
 */

#ifdef _WIN32
#include <windows.h>
#endif

#include "kdriver.h"

#include <robot.h>  //ROB_IDENT

#define NBBOTS 2

static char* botname[NBBOTS] = {(char*)"Kilo 1", (char*)"Kilo 2"};
static char* botdesc[NBBOTS] = {(char*)"Kilo 1", (char*)"Kilo 2"};

static KDriver *driver[NBBOTS];

static void initTrack(int index, tTrack* track, void *carHandle, void **carParmHandle, tSituation *s);
static void newRace(int index, tCarElt* car, tSituation *s);
static void drive(int index, tCarElt* car, tSituation *s);
static int pitcmd(int index, tCarElt* car, tSituation *s);
static void shutdown(int index);
static int InitFuncPt(int index, void *pt);
static void endRace(int index, tCarElt *car, tSituation *s);


// Module entry point.
extern "C" int kilo2008(tModInfo *modInfo)
{
    // Clear all structures.
    memset(modInfo, 0, 10*sizeof(tModInfo));

    for (int i = 0; i < NBBOTS; i++) {
        modInfo[i].name    = botname[i];            // name of the module (short).
        modInfo[i].desc    = botdesc[i];            // Description of the module (can be long).
        modInfo[i].fctInit = InitFuncPt;            // Init function.
        modInfo[i].gfId    = ROB_IDENT;             // Supported framework version.
        modInfo[i].index   = i + 1;                 // Indices from 0 to 9.
    }
    return 0;
}


// Module interface initialization.
static int InitFuncPt(int index, void *pt)
{
    tRobotItf *itf = (tRobotItf *)pt;

    // Create robot instance for index.
    driver[index-1] = new KDriver(index);
    driver[index-1]->bot = "kilo2008";
    itf->rbNewTrack = initTrack;    // Give the robot the track view called.
    itf->rbNewRace  = newRace;      // Start a new race.
    itf->rbDrive    = drive;        // Drive during race.
    itf->rbPitCmd   = pitcmd;       // Pit commands.
    itf->rbEndRace  = endRace;      // End of the current race.
    itf->rbShutdown = shutdown;     // Called before the module is unloaded.
    itf->index      = index;        // Index used if multiple interfaces.
    return 0;
}


// Called for every track change or new race.
static void initTrack(int index, tTrack* track, void *carHandle, void **carParmHandle, tSituation *s)
{
    driver[index-1]->initTrack(track, carHandle, carParmHandle, s);
}


// Start a new race.
static void newRace(int index, tCarElt* car, tSituation *s)
{
    driver[index-1]->newRace(car, s);
}


// Drive during race.
static void drive(int index, tCarElt* car, tSituation *s)
{
    driver[index-1]->drive(s);
}


// Pitstop callback.
static int pitcmd(int index, tCarElt* car, tSituation *s)
{
    return driver[index-1]->pitCommand(s);
}


// End of the current race.
static void endRace(int index, tCarElt *car, tSituation *s)
{
    driver[index-1]->endRace(s);
}


// Called before the module is unloaded.
static void shutdown(int index)
{
    delete driver[index-1];
}

