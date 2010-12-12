/***************************************************************************

    file                 : racemanagers.cpp
    created              : December 2010
    copyright            : (C) 2010 Jean-Philippe Meuret
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

#include <raceman.h>

#include "racemanagers.h"


struct Private
{
	// One GfRaceManager structure for each car (no special order).
	std::vector<GfRaceManager> vecRaceMans;

	// Map for quick access to GfRaceManager by id
	std::map<std::string, GfRaceManager*> mapRaceMansById;

	// Vector of type names, ordered by decreasing priority.
	std::vector<std::string> vecTypes;
};


GfRaceManagers* GfRaceManagers::_pSelf = 0;

GfRaceManagers *GfRaceManagers::self()
{
	if (!_pSelf)
		_pSelf = new GfRaceManagers;
	
	return _pSelf;
}

static bool hasHigherPriority(const GfRaceManager& left, const GfRaceManager& right)
{
	return left.getPriority() > right.getPriority();
}
	
GfRaceManagers::GfRaceManagers()
{
	_pPrivate = new Private;

	// Get the list of racemans from the xml files available in GetDataDir()/config/raceman folder
	tFList* lstFiles = GfDirGetListFiltered("config/raceman", "", PARAMEXT);
	if (!lstFiles)
	{
		GfLogFatal("No race manager available in %sconfig/raceman\n", GetDataDir());
		return;
	}

	// For each found XML file :
	tFList* pFile = lstFiles;
	do 
	{
		GfLogDebug("GfRaceManagers::GfRaceManagers() : Examining %s\n", pFile->name);
		
		// Open the XML descriptor file (look first in user settings, then in the install folder).
		std::ostringstream ossRaceManFileName;
		ossRaceManFileName << GetLocalDir() << "config/raceman/" << pFile->name;
		void* hparmRaceMan = GfParmReadFile(ossRaceManFileName.str().c_str(), GFPARM_RMODE_STD);
		if (!hparmRaceMan)
		{
			ossRaceManFileName.str("");
			ossRaceManFileName << "config/raceman/" << pFile->name;
			hparmRaceMan = GfParmReadFile(ossRaceManFileName.str().c_str(), GFPARM_RMODE_STD);

			// We got if from the data folder : write it to the user settings.
			if (hparmRaceMan)
			{
				ossRaceManFileName.str("");
				ossRaceManFileName << GetLocalDir() << "config/raceman/" << pFile->name;
				GfParmWriteFile(ossRaceManFileName.str().c_str(), hparmRaceMan, NULL);
			}
		}
		
		std::string strRaceManId(pFile->name, strlen(pFile->name) - strlen(PARAMEXT));
		if (!hparmRaceMan) {
			GfLogWarning("GfRaceManagers : Ignoring race manager %s (failed to read from config/raceman/%s in %s and %s)\n",
						 strRaceManId.c_str(), pFile->name, GetLocalDir(), GetDataDir());
			continue;
		}

		// Read race manager info and store it in the GfRaceManager structure.
		GfRaceManager raceMan;
		raceMan.setId(strRaceManId);
		raceMan.setName(GfParmGetStr(hparmRaceMan, RM_SECT_HEADER, RM_ATTR_NAME, "<unknown>"));
		raceMan.setType(GfParmGetStr(hparmRaceMan, RM_SECT_HEADER, RM_ATTR_TYPE, "<unknown>"));
		raceMan.setSubType(GfParmGetStr(hparmRaceMan, RM_SECT_HEADER, RM_ATTR_SUBTYPE, ""));
		//raceMan.setDescriptorFileName(ossRaceManFileName.str());
		raceMan.setPriority((int)GfParmGetNum(hparmRaceMan, RM_SECT_HEADER, RM_ATTR_PRIO, NULL, 10000));
		raceMan.setDescriptorHandle(hparmRaceMan);

		// Update the GfRaceManagers singleton.
		_pPrivate->vecRaceMans.push_back(raceMan);
		_pPrivate->mapRaceMansById[strRaceManId] = &_pPrivate->vecRaceMans.back();
		if (std::find(_pPrivate->vecTypes.begin(), _pPrivate->vecTypes.end(), raceMan.getType())
			== _pPrivate->vecTypes.end())
			_pPrivate->vecTypes.push_back(raceMan.getType());
	} 
	while ((pFile = pFile->next) != lstFiles);
	
	GfDirFreeList(lstFiles, NULL, true, true);

	// Sort the race manager vector by priority.
	std::sort(_pPrivate->vecRaceMans.begin(), _pPrivate->vecRaceMans.end(), hasHigherPriority);

	print();
}

const std::vector<std::string>& GfRaceManagers::getTypes() const
{
	return _pPrivate->vecTypes;
}

GfRaceManager* GfRaceManagers::getRaceManager(const std::string& strId) const
{
	std::map<std::string, GfRaceManager*>::const_iterator itRaceMan =
		_pPrivate->mapRaceMansById.find(strId);
	if (itRaceMan != _pPrivate->mapRaceMansById.end())
		return itRaceMan->second;
	
	return 0;
}

GfRaceManager* GfRaceManagers::getRaceManagerWithName(const std::string& strName) const
{
	std::vector<GfRaceManager>::iterator itRaceMan;
	for (itRaceMan = _pPrivate->vecRaceMans.begin(); itRaceMan != _pPrivate->vecRaceMans.end(); itRaceMan++)
		if (itRaceMan->getName() == strName)
			return &(*itRaceMan);

	return 0;
}

const std::vector<GfRaceManager*> GfRaceManagers::getRaceManagersWithType(const std::string& strType) const
{
	std::vector<GfRaceManager*> vecRaceMans;

	std::vector<GfRaceManager>::iterator itRaceMan;
	for (itRaceMan = _pPrivate->vecRaceMans.begin(); itRaceMan != _pPrivate->vecRaceMans.end(); itRaceMan++)
		if (strType.empty() || itRaceMan->getType() == strType)
			vecRaceMans.push_back(&(*itRaceMan));

	return vecRaceMans;
}

void GfRaceManagers::print() const
{
	GfLogDebug("GfRaceManagers : %d race managers, %d types\n",
			   _pPrivate->vecRaceMans.size(), _pPrivate->vecTypes.size());
	std::vector<std::string>::const_iterator itType;
	for (itType = _pPrivate->vecTypes.begin();
		 itType != _pPrivate->vecTypes.end(); itType++)
	{
		GfLogDebug("  %s type :\n", itType->c_str());
		const std::vector<GfRaceManager*> vecRaceMans =
			getRaceManagersWithType(itType->c_str());
		std::vector<GfRaceManager*>::const_iterator itRaceMan;
		for (itRaceMan = vecRaceMans.begin(); itRaceMan != vecRaceMans.end(); itRaceMan++)
			GfLogDebug("    %s : subtype='%s', name='%s'\n", (*itRaceMan)->getId().c_str(),
					   (*itRaceMan)->getSubType().c_str(), (*itRaceMan)->getName().c_str());
	}
}
