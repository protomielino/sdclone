/***************************************************************************

    file                 : carselect.cpp
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


/* Car selection / view menu */

#include <carinfo.h>
#include <tgfclient.h>

#include "carselect.h"


void RmCarSelectMenu::onActivateCB(void *pCarSelectMenu)
{
	// Get the RmCarSelectMenu instance.
	RmCarSelectMenu* pMenu = static_cast<RmCarSelectMenu*>(pCarSelectMenu);

	// Get infos about the current car (use the 1st one from the 1st category if none).
	const char* pszCurrentCarName = GfParmGetName(pMenu->getDriver()->carParmHdle);
	CarData* pCurrentCar =
		CarInfo::self()->GetCarDataFromRealName(GfParmGetName(pMenu->getDriver()->carParmHdle));
	if (!pCurrentCar)
		pCurrentCar =
			&CarInfo::self()->GetCarsInCategory(CarInfo::self()->GetCategoryNames()[0])[0];
	
	// Initialize the GUI contents.
	pMenu->resetCarCategoryComboBox(pCurrentCar->strCategoryName);
	pMenu->resetCarModelComboBox(pCurrentCar->strCategoryName, pCurrentCar->strRealName);
	pMenu->resetCarSkinComboBox(pCurrentCar->strRealName, "default");

	// Initialize the car preview image.

	// TODO.
	// Load settings from the <race type> XML file.
}

const CarData* RmCarSelectMenu::getSelectedCarModel() const
{
	const char* pszSelCarRealName =
		GfuiComboboxGetText(GetMenuHandle(), GetDynamicControlId("modelcombo"));

	if (pszSelCarRealName)
		return CarInfo::self()->GetCarDataFromRealName(pszSelCarRealName);

	return 0;
}

void RmCarSelectMenu::onChangeCategory(tComboBoxInfo *pInfo)
{
	// Get the RmCarSelectMenu instance from call-back user data.
	RmCarSelectMenu* pMenu = static_cast<RmCarSelectMenu*>(pInfo->userData);

	// Update GUI (car model and car skin combo-boxes, and preview image).
	pMenu->resetCarModelComboBox(pInfo->vecChoices[pInfo->nPos]);
	pMenu->resetCarSkinComboBox(pMenu->getSelectedCarModel()->strRealName);
}

void RmCarSelectMenu::onChangeModel(tComboBoxInfo *pInfo)
{
	// Get the RmCarSelectMenu instance from call-back user data.
	RmCarSelectMenu* pMenu = static_cast<RmCarSelectMenu*>(pInfo->userData);

	// Update GUI (car skin combo-box and preview image.
	pMenu->resetCarSkinComboBox(pInfo->vecChoices[pInfo->nPos]);
}

void RmCarSelectMenu::onChangeSkin(tComboBoxInfo *pInfo)
{
	// Get the RmCarSelectMenu instance from call-back user data.
	RmCarSelectMenu* pMenu = static_cast<RmCarSelectMenu*>(pInfo->userData);

	// Update GUI (preview image).
	// TODO.
}

void RmCarSelectMenu::onGarageCB(void *pCarSelectMenu)
{
	// Get the RmCarSelectMenu instance from call-back user data.
	const RmCarSelectMenu* pMenu = static_cast<RmCarSelectMenu*>(pCarSelectMenu);
}

void RmCarSelectMenu::onAcceptCB(void *pCarSelectMenu)
{
	// Get the RmCarSelectMenu instance from call-back user data.
	const RmCarSelectMenu* pMenu = static_cast<RmCarSelectMenu*>(pCarSelectMenu);

	// TODO.
	// Save settings in the <race type> XML file.
	
	// Back to previous screen.
	GfuiScreenActivate(pMenu->GetPreviousMenuHandle());
}

void RmCarSelectMenu::onCancelCB(void *pCarSelectMenu)
{
	// Get the RmCarSelectMenu instance from call-back user data.
	const RmCarSelectMenu* pMenu = static_cast<RmCarSelectMenu*>(pCarSelectMenu);
	GfuiScreenActivate(pMenu->GetPreviousMenuHandle());
}

RmCarSelectMenu::RmCarSelectMenu()
: GfuiMenuScreen("carselectmenu.xml"), m_pDriver(0)
{
}

void RmCarSelectMenu::resetCarCategoryComboBox(const std::string& strSelectedCategoryName)
{
	const std::vector<std::string> vecCatNames = CarInfo::self()->GetCategoryNames();
	unsigned nCurrentCategoryIndex = 0;
	int nCategoryComboId = GetDynamicControlId("categorycombo");
	GfuiComboboxClear(GetMenuHandle(), nCategoryComboId);
	for (unsigned nCatIndex = 0; nCatIndex < vecCatNames.size(); nCatIndex++)
	{
		GfuiComboboxAddText(GetMenuHandle(), nCategoryComboId, vecCatNames[nCatIndex].c_str());
		if (!strSelectedCategoryName.empty() && vecCatNames[nCatIndex] == strSelectedCategoryName)
			nCurrentCategoryIndex = nCatIndex;
	}
	
	//GfOut("resetCarCategoryComboBox(%s) : cur=%d\n",
	//	  strSelectedCategoryName.c_str(), nCurrentCategoryIndex);
	GfuiComboboxSetSelectedIndex(GetMenuHandle(), nCategoryComboId, nCurrentCategoryIndex);
}

void RmCarSelectMenu::resetCarModelComboBox(const std::string& strCategoryName,
											const std::string& strSelectedCarRealName)
{
	const std::vector<CarData> vecCarsInCat =	
		CarInfo::self()->GetCarsInCategory(strCategoryName);
	
	unsigned nCurrentCarIndexInCategory = 0;
	int nModelComboId = GetDynamicControlId("modelcombo");
	GfuiComboboxClear(GetMenuHandle(), nModelComboId);
	for (unsigned nCarIndex = 0; nCarIndex < vecCarsInCat.size(); nCarIndex++)
	{
		GfuiComboboxAddText(GetMenuHandle(), nModelComboId,
							vecCarsInCat[nCarIndex].strRealName.c_str());
		if (!strSelectedCarRealName.empty()
			&& vecCarsInCat[nCarIndex].strRealName == strSelectedCarRealName)
			nCurrentCarIndexInCategory = nCarIndex;
	}

	//GfOut("resetCarModelComboBox(cat=%s, selCar=%s) : cur=%d (nCarsInCat=%d)\n",
	//	  strCategoryName.c_str(), strSelectedCarRealName.c_str(),
	//	  nCurrentCarIndexInCategory, vecCarsInCat.size());
	GfuiComboboxSetSelectedIndex(GetMenuHandle(), nModelComboId, nCurrentCarIndexInCategory);
}

void RmCarSelectMenu::resetCarSkinComboBox(const std::string& strCarRealName,
										   const std::string& strSelectedSkinName)
{
	int nSkinComboId = GetDynamicControlId("skincombo");

	// TODO.
	GfuiComboboxClear(GetMenuHandle(), nSkinComboId);
	GfuiComboboxAddText(GetMenuHandle(), nSkinComboId, "default");
}

void RmCarSelectMenu::RunMenu(const trmdDrvElt* pDriver)
{
	// Save target driver.
	setDriver(pDriver);

	// Normally expected job.
	GfuiMenuScreen::RunMenu();
}

bool RmCarSelectMenu::Initialize()
{
	//GfOut("RmCarSelectMenu::Init\n");
	//CarInfo::self()->print();

	// Can only be initialized once.
	if (GetMenuHandle())
		return true;
	
	// Create the menu and all its controls.
	CreateMenuEx(NULL, this, onActivateCB, NULL, (tfuiCallback)NULL, 1);

    OpenXMLDescriptor();
    
    CreateStaticControls();
    
	int categoryId = CreateComboboxControl("categorycombo", this, onChangeCategory);
	int modelId = CreateComboboxControl("modelcombo", this, onChangeModel);
	int skinId = CreateComboboxControl("skincombo", this, onChangeSkin);
	int imageId = CreateStaticImageControl("previewimage");
	int weightId = CreateLabelControl("weightlabel");
	int powerId = CreateLabelControl("powerlabel");
	int topSpeedId = CreateProgressbarControl("topspeedprogress");
	int accelerationId = CreateProgressbarControl("accelerationprogress");
	int handlingId = CreateProgressbarControl("handlingprogress");
	int brakingId = CreateProgressbarControl("brakingprogress");

    CreateButtonControl("garagebutton", this, onGarageCB);
	CreateButtonControl("acceptbutton", this, onAcceptCB);
    CreateButtonControl("cancelbutton", this, onCancelCB);

    CloseXMLDescriptor();

    // Keyboard shortcuts.
    AddShortcut(GFUIK_ESCAPE, "Cancel", this, onCancelCB, NULL);
    AddShortcut(GFUIK_RETURN, "Accept", this, onAcceptCB, NULL);
    AddShortcut(GFUIK_F1, "Help", GetMenuHandle(), GfuiHelpScreen, NULL);
    AddShortcut(GFUIK_F12, "Screen-Shot", NULL, GfuiScreenShot, NULL);
    // TODO Keyboard shortcuts: Add support for shortcuts in GfuiCombobox ?
    //GfuiAddKey(ScrHandle, GFUIK_UP, "Move Up", this, onChangeModel, NULL);

	return true;
}

const trmdDrvElt* RmCarSelectMenu::getDriver() const
{
	return m_pDriver;
}

void RmCarSelectMenu::setDriver(const trmdDrvElt* pDriver)
{
	m_pDriver = pDriver;
}
