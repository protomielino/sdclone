/***************************************************************************
                   modinfo.h -- Tools for module interface management

    created              : Fri Aug 13 22:32:14 CEST 1999
    copyright            : (C) 2008 by Jean-Philippe Meuret                         
    email                : jpmeuret@free.fr
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
    	Tools for module interface management.
    @author	<a href=mailto:jpmeuret@free.fr>Jean-Philippe Meuret</a>
    @version	$Id$
*/

#include <cstring>

#include <modinfo.h>
#include <tgf.h> // To get malloc/calloc/free replacements under Windows


#ifdef WIN32
#include <direct.h>
#define dlsym   GetProcAddress
#define dlerror GetLastError
static const size_t SOFileExtLen = strlen(".dll");
#else
#include <dlfcn.h>
static const size_t SOFileExtLen = strlen(".so");
#endif


/* Allocate the module interfaces info array */
tModInfo *GfModInfoAllocate(int maxItf)
{
    tModInfo *array = (tModInfo*)calloc(maxItf, sizeof(tModInfo));
    if (!array)
    {
        GfError("GfModInfoAllocate: Failed to allocate tModInfo array (maxItf=%d)\n", maxItf);
    }

    return array;
}

/* Free the module interfaces info array */
void GfModInfoFree(tModInfo *array, int maxItf)
{
    if (array)
    {
	free(array);
    }
    else
    {
	GfError("GfModInfoFree: Null pointer\n");
    }
  
}

/**
 * Copies something of type tModList into something of type tModListNC
 * This function allocates new space for parts of tModList that are const.
 * @param constArray The source tModList
 * @param maxItf     The max number of interfaces of the module
 */
tModInfoNC *GfModInfoDuplicate(const tModInfo *constArray, int maxItf)
{
    int itfInd;

    // Allocate target array.
    tModInfoNC *array = (tModInfoNC*)calloc(maxItf, sizeof(tModInfoNC));
    if (!constArray)
    {
        GfError("GfModInfoAllocate: Failed to allocate tModInfoNC array (maxItf=%d)\n", maxItf);
    }

    // Copy constArray to array (null name indicates non used interface = end of usefull array).
    memset(array, 0, maxItf*sizeof(tModInfo));
    for( itfInd = 0; itfInd < maxItf && constArray[itfInd].name; ++itfInd )
    {
        array[itfInd].name    = constArray[itfInd].name ? strdup(constArray[itfInd].name) : 0;
        array[itfInd].desc    = constArray[itfInd].desc ? strdup(constArray[itfInd].desc) : 0;
        array[itfInd].fctInit = constArray[itfInd].fctInit;
        array[itfInd].gfId    = constArray[itfInd].gfId;
        array[itfInd].index   = constArray[itfInd].index;
        array[itfInd].prio    = constArray[itfInd].prio;
        array[itfInd].magic   = constArray[itfInd].magic;
    }

    return array;
}

/* Free the module interfaces info array */
void GfModInfoFreeNC(tModInfoNC *array, int maxItf)
{
    int itfInd;

    if (array)
    {
	for( itfInd = 0; itfInd < maxItf && array[itfInd].name; ++itfInd )
	{
	    if (array[itfInd].name)
		free(array[itfInd].name);
	    if (array[itfInd].desc)
		free(array[itfInd].desc);
	}
	free(array);
    }
    else
    {
	GfError("GfModInfoFreeNC: Null pointer\n");
    }
  
}

/*
 * Function
 *	GfModInitialize
 *
 * Description
 *	Initialize the module with given handle and library file path
 *
 * Parameters
 *	soHandle (in)	handle of the loaded shared library
 *	soPath   (in)	path of the loaded shared library
 *	gfid     (in)	id of the gaming framework that the module MUST implement to be initialized
 *	                (Not taken into account if == GfIdAny)
 *	mod      (in)	address of module entry to allocate and initialize  
 *	                (0 if any error occured or modules not retained for bad GFId)
 *
 * Return
 *	0	if initialization succeeded (even if the module was rejected for bad GFId)
 *	-1	if any error occured
 *
 * Remarks
 *	
 */
int GfModInitialize(tSOHandle soHandle, const char *soPath, unsigned int gfid, tModList **mod)
{
    tfModInfoMaxItf	fModInfoMaxItf;	/* function that gives the max nb of itf of the module */
    tfModInfoInit	fModInfoInit;	/* init function of the module */
    int			initSts = 0;	/* returned status */
    int			retained = 1;
    
    /* Allocate module entry in list */
    if (!(*mod = (tModList*)calloc(1, sizeof(tModList))))
    {
	GfError("GfModInitialize: Failed to allocate tModList for module %s\n", soPath);
	return -1;
    }
    
    /* Determine the number of interfaces of the module :
       1) Call the dedicated module function if present */
    if ((fModInfoMaxItf = (tfModInfoMaxItf)dlsym(soHandle, GfModInfoMaxItfFuncName)) != 0) 
    {
        /* DLL loaded, nbItf function exists, call it... */
        (*mod)->modInfoSize = fModInfoMaxItf();
    } 

    /* 2) If not present, default number of interfaces (backward compatibility) */
    else
    {
        (*mod)->modInfoSize = GfModInfoDefaultMaxItf;
    }

    /* Get module initialization function :
       1) Try the new sheme (fixed name) */
    if ((fModInfoInit = (tfModInfoInit)dlsym(soHandle, GfModInfoInitFuncName)) == 0)
    {
	/* 2) Backward compatibility (dll name) */
	char fname[256];
	const char* lastSlash = strrchr(soPath, '/');
	if (lastSlash) 
	    strcpy(fname, lastSlash+1);
	else
	    strcpy(fname, soPath);
	fname[strlen(fname) - SOFileExtLen] = 0; /* cut so file extension */
	fModInfoInit = (tfModInfoInit)dlsym(soHandle, fname);
    }


    /* Call module initialization function if found */
    if (fModInfoInit) 
    {
	/* Allocate module interfaces info array according to the size we got */
	tModInfo* constModInfo;
	if ((constModInfo = GfModInfoAllocate((*mod)->modInfoSize)) != 0) 
	{
	    /* Library loaded, init function exists, call it... */
	    if ((initSts = fModInfoInit(constModInfo)) == 0)
	    {
		/* Duplicate strings in each interface, in case the module gave us static data ! */
		if (((*mod)->modInfo = GfModInfoDuplicate(constModInfo, (*mod)->modInfoSize)) != 0) 
		{
		    /* Reject module if not of requested gaming framework Id */
		    if (gfid != GfIdAny && (*mod)->modInfo[0].gfId != gfid) 
		    {
			GfTrace("GfModInitialize: Module not retained %s\n", soPath);
			GfModInfoFreeNC((*mod)->modInfo, (*mod)->modInfoSize);
			retained = 0;
		    }
		    
		    /* Free the module info data returned by the module (we have a copy) */
		    GfModInfoFree(constModInfo, (*mod)->modInfoSize);
		}
		else
		{
		    initSts = -1;
		}
	    } 
	    else
	    {
	        GfError("GfModInitialize: Module init function failed %s\n", soPath);
	    }
	}
	else
	{
	    initSts = -1;
	}
    } 

    /* If init function not found, we have a problem ... */
    else
    {
	GfError("GfModInitialize: Module init function %s not found ...  %s\n", 
		soPath, dlerror());
	initSts = -1;
    }

    /* Store other module information */
    if (initSts == 0 && retained)
    {
        GfOut("Initialized module %s (maxItf=%d)\n", soPath, (*mod)->modInfoSize);
	(*mod)->handle = soHandle;
	(*mod)->sopath = strdup(soPath);
    }
    else
    {
        free(*mod);
	*mod = 0;
    }

    return initSts;
}

/*
 * Function
 *	GfModTerminate
 *
 * Description
 *	Terminate the module with given handle and library file path
 *
 * Parameters
 *	soHandle (in)	handle of the loaded shared library
 *	soPath   (in)	path of the loaded shared library
 *
 * Return
 *	0	if termination succeeded
 *	-1	if any error occured
 *
 * Remarks
 *	
 */
int GfModTerminate(tSOHandle soHandle, const char *soPath)
{
    tfModInfoTerm	fModInfoTerm;	/* Termination function of the module */
    int			termSts = 0;	/* Returned status */
    
    /* Get the module termination function if any :
       1) Try the new sheme (fixed name) */
    if ((fModInfoTerm = (tfModInfoTerm)dlsym(soHandle, GfModInfoTermFuncName)) == 0)
    {
	/* 2) Backward compatibility (dll name + "Shut") */
	char fname[256];
	const char* lastSlash = strrchr(soPath, '/');
	if (lastSlash)
	    strcpy(fname, lastSlash+1);
	else
	    strcpy(fname, soPath);
	strcpy(&fname[strlen(fname) - SOFileExtLen], "Shut"); /* cut so file ext */
	fModInfoTerm = (tfModInfoTerm)dlsym(soHandle, fname);
    }

    /* Call the module termination function if any */
    if (fModInfoTerm)
	termSts = fModInfoTerm();
    
    GfOut("Terminated module %s\n", soPath);

    return termSts;
}

