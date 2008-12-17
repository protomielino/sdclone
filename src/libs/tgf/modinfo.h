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


#ifndef __MODINFO__H__
#define __MODINFO__H__

#ifdef WIN32
#include <windows.h>
#endif

/** Maximum number of interfaces in one "legacy" module 
    (no limit for "Unlimited Number of Interfaces" modules)
    @see	ModList
 */
#define MAX_MOD_ITF 10
#define GfModInfoDefaultMaxItf MAX_MOD_ITF

/* Name of the module function that gives its max number of interfaces (new scheme) */
#define GfModInfoMaxItfFuncName "moduleMaxInterfaces"

/* Name of the module function entry point (new scheme) */
#define GfModInfoInitFuncName "moduleInitialize"

/* Name of the module function exit point (new scheme) */
#define GfModInfoTermFuncName "moduleTerminate"

/** initialisation of the function table 
    @see	ModInfo
*/
typedef int (*tfModPrivInit)(int index, void *);

/** Module interface information structure */
typedef struct ModInfo {
    const char		*name;		/**< name of the module (short) (NULL if no module) */
    const char		*desc;		/**< description of the module (can be long) */
    tfModPrivInit	fctInit;	/**< init function */
    unsigned int	gfId;		/**< supported framework version */
    int			index;		/**< index if multiple interface in one dll */
    int			prio;		/**< priority if needed */
    int			magic;		/**< magic number for integrity check */
} tModInfo;

/** Internal module interface information structure (see GfModInfoDuplicate) 
    WARNING: Must have the same fields ; only const-ness may differ */
typedef struct ModInfoNC {
    char		*name;		/**< name of the module (short) (NULL if no module) */
    char		*desc;		/**< description of the module (can be long) */
    tfModPrivInit	fctInit;	/**< init function */
    unsigned int	gfId;		/**< supported framework version */
    int			index;		/**< index if multiple interface in one dll */
    int			prio;		/**< priority if needed */
    int			magic;		/**< magic number for integrity check */
} tModInfoNC;

/** Shared library handle type */
#ifndef WIN32
typedef void* tSOHandle;
#else
typedef HMODULE tSOHandle;
#endif

/** list of module interfaces */
typedef struct ModList {
    int			modInfoSize;	/**< module max nb of interfaces */
    tModInfoNC		*modInfo;	/**< module interfaces info array, full or 0 terminated */
    tSOHandle		handle;		/**< handle of loaded shared lib */
    char		*sopath;	/**< path name of shared lib file */
    struct ModList	*next;		/**< next module in list */
} tModList;


/* Interface of module function to determine the max number of interfaces */
typedef int (*tfModInfoMaxItf)(); /* first function called in the module, if present */

/* Interface of module initialization function */
typedef int (*tfModInfoInit)(tModInfo *);  /* second/first function called in the module */

/* Interface of module termination function */
typedef int (*tfModInfoTerm)(void);	/* last function called in the module */


/********************************************
 * Tools for Dynamic Modules initialization *
 ********************************************/

/* Allocate the module interfaces info array */
extern tModInfo *GfModInfoAllocate(int maxItf);

/* Free the module interfaces info array */
extern void GfModInfoFree(tModInfo *array, int maxItf);

/* Free the module interfaces info array */
extern void GfModInfoFreeNC(tModInfoNC *array, int maxItf);

/* Duplicate a module interfaces info array from a const one to a non-const one */
extern tModInfoNC* GfModInfoDuplicate(const tModInfo *source, int maxItf);

/* Initialize the module with given handle and library file path */
extern int GfModInitialize(tSOHandle soHandle, const char *soPath, 
			   unsigned int gfid, tModList **mod);

/* Terminate the module with given handle and library file path */
extern int GfModTerminate(tSOHandle soHandle, const char *soPath);

#endif /* __MODINFO__H__ */
