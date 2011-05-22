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

#include <portability.h>
#include <tgfclient.h>

#include "racescreens.h"


static int	rmSaveId;
static void	*rmScrHdle = NULL;

static void rmPracticeResults(void *prevHdle, tRmInfo *info, int start);
static void rmRaceResults(void *prevHdle, tRmInfo *info, int start);
static void rmQualifResults(void *prevHdle, tRmInfo *info, int start);

static int NMaxResultLines = 0;	//Max number of result lines in the table (header excluded)

static int NLastDamage = 0;

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

void rmGetDriverType(const char* moduleName, char* driverType, size_t maxSize)
{
    char* pos;

    strncpy(driverType, moduleName, maxSize);
    driverType[maxSize-1] = 0; // Ensure 0 termination

    // Parse module name for last '_' char : 
    // assumed to be the separator between type and instance name for ubiquitous robots (ex: simplix)
    pos = strrchr(driverType, '_');
    if (pos)
		*pos = 0;

	// Commented-out because it shylessly truncates kilo2008's name ...
	// but we can assume we won't have anymore those old-patterned robots.
    // Otherwise, search for an isolated last digit in the module name :
    // old robot with hard-coded max cars of 10 may follow this pattern (ex: berniw2 and 3)
//     else
//     {
// 		pos = driverType + strlen(driverType) - 1;
// 		while (pos != driverType && isdigit(*pos))
// 			pos--;
// 		if (++pos == driverType + strlen(driverType) - 1)
// 			*pos = 0;
//     }
}

static void
rmPracticeResults(void *prevHdle, tRmInfo *info, int start)
{
    void		*results = info->results;
    const char		*race = info->_reRaceName;
    int			i;
    int			y;
    static char		buf[256];
    static char		path[1024];
    char		*str;
    int			totLaps;
    int 		damage; 

    // Create screen, load menu XML descriptor and create static controls.
    rmScrHdle = GfuiScreenCreate();

	GfLogTrace("Entering Practice Results menu\n");

    void *hmenu = GfuiMenuLoad("practiceresultsmenu.xml");
    GfuiMenuCreateStaticControls(hmenu, rmScrHdle);

    // Create variable title labels.
    snprintf(buf, sizeof(buf), "Practice Results on %s", info->track->name);
    const int titleId = GfuiMenuCreateLabelControl(rmScrHdle, hmenu, "Title");
    GfuiLabelSetText(rmScrHdle, titleId, buf);
 
    snprintf(path, sizeof(path), "%s/%s/%s", info->track->name, RE_SECT_RESULTS, race);
    snprintf(buf, sizeof(buf), "%s (%s)", GfParmGetStr(results, path, RM_ATTR_DRVNAME, NULL),
			 GfParmGetStr(results, path, RM_ATTR_CAR, NULL));

    const int subTitleId = GfuiMenuCreateLabelControl(rmScrHdle, hmenu, "PlayerTitle");
    GfuiLabelSetText(rmScrHdle, subTitleId, buf);
 
	// Get layout properties.
    NMaxResultLines = (int)GfuiMenuGetNumProperty(hmenu, "nMaxResultLines", 15);
    const int yTopLine = (int)GfuiMenuGetNumProperty(hmenu, "yTopLine", 400);
    const int yLineShift = (int)GfuiMenuGetNumProperty(hmenu, "yLineShift", 20);

	// Reset last damage value if top of the table.
	if (start == 0)
		NLastDamage = 0; 
	
	// Display the result table.
    y = yTopLine;
    
    sprintf(path, "%s/%s/%s", info->track->name, RE_SECT_RESULTS, race);
    totLaps = (int)GfParmGetEltNb(results, path);
    for (i = 0 + start; i < MIN(start + NMaxResultLines, totLaps); i++) {
		sprintf(path, "%s/%s/%s/%d", info->track->name, RE_SECT_RESULTS, race, i + 1);

		/* Lap */
		sprintf(buf, "%d", i+1);
		GfuiMenuCreateLabelControl(rmScrHdle, hmenu, "LapNumber", true, // From template.
								   buf, GFUI_TPL_X, y);

		/* Time */
		str = GfTime2Str(GfParmGetNum(results, path, RE_ATTR_TIME, NULL, 0), "  ", false, 2);;
		GfuiMenuCreateLabelControl(rmScrHdle, hmenu, "LapTime", true, // From template.
								   str, GFUI_TPL_X, y);
		free(str);

		/* Best Lap Time */
		str = GfTime2Str(GfParmGetNum(results, path, RE_ATTR_BEST_LAP_TIME, NULL, 0), "  ", false, 2);;
		GfuiMenuCreateLabelControl(rmScrHdle, hmenu, "BestTime", true, // From template.
								   str, GFUI_TPL_X, y);
		free(str);

		/* Top Spd */
		sprintf(buf, "%d", (int)(GfParmGetNum(results, path, RE_ATTR_TOP_SPEED, NULL, 0) * 3.6));
		GfuiMenuCreateLabelControl(rmScrHdle, hmenu, "TopSpeed", true, // From template.
								   buf, GFUI_TPL_X, y);

		/* Min Spd */
		sprintf(buf, "%d", (int)(GfParmGetNum(results, path, RE_ATTR_BOT_SPEED, NULL, 0) * 3.6));
		GfuiMenuCreateLabelControl(rmScrHdle, hmenu, "MinSpeed", true, // From template.
								   buf, GFUI_TPL_X, y);

		/* Damages */
		damage =  (int)(GfParmGetNum(results, path, RE_ATTR_DAMMAGES, NULL, 0)); 
		sprintf(buf, "%d (%d)", damage - NLastDamage, damage); 
		GfuiMenuCreateLabelControl(rmScrHdle, hmenu, "Damages", true, // From template.
								   buf, GFUI_TPL_X, y);
		NLastDamage = damage; 

		y -= yLineShift;
    }

    if (start > 0) {
		RmPrevRace.prevHdle = prevHdle;
		RmPrevRace.info     = info;
		RmPrevRace.start    = start - NMaxResultLines;
		GfuiMenuCreateButtonControl(rmScrHdle, hmenu, "previouspagearrow",
									(void*)&RmPrevRace, rmChgPracticeScreen);
		GfuiAddKey(rmScrHdle, GFUIK_PAGEUP,   "Previous Results", (void*)&RmPrevRace, rmChgPracticeScreen, NULL);
    }
    
    // Add "Continue" button
    GfuiMenuCreateButtonControl(rmScrHdle, hmenu, "continuebutton", prevHdle, GfuiScreenReplace);
    
    //Create 'save' button in the bottom right
    //rmSaveId = GfuiMenuCreateButtonControl(rmScrHdle, hmenu, "savebutton", info, rmSaveRes);
    
    if (i < totLaps) {
		RmNextRace.prevHdle = prevHdle;
		RmNextRace.info     = info;
		RmNextRace.start    = start + NMaxResultLines;
		GfuiMenuCreateButtonControl(rmScrHdle, hmenu, "nextpagearrow",
									(void*)&RmNextRace, rmChgPracticeScreen);
		GfuiAddKey(rmScrHdle, GFUIK_PAGEDOWN, "Next Results", (void*)&RmNextRace, rmChgPracticeScreen, NULL);
    }

    GfuiAddKey(rmScrHdle, GFUIK_ESCAPE, "Continue", prevHdle, GfuiScreenReplace, NULL);
    GfuiAddKey(rmScrHdle, GFUIK_RETURN, "Continue", prevHdle, GfuiScreenReplace, NULL);
    GfuiAddKey(rmScrHdle, GFUIK_F12, "Take a Screen Shot", NULL, GfuiScreenShot, NULL);
    GfuiAddKey(rmScrHdle, GFUIK_F1, "Help", rmScrHdle, GfuiHelpScreen, NULL);

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
    static char		buf[256];
    static char		path[512];
    char		*str;
    static float	fgcolor[4] = {1.0, 0.0, 1.0, 1.0};
    static float  green[4] = {0.196, 0.804, 0.196, 1.0};//Lime green, #32CD32
    static float	orange[4] = {0.953, 0.518, 0.0, 1.0};//Tangerine, #F28500
    static float	white[4] = {1.0, 1.0, 1.0, 1.0};
    
	GfLogTrace("Entering Race Results menu\n");

    //Screen title
    rmScrHdle = GfuiScreenCreate();
    void *hmenu = GfuiMenuLoad("raceresultsmenu.xml");
    GfuiMenuCreateStaticControls(hmenu,rmScrHdle);

    sprintf(buf, "%s", info->track->name);
    const int subTitleId = GfuiMenuCreateLabelControl(rmScrHdle, hmenu, "racetitle");
    GfuiLabelSetText(rmScrHdle, subTitleId, buf);

  
    //Column positions
    const int xRank = 10;
    const int xAdv = 35;
    const int xDriver = 55;
    const int xType = 195;
    const int xCar  = 255;
    const int xTotal = 435;
    const int xBest = 500;
    const int xLaps = 525;
    const int xTSpd = 555;
    const int xDamg = 595;
    const int xPStp = 625;
    
    //Heading
    int y = 400;
    GfuiLabelCreate(rmScrHdle, "Rk",     GFUI_FONT_MEDIUM_C, xRank, y, GFUI_ALIGN_HC_VB, 0, fgcolor);
    GfuiLabelCreate(rmScrHdle, "Adv",    GFUI_FONT_MEDIUM_C, xAdv, y, GFUI_ALIGN_HC_VB, 0, fgcolor);
    GfuiLabelCreate(rmScrHdle, "Driver", GFUI_FONT_MEDIUM_C, xDriver+10, y, GFUI_ALIGN_HL_VB, 0, fgcolor);
    GfuiLabelCreate(rmScrHdle, "Type",   GFUI_FONT_MEDIUM_C, xType+10, y, GFUI_ALIGN_HL_VB, 0, fgcolor);
    GfuiLabelCreate(rmScrHdle, "Car",    GFUI_FONT_MEDIUM_C, xCar+10, y, GFUI_ALIGN_HL_VB, 0, fgcolor);
    GfuiLabelCreate(rmScrHdle, "Total",  GFUI_FONT_MEDIUM_C, xTotal-15, y, GFUI_ALIGN_HR_VB, 0, fgcolor);
    GfuiLabelCreate(rmScrHdle, "Best",   GFUI_FONT_MEDIUM_C, xBest-15, y, GFUI_ALIGN_HR_VB, 0, fgcolor);
    GfuiLabelCreate(rmScrHdle, "Laps",   GFUI_FONT_MEDIUM_C, xLaps-10, y, GFUI_ALIGN_HC_VB, 0, fgcolor);
    GfuiLabelCreate(rmScrHdle, "T.Sp.",  GFUI_FONT_MEDIUM_C, xTSpd-5, y, GFUI_ALIGN_HC_VB, 0, fgcolor);
    GfuiLabelCreate(rmScrHdle, "Dam.",   GFUI_FONT_MEDIUM_C, xDamg-5, y, GFUI_ALIGN_HC_VB, 0, fgcolor);
    GfuiLabelCreate(rmScrHdle, "Pits",   GFUI_FONT_MEDIUM_C, xPStp, y, GFUI_ALIGN_HC_VB, 0, fgcolor);
    y -= 20;

	// Never used : remove ?
    //Get total laps, winner time
    //sprintf(path, "%s/%s/%s", info->track->name, RE_SECT_RESULTS, race);
    //int totLaps = (int)GfParmGetNum(results, path, RE_ATTR_LAPS, NULL, 0);
    //sprintf(path, "%s/%s/%s/%s/%d", info->track->name, RE_SECT_RESULTS, race, RE_SECT_RANK, 1);
    //tdble refTime = GfParmGetNum(results, path, RE_ATTR_TIME, NULL, 0);

    //Get number of cars
    sprintf(path, "%s/%s/%s/%s", info->track->name, RE_SECT_RESULTS, race, RE_SECT_RANK);
    int nbCars = (int)GfParmGetEltNb(results, path);
    
    int i;
    for (i = start; i < MIN(start + NMaxResultLines, nbCars); i++) {
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
        GfuiLabelCreate(rmScrHdle, buf, GFUI_FONT_MEDIUM_C, xAdv, y, GFUI_ALIGN_HC_VB, 0, c);

        //Driver name
        GfuiLabelCreate(rmScrHdle, GfParmGetStr(results, path, RE_ATTR_NAME, ""), GFUI_FONT_MEDIUM_C,
        xDriver, y, GFUI_ALIGN_HL_VB, 0);

        //Driver type
        rmGetDriverType(GfParmGetStr(results, path, RE_ATTR_MODULE, ""), buf, sizeof(buf));
        GfuiLabelCreate(rmScrHdle, buf, GFUI_FONT_MEDIUM_C, xType, y, GFUI_ALIGN_HL_VB, 0);

        //Car
        GfuiLabelCreate(rmScrHdle, GfParmGetStr(results, path, RE_ATTR_CAR, ""), GFUI_FONT_MEDIUM_C,
        xCar, y, GFUI_ALIGN_HL_VB, 0);

        GfuiLabelCreate(rmScrHdle, GfParmGetStr(results, path, RE_ATTR_NAME, NULL), GFUI_FONT_MEDIUM_C, xDriver, y, GFUI_ALIGN_HL_VB, 0);
        rmGetDriverType(GfParmGetStr(results, path, RE_ATTR_MODULE, NULL), buf, sizeof(buf));
        GfuiLabelCreate(rmScrHdle, buf, GFUI_FONT_MEDIUM_C, xType, y, GFUI_ALIGN_HL_VB, 0);
        GfuiLabelCreate(rmScrHdle, GfParmGetStr(results, path, RE_ATTR_CAR, NULL), GFUI_FONT_MEDIUM_C,
      xCar, y, GFUI_ALIGN_HL_VB, 0);

        //Total Time 
        str = GfTime2Str(GfParmGetNum(results, path, RE_ATTR_TIME, NULL, 0), "  ", false, 2); 
        GfuiLabelCreate(rmScrHdle, str, GFUI_FONT_MEDIUM_C,  xTotal, y, GFUI_ALIGN_HR_VB, 0); 
        free(str);
        
        //Best lap
        str = GfTime2Str(GfParmGetNum(results, path, RE_ATTR_BEST_LAP_TIME, NULL, 0), "  ", false, 2);
        GfuiLabelCreate(rmScrHdle, str, GFUI_FONT_MEDIUM_C,  xBest, y, GFUI_ALIGN_HR_VB, 0);
        free(str);
        
        //Laps covered
        sprintf(buf, "%d", laps);
        GfuiLabelCreate(rmScrHdle, buf, GFUI_FONT_MEDIUM_C,  xLaps, y, GFUI_ALIGN_HC_VB, 0);
        
        //Top speed
        sprintf(buf, "%d", (int)(GfParmGetNum(results, path, RE_ATTR_TOP_SPEED, NULL, 0) * 3.6));
        GfuiLabelCreate(rmScrHdle, buf, GFUI_FONT_MEDIUM_C,  xTSpd, y, GFUI_ALIGN_HC_VB, 0);
        
        //Damage
        sprintf(buf, "%d", (int)(GfParmGetNum(results, path, RE_ATTR_DAMMAGES, NULL, 0)));
        GfuiLabelCreate(rmScrHdle, buf, GFUI_FONT_MEDIUM_C,  xDamg, y, GFUI_ALIGN_HC_VB, 0);
        
        //Pitstops
        sprintf(buf, "%d", (int)(GfParmGetNum(results, path, RE_ATTR_NB_PIT_STOPS, NULL, 0)));
        GfuiLabelCreate(rmScrHdle, buf, GFUI_FONT_MEDIUM_C,  xPStp, y, GFUI_ALIGN_HC_VB, 0);
        
        y -= 15;  //Line feed
    }//for i

    //If it is not the first screen of the results, show a 'Prev' button
    if (start > 0) {
        RmPrevRace.prevHdle = prevHdle;
        RmPrevRace.info     = info;
        RmPrevRace.start    = start - NMaxResultLines;
        GfuiMenuCreateButtonControl(rmScrHdle, hmenu, "previouspagearrow",
          (void*)&RmPrevRace, rmChgRaceScreen);
        GfuiAddKey(rmScrHdle, GFUIK_PAGEUP,   "Previous Results", (void*)&RmPrevRace, rmChgRaceScreen, NULL);
    }//if start

    // Add "Continue" button
    GfuiMenuCreateButtonControl(rmScrHdle, hmenu, "continuebutton", prevHdle, GfuiScreenReplace);
    
    //Create 'save' button in the bottom right
    //rmSaveId = GfuiMenuCreateButtonControl(rmScrHdle, hmenu, "savebutton", info, rmSaveRes);

    //If we did not display all the results yet, let's show a 'Next' button
    if (i < nbCars) {
        RmNextRace.prevHdle = prevHdle;
        RmNextRace.info     = info;
        RmNextRace.start    = start + NMaxResultLines;
        GfuiMenuCreateButtonControl(rmScrHdle, hmenu, "nextpagearrow", (void*)&RmNextRace, rmChgRaceScreen);
        GfuiAddKey(rmScrHdle, GFUIK_PAGEDOWN, "Next Results", (void*)&RmNextRace, rmChgRaceScreen, NULL);
    }//if i

    //Link key handlers
    GfuiAddKey(rmScrHdle, GFUIK_ESCAPE, "Continue", prevHdle, GfuiScreenReplace, NULL);
    GfuiAddKey(rmScrHdle, GFUIK_RETURN, "Continue", prevHdle, GfuiScreenReplace, NULL);
    GfuiAddKey(rmScrHdle, GFUIK_F12, "Take a Screen Shot", NULL, GfuiScreenShot, NULL);
    GfuiAddKey(rmScrHdle, GFUIK_F1, "Help", rmScrHdle, GfuiHelpScreen, NULL);

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
    static char		buf[256];
    static char		path[512];
    char		*str;
    static float	fgcolor[4] = {1.0, 0.0, 1.0, 1.0};
    int			laps, totLaps;
    tdble		refTime;
    int			nbCars;

	GfLogTrace("Entering Qualification Results menu\n");

    rmScrHdle = GfuiScreenCreate();
    void *hmenu = GfuiMenuLoad("qualifsresultsmenu.xml");
    GfuiMenuCreateStaticControls(hmenu,rmScrHdle);

    sprintf(buf, "%s", info->track->name);
    const int subTitleId = GfuiMenuCreateLabelControl(rmScrHdle, hmenu, "racetitle");
    GfuiLabelSetText(rmScrHdle, subTitleId, buf);

    const int offset  = 50;
    const int xRank   = offset + 30;
    const int xDriver = offset + 60;
    const int xType   = offset + 240;
    const int xCar    = offset + 320;
    const int xTime   = offset + 520;

    y = 400;
    GfuiLabelCreate(rmScrHdle, "Rank",   GFUI_FONT_MEDIUM_C, xRank, y, GFUI_ALIGN_HC_VB, 0, fgcolor);
    GfuiLabelCreate(rmScrHdle, "Driver", GFUI_FONT_MEDIUM_C, xDriver+10, y, GFUI_ALIGN_HL_VB, 0, fgcolor);
    GfuiLabelCreate(rmScrHdle, "Type",   GFUI_FONT_MEDIUM_C, xType+10, y, GFUI_ALIGN_HL_VB, 0, fgcolor);
    GfuiLabelCreate(rmScrHdle, "Car",    GFUI_FONT_MEDIUM_C, xCar+10, y, GFUI_ALIGN_HL_VB, 0, fgcolor);
    GfuiLabelCreate(rmScrHdle, "Time",   GFUI_FONT_MEDIUM_C, xTime, y, GFUI_ALIGN_HR_VB, 0, fgcolor);
    y -= 20;
    
    sprintf(path, "%s/%s/%s", info->track->name, RE_SECT_RESULTS, race);
    totLaps = (int)GfParmGetNum(results, path, RE_ATTR_LAPS, NULL, 0);
    sprintf(path, "%s/%s/%s/%s/%d", info->track->name, RE_SECT_RESULTS, race, RE_SECT_RANK, 1);
    refTime = GfParmGetNum(results, path, RE_ATTR_TIME, NULL, 0);
    sprintf(path, "%s/%s/%s/%s", info->track->name, RE_SECT_RESULTS, race, RE_SECT_RANK);
    nbCars = (int)GfParmGetEltNb(results, path);
    for (i = start; i < MIN(start + NMaxResultLines, nbCars); i++) {
	sprintf(path, "%s/%s/%s/%s/%d", info->track->name, RE_SECT_RESULTS, race, RE_SECT_RANK, i + 1);
	laps = (int)GfParmGetNum(results, path, RE_ATTR_LAPS, NULL, 0);

	sprintf(buf, "%d", i+1);
	GfuiLabelCreate(rmScrHdle, buf, GFUI_FONT_MEDIUM_C, xRank, y, GFUI_ALIGN_HC_VB, 0);

	GfuiLabelCreate(rmScrHdle, GfParmGetStr(results, path, RE_ATTR_NAME, NULL), GFUI_FONT_MEDIUM_C,
			xDriver, y, GFUI_ALIGN_HL_VB, 0);
	rmGetDriverType(GfParmGetStr(results, path, RE_ATTR_MODULE, NULL), buf, sizeof(buf));
	GfuiLabelCreate(rmScrHdle, buf, GFUI_FONT_MEDIUM_C,
			xType, y, GFUI_ALIGN_HL_VB, 0);
	GfuiLabelCreate(rmScrHdle, GfParmGetStr(results, path, RE_ATTR_CAR, NULL), GFUI_FONT_MEDIUM_C,
			xCar, y, GFUI_ALIGN_HL_VB, 0);

	str = GfTime2Str(GfParmGetNum(results, path, RE_ATTR_BEST_LAP_TIME, NULL, 0), "  ", false, 2);
	GfuiLabelCreate(rmScrHdle, str, GFUI_FONT_MEDIUM_C,
			xTime, y, GFUI_ALIGN_HR_VB, 0);
	free(str);
	y -= 15;
    }


    if (start > 0) {
	RmPrevRace.prevHdle = prevHdle;
	RmPrevRace.info     = info;
	RmPrevRace.start    = start - NMaxResultLines;
	GfuiMenuCreateButtonControl(rmScrHdle, hmenu, "previouspagearrow",
			    (void*)&RmPrevRace, rmChgQualifScreen);
	GfuiAddKey(rmScrHdle, GFUIK_PAGEUP,   "Previous Results", (void*)&RmPrevRace, rmChgQualifScreen, NULL);
    }

    // Add "Continue" button 
    GfuiMenuCreateButtonControl(rmScrHdle, hmenu, "continuebutton", prevHdle, GfuiScreenReplace);
    
    //Create 'save' button in the bottom right
    //rmSaveId = GfuiMenuCreateButtonControl(rmScrHdle, hmenu, "savebutton", info, rmSaveRes);

    if (i < nbCars) {
	RmNextRace.prevHdle = prevHdle;
	RmNextRace.info     = info;
	RmNextRace.start    = start + NMaxResultLines;
	GfuiMenuCreateButtonControl(rmScrHdle, hmenu, "nextpagearrow",
			    (void*)&RmNextRace, rmChgQualifScreen);
	GfuiAddKey(rmScrHdle, GFUIK_PAGEDOWN, "Next Results", (void*)&RmNextRace, rmChgQualifScreen, NULL);
    }

    GfuiAddKey(rmScrHdle, GFUIK_ESCAPE, "Continue", prevHdle, GfuiScreenReplace, NULL);
    GfuiAddKey(rmScrHdle, GFUIK_RETURN, "Continue", prevHdle, GfuiScreenReplace, NULL);
    GfuiAddKey(rmScrHdle, GFUIK_F12, "Take a Screen Shot", NULL, GfuiScreenShot, NULL);
    GfuiAddKey(rmScrHdle, GFUIK_F1, "Help", rmScrHdle, GfuiHelpScreen, NULL);

    GfuiScreenActivate(rmScrHdle);
}

static void
rmChgStandingScreen(void *vprc)
{
    void		*prevScr = rmScrHdle;
    tRaceCall 	*prc = (tRaceCall*)vprc;

    RmShowStandings(prc->prevHdle, prc->info, prc->start);
    GfuiScreenRelease(prevScr);
}

/** 
 * RmShowStandings
 * 
 * Shows a results page, with optional prev/next results page buttons
 * 
 * @param prevHdle	handle for previous results page
 * @param info	race results information
 * @param start	page number
*/
void
RmShowStandings(void *prevHdle, tRmInfo *info, int start)
{
	int			i;
	int			y;
	static char		buf[256];
	static char		path[512];
	static float	fgcolor[4] = {1.0, 0.0, 1.0, 1.0};
	int			nbCars;
	void		*results = info->results;
	const char		*race = info->_reRaceName;

	GfLogTrace("Entering Standings menu\n");

	rmScrHdle = GfuiScreenCreate();

	void *hmenu = GfuiMenuLoad("standingsmenu.xml");
	GfuiMenuCreateStaticControls(hmenu,rmScrHdle);

	//Set title
	sprintf(buf, "%s Standings", race);
	const int subTitleId = GfuiMenuCreateLabelControl(rmScrHdle, hmenu, "racetitle");
	GfuiLabelSetText(rmScrHdle, subTitleId, buf);

	//Show header
	const int offset  = 50;
	const int xRank   = offset + 30;
	const int xDriver = offset + 60;
	const int xType   = offset + 240;
	const int xCar    = offset + 320;
	const int xPoints = offset + 520;
	y = 400;
	GfuiLabelCreate(rmScrHdle, "Rank",   GFUI_FONT_MEDIUM_C, xRank, y, GFUI_ALIGN_HC_VB, 0, fgcolor);
	GfuiLabelCreate(rmScrHdle, "Driver", GFUI_FONT_MEDIUM_C, xDriver+10, y, GFUI_ALIGN_HL_VB, 0, fgcolor);
	GfuiLabelCreate(rmScrHdle, "Type",   GFUI_FONT_MEDIUM_C, xType+10, y, GFUI_ALIGN_HL_VB, 0, fgcolor);
	GfuiLabelCreate(rmScrHdle, "Car",    GFUI_FONT_MEDIUM_C, xCar+10, y, GFUI_ALIGN_HL_VB, 0, fgcolor);
	GfuiLabelCreate(rmScrHdle, "Points", GFUI_FONT_MEDIUM_C, xPoints, y, GFUI_ALIGN_HR_VB, 0, fgcolor);
	y -= 20;

	//List results line by line, paginated
	nbCars = (int)GfParmGetEltNb(results, (char*)RE_SECT_STANDINGS);
	for (i = start; i < MIN(start + NMaxResultLines, nbCars); i++) {
		sprintf(path, "%s/%d", RE_SECT_STANDINGS, i + 1);
		
		//Rank
		sprintf(buf, "%d", i+1);
		GfuiLabelCreate(rmScrHdle, buf, GFUI_FONT_MEDIUM_C,
			xRank, y, GFUI_ALIGN_HC_VB, 0);

		//Driver name
		GfuiLabelCreate(rmScrHdle, GfParmGetStr(results, path, RE_ATTR_NAME, NULL), GFUI_FONT_MEDIUM_C,
			xDriver, y, GFUI_ALIGN_HL_VB, 0);
	
		//Driver type
		rmGetDriverType(GfParmGetStr(results, path, RE_ATTR_MODULE, NULL), buf, sizeof(buf));
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
		RmPrevRace.start    = start - NMaxResultLines;
		GfuiMenuCreateButtonControl(rmScrHdle, hmenu, "previouspagearrow",
				    (void*)&RmPrevRace, rmChgStandingScreen);
		GfuiAddKey(rmScrHdle, GFUIK_PAGEUP, "Previous Results", (void*)&RmPrevRace, rmChgStandingScreen, NULL);
	}//if start

	// Add "Continue" button in the bottom left
	GfuiMenuCreateButtonControl(rmScrHdle, hmenu, "continuebutton", prevHdle, GfuiScreenReplace);
    
	//Create 'save' button in the bottom right if ... not Career mode.
	if (!strcmp( GfParmGetStr( info->mainParams, RM_SECT_SUBFILES, RM_ATTR_HASSUBFILES, RM_VAL_NO ), RM_VAL_YES ) == 0) {
	    rmSaveId = GfuiMenuCreateButtonControl(rmScrHdle, hmenu, "savebutton", info, rmSaveRes);
	}

	//If there is a next page, show 'next results' button on the bottom extreme right
	if (i < nbCars) {
		RmNextRace.prevHdle = prevHdle;
		RmNextRace.info     = info;
		RmNextRace.start    = start + NMaxResultLines;
		GfuiMenuCreateButtonControl(rmScrHdle, hmenu, "nextpagearrow",
				    (void*)&RmNextRace, rmChgStandingScreen);
		GfuiAddKey(rmScrHdle, GFUIK_PAGEDOWN, "Next Results", (void*)&RmNextRace, rmChgStandingScreen, NULL);
	}//if i

    GfuiAddKey(rmScrHdle, GFUIK_ESCAPE, "Continue", prevHdle, GfuiScreenReplace, NULL);
    GfuiAddKey(rmScrHdle, GFUIK_RETURN, "Continue", prevHdle, GfuiScreenReplace, NULL);
    GfuiAddKey(rmScrHdle, GFUIK_F1, "Help", rmScrHdle, GfuiHelpScreen, NULL);
    GfuiAddKey(rmScrHdle, GFUIK_F12, "Take a Screen Shot", NULL, GfuiScreenShot, NULL);

    GfuiScreenActivate(rmScrHdle);
}//RmShowStandings


void
RmShowResults(void *prevHdle, tRmInfo *info)
{
    int nCars;
    char buffer[128];

    switch (info->s->_raceType)
	{
		case RM_TYPE_PRACTICE:
			snprintf( buffer, sizeof(buffer), "%s/%s", info->track->name, RE_SECT_DRIVERS );
			nCars = GfParmGetEltNb( info->results, buffer );
			if (nCars == 1)
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
