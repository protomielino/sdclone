/***************************************************************************

    file                 : genetic.h
    created              : Tue Nov 04 17:45:00 CET 2010
    copyright            : (C) 2010 by Wolf-Dieter Beelitz
    email                : wdb@wdbee.de

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
    @defgroup	genetictools	Tools for genetic parameter optimization.
    Collection of functions for genetic parameter optimization.
*/
#include "genetic.h"

#ifndef __GENETICGLOBAL_H__
#define __GENETICGLOBAL_H__

tgenResult MyResults;
bool genOptimization = false;
bool genOptInit = true;
int genLoops = 3;

#endif /* __GENETICGLOBAL_H__ */

