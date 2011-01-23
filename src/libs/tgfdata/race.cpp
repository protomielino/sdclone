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

#include <robot.h>
#include <car.h>

#include "cars.h"
#include "drivers.h"
#include "race.h"


// Private data for GfRace				  
class GfRace::Private
{
public:

	Private() : hparmRace(0), nMaxCompetitors(0) {};
	
public:

	// The race params handle.
	void* hparmRace;

	// Max authorized number of competitors.
	unsigned nMaxCompetitors;
	
	// One GfDriver pointer for each competitor (order = race starting grid).
	std::vector<GfDriver*> vecCompetitors;

	// TODO: Is this usefull ?
	// Map for quick access to GfDriver by { module name, interface index }
	typedef std::map<std::pair<std::string, int>, GfDriver*> TMapCompetitorsByKey;
	TMapCompetitorsByKey mapCompetitorsByKey;
};

GfRace::GfRace(void* hparmRace)
{
	_pPrivate = new GfRace::Private;
	load(hparmRace);
}

void GfRace::clear()
{
	_pPrivate->hparmRace = 0;
	_pPrivate->nMaxCompetitors = 0;
	_pPrivate->mapCompetitorsByKey.clear();
	_pPrivate->vecCompetitors.clear();
}

void GfRace::load(void* hparmRace)
{
	clear();

	_pPrivate->hparmRace = hparmRace;

	if (!_pPrivate->hparmRace)
		return;

	// Load data from the race params.
	_pPrivate->vecCompetitors.clear();
	_pPrivate->mapCompetitorsByKey.clear();
		
    _pPrivate->nMaxCompetitors =
		(unsigned)GfParmGetNum(_pPrivate->hparmRace, RM_SECT_DRIVERS, RM_ATTR_MAXNUM, NULL, 0);
    const int nCompetitors = GfParmGetEltNb(_pPrivate->hparmRace, RM_SECT_DRIVERS);
	GfLogDebug("Competitors : n=%d (max=%d)\n", nCompetitors, _pPrivate->nMaxCompetitors);
    for (int nCompIndex = 1; nCompIndex < nCompetitors+1; nCompIndex++)
	{
		// Get driver infos from the the starting grid in the race params file
		std::ostringstream ossSectionPath;
		ossSectionPath << RM_SECT_DRIVERS << '/' << nCompIndex;
		const char* pszModName =
			GfParmGetStr(hparmRace, ossSectionPath.str().c_str(), RM_ATTR_MODULE, "");
		const int nItfIndex =
			(int)GfParmGetNum(hparmRace, ossSectionPath.str().c_str(), RM_ATTR_IDX, NULL, 0);

		GfLogDebug("Competitor #%d : %s#%d\n", nCompIndex, pszModName, nItfIndex);

		// Try and retrieve this driver among all the available drivers
		GfDriver* pCompetitor = GfDrivers::self()->getDriver(pszModName, nItfIndex);
		if (!pCompetitor)
		{
			GfLogWarning("Ignoring '%s' driver #%d : not found in available drivers\n",
						 pszModName, nItfIndex);
			continue;
		}
		
		// We've got it : if we can keep it for the race, make it a competitor
		// (there is a threshold on the number of competitors) :
		if (acceptsMoreCompetitors())
		{
			const char* pszSkinName =
				GfParmGetStr(hparmRace, ossSectionPath.str().c_str(), RM_ATTR_SKINNAME, "");
			const int nSkinTargets =
				(int)GfParmGetNum(hparmRace, ossSectionPath.str().c_str(), RM_ATTR_SKINTARGETS, NULL, 0);
			const bool bExtended =
				GfParmGetNum(hparmRace, ossSectionPath.str().c_str(), RM_ATTR_EXTENDED, NULL, 0)
				? true : false;

			GfLogDebug("  Name = %s, ext=%d\n",
					   pCompetitor->getName().c_str(), bExtended ? 1 : 0);

			// Get the chosen car for the race if any specified (human only).
			const GfCar* pCar = 0;
			if (pCompetitor->isHuman() && bExtended)
			{
				ossSectionPath.str("");
				ossSectionPath << RM_SECT_DRIVERINFO << '/' << pszModName
							   << '/' << (bExtended ? 1 : 0) << '/' << nItfIndex;
				const char* pszCarId =
					GfParmGetStr(hparmRace, ossSectionPath.str().c_str(), RM_ATTR_CARNAME, 0);
				GfLogDebug("  Extended && Human, %s : %s\n", ossSectionPath.str().c_str(), pszCarId);
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
}

void GfRace::save()
{
	// Clear the race starting grid.
    GfParmListClean(_pPrivate->hparmRace, RM_SECT_DRIVERS);

	// And then rebuild it from the current Competitors list state
	// (for each competitor, module name, interface index, car name if human,
	//  skin name and targets if needed).
	std::vector<GfDriver*>::const_iterator itComp;
	for (itComp = _pPrivate->vecCompetitors.begin();
		 itComp != _pPrivate->vecCompetitors.end(); itComp++)
	{
		std::ostringstream ossSectionPath;
		ossSectionPath << RM_SECT_DRIVERS
					   << '/' << (unsigned)(itComp - _pPrivate->vecCompetitors.begin() + 1);
		const std::string strDrvSec(ossSectionPath.str());
		
		GfParmSetNum(_pPrivate->hparmRace, strDrvSec.c_str(), RM_ATTR_IDX, (char*)NULL,
					 (tdble)(*itComp)->getInterfaceIndex());
		GfParmSetStr(_pPrivate->hparmRace, strDrvSec.c_str(), RM_ATTR_MODULE,
					 (*itComp)->getModuleName().c_str());
		
		const GfCar* pCar = (*itComp)->getCar();
		if (pCar && (*itComp)->isHuman())
		{
			/* Set extended */
			GfParmSetNum(_pPrivate->hparmRace, strDrvSec.c_str(), RM_ATTR_EXTENDED, NULL, 1);
			
			ossSectionPath.str("");
			ossSectionPath << RM_SECT_DRIVERINFO << '/' << (*itComp)->getModuleName()
						   << '/' << 1 /*extended*/ << '/' << (*itComp)->getInterfaceIndex();

			GfParmSetStr(_pPrivate->hparmRace, ossSectionPath.str().c_str(), RM_ATTR_CARNAME,
						 pCar->getId().c_str());

			// Save also the chosen car as the default one for this human driver
			// (may be needed later for races where it is not specified in <race>.xml)
			std::ostringstream ossFilePath;
			ossFilePath << GfLocalDir() << "drivers/" << (*itComp)->getModuleName()
						   << '/' << (*itComp)->getModuleName() << PARAMEXT;
			void* hparmRobot = GfParmReadFile(ossFilePath.str().c_str(), GFPARM_RMODE_STD);
			ossSectionPath.str("");
			ossSectionPath << ROB_SECT_ROBOTS << '/' << ROB_LIST_INDEX
						   << '/' << (*itComp)->getInterfaceIndex();
			GfParmSetStr(hparmRobot, ossSectionPath.str().c_str(), ROB_ATTR_CAR,
						 pCar->getId().c_str());
			GfParmWriteFile(NULL, hparmRobot, (*itComp)->getModuleName().c_str());
			GfParmReleaseHandle(hparmRobot);
		}
		else
		{
			/* Not extended for robots yet in driverconfig */
			GfParmSetNum(_pPrivate->hparmRace, strDrvSec.c_str(), RM_ATTR_EXTENDED, NULL, 0);
		}

		// Skin and skin targets.
		const GfDriverSkin& skin = (*itComp)->getSkin();
		GfParmSetNum(_pPrivate->hparmRace, strDrvSec.c_str(), RM_ATTR_SKINTARGETS, (char*)NULL,
					 (tdble)skin.getTargets());
		if ((!skin.getName().empty())
			|| GfParmGetStr(_pPrivate->hparmRace, strDrvSec.c_str(), RM_ATTR_SKINNAME, 0))
			GfParmSetStr(_pPrivate->hparmRace, strDrvSec.c_str(), RM_ATTR_SKINNAME,
						 skin.getName().c_str());
	}
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
	Private::TMapCompetitorsByKey::iterator itComp;
		_pPrivate->mapCompetitorsByKey.find(compKey);
	if (itComp != _pPrivate->mapCompetitorsByKey.end())
		return itComp->second;

	return 0;
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
