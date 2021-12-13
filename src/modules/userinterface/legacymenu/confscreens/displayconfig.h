/***************************************************************************

    file                 : displayconfig.h
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

#ifndef _DISPLAYCONFIG_H_
#define _DISPLAYCONFIG_H_

#include <tgfclient.h>
#include <gui.h>

#include "confscreens.h"

// Comment-out to activate max. refresh rate settings.
#define NoMaxRefreshRate 1


class DisplayMenu : public GfuiMenuScreen
{
public:

	DisplayMenu();
	~DisplayMenu();
	
	bool initialize(void* pPreviousMenu);
	
	enum EDisplayMode { eFullScreen = 0, eWindowed = 1, nDisplayModes };
	enum EDisplayType { eNone = 0, e4by3, e16by9, e21by9, nDisplayTypes };
	enum ESpanSplit { eDisabled = 0, eEnabled = 1, nSpanSplits };

	void setDisplayMode(EDisplayMode eMode);
	void setScreenSizeIndex(int nIndex);
	void setMonitorType(EDisplayType eType);
	void setArcRatio(float ratio);
#ifndef NoMaxRefreshRate
	void setMaxRefreshRateIndex(int nIndex);
#endif	
	void storeSettings() const;
	void loadSettings();

	void updateControls();

protected:
	
	void resetColorDepths();
	void resetScreenSizes();

	// Control callback functions (must be static).
	static void onActivate(void *pDisplayMenu);
	static void onChangeScreenSize(tComboBoxInfo *pInfo);
	static void onChangeDisplayMode(tComboBoxInfo *pInfo);
	static void onChangeMonitorType(tComboBoxInfo *pInfo);
	static void onChangeArcRatio(void *pDisplayMenu);
#ifndef NoMaxRefreshRate
	static void onChangeMaxRefreshRate(tComboBoxInfo *pInfo);
#endif	

	static void onAccept(void *pDisplayMenu);
	static void onCancel(void *pDisplayMenu);

private:

	//! Possible screen sizes according to the current color depth and display mode.
	int _nNbScreenSizes;
	tScreenSize* _aScreenSizes;

	//! Currently selected display mode.
	EDisplayMode _eDisplayMode;

	//! Currently selected screen size.
	int _nScreenWidth;
	int _nScreenHeight;

	EDisplayType _eDisplayType;
	float	_fArcRatio;
	float	_fBezelComp;
	float	_fScreenDist;


#ifndef NoMaxRefreshRate
	//! Currently selected max. refresh rate (Hz).
	int _nMaxRefreshRate;
#endif	
};

extern void* DisplayMenuInit(void* pPreviousMenu);
extern void DisplayMenuRelease(void);

#endif //_DISPLAYCONFIG_H_
