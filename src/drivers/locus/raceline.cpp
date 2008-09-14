////////////////////////////////////////////////////////////////////////////
//
// K1999.cpp
//
// car driver for TORCS
// (c) Remi Coulom
// March 2000
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
////////////////////////////////////////////////////////////////////////////

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "tgf.h" 
#include "track.h" 
#include "car.h"
#include "raceman.h" 
#include "robot.h" 
#include "robottools.h"

#include "raceline.h"

////////////////////////////////////////////////////////////////////////////
// Parameters
////////////////////////////////////////////////////////////////////////////

//
// These parameters are for the computation of the path
//
static const int Iterations = 100;     // Number of smoothing operations
 
static const double SecurityR = 100.0; // Security radius
static double SideDistExt = 2.0; // Security distance wrt outside
static double SideDistInt = 1.0; // Security distance wrt inside          

/////////////////////////////////////////////////////////////////////////////
// Some utility macros and functions
/////////////////////////////////////////////////////////////////////////////

static double Mag(double x, double y)
{
 return sqrt(x * x + y * y);
}
 
static double Min(double x1, double x2)
{
 if (x1 < x2)
  return x1;
 else
  return x2;
}
 
static double Max(double x1, double x2)
{
 if (x1 < x2)
  return x2;
 else
  return x1;
}
 
/////////////////////////////////////////////////////////////////////////////
// Update tx and ty arrays
/////////////////////////////////////////////////////////////////////////////
void LRaceLine::UpdateTxTy(int i, int rl)
{
 tx[rl][i] = tLane[i] * txRight[i] + (1 - tLane[i]) * txLeft[i];
 ty[rl][i] = tLane[i] * tyRight[i] + (1 - tLane[i]) * tyLeft[i];
}                                                                               

/////////////////////////////////////////////////////////////////////////////
// Set segment info
/////////////////////////////////////////////////////////////////////////////
void LRaceLine::SetSegmentInfo(const tTrackSeg *pseg, double d, int i, double l)
{
 if (pseg)
 {
  tSegDist[pseg->id] = d;
  tSegIndex[pseg->id] = i;
  tElemLength[pseg->id] = l;
  if (pseg->id >= Segs)
   Segs = pseg->id + 1;
 }
}

/////////////////////////////////////////////////////////////////////////////
// Split the track into small elements
// ??? constant width supposed
/////////////////////////////////////////////////////////////////////////////
void LRaceLine::SplitTrack(tTrack *ptrack, int rl)
{
 Segs = 0;
 DivLength = 3;
 const tTrackSeg *psegCurrent = ptrack->seg;

 double Distance = 0;
 double Angle = psegCurrent->angle[TR_ZS];
 double xPos = (psegCurrent->vertex[TR_SL].x +
                psegCurrent->vertex[TR_SR].x) / 2;
 double yPos = (psegCurrent->vertex[TR_SL].y +
                psegCurrent->vertex[TR_SR].y) / 2;

 int i = 0;

 do
 {
  int Divisions = 1 + int(psegCurrent->length / DivLength);
  double Step = psegCurrent->length / Divisions;

  SetSegmentInfo(psegCurrent, Distance + Step, i, Step);
#if 0
  SetSegmentInfo(psegCurrent->lalt, Distance + Step, i, Step);
  SetSegmentInfo(psegCurrent->ralt, Distance + Step, i, Step);
  SetSegmentInfo(psegCurrent->lside, Distance + Step, i, Step);
  SetSegmentInfo(psegCurrent->rside, Distance + Step, i, Step);
#endif

  for (int j = Divisions; --j >= 0;)
  {
   double cosine = cos(Angle);
   double sine = sin(Angle);
   
   if (psegCurrent->type == TR_STR)
   {
    xPos += cosine * Step;
    yPos += sine * Step;
   }
   else
   {
    double r = psegCurrent->radius;
    double Theta = psegCurrent->arc / Divisions;
    double L = 2 * r * sin(Theta / 2);
    double x = L * cos(Theta / 2);
    double y;
    if (psegCurrent->type == TR_LFT)
    {
     Angle += Theta;
     y = L * sin(Theta / 2);
    }
    else
    {
     Angle -= Theta;
     y = -L * sin(Theta / 2);
    }
    xPos += x * cosine - y * sine;
    yPos += x * sine + y * cosine;
   }

   double dx = -psegCurrent->width * sin(Angle) / 2;
   double dy = psegCurrent->width * cos(Angle) / 2;
   txLeft[i] = xPos + dx;
   tyLeft[i] = yPos + dy;
   txRight[i] = xPos - dx;
   tyRight[i] = yPos - dy;
   tLane[i] = 0.5;
   tFriction[i] = psegCurrent->surface->kFriction;
   if (tFriction[i] < 1) // ??? ugly trick for dirt
   {
    //tFriction[i] *= 0.90;
    fDirt = 1;
    SideDistInt = -1.5;
    SideDistExt = 0.0;
   }
   UpdateTxTy(i, rl);

   Distance += Step;
   tDistance[i] = Distance;
   i++;
  }

  psegCurrent = psegCurrent->next;
 }
 while (psegCurrent != ptrack->seg);

 Divs = i - 1;
 Width = psegCurrent->width;
 Length = Distance;

}
 
/////////////////////////////////////////////////////////////////////////////
// Compute the inverse of the radius
/////////////////////////////////////////////////////////////////////////////
double LRaceLine::GetRInverse(int prev, double x, double y, int next, int rl)
{
 double x1 = tx[rl][next] - x;
 double y1 = ty[rl][next] - y;
 double x2 = tx[rl][prev] - x;
 double y2 = ty[rl][prev] - y;
 double x3 = tx[rl][next] - tx[rl][prev];
 double y3 = ty[rl][next] - ty[rl][prev];
 
 double det = x1 * y2 - x2 * y1;
 double n1 = x1 * x1 + y1 * y1;
 double n2 = x2 * x2 + y2 * y2;
 double n3 = x3 * x3 + y3 * y3;
 double nnn = sqrt(n1 * n2 * n3);
 
 return 2 * det / nnn;
}

/////////////////////////////////////////////////////////////////////////////
// Change lane value to reach a given radius
/////////////////////////////////////////////////////////////////////////////
void LRaceLine::AdjustRadius(int prev, int i, int next, double TargetRInverse, int rl, double Security)
{
 double OldLane = tLane[i];
 
 //
 // Start by aligning points for a reasonable initial lane
 //
 tLane[i] = (-(ty[rl][next] - ty[rl][prev]) * (txLeft[i] - tx[rl][prev]) +
              (tx[rl][next] - tx[rl][prev]) * (tyLeft[i] - ty[rl][prev])) /
            ( (ty[rl][next] - ty[rl][prev]) * (txRight[i] - txLeft[i]) -
              (tx[rl][next] - tx[rl][prev]) * (tyRight[i] - tyLeft[i]));
 if (tLane[i] < -0.2)
  tLane[i] = -0.2;
 else if (tLane[i] > 1.2)
  tLane[i] = 1.2;
 UpdateTxTy(i, rl);
 
 //
 // Newton-like resolution method
 //
 const double dLane = 0.0001;
 
 double dx = dLane * (txRight[i] - txLeft[i]);
 double dy = dLane * (tyRight[i] - tyLeft[i]);
 
 double dRInverse = GetRInverse(prev, tx[rl][i] + dx, ty[rl][i] + dy, next, rl);
 
 if (dRInverse > 0.000000001)
 {
  tLane[i] += (dLane / dRInverse) * TargetRInverse;
 
  double ExtLane = (ExtMargin + Security) / Width;
  double IntLane = ((IntMargin) + Security) / Width;
  if (ExtLane > 0.5)
   ExtLane = 0.5;
  if (IntLane > 0.5)
   IntLane = 0.5;

  if (TargetRInverse >= 0.0)
  {
   if (tLane[i] < IntLane)
    tLane[i] = IntLane;
   if (1 - tLane[i] < ExtLane)
   {
    if (1 - OldLane < ExtLane)
     tLane[i] = Min(OldLane, tLane[i]);
    else
     tLane[i] = 1 - ExtLane;
   }
  }
  else
  {
   if (tLane[i] < ExtLane)
   {
    if (OldLane < ExtLane)
     tLane[i] = Max(OldLane, tLane[i]);
    else
     tLane[i] = ExtLane;
   }
   if (1 - tLane[i] < IntLane)
    tLane[i] = 1 - IntLane;
  }
 }
 
 UpdateTxTy(i, rl);
}

/////////////////////////////////////////////////////////////////////////////
// Smooth path
/////////////////////////////////////////////////////////////////////////////
void LRaceLine::Smooth(int Step, int rl)
{
 int prev = ((Divs - Step) / Step) * Step;
 int prevprev = prev - Step;
 int next = Step;
 int nextnext = next + Step;
 
 for (int i = 0; i <= Divs - Step; i += Step)
 {
  double ri0 = GetRInverse(prevprev, tx[rl][prev], ty[rl][prev], i, rl);
  double ri1 = GetRInverse(i, tx[rl][next], ty[rl][next], nextnext, rl);
  double lPrev = Mag(tx[rl][i] - tx[rl][prev], ty[rl][i] - ty[rl][prev]);
  double lNext = Mag(tx[rl][i] - tx[rl][next], ty[rl][i] - ty[rl][next]);

  double TargetRInverse = (lNext * ri0 + lPrev * ri1) / (lNext + lPrev);
 
  double Security = lPrev * lNext / (8 * SecurityR);

  if (rl == LINE_RL)
  {
   if (ri0 * ri1 > 0)
   {
    double ac1 = fabs(ri0);
    double ac2 = fabs(ri1);
    {
     if (ac1 < ac2)
      ri0 += 0.12 * (ri1 - ri0);
     else if (ac2 < ac1)
      ri1 += 0.12 * (ri0 - ri1);
    }
 
    TargetRInverse = (lNext * ri0 + lPrev * ri1) / (lNext + lPrev);
   }
	}

  AdjustRadius(prev, i, next, TargetRInverse, rl, Security);
 
  prevprev = prev;
  prev = i;
  next = nextnext;
  nextnext = next + Step;
  if (nextnext > Divs - Step)
   nextnext = 0;
 }
}

/////////////////////////////////////////////////////////////////////////////
// Interpolate between two control points
/////////////////////////////////////////////////////////////////////////////
void LRaceLine::StepInterpolate(int iMin, int iMax, int Step, int rl)
{
 int next = (iMax + Step) % Divs;
 if (next > Divs - Step)
  next = 0;
 
 int prev = (((Divs + iMin - Step) % Divs) / Step) * Step;
 if (prev > Divs - Step)
  prev -= Step;
 
 double ir0 = GetRInverse(prev, tx[rl][iMin], ty[rl][iMin], iMax % Divs, rl);
 double ir1 = GetRInverse(iMin, tx[rl][iMax % Divs], ty[rl][iMax % Divs], next, rl);
 for (int k = iMax; --k > iMin;)
 {
  double x = double(k - iMin) / double(iMax - iMin);
  double TargetRInverse = x * ir1 + (1 - x) * ir0;
  AdjustRadius(iMin, k, iMax % Divs, TargetRInverse, rl);
 }
}
 
/////////////////////////////////////////////////////////////////////////////
// Calls to StepInterpolate for the full path
/////////////////////////////////////////////////////////////////////////////
void LRaceLine::Interpolate(int Step, int rl)
{
 if (Step > 1)
 {
  int i;
  for (i = Step; i <= Divs - Step; i += Step)
   StepInterpolate(i - Step, i, Step, rl);
  StepInterpolate(i - Step, Divs, Step, rl);
 }
}

void LRaceLine::InitTrack(tTrack* track, tSituation *p)
{
 //
 // split track
 //

 for (int rl=LINE_MID; rl<=LINE_RL; rl++)
 {
  memset(tRInverse, 0, sizeof(tRInverse));
  memset(tLane, 0, sizeof(tLane));

  SplitTrack(track, rl);
  //
  // Smoothing loop
  //
  int Iter = (rl == LINE_MID ? Iterations/4 : Iterations);
  for (int Step = 128; (Step /= 2) > 0;)
  {
   for (int i = Iter * int(sqrt((float) Step)); --i >= 0;)
    Smooth(Step, rl);
   Interpolate(Step, rl);
  }
 
  //
  // Compute curvature and speed along the path
  //
  for (int i = Divs; --i >= 0;)
  {
   double TireAccel = CornerSpeed * tFriction[i];
   if (rl == LINE_MID)
    TireAccel += AvoidSpeedAdjust;
   int next = (i + 1) % Divs;
   int prev = (i - 1 + Divs) % Divs;
 
   double rInverse = GetRInverse(prev, tx[rl][i], ty[rl][i], next, rl);
   tRInverse[i] = rInverse;
 
   double MaxSpeed;

   if (fabs(rInverse) > MinCornerInverse * 1.01)
    MaxSpeed = sqrt(TireAccel / (fabs(rInverse) - MinCornerInverse));
   else
    MaxSpeed = 10000;

   // TODO: increase or decrease speed depending on track camber

   // TODO: increase or decrease speed depending on track slope

   // TODO: increase or decrease speed depending on approaching bumps

   tSpeed[rl][i] = tMaxSpeed[i] = MaxSpeed;
  }
 
  //
  // Anticipate braking
  //
  for (int j = 32; --j >= 0;)
  for (int i = Divs; --i >= 0;)
  {
   double TireAccel = CornerSpeed * tFriction[i];
   int prev = (i - 1 + Divs) % Divs;
 
   double dx = tx[rl][i] - tx[rl][prev];
   double dy = ty[rl][i] - ty[rl][prev];
   double dist = Mag(dx, dy);
 
   double Speed = (tSpeed[rl][i] + tSpeed[rl][prev]) / 2;
 
   double LatA = tSpeed[rl][i] * tSpeed[rl][i] *
                 (fabs(tRInverse[prev]) + fabs(tRInverse[i])) / 2;
 
#if 0
   double TanA = TireAccel * TireAccel - LatA * LatA;
   if (TanA < 0.0)
    TanA = 0.0;
   TanA = sqrt(TanA) + MinCornerInverse * Speed * Speed;
   if (TanA > BrakeDelay)
    TanA = BrakeDelay;
#else
   double TanA = TireAccel * TireAccel +
                 MinCornerInverse * Speed * Speed - LatA * LatA;
   double brakedelay = BrakeDelay + (rl == LINE_MID ? AvoidSpeedAdjust : 0.0);
   if (TanA < 0.0)
    TanA = 0.0;
   if (TanA > brakedelay * tFriction[i])
    TanA = brakedelay * tFriction[i];
#endif
 
   double Time = dist / Speed;
   double MaxSpeed = tSpeed[rl][i] + TanA * Time;
   tSpeed[rl][prev] = Min(MaxSpeed, tMaxSpeed[prev]);
  }                                                                              
 }
} 

////////////////////////////////////////////////////////////////////////////
// New race
////////////////////////////////////////////////////////////////////////////
void LRaceLine::NewRace(tCarElt* newcar, tSituation *s)
{
 car = newcar;

 wheelbase = (car->priv.wheel[FRNT_RGT].relPos.x +
              car->priv.wheel[FRNT_LFT].relPos.x -
              car->priv.wheel[REAR_RGT].relPos.x -
              car->priv.wheel[REAR_LFT].relPos.x) / 2;
 wheeltrack = (car->priv.wheel[FRNT_LFT].relPos.y +
               car->priv.wheel[REAR_LFT].relPos.y -
               car->priv.wheel[FRNT_RGT].relPos.y -
               car->priv.wheel[REAR_RGT].relPos.y) / 2;
} 

////////////////////////////////////////////////////////////////////////////
// Car control
////////////////////////////////////////////////////////////////////////////
void LRaceLine::GetRaceLineData(tSituation *s, v2d *target, float *speed, float *avspeed, float *raceoffset, float *lookahead, float *racesteer)
{
 // 
 // Find index in data arrays
 //
 tTrackSeg *seg = car->_trkPos.seg;
 int SegId = car->_trkPos.seg->id;
 double dist = car->_trkPos.toStart;
 if (dist < 0)
  dist = 0;
 if (car->_trkPos.seg->type != TR_STR)
  dist *= car->_trkPos.seg->radius;
 int Index = tSegIndex[SegId] + int(dist / tElemLength[SegId]);
 This = Index;
 double d = tSegDist[SegId] + dist;

 Index = (Index + Divs - 5) % Divs;
 static const double Time = s->deltaTime*7; //0.50 + CornerAccel/80;
 double X4 = car->_pos_X + car->_speed_X * 0.5 / 2;
 double Y4 = car->_pos_Y + car->_speed_Y * 0.5 / 2;
 double X = car->_pos_X + car->_speed_X * Time / 2;
 double Y = car->_pos_Y + car->_speed_Y * Time / 2;
 *lookahead = 0.0f;
 while(1)
 {
  Next = (Index + 1) % Divs;
  double dx = tx[LINE_RL][Next] - car->_pos_X;
  double dy = ty[LINE_RL][Next] - car->_pos_Y;
  *lookahead = sqrt(dx*dx + dy*dy);
  if (*lookahead > 10.0f &&
      (tx[LINE_RL][Next] - tx[LINE_RL][Index]) * (X - tx[LINE_RL][Next]) +
      (ty[LINE_RL][Next] - ty[LINE_RL][Index]) * (Y - ty[LINE_RL][Next]) < 0.1)
   break;
  Index = Next;
 }

 if ((tRInverse[Next] > 0.0 && car->_trkPos.toMiddle < 0.0) ||
		 (tRInverse[Next] < 0.0 && car->_trkPos.toMiddle > 0.0))
  *lookahead *= MIN(4.0f, 1.5f + fabs(car->_trkPos.toMiddle*0.3));
 else
  *lookahead *= MAX(0.7f, 1.5f - fabs(car->_trkPos.toMiddle*0.2));

#if 0
  double dx = X4 - car->_pos_X;
  double dy = Y4 - car->_pos_Y;
  *lookahead = sqrt(dx*dx + dy*dy);
#endif
#if 1
 if ((tRInverse[Next] < 0.0 && car->_trkPos.toMiddle > 0.0) ||
     (tRInverse[Next] > 0.0 && car->_trkPos.toMiddle < 0.0))
 {
  *lookahead *= (float) MAX(1.0f, MIN(3.6f, 1.0f + (MIN(2.6f, fabs(car->_trkPos.toMiddle) / (seg->width/2)) / 2) * (1.0f + fabs(tRInverse[Next]) * 65.0f + car->_speed_x/120.0f)));
 }
 else if ((tRInverse[Next] < 0.0 && car->_trkPos.toRight < 5.0) ||
          (tRInverse[Next] > 0.0 && car->_trkPos.toLeft < 5.0))
 {
  *lookahead *= MAX(0.8f, MIN(1.0f, 1.0f - fabs(tRInverse[Next])*200.0 * ((5.0f-MIN(car->_trkPos.toRight, car->_trkPos.toLeft))/5.0f)));
 }
#endif

 target->x = (float) tx[LINE_RL][Next];
 target->y = (float) ty[LINE_RL][Next];

 //
 // Find target speed
 //
 double c0 = (tx[LINE_RL][Next] - tx[LINE_RL][Index]) * (tx[LINE_RL][Next] - X) +
             (ty[LINE_RL][Next] - ty[LINE_RL][Index]) * (ty[LINE_RL][Next] - Y);
 double c1 = (tx[LINE_RL][Next] - tx[LINE_RL][Index]) * (X - tx[LINE_RL][Index]) +
             (ty[LINE_RL][Next] - ty[LINE_RL][Index]) * (Y - ty[LINE_RL][Index]);
 {
  double sum = c0 + c1;
  c0 /= sum;
  c1 /= sum;
 }

 TargetSpeed = (1 - c0) * tSpeed[LINE_RL][Next] + c0 * tSpeed[LINE_RL][Index];
 *avspeed = (float) MAX(10.0f, tSpeed[LINE_MID][Next]);
 *speed = (float) MAX(*avspeed, TargetSpeed);

 if ((tRInverse[Next] > 0.0 && tLane[Next] > tLane[Index] && car->_trkPos.toLeft <= tLane[Next] * Width + 1.0) ||
     (tRInverse[Next] < 0.0 && tLane[Next] < tLane[Index] && car->_trkPos.toLeft >= tLane[Next] * Width - 1.0))
 {
  *avspeed = MAX(*speed, *avspeed);
 }
 else if ((tRInverse[Next] > 0.001 && tLane[Next] < tLane[Index] && car->_trkPos.toLeft < tLane[Next] * Width - 1.0) ||
          (tRInverse[Next] < -0.001 && tLane[Next] > tLane[Index] && car->_trkPos.toLeft > tLane[Next] * Width + 1.0))
 {
  *avspeed *= (float) MAX(0.7f, 1.0 - fabs(tRInverse[Next])*100);
 }


 double laneoffset = Width/2 - (tLane[Next] * Width);
 *raceoffset = (float) laneoffset;

 //
 // Find target curvature (for the inside wheel)
 //
 double TargetCurvature = (1 - c0) * tRInverse[Next] + c0 * tRInverse[Index];
 if (fabs(TargetCurvature) > 0.01)
 {
  double r = 1 / TargetCurvature;
  if (r > 0)
   r -= wheeltrack / 2;
  else
   r += wheeltrack / 2;
  TargetCurvature = 1 / r;
 }

 //
 // Steering control
 //
 double Error = 0;
 double VnError = 0;
 double carspeed = Mag(car->_speed_X, car->_speed_Y);
 
 //
 // Ideal value
 //
 double steer = atan(wheelbase * TargetCurvature) / car->_steerLock;

 //
 // Servo system to stay on the pre-computed path
 //
 {
  double dx = tx[LINE_RL][Next] - tx[LINE_RL][Index];
  double dy = ty[LINE_RL][Next] - ty[LINE_RL][Index];
  Error = (dx * (Y - ty[LINE_RL][Index]) - dy * (X - tx[LINE_RL][Index])) / Mag(dx, dy);
 }  

 int Prev = (Index + Divs - 1) % Divs;
 int PrevPrev = (Index + Divs - 5) % Divs;
 int NextNext = (Next + 1) % Divs;
 double Prevdx = tx[LINE_RL][Next] - tx[LINE_RL][Prev];
 double Prevdy = ty[LINE_RL][Next] - ty[LINE_RL][Prev];
 double Nextdx = tx[LINE_RL][NextNext] - tx[LINE_RL][Index];
 double Nextdy = ty[LINE_RL][NextNext] - ty[LINE_RL][Index];
 double dx = c0 * Prevdx + (1 - c0) * Nextdx;
 double dy = c0 * Prevdy + (1 - c0) * Nextdy;
 double n = Mag(dx, dy);
 dx /= n;
 dy /= n;
 double sError = (dx * car->_speed_Y - dy * car->_speed_X) / (carspeed + 0.01);
 double cError = (dx * car->_speed_X + dy * car->_speed_Y) / (carspeed + 0.01);
 VnError = asin(sError);
 if (cError < 0)
  VnError = PI - VnError;

 steer -= (atan(Error * (300 / (carspeed + 300)) / 15) + VnError) / car->_steerLock;

 //
 // Steer into the skid
 //
 double vx = car->_speed_X;
 double vy = car->_speed_Y;
 double dirx = cos(car->_yaw);
 double diry = sin(car->_yaw);
 double Skid = (dirx * vy - vx * diry) / (carspeed + 0.1);
 if (Skid > 0.9)
  Skid = 0.9;
 if (Skid < -0.9)
  Skid = -0.9;
 steer += (asin(Skid) / car->_steerLock) * 0.9;

 double yr = carspeed * TargetCurvature;
 double diff = car->_yaw_rate - yr;
 steer -= (0.08 * (100 / (carspeed + 100)) * diff) / car->_steerLock;

 {
  double trackangle = RtTrackSideTgAngleL(&(car->_trkPos));
  double angle = trackangle - car->_yaw;
  NORM_PI_PI(angle);
  angle = -angle;

  {
   if (fabs(angle) > 1.0)
   {
    if ((angle > 0.0 && steer > 0.0) || (angle < 0.0 && steer < 0.0))
     steer = -steer;
   }
   if (fabs(angle) > 1.6)
   {
    // we're facing the wrong way.  Set it to full steer in whatever direction for now ...
    if (steer > 0.0)
     steer = 1.0;
    else
     steer = -1.0;
   }
  }
 }

 *racesteer = steer;
} 

int LRaceLine::isOnLine()
{
 double lane2left = tLane[Next] * Width;
 
 if (fabs(car->_trkPos.toLeft - lane2left) < MAX(0.1, 1.0 - (car->_speed_x * (car->_speed_x/10))/600))
  return 1;

 return 0;
}

void LRaceLine::GetPoint( float offset, float lookahead, vec2f *rt )
{
 double lane = (Width/2 - offset) / Width;
 double length = 0.0;
 double la = (double) lookahead * MIN(1.0, MAX(0.8, car->_speed_x/TargetSpeed)); //0.8;
 vec2f last;
 last.x = (float) (lane * txRight[This] + (1 - lane) * txLeft[This]);
 last.y = (float) (lane * tyRight[This] + (1 - lane) * tyLeft[This]);
 //last.x = car->_pos_X;
 //last.y = car->_pos_Y;
 //int ndiv = (Next + 1 + int((lookahead)/DivLength)) % Divs;
 int ndiv = Next, count = 0;
 while (length < la && count < (int) (la/DivLength))
 {
  rt->x = (float) (lane * txRight[ndiv] + (1 - lane) * txLeft[ndiv]);
  rt->y = (float) (lane * tyRight[ndiv] + (1 - lane) * tyLeft[ndiv]);
  double dx = rt->x - last.x;
  double dy = rt->y - last.y;
  double thislength = sqrt(dx*dx + dy*dy);
	length += thislength;
  ndiv = (ndiv + 1) % Divs;
  count++;
  last.x = rt->x;
  last.y = rt->y;
 }
}

double LRaceLine::correctLimit()
{
 // this returns true if we're approaching a corner & are significantly
 // inside the ideal racing line.  The idea is to prevent a sudden outwards
 // movement at a time when we should be looking to turn in.
 
 double nlane2left = tLane[Next] * Width;
 if ((tRInverse[Next] > 0.001 && car->_trkPos.toLeft < nlane2left - 2.0) ||
     (tRInverse[Next] < -0.001 && car->_trkPos.toLeft > nlane2left + 2.0))
  return MAX(0.2, MIN(1.0, 1.0 - fabs(tRInverse[Next]) * 100.0));

 int nnext = (Next + (int) (car->_speed_x/3)) % Divs;
 double nnlane2left = tLane[nnext] * Width;
 if ((tRInverse[nnext] > 0.001 && car->_trkPos.toLeft < nnlane2left - 2.0) ||
     (tRInverse[nnext] < -0.001 && car->_trkPos.toLeft > nnlane2left + 2.0))
  return MAX(0.3, MIN(1.0, 1.0 - fabs(tRInverse[nnext]) * 40.0));

 // ok, we're not inside the racing line.  Check and see if we're outside it and turning 
 // into a corner, in which case we want to correct more to try and get closer to the
 // apex.
 if ((tRInverse[Next] > 0.001 && tLane[Next] <= tLane[This] && car->_trkPos.toLeft > nlane2left + 2.0) ||
     (tRInverse[Next] < -0.001 && tLane[Next] >= tLane[This] && car->_trkPos.toLeft < nlane2left - 2.0))
  return MAX(1.0, MIN(1.5, 1.0 + fabs(tRInverse[Next])));
 
 return 1.0;
}

double LRaceLine::getAvoidSpeed( float distance1, float distance2 )
{
 int i;
 double speed1 = 1000.0, speed2 = 1000.0, speed3 = 1000.0;

 int count = 0;
 for (i=Next; count < (int) (distance1/DivLength); i++)
 {
  count++;
  i = (i % Divs);
  speed1 = MIN(speed1, tSpeed[LINE_MID][i]);
 }

 count = 0;
 distance2 = (MIN(distance2, distance1 * 3) - distance1) / DivLength;
 for (; count < (int) distance2; i++)
 {
  count++;
  i = (i % Divs);
  speed2 = MIN(speed2, tSpeed[LINE_MID][i] + (double)count * 0.25);
 }

 return MIN(speed1, speed2);
} 
