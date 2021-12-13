/***************************************************************************

    file                 : displayconfig.cpp
    created              : October 2010
    copyright            : (C) 2010 Jean-Philippe Meuret
    web                  : speed-dreams.sourceforge.net
    version              : $Id$

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/* Display configuration menu */

#include <sstream>

#include <tgfclient.h>
#include <graphic.h>

#include "legacymenu.h"
#include "displayconfig.h"


// Some consts.
static const char* ADisplayModes[DisplayMenu::nDisplayModes] = { "Full-screen", "Windowed" };
static const char* MonitorTypes[DisplayMenu::nDisplayTypes] = { "None", "4:3", "16:9", "21:9" };
static const char* SpansplitValues[] = { GR_VAL_NO, GR_VAL_YES };
#ifndef NoMaxRefreshRate
static const int AMaxRefreshRates[] = { 0, 30, 40, 50, 60, 75, 85, 100, 120, 150, 200 };
static const int NMaxRefreshRates = sizeof(AMaxRefreshRates) / sizeof(AMaxRefreshRates[0]);
#endif	

// The unique DisplayMenu instance.
static DisplayMenu* PDisplayMenu = 0;

static int sBezelCompID;
static int sArcRatioID;


// Call-backs ================================================================
void DisplayMenu::onActivate(void *pDisplayMenu)
{
	// Get the DisplayMenu instance.
	DisplayMenu* pMenu = static_cast<DisplayMenu*>(pDisplayMenu);

	// Load settings from XML file.
    pMenu->loadSettings();

	// Initialize GUI from loaded values.
	pMenu->updateControls();
}

void DisplayMenu::onChangeDisplayMode(tComboBoxInfo *pInfo)
{
 	// Get the DisplayMenu instance from call-back user data.
	DisplayMenu* pMenu = static_cast<DisplayMenu*>(pInfo->userData);

	pMenu->setDisplayMode((EDisplayMode)pInfo->nPos);
}

void DisplayMenu::onChangeScreenSize(tComboBoxInfo *pInfo)
{
 	// Get the DisplayMenu instance from call-back user data.
	DisplayMenu* pMenu = static_cast<DisplayMenu*>(pInfo->userData);

	pMenu->setScreenSizeIndex(pInfo->nPos);
}

void DisplayMenu::onChangeMonitorType(tComboBoxInfo *pInfo)
{
 	// Get the DisplayMenu instance from call-back user data.
	DisplayMenu* pMenu = static_cast<DisplayMenu*>(pInfo->userData);

	pMenu->setMonitorType((EDisplayType)pInfo->nPos);
}

void DisplayMenu::onChangeArcRatio(void *pDisplayMenu)
{
    printf("DisplayMenu::onChangeArcRatio() \n");
    DisplayMenu* pMenu = static_cast<DisplayMenu*>(pDisplayMenu);
    //printf("this = %p \n", this);
    printf("pMenu->getMenuHandle() = %p \n", pMenu->getMenuHandle());
    printf("PDisplayMenu->getMenuHandle() = %p \n", PDisplayMenu->getMenuHandle());

    char* val = GfuiEditboxGetString(PDisplayMenu->getMenuHandle(), sArcRatioID);
    sscanf(val, "%g", &pMenu->_fArcRatio);
    if (PDisplayMenu->_fArcRatio > 2.0f)
        PDisplayMenu->_fArcRatio = 2.0f;
    else if (PDisplayMenu->_fArcRatio < 0.0f)
        PDisplayMenu->_fArcRatio = 0.0f;

    char buf[32];
    sprintf(buf, "%g", PDisplayMenu->_fArcRatio);
    GfuiEditboxSetString(PDisplayMenu->getMenuHandle(), sArcRatioID, buf);

}

#ifndef NoMaxRefreshRate
void DisplayMenu::onChangeMaxRefreshRate(tComboBoxInfo *pInfo)
{
 	// Get the DisplayMenu instance from call-back user data.
	DisplayMenu* pMenu = static_cast<DisplayMenu*>(pInfo->userData);

	pMenu->setMaxRefreshRateIndex(pInfo->nPos);
}
#endif	

// Re-init screen to take new graphical settings into account (implies process restart).
void DisplayMenu::onAccept(void *pDisplayMenu)
{
	// Get the DisplayMenu instance from call-back user data.
	DisplayMenu* pMenu = static_cast<DisplayMenu*>(pDisplayMenu);

    // Force current control to loose focus (if one had it) and update associated variable.
    GfuiUnSelectCurrent();

    // Save display settings.
    pMenu->storeSettings();

    // Shutdown the user interface.
	LegacyMenu::self().shutdown();

    // Restart the game.
    GfuiApp().restart();
}

void DisplayMenu::onCancel(void *pDisplayMenu)
{
	// Get the DisplayMenu instance from call-back user data.
	const DisplayMenu* pMenu = static_cast<DisplayMenu*>(pDisplayMenu);

	// Back to previous screen.
	GfuiScreenActivate(pMenu->getPreviousMenuHandle());
}

void DisplayMenu::updateControls()
{
	int nControlId = getDynamicControlId("DisplayModeCombo");
	GfuiComboboxSetSelectedIndex(getMenuHandle(), nControlId, _eDisplayMode);
	
	resetScreenSizes();

#ifndef NoMaxRefreshRate
	nControlId = getDynamicControlId("MaxRefreshRateCombo");
	int nMaxRefRateIndex = 0; // Defaults to None.
	for (int nMaxRefRateInd = 0; nMaxRefRateInd < NMaxRefreshRates; nMaxRefRateInd++)
		if (_nMaxRefreshRate <= AMaxRefreshRates[nMaxRefRateInd])
		{
			nMaxRefRateIndex = nMaxRefRateInd;
			break;
		}
	GfuiComboboxSetSelectedIndex(getMenuHandle(), nControlId, nMaxRefRateIndex);
#endif	
}

void DisplayMenu::loadSettings()
{
	// Open screen config params file.
	void* hScrConfParams =
		GfParmReadFileLocal(GFSCR_CONF_FILE, GFPARM_RMODE_STD | GFPARM_RMODE_CREAT);

	// Select the screen properties to edit : the 'in-test' ones if present,
	// otherwise the 'validated' ones.
	const char* pszScrPropSec =
		GfParmExistsSection(hScrConfParams, GFSCR_SECT_INTESTPROPS)
		? GFSCR_SECT_INTESTPROPS : GFSCR_SECT_VALIDPROPS;

	// Display mode : Full-screen or Windowed.
	const char *pszFullScreen =
		GfParmGetStr(hScrConfParams, pszScrPropSec, GFSCR_ATT_FSCR, GFSCR_VAL_NO);
	_eDisplayMode = strcmp(pszFullScreen, GFSCR_VAL_YES) ? eWindowed : eFullScreen;

	// Screen / window size.
	_nScreenWidth = (int)GfParmGetNum(hScrConfParams, pszScrPropSec, GFSCR_ATT_WIN_X, NULL, 800);
	_nScreenHeight = (int)GfParmGetNum(hScrConfParams, pszScrPropSec, GFSCR_ATT_WIN_Y, NULL, 600);

#ifndef NoMaxRefreshRate
	// Max. refresh rate (Hz).
	_nMaxRefreshRate =
		(int)GfParmGetNum(hScrConfParams, pszScrPropSec, GFSCR_ATT_MAXREFRESH, NULL, 0);
#endif	
	
	// Release screen config params file.
	GfParmReleaseHandle(hScrConfParams);
}

// Save graphical settings to XML file.
void DisplayMenu::storeSettings() const
{
	// Open screen config params file.
	void* hScrConfParams =
		GfParmReadFileLocal(GFSCR_CONF_FILE, GFPARM_RMODE_STD | GFPARM_RMODE_CREAT);

	// Write new screen properties to the 'in-test' section, with 'to do' test state
	// (will become 'validated' after a succesfull restart, once we are sure they are OK :
	//  see guiscreen::GfScrShutdown).
	GfParmSetStr(hScrConfParams, GFSCR_SECT_INTESTPROPS, GFSCR_ATT_TESTSTATE, GFSCR_VAL_TODO);
	
	GfParmSetNum(hScrConfParams, GFSCR_SECT_INTESTPROPS, GFSCR_ATT_WIN_X, (char*)NULL, _nScreenWidth);
	GfParmSetNum(hScrConfParams, GFSCR_SECT_INTESTPROPS, GFSCR_ATT_WIN_Y, (char*)NULL, _nScreenHeight);
#ifndef NoMaxRefreshRate
	GfParmSetNum(hScrConfParams, GFSCR_SECT_INTESTPROPS, GFSCR_ATT_MAXREFRESH, (char*)NULL, _nMaxRefreshRate);
#endif

	const char* pszDisplMode =
		(_eDisplayMode == eFullScreen) ? GFSCR_VAL_YES : GFSCR_VAL_NO;
	GfParmSetStr(hScrConfParams, GFSCR_SECT_INTESTPROPS, GFSCR_ATT_FSCR, pszDisplMode);

	// Write and release screen config params file.
	GfParmWriteFile(NULL, hScrConfParams, "Screen");
	GfParmReleaseHandle(hScrConfParams);
}

void DisplayMenu::setDisplayMode(EDisplayMode eMode)
{
	if (_eDisplayMode != eMode)
	{
		_eDisplayMode = eMode;
		
		resetScreenSizes();
	}
}

void DisplayMenu::resetScreenSizes()
{
	// Determine possible / supported screen sizes for the current display mode, color depth,
	// and video features detection mode.
	int nDefScreenSizes;
	tScreenSize* aDefScreenSizes = GfScrGetDefaultSizes(&nDefScreenSizes);
	if (_aScreenSizes && _aScreenSizes != aDefScreenSizes)
		free(_aScreenSizes);

	_aScreenSizes =
		GfScrGetSupportedSizes(32, _eDisplayMode == eFullScreen, &_nNbScreenSizes);
	if (_eDisplayMode != eFullScreen || _aScreenSizes == (tScreenSize*)-1 || _aScreenSizes == 0)
	{
		_aScreenSizes = aDefScreenSizes;
		_nNbScreenSizes = nDefScreenSizes;
	}


	// Update combo-box with new possible sizes.
	const int nComboId = getDynamicControlId("ScreenSizeCombo");
	GfuiComboboxClear(getMenuHandle(), nComboId);
	std::ostringstream ossSize;
	for (int nSizeIndex = 0; nSizeIndex < _nNbScreenSizes; nSizeIndex++)
	{
		ossSize.str("");
		ossSize << _aScreenSizes[nSizeIndex].width << " x " << _aScreenSizes[nSizeIndex].height;
		GfuiComboboxAddText(getMenuHandle(), nComboId, ossSize.str().c_str());
	}
	
	// Try and find the closest screen size to the current choice in the new list.
	// 1) Is there an exact match ?
	int nScreenSizeIndex = -1;
	for (int nSizeInd = 0; nSizeInd < _nNbScreenSizes; nSizeInd++)
		if (_nScreenWidth == _aScreenSizes[nSizeInd].width
			&& _nScreenHeight == _aScreenSizes[nSizeInd].height)
		{
			nScreenSizeIndex = nSizeInd;
			break;
		}

	// 2) Is there an approximative match ?
	if (nScreenSizeIndex < 0)
		for (int nSizeInd = 0; nSizeInd < _nNbScreenSizes; nSizeInd++)
			if (_nScreenWidth <= _aScreenSizes[nSizeInd].width
				&& _nScreenHeight <= _aScreenSizes[nSizeInd].height)
			{
				nScreenSizeIndex = nSizeInd;
				break;
			}

	// 3) Not found : the closest is the biggest.
	if (nScreenSizeIndex < 0)
		nScreenSizeIndex = _nNbScreenSizes - 1;

	// 4) Store new screen size.
	_nScreenWidth = _aScreenSizes[nScreenSizeIndex].width;
	_nScreenHeight = _aScreenSizes[nScreenSizeIndex].height;
	
	// Select the found one in the combo-box.
	GfuiComboboxSetSelectedIndex(getMenuHandle(), nComboId, nScreenSizeIndex);
}

void DisplayMenu::setScreenSizeIndex(int nIndex)
{
	_nScreenWidth = _aScreenSizes[nIndex].width;
	_nScreenHeight = _aScreenSizes[nIndex].height;
}

void DisplayMenu::setMonitorType(EDisplayType eType)
{
	if (_eDisplayType != eType)
	{
		_eDisplayType = eType;
	}
}

void DisplayMenu::setArcRatio(float ratio)
{
    printf("DisplayMenu::setArcRatio() \n");
    printf("this = %p \n", this);

    _fArcRatio = ratio;
    if (_fArcRatio > 2.0f)
    {
        _fArcRatio = 2.0f;
    }
    else if (_fArcRatio < 0.0f)
    {
        _fArcRatio = 0.0f;
    }
}

#ifndef NoMaxRefreshRate
void DisplayMenu::setMaxRefreshRateIndex(int nIndex)
{
	_nMaxRefreshRate = AMaxRefreshRates[nIndex];
}
#endif	

DisplayMenu::DisplayMenu()
: GfuiMenuScreen("displayconfigmenu.xml")
{
	_nNbScreenSizes = -1;
	_aScreenSizes = 0;
	_eDisplayMode = eWindowed;
	_nScreenWidth = 800;
	_nScreenHeight = 600;
	_eDisplayType = eNone;
	_fArcRatio = 1.0f;
	_fBezelComp = 110.0f;
	_fScreenDist = 1.0f;
#ifndef NoMaxRefreshRate
	_nMaxRefreshRate = 0;
#endif	
    printf("this = %p \n", this);
}

DisplayMenu::~DisplayMenu()
{
	int nDefScreenSizes;
	tScreenSize* aDefScreenSizes = GfScrGetDefaultSizes(&nDefScreenSizes);
	if (_aScreenSizes && _aScreenSizes != aDefScreenSizes)
		free(_aScreenSizes);
}

bool DisplayMenu::initialize(void *pPreviousMenu)
{
	// Save the menu to return to.
	setPreviousMenuHandle(pPreviousMenu);

	// Create the menu and all its controls.
	createMenu(NULL, this, onActivate, NULL, (tfuiCallback)NULL, 1);

    openXMLDescriptor();
    
    createStaticControls();
    
	createComboboxControl("ScreenSizeCombo", this, onChangeScreenSize);

	const int nDisplayModeComboId =
		createComboboxControl("DisplayModeCombo", this, onChangeDisplayMode);

	const int nDisplayTypeComboId =
		createComboboxControl("MonitorTypeCombo", this, onChangeMonitorType);

	const int nSpanSplitsComboId =
		createComboboxControl("SpanSplitsCombo", this, NULL);

	sBezelCompID = createEditControl("bezelcompedit", this, NULL, NULL);
	createEditControl("screendistedit", this, NULL, NULL);
	sArcRatioID = createEditControl("arcratioedit", this, NULL, onChangeArcRatio);


#ifndef NoMaxRefreshRate
	const int nMaxRefRateComboId =
		createComboboxControl("MaxRefreshRateCombo", this, onChangeMaxRefreshRate);
#endif	

	createButtonControl("ApplyButton", this, onAccept);
	createButtonControl("CancelButton", this, onCancel);

	addShortcut(GFUIK_RETURN, "Apply", this, onAccept, 0);
	addShortcut(GFUIK_ESCAPE, "Cancel", this, onCancel, 0);
    // TODO Keyboard shortcuts: Add support for shortcuts in GfuiCombobox ?
	//addShortcut(GFUIK_LEFT, "Previous Resolution", this, onChangeScreenSize, 0);
	//addShortcut(GFUIK_RIGHT, "Next Resolution", this, onChangeScreenSize, 0);
	addShortcut(GFUIK_F1, "Help", getMenuHandle(), GfuiHelpScreen, 0);
	addShortcut(GFUIK_F12, "Screen-Shot", 0, GfuiScreenShot, 0);

    closeXMLDescriptor();

	// Load constant value lists in combo-boxes.
	// 1) Color depths combo : not constant, as depends on selected video detection mode.
	
	// 2) Display modes combo.
	for (int nDispModeInd = 0; nDispModeInd < nDisplayModes; nDispModeInd++)
		GfuiComboboxAddText(getMenuHandle(), nDisplayModeComboId, ADisplayModes[nDispModeInd]);

	// 3) Monitor type combo.
	for (int nDisplayTypeInd = 0; nDisplayTypeInd < nDisplayTypes; nDisplayTypeInd++)
		GfuiComboboxAddText(getMenuHandle(), nDisplayTypeComboId, MonitorTypes[nDisplayTypeInd]);

	for (int index = 0; index < 2; index++)
		GfuiComboboxAddText(getMenuHandle(), nSpanSplitsComboId, SpansplitValues[index]);


#ifndef NoMaxRefreshRate
	// 6) Max refresh rate combo.
	std::ostringstream ossMaxRefRate;
	for (int nRefRateInd = 0; nRefRateInd < NMaxRefreshRates; nRefRateInd++)
	{
		ossMaxRefRate.str("");
		if (AMaxRefreshRates[nRefRateInd] != 0)
			ossMaxRefRate << AMaxRefreshRates[nRefRateInd];
		else
			ossMaxRefRate << "None";
		GfuiComboboxAddText(getMenuHandle(), nMaxRefRateComboId, ossMaxRefRate.str().c_str());
	}
#endif	

	return true;
}

/** Create and activate the display options menu screen.
    @ingroup	screen
    @param	precMenu	previous menu to return to
*/
void* DisplayMenuInit(void *pPreviousMenu)
{
	if (!PDisplayMenu)
	{
		PDisplayMenu = new DisplayMenu;
	
		PDisplayMenu->initialize(pPreviousMenu);
	}

	return PDisplayMenu->getMenuHandle();
}

/** Relase the display options menu screen.
    @ingroup	screen
*/
void DisplayMenuRelease(void)
{
	delete PDisplayMenu;
   PDisplayMenu = NULL;
}