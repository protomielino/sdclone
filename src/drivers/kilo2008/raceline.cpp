/*
 *      raceline.cpp
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

#include "raceline.h"
#include <robottools.h> //Rt*
#include <algorithm>    //for_each
#include "linalg.h"     //v2d
#include "util.h"       //Mag, sign, Between*
#include "driver.h"     //BT_SECT...

////////////////////////////////////////////////////////////////////////////
// Parameters
////////////////////////////////////////////////////////////////////////////

//
// These parameters are for the computation of the path
//
static const int Iterations = 100;      // Number of smoothing operations

/////////////////////////////////////////////////////////////////////////////
// Update tx and ty arrays
/////////////////////////////////////////////////////////////////////////////
void rlSegment::UpdateTxTy(const int rl)
{
  tx[rl] = tLane * txRight + (1 - tLane) * txLeft;
  ty[rl] = tLane * tyRight + (1 - tLane) * tyLeft;
}

static int g_rl;
void Nullify(rlSegment &d)
{
  d.tx[g_rl] = 0.0;
  d.ty[g_rl] = 0.0;
  d.tz[g_rl] = 0.0;
  d.tRInverse = 0.0;
  d.tSpeed[g_rl] = 0.0;
  d.tMaxSpeed = 0.0;
  d.txRight = 0.0;
  d.tyRight = 0.0;
  d.txLeft = 0.0;
  d.tyLeft = 0.0;
  d.tLane = 0.0;
  d.tLaneLMargin = 0.0;
  d.tLaneRMargin = 0.0;
}


/////////////////////////////////////////////////////////////////////////////
// Set segment info
/////////////////////////////////////////////////////////////////////////////
void
LRaceLine::SetSegmentInfo(const tTrackSeg * pseg, const int i, const double l)
{
  if(pseg)
    {
      std::pair<int, double> info(i, l);
      m_SegInfo.push_back(info);
    }
}

/////////////////////////////////////////////////////////////////////////////
// Split the track into small elements
// ??? constant width supposed
/////////////////////////////////////////////////////////////////////////////
#include <iostream>

void
LRaceLine::SplitTrack(const tTrack * const ptrack, const int rl, const tSituation *s)
{
  m_lDivLength = 3; // Length of path elements in meters
  const tTrackSeg *psegCurrent = ptrack->seg;

  double dAngle = psegCurrent->angle[TR_ZS];
  double dXPos  = (psegCurrent->vertex[TR_SL].x +
                psegCurrent->vertex[TR_SR].x) / 2;
  double dYPos  = (psegCurrent->vertex[TR_SL].y +
                psegCurrent->vertex[TR_SR].y) / 2;

  //Determine pit start and end
  const tTrackPitInfo *pPits = &ptrack->pits;
  double dpitstart = 0.0;
  double dpitend = 0.0;
  if(pPits->type != TR_PIT_NONE)
    {
      dpitstart = pPits->pitEntry->lgfromstart - 50.0;
      dpitend = pPits->pitExit->lgfromstart + pPits->pitExit->length + 50.0;
      if(dpitstart > dpitend)
        {
          if(psegCurrent->lgfromstart >= dpitstart)
            dpitend += ptrack->length;
          else
            dpitstart -= ptrack->length;
        }//if dpitstart > dpitend
    }//if pits.type


  m_SegInfo.reserve(ptrack->nseg);
  unsigned int i = 0;
  do
    {
      int Divisions = int(psegCurrent->length / m_lDivLength) + 1;
      double Step = psegCurrent->length / Divisions;

      SetSegmentInfo(psegCurrent, i, Step);

      //Take sides into account if they are useable
      double lft_margin = 0.0;
      double rgt_margin = 0.0;

      if(rl == LINE_RL)
        {
          for(int side = 0; side < 2; side++)
            {
              tTrackSeg *psegSide = psegCurrent->side[side];
              double dmargin = 0.0;

              while(psegSide)
                {
                  //Avoid walls and fences
                  if(psegSide->style == TR_WALL || psegSide->style == TR_FENCE)
                    {
                      dmargin = MAX(0.0, dmargin - (psegCurrent->type == TR_STR ? 0.5 : 1.0));
                    }

                  //Avoid slippery, rough or not plain side surfaces
                  if(/*psegSide->style != TR_PLAN
                    || */psegSide->surface->kFriction < psegCurrent->surface->kFriction * 0.8f
                    || psegSide->surface->kRoughness > MAX(0.02, psegCurrent->surface->kRoughness * 1.2)
                    || psegSide->surface->kRollRes > MAX(0.005, psegCurrent->surface->kRollRes * 1.2)
                    )
                    break;

                  //Do not use the side if it is the pitlane, stewards will get grumpy
                  if(pPits->type != TR_PIT_NONE)
                    {
                      //if we plan to use the side the pit is on, too
                      if(
                        ((side == TR_SIDE_LFT && pPits->side == TR_LFT)
                        || (side == TR_SIDE_RGT && pPits->side == TR_RGT))
                        &&
                        BetweenLoose(psegCurrent->lgfromstart, dpitstart, dpitend))
                          break;
                    }//if pits.type

#if 0
                  //kilo HACK Street-1
                  //No wandering in pit line during the race
                  if(1 && //s->_raceType == RM_TYPE_RACE &&
                    ((strcmp(psegCurrent->name, "curve 34") == 0)
                    || (strcmp(psegCurrent->name, "final straigth") == 0)
                    || (side == TR_SIDE_RGT && strcmp(psegCurrent->name, "start") == 0)
                    || (side == TR_SIDE_RGT && strcmp(psegCurrent->name, "pit end") == 0)
                    || (side == TR_SIDE_RGT && strcmp(psegCurrent->name, "pit exit") == 0)
                    )) {
                    break;
                  } //can enter pit lane on a qualy or practice
#endif
#if 0
                  else if((s->_raceType == RM_TYPE_QUALIF || s->_raceType == RM_TYPE_PRACTICE) &&
                    ((strcmp(psegCurrent->name, "curve 34") == 0)
                    || (strcmp(psegCurrent->name, "final straigth") == 0)
                    )) {
                    break;
                  }
#endif
                  //end kilo HACK Street-1
#if 0
                  //Aalborg HACK kilo
                  if(
                    (strcmp(psegCurrent->name, "140") == 0) //evil double left-hander
                    //|| (strcmp(psegCurrent->name, "171") == 0) //turn before back straight
                    || (strcmp(psegCurrent->name, "172") == 0)
                    || (strcmp(psegCurrent->name, "180") == 0)
                    //|| (strcmp(psegCurrent->name, "210") == 0) //turn ending back straigth
                    || (strcmp(psegCurrent->name, "220") == 0)
                    || (strcmp(psegCurrent->name, "230") == 0)
                    )
                    {
                      //fprintf(stderr, "%s %d\n", psegCurrent->name, i);
                      break;
                    }
                  //end HACK kilo

                  if((strcmp(psegCurrent->name, "turn22") == 0) //corner before long curve
                    || (strcmp(psegCurrent->name, "turn23") == 0)
                    || (strcmp(psegCurrent->name, "segment24") == 0)
                    || (strcmp(psegCurrent->name, "turn25") == 0)
                    )
                    {
                      break;
                    }
#endif

                  //Phew, we CAN use the side
                  //TODO: check not to leave the track completely if TRB rules change...
                  double dSideWidth = MIN(psegSide->startWidth, psegSide->endWidth);
                  if(psegSide->type == TR_STR)
                    if((side == TR_SIDE_LFT && (psegCurrent->type == TR_RGT || psegCurrent->next->type != TR_LFT))
                      || (side == TR_SIDE_RGT && (psegCurrent->type == TR_LFT || psegCurrent->next->type != TR_RGT))
                      )
                      dSideWidth *= 0.6;
                  dmargin += dSideWidth;
                  psegSide = psegSide->side[side];

                }//while psegSide

              dmargin = MAX(0.0, dmargin);
              if(dmargin > 0.0)
                {
                  dmargin /= psegCurrent->width;
                  if(side == TR_SIDE_LFT)
                    lft_margin += dmargin;
                  else
                    rgt_margin += dmargin;
                }//if dmargin
            }//for side
        }//if rl

      for(int j = Divisions; --j >= 0;)
        {
          double cosine = cos(dAngle);
          double sine = sin(dAngle);

          if(psegCurrent->type == TR_STR)
            {
              dXPos += cosine * Step;
              dYPos += sine * Step;
            }
          else
            {
              double r = psegCurrent->radius;
              double Theta = psegCurrent->arc / Divisions;
              double L = 2 * r * sin(Theta / 2);
              double x = L * cos(Theta / 2);
              double y;
              if(psegCurrent->type == TR_LFT)
                {
                  dAngle += Theta;
                  y = L * sin(Theta / 2);
                }
              else
                {
                  dAngle -= Theta;
                  y = -L * sin(Theta / 2);
                }
              dXPos += x * cosine - y * sine;
              dYPos += x * sine + y * cosine;
            }

          double dx = -psegCurrent->width * sin(dAngle) / 2;
          double dy = psegCurrent->width * cos(dAngle) / 2;

          if(m_Seg.size() <= i)//if it is the first run
            {   //Create & store new segment
              rlSegment *newSeg = new rlSegment;
              m_Seg.push_back(*newSeg);
              delete newSeg;
            }
            
          //We can be quite sure m_Seg[i] exists as we created it just above
          m_Seg[i].txLeft = dXPos + dx;
          m_Seg[i].tyLeft = dYPos + dy;
          m_Seg[i].txRight = dXPos - dx;
          m_Seg[i].tyRight = dYPos - dy;
          m_Seg[i].tLane = 0.5;
          m_Seg[i].tLaneLMargin = lft_margin;
          m_Seg[i].tLaneRMargin = rgt_margin;
          m_Seg[i].tFriction = psegCurrent->surface->kFriction;
          SetSegCamber(psegCurrent, i);
          //cerr << i << " " << psegCurrent->name << endl;

          // ??? ugly trick for dirt
          //if(m_Seg[i].tFriction < 1)
            //m_Seg[i].tFriction *= 0.90;

          m_Seg[i].UpdateTxTy(rl);

          i++;
        }//for j
      psegCurrent = psegCurrent->next;
    }//do
  while(psegCurrent != ptrack->seg);

  m_cDivs = i - 1; //m_Seg.size-1!
  m_dWidth = psegCurrent->width;
}

/////////////////////////////////////////////////////////////////////////////
// Compute the inverse of the radius
/////////////////////////////////////////////////////////////////////////////
double
LRaceLine::getRInverse(const int prev,
                    const double x,
                    const double y,
                    const int next,
                    const int rl) const
{
  //vector<rlSegment>::iterator itNext = m_Seg[next];
  double x1 = m_Seg[next].tx[rl] - x;
  double y1 = m_Seg[next].ty[rl] - y;
  double x2 = m_Seg[prev].tx[rl] - x;
  double y2 = m_Seg[prev].ty[rl] - y;
  double x3 = m_Seg[next].tx[rl] - m_Seg[prev].tx[rl];
  double y3 = m_Seg[next].ty[rl] - m_Seg[prev].ty[rl];

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
void
LRaceLine::AdjustRadius(int prev, int i, int next, double TargetRInverse,
            int rl, double Security)
{
  double OldLane = m_Seg[i].tLane;

  //
  // Start by aligning points for a reasonable initial lane
  //
  m_Seg[i].tLane = (-(m_Seg[next].ty[rl] - m_Seg[prev].ty[rl]) * (m_Seg[i].txLeft - m_Seg[prev].tx[rl]) +
          (m_Seg[next].tx[rl] - m_Seg[prev].tx[rl]) * (m_Seg[i].tyLeft - m_Seg[prev].ty[rl])) /
    ((m_Seg[next].ty[rl] - m_Seg[prev].ty[rl]) * (m_Seg[i].txRight - m_Seg[i].txLeft) -
     (m_Seg[next].tx[rl] - m_Seg[prev].tx[rl]) * (m_Seg[i].tyRight - m_Seg[i].tyLeft));

  if(rl == LINE_RL)
    {
      m_Seg[i].tLane = MAX(m_Seg[i].tLane, -1.2 - m_Seg[i].tLaneLMargin);
      m_Seg[i].tLane = MIN(m_Seg[i].tLane,  1.2 + m_Seg[i].tLaneRMargin);
    }//if rl
  m_Seg[i].UpdateTxTy(rl);

  //
  // Newton-like resolution method
  //
  const double dLane = 0.0001;

  double dx = dLane * (m_Seg[i].txRight - m_Seg[i].txLeft);
  double dy = dLane * (m_Seg[i].tyRight - m_Seg[i].tyLeft);

  double dRInverse =
    getRInverse(prev, m_Seg[i].tx[rl] + dx, m_Seg[i].ty[rl] + dy, next, rl);

  if(dRInverse > 0.000000001)
    {
      m_Seg[i].tLane += (dLane / dRInverse) * TargetRInverse;

      double ExtLane = MIN((m_dExtMargin + Security) / m_dWidth, 0.5);
      double IntLane = MIN((m_dIntMargin + Security) / m_dWidth, 0.5);
      //~ //kilo HACK E-track-1
      //~ if(BetweenLoose(i, 1077, 1112)) //bus-stop
        //~ {
          //~ //ExtLane = MIN((m_dExtMargin + 20.0) / m_dWidth, 0.5);
          //~ //IntLane = MIN((m_dIntMargin + 0.0) / m_dWidth, 0.5);
          //~ IntLane = 0.27;
        //~ }
      //~ //end kilo HACK
      //kilo HACK Alpine-2
      if(BetweenLoose(i, 1470, 1490)) //left-right bump after tunnel
        {
          //ExtLane = MIN((m_dExtMargin + 20.0) / m_dWidth, 0.5);
          //IntLane = MIN((m_dIntMargin + 0.0) / m_dWidth, 0.5);
          //ExtLane = 4.0;
          //IntLane = 0.3;
        }
      //end kilo HACK

      if(rl == LINE_RL)
        {
          if(TargetRInverse >= 0.0)
            {
              IntLane -= m_Seg[i].tLaneLMargin;
              ExtLane -= m_Seg[i].tLaneRMargin;
            }
          else
            {
              ExtLane -= m_Seg[i].tLaneLMargin;
              IntLane -= m_Seg[i].tLaneRMargin;
            }
        }//if rl

      if(TargetRInverse >= 0.0)
        {
          if(m_Seg[i].tLane < IntLane)
            m_Seg[i].tLane = IntLane;
          if(1 - m_Seg[i].tLane < ExtLane)
            {
              if(1 - OldLane < ExtLane)
                m_Seg[i].tLane = MIN(OldLane, m_Seg[i].tLane);
              else
                m_Seg[i].tLane = 1 - ExtLane;
            }
        }
      else
        {
          if(m_Seg[i].tLane < ExtLane)
            {
              if(OldLane < ExtLane)
                m_Seg[i].tLane = MAX(OldLane, m_Seg[i].tLane);
              else
                m_Seg[i].tLane = ExtLane;
            }
          if(1 - m_Seg[i].tLane < IntLane)
            m_Seg[i].tLane = 1 - IntLane;
        }
    }

  m_Seg[i].UpdateTxTy(rl);
}

/////////////////////////////////////////////////////////////////////////////
// Smooth path
/////////////////////////////////////////////////////////////////////////////
void
LRaceLine::Smooth(const int Step, const int rl)
{
  int prev = ((m_cDivs - Step) / Step) * Step;
  int prevprev = prev - Step;
  int next = Step;
  int nextnext = next + Step;

  for(int i = 0; i <= m_cDivs - Step; i += Step)
    {
      double ri0 = getRInverse(prevprev, m_Seg[prev].tx[rl], m_Seg[prev].ty[rl], i, rl);
      double ri1 = getRInverse(i, m_Seg[next].tx[rl], m_Seg[next].ty[rl], nextnext, rl);
      double lPrev = Mag(m_Seg[i].tx[rl] - m_Seg[prev].tx[rl], m_Seg[i].ty[rl] - m_Seg[prev].ty[rl]);
      double lNext = Mag(m_Seg[i].tx[rl] - m_Seg[next].tx[rl], m_Seg[i].ty[rl] - m_Seg[next].ty[rl]);

      double TargetRInverse = (lNext * ri0 + lPrev * ri1) / (lNext + lPrev);

      double Security = lPrev * lNext / (8 * m_dSecurityRadius);

      if(rl == LINE_RL)
        {
          if(ri0 * ri1 > 0)
            {
              double ac1 = abs(ri0);
              double ac2 = abs(ri1);

              if(ac1 < ac2) //curve increasing
                ri0 += 0.11 * (ri1 - ri0);
              else if(ac2 < ac1)    //curve decreasing
                ri1 += 0.11 * (ri0 - ri1);

              TargetRInverse = (lNext * ri0 + lPrev * ri1) / (lNext + lPrev);
            }
        }

      AdjustRadius(prev, i, next, TargetRInverse, rl, Security);

      prevprev = prev;
      prev = i;
      next = nextnext;
      nextnext = next + Step;
      if(nextnext > m_cDivs - Step)
        nextnext = 0;
    }
}

/////////////////////////////////////////////////////////////////////////////
// Interpolate between two control points
/////////////////////////////////////////////////////////////////////////////
void
LRaceLine::StepInterpolate(int iMin, int iMax, int Step, int rl)
{
  int next = (iMax + Step) % m_cDivs;
  if(next > m_cDivs - Step)
    next = 0;

  int prev = (((m_cDivs + iMin - Step) % m_cDivs) / Step) * Step;
  if(prev > m_cDivs - Step)
    prev -= Step;

  double ir0 = getRInverse(prev, m_Seg[iMin].tx[rl], m_Seg[iMin].ty[rl], iMax % m_cDivs, rl);
  double ir1 =
    getRInverse(iMin, m_Seg[iMax % m_cDivs].tx[rl], m_Seg[iMax % m_cDivs].ty[rl], next, rl);
  for(int k = iMax; --k > iMin;)
    {
      double x = double (k - iMin) / double (iMax - iMin);
      double TargetRInverse = x * ir1 + (1 - x) * ir0;
      AdjustRadius(iMin, k, iMax % m_cDivs, TargetRInverse, rl);
    }
}

/////////////////////////////////////////////////////////////////////////////
// Calls to StepInterpolate for the full path
/////////////////////////////////////////////////////////////////////////////
void
LRaceLine::Interpolate(int Step, int rl)
{
  if(Step > 1)
    {
      int i;
      for(i = Step; i <= m_cDivs - Step; i += Step)
        StepInterpolate(i - Step, i, Step, rl);
      StepInterpolate(i - Step, m_cDivs, Step, rl);
    }
}


void
LRaceLine::InitTrack(const tTrack * const track, void **carParmHandle, const tSituation *s)
{
  m_dMinCornerInverse =
    GfParmGetNum(*carParmHandle, BT_SECT_PRIV, "MinCornerInverse",
         (char *) NULL, 0.002);
  m_dCornerSpeed =
    GfParmGetNum(*carParmHandle, BT_SECT_PRIV, "CornerSpeed",
         (char *) NULL, 15.0);
  m_dAvoidSpeedAdjust =
    GfParmGetNum(*carParmHandle, BT_SECT_PRIV, "AvoidSpeedAdjust",
         (char *) NULL, 2.0);
  m_dCornerAccel =
    GfParmGetNum(*carParmHandle, BT_SECT_PRIV, "CornerAccel",
         (char *) NULL, 1.0);
  m_dIntMargin = GfParmGetNum(*carParmHandle, BT_SECT_PRIV, "IntMargin",
                  (char *) NULL, 1.0);
  m_dExtMargin = GfParmGetNum(*carParmHandle, BT_SECT_PRIV, "ExtMargin",
                  (char *) NULL, 2.0);
  m_dBrakeDelay = GfParmGetNum(*carParmHandle, BT_SECT_PRIV, "BrakeDelay",
                (char *) NULL, 10.0);
  m_dSecurityRadius = GfParmGetNum(*carParmHandle, BT_SECT_PRIV, BT_ATT_SECRADIUS,
                (char *) NULL, 100.0);

  // split track
  for(int rl = LINE_MID; rl <= LINE_RL; rl++)
    {
      g_rl = rl;
      std::for_each(m_Seg.begin(), m_Seg.end(), Nullify);

      SplitTrack(track, rl, s);

      // Smoothing loop
      int Iter = (rl == LINE_MID ? Iterations / 4 : Iterations);
      for(int Step = 128; (Step /= 2) > 0;)
        {
          for(int i = Iter * int (sqrt((double)Step)); --i >= 0;)
            Smooth(Step, rl);
          Interpolate(Step, rl);
        }

      // Compute curvature and speed along the path
      for(int i = m_cDivs; --i >= 0;)
        {
          double TireAccel = m_dCornerSpeed * m_Seg[i].tFriction;
          if(rl == LINE_MID)
            TireAccel += m_dAvoidSpeedAdjust;
          int next = (i + 1) % m_cDivs;
          int prev = (i - 1 + m_cDivs) % m_cDivs;

          double rInverse = getRInverse(prev, m_Seg[i].tx[rl], m_Seg[i].ty[rl], next, rl);
          m_Seg[i].tRInverse = rInverse;

          double MaxSpeed = 10000.0;
          double dAbsInverse = abs(rInverse);
          if(dAbsInverse > m_dMinCornerInverse * 1.01)
            MaxSpeed = sqrt(TireAccel / (dAbsInverse - m_dMinCornerInverse));

          //Increase or decrease speed depending on track camber
          if(dAbsInverse > 0.002)
            {
              double camber = m_Seg[i].dCamber;

              if(camber < -0.02)  //bad camber. slow us down.
                MaxSpeed -= MIN(MaxSpeed/4, abs(camber) * 20);
              else if(camber > 0.02)  //good camber, speed us up.
                MaxSpeed += camber * 10;
            }//if dAbsInverse

          // TODO: increase or decrease speed depending on track slope

          // TODO: increase or decrease speed depending on approaching bumps

          m_Seg[i].tSpeed[rl] = m_Seg[i].tMaxSpeed = MaxSpeed;
        }//for i

      // Anticipate braking
      for(int j = 32; --j >= 0;)
        for(int i = m_cDivs; --i >= 0;)
          {
            double TireAccel = m_dCornerSpeed * m_Seg[i].tFriction;
            int prev = (i - 1 + m_cDivs) % m_cDivs;

            double dx = m_Seg[i].tx[rl] - m_Seg[prev].tx[rl];
            double dy = m_Seg[i].ty[rl] - m_Seg[prev].ty[rl];

            double dist = Mag(dx, dy);
            double Speed = (m_Seg[i].tSpeed[rl] + m_Seg[prev].tSpeed[rl]) / 2;

            double LatA = m_Seg[i].tSpeed[rl] * m_Seg[i].tSpeed[rl] *
                (abs(m_Seg[prev].tRInverse) + abs(m_Seg[i].tRInverse)) / 2;

            double TanA = TireAccel * TireAccel +
                m_dMinCornerInverse * Speed * Speed - LatA * LatA;
            double brakedelay =
                m_dBrakeDelay + (rl == LINE_MID ? m_dAvoidSpeedAdjust : 0.0);
            TanA = MAX(TanA, 0.0);
            TanA = MIN(TanA, brakedelay * m_Seg[i].tFriction);

            double Time = dist / Speed;
            double MaxSpeed = m_Seg[i].tSpeed[rl] + TanA * Time;
            m_Seg[prev].tSpeed[rl] = MIN(MaxSpeed, m_Seg[prev].tMaxSpeed);
          }//for i
    }//for rl
   
}//InitTrack


////////////////////////////////////////////////////////////////////////////
// New race
////////////////////////////////////////////////////////////////////////////
void
LRaceLine::NewRace()
{
  const tPrivCar * const cp = &m_pCar->priv;
  m_dWheelBase = (cp->wheel[FRNT_RGT].relPos.x +
                cp->wheel[FRNT_LFT].relPos.x -
                cp->wheel[REAR_RGT].relPos.x -
                cp->wheel[REAR_LFT].relPos.x) / 2;

  m_dWheelTrack = (cp->wheel[FRNT_LFT].relPos.y +
                cp->wheel[REAR_LFT].relPos.y -
                cp->wheel[FRNT_RGT].relPos.y -
                cp->wheel[REAR_RGT].relPos.y) / 2;
}

////////////////////////////////////////////////////////////////////////////
// Car control
////////////////////////////////////////////////////////////////////////////
void
LRaceLine::GetRaceLineData(const tSituation * const s,
                v2d *target,
                double *speed,
                double *avspeed,
                double *raceoffset,
                double *lookahead,
                double *racesteer)
{
  // Find index in data arrays
  tTrackSeg *pSeg = m_pCar->_trkPos.seg;
  double dist = MAX(m_pCar->_trkPos.toStart, 0.0);
  if(pSeg->type != TR_STR)
    dist *= pSeg->radius;
  int Index = m_SegInfo[pSeg->id].first + int(dist / m_SegInfo[pSeg->id].second);
  This = Index;

  Index = (Index + m_cDivs - 5) % m_cDivs;
  static const double Time = s->deltaTime * 9 + m_dCornerAccel/80.0;  //0.50 + m_dCornerAccel/80;
  double X = m_pCar->_pos_X + m_pCar->_speed_X * Time / 2;
  double Y = m_pCar->_pos_Y + m_pCar->_speed_Y * Time / 2;
  // *lookahead = 0.0;

  while(1)
    {
      Next = (Index + 1) % m_cDivs;
      double dx = m_Seg[Next].tx[LINE_RL] - m_pCar->_pos_X;
      double dy = m_Seg[Next].ty[LINE_RL] - m_pCar->_pos_Y;
      *lookahead = Mag(dx, dy);
      if(*lookahead > 10.0 &&
        (m_Seg[Next].tx[LINE_RL] - m_Seg[Index].tx[LINE_RL]) * (X - m_Seg[Next].tx[LINE_RL]) +
        (m_Seg[Next].ty[LINE_RL] - m_Seg[Index].ty[LINE_RL]) * (Y - m_Seg[Next].ty[LINE_RL]) <
        0.1)
        break;
      Index = Next;
    }

  double toMiddle = m_pCar->_trkPos.toMiddle;
  if((m_Seg[Next].tRInverse > 0.0 && toMiddle < 0.0) ||
     (m_Seg[Next].tRInverse < 0.0 && toMiddle > 0.0))
    *lookahead *= MIN(4.0, 1.5 + abs(toMiddle * 0.3));
  else
    *lookahead *= MAX(0.7, 1.5 - abs(toMiddle * 0.2));

  if((m_Seg[Next].tRInverse < 0.0 && toMiddle > 0.0) ||
     (m_Seg[Next].tRInverse > 0.0 && toMiddle < 0.0))
    {
      *lookahead *= MAX(1.0, MIN(3.6, 1.0 +
                    (MIN(2.6, abs(toMiddle) / (pSeg->width / 2)) /
                    2) * (1.0 + abs(m_Seg[Next].tRInverse) * 65.0 +
                    m_pCar->_speed_x / 120.0)));
    }
  else if((m_Seg[Next].tRInverse < 0.0 && m_pCar->_trkPos.toRight < 5.0) ||
      (m_Seg[Next].tRInverse > 0.0 && m_pCar->_trkPos.toLeft < 5.0))
    {
      *lookahead *= MAX(0.8, MIN(1.0, 1.0 -
        abs(m_Seg[Next].tRInverse) * 200.0 * ((5.0 -
        MIN(m_pCar->_trkPos.toRight, m_pCar->_trkPos.toLeft)) / 5.0)));
    }

  target->x = m_Seg[Next].tx[LINE_RL];
  target->y = m_Seg[Next].ty[LINE_RL];

  //
  // Find target speed
  //
  double c0 =
    (m_Seg[Next].tx[LINE_RL] - m_Seg[Index].tx[LINE_RL]) * (m_Seg[Next].tx[LINE_RL] - X) +
    (m_Seg[Next].ty[LINE_RL] - m_Seg[Index].ty[LINE_RL]) * (m_Seg[Next].ty[LINE_RL] - Y);
  double c1 =
    (m_Seg[Next].tx[LINE_RL] - m_Seg[Index].tx[LINE_RL]) * (X - m_Seg[Index].tx[LINE_RL]) +
    (m_Seg[Next].ty[LINE_RL] - m_Seg[Index].ty[LINE_RL]) * (Y - m_Seg[Index].ty[LINE_RL]);

  double sum = c0 + c1;
  c0 /= sum;
  c1 /= sum;

  m_dTargetSpeed =
    (1 - c0) * m_Seg[Next].tSpeed[LINE_RL] + c0 * m_Seg[Index].tSpeed[LINE_RL];
  *avspeed = MAX(10.0, m_Seg[Next].tSpeed[LINE_MID]);
  *speed = MAX(*avspeed, m_dTargetSpeed);

  if((m_Seg[Next].tRInverse > 0.0 && m_Seg[Next].tLane > m_Seg[Index].tLane
      && m_pCar->_trkPos.toLeft <= m_Seg[Next].tLane * m_dWidth + 1.0)
     || (m_Seg[Next].tRInverse < 0.0 && m_Seg[Next].tLane < m_Seg[Index].tLane
     && m_pCar->_trkPos.toLeft >= m_Seg[Next].tLane * m_dWidth - 1.0))
    {
      *avspeed = MAX(*speed, *avspeed);
    }
  else
    if((m_Seg[Next].tRInverse > 0.001 && m_Seg[Next].tLane < m_Seg[Index].tLane
    && m_pCar->_trkPos.toLeft < m_Seg[Next].tLane * m_dWidth - 1.0)
       || (m_Seg[Next].tRInverse < -0.001 && m_Seg[Next].tLane > m_Seg[Index].tLane
       && m_pCar->_trkPos.toLeft > m_Seg[Next].tLane * m_dWidth + 1.0))
    {
      *avspeed *= MAX(0.7, 1.0 - abs(m_Seg[Next].tRInverse) * 100);
    }


  double laneoffset = m_dWidth / 2 - (m_Seg[Next].tLane * m_dWidth);
  *raceoffset = laneoffset;

  //
  // Find target curveture (for the inside wheel)
  //
  double TargetCurvature = (1 - c0) * m_Seg[Next].tRInverse + c0 * m_Seg[Index].tRInverse;
  if(abs(TargetCurvature) > 0.01)
    {
      double r = 1 / TargetCurvature;
      if(r > 0)
        r -= m_dWheelTrack / 2;
      else
        r += m_dWheelTrack / 2;
      TargetCurvature = 1 / r;
    }

  //
  // Steering control
  //
  double Error = 0;
  double VnError = 0;
  double carspeed = Mag(m_pCar->_speed_X, m_pCar->_speed_Y);

  //
  // Ideal value
  //
  double steer = atan(m_dWheelBase * TargetCurvature) / m_pCar->_steerLock;

  //
  // Servo system to stay on the pre-computed path
  //
    {
      double dx = m_Seg[Next].tx[LINE_RL] - m_Seg[Index].tx[LINE_RL];
      double dy = m_Seg[Next].ty[LINE_RL] - m_Seg[Index].ty[LINE_RL];
      Error =
        (dx * (Y - m_Seg[Index].ty[LINE_RL]) -
        dy * (X - m_Seg[Index].tx[LINE_RL])) / Mag(dx, dy);
    }

  int Prev = (Index + m_cDivs - 1) % m_cDivs;
  int NextNext = (Next + 1) % m_cDivs;
  double Prevdx = m_Seg[Next].tx[LINE_RL] - m_Seg[Prev].tx[LINE_RL];
  double Prevdy = m_Seg[Next].ty[LINE_RL] - m_Seg[Prev].ty[LINE_RL];
  double Nextdx = m_Seg[NextNext].tx[LINE_RL] - m_Seg[Index].tx[LINE_RL];
  double Nextdy = m_Seg[NextNext].ty[LINE_RL] - m_Seg[Index].ty[LINE_RL];
  double dx = c0 * Prevdx + (1 - c0) * Nextdx;
  double dy = c0 * Prevdy + (1 - c0) * Nextdy;
  double n = Mag(dx, dy);
  dx /= n;
  dy /= n;

  double sError =
    (dx * m_pCar->_speed_Y - dy * m_pCar->_speed_X) / (carspeed + 0.01);
  double cError =
    (dx * m_pCar->_speed_X + dy * m_pCar->_speed_Y) / (carspeed + 0.01);
  VnError = asin(sError);
  if(cError < 0)
    VnError = PI - VnError;

  steer -=
    (atan(Error * (300 / (carspeed + 300)) / 15) + VnError) / m_pCar->_steerLock;

  //
  // Steer into the skid
  //
  double vx = m_pCar->_speed_X;
  double vy = m_pCar->_speed_Y;
  double dirx = cos(m_pCar->_yaw);
  double diry = sin(m_pCar->_yaw);
  double Skid = (dirx * vy - vx * diry) / (carspeed + 0.1);
  if(Skid > 0.9)
    Skid = 0.9;
  if(Skid < -0.9)
    Skid = -0.9;
  steer += (asin(Skid) / m_pCar->_steerLock) * 0.9;

  double yr = carspeed * TargetCurvature;
  double diff = m_pCar->_yaw_rate - yr;
  steer -= (0.06 * (100 / (carspeed + 100)) * diff) / m_pCar->_steerLock;

  double trackangle = RtTrackSideTgAngleL(&(m_pCar->_trkPos));
  double angle = trackangle - m_pCar->_yaw;
  NORM_PI_PI(angle);
  angle = -angle;

  if(abs(angle) > 1.0)
    {
      if((angle > 0.0 && steer > 0.0) || (angle < 0.0 && steer < 0.0))
        steer = -steer;
    }

  //We're facing the wrong way.
  //Set it to full steer in whatever direction for now ...
  if(abs(angle) > 1.6)
    steer = sign(steer);

  *racesteer = steer;
}

bool
LRaceLine::isOnLine() const
{
  bool ret = false;

  double lane2left = m_Seg[Next].tLane * m_dWidth;
  if(abs(m_pCar->_trkPos.toLeft - lane2left) <
     MAX(0.1, 1.0 - (m_pCar->_speed_x * (m_pCar->_speed_x / 10)) / 600))
    ret = true;

  return ret;
}


void
LRaceLine::GetPoint(
                const double offset,
                const double lookahead,
                vec2f * const rt) const
{
  double dLane = (m_dWidth / 2.0 - offset) / m_dWidth;
  vec2f last;
  last.x = dLane * m_Seg[This].txRight + (1.0 - dLane) * m_Seg[This].txLeft;
  last.y = dLane * m_Seg[This].tyRight + (1.0 - dLane) * m_Seg[This].tyLeft;

  int ndiv = Next;
  double dLength = 0.0;
  double la = (double)lookahead
                * MIN(1.0, MAX(0.8, m_pCar->_speed_x / m_dTargetSpeed));
  int iLookaheadLimit = (int)(la / m_lDivLength);
  for(int count = 0;
        count < iLookaheadLimit && dLength < la;
        count++)
    {
      rt->x = dLane * m_Seg[ndiv].txRight + (1 - dLane) * m_Seg[ndiv].txLeft;
      rt->y = dLane * m_Seg[ndiv].tyRight + (1 - dLane) * m_Seg[ndiv].tyLeft;
      vec2f d = (*rt) - last;
      dLength += Mag(d.x, d.y);

      ndiv = (ndiv + 1) % m_cDivs;
      last = (*rt);
    }//for
}//GetPoint


double
LRaceLine::correctLimit(void) const
{
  // this returns true if we're approaching a corner & are significantly
  // inside the ideal racing line.  The idea is to prevent a sudden outwards
  // movement at a time when we should be looking to turn in.

  double toLeft = m_pCar->_trkPos.toLeft;
  double nlane2left = m_Seg[Next].tLane * m_dWidth;
  if((m_Seg[Next].tRInverse > 0.001 && toLeft < nlane2left - 2.0)
    || (m_Seg[Next].tRInverse < -0.001 && toLeft > nlane2left + 2.0))
    return MAX(0.2, MIN(1.0, 1.0 - abs(m_Seg[Next].tRInverse) * 100.0));

  int nnext = (Next + (int)(m_pCar->_speed_x / 3)) % m_cDivs;
  double nnlane2left = m_Seg[nnext].tLane * m_dWidth;
  if((m_Seg[nnext].tRInverse > 0.001 && toLeft < nnlane2left - 2.0)
    || (m_Seg[nnext].tRInverse < -0.001 && toLeft > nnlane2left + 2.0))
    return MAX(0.3, MIN(1.0, 1.0 - abs(m_Seg[nnext].tRInverse) * 40.0));

  // ok, we're not inside the racing line.  Check and see if we're outside it and turning
  // into a corner, in which case we want to correct more to try and get closer to the
  // apex.
  if((m_Seg[Next].tRInverse > 0.001
        && m_Seg[Next].tLane <= m_Seg[This].tLane
        && toLeft > nlane2left + 2.0)
      || (m_Seg[Next].tRInverse < -0.001
        && m_Seg[Next].tLane >= m_Seg[This].tLane
        && toLeft < nlane2left - 2.0))
    return MAX(1.0, MIN(1.5, 1.0 + abs(m_Seg[Next].tRInverse)));

  return 1.0;
}

double
LRaceLine::getAvoidSpeed(const double distance1, const double distance2) const
{
  double speed1 = 1000.0;
  double speed2 = 1000.0;

  int i = Next;
  int dist_limit = (int)(distance1 / m_lDivLength);
  for(int count = 0; count < dist_limit; count++, i++)
    {
      i = i % m_cDivs;
      speed1 = MIN(speed1, m_Seg[i].tSpeed[LINE_MID]);
    }

  dist_limit = (int)(MIN(distance2, distance1 * 3) - distance1) / m_lDivLength;
  for(int count = 0; count < dist_limit; count++, i++)
    {
      i = i % m_cDivs;
      speed2 = MIN(speed2, m_Seg[i].tSpeed[LINE_MID] + (double) count * 0.25);
    }

  return MIN(speed1, speed2);
}

void
LRaceLine::SetSegCamber(const tTrackSeg *seg, const int div)
{
  double dDistRatio = 0.7;
  double dCamberStart = seg->vertex[TR_SR].z - seg->vertex[TR_SL].z;
  double dCamberEnd = seg->vertex[TR_ER].z - seg->vertex[TR_EL].z;
  double dCamber = dCamberStart * dDistRatio + dCamberEnd * (1.0 - dDistRatio);
  
  dCamberStart /= seg->width;
  dCamberEnd /= seg->width;
  dCamber /= seg->width;
  
  if(m_Seg[div].tRInverse < 0.0)
    {
      dCamber       *= -1.0;
      dCamberStart  *= -1.0;
      dCamberEnd    *= -1.0;
    }
    
  if(dCamberEnd < dCamberStart)
    dCamber -= (dCamberStart - dCamberEnd) * 3.0;
  else if (dCamberEnd > dCamberStart)
    dCamber += (dCamberEnd - dCamberStart) * 0.4;

  m_Seg[div].dCamber = dCamber;
}//SetSegCamber

