/***************************************************************************

    file                 : carsetupmenu.cpp
    created              : April 2020
    copyright            : (C) 2020 Robert Reif
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
This file deals with car setup
*/

#include <sstream>
#include <iomanip>

#include <race.h>
#include <car.h>
#include <cars.h>
#include <tracks.h>
#include <drivers.h>
#include <playerpref.h>

#include "carsetupmenu.h"


void CarSetupMenu::onActivate(void *pMenu)
{
    GfLogInfo("Entering Car Setup menu\n");

    // Get the CarSetupMenu instance.
    CarSetupMenu *pCarSetupMenu = static_cast<CarSetupMenu*>(pMenu);

    // Load settings from XML file.
    pCarSetupMenu->loadSettings();

    // Initialize GUI from loaded values.
    pCarSetupMenu->updateControls();
}

void CarSetupMenu::onAccept(void *pMenu)
{
    CarSetupMenu *pCarSetupMenu = static_cast<CarSetupMenu*>(pMenu);

    pCarSetupMenu->storeSettings();

    GfuiScreenActivate(pCarSetupMenu->getPreviousMenuHandle());
}

void CarSetupMenu::onCancel(void *pMenu)
{
    // Get the CarSetupMenu instance from call-back user data.
    CarSetupMenu *pCarSetupMenu = static_cast<CarSetupMenu*>(pMenu);

    // Back to previous screen.
    GfuiScreenActivate(pCarSetupMenu->getPreviousMenuHandle());
}

void CarSetupMenu::onReset(void *pMenu)
{
    // Get the CarSetupMenu instance from call-back user data.
    CarSetupMenu *pCarSetupMenu = static_cast<CarSetupMenu*>(pMenu);

    pCarSetupMenu->brakeRepartition.value = pCarSetupMenu->brakeRepartition.defaultValue;
    pCarSetupMenu->frontARB.value = pCarSetupMenu->frontARB.defaultValue;
    pCarSetupMenu->rearARB.value = pCarSetupMenu->rearARB.defaultValue;
    pCarSetupMenu->frontWing.value = pCarSetupMenu->frontWing.defaultValue;
    pCarSetupMenu->rearWing.value = pCarSetupMenu->rearWing.defaultValue;
    pCarSetupMenu->rearDiffSlip.value = pCarSetupMenu->rearDiffSlip.defaultValue;
    pCarSetupMenu->rearDiffCoastSlip.value = pCarSetupMenu->rearDiffCoastSlip.defaultValue;
    pCarSetupMenu->rearDiffRatio.value = pCarSetupMenu->rearDiffRatio.defaultValue;

    pCarSetupMenu->updateControls();
}

void CarSetupMenu::updateControl(const attnum &att)
{
    if (att.exists)
    {
        std::ostringstream ossValue;
        ossValue << std::fixed << std::setprecision(att.precision) << att.value;
        GfuiEditboxSetString(getMenuHandle(), att.editId, ossValue.str().c_str());

        if (att.minValue == att.maxValue)
            GfuiEnable(getMenuHandle(), att.editId, GFUI_DISABLE);
        else
            GfuiEnable(getMenuHandle(), att.editId, GFUI_ENABLE);
    }
    else
    {
        GfuiEditboxSetString(getMenuHandle(), att.editId, "----");
        GfuiEnable(getMenuHandle(), att.editId, GFUI_DISABLE);
    }
}

void CarSetupMenu::updateControls()
{
    updateControl(brakeRepartition);
    updateControl(frontARB);
    updateControl(rearARB);
    updateControl(frontWing);
    updateControl(rearWing);
    updateControl(rearDiffSlip);
    updateControl(rearDiffCoastSlip);
    updateControl(rearDiffRatio);
}

void CarSetupMenu::loadSetting(const char *label, const char *edit, const char *defaultLabel, 
                               void *hparmCar, void *hparmCarSetup, attnum &att, 
                               const char *section, const char *param, const char *labelStr,
                               const char *units, int precision)
{
    att.labelId = getDynamicControlId(label);
    att.editId = getDynamicControlId(edit);
    att.defaultLabelId = getDynamicControlId(defaultLabel);
    att.section = section;
    att.param = param;
    att.units = units;
    att.precision = precision;

    // Set label text.
    std::ostringstream ossLabel;
    ossLabel << labelStr;
    if (!att.units.empty())
        ossLabel << " (" << att.units << ")"; 
    ossLabel << ":";
    GfuiLabelSetText(getMenuHandle(),
                     att.labelId,
                     ossLabel.str().c_str());
    
    // Read values from car.
    att.exists = GfParmGetNumWithLimits(hparmCar, att.section.c_str(), att.param.c_str(), att.units.c_str(),
                                        &att.defaultValue, &att.minValue, &att.maxValue) == 0;

    ossLabel.str("");
    ossLabel.clear();

    // Set default label text.
    if (att.exists)
    {
        // Check for missing min and max.
        if (att.minValue == att.maxValue)
        {
            ossLabel << std::fixed << std::setprecision(att.precision)
                     << "Default: " << att.defaultValue;
        }
        else
        {
            ossLabel << std::fixed << std::setprecision(att.precision)
                     << "Min: " << att.minValue
                     << "  Default: " << att.defaultValue
                     << "  Max: " << att.maxValue;
        }
    }
    GfuiLabelSetText(getMenuHandle(),
                     att.defaultLabelId,
                     ossLabel.str().c_str());

    // Read value from car setup if avaliable.
    if (hparmCarSetup)
        att.value = GfParmGetNum(hparmCarSetup, att.section.c_str(), att.param.c_str(),
                                 att.units.c_str(), att.defaultValue);
    else
        att.value = att.defaultValue;

    GfLogDebug("section: %s param: %s value: %f min: %f default: %f max: %f\n",
               att.section.c_str(), att.param.c_str(), att.value, att.minValue,
               att.defaultValue, att.maxValue);
}

void CarSetupMenu::loadSettings()
{
    GfuiLabelSetText(getMenuHandle(),
                     getDynamicControlId("CarNameLabel"),
                     getCar()->getName().c_str());

    // Open the XML file of the car.
    std::ostringstream ossCarFileName;
    std::string strCarId = getCar()->getId();
    ossCarFileName << "cars/models/" << strCarId << '/' << strCarId << PARAMEXT;
    void *hparmCar = GfParmReadFile(ossCarFileName.str().c_str(), GFPARM_RMODE_STD);
    if (!hparmCar)
    {
        GfLogError("Car %s (file %s not %s)\n",
                   getCar()->getName().c_str(), ossCarFileName.str().c_str(),
                   GfFileExists(ossCarFileName.str().c_str()) ? "readable" : "found");
        return;
    }

    GfLogInfo("Opened car file: %s\n", ossCarFileName.str().c_str());

    // Open the XML file of the car setup.
    std::ostringstream ossCarSetupFileName;
    std::string strTrackId = getTrack()->getId();
    ossCarSetupFileName << GfLocalDir() << "drivers/human/cars/" << strCarId << '/' << strTrackId << PARAMEXT;
    void *hparmCarSetup = GfParmReadFile(ossCarSetupFileName.str().c_str(), GFPARM_RMODE_STD);
    if (!hparmCarSetup)
    {
        GfLogError("Car Setup %s/%s (file %s not %s)\n",
                   getCar()->getName().c_str(), getTrack()->getName().c_str(), ossCarSetupFileName.str().c_str(),
                   GfFileExists(ossCarSetupFileName.str().c_str()) ? "readable" : "found");
    }
    else
        GfLogInfo("Opened car setup file: %s\n", ossCarSetupFileName.str().c_str());

    loadSetting("BrakeBiasLabel", "BrakeBiasEdit", "BrakeBiasDefaultLabel",
                hparmCar, hparmCarSetup, brakeRepartition, SECT_BRKSYST, PRM_BRKREP,
                "Brake Bias", "%", 1);
    loadSetting("FrontARBLabel", "FrontARBEdit", "FrontARBDefaultLabel",
                hparmCar, hparmCarSetup, frontARB, SECT_FRNTARB, PRM_SPR,
                "Front Anti-Roll Bar", "kN/m", 1);
    loadSetting("RearARBLabel", "RearARBEdit", "RearARBDefaultLabel",
                hparmCar, hparmCarSetup, rearARB, SECT_REARARB, PRM_SPR,
                "Rear Anti-Roll Bar", "kN/m", 1);
    loadSetting("FrontWingLabel", "FrontWingEdit", "FrontWingDefaultLabel",
                hparmCar, hparmCarSetup, frontWing, SECT_FRNTWING, PRM_WINGANGLE,
                "Front Wing Angle", "deg", 1);
    loadSetting("RearWingLabel", "RearWingEdit", "RearWingDefaultLabel",
                hparmCar, hparmCarSetup, rearWing, SECT_REARWING, PRM_WINGANGLE,
                "Rear Wing Angle", "deg", 1);
    loadSetting("RearDiffSlipLabel", "RearDiffSlipEdit", "RearDiffSlipDefaultLabel",
                hparmCar, hparmCarSetup, rearDiffSlip, SECT_REARDIFFERENTIAL, PRM_MAX_SLIP_BIAS,
                "Rear Diff Max Slip Bias", "%", 1);
    loadSetting("RearDiffCoastSlipLabel", "RearDiffCoastSlipEdit", "RearDiffCoastSlipDefaultLabel",
                hparmCar, hparmCarSetup, rearDiffCoastSlip, SECT_REARDIFFERENTIAL, PRM_COAST_MAX_SLIP_BIAS,
                "Rear Diff Coast Max Slip Bias", "%", 1);
    loadSetting("RearDiffRatioLabel", "RearDiffRatioEdit", "RearDiffRatioDefaultLabel",
                hparmCar, hparmCarSetup, rearDiffRatio, SECT_REARDIFFERENTIAL, PRM_RATIO,
                "Rear Diff Ratio", "", 3);

    // Close the XML file of the car.
    GfParmReleaseHandle(hparmCar);

    // Close the XML file of the car setup.
    if (hparmCarSetup)
        GfParmReleaseHandle(hparmCarSetup);
}

void CarSetupMenu::storeSetting(void *hparmCarSetup, attnum &att)
{
    if (att.exists)
    {
        std::string strValue(GfuiEditboxGetString(getMenuHandle(), att.editId));
        std::istringstream issValue(strValue);
        issValue >> att.value;

        GfParmSetNum(hparmCarSetup, att.section.c_str(), att.param.c_str(), att.units.c_str(),
                     att.value, att.minValue, att.maxValue);
    }
}

// Save car setup to XML file.
void CarSetupMenu::storeSettings()
{
    // Open the XML file of the car setup.
    std::ostringstream ossCarSetupFileName;
    std::string strCarId = getCar()->getId();
    std::string strTrackId = getTrack()->getId();
    ossCarSetupFileName << GfLocalDir() << "drivers/human/cars/" << strCarId << '/' << strTrackId << PARAMEXT;
    void *hparmCarSetup = GfParmReadFile(ossCarSetupFileName.str().c_str(), GFPARM_RMODE_STD);
    if (!hparmCarSetup)
    {
        GfLogInfo("Creating car setup file: %s\n", ossCarSetupFileName.str().c_str());

        // Create the car setup file directory if it doesn't exist.
        std::string strDir = ossCarSetupFileName.str();
        strDir.resize(strDir.find_last_of('/'));
        if (!GfDirExists(strDir.c_str()))
            GfDirCreate(strDir.c_str());

        hparmCarSetup = GfParmReadFile(ossCarSetupFileName.str().c_str(),
                                       GFPARM_RMODE_STD | GFPARM_RMODE_CREAT | GFPARM_TEMPLATE);
        if (!hparmCarSetup)
        {
            GfLogError("Car Setup %s/%s (file %s not %s)\n",
                       getCar()->getName().c_str(), getTrack()->getName().c_str(),
                       ossCarSetupFileName.str().c_str(),
                       GfFileExists(ossCarSetupFileName.str().c_str()) ? "readable" : "found");
            return;
        }

        GfLogInfo("Created car setup file: %s\n", ossCarSetupFileName.str().c_str());
    }
    else
        GfLogInfo("Opened car setup file: %s\n", ossCarSetupFileName.str().c_str());

    storeSetting(hparmCarSetup, brakeRepartition);
    storeSetting(hparmCarSetup, frontARB);
    storeSetting(hparmCarSetup, rearARB);
    storeSetting(hparmCarSetup, frontWing);
    storeSetting(hparmCarSetup, rearWing);
    storeSetting(hparmCarSetup, rearDiffSlip);
    storeSetting(hparmCarSetup, rearDiffCoastSlip);
    storeSetting(hparmCarSetup, rearDiffRatio);

    // Write the XML file of the car setup.
    GfParmWriteFile(NULL, hparmCarSetup, strCarId.c_str());

    // Close the XML file of the car setup.
    GfParmReleaseHandle(hparmCarSetup);
}

CarSetupMenu::CarSetupMenu()
: GfuiMenuScreen("carsetupmenu.xml")
{
}

bool CarSetupMenu::initialize(void *pMenu, const GfRace *pRace, const GfDriver *pDriver)
{
    _pRace = pRace;
    _pDriver = pDriver;
    setPreviousMenuHandle(pMenu);

    GfLogDebug("Initializing Car Setup menu: \"%s\"\n", pDriver->getCar()->getName().c_str());

    createMenu(NULL, this, onActivate, NULL, (tfuiCallback)NULL, 1);

    openXMLDescriptor();
    
    createStaticControls();

    createLabelControl("CarNameLabel");

    createLabelControl("BrakeBiasLabel");
    createEditControl("BrakeBiasEdit", this, NULL, NULL);
    createLabelControl("BrakeBiasDefaultLabel");

    createLabelControl("FrontARBLabel");
    createEditControl("FrontARBEdit", this, NULL, NULL);
    createLabelControl("FrontARBDefaultLabel");

    createLabelControl("RearARBLabel");
    createEditControl("RearARBEdit", this, NULL, NULL);
    createLabelControl("RearARBDefaultLabel");

    createLabelControl("FrontWingLabel");
    createEditControl("FrontWingEdit", this, NULL, NULL);
    createLabelControl("FrontWingDefaultLabel");

    createLabelControl("RearWingLabel");
    createEditControl("RearWingEdit", this, NULL, NULL);
    createLabelControl("RearWingDefaultLabel");

    createLabelControl("RearDiffSlipLabel");
    createEditControl("RearDiffSlipEdit", this, NULL, NULL);
    createLabelControl("RearDiffSlipDefaultLabel");

    createLabelControl("RearDiffCoastSlipLabel");
    createEditControl("RearDiffCoastSlipEdit", this, NULL, NULL);
    createLabelControl("RearDiffCoastSlipDefaultLabel");

    createLabelControl("RearDiffRatioLabel");
    createEditControl("RearDiffRatioEdit", this, NULL, NULL);
    createLabelControl("RearDiffRatioDefaultLabel");

    createButtonControl("ApplyButton", this, onAccept);
    createButtonControl("CancelButton", this, onCancel);
    createButtonControl("ResetButton", this, onReset);

    closeXMLDescriptor();

    // Keyboard shortcuts.
    addShortcut(GFUIK_ESCAPE, "Cancel", this, onCancel, NULL);
    addShortcut(GFUIK_RETURN, "Accept", this, onAccept, NULL);
    addShortcut(GFUIK_F1, "Help", getMenuHandle(), GfuiHelpScreen, NULL);
    addShortcut(GFUIK_F12, "Screen-Shot", NULL, GfuiScreenShot, NULL);

    return true;
}
