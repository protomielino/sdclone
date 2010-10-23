/*
 *      strategy.h
 *      
 *      Copyright 2009 kilo aka Gabor Kmetyko <kg.kilo@gmail.com>
 *      Based on work by Bernhard Wymann and Andrew Sumner.
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

/*
    Pit strategy for drivers.
*/

#ifndef SRC_DRIVERS_KILO2008_STRATEGY_H_
#define SRC_DRIVERS_KILO2008_STRATEGY_H_

#include <track.h>    // tTrack
#include <car.h>      // tCarElt
#include <raceman.h>  // tSituation

#include <deque>

class KStrategy {
 public:
  KStrategy();
  ~KStrategy() {delete m_last_damages;}

  // Interface
  void Update();
  bool NeedPitstop() const;
  int PitRepair() const;
  double PitRefuel();
  void SetFuelAtRaceStart(const tTrack * const t,
                            void ** const carParmHandle,
                            const tSituation * const s,
                            const int index);
  void set_car(const tCarElt * const car) {this->m_car = car;}

 protected:
  bool IsPitFree() const;
  int GetAvgDamage() const;
  inline int LapsToGo() const
      {return m_car->_remainingLaps - m_car->_lapsBehindLeader;}
  void UpdateFuelStrategy();
  void ComputeBestNumberOfPits(const double tankCapacity,
                                const double requiredFuel,
                                const int remainingLaps,
                                const bool preRace);

  const tCarElt * m_car;
  int m_laps;
  std::deque<int> *m_last_damages;
  int m_remaining_stops;
  double m_fuel_per_stint;
  double m_pittime;           // Expected additional time for pit stop.
  double m_best_lap;          // Best possible lap, empty tank and alone.
  double m_worst_lap;         // Worst possible lap, full tank and alone.
  bool m_fuel_checked;        // Fuel statistics updated.
  double m_fuel_per_lap;      // Maximum amount of fuel we needed for a lap.
  double m_last_pit_fuel;     // Amount refueled, special case when we refuel.
  double m_last_fuel;         // Fuel available when we cross the start lane.
  double m_expected_fuel_per_lap;  // Expected fuel per lap (may be inaccurate).
  double m_fuel_sum;          // All the fuel used.

  static const double MAX_FUEL_PER_METER;   // [kg/m] fuel consumtion.
  static const int PIT_DAMAGE;  // If damage > we request a pit stop.
  static const double SAFE_LAPS;   // Can go this # of laps before req. refuel.
  static const int LAST_LAPS;   // Store this count of last laps' damage datae
};

#endif  // SRC_DRIVERS_KILO2008_STRATEGY_H_
