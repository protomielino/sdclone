/***************************************************************************

    file        : soundconfig.cpp
    created     : Thu Dec 12 15:12:07 CET 2004
    copyright   : (C) 2004 Eric Espié, Bernhard Wymann, baseed on simuconfig.cpp
    email       : berniw@bluewin.ch
    version     : $Id: soundconfig.cpp,v 1.6 2005/08/05 09:03:39 berniw Exp $

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
    		
    @version	$Id: soundconfig.cpp,v 1.6 2005/08/05 09:03:39 berniw Exp $
*/

#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <tgfclient.h>
#include <raceinit.h>
#include <graphic.h>

#include "soundconfig.h"

static float LabelColor[] = {1.0, 0.0, 1.0, 1.0};

// list of options.
static const char *soundOptionList[] = {GR_ATT_SOUND_STATE_OPENAL,
								  GR_ATT_SOUND_STATE_PLIB,
								  GR_ATT_SOUND_STATE_DISABLED};
static const int nbOptions = sizeof(soundOptionList) / sizeof(soundOptionList[0]);
static int curOption = 0;

// gui label id.
static int SoundOptionId;

// volume
static float VolumeValue = 100.0f;
static int VolumeValueId;

// gui screen handles.
static void *scrHandle = NULL;
static void *prevHandle = NULL;


// Read sound configuration.
static void readSoundCfg(void)
{
	const char *optionName;
	int	i;
	char buf[1024];

	// Sound interface.
	sprintf(buf, "%s%s", GetLocalDir(), GR_SOUND_PARM_CFG);
	void *paramHandle = GfParmReadFile(buf, GFPARM_RMODE_REREAD | GFPARM_RMODE_CREAT);
	optionName = GfParmGetStr(paramHandle, GR_SCT_SOUND, GR_ATT_SOUND_STATE, soundOptionList[0]);

	for (i = 0; i < nbOptions; i++) {
		if (strcmp(optionName, soundOptionList[i]) == 0) {
			curOption = i;
			break;
		}
	}

	GfuiLabelSetText(scrHandle, SoundOptionId, soundOptionList[curOption]);

	// Sound volume.
	VolumeValue = GfParmGetNum(paramHandle, GR_SCT_SOUND, GR_ATT_SOUND_VOLUME, "%", 100.0f);
	if (VolumeValue>100.0f) {
		VolumeValue = 100.0f;
	} 
	else if (VolumeValue < 0.0f) {
		VolumeValue = 0.0f;
	}

	sprintf(buf, "%g", VolumeValue);
	GfuiEditboxSetString(scrHandle, VolumeValueId, buf);

	GfParmReleaseHandle(paramHandle);
}


// Save the choosen values in the corresponding parameter file.
static void saveSoundOption(void *)
{
	// Force current edit to loose focus (if one has it) and update associated variable.
	GfuiUnSelectCurrent();

	char buf[1024];
	sprintf(buf, "%s%s", GetLocalDir(), GR_SOUND_PARM_CFG);
	void *paramHandle = GfParmReadFile(buf, GFPARM_RMODE_REREAD | GFPARM_RMODE_CREAT);
	GfParmSetStr(paramHandle, GR_SCT_SOUND, GR_ATT_SOUND_STATE, soundOptionList[curOption]);
	GfParmSetNum(paramHandle, GR_SCT_SOUND, GR_ATT_SOUND_VOLUME, "%", VolumeValue);
	GfParmWriteFile(NULL, paramHandle, "sound");
	GfParmReleaseHandle(paramHandle);

	// Return to previous screen.
	GfuiScreenActivate(prevHandle);
	return;
}


// Toggle sound state openal/plib/disabled.
static void changeSoundState(void *vp)
{
	if (vp == 0) {
		curOption--;
		if (curOption < 0) {
	    	curOption = nbOptions - 1;
		}
	} else {
		curOption++;
		if (curOption == nbOptions) {
	    curOption = 0;
	}
    }
    GfuiLabelSetText(scrHandle, SoundOptionId, soundOptionList[curOption]);
}

// Volume
static void changeVolume(void * )
{
    char	*val;
    char buf[256];
    val = GfuiEditboxGetString(scrHandle, VolumeValueId);
    sscanf(val, "%g", &VolumeValue);
    if (VolumeValue > 100.0f) {
	VolumeValue = 100.0f;
    } 
    else if (VolumeValue < 0.0f) {
	VolumeValue = 0.0f;
    }
    sprintf(buf, "%g", VolumeValue);
    GfuiEditboxSetString(scrHandle, VolumeValueId, buf);
}

static void onActivate(void * /* dummy */)
{
	readSoundCfg();
}


// Sound menu
void * SoundMenuInit(void *prevMenu)
{
	// Has screen already been created?
	if (scrHandle) {
		return scrHandle;
	}

	prevHandle = prevMenu;

	scrHandle = GfuiScreenCreateEx((float*)NULL, NULL, onActivate, NULL, (tfuiCallback)NULL, 1);

	void *param = LoadMenuXML("soundmenu.xml");
	CreateStaticControls(param,scrHandle);

	CreateButtonControl(scrHandle,param,"soundleftarrow",(void*)-1,changeSoundState);
	CreateButtonControl(scrHandle,param,"soundrightarrow",(void*)1,changeSoundState);

	SoundOptionId = CreateLabelControl(scrHandle,param,"soundlabel");
	CreateButtonControl(scrHandle,param,"accept",NULL,saveSoundOption);
	CreateButtonControl(scrHandle,param,"cancel",prevMenu,GfuiScreenActivate);

	VolumeValueId = CreateEditControl(scrHandle,param,"volumeedit",NULL,NULL,changeVolume);

	GfParmReleaseHandle(param);
    
	GfuiAddKey(scrHandle, 13, "Save", NULL, saveSoundOption, NULL);
	GfuiAddKey(scrHandle, 27, "Cancel Selection", prevMenu, GfuiScreenActivate, NULL);
	GfuiAddSKey(scrHandle, GLUT_KEY_F1, "Help", scrHandle, GfuiHelpScreen, NULL);
	GfuiAddSKey(scrHandle, GLUT_KEY_F12, "Screen-Shot", NULL, GfuiScreenShot, NULL);
	GfuiAddSKey(scrHandle, GLUT_KEY_LEFT, "Previous Option in list", (void*)0, changeSoundState, NULL);
	GfuiAddSKey(scrHandle, GLUT_KEY_RIGHT, "Next Option in list", (void*)1, changeSoundState, NULL);

	return scrHandle;
}
