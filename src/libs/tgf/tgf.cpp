/***************************************************************************
                          tgf.cpp -- The Gaming Framework                            
                             -------------------                                         
    created              : Fri Aug 13 22:31:43 CEST 1999
    copyright            : (C) 1999 by Eric Espie
    email                : torcs@free.fr
    version              : $Id: tgf.cpp,v 1.16 2005/08/05 09:21:59 berniw Exp $
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifdef WIN32
#include <windows.h>
#include <direct.h>
#else
#include <sys/stat.h>
#include <sys/types.h>
#endif
#include <errno.h>

#include <tgf.h>
#include <time.h>
#include <cstring>

extern void gfDirInit(void);
extern void gfModInit(void);
extern void gfOsInit(void);
extern void gfParamInit(void);
extern void gfRlstInit(void);


/*
 * Function
 *	GfModIsInList
 *
 * Description
 *	Check if a module with given shared library file is present the given list
 *      WARNING: Only compare given sopath to stored ones, NOT shared library files identity
 *               (so, avoid mixing relative and absolute path-names in modlist).
 *
 * Parameters
 *	sopath  path-name of the shared library file to check
 *	modlist	list of module interfaces description structure
 *
 * Return
 *	The address of the matching modlist item if found, 0 otherwise.
 *
 * Remarks
 *	
 */
tModList *GfModIsInList(const char *sopath, tModList *modlist)
{
    tModList		*curMod;
    tModList		*nextMod;

    curMod = modlist;
    if (curMod == 0) {
	return 0;
    }
    nextMod = curMod->next;
    do {
	curMod = nextMod;
	nextMod = curMod->next;
	if (!strcmp(sopath, curMod->sopath))
	  return curMod;
    } while (curMod != modlist);
    
    return 0;
}

/*
 * Function
 *	GfModAddInList
 *
 * Description
 *	Add the given module in the given list
 *
 * Parameters
 *	mod      module interfaces description structure
 *	modlist	 list of module interfaces description structure
 *	priosort flag to sort list by prio
 *
 * Return
 *	Nothing
 *
 * Remarks
 *	
 */
void GfModAddInList(tModList *mod, tModList **modlist, int priosort)
{
    tModList		*curMod;
    int			prio;

    if (*modlist == 0) {
        *modlist = mod;
	mod->next = mod;
    } else {
        /* sort by prio if specified, otherwise put at list head */
        prio = mod->modInfo[0].prio;
	if (!priosort || prio >= (*modlist)->modInfo[0].prio) {
	    mod->next = (*modlist)->next;
	    (*modlist)->next = mod;
	    *modlist = mod;
	} else {
	    curMod = *modlist;
	    do {
	        if (prio < curMod->next->modInfo[0].prio) {
		    mod->next = curMod->next;
		    curMod->next = mod;
		    break;
		}
		curMod = curMod->next;
	    } while (curMod != *modlist);
	}
    }
}

/*
 * Function
 *	GfModMoveToListHead
 *
 * Description
 *	Move the given module to the head of the given list
 *
 * Parameters
 *	mod      module interfaces description structure to move
 *	modlist	 list of module interfaces description structure
 *
 * Return
 *	Nothing
 *
 * Remarks
 *	Nothing done if mod or *modlist is NULL
 *	
 */
void GfModMoveToListHead(tModList *mod, tModList **modlist)
{
    tModList *curMod;

    if (mod && *modlist) {

        // Search for mod in modlist
        curMod = *modlist;
	do {
	    // If found, make *modlist point on it and return
	    if (curMod == mod) {
	      *modlist = mod;
	      break;
	    }
	    curMod = curMod->next;
	} while (curMod != *modlist);
    }
}

/*
* Function
*	GfModFreeInfoList
*
* Description
*	Free a modules info list without unloading the modules
*
* Parameters
*	modlist	(in/out) list of info to free
*
* Return
*	0	Ok
*	-1	Error
*
* Remarks
*	
*/
int GfModFreeInfoList(tModList **modlist)
{
    tModList	*curMod;
    tModList	*nextMod;
    
    curMod = *modlist;
    if (curMod == 0)
	return 0;

    do 
    {
	nextMod = curMod->next;

	GfModInfoFreeNC(curMod->modInfo, curMod->modInfoSize);
	free(curMod->sopath);
	free(curMod);

	curMod = nextMod;
    }
    while (curMod != *modlist);
    
    *modlist = 0;

    return 0;
}


#ifdef WIN32
#include <crtdbg.h>
#include <assert.h>


void * _tgf_win_malloc(size_t size)
{
#ifdef _DEBUG
	char * p = (char*)GlobalAlloc(GMEM_FIXED, size + 3*sizeof(int));
	*(int*)(p) = size + 3*sizeof(int);
	*((int*)p + 1) = 123456789;
	*((int*)(p + size + 3*sizeof(int)) - 1) = 987654321;

	return p + 2*sizeof(int);
#else // _DEBUG
	char * p = (char*)GlobalAlloc(GMEM_FIXED, size + sizeof(int));
	if (p == NULL) {
		return NULL;
	}
	*(int*)(p) = size;
	return p + sizeof(int);
#endif // _DEBUG
}


void * _tgf_win_calloc(size_t num, size_t size)
{
	void * p = _tgf_win_malloc(num * size);
	memset(p, 0, num * size);
	return p;
}


void * _tgf_win_realloc(void * memblock, size_t size)
{
	if (size == 0) {
		_tgf_win_free(memblock);
		return NULL;
	}

	void * p = _tgf_win_malloc(size);
	if (p == NULL) {
		return NULL;
	}

	if (memblock != NULL) {
#ifdef _DEBUG
		memcpy(p, memblock, min(*(int*)((char*)memblock-2*sizeof(int)), (int)size));
#else // _DEBUG
		memcpy(p, memblock, min(*(int*)((char*)memblock-sizeof(int)), (int)size));
#endif // _DEBUG
		_tgf_win_free(memblock);
	}
	return p;
}


void _tgf_win_free(void * memblock)
{
	if (!memblock) {
		return;
	}

#ifdef _DEBUG
	char * p = (char*)memblock - 2*sizeof(int);

	if (!_CrtIsValidPointer(p, sizeof(int), TRUE)) {
		assert(0);
	}

	if (!_CrtIsValidPointer(p, *(int*)p, TRUE)) {
		assert( 0 );
	}

	if (*((int*)p + 1) != 123456789) {
		assert( 0 );
	}

	if(*((int*)(p + *(int*)p ) - 1) != 987654321) {
		assert( 0 );
	}

	GlobalFree((char*)memblock - 2*sizeof(int));
#else // _DEBUG
	GlobalFree((char*)memblock - sizeof(int));
#endif // _DEBUG
}


char * _tgf_win_strdup(const char * str)
{
	char * s = (char*)_tgf_win_malloc(strlen(str)+1);
	strcpy(s,str);

	return s;
}
#endif // WIN32


void GfInit(void)
{
	gfDirInit();
	gfModInit();
	gfOsInit();
	gfParamInit();
}


void gfMeanReset(tdble v, tMeanVal *pvt)
{
	int i;

	for (i = 0; i < GF_MEAN_MAX_VAL; i++) {
		pvt->val[i] = v;
	}
}


tdble gfMean(tdble v, tMeanVal *pvt, int n, int w)
{
	int i;
	tdble sum;

	if (n > pvt->curNum) {
		if (pvt->curNum < GF_MEAN_MAX_VAL) {
			pvt->curNum++;
		}
		n = pvt->curNum;
	} else {
		pvt->curNum = n;
	}

	sum = 0;
	for (i = 0; i < n; i++) {
		pvt->val[i] = pvt->val[i + 1];
		sum += pvt->val[i];
	}

	pvt->val[n] = v;
	sum += (tdble)w * v;
	sum /= (tdble)(n + w);

	return sum;
}


static char bufstr[1024];

char * GfGetTimeStr(void)
{
	struct tm *stm;
	time_t t;

	t = time(NULL);
	stm = localtime(&t);
	sprintf(bufstr, "%4d%02d%02d%02d%02d%02d",
		stm->tm_year+1900,
		stm->tm_mon+1,
		stm->tm_mday,
		stm->tm_hour,
		stm->tm_min,
		stm->tm_sec);

	return bufstr;
}


/** Convert a time in seconds (float) to an ascii string.
    @ingroup	screen
    @param	sec	Time to convert
    @param	sgn	Flag to indicate if the sign (+) is to be displayed for positive values of time.
    @return	Time string.
    @warning	The returned string has to be freed by the caller.
 */
char * GfTime2Str(tdble sec, int sgn)
{
	char buf[256];
	const char* sign = (sec < 0.0 ? "-" : (sgn ? "+" : "  ") );

	if (sec < 0.0) {
		sec = -sec;
		sign = "-";
	} else {
		if (sgn) {
			sign = "+";
		} else {
			sign = "  ";
		}
	}

	int h = (int)(sec / 3600.0);
	sec -= 3600 * h;
	int m = (int)(sec / 60.0);
	sec -= 60 * m;
	int s = (int)(sec);
	sec -= s;
	int c = (int)floor((sec) * 100.0);

	if (h) {
		(void)sprintf(buf, "%s%2.2d:%2.2d:%2.2d:%2.2d", sign,h,m,s,c);
	} else if (m) {
		(void)sprintf(buf, "   %s%2.2d:%2.2d:%2.2d", sign,m,s,c);
	} else {
		(void)sprintf(buf, "      %s%2.2d:%2.2d", sign,s,c);
	}
	return strdup(buf);
}


static char *localDir = NULL;
static char *libDir = NULL;
static char *dataDir = NULL;


char * GetLocalDir(void)
{
	return localDir;
}


void SetLocalDir(const char *buf)
{
	localDir = strdup(buf);
	GfOut("LocalDir='%s'\n", localDir);
}


char * GetLibDir(void)
{
	return libDir;
}


void SetLibDir(const char *buf)
{
	libDir = strdup(buf);
	GfOut("LibDir='%s'\n", libDir);
}


char * GetDataDir(void)
{
	return dataDir;
}


void SetDataDir(const char *buf)
{
	dataDir = strdup(buf);
	GfOut("DataDir='%s'\n", dataDir);
}


static int singleTextureMode = 0;


int GetSingleTextureMode (void)
{
	return singleTextureMode;
}


void SetSingleTextureMode (void)
{
	singleTextureMode = 1;
}


int GfNearestPow2 (int x)
{
	int r;

	if (!x) {
		return 0;
	}

	x++;
	r = 1;
	while ((1 << r) < x) {
		r++;
	}
	r--;

	return (1 << r);
}


int GfCreateDir(char *path)
{
	if (path == NULL) {
		return GF_DIR_CREATION_FAILED;
	}

	const int BUFSIZE = 1024;
	char buf[BUFSIZE];
	strncpy(buf, path, BUFSIZE);
	path = buf;

#ifdef WIN32
#define mkdir(x) _mkdir(x)

	// Translate path.
	const char DELIM = '\\';
	int i;
	for (i = 0; i < BUFSIZE && buf[i] != '\0'; i++) {
		if (buf[i] == '/') {
			buf[i] = DELIM;
		}
	}
	
#else // WIN32
#define mkdir(x) mkdir((x), S_IRWXU);

	const char DELIM = '/';

#endif // WIN32

	int err = mkdir(buf);
	if (err == -1) {
		if (errno == ENOENT) {
			char *end = strrchr(buf, DELIM);
			*end = '\0';
			GfCreateDir(buf);
			*end = DELIM;
			err = mkdir(buf);

		}
	}

	if (err == -1 && errno != EEXIST) {
		return GF_DIR_CREATION_FAILED;
	} else {
		return GF_DIR_CREATED;
	}
}

