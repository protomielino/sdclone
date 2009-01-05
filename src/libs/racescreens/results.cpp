/***************************************************************************

    file                 : results.cpp
    created              : Fri Apr 14 22:36:36 CEST 2000
    copyright            : (C) 2000 by Eric Espie
    email                : torcs@free.fr
    version              : $Id: results.cpp,v 1.6 2005/07/21 21:27:14 berniw Exp $

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
    @version	$Id: results.cpp,v 1.6 2005/07/21 21:27:14 berniw Exp $
*/

#include <stdlib.h>
#include <stdio.h>
#include <cstring>
#ifdef WIN32
#include <windows.h>
#endif
#include <tgfclient.h>
#include <osspec.h>
#include <robottools.h>
#include <robot.h>

#include "driver.h"
#include "racescreens.h"

static int	rmSaveId;
static void	*rmScrHdle = NULL;

static void rmPracticeResults(void *prevHdle, tRmInfo *info, int start);
static void rmRaceResults(void *prevHdle, tRmInfo *info, int start);
static void rmQualifResults(void *prevHdle, tRmInfo *info, int start);
static void rmShowStandings(void *prevHdle, tRmInfo *info, int start);

#define MAX_LINES	20

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

    GfParmWriteFile(0, info->results, "Results");

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
    sprintf(buf, "Practice Results");
    GfuiTitleCreate(rmScrHdle, buf, strlen(buf));
    sprintf(path, "%s/%s/%s", info->track->name, RE_SECT_RESULTS, race);
    sprintf(buf, "%s on track %s", GfParmGetStr(results, path, RM_ATTR_DRVNAME, ""), info->track->name);
    GfuiLabelCreate(rmScrHdle, buf, GFUI_FONT_LARGE_C,
		    320, 420, GFUI_ALIGN_HC_VB, 0);
    GfuiScreenAddBgImg(rmScrHdle, "data/img/splash-result.png");
    
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
	GfuiGrButtonCreate(rmScrHdle, "data/img/arrow-up.png", "data/img/arrow-up.png",
			   "data/img/arrow-up.png", "data/img/arrow-up-pushed.png",
			   80, 40, GFUI_ALIGN_HL_VB, 1,
			   (void*)&RmPrevRace, rmChgPracticeScreen,
			   NULL, (tfuiCallback)NULL, (tfuiCallback)NULL);
	GfuiAddSKey(rmScrHdle, GLUT_KEY_PAGE_UP,   "Previous Results", (void*)&RmPrevRace, rmChgPracticeScreen, NULL);
    }
    

    GfuiButtonCreate(rmScrHdle,
		     "Continue",
		     GFUI_FONT_LARGE,
		     320,
		     40,
		     150,
		     GFUI_ALIGN_HC_VB,
		     0,
		     prevHdle,
		     GfuiScreenReplace,
		     NULL,
		     (tfuiCallback)NULL,
		     (tfuiCallback)NULL);

/*     rmSaveId = GfuiButtonCreate(rmScrHdle, */
/* 				"Save", */
/* 				GFUI_FONT_LARGE, */
/* 				430, */
/* 				40, */
/* 				150, */
/* 				GFUI_ALIGN_HC_VB, */
/* 				0, */
/* 				info, */
/* 				rmSaveRes, */
/* 				NULL, */
/* 				(tfuiCallback)NULL, */
/* 				(tfuiCallback)NULL); */

    if (i < totLaps) {
	RmNextRace.prevHdle = prevHdle;
	RmNextRace.info     = info;
	RmNextRace.start    = start + MAX_LINES;
	GfuiGrButtonCreate(rmScrHdle, "data/img/arrow-down.png", "data/img/arrow-down.png",
			   "data/img/arrow-down.png", "data/img/arrow-down-pushed.png",
			   540, 40, GFUI_ALIGN_HL_VB, 1,
			   (void*)&RmNextRace, rmChgPracticeScreen,
			   NULL, (tfuiCallback)NULL, (tfuiCallback)NULL);
	GfuiAddSKey(rmScrHdle, GLUT_KEY_PAGE_DOWN, "Next Results", (void*)&RmNextRace, rmChgPracticeScreen, NULL);
    }

    GfuiAddKey(rmScrHdle, (unsigned char)27, "", prevHdle, GfuiScreenReplace, NULL);
    GfuiAddKey(rmScrHdle, (unsigned char)13, "", prevHdle, GfuiScreenReplace, NULL);
    GfuiAddSKey(rmScrHdle, GLUT_KEY_F12, "Take a Screen Shot", NULL, GfuiScreenShot, NULL);

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
    int			i;
    int			dlap;
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
    sprintf(buf, "Race Results");
    GfuiTitleCreate(rmScrHdle, buf, strlen(buf));
    sprintf(buf, "%s", info->track->name);
    GfuiLabelCreate(rmScrHdle, buf, GFUI_FONT_LARGE_C,
		    320, 420, GFUI_ALIGN_HC_VB, 0);
    GfuiScreenAddBgImg(rmScrHdle, "data/img/splash-result.png");
    
    const int xRank = 10;
    const int xDriver = 25;
    const int xType = 180;
    const int xCar  = 250;
    const int xTotal = 425;
    const int xBest = 490;
    const int xLaps = 515;
    const int xTSpd = 550;
    const int xDamg = 590;
    const int xPStp = 625;
    
    y = 400;
    GfuiLabelCreateEx(rmScrHdle, "Rk",      fgcolor, GFUI_FONT_MEDIUM_C, xRank, y, GFUI_ALIGN_HC_VB, 0);
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

	GfuiLabelCreate(rmScrHdle, GfParmGetStr(results, path, RE_ATTR_NAME, ""), GFUI_FONT_MEDIUM_C,
			xDriver, y, GFUI_ALIGN_HL_VB, 0);
	rmdGetDriverType(GfParmGetStr(results, path, RE_ATTR_MODULE, ""), buf, maxBufSize);
	GfuiLabelCreate(rmScrHdle, buf, GFUI_FONT_MEDIUM_C,
			xType, y, GFUI_ALIGN_HL_VB, 0);
	GfuiLabelCreate(rmScrHdle, GfParmGetStr(results, path, RE_ATTR_CAR, ""), GFUI_FONT_MEDIUM_C,
			xCar, y, GFUI_ALIGN_HL_VB, 0);

	if (laps == totLaps) {
		if (i == 0) {
			str = GfTime2Str(GfParmGetNum(results, path, RE_ATTR_TIME, NULL, 0), 0);
	    } else {
			str = GfTime2Str(GfParmGetNum(results, path, RE_ATTR_TIME, NULL, 0) - refTime, 1);
	    }
		GfuiLabelCreate(rmScrHdle, str, GFUI_FONT_MEDIUM_C, xTotal, y, GFUI_ALIGN_HR_VB, 0);
		free(str);
	} else {
	    dlap = totLaps - laps;
	    if (dlap == 1) {
		sprintf(buf,"+1 Lap");
	    } else {
		sprintf(buf,"+%d Laps", dlap);
	    }
		GfuiLabelCreate(rmScrHdle, buf, GFUI_FONT_MEDIUM_C, xTotal, y, GFUI_ALIGN_HR_VB, 0);
	}

	str = GfTime2Str(GfParmGetNum(results, path, RE_ATTR_BEST_LAP_TIME, NULL, 0), 0);
	GfuiLabelCreate(rmScrHdle, str, GFUI_FONT_MEDIUM_C,
			xBest, y, GFUI_ALIGN_HR_VB, 0);
	free(str);

	sprintf(buf, "%d", laps);
	GfuiLabelCreate(rmScrHdle, buf, GFUI_FONT_MEDIUM_C,
			xLaps, y, GFUI_ALIGN_HC_VB, 0);

	sprintf(buf, "%d", (int)(GfParmGetNum(results, path, RE_ATTR_TOP_SPEED, NULL, 0) * 3.6));
	GfuiLabelCreate(rmScrHdle, buf, GFUI_FONT_MEDIUM_C,
			xTSpd, y, GFUI_ALIGN_HC_VB, 0);

	sprintf(buf, "%d", (int)(GfParmGetNum(results, path, RE_ATTR_DAMMAGES, NULL, 0)));
	GfuiLabelCreate(rmScrHdle, buf, GFUI_FONT_MEDIUM_C,
			xDamg, y, GFUI_ALIGN_HC_VB, 0);

	sprintf(buf, "%d", (int)(GfParmGetNum(results, path, RE_ATTR_NB_PIT_STOPS, NULL, 0)));
	GfuiLabelCreate(rmScrHdle, buf, GFUI_FONT_MEDIUM_C,
			xPStp, y, GFUI_ALIGN_HC_VB, 0);
	y -= 15;
    }


    if (start > 0) {
	RmPrevRace.prevHdle = prevHdle;
	RmPrevRace.info     = info;
	RmPrevRace.start    = start - MAX_LINES;
	GfuiGrButtonCreate(rmScrHdle, "data/img/arrow-up.png", "data/img/arrow-up.png",
			   "data/img/arrow-up.png", "data/img/arrow-up-pushed.png",
			   80, 40, GFUI_ALIGN_HL_VB, 1,
			   (void*)&RmPrevRace, rmChgRaceScreen,
			   NULL, (tfuiCallback)NULL, (tfuiCallback)NULL);
	GfuiAddSKey(rmScrHdle, GLUT_KEY_PAGE_UP,   "Previous Results", (void*)&RmPrevRace, rmChgRaceScreen, NULL);
    }

    GfuiButtonCreate(rmScrHdle,
		     "Continue",
		     GFUI_FONT_LARGE,
		     /* 210, */
		     320,
		     40,
		     150,
		     GFUI_ALIGN_HC_VB,
		     0,
		     prevHdle,
		     GfuiScreenReplace,
		     NULL,
		     (tfuiCallback)NULL,
		     (tfuiCallback)NULL);

/*     rmSaveId = GfuiButtonCreate(rmScrHdle, */
/* 				"Save", */
/* 				GFUI_FONT_LARGE, */
/* 				430, */
/* 				40, */
/* 				150, */
/* 				GFUI_ALIGN_HC_VB, */
/* 				0, */
/* 				info, */
/* 				rmSaveRes, */
/* 				NULL, */
/* 				(tfuiCallback)NULL, */
/* 				(tfuiCallback)NULL); */

    if (i < nbCars) {
	RmNextRace.prevHdle = prevHdle;
	RmNextRace.info     = info;
	RmNextRace.start    = start + MAX_LINES;
	GfuiGrButtonCreate(rmScrHdle, "data/img/arrow-down.png", "data/img/arrow-down.png",
			   "data/img/arrow-down.png", "data/img/arrow-down-pushed.png",
			   540, 40, GFUI_ALIGN_HL_VB, 1,
			   (void*)&RmNextRace, rmChgRaceScreen,
			   NULL, (tfuiCallback)NULL, (tfuiCallback)NULL);
	GfuiAddSKey(rmScrHdle, GLUT_KEY_PAGE_DOWN, "Next Results", (void*)&RmNextRace, rmChgRaceScreen, NULL);
    }

    GfuiAddKey(rmScrHdle, (unsigned char)27, "", prevHdle, GfuiScreenReplace, NULL);
    GfuiAddKey(rmScrHdle, (unsigned char)13, "", prevHdle, GfuiScreenReplace, NULL);
    GfuiAddSKey(rmScrHdle, GLUT_KEY_F12, "Take a Screen Shot", NULL, GfuiScreenShot, NULL);

    GfuiScreenActivate(rmScrHdle);
}

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
    sprintf(buf, "Qualification Results");
    GfuiTitleCreate(rmScrHdle, buf, strlen(buf));
    sprintf(buf, "%s", info->track->name);
    GfuiLabelCreate(rmScrHdle, buf, GFUI_FONT_LARGE_C,
		    320, 420, GFUI_ALIGN_HC_VB, 0);
    GfuiScreenAddBgImg(rmScrHdle, "data/img/splash-result.png");

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

	GfuiLabelCreate(rmScrHdle, GfParmGetStr(results, path, RE_ATTR_NAME, ""), GFUI_FONT_MEDIUM_C,
			xDriver, y, GFUI_ALIGN_HL_VB, 0);
	rmdGetDriverType(GfParmGetStr(results, path, RE_ATTR_MODULE, ""), buf, maxBufSize);
	GfuiLabelCreate(rmScrHdle, buf, GFUI_FONT_MEDIUM_C,
			xType, y, GFUI_ALIGN_HL_VB, 0);
	GfuiLabelCreate(rmScrHdle, GfParmGetStr(results, path, RE_ATTR_CAR, ""), GFUI_FONT_MEDIUM_C,
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
	GfuiGrButtonCreate(rmScrHdle, "data/img/arrow-up.png", "data/img/arrow-up.png",
			   "data/img/arrow-up.png", "data/img/arrow-up-pushed.png",
			   80, 40, GFUI_ALIGN_HL_VB, 1,
			   (void*)&RmPrevRace, rmChgQualifScreen,
			   NULL, (tfuiCallback)NULL, (tfuiCallback)NULL);
	GfuiAddSKey(rmScrHdle, GLUT_KEY_PAGE_UP,   "Previous Results", (void*)&RmPrevRace, rmChgQualifScreen, NULL);
    }

    GfuiButtonCreate(rmScrHdle,
		     "Continue",
		     GFUI_FONT_LARGE,
		     320,
		     40,
		     150,
		     GFUI_ALIGN_HC_VB,
		     0,
		     prevHdle,
		     GfuiScreenReplace,
		     NULL,
		     (tfuiCallback)NULL,
		     (tfuiCallback)NULL);

/*     rmSaveId = GfuiButtonCreate(rmScrHdle, */
/* 				"Save", */
/* 				GFUI_FONT_LARGE, */
/* 				430, */
/* 				40, */
/* 				150, */
/* 				GFUI_ALIGN_HC_VB, */
/* 				0, */
/* 				info, */
/* 				rmSaveRes, */
/* 				NULL, */
/* 				(tfuiCallback)NULL, */
/* 				(tfuiCallback)NULL); */

    if (i < nbCars) {
	RmNextRace.prevHdle = prevHdle;
	RmNextRace.info     = info;
	RmNextRace.start    = start + MAX_LINES;
	GfuiGrButtonCreate(rmScrHdle, "data/img/arrow-down.png", "data/img/arrow-down.png",
			   "data/img/arrow-down.png", "data/img/arrow-down-pushed.png",
			   540, 40, GFUI_ALIGN_HL_VB, 1,
			   (void*)&RmNextRace, rmChgQualifScreen,
			   NULL, (tfuiCallback)NULL, (tfuiCallback)NULL);
	GfuiAddSKey(rmScrHdle, GLUT_KEY_PAGE_DOWN, "Next Results", (void*)&RmNextRace, rmChgQualifScreen, NULL);
    }

    GfuiAddKey(rmScrHdle, (unsigned char)27, "", prevHdle, GfuiScreenReplace, NULL);
    GfuiAddKey(rmScrHdle, (unsigned char)13, "", prevHdle, GfuiScreenReplace, NULL);
    GfuiAddSKey(rmScrHdle, GLUT_KEY_F12, "Take a Screen Shot", NULL, GfuiScreenShot, NULL);

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
    sprintf(buf, "%s Standings", race);
    GfuiTitleCreate(rmScrHdle, buf, strlen(buf));

    GfuiScreenAddBgImg(rmScrHdle, "data/img/splash-result.png");

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
    
    nbCars = (int)GfParmGetEltNb(results, RE_SECT_STANDINGS);
    for (i = start; i < MIN(start + MAX_LINES, nbCars); i++) {
	sprintf(path, "%s/%d", RE_SECT_STANDINGS, i + 1);

	sprintf(buf, "%d", i+1);
	GfuiLabelCreate(rmScrHdle, buf, GFUI_FONT_MEDIUM_C,
			xRank, y, GFUI_ALIGN_HC_VB, 0);

	GfuiLabelCreate(rmScrHdle, GfParmGetStr(results, path, RE_ATTR_NAME, ""), GFUI_FONT_MEDIUM_C,
			xDriver, y, GFUI_ALIGN_HL_VB, 0);
	rmdGetDriverType(GfParmGetStr(results, path, RE_ATTR_MODULE, ""), buf, maxBufSize);
	GfuiLabelCreate(rmScrHdle, buf, GFUI_FONT_MEDIUM_C,
			xType, y, GFUI_ALIGN_HL_VB, 0);
	GfuiLabelCreate(rmScrHdle, GfParmGetStr(results, path, RE_ATTR_CAR, ""), GFUI_FONT_MEDIUM_C,
			xCar, y, GFUI_ALIGN_HL_VB, 0);

	sprintf(buf, "%d", (int)GfParmGetNum(results, path, RE_ATTR_POINTS, NULL, 0));
	GfuiLabelCreate(rmScrHdle, buf, GFUI_FONT_MEDIUM_C,
			xPoints, y, GFUI_ALIGN_HR_VB, 0);
	y -= 15;
    }


    if (start > 0) {
	RmPrevRace.prevHdle = prevHdle;
	RmPrevRace.info     = info;
	RmPrevRace.start    = start - MAX_LINES;
	GfuiGrButtonCreate(rmScrHdle, "data/img/arrow-up.png", "data/img/arrow-up.png",
			   "data/img/arrow-up.png", "data/img/arrow-up-pushed.png",
			   80, 40, GFUI_ALIGN_HL_VB, 1,
			   (void*)&RmPrevRace, rmChgStandingScreen,
			   NULL, (tfuiCallback)NULL, (tfuiCallback)NULL);
	GfuiAddSKey(rmScrHdle, GLUT_KEY_PAGE_UP,   "Previous Results", (void*)&RmPrevRace, rmChgStandingScreen, NULL);
    }

    GfuiButtonCreate(rmScrHdle, "Continue", GFUI_FONT_LARGE, 210, 40, 150, GFUI_ALIGN_HC_VB, 0,
		     prevHdle, GfuiScreenReplace,
		     NULL, (tfuiCallback)NULL, (tfuiCallback)NULL);

	rmSaveId = GfuiButtonCreate(rmScrHdle, "Save", GFUI_FONT_LARGE, 430, 40, 150,
				    GFUI_ALIGN_HC_VB, 0, info, rmSaveRes, 
				    NULL, (tfuiCallback)NULL, (tfuiCallback)NULL);

    if (i < nbCars) {
	RmNextRace.prevHdle = prevHdle;
	RmNextRace.info     = info;
	RmNextRace.start    = start + MAX_LINES;
	GfuiGrButtonCreate(rmScrHdle, "data/img/arrow-down.png", "data/img/arrow-down.png",
			   "data/img/arrow-down.png", "data/img/arrow-down-pushed.png",
			   540, 40, GFUI_ALIGN_HL_VB, 1,
			   (void*)&RmNextRace, rmChgStandingScreen,
			   NULL, (tfuiCallback)NULL, (tfuiCallback)NULL);
	GfuiAddSKey(rmScrHdle, GLUT_KEY_PAGE_DOWN, "Next Results", (void*)&RmNextRace, rmChgStandingScreen, NULL);
    }

    GfuiAddKey(rmScrHdle, (unsigned char)27, "", prevHdle, GfuiScreenReplace, NULL);
    GfuiAddKey(rmScrHdle, (unsigned char)13, "", prevHdle, GfuiScreenReplace, NULL);
    GfuiAddSKey(rmScrHdle, GLUT_KEY_F12, "Take a Screen Shot", NULL, GfuiScreenShot, NULL);

    GfuiScreenActivate(rmScrHdle);
}

void
RmShowResults(void *prevHdle, tRmInfo *info)
{
    switch (info->s->_raceType) {
    case RM_TYPE_PRACTICE:
	rmPracticeResults(prevHdle, info, 0);
	return;

    case RM_TYPE_RACE:
	rmRaceResults(prevHdle, info, 0);
	return;

    case RM_TYPE_QUALIF:
	rmQualifResults(prevHdle, info, 0);
	return;
    }
}


void
RmShowStandings(void *prevHdle, tRmInfo *info)
{
    rmShowStandings(prevHdle, info, 0);
}
