/***************************************************************************

    file                 : carinfo.cpp
    created              : July 2009
    copyright            : (C) 2009 Brian Gavin
    web                  : speed-dreams.sourceforge.net

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
#include <set>
#include <sstream>

#include "carinfo.h"


struct PrivateData
{
	// One CarData structure for each car (order = sorted directory one).
	std::vector<CarData> vecCars;

	// Map for quick access to CarData by name
	std::map<std::string, int> mapCarNameIndices;

	// Set of category names.
	std::set<std::string> setCategoryNames;
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

	// Load the category list from the "cars" folder contents.
	curFile = files = GfDirGetList("cars");
	if (curFile) 
	{
		do 
		{
			// Ignore "." and ".." folders.
			if (curFile->name[0] == '.') 
				continue;
			
			// Open the XML file of the car.
			const char* pszCarName = curFile->name;
			
			std::ostringstream ossCarFileName;
			ossCarFileName << "cars/" << pszCarName << '/' << pszCarName << ".xml";
			carParmHdle = GfParmReadFile(ossCarFileName.str().c_str(), GFPARM_RMODE_STD);
			if (!carParmHdle) {
				GfError("CarInfo : Ignoring car %s (failed to read from %s)\n",
						pszCarName, ossCarFileName.str().c_str());
				continue;
			}

			// Read car info and store it in the CarData structure.
			CarData carData;
			carData.strName = pszCarName;
			carData.strCategoryName = GfParmGetStr(carParmHdle, "Car", "category", "");
			carData.strRealName = GfParmGetName(carParmHdle);
			carData.strXMLPath = ossCarFileName.str();

			// Update the CarInfo singleton.
			m_priv->vecCars.push_back(carData);
			m_priv->mapCarNameIndices[pszCarName] = m_priv->vecCars.size() - 1;
			m_priv->setCategoryNames.insert(carData.strCategoryName);

			// Close the XML file of the car.
			GfParmReleaseHandle(carParmHdle);
		} 
		while ((curFile = curFile->next) != files);
	}
	
	GfDirFreeList(files, NULL, true, true);
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

std::vector<CarData> CarInfo::GetCarsInCategory(const std::string& strCatName) const
{
	std::vector<CarData> vecCarsInCat;

	std::vector<CarData>::const_iterator iterCar;
	for (iterCar = m_priv->vecCars.begin(); iterCar != m_priv->vecCars.end(); iterCar++)
		if (strCatName == "All" || iterCar->strCategoryName == strCatName)
			vecCarsInCat.push_back(*iterCar);

	return vecCarsInCat;
}

std::vector<std::string> CarInfo::GetCategoryNames() const
{
	std::vector<std::string> vecCatNames;
	
	std::set<std::string>::const_iterator iterCat;
	for (iterCat = m_priv->setCategoryNames.begin();
		 iterCat != m_priv->setCategoryNames.end(); iterCat++)
		vecCatNames.push_back(*iterCat);
	
	return vecCatNames;
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
	GfOut("CarInfo : %d cars, %d categories\n",
		  m_priv->vecCars.size(), m_priv->setCategoryNames.size());
	std::set<std::string>::const_iterator iterCat;
	for (iterCat = m_priv->setCategoryNames.begin();
		 iterCat != m_priv->setCategoryNames.end(); iterCat++)
	{
		GfOut("  %s category :\n", iterCat->c_str());
		const std::vector<CarData> vecCarsInCat = GetCarsInCategory(iterCat->c_str());
		std::vector<CarData>::const_iterator iterCar;
		for (iterCar = vecCarsInCat.begin(); iterCar != vecCarsInCat.end(); iterCar++)
		{
			GfOut("    %s (%s) : %s\n",
				  iterCar->strRealName.c_str(), iterCar->strName.c_str(), iterCar->strXMLPath.c_str());
		}
	}
}
