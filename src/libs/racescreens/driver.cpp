/***************************************************************************
                  driver.cpp -- drivers manipuation functions                              
                             -------------------                                         
    created              : Sun Dec 04 19:00:00 CET 2008
    copyright            : (C) 2008 by Jean-Philippe MEURET
    email                : jpmeuret@free.fr
                      
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/** @file
    		This is a collection of useful functions for managing drivers.
    @ingroup	racemantools
    @author	<a href=mailto:jpmeuret@free.fr>Jean-Philippe MEURET</a>
*/


#include <sys/stat.h>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <algorithm>

#include <car.h>

#include "driver.h"

				  
// Some consts.
const char* rmdStdSkinName = "standard";

static const char* pszSkinFileExt = ".png";
static const char* pszPreviewFileSuffix = "-preview.jpg";
static const char* pszLogoFileName = "logo"; // Warning: Must be consistent with grscene.cpp
static const char* pszWheel3DFileName = "wheel3d"; // Warning: Must be consistent with wheel<i>.ac/acc

static const char* apszExcludedSkinFileSuffixes[] =
{ "rpm.png", "speed.png", "int.png" };
static const int nExcludedSkinFileSuffixes = sizeof(apszExcludedSkinFileSuffixes) / sizeof(char*);


int rmdDriverMatchesFilters(const trmdDrvElt *drv, const char* carCat, const char* drvTyp,
			    const char* anyCarCat, const char* anyDrvTyp)
{
    return (!strcmp(carCat, anyCarCat)
			|| !strcmp(GfParmGetStr(drv->carParmHdle, SECT_CAR, PRM_CATEGORY, ""), carCat))
		   && (!strcmp(drvTyp, anyDrvTyp)
			   || strstr(drv->moduleName, drvTyp) == drv->moduleName);
}

void rmdGetDriverType(const char* moduleName, char* driverType, size_t maxSize)
{
    char* pos;

    strncpy(driverType, moduleName, maxSize);
    driverType[maxSize-1] = 0; // Ensure 0 termination

    // Parse module name for last '_' char : 
    // assumed to be the separator between type and instance name for ubiquitous robots (ex: simplix)
    pos = strrchr(driverType, '_');
    if (pos)
	*pos = 0;

    // Otherwise, search for an isolated last digit in the module name :
    // old robot with hard-coded max cars of 10 may follow this pattern (ex: berniw2 and 3)
    else
    {
	pos = driverType + strlen(driverType) - 1;
	while (pos != driverType && isdigit(*pos))
	    pos--;
	if (++pos == driverType + strlen(driverType) - 1)
	    *pos = 0;
    }
}

void rmdGetCarSkinsInFolder(const char* pszCarName, const char* pszFolderPath,
							std::vector<std::string>& vecSkinNames,
							std::map<std::string, int>& mapSkinTargets,
							std::map<std::string, std::string>& mapPreviewFiles)
{
	//GfLogDebug("  rmdGetCarSkinsInFolder(car=%s, path=%s) ...\n", pszCarName, pszFolderPath);

	// Search for livery skin files, and asociated preview files if any.
	tFList *pSkinFileList = GfDirGetListFiltered(pszFolderPath, pszCarName, pszSkinFileExt);
	if (pSkinFileList)
	{
		tFList *pCurSkinFile = pSkinFileList;
		do
		{
			pCurSkinFile = pCurSkinFile->next;

			// Ignore files with an excluded suffix.
			int nExclSfxInd = 0;
			for (; nExclSfxInd < nExcludedSkinFileSuffixes; nExclSfxInd++)
				 if (strstr(pCurSkinFile->name, apszExcludedSkinFileSuffixes[nExclSfxInd]))
					 break;
			if (nExclSfxInd < nExcludedSkinFileSuffixes)
				continue;

			// Extract the skin name from the skin file name.
			const int nSkinNameLen = // Expecting "<car name>-<skin name>.png"
				strlen(pCurSkinFile->name) - strlen(pszCarName) - 1 - strlen(pszSkinFileExt);
			std::string strSkinName;
			if (nSkinNameLen > 0)
				strSkinName =
					std::string(pCurSkinFile->name)
					.substr(strlen(pszCarName) + 1, nSkinNameLen);
			else // Assuming default/standard "<car name>.png"
				strSkinName = rmdStdSkinName;
			
			// Ignore skins that are already in the list (path search priority).
			if (std::find(vecSkinNames.begin(), vecSkinNames.end(), strSkinName)
				== vecSkinNames.end())
			{
				// Add found skin in the list
				vecSkinNames.push_back(strSkinName);

				// Add the whole car livery to the skin targets.
				if (mapSkinTargets.find(strSkinName) == mapSkinTargets.end())
					mapSkinTargets[strSkinName] = 0;
				mapSkinTargets[strSkinName] |= RM_CAR_SKIN_TARGET_WHOLE_LIVERY;
				
				GfLogDebug("  Found %s%s livery\n", strSkinName.c_str(),
						   strSkinName == rmdStdSkinName ? "" : "-skinned");
				
				// Add associated preview image, without really checking file existence
				// (warn only ; up to the client GUI to do what to do if it doesn't exist).
				std::ostringstream ossPreviewName;
				ossPreviewName << pszFolderPath << '/' << pszCarName;
				if (strSkinName != rmdStdSkinName)
					ossPreviewName << '-' << strSkinName;
				ossPreviewName << pszPreviewFileSuffix;
				mapPreviewFiles[strSkinName] = ossPreviewName.str();

				struct stat st;
				if (stat(ossPreviewName.str().c_str(), &st))
					GfLogWarning("Preview file not found for %s %s skin (%s)\n",
								 pszCarName, strSkinName.c_str(), ossPreviewName.str().c_str());

				//GfLogDebug("* found skin=%s, preview=%s\n",
				//	  strSkinName.c_str(), ossPreviewName.str().c_str());
			}

		} while (pCurSkinFile != pSkinFileList);
	}
	
	GfDirFreeList(pSkinFileList, NULL);
	
	// Search for skinned logo files if any.
	tFList *pLogoFileList =
		GfDirGetListFiltered(pszFolderPath, pszLogoFileName, pszSkinFileExt);
	if (pLogoFileList)
	{
		tFList *pCurLogoFile = pLogoFileList;
		do
		{
			pCurLogoFile = pCurLogoFile->next;

			// Extract the skin name from the logo file name.
			const int nSkinNameLen = // Expecting "logo-<skin name>.png"
				strlen(pCurLogoFile->name) - strlen(pszLogoFileName)
				- 1 - strlen(pszSkinFileExt);
			if (nSkinNameLen > 0)
			{
				const std::string strSkinName =
					std::string(pCurLogoFile->name)
					.substr(strlen(pszLogoFileName) + 1, nSkinNameLen);
			
				// Add the pit door to the skin targets.
				if (mapSkinTargets.find(strSkinName) == mapSkinTargets.end())
					mapSkinTargets[strSkinName] = 0;
				mapSkinTargets[strSkinName] |= RM_CAR_SKIN_TARGET_PIT_DOOR;

				GfLogDebug("  Found %s-skinned logo (targets:%x)\n",
						   strSkinName.c_str(), mapSkinTargets[strSkinName]);
			}
				
		} while (pCurLogoFile != pLogoFileList);
	}
	
	GfDirFreeList(pLogoFileList, NULL);
	
	// Search for skinned 3D wheel files if any.
	tFList *pWheel3DFileList =
		GfDirGetListFiltered(pszFolderPath, pszWheel3DFileName, pszSkinFileExt);
	if (pWheel3DFileList)
	{
		tFList *pCurWheel3DFile = pWheel3DFileList;
		do
		{
			pCurWheel3DFile = pCurWheel3DFile->next;

			// Extract the skin name from the 3D wheel texture file name.
			const int nSkinNameLen = // Expecting "logo-<skin name>.png"
				strlen(pCurWheel3DFile->name) - strlen(pszWheel3DFileName)
				- 1 - strlen(pszSkinFileExt);
			if (nSkinNameLen > 0)
			{
				const std::string strSkinName =
					std::string(pCurWheel3DFile->name)
					.substr(strlen(pszWheel3DFileName) + 1, nSkinNameLen);
			
				// Add the 3D wheels to the skin targets.
				if (mapSkinTargets.find(strSkinName) == mapSkinTargets.end())
					mapSkinTargets[strSkinName] = 0;
				mapSkinTargets[strSkinName] |= RM_CAR_SKIN_TARGET_3D_WHEELS;

				GfLogDebug("  Found %s-skinned 3D wheels (targets:%x)\n",
						   strSkinName.c_str(), mapSkinTargets[strSkinName]);
			}
				
		} while (pCurWheel3DFile != pWheel3DFileList);
	}
	
	GfDirFreeList(pWheel3DFileList, NULL);
	
}

void rmdGetCarSkinsInSearchPath(const trmdDrvElt *pDriver, const char* pszForcedCarName,
								std::vector<std::string>& vecSkinNames,
								std::map<std::string, int>& mapSkinTargets,
								std::map<std::string, std::string>& mapPreviewFiles)
{
	const char* pszCarName = pszForcedCarName ? pszForcedCarName : pDriver->carName;
	std::ostringstream ossDirPath;
	std::string strPreviewName;

	GfLogDebug("Checking skins for %s ...\n", pszCarName);

	// Clear the skin and preview lists.
	vecSkinNames.clear();
	mapPreviewFiles.clear();
	mapSkinTargets.clear();

	// Get/check skins/skin targets/previews from the directories in the search path
	// WARNING: Must be consistent with the search paths used in grcar.cpp, grboard.cpp,
	//          grscene.cpp ... etc ... but it is not currently 100% achieved
	//          (pit door logos are not searched by the graphics engine
	//           in the car-dedicated folders ... so they may be "over-detected" here).
	ossDirPath << GetLocalDir() << "drivers/" << pDriver->moduleName
			   << '/' << pDriver->interfaceIndex << '/' << pszCarName;
	rmdGetCarSkinsInFolder(pszCarName, ossDirPath.str().c_str(),
						   vecSkinNames, mapSkinTargets, mapPreviewFiles);

	ossDirPath.str("");
	ossDirPath << GetLocalDir() << "drivers/" << pDriver->moduleName
			   << '/' << pszCarName;
	rmdGetCarSkinsInFolder(pszCarName, ossDirPath.str().c_str(),
						   vecSkinNames, mapSkinTargets, mapPreviewFiles);

	ossDirPath.str("");
	ossDirPath << GetLocalDir() << "drivers/" << pDriver->moduleName;
	rmdGetCarSkinsInFolder(pszCarName, ossDirPath.str().c_str(),
						   vecSkinNames, mapSkinTargets, mapPreviewFiles);

	ossDirPath.str("");
	ossDirPath << "drivers/" << pDriver->moduleName
			   << '/' << pDriver->interfaceIndex << '/' << pszCarName;
	rmdGetCarSkinsInFolder(pszCarName, ossDirPath.str().c_str(),
						   vecSkinNames, mapSkinTargets, mapPreviewFiles);

	ossDirPath.str("");
	ossDirPath << "drivers/" << pDriver->moduleName
			   << '/' << pDriver->interfaceIndex;
	rmdGetCarSkinsInFolder(pszCarName, ossDirPath.str().c_str(),
						   vecSkinNames, mapSkinTargets, mapPreviewFiles);

	ossDirPath.str("");
	ossDirPath << "drivers/" << pDriver->moduleName
			   << '/' << pszCarName;
	rmdGetCarSkinsInFolder(pszCarName, ossDirPath.str().c_str(),
						   vecSkinNames, mapSkinTargets, mapPreviewFiles);

	ossDirPath.str("");
	ossDirPath << "drivers/" << pDriver->moduleName;
	rmdGetCarSkinsInFolder(pszCarName, ossDirPath.str().c_str(),
						   vecSkinNames, mapSkinTargets, mapPreviewFiles);

	ossDirPath.str("");
	ossDirPath << "cars/" << pszCarName;
	rmdGetCarSkinsInFolder(pszCarName, ossDirPath.str().c_str(),
						   vecSkinNames, mapSkinTargets, mapPreviewFiles);

	// If we have at least 1 skin, make sure that if the standard one is inside,
	// it is the first one.
	if (!vecSkinNames.empty())
	{
		std::vector<std::string>::iterator iterSkin =
			std::find(vecSkinNames.begin(), vecSkinNames.end(), rmdStdSkinName);
		if (iterSkin != vecSkinNames.end() && iterSkin != vecSkinNames.begin())
		{
			vecSkinNames.erase(iterSkin);
			vecSkinNames.insert(vecSkinNames.begin(), rmdStdSkinName);
		}
	}
	
	// If no skin was found, add the car's standard one
	// (that way, the skin list will never be empty, and that's safer)
	else
	{
		GfLogError("No skin at all found for '%s/%d/%s' : adding dummy '%s' one\n",
				   pDriver->moduleName, pDriver->interfaceIndex, pszCarName, rmdStdSkinName);
		
		// Skin.
		vecSkinNames.push_back(rmdStdSkinName);
		
		// Associated preview image.
		std::ostringstream ossPreviewName;
		ossPreviewName << "cars/" << pszCarName << '/' << pszCarName << pszPreviewFileSuffix;

		mapPreviewFiles[rmdStdSkinName] = ossPreviewName.str();

		struct stat st;
		if (stat(ossPreviewName.str().c_str(), &st))
			GfLogWarning("No preview file %s found for dummy '%s' skin\n",
						 ossPreviewName.str().c_str(), rmdStdSkinName);
	}
}
