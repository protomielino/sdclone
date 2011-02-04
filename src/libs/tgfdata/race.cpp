/***************************************************************************

    file                 : competitors.cpp
    created              : Sun Nov 21 19:00:00 CET 2010
    copyright            : (C) 2010 by Jean-Philippe MEURET
    web                  : speed-dreams.sourceforge.net
    version              : $Id$
                      
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <cstdlib>
#include <sstream>
#include <map>
#include <algorithm>

#include <raceman.h>
#include <robot.h>
#include <car.h>

#include "cars.h"
#include "drivers.h"
#include "tracks.h"
#include "racemanagers.h"
#include "race.h"


// Constants.
static const char *DispModeNames[GfRace::nDisplayModeNumber] =
	{ RM_VAL_VISIBLE, RM_VAL_INVISIBLE};
static const char *TimeOfDaySpecNames[GfRace::nTimeSpecNumber] = RM_VALS_TIME;
static const char* CloudsSpecNames[GfRace::nCloudsSpecNumber] = RM_VALS_CLOUDS;
static const char *RainSpecNames[GfRace::nRainSpecNumber] = RM_VALS_RAIN;

// Private data for GfRace				  
class GfRace::Private
{
public:

	Private() : pRaceMan(0), pParameters(0), nMaxCompetitors(0), nFocusedItfIndex(-1) {};
	
public:

	// The "parent" race manager.
	GfRaceManager* pRaceMan;

	// Session type (like Qualification 1/2/3 ... Race ...)
	std::string strSessionName;

	// Race parameters.
	Parameters* pParameters;
	
	// Max authorized number of competitors.
	unsigned nMaxCompetitors;
	
	// One GfDriver pointer for each competitor (order = race starting grid).
	std::vector<GfDriver*> vecCompetitors;

	// Map for quick access to GfDriver by { module name, interface index }
	typedef std::map<std::pair<std::string, int>, GfDriver*> TMapCompetitorsByKey;
	TMapCompetitorsByKey mapCompetitorsByKey;

	// Focused competitor (what for ?).
	std::string strFocusedModuleName;
	int nFocusedItfIndex;
};

GfRace::GfRace()
{
	_pPrivate = new GfRace::Private;
}

void GfRace::clear()
{
	_pPrivate->pRaceMan = 0;
	_pPrivate->nMaxCompetitors = 0;
	_pPrivate->mapCompetitorsByKey.clear();
	_pPrivate->vecCompetitors.clear();
}

void GfRace::load(GfRaceManager* pRaceMan)
{
	// Clear the race.
	clear();

	// Save the new race manager.
	_pPrivate->pRaceMan = pRaceMan;

	// Check if usable, and exit if not.
	if (!_pPrivate->pRaceMan)
		return;

	void* hparmRaceMan = pRaceMan->getDescriptorHandle();

	if (!hparmRaceMan)
		return;

	// Load race parameters (from the "race config" configuration, if any).
	// a) Search for the "race config" configuration.
	std::ostringstream ossConfSecPath;
	const char* pszSessionName = 0;
	const int nbConfs = GfParmGetEltNb(hparmRaceMan, RM_SECT_CONF);
	int nConfInd = 1;
	while (nConfInd <= nbConfs)
	{
		ossConfSecPath.str("");
		ossConfSecPath << RM_SECT_CONF << '/' << nConfInd;
		const char* pszConfType =
			GfParmGetStr(hparmRaceMan, ossConfSecPath.str().c_str(), RM_ATTR_TYPE, 0);
		if (pszConfType && !strcmp(pszConfType, RM_VAL_RACECONF))
		{
			pszSessionName =
				GfParmGetStr(hparmRaceMan, ossConfSecPath.str().c_str(), RM_ATTR_RACE, "Race");
			break;
		}
		nConfInd++;
	}

	// b) If found, load the race parameters from it.
	if (pszSessionName)
	{
		_pPrivate->pParameters = new Parameters;

		_pPrivate->strSessionName = pszSessionName;
		
		_pPrivate->pParameters->nLaps =
			(int)GfParmGetNum(hparmRaceMan, pszSessionName, RM_ATTR_LAPS, NULL, 25);
		_pPrivate->pParameters->nDistance =
			(int)GfParmGetNum(hparmRaceMan, pszSessionName, RM_ATTR_DISTANCE, "km", 0);
		_pPrivate->pParameters->nDuration =
			(int)GfParmGetNum(hparmRaceMan, pszSessionName, RM_ATTR_SESSIONTIME, "s", 0);

		_pPrivate->pParameters->eDisplayMode =
			strcmp(GfParmGetStr(hparmRaceMan, pszSessionName, RM_ATTR_DISPMODE, RM_VAL_VISIBLE),
				   RM_VAL_INVISIBLE) ? eDisplayNormal : eDisplayResultsOnly;

		const char* pszTimeOfDaySpec =
			GfParmGetStr(hparmRaceMan, pszSessionName, RM_ATTR_TIME_OF_DAY, RM_VAL_TIME_AFTERNOON);
		for (int i = 0; i < nTimeSpecNumber; i++)
			if (!strcmp(pszTimeOfDaySpec, TimeOfDaySpecNames[i]))
			{
				_pPrivate->pParameters->eTimeOfDaySpec = (ETimeOfDaySpec)i;
				break;
			}

		const char* pszCloudsSpec =
			GfParmGetStr(hparmRaceMan, pszSessionName, RM_ATTR_CLOUDS, RM_VAL_CLOUDS_NONE);
		for (int i = 0; i < nCloudsSpecNumber; i++)
			if (!strcmp(pszCloudsSpec, CloudsSpecNames[i]))
			{
				_pPrivate->pParameters->eCloudsSpec = (ECloudsSpec)i;
				break;
			}

		const char* pszRainSpec =
			GfParmGetStr(hparmRaceMan, pszSessionName, RM_ATTR_RAIN, RM_VAL_RAIN_NONE);
		for (int i = 0; i < nRainSpecNumber; i++)
			if (!strcmp(pszRainSpec, RainSpecNames[i]))
			{
				_pPrivate->pParameters->eRainSpec = (ERainSpec)i;
				break;
			}
	}
		
	// Load competitors data from the raceman params.
	_pPrivate->vecCompetitors.clear();
	_pPrivate->mapCompetitorsByKey.clear();
		
    _pPrivate->nMaxCompetitors =
		(unsigned)GfParmGetNum(hparmRaceMan, RM_SECT_DRIVERS, RM_ATTR_MAXNUM, NULL, 0);
    const int nCompetitors = GfParmGetEltNb(hparmRaceMan, RM_SECT_DRIVERS);
	std::ostringstream ossDrvSecPath;
    for (int nCompIndex = 1; nCompIndex < nCompetitors+1; nCompIndex++)
	{
		// Get driver infos from the the starting grid in the race params file.
		ossDrvSecPath.str("");
		ossDrvSecPath << RM_SECT_DRIVERS << '/' << nCompIndex;
		const char* pszModName =
			GfParmGetStr(hparmRaceMan, ossDrvSecPath.str().c_str(), RM_ATTR_MODULE, "");
		const int nItfIndex =
			(int)GfParmGetNum(hparmRaceMan, ossDrvSecPath.str().c_str(), RM_ATTR_IDX, NULL, 0);

		//GfLogDebug("Competitor #%d : %s#%d\n", nCompIndex, pszModName, nItfIndex);

		// Try and retrieve this driver among all the available drivers
		GfDriver* pCompetitor = GfDrivers::self()->getDriver(pszModName, nItfIndex);
		if (!pCompetitor)
		{
			GfLogWarning("Ignoring '%s' driver #%d : not found in available drivers\n",
						 pszModName, nItfIndex);
			continue;
		}

		// Check if this driver can compete in this race.
		if (pCompetitor->isNetwork() && !pRaceMan->isNetwork())
		{
			GfLogWarning("Ignoring '%s' driver #%d (%s) : Network humans can only race in network races\n",
						 pszModName, nItfIndex, pCompetitor->getName().c_str());
			continue;
		}
		
		// We've got it : if we can keep it for the race, make it a competitor
		// (there is a threshold on the number of competitors) :
		if (acceptsMoreCompetitors())
		{
			const char* pszSkinName =
				GfParmGetStr(hparmRaceMan, ossDrvSecPath.str().c_str(), RM_ATTR_SKINNAME, "");
			const int nSkinTargets =
				(int)GfParmGetNum(hparmRaceMan, ossDrvSecPath.str().c_str(), RM_ATTR_SKINTARGETS, NULL, 0);
			const bool bExtended =
				GfParmGetNum(hparmRaceMan, ossDrvSecPath.str().c_str(), RM_ATTR_EXTENDED, NULL, 0)
				? true : false;

			// Get the chosen car for the race if any specified (human only).
			const GfCar* pCar = 0;
			if (pCompetitor->isHuman() && bExtended)
			{
				ossDrvSecPath.str("");
				ossDrvSecPath << RM_SECT_DRIVERINFO << '/' << pszModName
							   << '/' << (bExtended ? 1 : 0) << '/' << nItfIndex;
				const char* pszCarId =
					GfParmGetStr(hparmRaceMan, ossDrvSecPath.str().c_str(), RM_ATTR_CARNAME, 0);
				pCar = GfCars::self()->getCar(pszCarId);
				if (!pCar)
					GfLogError("Falling back to default car '%s' "
							   "for %s because '%s' is not available\n",
							   pCompetitor->getCar()->getName().c_str(),
							   pCompetitor->getName().c_str(), pszCarId);
			}

			// Update the driver.
			GfDriverSkin skin(pszSkinName);
			skin.setTargets(nSkinTargets);
			pCompetitor->setSkin(skin);
			if (pCar) // Override default car.
				pCompetitor->setCar(pCar);
			
			// Update the GfRace.
			appendCompetitor(pCompetitor);
		}
		else
		{
			GfLogWarning("Ignoring subsequent competitors (max=%u)\n",
						 _pPrivate->nMaxCompetitors);
			break;
		}
	}
	
	// Load focused competitor data from the raceman params.
    _pPrivate->strFocusedModuleName =
		GfParmGetStr(hparmRaceMan, RM_SECT_DRIVERS, RM_ATTR_FOCUSED, "");
    _pPrivate->nFocusedItfIndex =
		(int)GfParmGetNum(hparmRaceMan, RM_SECT_DRIVERS, RM_ATTR_FOCUSEDIDX, NULL, 0);
}

void GfRace::save()
{
	if (!_pPrivate->pRaceMan)
		return;

	void* hparmRaceMan = _pPrivate->pRaceMan->getDescriptorHandle();

	if (!hparmRaceMan)
		return;

	// Save race manager level data.
	_pPrivate->pRaceMan->save();
	
	// Save race session and associated parameters.
	if (_pPrivate->pParameters)
	{
		const Parameters* pParams = _pPrivate->pParameters;
		const char* pszSessionName = _pPrivate->strSessionName.c_str();
		GfParmSetNum(hparmRaceMan, pszSessionName, RM_ATTR_LAPS,
					 (char*)NULL, (tdble)pParams->nLaps);
		GfParmSetNum(hparmRaceMan, pszSessionName, RM_ATTR_DISTANCE,
					 "km", (tdble)pParams->nDistance);
		GfParmSetNum(hparmRaceMan, pszSessionName, RM_ATTR_SESSIONTIME,
					 "s", (tdble)pParams->nDuration);
		GfParmSetStr(hparmRaceMan, pszSessionName, RM_ATTR_DISPMODE,
					 DispModeNames[pParams->eDisplayMode]);
		GfParmSetStr(hparmRaceMan, pszSessionName, RM_ATTR_TIME_OF_DAY,
					 TimeOfDaySpecNames[pParams->eTimeOfDaySpec]);
		GfParmSetStr(hparmRaceMan, pszSessionName, RM_ATTR_CLOUDS,
					 CloudsSpecNames[pParams->eCloudsSpec]);
		GfParmSetStr(hparmRaceMan, pszSessionName, RM_ATTR_RAIN,
					 RainSpecNames[pParams->eRainSpec]);
	}
	
	// Clear the race starting grid.
    GfParmListClean(hparmRaceMan, RM_SECT_DRIVERS);

	// And then rebuild it from the current Competitors list state
	// (for each competitor, module name, interface index, car name if human,
	//  skin name and targets if needed).
	std::vector<GfDriver*>::const_iterator itComp;
	for (itComp = _pPrivate->vecCompetitors.begin();
		 itComp != _pPrivate->vecCompetitors.end(); itComp++)
	{
		std::ostringstream ossDrvSecPath;
		ossDrvSecPath << RM_SECT_DRIVERS
					   << '/' << (unsigned)(itComp - _pPrivate->vecCompetitors.begin() + 1);
		const std::string strDrvSec(ossDrvSecPath.str());
		
		GfParmSetNum(hparmRaceMan, strDrvSec.c_str(), RM_ATTR_IDX, (char*)NULL,
					 (tdble)(*itComp)->getInterfaceIndex());
		GfParmSetStr(hparmRaceMan, strDrvSec.c_str(), RM_ATTR_MODULE,
					 (*itComp)->getModuleName().c_str());
		
		const GfCar* pCar = (*itComp)->getCar();
		if (pCar && (*itComp)->isHuman())
		{
			/* Set extended */
			GfParmSetNum(hparmRaceMan, strDrvSec.c_str(), RM_ATTR_EXTENDED, NULL, 1);
			
			ossDrvSecPath.str("");
			ossDrvSecPath << RM_SECT_DRIVERINFO << '/' << (*itComp)->getModuleName()
						   << '/' << 1 /*extended*/ << '/' << (*itComp)->getInterfaceIndex();

			GfParmSetStr(hparmRaceMan, ossDrvSecPath.str().c_str(), RM_ATTR_CARNAME,
						 pCar->getId().c_str());

			// Save also the chosen car as the default one for this human driver
			// (may be needed later for races where it is not specified in <race>.xml)
			std::ostringstream ossFilePath;
			ossFilePath << GetLocalDir() << "drivers/" << (*itComp)->getModuleName()
						   << '/' << (*itComp)->getModuleName() << PARAMEXT;
			void* hparmRobot = GfParmReadFile(ossFilePath.str().c_str(), GFPARM_RMODE_STD);
			ossDrvSecPath.str("");
			ossDrvSecPath << ROB_SECT_ROBOTS << '/' << ROB_LIST_INDEX
						   << '/' << (*itComp)->getInterfaceIndex();
			GfParmSetStr(hparmRobot, ossDrvSecPath.str().c_str(), ROB_ATTR_CAR,
						 pCar->getId().c_str());
			GfParmWriteFile(NULL, hparmRobot, (*itComp)->getModuleName().c_str());
			GfParmReleaseHandle(hparmRobot);
		}
		else
		{
			/* Not extended for robots yet in driverconfig */
			GfParmSetNum(hparmRaceMan, strDrvSec.c_str(), RM_ATTR_EXTENDED, NULL, 0);
		}

		// Skin and skin targets.
		const GfDriverSkin& skin = (*itComp)->getSkin();
		GfParmSetNum(hparmRaceMan, strDrvSec.c_str(), RM_ATTR_SKINTARGETS, NULL,
					 (tdble)skin.getTargets());
		if ((!skin.getName().empty())
			|| GfParmGetStr(hparmRaceMan, strDrvSec.c_str(), RM_ATTR_SKINNAME, 0))
			GfParmSetStr(hparmRaceMan, strDrvSec.c_str(), RM_ATTR_SKINNAME,
						 skin.getName().c_str());
	}
	
	// Save focused competitor data to the raceman params.
	const GfDriver* pFocComp = getFocusedCompetitor();
	GfParmSetStr(hparmRaceMan, RM_SECT_DRIVERS, RM_ATTR_FOCUSED,
				 _pPrivate->strFocusedModuleName.c_str());
	GfParmSetNum(hparmRaceMan, RM_SECT_DRIVERS, RM_ATTR_FOCUSEDIDX, NULL,
				 (tdble)_pPrivate->nFocusedItfIndex);
}

GfRaceManager* GfRace::getManager() const
{
	return _pPrivate->pRaceMan;
}

const std::string& GfRace::getSessionName() const
{
	return _pPrivate->strSessionName;
}

GfRace::Parameters* GfRace::getParameters()
{
	return _pPrivate->pParameters;
}

int GfRace::getSupportedFeatures() const
{
	int nFeatures = 0;

	std::vector<GfDriver*>::const_iterator itComp;
	for (itComp = _pPrivate->vecCompetitors.begin();
		 itComp != _pPrivate->vecCompetitors.end(); itComp++)
	{
		if (itComp == _pPrivate->vecCompetitors.begin())
			nFeatures = (*itComp)->getSupportedFeatures();
		else
			nFeatures &= (*itComp)->getSupportedFeatures();
	}
	
	return nFeatures;
}

const std::vector<GfDriver*>& GfRace::getCompetitors() const
{
	return _pPrivate->vecCompetitors;
}

unsigned GfRace::getCompetitorsCount() const
{
	return _pPrivate->vecCompetitors.size();
}

bool GfRace::acceptsMoreCompetitors() const
{
	return _pPrivate->vecCompetitors.size() < _pPrivate->nMaxCompetitors;
}

GfDriver* GfRace::getCompetitor(const std::string& strModName, int nItfIndex) const
{
	const std::pair<std::string, int> compKey(strModName, nItfIndex);
	Private::TMapCompetitorsByKey::iterator itComp =
		_pPrivate->mapCompetitorsByKey.find(compKey);
	if (itComp != _pPrivate->mapCompetitorsByKey.end())
		return itComp->second;

	return 0;
}

bool GfRace::isCompetitorFocused(const GfDriver* pComp) const
{
	return _pPrivate->strFocusedModuleName == pComp->getModuleName()
		   && _pPrivate->nFocusedItfIndex == pComp->getInterfaceIndex();
}

GfDriver* GfRace::getFocusedCompetitor() const
{
	return getCompetitor(_pPrivate->strFocusedModuleName, _pPrivate->nFocusedItfIndex);
}

void GfRace::setFocusedCompetitor(const GfDriver* pComp)
{
	_pPrivate->strFocusedModuleName = pComp ? pComp->getModuleName() : "";
	_pPrivate->nFocusedItfIndex = pComp ? pComp->getInterfaceIndex() : -1;
}

bool GfRace::appendCompetitor(GfDriver* pComp)
{
	const bool bAppended = acceptsMoreCompetitors();
	
	if (bAppended)
	{
		_pPrivate->vecCompetitors.push_back(pComp);
		const std::pair<std::string, int> compKey(pComp->getModuleName(),
												  pComp->getInterfaceIndex());
		_pPrivate->mapCompetitorsByKey[compKey] = pComp;
	}

	return bAppended;
}

bool GfRace::removeCompetitor(GfDriver* pComp)
{
	bool bRemoved = true;
	
	// Remove it from the vector.
	std::vector<GfDriver*>::iterator itVComp =
		std::find(_pPrivate->vecCompetitors.begin(), _pPrivate->vecCompetitors.end(), pComp);
	if (itVComp != _pPrivate->vecCompetitors.end())
		_pPrivate->vecCompetitors.erase(itVComp);
	else
		bRemoved = false;
	
	// Remove it from the map.
	const std::pair<std::string, int> compKey(pComp->getModuleName(), pComp->getInterfaceIndex());
	Private::TMapCompetitorsByKey::iterator itMComp =
		_pPrivate->mapCompetitorsByKey.find(compKey);
	if (itMComp != _pPrivate->mapCompetitorsByKey.end())
		_pPrivate->mapCompetitorsByKey.erase(itMComp);
	else
		bRemoved = false;

	return bRemoved;
}

bool GfRace::moveCompetitor(GfDriver* pComp, int nDeltaPlace)
{
	// Nothing to do if no real move.
	if (nDeltaPlace == 0)
		return false;

	// Neither if competitor not found in race.
	std::vector<GfDriver*>::iterator itComp =
		std::find(_pPrivate->vecCompetitors.begin(), _pPrivate->vecCompetitors.end(), pComp);
	if (itComp == _pPrivate->vecCompetitors.end())
		return false;

	// Remove the competitor from his place.
	_pPrivate->vecCompetitors.erase(itComp);
	
	// Determine his new place.
	const int nNewIndex = (itComp - _pPrivate->vecCompetitors.begin()) + nDeltaPlace;
	if (nNewIndex < 0)
		itComp = _pPrivate->vecCompetitors.begin();
	else if (nNewIndex >= (int)_pPrivate->vecCompetitors.size())
		itComp = _pPrivate->vecCompetitors.end();
	else
		itComp = _pPrivate->vecCompetitors.begin() + nNewIndex;

	// Insert it at his new place.
	_pPrivate->vecCompetitors.insert(itComp, pComp);

	return true;
}


bool GfRace::removeAllCompetitors()
{
	const bool bAnyRemoved = !_pPrivate->vecCompetitors.empty();

	_pPrivate->vecCompetitors.clear();

	return true;
}

bool GfRace::shuffleCompetitors()
{
	// Get the number of competitors ('cause nothing to do if less than 2).
	const unsigned nCompetitors = _pPrivate->vecCompetitors.size();
	if (nCompetitors < 2)
		return false; // Didn't change anything.

	// Make a copy of the competitors vector, and clear it.
	std::vector<GfDriver*> vecCompetitors = _pPrivate->vecCompetitors;
	_pPrivate->vecCompetitors.clear();

	// Pickup a random competitor from the old vector, and add it at the end o fthe new one.
	for (unsigned nCount = 1; nCount < nCompetitors; nCount++)
	{
		// Get a random competitor index in the remaining list.
		const unsigned nPickedCompInd = rand() % vecCompetitors.size();

		// Put this competitor at the end of the new list.
		_pPrivate->vecCompetitors.push_back(vecCompetitors[nPickedCompInd]);

		// Remove it from the old list.
		vecCompetitors.erase(vecCompetitors.begin() + nPickedCompInd);
	}

	// Put the last competitor at the end of the new list.
	_pPrivate->vecCompetitors.push_back(vecCompetitors[0]);
	
	return true;
}

GfTrack* GfRace::getTrack() const
{
	return _pPrivate->pRaceMan ? _pPrivate->pRaceMan->getCurrentEventTrack() : 0;
}

void GfRace::setTrack(GfTrack* pTrack)
{
	if (_pPrivate->pRaceMan && pTrack)
		_pPrivate->pRaceMan->setCurrentEventTrack(pTrack);
}

void GfRace::print() const
{
	// TODO.
	GfLogWarning("GfTrack::print() not yet implemented\n");
}
