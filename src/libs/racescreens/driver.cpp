/***************************************************************************
                  driver.cpp -- drivers manipuation functions                              
                             -------------------                                         
    created              : Sun Dec 04 19:00:00 CET 2008
    copyright            : (C) 2008 by Jean-Philippe MEURET
    email                : jpmeuret@free.fr
                      
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
    		This is a collection of useful functions for managing drivers.
    @ingroup	racemantools
    @author	<a href=mailto:jpmeuret@free.fr>Jean-Philippe MEURET</a>
*/


#include <stdlib.h>
#include <cstring>

#include <car.h>

#include "driver.h"


int rmdDriverMatchesFilters(const trmdDrvElt *drv, const char* carCat, const char* drvTyp,
			    const char* anyCarCat, const char* anyDrvTyp)
{
    return (!strcmp(carCat, anyCarCat) || !strcmp(GfParmGetStr(drv->carParmHdle, SECT_CAR, PRM_CATEGORY, ""), carCat))
	   && (!strcmp(drvTyp, anyDrvTyp) || strstr(drv->moduleName, drvTyp) == drv->moduleName);
}

void rmdGetDriverType(const char* moduleName, char* driverType, size_t maxSize)
{
    char* pos;

    strncpy(driverType, moduleName, maxSize);
    driverType[maxSize-1] = 0; // Guarantee 0 termination

    // Parse module name for last '_' char : 
    // assumed to be the separator between type and instance name for ubiquitous robots (ex: simplix)
    pos = strrchr(driverType, '_');
    if (pos)
	*pos = 0;

    // Otherwise, search for an isolated last digit in the module name :
    // old robot with hard-coded max cars of 10 may follow this pattern (ex: berniw2 and 3)
    else
    {
	pos = driverType + strlen(driverType) - 1;
	while (pos != driverType && isdigit(*pos))
	    pos--;
	if (++pos == driverType + strlen(driverType) - 1)
	    *pos = 0;
    }
}
