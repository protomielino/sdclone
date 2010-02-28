/***************************************************************************

    file                 : pitmenu.cpp
    created              : Mon Apr 24 18:16:37 CEST 2000
    copyright            : (C) 2000 by Eric Espie
    email                : torcs@free.fr
    version              : $Id: pitmenu.cpp,v 1.4 2005/08/11 19:43:35 berniw Exp $

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
    @version	$Id: pitmenu.cpp,v 1.4 2005/08/11 19:43:35 berniw Exp $
*/
#include <stdlib.h>
#ifdef WIN32
#include <windows.h>
#endif

#include <tgfclient.h>
#include <car.h>
#include <raceman.h>



static void		*menuHandle = NULL;
static int		fuelId;
static int		repairId;
static tCarElt		*rmCar;

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

static tfuiCallback rmCallback;
static void *rmUserData;

static void
rmStopAndGo(void * /* dummy */)
{
    rmCar->_pitStopType = RM_PIT_STOPANDGO;
    rmCallback(rmUserData);
}

static void
rmRepair(void* /* dummy */)
{
   rmCar->_pitStopType = RM_PIT_REPAIR;
   rmCallback(rmUserData);
}


/**
 * This function shows the pit menu and let the user fill in the amount
 * of fuel he wants and the number of damage he want to repair
 *
 * @param s The current situation
 * @param car The current car
 * @param userdata The parameter for the @p callback callback function
 * @param callback The function which is called after the user made a decision
 */
void
RmPitMenuStart(tSituation *s, tCarElt *car, void *userdata, tfuiCallback callback)
{
    char	buf[256];

    rmCar = car;
    rmCallback = callback;
    rmUserData = userdata;

    if (menuHandle) {
	GfuiScreenRelease(menuHandle);
    }

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
    GfuiAddSKey(menuHandle, GFUIK_F1, "Help", menuHandle, GfuiHelpScreen, NULL);
    GfuiAddSKey(menuHandle, GFUIK_F12, "Screen-Shot", NULL, GfuiScreenShot, NULL);

    GfuiScreenActivate(menuHandle);
}
