/***************************************************************************

    file                 : joystickconfig.cpp
    created              : Wed Mar 21 21:46:11 CET 2001
    copyright            : (C) 2001 by Eric Espie
    email                : eric.espie@torcs.org
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
    		Human player joystick configuration menu
    @author	<a href=mailto:eric.espie@torcs.org>Eric Espie</a>
    @version	$Id$
*/


#include <stdio.h>
#include <stdlib.h>
#include <cstring>

#include <tgf.hpp>
#include <tgfclient.h>

#include "controlconfig.h"
#include "joystickconfig.h"


// Constants.
static const int NbMaxCalAxis = 4;
static const int NbCalSteps = 6;

static const int CmdOffset = -1;

// WARNING: These strings must match the names used for labels in the XML menu descriptor.
static const char *LabName[] = { "steer", "throttle", "brake", "clutch" };

// TODO: Put these strings in joystickconfigmenu.xml for translation.
static const char *Instructions[] = {
    "Center the joystick then press a button",
    "Hold steer left and press a button",
    "Hold steer right and press a button",
    "Apply full throttle and press a button",
    "Apply full brake and press a button",
    "Apply full clutch then press a button",
    "Calibration is successfully terminated",
    "Calibration failed"
};

// Current calibration step
static int CalState;

// Current command configuration to calibrate.
static tCmdInfo *Cmd;
static int MaxCmd;

// Joystick info.
static jsJoystick* Joystick[GFCTRL_JOY_NUMBER];
static float       JoyAxis[GFCTRL_JOY_MAX_AXES * GFCTRL_JOY_NUMBER];
static float       JoyAxisCenter[GFCTRL_JOY_MAX_AXES * GFCTRL_JOY_NUMBER];
static int         JoyButtons[GFCTRL_JOY_NUMBER];

// Menu screen handle.
static void *ScrHandle = NULL;

// Next menu screen handle.
static void* NextMenuHandle = NULL;

// Screen controls Ids
static int InstId;

static int LabAxisId[NbMaxCalAxis];
static int LabMinId[NbMaxCalAxis];
static int LabMaxId[NbMaxCalAxis];



static void
onNext(void * /* dummy */)
{
    int index;

    /* Release up and running joysticks */
    for (index = 0; index < GFCTRL_JOY_NUMBER; index++)
	if (Joystick[index]) {
	    delete Joystick[index];
	    Joystick[index] = 0;
	}

    /* Back to previous screen */
    GfuiScreenActivate(NextMenuHandle);
}

static void advanceStep (void)
{
    do {
	CalState++;
    } while (Cmd[CalState + CmdOffset].ref.type != GFCTRL_TYPE_JOY_AXIS && CalState < NbCalSteps);
}


static void
JoyCalAutomaton(void)
{
    static char buf[64];

    int axis;

    switch (CalState) {
    case 0:
	memcpy(JoyAxisCenter, JoyAxis, sizeof(JoyAxisCenter));
	advanceStep();
	break;
    case 1:
	axis = Cmd[CalState + CmdOffset].ref.index;
	Cmd[CalState + CmdOffset].min = JoyAxis[axis];
	Cmd[CalState + CmdOffset].max = JoyAxisCenter[axis];
	Cmd[CalState + CmdOffset].pow = 1.0;
	sprintf(buf, "%.2g", JoyAxis[axis]);
	GfuiLabelSetText(ScrHandle, LabMinId[0], buf);
	advanceStep();
	break;
    case 2:
	axis = Cmd[CalState + CmdOffset].ref.index;
	Cmd[CalState + CmdOffset].min = JoyAxisCenter[axis];
	Cmd[CalState + CmdOffset].max = JoyAxis[axis];
	Cmd[CalState + CmdOffset].pow = 1.0;
	sprintf(buf, "%.2g", JoyAxis[axis]);
	GfuiLabelSetText(ScrHandle, LabMaxId[0], buf);
	advanceStep();
	break;
    case 3:
    case 4:
    case 5:
	axis = Cmd[CalState + CmdOffset].ref.index;
	Cmd[CalState + CmdOffset].min = JoyAxisCenter[axis];
	Cmd[CalState + CmdOffset].max = JoyAxis[axis]*1.1;
	Cmd[CalState + CmdOffset].pow = 1.2;
	sprintf(buf, "%.2g", JoyAxisCenter[axis]);
	GfuiLabelSetText(ScrHandle, LabMinId[CalState - 2], buf);
	sprintf(buf, "%.2g", JoyAxis[axis]*1.1);
	GfuiLabelSetText(ScrHandle, LabMaxId[CalState - 2], buf);
	advanceStep();
	break;
    }
    GfuiLabelSetText(ScrHandle, InstId, Instructions[CalState]);
}


static void
Idle2(void)
{
    int		mask;
    int		b, i;
    int		index;

    for (index = 0; index < GFCTRL_JOY_NUMBER; index++) {
	if (Joystick[index]) {
	    Joystick[index]->read(&b, &JoyAxis[index * GFCTRL_JOY_MAX_AXES]);
	    
	    /* Joystick buttons */
	    for (i = 0, mask = 1; i < 32; i++, mask *= 2) {
		if (((b & mask) != 0) && ((JoyButtons[index] & mask) == 0)) {
		    /* Button fired */
		    JoyCalAutomaton();
		    if (CalState >= NbCalSteps) {
			GfuiApp().eventLoop().setIdleCB(0);
		    }
		    GfuiApp().eventLoop().postRedisplay();
		    JoyButtons[index] = b;
		    return;
		}
	    }
	    JoyButtons[index] = b;
	}
    }

    /* Let CPU take breath (and fans stay at low and quite speed) */
    GfSleep(0.001);
}


static void
onActivate(void * /* dummy */)
{
    int i;
    int index;
    int step;
    
    // Create and test joysticks ; only keep the up and running ones.
    for (index = 0; index < GFCTRL_JOY_NUMBER; index++) {
	Joystick[index] = new jsJoystick(index);
	if (Joystick[index]->notWorking()) {
	    /* don't configure the joystick */
	    delete Joystick[index];
	    Joystick[index] = 0;
	}
    }

    CalState = 0;
    GfuiLabelSetText(ScrHandle, InstId, Instructions[CalState]);
    GfuiApp().eventLoop().setIdleCB(Idle2);
    GfuiApp().eventLoop().postRedisplay();
    for (index = 0; index < GFCTRL_JOY_NUMBER; index++) {
	if (Joystick[index]) {
	    Joystick[index]->read(&JoyButtons[index], &JoyAxis[index * GFCTRL_JOY_MAX_AXES]); /* initial value */
	}
    }

    for (i = 0; i < NbMaxCalAxis; i++) {
	if (i > 0) {
	    step = i + 2;
	} else {
	    step = i + 1;
	}
	if (Cmd[step + CmdOffset].ref.type == GFCTRL_TYPE_JOY_AXIS) {
	    GfuiLabelSetText(ScrHandle, LabAxisId[i], GfctrlGetNameByRef(GFCTRL_TYPE_JOY_AXIS, Cmd[step + CmdOffset].ref.index));
	} else {
	    GfuiLabelSetText(ScrHandle, LabAxisId[i], "---");
	}
	GfuiLabelSetText(ScrHandle, LabMinId[i], "");
 	GfuiLabelSetText(ScrHandle, LabMaxId[i], "");
    }
}


void *
JoyCalMenuInit(void *nextMenu, tCmdInfo *cmd, int maxcmd)
{
    int i;
    char pszBuf[64];

    Cmd = cmd;
    MaxCmd = maxcmd;
    NextMenuHandle = nextMenu;

    if (ScrHandle) {
	return ScrHandle;
    }
    
    // Create screen, load menu XML descriptor and create static controls.
    ScrHandle = GfuiScreenCreateEx(NULL, NULL, onActivate, NULL, NULL, 1);

    void *menuXMLDescHdle = LoadMenuXML("joystickconfigmenu.xml");

    CreateStaticControls(menuXMLDescHdle, ScrHandle);

    // Create joystick axis label controls (axis name, axis Id, axis min value, axis max value)
    for (i = 0; i < NbMaxCalAxis; i++) {
	sprintf(pszBuf, "%saxislabel", LabName[i]);
	LabAxisId[i] = CreateLabelControl(ScrHandle, menuXMLDescHdle, pszBuf);
	sprintf(pszBuf, "%sminlabel", LabName[i]);
	LabMinId[i] = CreateLabelControl(ScrHandle, menuXMLDescHdle, pszBuf);
	sprintf(pszBuf, "%smaxlabel", LabName[i]);
	LabMaxId[i] = CreateLabelControl(ScrHandle, menuXMLDescHdle, pszBuf);
    }

    // Create instruction variable label.
    InstId = CreateLabelControl(ScrHandle, menuXMLDescHdle, "instructionlabel");
    
    // Create Back and Reset buttons.
    CreateButtonControl(ScrHandle, menuXMLDescHdle, "nextbutton", NULL, onNext);
    CreateButtonControl(ScrHandle, menuXMLDescHdle, "resetbutton", NULL, onActivate);

    // Close menu XML descriptor.
    GfParmReleaseHandle(menuXMLDescHdle);
    
    // Register keyboard shortcuts.
    GfuiMenuDefaultKeysAdd(ScrHandle);
    GfuiAddKey(ScrHandle, GFUIK_ESCAPE, "Next", NULL, onNext, NULL);
    GfuiAddKey(ScrHandle, GFUIK_RETURN, "Next", NULL, onNext, NULL);

    return ScrHandle;
}





