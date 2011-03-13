/***************************************************************************

    file                 : pitmenu.cpp
    created              : Mon Apr 24 18:16:37 CEST 2000
    copyright            : (C) 2000 by Eric Espie
    email                : torcs@free.fr
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
    		Pit menu command
    @ingroup	racemantools
    @author	<a href=mailto:torcs@free.fr>Eric Espie</a>
    @version	$Id$
*/
#include <cstdlib>

#include <tgfclient.h>
#include <car.h>
#include <raceman.h>

#include "racescreens.h"


static void		*menuHandle = NULL;
static int		fuelId;
static int		repairId;
static tCarElt		*rmCar;
static tfuiCallback rmCallback;


static void
rmUpdtFuel(void * /* dummy */)
{
    char	*val;
    char	buf[32];
    
    val = GfuiEditboxGetString(menuHandle, fuelId);
    rmCar->pitcmd.fuel = (tdble)strtod(val, (char **)NULL);
    sprintf(buf, "%.1f", rmCar->pitcmd.fuel);
    GfuiEditboxSetString(menuHandle, fuelId, buf);
}

static void
rmUpdtRepair(void * /* dummy */)
{
    char	*val;
    char	buf[32];
    
    val = GfuiEditboxGetString(menuHandle, repairId);
    rmCar->pitcmd.repair = strtol(val, (char **)NULL, 0);
    sprintf(buf, "%d", rmCar->pitcmd.repair);
    GfuiEditboxSetString(menuHandle, repairId, buf);
}

static void
rmStopAndGo(void * /* dummy */)
{
    rmCar->_pitStopType = RM_PIT_STOPANDGO;
    rmCallback(rmCar);
}

static void
rmRepair(void* /* dummy */)
{
	rmCar->_pitStopType = RM_PIT_REPAIR;
	rmCallback(rmCar);
}


/**
 * This function shows the pit menu and let the user fill in the amount
 * of fuel he wants and the number of damage he want to repair
 *
 * @param car The current car (pitcmd is modified on user decisions)
 * @param callback The function which is called after the user made a decision
 */
void
RmPitMenuStart(tCarElt *car, tfuiCallback callback)
{
    char buf[32];

    rmCar = car;
    rmCallback = callback;

    if (menuHandle)
        GfuiScreenRelease(menuHandle);

	GfLogInfo("Entering Pit menu\n");

    // Create screen, load menu XML descriptor and create static controls.
    menuHandle = GfuiScreenCreateEx(NULL, NULL, NULL, NULL, NULL, 1);

    void *menuXMLDescHdle = LoadMenuXML("pitmenu.xml");

    CreateStaticControls(menuXMLDescHdle, menuHandle);

    // Create labels for driver name, remaining laps and remaining fuel.
    int driverNameId = CreateLabelControl(menuHandle, menuXMLDescHdle, "drivernamelabel");
    GfuiLabelSetText(menuHandle, driverNameId, car->_name);

    int remainLapsId = CreateLabelControl(menuHandle, menuXMLDescHdle, "remaininglapslabel");
    sprintf(buf, "%d", car->_remainingLaps);
    GfuiLabelSetText(menuHandle, remainLapsId, buf);

    int remainFuelId = CreateLabelControl(menuHandle, menuXMLDescHdle, "remainingfuellabel");
    sprintf(buf, "%.1f l", car->_fuel);
    GfuiLabelSetText(menuHandle, remainFuelId, buf);

    // Create edit boxes for fuel and repair amounts.
    fuelId = CreateEditControl(menuHandle, menuXMLDescHdle, "fuelamountedit", NULL, NULL, rmUpdtFuel);
    sprintf(buf, "%.1f", car->pitcmd.fuel);
    GfuiEditboxSetString(menuHandle, fuelId, buf);

    repairId = CreateEditControl(menuHandle, menuXMLDescHdle, "repairamountedit", NULL, NULL, rmUpdtRepair);
    sprintf(buf, "%d", (int)car->pitcmd.repair);
    GfuiEditboxSetString(menuHandle, repairId, buf);

    // Create Back and Reset buttons.
    CreateButtonControl(menuHandle, menuXMLDescHdle, "repairbutton", NULL, rmRepair);
    CreateButtonControl(menuHandle, menuXMLDescHdle, "stopgobutton", NULL, rmStopAndGo);

    // Close menu XML descriptor.
    GfParmReleaseHandle(menuXMLDescHdle);
    
    // Register keyboard shortcuts.
    GfuiMenuDefaultKeysAdd(menuHandle);

    // Activate the created screen.
    GfuiScreenActivate(menuHandle);
}
