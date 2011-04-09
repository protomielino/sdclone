/***************************************************************************

    file        : controlconfig.cpp
    created     : Wed Mar 12 21:20:34 CET 2003
    copyright   : (C) 2003 by Eric Espie
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
    		Human player control configuration menu
    @author	<a href=mailto:eric.espie@torcs.org>Eric Espie</a>
    @version	$Id$
*/

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

#include <tgfclient.h>
#include <track.h>
#include <robot.h>
#include <playerpref.h>

#include "controlconfig.h"
#include "mouseconfig.h"
#include "joystickconfig.h"


static void *ScrHandle = NULL;
static void	*PrevScrHandle = NULL;
static void	*PrefHdle = NULL;

static tCtrlMouseInfo MouseInfo;
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
    {HM_ATT_GEAR_6,     {-1, GFCTRL_TYPE_NOT_AFFECTED}, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {HM_ATT_EBRAKE_CMD, {-1, GFCTRL_TYPE_NOT_AFFECTED}, 0, 0, 0, 0, 0, 0, 0, 0, 1}
};

static const int MaxCmd = sizeof(Cmd) / sizeof(Cmd[0]);
static const int ICmdReverseGear = 9;
static const int ICmdNeutralGear = 10;

/* Command editbox display info according to the selected gear changing mode */
typedef struct tCmdDispInfo
{
    unsigned gearChangeModeMask;
} tCmdDispInfo;

static tCmdDispInfo CmdDispInfo[] = {
    { GEAR_MODE_AUTO | GEAR_MODE_SEQ | GEAR_MODE_GRID }, // LEFTSTEER,
    { GEAR_MODE_AUTO | GEAR_MODE_SEQ | GEAR_MODE_GRID }, // RIGHTSTEER
    { GEAR_MODE_AUTO | GEAR_MODE_SEQ | GEAR_MODE_GRID }, // THROTTLE, 
    { GEAR_MODE_AUTO | GEAR_MODE_SEQ | GEAR_MODE_GRID }, // BRAKE,    
    { GEAR_MODE_AUTO | GEAR_MODE_SEQ | GEAR_MODE_GRID }, // CLUTCH,   
    { GEAR_MODE_AUTO | GEAR_MODE_SEQ | GEAR_MODE_GRID }, // ABS_CMD,  
    { GEAR_MODE_AUTO | GEAR_MODE_SEQ | GEAR_MODE_GRID }, // ASR_CMD,  
    { GEAR_MODE_AUTO | GEAR_MODE_SEQ | GEAR_MODE_GRID }, // SPDLIM_CMD
    { GEAR_MODE_AUTO | GEAR_MODE_SEQ | GEAR_MODE_GRID }, // LIGHT1_CMD
    { GEAR_MODE_AUTO | GEAR_MODE_SEQ | GEAR_MODE_GRID }, // GEAR_R,   
    { GEAR_MODE_AUTO | GEAR_MODE_SEQ | GEAR_MODE_GRID }, // GEAR_N,   
    { GEAR_MODE_AUTO | GEAR_MODE_SEQ,                 }, // DN_SHFT,  
    { GEAR_MODE_AUTO | GEAR_MODE_SEQ,                 }, // UP_SHFT,  
    {                                  GEAR_MODE_GRID }, // GEAR_1,   
    {                                  GEAR_MODE_GRID }, // GEAR_2,   
    {                                  GEAR_MODE_GRID }, // GEAR_3,   
    {                                  GEAR_MODE_GRID }, // GEAR_4,   
    {                                  GEAR_MODE_GRID }, // GEAR_5,   
    {                                  GEAR_MODE_GRID }, // GEAR_6,   
    { GEAR_MODE_AUTO | GEAR_MODE_SEQ | GEAR_MODE_GRID }	 // EBRAKE_CMD
};

static jsJoystick	*Joystick[GFCTRL_JOY_NUMBER];
static float		JoyAxis[GFCTRL_JOY_MAX_AXES * GFCTRL_JOY_NUMBER];
static float 		JoyAxisCenter[GFCTRL_JOY_MAX_AXES * GFCTRL_JOY_NUMBER];
static int		JoyButtons[GFCTRL_JOY_NUMBER];

static float SteerSensVal;
static float DeadZoneVal;
static float SteerSpeedSensVal;

static char buf[1024];

static int SteerSensEditId;
static int DeadZoneLabelId;
static int DeadZoneEditId;
static int SteerSpeedSensEditId;
static int CalibrateButtonId;

static tGearChangeMode GearChangeMode;

static int ReloadValues = 1;

static int AcceptMouseClicks = 1;

static int MouseCalNeeded;
static int JoyCalNeeded;

static void
onSteerSensChange(void * /* dummy */)
{
    char	*val;
    float	fv;

    val = GfuiEditboxGetString(ScrHandle, SteerSensEditId);
    if (sscanf(val, "%f", &fv) == 1) {
	if (fv <= 0.0)
	    fv = 1.0e-6;
	sprintf(buf, "%6.4f", fv);
	GfuiEditboxSetString(ScrHandle, SteerSensEditId, buf);
	SteerSensVal = fv;
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
	if (fv < 0.0)
	    fv = 0.0;
	else if (fv > 1.0)
		fv = 1.0;
	sprintf(buf, "%6.4f", fv);
	GfuiEditboxSetString(ScrHandle, DeadZoneEditId, buf);
	DeadZoneVal = fv;
    } else {
	GfuiEditboxSetString(ScrHandle, SteerSensEditId, "");
    }
    
}

static void
onSteerSpeedSensChange(void * /* dummy */)
{
    char	*val;
    float	fv;

    val = GfuiEditboxGetString(ScrHandle, SteerSpeedSensEditId);
    if (sscanf(val, "%f", &fv) == 1) {
	if (fv < 0.0)
	    fv = 0.0;
	sprintf(buf, "%6.4f", fv);
	GfuiEditboxSetString(ScrHandle, SteerSpeedSensEditId, buf);
	SteerSpeedSensVal = fv;
    } else {
	GfuiEditboxSetString(ScrHandle, SteerSpeedSensEditId, "");
    }
    
}

/* Quit current menu */
static void
onQuit(void *prevMenu)
{
    /* Release joysticks */
    for (int jsInd = 0; jsInd < GFCTRL_JOY_NUMBER; jsInd++)
	if (Joystick[jsInd]) {
	    delete Joystick[jsInd];
	    Joystick[jsInd] = 0;
	}

    /* Back to previous screen */
    GfuiScreenActivate(prevMenu);
}

/* Save settings in the players preferences and go back to previous screen */
static void
onSave(void *prevMenu)
{
    // Force current edit to loose focus (if one has it) and update associated variable.
    GfuiUnSelectCurrent();

    ControlPutSettings();

    onQuit(prevMenu);
}

static void
updateButtonText(void)
{
    int		cmdInd;
    const char	*str;

    /* No calibration / no dead zone needed for the moment (but let's check this ...) */
    MouseCalNeeded = 0;
    JoyCalNeeded   = 0;

    /* For each control: */
    for (cmdInd = 0; cmdInd < MaxCmd; cmdInd++) {

	/* Update associated editbox according to detected input device action */
	str = GfctrlGetNameByRef(Cmd[cmdInd].ref.type, Cmd[cmdInd].ref.index);
	if (str) {
	    GfuiButtonSetText (ScrHandle, Cmd[cmdInd].Id, str);
	} else {
	    GfuiButtonSetText (ScrHandle, Cmd[cmdInd].Id, "---");
	}

	/* According to detected action, update the "calibration needed" flags */
	if (Cmd[cmdInd].ref.type == GFCTRL_TYPE_MOUSE_AXIS) {
	    MouseCalNeeded = 1;
	} else if (Cmd[cmdInd].ref.type == GFCTRL_TYPE_JOY_AXIS) {
	    JoyCalNeeded = 1;
	}
    }

	/* According to detected action, update the "dead zone needed" flag */
    int deadZoneNeeded = 1;
	if ((Cmd[0].ref.type == GFCTRL_TYPE_KEYBOARD
			|| Cmd[0].ref.type == GFCTRL_TYPE_JOY_BUT
			|| Cmd[0].ref.type == GFCTRL_TYPE_MOUSE_BUT)
		&& (Cmd[1].ref.type == GFCTRL_TYPE_KEYBOARD
			|| Cmd[1].ref.type == GFCTRL_TYPE_JOY_BUT
			|| Cmd[1].ref.type == GFCTRL_TYPE_MOUSE_BUT)) {
	    deadZoneNeeded = 0;
	}

    /* Update Steer Sensibility editbox */
    sprintf(buf, "%6.4f", SteerSensVal);
    GfuiEditboxSetString(ScrHandle, SteerSensEditId, buf);

    /* Update Steer Dead Zone editbox */
    sprintf(buf, "%6.4f", DeadZoneVal);
    GfuiEditboxSetString(ScrHandle, DeadZoneEditId, buf);

    /* Update Steer Speed Sensitivity editbox */
    sprintf(buf, "%6.4f", SteerSpeedSensVal);
    GfuiEditboxSetString(ScrHandle, SteerSpeedSensEditId, buf);

    /* Show / hide mouse / joystick calibration button,
       according to the detected input device actions */
    GfuiVisibilitySet(ScrHandle, CalibrateButtonId, 
		      MouseCalNeeded|JoyCalNeeded ? GFUI_VISIBLE : GFUI_INVISIBLE);
	
    /* Show / hide dead zone label /editbox,
       according to the detected input device actions */
    GfuiVisibilitySet(ScrHandle, DeadZoneLabelId, 
		      deadZoneNeeded ? GFUI_VISIBLE : GFUI_INVISIBLE);
    GfuiVisibilitySet(ScrHandle, DeadZoneEditId, 
		      deadZoneNeeded ? GFUI_VISIBLE : GFUI_INVISIBLE);
}

static void
onFocusLost(void * /* dummy */)
{
    updateButtonText();
}

static int CurrentCmd;

static int InputWaited = 0;

static int
onKeyAction(int key, int /* modifier */, int state)
{
    if (!InputWaited || state == GFUI_KEY_UP) {
	return 0;
    }
    if (key == GFUIK_ESCAPE) {
	/* escape */
	Cmd[CurrentCmd].ref.index = -1;
	Cmd[CurrentCmd].ref.type = GFCTRL_TYPE_NOT_AFFECTED;
	GfParmSetStr(PrefHdle, CurrentSection, Cmd[CurrentCmd].name, "");
    } else {
	const char* name = GfctrlGetNameByRef(GFCTRL_TYPE_KEYBOARD, key);
	Cmd[CurrentCmd].ref.index = key;
	Cmd[CurrentCmd].ref.type = GFCTRL_TYPE_KEYBOARD;
	GfParmSetStr(PrefHdle, CurrentSection, Cmd[CurrentCmd].name, name);
    }

    GfelSetIdleCB(0);
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

/* Game event loop idle function : For collecting input devices actions */
static void
IdleWaitForInput(void)
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
	    AcceptMouseClicks = 0;
	    GfelSetIdleCB(0);
	    InputWaited = 0;
	    str = GfctrlGetNameByRef(GFCTRL_TYPE_MOUSE_BUT, i);
	    Cmd[CurrentCmd].ref.index = i;
	    Cmd[CurrentCmd].ref.type = GFCTRL_TYPE_MOUSE_BUT;
	    GfuiButtonSetText (ScrHandle, Cmd[CurrentCmd].Id, str);
	    GfelPostRedisplay();
	    updateButtonText();
	    return;
	}
    }

    /* Check for a mouse axis moved */
    for (i = 0; i < 4; i++) {
	if (MouseInfo.ax[i] > 20.0) {
	    GfelSetIdleCB(0);
	    InputWaited = 0;
	    str = GfctrlGetNameByRef(GFCTRL_TYPE_MOUSE_AXIS, i);
	    Cmd[CurrentCmd].ref.index = i;
	    Cmd[CurrentCmd].ref.type = GFCTRL_TYPE_MOUSE_AXIS;
	    GfuiButtonSetText (ScrHandle, Cmd[CurrentCmd].Id, str);
	    GfelPostRedisplay();
	    updateButtonText();
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
		    GfelSetIdleCB(0);
		    InputWaited = 0;
		    str = GfctrlGetNameByRef(GFCTRL_TYPE_JOY_BUT, i + 32 * index);
		    Cmd[CurrentCmd].ref.index = i + 32 * index;
		    Cmd[CurrentCmd].ref.type = GFCTRL_TYPE_JOY_BUT;
		    GfuiButtonSetText (ScrHandle, Cmd[CurrentCmd].Id, str);
		    GfelPostRedisplay();
		    JoyButtons[index] = b;
		    updateButtonText();
		    return;
		}
	    }
	    JoyButtons[index] = b;
	}
    }

    /* detect joystick movement */
    axis = getMovedAxis();
    if (axis != -1) {
	GfelSetIdleCB(0);
	InputWaited = 0;
	Cmd[CurrentCmd].ref.type = GFCTRL_TYPE_JOY_AXIS;
	Cmd[CurrentCmd].ref.index = axis;
	str = GfctrlGetNameByRef(GFCTRL_TYPE_JOY_AXIS, axis);
	GfuiButtonSetText (ScrHandle, Cmd[CurrentCmd].Id, str);
	GfelPostRedisplay();
	updateButtonText();
	return;
    }

    /* Let CPU take breath (and fans stay at low and quiet speed) */
    GfSleep(0.001);
}

/* Push button callback for each command button : activate input devices action collection loop */
static void
onPush(void *vi)
{
    int		index;    
    long	i = (long)vi;

    /* Do nothing if mouse button clicks are to be refused */
    if (!AcceptMouseClicks) {
	AcceptMouseClicks = 1;
	return;
    }

    /* Selected given command as the currently awaited one */
    CurrentCmd = i;

    /* Empty button text to tell the user we will soon be waiting for its input */
    GfuiButtonSetText (ScrHandle, Cmd[i].Id, "");

    /* Reset selected command action */
    Cmd[i].ref.index = -1;
    Cmd[i].ref.type = GFCTRL_TYPE_NOT_AFFECTED;

    /* State that a keyboard action is awaited */
    if (Cmd[CurrentCmd].keyboardPossible)
	InputWaited = 1;

    /* Read initial mouse status */
    GfctrlMouseInitCenter();
    memset(&MouseInfo, 0, sizeof(MouseInfo));
    GfctrlMouseGetCurrent(&MouseInfo);

    /* Read initial joysticks status */
    for (index = 0; index < GFCTRL_JOY_NUMBER; index++)
	if (Joystick[index])
	    Joystick[index]->read(&JoyButtons[index], &JoyAxis[index * GFCTRL_JOY_MAX_AXES]);
    memcpy(JoyAxisCenter, JoyAxis, sizeof(JoyAxisCenter));

    /* Now, wait for input device actions */
    GfelSetIdleCB(IdleWaitForInput);
}

static void
onActivate(void * /* dummy */)
{
    // Create and test joysticks ; only keep the up and running ones.
    for (int jsInd = 0; jsInd < GFCTRL_JOY_NUMBER; jsInd++) {
	if (!Joystick[jsInd])
	    Joystick[jsInd] = new jsJoystick(jsInd);
	if (Joystick[jsInd]->notWorking()) {
	    /* don't configure the joystick */
	    delete Joystick[jsInd];
	    Joystick[jsInd] = 0;
	} else {
	  GfOut("Detected joystick #%d type '%s' %d axes\n", 
		jsInd, Joystick[jsInd]->getName(), Joystick[jsInd]->getNumAxes());
	}
    }

    if (ReloadValues) {

        /* Load command settings from preference params for current player */
        ControlGetSettings();

	/* For each control : */
	for (int cmdInd = 0; cmdInd < MaxCmd; cmdInd++) {

	    /* Show / hide control editboxes according to selected gear changing mode code */
	    if (GearChangeMode & CmdDispInfo[cmdInd].gearChangeModeMask)
	    {
	        GfuiVisibilitySet(ScrHandle, Cmd[cmdInd].labelId, GFUI_VISIBLE);
	        GfuiVisibilitySet(ScrHandle, Cmd[cmdInd].Id, GFUI_VISIBLE);
	    }
	    else
	    {
	        GfuiVisibilitySet(ScrHandle, Cmd[cmdInd].labelId, GFUI_INVISIBLE);
	        GfuiVisibilitySet(ScrHandle, Cmd[cmdInd].Id, GFUI_INVISIBLE);
	    }
	}
    }
    
    updateButtonText();

    AcceptMouseClicks = 1;
}

static void
DevCalibrate(void * /* dummy */)
{
    void* firstCalMenu = 0;

    // No need to reload command settings from preference on return
    ReloadValues = 0;

    // Create calibration "wizard" (1 menu for each device to calibrate
    if (MouseCalNeeded) {
	if (JoyCalNeeded) {
	    void* nextCalMenu = JoyCalMenuInit(ScrHandle, Cmd, MaxCmd);
	    firstCalMenu = MouseCalMenuInit(nextCalMenu, Cmd, MaxCmd);
	} else {
	    firstCalMenu = MouseCalMenuInit(ScrHandle, Cmd, MaxCmd);
	}
    } else if (JoyCalNeeded) {
	firstCalMenu = JoyCalMenuInit(ScrHandle, Cmd, MaxCmd);
    }
	 
    // Activate first wizard screen
    if (firstCalMenu)
	GfuiScreenActivate(firstCalMenu);
}


/* */
void *
ControlMenuInit(void *prevMenu, void *prefHdle, unsigned index, tGearChangeMode gearChangeMode)
{
    int	i;

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

    /* Initialize joysticks array */
    for (int jsInd = 0; jsInd < GFCTRL_JOY_NUMBER; jsInd++)
	Joystick[jsInd] = 0;

    /* Create screen */
    ScrHandle = GfuiScreenCreateEx((float*)NULL, NULL, onActivate, NULL, (tfuiCallback)NULL, 1);

    void *param = LoadMenuXML("controlconfigmenu.xml");
    CreateStaticControls(param,ScrHandle);

    /* Default keyboard shortcuts */
    GfuiMenuDefaultKeysAdd(ScrHandle);

    /* For each control (in Cmd array), create the associated label and editbox */
    for (i = 0; i < MaxCmd; i++) 
    {
	Cmd[i].labelId = CreateLabelControl(ScrHandle,param,Cmd[i].name);
	std::string strCmdEdit(Cmd[i].name);
	strCmdEdit += " button";
	Cmd[i].Id = CreateButtonControlEx(ScrHandle,param,strCmdEdit.c_str(),(void*)i,onPush,NULL,(tfuiCallback)NULL,onFocusLost);
	}
    
    /* Steer Sensibility label and associated editbox */
    CreateLabelControl(ScrHandle,param,"Steer Sensitivity");
    SteerSensEditId = CreateEditControl(ScrHandle,param,"Steer Sensitivity Edit",NULL,NULL,onSteerSensChange);

    /* Steer Dead Zone label and associated editbox */
    DeadZoneLabelId = CreateLabelControl(ScrHandle,param,"Steer Dead Zone");
    DeadZoneEditId = CreateEditControl(ScrHandle,param,"Steer Dead Zone Edit",NULL,NULL,onDeadZoneChange);

    /* Steer Speed Sensibility label and associated editbox */
    CreateLabelControl(ScrHandle,param,"Steer Speed Sensitivity");
    SteerSpeedSensEditId = CreateEditControl(ScrHandle,param,"Steer Speed Sensitivity Edit",NULL,NULL,onSteerSpeedSensChange);

    /* Save button and associated keyboard shortcut */
    CreateButtonControl(ScrHandle,param,"save",PrevScrHandle,onSave);
    GfuiAddKey(ScrHandle, GFUIK_RETURN /* Return */, "Save", PrevScrHandle, onSave, NULL);

    /* Mouse calibration screen access button */
    CalibrateButtonId = CreateButtonControl(ScrHandle,param,"calibrate",NULL, DevCalibrate);

    /* Cancel button and associated keyboard shortcut */
    CreateButtonControl(ScrHandle,param,"cancel",PrevScrHandle,onQuit);
    GfuiAddKey(ScrHandle, GFUIK_ESCAPE, "Cancel", PrevScrHandle, onQuit, NULL);

    /* General callback for keyboard keys */
    GfuiKeyEventRegister(ScrHandle, onKeyAction);

    GfParmReleaseHandle(param);
    
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
	if (SteerSensVal <= 0.0)
	    SteerSensVal = 1.0e-6;

    /* Load also Dead zone (default from mouse prefs) */
    DeadZoneVal = GfParmGetNum(prefHdle, HM_SECT_MOUSEPREF, HM_ATT_STEER_DEAD, NULL, 0);
    DeadZoneVal = GfParmGetNum(prefHdle, CurrentSection, HM_ATT_STEER_DEAD, NULL, DeadZoneVal);
	if (DeadZoneVal < 0.0)
	    DeadZoneVal = 0.0;
	else if (DeadZoneVal > 1.0)
		DeadZoneVal = 1.0;
	
    /* Load also Steer speed sensibility (default from mouse prefs) */
    SteerSpeedSensVal = GfParmGetNum(prefHdle, HM_SECT_MOUSEPREF, HM_ATT_STEER_SPD, NULL, 0);
    SteerSpeedSensVal = GfParmGetNum(prefHdle, CurrentSection, HM_ATT_STEER_SPD, NULL, SteerSpeedSensVal);
	if (SteerSpeedSensVal < 0.0)
	    SteerSpeedSensVal = 0.0;
}

/* From global vars (Cmd, SteerSensVal, DeadZoneVal) to parms (prefHdle) */
void ControlPutSettings(void *prefHdle, unsigned index, tGearChangeMode gearChangeMode)
{
    int iCmd;
    const char* str;
    const char* pszReverseCmd;
    const char* pszNeutralCmd;

    /* If handle on preferences not given, get current */
    if (!prefHdle)
        prefHdle = PrefHdle;

    /* Change current player section in the players preferences if specified */
    if (index > 0)
        sprintf(CurrentSection, "%s/%s/%d", HM_SECT_PREF, HM_LIST_DRV, index);

    /* Select current player gear change mode if not specified */
    if (gearChangeMode == GEAR_MODE_NONE)
        gearChangeMode = GearChangeMode;

    /* Allow neutral gear in sequential mode if nor reverse nor neutral gear command defined */
    pszReverseCmd = GfctrlGetNameByRef(Cmd[ICmdReverseGear].ref.type, Cmd[ICmdReverseGear].ref.index);
    pszNeutralCmd = GfctrlGetNameByRef(Cmd[ICmdNeutralGear].ref.type, Cmd[ICmdNeutralGear].ref.index);
    if (gearChangeMode == GEAR_MODE_SEQ
	&& (!pszReverseCmd || !strcmp(pszReverseCmd, "-")
	    || !pszNeutralCmd || !strcmp(pszNeutralCmd, "-")))
	GfParmSetStr(prefHdle, CurrentSection, HM_ATT_SEQSHFT_ALLOW_NEUTRAL, HM_VAL_YES);
    else
	GfParmSetStr(prefHdle, CurrentSection, HM_ATT_SEQSHFT_ALLOW_NEUTRAL, HM_VAL_NO);

    /* Release gear lever goes neutral in grid mode if no neutral gear command defined */
    if (gearChangeMode == GEAR_MODE_GRID
	&& (!pszNeutralCmd || !strcmp(pszNeutralCmd, "-")))
	GfParmSetStr(prefHdle, CurrentSection, HM_ATT_REL_BUT_NEUTRAL, HM_VAL_YES);
    else
	GfParmSetStr(prefHdle, CurrentSection, HM_ATT_REL_BUT_NEUTRAL, HM_VAL_NO);

    /* Steer sensitivity and dead zone */
    GfParmSetNum(prefHdle, CurrentSection, HM_ATT_STEER_SENS, NULL, SteerSensVal);
    GfParmSetNum(prefHdle, CurrentSection, HM_ATT_STEER_DEAD, NULL, DeadZoneVal);
    GfParmSetNum(prefHdle, CurrentSection, HM_ATT_STEER_SPD,  NULL, SteerSpeedSensVal);

    /* Name, min, max and power, for each possible command */
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
