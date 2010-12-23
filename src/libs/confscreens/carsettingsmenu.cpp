/***************************************************************************

    file                 : carsettingsmenu.cpp
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
This file deals with car settings
*/

#include <cars.h>
#include <network.h>

#include "carsettingsmenu.h"


static void *pPrevMenu = NULL;
std::string CarSettingsMenu::m_strCar;

void CarSettingsMenu::onActCB(void *P)
{

}

void CarSettingsMenu::CarPickCB(tComboBoxInfo *pInfo)
{
	m_strCar = pInfo->vecChoices[pInfo->nPos];
}

void CarSettingsMenu::onAcceptCB(void *p)
{
	GfCar *pCar = GfCars::self()->getCarWithName(m_strCar);
	GetNetwork()->SetCarInfo(pCar->getId().c_str());
	GfuiScreenActivate(pPrevMenu);
}

void CarSettingsMenu::onCancelCB(void *p)
{
	GfuiScreenActivate(pPrevMenu);
}

CarSettingsMenu::CarSettingsMenu()
: GfuiMenuScreen("carselectionmenu.xml")
{
}

bool CarSettingsMenu::Init(void* pMenu,const char *pszCar)
{
	std::string strCarCat;
	bool bCollisions;
	GetNetwork()->GetHostSettings(strCarCat,bCollisions);
	pPrevMenu = pMenu;

	void* pMenuHandle = GfuiScreenCreateEx(NULL,NULL,onActCB, 
										   NULL, (tfuiCallback)NULL, 
										   1);
	SetMenuHandle(pMenuHandle);

    OpenXMLDescriptor();
    
    CreateStaticControls();
    
	int carCatId = CreateComboboxControl("modelcombo",NULL,CarPickCB);
	int skinId = CreateComboboxControl("skincombo",NULL,NULL);
	int carImageId = CreateStaticImageControl("carpreviewimage");
	int progressId = CreateProgressbarControl("topspeedprogress");
	int accelerationId = CreateProgressbarControl("accelerationprogress");
	int handlingId = CreateProgressbarControl("handlingprogress");
	int brakingId = CreateProgressbarControl("brakingprogress");

	const std::vector<std::string> vecCarRealNames =
		GfCars::self()->getCarNamesInCategory(strCarCat);
	
	m_strCar = pszCar;
	int carIndex = 0;
	for (unsigned i=0;i<vecCarRealNames.size();i++)
	{
		GfuiComboboxAddText(pMenuHandle,carCatId,vecCarRealNames[i].c_str());
		if (vecCarRealNames[i] == m_strCar)
			carIndex = i;
	}
	
	GfuiComboboxSetSelectedIndex(pMenuHandle,carCatId,carIndex);

	CreateButtonControl("accept",NULL,onAcceptCB);
    CreateButtonControl("cancel",NULL,onCancelCB);

    OpenXMLDescriptor();
    
	return true;
}
