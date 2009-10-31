/***************************************************************************

    file        : controlconfig.cpp
    created     : Wed Mar 12 21:20:34 CET 2003
    copyright   : (C) 2003 by Eric Espi�                        
    email       : eric.espie@torcs.org   
    version     : $Id: controlconfig.cpp,v 1.5 19 Mar 2006 18:15:16  torcs Exp $                                  

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
    @version	$Id: controlconfig.cpp,v 1.6 2008/03/27 21:26:51 torcs Exp $
*/


#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <string>

#include <tgfclient.h>
#include <track.h>
#include <robot.h>
#include <playerpref.h>

#include <plib/js.h>

#include "controlconfig.h"
#include "mouseconfig.h"
#include "joystickconfig.h"

static void 	*ScrHandle = NULL;
static void	*PrevScrHandle = NULL;
static void	*PrefHdle = NULL;

static int	MouseCalButton;
static int	JoyCalButton;

static tCtrlMouseInfo	MouseInfo;
static char	CurrentSection[256];

/* Control command information */
static tCmdInfo Cmd[] = {
    {HM_ATT_LEFTSTEER,  {1,  GFCTRL_TYPE_MOUSE_AXIS},   0, 0, HM_ATT_LEFTSTEER_MIN,  0, HM_ATT_LEFTSTEER_MAX,  0, HM_ATT_LEFTSTEER_POW,  1.0, 1},
    {HM_ATT_RIGHTSTEER, {2,  GFCTRL_TYPE_MOUSE_AXIS},   0, 0, HM_ATT_RIGHTSTEER_MIN, 0, HM_ATT_RIGHTSTEER_MAX, 0, HM_ATT_RIGHTSTEER_POW, 1.0, 1},
    {HM_ATT_THROTTLE,   {1,  GFCTRL_TYPE_MOUSE_BUT},    0, 0, HM_ATT_THROTTLE_MIN,   0, HM_ATT_THROTTLE_MAX,   0, HM_ATT_THROTTLE_POW,   1.0, 1},
    {HM_ATT_BRAKE,      {2,  GFCTRL_TYPE_MOUSE_BUT},    0, 0, HM_ATT_BRAKE_MIN,      0, HM_ATT_BRAKE_MAX,      0, HM_ATT_BRAKE_POW,      1.0, 1},
    {HM_ATT_CLUTCH,     {3,  GFCTRL_TYPE_MOUSE_BUT},    0, 0, HM_ATT_CLUTCH_MIN,     0, HM_ATT_CLUTCH_MAX,     0, HM_ATT_CLUTCH_POW,     1.0, 1},
    {HM_ATT_ABS_CMD,    {-1, GFCTRL_TYPE_NOT_AFFECTED}, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {HM_ATT_ASR_CMD,    {-1, GFCTRL_TYPE_NOT_AFFECTED}, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {HM_ATT_SPDLIM_CMD, {-1, GFCTRL_TYPE_NOT_AFFECTED}, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {HM_ATT_LIGHT1_CMD, {-1, GFCTRL_TYPE_NOT_AFFECTED}, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {HM_ATT_GEAR_R,     {-1, GFCTRL_TYPE_NOT_AFFECTED}, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {HM_ATT_GEAR_N,     {-1, GFCTRL_TYPE_NOT_AFFECTED}, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {HM_ATT_DN_SHFT,    {-1, GFCTRL_TYPE_NOT_AFFECTED}, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {HM_ATT_UP_SHFT,    {-1, GFCTRL_TYPE_NOT_AFFECTED}, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {HM_ATT_GEAR_1,     {-1, GFCTRL_TYPE_NOT_AFFECTED}, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {HM_ATT_GEAR_2,     {-1, GFCTRL_TYPE_NOT_AFFECTED}, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {HM_ATT_GEAR_3,     {-1, GFCTRL_TYPE_NOT_AFFECTED}, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {HM_ATT_GEAR_4,     {-1, GFCTRL_TYPE_NOT_AFFECTED}, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {HM_ATT_GEAR_5,     {-1, GFCTRL_TYPE_NOT_AFFECTED}, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {HM_ATT_GEAR_6,     {-1, GFCTRL_TYPE_NOT_AFFECTED}, 0, 0, 0, 0, 0, 0, 0, 0, 1}
};

static const int MaxCmd = sizeof(Cmd) / sizeof(Cmd[0]);
static const int ICmdReverseGear = 9;
static const int ICmdNeutralGear = 10;

/* Command editbox display info according to the selected gear changing mode */
typedef struct tCmdDispInfo
{
    unsigned gearChangeModeMask;
    int      y;
} tCmdDispInfo;

static tCmdDispInfo CmdDispInfo[] = {
    { GEAR_MODE_AUTO | GEAR_MODE_SEQ | GEAR_MODE_GRID, 400 },
    { GEAR_MODE_AUTO | GEAR_MODE_SEQ | GEAR_MODE_GRID, 370 },
    { GEAR_MODE_AUTO | GEAR_MODE_SEQ | GEAR_MODE_GRID, 340 },
    { GEAR_MODE_AUTO | GEAR_MODE_SEQ | GEAR_MODE_GRID, 310 },
    { GEAR_MODE_AUTO | GEAR_MODE_SEQ | GEAR_MODE_GRID, 280 },
    { GEAR_MODE_AUTO | GEAR_MODE_SEQ | GEAR_MODE_GRID, 250 },
    { GEAR_MODE_AUTO | GEAR_MODE_SEQ | GEAR_MODE_GRID, 220 },
    { GEAR_MODE_AUTO | GEAR_MODE_SEQ | GEAR_MODE_GRID, 190 },
    { GEAR_MODE_AUTO | GEAR_MODE_SEQ | GEAR_MODE_GRID, 160 },
    { GEAR_MODE_AUTO | GEAR_MODE_SEQ | GEAR_MODE_GRID, 400 },
    { GEAR_MODE_AUTO | GEAR_MODE_SEQ | GEAR_MODE_GRID, 370 },
    { GEAR_MODE_AUTO | GEAR_MODE_SEQ,                  340 },
    { GEAR_MODE_AUTO | GEAR_MODE_SEQ,                  310 },
    {                                  GEAR_MODE_GRID, 340 },
    {                                  GEAR_MODE_GRID, 310 },
    {                                  GEAR_MODE_GRID, 280 },
    {                                  GEAR_MODE_GRID, 250 },
    {                                  GEAR_MODE_GRID, 220 },
    {                                  GEAR_MODE_GRID, 190 }
};

static jsJoystick	*Joystick[GFCTRL_JOY_NUMBER] = {NULL};
static float		JoyAxis[GFCTRL_JOY_MAX_AXES * GFCTRL_JOY_NUMBER] = {0};
static float 		JoyAxisCenter[GFCTRL_JOY_MAX_AXES * GFCTRL_JOY_NUMBER];
static int		JoyButtons[GFCTRL_JOY_NUMBER] = {0};

static float SteerSensVal;
static float DeadZoneVal;

static char buf[1024];

static int SteerSensEditId;
static int DeadZoneEditId;
static tGearChangeMode GearChangeMode;

static int ReloadValues = 1;


static void
onSteerSensChange(void * /* dummy */)
{
    char	*val;
    float	fv;

    val = GfuiEditboxGetString(ScrHandle, SteerSensEditId);
    if (sscanf(val, "%f", &fv) == 1) {
	sprintf(buf, "%f", fv);
	SteerSensVal = fv;
	GfuiEditboxSetString(ScrHandle, SteerSensEditId, buf);
    } else {
	GfuiEditboxSetString(ScrHandle, SteerSensEditId, "");
    }
    
}

static void
onDeadZoneChange(void * /* dummy */)
{
    char	*val;
    float	fv;

    val = GfuiEditboxGetString(ScrHandle, DeadZoneEditId);
    if (sscanf(val, "%f", &fv) == 1) {
	sprintf(buf, "%f", fv);
	DeadZoneVal = fv;
	GfuiEditboxSetString(ScrHandle, DeadZoneEditId, buf);
    } else {
	GfuiEditboxSetString(ScrHandle, SteerSensEditId, "");
    }
    
}

/* Save settings in the players preferences and go back to previous screen */
static void
onSave(void * /* dummy */)
{
    ControlPutSettings();

    GfuiScreenActivate(PrevScrHandle);
}

static void
updateButtonText(void)
{
    int		i;
    const char	*str;
    int		displayMouseCal = GFUI_INVISIBLE;
    int		displayJoyCal = GFUI_INVISIBLE;

    /* For each control: */
    for (i = 0; i < MaxCmd; i++) {

	/* Update associated editbox according to detected input device action */
	str = GfctrlGetNameByRef(Cmd[i].ref.type, Cmd[i].ref.index);
	if (str) {
	    GfuiButtonSetText (ScrHandle, Cmd[i].Id, str);
	} else {
	    GfuiButtonSetText (ScrHandle, Cmd[i].Id, "---");
	}

	/* According to detected action, set the visibility of the mouse or joystick
	   calibration button */
	if (Cmd[i].ref.type == GFCTRL_TYPE_MOUSE_AXIS) {
	    displayMouseCal = GFUI_VISIBLE;
	} else if (Cmd[i].ref.type == GFCTRL_TYPE_JOY_AXIS) {
	    displayJoyCal = GFUI_VISIBLE;
	}
    }

    /* Update Steer Sensibility editbox */
    sprintf(buf, "%f", SteerSensVal);
    GfuiEditboxSetString(ScrHandle, SteerSensEditId, buf);

    /* Update Steer Dead Zone editbox */
    sprintf(buf, "%f", DeadZoneVal);
    GfuiEditboxSetString(ScrHandle, DeadZoneEditId, buf);

    /* Show / hide mouse / joystick calibration button(s),
       according to the detected input device actions */
    GfuiVisibilitySet(ScrHandle, MouseCalButton, displayMouseCal);
    GfuiVisibilitySet(ScrHandle, JoyCalButton, displayJoyCal);
}

static void
onFocusLost(void * /* dummy */)
{
    updateButtonText();
}

static int CurrentCmd;

static int InputWaited = 0;

static int
onKeyAction(unsigned char key, int /* modifier */, int state)
{
    const char *name;

    if (!InputWaited || state == GFUI_KEY_UP) {
	return 0;
    }
    if (key == 27) {
	/* escape */
	Cmd[CurrentCmd].ref.index = -1;
	Cmd[CurrentCmd].ref.type = GFCTRL_TYPE_NOT_AFFECTED;
    } else {
	name = GfctrlGetNameByRef(GFCTRL_TYPE_KEYBOARD, (int)key);
	Cmd[CurrentCmd].ref.index = (int)key;
	Cmd[CurrentCmd].ref.type = GFCTRL_TYPE_KEYBOARD;
    }

    glutIdleFunc(0);
    InputWaited = 0;
    updateButtonText();
    return 1;
}

static int
onSKeyAction(int key, int /* modifier */, int state)
{
    const char *name;

    if (!InputWaited || state == GFUI_KEY_UP) {
	return 0;
    }
    name = GfctrlGetNameByRef(GFCTRL_TYPE_SKEYBOARD, key);
    Cmd[CurrentCmd].ref.index = key;
    Cmd[CurrentCmd].ref.type = GFCTRL_TYPE_SKEYBOARD;

    glutIdleFunc(0);
    InputWaited = 0;
    updateButtonText();
    return 1;
}

static int
getMovedAxis(void)
{
    int		i;
    int		Index = -1;
    float	maxDiff = 0.3;

    for (i = 0; i < GFCTRL_JOY_MAX_AXES * GFCTRL_JOY_NUMBER; i++) {
	if (maxDiff < fabs(JoyAxis[i] - JoyAxisCenter[i])) {
	    maxDiff = fabs(JoyAxis[i] - JoyAxisCenter[i]);
	    Index = i;
	}
    }
    return Index;
}

/* GLUT idle function : For collecting input devices actions */
static void
Idle(void)
{
    int		mask;
    int		b, i;
    int		index;
    const char	*str;
    int		axis;

    GfctrlMouseGetCurrent(&MouseInfo);

    /* Check for a mouse button pressed */
    for (i = 0; i < 3; i++) {
	if (MouseInfo.edgedn[i]) {
	    glutIdleFunc(0);
	    InputWaited = 0;
	    str = GfctrlGetNameByRef(GFCTRL_TYPE_MOUSE_BUT, i);
	    Cmd[CurrentCmd].ref.index = i;
	    Cmd[CurrentCmd].ref.type = GFCTRL_TYPE_MOUSE_BUT;
	    GfuiButtonSetText (ScrHandle, Cmd[CurrentCmd].Id, str);
	    glutPostRedisplay();
	    return;
	}
    }

    /* Check for a mouse axis moved */
    for (i = 0; i < 4; i++) {
	if (MouseInfo.ax[i] > 20.0) {
	    glutIdleFunc(0);
	    InputWaited = 0;
	    str = GfctrlGetNameByRef(GFCTRL_TYPE_MOUSE_AXIS, i);
	    Cmd[CurrentCmd].ref.index = i;
	    Cmd[CurrentCmd].ref.type = GFCTRL_TYPE_MOUSE_AXIS;
	    GfuiButtonSetText (ScrHandle, Cmd[CurrentCmd].Id, str);
	    glutPostRedisplay();
	    return;
	}
    }

    /* Check for a Joystick button pressed */
    for (index = 0; index < GFCTRL_JOY_NUMBER; index++) {
	if (Joystick[index]) {
	    Joystick[index]->read(&b, &JoyAxis[index * GFCTRL_JOY_MAX_AXES]);

	    /* Joystick buttons */
	    for (i = 0, mask = 1; i < 32; i++, mask *= 2) {
		if (((b & mask) != 0) && ((JoyButtons[index] & mask) == 0)) {
		    /* Button i fired */
		    glutIdleFunc(0);
		    InputWaited = 0;
		    str = GfctrlGetNameByRef(GFCTRL_TYPE_JOY_BUT, i + 32 * index);
		    Cmd[CurrentCmd].ref.index = i + 32 * index;
		    Cmd[CurrentCmd].ref.type = GFCTRL_TYPE_JOY_BUT;
		    GfuiButtonSetText (ScrHandle, Cmd[CurrentCmd].Id, str);
		    glutPostRedisplay();
		    JoyButtons[index] = b;
		    return;
		}
	    }
	    JoyButtons[index] = b;
	}
    }

    /* detect joystick movement */
    axis = getMovedAxis();
    if (axis != -1) {
	glutIdleFunc(0);
	InputWaited = 0;
	Cmd[CurrentCmd].ref.type = GFCTRL_TYPE_JOY_AXIS;
	Cmd[CurrentCmd].ref.index = axis;
	str = GfctrlGetNameByRef(GFCTRL_TYPE_JOY_AXIS, axis);
	GfuiButtonSetText (ScrHandle, Cmd[CurrentCmd].Id, str);
	glutPostRedisplay();
	return;
    }

	/* Let CPU take breath (and fans stay at low and quiet speed) */
	GfuiSleep(0.001);
}

/* Push button callback for each command button : activate input devices action collection loop */
static void
onPush(void *vi)
{
    int		index;    
    long	i = (long)vi;
    
    /* Selected given command as the currently awaited one */
    CurrentCmd = i;

    /* Empty button text to tell the user we will soon be waiting for its input */
    GfuiButtonSetText (ScrHandle, Cmd[i].Id, "");

    /* Reset selected command action */
    Cmd[i].ref.index = -1;
    Cmd[i].ref.type = GFCTRL_TYPE_NOT_AFFECTED;

    /* State that a keyboard action is awaited */
    if (Cmd[CurrentCmd].keyboardPossible) {
	InputWaited = 1;
    }

    /* Read initial mouse status */
    glutIdleFunc(Idle);
    GfctrlMouseInitCenter();
    memset(&MouseInfo, 0, sizeof(MouseInfo));
    GfctrlMouseGetCurrent(&MouseInfo);

    /* Read initial joysticks status */
    for (index = 0; index < GFCTRL_JOY_NUMBER; index++) {
	if (Joystick[index]) {
	    Joystick[index]->read(&JoyButtons[index], &JoyAxis[index * GFCTRL_JOY_MAX_AXES]);
	}
    }
    memcpy(JoyAxisCenter, JoyAxis, sizeof(JoyAxisCenter));
}

static void
onActivate(void * /* dummy */)
{
    int	cmd;

    if (ReloadValues) {

        /* Load command settings from preference params for current player */
        ControlGetSettings();

	/* For each control : */
	for (cmd = 0; cmd < MaxCmd; cmd++) {

	    /* Show / hide control editbox according to selected gear changing mode code */
	    if (GearChangeMode & CmdDispInfo[cmd].gearChangeModeMask)
	    {
	        GfuiVisibilitySet(ScrHandle, Cmd[cmd].labelId, GFUI_VISIBLE);
	        GfuiVisibilitySet(ScrHandle, Cmd[cmd].Id, GFUI_VISIBLE);
	    }
	    else
	    {
	        GfuiVisibilitySet(ScrHandle, Cmd[cmd].labelId, GFUI_INVISIBLE);
	        GfuiVisibilitySet(ScrHandle, Cmd[cmd].Id, GFUI_INVISIBLE);
	    }
	}
    }
    
    updateButtonText();
}

static void
DevCalibrate(void *menu)
{
    ReloadValues = 0;
    GfuiScreenActivate(menu);
}


/* */
void *
ControlMenuInit(void *prevMenu, void *prefHdle, unsigned index, tGearChangeMode gearChangeMode)
{
    int		x, x2, i;
    int		jsInd;

    ReloadValues = 1;

    PrevScrHandle = prevMenu;

    PrefHdle = prefHdle;

    /* Select current player section in the players preferences */
    sprintf(CurrentSection, "%s/%s/%u", HM_SECT_PREF, HM_LIST_DRV, index);

    /* Set specified gear changing mode for current player */
    GearChangeMode = gearChangeMode;

    /* Don't recreate screen if already done */
    if (ScrHandle) {
	return ScrHandle;
    }

    /* Initialize joysticks layer */
    for (jsInd = 0; jsInd < GFCTRL_JOY_NUMBER; jsInd++) {
	if (Joystick[jsInd] == NULL) {
	    Joystick[jsInd] = new jsJoystick(jsInd);
	}
    
	if (Joystick[jsInd]->notWorking()) {
	    /* don't configure the joystick */
	    delete Joystick[jsInd];
	    Joystick[jsInd] = NULL;
	}
	else {
	  GfOut("Detected joystick #%d type '%s' %d axes\n", 
		jsInd, Joystick[jsInd]->getName(), Joystick[jsInd]->getNumAxes());
	}
    }
    
    ScrHandle = GfuiScreenCreateEx((float*)NULL, NULL, onActivate, NULL, (tfuiCallback)NULL, 1);

    void *param = LoadMenuXML("controlconfigmenu.xml");
    CreateStaticControls(param,ScrHandle);

    /* Default keyboard shortcuts */
    GfuiMenuDefaultKeysAdd(ScrHandle);

    /* Screen coordinates for labels, buttons, ... */
    x = 10;
    x2 = 210;

    /* For each control (in Cmd array), create the associated label and editbox */
    for (i = 0; i < MaxCmd; i++) 
    {
	std::string strCmd = Cmd[i].name;
	Cmd[i].labelId = CreateLabelControl(ScrHandle,param,strCmd.c_str());
	std::string strCmdEdit = strCmd+" button";
	Cmd[i].Id = CreateButtonControlEx(ScrHandle,param,strCmdEdit.c_str(),(void*)i,onPush,NULL,(tfuiCallback)NULL,onFocusLost);
	
	/* If first column done, change to the second */
	if (i == MaxCmd / 2 - 1) {
	    x = 320;
	    x2 = 220;
	}
    }
    
    /* Steer Sensibility label and associated editbox */
    CreateLabelControl(ScrHandle,param,"Steer Sensitivity");
    SteerSensEditId = CreateEditControl(ScrHandle,param,"SteerSensitivityEdit",NULL,NULL,onSteerSensChange);

    /* Steer Dead Zone label and associated editbox */
    CreateLabelControl(ScrHandle,param,"Steer Dead Zone");
    DeadZoneEditId = CreateEditControl(ScrHandle,param,"Steer Dead Zone Edit",NULL,NULL,onDeadZoneChange);

    /* Save button and associated keyboard shortcut */
    CreateButtonControl(ScrHandle,param,"save",NULL,onSave);
    GfuiAddKey(ScrHandle, 13 /* Return */, "Save", NULL, onSave, NULL);

    /* Mouse calibration screen access button */
    MouseCalButton = CreateButtonControl(ScrHandle,param,"mousecalibrate",MouseCalMenuInit(ScrHandle, Cmd, MaxCmd), DevCalibrate);

    /* Joystick/joypad/wheel calibration screen access button */
    JoyCalButton = CreateButtonControl(ScrHandle,param,"joycalibrate",JoyCalMenuInit(ScrHandle, Cmd, MaxCmd), DevCalibrate);

    /* Cancel button and associated keyboard shortcut */
    GfuiAddKey(ScrHandle, 27 /* Escape */, "Cancel", prevMenu, GfuiScreenActivate, NULL);
    CreateButtonControl(ScrHandle,param,"cancel",prevMenu,GfuiScreenActivate);

    /* General callbacks for keyboard keys and special keys */
    GfuiKeyEventRegister(ScrHandle, onKeyAction);
    GfuiSKeyEventRegister(ScrHandle, onSKeyAction);

    return ScrHandle;
}

/* From parms (prefHdle) to global vars (Cmd, SteerSensVal, DeadZoneVal) */
void ControlGetSettings(void *prefHdle, unsigned index)
{
    int		iCmd;
    const char	*prm;
    tCtrlRef	*ref;

    /* If handle on preferences params not given, get current */
    if (!prefHdle)
        prefHdle = PrefHdle;

    /* Select current player section in the players preferences if specified */
    if (index > 0)
        sprintf(CurrentSection, "%s/%s/%u", HM_SECT_PREF, HM_LIST_DRV, index);

    /* For each control : */
    for (iCmd = 0; iCmd < MaxCmd; iCmd++) {
        prm = GfctrlGetNameByRef(Cmd[iCmd].ref.type, Cmd[iCmd].ref.index);
	if (!prm) {
	    prm = "---";
	}
	/* Load associated command settings from preferences params for the current player,
	   by default from the default "mouse" settings */
	prm = GfParmGetStr(prefHdle, HM_SECT_MOUSEPREF, Cmd[iCmd].name, prm);
	prm = GfParmGetStr(prefHdle, CurrentSection, Cmd[iCmd].name, prm);
	ref = GfctrlGetRefByName(prm);
	Cmd[iCmd].ref.type = ref->type;
	Cmd[iCmd].ref.index = ref->index;
	
	if (Cmd[iCmd].minName) {
	    Cmd[iCmd].min = GfParmGetNum(prefHdle, HM_SECT_MOUSEPREF, Cmd[iCmd].minName, NULL, Cmd[iCmd].min);
	    Cmd[iCmd].min = GfParmGetNum(prefHdle, CurrentSection, Cmd[iCmd].minName, NULL, Cmd[iCmd].min);
	}
	if (Cmd[iCmd].maxName) {
	    Cmd[iCmd].max = GfParmGetNum(prefHdle, HM_SECT_MOUSEPREF, Cmd[iCmd].maxName, NULL, Cmd[iCmd].max);
	    Cmd[iCmd].max = GfParmGetNum(prefHdle, CurrentSection, Cmd[iCmd].maxName, NULL, Cmd[iCmd].max);
	}
	if (Cmd[iCmd].powName) {
	    Cmd[iCmd].pow = GfParmGetNum(prefHdle, HM_SECT_MOUSEPREF, Cmd[iCmd].powName, NULL, Cmd[iCmd].pow);
	    Cmd[iCmd].pow = GfParmGetNum(prefHdle, CurrentSection, Cmd[iCmd].powName, NULL, Cmd[iCmd].pow);
	}
    }

    /* Load also Steer sensibility (default from mouse prefs) */
    SteerSensVal = GfParmGetNum(prefHdle, HM_SECT_MOUSEPREF, HM_ATT_STEER_SENS, NULL, 0);
    SteerSensVal = GfParmGetNum(prefHdle, CurrentSection, HM_ATT_STEER_SENS, NULL, SteerSensVal);

    /* Load also Dead zone (default from mouse prefs) */
    DeadZoneVal = GfParmGetNum(prefHdle, HM_SECT_MOUSEPREF, HM_ATT_STEER_DEAD, NULL, 0);
    DeadZoneVal = GfParmGetNum(prefHdle, CurrentSection, HM_ATT_STEER_DEAD, NULL, DeadZoneVal);
}

/* From global vars (Cmd, SteerSensVal, DeadZoneVal) to parms (prefHdle) */
void ControlPutSettings(void *prefHdle, unsigned index, tGearChangeMode gearChangeMode)
{
    int		iCmd;
    const char	*str;

    /* If handle on preferences not given, get current */
    if (!prefHdle)
        prefHdle = PrefHdle;

    /* Change current player section in the players preferences if specified */
    if (index > 0)
        sprintf(CurrentSection, "%s/%s/%d", HM_SECT_PREF, HM_LIST_DRV, index);

    /* Select current player gear change mode if not specified */
    if (gearChangeMode == GEAR_MODE_NONE)
        gearChangeMode = GearChangeMode;

    /* Allow neutral gear in sequential mode if no reverse gear command defined */
    if (gearChangeMode == GEAR_MODE_SEQ
	&& !GfctrlGetNameByRef(Cmd[ICmdReverseGear].ref.type, Cmd[ICmdReverseGear].ref.index))
	GfParmSetStr(prefHdle, CurrentSection, HM_ATT_SEQSHFT_ALLOW_NEUTRAL, HM_VAL_YES);
    else
	GfParmSetStr(prefHdle, CurrentSection, HM_ATT_SEQSHFT_ALLOW_NEUTRAL, HM_VAL_NO);

    /* Release gear lever goes neutral in grid mode if no neutral gear command defined */
    if (gearChangeMode == GEAR_MODE_GRID
	&& !GfctrlGetNameByRef(Cmd[ICmdNeutralGear].ref.type, Cmd[ICmdNeutralGear].ref.index))
	GfParmSetStr(prefHdle, CurrentSection, HM_ATT_REL_BUT_NEUTRAL, HM_VAL_YES);
    else
	GfParmSetStr(prefHdle, CurrentSection, HM_ATT_REL_BUT_NEUTRAL, HM_VAL_NO);

    GfParmSetNum(prefHdle, CurrentSection, HM_ATT_STEER_SENS, NULL, SteerSensVal);
    GfParmSetNum(prefHdle, CurrentSection, HM_ATT_STEER_DEAD, NULL, DeadZoneVal);

    for (iCmd = 0; iCmd < MaxCmd; iCmd++) {
	str = GfctrlGetNameByRef(Cmd[iCmd].ref.type, Cmd[iCmd].ref.index);
	if (str) {
	    GfParmSetStr(prefHdle, CurrentSection, Cmd[iCmd].name, str);
	} else {
	    GfParmSetStr(prefHdle, CurrentSection, Cmd[iCmd].name, "");
	}
	if (Cmd[iCmd].minName) {
	    GfParmSetNum(prefHdle, CurrentSection, Cmd[iCmd].minName, NULL, Cmd[iCmd].min);
	}
	if (Cmd[iCmd].maxName) {
	    GfParmSetNum(prefHdle, CurrentSection, Cmd[iCmd].maxName, NULL, Cmd[iCmd].max);
	}
	if (Cmd[iCmd].powName) {
	    GfParmSetNum(prefHdle, CurrentSection, Cmd[iCmd].powName, NULL, Cmd[iCmd].pow);
	}
    }
}
