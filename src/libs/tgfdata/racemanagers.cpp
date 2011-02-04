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

#include "tracks.h"
#include "racemanagers.h"


class GfRaceManagers::Private
{
public:
	
	// One GfRaceManager structure for each car (no special order).
	std::vector<GfRaceManager*> vecRaceMans;

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

GfRaceManagers::~GfRaceManagers()
{
	std::vector<GfRaceManager*>::const_iterator itRaceMan;
	for (itRaceMan = _pPrivate->vecRaceMans.begin();
		 itRaceMan != _pPrivate->vecRaceMans.end(); itRaceMan++)
		delete *itRaceMan;
}

static bool hasHigherPriority(const GfRaceManager* pLeft, const GfRaceManager* pRight)
{
	return pLeft->getPriority() > pRight->getPriority();
}
	
GfRaceManagers::GfRaceManagers()
{
	_pPrivate = new Private;

	// Get the list of racemans from the xml files available in GfDataDir()/config/raceman folder
	tFList* lstFiles = GfDirGetListFiltered("config/raceman", "", PARAMEXT);
	if (!lstFiles)
	{
		GfLogFatal("No race manager available in %sconfig/raceman\n", GfDataDir());
		return;
	}

	// For each found XML file :
	tFList* pFile = lstFiles;
	do 
	{
		// Open the XML descriptor file (look first in user settings, then in the install folder).
		std::ostringstream ossRaceManFileName;
		ossRaceManFileName << GfLocalDir() << "config/raceman/" << pFile->name;
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
				ossRaceManFileName << GfLocalDir() << "config/raceman/" << pFile->name;
				GfParmWriteFile(ossRaceManFileName.str().c_str(), hparmRaceMan, NULL);
			}
		}
		
		std::string strRaceManId(pFile->name);
		strRaceManId.erase(strlen(pFile->name) - strlen(PARAMEXT));
		if (!hparmRaceMan)
		{
			GfLogWarning("GfRaceManagers : Ignoring race manager %s (failed to read from config/raceman/%s in %s and %s)\n",
						 strRaceManId.c_str(), pFile->name, GfLocalDir(), GfDataDir());
			continue;
		}

		// Create the race manager and load it from the params file.
		GfRaceManager* pRaceMan = new GfRaceManager(strRaceManId, hparmRaceMan);

		// Update the GfRaceManagers singleton.
		_pPrivate->vecRaceMans.push_back(pRaceMan);
		_pPrivate->mapRaceMansById[strRaceManId] = pRaceMan;
		if (std::find(_pPrivate->vecTypes.begin(), _pPrivate->vecTypes.end(), pRaceMan->getType())
			== _pPrivate->vecTypes.end())
			_pPrivate->vecTypes.push_back(pRaceMan->getType());
	} 
	while ((pFile = pFile->next) != lstFiles);
	
	GfDirFreeList(lstFiles, NULL, true, true);

	// Sort the race manager vector by priority.
	std::sort(_pPrivate->vecRaceMans.begin(), _pPrivate->vecRaceMans.end(), hasHigherPriority);

	// And log what we've got now.
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
	std::vector<GfRaceManager*>::iterator itRaceMan;
	for (itRaceMan = _pPrivate->vecRaceMans.begin();
		 itRaceMan != _pPrivate->vecRaceMans.end(); itRaceMan++)
		if ((*itRaceMan)->getName() == strName)
			return *itRaceMan;

	return 0;
}

std::vector<GfRaceManager*> GfRaceManagers::getRaceManagersWithType(const std::string& strType) const
{
	std::vector<GfRaceManager*> vecRaceMans;

	std::vector<GfRaceManager*>::iterator itRaceMan;
	for (itRaceMan = _pPrivate->vecRaceMans.begin(); itRaceMan != _pPrivate->vecRaceMans.end(); itRaceMan++)
		if (strType.empty() || (*itRaceMan)->getType() == strType)
			vecRaceMans.push_back(*itRaceMan);

	return vecRaceMans;
}

void GfRaceManagers::print() const
{
	GfLogTrace("Race managers : %d types, %d race managers\n",
			   _pPrivate->vecTypes.size(), _pPrivate->vecRaceMans.size());
	std::vector<std::string>::const_iterator itType;
	for (itType = _pPrivate->vecTypes.begin();
		 itType != _pPrivate->vecTypes.end(); itType++)
	{
		GfLogTrace("  %s type :\n", itType->c_str());
		const std::vector<GfRaceManager*> vecRaceMans =
			getRaceManagersWithType(itType->c_str());
		std::vector<GfRaceManager*>::const_iterator itRaceMan;
		for (itRaceMan = vecRaceMans.begin(); itRaceMan != vecRaceMans.end(); itRaceMan++)
			GfLogTrace("    %s : subtype='%s', name='%s', events=%d\n",
					   (*itRaceMan)->getId().c_str(), (*itRaceMan)->getSubType().c_str(),
					   (*itRaceMan)->getName().c_str(), (*itRaceMan)->getEventCount());
	}
}

// GfRaceManager class ---------------------------------------------------------------

GfRaceManager::GfRaceManager(const std::string& strId, void* hparmHandle)
{
	_strId = strId;

	// Load constant properties (never changed afterwards).
	_strName = GfParmGetStr(hparmHandle, RM_SECT_HEADER, RM_ATTR_NAME, "<unknown>");
	_strType = GfParmGetStr(hparmHandle, RM_SECT_HEADER, RM_ATTR_TYPE, "<unknown>");
	_strSubType = GfParmGetStr(hparmHandle, RM_SECT_HEADER, RM_ATTR_SUBTYPE, "");
	_nPriority = (int)GfParmGetNum(hparmHandle, RM_SECT_HEADER, RM_ATTR_PRIO, NULL, 10000);

	// Load other "mutable" properties.
	reset(hparmHandle, false);
}

void GfRaceManager::reset(void* hparmHandle, bool bClosePrevHdle)
{
	if (bClosePrevHdle && _hparmHandle)
		GfParmReleaseHandle(_hparmHandle);
	_hparmHandle = hparmHandle;

	// Get current event in the schedule.
	_nCurrentEventInd =
		(int)GfParmGetNum(_hparmHandle, RM_SECT_TRACKS, RE_ATTR_CUR_TRACK, NULL, 1) - 1;

	// Load track id for each event in the schedule.
	std::ostringstream ossSectionPath;
	int nEventNum = 1;
	const char* pszTrackId;
	do
	{
		ossSectionPath.str("");
		ossSectionPath << RM_SECT_TRACKS << '/' << nEventNum;
		pszTrackId = GfParmGetStr(_hparmHandle, ossSectionPath.str().c_str(), RM_ATTR_NAME, 0);
		if (pszTrackId)
		{
			_vecEventTrackIds.push_back(pszTrackId);
			nEventNum++;
		}
	}
	while (pszTrackId);
}

void GfRaceManager::save()
{
	if (!_hparmHandle)
		return;

	// Note: No need to save constant properties (never changed).
	
	// Current event in the schedule.
	GfParmSetNum(_hparmHandle, RM_SECT_TRACKS, RE_ATTR_CUR_TRACK, NULL,
				 (tdble)(_nCurrentEventInd + 1));
	
	// Info about each event in the schedule.
	std::ostringstream ossSectionPath;
	for (unsigned nEventInd = 0; nEventInd < _vecEventTrackIds.size(); nEventInd++)
	{
		ossSectionPath.str("");
		ossSectionPath << RM_SECT_TRACKS << '/' << nEventInd + 1;
		GfParmSetStr(_hparmHandle, ossSectionPath.str().c_str(), RM_ATTR_NAME,
					 _vecEventTrackIds[nEventInd].c_str());
		GfTrack* pTrack = GfTracks::self()->getTrack(_vecEventTrackIds[nEventInd]);
		GfParmSetStr(_hparmHandle, ossSectionPath.str().c_str(), RM_ATTR_CATEGORY,
					 pTrack->getCategoryId().c_str());
	}
}

GfRaceManager::~GfRaceManager()
{
	if (_hparmHandle)
		GfParmReleaseHandle(_hparmHandle);
}
			 
const std::string& GfRaceManager::getId() const
{
	return _strId;
}

void* GfRaceManager::getDescriptorHandle() const
{
	return _hparmHandle;
}

std::string GfRaceManager::getDescriptorFileName() const
{
	return const_cast<const char*>(GfParmGetFileName(_hparmHandle));
}

const std::string& GfRaceManager::getName() const
{
	return _strName;
}

const std::string& GfRaceManager::getType() const
{
	return _strType;
}

const std::string& GfRaceManager::getSubType() const
{
	return _strSubType;
}

bool GfRaceManager::isNetwork() const
{
	return _strType == "Online";
}

const int GfRaceManager::getPriority() const
{
	return _nPriority;
}

unsigned GfRaceManager::getEventCount() const
{
	return _vecEventTrackIds.size();
}

bool GfRaceManager::stepToNextEvent()
{
	if (_nCurrentEventInd < (int)_vecEventTrackIds.size() - 1)
	{
		_nCurrentEventInd++;
		return true;
	}

	return false;
}

GfTrack* GfRaceManager::getCurrentEventTrack()
{
	GfTrack* pTrack = 0;
	
	// If the race manager has any event, get the current one track.
	// Note: For the moment, Career raceman has none (but this will probably change, WIP).
	if (_vecEventTrackIds.size() > 0)
		pTrack = GfTracks::self()->getTrack(_vecEventTrackIds[_nCurrentEventInd]);
	
	// If the current event track is not usable, take the first usable one.
	if (!pTrack)
		pTrack = GfTracks::self()->getFirstUsableTrack();
		   
	return pTrack;
}

void GfRaceManager::setCurrentEventTrack(GfTrack* pTrack)
{
	if (!pTrack)
		return;

	_vecEventTrackIds[_nCurrentEventInd] = pTrack->getId();
}
