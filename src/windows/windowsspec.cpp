/***************************************************************************

    file                 : windowsspec.cpp
    created              : Sat Sep  2 10:45:39 CEST 2000
    copyright            : (C) 2000 by Patrice & Eric Espie
    email                : torcs@free.fr
    version              : $Id: windowsspec.cpp,v 1.12 2005/08/05 10:34:47 berniw Exp $

***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include <cstddef>
#include <sys/types.h>
#include <time.h>
#include <direct.h>
#include <io.h>
#include <windows.h>

#include <tgf.h>
#include <os.h>


/*
* Function
*	windowsModLoad
*
* Description
*	Load the module of given DLL file
*	(Load the DLL, then retrieve info about the module (tModInfo struct) ;
*	 the DLL is NOT unloaded).
*
* Parameters
*	soPath  (in)     path of the DLL file to load
*	modlist	(in/out) list of module interfaces description structure (may begin empty)
*
* Return
*	0	Ok
*	-1	error
*
* Remarks
*	The loaded module info structure is added at the HEAD of the list (**modlist).
*	
*/
static int
windowsModLoad(unsigned int /* gfid */, const char *soPath, tModList **modlist)
{
    tSOHandle	 handle;
    tModList	*curMod;
    
    /* Try and avoid loading the same module twice (WARNING: Only checks soPath equality !) */
    if ((curMod = GfModIsInList(soPath, *modlist)) != 0)
    {
      GfOut("Module %s already loaded\n", soPath);
      GfModMoveToListHead(curMod, modlist); // Force module to be the first in the list.
      return 0;
    }

    GfOut("Loading module %s\n", soPath);
    
    char fname[256];
    const char* lastSlash = strrchr(soPath, '/');
    if (lastSlash) 
	strcpy(fname, lastSlash+1);
    else
	strcpy(fname, soPath);
    fname[strlen(fname) - 4] = 0; /* cut .dll */

    /* Load the DLL */
    handle = LoadLibrary( soPath ); 
    if (handle) 
    {
        if (GfModInitialize(handle, soPath, GfIdAny, &curMod) == 0)
		{
			if (curMod) /* Retained against GfIdAny */
			// Add the loaded module at the head of the list (no sort by priority).
			GfModAddInList(curMod, modlist, /* priosort */ 0);
		}
		else 
		{
			FreeLibrary(SOHandle(handle));
			GfError("windowsModLoad: Module init function failed %s\n", soPath);
			return -1;
		}
    }
    else
    {
		GfError("windowsModLoad: ...  can't open dll %s\n", soPath);
		return -1;
    }
      
    GfOut("Module %s loaded\n",soPath);
    return 0;
}

/*
* Function
*	windowsModInfo
*
* Description
*	Retrieve info about the module of given DLL file,
*	(Load the DLL, then retrieve info about the module (tModInfo struct),
*	 and finally unload the DLL).
*
* Parameters
*	soPath  (in)     path of the DLL file to load
*	modlist	(in/out) list of module interfaces description structure (may begin empty)
*
* Return
*	0	Ok
*	-1	error
*
* Remarks
*	The loaded module info structure is added at the HEAD of the list (**modlist).
*	
*/
static int
windowsModInfo(unsigned int /* gfid */, const char *soPath, tModList **modlist)
{
    tSOHandle  handle;
    tModList  *curMod;
    int        infoSts = 0;
    
    /* Try and avoid loading the same module twice (WARNING: Only checks soPath equality !) */
    if ((curMod = GfModIsInList(soPath, *modlist)) != 0)
    {
      GfOut("Module %s already loaded\n", soPath);
      GfModMoveToListHead(curMod, modlist); // Force module to be the first in the list.
      return infoSts;
    }

    GfOut("Querying module %s\n", soPath);

    /* Load the DLL */
    handle = LoadLibrary( soPath );
    if (handle)
    {
	/* Initialize the module */
        if (GfModInitialize(handle, soPath, GfIdAny, &curMod) == 0) 
	{
	    if (curMod) /* Retained against GfIdAny */
	    {
	        // Add the loaded module at the head of the list (no sort by priority).
	        GfModAddInList(curMod, modlist, /* priosort */ 0);
	    }

	    /* Terminate the module */
	    infoSts = GfModTerminate(handle, soPath);
	}
	else 
	{
	    GfOut("windowsModInfo: Module init function failed %s\n", soPath);
	    infoSts = -1;
	}
	
	/* Unload the DLL */
	FreeLibrary(SOHandle(handle));
    } 
    else
    {
	GfError("windowsModInfo: ...  %d\n", GetLastError());
	infoSts = -1;
    }
    
    return infoSts;
}

/*
* Function
*	windowsModLoadDir
*
* Description
*	Load the modules whose DLL files are contained in a given directory
*	(for each DLL, load it and retrieve info about the module (tModInfo struct) ;
*	 the DLL is NOT unloaded)
*
* Parameters
*	gfid    (in)		id of the gaming framework of the modules to load,
*	dir     (in)		directory to search (relative)
*	modlist (in/out)	list of module description structure (may begin empty)
*
* Return
*	>=0	number of modules loaded
*	-1	error
*
* Remarks
*	The loaded module info structures are added in the list according to each module's priority
*	(NOT at the head of the list).
*	
*/
static int
windowsModLoadDir(unsigned int gfid, const char *dir, tModList **modlist)
{
    char	soPath[256];	/* path of the lib[x].so */
    tSOHandle	handle;		/* */
    int		modnb;		/* number on loaded modules */
    tModList	*curMod;
    
    modnb = 0;
    curMod = (tModList*)calloc(1, sizeof(tModList));

    // Scan directory
    _finddata_t FData;
    char Dir_name[ 1024 ];
    sprintf( Dir_name, "%s\\*.dll", dir );
    long Dirent = _findfirst( Dir_name, &FData );
    if ( Dirent != -1 )
    {
	do 
	{
	    sprintf(soPath, "%s\\%s", dir, FData.name);
	    /* Try and avoid loading the same module twice (WARNING: Only checks soPath equality !) */
	    if (!GfModIsInList(soPath, *modlist))
	    {
		/* Load the DLL */
		GfOut("Loading module %s\n", soPath);
	        handle = LoadLibrary( soPath );
		if (handle)
		{
		    /* Initialize the module */
		    if (GfModInitialize(handle, soPath, gfid, &curMod) == 0)
		    {
		        if (curMod) /* Retained against gfid */
			{
			    modnb++;
			    GfModAddInList(curMod, modlist, /* priosort */ 1);
			}
		    }
		    else 
		    {
		        FreeLibrary(SOHandle(handle));
			modnb = -1;
			break;
		    }
		} 
		else
		{
		    GfError("windowsModLoadDir: ...  %s\n", GetLastError());
		    modnb = -1;
		    break;
		}
	    }
	} 
	while ( _findnext( Dirent, &FData ) != -1 );
    }
    _findclose( Dirent );

    return modnb;
}
/*
* Function
*	windowsModInfoDir
*
* Description
*	Retrieve info about the modules whose DLL files are contained in a given directory
*	(for each DLL, load it, retrieve info about the module (tModInfo struct),
*	 and finally unload the DLL).
*
* Parameters
*	dir	(in)     directory to search (relative)
*	level   (in)     if 1, load any DLL contained in the subdirs of dir
*	                 and whose name is the same as the containing subdir (ex: bt/bt.so)
*	                 if 0, load any DLL contained in dir (ignore subdirs)
*	modlist	(in/out) list of module description structure (may begin empty)
*
* Return
*	>=0	number of modules loaded
*	-1	error
*
* Remarks
*	The loaded module info structures are added in the list according to each module's priority
*	(NOT at the head of the list).
*	
*/
static int
windowsModInfoDir(unsigned int /* gfid */, const char *dir, int level, tModList **modlist)
{
    char	soPath[256];	/* path of the lib[x].so */
    tSOHandle	handle;		/* */
    int		modnb;		/* number on loaded modules */
    tModList	*curMod;
    
    modnb = 0;
    curMod = (tModList*)calloc(1, sizeof(tModList));
    
    /* open the current directory */
    _finddata_t FData;

    char Dir_name[ 1024 ];
    sprintf( Dir_name, "%s\\*.*", dir );
    GfOut("trying dir info %s\n",dir);
    long Dirent = _findfirst( Dir_name, &FData );
    if ( Dirent != -1 )
    {
	do 
	{
	    if (((strlen(FData.name) > 5) && 
		 (strcmp(".dll", FData.name+strlen(FData.name)-4) == 0)) /* xxxx.dll */
		|| (level == 1 && FData.name[0] != '.'))
	    {
		if (level == 1) 
		    sprintf(soPath, "%s/%s/%s.dll", dir, FData.name, FData.name);
		else
		    sprintf(soPath, "%s/%s", dir, FData.name);

		/* Try and avoid loading the same module twice (WARNING: Only checks soPath equality !) */
		if (!GfModIsInList(soPath, *modlist))
		{
		    /* Load the DLL */
		    GfOut("Querying module %s\n", soPath);
		    handle = (tSOHandle)LoadLibrary( soPath );
		    if (handle)
		    {
			/* Initialize the module */
		        if (GfModInitialize(handle, soPath, GfIdAny, &curMod) == 0)
			{
			    if (curMod) /* Retained against GfIdAny */
			    {
				/* Get associated info */
				GfModAddInList(curMod, modlist, /* priosort */ 1);
			        modnb++;
			    }

			    /* Terminate the module */
			    GfModTerminate(handle, soPath);
			} 
			else
			{
			    GfOut("windowsModInfo: Module init function failed %s\n", soPath);
			    modnb = -1;
			    break;
			}

			/* Close the DLL */
			FreeLibrary(SOHandle(handle));
		    } 
		    else 
		    {
		        GfTrace("windowsModInfoDir: ...  can't open dll %s\n", soPath);
		    }
		}
	    }
	} 
	while ( _findnext( Dirent, &FData ) != -1 );
    }
    _findclose( Dirent );

    return modnb;
}

/*
* Function
*	windowsModUnloadList
*
* Description
*	Unload the modules of a list
*
* Parameters
*	modlist	(in/out) list of modules to unload
*
* Return
*	0	Ok
*	-1	Error
*
* Remarks
*	
*/
static int
windowsModUnloadList(tModList **modlist)
{
    tModList	*curMod;
    tModList	*nextMod;
    int         termSts;
    int         unloadSts = 0;
    
    curMod = *modlist;
    if (curMod == 0)
	return 0;

    do
    {
	nextMod = curMod->next;

	if (curMod->handle)
	{
	    termSts = GfModTerminate(curMod->handle, curMod->sopath);
	    if (termSts)
		unloadSts = termSts;

	    FreeLibrary(SOHandle(curMod->handle));
	}
	GfOut("Unloaded module %s\n", curMod->sopath);

	GfModInfoFreeNC(curMod->modInfo, curMod->modInfoSize);
	free(curMod->sopath);
	free(curMod);

	curMod = nextMod;
    }
    while (curMod != *modlist);
    
    *modlist = (tModList *)NULL;
    return unloadSts;
}

/*
* Function
*	windowsDirGetList
*
* Description
*	Get a list of entries in a directory
*
* Parameters
*	directory name
*
* Return
*	list of directory entries
*/
static tFList *
windowsDirGetList(const char *dir)
{
    tFList	*flist = NULL;
    tFList	*curf;
	
    _finddata_t FData;
    char Dir_name[ 1024 ];
    sprintf( Dir_name, "%s\\*.*", dir );
    GfOut("trying dir %s\n",dir);
    long Dirent = _findfirst( Dir_name, &FData );
    if ( Dirent != -1 ) {
	do {
	    if ( strcmp(FData.name, ".") != 0 && strcmp(FData.name, "..") != 0 ) {
		curf = (tFList*)calloc(1, sizeof(tFList));
		curf->name = strdup(FData.name);
		if (flist == (tFList*)NULL) {
		    curf->next = curf;
		    curf->prev = curf;
		    flist = curf;
		} else {
		    /* sort entries... */
		    if (_stricmp(curf->name, flist->name) > 0) {
			do {
			    flist = flist->next;
			} while ((stricmp(curf->name, flist->name) > 0) && (stricmp(flist->name, flist->prev->name) > 0));
			flist = flist->prev;
		    } else {
			do {
			    flist = flist->prev;
			} while ((stricmp(curf->name, flist->name) < 0) && (stricmp(flist->name, flist->next->name) < 0));
		    }
		    curf->next = flist->next;
		    flist->next = curf;
		    curf->prev = curf->next->prev;
		    curf->next->prev = curf;
		    flist = curf;
		}
	    }
	} while ( _findnext( Dirent, &FData ) != -1 );
    }
    
    return flist;
}

/*
* Function
*	windowsDirGetListFiltered
*
* Description
*	Get a list of entries in a directory
*
* Parameters
*	dir : directory name
*	prefix : file name prefix to match (may be NULL)
*	suffix : file name suffix to match (may be NULL)
*
* Return
*	list of directory entries
*/
static tFList *
windowsDirGetListFiltered(const char *dir, const char *prefix, const char *suffix)
{
    tFList *flist = NULL;
    tFList *curf;
    int	prefixLg, suffixLg;
    int fnameLg;

    if ((!prefix || strlen(prefix) == 0) && (!suffix || strlen(suffix) == 0))
	return windowsDirGetList(dir);

    suffixLg = suffix ? strlen(suffix) : 0;
    prefixLg = prefix ? strlen(prefix) : 0;
	
    _finddata_t FData;
    char Dir_name[1024];
    sprintf(Dir_name, "%s\\*.*", dir);
    long Dirent = _findfirst(Dir_name, &FData);
    if (Dirent != -1) {
	do {
	    fnameLg = strlen(FData.name);
	    if ((!prefix || (fnameLg > prefixLg
						 && strncmp(FData.name, prefix, prefixLg) == 0))
			&& (!suffix || (fnameLg > suffixLg
							&& strncmp(FData.name + fnameLg - suffixLg, suffix, suffixLg) == 0))) {
		curf = (tFList*)calloc(1, sizeof(tFList));
		curf->name = strdup(FData.name);
		curf->dispName = 0;
		curf->userData = 0;
		if (flist == (tFList*)NULL) {
		    curf->next = curf;
		    curf->prev = curf;
		    flist = curf;
		} else {
		    /* sort entries... */
		    if (_stricmp(curf->name, flist->name) > 0) {
			do {
			    flist = flist->next;
			} while (stricmp(curf->name, flist->name) > 0
					 && stricmp(flist->name, flist->prev->name) > 0);
			flist = flist->prev;
		    } else {
			do {
			    flist = flist->prev;
			} while (stricmp(curf->name, flist->name) < 0
					 && stricmp(flist->name, flist->next->name) < 0);
		    }
		    curf->next = flist->next;
		    flist->next = curf;
		    curf->prev = curf->next->prev;
		    curf->next->prev = curf;
		    flist = curf;
		}
	    }
	} while (_findnext( Dirent, &FData ) != -1);
    }
    
    return flist;
}

static double
windowsTimeClock(void)
{
    LARGE_INTEGER Frequency;
    if ( !QueryPerformanceFrequency( &Frequency ) )
	return( 0 );

    LARGE_INTEGER Counter;
    if ( !QueryPerformanceCounter( &Counter ) )
	return( 0 );

    double D = (double)Counter.QuadPart / (double)Frequency.QuadPart;
    return( D );
}

/*
* Function
*	windowsGetOSInfo
*
* Description
*	Get some info about the running Windows OS.
*
* Parameters
*	pnMajor : pointer to the target OS major version integer
*	pnMinor : pointer to the target OS minor version integer
*	pnBits  : pointer to the target OS number of bits (32 or 64) integer
*
* Return
*	none
*/
static bool
windowsGetOSInfo(int* pnMajor, int* pnMinor, int* pnBits)
{
    static char pszVerSionString[512];

    OSVERSIONINFOEX osvi;
    SYSTEM_INFO si;
    typedef void (WINAPI *PGNSI)(LPSYSTEM_INFO);
    PGNSI pGNSI;

    ZeroMemory(&si, sizeof(SYSTEM_INFO));
    ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
    
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    
    if (!GetVersionEx((OSVERSIONINFO*)&osvi))
    {
	GfOut("Error: Could not get Windows OS version info");
	return false;
    }

    // Call GetNativeSystemInfo if supported or GetSystemInfo otherwise.
    pGNSI = (PGNSI)GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")), "GetNativeSystemInfo");
    if (pGNSI)
	pGNSI(&si);
    else 
	GetSystemInfo(&si);

    *pnMajor = osvi.dwMajorVersion;
    *pnMinor = osvi.dwMinorVersion;
    *pnBits  = 32; // Default value, fixed afterward if necessary.

    strcpy(pszVerSionString, "Windows ");

    // Windows <= 4
    if (osvi.dwPlatformId != VER_PLATFORM_WIN32_NT || osvi.dwMajorVersion <= 4)
    {
	strcat(pszVerSionString, "NT 4 or older");
    }

    // Windows Vista
    else if (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 0)
    {
 	    if( osvi.wProductType == VER_NT_WORKSTATION)
		strcat(pszVerSionString, "Vista ");
	    else 
		strcat(pszVerSionString, "Server 2008 ");
    }

    // Windows 7
    else if (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 1)
    {
	if( osvi.wProductType == VER_NT_WORKSTATION)
	    strcat(pszVerSionString, "7 ");
	else 
	    strcat(pszVerSionString, "Server 2008 R2 ");
    }

    // Windows Server or XP Pro x64
    else if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 2)
    {
	if (osvi.wProductType == VER_NT_WORKSTATION
	    && si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_AMD64)
	{
	    *pnBits = 64;
	    strcat(pszVerSionString, "XP Professional x64 Edition");
	}
	else 
	    strcat(pszVerSionString, 
		   "Server 2003 / 2003 R2 / Storage Server 2003 or Home Server, ");

	// Test for the server type.
	if (osvi.wProductType != VER_NT_WORKSTATION)
	{
	    if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64)
	    {
		*pnBits = 64;
		strcat(pszVerSionString, "Some Itanium Edition");
	    }

	    else if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64)
	    {
		*pnBits = 64;
		strcat(pszVerSionString, "Some x64 Edition");
	    }

	    else
		strcat(pszVerSionString, "Some 32bit Edition");
	}
    }

    // Windows XP
    else if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1)
    {
	strcat(pszVerSionString, "XP ");
	if (osvi.wSuiteMask & VER_SUITE_PERSONAL)
	    strcat(pszVerSionString, "Home Edition");
	else 
	    strcat(pszVerSionString, "Professional");
    }

    // Windows 2000
    else if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0)
    {
	strcat(pszVerSionString, "2000 ");

	if (osvi.wProductType == VER_NT_WORKSTATION)
	{
	    strcat(pszVerSionString, "Professional");
	}
	else 
	{
	    if (osvi.wSuiteMask & VER_SUITE_DATACENTER)
		strcat(pszVerSionString, "Datacenter Server");
	    else if (osvi.wSuiteMask & VER_SUITE_ENTERPRISE)
		strcat(pszVerSionString, "Advanced Server");
	    else 
		strcat(pszVerSionString, "Server");
	}
    }

    // Include service pack and build number.
    if (strlen(osvi.szCSDVersion) > 0)
    {
	 strcat(pszVerSionString, " ");
	 strcat(pszVerSionString, osvi.szCSDVersion);
    }

    // Include build number.
    char buf[80];
    sprintf(buf, " (build %d)", osvi.dwBuildNumber);
    strcat(pszVerSionString, buf);

    if (osvi.dwMajorVersion >= 6)
    {
	if (si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_AMD64)
	{
	    strcat(pszVerSionString, ", 64-bit");
	    *pnBits = 64;
	}
	else if (si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_INTEL)
	{
	  strcat(pszVerSionString, ", 32-bit");
	}
    }

    GfOut("Detected %s (%d.%d)\n", pszVerSionString, *pnMajor, *pnMinor);

    return true;
}

/*
* Function
*	windowsGetNumberOfCPUs
*
* Description
*	Retrieve the actual number of CPUs in the system
*       Note that a core is considered here as a "CPU", and an Intel hyper-threaded processor
*       will report twice as many "CPUs" as actual cores ...
*
* Parameters
*	None
*
* Return
*	The number of CPUs in the system
*
* Remarks
*	
*/
static unsigned 
windowsGetNumberOfCPUs()
{
    static unsigned nCPUs = 0;

	if (nCPUs == 0)
	{
		SYSTEM_INFO sysinfo;
		GetSystemInfo(&sysinfo);

		nCPUs = sysinfo.dwNumberOfProcessors;
		
		if (nCPUs < 1)
		{
			GfOut("Could not get the number of CPUs here ; assuming only 1\n");
			nCPUs = 1;
		}
		else
			GfOut("Detected %u CPUs\n", nCPUs);
	}

    return nCPUs;
}

/*
* Function
*    windowsSetThreadAffinity
*
* Description
*    Force the current thread to run on the specified CPU.
*
* Parameters
*    nCPUId : the index in [0, # of actual CPUs on the system [ (any other value will actually reset the thread affinity to the "system" affinity mask, meaning no special CPU assignement)
*
* Return
*    true if any error occured, false otherwise
*
* Remarks
*    
*/
static bool
windowsSetThreadAffinity(int nCPUId)
{
	// Get the system affinity mask : it is what we want for the thread
	// (1 bit for all the cores available in the system)
	// Note: We don't care about the process affinity mask here.
	DWORD_PTR nProcessMask, nSystemMask;
	GetProcessAffinityMask(GetCurrentProcess(), &nProcessMask, &nSystemMask);

	// Determine the affinity mask to set
	ULONGLONG nThreadAffinityMask;
	if (nCPUId == GfAffinityAnyCPU)
	{
		// No special affinity on any processor => set "system" affinity mask.
		nThreadAffinityMask = nSystemMask;
	}
	else
	{
		// Affinity on a specified CPU => compute its mask (1 bit in the "system" mask).
		int nCPUIndex = -1;
		int nBitIndex = 0;
		while (nBitIndex < sizeof(nSystemMask)*8 && nCPUIndex < nCPUId)
		{
			if (nSystemMask & 1)
				nCPUIndex++;
			nSystemMask >>= 1;
			nBitIndex++;
		}
		nBitIndex--;
		if (nCPUIndex != nCPUId)
		{
			GfError("Target CPU %d not found (erroneous id specified ?)\n", nCPUId);
			return false;
		}

		// We've got it.
		nThreadAffinityMask = (1 << nBitIndex);
	}
	
	// Get the handle for the current thread.
    HANDLE hCurrThread = GetCurrentThread();
    GfOut("Current thread handle is 0x%X\n", hCurrThread);

    // Set the affinity mask for the current thread ("stick" it to the target core).
    if (SetThreadAffinityMask(hCurrThread, (DWORD_PTR)nThreadAffinityMask) == 0)
    {
        GfError("Failed to set current thread (handle=0x%X) affinity mask to 0x%X)\n",
                hCurrThread, nThreadAffinityMask);
        return false;
    }
    else
        GfOut("Affinity mask set to 0x%X for current thread (handle=0x%X)\n",
              nThreadAffinityMask, hCurrThread);

    return true;
}

/*
* Function
*    WindowsSpecInit
*
* Description
*    Init the specific windows functions
*
* Parameters
*    none
*
* Return
*    none
*
* Remarks
*    
*/
void
WindowsSpecInit(void)
{
    memset(&GfOs, 0, sizeof(GfOs));
    
    GfOs.modLoad = windowsModLoad;
    GfOs.modLoadDir = windowsModLoadDir;
    GfOs.modUnloadList = windowsModUnloadList;
    GfOs.modInfo = windowsModInfo;
    GfOs.modInfoDir = windowsModInfoDir;
    GfOs.dirGetList = windowsDirGetList;
    GfOs.dirGetListFiltered = windowsDirGetListFiltered;
    GfOs.timeClock = windowsTimeClock;
    GfOs.sysGetNumberOfCPUs = windowsGetNumberOfCPUs;
    GfOs.sysSetThreadAffinity = windowsSetThreadAffinity;
}
