/***************************************************************************

    file                 : racemanagers.h
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

#ifndef __TGFRACEMANAGERS__H__
#define __TGFRACEMANAGERS__H__

#include <string>
#include <vector>

#include "tgfdata.h"

class GfTrack;


/** @file   
    		Singleton holding information on the available race managers
    @defgroup	tgfdata	Data manager for the client gaming framework.
*/

class TGFDATA_API GfRaceManager
{
public:
	
	GfRaceManager(const std::string& strId, void* hparmHandle);
	void reset(void* hparmHandle, bool bClosePrevHdle = false);
	
	~GfRaceManager();

	const std::string& getId() const;
	void* getDescriptorHandle() const;
	std::string getDescriptorFileName() const;

	const std::string& getName() const;
	const std::string& getType() const;
	const std::string& getSubType() const;
	const int getPriority() const;

	unsigned getEventCount() const;
	bool stepToNextEvent();
	GfTrack* getCurrentEventTrack();
	void setCurrentEventTrack(GfTrack* pTrack);

	//! Save data to params (in-memory).
	void save();
	
protected:
	
	std::string _strId; // XML file name (ex: quickrace, singleevent-endurance, championship-sc)
	void*       _hparmHandle; // Params handle to the descriptor file.

	std::string _strName; // User friendly full name (ex: Quick Race, Supercar Championship).
	std::string _strType; // User friendly type name (ex: Quick Race, Single Event, Championship).
	std::string _strSubType; // User friendly sub-type name (ex: "", Endurance, Challenge, Supercars").
	int         _nPriority; // Gives the order of the buttons in the race select menu

	std::vector<std::string> _vecEventTrackIds; // Id of the track for each event.
	int _nCurrentEventInd;
};

class TGFDATA_API GfRaceManagers
{
public:

	// Accessor to the unique instance of the singleton.
	static GfRaceManagers* self();
	
	const std::vector<std::string>& getTypes() const;

	GfRaceManager* getRaceManager(const std::string& strId) const;
	GfRaceManager* getRaceManagerWithName(const std::string& strName) const;

	std::vector<GfRaceManager*> getRaceManagersWithType(const std::string& strType = "") const;
	
	void print() const;

protected:

	// Protected constructor and destructor : clients can not use them.
	GfRaceManagers();
	~GfRaceManagers();
	
protected:

	// The singleton itself.
	static GfRaceManagers* _pSelf;

	// Its private data.
	class Private;
	Private* _pPrivate;
};

#endif /* __TGFRACEMANAGERS__H__ */
