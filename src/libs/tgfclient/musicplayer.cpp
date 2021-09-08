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

#include <string.h>
#include <string>
#include <map>
#include <tgf.h>
#include "tgfclient.h"
#include <portability.h>
#include <sound.h>
#include <SDL_mixer.h>

#define MAX_MUSIC_PATH 1024
#define NOMUSIC "None"

#include "sdl2musicplayer.h"
 
static const char *musicDisabledStr = SND_VAL_MUSIC_STATE_DISABLED;

static bool enabled = true;
static char currentMusicfile[MAX_MUSIC_PATH] = {0};
static char defaultMusic[MAX_MUSIC_PATH] = {0}; //"data/music/main.ogg";
static float maxMusicVolume = 1.0;
static SDL_mutex *mapMutex = NULL;

std::map<std::string,SDL2MusicPlayer*> mapSDL2Players;

static void readConfig();

 static bool isEnabled()
{
	return enabled;
}

static SDL2MusicPlayer* getMusicPlayer(char* oggFilePath)
{
	SDL2MusicPlayer* player = NULL;
	
	SDL_LockMutex(mapMutex);
	const std::map<std::string, SDL2MusicPlayer*>::const_iterator itPlayers = mapSDL2Players.find(oggFilePath);

	if (itPlayers == mapSDL2Players.end()) {
		player = new SDL2MusicPlayer(oggFilePath);
		mapSDL2Players[oggFilePath] = player;
		player->setvolume(maxMusicVolume);
	} else {
		player = mapSDL2Players[oggFilePath];
	}
	SDL_UnlockMutex(mapMutex);
	return player;
}

void initMusic()
{
	readConfig();
	if (isEnabled()) {
		GfLogInfo("(Re-)Initializing music player \n");
		mapMutex = SDL_CreateMutex();
		if( Mix_OpenAudio( 44100, MIX_DEFAULT_FORMAT, 2, 2048 ) >= 0 ){
			(void)getMusicPlayer(defaultMusic);
			strcpy(currentMusicfile,defaultMusic);
			SDL2MusicPlayer* player = getMusicPlayer(currentMusicfile);
			if(player){
				player->resume();
			}
		}
	}
	else {
		GfLogInfo("Music player is disabled \n");
	}
}


void shutdownMusic()
{
	SDL_LockMutex(mapMutex);
	std::map<std::string, SDL2MusicPlayer*>::const_iterator itPlayers = mapSDL2Players.begin();
	while(itPlayers != mapSDL2Players.end()) {
		SDL2MusicPlayer* player = itPlayers->second;
		player->stop();
		player->rewind();
		++itPlayers;
	}
	itPlayers = mapSDL2Players.begin();
	while(itPlayers != mapSDL2Players.end()) {
		SDL2MusicPlayer* player = itPlayers->second;
		delete player;
		++itPlayers;
	}
	mapSDL2Players.clear();
	SDL_UnlockMutex(mapMutex);
	SDL_DestroyMutex(mapMutex);
	mapMutex = NULL;
	Mix_Quit();
}


void playMusic(char* filename)
{
	if (isEnabled()) {
		SDL2MusicPlayer* player = NULL;
		if(filename != NULL)
		{
			if(0 == strcmp(NOMUSIC,filename))
			{
				player = getMusicPlayer(currentMusicfile);
				player->stop();
				strcpy(currentMusicfile,filename);
				GfLogInfo("Music changing to: %s \n", filename);
				return;
			}
			if(0 != strcmp(currentMusicfile,filename))
			{
				if(0 != strcmp(NOMUSIC,currentMusicfile)){
					player = getMusicPlayer(currentMusicfile);
					player->stop();
				}
				strcpy(currentMusicfile,filename);
				GfLogInfo("Music changing to: %s \n", filename);
				player = getMusicPlayer(filename);
				player->resume();
			}
		} 
		else 
		{
			if(0 != strcmp(currentMusicfile,defaultMusic))
			{
				if(0 != strcmp(NOMUSIC,currentMusicfile))
				{
					player = getMusicPlayer(currentMusicfile);
					player->stop();
				}
				strcpy(currentMusicfile,defaultMusic);
				GfLogInfo("Music changing to: %s \n", defaultMusic);
				player = getMusicPlayer(defaultMusic);
				player->resume();
			}
			else
			{
				player = getMusicPlayer(defaultMusic);
				player->resume();
			}
		}
	}
}

void setDefaultMusic(const char* filename)
{
	if(0 != filename){
		if(strlen(filename) < MAX_MUSIC_PATH){
			if(0 != strcmp(defaultMusic,filename)){
				strcpy(defaultMusic,filename);
				GfLogInfo("Default Music changing to: %s \n", filename);
			}
		} else {
			GfLogError("Default Music File Path too long. [  %s  ]\n", filename);
		}
	} else {
		defaultMusic[0] = 0;
		GfLogInfo("Default Music changing to: %s \n", "NULL");
	}
}

static void readConfig()
{
	char fnbuf[1024];
	snprintf(fnbuf, sizeof(fnbuf), "%s%s", GfLocalDir(), SND_PARAM_FILE);

	GfLogInfo("Reading music player config\n");
	void *paramHandle = GfParmReadFile(fnbuf, GFPARM_RMODE_REREAD | GFPARM_RMODE_CREAT);
	const char *musicenabled = GfParmGetStr(paramHandle, SND_SCT_MUSIC, SND_ATT_MUSIC_STATE, musicDisabledStr);

	float music_volume = GfParmGetNum(paramHandle, SND_SCT_MUSIC, SND_ATT_MUSIC_VOLUME, "%", 100.0f);
	if (music_volume>100.0f) {
		music_volume = 100.0f;
	} 
	else if (music_volume < 0.0f) {
		music_volume = 0.0f;
	}
	maxMusicVolume = music_volume/100.0f;

	if (0 == strcmp(musicenabled, SND_VAL_MUSIC_STATE_ENABLED)) {
		enabled = true;
	} else {
		enabled = false;
	}
	const char* defmusic = GfParmGetStr(paramHandle, SND_SCT_MUSIC, SND_ATT_MUSIC_DEFAULT_MUSIC, "data/music/main.ogg");
	setDefaultMusic(defmusic);

	//TODO: Remove this hack after plib is deprecated
	// Using plib for the sound effects sometimes crashes when OpenAL already has the sound device
	const char* isplib = GfParmGetStr(paramHandle, SND_SCT_SOUND, SND_ATT_SOUND_STATE, "");
	if (!strcmp(isplib, "plib")) {
		//enabled = false;
		GfLogInfo("Music player disabled when using PLIB for sound effects\n");
	}
	//TODO end of section to Remove

	GfParmReleaseHandle(paramHandle);
	paramHandle = NULL;
}

void setMusicVolume(float vol)
{
	if (vol < 0)
		vol = 0.0f;
	else if (vol > 1.0f)
		vol = 1.0f;
	
	maxMusicVolume = vol;

	GfLogInfo("Music volume set to %.2f\n", maxMusicVolume);
}
