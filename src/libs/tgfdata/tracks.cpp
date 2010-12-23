/***************************************************************************

    file                 : tracks.cpp
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

#include <map>
#include <sstream>
#include <algorithm>

#include <tgf.h>
#include <track.h>

#include "tracks.h"

				  
// Private data for GfTracks				  
class GfTracks::Private
{
public:
	
	// One GfTrack structure for each track (order = sorted directory one).
	std::vector<GfTrack*> vecTracks;

	// Map for quick access to GfTrack by id
	std::map<std::string, GfTrack*> mapTracksById;

	// Set of category Ids.
	std::vector<std::string> vecCatIds;
};


GfTracks* GfTracks::_pSelf = 0;

GfTracks *GfTracks::self()
{
	if (!_pSelf)
		_pSelf = new GfTracks;
	
	return _pSelf;
}

GfTracks::~GfTracks()
{
	std::vector<GfTrack*>::const_iterator itTrack;
	for (itTrack = _pPrivate->vecTracks.begin();
		 itTrack != _pPrivate->vecTracks.end(); itTrack++)
		delete *itTrack;
}

GfTracks::GfTracks()
{
	_pPrivate = new GfTracks::Private;

	// Get the list of sub-dirs in the "tracks" folder (the track categories).
	tFList* lstCatFolders = GfDirGetList("tracks");
	if (!lstCatFolders)
	{
		GfLogFatal("No track category available in the 'tracks' folder\n");
		return;
	}
	
	tFList* pCatFolder = lstCatFolders;
	do 
	{
		//GfLogDebug("GfTracks::GfTracks() : Examining category %s\n", pCatFolder->name);
		
		// Ignore "." and ".." folders.
		const char* pszCatId = pCatFolder->name;
		if (pszCatId[0] == '.') 
			continue;
			
		// Get the list of sub-dirs in the "tracks" folder (the track categories).
		tFList* lstTrackFolders = GfDirGetList("tracks");
		if (!lstTrackFolders)
		{
			GfLogFatal("No category available in the 'tracks' folder\n");
			return;
		}

		// Add new category.
		_pPrivate->vecCatIds.push_back(pszCatId);

		// Look at the tracks in this category.
		tFList* pTrackFolder = lstTrackFolders;
		do 
		{
			//GfLogDebug("GfTracks::GfTracks() : Examining track %s\n", pTrackFolder->name);
		
			// Open the XML file of the car.
			const char* pszTrackId = pTrackFolder->name;
			
			std::ostringstream ossFileName;
			ossFileName << "tracks/" << pszCatId << '/' << pszTrackId
							 << '/' << pszTrackId << PARAMEXT;
			const std::string strTrackFileName(ossFileName.str());
			void* hparmTrack = GfParmReadFile(strTrackFileName.c_str(), GFPARM_RMODE_STD);
			if (!hparmTrack)
			{
				GfLogWarning("GfTracks : Ignoring track %s (failed to read from %s)\n",
							 pszTrackId, strTrackFileName.c_str());
				continue;
			}

			// Read track info.
			ossFileName.str("");
			ossFileName << "tracks/" << pszCatId << '/' << pszTrackId
						<< '/' << pszTrackId << ".jpg";
			std::string strPreviewFileName(ossFileName.str());
			if (!GfFileExists(strPreviewFileName.c_str()))
			{
				ossFileName.str("");
				ossFileName << "tracks/" << pszCatId << '/' << pszTrackId
							<< '/' << pszTrackId << ".png";
				strPreviewFileName = ossFileName.str();
			}
			if (!GfFileExists(strPreviewFileName.c_str()))
				strPreviewFileName = "data/img/splash-racemanmenu.jpg";
			
			ossFileName.str("");
			ossFileName << "tracks/" << pszCatId << '/' << pszTrackId << '/' << "outline.png";
			std::string strOutlineFileName(ossFileName.str());
			if (!GfFileExists(strOutlineFileName.c_str()))
				strPreviewFileName = "data/img/transparent.png";
			
			// Store track info in the GfTrack structure.
			GfTrack* pTrack = new GfTrack;
			pTrack->setId(pszTrackId);
			pTrack->setCategoryId(pszCatId);
			pTrack->setName(GfParmGetStr(hparmTrack, TRK_SECT_HDR, TRK_ATT_NAME, pszTrackId));
			pTrack->setDescriptorFileName(strTrackFileName);
			pTrack->setPreviewFileName(strPreviewFileName);
			pTrack->setOutlineFileName(strOutlineFileName);

			// Update the GfTracks singleton.
			_pPrivate->vecTracks.push_back(pTrack);
			_pPrivate->mapTracksById[pszTrackId] = pTrack;

			// Close the XML file of the track.
			GfParmReleaseHandle(hparmTrack);
		}
		while ((pTrackFolder = pTrackFolder->next) != lstTrackFolders);
	} 
	while ((pCatFolder = pCatFolder->next) != lstCatFolders);
	
	GfDirFreeList(lstCatFolders, NULL, true, true);
	
	// Sort the car category ids and driver types vectors.
	std::sort(_pPrivate->vecCatIds.begin(), _pPrivate->vecCatIds.end());

	// Trace what we got.
	print();
}

const std::vector<std::string>& GfTracks::getCategoryIds() const
{
	return _pPrivate->vecCatIds;
}

GfTrack* GfTracks::getTrack(const std::string& strId) const
{
	std::map<std::string, GfTrack*>::iterator itTrack =
		_pPrivate->mapTracksById.find(strId);
	if (itTrack != _pPrivate->mapTracksById.end())
		return itTrack->second;
	
	return 0;
}

GfTrack* GfTracks::getTrackWithName(const std::string& strName) const
{
	std::vector<GfTrack*>::iterator itTrack;
	for (itTrack = _pPrivate->vecTracks.begin(); itTrack != _pPrivate->vecTracks.end(); itTrack++)
		if ((*itTrack)->getName() == strName)
			return *itTrack;

	return 0;
}

std::vector<GfTrack*> GfTracks::getTracksInCategory(const std::string& strCatId) const
{
	std::vector<GfTrack*> vecTracksInCat;

	std::vector<GfTrack*>::iterator itTrack;
	for (itTrack = _pPrivate->vecTracks.begin(); itTrack != _pPrivate->vecTracks.end(); itTrack++)
		if (strCatId.empty() || (*itTrack)->getCategoryId() == strCatId)
			vecTracksInCat.push_back(*itTrack);

	return vecTracksInCat;
}

std::vector<std::string> GfTracks::getTrackIdsInCategory(const std::string& strCatId) const
{
	std::vector<std::string> vecTrackIds;

	std::vector<GfTrack*>::const_iterator itTrack;
	for (itTrack = _pPrivate->vecTracks.begin(); itTrack != _pPrivate->vecTracks.end(); itTrack++)
		if (strCatId.empty() || (*itTrack)->getCategoryId() == strCatId)
			vecTrackIds.push_back((*itTrack)->getId());

	return vecTrackIds;
}

std::vector<std::string> GfTracks::getTrackNamesInCategory(const std::string& strCatId) const
{
	std::vector<std::string> vecTrackNames;

	std::vector<GfTrack*>::const_iterator itTrack;
	for (itTrack = _pPrivate->vecTracks.begin(); itTrack != _pPrivate->vecTracks.end(); itTrack++)
		if (strCatId.empty() || (*itTrack)->getCategoryId() == strCatId)
			vecTrackNames.push_back((*itTrack)->getName());

	return vecTrackNames;
}

void GfTracks::print() const
{
	GfLogTrace("Track base : %d categories, %d tracks\n",
			   _pPrivate->vecCatIds.size(), _pPrivate->vecTracks.size());
	std::vector<std::string>::const_iterator itCat;
	for (itCat = _pPrivate->vecCatIds.begin(); itCat != _pPrivate->vecCatIds.end(); itCat++)
	{
		GfLogTrace("  '%s' category :\n", itCat->c_str());
		const std::vector<GfTrack*> vecTracksInCat = getTracksInCategory(*itCat);
		std::vector<GfTrack*>::const_iterator itTrack;
		for (itTrack = vecTracksInCat.begin(); itTrack != vecTracksInCat.end(); itTrack++)
			GfLogTrace("    %-22s : %s\n", (*itTrack)->getName().c_str(),
					   (*itTrack)->getDescriptorFileName().c_str());
	}
}
