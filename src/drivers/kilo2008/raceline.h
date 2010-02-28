/*
 *      raceline.h
 *      
 *      Copyright 2009 kilo aka Gabor Kmetyko <kg.kilo@gmail.com>
 *      Based on work by Bernhard Wymann, Andrew Sumner, Remi Coulom.
 *      
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *      
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *      
 *      You should have received a copy of the GNU General Public License
 *      along with this program; if not, write to the Free Software
 *      Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *      MA 02110-1301, USA.
 * 
 *      $Id$
 * 
 */

#ifndef _RACELINE_H_
#define _RACELINE_H_

#include "linalg.h"

enum { LINE_MID=0, LINE_RL };

#define MAXSEGMENTS 7000
#define MAXDIVS 12000

class LRaceLine {
 public:
  LRaceLine()
  {
   fDirt = 0;
  }
   
  void setMinCornerInverse( double wi ) { MinCornerInverse = wi; }
  void setCornerSpeed( double wi ) { CornerSpeed = wi; }
  void setCornerAccel( double wi ) { CornerAccel = wi; }
  void setBrakeDelay( double wi ) { BrakeDelay = wi; }
  void setIntMargin( double wi ) { IntMargin = wi; }
  void setExtMargin( double wi ) { ExtMargin = wi; }
  void setAvoidSpeedAdjust( double wi ) { AvoidSpeedAdjust = wi; }
  void setCar( tCarElt *mycar ) { car = mycar; }

  double MinCornerInverse;
  double CornerSpeed;
  double CornerAccel;
  double BrakeDelay;
  double IntMargin;
  double ExtMargin;
  double AvoidSpeedAdjust;

  double wheelbase;
  double wheeltrack;

  int Divs;
  int DivLength;
  int Segs;
  double TargetSpeed;
  double Width;
  double Length;
  double tSegDist[MAXSEGMENTS];
  int tSegIndex[MAXSEGMENTS];
  double tElemLength[MAXSEGMENTS];
  double tx[2][MAXDIVS];
  double ty[2][MAXDIVS];
  double tDistance[MAXDIVS];
  double tRInverse[MAXDIVS];
  double tMaxSpeed[MAXDIVS];
  double tSpeed[2][MAXDIVS];
  double txLeft[MAXDIVS];
  double tyLeft[MAXDIVS];
  double txRight[MAXDIVS];
  double tyRight[MAXDIVS];
  double tLane[MAXDIVS];
  double tFriction[MAXDIVS];

  int fDirt;
  int Next;
  int This;

  tCarElt *car;

  void UpdateTxTy(int i, int rl);
  void SetSegmentInfo(const tTrackSeg *pseg, double d, int i, double l);
  void SplitTrack(tTrack *ptrack, int rl);
  double GetRInverse(int prev, double x, double y, int next, int rl);
  void AdjustRadius(int prev, int i, int next, double TargetRInverse, int rl, double Security = 0);
  void Smooth(int Step, int rl);
  void StepInterpolate(int iMin, int iMax, int Step, int rl);
  void Interpolate(int Step, int rl);
  void InitTrack(tTrack* track, tSituation *p);
  void NewRace(tCarElt* newcar, tSituation *s);
  void GetRaceLineData(tSituation *s, v2d *target, float *speed, float *avspeed, float *raceoffset, float *lookahead, float *racesteer);
  void GetPoint( float offset, float lookahead, vec2f *rt );
  int isOnLine();
  double correctLimit();
  double getAvoidSpeed( float distance1, float distance2 );
  double getRInverse(void) { return tRInverse[Next]; }
  double getRInverse(double distance) {int d = ((Next + int(distance / DivLength)) % Divs); return tRInverse[d]; }
};

#endif // _SPLINE_H_

