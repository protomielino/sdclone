/*
 *      strategy.cpp
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

#include "strategy.h"

#include "driver.h" //BT_ATT, BT_SECT

//#define STRAT_DEBUG

// [kg/m] fuel consumption.
const float KStrategy::MAX_FUEL_PER_METER = 0.0006f; 
// [-] Repair needed if damage is beyond this value
const int   KStrategy::PIT_DAMAGE = 5000;   
// [laps] how many laps' fuel to leave in tank before deciding on refuel?
const float KStrategy::SAFE_LAPS = 2.0f;  
// [-] Store this count of laps' damages
const int   KStrategy::LAST_LAPS = 10; 


KStrategy::KStrategy()
{
  m_last_damages = new std::deque<int>;
    
  m_laps = 0;
  m_fuelchecked = false;
  m_fuelperlap = 0.0f;
  m_lastpitfuel = 0.0f;
  m_fuelsum = 0.0f;
}//KStrategy


/*
 * 
 * name: setFuelAtRaceStart
 * @param tTrack *t
 * @param void **carParmHandle
 * @param tSituation *s
 * @param int index
 * @return void
 */
void
KStrategy::setFuelAtRaceStart(const tTrack * const t,
                void ** const carParmHandle,
                const tSituation * const s,
                const int index)
{
  // Load and set parameters.
  float fuel =
    GfParmGetNum(*carParmHandle, BT_SECT_PRIV, BT_ATT_FUELPERLAP,
         (char *) NULL, t->length * MAX_FUEL_PER_METER);
  m_expectedfuelperlap = fuel;
  // Pittime is pittime without refuel.
  m_pittime =
    GfParmGetNum(*carParmHandle, BT_SECT_PRIV, BT_ATT_PITTIME, (char *) NULL,
         25.0f);
  m_bestlap =
    GfParmGetNum(*carParmHandle, BT_SECT_PRIV, BT_ATT_BESTLAP, (char *) NULL,
         87.0f);
  m_worstlap =
    GfParmGetNum(*carParmHandle, BT_SECT_PRIV, BT_ATT_WORSTLAP, (char *) NULL,
         87.0f);
  //Fuel tank capacity
  float maxfuel =
    GfParmGetNum(*carParmHandle, SECT_CAR, PRM_TANK, (char *) NULL, 100.0f);

  // Fuel for the whole race. A race needs one more lap - why???
  float fuelForRace;
  if(s->_raceType == RM_TYPE_RACE)
    fuelForRace = (s->_totLaps + 1.0f) * fuel;
  else
    fuelForRace = s->_totLaps * fuel;
  
  // Compute race times for min to min + 9 pit stops.
  computeBestNumberOfPits(maxfuel, fuelForRace, s->_totLaps, TRUE);
  m_lastfuel = m_fuelperstint;
  
  //If the setup defines initial fuel amount, use that value in races.
  //Otherwise use computed amount.
  float initial_fuel = //(fuelForRace < maxfuel ? fuelForRace : 
    GfParmGetNum(*carParmHandle, SECT_CAR, PRM_FUEL, (char *) NULL, 0.0f);
  if(s->_raceType == RM_TYPE_RACE && initial_fuel)
    GfParmSetNum(*carParmHandle, SECT_CAR, PRM_FUEL, (char *) NULL, initial_fuel);
  else
    // Add fuel dependent on index to avoid fuel stop in the same lap.
    GfParmSetNum(*carParmHandle, SECT_CAR, PRM_FUEL, (char *) NULL,
           m_lastfuel + index * m_expectedfuelperlap);
}


/*
 * 
 * name: updateFuelStrategy
 * 
 * Computes the fuel amount required to finish the race.
 * If there is not enough fuel in the tank, re-runs pit computations.
 * 
 * @param: -
 * @return void
 */
void
KStrategy::updateFuelStrategy()
{
  // Required additional fuel for the rest of the race.
  // +1 because the computation happens right after crossing the start line.
  float fuelperlap = (m_fuelperlap == 0.0f ? 2.5f : m_fuelperlap); //average 
  float required_fuel = ((laps_to_go() + 1) - ceil(m_car->_fuel / fuelperlap))
                            * fuelperlap;
  
  // We don't have enough fuel to end the race, need at least one stop.
  if(required_fuel >= 0.0f)
    {                       
      // Compute race times for different pit strategies.
      computeBestNumberOfPits(m_car->_tank, required_fuel, laps_to_go(), FALSE);
    }
}//updateFuelStrategy


/*
 * 
 * name: pitRefuel
 * @param: -
 * @return float, amount of fuel requested from race engine.
 */
float
KStrategy::pitRefuel()
{
  updateFuelStrategy();
  
  float fuel;
  if(m_remainingstops > 1)
    {
      fuel = MIN(MAX(m_fuelperstint, 40.0), m_car->_tank - m_car->_fuel); //!!!
      m_remainingstops--;
    }
  else
    {
      float cmpfuel =
        (m_fuelperlap == 0.0f) ? m_expectedfuelperlap : m_fuelperlap;
      fuel = MAX(MIN((laps_to_go() + 1.0f) * cmpfuel - m_car->_fuel,
                m_car->_tank - m_car->_fuel), 0.0f);
    }

  m_lastpitfuel = fuel;
  return fuel;
}//pitRefuel


/*
 * Based on fuel tank capacity, remaining laps and current fuel situtation,
 * decides the optimum number of pits to complete the race the fastest way.
 * Ie: it may be advantegous to pit more and refill less each time.
 * param preRace = is it the pre-race, first time calculation?
 */
void
KStrategy::computeBestNumberOfPits(
                const float tankCapacity,
                const float requiredFuel,
                const int remainingLaps,
                const bool preRace)
{
  int pitstopMin = int (ceil(requiredFuel / tankCapacity));
  float mintime = FLT_MAX;
  int beststops = pitstopMin;
  for(int i = 0; i < (preRace ? 5 : 4); i++)
    {
      float stintFuel = requiredFuel / (pitstopMin + i + 1);
      float fillratio = stintFuel / tankCapacity;
      float avglapest = m_bestlap + (m_worstlap - m_bestlap) * fillratio;
      float racetime = (pitstopMin + i) * (m_pittime + stintFuel / 8.0f) +
                remainingLaps * avglapest;
      if(mintime > racetime)
        {
          mintime = racetime;
          beststops = pitstopMin + i - (preRace ? 1 : 0);
          m_fuelperstint = stintFuel;
        }
    }
  m_remainingstops = beststops;
}//computeBestNumberOfPits


/*
 * 
 * name: update
 * 
 * Stores last 5 laps' damage values.
 * Updates best & worst lap times.
 * Then resumes fuel statistics updates.
 *
 * @param: -
 * @return: -
 */
void
KStrategy::update()
{
  if(m_car->_laps > m_laps)   //If a new lap has been finished
    {
      m_laps = m_car->_laps;
      m_last_damages->push_front(m_car->_dammage); //store last lap's damage
      if((int)m_last_damages->size() > LAST_LAPS) //and keep deque size at limit
        m_last_damages->pop_back();

#ifdef STRAT_DEBUG
      //print damage values in reverse order
      cerr << car->_name << ": damages";
      for(deque<int>::reverse_iterator rit = m_last_damages->rbegin();
            rit < m_last_damages->rend();
            rit++)
        cerr << " " << *rit;
      cerr << endl;
#endif
    }

  // Update best & worst lap times
  m_bestlap = MIN((m_bestlap == 0.0f ? m_car->_lastLapTime : m_bestlap), m_car->_lastLapTime);
  m_worstlap = MAX(m_worstlap, m_car->_lastLapTime);

  
  // Fuel statistics update.
  int id = m_car->_trkPos.seg->id;
  // Range must include enough segments to be executed once guaranteed.
  if(id >= 0 && id < 5 && !m_fuelchecked)
    {
      if(m_car->race.laps > 1)
        {
          m_fuelsum += (m_lastfuel + m_lastpitfuel - m_car->priv.fuel);
          m_fuelperlap = (m_fuelsum / (m_car->race.laps - 1));
          // This is here for adding strategy decisions, otherwise it could be moved to pitRefuel
          // for efficiency.
          updateFuelStrategy();
        }
      m_lastfuel = m_car->priv.fuel;
      m_lastpitfuel = 0.0f;
      m_fuelchecked = true;
    }
  else if(id > 5)
    {
      m_fuelchecked = false;
    }
}//update


/*
 * 
 * name: needPitstop
 * 
 * Checks if we need to pit.
 * We need a pit stop if fuel is low,
 * or if damage is over the limit.
 * In the last 5 laps we want to pit only if the predicted damage value
 * would go beyond the dreaded 10k value.
 *
 * @param: -
 * @return: bool, True if we need to visit the pit
 */
bool
KStrategy::needPitstop() const
{
  // Question makes only sense if there is a pit.
  if(m_car->_pit != NULL)
    {
      //Ideally we shouldn't pit on the last lap...
      //just get to the finish line somehow.
      if(laps_to_go() > 0)
        {
          // Do we need to refuel?
          float cmpfuel = (m_fuelperlap == 0.0f) ? m_expectedfuelperlap : m_fuelperlap;
          #ifdef STRAT_DEBUG
          if(strcmp(car->_name, "Kilo 2")==0 && car->_fuel < 5.0)
            cerr << car->_name
                << " laps_to_go:" << laps_to_go(car)
                << " dam_avg:" << get_avg_damage()
                << " fuel:" << car->_fuel
                << " cmpfuel:" << cmpfuel
                << " 1.5:" << bool(car->_fuel < 1.5 * cmpfuel)
                << " cf<ltg(c)*cmp:" bool(car->_fuel < laps_to_go(car) * cmpfuel)
                << endl;
          #endif
          if((m_car->_fuel < SAFE_LAPS * cmpfuel) && (m_car->_fuel < laps_to_go() * cmpfuel))
            {
              #ifdef STRAT_DEBUG
              cerr << car->_name << " REFUEL" << endl;
              #endif
              return true;
            }

          // Do we need to repair and is the pit free?
          if(m_car->_dammage > PIT_DAMAGE)
            {
              //Let's see if we can make it somehow onto the finish line
              //BEWARE doesnt check for limits, works for races > 5 laps!!!
              if(laps_to_go() <= LAST_LAPS)
                {
                  //If prediction shows we would top the damage limit, let's visit the pit
                  if(m_car->_dammage + get_avg_damage() * laps_to_go() >= 10000)
                    return isPitFree();
                }
              else
                return isPitFree();
            }
        }
      #ifdef STRAT_DEBUG
      else cerr << car->_name << " LAP_TO_GO ZERO" << endl;
      #endif
    }
  #ifdef STRAT_DEBUG
  else cerr << car->_name << " PIT NULL" << endl;
  #endif
  return false;
}//needPitstop


/*
 * 
 * name: get_avg_damage
 * 
 * Computes last 5 laps' average damage increment
 *
 * @param: -
 * @return: int, average damage increment
 */
int
KStrategy::get_avg_damage(void) const
{
  return (m_last_damages->front() - m_last_damages->back())
    / MAX(m_last_damages->size(), 1);
}


/*
 * 
 * name: pitRepair
 * 
 * Tells TORCS how much damage we ask to be repaired.
 * On the last 5 laps, we push fast pitstops with minimal repairs.
 * Otherwise, let's repair all damage.
 *
 * @param: -
 * @return: int, how much damage to repair
 */
int
KStrategy::pitRepair() const
{
  int ret = laps_to_go() <= LAST_LAPS
    ? get_avg_damage() * laps_to_go()
    : m_car->_dammage;
    
#ifdef STRAT_DEBUG
  cerr << car->_name
    << " pitRepair:" << ret
    << endl;
#endif
  
  //Clear buffer
  //m_last_damages->assign(m_last_damages->size(), 0);
  m_last_damages->clear();
  
  return ret;
}//pitRepair


bool
KStrategy::isPitFree() const
{
  if(m_car->_pit != NULL && m_car->_pit->pitCarIndex == TR_PIT_STATE_FREE)
    return true;
  else
    return false;
}//isPitFree
