/***************************************************************************

    file        : miscscreens.cpp
    created     : Sun Dec  8 13:01:47 CET 2002
    copyright   : (C) 2002 by Eric Espi�                        
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
                
    @author     <a href=mailto:eric.espie@torcs.org>Eric Espie</a>
    @version    $Id$
*/

#include <cstdio>

#include <tgfclient.h>
#include <robot.h>

#include "racescreens.h"


/*********************************************************
 * 2, 3, 4 or 5 buttons "Stop race" screens
 */

static void *QuitHdle[5] = { 0, 0, 0, 0, 0 };

// Descriptor for 1 button.
typedef struct {
    
    const char *label;  // Label to display.
    const char *tip;    // Tip displayed when mouse hover.
    void       *screen; // Screen to activate if clicked.

} tButtonDesc;

// "Stop race" screen creation and activation.
static void *
rmStopRaceScreen(const char *title, const tButtonDesc aButtons[], int nButtons, int nCancelIndex)
{
    void *screenHdle = 0;
        
    // Create screen, load menu XML descriptor and create static controls.
    screenHdle = GfuiScreenCreateEx(NULL, NULL, NULL, NULL, NULL, 1);

    void *menuXMLDescHdle = LoadMenuXML("stopracemenu.xml");

    CreateStaticControls(menuXMLDescHdle, screenHdle);

    // Create variable title label.
    int titleId = CreateLabelControl(screenHdle, menuXMLDescHdle, "titlelabel");
    GfuiLabelSetText(screenHdle, titleId, title);

    // Create specified buttons, left aligned.
    for (int nButInd = 0; nButInd < nButtons; nButInd++)
    {
        const int id =
			GfuiMenuButtonCreate(screenHdle, aButtons[nButInd].label, aButtons[nButInd].tip, 
								 aButtons[nButInd].screen, GFUI_ALIGN_HL_VB, GfuiScreenActivate);

		GfuiButtonShowBox(screenHdle, id, false);
		Color c, fc, pc;
		c.red  = 1.0;   c.green  = 1.0; c.blue  = 1.0; c.alpha  = 1.0;
		fc.red = 1.0;   fc.green = 0.8; fc.blue = 0.0; fc.alpha = 1.0;
		pc.red = 0.902; pc.green = 0.1; pc.blue = 0.2; pc.alpha = 1.0;

        GfuiButtonSetColor(screenHdle, id, c);
        GfuiButtonSetFocusColor(screenHdle, id, fc);
        GfuiButtonSetPushedColor(screenHdle, id, pc);
    }

    // Close menu XML descriptor.
    GfParmReleaseHandle(menuXMLDescHdle);
    
    // Register keyboard shortcuts.
    GfuiMenuDefaultKeysAdd(screenHdle);
    GfuiAddKey(screenHdle, GFUIK_ESCAPE, aButtons[nCancelIndex].tip, 
               aButtons[nCancelIndex].screen, GfuiScreenActivate, NULL);

    // Activate the created screen.
    GfuiScreenActivate(screenHdle);

    return screenHdle;
}

// "quit race" screen creation and activation.
void *
RmStopRaceScreen(const char *title,
				 const char *label1, const char *tip1, void *screen1,
				 const char *label2, const char *tip2, void *screen2,
				 const char *label3, const char *tip3, void *screen3,
				 const char *label4, const char *tip4, void *screen4,
				 const char *label5, const char *tip5, void *screen5)
{
    const tButtonDesc aButtons[5] =
    {
        { label1, tip1, screen1 },
        { label2, tip2, screen2 },
        { label3, tip3, screen3 },
        { label4, tip4, screen4 },
        { label5, tip5, screen5 }
    };
	
    int nButtons = 2;
	if (label3 && tip3 && screen3)
	{
		nButtons++;
		if (label4 && tip4 && screen4)
		{
			nButtons++;
			if (label5 && tip5 && screen5)
				nButtons++;
		}
	}
        
    if (QuitHdle[nButtons-1])
        GfuiScreenRelease(QuitHdle[nButtons-1]);
        
    QuitHdle[nButtons-1] = rmStopRaceScreen(title, aButtons, nButtons, nButtons-1);
    
    return QuitHdle[nButtons-1];
}

/*********************************************************
 * Start race screen
 */

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

static void rmDisplayStartRace(tRmInfo *info, void *startScr, void *abortScr, int start);

static void
rmChgStartScreen(void *vpsrc)
{
    void                *prevScr = rmScrHdle;
    tStartRaceCall      *psrc = (tStartRaceCall*)vpsrc;
    
    rmDisplayStartRace(psrc->info, psrc->startScr, psrc->abortScr, psrc->start);
    GfuiScreenRelease(prevScr);
}

static void
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
            CreateButtonControl(rmScrHdle, menuXMLDescHdle, "previouspagearrow",
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
            CreateButtonControl(rmScrHdle, menuXMLDescHdle, "nextpagearrow",
                                (void*)&nextStartRace, rmChgStartScreen);
            GfuiAddKey(rmScrHdle, GFUIK_PAGEDOWN, "Next Drivers", 
                        (void*)&nextStartRace, rmChgStartScreen, NULL);
        }
    }
        
    // Create Start and Abandon buttons.
    CreateButtonControl(rmScrHdle, menuXMLDescHdle, "startbutton", startScr, GfuiScreenReplace);
    CreateButtonControl(rmScrHdle, menuXMLDescHdle, "abandonbutton", abortScr, GfuiScreenReplace);

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
RmDisplayStartRace(tRmInfo *info, void *startScr, void *abortScr)
{
    rmDisplayStartRace(info, startScr, abortScr, 0);
}
