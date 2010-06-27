/***************************************************************************

    file        : schedulespy.cpp
    author      : Jean-Philippe Meuret (jpmeuret@free.fr)

    A tool to study the way some special code sections (named "events" here)
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

#include <vector>
#include <map>
#include <fstream>

#include <portability.h>

#include "tgf.h"


class GfScheduleEventLog
{
 public:
	GfScheduleEventLog(unsigned nMaxEvents, double dIgnoreDelay);
	void configure(unsigned nMaxEvents, double dIgnoreDelay);
	void reinit(double dZeroTime);
	void beginEvent();
	void endEvent();
	unsigned nbEvents() const { return _vecDurations.size(); };
	float startTime(unsigned nEventInd) const { return _vecStartTimes[nEventInd]; };
	float duration(unsigned nEventInd) const { return _vecDurations[nEventInd]; };
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
	_nMaxEvents = nMaxEvents <= _vecStartTimes.max_size() ? nMaxEvents : _vecStartTimes.max_size();
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
	// Ignore events after the _nMaxEvents'th (to avoid vector resize, which is slow).
	if (_vecStartTimes.size() < _vecStartTimes.capacity())
	{
		const double dNowTime = GfTimeClock();
		if (dNowTime >= _dZeroTime + _dIgnoreDelay)
			_vecStartTimes.push_back((float)(dNowTime - _dZeroTime));
	}
}

// Precondition : beginEvent()
void GfScheduleEventLog::endEvent()
{
	// Ignore events after the _nMaxEvents'th (to avoid vector resize, which is slow).
	if (_vecStartTimes.size() < _vecStartTimes.capacity())
	{
		const double dNowTime = GfTimeClock();
		if (dNowTime >= _dZeroTime + _dIgnoreDelay)
			_vecDurations.push_back((float)(dNowTime - _dZeroTime)
									- _vecStartTimes[_vecStartTimes.size() - 1]);
	}
}

// GfScheduleSpy class implementation //-------------------------------------------------------

GfScheduleSpy* GfScheduleSpy::_pSelf = 0;

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
	// Output file :
	// a) Build absolute path (mandatory storage under GetLocalDir()/debug folder.
	static char pszFilePathName[512];
	snprintf(pszFilePathName, 512, "%sdebug/%s", GetLocalDir(), pszFileName);
	pszFilePathName[511] = 0; // Ensure last char is \0 (MS doesn't do).

	// b) Create parent dir(s) if needed.
	char *pDirNameSep = strrchr(pszFilePathName, '/');
	*pDirNameSep = '\0';
	GfCreateDir(pszFilePathName);
	*pDirNameSep = '/';

	// c) Finally open in write mode.
	std::ofstream outFStream(pszFilePathName);
	if (!outFStream)
	{
		GfError("Could not open %s for writing report\n", pszFilePathName);
		return;
	}
	
	static const char* pszEventFormat = "\t%6.4f\t%6.4f"; // Min. length = 14 chars through sprintf.
	static const unsigned nEventStringMaxSize = 32; // More than the 14 chars above, as sprintf can produce more. 
	static const unsigned nLineBufSize = nEventStringMaxSize*_mapEventLogs.size(); 
	char* pszLineBuf = new char[nLineBufSize];
	
	// Initialize the next event index for each log (kind of a cursor in each log).
	std::map<const char*, unsigned> mapNextEventInd;
	std::map<const char*, GfScheduleEventLog*>::const_iterator itLog;
	for (itLog = _mapEventLogs.begin(); itLog != _mapEventLogs.end(); itLog++)
	{
		const char* pszLogName = (*itLog).first;
		mapNextEventInd[pszLogName] = 0;
	}

	// Print columns header.
	for (itLog = _mapEventLogs.begin(); itLog != _mapEventLogs.end(); itLog++)
	{
		const char* pszLogName = (*itLog).first;
		snprintf(pszLineBuf + strlen(pszLineBuf), nEventStringMaxSize, "\t%s\t", pszLogName);
		pszLineBuf[nLineBufSize-1] = 0; // Ensure last char is \0 (MS doesn't do).
	}
	outFStream << pszLineBuf << std::endl;
	
	snprintf(pszLineBuf, 5, "Time");
	for (itLog = _mapEventLogs.begin(); itLog != _mapEventLogs.end(); itLog++)
	{
		const char* pszLogName = (*itLog).first;
		snprintf(pszLineBuf + strlen(pszLineBuf), nEventStringMaxSize, "\tStart\tDuration");
		pszLineBuf[nLineBufSize-1] = 0; // Ensure last char is \0 (MS doesn't do).
	}
	outFStream << pszLineBuf << std::endl;
	
	// For each time step (fTimeResolution), print events info.
	double dRelTime = 0.0;
	bool bEventAreLeft = true;
	while (bEventAreLeft)
	{

		bool bEventAreLeftInStep = true;
		while (bEventAreLeftInStep)
		{
			unsigned nbProcessedEvents = 0;
			snprintf(pszLineBuf, 10, "%6.3f", dRelTime);
			for (itLog = _mapEventLogs.begin(); itLog != _mapEventLogs.end(); itLog++)
			{
				const char* pszLogName = (*itLog).first;
				const GfScheduleEventLog* pEventLog = (*itLog).second;
				const unsigned& nEventInd = mapNextEventInd[pszLogName];
				if (nEventInd < pEventLog->nbEvents()
					&& pEventLog->startTime(nEventInd) >= dRelTime
					&& pEventLog->startTime(nEventInd) < dRelTime + fTimeResolution)
				{
					snprintf(pszLineBuf + strlen(pszLineBuf), nEventStringMaxSize, pszEventFormat,
							 pEventLog->startTime(nEventInd), pEventLog->duration(nEventInd));
					pszLineBuf[nLineBufSize-1] = 0; // Ensure last char is \0 (MS doesn't do).
					mapNextEventInd[pszLogName]++; // Event processed.
					nbProcessedEvents++;
				}
				else
				{
					strncat(pszLineBuf, "\t\t", 2);
				}
			}
			
			// Print report line if any was produced,
			// and check if any event left to process in this time step.
			if (nbProcessedEvents > 0)
			{
				outFStream << pszLineBuf << std::endl;
			}
			else
			{
				bEventAreLeftInStep = false;
			}
		}

		// Check if any event left to process.
		bEventAreLeft = false;
		for (itLog = _mapEventLogs.begin(); itLog != _mapEventLogs.end(); itLog++)
		{
			const char* pszLogName = (*itLog).first;
			const GfScheduleEventLog* pEventLog = (*itLog).second;
			if (mapNextEventInd[pszLogName] < pEventLog->nbEvents())
			{
				bEventAreLeft = true;
				break;
			}
		}
				
		// Next time step.
		dRelTime += fTimeResolution;
	}

	outFStream.close();
	
	delete [] pszLineBuf;
}

// C interface functions //-----------------------------------------------------------------

/* Configure an event log before using it (can be called more than once to change settings)
   @ingroup schedulespy
   \param pszLogName   Name/id
   \param nMaxEvents   Maximum number of events to be logged (other ignored)
   \param dIgnoreDelay Delay (s) before taking Begin/EndEvent into account (from BeginSession time) */
void GfSchedConfigureEventLog(const char* pszLogName, unsigned nMaxEvents, double dIgnoreDelay)
{
	GfScheduleSpy::self()->configureEventLog(pszLogName, nMaxEvents, dIgnoreDelay);
}

/* Start a new spying session
   @ingroup schedulespy
   \precondition All event logs must be configured at least once before) */
void GfSchedBeginSession()
{
	GfScheduleSpy::self()->beginSession();
}

/* Log the beginning of an event (enter the associated code section)
   @ingroup schedulespy
   \precondition BeginSession
   \param pszLogName   Name/id                                        */
void GfSchedBeginEvent(const char* pszLogName)
{
	GfScheduleSpy::self()->beginEvent(pszLogName);
}

/* Log the end of an event (exit from the associated code section)
   @ingroup schedulespy
   \precondition BeginEvent(pszLogName)
   \param pszLogName   Name/id                                        */
void GfSchedEndEvent(const char* pszLogName)
{
	GfScheduleSpy::self()->endEvent(pszLogName);
}

/* Terminate the current spying session
   @ingroup schedulespy
   \precondition BeginSession             */
void GfSchedEndSession()
{
	GfScheduleSpy::self()->endSession();
}

/* Print a table log (2 columns for each event log : start time and duration)
   @ingroup schedulespy
   \param pszFileName      Target text file for the log
   \param fTimeResolution  Time resolution to use
                           (minimum delay between 2 event starts)
   \precondition EndSession                                                     */
void GfSchedPrintReport(const char* pszFileName, double fTimeResolution)
{
	GfScheduleSpy::self()->printReport(pszFileName, fTimeResolution);
}

#endif // SCHEDULE_SPY
