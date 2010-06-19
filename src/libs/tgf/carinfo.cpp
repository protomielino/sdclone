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

#include "carinfo.h"


struct PrivateData
{
	std::vector<CarData> vecCarData;
	std::map<std::string, int> mapCarName;
	std::set<std::string> setCarCat;
};


static CarInfo g_carInfo;

CarInfo *GetCarInfo()
{
	return &g_carInfo;
}

CarInfo::CarInfo()
{
	m_bInit = false;
}

void CarInfo::Init()
{
	if (m_bInit)
		return;

	m_priv = new PrivateData;

	// Read in car info
	tFList	*files;
	tFList	*curFile;
	char buf[1024];
	void	*hdle;

	// Load the category list
	files = GfDirGetList("cars");
	curFile = files;
	if (curFile && curFile->name[0] != '.') 
	{
		do 
		{
			curFile = curFile->next;
			std::string strName = strdup(curFile->name);
			
			sprintf(buf, "cars/%s/%s.xml", strName.c_str(),strName.c_str());
			hdle = GfParmReadFile(buf, GFPARM_RMODE_STD);
			if (!hdle) {
				continue;
			}
			CarData cardata;
			cardata.strName = strName;
			cardata.strCategory = GfParmGetStr(hdle,"Car","category", "");
			cardata.strRealCarName = GfParmGetName(hdle);
			cardata.strXMLPath = buf;
			m_priv->vecCarData.push_back(cardata);
			
			m_priv->mapCarName[strName] = m_priv->vecCarData.size()-1;
			m_priv->setCarCat.insert(cardata.strCategory);
			
			GfParmReleaseHandle(hdle);
		} 
		while (curFile != files);
	}
	
	GfDirFreeList(files, NULL, true, true);

	m_bInit = true;
}

std::string CarInfo::GetRealCarName(const char *pszName)
{
	Init();

	std::string strRealName;

	std::map<std::string,int>::iterator p;
	p = m_priv->mapCarName.find(pszName);
	if (p!=m_priv->mapCarName.end())
	{
		CarData *pCar = &m_priv->vecCarData[p->second];
		strRealName = pCar->strRealCarName;
	}

	return strRealName;
}

void CarInfo::GetCarsInCategory(const char *pszCat, std::vector<std::string>& vecCars)
{
	Init();

	std::string strCat = pszCat;

	for (unsigned i=0;i<m_priv->vecCarData.size();i++)
	{
		if (strCat == "All")
			vecCars.push_back(m_priv->vecCarData[i].strName);
		else if (m_priv->vecCarData[i].strCategory == strCat)
			vecCars.push_back(m_priv->vecCarData[i].strName);
	}
}

void CarInfo::GetCategories(std::vector<std::string>&vecCats)
{
	Init();

	std::set<std::string>::iterator p;
	p = m_priv->setCarCat.begin();

	while (p!= m_priv->setCarCat.end())
	{
		vecCats.push_back(*p);
		p++;
	}
}

void CarInfo::GetCarsInCategoryRealName(const char *pszCat,std::vector<std::string>&vecCars)
{
	Init();

	std::string strCat = pszCat;

	for (unsigned i=0;i<m_priv->vecCarData.size();i++)
	{
		if (strCat == "All")
			vecCars.push_back(m_priv->vecCarData[i].strRealCarName);
		else if (m_priv->vecCarData[i].strCategory == strCat)
			vecCars.push_back(m_priv->vecCarData[i].strRealCarName);
		
	}
}

CarData * CarInfo::GetCarDataFromRealName(const char *pszRealName)
{
	Init();

	for (unsigned i=0;i<m_priv->vecCarData.size();i++)
	{
		if (m_priv->vecCarData[i].strRealCarName==pszRealName)
		{
			return &m_priv->vecCarData[i];
		}
	}

	return NULL;
}

CarData * CarInfo::GetCarData(const char *pszName)
{
	Init();

	std::map<std::string,int>::iterator p;
	p = m_priv->mapCarName.find(pszName);
	if (p!=m_priv->mapCarName.end())
		return &m_priv->vecCarData[p->second];
	
	return NULL;
}
