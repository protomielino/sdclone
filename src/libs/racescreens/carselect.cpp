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

#include <sys/stat.h>
#include <algorithm>
#include <string>
#include <sstream>

#include <tgfclient.h>
#include <cars.h>
#include <drivers.h>

#include "carselect.h"


void RmCarSelectMenu::onActivateCB(void *pCarSelectMenu)
{
	GfLogTrace("Entering Car Select menu\n");

	// Get the RmCarSelectMenu instance.
	RmCarSelectMenu* pMenu = static_cast<RmCarSelectMenu*>(pCarSelectMenu);

	// Get infos about the current car for the current driver
	const GfDriver* pDriver = pMenu->getDriver();
	const GfCar* pCurCar = pDriver->getCar();

	// Initialize the GUI contents.
	GfuiLabelSetText(pMenu->GetMenuHandle(), pMenu->GetDynamicControlId("DriverNameLabel"),
					 pDriver->getName().c_str());
	pMenu->resetCarCategoryComboBox(pCurCar->getCategoryName());
	pMenu->resetCarModelComboBox(pCurCar->getCategoryName(), pCurCar->getName());
	pMenu->resetCarDataSheet(pCurCar->getId());
	pMenu->resetSkinComboBox(pCurCar->getName(), &pDriver->getSkin());
	pMenu->resetCarPreviewImage(pDriver->getSkin());
}

const GfCar* RmCarSelectMenu::getSelectedCarModel() const
{
	const char* pszSelCarName =
		GfuiComboboxGetText(GetMenuHandle(), GetDynamicControlId("ModelCombo"));

	if (pszSelCarName)
		return GfCars::self()->getCarWithName(pszSelCarName);

	return 0;
}

const GfDriverSkin& RmCarSelectMenu::getSelectedSkin() const
{
	return _vecPossSkins[_nCurSkinIndex];
}

void RmCarSelectMenu::setSelectedSkinIndex(int nSkinIndex)
{
	_nCurSkinIndex = nSkinIndex;
}


void RmCarSelectMenu::onChangeCategory(tComboBoxInfo *pInfo)
{
	// Get the RmCarSelectMenu instance from call-back user data.
	RmCarSelectMenu* pMenu = static_cast<RmCarSelectMenu*>(pInfo->userData);

	// Update GUI.
	pMenu->resetCarModelComboBox(pInfo->vecChoices[pInfo->nPos]);
	const GfCar* pSelCar = pMenu->getSelectedCarModel();
	pMenu->resetCarDataSheet(pSelCar->getId());
	pMenu->resetSkinComboBox(pSelCar->getName());
	pMenu->resetCarPreviewImage(pMenu->getSelectedSkin());
}

void RmCarSelectMenu::onChangeModel(tComboBoxInfo *pInfo)
{
	// Get the RmCarSelectMenu instance from call-back user data.
	RmCarSelectMenu* pMenu = static_cast<RmCarSelectMenu*>(pInfo->userData);

	// Update GUI.
	const GfCar* pSelCar = pMenu->getSelectedCarModel();
	pMenu->resetCarDataSheet(pSelCar->getId());
	pMenu->resetSkinComboBox(pSelCar->getName());
	pMenu->resetCarPreviewImage(pMenu->getSelectedSkin());
}

void RmCarSelectMenu::onChangeSkin(tComboBoxInfo *pInfo)
{
	// Get the RmCarSelectMenu instance from call-back user data.
	RmCarSelectMenu* pMenu = static_cast<RmCarSelectMenu*>(pInfo->userData);

	// Update currently selected skin skin index.
	pMenu->setSelectedSkinIndex(pInfo->nPos);
	
	// Update GUI.
	pMenu->resetCarPreviewImage(pMenu->getSelectedSkin());
}

void RmCarSelectMenu::onGarageCB(void *pCarSelectMenu)
{
	// Get the RmCarSelectMenu instance from call-back user data.
	// const RmCarSelectMenu* pMenu = static_cast<RmCarSelectMenu*>(pCarSelectMenu);
	// TODO.
}

void RmCarSelectMenu::onAcceptCB(void *pCarSelectMenu)
{
	// Get the RmCarSelectMenu instance from call-back user data.
	RmCarSelectMenu* pMenu = static_cast<RmCarSelectMenu*>(pCarSelectMenu);

	// Assign new skin choice to the driver.
	GfDriver* pDriver = pMenu->getDriver();
	pDriver->setSkin(pMenu->getSelectedSkin());
	
	// Assign new car choice to the driver (only human drivers can change it).
	if (pDriver->isHuman())
		pDriver->setCar(pMenu->getSelectedCarModel());
	
	// Back to previous screen.
	GfuiScreenActivate(pMenu->GetPreviousMenuHandle());
}

void RmCarSelectMenu::onCancelCB(void *pCarSelectMenu)
{
	// Get the RmCarSelectMenu instance from call-back user data.
	const RmCarSelectMenu* pMenu = static_cast<RmCarSelectMenu*>(pCarSelectMenu);

	// Back to previous screen.
	GfuiScreenActivate(pMenu->GetPreviousMenuHandle());
}

RmCarSelectMenu::RmCarSelectMenu()
: GfuiMenuScreen("carselectmenu.xml"), _nCurSkinIndex(0), _pDriver(0)
{
}

void RmCarSelectMenu::resetCarCategoryComboBox(const std::string& strSelCatName)
{
	const int nCatComboId = GetDynamicControlId("CategoryCombo");

	// Disable the combo-box for non human drivers (robot drivers can't change their car).
	GfuiEnable(GetMenuHandle(), nCatComboId, getDriver()->isHuman() ? GFUI_ENABLE : GFUI_DISABLE);
	
	// Retrieve the available car categories.
	const std::vector<std::string>& vecCatNames = GfCars::self()->getCategoryNames();

	// Load the combo-box from their names (and determine the requested category index).
	unsigned nCurCatIndex = 0;
	GfuiComboboxClear(GetMenuHandle(), nCatComboId);
	for (unsigned nCatIndex = 0; nCatIndex < vecCatNames.size(); nCatIndex++)
	{
		GfuiComboboxAddText(GetMenuHandle(), nCatComboId, vecCatNames[nCatIndex].c_str());
		if (!strSelCatName.empty() && vecCatNames[nCatIndex] == strSelCatName)
			nCurCatIndex = nCatIndex;
	}
	
	// Select the requested category in the combo-box.
	GfuiComboboxSetSelectedIndex(GetMenuHandle(), nCatComboId, nCurCatIndex);

	//GfLogDebug("resetCarCategoryComboBox(%s) : cur=%d\n",
	//		   strSelCatName.c_str(), nCurCatIndex);
}

void RmCarSelectMenu::resetCarModelComboBox(const std::string& strCatName,
											const std::string& strSelCarName)
{
	const int nModelComboId = GetDynamicControlId("ModelCombo");

	// Disable the combo-box for non human drivers (robot drivers can't change their car).
	GfuiEnable(GetMenuHandle(), nModelComboId, getDriver()->isHuman() ? GFUI_ENABLE : GFUI_DISABLE);
	
	// Retrieve car models in the selected category.
	const std::vector<GfCar*> vecCarsInCat =	
		GfCars::self()->getCarsInCategoryWithName(strCatName);

	// Load the combo-box from their real names (and determine the selected model index).
	unsigned nCurrCarIndexInCat = 0;
	GfuiComboboxClear(GetMenuHandle(), nModelComboId);
	for (unsigned nCarIndex = 0; nCarIndex < vecCarsInCat.size(); nCarIndex++)
	{
		GfuiComboboxAddText(GetMenuHandle(), nModelComboId,
							vecCarsInCat[nCarIndex]->getName().c_str());
		if (!strSelCarName.empty() && vecCarsInCat[nCarIndex]->getName() == strSelCarName)
			nCurrCarIndexInCat = nCarIndex;
	}

	// Select the right car in the combo-box.
	GfuiComboboxSetSelectedIndex(GetMenuHandle(), nModelComboId, nCurrCarIndexInCat);

	//GfLogDebug("resetCarModelComboBox(cat=%s, selCar=%s) : cur=%d (nCarsInCat=%d)\n",
	//		   strCatName.c_str(), strSelCarName.c_str(),
	//		   nCurrCarIndexInCat, vecCarsInCat.size());
}

void RmCarSelectMenu::resetCarDataSheet(const std::string& strSelCarId)
{
	static const char* pszDriveWheels[] = { "Rear", "Front", "4" };
	
	// Retrieve selected car.
	const GfCar* pSelCar = GfCars::self()->getCar(strSelCarId);
	
	// Update GUI.
	std::ostringstream ossSpecValue;
	
	ossSpecValue << (long)pSelCar->getMass() << " kg ";
	const long nFRMassPercent = (long)(pSelCar->getFrontRearMassRatio() * 100);
	if (nFRMassPercent > 50)
		ossSpecValue << "(" << nFRMassPercent << "% front)";
	else
		ossSpecValue << "(" << 100 - nFRMassPercent << "% rear)";
	GfuiLabelSetText(GetMenuHandle(), GetDynamicControlId("MassLabel"),
					 ossSpecValue.str().c_str());

	ossSpecValue.str("");
	ossSpecValue << pszDriveWheels[pSelCar->getDriveTrain()] << " WD, "
				 << pSelCar->getGearsCount() << " gears";
	GfuiLabelSetText(GetMenuHandle(), GetDynamicControlId("DriveTrainLabel"),
					 ossSpecValue.str().c_str());

	ossSpecValue.str("");
	ossSpecValue << (long)(pSelCar->getMaxPower() / 75 / G) << " bhp ("
				 << (long)(pSelCar->getMaxPowerSpeed() * 30.0 / PI) << " rpm)";
	GfuiLabelSetText(GetMenuHandle(), GetDynamicControlId("MaxPowerLabel"),
					 ossSpecValue.str().c_str());
	
	ossSpecValue.str("");
	ossSpecValue << (long)pSelCar->getMaxTorque() << " N.m ("
						   << (long)(pSelCar->getMaxTorqueSpeed() * 30.0 / PI) << " rpm)";
	GfuiLabelSetText(GetMenuHandle(), GetDynamicControlId("MaxTorqueLabel"),
					 ossSpecValue.str().c_str());

	GfuiLabelSetText(GetMenuHandle(), GetDynamicControlId("Engine1Label"),
					 "? cyl., ???? cm3");
	
	ossSpecValue.str("");
	ossSpecValue << (pSelCar->isTurboCharged() ? "Turbo-charged" : "Naturally-aspirated");
	GfuiLabelSetText(GetMenuHandle(), GetDynamicControlId("Engine2Label"),
					 ossSpecValue.str().c_str());

	GfuiProgressbarSetValue(GetMenuHandle(), GetDynamicControlId("TopSpeedProgress"),
							pSelCar->getTopSpeed() * 3.6f);
	GfuiProgressbarSetValue(GetMenuHandle(), GetDynamicControlId("PowerMassRatioProgress"),
							pSelCar->getMaxPower() / 75 / G / pSelCar->getMass());
	GfuiProgressbarSetValue(GetMenuHandle(), GetDynamicControlId("LowSpeedGripProgress"),
							pSelCar->getLowSpeedGrip());
	GfuiProgressbarSetValue(GetMenuHandle(), GetDynamicControlId("HighSpeedGripProgress"),
							pSelCar->getHighSpeedGrip());
	GfuiProgressbarSetValue(GetMenuHandle(), GetDynamicControlId("CorneringProgress"),
							pSelCar->getInvertedZAxisInertia());
	
	GfLogDebug("%s : ts=%f, mpr=%f, lsg=%f, hsg=%f, izi=%f\n", strSelCarId.c_str(),
			   pSelCar->getTopSpeed()*3.6f, pSelCar->getMaxPower() / 75 / G / pSelCar->getMass(),
			   pSelCar->getLowSpeedGrip(), pSelCar->getHighSpeedGrip(),
			   pSelCar->getInvertedZAxisInertia());
}

void RmCarSelectMenu::resetSkinComboBox(const std::string& strCarName,
										const GfDriverSkin* pSelSkin)
{
	const int nSkinComboId = GetDynamicControlId("SkinCombo");

	// Get really available skins and previews for this car and current driver.
	const std::string strCarId =
		GfCars::self()->getCarWithName(strCarName)->getId();
	_vecPossSkins = getDriver()->getPossibleSkins(strCarId);
		
	// Load the skin list in the combo-box (and determine the selected skin index).
	GfuiComboboxClear(GetMenuHandle(), nSkinComboId);
	_nCurSkinIndex = 0;
	std::vector<GfDriverSkin>::const_iterator itSkin;
	for (itSkin = _vecPossSkins.begin(); itSkin != _vecPossSkins.end(); itSkin++)
	{
		const std::string strDispSkinName =
			itSkin->getName().empty() ? "standard" : itSkin->getName();
		GfuiComboboxAddText(GetMenuHandle(), nSkinComboId, strDispSkinName.c_str());
		if (pSelSkin && itSkin->getName() == pSelSkin->getName())
			_nCurSkinIndex = itSkin - _vecPossSkins.begin();
	}

	// Select the right skin in the combo-box.
	GfuiComboboxSetSelectedIndex(GetMenuHandle(), nSkinComboId, _nCurSkinIndex);

	// Desactivate the combo if only 1 skin, activate it otherwise.
	GfuiEnable(GetMenuHandle(), nSkinComboId,
			   _vecPossSkins.size() > 1 ? GFUI_ENABLE : GFUI_DISABLE);
}

void RmCarSelectMenu::resetCarPreviewImage(const GfDriverSkin& selSkin)
{
	const int nCarImageId = GetDynamicControlId("PreviewImage");

	// Load the preview image.
	if (GfFileExists(selSkin.getCarPreviewFileName().c_str()))
		GfuiStaticImageSet(GetMenuHandle(), nCarImageId, selSkin.getCarPreviewFileName().c_str(),
						   /* index= */ 0, /* canDeform= */false);
	else
		GfuiStaticImageSet(GetMenuHandle(), nCarImageId, "data/img/nocarpreview.png");
}

void RmCarSelectMenu::RunMenu(GfDriver* pDriver)
{
	// Initialize if not already done.
	if (!GetMenuHandle())
		Initialize();
	
	// Store target driver.
	setDriver(pDriver);

	// Normally expected job.
	GfuiMenuScreen::RunMenu();
}

bool RmCarSelectMenu::Initialize()
{
	// Create the menu and all its controls.
	CreateMenuEx(NULL, this, onActivateCB, NULL, (tfuiCallback)NULL, 1);

    OpenXMLDescriptor();
    
    CreateStaticControls();
    
	CreateLabelControl("DriverNameLabel");
	
	CreateComboboxControl("CategoryCombo", this, onChangeCategory);
	CreateComboboxControl("ModelCombo", this, onChangeModel);
	CreateComboboxControl("SkinCombo", this, onChangeSkin);
	
	CreateStaticImageControl("PreviewImage");
	
	CreateLabelControl("DriveTrainLabel");
	CreateLabelControl("MaxPowerLabel");
	CreateLabelControl("MaxTorqueLabel");
	CreateLabelControl("MassLabel");
	CreateLabelControl("Engine1Label");
	CreateLabelControl("Engine2Label");
	
	CreateProgressbarControl("TopSpeedProgress");
	CreateProgressbarControl("PowerMassRatioProgress");
	CreateProgressbarControl("HighSpeedGripProgress");
	CreateProgressbarControl("LowSpeedGripProgress");
	CreateProgressbarControl("CorneringProgress");

    CreateButtonControl("GarageButton", this, onGarageCB);
	CreateButtonControl("AcceptButton", this, onAcceptCB);
    CreateButtonControl("CancelButton", this, onCancelCB);

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

const GfDriver* RmCarSelectMenu::getDriver() const
{
	return _pDriver;
}

GfDriver* RmCarSelectMenu::getDriver()
{
	return _pDriver;
}

void RmCarSelectMenu::setDriver(GfDriver* pDriver)
{
	_pDriver = pDriver;
}
