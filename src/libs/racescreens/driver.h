/***************************************************************************

    file                 : driver.h
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
 
/**
    @defgroup	racemantools	Tools for race managers.
    This is a collection of useful functions for managing drivers.
*/

#ifndef __DRIVER_H__
#define __DRIVER_H__

#include <tgf.h>

// Driver description
typedef struct rmdDrvElt
{
    char	*moduleName;	// Module name
    int		interfaceIndex;	// Index of associated interface in the module
    char	*name;		// Driver name
    int		isSelected;	// Selected for race ?
    int		isHuman;	// Human driver ?
    void	*carParmHdle;	// Handle to the car XML params file
    GF_TAILQ_ENTRY(struct rmdDrvElt)	link;
} trmdDrvElt;

extern void rmdGetDriverType(const char* moduleName, char* driverType, size_t maxSize);
extern int rmdDriverMatchesFilters(const trmdDrvElt *drv, const char* carCat, const char* drvTyp,
				   const char* anyCarCat, const char* anyDrvTyp);


#endif /* __DRIVER_H__ */

