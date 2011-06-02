//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
// unitcarparam.cpp
//--------------------------------------------------------------------------*
// TORCS: "The Open Racing Car Simulator"
// A robot for Speed Dreams-Version 1.4.0/2.X
//--------------------------------------------------------------------------*
// Tuningparameter des Fahrzeugs
//
// File         : unitcarparam.cpp
// Created      : 2007.11.25
// Last changed : 2011.06.02
// Copyright    : � 2007-2011 Wolf-Dieter Beelitz
// eMail        : wdb@wdbee.de
// Version      : 3.01.000
//--------------------------------------------------------------------------*
// Diese Unit basiert auf dem Roboter mouse_2006
//
//    Copyright: (C) 2006-2007 Tim Foden
//
//--------------------------------------------------------------------------*
// This program was developed and tested on windows XP
// There are no known Bugs, but:
// Who uses the files accepts, that no responsibility is adopted
// for bugs, dammages, aftereffects or consequential losses.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
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
#include "unitglobal.h"
#include "unitcommon.h"

#include "unitparabel.h"
#include "unitcarparam.h"

//==========================================================================*
// Default constructor
//--------------------------------------------------------------------------*
TCarParam::TCarParam():
  oScaleMu(0.8),
  oScaleMinMu(0.8),
  oScaleBrake(0.8),
  oScaleBrakePit(1.0),
  oScaleBump(0.4f),
  oScaleBumpOuter(0.6f),
  oScaleBumpLeft(0),
  oScaleBumpRight(0)
{
}
//==========================================================================*

//==========================================================================*
// Destructor
//--------------------------------------------------------------------------*
TCarParam::~TCarParam()
{
}
//===========================================================================

//==========================================================================*
// Zuweisung
//--------------------------------------------------------------------------*
TCarParam& TCarParam::operator= (const TCarParam& CarParam)
{
  oScaleMu = CarParam.oScaleMu;
  oScaleMinMu = CarParam.oScaleMinMu;
  oScaleBrake = CarParam.oScaleBrake;
  oScaleBrakePit = CarParam.oScaleBrakePit;
  oScaleBump = CarParam.oScaleBump;
  oScaleBumpOuter = CarParam.oScaleBumpOuter;
  oScaleBumpLeft = CarParam.oScaleBumpLeft;
  oScaleBumpRight = CarParam.oScaleBumpRight;

  return *this;
}
//===========================================================================

//--------------------------------------------------------------------------*
// end of file unitcarparam.cpp
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
