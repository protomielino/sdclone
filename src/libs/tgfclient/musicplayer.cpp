/***************************************************************************

    file                 : musicplayer.cpp
    created              : Fri Dec 23 17:35:18 CET 2011
    copyright            : (C) 2011 Bernhard Wymann
    email                : berniw@bluewin.ch
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

#include "musicplayer.h"

//#include <GL/glut.h>
#include <string.h>
#include <tgf.h>
#include "tgfclient.h"
#include <portability.h>

#if MENU_MUSIC
#include "oggsoundstream.h"
#include "openalmusicplayer.h"
 
 
static void playMenuMusic(int /* value */);


 static bool isEnabled()
 {
	// TODO - fix this (needs UI)
	return true;
	
#if 0
	const int BUFSIZE = 1024;
	char buf[BUFSIZE];
	snprintf(buf, BUFSIZE, "%s%s", GetLocalDir(), MM_SOUND_PARM_CFG);
	bool enabled = false;
	
	void *handle = GfParmReadFile(buf, GFPARM_RMODE_STD | GFPARM_RMODE_CREAT);
	const char* s = GfParmGetStr(handle, MM_SCT_SOUND, MM_ATT_SOUND_ENABLE, MM_VAL_SOUND_DISABLED);
	
	if (strcmp(s, MM_VAL_SOUND_ENABLED) == 0) {
		enabled = true;
	}
	
	GfParmReleaseHandle(handle);

	return enabled;
#endif
}


// Path relative to CWD, e.g "data/music/main.ogg"
static SoundStream* getMenuSoundStream(char* oggFilePath)
{
	static OggSoundStream stream(oggFilePath);
	return &stream;
}


static OpenALMusicPlayer* getMusicPlayer()
{
	const int BUFSIZE = 1024;
	char oggFilePath[BUFSIZE];
	
	// TODO - get from config??
	strncpy(oggFilePath, "data/music/main.ogg", BUFSIZE);

	static OpenALMusicPlayer player(getMenuSoundStream(oggFilePath));
	return &player;
}

// TODO rethink...
static Uint32 sdlTimerFunc(Uint32 interval, void* /* pEvLoopPriv */)
{
	playMenuMusic(0);
	return 1;
	//return 0;
}

// TODO clean this up
SDL_TimerID timerId = 0;
static void playMenuMusic(int /* value */)
{
	const int nextcallinms = 200;
	
	OpenALMusicPlayer* player = getMusicPlayer();
	if (player->playAndManageBuffer()) {
		if(timerId == 0){
			timerId = SDL_AddTimer(nextcallinms, sdlTimerFunc, (void*)NULL);
		}
		//glutTimerFunc(nextcallinms, playMenuMusic, 0);
	}
}
#endif
void startMenuMusic()
{
#if MENU_MUSIC
	if (isEnabled()) {
		OpenALMusicPlayer* player = getMusicPlayer();
		player->start();
		playMenuMusic(0);
	}
#endif
}


void stopMenuMusic()
{
#if MENU_MUSIC
	if(timerId != 0){
		SDL_RemoveTimer(timerId);
		timerId = 0;
	}
	OpenALMusicPlayer* player = getMusicPlayer();
	player->stop();
	player->rewind();
#endif
}

void pauseMenuMusic()
{
#if MENU_MUSIC
		if(timerId != 0){
		SDL_RemoveTimer(timerId);
		timerId = 0;
		}
		OpenALMusicPlayer* player = getMusicPlayer();
		player->pause();
#endif
}

void resumeMenuMusic(int sourceId)
{
#if MENU_MUSIC
	if (isEnabled()) {
	getMusicPlayer()->resume(sourceId);
	playMenuMusic(0);
	}
#endif
}
