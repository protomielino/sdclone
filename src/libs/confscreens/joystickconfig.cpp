/***************************************************************************

    file                 : joystickconfig.cpp
    created              : Wed Mar 21 21:46:11 CET 2001
    copyright            : (C) 2001 by Eric Espie
    email                : eric.espie@torcs.org
    version              : $Id: joystickconfig.cpp,v 1.5 2003/11/08 16:37:18 torcs Exp $

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
#include <track.h>
#include <robot.h>
#include "driverconfig.h"
#include <playerpref.h>

#include "controlconfig.h"
#include "joystickconfig.h"

static void	*scrHandle2 = NULL;

static tCmdInfo *Cmd;
static int maxCmd;

static char buf[1024];

static jsJoystick 	*Joystick[GFCTRL_JOY_NUMBER];
static float 		JoyAxis[GFCTRL_JOY_MAX_AXES * GFCTRL_JOY_NUMBER];
static float 		JoyAxisCenter[GFCTRL_JOY_MAX_AXES * GFCTRL_JOY_NUMBER];
static int   		JoyButtons[GFCTRL_JOY_NUMBER];

#define NB_STEPS	6

const int OFFSET_CMD	= -1;

static const char *Instructions[] = {
    "Center the joystick then press a button",
    "Steer left then press a button",
    "Steer right then press a button",
    "Apply full throttle then press a button",
    "Apply full brake then press a button",
    "Apply full clutch then press a button",
    "Calibration terminated",
    "Calibration failed"
};

static int CalState;
static int InstId;

static const char *LabName[] = { "Steer", "Throttle", "Brake", "Clutch" };
static int  LabAxisId[4];
static int  LabMinId[4];
static int  LabMaxId[4];



static void
onBack(void *prevMenu)
{
    int index;

    /* Release up and running joysticks */
    for (index = 0; index < GFCTRL_JOY_NUMBER; index++)
	if (Joystick[index]) {
	    delete Joystick[index];
	    Joystick[index] = 0;
	}

    /* Back to previous screen */
    GfuiScreenActivate(prevMenu);
}

static void advanceStep (void)
{
    do {
	CalState++;
    } while (Cmd[CalState + OFFSET_CMD].ref.type != GFCTRL_TYPE_JOY_AXIS && CalState < NB_STEPS);
}


static void
JoyCalAutomaton(void)
{
    static int axis;

    switch (CalState) {
    case 0:
	memcpy(JoyAxisCenter, JoyAxis, sizeof(JoyAxisCenter));
	advanceStep();
	break;
    case 1:
	axis = Cmd[CalState + OFFSET_CMD].ref.index;
	Cmd[CalState + OFFSET_CMD].min = JoyAxis[axis];
	Cmd[CalState + OFFSET_CMD].max = JoyAxisCenter[axis];
	Cmd[CalState + OFFSET_CMD].pow = 1.0;
	sprintf(buf, "%.2g", JoyAxis[axis]);
	GfuiLabelSetText(scrHandle2, LabMinId[0], buf);
	advanceStep();
	break;
    case 2:
	axis = Cmd[CalState + OFFSET_CMD].ref.index;
	Cmd[CalState + OFFSET_CMD].min = JoyAxisCenter[axis];
	Cmd[CalState + OFFSET_CMD].max = JoyAxis[axis];
	Cmd[CalState + OFFSET_CMD].pow = 1.0;
	sprintf(buf, "%.2g", JoyAxis[axis]);
	GfuiLabelSetText(scrHandle2, LabMaxId[0], buf);
	advanceStep();
	break;
    case 3:
    case 4:
    case 5:
	axis = Cmd[CalState + OFFSET_CMD].ref.index;
	Cmd[CalState + OFFSET_CMD].min = JoyAxisCenter[axis];
	Cmd[CalState + OFFSET_CMD].max = JoyAxis[axis]*1.1;
	Cmd[CalState + OFFSET_CMD].pow = 1.2;
	sprintf(buf, "%.2g", JoyAxisCenter[axis]);
	GfuiLabelSetText(scrHandle2, LabMinId[CalState - 2], buf);
	sprintf(buf, "%.2g", JoyAxis[axis]*1.1);
	GfuiLabelSetText(scrHandle2, LabMaxId[CalState - 2], buf);
	advanceStep();
	break;
    }
    GfuiLabelSetText(scrHandle2, InstId, Instructions[CalState]);
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
		    if (CalState >= NB_STEPS) {
			glutIdleFunc(0);
		    }
		    glutPostRedisplay();
		    JoyButtons[index] = b;
		    return;
		}
	    }
	    JoyButtons[index] = b;
	}
    }

	/* Let CPU take breath (and fans stay at low and quite speed) */
	GfuiSleep(0.001);
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
    GfuiLabelSetText(scrHandle2, InstId, Instructions[CalState]);
    glutIdleFunc(Idle2);
    glutPostRedisplay();
    for (index = 0; index < GFCTRL_JOY_NUMBER; index++) {
	if (Joystick[index]) {
	    Joystick[index]->read(&JoyButtons[index], &JoyAxis[index * GFCTRL_JOY_MAX_AXES]); /* initial value */
	}
    }

    for (i = 0; i < 4; i++) {
	if (i > 0) {
	    step = i + 2;
	} else {
	    step = i + 1;
	}
	if (Cmd[step + OFFSET_CMD].ref.type == GFCTRL_TYPE_JOY_AXIS) {
	    GfuiLabelSetText(scrHandle2, LabAxisId[i], GfctrlGetNameByRef(GFCTRL_TYPE_JOY_AXIS, Cmd[step + OFFSET_CMD].ref.index));
	} else {
	    GfuiLabelSetText(scrHandle2, LabAxisId[i], "---");
	}
	GfuiLabelSetText(scrHandle2, LabMinId[i], "");
 	GfuiLabelSetText(scrHandle2, LabMaxId[i], "");
    }
}


void *
JoyCalMenuInit(void *prevMenu, tCmdInfo *cmd, int maxcmd)
{
    int x, y, dy, i, index;

    Cmd = cmd;
    maxCmd = maxcmd;

    if (scrHandle2) {
	return scrHandle2;
    }
    
    // Create screen and controls.
    scrHandle2 = GfuiScreenCreateEx(NULL, NULL, onActivate, NULL, NULL, 1);
    GfuiTitleCreate(scrHandle2, "Joystick Calibration", 0);

    GfuiScreenAddBgImg(scrHandle2, "data/img/splash-joycal.png");

    x = 128;
    y = 300;
    dy = 50;
    
    for (i = 0; i < 4; i++) {
	GfuiLabelCreate(scrHandle2, LabName[i], GFUI_FONT_LARGE, x, y, GFUI_ALIGN_HC_VC, 0);
	LabAxisId[i] = GfuiLabelCreate(scrHandle2, "                ", GFUI_FONT_MEDIUM, 2 * x, y, GFUI_ALIGN_HC_VC, 0);
	LabMinId[i] = GfuiLabelCreate(scrHandle2,  "                ", GFUI_FONT_MEDIUM, 3 * x, y, GFUI_ALIGN_HC_VC, 0);
	LabMaxId[i] = GfuiLabelCreate(scrHandle2,  "                ", GFUI_FONT_MEDIUM, 4 * x, y, GFUI_ALIGN_HC_VC, 0);
	y -= dy;
    }

    InstId = GfuiLabelCreate(scrHandle2, Instructions[0], GFUI_FONT_MEDIUM, 320, 80, GFUI_ALIGN_HC_VB, 60);

    GfuiButtonCreate(scrHandle2, "Back", GFUI_FONT_LARGE, 160, 40, 150, GFUI_ALIGN_HC_VB, GFUI_MOUSE_UP,
		     prevMenu, onBack, NULL, (tfuiCallback)NULL, (tfuiCallback)NULL);
    
    GfuiButtonCreate(scrHandle2, "Reset", GFUI_FONT_LARGE, 480, 40, 150, GFUI_ALIGN_HC_VB, GFUI_MOUSE_UP,
		     NULL, onActivate, NULL, (tfuiCallback)NULL, (tfuiCallback)NULL);

    // Register keyboard shortcuts.
    GfuiMenuDefaultKeysAdd(scrHandle2);
    GfuiAddKey(scrHandle2, 27, "Back", prevMenu, onBack, NULL);
    GfuiAddKey(scrHandle2, 13, "Back", prevMenu, onBack, NULL);

    return scrHandle2;
}





