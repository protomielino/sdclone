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
#include "network.h"

#include <tgfclient.h>
#include "carsettingsmenu.h"
#include "racemanmenu.h"
#include "carinfo.h"
#include <map>

static void *pPrevMenu = NULL;
std::string CarMenuSettings::m_strCar;

void CarMenuSettings::onActCB(void *P)
{

}

void CarMenuSettings::CarPickCB(tChoiceInfo *pChoices)
{
	m_strCar = pChoices->vecChoices[pChoices->nPos];
}

void CarMenuSettings::onAcceptCB(void *p)
{
	CarData *pData = GetCarInfo()->GetCarDataFromRealName(m_strCar.c_str());
	GetNetwork()->SetCarInfo(pData->strName.c_str());
	GfuiScreenActivate(pPrevMenu);
}

void CarMenuSettings::onCancelCB(void *p)
{
	GfuiScreenActivate(pPrevMenu);
}

bool CarMenuSettings::Init(void* pMenu,const char *pszCar)
{
	std::string strCarCat;
	bool bCollisions;
	GetNetwork()->GetHostSettings(strCarCat,bCollisions);
	pPrevMenu = pMenu;

	m_pMenuHandle = GfuiScreenCreateEx(NULL,NULL,onActCB, 
					 NULL, (tfuiCallback)NULL, 
					 1);

    void *param = LoadMenuXML("carsettingsmenu.xml");
    CreateStaticControls(param,m_pMenuHandle);
    
	int carCatId = CreateComboboxControl(m_pMenuHandle,param,"carcombo",CarPickCB);
	int carImageId = CreateStaticImageControl(m_pMenuHandle,param,"carimage");
	int progressId = CreateProgressbarControl(m_pMenuHandle,param,"topspeedprogress");
	int accelerationId = CreateProgressbarControl(m_pMenuHandle,param,"accelerationprogress");
	int handlingId = CreateProgressbarControl(m_pMenuHandle,param,"handlingprogress");
	int brakingId = CreateProgressbarControl(m_pMenuHandle,param,"brakingprogress");

	std::vector<std::string> vecCars;	
	GetCarInfo()->GetCarsInCategoryRealName(strCarCat.c_str(),vecCars);
	
	m_strCar = pszCar;
	int carIndex = 0;
	for (unsigned i=0;i<vecCars.size();i++)
	{
		GfuiComboboxAddText(m_pMenuHandle,carCatId,vecCars[i].c_str());
		if (vecCars[i] == m_strCar)
			carIndex = i;
	}
	
	GfuiComboboxSetSelectedIndex(m_pMenuHandle,carCatId,carIndex);

	CreateButtonControl(m_pMenuHandle,param,"garage",NULL,NULL);
	CreateButtonControl(m_pMenuHandle,param,"accept",NULL,onAcceptCB);
    CreateButtonControl(m_pMenuHandle,param,"cancel",NULL,onCancelCB);

    GfParmReleaseHandle(param);


    GfuiScreenActivate(m_pMenuHandle);

	SetRacemanMenuHandle(m_pMenuHandle);

	return true;
}

