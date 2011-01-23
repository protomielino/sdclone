/***************************************************************************
                    tgf.h -- Interface file for The Gaming Framework                                    
                             -------------------                                         
    created              : Fri Aug 13 22:32:14 CEST 1999
    copyright            : (C) 1999 by Eric Espie                         
    email                : torcs@free.fr   
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
    	The Gaming Framework API.
    @author	<a href=mailto:torcs@free.fr>Eric Espie</a>
    @version	$Id$
*/

#ifndef __TGF__H__
#define __TGF__H__

#include <cstdio>
#include <cstdlib>
#include <cstdarg>
#include <cstring>
#include <climits>
#include <cmath>

#include <osspec.h>

// DLL exported symbols declarator for Windows.
#ifdef WIN32
# ifdef TGF_DLL
#  define TGF_API __declspec(dllexport)
# else
#  define TGF_API __declspec(dllimport)
# endif
#else
# define TGF_API
#endif

#include "modinfo.h" // Don't move this include line : needs TGF_API definition.


/** Floating point type used everywhere.
    @ingroup definitions
*/
typedef float tdble;
/* typedef double tdble; */

/** Maximum between two values */
#ifndef MAX
#define MAX(x,y) ((x) > (y) ? (x) : (y))
#endif

/** Minimum between two values */
#ifndef MIN
#define MIN(x,y) ((x) < (y) ? (x) : (y))
#endif

#define FREEZ(x) do {				\
    if (x) {					\
	free(x);				\
	x = 0;					\
    }						\
} while (0)

#define freez FREEZ

const double PI = 3.14159265358979323846;  /**< PI */
const tdble G = 9.80665f; /**< m/s/s */

/* conversion */
#define RADS2RPM(x) ((x)*9.549296585)		/**< Radian/s to RPM conversion */
#define RPM2RADS(x) ((x)*.104719755)		/**< RPM to Radian/s conversion */
#define RAD2DEG(x)  ((x)*(180.0/PI))		/**< Radian to degree conversion */
#define FLOAT_RAD2DEG(x)  ((x)*(float)(180.0/PI))/**< Radian to degree conversion */
#define DEG2RAD(x)  ((x)*(PI/180.0))		/**< Degree to radian conversion */
#define FLOAT_DEG2RAD(x)  ((x)*(float)(PI/180.0))/**< Degree to radian conversion */
#define FEET2M(x)   ((x)*0.304801)		/**< Feet to meter conversion */
#define SIGN(x)     ((x) < 0 ? -1.0 : 1.0)	/**< Sign of the expression */

/** Angle normalization between 0 and 2 * PI */
#define NORM0_2PI(x) 				\
do {						\
	while ((x) > 2*PI) { (x) -= 2*PI; }	\
	while ((x) < 0) { (x) += 2*PI; } 	\
} while (0)

/** Angle normalization between -PI and PI */
#define NORM_PI_PI(x) 				\
do {						\
	while ((x) > PI) { (x) -= 2*PI; }	\
	while ((x) < -PI) { (x) += 2*PI; } 	\
} while (0)

/** Angle normalization between -PI and PI */
#define FLOAT_NORM_PI_PI(x) 				\
do {						\
	while ((x) > PI) { (x) -= (tdble) (2*PI); }	\
	while ((x) < -PI) { (x) += (tdble) (2*PI); } 	\
} while (0)


#ifndef DIST
/** Distance between two points */
#define DIST(x1, y1, x2, y2) sqrt(((x1) - (x2)) * ((x1) - (x2)) + ((y1) - (y2)) * ((y1) - (y2)))
#endif


typedef struct {
    float	x;
    float	y;
    float	z;
} t3Df;

/** 3D point.
    @ingroup definitions
*/
typedef struct {
    tdble	x;		/**< x coordinate */
    tdble	y;		/**< y coordinate */
    tdble	z;		/**< z coordinate */
} t3Dd;

typedef struct {
    int		x;
    int		y;
    int		z;
} t3Di;

/** 6 DOF position.
    @ingroup definitions
*/
typedef struct {
    tdble	x;		/**< x coordinate */
    tdble	y;		/**< y coordinate */
    tdble	z;		/**< z coordinate */
    tdble	ax;		/**< angle along x axis */
    tdble	ay;		/**< angle along y axis */
    tdble	az;		/**< angle along z axis */
} tPosd;

/** Dynamic point structure.
    @ingroup definitions
*/
typedef struct 
{
    tPosd pos; /**< position */
    tPosd vel; /**< velocity */
    tPosd acc; /**< acceleration */
} tDynPt;

/** Forces and moments */
typedef struct
{
    t3Dd F; /**< Forces */
    t3Dd M; /**< Moments */
} tForces;

/******************************
 * Gaming framework managment *
 ******************************/
TGF_API void GfInit(void);
TGF_API void GfRestart(bool bHardwareMouse = false);


/************************************************************************
 * Memory pools                                                         *
 * Allocate as many items as needed, deallocate all at once at the end) *
 ************************************************************************/

typedef struct MemoryPoolItem tMemoryPoolItem;
typedef tMemoryPoolItem* tMemoryPool;

typedef struct MemoryPoolItem
{
	struct MemoryPoolItem *prev;
	struct MemoryPoolItem *next;
	tMemoryPool *pool; /* NULL if not the first item, pointer to the pool otherwise */
} tMemoryPoolItem;

typedef tMemoryPoolItem* tMemoryPool;

TGF_API void* GfPoolMalloc(size_t size, tMemoryPool* pool);
TGF_API void GfPoolFree(void *pointer);
TGF_API void GfPoolFreePool(tMemoryPool* pool);
TGF_API void GfPoolMove(tMemoryPool* oldPool, tMemoryPool* newPool);

/*********************************
 * Memory debug tools            *
 *********************************/

// <esppat>
#ifdef WIN32

#define malloc _tgf_win_malloc
#define calloc _tgf_win_calloc
#define realloc _tgf_win_realloc
#define free _tgf_win_free
#ifdef strdup
#undef strdup
#endif
#define strdup _tgf_win_strdup
#define _strdup _tgf_win_strdup
TGF_API void * _tgf_win_malloc(size_t size);
TGF_API void * _tgf_win_calloc(size_t num, size_t size);
TGF_API void * _tgf_win_realloc(void * memblock, size_t size);
TGF_API void _tgf_win_free(void * memblock);
TGF_API char * _tgf_win_strdup(const char * str);

#endif // WIN32
// </esppat>

/*********************************
 * Interface For Dynamic Modules *
 *********************************/

/* Gaming Framework Id value for "no filtering on GFId" */
#define GfIdAny		UINT_MAX

TGF_API tModList *GfModIsInList(const char *dllname, tModList *modlist);
TGF_API void GfModAddInList(tModList *mod, tModList **modlist, int priosort);
TGF_API void GfModMoveToListHead(tModList *mod, tModList **modlist);
TGF_API int GfModLoad(unsigned int gfid, const char *dllname, tModList **modlist);
TGF_API int GfModUnloadList(tModList **modlist);
TGF_API int GfModInfo(unsigned int gfid, const char *filename, tModList **modlist);
TGF_API int GfModInfoDir(unsigned int gfid, const char *dir, int level, tModList **modlist);
TGF_API int GfModFreeInfoList(tModList **modlist);


/************************
 * Directory management *
 ************************/

/** List of files for a Directory 
    @see	GfDirGetList
*/
typedef struct FList 
{
    struct FList	*next;		/**< Next entry */
    struct FList	*prev;		/**< Previous entry */
    char		*name;		/**< File name */
    char		*dispName;	/**< Name to display on screen */
    void		*userData;	/**< User data */
} tFList;

TGF_API int GfDirCreate(const char *path);
TGF_API tFList *GfDirGetList(const char *dir);
TGF_API tFList *GfDirGetListFiltered(const char *dir, const char *prefix, const char *suffix);
typedef void (*tfDirfreeUserData)(void*);	/**< Function to call for releasing the user data associated with file entry */
TGF_API void GfDirFreeList(tFList *list, tfDirfreeUserData freeUserData, bool freeName = false, bool freeDispName = false);


/************************
 * File management      *
 ************************/

TGF_API bool GfFileExists(const char* pszName);
TGF_API bool GfFileCopy(const char* pszSrcName, const char* pszTgtName);


/**************************************
 * Directory and file path management *
 **************************************/

TGF_API bool GfPathIsAbsolute(const char *pszPath);
TGF_API char* GfPathNormalizeDir(char* pszPath, size_t nMaxPathLen);
TGF_API char* GfPathMakeOSCompatible(char* path);


/**********************************
 *  Interface For Parameter Files *
 **********************************/

/*
 *	This set of function is used to store and retrieve
 *	values in parameters files.
 */

/* parameters file type */
#define GFPARM_PARAMETER	0	/**< Parameter file */
#define GFPARM_TEMPLATE		1	/**< Template file */
#define GFPARM_PARAM_STR	"param"
#define GFPARM_TEMPL_STR	"template"

/* parameters access mode */
#define GFPARM_MODIFIABLE	1	/**< Parameter file allowed to be modified */
#define GFPARM_WRITABLE		2	/**< Parameter file allowed to be saved on disk */

/* parameter file read */
#define GFPARM_RMODE_STD	0x01	/**< if handle already open, return it */
#define GFPARM_RMODE_REREAD	0x02	/**< reread the parameters from file and release the previous ones */
#define GFPARM_RMODE_CREAT	0x04	/**< Create the file if doesn't exist */
#define GFPARM_RMODE_PRIVATE	0x08

TGF_API void * GfParmReadFileLocal(const char *file, int mode, bool neededFile = true);
TGF_API void *GfParmReadFile(const char *file, int mode, bool neededFile = true);
/* parameter file write */
TGF_API int GfParmWriteFileLocal(const char *file, void* handle, const char *name);
TGF_API int GfParmWriteFile(const char *file, void* handle, const char *name);
TGF_API int GfParmWriteFileSDHeader(const char *file, void* handle, const char *name, const char *author);

TGF_API char *GfParmGetName(void *handle);
TGF_API char *GfParmGetFileName(void *handle);
TGF_API int GfParmGetMajorVersion(void *handle);
TGF_API int GfParmGetMinorVersion(void *handle);

/* set the dtd and header values */
TGF_API void GfParmSetDTD (void *parmHandle, char *dtd, char*header);

/* get string parameter value */
TGF_API const char *GfParmGetStr(void *handle, const char *path, const char *key, const char *deflt);
TGF_API char *GfParmGetStrNC(void *handle, const char *path, const char *key, char *deflt);
/* get string parameter value */
TGF_API const char *GfParmGetCurStr(void *handle, const char *path, const char *key, const char *deflt);
TGF_API char *GfParmGetCurStrNC(void *handle, const char *path, const char *key, char *deflt);
/* set string parameter value */
TGF_API int GfParmSetStr(void *handle, const char *path, const char *key, const char *val);
/* set string parameter value */
TGF_API int GfParmSetCurStr(void *handle, const char *path, const char *key, const char *val);

/* get num parameter value */
TGF_API tdble GfParmGetNum(void *handle, const char *path, const char *key, const char *unit, tdble deflt);
/* get num parameter value */
TGF_API tdble GfParmGetCurNum(void *handle, const char *path, const char *key, const char *unit, tdble deflt);
/* set num parameter value */
TGF_API int GfParmSetNum(void *handle, const char *path, const char *key, const char *unit, tdble val);
/* set num parameter value */
TGF_API int GfParmSetCurNum(void *handle, const char *path, const char *key, const char *unit, tdble val);

/* is formula */
TGF_API int GfParmIsFormula(void *handle, char const *path, char const *key);
/* get formula */
TGF_API char* GfParmGetFormula(void *hanlde, char const *path, char const *key);
TGF_API char* GfParmGetCurFormula(void *hanlde, char const *path, char const *key);
/* set formula */
TGF_API int GfParmSetFormula(void* hanlde, char const *path, char const *key, char const *formula);
TGF_API int GfParmSetCurFormula(void* hanlde, char const *path, char const *key, char const *formula);
 
/* clean all the parameters of a set */
TGF_API void GfParmClean(void *handle);
/* clean the parms and release the handle without updating the file */
TGF_API void GfParmReleaseHandle(void *handle);

/* Convert a value in "units" into SI */
TGF_API tdble GfParmUnit2SI(const char *unit, tdble val);
/* convert a value in SI to "units" */
TGF_API tdble GfParmSI2Unit(const char *unit, tdble val);

/* compare and merge different handles */
TGF_API int GfParmCheckHandle(void *ref, void *tgt);
#define GFPARM_MMODE_SRC	1 /**< use ref and modify existing parameters with tgt */
#define GFPARM_MMODE_DST	2 /**< use tgt and verify ref parameters */
#define GFPARM_MMODE_RELSRC	4 /**< release ref after the merge */
#define GFPARM_MMODE_RELDST	8 /**< release tgt after the merge */
TGF_API void *GfParmMergeHandles(void *ref, void *tgt, int mode);
TGF_API int GfParmGetNumBoundaries(void *handle, char *path, char *key, tdble *min, tdble *max);

TGF_API int GfParmGetEltNb(void *handle, const char *path);
TGF_API int GfParmListSeekFirst(void *handle, const char *path);
TGF_API int GfParmListSeekNext(void *handle, const char *path);
TGF_API char *GfParmListGetCurEltName(void *handle, const char *path);
TGF_API int GfParmListRemoveElt(void *handle, const char *path, const char *key);
TGF_API int GfParmListRenameElt(void *handle, const char *path, const char *oldKey, const char *newKey);
TGF_API int GfParmListClean(void *handle, const char *path);

TGF_API void GfParmSetVariable(void *handle, char const *path, char const *key, tdble val);
TGF_API void GfParmRemoveVariable(void *handle, char const *path, char const *key);
TGF_API tdble GfParmGetVariable(void *handle, char const *path, char const *key);

/********************************************************************************
 * Log/Trace Interface                                                          *
 *  - Write formated string messages at run-time to the log stream,             *
 *    with automatic prepending of current time and trace level                 *
 *    (Ex: 12:27.35.267 Debug  My formated message)                             *
 *  - GfLogFatal also exits the program after logging the message
 *  - Messages are given an integer "level" = "criticity",                      *
 *    (0=Fatal, 1=Error, 2=Warning, 3=Info, 4=Trace, 5=Debug, ...)              *
 *  - Messages are actually logged into the stream only if their level          *
 *    is lower than the current threshold,                                      *
 *  - Default stream is stderr, but it can be changed at run-time to any FILE*, *
 *  - Default level threshold is #defined at compile time through TRACE_LEVEL,  *
 *    but it can be changed at run-time to any other level.                     *
 ********************************************************************************/

TGF_API void GfLogFatal(const char *pszFmt, ...);

#ifdef TRACE_OUT

TGF_API void GfLogError(const char *pszFmt, ...);
TGF_API void GfLogWarning(const char *pszFmt, ...);
TGF_API void GfLogInfo(const char *pszFmt, ...);
TGF_API void GfLogTrace(const char *pszFmt, ...);
TGF_API void GfLogDebug(const char *pszFmt, ...);

TGF_API void GfLogMessage(int nLevel, const char *pszFmt, ...);
TGF_API void GfLogSetFile(const char* pszFileName);
TGF_API void GfLogSetStream(FILE* fStream);
TGF_API void GfLogSetLevelThreshold(int nLevel);

#else // TRACE_OUT

static inline void GfLogNothing(const char *pszFmt, ...) {};

#define GfLogError GfLogNothing
#define GfLogWarning GfLogNothing
#define GfLogInfo GfLogNothing
#define GfLogTrace GfLogNothing
#define GfLogDebug GfLogNothing

static inline void GfLogMessage(int nLevel, const char *pszFmt, ...) {};
#define GfLogSetFile(pszFileName)
#define GfLogSetStream(fStream)
#define GfLogSetLevelThreshold(nLevel)

#endif // TRACE_OUT

// Backward compatibility.
#define GfFatal GfLogFatal
#define GfError GfLogError
#define GfOut   GfLogInfo
#define GfTrace GfLogTrace


/******************* 
 * Time  Interface *
 *******************/
TGF_API double GfTimeClock(void);
TGF_API char *GfTime2Str(double sec, const char* plus="", bool zeros=true, int prec=3);


/******************
 * Miscellaneous. *
 ******************/
TGF_API int GfNearestPow2(int x);

/* Mean values */
#define GF_MEAN_MAX_VAL	5

typedef struct 
{
    int		curNum;
    tdble	val[GF_MEAN_MAX_VAL+1];
} tMeanVal;

TGF_API tdble gfMean(tdble v, tMeanVal *pvt, int n, int w);
TGF_API void gfMeanReset(tdble v, tMeanVal *pvt);


/********************
 * System Interface *
 ********************/
TGF_API unsigned GfGetNumberOfCPUs();

enum { GfAffinityAnyCPU = -1 };
TGF_API bool GfSetThreadAffinity(int nCPUId);

TGF_API void GfSleep(double seconds);


/***************************
 * Run-time dirs accessors *
 ***************************/

TGF_API void GfInitInstallDir(const char *pszExecutablePath);
TGF_API const char* GfGetInstallDir();

TGF_API const char *GetLocalDir();
TGF_API const char *SetLocalDir(const char *pszPath);

TGF_API const char *GetLibDir();
TGF_API const char *SetLibDir(const char *pszPath);

TGF_API const char *GetDataDir();
TGF_API const char *SetDataDir(const char *pszPath);

TGF_API const char *GetBinDir();
TGF_API const char *SetBinDir(const char *pszPath);


/************************************************
 * User settings files run-time update/install. *
 ************************************************/
TGF_API void GfFileSetup();


/***************************
 * Tail queue definitions. *
 ***************************/

/*
 * Copyright (c) 1991, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)queue.h	8.5 (Berkeley) 8/20/94
 */

/** Head type definition
    @ingroup tailq */
#define GF_TAILQ_HEAD(name, type)					\
typedef struct name {							\
	type *tqh_first;	/* first element */			\
	type **tqh_last;	/* addr of last next element */		\
} t ## name

/** Entry in structure
    @ingroup tailq */
#define GF_TAILQ_ENTRY(type)						\
struct {								\
	type *tqe_next;	/* next element */				\
	type **tqe_prev;	/* address of previous next element */	\
}

/** First element of a TAILQ
    @ingroup tailq */
#define	GF_TAILQ_FIRST(head)		((head)->tqh_first)
/** Next element of a TAILQ
    @ingroup tailq */
#define	GF_TAILQ_NEXT(elm, field)	((elm)->field.tqe_next)
/** End of a TAILQ
    @ingroup tailq */
#define	GF_TAILQ_END(head)		NULL
/** Last element of a TAILQ
    @ingroup tailq */
#define GF_TAILQ_LAST(head, headname) 					\
	(*(((struct headname *)((head)->tqh_last))->tqh_last))
/** Previous element of a TAILQ
    @ingroup tailq */
#define GF_TAILQ_PREV(elm, headname, field) 				\
	(*(((struct headname *)((elm)->field.tqe_prev))->tqh_last))

/*
 * Tail queue functions.
 */
/** Head initialization (Mandatory)
    @ingroup tailq */
#define	GF_TAILQ_INIT(head) do {					\
	(head)->tqh_first = NULL;					\
	(head)->tqh_last = &(head)->tqh_first;				\
} while (0)

/** Entry initialization (optionnal if inserted)
    @ingroup tailq */
#define GF_TAILQ_INIT_ENTRY(elm, field) do {	\
  (elm)->field.tqe_next = 0;			\
  (elm)->field.tqe_prev = 0;			\
} while (0)

/** Insert an element at the head
    @ingroup tailq */
#define GF_TAILQ_INSERT_HEAD(head, elm, field) do {			\
	if (((elm)->field.tqe_next = (head)->tqh_first) != NULL)	\
		(head)->tqh_first->field.tqe_prev =			\
		    &(elm)->field.tqe_next;				\
	else								\
		(head)->tqh_last = &(elm)->field.tqe_next;		\
	(head)->tqh_first = (elm);					\
	(elm)->field.tqe_prev = &(head)->tqh_first;			\
} while (0)

/** Insert an element at the tail
    @ingroup tailq */
#define GF_TAILQ_INSERT_TAIL(head, elm, field) do {			\
	(elm)->field.tqe_next = NULL;					\
	(elm)->field.tqe_prev = (head)->tqh_last;			\
	*(head)->tqh_last = (elm);					\
	(head)->tqh_last = &(elm)->field.tqe_next;			\
} while (0)

/** Insert an element after another element
    @ingroup tailq */
#define GF_TAILQ_INSERT_AFTER(head, listelm, elm, field) do {		\
	if (((elm)->field.tqe_next = (listelm)->field.tqe_next) != NULL)\
		(elm)->field.tqe_next->field.tqe_prev = 		\
		    &(elm)->field.tqe_next;				\
	else								\
		(head)->tqh_last = &(elm)->field.tqe_next;		\
	(listelm)->field.tqe_next = (elm);				\
	(elm)->field.tqe_prev = &(listelm)->field.tqe_next;		\
} while (0)

/** Insert an element before another element
    @ingroup tailq */
#define	GF_TAILQ_INSERT_BEFORE(listelm, elm, field) do {		\
	(elm)->field.tqe_prev = (listelm)->field.tqe_prev;		\
	(elm)->field.tqe_next = (listelm);				\
	*(listelm)->field.tqe_prev = (elm);				\
	(listelm)->field.tqe_prev = &(elm)->field.tqe_next;		\
} while (0)

/** Remove an element
    @ingroup tailq */
#define GF_TAILQ_REMOVE(head, elm, field) do {				\
	if (((elm)->field.tqe_next) != NULL)				\
		(elm)->field.tqe_next->field.tqe_prev = 		\
		    (elm)->field.tqe_prev;				\
	else								\
		(head)->tqh_last = (elm)->field.tqe_prev;		\
	*(elm)->field.tqe_prev = (elm)->field.tqe_next;			\
} while (0)


/**************************************************************************
 * Profiler definitions.                                                  *
 * A simple high-level profiler for non-threaded non-recursive functions. *
 * \author Henrik Enqvist IB (henqvist@abo.fi)                            *
 **************************************************************************/
#ifdef PROFILER

TGF_API void GfProfStartProfile(const char* pszName);
TGF_API void GfProfStopProfile(const char* pszName);
TGF_API void GfProfStopActiveProfiles();
TGF_API void GfProfPrintReport();

#else // PROFILER

#define GfProfStartProfile(pszName)
#define GfProfStopProfile(pszName)
#define GfProfStopActiveProfiles()
#define GfProfPrintReport()

#endif // PROFILER


/**************************************************************
 * ScheduleSpy definitions.                                   *
 *   \author J.P. Meuret (jpmeuret@free.fr)                   *
 *   A tool to study the way some special code sections       *
 *   (named "events) in the program are actually scheduled    *
 *   at a fine grain level (see schedulespy.cpp for details   *
 *   and raceengine.cpp for an example of how to use it).     *
 **************************************************************/

#ifdef SCHEDULE_SPY

TGF_API void GfSchedConfigureEventLog(const char* pszSpyName, const char* pszLogName,
									  unsigned nMaxEvents, double dIgnoreDelay);
TGF_API void GfSchedBeginSession(const char* pszSpyName);
TGF_API void GfSchedBeginEvent(const char* pszSpyName, const char* pszLogName);
TGF_API void GfSchedEndEvent(const char* pszSpyName, const char* pszLogName);
TGF_API void GfSchedEndSession(const char* pszSpyName);
TGF_API void GfSchedPrintReport(const char* pszSpyName, const char* pszFileName,
								double fTimeResolution,
								double fDurationUnit, double fDurationResolution);

#else // SCHEDULE_SPY

#define GfSchedConfigureEventLog(pszSpyName, pszLogName, nMaxEvents, dIgnoreDelay)
#define GfSchedBeginSession(pszSpyName)
#define GfSchedBeginEvent(pszSpyName, pszLogName)
#define GfSchedEndEvent(pszSpyName, pszLogName)
#define GfSchedEndSession(pszSpyName)
#define GfSchedPrintReport(pszSpyName, pszFileName, \
						   fTimeResolution, fDurationUnit, fDurationResolution)

#endif // SCHEDULE_SPY


/*******************/
/*   Hash Tables   */
/*******************/
#define GF_HASH_TYPE_STR	0	/**< String key based hash table */
#define GF_HASH_TYPE_BUF	1	/**< Memory buffer key based hash table */

typedef void (*tfHashFree)(void*);	/**< Function to call for releasing the user data associated with hash table */

TGF_API void *GfHashCreate(int type);
TGF_API int GfHashAddStr(void *hash, const char *key, void *data);
TGF_API void *GfHashRemStr(void *hash, char *key);
TGF_API void *GfHashGetStr(void *hash, const char *key);
TGF_API void GfHashAddBuf(void *hash, char *key, size_t sz, void *data);
TGF_API void *GfHashRemBuf(void *hash, char *key, size_t sz);
TGF_API void *GfHashGetBuf(void *hash, char *key, size_t sz);
TGF_API void GfHashRelease(void *hash, tfHashFree hashFree);
TGF_API void *GfHashGetFirst(void *hash);
TGF_API void *GfHashGetNext(void *hash);

#define GF_DIR_CREATION_FAILED 0
#define GF_DIR_CREATED 1


/*******************/
/*   Formulas      */
/*******************/

TGF_API void* GfFormParseFormulaString(const char *string);
TGF_API void* GfFormParseFormulaStringNew(const char *string);
TGF_API tdble GfFormCalcFunc(void *cmd, void *parmHandle, char*path);
TGF_API char GfFormCalcFuncNew(void *cmd, void *parmHandle, char const* path, char *boolean, int *integer, tdble *number, char ** string);
TGF_API void GfFormFreeCommand(void *cmd);
TGF_API void GfFormFreeCommandNew(void *cmd);

#endif /* __TGF__H__ */


