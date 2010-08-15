/***************************************************************************
                                   TRACE                   
                             -------------------                                         
    created              : Fri Aug 13 22:32:45 CEST 1999
    copyright            : (C) 1999 by Eric Espie                         
    email                : torcs@free.fr   
    version              : $Id: trace.cpp,v 1.7 2005/02/01 15:55:54 berniw Exp $
                                  
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
    Allow the trace in the file <tt>trace.txt</tt>
    @author	<a href=mailto:torcs@free.fr>Eric Espie</a>
    @version	$Id: trace.cpp,v 1.7 2005/02/01 15:55:54 berniw Exp $
    @ingroup	trace
*/


#include <cerrno>
#include <cstdio>
#include <cassert>
#include <cstring>
#include <ctime>

#if (defined(_WIN32) || defined(WIN32))
#include <windows.h>
#include <windowsx.h>
#else
#include <sys/param.h>
#endif // _WIN32 || WIN32

#include "tgf.h"


// Log levels.
enum { gfLogFatal = 0, gfLogError, gfLogWarning, gfLogInfo, gfLogTrace, gfLogDebug };

static const char* gfLogLevelNames[] = { "Fatal", "Error", "Warning", "Info", "Trace", "Debug"};


// Log stream.
static FILE* gfLogStream = 0;

// Log level threshold.
static int gfLogLevelThreshold = -1;

// Flag indicating if the last logged line ended with a new-line.
static bool gfLogNeedLineHeader = true;


void
gfTraceInit(void)
{
	gfLogNeedLineHeader = true;
    GfLogSetLevelThreshold(TRACE_LEVEL);
    GfLogSetStream(stderr);
}

#ifdef TRACE_OUT

void GfLogSetStream(FILE* fStream)
{
    if (fStream)
    {
		// Close previous stream if needed.
		if (gfLogStream && gfLogStream != stderr && gfLogStream != stdout)
			fclose(gfLogStream);
		
		gfLogStream = fStream;
    }
    else
		fprintf(gfLogStream ? gfLogStream : stderr, 
				"Error\tGfLogSetStream : %s", strerror(errno));
	
    if (gfLogStream)
    {
        // Trace date and time.
		time_t t = time(NULL);
		struct tm *stm = localtime(&t);
		fprintf(gfLogStream, "Info\t%4d/%02d/%02d %02d:%02d:%02d\n",
				stm->tm_year+1900, stm->tm_mon+1, stm->tm_mday,
				stm->tm_hour, stm->tm_min, stm->tm_sec);
		
		// Trace current trace level threshold.
		fprintf(gfLogStream, "Info\tCurrent trace level threshold : ");
		if (gfLogLevelThreshold >= gfLogFatal && gfLogLevelThreshold <= gfLogDebug)
			fprintf(gfLogStream, "%s\n", gfLogLevelNames[gfLogLevelThreshold]);
		else
			fprintf(gfLogStream, "%d\n", gfLogLevelThreshold);
		fflush(gfLogStream);
    }
}

void GfLogSetLevelThreshold(int nLevel)
{
    gfLogLevelThreshold = nLevel;

    // Trace new trace level threshold.
    if (gfLogStream)
    {
		fprintf(gfLogStream, "Info\tNew trace level threshold : ");
		if (gfLogLevelThreshold >= gfLogFatal && gfLogLevelThreshold <= gfLogDebug)
			fprintf(gfLogStream, "%s\n", gfLogLevelNames[gfLogLevelThreshold]);
		else
			fprintf(gfLogStream, "%d\n", gfLogLevelThreshold);
		fflush(gfLogStream);
    }
}

#endif // TRACE_OUT

void GfLogFatal(const char *pszFmt, ...)
{
#ifdef TRACE_OUT
    if (gfLogLevelThreshold >= gfLogFatal)
    {
		if (gfLogNeedLineHeader)
            fprintf(gfLogStream, "Fatal\t");
        va_list vaArgs;
        va_start(vaArgs, pszFmt);
        vfprintf(gfLogStream, pszFmt, vaArgs);
        va_end(vaArgs);
		fflush(gfLogStream);
		gfLogNeedLineHeader = strrchr(pszFmt, '\n') ? true : false;
    }
#endif // TRACE_OUT

    // GfScrShutdown();
    assert(0);
    exit(1);
}

#ifdef TRACE_OUT

TGF_API void GfLogError(const char *pszFmt, ...)
{
    if (gfLogLevelThreshold >= gfLogError)
    {
        if (gfLogNeedLineHeader)
            fprintf(gfLogStream, "Error\t");
        va_list vaArgs;
        va_start(vaArgs, pszFmt);
        vfprintf(gfLogStream, pszFmt, vaArgs);
        va_end(vaArgs);
		fflush(gfLogStream);
		gfLogNeedLineHeader = strrchr(pszFmt, '\n') ? true : false;
    }
}

TGF_API void GfLogWarning(const char *pszFmt, ...)
{
    if (gfLogLevelThreshold >= gfLogWarning)
    {
        if (gfLogNeedLineHeader)
            fprintf(gfLogStream, "Warning\t");
        va_list vaArgs;
        va_start(vaArgs, pszFmt);
        vfprintf(gfLogStream, pszFmt, vaArgs);
        va_end(vaArgs);
		fflush(gfLogStream);
		gfLogNeedLineHeader = strrchr(pszFmt, '\n') ? true : false;
    }
}

TGF_API void GfLogInfo(const char *pszFmt, ...)
{
    if (gfLogLevelThreshold >= gfLogInfo)
    {
        if (gfLogNeedLineHeader)
            fprintf(gfLogStream, "Info\t");
        va_list vaArgs;
        va_start(vaArgs, pszFmt);
        vfprintf(gfLogStream, pszFmt, vaArgs);
        va_end(vaArgs);
		fflush(gfLogStream);
		gfLogNeedLineHeader = strrchr(pszFmt, '\n') ? true : false;
    }
}

TGF_API void GfLogTrace(const char *pszFmt, ...)
{
    if (gfLogLevelThreshold >= gfLogTrace)
    {
        if (gfLogNeedLineHeader)
            fprintf(gfLogStream, "Trace\t");
        va_list vaArgs;
        va_start(vaArgs, pszFmt);
        vfprintf(gfLogStream, pszFmt, vaArgs);
        va_end(vaArgs);
		fflush(gfLogStream);
		gfLogNeedLineHeader = strrchr(pszFmt, '\n') ? true : false;
    }
}

TGF_API void GfLogDebug(const char *pszFmt, ...)
{
    if (gfLogLevelThreshold >= gfLogDebug)
    {
        if (gfLogNeedLineHeader)
            fprintf(gfLogStream, "Debug\t");
        va_list vaArgs;
        va_start(vaArgs, pszFmt);
        vfprintf(gfLogStream, pszFmt, vaArgs);
        va_end(vaArgs);
		fflush(gfLogStream);
		gfLogNeedLineHeader = strrchr(pszFmt, '\n') ? true : false;
    }
}

TGF_API void GfLogMessage(int nLevel, const char *pszFmt, ...)
{
    if (gfLogLevelThreshold >= nLevel)
    {
        if (nLevel >= gfLogFatal && nLevel <= gfLogDebug)
		{
			if (gfLogNeedLineHeader)
				fprintf(gfLogStream, "%s\t", gfLogLevelNames[nLevel]);
		}
		else
		{
			if (gfLogNeedLineHeader)
				fprintf(gfLogStream, "Level%d\t", nLevel);
		}
		va_list vaArgs;
		va_start(vaArgs, pszFmt);
		vfprintf(gfLogStream, pszFmt, vaArgs);
		va_end(vaArgs);
		fflush(gfLogStream);
		gfLogNeedLineHeader = strrchr(pszFmt, '\n') ? true : false;
    }
}

#endif // TRACE_OUT


/* static FILE *outTrace = (FILE*)NULL; */

/* static char TraceStr[1024]; */

/** Print a message in the trace file.
    The file is openned the first time
    @ingroup	trace
    @param	szTrc	message to trace
*/
/* void GfTrace(char *fmt, ...) */
/* { */
/*     va_list		ap; */
/*     struct tm		*stm; */
/*     time_t		t; */
/*     char		*s = TraceStr; */

/*     fprintf(stderr, "ERROR: "); */
/*     va_start(ap, fmt); */
/*     vfprintf(stderr, fmt, ap); */
/*     va_end(ap); */
/*     fflush(stderr); */

/*     if (outTrace == NULL) { */
/* 	if ((outTrace = fopen("trace.txt", "w+")) == NULL) { */
/* 	    perror("trace.txt"); */
/* 	    return; */
/* 	} */
/*     } */
/*     t = time(NULL); */
/*     stm = localtime(&t); */
/*     s += sprintf(TraceStr, "%4d/%02d/%02d %02d:%02d:%02d ", */
/* 		 stm->tm_year+1900, stm->tm_mon+1, stm->tm_mday, */
/* 		 stm->tm_hour, stm->tm_min, stm->tm_sec); */

/*     va_start(ap, fmt); */
/*     vsnprintf(s, 1023 - strlen(TraceStr), fmt, ap); */
/*     va_end(ap); */

/*     fwrite(TraceStr, strlen(TraceStr), 1, outTrace); */
/*     fflush(outTrace); */
/* } */

