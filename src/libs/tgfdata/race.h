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


class GfRaceManager;
class GfDriver;
class GfTrack;


class TGFDATA_API GfRace
{
public:

	//! Constructor.
	GfRace();

	//! Load from the given race manager params and results file if specified.
	void load(GfRaceManager* pRaceMan, void* hparmResults = 0);

	//! Clear the race.
	void clear();
	
	//! Store to the race manager params file.
	void store();

	GfRaceManager* getManager() const;

	const std::string& getSessionName() const;

	enum EDisplayMode { eDisplayNormal, eDisplayResultsOnly,
						nDisplayModeNumber };
	enum ETimeOfDaySpec { eTimeDawn, eTimeMorning, eTimeNoon, eTimeAfternoon,
						  eTimeDusk, eTimeNight, eTimeNow, eTimeFromTrack,
						  nTimeSpecNumber };
	enum ECloudsSpec { eCloudsNone, eCloudsFew, eCloudsScarce, eCloudsMany, eCloudsFull,
					   nCloudsSpecNumber};
	enum ERainSpec { eRainNone, eRainLittle, eRainMedium, eRainHeavy, eRainRandom,
					 nRainSpecNumber };
	class Parameters
	{
	  public:
		int nLaps;
		int nDistance; // km
		int nDuration; // s
		EDisplayMode eDisplayMode;
		ETimeOfDaySpec eTimeOfDaySpec;
		ECloudsSpec eCloudsSpec;
		ERainSpec eRainSpec;
	};
	
	Parameters* getParameters();

	int getSupportedFeatures() const;

	bool acceptsDriverType(const std::string& strType) const;
	const std::vector<std::string>& getAcceptedDriverTypes() const;
	bool acceptsCarCategory(const std::string& strCatId) const;
	const std::vector<std::string>& getAcceptedCarCategoryIds() const;
	
	unsigned getCompetitorsCount() const;
	const std::vector<GfDriver*>& getCompetitors() const;
	bool acceptsMoreCompetitors() const;
	bool appendCompetitor(GfDriver* pComp);
	bool removeCompetitor(GfDriver* pComp);
	bool moveCompetitor(GfDriver* pComp, int nDeltaPlace);
	bool removeAllCompetitors();
	bool shuffleCompetitors();

 	GfDriver* getCompetitor(const std::string& strModName, int nItfIndex) const;

	bool isCompetitorFocused(const GfDriver* pComp) const;
	GfDriver* getFocusedCompetitor() const;
	void setFocusedCompetitor(const GfDriver* pComp);

	GfTrack* getTrack() const;

	void* getResultsDescriptorHandle() const;
	
 	void print() const;

protected:
	
	// Its private data.
	class Private;
	Private* _pPrivate;
};

#endif /* __TGFRACE_H__ */

