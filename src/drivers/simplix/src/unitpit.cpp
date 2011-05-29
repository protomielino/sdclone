//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
// unitpit.cpp
//--------------------------------------------------------------------------*
// TORCS: "The Open Racing Car Simulator"
// A robot for Speed Dreams-Version 1.4.0/2.X
//--------------------------------------------------------------------------*
// Pit ans pitlane
// Box und Boxengasse
//
// File         : unitpit.cpp
// Created      : 2007.02.20
// Last changed : 2011.05.29
// Copyright    : © 2007-2011 Wolf-Dieter Beelitz
// eMail        : wdb@wdbee.de
// Version      : 3.00.003
//--------------------------------------------------------------------------*
// Diese Unit basiert auf dem erweiterten Robot-Tutorial bt
//
//    Copyright: (C) 2002-2004 Bernhard Wymann
//    eMail    : berniw@bluewin.ch
//
// und dem Roboter delphin 2006
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

#include "unitlinalg.h"
#include "unitpit.h"
#include "unitcubicspline.h"

//==========================================================================*
// [m/s] savety margin to avoid pit speeding.
//--------------------------------------------------------------------------*
const float TPit::SPEED_LIMIT_MARGIN = 0.5;
//==========================================================================*

//==========================================================================*
// Konstruktor
//--------------------------------------------------------------------------*
TPit::TPit(TDriver *Driver)
{
  oTrack = Driver->Track();
  oCar = Driver->Car();
  oMyPit = Driver->Car()->_pit;
  oPitInfo = &oTrack->pits;
  oPitStop = oInPitLane = false;
  oPitTimer = 0.0;

  if (oMyPit != NULL)
  {
	oSpeedLimit = oPitInfo->speedLimit - SPEED_LIMIT_MARGIN;
	oSpeedLimitSqr = oSpeedLimit*oSpeedLimit;
	oPitSpeedLimitSqr = oPitInfo->speedLimit*oPitInfo->speedLimit;
  }
  else
    GfOut("\n\n\n SIMPLIX: NO PIT \n\n\n");

  for (int I = 0; I < gNBR_RL; I++)
    oPitLane[I].Init(Driver->Car());
}
//==========================================================================*

//==========================================================================*
// Destruktor
//--------------------------------------------------------------------------*
TPit::~TPit()
{
}
//==========================================================================*

//==========================================================================*
// Transforms track coordinates to spline parameter coordinates.
//--------------------------------------------------------------------------*
float TPit::ToSplineCoord(float X)
{
  X -= oPitEntry;
  while (X < 0.0f)
	X += oTrack->length;

  return X;
}
//==========================================================================*

//==========================================================================*
// Computes offset to track middle for trajectory.
//--------------------------------------------------------------------------*
float TPit::GetPitOffset(float Offset, float FromStart)
{
  if (oMyPit != NULL)
  {
	if (GetInPit() || (GetPitstop() && IsBetween(FromStart)))
	  FromStart = ToSplineCoord(FromStart);
  }
  return Offset;
}
//==========================================================================*

//==========================================================================*
// Sets the pitstop flag if we are not in the pit range.
//--------------------------------------------------------------------------*
void TPit::SetPitstop(bool PitStop)
{
  if (oMyPit == NULL)
	return;

  float FromStart = DistanceFromStartLine;

  if (!PitStop)
	this->oPitStop = PitStop;                 // Reset every time
  else if (!IsBetween(FromStart))
	this->oPitStop = PitStop;
  else if (!PitStop)
  {
	this->oPitStop = PitStop;
	oPitTimer = 0.0f;
  }
}
//==========================================================================*

//==========================================================================*
// Check if the argument fromstart is in the range of the pit.
//--------------------------------------------------------------------------*
bool TPit::IsBetween(float FromStart)
{
  if (oPitEntry <= oPitExit)
  {
	GfOut("1. FromStart: %g\n",FromStart);
	if (FromStart >= oPitEntry && FromStart <= oPitExit)
  	  return true;
	else
	  return false;
  }
  else
  {
	// Warning: TORCS reports sometimes negative values for "fromstart"!
	GfOut("2. FromStart: %g\n",FromStart);
	if (FromStart <= oPitExit || FromStart >= oPitEntry)
	  return true;
	else
	  return false;
  }
}
//==========================================================================*

//==========================================================================*
// Checks if we stay too long without getting captured by the pit.
// Distance is the distance to the pit along the track, when the pit is
// ahead it is > 0, if we overshoot the pit it is < 0.
//--------------------------------------------------------------------------*
bool TPit::IsTimeout(float Distance)
{
  if (CarSpeedLong > 1.0f || Distance > 3.0f || !GetPitstop())
  {
	oPitTimer = 0.0f;
	return false;
  }
  else
  {
	oPitTimer += (float) RCM_MAX_DT_ROBOTS;
	if (oPitTimer > 3.0f)
	{
	  oPitTimer = 0.0f;
	  return true;
	}
	else
	  return false;
  }
}
//==========================================================================*

//==========================================================================*
// Update pit data and strategy.
//--------------------------------------------------------------------------*
void TPit::Update()
{
  if (oMyPit != NULL)
  {
	if (IsBetween(DistanceFromStartLine))
	{
	  if (GetPitstop())
		SetInPit(true);
	}
	else
	  SetInPit(false);

	if (GetPitstop())
	  CarRaceCmd = RM_CMD_PIT_ASKED;

  }
}
//==========================================================================*

//==========================================================================*
// Get speed limit brake
//--------------------------------------------------------------------------*
float TPit::GetSpeedLimitBrake(float SpeedSqr)
{
  return (SpeedSqr-oSpeedLimitSqr)/(oPitSpeedLimitSqr-oSpeedLimitSqr);
}
//==========================================================================*

//==========================================================================*
// Make Path
//--------------------------------------------------------------------------*
void TPitLane::Init(PtCarElt Car)
{
  oCar = Car;
  oPitStopOffset = 0.0;
}
//==========================================================================*

//==========================================================================*
// Smooth Path with pitlane
//--------------------------------------------------------------------------*
void TPitLane::SmoothPitPath
  (const TParam& Param)
{
  int I;                                         // Loop counter

  int NSEG = oTrack->Count();                    // Number of sections in the path
  int Idx0 = oTrack->IndexFromPos(oPitEntryPos); // First index and
  int Idx1 = oTrack->IndexFromPos(oPitExitPos);  // last index to modify

  // Modify path for use of normal smoothing
  for (I = Idx0; I != Idx1; I = (I + 1) % NSEG)
  {
    oPathPoints[I].WToL = oPathPoints[I].WPitToL;
    oPathPoints[I].WToR = oPathPoints[I].WPitToR;
  }

  // Smooth pit path
  float BumpMode = (float) Param.oCarParam.oScaleBump;
  SmoothPath(TClothoidLane::TOptions(BumpMode));
}
//==========================================================================*

//==========================================================================*
// Make Path with pitlane
//--------------------------------------------------------------------------*
void TPitLane::MakePath
  (char* Filename, 
  TAbstractStrategy* Strategy,
  TClothoidLane* BasePath,
  const TParam& Param, int Index)
{
  // 1. Check whether we got a place to build our pit
  const tTrackOwnPit* Pit = CarPit;              // Get my pit
  if (Pit == NULL)                               // If pit is NULL
  {                                              // nothing to do
	GfOut("\n\nPit = NULL\n\n");                 // here
	return;
  }

  // 2. We have a place, let us build all we need to use it ...
  const int NPOINTS = 7;                         // Nbr of points defined for pitlane
  double X[NPOINTS];                             // X-coordinates
  double Y[NPOINTS];                             // Y-coordinates
  double S[NPOINTS];                             // Directions

  bool FirstPit = false;                         // Reset flag
  int I;                                         // Loop counter
  TCarParam CarParam = Param.oCarParam3;         // Copy parameters
  TLane::SetLane(*BasePath);                     // Copy Pathpoints
  const tTrackPitInfo* PitInfo =                 // Get pit infos
	&oTrack->Track()->pits;

  // At which side of the track is the place to build the pit?
  int Sign =                                     // Get the side of pits
	(PitInfo->side == TR_LFT) ? -1 : 1;

  // To be able to avoid in the pitlane we need he offsets
  float F[3] = {0.5, 1.0, 0.0};                  // Avoid offsets
  if (Sign < 0)                                  // If pits are on the
  {                                              // left side
    F[1] = 0.0;                                  // swap
    F[2] = 1.0;
  }

  // Different cars need different distances
  oStoppingDist = Param.Pit.oStoppingDist;       // Distance to brake
  oPitStopOffset = Param.Pit.oLongOffset;        // Offset for fine tuning
  oCarParam.oScaleBrake =                        // Limit brake to be used
	MAX(0.10f,CarParam.oScaleBrake);             //   in pitlane
  oCarParam.oScaleMu =                           // Scale friction estimation
	MAX(0.10f,CarParam.oScaleMu);                //   of pitlane

  // Get the distance of pit to middle of the track
  double PitLaneOffset =                         // Offset of the pitlane
	fabs(PitInfo->driversPits->pos.toMiddle)     //   from the middle of the
	- PitInfo->width;                            //   track

  // To get same steering angles the distance to start to steer to the pit
  // depends on the side of the driven lane in the pitlane (from avoiding)
  float Ratio = (float) (0.5 * 
	  PitInfo->len / (fabs(PitInfo->driversPits->pos.toMiddle) - PitLaneOffset));

  // Compute pit spline points along the track defined by TORCS/SD
  X[0] = PitInfo->pitEntry->lgfromstart          // Start of Pitlane defined
	  + Param.Pit.oEntryLong;                    //   our own offset along the track
  X[1] = PitInfo->pitStart->lgfromstart;         // Start of speedlimit
  X[3] = Pit->pos.seg->lgfromstart               // Center of our own pit
	+ Pit->pos.toStart + oPitStopOffset;         //   with own offset along track
  X[2] = X[3] - PitInfo->len                     // Start enter own pit here
	- F[Index] * Ratio * Param.Pit.oLaneEntryOffset;
  X[4] = X[3] + PitInfo->len                     // Leave own pit here
	+ F[Index] * Ratio * Param.Pit.oLaneExitOffset;
  X[5] = X[1] + PitInfo->nPitSeg * PitInfo->len; // End of speed limit
  X[6] = PitInfo->pitExit->lgfromstart           // End of pitlane defind
	+ PitInfo->pitExit->length                   //   and own offset along track
	+ Param.Pit.oExitLong;

  oPitEntryPos = X[0];                           // Save this value as start of spline

  // Normalizing spline segments to X[I+1] >= X[I] if startline is crossed
  for (I = 1; I < NPOINTS; I++)
  {
    X[I] = ToSplinePos(X[I]);
    S[I] = 0.0;
  }

  // Now the dark side of TORCS and SD ...

  // Fix start of pitlane point for first pit if necessary.
  if (X[2] < X[1])
  {
    FirstPit = true;                             // Hey we use the first pit!
    X[1] = X[2] - 1.0;                           // Congratulation 
  }

  // Fix end of pitlane point for last pit if necessary.
  if (X[5] < X[4])                               // May be this is not your race
    X[5] = X[4] + 1.0;

  // Fix broken pit exit.
  if (X[6] < X[5])
    X[6] = X[5] + 50.0;

  // Recalculate spline segments to X[I+1] >= X[I] if startline is crossed
  for (I = 1; I < NPOINTS; I++)
  {
    X[I] = ToSplinePos(X[I]);
    S[I] = 0.0;
  }

  oPitEntryPos = X[0];                           // Save this values for later
  oPitStartPos = X[1];                           // use
  oPitEndPos   = X[5];
  oPitExitPos  = X[6];

  // For tuning the exit we need to know the distance 
  // from pit to end of pitlane
  Strategy->oPit->oDistToPitEnd = 
	oPitEndPos - X[3];

  // Get track positions from the spline station
  if (oPitStartPos > oTrack->Length())           // Use normalized position
	  oPitStartPos -= oTrack->Length();
  if (oPitEndPos > oTrack->Length())             // Use normalized position
	  oPitEndPos -= oTrack->Length();
  if (oPitExitPos > oTrack->Length())            // Use normalized position
	  oPitExitPos -= oTrack->Length();

  // Splice entry/exit of pit path into the base path provided.
  TLanePoint Point;                              // Data of the point
  BasePath->GetLanePoint(oPitEntryPos, Point);   // as defined by TORCS
  Y[0] = Point.Offset;                           // Lateral distance
  S[0] = -tan(Point.Angle                        // Direction of our
	- oTrack->ForwardAngle(oPitEntryPos));       //   basic racingline

  BasePath->GetLanePoint(oPitExitPos, Point);    // As defined by TORCS
  Y[6] = Point.Offset;                           // Lateral distance and
  S[6] = -tan(Point.Angle                        // and direction at the
	- oTrack->ForwardAngle(oPitExitPos));        // TORCS end of pitlane

  // First use a generic path through the pitlane without the pit itself
  if (Param.Pit.oUseFirstPit && FirstPit)        // If allowed and possible
  {                                              // we will use a special path to
	Y[3] = Y[2] = Y[1] = Sign *                  // the first pit with
	  (fabs(PitInfo->driversPits->pos.toMiddle   //   TORCS defined offset
	  - 0.5 + Param.Pit.oLatOffset));            //   and our own lateral offset
  }
  else                                           // All other pits
  {                                              // have to be reached over
//    Y[3] = Y[2] = Y[1] = Sign *                  // a path defined here
    Y[2] = Y[1] = Sign *                         // a path defined here
	  (PitLaneOffset -                           // Sign gives the side of the pits
	  Param.Pit.oLaneEntryOffset * F[Index]);    // and we correct the TORCS offset

	Y[3] = Sign *                                // we have to set the pit 
	  (fabs(PitInfo->driversPits->pos.toMiddle)  // offset
	  + Param.Pit.oLatOffset);                   // 
  }

  Y[5] = Y[4] = Sign *                           // Leaving the own pit, we will
    (PitLaneOffset                               //   go to the pitlane and use
    - Param.Pit.oLaneExitOffset * F[Index]);     //   an additional offset

  // Calculate the splines for entry and exit of pitlane
  TCubicSpline PreSpline(NPOINTS, X, Y, S);      

  // Modify points in line path for pits ...
  int NSEG = oTrack->Count();                    // Number of sections in the path

  // Start at section with speedlimit
  int Idx0 = oTrack->IndexFromPos(oPitStartPos); // Index to first point
  int Idx1 = oTrack->IndexFromPos(oPitEndPos);   // Index to last point
  for (I = Idx0; I != Idx1; I = (I + 1) % NSEG)  // Set Flag, to keep the points
	oPathPoints[I].Fix = true;                   //   while smooting

  // Pit entry and pit exit as defined by TORCS/SD
  Idx0 = (1 + oTrack->IndexFromPos(oPitEntryPos)) % NSEG;
  Idx1 = oTrack->IndexFromPos(oPitExitPos);

  // Change offsets to go to the pitlane based on the splines
  for (I = Idx0; I != Idx1; I = (I + 1) % NSEG)
  {
    double SplineY;                              // Offset lateral to track
    double SplineX =                             // Station in spline coordinates
	  ToSplinePos(oTrack->Section(I).DistFromStart);

	// Restrict to length of spline
	if (SplineX > X[6])
		SplineX = X[6];

	// Calculate offset to side depending on pit side
    if (Sign < 0) 
      SplineY = MAX(PreSpline.CalcOffset(SplineX),-(oPathPoints[I].WPitToL - 1.3));
    else
      SplineY = MIN(PreSpline.CalcOffset(SplineX),oPathPoints[I].WPitToR - 1.3);

	oPathPoints[I].Offset = SplineY;             // Offset lateral to track
    oPathPoints[I].Point =                       // Recalculate point coordinates
	  oPathPoints[I].CalcPt();                   //   from offset
    //GfOut("Spline: %g/%g\n",SplineX,oPathPoints[I].Offset);
  }

  // Prepare speed calculation with changed path
  int FwdRange = 110;
  CalcFwdAbsCrv(FwdRange);
  CalcCurvaturesXY();
  CalcCurvaturesZ();
  CalcMaxSpeeds(1);

  // Overwrite speed calculations at section with speed limit
  Idx0 = (oTrack->IndexFromPos(oPitStartPos) + NSEG - 5) % NSEG;
  Idx1 = (oTrack->IndexFromPos(oPitEndPos) + 1) % NSEG;

  // Allowed speed in pitlane itself
  for (I = Idx0; I != Idx1; I = (I + 1) % NSEG)
  {
    double Speed = MIN(oPathPoints[I].Speed, PitInfo->speedLimit - 0.5);
    oPathPoints[I].MaxSpeed = oPathPoints[I].Speed = Speed;
  }

  // Save stop position
  double StopPos = Pit->pos.seg->lgfromstart     // As defined by TROCS/SD
	+ Pit->pos.toStart + oPitStopOffset;         // with own offset along track#

  // Normalize it to be 0 >= StopPos > track length
  if (StopPos >= oTrack->Length())               
    StopPos -= oTrack->Length();
  else if (StopPos < 0)
    StopPos += oTrack->Length();

  // Section at pit stop
  oStopIdx = Idx0 = oTrack->IndexFromPos(StopPos);

  // Set speed to restart faster
  for (I = 0; I < 15; I++)
  {
    Idx0 = (Idx0 + 1) % NSEG;
    oPathPoints[Idx0].MaxSpeed = 
	  oPathPoints[Idx0].Speed = 
	    PitInfo->speedLimit - 0.5;
  }

  oStopPos = StopPos;
  oPitStopPos = oPathPoints[oStopIdx].Dist();
  GfOut("#\n");
  GfOut("#\n");
  GfOut("#StopPos: %d (%.2f m)\n",oStopIdx,oStopPos);

  // Set target speed at stop position
  oPathPoints[oStopIdx].MaxSpeed = oPathPoints[oStopIdx].Speed = 1.0;

  // Calculate braking
  PropagatePitBreaking((tdble) oPitStopPos,(tdble) Param.oCarParam.oScaleMu);

  // Look for point to decide to go to pit
  Idx0 = oTrack->IndexFromPos(oPitEntryPos);
  double DeltaSpeed;
  int Steps = 0;
  do
  {
    Idx0 = (Idx0 + NSEG - 1) % NSEG;
    DeltaSpeed = 
	  MIN(BasePath->oPathPoints[Idx0].Speed,BasePath->oPathPoints[Idx0].AccSpd) - 
	  oPathPoints[Idx0].Speed;
    //GfOut("#DeltaSpeed: %d (%.2f km/h)(%.1f km/h)(%.1f km/h)\n",Idx0,DeltaSpeed*3.6,oPathPoints[Idx0].Speed*3.6,BasePath->oPathPoints[Idx0].Speed*3.6);
  } while ((DeltaSpeed > 1.0) && (++Steps < NSEG));
  GfOut("#Steps to pit entry: %d\n",Steps);

  // Distance of pit entry to pit stop point
  oPitDist = oPitStopPos - oPathPoints[Idx0].Dist();
  GfOut("#Pit dist      : %.2f\n",oPitDist);
  if (oPitDist < 0)
    oPitDist += oTrack->Length();
  GfOut("#Pit dist norm.: %.2f\n",oPitDist);

}
//==========================================================================*

//==========================================================================*
// Are we in Pit Section?
//--------------------------------------------------------------------------*
bool TPitLane::InPitSection(double TrackPos) const
{
  TrackPos = ToSplinePos(TrackPos);
  return oPitEntryPos < TrackPos && TrackPos < ToSplinePos(oPitExitPos);
}
//==========================================================================*

//==========================================================================*
// Check wether we can stop in pit
//--------------------------------------------------------------------------*
bool TPitLane::CanStop(double TrackPos) const
{
  double D = DistToPitStop(TrackPos, true);
  if ((D < oStoppingDist) || (oTrack->Length() - D < oStoppingDist))
    return true;
  else
    return false;
}
//==========================================================================*

//==========================================================================*
// Check wether we overrun stop pos
//--------------------------------------------------------------------------*
bool TPitLane::Overrun(double TrackPos) const
{
  double D = DistToPitStop(TrackPos, true);
  if ((D > oTrack->Length() / 2) && (oTrack->Length() - D > oStoppingDist))
	return true;
  else
    return false;
}
//==========================================================================*

//==========================================================================*
// Get Distance to Pit entry
//--------------------------------------------------------------------------*
double TPitLane::DistToPitEntry(double TrackPos) const
{
  double Dist = oPitEntryPos - TrackPos;
  if (Dist < 0)
	Dist += oTrack->Length();
  return Dist;
}
//==========================================================================*

//==========================================================================*
// Get Distance to Pit entry
//--------------------------------------------------------------------------*
double TPitLane::DistToPitStop(double TrackPos, bool Pitting) const
{
  double Dist;
  float DL, DW;

  if (Pitting)
  {
//	  dist = oPitStopPos - trackPos;
    RtDistToPit(oCar,oTrack->Track(),&DL,&DW);
    DL += (float)(oPitStopOffset - TRACKRES / 2);
//	  GfOut("DistToPitStop: %g-%g=%g\n",dist,DL,dist-DL);
    Dist = DL;
  	if (Dist < 0)
	  Dist += oTrack->Length();
  }
  else
  {
	Dist = oPitStopPos - oPitEntryPos;
	if (Dist < 0)
	  Dist += oTrack->Length();
	  Dist += DistToPitEntry(TrackPos);
  }
  return Dist;
}
//==========================================================================*

//==========================================================================*
// Calculate local position
//--------------------------------------------------------------------------*
double TPitLane::ToSplinePos(double TrackPos) const
{
  if (TrackPos < oPitEntryPos)
	TrackPos += oTrack->Length();
  return TrackPos;
}
//--------------------------------------------------------------------------*
// end of file unitpit.cpp
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*
