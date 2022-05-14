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
#include <sound.h>
#include <musicplayer.h>
#include <guimenusfx.h>

#include "soundconfig.h"

#include "legacymenu.h"

// list of options.
static const char *soundOptionList[] = {SND_ATT_SOUND_STATE_OPENAL,
								  SND_ATT_SOUND_STATE_PLIB,
								  SND_ATT_SOUND_STATE_DISABLED};
static const int nbOptions = sizeof(soundOptionList) / sizeof(soundOptionList[0]);
static int curOption = 0;

// gui label id.
static int SoundOptionId;

// volume
static float VolumeValue = 100.0f;
static int VolumeValueId;

// list of music states.
static const char *musicStateList[] = {SND_VAL_MUSIC_STATE_ENABLED,
								  SND_VAL_MUSIC_STATE_DISABLED};
static const int nbMusicStates = sizeof(musicStateList) / sizeof(musicStateList[0]);
static int curMusicState = 0;
static int OriginalMusicState = 0;

// gui label id.
static int MusicStateId;

// volume
static float MusicVolumeValue = 100.0f;
static float OriginalMusicVolumeValue = 100.0f;
static int MusicVolumeValueId;


// list of MenuSfx states.
static const char *menusfxStateList[] = {SND_VAL_MENUSFX_STATE_ENABLED,
								  SND_VAL_MENUSFX_STATE_DISABLED};
static const int nbMenuSfxStates = sizeof(menusfxStateList) / sizeof(menusfxStateList[0]);
static int curMenuSfxState = 0;
static int OriginalMenuSfxState = 0;

// gui label id.
static int MenuSfxStateId;

// volume
static float MenuSfxVolumeValue = 100.0f;
static float OriginalMenuSfxVolumeValue = 100.0f;
static int MenuSfxVolumeValueId;

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
	void *paramHandle = GfParmReadFileLocal(SND_PARAM_FILE, GFPARM_RMODE_REREAD | GFPARM_RMODE_CREAT);
	optionName = GfParmGetStr(paramHandle, SND_SCT_SOUND, SND_ATT_SOUND_STATE, soundOptionList[0]);

	for (i = 0; i < nbOptions; i++) {
		if (strcmp(optionName, soundOptionList[i]) == 0) {
			curOption = i;
			break;
		}
	}

	GfuiLabelSetText(scrHandle, SoundOptionId, soundOptionList[curOption]);

	// Sound volume.
	VolumeValue = GfParmGetNum(paramHandle, SND_SCT_SOUND, SND_ATT_SOUND_VOLUME, "%", 100.0f);
	if (VolumeValue>100.0f) {
		VolumeValue = 100.0f;
	} 
	else if (VolumeValue < 0.0f) {
		VolumeValue = 0.0f;
	}

	sprintf(buf, "%g", VolumeValue);
	GfuiEditboxSetString(scrHandle, VolumeValueId, buf);

	optionName = GfParmGetStr(paramHandle, SND_SCT_MUSIC, SND_ATT_MUSIC_STATE, musicStateList[0]);

	for (i = 0; i < nbMusicStates; i++) {
		if (strcmp(optionName, musicStateList[i]) == 0) {
			curMusicState = i;
			break;
		}
	}
	OriginalMusicState = curMusicState;

	GfuiLabelSetText(scrHandle, MusicStateId, musicStateList[curMusicState]);

	// Music volume.
	MusicVolumeValue = GfParmGetNum(paramHandle, SND_SCT_MUSIC, SND_ATT_MUSIC_VOLUME, "%", 100.0f);
	if (MusicVolumeValue>100.0f) {
		MusicVolumeValue = 100.0f;
	} 
	else if (MusicVolumeValue < 0.0f) {
		MusicVolumeValue = 0.0f;
	}
	OriginalMusicVolumeValue = MusicVolumeValue;

	sprintf(buf, "%g", MusicVolumeValue);
	GfuiEditboxSetString(scrHandle, MusicVolumeValueId, buf);

	optionName = GfParmGetStr(paramHandle, SND_SCT_MENUSFX, SND_ATT_MENUSFX_STATE, menusfxStateList[0]);

	for (i = 0; i < nbMenuSfxStates; i++) {
		if (strcmp(optionName, menusfxStateList[i]) == 0) {
			curMenuSfxState = i;
			break;
		}
	}
	OriginalMenuSfxState = curMenuSfxState;

	GfuiLabelSetText(scrHandle, MenuSfxStateId, menusfxStateList[curMenuSfxState]);

	// Music volume.
	MenuSfxVolumeValue = GfParmGetNum(paramHandle, SND_SCT_MENUSFX, SND_ATT_MENUSFX_VOLUME, "%", 100.0f);
	if (MenuSfxVolumeValue>100.0f) {
		MenuSfxVolumeValue = 100.0f;
	} 
	else if (MenuSfxVolumeValue < 0.0f) {
		MenuSfxVolumeValue = 0.0f;
	}
	OriginalMenuSfxVolumeValue = MenuSfxVolumeValue;

	sprintf(buf, "%g", MenuSfxVolumeValue);
	GfuiEditboxSetString(scrHandle, MenuSfxVolumeValueId, buf);

	GfParmReleaseHandle(paramHandle);
}

static void saveMenuSfxSettings()
{
	void *paramHandle = GfParmReadFileLocal(SND_PARAM_FILE, GFPARM_RMODE_REREAD | GFPARM_RMODE_CREAT);
	GfParmSetStr(paramHandle, SND_SCT_MENUSFX, SND_ATT_MENUSFX_STATE, menusfxStateList[curMenuSfxState]);
	GfParmSetNum(paramHandle, SND_SCT_MENUSFX, SND_ATT_MENUSFX_VOLUME, "%", MenuSfxVolumeValue);

	GfParmWriteFile(NULL, paramHandle, "sound");
	GfParmReleaseHandle(paramHandle);
}

static void saveMusicSettings()
{
	void *paramHandle = GfParmReadFileLocal(SND_PARAM_FILE, GFPARM_RMODE_REREAD | GFPARM_RMODE_CREAT);
	GfParmSetStr(paramHandle, SND_SCT_MUSIC, SND_ATT_MUSIC_STATE, musicStateList[curMusicState]);
	GfParmSetNum(paramHandle, SND_SCT_MUSIC, SND_ATT_MUSIC_VOLUME, "%", MusicVolumeValue);

	GfParmWriteFile(NULL, paramHandle, "sound");
	GfParmReleaseHandle(paramHandle);
}

// Save the choosen values in the corresponding parameter file.
static void saveSoundOption()
{
	// Force current edit to loose focus (if one has it) and update associated variable.
	GfuiUnSelectCurrent();

	void *paramHandle = GfParmReadFileLocal(SND_PARAM_FILE, GFPARM_RMODE_REREAD | GFPARM_RMODE_CREAT);
	GfParmSetStr(paramHandle, SND_SCT_SOUND, SND_ATT_SOUND_STATE, soundOptionList[curOption]);
	GfParmSetNum(paramHandle, SND_SCT_SOUND, SND_ATT_SOUND_VOLUME, "%", VolumeValue);
	GfParmSetStr(paramHandle, SND_SCT_MUSIC, SND_ATT_MUSIC_STATE, musicStateList[curMusicState]);
	GfParmSetNum(paramHandle, SND_SCT_MUSIC, SND_ATT_MUSIC_VOLUME, "%", MusicVolumeValue);
	GfParmSetStr(paramHandle, SND_SCT_MENUSFX, SND_ATT_MENUSFX_STATE, menusfxStateList[curMenuSfxState]);
	GfParmSetNum(paramHandle, SND_SCT_MENUSFX, SND_ATT_MENUSFX_VOLUME, "%", MenuSfxVolumeValue);

	GfParmWriteFile(NULL, paramHandle, "sound");
	GfParmReleaseHandle(paramHandle);
}



static void onAccept(void *)
{
	saveSoundOption();

	saveMusicSettings();

	saveMenuSfxSettings();

	enableMusic(1 - curMusicState);
	setMusicVolume(MusicVolumeValue);

	enableMenuSfx(1 - curMenuSfxState);
	setMenuSfxVolume(MenuSfxVolumeValue);

	// Return to previous screen.
	GfuiScreenActivate(prevHandle);
}

// Reset any changes 
static void onCancel(void *)
{
	// Return to previous screen.
	GfuiScreenActivate(prevHandle);
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

// Toggle music state enabled/disabled.
static void changeMusicState(void *vp)
{
	curMusicState = (curMusicState + (int)(long)vp + nbMusicStates) % nbMusicStates;

	GfuiLabelSetText(scrHandle, MusicStateId, musicStateList[curMusicState]);

}

// Music Volume
static void changeMusicVolume(void * )
{
    char* val = GfuiEditboxGetString(scrHandle, MusicVolumeValueId);
    sscanf(val, "%g", &MusicVolumeValue);
    if (MusicVolumeValue > 100.0f)
		MusicVolumeValue = 100.0f;
    else if (MusicVolumeValue < 0.0f)
		MusicVolumeValue = 0.0f;
	
    char buf[32];
    sprintf(buf, "%g", MusicVolumeValue);
    GfuiEditboxSetString(scrHandle, MusicVolumeValueId, buf);
}

// Toggle Menu SFX state enabled/disabled.
static void changeMenuSfxState(void *vp)
{
	curMenuSfxState = (curMenuSfxState + (int)(long)vp + nbMenuSfxStates) % nbMenuSfxStates;

	GfuiLabelSetText(scrHandle, MenuSfxStateId, menusfxStateList[curMenuSfxState]);

}

//  Menu SFX Volume
static void changeMenuSfxVolume(void * )
{
	char* val = GfuiEditboxGetString(scrHandle, MenuSfxVolumeValueId);
	sscanf(val, "%g", &MenuSfxVolumeValue);
	if (MenuSfxVolumeValue > 100.0f)
		MenuSfxVolumeValue = 100.0f;
	else if (MenuSfxVolumeValue < 0.0f)
		MenuSfxVolumeValue = 0.0f;
	
	char buf[32];
	sprintf(buf, "%g", MenuSfxVolumeValue);
	GfuiEditboxSetString(scrHandle, MenuSfxVolumeValueId, buf);
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

	void *param = GfuiMenuLoad("soundconfigmenu.xml");
	GfuiMenuCreateStaticControls(scrHandle, param);

	GfuiMenuCreateButtonControl(scrHandle,param,"soundleftarrow",(void*)-1,changeSoundState);
	GfuiMenuCreateButtonControl(scrHandle,param,"soundrightarrow",(void*)1,changeSoundState);

	SoundOptionId = GfuiMenuCreateLabelControl(scrHandle,param,"soundlabel");
	GfuiMenuCreateButtonControl(scrHandle,param,"ApplyButton",NULL,onAccept);
	GfuiMenuCreateButtonControl(scrHandle,param,"CancelButton",NULL,onCancel);

	VolumeValueId = GfuiMenuCreateEditControl(scrHandle,param,"volumeedit",NULL,NULL,changeVolume);

	GfuiMenuCreateButtonControl(scrHandle,param,"musicleftarrow",(void*)-1,changeMusicState);
	GfuiMenuCreateButtonControl(scrHandle,param,"musicrightarrow",(void*)1,changeMusicState);

	MusicStateId = GfuiMenuCreateLabelControl(scrHandle,param,"musiclabel");

	MusicVolumeValueId = GfuiMenuCreateEditControl(scrHandle,param,"musicvolumeedit",NULL,NULL,changeMusicVolume);

	GfuiMenuCreateButtonControl(scrHandle,param,"menusfxleftarrow",(void*)-1,changeMenuSfxState);
	GfuiMenuCreateButtonControl(scrHandle,param,"menusfxrightarrow",(void*)1,changeMenuSfxState);

	MenuSfxStateId = GfuiMenuCreateLabelControl(scrHandle,param,"menusfxlabel");

	MenuSfxVolumeValueId = GfuiMenuCreateEditControl(scrHandle,param,"menusfxvolumeedit",NULL,NULL,changeMenuSfxVolume);

	GfParmReleaseHandle(param);

	GfuiMenuDefaultKeysAdd(scrHandle);
	GfuiAddKey(scrHandle, GFUIK_RETURN, "Apply", NULL, onAccept, NULL);
	GfuiAddKey(scrHandle, GFUIK_ESCAPE, "Cancel", NULL, onCancel, NULL);
	GfuiAddKey(scrHandle, GFUIK_LEFT, "Previous Option in list", (void*)-1, changeSoundState, NULL);
	GfuiAddKey(scrHandle, GFUIK_RIGHT, "Next Option in list", (void*)1, changeSoundState, NULL);

	return scrHandle;
}
