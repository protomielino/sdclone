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
#if defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
#include <sys/param.h>
#include <sys/sysctl.h>
#else
#include <unistd.h>
#endif
#endif
#include <errno.h>

#include <time.h>
#include <cstring>

#include "tgf.h"

#include "portability.h"


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

/**
 * This function allocates memory and returns a pointer to it of size @p size.
 * This pointer *must* be free'ed with the GfPoolFree function, or *must*
 * be free'ed by destroying the whole memory pool with GfPoolFreePool.
 * For a given pool, the first call to GfPoolMalloc must get *pool = 0.
 *
 * @param size The size of the pointer to allocate
 * @param pool Pointer to a memory pool
 * @return Newly created pointer of size @p size
 */
void* GfPoolMalloc(size_t size, tMemoryPool* pool)
{
	tMemoryPoolItem *data;
	
	if( !pool )
		return 0;

	/* Init tMemoryPool structure */
	data = (tMemoryPoolItem*)malloc( sizeof(tMemoryPoolItem) + size );
	data->prev = NULL;
	data->next = *pool;
	data->pool = pool;

	/* Insert in front of the pool */
	if( data->next )
	{
		data->next->pool = NULL; /* Zero pool: not first any more ... */
		data->next->prev = data; /* ... and now has a previous item */
	}
	*pool = data;

	return (void*)( data + 1 );
}

/**
 * Free a pointer created with GfPoolMalloc.
 *
 * @param pointer Pointer created with GfPoolMalloc which must be free'ed.
 */
void GfPoolFree(void* pointer)
{
	tMemoryPoolItem *data = ((tMemoryPoolItem*)pointer)-1;

	if( !pointer )
		return;

	if( data->next )
		data->next->prev = data->prev;
	if( data->prev )
		data->prev->next = data->next;
	else
	{
		/* Delete first from list, correct pool */
		*data->pool = data->next;
		if( data->next )
			data->next->pool = data->pool;
	}

	free( data );
}

/**
 * Free all the pointers in the memory pool
 *
 * @param pool The memory pool which must be free'ed.
 */
void GfPoolFreePool(tMemoryPool* pool)
{
	tMemoryPoolItem *cur;
	tMemoryPoolItem *prev = NULL;

	if( !pool )
		return;

	cur = *pool;

	/* Zero the pool */
	*pool = NULL;

	while( cur )
	{
		prev = cur;
		cur = cur->next;

		free( prev );
	}
}

/**
 * Move all the pointers from one pool to another pool
 *
 * @param oldpool Old pool
 * @param newpool New pool
 */
void GfPoolMove(tMemoryPool* oldpool, tMemoryPool* newpool)
{
	*newpool = *oldpool;
	*oldpool = NULL;

	if( *newpool )
		(*newpool)->pool = newpool;
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
		(void)sprintf(buf, "%s%2.2d:%2.2d:%2.2d.%2.2d", sign,h,m,s,c);
	} else if (m) {
		(void)sprintf(buf, "   %s%2.2d:%2.2d.%2.2d", sign,m,s,c);
	} else {
		(void)sprintf(buf, "      %s%2.2d.%2.2d", sign,s,c);
	}
	return strdup(buf);
}

// Convert any path to an absolute one.
#ifdef WIN32
#define absolutePath _fullpath
#else
static char *absolutePath(char *absPath, const char *srcPath, int maxLength)
{
	// TODO : really compute an absolute path (if not heading / or ~, prepend cwd+/)
	strcpy(absPath, srcPath);

	return absPath;
}
#endif // WIN32

/* Normalize a directory path (~ management, \ to / conversion, mandatory unique trailing /).
   Warning: The returned path is allocated on the heap (malloc) and must be free'd by the caller. 
*/
static char* normalizeDirPath(const char* srcPath)
{
	static const size_t bufSize = 1024;
	
	// Allocate target buffer (must be freed by caller when useless).
	char* absPath = (char *)malloc(bufSize);

	// Some Linux addicted user may have escaped ...
	// Build absolute path
	if (srcPath[0] == '~')
	{
#ifdef WIN32
		strcpy(absPath, getenv("USERPROFILE"));
#else
		strcpy(absPath, getenv("HOME"));
#endif
		const size_t i = strlen(absPath);
		size_t j;
		const size_t k = strlen(srcPath);
		for (j = 1; j < k; ++j)
			absPath[i+j-1] = srcPath[j];
		absPath[i+k-1] = '\0';
	}
	else if (!absolutePath(absPath, srcPath, bufSize))
	{
		free(absPath);
		absPath = 0;
	}

	if (absPath)
	{
#ifdef WIN32
		// Replace '\' by '/' in pathes
		size_t i;
		for (i = 0; i < strlen(absPath); i++) {
			if (absPath[i] == '\\') {
				absPath[i] = '/';
			}
		}
#endif

		// Add a trailing '/' if not present.
		if (absPath[strlen(absPath)-1] != '/')
		{
			if (strlen(absPath) < bufSize)
				strncat(absPath, "/", 1);
			else
			{
				free(absPath);
				absPath = 0;
			}
		}
	}

	if (!absPath)
		GfError("Warning: Path '%s' too long ; ignored when made absolute\n", srcPath);

	return absPath;
}


/* Game run-time folders :
   - localDir : User settings (should be ~/.speed-dreams of <My documents>/speed-dreams.settings)
   - libDir   : Modules and shared libs installation folder (+ binaries under 'nixes)
   - binDir   : Executables (and/or scripts under 'nixes) installation folder
   - dataDir  : Static data (tracks, cars, textures, ...) installation folder
*/
static char *localDir = NULL;
static char *libDir = NULL;
static char *dataDir = NULL;
static char *binDir = NULL;

const char * GetLocalDir(void)
{
	return localDir;
}

const char * SetLocalDir(const char *buf)
{
	if (localDir)
		free(localDir);
	localDir = normalizeDirPath(buf);
	GfOut("User settings in %s\n", localDir);
	return localDir;
}

const char * GetLibDir(void)
{
	return libDir;
}

const char * SetLibDir(const char *buf)
{
	if (libDir)
		free(libDir);
	libDir = normalizeDirPath(buf);
	GfOut("Libraries in %s\n", libDir);
	return libDir;
}

const char * GetDataDir(void)
{
	return dataDir;
}

const char * SetDataDir(const char *buf)
{
	if (dataDir)
		free(dataDir);
	dataDir = normalizeDirPath(buf);
	GfOut("Data in %s\n", dataDir);
	return dataDir;
}

const char * GetBinDir(void)
{
	return binDir;
}

const char * SetBinDir(const char *buf)
{
	if (binDir)
		free(binDir);
	binDir = normalizeDirPath(buf);
	GfOut("Executables in %s\n", binDir);
	return binDir;
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


// Nearest power of 2 integer
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

// Create a directory
int GfCreateDir(const char *path)
{
	if (path == NULL) {
		return GF_DIR_CREATION_FAILED;
	}

	static const int nPathBufSize = 1024;
	char buf[nPathBufSize];
	strncpy(buf, path, nPathBufSize);

#ifdef WIN32

	// Translate path.
	static const char cPathSeparator = '\\';
	int i;
	for (i = 0; i < nPathBufSize && buf[i] != '\0'; i++)
		if (buf[i] == '/')
			buf[i] = cPathSeparator;
	
#else // WIN32

// mkdir with u+rwx access grants by default
#ifdef mkdir
# undef mkdir
#endif
#define mkdir(x) mkdir((x), S_IRWXU)

	static const char cPathSeparator = '/';

#endif // WIN32

	// Try to create the requested folder.
	int err = mkdir(buf);

	// If this fails, try and create the parent one (recursive), and the retry.
	if (err == -1 && errno == ENOENT)
	{
		// Try the parent one (recursive).
		char *end = strrchr(buf, cPathSeparator);
		*end = '\0';
		GfCreateDir(buf);

		// Retry.
		*end = cPathSeparator;
		err = mkdir(buf);
	}
	
	return (err == -1 && errno != EEXIST) ? GF_DIR_CREATION_FAILED : GF_DIR_CREATED;
}

/* Get the actual number of CPUs / cores

   TODO: Be careful about fake CPUs like those displayed by hyperthreaded processors ...
   TODO: Test under pltaforms other than Windows, Linux, Solaris, AIX (mainly BDS and MacOS X).
*/
int GfGetNumberOfCPUs()
{
	int nCPUs = 0;
	
// Windows
#if defined(WIN32)

	SYSTEM_INFO sysinfo;
	GetSystemInfo(&sysinfo);

	nCPUs = sysinfo.dwNumberOfProcessors;

// MacOS X, FreeBSD, OpenBSD, NetBSD, etc ...
#elif defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)

	nt mib[4];
	size_t len; 

	// Set the mib for hw.ncpu

	// Get the number of CPUs from the system
	// 1) Try HW_AVAILCPU first.
	mib[0] = CTL_HW;
	mib[1] = HW_AVAILCPU;  // alternatively, try HW_NCPU;
	sysctl(mib, 2, &nCPUs, &len, NULL, 0);

	if (nCPUs < 1) 
	{
		// 2) Try alternatively HW_NCPU.
		mib[1] = HW_NCPU;
		sysctl(mib, 2, &nCPUs, &len, NULL, 0);
	}

// Linux, Solaris, AIX
#elif defined(linux) || defined(__linux__)

	nCPUs = sysconf(_SC_NPROCESSORS_ONLN);

#else

#warning "Unsupported OS"

#endif // WIN32

	if (nCPUs < 1)
	{
		GfOut("Could not get the number of CPUs here ; assuming only 1\n");
		nCPUs = 1;
	}
	else
		GfOut("Detected %d CPUs\n", nCPUs);

	return nCPUs;
}
