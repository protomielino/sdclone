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

	LmRaceEngine().abortRace();
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

	LmRaceEngine().skipRaceSession();
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
	LmRaceEngine().start();
	
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
	LmRaceEngine().restartRace();
	
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
		GfuiScreenActivate(ExitMenuInit(rmStopScrHandle));
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
    
    const char* role;  // Button role.
    void       *screen; // Screen to activate if clicked.

} tButtonDesc;

// Generic function for creating and activating the menu.
static void*
rmStopRaceScreen(const tButtonDesc aButtons[], int nButtons, int nCancelIndex)
{
    // Create screen, load menu XML descriptor and create static controls.
    void *hscreen = GfuiScreenCreate(NULL, NULL, NULL, NULL, NULL, 1);

    void *hmenu = GfuiMenuLoad("stopracemenu.xml");

    GfuiMenuCreateStaticControls(hmenu, hscreen);

    // Create buttons from menu properties and button template.
    const int xpos = (int)GfuiMenuGetNumProperty(hmenu, "xButton", 270);
    const int dy = (int)GfuiMenuGetNumProperty(hmenu, "buttonShift", 30);
	int ypos = (int)GfuiMenuGetNumProperty(hmenu, "yTopButton", 380);
	char pszPropName[64];
	const char* pszCancelTip = "";
    for (int nButInd = 0; nButInd < nButtons; nButInd++)
    {
		// Get text and tip from button role and menu properties.
		sprintf(pszPropName, "%s.text", aButtons[nButInd].role);
		const char* pszText = GfuiMenuGetStrProperty(hmenu, pszPropName, "");
		sprintf(pszPropName, "%s.tip", aButtons[nButInd].role);
		const char* pszTip = GfuiMenuGetStrProperty(hmenu, pszPropName, "");
		if (nButInd == nCancelIndex)
			pszCancelTip = pszTip;
		
		// Create the button from the template.
		GfuiMenuCreateTextButtonControl(hscreen, hmenu, "button",
										aButtons[nButInd].screen, GfuiScreenActivate, 0, 0, 0,
										true, // From template.
										pszText, pszTip, xpos, ypos);

		// Next button if not last.
		ypos -= dy;
    }

    // Close menu XML descriptor.
    GfParmReleaseHandle(hmenu);
    
    // Register keyboard shortcuts.
    GfuiMenuDefaultKeysAdd(hscreen);
    GfuiAddKey(hscreen, GFUIK_ESCAPE, pszCancelTip,
			   aButtons[nCancelIndex].screen, GfuiScreenActivate, NULL);

    // Activate the created screen.
    GfuiScreenActivate(hscreen);

    return hscreen;
}

// Simpler front-end function for creating and activating the menu.
static void*
rmStopRaceScreen(const char *buttonRole1, void *screen1,
				 const char *buttonRole2, void *screen2,
				 const char *buttonRole3 = 0, void *screen3 = 0,
				 const char *buttonRole4 = 0, void *screen4 = 0,
				 const char *buttonRole5 = 0, void *screen5 = 0)
{
    const tButtonDesc aButtons[5] =
    {
        { buttonRole1, screen1 },
        { buttonRole2, screen2 },
        { buttonRole3, screen3 },
        { buttonRole4, screen4 },
        { buttonRole5, screen5 }
    };
	
    int nButtons = 2;
	if (buttonRole3 && screen3)
	{
		nButtons++;
		if (buttonRole4 && screen4)
		{
			nButtons++;
			if (buttonRole5 && screen5)
				nButtons++;
		}
	}
        
    if (QuitHdle[nButtons-1])
        GfuiScreenRelease(QuitHdle[nButtons-1]);
        
    QuitHdle[nButtons-1] = rmStopRaceScreen(aButtons, nButtons, nButtons-1);
    
    return QuitHdle[nButtons-1];
}

void
RmStopRaceScreen()
{
	void* params = LmRaceEngine().outData()->params;
	const char* pszRaceName = LmRaceEngine().outData()->_reRaceName;

	if (!strcmp(GfParmGetStr(params, pszRaceName, RM_ATTR_ALLOW_RESTART, RM_VAL_NO), RM_VAL_NO)) 
	{
		if (strcmp(GfParmGetStr(params, pszRaceName, RM_ATTR_MUST_COMPLETE, RM_VAL_YES), RM_VAL_YES)) 
		{
			rmStopScrHandle =
				rmStopRaceScreen
				    ("abort", rmAbortRaceHookInit(),
					 "resume", rmBackToRaceHookInit(),
					 "skip", rmSkipSessionHookInit(),
					 "quit", rmQuitHookInit());
		}
		else 
		{
			rmStopScrHandle =
				rmStopRaceScreen
				    ("abort", rmAbortRaceHookInit(),
					 "resume", rmBackToRaceHookInit(),
					 "quit", rmQuitHookInit());
		}
	}
	else 
	{
		if (strcmp(GfParmGetStr(params, pszRaceName, RM_ATTR_MUST_COMPLETE, RM_VAL_YES), RM_VAL_YES)) 
		{
			rmStopScrHandle =
				rmStopRaceScreen
				    ("restart", rmRestartRaceHookInit(),
					 "abort", rmAbortRaceHookInit(),
					 "resume", rmBackToRaceHookInit(),
					 "skip", rmSkipSessionHookInit(),
					 "quit", rmQuitHookInit());
		}
		else 
		{
			rmStopScrHandle =
				rmStopRaceScreen
				    ("restart", rmRestartRaceHookInit(),
					 "abort", rmAbortRaceHookInit(),
					 "resume", rmBackToRaceHookInit(),
					 "quit", rmQuitHookInit());
		}
	}
}
