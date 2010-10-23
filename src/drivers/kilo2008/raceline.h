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

#ifndef SRC_DRIVERS_KILO2008_RACELINE_H_
#define SRC_DRIVERS_KILO2008_RACELINE_H_

#include <raceman.h>    // tSituation
#include <track.h>      // tTrack

#ifdef KILO_DEBUG
#include <iostream>     // ostream  // NOLINT(readability/streams) only debug
#endif
#include <vector>       // vector
#include <utility>      // pair

class v2d;

enum { LINE_MID = 0, LINE_RL };

class rlSegment {
 public:
  ~rlSegment() {}

  void UpdateTxTy(const int rl);
  friend void Nullify(rlSegment &d);  // NOLINT(runtime/references)
  // The above function is called in an STL for_each operation
#ifdef KILO_DEBUG
  friend std::ostream &operator<<(std::ostream &output, const rlSegment &s) {
    output << s.tx[0] << ' ' << s.tx[1] << ' '
        << s.ty[0] << ' ' << s.ty[1] << ' '
        << s.tz[0] << ' ' << s.tz[1] << ' '
        << s.tRInverse << ' ' << s.tMaxSpeed << ' '
        << s.tSpeed[0] << ' ' << s.tSpeed[1] << ' '
        << s.txLeft << ' ' << s.tyLeft << ' '
        << s.txRight << ' ' << s.tyRight << ' '
        << s.tLane << ' ' << s.tLaneLMargin << ' ' << s.tLaneRMargin << ' '
        << s.tFriction << ' ' << s.dCamber << std::endl;

    return output;
  }
#endif

 public:
  double tx[2];
  double ty[2];
  double tz[2];
  double tRInverse;
  double tMaxSpeed;
  double tSpeed[2];
  double txLeft;
  double tyLeft;
  double txRight;
  double tyRight;
  double tLane;
  double tLaneLMargin;
  double tLaneRMargin;
  double tFriction;
  double dCamber;
};


class LRaceLine {
 public:
  LRaceLine() {}
  virtual ~LRaceLine() {}

  inline void setCar(tCarElt * const car) {m_pCar = car;}

  void InitTrack(const tTrack * const track, void **carParmHandle,
                  const tSituation *s, const double filterSideSkill);
  void NewRace();
  void GetRaceLineData(const tSituation * const s, v2d * target,
                double *speed, double *avspeed, double *raceoffset,
                double *lookahead, double *racesteer);
  void GetPoint(const double offset, const double lookahead,
                  vec2f * const rt) const;
  bool isOnLine(void) const;
  double correctLimit(void) const;
  inline double getRInverse(void) const {return m_Seg[Next].tRInverse;}
  inline double getRInverse(const double distance) const {
    int d = ((Next + static_cast<int>(distance / m_lDivLength)) % m_cDivs);
    return m_Seg[d].tRInverse;
  }

 private:
  void SetSegmentInfo(const tTrackSeg * pseg, const int i, const double l);
  void SplitTrack(const tTrack * const ptrack, const int rl,
                      const tSituation *s);
  double getRInverse(const int prev, const double x, const double y,
                      const int next, const int rl) const;
  void AdjustRadius(int prev, int i, int next, double TargetRInverse, int rl,
            double Security = 0);
  void Smooth(const int Step, const int rl);
  void StepInterpolate(int iMin, int iMax, int Step, int rl);
  void Interpolate(int Step, int rl);
  double getAvoidSpeed(const double distance1, const double distance2) const;
  void SetSegCamber(const tTrackSeg *seg, const int div);

 private:
  double m_dMinCornerInverse;
  double m_dCornerSpeed;
  double m_dCornerAccel;
  double m_dBrakeDelay;
  double m_dIntMargin;
  double m_dExtMargin;
  double m_dAvoidSpeedAdjust;
  double m_dSecurityRadius;
  tCarElt * m_pCar;

  double m_dWheelBase;
  double m_dWheelTrack;

  int m_cDivs;
  int m_lDivLength;
  double m_dTargetSpeed;
  double m_dWidth;

  // first: segment index, second: elem length
  std::vector< std::pair<int, double> > m_SegInfo;

  std::vector<rlSegment> m_Seg;

  int Next;
  int This;
};

#endif  // SRC_DRIVERS_KILO2008_RACELINE_H_
