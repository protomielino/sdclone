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

#include "displayconfig.h"


// Some consts.
static const char* ADisplayModes[DisplayMenu::nDisplayModes] = { "Full-screen", "Windowed" };
static const char* AVideoDetectModes[DisplayMenu::nVideoDetectModes] = { "Auto", "Manual" };
static const char* AVideoInitModes[DisplayMenu::nVideoInitModes] = { "Compatible", "Best possible" };
#ifndef NoMaxRefreshRate
static const int AMaxRefreshRates[] = { 0, 30, 40, 50, 60, 75, 85, 100, 120, 150, 200 };
static const int NMaxRefreshRates = sizeof(AMaxRefreshRates) / sizeof(AMaxRefreshRates[0]);
#endif	

// The unique DisplayMenu instance.
static DisplayMenu* PDisplayMenu = 0;


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

void DisplayMenu::onChangeColorDepth(tComboBoxInfo *pInfo)
{
 	// Get the DisplayMenu instance from call-back user data.
	DisplayMenu* pMenu = static_cast<DisplayMenu*>(pInfo->userData);

	pMenu->setColorDepthIndex(pInfo->nPos);
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

void DisplayMenu::onChangeVideoDetectMode(tComboBoxInfo *pInfo)
{
 	// Get the DisplayMenu instance from call-back user data.
	DisplayMenu* pMenu = static_cast<DisplayMenu*>(pInfo->userData);

	pMenu->setVideoDetectMode((EVideoDetectMode)pInfo->nPos);
}

void DisplayMenu::onChangeVideoInitMode(tComboBoxInfo *pInfo)
{
 	// Get the DisplayMenu instance from call-back user data.
	DisplayMenu* pMenu = static_cast<DisplayMenu*>(pInfo->userData);

	pMenu->setVideoInitMode((EVideoInitMode)pInfo->nPos);
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

    // Release screen allocated resources.
    GfScrShutdown();

    // Restart the game.
	GfRestart(GfuiMouseIsHWPresent(), GfglIsMultiTexturingEnabled());

	// TODO: A nice system to get back to previous display settings if the chosen ones
	//       keep the game from really restarting (ex: unsupported full screen size) ?
}

void DisplayMenu::onCancel(void *pDisplayMenu)
{
	// Get the DisplayMenu instance from call-back user data.
	const DisplayMenu* pMenu = static_cast<DisplayMenu*>(pDisplayMenu);

	// Back to previous screen.
	GfuiScreenActivate(pMenu->GetPreviousMenuHandle());
}

void DisplayMenu::updateControls()
{
	resetColorDepths();
	
	int nControlId = GetDynamicControlId("DisplayModeCombo");
	GfuiComboboxSetSelectedIndex(GetMenuHandle(), nControlId, _eDisplayMode);
	
	resetScreenSizes();
	
	nControlId = GetDynamicControlId("VideoDetectModeCombo");
	GfuiComboboxSetSelectedIndex(GetMenuHandle(), nControlId, _eVideoDetectMode);

	nControlId = GetDynamicControlId("VideoInitModeCombo");
	GfuiComboboxSetSelectedIndex(GetMenuHandle(), nControlId, _eVideoInitMode);

#ifndef NoMaxRefreshRate
	nControlId = GetDynamicControlId("MaxRefreshRateCombo");
	int nMaxRefRateIndex = 0; // Defaults to None.
	for (int nMaxRefRateInd = 0; nMaxRefRateInd < NMaxRefreshRates; nMaxRefRateInd++)
		if (_nMaxRefreshRate <= AMaxRefreshRates[nMaxRefRateInd])
		{
			nMaxRefRateIndex = nMaxRefRateInd;
			break;
		}
	GfuiComboboxSetSelectedIndex(GetMenuHandle(), nControlId, nMaxRefRateIndex);
#endif	
}

void DisplayMenu::loadSettings()
{
	// Load screen config params file.
	std::ostringstream ossConfFile;
	ossConfFile << GetLocalDir() << GFSCR_CONF_FILE;
	void* hScrConfParams =
		GfParmReadFile(ossConfFile.str().c_str(), GFPARM_RMODE_STD | GFPARM_RMODE_CREAT);
	
	// Video detection mode : Auto or Manual.
	const char *pszVideoDetectMode =
		GfParmGetStr(hScrConfParams, GFSCR_SECT_PROP, GFSCR_ATT_VDETECT, GFSCR_VAL_VDETECT_AUTO);
	_eVideoDetectMode =
		strcmp(pszVideoDetectMode, GFSCR_VAL_VDETECT_AUTO) ? eManual : eAuto;

	// Color depth (bits per pixel, alpha included).
	_nColorDepth = (int)GfParmGetNum(hScrConfParams, GFSCR_SECT_PROP, GFSCR_ATT_BPP, NULL, 32);

	// Display mode : Full-screen or Windowed.
	const char *pszFullScreen =
		GfParmGetStr(hScrConfParams, GFSCR_SECT_PROP, GFSCR_ATT_FSCR, GFSCR_VAL_NO);
	_eDisplayMode = strcmp(pszFullScreen, GFSCR_VAL_YES) ? eWindowed : eFullScreen;

	// Screen / window size.
	_nScreenWidth = (int)GfParmGetNum(hScrConfParams, GFSCR_SECT_PROP, GFSCR_ATT_X, NULL, 800);
	_nScreenHeight = (int)GfParmGetNum(hScrConfParams, GFSCR_SECT_PROP, GFSCR_ATT_Y, NULL, 600);
	
	// Video initialization mode : Compatible or Best.
	const char *pszVideoInitMode =
		GfParmGetStr(hScrConfParams, GFSCR_SECT_PROP, GFSCR_ATT_VINIT, GFSCR_VAL_VINIT_COMPATIBLE);
	_eVideoInitMode =
		strcmp(pszVideoInitMode, GFSCR_VAL_VINIT_COMPATIBLE) ? eBestPossible : eCompatible;

#ifndef NoMaxRefreshRate
	// Max. refresh rate (Hz).
	_nMaxRefreshRate =
		(int)GfParmGetNum(hScrConfParams, GFSCR_SECT_PROP, GFSCR_ATT_MAXREFRESH, NULL, 0);
#endif	
	
	// Release screen config params file.
	GfParmReleaseHandle(hScrConfParams);
}

// Save graphical settings to XML file.
void DisplayMenu::storeSettings() const
{
	// Load screen config params file.
	std::ostringstream ossConfFile;
	ossConfFile << GetLocalDir() << GFSCR_CONF_FILE;
	void* hScrConfParams =
		GfParmReadFile(ossConfFile.str().c_str(), GFPARM_RMODE_STD | GFPARM_RMODE_CREAT);

	// Write new settings.
	GfParmSetNum(hScrConfParams, GFSCR_SECT_PROP, GFSCR_ATT_X, (char*)NULL, _nScreenWidth);
	GfParmSetNum(hScrConfParams, GFSCR_SECT_PROP, GFSCR_ATT_Y, (char*)NULL, _nScreenHeight);
	GfParmSetNum(hScrConfParams, GFSCR_SECT_PROP, GFSCR_ATT_WIN_X, (char*)NULL, _nScreenWidth);
	GfParmSetNum(hScrConfParams, GFSCR_SECT_PROP, GFSCR_ATT_WIN_Y, (char*)NULL, _nScreenHeight);
	GfParmSetNum(hScrConfParams, GFSCR_SECT_PROP, GFSCR_ATT_BPP, (char*)NULL, _nColorDepth);
#ifndef NoMaxRefreshRate
	GfParmSetNum(hScrConfParams, GFSCR_SECT_PROP, GFSCR_ATT_MAXREFRESH, (char*)NULL, _nMaxRefreshRate);
#endif	

	const char* pszVDetectMode =
		(_eVideoDetectMode == eAuto) ? GFSCR_VAL_VDETECT_AUTO : GFSCR_VAL_VDETECT_MANUAL;
	GfParmSetStr(hScrConfParams, GFSCR_SECT_PROP, GFSCR_ATT_VDETECT, pszVDetectMode);

	const char* pszVInitMode =
		(_eVideoInitMode == eCompatible) ? GFSCR_VAL_VINIT_COMPATIBLE : GFSCR_VAL_VINIT_BEST;
	GfParmSetStr(hScrConfParams, GFSCR_SECT_PROP, GFSCR_ATT_VINIT, pszVInitMode);

	const char* pszDisplMode =
		(_eDisplayMode == eFullScreen) ? GFSCR_VAL_YES : GFSCR_VAL_NO;
	GfParmSetStr(hScrConfParams, GFSCR_SECT_PROP, GFSCR_ATT_FSCR, pszDisplMode);
	
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

void DisplayMenu::setColorDepthIndex(int nIndex)
{
	if (_nColorDepth != _aColorDepths[nIndex])
	{
		_nColorDepth = _aColorDepths[nIndex];
		
		resetScreenSizes();
	}
}

void DisplayMenu::resetColorDepths()
{
	// Determine possible / supported color depths
	// (possible, or only supported ones, whether auto. or manual video features detection mode).
	int nDefColorDepths;
	int* aDefColorDepths = GfScrGetDefaultColorDepths(&nDefColorDepths);
	if (_aColorDepths && _aColorDepths != aDefColorDepths)
		free(_aColorDepths);
	if (_eVideoDetectMode == eAuto)
		_aColorDepths = GfScrGetSupportedColorDepths(&_nNbColorDepths);
	else
	{
		_aColorDepths = aDefColorDepths;
		_nNbColorDepths = nDefColorDepths;
	}
	
	// Update combo-box with new possible color depths.
	const int nComboId = GetDynamicControlId("ColorDepthCombo");
	GfuiComboboxClear(GetMenuHandle(), nComboId);
	std::ostringstream ossColorDepth;
	for (int nColorDepthInd = 0; nColorDepthInd < _nNbColorDepths; nColorDepthInd++)
	{
		ossColorDepth.str("");
		ossColorDepth << _aColorDepths[nColorDepthInd];
		GfuiComboboxAddText(GetMenuHandle(), nComboId, ossColorDepth.str().c_str());
	}
	
	// Try and find the closest color depth to the current choice in the new list.
	int nColorDepthIndex = _nNbColorDepths-1; // Defaults to the max possible value.
	for (int nDepthInd = 0; nDepthInd < _nNbColorDepths; nDepthInd++)
		if (_nColorDepth == _aColorDepths[nDepthInd])
		{
			nColorDepthIndex = nDepthInd;
			break;
		}

	// Store the new current color depth (in case it changed).
	_nColorDepth = _aColorDepths[nColorDepthIndex];
	
	// Select the found one in the combo-box.
	GfuiComboboxSetSelectedIndex(GetMenuHandle(), nComboId, nColorDepthIndex);
}

void DisplayMenu::resetScreenSizes()
{
	// Determine possible / supported screen sizes for the current display mode, color depth,
	// and video features detection mode.
	int nDefScreenSizes;
	tScreenSize* aDefScreenSizes = GfScrGetDefaultSizes(&nDefScreenSizes);
	if (_aScreenSizes && _aScreenSizes != aDefScreenSizes)
		free(_aScreenSizes);
	if (_eVideoDetectMode == eAuto)
		_aScreenSizes =
			GfScrGetSupportedSizes(_nColorDepth, _eDisplayMode == eFullScreen, &_nNbScreenSizes);
	if (_eVideoDetectMode == eManual || _aScreenSizes == (tScreenSize*)-1 || _aScreenSizes == 0)
		_aScreenSizes = aDefScreenSizes;
		_nNbScreenSizes = nDefScreenSizes;

	// Update combo-box with new possible sizes.
	const int nComboId = GetDynamicControlId("ScreenSizeCombo");
	GfuiComboboxClear(GetMenuHandle(), nComboId);
	std::ostringstream ossSize;
	for (int nSizeIndex = 0; nSizeIndex < _nNbScreenSizes; nSizeIndex++)
	{
		ossSize.str("");
		ossSize << _aScreenSizes[nSizeIndex].width << " x " << _aScreenSizes[nSizeIndex].height;
		GfuiComboboxAddText(GetMenuHandle(), nComboId, ossSize.str().c_str());
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
	GfuiComboboxSetSelectedIndex(GetMenuHandle(), nComboId, nScreenSizeIndex);
}

void DisplayMenu::setScreenSizeIndex(int nIndex)
{
	_nScreenWidth = _aScreenSizes[nIndex].width;
	_nScreenHeight = _aScreenSizes[nIndex].height;
}

void DisplayMenu::setVideoDetectMode(EVideoDetectMode eMode)
{
	if (_eVideoDetectMode != eMode)
	{
		_eVideoDetectMode = eMode;
		
		resetColorDepths();
		resetScreenSizes();
	}
}

void DisplayMenu::setVideoInitMode(EVideoInitMode eMode)
{
	_eVideoInitMode = eMode;
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

	_nNbColorDepths = -1;
	_aColorDepths = 0;

	_nColorDepth = 24;
	_eDisplayMode = eWindowed;
	_nScreenWidth = 800;
	_nScreenHeight = 600;
	_eVideoDetectMode = eAuto;
	_eVideoInitMode = eCompatible;
#ifndef NoMaxRefreshRate
	_nMaxRefreshRate = 0;
#endif	
}

bool DisplayMenu::initialize(void *pPreviousMenu)
{
	// Save the menu to return to.
	SetPreviousMenuHandle(pPreviousMenu);

	// Create the menu and all its controls.
	CreateMenuEx(NULL, this, onActivate, NULL, (tfuiCallback)NULL, 1);

    OpenXMLDescriptor();
    
    CreateStaticControls();
    
	CreateComboboxControl("ScreenSizeCombo", this, onChangeScreenSize);

	const int nColorDepthComboId =
		CreateComboboxControl("ColorDepthCombo", this, onChangeColorDepth);

	const int nDisplayModeComboId =
		CreateComboboxControl("DisplayModeCombo", this, onChangeDisplayMode);

#ifndef NoMaxRefreshRate
	const int nMaxRefRateComboId =
		CreateComboboxControl("MaxRefreshRateCombo", this, onChangeMaxRefreshRate);
#endif	

	const int nVideoDetectComboId =
		CreateComboboxControl("VideoDetectModeCombo", this, onChangeVideoDetectMode);

	const int nVideoInitComboId =
		CreateComboboxControl("VideoInitModeCombo", this, onChangeVideoInitMode);

	CreateButtonControl("ApplyButton", this, onAccept);
	CreateButtonControl("CancelButton", this, onCancel);

	AddShortcut(GFUIK_RETURN, "Apply", this, onAccept, 0);
	AddShortcut(GFUIK_ESCAPE, "Cancel", this, onCancel, 0);
    // TODO Keyboard shortcuts: Add support for shortcuts in GfuiCombobox ?
	//AddShortcut(GFUIK_LEFT, "Previous Resolution", this, onChangeScreenSize, 0);
	//AddShortcut(GFUIK_RIGHT, "Next Resolution", this, onChangeScreenSize, 0);
	AddShortcut(GFUIK_F1, "Help", GetMenuHandle(), GfuiHelpScreen, 0);
	AddShortcut(GFUIK_F12, "Screen-Shot", 0, GfuiScreenShot, 0);

    CloseXMLDescriptor();

	// Load constant value lists in combo-boxes.
	// 1) Color depths combo : not constant, as depends on selected video detection mode.
	
	// 2) Display modes combo.
	for (int nDispModeInd = 0; nDispModeInd < nDisplayModes; nDispModeInd++)
		GfuiComboboxAddText(GetMenuHandle(), nDisplayModeComboId, ADisplayModes[nDispModeInd]);

	// 3) Video detection modes combo.
	for (int nVDetectModeInd = 0; nVDetectModeInd < nVideoDetectModes; nVDetectModeInd++)
		GfuiComboboxAddText(GetMenuHandle(), nVideoDetectComboId, AVideoDetectModes[nVDetectModeInd]);

	// 4) Video initialization modes combo.
	for (int nVInitModeInd = 0; nVInitModeInd < nVideoInitModes; nVInitModeInd++)
		GfuiComboboxAddText(GetMenuHandle(), nVideoInitComboId, AVideoInitModes[nVInitModeInd]);

	// 5) Screen sizes combo : not constant, as depends on selected color depth and display mode.

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
		GfuiComboboxAddText(GetMenuHandle(), nMaxRefRateComboId, ossMaxRefRate.str().c_str());
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

	return PDisplayMenu->GetMenuHandle();
}


	
