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


#include <stddef.h>
#include <sys/types.h>
#include <time.h>
#include <direct.h>
#include <io.h>

#include <tgf.h>
#include <os.h>

// Static link for ssggraph.
extern "C" int ssggraph_moduleWelcome(const tModWelcomeIn* welcomeIn, tModWelcomeOut*);
extern "C" int ssggraph_moduleInitialize(tModInfo *modInfo);


/*
* Function
*	modLoadSsggraph
*
* Description
*	Load statically the ssggraph module (can't be loaded dynamically)
*
* Parameters
*	sopath  path of the DLL file to load
*	modlist	list of module interfaces description structure
*
* Return
*	0	Ok
*	-1	error
*
* Remarks
*	
*/
static int
modLoadSsggraph(const char *soPath, tModList **modlist)
{
    HMODULE	 handle;
    tModList	*curMod;
    int		 initSts = 0;	/* returned status */
    char         soName[256];
    char         soDir[1024];
    char*        lastSlash;
    
    /* Try and avoid loading the same module twice (WARNING: Only checks soPath equality !) */
    if ((curMod = GfModIsInList(soPath, *modlist)) != 0)
    {
      GfOut("Module %s already loaded\n", soPath);
      GfModMoveToListHead(curMod, modlist); // Force module to be the first in the list.
      return initSts;
    }

    if ((curMod = (tModList*)calloc(1, sizeof(tModList))) != 0)
    {
	/* Determine the shared library / DLL name and load dir */
	strcpy(soDir, soPath);
	lastSlash = strrchr(soDir, '/');
	if (lastSlash)
	{
	    strcpy(soName, lastSlash+1);
	    *lastSlash = 0;
	}
	else
	{
	    strcpy(soName, soPath);
	    *soDir = 0;
	}
	soName[strlen(soName) - 4] = 0; /* cut so file extension */

	/* Welcome the module : exchange informations with it */
	/* 1) Prepare information to give to the module */
	tModWelcomeIn welcomeIn;
	welcomeIn.itfVerMajor = 1;
	welcomeIn.itfVerMinor = 0;
	welcomeIn.name = soName;
	welcomeIn.loadPath = soDir;
	
	/* 2) Prepare a place for the module-given information */
	tModWelcomeOut welcomeOut;
	
	/* 3) Call the welcome function */
	initSts = ssggraph_moduleWelcome(&welcomeIn, &welcomeOut);  
	
	/* 4) Save information given by the module */
	curMod->modInfoSize = welcomeOut.maxNbItf;

	/* Allocate module interfaces info array according to the size we got */
	tModInfo* constModInfo;
	if ((constModInfo = GfModInfoAllocate(curMod->modInfoSize)) != 0)
	{
	    /* Call the module entry point, to initialize the interfaces info array */
	    if (ssggraph_moduleInitialize(constModInfo) == 0)
	    {
		/* Duplicate strings in each interface, in case the module gave us static data ! */
		if ((curMod->modInfo = GfModInfoDuplicate(constModInfo, curMod->modInfoSize)) != 0) 
		{
		    curMod->handle = 0; // Mark this module as not unloadable.
		    curMod->sopath = strdup(soPath);
		    GfModAddInList(curMod, modlist, /* priosort */ 0); // Really don't sort / prio ?
		    /* Free the module info data returned by the module (we have a copy) */
		    GfModInfoFree(constModInfo, curMod->modInfoSize);

		    GfOut("Statically initialized windows module %s\n", soPath);
		}
	    }
	    else
	    {
	      GfError("windowsModLoad: Module init function failed %s\n", soPath);
	      initSts = -1;
	    }
	} 
	else
	{
	  initSts = -1;
	}
    } 
    else
    {
      GfError("windowsModLoad: Failed to allocate tModList for module %s\n", soPath);
      initSts = -1;
    }

    return initSts;
} 

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
    
    /* ssggraph is statically linked under Windows ... */
    char fname[256];
    const char* lastSlash = strrchr(soPath, '/');
    if (lastSlash) 
	strcpy(fname, lastSlash+1);
    else
	strcpy(fname, soPath);
    fname[strlen(fname) - 4] = 0; /* cut .dll */
    if (strcmp(fname, "ssggraph") == 0)
      return modLoadSsggraph(soPath, modlist);

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
	    FreeLibrary(handle);
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
	FreeLibrary(handle);
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
		        FreeLibrary(handle);
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
		    handle = LoadLibrary( soPath );
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
			FreeLibrary(handle);
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

	 // Don't terminate/unload ssgraph, as it was loaded statically
	if (curMod->handle)
	{
	    termSts = GfModTerminate(curMod->handle, curMod->sopath);
	    if (termSts)
		unloadSts = termSts;

	    FreeLibrary(curMod->handle);
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
*	directory name
*
* Return
*	list of directory entries
*/
static tFList *
windowsDirGetListFiltered(const char *dir, const char *suffix)
{
    tFList	*flist = NULL;
    tFList	*curf;
    int		suffixLg;
    int		fnameLg;

    if ((suffix == NULL) || (strlen(suffix) == 0))
	return windowsDirGetList(dir);

    suffixLg = strlen(suffix);
	
    _finddata_t FData;
    char Dir_name[ 1024 ];
    sprintf( Dir_name, "%s\\*.*", dir );
    GfOut("trying dir %s\n",dir);
    long Dirent = _findfirst( Dir_name, &FData );
    if ( Dirent != -1 ) {
	do {
	    fnameLg = strlen(FData.name);
	    if ((fnameLg > suffixLg) && (strcmp(FData.name + fnameLg - suffixLg, suffix) == 0)) {
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
*	WindowsSpecInit
*
* Description
*	Init the specific windows functions
*
* Parameters
*	none
*
* Return
*	none
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


}
