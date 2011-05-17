/***************************************************************************

    file        : racestartmenu.cpp
    created     : Sun Dec  8 13:01:47 CET 2002
    copyright   : (C) 2002 by Eric Espie
    email       : eric.espie@torcs.org
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
                The race start menu
    @author     <a href=mailto:eric.espie@torcs.org>Eric Espie</a>
    @version    $Id$
*/

#include <cstdio>

#include <tgfclient.h>
#include <robot.h>

#include "legacymenu.h"
#include "racescreens.h"


// Abandon race hook ******************************************************
static void
rmAbandonRaceHookActivate(void * /* vforce */)
{
	LmRaceEngine().abandonRace();
	
	RmGameScreen();
}

static void *
rmAbandonRaceHookInit(void)
{
	static void *pvAbandonRaceHookHandle = 0;

	if (!pvAbandonRaceHookHandle)
		pvAbandonRaceHookHandle = GfuiHookCreate(0, rmAbandonRaceHookActivate);

	return pvAbandonRaceHookHandle;
}

// Start race hook ******************************************************
static void
rmStartRaceHookActivate(void * /* dummy */)
{
	LmRaceEngine().startRace();
}

static void *
rmStartRaceHookInit(void)
{
	static void	*pvStartRaceHookHandle = 0;

	if (!pvStartRaceHookHandle)
		pvStartRaceHookHandle = GfuiHookCreate(0, rmStartRaceHookActivate);

	return pvStartRaceHookHandle;
}

// The menu itself ******************************************************
static const int NMaxLines = 20;

typedef struct 
{
    void        *startScr;
    void        *abortScr;
    tRmInfo     *info;
    int         start;
} tStartRaceCall;

static tStartRaceCall   nextStartRace, prevStartRace;
static void             *rmScrHdle = 0;

static void rmDisplayStartRace(tRmInfo *info, void *startScr, void *abortScr, int start = 0);

static void
rmChgStartScreen(void *vpsrc)
{
    void                *prevScr = rmScrHdle;
    tStartRaceCall      *psrc = (tStartRaceCall*)vpsrc;
    
    rmDisplayStartRace(psrc->info, psrc->startScr, psrc->abortScr, psrc->start);
    GfuiScreenRelease(prevScr);
}

void
rmDisplayStartRace(tRmInfo *info, void *startScr, void *abortScr, int start)
{
    static char path[1024];
    int         nCars;
    int         i;
    int         y;
    int         x, dx;
    int         rows, curRow;
    const char  *img;
    const char  *name;
    int         robotIdx;
    int         extended;
    void        *robhdle;
    void        *carHdle;
    const char  *carName;
    void        *params = info->params;
    const char  *race = info->_reRaceName;
    
	GfLogTrace("Entering Start Race menu\n");
	
    // Create screen, load menu XML descriptor and create static controls.
    rmScrHdle = GfuiScreenCreate();

    void *menuXMLDescHdle = GfuiMenuLoad("startracemenu.xml");

    GfuiMenuCreateStaticControls(menuXMLDescHdle, rmScrHdle);

    // Create variable title label.
    int titleId = GfuiMenuCreateLabelControl(rmScrHdle, menuXMLDescHdle, "titlelabel");
    GfuiLabelSetText(rmScrHdle, titleId, race);

    // Create background image if any.
    img = GfParmGetStr(params, RM_SECT_HEADER, RM_ATTR_STARTIMG, 0);
    if (img) {
        GfuiScreenAddBgImg(rmScrHdle, img);
    }
        
    // Create starting grid labels if specified in race params.
    if (!strcmp(GfParmGetStr(params, race, RM_ATTR_DISP_START_GRID, RM_VAL_YES), RM_VAL_YES)) {

        // Create starting grid subtitle label.
        GfuiMenuCreateLabelControl(rmScrHdle, menuXMLDescHdle, "subtitlelabel");

        sprintf(path, "%s/%s", race, RM_SECT_STARTINGGRID);
        rows = (int)GfParmGetNum(params, path, RM_ATTR_ROWS, (char*)NULL, 2);
                
        // Create drivers info table.
        dx = 0;
        x = 40;
        y = 400;
        curRow = 0;
        nCars = GfParmGetEltNb(params, RM_SECT_DRIVERS_RACING);
                
        for (i = start; i < MIN(start + NMaxLines, nCars); i++)
		{
            /* Find starting driver's name */
            sprintf(path, "%s/%d", RM_SECT_DRIVERS_RACING, i + 1);
            name = GfParmGetStr(info->params, path, RM_ATTR_MODULE, "");
            robotIdx = (int)GfParmGetNum(info->params, path, RM_ATTR_IDX, NULL, 0);
            extended = GfParmGetNum(info->params, path, RM_ATTR_EXTENDED, NULL, 0);
            carName = NULL;
            robhdle = NULL;

            if( extended )
            {
                sprintf(path, "%s/%s/%d/%d", RM_SECT_DRIVERINFO, name, extended, robotIdx);
                carName = GfParmGetStr(info->params, path, RM_ATTR_CARNAME, NULL);
            }
            else
            {
                sprintf(path, "%sdrivers/%s/%s.xml", GfLocalDir(), name, name);
                robhdle = GfParmReadFile(path, GFPARM_RMODE_STD);
                if (!robhdle)
				{
                    sprintf(path, "drivers/%s/%s.xml", name, name);
                    robhdle = GfParmReadFile(path, GFPARM_RMODE_STD);
                }
  
                if (robhdle)
				{
                    sprintf(path, "%s/%s/%d", ROB_SECT_ROBOTS, ROB_LIST_INDEX, robotIdx);
                    name = GfParmGetStr(robhdle, path, ROB_ATTR_NAME, "<none>");
                    carName = GfParmGetStr(robhdle, path, ROB_ATTR_CAR, "");
                }
            }

            if( carName )
            {
                sprintf(path, "cars/%s/%s.xml", carName, carName);
                carHdle = GfParmReadFile(path, GFPARM_RMODE_STD);
                carName = GfParmGetName(carHdle);
                        
                sprintf(path, "%d - %s - (%s)", i + 1, name, carName);
                GfuiLabelCreate(rmScrHdle, path, GFUI_FONT_MEDIUM_C,
                                x + curRow * dx, y, GFUI_ALIGN_HL_VB, 0);

                GfParmReleaseHandle(carHdle);
            }

            curRow = (curRow + 1) % rows;
            y -= 15;

			// Release robhdle only when no more need for read strings (like carname).
			if (robhdle)
				GfParmReleaseHandle(robhdle);
        }
                
        if (start > 0) {
            prevStartRace.startScr = startScr;
            prevStartRace.abortScr = abortScr;
            prevStartRace.info     = info;
            prevStartRace.start    = start - NMaxLines;

            // Create Previous page button and associated keyboard shortcut if needed.
            GfuiMenuCreateButtonControl(rmScrHdle, menuXMLDescHdle, "previouspagearrow",
                                (void*)&prevStartRace, rmChgStartScreen);
            GfuiAddKey(rmScrHdle, GFUIK_PAGEUP, "Previous drivers", 
                        (void*)&prevStartRace, rmChgStartScreen, NULL);
        }
                
        if (i < nCars) {
            nextStartRace.startScr = startScr;
            nextStartRace.abortScr = abortScr;
            nextStartRace.info     = info;
            nextStartRace.start    = start + NMaxLines;

            // Create Next page button and associated keyboard shortcut if needed.
            GfuiMenuCreateButtonControl(rmScrHdle, menuXMLDescHdle, "nextpagearrow",
                                (void*)&nextStartRace, rmChgStartScreen);
            GfuiAddKey(rmScrHdle, GFUIK_PAGEDOWN, "Next Drivers", 
                        (void*)&nextStartRace, rmChgStartScreen, NULL);
        }
    }
        
    // Create Start and Abandon buttons.
    GfuiMenuCreateButtonControl(rmScrHdle, menuXMLDescHdle, "startbutton", startScr, GfuiScreenReplace);
    GfuiMenuCreateButtonControl(rmScrHdle, menuXMLDescHdle, "abandonbutton", abortScr, GfuiScreenReplace);

    // Close menu XML descriptor.
    GfParmReleaseHandle(menuXMLDescHdle);
    
    // Register keyboard shortcuts.
    GfuiAddKey(rmScrHdle, GFUIK_RETURN, "Start", startScr, GfuiScreenReplace, NULL);
    GfuiAddKey(rmScrHdle, GFUIK_ESCAPE, "Abandon", abortScr, GfuiScreenReplace, NULL);
    GfuiAddKey(rmScrHdle, GFUIK_F1, "Help", rmScrHdle, GfuiHelpScreen, NULL);
    GfuiAddKey(rmScrHdle, GFUIK_F12, "Take a Screen Shot", NULL, GfuiScreenShot, NULL);
        
    // Activate the created screen.
    GfuiScreenActivate(rmScrHdle);
}

void
RmDisplayStartRace()
{
	rmDisplayStartRace(LmRaceEngine().inData(),
					   rmStartRaceHookInit(), rmAbandonRaceHookInit());
}

