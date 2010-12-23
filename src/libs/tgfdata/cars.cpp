/***************************************************************************

    file                 : cars.cpp
    created              : July 2009
    copyright            : (C) 2009 Brian Gavin, 2010 Jean-Philippe Meuret
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

#include <map>
#include <sstream>
#include <algorithm>

#include <tgf.h>

#include <car.h>

#include "cars.h"


class GfCars::Private
{
public:
	
	// One GfCar structure for each car (order = sorted directory one).
	std::vector<GfCar*> vecCars;

	// Map for quick access to GfCar by id
	std::map<std::string, GfCar*> mapCarsById;

	// Vector of category Ids.
	std::vector<std::string> vecCatIds;
	
	// Vector of category names.
	std::vector<std::string> vecCatNames;
};


GfCars* GfCars::_pSelf = 0;

GfCars *GfCars::self()
{
	if (!_pSelf)
		_pSelf = new GfCars;
	
	return _pSelf;
}

GfCars::~GfCars()
{
	std::vector<GfCar*>::const_iterator itCar;
	for (itCar = _pPrivate->vecCars.begin(); itCar != _pPrivate->vecCars.end(); itCar++)
		delete *itCar;
}

GfCars::GfCars()
{
	_pPrivate = new Private;

	// Get the list of sub-dirs in the "cars" folder.
	tFList* lstFolders = GfDirGetList("cars");
	if (!lstFolders)
	{
		GfLogFatal("No car available in the 'cars' folder\n");
		return;
	}
	
	std::string strLastCatId("none");
	std::string strCatName;
	tFList* pFolder = lstFolders;
	do 
	{
		//GfLogDebug("GfCars::GfCars() : Examining %s\n", pFolder->name);
		
		// Ignore "." and ".." folders.
		if (pFolder->name[0] == '.') 
			continue;
			
		// Open the XML file of the car.
		const char* pszCarId = pFolder->name;
			
		std::ostringstream ossCarFileName;
		ossCarFileName << "cars/" << pszCarId << '/' << pszCarId << PARAMEXT;
		void* hparmCar = GfParmReadFile(ossCarFileName.str().c_str(), GFPARM_RMODE_STD);
		if (!hparmCar)
		{
			GfLogWarning("Ignoring car %s (file %s not %s)\n",
						 pszCarId, ossCarFileName.str().c_str(),
						 GfFileExists(ossCarFileName.str().c_str()) ? "readable" : "found");
			continue;
		}

		// Read car info.
		const std::string strCatId = GfParmGetStr(hparmCar, SECT_CAR, PRM_CATEGORY, "");
		if (strCatId != strLastCatId)
		{
			// Little optimization : don't load category file if same as the previous car's.
			std::ostringstream ossCatFileName;
			ossCatFileName << "categories/" << strCatId << PARAMEXT;
			void* hparmCat = GfParmReadFile(ossCatFileName.str().c_str(), GFPARM_RMODE_STD);
			if (!hparmCat)
			{
				GfLogWarning("Ignoring car %s (category file %s not %s)\n",
							 pszCarId, ossCatFileName.str().c_str(),
							 GfFileExists(ossCatFileName.str().c_str()) ? "readable" : "found");
				GfParmReleaseHandle(hparmCar);
				continue;
			}
			strLastCatId = strCatId;
			strCatName = GfParmGetName(hparmCat);
			GfParmReleaseHandle(hparmCat);
		}

		// Store it in the GfCar structure.
		GfCar* pCar = new GfCar;
		pCar->setId(pszCarId);
		pCar->setCategoryId(strCatId);
		pCar->setName(GfParmGetName(hparmCar));
		pCar->setDescriptorFileName(ossCarFileName.str());
		pCar->setCategoryName(strCatName);

		// Update the GfCars singleton.
		_pPrivate->vecCars.push_back(pCar);
		_pPrivate->mapCarsById[pszCarId] = pCar;
		if (std::find(_pPrivate->vecCatIds.begin(), _pPrivate->vecCatIds.end(), strCatId)
			== _pPrivate->vecCatIds.end())
		{
			_pPrivate->vecCatIds.push_back(strCatId);
			_pPrivate->vecCatNames.push_back(strCatName);
		}

		// Close the XML file of the car and category.
		GfParmReleaseHandle(hparmCar);
	} 
	while ((pFolder = pFolder->next) != lstFolders);
	
	GfDirFreeList(lstFolders, NULL, true, true);
	
	// Trace what we got.
	print();
}

const std::vector<std::string>& GfCars::getCategoryIds() const
{
	return _pPrivate->vecCatIds;
}

const std::vector<std::string>& GfCars::getCategoryNames() const
{
	return _pPrivate->vecCatNames;
}

GfCar* GfCars::getCar(const std::string& strId) const
{
	std::map<std::string, GfCar*>::iterator itCar =
		_pPrivate->mapCarsById.find(strId);
	if (itCar != _pPrivate->mapCarsById.end())
		return itCar->second;
	
	return 0;
}

GfCar* GfCars::getCarWithName(const std::string& strName) const
{
	std::vector<GfCar*>::iterator itCar;
	for (itCar = _pPrivate->vecCars.begin(); itCar != _pPrivate->vecCars.end(); itCar++)
		if ((*itCar)->getName() == strName)
			return *itCar;

	return 0;
}

std::vector<GfCar*> GfCars::getCarsInCategory(const std::string& strCatId) const
{
	std::vector<GfCar*> vecCarsInCat;

	std::vector<GfCar*>::iterator itCar;
	for (itCar = _pPrivate->vecCars.begin(); itCar != _pPrivate->vecCars.end(); itCar++)
		if (strCatId.empty() || (*itCar)->getCategoryId() == strCatId)
			vecCarsInCat.push_back(*itCar);

	return vecCarsInCat;
}

std::vector<GfCar*> GfCars::getCarsInCategoryWithName(const std::string& strCatName) const
{
	std::vector<GfCar*> vecCarsInCat;

	std::vector<GfCar*>::iterator itCar;
	for (itCar = _pPrivate->vecCars.begin(); itCar != _pPrivate->vecCars.end(); itCar++)
		if (strCatName.empty() || (*itCar)->getCategoryName() == strCatName)
			vecCarsInCat.push_back(*itCar);

	return vecCarsInCat;
}

std::vector<std::string> GfCars::getCarIdsInCategory(const std::string& strCatId) const
{
	std::vector<std::string> vecCarIds;

	std::vector<GfCar*>::const_iterator itCar;
	for (itCar = _pPrivate->vecCars.begin(); itCar != _pPrivate->vecCars.end(); itCar++)
		if (strCatId.empty() || (*itCar)->getCategoryId() == strCatId)
			vecCarIds.push_back((*itCar)->getId());

	return vecCarIds;
}

std::vector<std::string> GfCars::getCarNamesInCategory(const std::string& strCatId) const
{
	std::vector<std::string> vecCarNames;

	std::vector<GfCar*>::const_iterator itCar;
	for (itCar = _pPrivate->vecCars.begin(); itCar != _pPrivate->vecCars.end(); itCar++)
		if (strCatId.empty() || (*itCar)->getCategoryId() == strCatId)
			vecCarNames.push_back((*itCar)->getName());

	return vecCarNames;
}

void GfCars::print() const
{
	GfLogTrace("Car base : %d categories, %d cars\n",
			   _pPrivate->vecCatIds.size(), _pPrivate->vecCars.size());
	std::vector<std::string>::const_iterator itCatName;
	for (itCatName = _pPrivate->vecCatNames.begin();
		 itCatName != _pPrivate->vecCatNames.end(); itCatName++)
	{
		GfLogTrace("  '%s' category :\n", itCatName->c_str());
		const std::vector<GfCar*> vecCarsInCat = getCarsInCategoryWithName(*itCatName);
		std::vector<GfCar*>::const_iterator itCar;
		for (itCar = vecCarsInCat.begin(); itCar != vecCarsInCat.end(); itCar++)
			GfLogTrace("    %-22s: %s\n", (*itCar)->getName().c_str(),
					   (*itCar)->getDescriptorFileName().c_str());
	}
}

// GfCar class ------------------------------------------------------------------

const std::string& GfCar::getId() const
{
	return _strId;
}

const std::string& GfCar::getName() const
{
	return _strName;
}

const std::string& GfCar::getCategoryId() const
{
	return _strCatId;
}

const std::string& GfCar::getCategoryName() const
{
	return _strCatName;
}

const std::string& GfCar::getDescriptorFileName() const
{
	return _strDescFile;
}

	
void GfCar::setId(const std::string& strId)
{
	_strId = strId;
}

void GfCar::setName(const std::string& strName)
{
	_strName = strName;
}

void GfCar::setCategoryId(const std::string& strCatId)
{
	_strCatId = strCatId;
}

void GfCar::setCategoryName(const std::string& strCatName)
{
	_strCatName = strCatName;
}

void GfCar::setDescriptorFileName(const std::string& strDescFile)
{
	_strDescFile = strDescFile;
}
