/***************************************************************************

    file                 : carinfo.h
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

#ifndef __CARINFO__H__
#define __CARINFO__H__

#include <string>
#include <vector>

#include "tgf.h"


/** @file   
    		Singleton holding information on available cars and categories
*/

class TGF_API CarData
{
public:
	std::string strName; // XML file / folder name (ex: "sc-boxer-96")
	std::string strRealName; // User friendly name (ex: "SC Boxer 96").
	std::string strCategoryName; // Category name (ex: "LS-GT1").
	std::string strCategoryRealName; // Category name (ex: "Long Day Series GT1").
	std::string strXMLPath; // Path-name of the car XML file.
};

class TGF_API CarInfo
{
public:

	// Accessor to the unique instance of the singleton.
	static CarInfo* self();
	
	const std::vector<std::string>& GetCategoryNames() const;
	const std::vector<std::string>& GetCategoryRealNames() const;

	CarData* GetCarData(const std::string& strCarName) const;
	std::string GetCarRealName(const std::string& strCarName) const;
	CarData* GetCarDataFromRealName(const std::string& strCarRealName) const;

	std::vector<CarData*> GetCarsInCategory(const std::string& strCatName = "All") const;
	std::vector<CarData*> GetCarsInCategoryRealName(const std::string& strCatRealName = "All") const;
	std::vector<std::string> GetCarNamesInCategory(const std::string& strCatName = "All") const;
	std::vector<std::string> GetCarRealNamesInCategory(const std::string& strCatName = "All") const;
	
	void print() const;

protected:

	// Protected constructor : clients can not use it.
	CarInfo();
	
protected:

	// The singleton itself.
	static CarInfo* m_pSelf;

	// Its private data.
	struct PrivateData* m_priv;
};

#endif /* __CARINFO__H__ */
