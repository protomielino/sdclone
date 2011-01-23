/***************************************************************************

    file                 : drivers.cpp
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

#include <cctype>
#include <map>
#include <sstream>
#include <algorithm>

#include <robot.h>

#include "cars.h"
#include "drivers.h"


// Private data for GfDrivers				  
class GfDrivers::Private
{
public:
	
	// One GfDriver structure for each driver (order = sorted directory one).
	std::vector<GfDriver*> vecDrivers;

	// Map for quick access to GfDriver by { module name, interface index }
	typedef std::map<std::pair<std::string, int>, GfDriver*> TMapDriversByKey;
	TMapDriversByKey mapDriversByKey;

	// Vector of driver types.
	std::vector<std::string> vecTypes;
	
	// Vector of driver car categories.
	std::vector<std::string> vecCarCategoryIds;
};


GfDrivers* GfDrivers::_pSelf = 0;

GfDrivers *GfDrivers::self()
{
	if (!_pSelf)
		_pSelf = new GfDrivers;
	
	return _pSelf;
}

GfDrivers::~GfDrivers()
{
	std::vector<GfDriver*>::const_iterator itDriver;
	for (itDriver = _pPrivate->vecDrivers.begin();
		 itDriver != _pPrivate->vecDrivers.end(); itDriver++)
		delete *itDriver;
}

GfDrivers::GfDrivers()
{
	_pPrivate = new GfDrivers::Private;

	// Load robot modules from the "drivers" installed folder.
	std::string strDriversDirName(GfLibDir());
	strDriversDirName += "drivers";

    tModList* lstDriverModules = 0;
    GfModInfoDir(CAR_IDENT, strDriversDirName.c_str(), 1, &lstDriverModules);
	if (!lstDriverModules)
	{
		GfLogFatal("Could not load any driver module from %s", strDriversDirName.c_str());
		return;
	}
	
	// For each module found, load drivers information.
    tModList *pCurModule = lstDriverModules;
	do
	{
		pCurModule = pCurModule->next;

		// Determine the module name.
		std::string strModName(pCurModule->sopath);
		strModName.erase(strlen(pCurModule->sopath) - strlen(DLLEXT) - 1); // Truncate file ext.
		const size_t nLastSlashInd = strModName.rfind('/');
		if (nLastSlashInd != std::string::npos)
			strModName = strModName.substr(nLastSlashInd+1); // Remove heading folder path.

		// Load the module XML descriptor file (try  user settings first, and then installed one)
		std::ostringstream ossRobotFileName;
		ossRobotFileName << GfLocalDir() << "drivers/" << strModName
						 << '/' << strModName << PARAMEXT;
		void *hparmRobot = GfParmReadFile(ossRobotFileName.str().c_str(), GFPARM_RMODE_STD);
		if (!hparmRobot)
		{
			ossRobotFileName.str("");
			ossRobotFileName << "drivers/" << strModName << '/' << strModName << PARAMEXT;
			hparmRobot = GfParmReadFile(ossRobotFileName.str().c_str(), GFPARM_RMODE_STD);
		}
		if (!hparmRobot)
		{
			GfLogError("No usable '%s' driver (%s.xml not found or not readable)\n",
					   strModName.c_str(), strModName.c_str());
			continue;
		}

		// For each driver (= interface) "in" the module
		for (int nItfInd = 0; nItfInd < pCurModule->modInfoSize; nItfInd++)
		{
			// Ignore empty names 
			if (!pCurModule->modInfo[nItfInd].name || pCurModule->modInfo[nItfInd].name[0] == '\0')
			{
				GfLogWarning("Ignoring '%s' driver #%d (empty name)\n",
							 strModName.c_str(), nItfInd);
				continue;
			}

			// Read driver info from the XML file.
			std::ostringstream ossDriverListPath;
			ossDriverListPath << ROB_SECT_ROBOTS << '/' << ROB_LIST_INDEX
							  << '/' << pCurModule->modInfo[nItfInd].index;
			const char* pszCarId = GfParmGetStr(hparmRobot, ossDriverListPath.str().c_str(), ROB_ATTR_CAR, "");
			const bool bIsHuman = strcmp(GfParmGetStr(hparmRobot, ossDriverListPath.str().c_str(), ROB_ATTR_TYPE, ROB_VAL_ROBOT), ROB_VAL_ROBOT) != 0;

			// Compute/retrieve other information.
			const GfCar* pCar = GfCars::self()->getCar(pszCarId);
			if (pCar)
			{
				// Store it in the GfDriver structure.
				GfDriver* pDriver = new GfDriver;
				pDriver->setName(pCurModule->modInfo[nItfInd].name);
				pDriver->setModuleName(strModName);
				pDriver->setInterfaceIndex(pCurModule->modInfo[nItfInd].index);
				pDriver->setIsHuman(bIsHuman);
				pDriver->setCar(pCar);
			
				// Update the GfCars singleton.
				_pPrivate->vecDrivers.push_back(pDriver);
				const std::pair<std::string, int> driverKey(pDriver->getModuleName(),
															pDriver->getInterfaceIndex());
				_pPrivate->mapDriversByKey[driverKey] = pDriver;
				if (std::find(_pPrivate->vecTypes.begin(), _pPrivate->vecTypes.end(),
							  pDriver->getType()) == _pPrivate->vecTypes.end())
					_pPrivate->vecTypes.push_back(pDriver->getType());
				if (std::find(_pPrivate->vecCarCategoryIds.begin(), _pPrivate->vecCarCategoryIds.end(),
							  pCar->getCategoryId()) == _pPrivate->vecCarCategoryIds.end())
					_pPrivate->vecCarCategoryIds.push_back(pCar->getCategoryId());
			}
			else
			{
				GfLogWarning("Ignoring '%s' driver '%s' (#%d) (default car %s not available)\n",
							 strModName.c_str(), pCurModule->modInfo[nItfInd].name, nItfInd, pszCarId);
			}
		}
		
		// Close driver module descriptor file if open
		if (hparmRobot)
			GfParmReleaseHandle(hparmRobot);
		
	} while (pCurModule != lstDriverModules);

	// Free the module list.
    GfModFreeInfoList(&lstDriverModules);

	// Sort the car category ids and driver types vectors.
	std::sort(_pPrivate->vecCarCategoryIds.begin(), _pPrivate->vecCarCategoryIds.end());
	std::sort(_pPrivate->vecTypes.begin(), _pPrivate->vecTypes.end());

	// Trace what we got.
	print();
}

unsigned GfDrivers::getCount() const
{
	return _pPrivate->vecDrivers.size();
}

const std::vector<std::string>& GfDrivers::getTypes() const
{
	return _pPrivate->vecTypes;
}

GfDriver* GfDrivers::getDriver(const std::string& strModName, int nItfIndex) const
{
	const std::pair<std::string, int> driverKey(strModName, nItfIndex);
	Private::TMapDriversByKey::iterator itDriver =
		_pPrivate->mapDriversByKey.find(driverKey);
	if (itDriver != _pPrivate->mapDriversByKey.end())
		return itDriver->second;
	
	return 0;
}

std::vector<GfDriver*> GfDrivers::getDriversWithTypeAndCategory(const std::string& strType,
																const std::string& strCarCatId) const
{
	std::vector<GfDriver*> vecSelDrivers;
	std::vector<GfDriver*>::iterator itDriver;
	for (itDriver = _pPrivate->vecDrivers.begin();
		 itDriver != _pPrivate->vecDrivers.end(); itDriver++)
		if ((*itDriver)->matchesTypeAndCategory(strType, strCarCatId))
			vecSelDrivers.push_back(*itDriver);

	return vecSelDrivers;
}

void GfDrivers::print() const
{
	GfLogTrace("Driver base : %d types, %d car categories, %d drivers\n",
			   _pPrivate->vecTypes.size(), _pPrivate->vecCarCategoryIds.size(),
			   _pPrivate->vecDrivers.size());
	std::vector<std::string>::const_iterator itType;
	for (itType = _pPrivate->vecTypes.begin(); itType != _pPrivate->vecTypes.end(); itType++)
	{
		GfLogTrace("  '%s' type :\n", itType->c_str());
		std::vector<std::string>::const_iterator itCarCatId;
		for (itCarCatId = _pPrivate->vecCarCategoryIds.begin();
			 itCarCatId != _pPrivate->vecCarCategoryIds.end(); itCarCatId++)
		{
			const std::vector<GfDriver*> vecDrivers =
				getDriversWithTypeAndCategory(*itType, *itCarCatId);
			if (vecDrivers.empty())
				continue;
			GfLogTrace("      '%s' car category :\n", itCarCatId->c_str());
			std::vector<GfDriver*>::const_iterator itDriver;
			for (itDriver = vecDrivers.begin(); itDriver != vecDrivers.end(); itDriver++)
				GfLogTrace("          %-24s : %s\n", (*itDriver)->getName().c_str(),
						   (*itDriver)->getCar()->getName().c_str());
		}
	}
}

// GfDriverSkin class ---------------------------------------------------------------

GfDriverSkin::GfDriverSkin(const std::string& strName) : _strName(strName), _nTargets(0)
{
}

int GfDriverSkin::getTargets() const
{
	return _nTargets;
}

const std::string& GfDriverSkin::getName() const
{
	return _strName;
}

const std::string& GfDriverSkin::getCarPreviewFileName() const
{
	return _strCarPreviewFileName;
}

void GfDriverSkin::setTargets(int nTargets)
{
	_nTargets = nTargets;
}

void GfDriverSkin::addTargets(int nTargets)
{
	_nTargets |= nTargets;
}

void GfDriverSkin::setName(const std::string& strName)
{
	_strName = strName;
}

void GfDriverSkin::setCarPreviewFileName(const std::string& strFileName)
{
	_strCarPreviewFileName = strFileName;
}

// GfDriver class -------------------------------------------------------------------

GfDriver::GfDriver() : _nItfIndex(-1), _bIsHuman(false), _pCar(0)
{
}

const std::string& GfDriver::getName() const
{
	return _strName;
}

const std::string& GfDriver::getModuleName() const
{
	return _strModName;
}

int GfDriver::getInterfaceIndex() const
{
	return _nItfIndex;
}

bool GfDriver::isHuman() const
{
	return _bIsHuman;
}

const GfCar* GfDriver::getCar() const
{
	return _pCar;
}

// GfCar* GfDriver::getCar()
// {
// 	return _pCar;
// }

const GfDriverSkin& GfDriver::getSkin() const
{
	return _skin;
}

const std::string& GfDriver::getType() const
{
	if (_strType.empty())
	{
		// Parse module name for last '_' char : assumed to be the separator between type
		// and instance name for ubiquitous robots (ex: simplix)
		size_t nTruncPos = _strModName.rfind('_');
		if (nTruncPos == std::string::npos)
			_strType = _strModName; // Copy.
		else
			_strType = _strModName.substr(0, nTruncPos); // Copy + truncate.
	}

	return _strType;
	
}

bool GfDriver::matchesTypeAndCategory(const std::string& strType,
									  const std::string& strCarCatId) const
{
	return (strType.empty() || getType() == strType)
		   && (strCarCatId.empty() || getCar()->getCategoryId() == strCarCatId);
}

void GfDriver::setName(const std::string& strName)
{
	_strName = strName;
}

void GfDriver::setModuleName(const std::string& strModName)
{
	_strModName = strModName;
}

void GfDriver::setInterfaceIndex(int nItfIndex)
{
	_nItfIndex = nItfIndex;
}

void GfDriver::setIsHuman(bool bIsHuman)
{
	_bIsHuman = bIsHuman;
}

void GfDriver::setCar(const GfCar* pCar)
{
	_pCar = pCar;
}

void GfDriver::setSkin(const GfDriverSkin& skin)
{
	_skin = skin;
}

static const char* pszLiveryTexExt = ".png";
static const char* pszPreviewTexSufx = "-preview.jpg";
static const char* pszInteriorTexExt = ".png";
static const char* pszInteriorTexSufx = "-int";
static const char* pszLogoTexName = "logo"; // Warning: Must be consistent with grscene.cpp
static const char* pszLogoTexExt = ".png";
static const char* pszWheel3DTexName = "wheel3d"; // Warning: Must be consistent with wheel<i>.ac/acc
static const char* pszWheel3DTexExt = ".png";

static const char* apszExcludedSkinNamePrefixes[] = { "rpm", "speed", "int" };
static const int nExcludedSkinNamePrefixes = sizeof(apszExcludedSkinNamePrefixes) / sizeof(char*);


std::vector<GfDriverSkin>::iterator GfDriver::findSkin(std::vector<GfDriverSkin>& vecSkins,
													   const std::string& strName)
{
	std::vector<GfDriverSkin>::iterator itSkin;
	for (itSkin = vecSkins.begin(); itSkin != vecSkins.end(); itSkin++)
	{
		if (itSkin->getName() == strName)
			return itSkin;
	}

	return vecSkins.end();
}


void GfDriver::getPossibleSkinsInFolder(const std::string& strCarId,
										const std::string& strFolderPath,
										std::vector<GfDriverSkin>& vecPossSkins) const
{
	//GfLogDebug("  getPossibleSkinsInFolder(car=%s, car=%s) ...\n",
	//		   strCarId.c_str(), strFolderPath.c_str());

	// Search for skinned livery files, and associated preview files if any.
	tFList *pLiveryFileList =
		GfDirGetListFiltered(strFolderPath.c_str(), strCarId.c_str(), pszLiveryTexExt);
	if (pLiveryFileList)
	{
		tFList *pCurLiveryFile = pLiveryFileList;
		do
		{
			pCurLiveryFile = pCurLiveryFile->next;

			// Extract the skin name from the livery file name.
			const int nSkinNameLen = // Expecting "<car name>-<skin name>.png"
				strlen(pCurLiveryFile->name) - strCarId.length() - 1 - strlen(pszLiveryTexExt);
			std::string strSkinName;
			if (nSkinNameLen > 0) // Otherwise, default/standard "<car name>.png"
			{
				strSkinName =
					std::string(pCurLiveryFile->name)
					.substr(strCarId.length() + 1, nSkinNameLen);
				
				// Ignore skins with an excluded prefix.
				int nExclPrfxInd = 0;
				for (; nExclPrfxInd < nExcludedSkinNamePrefixes; nExclPrfxInd++)
					if (strSkinName.find(apszExcludedSkinNamePrefixes[nExclPrfxInd]) == 0)
						break;
				if (nExclPrfxInd < nExcludedSkinNamePrefixes)
					continue;
			}
			
			// Ignore skins that are already in the list (path search priority).
			if (findSkin(vecPossSkins, strSkinName) == vecPossSkins.end())
			{
				// Create the new skin.
				GfDriverSkin skin(strSkinName);

				// Add the whole car livery to the skin targets.
				skin.addTargets(RM_CAR_SKIN_TARGET_WHOLE_LIVERY);
				
				GfLogDebug("  Found %s%s livery\n",
						   strSkinName.empty() ? "standard" : strSkinName.c_str(),
						   strSkinName.empty() ? "" : "-skinned");
				
				// Add associated preview image, without really checking file existence
				// (warn only ; up to the client GUI to do what to do if it doesn't exist).
				std::ostringstream ossPreviewName;
				ossPreviewName << strFolderPath << '/' << strCarId;
				if (!strSkinName.empty())
					ossPreviewName << '-' << strSkinName;
				ossPreviewName << pszPreviewTexSufx;
				skin.setCarPreviewFileName(ossPreviewName.str());

				if (!GfFileExists(ossPreviewName.str().c_str()))
					GfLogWarning("Preview file not found for %s %s skin (%s)\n",
								 strCarId.c_str(), strSkinName.c_str(), ossPreviewName.str().c_str());
				//else
				//	GfLogDebug("* found skin=%s, preview=%s\n",
				//			   strSkinName.c_str(), ossPreviewName.str().c_str());

				// Add the new skin to the list.
				vecPossSkins.push_back(skin);
			}

		}
		while (pCurLiveryFile != pLiveryFileList);
	}
	
	GfDirFreeList(pLiveryFileList, NULL);
	
	// Search for skinned interior files, if any.
	std::string strInteriorPrefix(strCarId);
	strInteriorPrefix += pszInteriorTexSufx;
	tFList *pIntFileList =
		GfDirGetListFiltered(strFolderPath.c_str(), strInteriorPrefix.c_str(), pszInteriorTexExt);
	if (pIntFileList)
	{
		tFList *pCurIntFile = pIntFileList;
		do
		{
			pCurIntFile = pCurIntFile->next;

			// Extract the skin name from the interior file name.
			const int nSkinNameLen = // Expecting "<car name>-int-<skin name>.png"
				strlen(pCurIntFile->name) - strInteriorPrefix.length()
				- 1 - strlen(pszInteriorTexExt);
			std::string strSkinName;
			if (nSkinNameLen > 0)
			{
				strSkinName =
					std::string(pCurIntFile->name)
					.substr(strInteriorPrefix.length() + 1, nSkinNameLen);

				// If a skin with such name already exists in the list, update it.
				std::vector<GfDriverSkin>::iterator itSkin =
					findSkin(vecPossSkins, strSkinName);
				if (itSkin != vecPossSkins.end())
				{
					itSkin->addTargets(RM_CAR_SKIN_TARGET_INTERIOR);
					GfLogDebug("  Found %s-skinned interior (targets:%x)\n",
							   strSkinName.c_str(), itSkin->getTargets());
				}
			}
		}
		while (pCurIntFile != pIntFileList);
	}
	
	// Search for skinned logo files if any.
	tFList *pLogoFileList =
		GfDirGetListFiltered(strFolderPath.c_str(), pszLogoTexName, pszLogoTexExt);
	if (pLogoFileList)
	{
		tFList *pCurLogoFile = pLogoFileList;
		do
		{
			pCurLogoFile = pCurLogoFile->next;

			// Extract the skin name from the logo file name.
			const int nSkinNameLen = // Expecting "logo-<skin name>.png"
				strlen(pCurLogoFile->name) - strlen(pszLogoTexName)
				- 1 - strlen(pszLogoTexExt);
			if (nSkinNameLen > 0)
			{
				const std::string strSkinName =
					std::string(pCurLogoFile->name)
					.substr(strlen(pszLogoTexName) + 1, nSkinNameLen);
			
				// If a skin with such name already exists in the list, update it.
				std::vector<GfDriverSkin>::iterator itSkin = findSkin(vecPossSkins, strSkinName);
				if (itSkin != vecPossSkins.end())
				{
					itSkin->addTargets(RM_CAR_SKIN_TARGET_PIT_DOOR);
					GfLogDebug("  Found %s-skinned logo (targets:%x)\n",
							   strSkinName.c_str(), itSkin->getTargets());
				}
			}
				
		}
		while (pCurLogoFile != pLogoFileList);
	}
	
	GfDirFreeList(pLogoFileList, NULL);
	
	// Search for skinned 3D wheel files if any.
	tFList *pWheel3DFileList =
		GfDirGetListFiltered(strFolderPath.c_str(), pszWheel3DTexName, pszWheel3DTexExt);
	if (pWheel3DFileList)
	{
		tFList *pCurWheel3DFile = pWheel3DFileList;
		do
		{
			pCurWheel3DFile = pCurWheel3DFile->next;

			// Extract the skin name from the 3D wheel texture file name.
			const int nSkinNameLen = // Expecting "logo-<skin name>.png"
				strlen(pCurWheel3DFile->name) - strlen(pszWheel3DTexName)
				- 1 - strlen(pszWheel3DTexExt);
			if (nSkinNameLen > 0)
			{
				const std::string strSkinName =
					std::string(pCurWheel3DFile->name)
					.substr(strlen(pszWheel3DTexName) + 1, nSkinNameLen);
			
				// If a skin with such name already exists in the list, update it.
				std::vector<GfDriverSkin>::iterator itSkin = findSkin(vecPossSkins, strSkinName);
				if (itSkin != vecPossSkins.end())
				{
					itSkin->addTargets(RM_CAR_SKIN_TARGET_3D_WHEELS);
					GfLogDebug("  Found %s-skinned 3D wheels (targets:%x)\n",
							   strSkinName.c_str(), itSkin->getTargets());
				}
			}
		}
		while (pCurWheel3DFile != pWheel3DFileList);
	}
	
	GfDirFreeList(pWheel3DFileList, NULL);
}

std::vector<GfDriverSkin> GfDriver::getPossibleSkins(const std::string& strAltCarId) const
{
	const std::string strCarId = strAltCarId.empty() ? _pCar->getId() : strAltCarId;

	GfLogDebug("Checking skins for %s ...\n", strCarId.c_str());

	// Clear the skin and preview lists.
	std::vector<GfDriverSkin> vecPossSkins;

	// Get/check skins/skin targets/previews from the directories in the search path
	// WARNING: Must be consistent with the search paths used in grcar.cpp, grboard.cpp,
	//          grscene.cpp ... etc ... but it is not currently 100% achieved
	//          (pit door logos are not searched by the graphics engine
	//           in the car-dedicated folders ... so they may be "over-detected" here).
	std::ostringstream ossDirPath;
	ossDirPath << GfLocalDir() << "drivers/" << _strModName
			   << '/' << _nItfIndex << '/' << strCarId;
	getPossibleSkinsInFolder(strCarId, ossDirPath.str(), vecPossSkins);

	ossDirPath.str("");
	ossDirPath << GfLocalDir() << "drivers/" << _strModName
			   << '/' << strCarId;
	getPossibleSkinsInFolder(strCarId, ossDirPath.str(), vecPossSkins);

	ossDirPath.str("");
	ossDirPath << GfLocalDir() << "drivers/" << _strModName;
	getPossibleSkinsInFolder(strCarId, ossDirPath.str(), vecPossSkins);

	ossDirPath.str("");
	ossDirPath << "drivers/" << _strModName
			   << '/' << _nItfIndex << '/' << strCarId;
	getPossibleSkinsInFolder(strCarId, ossDirPath.str(), vecPossSkins);

	ossDirPath.str("");
	ossDirPath << "drivers/" << _strModName
			   << '/' << _nItfIndex;
	getPossibleSkinsInFolder(strCarId, ossDirPath.str(), vecPossSkins);

	ossDirPath.str("");
	ossDirPath << "drivers/" << _strModName
			   << '/' << strCarId;
	getPossibleSkinsInFolder(strCarId, ossDirPath.str(), vecPossSkins);

	ossDirPath.str("");
	ossDirPath << "drivers/" << _strModName;
	getPossibleSkinsInFolder(strCarId, ossDirPath.str(), vecPossSkins);

	ossDirPath.str("");
	ossDirPath << "cars/" << strCarId;
	getPossibleSkinsInFolder(strCarId, ossDirPath.str(), vecPossSkins);

	// If we have at least 1 skin, make sure that, if the standard one is inside,
	// it is the first one.
	if (!vecPossSkins.empty())
	{
		std::vector<GfDriverSkin>::iterator itSkin;
		for (itSkin = vecPossSkins.begin(); itSkin != vecPossSkins.end(); itSkin++)
		{
			if (itSkin->getName().empty() && itSkin != vecPossSkins.begin())
			{
				GfDriverSkin stdSkin = *itSkin;
				vecPossSkins.erase(itSkin);
				vecPossSkins.insert(vecPossSkins.begin(), stdSkin);
				break;
			}
		}
	}
	
	// If no skin was found, add the car's standard one
	// (that way, the skin list will never be empty, and that's safer)
	else
	{
		GfLogError("No skin at all found for '%s/%d/%s' : adding dummy '%s' one\n",
				   _strModName.c_str(), _nItfIndex, strCarId.c_str(), "standard");
		
		GfDriverSkin stdSkin;
		std::ostringstream ossPreviewName;
		ossPreviewName << "cars/" << strCarId << '/' << strCarId << pszPreviewTexSufx;
		stdSkin.setCarPreviewFileName(ossPreviewName.str());

		if (!GfFileExists(ossPreviewName.str().c_str()))
			GfLogWarning("No preview file %s found for dummy '%s' skin\n",
						 ossPreviewName.str().c_str(), "standard");

		vecPossSkins.push_back(stdSkin);
	}

	return vecPossSkins;
}

