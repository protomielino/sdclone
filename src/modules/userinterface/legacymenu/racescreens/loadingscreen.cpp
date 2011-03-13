/***************************************************************************

    file                 : loadingscreen.cpp
    created              : Sun Feb 25 00:34:46 /etc/localtime 2001
    copyright            : (C) 2000 by Eric Espie
    email                : eric.espie@torcs.org
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

/** @file
    		This is a Loading... screen management.
    @ingroup	racemantools		
    @author	<a href=mailto:eric.espie@torcs.org>Eric Espie</a>
    @version	$Id$
*/

#include <cstring>
#include <cstdlib>

#include <tgfclient.h>

#include <car.h>

#include "racescreens.h"


static const	unsigned NbTextLines = 23;

static float	BGColor[4] = {0.0, 0.0, 0.0, 0.0};
static float	FGColors[NbTextLines][4];

static void		*MenuHandle = NULL;
static int		 TextLineIds[NbTextLines];
static char		*TextLines[NbTextLines] = {0};
static int		 CurTextLineIdx;


static void
rmDeativate(void * /* dummy */)
{
}


/** 
    @ingroup	racemantools
    @param	title	Screen title.
    @param	bgimg	Optional background image (0 for no img).
    @return	None.
*/
void
RmLoadingScreenStart(const char *title, const char *bgimg)
{
    int unsigned i;
    int		y;

    if (GfuiScreenIsActive(MenuHandle)) {
	/* Already active */
	return;
    }
    
    if (MenuHandle) {
	GfuiScreenRelease(MenuHandle);
    }

    // Create screen, load menu XML descriptor and create static controls.
    MenuHandle = GfuiScreenCreateEx(BGColor, NULL, NULL, NULL, rmDeativate, 0);

    void *menuXMLDescHdle = LoadMenuXML("loadingscreen.xml");

    CreateStaticControls(menuXMLDescHdle, MenuHandle);

    // Create variable title label.
    int titleId = CreateLabelControl(MenuHandle, menuXMLDescHdle, "titlelabel");
    GfuiLabelSetText(MenuHandle, titleId, title);
    
    // Create one label for each text line (TODO: Get layout constants from XML when available)
    for (i = 0, y = 400; i < NbTextLines; i++, y -= 16) {
	FGColors[i][0] = FGColors[i][1] = FGColors[i][2] = 1.0;
	FGColors[i][3] = (float)i * 0.0421 + 0.2;
	TextLineIds[i] = GfuiLabelCreate(MenuHandle, "", GFUI_FONT_MEDIUM_C, 60, y, 
									 GFUI_ALIGN_HL_VB, 100, FGColors[i]);
	if (TextLines[i]) {
	    /* free old text */
	    free(TextLines[i]);
	    TextLines[i] = NULL;
	}
    }

    CurTextLineIdx = 0;
    
    // Add background image.
    if (bgimg) {
	GfuiScreenAddBgImg(MenuHandle, bgimg);
    }

    // Close menu XML descriptor.
    GfParmReleaseHandle(menuXMLDescHdle);
    
    // Display screen.
    GfuiScreenActivate(MenuHandle);
    GfuiDisplay();
}

void
RmLoadingScreenShutdown(void)
{
    if (MenuHandle) {
	GfuiScreenRelease(MenuHandle);
	MenuHandle = 0;
    }
}


/** 
    @ingroup	racemantools
    @param	text	Text to display.
    @return	None.
*/
void
RmLoadingScreenSetText(const char *text)
{
    int		i, j;
    
    GfOut("%s\n", text);
    
    if (MenuHandle) {
	if (TextLines[CurTextLineIdx]) {
	    free(TextLines[CurTextLineIdx]);
	}
	if (text) {
	    TextLines[CurTextLineIdx] = strdup(text);
	    CurTextLineIdx = (CurTextLineIdx + 1) % NbTextLines;
	}
	
	i = CurTextLineIdx;
	j = 0;
	do {
	    if (TextLines[i]) {
		GfuiLabelSetText(MenuHandle, TextLineIds[j], TextLines[i]);
	    }
	    j++;
	    i = (i + 1) % NbTextLines;
	} while (i != CurTextLineIdx);
	
	GfuiDisplay();
    }
}
 
