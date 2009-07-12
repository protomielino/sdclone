//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
// unitcommondata.h
//--------------------------------------------------------------------------*
// TORCS: "The Open Racing Car Simulator"
// Roboter für TORCS-Version 1.3.0
// Zentrale Klasse für das Fahren bzw. den Fahrer/Roboter
//
// File         : unitcommondata.h
// Created      : 2007.11.17
// Last changed : 1009.02.25
// Copyright    : © 2007-2009 Wolf-Dieter Beelitz
// eMail        : wdb@wdbee.de
// Version      : 2.00.000
//--------------------------------------------------------------------------*
// Ein erweiterter TORCS-Roboters
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
// dem Roboter delphin
//
//    Copyright: (C) 2006-2007 Wolf-Dieter Beelitz
//    eMail    : wdb@wdbee.de
//
// dem Roboter wdbee_2007
//
//    Copyright: (C) 2006-2007 Wolf-Dieter Beelitz
//    eMail    : wdb@wdbee.de
//
// und dem Roboter mouse_2006
//
//    Copyright: (C) 2006-2007 Tim Foden
//
//--------------------------------------------------------------------------*
// Diese Version wurde mit MS Visual C++ 2005 Express Edition entwickelt.
//--------------------------------------------------------------------------*
// Das Programm wurde unter Windows XP entwickelt und getestet.
// Fehler sind nicht bekannt, dennoch gilt:
// Wer die Dateien verwendet erkennt an, dass für Fehler, Schäden,
// Folgefehler oder Folgeschäden keine Haftung übernommen wird.
//
// Im übrigen gilt für die Nutzung und/oder Weitergabe die
// GNU GPL (General Public License)
// Version 2 oder nach eigener Wahl eine spätere Version.
//--------------------------------------------------------------------------*
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//--------------------------------------------------------------------------*
#ifndef _UNITCOMMONDATA_H_
#define _UNITCOMMONDATA_H_

//#undef TORCS_NG

#include <track.h>
#include "unitglobal.h"
#ifdef TORCS_NG
#else
#include "unitteammanager.h"
#endif
#include "unitclothoid.h"

//==========================================================================*
// Deklaration der Klasse TCommonData
//--------------------------------------------------------------------------*
class TCommonData  
{
  public:
    TCommonData();                               // Default constructor
    ~TCommonData();                              // Destructor

  public:
#ifdef TORCS_NG
#else
    TTeamManager TeamManager;                    // Team manager 
#endif
    PTrack Track;                                // TORCS Track data  
};
//==========================================================================*
#endif // _UNITCOMMONDATA_H_
//--------------------------------------------------------------------------*
// end of file unitcommondata.h
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
