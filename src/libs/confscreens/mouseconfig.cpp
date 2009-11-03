/***************************************************************************

    file        : mouseconfig.cpp
    created     : Thu Mar 13 21:27:03 CET 2003
    copyright   : (C) 2003 by Eric Espi�                        
    email       : eric.espie@torcs.org   
    version     : $Id: mouseconfig.cpp,v 1.5 2003/11/08 16:37:18 torcs Exp $                                  

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <cstring>

#include <tgfclient.h>

#include "controlconfig.h"
#include "mouseconfig.h"

// Constants.
static const int CmdOffset = 0;

// TODO: Put these strings in mouseconfigmenu.xml for translation.
static const char *Instructions[] = {
    "Move Mouse for maximum left steer then press a button",
    "Move Mouse for maximum right steer then press a button",
    "Move Mouse for full throttle then press a button",
    "Move Mouse for full brake then press a button",
    "Calibration terminated",
    "Calibration failed"
};

// Current calibration step
static int CalState;

// Current command configuration to calibrate.
static tCmdInfo *Cmd;
static int MaxCmd;

// Mouse info.
static tCtrlMouseInfo MouseInfo;

// Menu screen handle.
static void *ScrHandle = NULL;

// Screen controls Ids
static int InstId;


static int
GetNextAxis(void)
{
    int i;

    for (i = CalState; i < 4; i++) {
	if (Cmd[CmdOffset + i].ref.type == GFCTRL_TYPE_MOUSE_AXIS) {
	    return i;
	}
    }

    return i;
}


static void Idle2(void);

static void
MouseCalAutomaton(void)
{
    float axv;

    switch (CalState) {
    case 0:
    case 1:
	GfctrlMouseGetCurrent(&MouseInfo);
	axv = MouseInfo.ax[Cmd[CmdOffset + CalState].ref.index];
	if (fabs(axv) < 0.01) {
	    return;		/* ignore no move input */
	}
	Cmd[CmdOffset + CalState].max = axv;
	Cmd[CmdOffset + CalState].pow = 1.0 / axv;
	break;

    case 2:
    case 3:
	GfctrlMouseGetCurrent(&MouseInfo);
	axv = MouseInfo.ax[Cmd[CmdOffset + CalState].ref.index];
	if (fabs(axv) < 0.01) {
	    return;		/* ignore no move input */
	}
	Cmd[CmdOffset + CalState].max = axv;
	Cmd[CmdOffset + CalState].pow = 1.0 / axv;
	break;
	
    }

    CalState++;
    CalState = GetNextAxis();
    GfuiLabelSetText(ScrHandle, InstId, Instructions[CalState]);
    if (CalState < 4) {
	glutIdleFunc(Idle2);
    } else {
	glutIdleFunc(0);
	glutPostRedisplay();
    }
}

static void
Idle2(void)
{
    int	i;

    GfctrlMouseGetCurrent(&MouseInfo);

    /* Check for a mouse button pressed */
    for (i = 0; i < 3; i++) {
	if (MouseInfo.edgedn[i]) {
	    MouseCalAutomaton();
	    return;
	}
    }

    /* Let CPU take breath (and fans stay at low and quiet speed) */
    GfuiSleep(0.001);
}

static void
IdleMouseInit(void)
{
    /* Get the center mouse position  */
    memset(&MouseInfo, 0, sizeof(MouseInfo));
    GfctrlMouseGetCurrent(&MouseInfo);
    GfctrlMouseInitCenter();
    glutIdleFunc(Idle2);
}

static void
onActivate(void * /* dummy */)
{
    //int dummy;

    //GfScrGetSize(&ScreenWidth, &ScreenHeight, &dummy, &dummy);
    CalState = 0;
    GetNextAxis();
    GfuiLabelSetText(ScrHandle, InstId, Instructions[CalState]);
    if (CalState < 4) {
	glutIdleFunc(IdleMouseInit);
	GfctrlMouseCenter();
    }
}

void *
MouseCalMenuInit(void *prevMenu, tCmdInfo *cmd, int maxcmd)
{
    Cmd = cmd;
    MaxCmd = maxcmd;
    
    if (ScrHandle) {
	return ScrHandle;
    }

    // Create screen, load menu XML descriptor and create static controls.
    ScrHandle = GfuiScreenCreateEx(NULL, NULL, onActivate, NULL, NULL, 1);

    void *menuXMLDescHdle = LoadMenuXML("mouseconfigmenu.xml");

    CreateStaticControls(menuXMLDescHdle, ScrHandle);

    // Create instruction variable label.
    InstId = CreateLabelControl(ScrHandle, menuXMLDescHdle, "instructionlabel");
    
    // Create Back and Reset buttons.
    CreateButtonControl(ScrHandle, menuXMLDescHdle, "backbutton", prevMenu, GfuiScreenActivate);
    CreateButtonControl(ScrHandle, menuXMLDescHdle, "resetbutton", NULL, onActivate);

    // Register keyboard shortcuts.
    GfuiMenuDefaultKeysAdd(ScrHandle);
    GfuiAddKey(ScrHandle, 27, "Back", prevMenu, GfuiScreenActivate, NULL);
    GfuiAddKey(ScrHandle, 13, "Back", prevMenu, GfuiScreenActivate, NULL);

    return ScrHandle;
}
