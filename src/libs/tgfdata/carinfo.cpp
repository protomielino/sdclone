/***************************************************************************

    file                 : carinfo.cpp
    created              : July 2009
    copyright            : (C) 2009 Brian Gavin
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

#include "carinfo.h"


struct PrivateData
{
	// One CarData structure for each car (order = sorted directory one).
	std::vector<CarData> vecCars;

	// Map for quick access to CarData by name
	std::map<std::string, int> mapCarNameIndices;

	// Vector of category names.
	std::vector<std::string> vecCategoryNames;
	
	// Vector of category real names.
	std::vector<std::string> vecCategoryRealNames;
};


CarInfo* CarInfo::m_pSelf = 0;

CarInfo *CarInfo::self()
{
	if (!m_pSelf)
		m_pSelf = new CarInfo;
	
	return m_pSelf;
}

CarInfo::CarInfo()
{
	m_priv = new PrivateData;

	// Read in car info
	tFList	*files;
	tFList	*curFile;
	void	*carParmHdle;

	// Load the car list from the "cars" folder contents.
	curFile = files = GfDirGetList("cars");
	if (curFile) 
	{
		do 
		{
			//GfLogDebug("Examining car %s\n", curFile->name);
			
			// Ignore "." and ".." folders.
			if (curFile->name[0] == '.') 
				continue;
			
			// Open the XML file of the car.
			const char* pszCarName = curFile->name;
			
			std::ostringstream ossCarFileName;
			ossCarFileName << "cars/" << pszCarName << '/' << pszCarName << ".xml";
			carParmHdle = GfParmReadFile(ossCarFileName.str().c_str(), GFPARM_RMODE_STD);
			if (!carParmHdle) {
				GfLogWarning("CarInfo : Ignoring car %s (failed to read from %s)\n",
							 pszCarName, ossCarFileName.str().c_str());
				continue;
			}

			// Read car info and store it in the CarData structure (category real name = later).
			CarData carData;
			carData.strName = pszCarName;
			carData.strCategoryName = GfParmGetStr(carParmHdle, "Car", "category", "");
			carData.strRealName = GfParmGetName(carParmHdle);
			carData.strXMLPath = ossCarFileName.str();

			// Update the CarInfo singleton.
			m_priv->vecCars.push_back(carData);
			m_priv->mapCarNameIndices[pszCarName] = m_priv->vecCars.size() - 1;
			if (std::find(m_priv->vecCategoryNames.begin(),
						  m_priv->vecCategoryNames.end(), carData.strCategoryName)
				== m_priv->vecCategoryNames.end())
				m_priv->vecCategoryNames.push_back(carData.strCategoryName);

			// Close the XML file of the car.
			GfParmReleaseHandle(carParmHdle);
		} 
		while ((curFile = curFile->next) != files);
	}
	
	GfDirFreeList(files, NULL, true, true);

	// Sort car categories.
	std::sort(m_priv->vecCategoryNames.begin(), m_priv->vecCategoryNames.end());

	// Set car category real name (only here because m_priv->vecCategoryNames
	// has to be completed before we can GetCategoryRealNames().
	const std::vector<std::string> vecCatRealNames = GetCategoryRealNames();
	std::vector<std::string>::const_iterator iterCatName;
	for (unsigned nCatIndex = 0; nCatIndex < m_priv->vecCategoryNames.size(); nCatIndex++)
	{
		std::vector<CarData*> vecCarsInCat =
			GetCarsInCategory(m_priv->vecCategoryNames[nCatIndex]);
		std::vector<CarData*>::iterator iterCar;
		for (iterCar = vecCarsInCat.begin(); iterCar != vecCarsInCat.end(); iterCar++)
			(*iterCar)->strCategoryRealName = vecCatRealNames[nCatIndex];
	}

	print();
}

std::string CarInfo::GetCarRealName(const std::string& strCarName) const
{
	std::string strRealName;

	std::map<std::string, int>::const_iterator iterCar = m_priv->mapCarNameIndices.find(strCarName);
	if (iterCar != m_priv->mapCarNameIndices.end())
	{
		const CarData& car = m_priv->vecCars[iterCar->second];
		strRealName = car.strRealName;
	}

	return strRealName;
}

std::vector<std::string> CarInfo::GetCarNamesInCategory(const std::string& strCatName) const
{
	std::vector<std::string> vecCarNames;

	std::vector<CarData>::const_iterator iterCar;
	for (iterCar = m_priv->vecCars.begin(); iterCar != m_priv->vecCars.end(); iterCar++)
		if (strCatName == "All" || iterCar->strCategoryName == strCatName)
			vecCarNames.push_back(iterCar->strName);

	return vecCarNames;
}

std::vector<CarData*> CarInfo::GetCarsInCategory(const std::string& strCatName) const
{
	std::vector<CarData*> vecCarsInCat;

	std::vector<CarData>::iterator iterCar;
	for (iterCar = m_priv->vecCars.begin(); iterCar != m_priv->vecCars.end(); iterCar++)
		if (strCatName == "All" || iterCar->strCategoryName == strCatName)
			vecCarsInCat.push_back(&(*iterCar));

	return vecCarsInCat;
}

std::vector<CarData*> CarInfo::GetCarsInCategoryRealName(const std::string& strCatRealName) const
{
	std::vector<CarData*> vecCarsInCat;

	std::vector<CarData>::iterator iterCar;
	for (iterCar = m_priv->vecCars.begin(); iterCar != m_priv->vecCars.end(); iterCar++)
		if (strCatRealName == "All" || iterCar->strCategoryRealName == strCatRealName)
			vecCarsInCat.push_back(&(*iterCar));

	return vecCarsInCat;
}

const std::vector<std::string>& CarInfo::GetCategoryNames() const
{
	return m_priv->vecCategoryNames;
}

const std::vector<std::string>& CarInfo::GetCategoryRealNames() const
{
	if (m_priv->vecCategoryRealNames.empty())
	{
		std::vector<std::string>::const_iterator iterCatName;
		for (iterCatName = m_priv->vecCategoryNames.begin();
			 iterCatName != m_priv->vecCategoryNames.end(); iterCatName++)
		{
			std::ostringstream ossCatFileName;
			ossCatFileName << "categories/" << *iterCatName << PARAMEXT;
			const std::string strCatFileName = ossCatFileName.str();
			void* hCat = GfParmReadFile(strCatFileName.c_str(), GFPARM_RMODE_STD);
			if (hCat)
			{
				m_priv->vecCategoryRealNames.push_back(GfParmGetName(hCat));
				GfParmReleaseHandle(hCat);
			}
			else
			{
				m_priv->vecCategoryRealNames.push_back(*iterCatName); // Fall-back.
				GfLogWarning("Car category file %s not %s\n",
							 strCatFileName.c_str(),
							 GfFileExists(strCatFileName.c_str()) ? "readable" : "found");
			}
		}
	}

	return m_priv->vecCategoryRealNames;
}

std::vector<std::string> CarInfo::GetCarRealNamesInCategory(const std::string& strCatName) const
{
	std::vector<std::string> vecCarRealNames;

	std::vector<CarData>::const_iterator iterCar;
	for (iterCar = m_priv->vecCars.begin(); iterCar != m_priv->vecCars.end(); iterCar++)
		if (strCatName == "All" || iterCar->strCategoryName == strCatName)
			vecCarRealNames.push_back(iterCar->strRealName);

	return vecCarRealNames;
}

CarData* CarInfo::GetCarDataFromRealName(const std::string& strRealCarName) const
{
	std::vector<CarData>::iterator iterCar;
	for (iterCar = m_priv->vecCars.begin(); iterCar != m_priv->vecCars.end(); iterCar++)
		if (iterCar->strRealName == strRealCarName)
			return &(*iterCar);

	return NULL;
}

CarData* CarInfo::GetCarData(const std::string& strCarName) const
{
	std::map<std::string, int>::const_iterator iterCar =
		m_priv->mapCarNameIndices.find(strCarName);
	if (iterCar != m_priv->mapCarNameIndices.end())
		return &m_priv->vecCars[iterCar->second];
	
	return NULL;
}

void CarInfo::print() const
{
	GfLogDebug("Found %d cars in %d categories\n",
			   m_priv->vecCars.size(), m_priv->vecCategoryNames.size());
	std::vector<std::string>::const_iterator iterCat;
	for (unsigned nCatIndex = 0; nCatIndex < m_priv->vecCategoryNames.size(); nCatIndex++)
	{
		GfLogDebug("  %s (%s) :\n", m_priv->vecCategoryRealNames[nCatIndex].c_str(),
				   m_priv->vecCategoryNames[nCatIndex].c_str());
		const std::vector<CarData*> vecCarsInCat =
			GetCarsInCategory(m_priv->vecCategoryNames[nCatIndex]);
		std::vector<CarData*>::const_iterator iterCar;
		for (iterCar = vecCarsInCat.begin(); iterCar != vecCarsInCat.end(); iterCar++)
		{
			GfLogDebug("    %s (%s) : %s\n", (*iterCar)->strRealName.c_str(),
					   (*iterCar)->strName.c_str(), (*iterCar)->strXMLPath.c_str());
		}
	}
}
