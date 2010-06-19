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

#include <carinfo.h>
#include <network.h>

#include "hostsettingsmenu.h"


static void *pPrevMenu = NULL;

std::string HostSettingsMenu::m_strCarCat;
bool HostSettingsMenu::m_bCollisions;
bool HostSettingsMenu::m_bHumanHost;


void HostSettingsMenu::onActCB(void *p)
{
}

void HostSettingsMenu::CarControlCB(tChoiceInfo *pChoices)
{
	m_strCarCat = pChoices->vecChoices[pChoices->nPos];
}

void HostSettingsMenu::CarCollideCB(tChoiceInfo *pChoices)
{
	if (pChoices->vecChoices[pChoices->nPos] == "Off")
		m_bCollisions = false;
	else
		m_bCollisions = true;

}
void HostSettingsMenu::humanhostCB(tChoiceInfo *pChoices)
{
	if (pChoices->vecChoices[pChoices->nPos] == "Yes")
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

bool HostSettingsMenu::Init(void* pMenu)
{
	GetNetwork()->GetHostSettings(m_strCarCat,m_bCollisions);

	pPrevMenu = pMenu;

	m_pMenuHandle = GfuiScreenCreateEx(NULL,NULL,onActCB, 
					 NULL, (tfuiCallback)NULL, 
					 1);

    void *param = LoadMenuXML("hostsettingsmenu.xml");
    CreateStaticControls(param,m_pMenuHandle);
    
	int carCatId = CreateComboboxControl(m_pMenuHandle,param,"carcatcombobox",CarControlCB);
	std::vector<std::string> vecCategories;
	GetCarInfo()->GetCategories(vecCategories);
	
	int CatIndex = 0;
	for (unsigned int i=0;i<vecCategories.size();i++)
	{
		GfuiComboboxAddText(m_pMenuHandle,carCatId,vecCategories[i].c_str());
		if (m_strCarCat == vecCategories[i])
			CatIndex = i;
	}

	GfuiComboboxSetSelectedIndex(m_pMenuHandle,carCatId,CatIndex);

	int collId = CreateComboboxControl(m_pMenuHandle,param,"carcollidecombobox",CarCollideCB);
	GfuiComboboxAddText(m_pMenuHandle,collId,"On");
	GfuiComboboxAddText(m_pMenuHandle,collId,"Off");

	int humanHostId = CreateComboboxControl(m_pMenuHandle,param,"hosthumanplayercombobox",humanhostCB);
	GfuiComboboxAddText(m_pMenuHandle,humanHostId,"Yes");
	GfuiComboboxAddText(m_pMenuHandle,humanHostId,"No");

	GfuiComboboxSetSelectedIndex(m_pMenuHandle,humanHostId,0);

    CreateButtonControl(m_pMenuHandle,param,"accept",NULL,onAcceptCB);
    CreateButtonControl(m_pMenuHandle,param,"cancel",NULL,onCancelCB);

    GfParmReleaseHandle(param);

    GfuiScreenActivate(m_pMenuHandle);

	return true;
}
