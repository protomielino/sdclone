/***************************************************************************

    file                 : carinfo.h
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
#ifndef __CARINFO__H__
#define __CARINFO__H__

#include <string>
#include <vector>

#include <portability.h>

#include "tgf.h"

class TGF_API CarData
{
public:
	std::string strName;
	std::string strCategory;
	std::string strRealCarName;
	std::string strXMLPath;
};

// Export STL needed container types (needed with MSVC compilers).
#ifdef WIN32
ExportSTLVector(TGF_API, std::string);
ExportSTLVector(TGF_API, CarData);
//ExportSTLSet(TGF_API, std::string);
//ExportSTLMap(TGF_API, std::string, int);
#endif

class TGF_API CarInfo
{
public:
	CarInfo();
	std::string GetRealCarName(const char* pszName);
	void GetCarsInCategory(const char* pszCat, std::vector<std::string>& vecCars);
	void GetCarsInCategoryRealName(const char *pszCat, std::vector<std::string>& vecCars);
	void GetCategories(std::vector<std::string>& vecCats);
	CarData* GetCarData(const char* pszName);
	CarData* GetCarDataFromRealName(const char* pszRealName);
protected:
	void Init();
	bool m_bInit;
protected:
	struct PrivateData* m_priv;
	//std::vector<CarData> m_vecCarData;
	//std::map<std::string, int> m_mapCarName;
	//std::set<std::string> m_setCarCat;
};


TGF_API CarInfo* GetCarInfo();

#endif /* __CARINFO__H__ */
