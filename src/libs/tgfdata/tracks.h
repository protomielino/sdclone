/***************************************************************************

    file                 : tracks.h
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
 
/**
    @file   
		Singleton holding information on the available tracks and categories.
    @defgroup	tgfclientdata	Data manager for the client gaming framework.
*/

#ifndef __TGFTRACKS_H__
#define __TGFTRACKS_H__

#include <string>
#include <vector>

#include "tgfdata.h"


// Information on one track.
class TGFDATA_API GfTrack
{
public:

	const std::string& getId() const { return _strId; };
	const std::string& getName() const { return _strName; };
	const std::string& getCategoryId() const { return _strCatId; };
	const std::string& getDescriptorFileName() const { return _strDescFile; };
	const std::string& getOutlineFileName() const { return _strOutlineFile; };
	const std::string& getPreviewFileName() const { return _strPreviewFile; };
	
	void setId(const std::string& strId) { _strId = strId; };
	void setName(const std::string& strName) { _strName = strName; };
	void setCategoryId(const std::string& strCatId) { _strCatId = strCatId; };
	void setDescriptorFileName(const std::string& strDescFile) { _strDescFile = strDescFile; };
	void setOutlineFileName(const std::string& strOutlineFile) { _strOutlineFile = strOutlineFile; };
	void setPreviewFileName(const std::string& strPreviewFile) { _strPreviewFile = strPreviewFile; };
	
protected:
	
	std::string _strId;          // XML file / folder name (ex: "goldstone-sand")
 	std::string _strName;        // User friendly name (ex: "Goldstone Sand").
	std::string _strCatId;       // Category XML file / folder name (ex: "circuit").
	std::string _strDescFile;    // Path-name of the XML descriptor file.
	std::string _strOutlineFile; // Path-name of the outline image file.
	std::string _strPreviewFile; // Path-name of the preview image file.
};


// Information on all the available tracks (singleton pattern).
class TGFDATA_API GfTracks
{
public:

	// Accessor to the unique instance of the singleton.
	static GfTracks* self();
	
 	const std::vector<std::string>& getCategoryIds() const;

 	GfTrack* getTrack(const std::string& strId) const;
 	GfTrack* getTrackWithName(const std::string& strName) const;

 	std::vector<GfTrack*> getTracksInCategory(const std::string& strCatId = "") const;
 	std::vector<std::string> getTrackIdsInCategory(const std::string& strCatId = "") const;
 	std::vector<std::string> getTrackNamesInCategory(const std::string& strCatId = "") const;
	
 	void print() const;

protected:

	// Protected constructor and destructor : clients can not use them.
	GfTracks();
	~GfTracks();
	
protected:

	// The singleton itself.
	static GfTracks* _pSelf;

	// Its private data.
	class Private;
	Private* _pPrivate;
};

#endif /* __TGFTRACKS_H__ */

