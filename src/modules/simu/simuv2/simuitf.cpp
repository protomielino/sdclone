/***************************************************************************

    file                 : simuitf.cpp
    created              : Sun Mar 19 00:08:04 CET 2000
    copyright            : (C) 2000 by Eric Espie
    email                : torcs@free.fr
    version              : $Id: simuitf.cpp,v 1.7 2005/03/31 16:01:01 olethros Exp $

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <stdlib.h>

#include <tgf.h>

#include "sim.h"

#ifdef _WIN32
BOOL WINAPI DllEntryPoint (HINSTANCE hDLL, DWORD dwReason, LPVOID Reserved)
{
    return TRUE;
}
#endif

/*
 * Function
 *	simuInit
 *
 * Description
 *	init the simu functions
 *
 * Parameters
 *	
 *
 * Return
 *	
 *
 * Remarks
 *	
 */
static int
simuInit(int /* index */, void *pt)
{
    tSimItf	*sim = (tSimItf*)pt;
    
    sim->init     = SimInit;
    sim->config   = SimConfig;
    sim->reconfig = SimReConfig;
    sim->update   = SimUpdate;
    sim->shutdown = SimShutdown;
    
    return 0;
}


/*
 * Function
 *	moduleMaxInterfaces
 *
 * Description
 *	Return the max number of interfaces of the module
 *
 * Parameters
 *	None
 *
 * Return
 *	A positive or null integer, if no error occured 
 *	-1, if any error occured 
 *
 * Remarks
 *	MUST be called before moduleInitialize()
 */
extern "C" int moduleMaxInterfaces()
{
  return 1;
}

/*
 * Function
 *	moduleInitialize
 *
 * Description
 *	Module entry point
 *
 * Parameters
 *	modInfo : Module interfaces info array to fill-in
 *
 * Return
 *	0, if no error occured 
 *	non 0, otherwise
 *
 * Remarks
 *	
 */
extern "C" int
moduleInitialize(tModInfo *modInfo)
{
    modInfo->name = "simu";		/* name of the module (short) */
    modInfo->desc = "Simulation Engine V2.0";	/* description of the module (can be long) */
    modInfo->fctInit = simuInit;	/* init function */
    modInfo->gfId = SIM_IDENT;		/* ident */
    modInfo->index = 0;
    return 0;
}

/*
 * Function
 *	moduleTerminate
 *
 * Description
 *	Module exit point
 *
 * Parameters
 *	None
 *
 * Return
 *	0, if no error occured 
 *	non 0, otherwise
 *
 * Remarks
 *	
 */
extern "C" int
moduleTerminate()
{
    return 0;
}



