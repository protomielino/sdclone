/***************************************************************************

    file                 : hostsettingsmenu.h
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

#ifndef _HOSTSETTINGSMENU_H_
#define _HOSTSETTINGSMENU_H_

#include <tgfclient.h>

#include "confscreens.h"


class CONFSCREENS_API HostSettingsMenu : public GfuiMenuScreen
{
public:
	HostSettingsMenu();
	bool Init(void *pPrevMenu);
	void Activate(void* p);
	
protected:
	//callback functions must be static
	static void onActCB(void *p);
	static void onAcceptCB(void *p);
	static void onCancelCB(void *p);
	static void CarControlCB(tComboBoxInfo * pInfo);
	static void CarCollideCB(tComboBoxInfo * pInfo);
	static void humanHostCB(tComboBoxInfo *pChoices);
	static void onPlayerReady(void *p);

protected:
	static  std::string m_strCarCat;
	static bool m_bCollisions;
	static bool m_bHumanHost;

};

#endif /* _HOSTSETTINGSMENU_H_ */ 
