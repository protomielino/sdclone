/***************************************************************************

    file        : miscscreens.cpp
    created     : Sun Dec  8 13:01:47 CET 2002
    copyright   : (C) 2002 by Eric Espi�                        
    email       : eric.espie@torcs.org   
    version     : $Id: miscscreens.cpp,v 1.2.2.1 2008/05/31 15:52:38 berniw Exp $                                  

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
    		
    @author	<a href=mailto:eric.espie@torcs.org>Eric Espie</a>
    @version	$Id: miscscreens.cpp,v 1.2.2.1 2008/05/31 15:52:38 berniw Exp $
*/

#include <stdio.h>

#include <tgfclient.h>
#include <robot.h>

#include "racescreens.h"


/*********************************************************
 * 2, 3, 4 or 5 buttons "Quit race" screens
 */

static void *twoStateHdle = 0;
static void *triStateHdle = 0;
static void *fourStateHdle = 0;

// Descriptor for 1 button.
typedef struct {
    
    const char *label;  // Label to display.
    const char *tip;    // Tip displayed when mouse hover.
    void       *screen; // Screen to activate if clicked.

} tButtonDesc;

// Generic N state "quit race" screen creation and activation.
static void *
rmNStateScreen(const char *title, const tButtonDesc aButtons[], int nButtons, int nCancelIndex)
{
    void *screenHdle = 0;
	
    // Create screen, load menu XML descriptor and create static controls.
    screenHdle = GfuiScreenCreateEx(NULL, NULL, NULL, NULL, NULL, 1);

    void *menuXMLDescHdle = LoadMenuXML("stopracemenu.xml");

    CreateStaticControls(menuXMLDescHdle, screenHdle);

    // Create variable title label.
    int titleId = CreateLabelControl(screenHdle, menuXMLDescHdle, "titlelabel");
    GfuiLabelSetText(screenHdle, titleId, title);

    // Create specified buttons
    for (int nButInd = 0; nButInd < nButtons; nButInd++)
	GfuiMenuButtonCreate(screenHdle, aButtons[nButInd].label, aButtons[nButInd].tip, 
			     aButtons[nButInd].screen, GfuiScreenActivate);

    // Close menu XML descriptor.
    GfParmReleaseHandle(menuXMLDescHdle);
    
    // Register keyboard shortcuts.
    GfuiMenuDefaultKeysAdd(screenHdle);
    GfuiAddKey(screenHdle, 27, aButtons[nCancelIndex].tip, 
	       aButtons[nCancelIndex].screen, GfuiScreenActivate, NULL);
    GfuiAddSKey(screenHdle, GLUT_KEY_F1, "Help", screenHdle, GfuiHelpScreen, NULL);
    GfuiAddSKey(screenHdle, GLUT_KEY_F12, "Take a Screen Shot", NULL, GfuiScreenShot, NULL);

    // Activate the created screen.
    GfuiScreenActivate(screenHdle);

    return screenHdle;
}

// 2 state "quit race" screen creation and activation.
void *
RmTwoStateScreen(
	const char *title,
	const char *label1, const char *tip1, void *screen1,
	const char *label2, const char *tip2, void *screen2)
{
    static const int nButtons = 2;
    const tButtonDesc aButtons[nButtons]  =
    {
	{ label1, tip1, screen1 },
	{ label2, tip2, screen2 }
    };
	
    if (twoStateHdle) {
	GfuiScreenRelease(twoStateHdle);
    }
	
    twoStateHdle = rmNStateScreen(title, aButtons, nButtons, 1);
    
    return twoStateHdle;
}


// 3 state "quit race" screen creation and activation.
void *
RmTriStateScreen(
	const char *title,
	const char *label1, const char *tip1, void *screen1,
	const char *label2, const char *tip2, void *screen2,
	const char *label3, const char *tip3, void *screen3)
{
    static const int nButtons = 3;
    const tButtonDesc aButtons[nButtons]  =
    {
	{ label1, tip1, screen1 },
	{ label2, tip2, screen2 },
	{ label3, tip3, screen3 }
    };
	
    if (triStateHdle) {
	GfuiScreenRelease(triStateHdle);
    }
	
    triStateHdle = rmNStateScreen(title, aButtons, nButtons, 2);
    
    return triStateHdle;
}

// 4 state "quit race" screen creation and activation.
void *
RmFourStateScreen(
	const char *title,
	const char *label1, const char *tip1, void *screen1,
	const char *label2, const char *tip2, void *screen2,
	const char *label3, const char *tip3, void *screen3,
	const char *label4, const char *tip4, void *screen4)
{
    static const int nButtons = 4;
    const tButtonDesc aButtons[nButtons]  =
    {
	{ label1, tip1, screen1 },
	{ label2, tip2, screen2 },
	{ label3, tip3, screen3 },
	{ label4, tip4, screen4 }
    };
	
    if (fourStateHdle) {
	GfuiScreenRelease(fourStateHdle);
    }
	
    fourStateHdle = rmNStateScreen(title, aButtons, nButtons, 3);
    
    return fourStateHdle;
}


/*********************************************************
 * Start race screen
 */

static const int NMaxLines = 20;

typedef struct 
{
    void	*startScr;
    void	*abortScr;
    tRmInfo	*info;
    int		start;
} tStartRaceCall;

static tStartRaceCall	nextStartRace, prevStartRace;
static void		*rmScrHdle = 0;

static void rmDisplayStartRace(tRmInfo *info, void *startScr, void *abortScr, int start);

static void
rmChgStartScreen(void *vpsrc)
{
    void		*prevScr = rmScrHdle;
    tStartRaceCall 	*psrc = (tStartRaceCall*)vpsrc;
    
    rmDisplayStartRace(psrc->info, psrc->startScr, psrc->abortScr, psrc->start);
    GfuiScreenRelease(prevScr);
}

static void
rmDisplayStartRace(tRmInfo *info, void *startScr, void *abortScr, int start)
{
    static char	path[1024];
    int		nCars;
    int		i;
    int		y;
    int		x, dx;
    int		rows, curRow;
    const char	*img;
    const char	*name;
    int		robotIdx;
    void	*robhdle;
    void	*carHdle;
    const char	*carName;
    void	*params = info->params;
    const char	*race = info->_reRaceName;
    
    // Create screen, load menu XML descriptor and create static controls.
    rmScrHdle = GfuiScreenCreate();

    void *menuXMLDescHdle = LoadMenuXML("startracemenu.xml");

    CreateStaticControls(menuXMLDescHdle, rmScrHdle);

    // Create variable title label.
    int titleId = CreateLabelControl(rmScrHdle, menuXMLDescHdle, "titlelabel");
    GfuiLabelSetText(rmScrHdle, titleId, race);

    // Create background image if any.
    img = GfParmGetStr(params, RM_SECT_HEADER, RM_ATTR_STARTIMG, 0);
    if (img) {
	GfuiScreenAddBgImg(rmScrHdle, img);
    }
	
    // Create starting grid labels if specified in race params.
    if (!strcmp(GfParmGetStr(params, race, RM_ATTR_DISP_START_GRID, RM_VAL_YES), RM_VAL_YES)) {

	// Create starting grid subtitle label.
	CreateLabelControl(rmScrHdle, menuXMLDescHdle, "subtitlelabel");

	sprintf(path, "%s/%s", race, RM_SECT_STARTINGGRID);
	rows = (int)GfParmGetNum(params, path, RM_ATTR_ROWS, (char*)NULL, 2);
		
	// Create drivers info table.
	dx = 0;
	x = 40;
	y = 400;
	curRow = 0;
	nCars = GfParmGetEltNb(params, RM_SECT_DRIVERS_RACING);
		
	for (i = start; i < MIN(start + NMaxLines, nCars); i++) {
	    /* Find starting driver's name */
	    sprintf(path, "%s/%d", RM_SECT_DRIVERS_RACING, i + 1);
	    name = GfParmGetStr(info->params, path, RM_ATTR_MODULE, "");
	    robotIdx = (int)GfParmGetNum(info->params, path, RM_ATTR_IDX, NULL, 0);
			
	    sprintf(path, "%sdrivers/%s/%s.xml", GetLocalDir(), name, name);
	    robhdle = GfParmReadFile(path, GFPARM_RMODE_STD);
	    if (!robhdle) {
		sprintf(path, "drivers/%s/%s.xml", name, name);
		robhdle = GfParmReadFile(path, GFPARM_RMODE_STD);
	    }

	    if (robhdle) {
		sprintf(path, "%s/%s/%d", ROB_SECT_ROBOTS, ROB_LIST_INDEX, robotIdx);
		name = GfParmGetStr(robhdle, path, ROB_ATTR_NAME, "<none>");
		carName = GfParmGetStr(robhdle, path, ROB_ATTR_CAR, "");
				
		sprintf(path, "cars/%s/%s.xml", carName, carName);
		carHdle = GfParmReadFile(path, GFPARM_RMODE_STD);
		carName = GfParmGetName(carHdle);
			
		sprintf(path, "%d - %s - (%s)", i + 1, name, carName);
		GfuiLabelCreate(rmScrHdle, path, GFUI_FONT_MEDIUM_C,
				x + curRow * dx, y, GFUI_ALIGN_HL_VB, 0);

		GfParmReleaseHandle(carHdle);
		GfParmReleaseHandle(robhdle);
	    }

	    curRow = (curRow + 1) % rows;
	    y -= 15;
	}
		
		
	if (start > 0) {
	    prevStartRace.startScr = startScr;
	    prevStartRace.abortScr = abortScr;
	    prevStartRace.info     = info;
	    prevStartRace.start    = start - NMaxLines;

	    // Create Previous page button and associated keyboard shortcut if needed.
	    CreateButtonControl(rmScrHdle, menuXMLDescHdle, "previouspagearrow",
				(void*)&prevStartRace, rmChgStartScreen);
	    GfuiAddSKey(rmScrHdle, GLUT_KEY_PAGE_UP, "Previous drivers", 
			(void*)&prevStartRace, rmChgStartScreen, NULL);
	}
		
	if (i < nCars) {
	    nextStartRace.startScr = startScr;
	    nextStartRace.abortScr = abortScr;
	    nextStartRace.info     = info;
	    nextStartRace.start    = start + NMaxLines;

	    // Create Next page button and associated keyboard shortcut if needed.
	    CreateButtonControl(rmScrHdle, menuXMLDescHdle, "nextpagearrow",
				(void*)&nextStartRace, rmChgStartScreen);
	    GfuiAddSKey(rmScrHdle, GLUT_KEY_PAGE_DOWN, "Next Drivers", 
			(void*)&nextStartRace, rmChgStartScreen, NULL);
	}
    }
	
    // Create Start and Abandon buttons.
    CreateButtonControl(rmScrHdle, menuXMLDescHdle, "startbutton", startScr, GfuiScreenReplace);
    CreateButtonControl(rmScrHdle, menuXMLDescHdle, "abandonbutton", abortScr, GfuiScreenReplace);

    // Close menu XML descriptor.
    GfParmReleaseHandle(menuXMLDescHdle);
    
    // Register keyboard shortcuts.
    GfuiAddKey(rmScrHdle, (unsigned char)13, "Start", startScr, GfuiScreenReplace, NULL);
    GfuiAddKey(rmScrHdle, (unsigned char)27, "Abandon", abortScr, GfuiScreenReplace, NULL);
    GfuiAddSKey(rmScrHdle, GLUT_KEY_F1, "Help", rmScrHdle, GfuiHelpScreen, NULL);
    GfuiAddSKey(rmScrHdle, GLUT_KEY_F12, "Take a Screen Shot", NULL, GfuiScreenShot, NULL);
	
    // Activate the created screen.
    GfuiScreenActivate(rmScrHdle);
}


void
RmDisplayStartRace(tRmInfo *info, void *startScr, void *abortScr)
{
    rmDisplayStartRace(info, startScr, abortScr, 0);
}
