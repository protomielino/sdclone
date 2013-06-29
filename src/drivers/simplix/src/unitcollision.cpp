//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
// unitcollision.cpp
//--------------------------------------------------------------------------*
// TORCS: "The Open Racing Car Simulator"
// A robot for Speed Dreams-Version 1.4.0/2.X
//--------------------------------------------------------------------------*
// Information about collisions to avoid
// Informationen über drohende Kollisionen
//
// File         : unitcollision.cpp
// Created      : 2007.11.25
// Last changed : 2013.03.02
// Copyright    : © 2007-2011 Wolf-Dieter Beelitz
// eMail        : wdb@wdbee.de
// Version      : 4.00.000
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
// This program was developed and tested on windows XP
// There are no known Bugs, but:
// Who uses the files accepts, that no responsibility is adopted
// for bugs, dammages, aftereffects or consequential losses.
//
// Das Programm wurde unter Windows XP entwickelt und getestet.
// Fehler sind nicht bekannt, dennoch gilt:
// Wer die Dateien verwendet erkennt an, dass für Fehler, Schäden,
// Folgefehler oder Folgeschäden keine Haftung übernommen wird.
//--------------------------------------------------------------------------*
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// Im übrigen gilt für die Nutzung und/oder Weitergabe die
// GNU GPL (General Public License)
// Version 2 oder nach eigener Wahl eine spätere Version.
//--------------------------------------------------------------------------*
#include "unitglobal.h"
#include "unitcommon.h"

#include "unitdriver.h"
#include "unitcollision.h"

//==========================================================================*
// Default constructor
//--------------------------------------------------------------------------*
TCollision::TCollision()
{
}
//==========================================================================*

//==========================================================================*
// Destructor
//--------------------------------------------------------------------------*
TCollision::~TCollision()
{
}
//==========================================================================*

//==========================================================================*
// Avoid to side
//--------------------------------------------------------------------------*
double TCollision::AvoidTo
  (const TCollInfo& Coll, 
  const PCarElt oCar, TDriver& Me, bool& DoAvoid)
{
  double AvoidTo = 0.0;                          // Undefined side

  // First priority: Anyone at side?
  if (Coll.OppsAtSide)                           // Opponents at side?
  {
    int Flags = Coll.OppsAtSide;                 // Get corresponding flags
	double Offset;

    AvoidTo = (Flags & F_LEFT) ? 1.0 : -1.0;     // Go away from opponent

	if (Flags == (F_LEFT | F_RIGHT))             // Opps on both sides?
	{                                            //   Then use middle
	  Offset = 0.5 *                             // Offset is an estimate
		(Coll.MinRSideDist - Coll.MinLSideDist)  //   of where this is.
		- CarToMiddle;
	}
	else
	{
	  if (AvoidTo > 0)
	  {
		  double B = Coll.MinLSideDist + oCar->pub.trkPos.toRight;
		  double W = oCar->pub.trkPos.toLeft + oCar->pub.trkPos.toRight;
		  Offset = (W - B)/2;
	  }
	  else
	  {
		  double B = Coll.MinRSideDist + oCar->pub.trkPos.toLeft;
		  double W = oCar->pub.trkPos.toLeft + oCar->pub.trkPos.toRight;
		  Offset = -(W - B)/2;
	  }
	}

/*
	else if (Coll.OppsAhead)                     // Opponents in front?
	{ // Choose side that has potential of overtaking the car ahead
	  if ((Coll.OppsAhead == F_LEFT)             // Opponent ahead is left  
		&& (Flags == F_RIGHT))                   //   an another right? 
	  {
		Offset = -(Coll.MinLDist - 0.5)          // Take remaining place           
		  - CarToMiddle;
		LogSimplix.debug("Go between1: %g\n",Offset);
	  }
	  else if ((Coll.OppsAhead == F_RIGHT)       // Opponent ahead is right
		&& (Flags == F_LEFT))                    //   and another left?
	  {
		Offset = (Coll.MinRDist - 0.5)           // Take remaining place           
		  - CarToMiddle;
		LogSimplix.debug("Go between2: %g\n",Offset);
	  }
	  else if (Coll.OppsAhead == F_LEFT)         // Opponent ahead is left
	  {
	    if (Coll.MinLSideDist < 3.0) 
		{
	      Offset = 0.5 *                         // Take remaining place           
		    (3 - Coll.MinLSideDist)              
		    - CarToMiddle;
		  LogSimplix.debug("Go to right2: %g\n",Offset);
		}
		else
		{
          LogSimplix.debug("AvoidTo1: %g\n",AvoidTo);
	      return AvoidTo;                        // Do not Avoid
		}
	  }
	  else if (Coll.OppsAhead == F_RIGHT)        // Opponent ahead is right
	  {
	    if (Coll.MinRSideDist < 3.0)
		{
	      Offset = 0.5 *                         // Take remaining place           
		    (Coll.MinRSideDist - 3)                 
		    - CarToMiddle;
		  LogSimplix.debug("Go to left2: %g\n",Offset);
		}
		else
		{
          LogSimplix.debug("AvoidTo2: %g\n",AvoidTo);
	      return AvoidTo;                        // Do not Avoid
		}
	  }
	  else
	  {
        LogSimplix.debug("AvoidTo3: %g\n",AvoidTo);
	    return AvoidTo;                          // Do not Avoid
	  }
	}
    else
	{
	  if ((Coll.MinLSideDist < 2.5) || (Coll.MinRSideDist < 2.5))
        DoAvoid = true;                          // Avoid to side

	  if (DoAvoid)
        LogSimplix.debug("DoAvoid AvoidTo4: %g\n",AvoidTo);
	  else
        LogSimplix.debug("AvoidTo4: %g\n",AvoidTo);

	  return AvoidTo;                             
	}
*/
    DoAvoid = true;                              // Avoid to side
    Offset = Me.CalcPathTarget                   // Use offset to
      (DistanceFromStartLine, Offset);           //   find target
    LogSimplix.debug("DoAvoid Offset1: %g\n",Offset);
    return Offset; 
  }
  else // No opponents at side ...
  {
    int Flags; 
    double Offset = -1.0;                        // Calc target by offset
    
	// Second priority: Go to side depending on preview
	if (Coll.AvoidSide < 0)
	{
	    Flags = F_LEFT;
		Offset = Coll.AvoidSide;
		LogSimplix.debug("Go to right: %g\n",Offset);
	}
	else if (Coll.AvoidSide > 0)
	{
	    Flags = F_RIGHT;
		Offset = Coll.AvoidSide;
		LogSimplix.debug("Go to left: %g\n",Offset);
	}
	// Third priority: Anyone behind?
	else if (Coll.LappersBehind)                 // Lappers behind?
	{
	  Flags = Coll.LappersBehind;                // Get corresponding flags
	  if (Flags == (F_LEFT | F_RIGHT))
	  { // lapping cars on both sides behind!
	    Flags =                                  // Use the side defined
		  (Coll.NextSide < 0) ? F_LEFT : F_RIGHT;//   by collision
		Offset = (Flags & F_LEFT) ? 1.0 : -1.0;
		LogSimplix.debug("LappersBehind: %g\n",Offset);
	  }
	}
	// Fourth priority: More than one ahead?
    else if (Coll.OppsAhead == (F_LEFT | F_RIGHT))
    { // cars on both sides ahead, so avoid closest (or slowest) car
      Flags = (Coll.MinLDist < Coll.MinRDist) ? F_LEFT : F_RIGHT; 
	  Offset = (Flags & F_LEFT) ? 1.0 : -1.0;
      LogSimplix.debug("(Coll.OppsAhead == (F_LEFT | F_RIGHT)): %g\n",Offset);
	}
	// Fifth priority: Anyone ahead? 
    else if (Coll.OppsAhead)
	{ // cars on one side ahead
      Flags = Coll.OppsAhead;
	  Offset = (Flags & F_LEFT) ? 1.0 : -1.0;
      LogSimplix.debug("(Coll.OppsAhead): %g\n",Offset);
	}
	else 
	{
      //LogSimplix.debug("AvoidTo5: %g\n",AvoidTo);
      return AvoidTo;                            // Do not avoid
	}

	DoAvoid = true;                              // Flag avoid
    LogSimplix.debug("DoAvoid Offset2: %g\n",Offset);
    return Offset;                               // Use offset to find target
  }
}
//--------------------------------------------------------------------------*
// end of file unitcollision.cpp
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
