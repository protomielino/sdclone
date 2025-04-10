/***************************************************************************
    file                 : PlibSound.h
    created              : Tue Jul 18 19:57:35 CEST 2011
    copyright            : (C) 2005 Christos Dimitrakakis
    email                : dimitrak@idiap.ch

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef PLIB_SOUND_H
#define PLIB_SOUND_H

#include <plib/sl.h>

#include "Sound.h"


class PlibSound : public Sound {
protected:
	slSample* sample = nullptr; ///< sample data
	slEnvelope* volume_env = nullptr; ///< volume envelope
	slEnvelope* pitch_env = nullptr; ///< pitch envelope
	slEnvelope* lowpass_env = nullptr; ///< low pass filter envelope
	slScheduler* sched = nullptr; ///< plib sl scheduler (see sl.h)
public:
	PlibSound(slScheduler* sched,
			  const char* filename,
			  unsigned int flags = (ACTIVE_VOLUME|ACTIVE_PITCH),
			  bool loop = false);
	PlibSound(const PlibSound &); // = delete;
	PlibSound & operator = (const PlibSound &); // = delete;
	virtual ~PlibSound();
	virtual void setVolume(float vol);
	//virtual void setSource(sgVec3 p, sgVec3 u);
	//virtual void setListener (sgVec3 p, sgVec3 u);
	virtual void play();
	virtual void start();
	virtual void stop();
	virtual void resume();
	virtual void pause();
	virtual void update();
};

#endif
