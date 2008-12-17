/***************************************************************************

    file                 : ssggraph.cpp
    created              : Thu Aug 17 23:19:19 CEST 2000
    copyright            : (C) 2000 by Eric Espie
    email                : torcs@free.fr
    version              : $Id: ssggraph.cpp,v 1.12 2005/02/01 19:08:19 berniw Exp $

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
#endif
#include <GL/gl.h>

#include <tgfclient.h>

#include "grmain.h"

#ifdef DMALLOC
#include "dmalloc.h"
#endif

static int
graphInit(int /* idx */, void *pt)
{
    tGraphicItf *itf = (tGraphicItf*)pt;
    
    itf->inittrack     = initTrack;
    itf->initcars      = initCars;
    itf->initview      = initView;
    itf->refresh       = refresh;
    itf->shutdowncars  = shutdownCars;
    itf->shutdowntrack = shutdownTrack;
    //itf->bendcar       = bendCar;
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
// We need a unique global name for static link under Windows.
#ifdef WIN32
extern "C" int ssggraph_moduleMaxInterfaces()
#else
extern "C" int moduleMaxInterfaces()
#endif
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
// We need a unique global name for static link under Windows.
#ifdef WIN32
extern "C" int ssggraph_moduleInitialize(tModInfo *modInfo)
#else
extern "C" int moduleInitialize(tModInfo *modInfo)
#endif
{
    modInfo->name = "ssggraph";		        		/* name of the module (short) */
    modInfo->desc = "The Graphic Library using PLIB ssg";	/* description of the module (can be long) */
    modInfo->fctInit = graphInit;				/* init function */
    modInfo->gfId = 1;						/* v 1  */
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
// We need a unique global name for static link under Windows.
#ifdef WIN32
extern "C" int ssggraph_moduleTerminate()
#else
extern "C" int moduleTerminate()
#endif
{
    return 0;
}

