/***************************************************************************

    file                 : results.cpp
    created              : Fri Apr 14 22:36:36 CEST 2000
    copyright            : (C) 2000 by Eric Espie
    email                : torcs@free.fr
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
    		This is a set of tools useful for race managers to display results.
    @ingroup	racemantools
    @author	<a href=mailto:torcs@free.fr>Eric Espie</a>
    @version	$Id$
*/

#ifdef WIN32
#include <windows.h>
#endif

#include <tgfclient.h>

#include "racescreens.h"
#include "driver.h"	//rmdGetDriverType

static int	rmSaveId;
static void	*rmScrHdle = NULL;

static void rmPracticeResults(void *prevHdle, tRmInfo *info, int start);
static void rmRaceResults(void *prevHdle, tRmInfo *info, int start);
static void rmQualifResults(void *prevHdle, tRmInfo *info, int start);
static void rmShowStandings(void *prevHdle, tRmInfo *info, int start);

#define MAX_LINES	20	//Max number of result lines ona screen

typedef struct
{
    void	*prevHdle;
    tRmInfo	*info;
    int		start;
} tRaceCall;

tRaceCall	RmNextRace;
tRaceCall	RmPrevRace;


static void
rmSaveRes(void *vInfo)
{
    tRmInfo *info = (tRmInfo *)vInfo;

    GfParmWriteFile(0, info->results, (char*)"Results");

    GfuiVisibilitySet(rmScrHdle, rmSaveId, GFUI_INVISIBLE);
}

static void
rmChgPracticeScreen(void *vprc)
{
    void		*prevScr = rmScrHdle;
    tRaceCall 	*prc = (tRaceCall*)vprc;

    rmPracticeResults(prc->prevHdle, prc->info, prc->start);
    GfuiScreenRelease(prevScr);
}

static void
rmPracticeResults(void *prevHdle, tRmInfo *info, int start)
{
    void		*results = info->results;
    const char		*race = info->_reRaceName;
    int			i;
    int			y;
    static const unsigned maxBufSize = 256;
    static char		buf[maxBufSize];
    static char		path[1024];
    char		*str;
    static float	fgcolor[4] = {1.0, 0.0, 1.0, 1.0};
    int			totLaps;


    rmScrHdle = GfuiScreenCreate();

    void *menuXMLDescHdle = LoadMenuXML("practiceresultsmenu.xml");
    CreateStaticControls(menuXMLDescHdle,rmScrHdle);

    sprintf(path, "%s/%s/%s", info->track->name, RE_SECT_RESULTS, race);
    sprintf(buf, "%s on track %s", GfParmGetStr(results, path, RM_ATTR_DRVNAME, NULL), info->track->name);

    const int messId = CreateLabelControl(rmScrHdle, menuXMLDescHdle, "playertitle");
    GfuiLabelSetText(rmScrHdle, messId, buf);
 
    const int offset = 90;
    
    const int xLap = offset + 30;
    const int xTime = offset + 50;
    const int xBest = offset + 130;
    const int xTSpd = offset + 240;
    const int xMSpd = offset + 310;
    const int xDamg = offset + 400;
    
    y = 400;
    GfuiLabelCreateEx(rmScrHdle, "Lap",       fgcolor, GFUI_FONT_MEDIUM_C, xLap, y, GFUI_ALIGN_HC_VB, 0);
    GfuiLabelCreateEx(rmScrHdle, "Time",      fgcolor, GFUI_FONT_MEDIUM_C, xTime+20, y, GFUI_ALIGN_HL_VB, 0);
    GfuiLabelCreateEx(rmScrHdle, "Best",      fgcolor, GFUI_FONT_MEDIUM_C, xBest+20, y, GFUI_ALIGN_HL_VB, 0);
    GfuiLabelCreateEx(rmScrHdle, "Top Spd",   fgcolor, GFUI_FONT_MEDIUM_C, xTSpd, y, GFUI_ALIGN_HC_VB, 0);
    GfuiLabelCreateEx(rmScrHdle, "Min Spd",   fgcolor, GFUI_FONT_MEDIUM_C, xMSpd, y, GFUI_ALIGN_HC_VB, 0);
    GfuiLabelCreateEx(rmScrHdle, "Damages",  fgcolor, GFUI_FONT_MEDIUM_C, xDamg, y, GFUI_ALIGN_HC_VB, 0);
    y -= 20;
    
    sprintf(path, "%s/%s/%s", info->track->name, RE_SECT_RESULTS, race);
    totLaps = (int)GfParmGetEltNb(results, path);
    for (i = 0 + start; i < MIN(start + MAX_LINES, totLaps); i++) {
	sprintf(path, "%s/%s/%s/%d", info->track->name, RE_SECT_RESULTS, race, i + 1);

	/* Lap */
	sprintf(buf, "%d", i+1);
	GfuiLabelCreate(rmScrHdle, buf, GFUI_FONT_MEDIUM_C, xLap, y, GFUI_ALIGN_HC_VB, 0);

	/* Time */
	str = GfTime2Str(GfParmGetNum(results, path, RE_ATTR_TIME, NULL, 0), 0);;
	GfuiLabelCreate(rmScrHdle, str, GFUI_FONT_MEDIUM_C, xTime, y, GFUI_ALIGN_HL_VB, 0);
	free(str);

	/* Best Lap Time */
	str = GfTime2Str(GfParmGetNum(results, path, RE_ATTR_BEST_LAP_TIME, NULL, 0), 0);;
	GfuiLabelCreate(rmScrHdle, str, GFUI_FONT_MEDIUM_C, xBest, y, GFUI_ALIGN_HL_VB, 0);
	free(str);

	/* Top Spd */
	sprintf(buf, "%d", (int)(GfParmGetNum(results, path, RE_ATTR_TOP_SPEED, NULL, 0) * 3.6));
	GfuiLabelCreate(rmScrHdle, buf, GFUI_FONT_MEDIUM_C, xTSpd, y, GFUI_ALIGN_HC_VB, 0);

	/* Min Spd */
	sprintf(buf, "%d", (int)(GfParmGetNum(results, path, RE_ATTR_BOT_SPEED, NULL, 0) * 3.6));
	GfuiLabelCreate(rmScrHdle, buf, GFUI_FONT_MEDIUM_C, xMSpd, y, GFUI_ALIGN_HC_VB, 0);

	/* Damages */
	sprintf(buf, "%d", (int)(GfParmGetNum(results, path, RE_ATTR_DAMMAGES, NULL, 0)));
	GfuiLabelCreate(rmScrHdle, buf, GFUI_FONT_MEDIUM_C, xDamg, y, GFUI_ALIGN_HC_VB, 0);

	y -= 15;
    }

    if (start > 0) {
	RmPrevRace.prevHdle = prevHdle;
	RmPrevRace.info     = info;
	RmPrevRace.start    = start - MAX_LINES;
	CreateButtonControl(rmScrHdle, menuXMLDescHdle, "previouspagearrow",
			    (void*)&RmPrevRace, rmChgPracticeScreen);
	GfuiAddSKey(rmScrHdle, GFUIK_PAGEUP,   "Previous Results", (void*)&RmPrevRace, rmChgPracticeScreen, NULL);
    }
    
    // Add "Continue" button
    CreateButtonControl(rmScrHdle, menuXMLDescHdle, "continuebutton", prevHdle, GfuiScreenReplace);
    
    //Create 'save' button in the bottom right
    //rmSaveId = CreateButtonControl(rmScrHdle, menuXMLDescHdle, "savebutton", info, rmSaveRes);
    
    if (i < totLaps) {
	RmNextRace.prevHdle = prevHdle;
	RmNextRace.info     = info;
	RmNextRace.start    = start + MAX_LINES;
	CreateButtonControl(rmScrHdle, menuXMLDescHdle, "nextpagearrow",
			    (void*)&RmNextRace, rmChgPracticeScreen);
	GfuiAddSKey(rmScrHdle, GFUIK_PAGEDOWN, "Next Results", (void*)&RmNextRace, rmChgPracticeScreen, NULL);
    }

    GfuiAddKey(rmScrHdle, GFUIK_ESCAPE, "Continue", prevHdle, GfuiScreenReplace, NULL);
    GfuiAddKey(rmScrHdle, GFUIK_RETURN, "Continue", prevHdle, GfuiScreenReplace, NULL);
    GfuiAddSKey(rmScrHdle, GFUIK_F12, "Take a Screen Shot", NULL, GfuiScreenShot, NULL);
    GfuiAddSKey(rmScrHdle, GFUIK_F1, "Help", rmScrHdle, GfuiHelpScreen, NULL);

    GfuiScreenActivate(rmScrHdle);
}


static void
rmChgRaceScreen(void *vprc)
{
    void		*prevScr = rmScrHdle;
    tRaceCall 	*prc = (tRaceCall*)vprc;

    rmRaceResults(prc->prevHdle, prc->info, prc->start);
    GfuiScreenRelease(prevScr);
}

static void
rmRaceResults(void *prevHdle, tRmInfo *info, int start)
{
    void		*results = info->results;
    const char		*race = info->_reRaceName;
    static const unsigned maxBufSize = 256;
    static char		buf[maxBufSize];
    static char		path[1024];
    char		*str;
    static float	fgcolor[4] = {1.0, 0.0, 1.0, 1.0};
		static float  green[4] = {0.196, 0.804, 0.196, 1.0};//Lime green, #32CD32
		static float	orange[4] = {0.953, 0.518, 0.0, 1.0};//Tangerine, #F28500
		static float	white[4] = {1.0, 1.0, 1.0, 1.0};
		
		//Screen title
    rmScrHdle = GfuiScreenCreate();
	void *menuXMLDescHdle = LoadMenuXML("raceresultsmenu.xml");
    CreateStaticControls(menuXMLDescHdle,rmScrHdle);

    sprintf(buf, "%s", info->track->name);
    const int messId = CreateLabelControl(rmScrHdle, menuXMLDescHdle, "racetitle");
    GfuiLabelSetText(rmScrHdle, messId, buf);

  
		//Column positions
    const int xRank = 10;
		const int xAdv = 35;
    const int xDriver = 55;
    const int xType = 195;
    const int xCar  = 275;
    const int xTotal = 425;
    const int xBest = 490;
    const int xLaps = 515;
    const int xTSpd = 550;
    const int xDamg = 590;
    const int xPStp = 625;
    
		//Heading
    int y = 400;
    GfuiLabelCreateEx(rmScrHdle, "Rk",      fgcolor, GFUI_FONT_MEDIUM_C, xRank, y, GFUI_ALIGN_HC_VB, 0);
    GfuiLabelCreateEx(rmScrHdle, "Adv",     fgcolor, GFUI_FONT_MEDIUM_C, xAdv, y, GFUI_ALIGN_HC_VB, 0);
    GfuiLabelCreateEx(rmScrHdle, "Driver",  fgcolor, GFUI_FONT_MEDIUM_C, xDriver+10, y, GFUI_ALIGN_HL_VB, 0);
    GfuiLabelCreateEx(rmScrHdle, "Type",    fgcolor, GFUI_FONT_MEDIUM_C, xType+10, y, GFUI_ALIGN_HL_VB, 0);
    GfuiLabelCreateEx(rmScrHdle, "Car",     fgcolor, GFUI_FONT_MEDIUM_C, xCar+10, y, GFUI_ALIGN_HL_VB, 0);
    GfuiLabelCreateEx(rmScrHdle, "Total",   fgcolor, GFUI_FONT_MEDIUM_C, xTotal-15, y, GFUI_ALIGN_HR_VB, 0);
    GfuiLabelCreateEx(rmScrHdle, "Best",    fgcolor, GFUI_FONT_MEDIUM_C, xBest-15, y, GFUI_ALIGN_HR_VB, 0);
    GfuiLabelCreateEx(rmScrHdle, "Laps",    fgcolor, GFUI_FONT_MEDIUM_C, xLaps, y, GFUI_ALIGN_HC_VB, 0);
    GfuiLabelCreateEx(rmScrHdle, "T.Sp.",   fgcolor, GFUI_FONT_MEDIUM_C, xTSpd, y, GFUI_ALIGN_HC_VB, 0);
    GfuiLabelCreateEx(rmScrHdle, "Dam.",    fgcolor, GFUI_FONT_MEDIUM_C, xDamg, y, GFUI_ALIGN_HC_VB, 0);
    GfuiLabelCreateEx(rmScrHdle, "Pits",    fgcolor, GFUI_FONT_MEDIUM_C, xPStp, y, GFUI_ALIGN_HC_VB, 0);
    y -= 20;

		//Get total laps, winner time, number of cars
    sprintf(path, "%s/%s/%s", info->track->name, RE_SECT_RESULTS, race);
    int totLaps = (int)GfParmGetNum(results, path, RE_ATTR_LAPS, NULL, 0);
    sprintf(path, "%s/%s/%s/%s/%d", info->track->name, RE_SECT_RESULTS, race, RE_SECT_RANK, 1);
    tdble refTime = GfParmGetNum(results, path, RE_ATTR_TIME, NULL, 0);
    sprintf(path, "%s/%s/%s/%s", info->track->name, RE_SECT_RESULTS, race, RE_SECT_RANK);
    int nbCars = (int)GfParmGetEltNb(results, path);
		
		int i;
    for (i = start; i < MIN(start + MAX_LINES, nbCars); i++) {
			sprintf(path, "%s/%s/%s/%s/%d", info->track->name, RE_SECT_RESULTS, race, RE_SECT_RANK, i + 1);
			int laps = (int)GfParmGetNum(results, path, RE_ATTR_LAPS, NULL, 0);//Laps covered

			//Rank
			sprintf(buf, "%d", i+1);
			GfuiLabelCreate(rmScrHdle, buf, GFUI_FONT_MEDIUM_C, xRank, y, GFUI_ALIGN_HC_VB, 0);
			//Advance (The num.attrib 'index' holds the starting position)
			int advance = (int)(GfParmGetNum(results, path, RE_ATTR_INDEX, NULL, 0)) - i;
			//sprintf(buf, "%s%d", advance > 0 ? "^" : (advance < 0 ? "!" : "<"), abs(advance));
			sprintf(buf, "%d", advance);
			const float *c = advance > 0 ? green : (advance < 0 ? orange : white);
			GfuiLabelCreateEx(rmScrHdle, buf, c, GFUI_FONT_MEDIUM_C,	xAdv, y, GFUI_ALIGN_HC_VB, 0);
			//Driver name
			GfuiLabelCreate(rmScrHdle, GfParmGetStr(results, path, RE_ATTR_NAME, ""), GFUI_FONT_MEDIUM_C,
				xDriver, y, GFUI_ALIGN_HL_VB, 0);
			//Driver type
			rmdGetDriverType(GfParmGetStr(results, path, RE_ATTR_MODULE, ""), buf, maxBufSize);
			GfuiLabelCreate(rmScrHdle, buf, GFUI_FONT_MEDIUM_C,	xType, y, GFUI_ALIGN_HL_VB, 0);
			//Car
			GfuiLabelCreate(rmScrHdle, GfParmGetStr(results, path, RE_ATTR_CAR, ""), GFUI_FONT_MEDIUM_C,
				xCar, y, GFUI_ALIGN_HL_VB, 0);

	GfuiLabelCreate(rmScrHdle, GfParmGetStr(results, path, RE_ATTR_NAME, NULL), GFUI_FONT_MEDIUM_C,
			xDriver, y, GFUI_ALIGN_HL_VB, 0);
	rmdGetDriverType(GfParmGetStr(results, path, RE_ATTR_MODULE, NULL), buf, maxBufSize);
	GfuiLabelCreate(rmScrHdle, buf, GFUI_FONT_MEDIUM_C,
			xType, y, GFUI_ALIGN_HL_VB, 0);
	GfuiLabelCreate(rmScrHdle, GfParmGetStr(results, path, RE_ATTR_CAR, NULL), GFUI_FONT_MEDIUM_C,
			xCar, y, GFUI_ALIGN_HL_VB, 0);

			//Best lap
			str = GfTime2Str(GfParmGetNum(results, path, RE_ATTR_BEST_LAP_TIME, NULL, 0), 0);
			GfuiLabelCreate(rmScrHdle, str, GFUI_FONT_MEDIUM_C,	xBest, y, GFUI_ALIGN_HR_VB, 0);
			free(str);

			//Laps covered
			sprintf(buf, "%d", laps);
			GfuiLabelCreate(rmScrHdle, buf, GFUI_FONT_MEDIUM_C,	xLaps, y, GFUI_ALIGN_HC_VB, 0);

			//Top speed
			sprintf(buf, "%d", (int)(GfParmGetNum(results, path, RE_ATTR_TOP_SPEED, NULL, 0) * 3.6));
			GfuiLabelCreate(rmScrHdle, buf, GFUI_FONT_MEDIUM_C,	xTSpd, y, GFUI_ALIGN_HC_VB, 0);

			//Damage
			sprintf(buf, "%d", (int)(GfParmGetNum(results, path, RE_ATTR_DAMMAGES, NULL, 0)));
			GfuiLabelCreate(rmScrHdle, buf, GFUI_FONT_MEDIUM_C,	xDamg, y, GFUI_ALIGN_HC_VB, 0);

			//Pitstops
			sprintf(buf, "%d", (int)(GfParmGetNum(results, path, RE_ATTR_NB_PIT_STOPS, NULL, 0)));
			GfuiLabelCreate(rmScrHdle, buf, GFUI_FONT_MEDIUM_C,	xPStp, y, GFUI_ALIGN_HC_VB, 0);
	
			y -= 15;	//Line feed
    }//for i

		//If it is not the first screen of the results, show a 'Prev' button
    if (start > 0) {
	RmPrevRace.prevHdle = prevHdle;
	RmPrevRace.info     = info;
	RmPrevRace.start    = start - MAX_LINES;
	CreateButtonControl(rmScrHdle, menuXMLDescHdle, "previouspagearrow",
			    (void*)&RmPrevRace, rmChgRaceScreen);
			GfuiAddSKey(rmScrHdle, GFUIK_PAGEUP,   "Previous Results", (void*)&RmPrevRace, rmChgRaceScreen, NULL);
		}//if start

    // Add "Continue" button
    CreateButtonControl(rmScrHdle, menuXMLDescHdle, "continuebutton", prevHdle, GfuiScreenReplace);
    
    //Create 'save' button in the bottom right
    //rmSaveId = CreateButtonControl(rmScrHdle, menuXMLDescHdle, "savebutton", info, rmSaveRes);

		//If we did not display all the results yet, let's show a 'Next' button
    if (i < nbCars) {
	RmNextRace.prevHdle = prevHdle;
	RmNextRace.info     = info;
	RmNextRace.start    = start + MAX_LINES;
	CreateButtonControl(rmScrHdle, menuXMLDescHdle, "nextpagearrow",
			    (void*)&RmNextRace, rmChgRaceScreen);
			GfuiAddSKey(rmScrHdle, GFUIK_PAGEDOWN, "Next Results", (void*)&RmNextRace, rmChgRaceScreen, NULL);
    }//if i

		//Link key handlers
    GfuiAddKey(rmScrHdle, GFUIK_ESCAPE, "Continue", prevHdle, GfuiScreenReplace, NULL);
    GfuiAddKey(rmScrHdle, GFUIK_RETURN, "Continue", prevHdle, GfuiScreenReplace, NULL);
    GfuiAddSKey(rmScrHdle, GFUIK_F12, "Take a Screen Shot", NULL, GfuiScreenShot, NULL);
    GfuiAddSKey(rmScrHdle, GFUIK_F1, "Help", rmScrHdle, GfuiHelpScreen, NULL);

		//Show!
    GfuiScreenActivate(rmScrHdle);
}//rmRaceResults


static void
rmChgQualifScreen(void *vprc)
{
    void		*prevScr = rmScrHdle;
    tRaceCall 	*prc = (tRaceCall*)vprc;

    rmQualifResults(prc->prevHdle, prc->info, prc->start);
    GfuiScreenRelease(prevScr);
}

static void
rmQualifResults(void *prevHdle, tRmInfo *info, int start)
{
    void		*results = info->results;
    const char		*race = info->_reRaceName;
    int			i;
    int			y;
    static const unsigned maxBufSize = 256;
    static char		buf[maxBufSize];
    static char		path[1024];
    char		*str;
    static float	fgcolor[4] = {1.0, 0.0, 1.0, 1.0};
    int			laps, totLaps;
    tdble		refTime;
    int			nbCars;

    rmScrHdle = GfuiScreenCreate();
		void *menuXMLDescHdle = LoadMenuXML("qualifsresultsmenu.xml");
    CreateStaticControls(menuXMLDescHdle,rmScrHdle);

    sprintf(buf, "%s", info->track->name);
    const int messId = CreateLabelControl(rmScrHdle, menuXMLDescHdle, "racetitle");
    GfuiLabelSetText(rmScrHdle, messId, buf);

    const int offset  = 50;
    const int xRank   = offset + 30;
    const int xDriver = offset + 60;
    const int xType   = offset + 240;
    const int xCar    = offset + 320;
    const int xTime   = offset + 520;

    y = 400;
    GfuiLabelCreateEx(rmScrHdle, "Rank",    fgcolor, GFUI_FONT_MEDIUM_C, xRank, y, GFUI_ALIGN_HC_VB, 0);
    GfuiLabelCreateEx(rmScrHdle, "Driver",  fgcolor, GFUI_FONT_MEDIUM_C, xDriver+10, y, GFUI_ALIGN_HL_VB, 0);
    GfuiLabelCreateEx(rmScrHdle, "Type",    fgcolor, GFUI_FONT_MEDIUM_C, xType+10, y, GFUI_ALIGN_HL_VB, 0);
    GfuiLabelCreateEx(rmScrHdle, "Car",     fgcolor, GFUI_FONT_MEDIUM_C, xCar+10, y, GFUI_ALIGN_HL_VB, 0);
    GfuiLabelCreateEx(rmScrHdle, "Time",    fgcolor, GFUI_FONT_MEDIUM_C, xTime, y, GFUI_ALIGN_HR_VB, 0);
    y -= 20;
    
    sprintf(path, "%s/%s/%s", info->track->name, RE_SECT_RESULTS, race);
    totLaps = (int)GfParmGetNum(results, path, RE_ATTR_LAPS, NULL, 0);
    sprintf(path, "%s/%s/%s/%s/%d", info->track->name, RE_SECT_RESULTS, race, RE_SECT_RANK, 1);
    refTime = GfParmGetNum(results, path, RE_ATTR_TIME, NULL, 0);
    sprintf(path, "%s/%s/%s/%s", info->track->name, RE_SECT_RESULTS, race, RE_SECT_RANK);
    nbCars = (int)GfParmGetEltNb(results, path);
    for (i = start; i < MIN(start + MAX_LINES, nbCars); i++) {
	sprintf(path, "%s/%s/%s/%s/%d", info->track->name, RE_SECT_RESULTS, race, RE_SECT_RANK, i + 1);
	laps = (int)GfParmGetNum(results, path, RE_ATTR_LAPS, NULL, 0);

	sprintf(buf, "%d", i+1);
	GfuiLabelCreate(rmScrHdle, buf, GFUI_FONT_MEDIUM_C,
			xRank, y, GFUI_ALIGN_HC_VB, 0);

	GfuiLabelCreate(rmScrHdle, GfParmGetStr(results, path, RE_ATTR_NAME, NULL), GFUI_FONT_MEDIUM_C,
			xDriver, y, GFUI_ALIGN_HL_VB, 0);
	rmdGetDriverType(GfParmGetStr(results, path, RE_ATTR_MODULE, NULL), buf, maxBufSize);
	GfuiLabelCreate(rmScrHdle, buf, GFUI_FONT_MEDIUM_C,
			xType, y, GFUI_ALIGN_HL_VB, 0);
	GfuiLabelCreate(rmScrHdle, GfParmGetStr(results, path, RE_ATTR_CAR, NULL), GFUI_FONT_MEDIUM_C,
			xCar, y, GFUI_ALIGN_HL_VB, 0);

	str = GfTime2Str(GfParmGetNum(results, path, RE_ATTR_BEST_LAP_TIME, NULL, 0), 0);
	GfuiLabelCreate(rmScrHdle, str, GFUI_FONT_MEDIUM_C,
			xTime, y, GFUI_ALIGN_HR_VB, 0);
	free(str);
	y -= 15;
    }


    if (start > 0) {
	RmPrevRace.prevHdle = prevHdle;
	RmPrevRace.info     = info;
	RmPrevRace.start    = start - MAX_LINES;
	CreateButtonControl(rmScrHdle, menuXMLDescHdle, "previouspagearrow",
			    (void*)&RmPrevRace, rmChgQualifScreen);
	GfuiAddSKey(rmScrHdle, GFUIK_PAGEUP,   "Previous Results", (void*)&RmPrevRace, rmChgQualifScreen, NULL);
    }

    // Add "Continue" button 
    CreateButtonControl(rmScrHdle, menuXMLDescHdle, "continuebutton", prevHdle, GfuiScreenReplace);
    
    //Create 'save' button in the bottom right
    //rmSaveId = CreateButtonControl(rmScrHdle, menuXMLDescHdle, "savebutton", info, rmSaveRes);

    if (i < nbCars) {
	RmNextRace.prevHdle = prevHdle;
	RmNextRace.info     = info;
	RmNextRace.start    = start + MAX_LINES;
	CreateButtonControl(rmScrHdle, menuXMLDescHdle, "nextpagearrow",
			    (void*)&RmNextRace, rmChgQualifScreen);
	GfuiAddSKey(rmScrHdle, GFUIK_PAGEDOWN, "Next Results", (void*)&RmNextRace, rmChgQualifScreen, NULL);
    }

    GfuiAddKey(rmScrHdle, GFUIK_ESCAPE, "Continue", prevHdle, GfuiScreenReplace, NULL);
    GfuiAddKey(rmScrHdle, GFUIK_RETURN, "Continue", prevHdle, GfuiScreenReplace, NULL);
    GfuiAddSKey(rmScrHdle, GFUIK_F12, "Take a Screen Shot", NULL, GfuiScreenShot, NULL);
    GfuiAddSKey(rmScrHdle, GFUIK_F1, "Help", rmScrHdle, GfuiHelpScreen, NULL);

    GfuiScreenActivate(rmScrHdle);
}

static void
rmChgStandingScreen(void *vprc)
{
    void		*prevScr = rmScrHdle;
    tRaceCall 	*prc = (tRaceCall*)vprc;

    rmShowStandings(prc->prevHdle, prc->info, prc->start);
    GfuiScreenRelease(prevScr);
}

/** 
 * rmShowStandings
 * 
 * Shows a results page, with optional prev/next results page buttons
 * 
 * @param prevHdle	handle for previous results page
 * @param info	race results information
 * @param start	page number
*/
static void
rmShowStandings(void *prevHdle, tRmInfo *info, int start)
{
	int			i;
	int			y;
	static const unsigned maxBufSize = 256;
	static char		buf[maxBufSize];
	static char		path[1024];
	static float	fgcolor[4] = {1.0, 0.0, 1.0, 1.0};
	int			nbCars;
	void		*results = info->results;
	const char		*race = info->_reRaceName;

	rmScrHdle = GfuiScreenCreate();

	void *menuXMLDescHdle = LoadMenuXML("standingsmenu.xml");
	CreateStaticControls(menuXMLDescHdle,rmScrHdle);

	//Set title
	sprintf(buf, "%s Standings", race);
	const int messId = CreateLabelControl(rmScrHdle, menuXMLDescHdle, "racetitle");
	GfuiLabelSetText(rmScrHdle, messId, buf);

	//Show header
	const int offset  = 50;
	const int xRank   = offset + 30;
	const int xDriver = offset + 60;
	const int xType   = offset + 240;
	const int xCar    = offset + 320;
	const int xPoints = offset + 520;
	y = 400;
	GfuiLabelCreateEx(rmScrHdle, "Rank",   fgcolor, GFUI_FONT_MEDIUM_C, xRank, y, GFUI_ALIGN_HC_VB, 0);
	GfuiLabelCreateEx(rmScrHdle, "Driver", fgcolor, GFUI_FONT_MEDIUM_C, xDriver+10, y, GFUI_ALIGN_HL_VB, 0);
	GfuiLabelCreateEx(rmScrHdle, "Type",   fgcolor, GFUI_FONT_MEDIUM_C, xType+10, y, GFUI_ALIGN_HL_VB, 0);
	GfuiLabelCreateEx(rmScrHdle, "Car",    fgcolor, GFUI_FONT_MEDIUM_C, xCar+10, y, GFUI_ALIGN_HL_VB, 0);
	GfuiLabelCreateEx(rmScrHdle, "Points", fgcolor, GFUI_FONT_MEDIUM_C, xPoints, y, GFUI_ALIGN_HR_VB, 0);
	y -= 20;

	//List results line by line, paginated
	nbCars = (int)GfParmGetEltNb(results, (char*)RE_SECT_STANDINGS);
	for (i = start; i < MIN(start + MAX_LINES, nbCars); i++) {
		sprintf(path, "%s/%d", RE_SECT_STANDINGS, i + 1);
		
		//Rank
		sprintf(buf, "%d", i+1);
		GfuiLabelCreate(rmScrHdle, buf, GFUI_FONT_MEDIUM_C,
			xRank, y, GFUI_ALIGN_HC_VB, 0);

		//Driver name
		GfuiLabelCreate(rmScrHdle, GfParmGetStr(results, path, RE_ATTR_NAME, NULL), GFUI_FONT_MEDIUM_C,
			xDriver, y, GFUI_ALIGN_HL_VB, 0);
	
		//Driver type
		rmdGetDriverType(GfParmGetStr(results, path, RE_ATTR_MODULE, NULL), buf, maxBufSize);
		GfuiLabelCreate(rmScrHdle, buf, GFUI_FONT_MEDIUM_C,
			xType, y, GFUI_ALIGN_HL_VB, 0);
			
		//Car
		GfuiLabelCreate(rmScrHdle, GfParmGetStr(results, path, RE_ATTR_CAR, NULL), GFUI_FONT_MEDIUM_C,
			xCar, y, GFUI_ALIGN_HL_VB, 0);

		//Points
		sprintf(buf, "%d", (int)GfParmGetNum(results, path, RE_ATTR_POINTS, NULL, 0));
		GfuiLabelCreate(rmScrHdle, buf, GFUI_FONT_MEDIUM_C,
			xPoints, y, GFUI_ALIGN_HR_VB, 0);
			
		y -= 15;	//Next line
	}//for i

	//If not on first page, show 'previous results' button on the bottom left
	if (start > 0) {
		RmPrevRace.prevHdle = prevHdle;
		RmPrevRace.info     = info;
		RmPrevRace.start    = start - MAX_LINES;
		CreateButtonControl(rmScrHdle, menuXMLDescHdle, "previouspagearrow",
				    (void*)&RmPrevRace, rmChgStandingScreen);
		GfuiAddSKey(rmScrHdle, GFUIK_PAGEUP, "Previous Results", (void*)&RmPrevRace, rmChgStandingScreen, NULL);
	}//if start

	// Add "Continue" button in the bottom left
	CreateButtonControl(rmScrHdle, menuXMLDescHdle, "continuebutton", prevHdle, GfuiScreenReplace);
    
	//Create 'save' button in the bottom right if ????
	if (!strcmp( GfParmGetStr( info->mainParams, RM_SECT_SUBFILES, RM_ATTR_HASSUBFILES, RM_VAL_NO ), RM_VAL_YES ) == 0) {
	    rmSaveId = CreateButtonControl(rmScrHdle, menuXMLDescHdle, "savebutton", info, rmSaveRes);
	}

	//If there is a next page, show 'next results' button on the bottom extreme right
	if (i < nbCars) {
		RmNextRace.prevHdle = prevHdle;
		RmNextRace.info     = info;
		RmNextRace.start    = start + MAX_LINES;
		CreateButtonControl(rmScrHdle, menuXMLDescHdle, "nextpagearrow",
				    (void*)&RmNextRace, rmChgStandingScreen);
		GfuiAddSKey(rmScrHdle, GFUIK_PAGEDOWN, "Next Results", (void*)&RmNextRace, rmChgStandingScreen, NULL);
	}//if i

    GfuiAddKey(rmScrHdle, GFUIK_ESCAPE, "Continue", prevHdle, GfuiScreenReplace, NULL);
    GfuiAddKey(rmScrHdle, GFUIK_ESCAPE, "Continue", prevHdle, GfuiScreenReplace, NULL);
    GfuiAddSKey(rmScrHdle, GFUIK_F1, "Help", rmScrHdle, GfuiHelpScreen, NULL);
    GfuiAddSKey(rmScrHdle, GFUIK_F12, "Take a Screen Shot", NULL, GfuiScreenShot, NULL);

    GfuiScreenActivate(rmScrHdle);
}//rmShowStandings


void
RmShowResults(void *prevHdle, tRmInfo *info)
{
    switch (info->s->_raceType) {
    case RM_TYPE_PRACTICE:
	if (info->s->_ncars == 1)
	    rmPracticeResults(prevHdle, info, 0);
	else
	    rmQualifResults(prevHdle, info, 0);
	break;

    case RM_TYPE_RACE:
	rmRaceResults(prevHdle, info, 0);
	break;

    case RM_TYPE_QUALIF:
	rmQualifResults(prevHdle, info, 0);
	break;
    }//switch raceType
}//RmShowResults


void
RmShowStandings(void *prevHdle, tRmInfo *info)
{
    rmShowStandings(prevHdle, info, 0);
}//RmShowStandings
