/***************************************************************************

    file        : raceselectmenu.cpp
    created     : Sat Nov 16 09:36:29 CET 2002
    copyright   : (C) 2002 by Eric Espie
    email       : eric.espie@torcs.org   
    version     : $Id$                                  

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
    		Race selection menu
    @author	<a href=mailto:eric.espie@torcs.org>Eric Espie</a>
    @version	$Id$
*/

#include <map>
#include <algorithm>

#include <tgfclient.h>

#include <raceman.h>

#include <racemanagers.h>
#include <race.h>

#include "legacymenu.h"
#include "racescreens.h"


static void *reRaceSelectHandle = NULL;

static std::map<std::string, int> reMapSubTypeComboIds;


/* Called when the menu is activated */
static void
reOnActivate(void * /* dummy */)
{
	GfLogTrace("Entering Race Mode Select menu\n");

    /* Race engine init */
    LegacyMenu::self().raceEngine().initialize();
    LegacyMenu::self().raceEngine().data()->_reMenuScreen = reRaceSelectHandle;
}

/* Exit from Race engine */
static void
reOnRaceSelectShutdown(void *prevMenu)
{
    GfuiScreenActivate(prevMenu);
    LegacyMenu::self().raceEngine().shutdown();
}


static void
reOnSelectRaceMan(void *pvRaceManTypeIndex)
{
	// Get the race managers with the given type
	const std::vector<std::string>& vecRaceManTypes = GfRaceManagers::self()->getTypes();
	const std::string strRaceManType = vecRaceManTypes[(long)pvRaceManTypeIndex];
	const std::vector<GfRaceManager*> vecRaceMans =
		GfRaceManagers::self()->getRaceManagersWithType(strRaceManType);

	// If more than 1, get the one with the currently selected sub-type.
	GfRaceManager* pSelRaceMan = 0;
	if (vecRaceMans.size() > 1)
	{
		const int nSubTypeComboId = reMapSubTypeComboIds[strRaceManType];
		const char* pszSelSubType = GfuiComboboxGetText(reRaceSelectHandle, nSubTypeComboId);
		std::vector<GfRaceManager*>::const_iterator itRaceMan;
		for (itRaceMan = vecRaceMans.begin(); itRaceMan != vecRaceMans.end(); itRaceMan++)
		{
			if ((*itRaceMan)->getSubType() == pszSelSubType)
			{
				// Start configuring it.
				pSelRaceMan = *itRaceMan;
				break;
			}
		}
	}

	// If only 1, no choice.
	else if (vecRaceMans.size() == 1)
	{
		pSelRaceMan = vecRaceMans.back();
	}

	if (pSelRaceMan)
	{
		LegacyMenu::self().raceEngine().selectRaceman(pSelRaceMan);
		
		// (Re-)initialize the currrent race configuration from the selected race manager.
		LegacyMenu::self().raceEngine().race()->load(pSelRaceMan);

		// Start the race configuration menus sequence.
		LegacyMenu::self().raceEngine().configureRace(/* bInteractive */ true);
	}
	else
	{
		GfLogDebug("No such race manager (type '%s')\n", strRaceManType.c_str());
	}
}

static void
reOnChangeRaceMan(tComboBoxInfo *)
{
}

/* Initialize the single player menu */
void *
ReRaceSelectInit(void *prevMenu)
{
    if (reRaceSelectHandle) 
		return reRaceSelectHandle;
    
    // Create screen, load menu XML descriptor and create static controls.
    reRaceSelectHandle = GfuiScreenCreateEx((float*)NULL, 
											NULL, reOnActivate, 
											NULL, (tfuiCallback)NULL, 
											1);
    void *hMenuXMLDesc = LoadMenuXML("raceselectmenu.xml");
    CreateStaticControls(hMenuXMLDesc, reRaceSelectHandle);

    // Create the raceman type buttons and sub-type combo-boxes (if any).
	const std::vector<std::string>& vecRaceManTypes = GfRaceManagers::self()->getTypes();
	std::vector<std::string>::const_iterator itRaceManType;
	// For each race manager type :
	for (itRaceManType = vecRaceManTypes.begin();
		 itRaceManType != vecRaceManTypes.end(); itRaceManType++)
	{
		// Get the racemanagers with this type
		const std::vector<GfRaceManager*> vecRaceMans =
			GfRaceManagers::self()->getRaceManagersWithType(itRaceManType->c_str());
		GfLogDebug("ReRaceSelectInit : Type %s (%u)\n", itRaceManType->c_str(), vecRaceMans.size());

		// Create the race manager type button.
		std::string strButtonCtrlName(*itRaceManType);
		strButtonCtrlName.erase(std::remove(strButtonCtrlName.begin(), strButtonCtrlName.end(), ' '), strButtonCtrlName.end()); // Such a pain to remove spaces !
		strButtonCtrlName += "Button";
		int n = CreateButtonControl(reRaceSelectHandle, hMenuXMLDesc, strButtonCtrlName.c_str(),
									(void*)(itRaceManType - vecRaceManTypes.begin()),
									reOnSelectRaceMan);
		GfLogDebug("CreateButtonControl(%d, '%s')\n", n, strButtonCtrlName.c_str());

		// Look for sub-types : if any, we have a sub-type combo box for this type.
		bool bCreateCombo = false;
		std::vector<GfRaceManager*>::const_iterator itRaceMan;
		for (itRaceMan = vecRaceMans.begin(); itRaceMan != vecRaceMans.end(); itRaceMan++)
		{
			if (!(*itRaceMan)->getSubType().empty())
			{
				bCreateCombo = true;
				break;
			}
		}

		if (!bCreateCombo)
			continue;
		
		// Create the race manager sub-type combo-box.
		std::string strComboCtrlName(*itRaceManType);
		strComboCtrlName.erase(std::remove(strComboCtrlName.begin(), strComboCtrlName.end(), ' '), strComboCtrlName.end()); // Such a pain to remove spaces !
		strComboCtrlName += "Combo";
		reMapSubTypeComboIds[*itRaceManType] =
			CreateComboboxControl(reRaceSelectHandle, hMenuXMLDesc,
								  strComboCtrlName.c_str(), 0, reOnChangeRaceMan);
		GfLogDebug("CreateComboboxControl(%d, '%s')\n", reMapSubTypeComboIds[*itRaceManType], strComboCtrlName.c_str());

		// Add one item in the combo for each race manager of this type.
		for (itRaceMan = vecRaceMans.begin(); itRaceMan != vecRaceMans.end(); itRaceMan++)
		{
			GfuiComboboxAddText(reRaceSelectHandle, reMapSubTypeComboIds[*itRaceManType],
								(*itRaceMan)->getSubType().c_str());
			GfLogDebug("ReRaceSelectInit : SubType %s : \n", (*itRaceMan)->getSubType().c_str());
		}

		// Select the first one by default.
		GfuiComboboxSetPosition(reRaceSelectHandle, reMapSubTypeComboIds[*itRaceManType], 0);

		// Disable combo if only one race manager.
		if (vecRaceMans.size() == 1)
			GfuiEnable(reRaceSelectHandle, reMapSubTypeComboIds[*itRaceManType], GFUI_DISABLE);
	}
	
    // Create Back button
    CreateButtonControl(reRaceSelectHandle, hMenuXMLDesc, "BackButton",
						prevMenu, reOnRaceSelectShutdown);

    // Close menu XML descriptor.
    GfParmReleaseHandle(hMenuXMLDesc);
    
    // Register keyboard shortcuts.
    GfuiMenuDefaultKeysAdd(reRaceSelectHandle);
    GfuiAddKey(reRaceSelectHandle, GFUIK_ESCAPE, "Back To Main Menu",
			   prevMenu, reOnRaceSelectShutdown, NULL);

    // Give the race engine the menu to come back to.
    LegacyMenu::self().raceEngine().initializeState(reRaceSelectHandle);

    return reRaceSelectHandle;
}
