/***************************************************************************

    file        : soundconfig.cpp
    created     : Thu Dec 12 15:12:07 CET 2004
    copyright   : (C) 2004 Eric Espi�, Bernhard Wymann, baseed on simuconfig.cpp
    email       : berniw@bluewin.ch
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
    		
    @version	$Id$
*/

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <tgfclient.h>
#include <graphic.h>

#include "soundconfig.h"
#include "gui.h"


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
	sprintf(buf, "%s%s", GfLocalDir(), GR_SOUND_PARM_CFG);
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
	sprintf(buf, "%s%s", GfLocalDir(), GR_SOUND_PARM_CFG);
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
	curOption = (curOption + (int)(long)vp + nbOptions) % nbOptions;

    GfuiLabelSetText(scrHandle, SoundOptionId, soundOptionList[curOption]);
}

// Volume
static void changeVolume(void * )
{
    char* val = GfuiEditboxGetString(scrHandle, VolumeValueId);
    sscanf(val, "%g", &VolumeValue);
    if (VolumeValue > 100.0f)
		VolumeValue = 100.0f;
    else if (VolumeValue < 0.0f)
		VolumeValue = 0.0f;
	
    char buf[32];
    sprintf(buf, "%g", VolumeValue);
    GfuiEditboxSetString(scrHandle, VolumeValueId, buf);
}

static void onActivate(void * /* dummy */)
{
	readSoundCfg();
}


// Sound menu
void* SoundMenuInit(void *prevMenu)
{
	// Has screen already been created?
	if (scrHandle) {
		return scrHandle;
	}

	prevHandle = prevMenu;

	scrHandle = GfuiScreenCreate((float*)NULL, NULL, onActivate, NULL, (tfuiCallback)NULL, 1);

	void *param = GfuiMenuLoad("soundmenu.xml");
	GfuiMenuCreateStaticControls(scrHandle, param);

	GfuiMenuCreateButtonControl(scrHandle,param,"soundleftarrow",(void*)-1,changeSoundState);
	GfuiMenuCreateButtonControl(scrHandle,param,"soundrightarrow",(void*)1,changeSoundState);

	SoundOptionId = GfuiMenuCreateLabelControl(scrHandle,param,"soundlabel");
	GfuiMenuCreateButtonControl(scrHandle,param,"accept",NULL,saveSoundOption);
	GfuiMenuCreateButtonControl(scrHandle,param,"cancel",prevMenu,GfuiScreenActivate);

	VolumeValueId = GfuiMenuCreateEditControl(scrHandle,param,"volumeedit",NULL,NULL,changeVolume);

	GfParmReleaseHandle(param);
    
	GfuiAddKey(scrHandle, GFUIK_RETURN, "Save", NULL, saveSoundOption, NULL);
	GfuiAddKey(scrHandle, GFUIK_ESCAPE, "Cancel Selection", prevMenu, GfuiScreenActivate, NULL);
	GfuiAddKey(scrHandle, GFUIK_F1, "Help", scrHandle, GfuiHelpScreen, NULL);
	GfuiAddKey(scrHandle, GFUIK_F12, "Screen-Shot", NULL, GfuiScreenShot, NULL);
	GfuiAddKey(scrHandle, GFUIK_LEFT, "Previous Option in list", (void*)-1, changeSoundState, NULL);
	GfuiAddKey(scrHandle, GFUIK_RIGHT, "Next Option in list", (void*)1, changeSoundState, NULL);

	return scrHandle;
}
