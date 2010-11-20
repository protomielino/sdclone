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

#include <carinfo.h>
#include <tgfclient.h>

#include "driver.h"
#include "carselect.h"


void RmCarSelectMenu::onActivateCB(void *pCarSelectMenu)
{
	GfLogTrace("Entering Car Select menu\n");

	// Get the RmCarSelectMenu instance.
	RmCarSelectMenu* pMenu = static_cast<RmCarSelectMenu*>(pCarSelectMenu);

	// Get infos about the current car for the current driver
	// (use the 1st one from the 1st category if none).
	CarData* pCurCar = CarInfo::self()->GetCarData(pMenu->getDriver()->carName);
	if (!pCurCar)
		pCurCar = CarInfo::self()->GetCarsInCategory(CarInfo::self()->GetCategoryNames()[0])[0];

	// Get currently selected skin name for the current driver.
	const char* pszCurSkinName =
		pMenu->getDriver()->skinName ? pMenu->getDriver()->skinName : rmdStdSkinName;
	
	// Initialize the GUI contents.
	GfuiLabelSetText(pMenu->GetMenuHandle(), pMenu->GetDynamicControlId("drivernamelabel"),
					 pMenu->getDriver()->name);
	pMenu->resetCarCategoryComboBox(pCurCar->strCategoryRealName);
	pMenu->resetCarModelComboBox(pCurCar->strCategoryRealName, pCurCar->strRealName);
	pMenu->resetCarDataSheet(pCurCar->strName);
	pMenu->resetCarSkinComboBox(pCurCar->strRealName, pszCurSkinName);
	pMenu->resetCarPreviewImage(pszCurSkinName);
}

const CarData* RmCarSelectMenu::getSelectedCarModel() const
{
	const char* pszSelCarRealName =
		GfuiComboboxGetText(GetMenuHandle(), GetDynamicControlId("modelcombo"));

	if (pszSelCarRealName)
		return CarInfo::self()->GetCarDataFromRealName(pszSelCarRealName);

	return 0;
}

const char* RmCarSelectMenu::getSelectedCarSkin() const
{
	return GfuiComboboxGetText(GetMenuHandle(), GetDynamicControlId("skincombo"));
}

int RmCarSelectMenu::getSelectedCarSkinTargets() const
{
	int nSkinTargets = 0;
	
	const std::map<std::string, int>::const_iterator itSkinTargets =
		_mapSkinTargets.find(getSelectedCarSkin());
	if (itSkinTargets != _mapSkinTargets.end())
		nSkinTargets = itSkinTargets->second;
	
	return nSkinTargets;
}

void RmCarSelectMenu::onChangeCategory(tComboBoxInfo *pInfo)
{
	// Get the RmCarSelectMenu instance from call-back user data.
	RmCarSelectMenu* pMenu = static_cast<RmCarSelectMenu*>(pInfo->userData);

	// Update GUI.
	pMenu->resetCarModelComboBox(pInfo->vecChoices[pInfo->nPos]);
	const CarData* pSelCar = pMenu->getSelectedCarModel();
	pMenu->resetCarDataSheet(pSelCar->strName);
	pMenu->resetCarSkinComboBox(pSelCar->strRealName);
	pMenu->resetCarPreviewImage(pMenu->getSelectedCarSkin());
}

void RmCarSelectMenu::onChangeModel(tComboBoxInfo *pInfo)
{
	// Get the RmCarSelectMenu instance from call-back user data.
	RmCarSelectMenu* pMenu = static_cast<RmCarSelectMenu*>(pInfo->userData);

	// Update GUI.
	const CarData* pSelCar = pMenu->getSelectedCarModel();
	pMenu->resetCarDataSheet(pSelCar->strName);
	pMenu->resetCarSkinComboBox(pSelCar->strRealName);
	pMenu->resetCarPreviewImage(pMenu->getSelectedCarSkin());
}

void RmCarSelectMenu::onChangeSkin(tComboBoxInfo *pInfo)
{
	// Get the RmCarSelectMenu instance from call-back user data.
	RmCarSelectMenu* pMenu = static_cast<RmCarSelectMenu*>(pInfo->userData);

	// Update GUI.
	const CarData* pSelCar = pMenu->getSelectedCarModel();
	pMenu->resetCarPreviewImage(pMenu->getSelectedCarSkin());
}

void RmCarSelectMenu::onGarageCB(void *pCarSelectMenu)
{
	// Get the RmCarSelectMenu instance from call-back user data.
	const RmCarSelectMenu* pMenu = static_cast<RmCarSelectMenu*>(pCarSelectMenu);
}

void RmCarSelectMenu::onAcceptCB(void *pCarSelectMenu)
{
	// Get the RmCarSelectMenu instance from call-back user data.
	RmCarSelectMenu* pMenu = static_cast<RmCarSelectMenu*>(pCarSelectMenu);

	// Save skin choice into the driver structure.
	const char* pszOldCarSkin = pMenu->getDriver()->skinName;
	const char* pszNewCarSkin = pMenu->getSelectedCarSkin();
	if (pszNewCarSkin && (!pszOldCarSkin || strcmp(pszOldCarSkin, pszNewCarSkin)))
	{
		if (pszOldCarSkin)
			free(pMenu->getDriver()->skinName);
		pMenu->getDriver()->skinName = strdup(pszNewCarSkin);
	}
	pMenu->getDriver()->skinTargets = pMenu->getSelectedCarSkinTargets();
	
	// Save car choice into the driver structure if any change
	// (only human drivers can change it).
	if (pMenu->getDriver()->isHuman)
	{
		const CarData* pNewCar = pMenu->getSelectedCarModel();
		
		// Car name.
		const char* pszOldCarName = pMenu->getDriver()->carName;
		const char* pszNewCarName = pNewCar->strName.c_str();
		if (pszNewCarName && (!pszOldCarName || strcmp(pszOldCarName, pszNewCarName)))
		{
			if (pMenu->getDriver()->carName)
				free(pMenu->getDriver()->carName);
			pMenu->getDriver()->carName = strdup(pszNewCarName);

			// Car real name.
			if (pMenu->getDriver()->carRealName)
				free(pMenu->getDriver()->carRealName);
			const char* pszNewCarRealName = pNewCar->strRealName.c_str();
			pMenu->getDriver()->carRealName = strdup(pszNewCarRealName);

			// Car category.
			if (pMenu->getDriver()->carCategory)
				free(pMenu->getDriver()->carCategory);
			const char* pszNewCarCatName = pNewCar->strCategoryName.c_str();
			pMenu->getDriver()->carCategory = strdup(pszNewCarCatName);
		}
	}
	
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

void RmCarSelectMenu::resetCarCategoryComboBox(const std::string& strSelectedCategoryRealName)
{
	const int nCategoryComboId = GetDynamicControlId("categorycombo");

	// Disable the combo-box for non human drivers (robot drivers can't change their car).
	GfuiEnable(GetMenuHandle(), nCategoryComboId, getDriver()->isHuman ? GFUI_ENABLE : GFUI_DISABLE);
	
	// Retrieve the available car categories.
	const std::vector<std::string>& vecCatRealNames = CarInfo::self()->GetCategoryRealNames();

	// Load the combo-box from their names (and determine the requested category index).
	unsigned nCurrentCategoryIndex = 0;
	GfuiComboboxClear(GetMenuHandle(), nCategoryComboId);
	for (unsigned nCatIndex = 0; nCatIndex < vecCatRealNames.size(); nCatIndex++)
	{
		GfuiComboboxAddText(GetMenuHandle(), nCategoryComboId, vecCatRealNames[nCatIndex].c_str());
		if (!strSelectedCategoryRealName.empty() && vecCatRealNames[nCatIndex] == strSelectedCategoryRealName)
			nCurrentCategoryIndex = nCatIndex;
	}
	
	// Select the requested category in the combo-box.
	GfuiComboboxSetSelectedIndex(GetMenuHandle(), nCategoryComboId, nCurrentCategoryIndex);

	//GfLogDebug("resetCarCategoryComboBox(%s) : cur=%d\n",
	//		   strSelectedCategoryRealName.c_str(), nCurrentCategoryIndex);
}

void RmCarSelectMenu::resetCarModelComboBox(const std::string& strCategoryRealName,
											const std::string& strSelectedCarRealName)
{
	const int nModelComboId = GetDynamicControlId("modelcombo");

	// Disable the combo-box for non human drivers (robot drivers can't change their car).
	GfuiEnable(GetMenuHandle(), nModelComboId, getDriver()->isHuman ? GFUI_ENABLE : GFUI_DISABLE);
	
	// Retrieve car models in the selected category.
	const std::vector<CarData*> vecCarsInCat =	
		CarInfo::self()->GetCarsInCategoryRealName(strCategoryRealName);

	// Load the combo-box from their real names (and determine the selected model index).
	unsigned nCurrentCarIndexInCategory = 0;
	GfuiComboboxClear(GetMenuHandle(), nModelComboId);
	for (unsigned nCarIndex = 0; nCarIndex < vecCarsInCat.size(); nCarIndex++)
	{
		GfuiComboboxAddText(GetMenuHandle(), nModelComboId,
							vecCarsInCat[nCarIndex]->strRealName.c_str());
		if (!strSelectedCarRealName.empty()
			&& vecCarsInCat[nCarIndex]->strRealName == strSelectedCarRealName)
			nCurrentCarIndexInCategory = nCarIndex;
	}

	// Select the right car in the combo-box.
	GfuiComboboxSetSelectedIndex(GetMenuHandle(), nModelComboId, nCurrentCarIndexInCategory);

	//GfLogDebug("resetCarModelComboBox(cat=%s, selCar=%s) : cur=%d (nCarsInCat=%d)\n",
	//		   strCategoryRealName.c_str(), strSelectedCarRealName.c_str(),
	//		   nCurrentCarIndexInCategory, vecCarsInCat.size());
}

void RmCarSelectMenu::resetCarDataSheet(const std::string& strSelectedCarName)
{
	// TODO : Merge params with category / user settings ?

	// Open new car params.
	std::ostringstream ossCarXMLFileName;
	ossCarXMLFileName << "cars/" << strSelectedCarName << '/' << strSelectedCarName << PARAMEXT;
	void* newHdle = GfParmReadFile(ossCarXMLFileName.str().c_str(), GFPARM_RMODE_STD);
	
	// Update GUI.
	std::ostringstream ossSpecValue;
	std::ostringstream ossSpecPath;

	// Mass.
	ossSpecValue << (long)GfParmGetNum(newHdle, SECT_CAR, PRM_MASS, 0 /* SI */, 0) << " kg";
	GfuiLabelSetText(GetMenuHandle(), GetDynamicControlId("masslabel"),
					 ossSpecValue.str().c_str());

	const tdble dMaxRev = GfParmGetNum(newHdle, SECT_ENGINE, PRM_REVSLIM, 0, 0);

	tdble dMaxTorque = 0;
	tdble dMaxTorqueRev = 0;
	tdble dMaxPower = 0;
	tdble dMaxPowerRev = 0;
	ossSpecPath << SECT_ENGINE << '/' << ARR_DATAPTS;
	const int nEngineTqCurvePts = GfParmGetEltNb(newHdle, ossSpecPath.str().c_str());
	for (int nPtInd = 2; nPtInd <= nEngineTqCurvePts; nPtInd++)
	{
		ossSpecPath.str("");
		ossSpecPath << SECT_ENGINE << '/' << ARR_DATAPTS << '/' << nPtInd;
		const tdble dRev = GfParmGetNum(newHdle, ossSpecPath.str().c_str(), PRM_RPM, 0, 0);
		if (dRev > dMaxRev)
			break;
		const tdble dTorque = GfParmGetNum(newHdle, ossSpecPath.str().c_str(), PRM_TQ, 0, 0);
		if (dTorque > dMaxTorque)
		{
			dMaxTorque = dTorque;
			dMaxTorqueRev = dRev;
		}
		const tdble dPower = (tdble)(dTorque * dRev / (75 * G)); 
		if (dPower > dMaxPower)
		{
			dMaxPower = dPower;
			dMaxPowerRev = dRev;
		}
	}
	ossSpecValue.str("");
	ossSpecValue << (long)dMaxPower << " bhp (" << dMaxPowerRev * 30.0 / PI << " rpm)";
	GfuiLabelSetText(GetMenuHandle(), GetDynamicControlId("maxpowerlabel"),
					 ossSpecValue.str().c_str());
	
	ossSpecValue.str("");
	ossSpecValue << (long)dMaxTorque << " N.m (" << dMaxTorqueRev * 30.0 / PI << " rpm)";
	GfuiLabelSetText(GetMenuHandle(), GetDynamicControlId("maxtorquelabel"),
					 ossSpecValue.str().c_str());
}

void RmCarSelectMenu::resetCarSkinComboBox(const std::string& strCarRealName,
										   const std::string& strSelectedSkinName)
{
	const int nSkinComboId = GetDynamicControlId("skincombo");

	// Get really available skins and previews for this car and current driver.
	const char* pszCarName =
		CarInfo::self()->GetCarDataFromRealName(strCarRealName)->strName.c_str();
	rmdGetCarSkinsInSearchPath(getDriver(), pszCarName,
							   _vecSkinNames, _mapSkinTargets, _mapPreviewFiles);
		
	// Load the skin list in the combo-box (and determine the selected skin index).
	GfuiComboboxClear(GetMenuHandle(), nSkinComboId);
	unsigned nSelSkinIndex = 0;
	std::vector<std::string>::const_iterator iterSkin;
	for (iterSkin = _vecSkinNames.begin(); iterSkin != _vecSkinNames.end(); iterSkin++)
	{
		GfuiComboboxAddText(GetMenuHandle(), nSkinComboId, iterSkin->c_str());
		if (!strSelectedSkinName.empty() && *iterSkin == strSelectedSkinName)
			nSelSkinIndex = iterSkin - _vecSkinNames.begin();
	}

	// Select the right skin in the combo-box.
	GfuiComboboxSetSelectedIndex(GetMenuHandle(), nSkinComboId, nSelSkinIndex);
}

void RmCarSelectMenu::resetCarPreviewImage(const std::string& strSelectedSkinName)
{
	const int nCarImageId = GetDynamicControlId("previewimage");

	// Get the preview image file from the selected skin name.
	const std::map<std::string, std::string>::const_iterator iterPreviewFile =
		_mapPreviewFiles.find(strSelectedSkinName);
	
	// Load the preview image.
	struct stat st;
	if (iterPreviewFile != _mapPreviewFiles.end()
		&& !stat(iterPreviewFile->second.c_str(), &st))
		GfuiStaticImageSet(GetMenuHandle(), nCarImageId, iterPreviewFile->second.c_str(),
						   /* index= */ 0, /* canDeform= */false);
	else
		GfuiStaticImageSet(GetMenuHandle(), nCarImageId, "data/img/nocarpreview.png");
}

void RmCarSelectMenu::RunMenu(trmdDrvElt* pDriver)
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
    
	CreateLabelControl("drivernamelabel");
	CreateComboboxControl("categorycombo", this, onChangeCategory);
	CreateComboboxControl("modelcombo", this, onChangeModel);
	CreateComboboxControl("skincombo", this, onChangeSkin);
	CreateStaticImageControl("previewimage");
	CreateLabelControl("masslabel");
	CreateLabelControl("maxpowerlabel");
	CreateLabelControl("maxtorquelabel");
	CreateProgressbarControl("topspeedprogress");
	CreateProgressbarControl("accelerationprogress");
	CreateProgressbarControl("handlingprogress");
	CreateProgressbarControl("brakingprogress");

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
	return _pDriver;
}

trmdDrvElt* RmCarSelectMenu::getDriver()
{
	return _pDriver;
}

void RmCarSelectMenu::setDriver(trmdDrvElt* pDriver)
{
	_pDriver = pDriver;
}
