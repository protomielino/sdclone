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

    // Get the current page values.
    pCarSetupMenu->readCurrentPage();

    // Save all page values.
    pCarSetupMenu->storeSettings();

    // Switch back to garage menu.
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

    // Reset all values on current page to their defaults.
    for (size_t index = 0; index < ITEMS_PER_PAGE; index++)
    {
        attnum &att = pCarSetupMenu->items[pCarSetupMenu->currentPage][index];
        att.value = att.defaultValue;
    }

    // Update the GUI.
    pCarSetupMenu->updateControls();
}

void CarSetupMenu::readCurrentPage()
{
    for (size_t index = 0; index < ITEMS_PER_PAGE; index++)
    {
        attnum &att = items[currentPage][index];
        if (att.exists)
        {
            std::string strValue(GfuiEditboxGetString(getMenuHandle(), att.editId));
            std::istringstream issValue(strValue);
            issValue >> att.value;
        }
    }
}

void CarSetupMenu::onPrevious(void *pMenu)
{
    // Get the CarSetupMenu instance from call-back user data.
    CarSetupMenu *pCarSetupMenu = static_cast<CarSetupMenu*>(pMenu);

    // Get the current page values.
    pCarSetupMenu->readCurrentPage();

    // Switch to previous page.
    pCarSetupMenu->currentPage--;

    // Update the GUI.
    pCarSetupMenu->updateControls();
}

void CarSetupMenu::onNext(void *pMenu)
{
    // Get the CarSetupMenu instance from call-back user data.
    CarSetupMenu *pCarSetupMenu = static_cast<CarSetupMenu*>(pMenu);

    // Get the current page values.
    pCarSetupMenu->readCurrentPage();

    // Switch to next page.
    pCarSetupMenu->currentPage++;

    // update the GUI.
    pCarSetupMenu->updateControls();
}

void CarSetupMenu::updateControls()
{
    GfuiEnable(getMenuHandle(), getDynamicControlId("PreviousButton"), currentPage != 0 ? GFUI_ENABLE : GFUI_DISABLE);
    GfuiEnable(getMenuHandle(), getDynamicControlId("NextButton"), currentPage < (items.size() - 1) ? GFUI_ENABLE : GFUI_DISABLE);

    for (size_t index = 0; index < ITEMS_PER_PAGE; ++index)
    {
        attnum &att = items[currentPage][index];

        // Set label text.
        std::ostringstream ossLabel;
        if (!att.label.empty())
        {
            ossLabel << att.label;
            if (!att.units.empty())
                ossLabel << " (" << att.units << ")"; 
            ossLabel << ":";
        }

        GfuiLabelSetText(getMenuHandle(),
                         att.labelId,
                         ossLabel.str().c_str());

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
    }

    // Update the edit boxes.
    for (size_t index = 0; index < ITEMS_PER_PAGE; ++index)
    {
        attnum &att = items[currentPage][index];

        if (att.label.empty())
        {
            GfuiVisibilitySet(getMenuHandle(), att.editId, GFUI_INVISIBLE);
        }
        else
        {
            GfuiVisibilitySet(getMenuHandle(), att.editId, GFUI_VISIBLE);

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
    }
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
        GfLogInfo("Car Setup: %s/%s (file %s not %s)\n",
                   getCar()->getName().c_str(), getTrack()->getName().c_str(), ossCarSetupFileName.str().c_str(),
                   GfFileExists(ossCarSetupFileName.str().c_str()) ? "readable" : "found");
    }
    else
        GfLogInfo("Opened car setup file: %s\n", ossCarSetupFileName.str().c_str());

    void *hparmItems = GfuiMenuLoad("carsetupmenuitems.xml");
    if (!hparmItems)
    {
        GfLogError("Car Setup Items (file %s not %s)\n",
                   "carsetupmenuitems.xml",
                   GfFileExists("carsetupmenuitems.xml") ? "readable" : "found");
    }
    else
    {
        GfLogInfo("Opened car setup menu items file: %s\n", "carsetupmenuitems.xml");

        std::vector<std::string> sections = GfParmListGetSectionNamesList(hparmItems);

        for (size_t i = 0; i < sections.size(); ++i)
        {
            std::string strSection = sections[i];

            GfLogDebug("section %zu: %s\n", i, strSection.c_str());

            size_t page = GfParmGetNum(hparmItems, strSection.c_str(), "page", "", 0);
            size_t index = GfParmGetNum(hparmItems, strSection.c_str(), "index", "", 0);

            if (page <= items.size())
                items.resize(page + 1);

            if (index >= ITEMS_PER_PAGE)
            {
                GfLogError("Invalid index %zu\n", index);
                continue;
            }

            attnum &att = items[page][index];

            att.labelId = getDynamicControlId(std::string("Label" + std::to_string(index)).c_str());
            att.editId = getDynamicControlId(std::string("Edit" + std::to_string(index)).c_str());
            att.defaultLabelId = getDynamicControlId(std::string("DefaultLabel" + std::to_string(index)).c_str());
            att.section = GfParmGetStr(hparmItems, strSection.c_str(), "section", "");
            att.param = GfParmGetStr(hparmItems, strSection.c_str(), "param", "");
            att.units = GfParmGetStr(hparmItems, strSection.c_str(), "unit", "");
            att.label = GfParmGetStr(hparmItems, strSection.c_str(), "label", "");
            att.precision = GfParmGetNum(hparmItems, strSection.c_str(), "precision", "", 0);

            // Read values from car.
            att.exists = GfParmGetNumWithLimits(hparmCar, att.section.c_str(), att.param.c_str(), att.units.c_str(),
                                                &att.defaultValue, &att.minValue, &att.maxValue) == 0;

            // Read value from car setup if avaliable.
            if (hparmCarSetup)
                att.value = GfParmGetNum(hparmCarSetup, att.section.c_str(), att.param.c_str(),
                                         att.units.c_str(), att.defaultValue);
            else
                att.value = att.defaultValue;

            GfLogDebug("section: \"%s\" param: \"%s\" units: \"%s\" label: \"%s\" page: %zu "
                       "index: %zu precision: %d labelIs: %d editId: %d defaultLabelId: %d "
                       "exists: %d min: %f default %f max: %f value: %f\n",
                       att.section.c_str(), att.param.c_str(), att.units.c_str(), att.label.c_str(),
                       page, index, att.precision, att.labelId, att.editId, att.defaultLabelId,
                       att.exists, att.minValue, att.defaultValue, att.maxValue, att.value);
        }

        // Save the control id for all items.
        for (size_t page = 0; page < items.size(); ++page)
        {
            for (size_t index = 0; index < ITEMS_PER_PAGE; ++index)
            {
                attnum &att = items[page][index];

                if (!att.labelId)
                    att.labelId = getDynamicControlId(std::string("Label" + std::to_string(index)).c_str());
                if (!att.editId)
                    att.editId = getDynamicControlId(std::string("Edit" + std::to_string(index)).c_str());
                if (!att.defaultLabelId)
                    att.defaultLabelId = getDynamicControlId(std::string("DefaultLabel" + std::to_string(index)).c_str());
            }
        }
    }

    // Close the XML file of the car.
    GfParmReleaseHandle(hparmCar);

    // Close the XML file of the car setup.
    if (hparmCarSetup)
        GfParmReleaseHandle(hparmCarSetup);
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

    // Store all items.
    for (size_t page = 0; page < items.size(); ++page)
    {
        for (size_t index = 0; index < ITEMS_PER_PAGE; ++index)
        {
            attnum &att = items[page][index];
            if (att.exists)
            {
                GfParmSetNum(hparmCarSetup, att.section.c_str(), att.param.c_str(), att.units.c_str(),
                             att.value, att.minValue, att.maxValue);
            }
        }
    }

    // Write the XML file of the car setup.
    GfParmWriteFile(NULL, hparmCarSetup, strCarId.c_str());

    // Close the XML file of the car setup.
    GfParmReleaseHandle(hparmCarSetup);
}

CarSetupMenu::CarSetupMenu()
: GfuiMenuScreen("carsetupmenu.xml")
, currentPage(0)
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

    // Create items.
    for (size_t index = 0; index < ITEMS_PER_PAGE; ++index)
    {
        createLabelControl(std::string("Label" + std::to_string(index)).c_str());
        createEditControl(std::string("Edit" + std::to_string(index)).c_str(), this, NULL, NULL);
        createLabelControl(std::string("DefaultLabel" + std::to_string(index)).c_str());
    }

    // Create buttons.
    createButtonControl("ApplyButton", this, onAccept);
    createButtonControl("CancelButton", this, onCancel);
    createButtonControl("ResetButton", this, onReset);
    createButtonControl("PreviousButton", this, onPrevious);
    createButtonControl("NextButton", this, onNext);

    closeXMLDescriptor();

    // Keyboard shortcuts.
    addShortcut(GFUIK_ESCAPE, "Cancel", this, onCancel, NULL);
    addShortcut(GFUIK_RETURN, "Accept", this, onAccept, NULL);
    addShortcut(GFUIK_F1, "Help", getMenuHandle(), GfuiHelpScreen, NULL);
    addShortcut(GFUIK_F12, "Screen-Shot", NULL, GfuiScreenShot, NULL);

    return true;
}
