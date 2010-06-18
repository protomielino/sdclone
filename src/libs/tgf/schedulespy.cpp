/***************************************************************************

    file        : schedulespy.cpp
    author      : Jean-Philippe Meuret (jpmeuret@free.fr)

    A tool to study the way some special code sections (named "events)
    in the program are actually scheduled at a fine grain level.
    Absolute time and duration are logged in memory each time declared sections
    are executed.
    A detailed schedule of these events can be printed to a text file at the end.

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifdef SCHEDULE_SPY

//#include <algorithm>
#include <iostream>
//#include <iomanip>

#include <cstring>

#include "tgf.h"

#include <vector>
#include <map>

class GfScheduleEventLog
{
 public:
	GfScheduleEventLog(unsigned nMaxEvents, double dIgnoreDelay);
	void configure(unsigned nMaxEvents, double dIgnoreDelay);
	void reinit(double dZeroTime);
	void beginEvent();
	void endEvent();
 private:
	double _dZeroTime;
	double _dIgnoreDelay;
	unsigned _nMaxEvents;
	std::vector<float> _vecStartTimes;
	std::vector<float> _vecDurations;
};

class GfScheduleSpy
{
 protected:
	GfScheduleSpy();
 public:
	~GfScheduleSpy();
	static GfScheduleSpy* self();
	void configureEventLog(const char* pszLogName, unsigned nMaxEvents, double dIgnoreDelay);
	void beginSession();
	void beginEvent(const char* pszLogName);
	void endEvent(const char* pszLogName);
	void endSession();
	void printReport(const char* pszFileName, double fTimeResolution);
 private:
	static GfScheduleSpy* _pSelf;
	double _dZeroTime;
	std::map<const char*, GfScheduleEventLog*> _mapEventLogs;
};

// GfScheduleEventLog class implementation //-------------------------------------------------

// 
GfScheduleEventLog::GfScheduleEventLog(unsigned nMaxEvents, double dIgnoreDelay)
{
	configure(nMaxEvents, dIgnoreDelay);
}

// 
void GfScheduleEventLog::configure(unsigned nMaxEvents, double dIgnoreDelay)
{
	_nMaxEvents = nMaxEvents;
	_dIgnoreDelay = dIgnoreDelay;
}

// Precondition : configure()
void GfScheduleEventLog::reinit(double dZeroTime)
{
	_dZeroTime = dZeroTime;
	_vecStartTimes.reserve(_nMaxEvents);
	_vecDurations.reserve(_nMaxEvents);
}

// Precondition : reinit()
void GfScheduleEventLog::beginEvent()
{
	const double dNowTime = GfTimeClock();
	if (dNowTime >= _dZeroTime + _dIgnoreDelay)
		_vecStartTimes.push_back((float)(dNowTime - _dZeroTime));
}

// Precondition : beginEvent()
void GfScheduleEventLog::endEvent()
{
	const double dNowTime = GfTimeClock();
	if (dNowTime >= _dZeroTime + _dIgnoreDelay)
		_vecDurations.push_back((float)(dNowTime - _dZeroTime)
								- _vecStartTimes[_vecStartTimes.size() - 1]);
}

// GfScheduleSpy class implementation //-------------------------------------------------------

GfScheduleSpy* GfScheduleSpy::_pSelf = NULL;

GfScheduleSpy::GfScheduleSpy()
{
}

GfScheduleSpy::~GfScheduleSpy()
{
	_pSelf = 0;
}

GfScheduleSpy* GfScheduleSpy::self()
{
	if (!_pSelf)
		_pSelf = new GfScheduleSpy();

	return _pSelf;
}

// Precondition : beginEvent()
void GfScheduleSpy::configureEventLog(const char* pszLogName,
									  unsigned nMaxEvents, double dIgnoreDelay)
{
	if (_mapEventLogs.find(pszLogName) == _mapEventLogs.end())
		_mapEventLogs[pszLogName] = new GfScheduleEventLog(nMaxEvents, dIgnoreDelay);
	else
		_mapEventLogs[pszLogName]->configure(nMaxEvents, dIgnoreDelay);
}

// Precondition : for all needed event logs, configureEventLog(...)
void GfScheduleSpy::beginSession()
{
	GfOut("Beginning schedule spy session\n");
	
	_dZeroTime = GfTimeClock();
	std::map<const char*, GfScheduleEventLog*>::iterator iterLogs;
	for (iterLogs = _mapEventLogs.begin(); iterLogs != _mapEventLogs.end(); iterLogs++)
		(*iterLogs).second->reinit(_dZeroTime);
}

// Precondition : beginSession()
void GfScheduleSpy::beginEvent(const char* pszLogName)
{
	if (_mapEventLogs.find(pszLogName) != _mapEventLogs.end())
		_mapEventLogs[pszLogName]->beginEvent();
	else
		GfError("GfScheduleSpy : Can't beginEvent in undeclared event log '%s'\n", pszLogName);
}

// Precondition : beginEvent(pszLogName)
void GfScheduleSpy::endEvent(const char* pszLogName)
{
	if (_mapEventLogs.find(pszLogName) != _mapEventLogs.end())
		_mapEventLogs[pszLogName]->endEvent();
	else
		GfError("GfScheduleSpy : Can't endEvent in undeclared event log '%s'\n", pszLogName);
}

// Precondition : beginSession()
void GfScheduleSpy::endSession()
{
	GfOut("Ending schedule spy session\n");
}

// Precondition : endSession()
void GfScheduleSpy::printReport(const char* pszFileName, double fTimeResolution)
{
	// Not yet implemented.
}

#endif // SCHEDULE_SPY


// C interface functions //-----------------------------------------------------------------

/* Configure an event log before using it (can be called more than once to change settings)
   @ingroup schedulespy
   \param pszLogName   Name/id
   \param nMaxEvents   Maximum number of events to be logged (other ignored)
   \param dIgnoreDelay Delay (s) before taking Begin/EndEvent into account (from BeginSession time) */
void GfssConfigureEventLog(const char* pszLogName, unsigned nMaxEvents, double dIgnoreDelay)
{
#ifdef SCHEDULE_SPY
	GfScheduleSpy::self()->configureEventLog(pszLogName, nMaxEvents, dIgnoreDelay);
#endif // SCHEDULE_SPY
}

/* Start a new spying session
   @ingroup schedulespy
   \precondition All event logs must be configured at least once before) */
void GfssBeginSession()
{
#ifdef SCHEDULE_SPY
	GfScheduleSpy::self()->beginSession();
#endif // SCHEDULE_SPY
}

/* Log the beginning of an event (enter the associated code section)
   @ingroup schedulespy
   \precondition BeginSession
   \param pszLogName   Name/id                                        */
void GfssBeginEvent(const char* pszLogName)
{
#ifdef SCHEDULE_SPY
	GfScheduleSpy::self()->beginEvent(pszLogName);
#endif // SCHEDULE_SPY
}

/* Log the end of an event (exit from the associated code section)
   @ingroup schedulespy
   \precondition BeginEvent(pszLogName)
   \param pszLogName   Name/id                                        */
void GfssEndEvent(const char* pszLogName)
{
#ifdef SCHEDULE_SPY
	GfScheduleSpy::self()->endEvent(pszLogName);
#endif // SCHEDULE_SPY
}

/* Terminate the current spying session
   @ingroup schedulespy
   \precondition BeginSession             */
void GfssEndSession()
{
#ifdef SCHEDULE_SPY
	GfScheduleSpy::self()->endSession();
#endif // SCHEDULE_SPY
}

/* Print a table log (2 columns for each event log : start time and duration)
   @ingroup schedulespy
   \param pszFileName      Target text file for the log
   \param fTimeResolution  Time resolution to use
                           (minimum delay between 2 event starts)
   \precondition EndSession                                                     */
void GfssPrintReport(const char* pszFileName, double fTimeResolution)
{
#ifdef SCHEDULE_SPY
	GfScheduleSpy::self()->printReport(pszFileName, fTimeResolution);
#endif // SCHEDULE_SPY
}
