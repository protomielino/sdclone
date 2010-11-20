/***************************************************************************

    file                 : driver.h
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
 
/**
    @defgroup	racemantools	Tools for race managers.
    This is a collection of useful functions for managing drivers.
*/

#ifndef __DRIVER_H__
#define __DRIVER_H__

#include <string>
#include <vector>
#include <map>

#include <tgf.h>
#include <carinfo.h>


// Driver description
typedef struct rmdDrvElt
{
    char	*moduleName;	// Module name
    int		interfaceIndex;	// Index of associated interface in the module
    char	*name;		// Driver name
    int		isSelected;	// Selected for race ?
    int		isHuman;	// Human driver ?
    char	*carName;		// Car XML file / folder name
    char	*carRealName;	// Car user-friendly name
    char	*carCategory;	// Car category XML file / folder name
	int		skinTargets;	// Skin targets bit-field (see car.h for possible values)   
    char	*skinName;		// Skin name (or 0 if standard skin)
    GF_TAILQ_ENTRY(struct rmdDrvElt)	link;
} trmdDrvElt;

//! Standard skin name.
extern const char* rmdStdSkinName;

//! Determine the driver type from its module name.
extern void rmdGetDriverType(const char* moduleName, char* driverType, size_t maxSize);

//! Tell if the given driven matches the giver filters on type and car category. 
extern int rmdDriverMatchesFilters(const trmdDrvElt *drv, const char* carCat, const char* drvTyp,
								   const char* anyCarCat, const char* anyDrvTyp);

//! Retrieve the skins and associated preview images found in the given folder for the given car.
extern void rmdGetCarSkinsInFolder(const char* pszCarName, const char* pszFolderPath,
								   std::vector<std::string>& vecSkinNames,
								   std::map<std::string, int>& mapSkinTargets,
								   std::map<std::string, std::string>& mapPreviewFiles);

//! Retrieve the skins and associated preview images found in the search path for the given driver (use pszForcedCarName in place of pDriver->carName if not null).
extern void rmdGetCarSkinsInSearchPath(const trmdDrvElt *pDriver, const char* pszForcedCarName,
									   std::vector<std::string>& vecSkinNames,
									   std::map<std::string, int>& mapSkinTargets,
									   std::map<std::string, std::string>& mapPreviewFiles);

#endif /* __DRIVER_H__ */

