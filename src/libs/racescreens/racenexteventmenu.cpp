/***************************************************************************

    file        : racenexteventmenu.cpp
    created     : Fri Jan  3 22:24:41 CET 2003
    copyright   : (C) 2003 by Eric Espie                        
    email       : eric.espie@torcs.org   
    version     : $Id$

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/** @file   
    		The Next Event menu (where one sees which track the next event is run on)
    @author	<a href=mailto:eric.espie@torcs.org>Eric Espie</a>
    @version	$Id$
*/

#include <cstdlib>
#include <cstdio>

#include <portability.h>

#include <raceman.h>
#include <tgfclient.h>

#include "racescreens.h"


// New track menu.
static void	*ScrHandle = NULL;

static void
reStateManage(void * /* dummy */)
{
	RmRaceEngine().updateState();
}

int
ReNextEventMenu(void)
{
	char buf[128];

	tRmInfo* reInfo = RmRaceEngine().data();
	void	*params = reInfo->params;
	void	*results = reInfo->results;
	int		raceNumber;
	int		 xx;

	if (ScrHandle) {
		GfuiScreenRelease(ScrHandle);
	}

	// Create screen, load menu XML descriptor and create static controls.
	ScrHandle = GfuiScreenCreateEx(NULL, 
								  NULL, (tfuiCallback)NULL, 
								  NULL, (tfuiCallback)NULL, 
								  1);
	void *menuXMLDescHdle = LoadMenuXML("racenexteventmenu.xml");
	CreateStaticControls(menuXMLDescHdle, ScrHandle);

	// Create background image from race params.
	const char* pszBGImg = GfParmGetStr(params, RM_SECT_HEADER, RM_ATTR_BGIMG, 0);
	if (pszBGImg) {
		GfuiScreenAddBgImg(ScrHandle, pszBGImg);
	}

	// Create variable title label from race params.
	int titleId = CreateLabelControl(ScrHandle, menuXMLDescHdle, "titlelabel");
	GfuiLabelSetText(ScrHandle, titleId, reInfo->_reName);

	// Calculate which race of the series this is
	raceNumber = 1;
	for (xx = 1; xx < (int)GfParmGetNum(results, RE_SECT_CURRENT, RE_ATTR_CUR_TRACK, NULL, 1); ++xx) 
	{
		snprintf(buf, sizeof(buf), "%s/%d", RM_SECT_TRACKS, xx);
		if (!strcmp( GfParmGetStr(reInfo->params, buf, RM_ATTR_NAME, "free"), "free") == 0)
			++raceNumber;
	}

	// Create variable subtitle label from race params.
	snprintf(buf, sizeof(buf), "Race Day #%d/%d at %s",
			 raceNumber,
			 (int)GfParmGetNum(params, RM_SECT_TRACKS, RM_ATTR_NUMBER, NULL, -1 ) >= 0 ?
			 (int)GfParmGetNum(params, RM_SECT_TRACKS, RM_ATTR_NUMBER, NULL, -1 ) :
			 GfParmGetEltNb(params, RM_SECT_TRACKS), 
			 reInfo->track->name);
	int subTitleId = CreateLabelControl(ScrHandle, menuXMLDescHdle, "subtitlelabel");
	GfuiLabelSetText(ScrHandle, subTitleId, buf);

	// Create Start and Abandon buttons.
	CreateButtonControl(ScrHandle, menuXMLDescHdle, "startbutton", NULL, reStateManage);
	CreateButtonControl(ScrHandle, menuXMLDescHdle, "abandonbutton", reInfo->_reMenuScreen, GfuiScreenActivate);

	// Close menu XML descriptor.
	GfParmReleaseHandle(menuXMLDescHdle);
	
	// Register keyboard shortcuts.
	GfuiMenuDefaultKeysAdd(ScrHandle);
	GfuiAddKey(ScrHandle, GFUIK_RETURN, "Start Event", NULL, reStateManage, NULL);
	GfuiAddKey(ScrHandle, GFUIK_ESCAPE, "Abandon", reInfo->_reMenuScreen, GfuiScreenActivate, NULL);

	// Activate screen.
	GfuiScreenActivate(ScrHandle);

	return RM_ASYNC | RM_NEXT_STEP;
}

