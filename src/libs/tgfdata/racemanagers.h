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


/** @file   
    		Singleton holding information on the available race managers
*/

class TGFDATA_API GfRaceManager
{
public:
	
	const std::string& getId() const { return _strId; }; // TODO: => file name ?
	const std::string& getName() const { return _strName; };
	const std::string& getType() const { return _strType; };
	const std::string& getSubType() const { return _strSubType; };
	const int getPriority() const { return _nPriority; };
	//const std::string& getDescriptorFileName() const { return _strDescFile; }; // 
	void* getDescriptorHandle() const { return _hparmHandle; }; // 

	void setId(const std::string& strId) { _strId = strId; };
	void setName(const std::string& strName) { _strName = strName; };
	void setType(const std::string& strType) { _strType = strType ; };
	void setSubType(const std::string& strSubType) { _strSubType = strSubType; };
	void setPriority(int nPriority) { _nPriority = nPriority; };
	//void setDescriptorFileName(const std::string& strDescFile) { _strDescFile = strDescFile; };
	void setDescriptorHandle(void* hparmHandle) { _hparmHandle = hparmHandle; };
	
protected:
	
	std::string _strId; // XML file name (ex: quickrace, singleevent-endurance, championship-sc)
	std::string _strName; // User friendly full name (ex: Quick Race, Supercar Championship).
	std::string _strType; // User friendly type name (ex: Quick Race, Single Event, Championship).
	std::string _strSubType; // User friendly sub-type name (ex: "", Endurance, Challenge, Supercars").
	std::string _strDescFile; // Path-name of the XML descriptor file.
	int         _nPriority; // Gives the order of the buttons in the race select menu
	void*       _hparmHandle; // Params handle to the descriptor file.
};

class TGFDATA_API GfRaceManagers
{
public:

	// Accessor to the unique instance of the singleton.
	static GfRaceManagers* self();
	
	const std::vector<std::string>& getTypes() const;

	GfRaceManager* getRaceManager(const std::string& strId) const;
	GfRaceManager* getRaceManagerWithName(const std::string& strName) const;

	const std::vector<GfRaceManager*> getRaceManagersWithType(const std::string& strType = "") const;
	
	void print() const;

protected:

	// Protected constructor : clients cannot use it (singleton pattern).
	GfRaceManagers();
	
protected:

	// The singleton itself.
	static GfRaceManagers* _pSelf;

	// Its private data.
	struct Private* _pPrivate;
};

#endif /* __TGFRACEMANAGERS__H__ */
