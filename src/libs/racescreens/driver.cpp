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


//#include <sys/stat.h>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <algorithm>

#include <car.h>

#include "driver.h"

				  
// Some consts.
const char* rmdStdSkinName = "standard";

// Temporary workaround for MS shit : moved to driverselect.cpp
// static const char* pszSkinFileExt = ".png";
// static const char* pszPreviewFileSuffix = "-preview.png";

// static const char* apszExcludedSkinFileSuffixes[] =
// { "-rpm.png", "-speed.png", pszPreviewFileSuffix };
// static const int nExcludedSkinFileSuffixes = sizeof(apszExcludedSkinFileSuffixes) / sizeof(char*);


int rmdDriverMatchesFilters(const trmdDrvElt *drv, const char* carCat, const char* drvTyp,
			    const char* anyCarCat, const char* anyDrvTyp)
{
    return (!strcmp(carCat, anyCarCat) || !strcmp(GfParmGetStr(drv->carParmHdle, SECT_CAR, PRM_CATEGORY, ""), carCat))
	   && (!strcmp(drvTyp, anyDrvTyp) || strstr(drv->moduleName, drvTyp) == drv->moduleName);
}

void rmdGetDriverType(const char* moduleName, char* driverType, size_t maxSize)
{
    char* pos;

    strncpy(driverType, moduleName, maxSize);
    driverType[maxSize-1] = 0; // Guarantee 0 termination

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

// Temporary workaround for MS shit : moved to driverselect.cpp
#if 0
void rmdGetCarSkinsInFolder(const trmdDrvElt *pDriver, const char* pszFolderPath,
							std::vector<std::string>& lstSkinNames,
							std::vector<std::string>& lstPreviewFiles)
{
	//struct stat st;
	tFList *pSkinFileList, *pCurSkinFile;

	GfOut("rmdGetCarSkinsInFolder(%s) :\n", pszFolderPath);

	pCurSkinFile = pSkinFileList =
		GfDirGetListFiltered(pszFolderPath, pDriver->carName, pszSkinFileExt);
		
	if (pSkinFileList)
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
				strlen(pCurSkinFile->name) - strlen(pDriver->carName) - 1 - strlen(pszSkinFileExt);
			std::string strSkinName;
			if (nSkinNameLen > 0)
				strSkinName =
					std::string(pCurSkinFile->name)
					.substr(strlen(pDriver->carName) + 1, nSkinNameLen);
			else // Assuming default/standard "<car name>.png"
				strSkinName = rmdStdSkinName;
			
			// Ignore skins that are already in the list (path search priority).
			if (std::find(lstSkinNames.begin(), lstSkinNames.end(), strSkinName)
				== lstSkinNames.end())
			{
				// Add found skin in the list
				lstSkinNames.push_back(strSkinName);
				
				// Add associated preview image
				// (don't check file existence now, will be needed only at display time).
				std::ostringstream ossPreviewName;
				ossPreviewName << pszFolderPath << '/' << pDriver->carName;
				if (strSkinName != rmdStdSkinName)
					ossPreviewName << '-' << strSkinName;
				ossPreviewName << pszPreviewFileSuffix;
				//if (!stat(ossPreviewName.str().c_str(), &st))
				{
					lstPreviewFiles.push_back(ossPreviewName.str());
					GfOut("* found skin=%s, preview=%s\n",
						  strSkinName.c_str(), ossPreviewName.str().c_str());
				}
// 				else
// 					GfError("Ignoring '%s' skin for %s/%d/%s because preview file %s not found\n",
// 							strSkinName.c_str(), pDriver->moduleName, pDriver->interfaceIndex,
// 							pDriver->carName, ossPreviewName.str().c_str());
			}
				
		} while (pCurSkinFile != pSkinFileList);
		
	GfDirFreeList(pSkinFileList, NULL);
}

void rmdGetCarSkinsInSearchPath(const trmdDrvElt *pDriver,
								std::vector<std::string>& lstSkinNames,
								std::vector<std::string>& lstPreviewFiles)
{
	std::ostringstream ossDirPath;
	std::string strPreviewName;

	GfOut("rmdGetCarSkinsInSearchPath : module=%s, idx=%d, car=%s ...\n",
		  pDriver->moduleName, pDriver->interfaceIndex, pDriver->carName);

	// Clear the skin and preview lists.
	lstSkinNames.clear();
	lstPreviewFiles.clear();

	// Get skins/previews from the directories in the search path
	// (WARNING: Must be consistent with the search path passed to ssgTexturePath in grcar.cpp,
	//           at least for the car skin file search).
	ossDirPath.str("");
	ossDirPath << GetLocalDir() << "drivers/" << pDriver->moduleName
			   << '/' << pDriver->carName;
	rmdGetCarSkinsInFolder(pDriver, ossDirPath.str().c_str(),
						   lstSkinNames, lstPreviewFiles);

	ossDirPath.str("");
	ossDirPath << "drivers/" << pDriver->moduleName
			   << '/' << pDriver->interfaceIndex << '/' << pDriver->carName;
	rmdGetCarSkinsInFolder(pDriver, ossDirPath.str().c_str(),
						   lstSkinNames, lstPreviewFiles);

	ossDirPath.str("");
	ossDirPath << "drivers/" << pDriver->moduleName
			   << '/' << pDriver->interfaceIndex;
	rmdGetCarSkinsInFolder(pDriver, ossDirPath.str().c_str(),
						   lstSkinNames, lstPreviewFiles);

	ossDirPath.str("");
	ossDirPath << "drivers/" << pDriver->moduleName
			   << '/' << pDriver->carName;
	rmdGetCarSkinsInFolder(pDriver, ossDirPath.str().c_str(),
						   lstSkinNames, lstPreviewFiles);

	ossDirPath.str("");
	ossDirPath << "drivers/" << pDriver->moduleName;
	rmdGetCarSkinsInFolder(pDriver, ossDirPath.str().c_str(),
						   lstSkinNames, lstPreviewFiles);

	ossDirPath.str("");
	ossDirPath << "cars/" << pDriver->carName;
	rmdGetCarSkinsInFolder(pDriver, ossDirPath.str().c_str(),
						   lstSkinNames, lstPreviewFiles);

// 	// If no skin was found, add the car's standard one, should always be there !
// 	if (lstSkinNames.empty())
// 	{
// 		// Skin.
// 		lstSkinNames.push_back(rmdStdSkinName);
		
// 		// Associated preview image.
// 		std::ostringstream ossPreviewName;
// 		ossPreviewName << "cars/" << pDriver->carName << '/' << pDriver->carName
// 					   << pszPreviewFileSuffix;
// 		lstPreviewFiles.push_back(ossPreviewName.str());
		
// 		GfOut("... skin=<%s>, preview=%s\n",
// 			  rmdStdSkinName, ossPreviewName.str().c_str());
// 	}
	
	// If we have at least 1 skin, make sure that if the standard one is inside,
	// it is the first one.
	if (!lstSkinNames.empty())
	{
		std::vector<std::string>::iterator iterSkin =
			std::find(lstSkinNames.begin(), lstSkinNames.end(), rmdStdSkinName);
		if (iterSkin != lstSkinNames.end() && iterSkin != lstSkinNames.begin())
		{
			std::vector<std::string>::iterator iterPreview =
				lstPreviewFiles.begin() + (iterSkin - lstSkinNames.begin());
			strPreviewName = *iterPreview;
			lstSkinNames.erase(iterSkin);
			lstPreviewFiles.erase(iterPreview);
			
			lstSkinNames.insert(lstSkinNames.begin(), rmdStdSkinName);
			lstPreviewFiles.insert(lstPreviewFiles.begin(), strPreviewName);
		}
	}
}
#endif
