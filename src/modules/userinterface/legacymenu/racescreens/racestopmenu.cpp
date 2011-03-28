/**************************************************************************

    file        : racestartstop.cpp
    copyright   : (C) 2011 by Jean-Philippe Meuret                        
    email       : pouillot@users.sourceforge.net   
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

#include <tgfclient.h>

#include "legacymenu.h"
#include "exitmenu.h"
#include "racescreens.h"


// Abort race hook ******************************************************
static void
rmAbortRaceHookActivate(void * /* dummy */)
{
	RmGameScreen();

	LegacyMenu::self().raceEngine().abortRace();
}

static void *
rmAbortRaceHookInit(void)
{
	static void *pvAbortRaceHookHandle = 0;

	if (!pvAbortRaceHookHandle)
		pvAbortRaceHookHandle = GfuiHookCreate(0, rmAbortRaceHookActivate);

	return pvAbortRaceHookHandle;
}

// Skip session hook ***************************************************
static void
rmSkipSessionHookActivate(void * /* dummy */)
{
	RmGameScreen();

	LegacyMenu::self().raceEngine().skipRaceSession();
}

static void *
rmSkipSessionHookInit(void)
{
	static void	*pvSkipSessionHookHandle = 0;

	if (!pvSkipSessionHookHandle)
		pvSkipSessionHookHandle = GfuiHookCreate(0, rmSkipSessionHookActivate);

	return pvSkipSessionHookHandle;
}

// Back to race hook ***************************************************
static void
rmBackToRaceHookActivate(void * /* dummy */)
{
	LegacyMenu::self().raceEngine().continueRace();
	
	RmGameScreen();
}

static void *
rmBackToRaceHookInit(void)
{
	static void	*pvBackToRaceHookHandle = 0;

	if (!pvBackToRaceHookHandle)
		pvBackToRaceHookHandle = GfuiHookCreate(0, rmBackToRaceHookActivate);

	return pvBackToRaceHookHandle;
}

// Restart race hook ***************************************************
static void
rmRestartRaceHookActivate(void * /* dummy */)
{
	LegacyMenu::self().raceEngine().restartRace();
	
	RmGameScreen();
}

static void *
rmRestartRaceHookInit(void)
{
	static void	*pvRestartRaceHookHandle = 0;

	if (!pvRestartRaceHookHandle)
		pvRestartRaceHookHandle = GfuiHookCreate(0, rmRestartRaceHookActivate);

	return pvRestartRaceHookHandle;
}

// Quit race hook *****************************************************
static void	*rmStopScrHandle = 0;

static void
rmQuitHookActivate(void * /* dummy */)
{
	if (rmStopScrHandle) 
	{
		GfuiScreenActivate(ExitMenuInit(rmStopScrHandle));
	}
}

static void *
rmQuitHookInit(void)
{
	static void	*pvQuitHookHandle = 0;

	if (!pvQuitHookHandle)
		pvQuitHookHandle = GfuiHookCreate(0, rmQuitHookActivate);

	return pvQuitHookHandle;
}

// 2, 3, 4 or 5 buttons "Stop race" menu *****************************************

static void *QuitHdle[5] = { 0, 0, 0, 0, 0 };

// Descriptor for 1 button.
typedef struct {
    
    const char *label;  // Label to display.
    const char *tip;    // Tip displayed when mouse hover.
    void       *screen; // Screen to activate if clicked.

} tButtonDesc;

// Generic function for creating and activating the menu.
static void*
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

// Simpler front-end function for creating and activating the menu.
static void*
rmStopRaceScreen(const char *title,
				 const char *label1, const char *tip1, void *screen1,
				 const char *label2, const char *tip2, void *screen2,
				 const char *label3 = 0, const char *tip3 = 0, void *screen3 = 0,
				 const char *label4 = 0, const char *tip4 = 0, void *screen4 = 0,
				 const char *label5 = 0, const char *tip5 = 0, void *screen5 = 0)
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

void
RmStopRaceScreen()
{
	void* params = LegacyMenu::self().raceEngine().data()->params;
	const char* pszRaceName = LegacyMenu::self().raceEngine().data()->_reRaceName;

	if (!strcmp(GfParmGetStr(params, pszRaceName, RM_ATTR_ALLOW_RESTART, RM_VAL_NO), RM_VAL_NO)) 
	{
		if (strcmp(GfParmGetStr(params, pszRaceName, RM_ATTR_MUST_COMPLETE, RM_VAL_YES), RM_VAL_YES)) 
		{
			rmStopScrHandle =
				rmStopRaceScreen
				    ("Race Stopped",
					 "Abandon Race", "Abort current race", rmAbortRaceHookInit(),
					 "Resume Race", "Return to Race", rmBackToRaceHookInit(),
					 "Skip Session", "Skip Session", rmSkipSessionHookInit(),
					 "Quit Game", "Quit the game", rmQuitHookInit());
		}
		else 
		{
			rmStopScrHandle =
				rmStopRaceScreen
				    ("Race Stopped",
					 "Abandon Race", "Abort current race", rmAbortRaceHookInit(),
					 "Resume Race", "Return to Race", rmBackToRaceHookInit(),
					 "Quit Game", "Quit the game", rmQuitHookInit());
		}
	}
	else 
	{
		if (strcmp(GfParmGetStr(params, pszRaceName, RM_ATTR_MUST_COMPLETE, RM_VAL_YES), RM_VAL_YES)) 
		{
			rmStopScrHandle =
				rmStopRaceScreen
				    ("Race Stopped",
					 "Restart Race", "Restart the current race", rmRestartRaceHookInit(),
					 "Abandon Race", "Abort current race", rmAbortRaceHookInit(),
					 "Resume Race", "Return to Race", rmBackToRaceHookInit(),
					 "Skip Session", "Skip Session", rmSkipSessionHookInit(),
					 "Quit Game", "Quit the game", rmQuitHookInit());
		}
		else 
		{
			rmStopScrHandle =
				rmStopRaceScreen
				    ("Race Stopped",
					 "Restart Race", "Restart the current race", rmRestartRaceHookInit(),
					 "Abandon Race", "Abort current race", rmAbortRaceHookInit(),
					 "Resume Race", "Return to Race", rmBackToRaceHookInit(),
					 "Quit Game", "Quit the game", rmQuitHookInit());
		}
	}
}
