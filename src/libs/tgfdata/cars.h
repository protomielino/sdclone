/***************************************************************************

    file                 : cars.h
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

#ifndef __TGFCARS__H__
#define __TGFCARS__H__

#include <string>
#include <vector>

#include "tgfdata.h"


/** @file   
		Singleton holding information on the available cars and categories
    @defgroup	tgfdata	Data manager for the client gaming framework.
*/

class TGFDATA_API GfCar
{
public:
	
	const std::string& getId() const;
	const std::string& getName() const;
	const std::string& getCategoryId() const;
	const std::string& getCategoryName() const;
	const std::string& getDescriptorFileName() const;
	
	void setId(const std::string& strId);
	void setName(const std::string& strName);
	void setCategoryId(const std::string& strCatId);
	void setCategoryName(const std::string& strCatName);
	void setDescriptorFileName(const std::string& strDescFile);

protected:
	
	std::string _strId; // XML file / folder name (ex: "sc-boxer-96")
	std::string _strName; // User friendly name (ex: "SC Boxer 96").
	std::string _strCatId; // Category XML file / folder name (ex: "LS-GT1").
	std::string _strCatName; // User friendly category name (ex: "Long day Series GT1").
	std::string _strDescFile; // Path-name of the XML descriptor file.
};

class TGFDATA_API GfCars
{
public:

	// Accessor to the unique instance of the singleton.
	static GfCars* self();
	
	const std::vector<std::string>& getCategoryIds() const;
	const std::vector<std::string>& getCategoryNames() const;

	GfCar* getCar(const std::string& strId) const;
	GfCar* getCarWithName(const std::string& strName) const;

	std::vector<GfCar*> getCarsInCategory(const std::string& strCatId = "") const;
	std::vector<GfCar*> getCarsInCategoryWithName(const std::string& strCatName = "") const;
	std::vector<std::string> getCarIdsInCategory(const std::string& strCatId = "") const;
	std::vector<std::string> getCarNamesInCategory(const std::string& strCatId = "") const;
	
	void print() const;

protected:

	// Protected constructor and destructor : clients can not use them.
	GfCars();
	~GfCars();
	
protected:

	// The singleton itself.
	static GfCars* _pSelf;

	// Its private data.
	class Private;
	Private* _pPrivate;
};

#endif /* __TGFCARS__H__ */
