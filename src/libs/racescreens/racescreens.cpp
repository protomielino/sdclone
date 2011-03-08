/***************************************************************************

    file                 : racescreens.cpp
    copyright            : (C) 2010 by Jean-Philippe Meuret                        
    email                : pouillot@users.sourceforge.net   
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
 
#include "racescreens.h"


static IRaceEngine* rmPRaceEngine;


void RmSetRaceEngine(IRaceEngine& raceEngine)
{
	rmPRaceEngine = &raceEngine;
}

IRaceEngine& RmRaceEngine()
{
	return *rmPRaceEngine;
}


