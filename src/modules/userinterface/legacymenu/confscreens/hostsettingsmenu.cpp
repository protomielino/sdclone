/***************************************************************************

    file                 : hostsettingsmenu.cpp
    created              : December 2009
    copyright            : (C) 2009 Brian Gavin
    web                  : speed-dreams.sourceforge.net

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


/*
This file deals with network host settings
*/

#include <cars.h>
#include <network.h>

#include "hostsettingsmenu.h"


static void *pPrevMenu = NULL;

std::string HostSettingsMenu::m_strCarCat;
bool HostSettingsMenu::m_bCollisions = true;
bool HostSettingsMenu::m_bHumanHost = true;


void HostSettingsMenu::onActCB(void *p)
{
}

void HostSettingsMenu::CarControlCB(tComboBoxInfo *pInfo)
{
	m_strCarCat = pInfo->vecChoices[pInfo->nPos];
}

void HostSettingsMenu::CarCollideCB(tComboBoxInfo *pInfo)
{
	if (pInfo->vecChoices[pInfo->nPos] == "Off")
		m_bCollisions = false;
	else
		m_bCollisions = true;

}
void HostSettingsMenu::humanHostCB(tComboBoxInfo *pInfo)
{
	if (pInfo->vecChoices[pInfo->nPos] == "Yes")
		m_bHumanHost = true;
	else
		m_bHumanHost = false;

}

void HostSettingsMenu::onPlayerReady(void *p)
{
	//TODO
}

void HostSettingsMenu::onAcceptCB(void *p)
{
	GetServer()->SetHostSettings(m_strCarCat.c_str(),m_bCollisions);
	GfuiScreenActivate(pPrevMenu);
}

void HostSettingsMenu::onCancelCB(void *p)
{
	GfuiScreenActivate(pPrevMenu);
}

HostSettingsMenu::HostSettingsMenu()
: GfuiMenuScreen("hostsettingsmenu.xml")
{
}

bool HostSettingsMenu::Init(void* pMenu)
{
	GetNetwork()->GetHostSettings(m_strCarCat,m_bCollisions);

	pPrevMenu = pMenu;

	void* pMenuHandle = GfuiScreenCreateEx(NULL,NULL,onActCB, 
										   NULL, (tfuiCallback)NULL, 
										   1);
	SetMenuHandle(pMenuHandle);

    OpenXMLDescriptor();
    
    CreateStaticControls();

	int carCatId = CreateComboboxControl("carcatcombobox",NULL,CarControlCB);
	const std::vector<std::string>& vecCategories = GfCars::self()->getCategoryIds();
	
	int CatIndex = 0;
	for (unsigned int i=0;i<vecCategories.size();i++)
	{
		GfuiComboboxAddText(pMenuHandle,carCatId,vecCategories[i].c_str());
		if (m_strCarCat == vecCategories[i])
			CatIndex = i;
	}

	GfuiComboboxSetSelectedIndex(pMenuHandle,carCatId,CatIndex);

	int collId = CreateComboboxControl("carcollidecombobox",NULL,CarCollideCB);
	GfuiComboboxAddText(pMenuHandle,collId,"On");
	GfuiComboboxAddText(pMenuHandle,collId,"Off");

	int humanHostId = CreateComboboxControl("hosthumanplayercombobox",NULL,humanHostCB);
	GfuiComboboxAddText(pMenuHandle,humanHostId,"Yes");
	GfuiComboboxAddText(pMenuHandle,humanHostId,"No");

	GfuiComboboxSetSelectedIndex(pMenuHandle,humanHostId,0);

    CreateButtonControl("accept",NULL,onAcceptCB);
    CreateButtonControl("cancel",NULL,onCancelCB);

    CloseXMLDescriptor();

	return true;
}
