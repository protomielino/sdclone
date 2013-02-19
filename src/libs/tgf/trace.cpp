/***************************************************************************
                                   TRACE                   
                             -------------------                                         
    created              : Fri Aug 13 22:32:45 CEST 1999
    copyright            : (C) 1999 by Eric Espie, 2010 by Jean-Philippe Meuret
    web                  : www.speed-dreams.org
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
    Tracing / logging system
    @version	$Id$
    @ingroup	trace
*/


#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstdarg>
//#include <cassert>
#include <cstring>
#include <ctime>

#include <map>
				  
#ifdef WIN32
#include <windows.h>
#include <windowsx.h>
#else
#include <sys/param.h>
#endif // WIN32

#include "tgf.hpp"


// Level names. Temporary "Optimisation".
const char* GfLogger::astrLevelNames[] = { "Fatal", "Optimisation", "Error", "Warning", "Info", "Trace", "Debug"};

// Logger instances.
std::map<std::string, GfLogger*> gfMapLoggersByName;

// Flag indicating if output is enabled (for all loggers).
bool GfLogger::_bOutputEnabled = false;

// The default logger : created and initialized first of all (see GfLogger::setup() below).
static const char* pszDefLoggerName = "default";
GfLogger* GfPLogDefault = 0;

void
gfTraceInit(bool bWithLogging)
{
    GfLogger::setup(bWithLogging);
}

// GfLogger class implementation ==================================================

GfLogger& GfLogger::instance(const std::string& strName)
{
	// If the specified logger does not exists yet, create it and put it into the map.
	std::map<std::string, GfLogger*>::iterator itLog = gfMapLoggersByName.find(strName);
	if (itLog == gfMapLoggersByName.end())
	{
        // Default settings (null stream if output disabled).
		GfLogger* pLog =
            (_bOutputEnabled ? new GfLogger(strName) : new GfLogger(strName, 0));
		gfMapLoggersByName[strName] = pLog;

		// Get again from the map : should never fail.
		itLog = gfMapLoggersByName.find(strName);
	}

	return *(itLog->second);
}

void GfLogger::setup(bool bWithLogging)
{
    // Save global settings.
    _bOutputEnabled = bWithLogging;

	// Create the "default" logger and pre-initialize it with hard coded default settings
	// (we need it for tracing stuff happening when completing initialization).
	GfPLogDefault = &GfLogger::instance(pszDefLoggerName);

	// Load logger settings and create loggers.
	// TODO.
}

GfLogger::GfLogger()
: _strName(""), _bfHdrCols(0), _pStream(0), _nLvlThresh(0), _bNeedsHeader(true)
{
}

GfLogger::GfLogger(const std::string& strName, FILE* pFile, int nLvlThresh, unsigned bfHdrCols)
: _strName(strName), _bfHdrCols(bfHdrCols), _pStream(pFile), _nLvlThresh(nLvlThresh),
  _bNeedsHeader(true)
{
}

GfLogger::~GfLogger()
{
	// Close stream if needed.
	if (_pStream && _pStream != stderr && _pStream != stdout)
		fclose(_pStream);
}

const std::string& GfLogger::name() const
{
	return _strName;
}

unsigned GfLogger::headerColumns() const
{
	return _bfHdrCols;
}

void GfLogger::setHeaderColumns(unsigned bfHdrCols)
{
	_bfHdrCols = bfHdrCols;
}

FILE* GfLogger::stream() const
{
	return _pStream;
}

void GfLogger::setStream(FILE* pFile)
{
    if (pFile)
    {
		char* pszClock = GfTime2Str(GfTimeClock(), 0, true, 3);
		fprintf(_pStream ? _pStream : stderr,
				"%s Info    New trace stream : %p\n", pszClock, pFile);
		free(pszClock);
		fflush(_pStream ? _pStream : stderr);

		// Close previous stream if needed.
		if (_pStream && _pStream != stderr && _pStream != stdout)
			fclose(_pStream);
		
		_pStream = pFile;
    }
    else
	{
		const int nErrNo = errno;
		char* pszClock = GfTime2Str(GfTimeClock(), 0, true, 3);
		fprintf(_pStream ? _pStream : stderr, 
				"%s Error   GfLogSetStream : Null stream (%s)\n", pszClock, strerror(nErrNo));
		free(pszClock);
		fflush(_pStream ? _pStream : stderr);
	}
	
    if (_pStream)
    {
        // Trace date and time.
		time_t t = time(NULL);
		struct tm *stm = localtime(&t);
		char* pszClock = GfTime2Str(GfTimeClock(), 0, true, 3);
		fprintf(_pStream, "%s Info    Date and time : %4d/%02d/%02d %02d:%02d:%02d\n",
				pszClock, stm->tm_year+1900, stm->tm_mon+1, stm->tm_mday,
				stm->tm_hour, stm->tm_min, stm->tm_sec);
		fprintf(_pStream, "%s Info    Version : %s\n", pszClock, GfApp().version().c_str());

		// Trace current trace level threshold.
		fprintf(_pStream, "%s Info    Current trace level threshold : ", pszClock);
		if (_nLvlThresh >= eFatal && _nLvlThresh <= eDebug)
			fprintf(_pStream, "%s\n", astrLevelNames[_nLvlThresh]);
		else
			fprintf(_pStream, "Level%d\n", _nLvlThresh);

		// That's all.
		fflush(_pStream);
		free(pszClock);
    }
}

void GfLogger::setStream(const std::string& strPathname)
{
	FILE* pFile = fopen(strPathname.c_str(), "w");
    if (pFile)
	{
		char* pszClock = GfTime2Str(GfTimeClock(), 0, true, 3);
		fprintf(_pStream ? _pStream : stderr,
				"%s Info    New trace file : %s\n", pszClock, strPathname.c_str());
		free(pszClock);
		fflush(_pStream ? _pStream : stderr);
		
		setStream(pFile);
	}
	else
	{
		const int nErrNo = errno;
		char* pszClock = GfTime2Str(GfTimeClock(), 0, true, 3);
		fprintf(_pStream ? _pStream : stderr, 
				"%s Error   GfLogger::setStream(%s) : Failed to open file for writing (%s)\n",
				pszClock, strPathname.c_str(), strerror(nErrNo));
		free(pszClock);
		fflush(_pStream ? _pStream : stderr);
	}
}

int GfLogger::levelThreshold() const
{
	return _nLvlThresh;
}

void GfLogger::setLevelThreshold(int nLevel)
{
    _nLvlThresh = nLevel;

    // Trace new trace level threshold.
    if (_pStream)
    {
		char* pszClock = GfTime2Str(GfTimeClock(), 0, true, 3);
		fprintf(_pStream, "%s Info    New trace level threshold : ", pszClock);
		free(pszClock);
		if (_nLvlThresh >= eFatal && _nLvlThresh <= eDebug)
			fprintf(_pStream, "%s\n", astrLevelNames[_nLvlThresh]);
		else
			fprintf(_pStream, "%d\n", _nLvlThresh);
		fflush(_pStream);
    }
}

void GfLogger::fatal(const char *pszFmt, ...)
{
#ifdef TRACE_OUT
    if (_pStream && _nLvlThresh >= eFatal)
    {
		if (_bNeedsHeader)
		{
			char* pszClock = GfTime2Str(GfTimeClock(), 0, true, 3);
            fprintf(_pStream, "%s Fatal   ", pszClock);
			free(pszClock);
		}
        va_list vaArgs;
        va_start(vaArgs, pszFmt);
        vfprintf(_pStream, pszFmt, vaArgs);
        va_end(vaArgs);
		fflush(_pStream);
		_bNeedsHeader = strrchr(pszFmt, '\n') ? true : false;
    }
#endif // TRACE_OUT

#ifdef WIN32
	MessageBox(NULL, "Please contact the maintenance team\n"
			   "and notify them about the error messages in the console",
			   TEXT("Fatal error"), MB_OK|MB_ICONERROR|MB_SETFOREGROUND);
#endif

    ::exit(1);
}

#ifdef TRACE_OUT

// Temporary.
void GfLogger::optim(const char *pszFmt, ...)
{
    if (_pStream && _nLvlThresh >= eOptim)
    {
/*
        if (_bNeedsHeader)
		{
			char* pszClock = GfTime2Str(GfTimeClock(), 0, true, 3);
            fprintf(_pStream, "%s Optimis.", pszClock);
			free(pszClock);
		}
*/
        va_list vaArgs;
        va_start(vaArgs, pszFmt);
        vfprintf(_pStream, pszFmt, vaArgs);
        va_end(vaArgs);
		fflush(_pStream);
//		_bNeedsHeader = strrchr(pszFmt, '\n') ? true : false;
    }
}

void GfLogger::error(const char *pszFmt, ...)
{
    if (_pStream && _nLvlThresh >= eError)
    {
        if (_bNeedsHeader)
		{
			char* pszClock = GfTime2Str(GfTimeClock(), 0, true, 3);
            fprintf(_pStream, "%s Error   ", pszClock);
			free(pszClock);
		}
        va_list vaArgs;
        va_start(vaArgs, pszFmt);
        vfprintf(_pStream, pszFmt, vaArgs);
        va_end(vaArgs);
		fflush(_pStream);
		_bNeedsHeader = strrchr(pszFmt, '\n') ? true : false;
    }
}

void GfLogger::warning(const char *pszFmt, ...)
{
    if (_pStream && _nLvlThresh >= eWarning)
    {
        if (_bNeedsHeader)
		{
			char* pszClock = GfTime2Str(GfTimeClock(), 0, true, 3);
            fprintf(_pStream, "%s Warning ", pszClock);
			free(pszClock);
		}
        va_list vaArgs;
        va_start(vaArgs, pszFmt);
        vfprintf(_pStream, pszFmt, vaArgs);
        va_end(vaArgs);
		fflush(_pStream);
		_bNeedsHeader = strrchr(pszFmt, '\n') ? true : false;
    }
}

void GfLogger::info(const char *pszFmt, ...)
{
    if (_pStream && _nLvlThresh >= eInfo)
    {
        if (_bNeedsHeader)
		{
			char* pszClock = GfTime2Str(GfTimeClock(), 0, true, 3);
            fprintf(_pStream, "%s Info    ", pszClock);
			free(pszClock);
		}
        va_list vaArgs;
        va_start(vaArgs, pszFmt);
        vfprintf(_pStream, pszFmt, vaArgs);
        va_end(vaArgs);
		fflush(_pStream);
		_bNeedsHeader = strrchr(pszFmt, '\n') ? true : false;
    }
}

void GfLogger::trace(const char *pszFmt, ...)
{
    if (_pStream && _nLvlThresh >= eTrace)
    {
        if (_bNeedsHeader)
		{
			char* pszClock = GfTime2Str(GfTimeClock(), 0, true, 3);
            fprintf(_pStream, "%s Trace   ", pszClock);
			free(pszClock);
		}
        va_list vaArgs;
        va_start(vaArgs, pszFmt);
        vfprintf(_pStream, pszFmt, vaArgs);
        va_end(vaArgs);
		fflush(_pStream);
		_bNeedsHeader = strrchr(pszFmt, '\n') ? true : false;
    }
}

void GfLogger::debug(const char *pszFmt, ...)
{
    if (_pStream && _nLvlThresh >= eDebug)
    {
        if (_bNeedsHeader)
		{
			char* pszClock = GfTime2Str(GfTimeClock(), 0, true, 3);
            fprintf(_pStream, "%s Debug   ", pszClock);
			free(pszClock);
		}
        va_list vaArgs;
        va_start(vaArgs, pszFmt);
        vfprintf(_pStream, pszFmt, vaArgs);
        va_end(vaArgs);
		fflush(_pStream);
		_bNeedsHeader = strrchr(pszFmt, '\n') ? true : false;
    }
}

void GfLogger::message(int nLevel, const char *pszFmt, ...)
{
    if (_pStream && _nLvlThresh >= nLevel)
    {
		if (_bNeedsHeader)
		{
			char* pszClock = GfTime2Str(GfTimeClock(), 0, true, 3);
			if (nLevel >= eFatal && nLevel <= eDebug)
				fprintf(_pStream, "%s %.7s ", astrLevelNames[nLevel], pszClock);
			else
				fprintf(_pStream, "%s Level%d ", pszClock, nLevel);
			free(pszClock);
		}
		va_list vaArgs;
		va_start(vaArgs, pszFmt);
		vfprintf(_pStream, pszFmt, vaArgs);
		va_end(vaArgs);
		fflush(_pStream);
		_bNeedsHeader = strrchr(pszFmt, '\n') ? true : false;
    }
}

#endif // TRACE_OUT
