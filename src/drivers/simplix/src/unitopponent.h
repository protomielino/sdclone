//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
// unitopponent.h
//--------------------------------------------------------------------------*
// TORCS: "The Open Racing Car Simulator"
// A robot for Speed Dreams-Version 1.4.0/2.X
//--------------------------------------------------------------------------*
// Rivalen (und Teammitglieder)
//
// File         : unitopponent.h
// Created      : 2007.11.17
// Last changed : 2011.06.02
// Copyright    : � 2007-2011 Wolf-Dieter Beelitz
// eMail        : wdb@wdbee.de
// Version      : 3.01.000
//--------------------------------------------------------------------------*
// Teile diese Unit basieren auf diversen Header-Dateien von TORCS
//
//    Copyright: (C) 2000 by Eric Espie
//    eMail    : torcs@free.fr
//
// dem erweiterten Robot-Tutorial bt
//
//    Copyright: (C) 2002-2004 Bernhard Wymann
//    eMail    : berniw@bluewin.ch
//
// dem Roboter mouse_2006
//
//    Copyright: (C) 2006-2007 Tim Foden
//
// dem Roboter wdbee_2007
//
//    Copyright: (C) 2006-2007 Wolf-Dieter Beelitz
//    eMail    : wdb@wdbee.de
//
// und dem Roboter delphin
//
//    Copyright: (C) 2006-2007 Wolf-Dieter Beelitz
//    eMail    : wdb@wdbee.de
//
//--------------------------------------------------------------------------*
// Das Programm wurde unter Windows XP entwickelt und getestet.
// Fehler sind nicht bekannt, dennoch gilt:
// Wer die Dateien verwendet erkennt an, dass f�r Fehler, Sch�den,
// Folgefehler oder Folgesch�den keine Haftung �bernommen wird.
//
// Im �brigen gilt f�r die Nutzung und/oder Weitergabe die
// GNU GPL (General Public License)
// Version 2 oder nach eigener Wahl eine sp�tere Version.
//--------------------------------------------------------------------------*
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//--------------------------------------------------------------------------*
#ifndef _UNITOPPONENT_H_
#define _UNITOPPONENT_H_

//#undef SPEED_DREAMS

#include <car.h>
#include <raceman.h>

#include "unitglobal.h"
#include "unitcommon.h"

#ifdef SPEED_DREAMS
#else
#include "unitteammanager.h"
#endif

//==========================================================================*
// Class TOpponent
//--------------------------------------------------------------------------*
class TOpponent
{
  public:
	struct TState
	{
	  double Speed;

	  double TrackVelLong;	// Vel. longitudinal to track.
	  double TrackVelLat;	// Vel. lateral to track.
	  double TrackYaw;

	  double CarDistLong;
	  double CarDistLat;
	  double CarDiffVelLong;
	  double CarDiffVelLat;

	  double AvgVelLong;
	  double AvgVelLat;
	  double CarAvgVelLong;
	  double AvgAccLong;
	  double AvgAccLat;
	  double CarAvgAccLong;
	  double CarAvgAccLat;

	  double MinDistLong;                        // Minimum distance longitudinal
	  double MinDistLat;                         // Minimum distance lateral

	  double RelPos;                             // Relative position along track
	  double Offset;                             // Offset to center line of track
	};

	struct TInfo
	{
	  TInfo() {Clear();}
	  void Clear() {memset(this, 0, sizeof(*this));}
	  bool GotFlags(int F) const {return (Flags & F) == F;}

	  TState State;

	  int Flags;

	  double AvoidLatchTime;
	  double CatchTime;
	  double CatchSpeed;
	  double CatchDecel;
	  double CatchAccTime;
	  double CatchOffset;
	  double CarDistLong;
	  double DangerousLatchTime;
	  double TeamMateDamage;
	  double AvoidSide;
	  bool Blocked[9];
	};

  public:
	TOpponent();                                 // Default constructor
	~TOpponent();                                // Destructor

	void Initialise                              // Initialize opponent
	  (PTrackDescription Track,
	  const PSituation Situation,
	  int Index);

	PCarElt Car();                               // Get car pointer of opponent
	const TInfo& Info() const;                   // Get info as const
	TInfo& Info();                               // Get info

	void Update                                  // Update
	  (const PCarElt MyCar,
#ifdef SPEED_DREAMS
#else
	  PTeamManager TeamManager,
#endif
      double MyDirX, double MyDirY,
      float &MinDistBack,
      double &MinTimeSlot);

	bool Classify                                // Classification of opponents
	  (const PCarElt MyCar,
	  const TState& MyState,
	  /*bool OutOfPitlane,*/
	  double MyMaxAccX);

  private:
    PTrackDescription oTrack;                    // Track description
	PCarElt oCar;                                // Opponents car
	double oDeltaTime;                           // Simulation delta time
    int oIndex;                                  // Opponents cars index
	TInfo oInfo;                                 // info of this opponent
#ifdef SPEED_DREAMS
#else
    PTeamManager oTeamManager;                   // Teammanager
#endif
	double LapBackTimer;

};
//==========================================================================*
#endif // _UNITOPPONENT_H_
//--------------------------------------------------------------------------*
// end of file unitopponent.h
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
