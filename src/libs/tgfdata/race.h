/***************************************************************************

    file                 : race.h
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
 
/** @file   
		Singleton holding information on a race (mainly the starting grid)
    @defgroup	tgfdata	Data manager for the client gaming framework.
*/


#ifndef __TGFRACE_H__
#define __TGFRACE_H__

#include <string>
#include <vector>

#include "tgfdata.h"


class GfDriver;


class TGFDATA_API GfRace
{
public:

	GfRace(void* hparmRace = 0);

	//! Load from the race params.
	void load(void* hparmRace);

	//! Clear the race.
	void clear();
	
	//! Save to the race params.
	void save();
	
	unsigned getCompetitorsCount() const;
	const std::vector<GfDriver*>& getCompetitors() const;
	bool acceptsMoreCompetitors() const;
	bool appendCompetitor(GfDriver* pComp);
	bool removeCompetitor(GfDriver* pComp);
	bool removeAllCompetitors();
	bool shuffleCompetitors();

 	GfDriver* getCompetitor(const std::string& strModName, int nItfIndex) const;
//	GfDriver* getCompetitorWithName(const std::string& strName) const;

// 	const std::string& getCompetitorName(const std::string& strId) const;
// 	std::vector<GfDriver*> getCompetitorsInCategory(const std::string& strCatId = "") const;
// 	std::vector<std::string> getCompetitorIdsInCategory(const std::string& strCatId = "") const;
// 	std::vector<std::string> getCompetitorIdsInCategory(const std::string& strCatId = "") const;
	
// 	void print() const;

protected:
	
	// Its private data.
	class Private;
	Private* _pPrivate;
};

#endif /* __TGFRACE_H__ */

