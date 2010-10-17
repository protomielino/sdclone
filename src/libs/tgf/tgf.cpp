/***************************************************************************
                          tgf.cpp -- The Gaming Framework                            
                             -------------------                                         
    created              : Fri Aug 13 22:31:43 CEST 1999
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

#ifdef WIN32
#include <windows.h>
#include <direct.h>
#include <shlobj.h>
#else
#include <unistd.h> // getcwd
#endif

#include <cerrno>
#include <ctime>
#include <cstring>

#include <SDL/SDL.h>

#include "tgf.h"

#include "portability.h"


extern void gfTraceInit(void);
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

// Build a new path string compatible with current OS and usable as a command line arg.
static char* gfPathBuildCommandLineArg(const char *path)
{
#ifdef WIN32
  char *osPath = (char*)malloc(strlen(path)+3);
  sprintf(osPath, "\"%s", path);
  if (osPath[strlen(osPath)-1] == '/')
    osPath[strlen(osPath)-1] = 0; // Remove trailing '/' for command line
  strcat(osPath, "\"");
#else
  char *osPath = strdup(path);
#endif //WIN32
  
  GfPathMakeOSCompatible(osPath);
  
  return osPath;
}


/** Initialize the gaming framework.
    @ingroup	tgf
    @return	None
 */
void GfInit(void)
{
	gfTraceInit();
	gfDirInit();
	gfModInit();
	gfOsInit();
	gfParamInit();

	// Initialize SDL subsystems usefull for TGF.
	if (SDL_Init(SDL_INIT_TIMER) < 0)
		GfLogFatal("Couldn't initialize SDL(timer) (%s)\n", SDL_GetError());
}


/** Restart the gaming framework (restart the current process).
    @ingroup	tgf
    @param	sec	Time to convert
    @param	plus	String to display as the positive sign (+) for positive values of time.
    @return	None
    @warning	Never returns (retart the process).
 */
void GfRestart(bool bHardwareMouse, bool bMultiTexturing)
{
    int retcode = 0;
    static const int CMDSIZE = 1024;
    char cmd[CMDSIZE];

    char** args;
    int	i, nArgs;
    int	argInd;

    // Command name.
    sprintf(cmd, "%sspeed-dreams", GetBinDir());
#ifdef WIN32
    strcat(cmd, ".exe");
#endif
    GfPathMakeOSCompatible(cmd);

    // Compute number of args.
    nArgs = 1; // Executable is always the first arg.
    
    if (bHardwareMouse)
	  nArgs += 1;
    if (!bMultiTexturing)
	  nArgs += 1;
    if (GetLocalDir() && strlen(GetLocalDir()))
	  nArgs += 2;
    if (GetBinDir() && strlen(GetBinDir()))
	  nArgs += 2;
    if (GetLibDir() && strlen(GetLibDir()))
	  nArgs += 2;
    if (GetDataDir() && strlen(GetDataDir()))
	  nArgs += 2;

    nArgs++; // Last arg must be a null pointer.

    // Allocate args array.
    args = (char**)malloc(sizeof(char*)*nArgs);
	
    // First arg is the executable path-name.
    argInd = 0;
    args[argInd++] = gfPathBuildCommandLineArg(cmd);

    // Then add subsequent args.
    if (bHardwareMouse)
        args[argInd++] = strdup("-m");
    
    if (!bMultiTexturing)
        args[argInd++] = strdup("-s");
    
    if (GetLocalDir() && strlen(GetLocalDir()))
    {
        args[argInd++] = strdup("-l");
		args[argInd++] = gfPathBuildCommandLineArg(GetLocalDir());
    }

    if (GetBinDir() && strlen(GetBinDir()))
    {
        args[argInd++] = strdup("-B");
		args[argInd++] = gfPathBuildCommandLineArg(GetBinDir());
    }
	
    if (GetLibDir() && strlen(GetLibDir()))
    {
        args[argInd++] = strdup("-L");
		args[argInd++] = gfPathBuildCommandLineArg(GetLibDir());
    }
	
    if (GetDataDir() && strlen(GetDataDir()))
    {
        args[argInd++] = strdup("-D");
		args[argInd++] = gfPathBuildCommandLineArg(GetDataDir ());
    }
	
    // Finally, last null arg.
    args[argInd] = 0;
	  
    // Exec the command : restart the game (simply replacing current process)
    GfLogInfo("Restarting ");
    for (i = 0; args[i]; i++)
        GfLogInfo("%s ", args[i]);
    GfLogInfo("...\n");
    retcode = execvp(cmd, args);

    // If successfull, we never get here ... but if failed ...
    GfLogError("Failed to restart (exit code %d, %s)\n", retcode, strerror(errno));
    for (i = 0; args[i]; i++)
		free(args[i]);
    free(args);
    
    exit(1);
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


/** Convert a time in seconds (float) to an ascii string.
    @ingroup	tgf
    @param	sec	Time to convert
    @param	plus	String to display as the positive sign (+) for positive values of time.
    @param	zeros	Flag to indicate if heading zeros are to be displayed or not.
    @param	prec	Numer of figures to display after the decimal point (< 2 forced to 1).
    @return	Time string.
    @warning	The returned string has to be freed by the caller.
 */
char* GfTime2Str(double sec, const char* plus, bool zeros, int prec)
{
	char* buf = (char*)malloc(256*sizeof(char));
	
	const char* sign = (sec < 0.0) ? "-" : (plus ? plus : "");
	if (sec < 0.0)
		sec = -sec;

	// Hours.
	const int h = (int)(sec / 3600.0);
	sec -= 3600 * h;

	// Minutes.
	const int m = (int)(sec / 60.0);
	sec -= 60 * m;

	// Seconds.
	const int s = (int)sec;
	sec -= s;

	// Fractions of the second (limited resolution).
	int mult = 10;
	int digits = prec;
	while (digits-- > 1)
		mult *= 10;
	const int f = (int)floor(sec * mult);

	if (h || zeros) {
		(void)sprintf(buf, "%s%2.2d:%2.2d:%2.2d.%0.*d", sign, h, m, s, prec, f);
	} else if (m) {
		(void)sprintf(buf, "   %s%2.2d:%2.2d.%0.*d", sign, m, s, prec, f);
	} else {
		(void)sprintf(buf, "      %s%2.2d.%0.*d", sign, s, prec, f);
	}
	return buf;
}

/** In-place convert internal file or dir path to an OS compatible path
    @ingroup	tgf
    @param	path	The path to convert
    @return	The converted path.
*/
// In-place convert internal file or dir path to an OS compatible path
char* GfPathMakeOSCompatible(char* path)
{
#ifdef WIN32
  size_t i;
  for (i = 0; i < strlen(path); i++)
	if (path[i] == '/')
	  path[i] = '\\';
#endif //WIN32
  return path;
}

// Determine if a dir or file path is absolute or not.
bool GfPathIsAbsolute(const char *pszPath)
{
	return pszPath != 0 && strlen(pszPath) > 0
#ifdef WIN32
		   && (pszPath[0] == '/'  // Leading '/'
			   || pszPath[0] == '\\' // Leading '\'
			   || (strlen(pszPath) > 2 && pszPath[1] == ':'
				   && (pszPath[2] == '/' || pszPath[2] == '\\'))); // Leading 'x:/' or 'x:\'
#else
	       && pszPath[0] == '/' ; // Leading '/'
#endif
}

// Normalize a directory path in-place : \ to / conversion + mandatory unique trailing /.
char* GfPathNormalizeDir(char* pszPath, size_t nMaxPathLen)
{
#ifdef WIN32
	// Replace '\' by '/'
	size_t i;
	for (i = 0; i < strlen(pszPath); i++)
		if (pszPath[i] == '\\')
			pszPath[i] = '/';
#endif

	// Add a trailing '/' if not present.
	if (pszPath[strlen(pszPath)-1] != '/')
		if (strlen(pszPath) < nMaxPathLen - 1)
			strcat(pszPath, "/");
		else
			GfLogFatal("Path '%s' too long ; could not normalize\n", pszPath);

	return pszPath;
}

/* Game run-time folders :
   - installDir : The folder containing the parent folder of the game executable
   - localDir : User settings (should be ~/.speed-dreams or <My documents>/speed-dreams.settings)
   - libDir   : Modules and shared libs installation folder (+ binaries under 'nixes)
   - binDir   : Executables (and/or scripts under 'nixes) installation folder
   - dataDir  : Static data (tracks, cars, textures, ...) installation folder
*/
static char* gfInstallDir = 0;
static char* gfLocalDir = 0;
static char* gfLibDir = 0;
static char* gfDataDir = 0;
static char* gfBinDir = 0;

/* Translate a directory path into a run-time dir path :
   - ~ management, 
   - \ to / conversion, 
   - mandatory unique trailing /,
   - if not absolute, make absolute through gfInstallDir if already available,
     or through getcwd otherwise.
   Warning: The returned path is allocated on the heap (malloc) and must be free'd by the caller. 
*/

static char* makeRunTimeDirPath(const char* srcPath)
{
	static const size_t bufSize = 512;
	
	// Allocate target buffer (must be freed by caller when useless).
	char* tgtPath = (char *)malloc(bufSize);
	tgtPath[0] = 0;

	// If the path starts with a ~, substitute ~ with $HOME / <My documents>
	// (to give the user an easy access to advanced settings).
	if (strlen(srcPath) > 0 && srcPath[0] == '~'
		&& (strlen(srcPath) == 1 || (srcPath[1] == '/' || srcPath[1] == '\\')))
	{
#ifdef WIN32
		LPITEMIDLIST pidl;
		if (!SUCCEEDED(SHGetSpecialFolderLocation(NULL, CSIDL_PERSONAL, &pidl))
			|| !SHGetPathFromIDList(pidl, tgtPath))
			GfLogFatal("Could not get user's My documents folder path\n");
#else
		const char* pszHomeDir = getenv("HOME");
		if (pszHomeDir && strlen(pszHomeDir) > 0)
			strcpy(tgtPath, pszHomeDir);
		else
			GfLogFatal("Could not get user's HOME folder path, or it is empty\n");
#endif
		if (strlen(tgtPath) + strlen(srcPath) - 1 < bufSize - 1)
			strcat(tgtPath, srcPath+1); // Don't keep the ~.
		else
		{
			free(tgtPath);
			tgtPath = 0;
		}
	}

	// If the path is not already an absolute one,
	// prefix it with the install dir if we know it already.
	else if (strlen(srcPath) > 0 && srcPath[0] != '/' && srcPath[0] != '\\'
			 && !(strlen(srcPath) > 1 && srcPath[1] == ':'))
	{
		if (gfInstallDir)
			strcpy(tgtPath, gfInstallDir);
		else
		{
			getcwd(tgtPath, bufSize);
			strcat(tgtPath, "/");
		}
		if (!strcmp(srcPath, "."))
			; // Nothing more to append.
		else if (strlen(tgtPath) + strlen(srcPath) < bufSize - 1)
			strcat(tgtPath, srcPath);
		else
		{
			free(tgtPath);
			tgtPath = 0;
		}
	}

	// Already an absolute path : simply copy it.
	else
		strcpy(tgtPath, srcPath);

	// Fix \ and add a trailing / is needed.
	if (tgtPath)
		GfPathNormalizeDir(tgtPath, bufSize - 1);

	if (!tgtPath)
		GfLogFatal("Path '%s' too long ; could not make as a run-time path\n", srcPath);

	return tgtPath;
}


// Accessors to game run-time folders
const char* GfGetInstallDir(void)
{
	return gfInstallDir;
}

void GfInitInstallDir(const char *pszExecutablePath)
{
	if (gfInstallDir)
		free(gfInstallDir);

	// Search for the last path separator and cut there.
	char pszPath[512];
	strcpy(pszPath, pszExecutablePath);
	char* pLastPathSep = strrchr(pszPath, '/');
#ifdef WIN32
	if (!pLastPathSep)
		pLastPathSep = strrchr(pszPath, '\\');
#endif
	if (!pLastPathSep)
		pLastPathSep = pszPath;
	*pLastPathSep = 0;
	gfInstallDir = makeRunTimeDirPath(pszPath);

	// If the path to the folder where the executable is stored ends with SD_BINDIR,
	// then the install dir path ends right at the beginning of SD_BINDIR.
	char* pBinDir = strstr(gfInstallDir, SD_BINDIR);
	if (pBinDir - gfInstallDir == strlen(gfInstallDir) - strlen(SD_BINDIR))
	{
		*pBinDir = 0;
	}	
	// Otherwise, let's consider the install dir is the current dir
	// (quite strange : the executable is not in SD_BINDIR ? When can this happen ?).
	else
	{
		getcwd(pszPath, 512);
		gfInstallDir = makeRunTimeDirPath(pszPath);
	}
	
	GfLogInfo("Install dir is %s (from executable %s)\n", gfInstallDir, pszExecutablePath);
}

const char* GetLocalDir(void)
{
	return gfLocalDir;
}

const char* SetLocalDir(const char *pszPath)
{
	if (gfLocalDir)
		free(gfLocalDir);
	gfLocalDir = makeRunTimeDirPath(pszPath);
	GfLogInfo("User settings in %s (from %s)\n", gfLocalDir, pszPath);
	return gfLocalDir;
}

const char* GetLibDir(void)
{
	return gfLibDir;
}

const char* SetLibDir(const char *pszPath)
{
	if (gfLibDir)
		free(gfLibDir);
	gfLibDir = makeRunTimeDirPath(pszPath);
	GfLogInfo("Libraries in %s (from %s)\n", gfLibDir, pszPath);
	return gfLibDir;
}

const char* GetDataDir(void)
{
	return gfDataDir;
}

const char* SetDataDir(const char *pszPath)
{
	if (gfDataDir)
		free(gfDataDir);
	gfDataDir = makeRunTimeDirPath(pszPath);
	GfLogInfo("Data in %s (from %s)\n", gfDataDir, pszPath);
	return gfDataDir;
}

const char* GetBinDir(void)
{
	return gfBinDir;
}

const char* SetBinDir(const char *pszPath)
{
	if (gfBinDir)
		free(gfBinDir);
	gfBinDir = makeRunTimeDirPath(pszPath);
	GfLogInfo("Executables in %s (from %s)\n", gfBinDir, pszPath);
	return gfBinDir;
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
