/***************************************************************************

    file                 : loadingscreen.cpp
    created              : Sun Feb 25 00:34:46 /etc/localtime 2001
    copyright            : (C) 2000 by Eric Espie
    email                : eric.espie@torcs.org
    version              : $Id: loadingscreen.cpp,v 1.2 2003/06/24 21:02:24 torcs Exp $

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
    @version	$Id: loadingscreen.cpp,v 1.2 2003/06/24 21:02:24 torcs Exp $
*/

#include <cstring>
#include <stdlib.h>
#ifdef WIN32
#include <windows.h>
#endif
#include <tgfclient.h>
#include <car.h>


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
    int		i;
    int		y;

    if (GfuiScreenIsActive(MenuHandle)) {
	/* Already active */
	return;
    }
    
    if (MenuHandle) {
	GfuiScreenRelease(MenuHandle);
    }
    MenuHandle = GfuiScreenCreateEx(BGColor, NULL, NULL, NULL, rmDeativate, 0);

    GfuiTitleCreate(MenuHandle, title, strlen(title));

    /* create one label for each text line*/
    for (i = 0, y = 400; i < NbTextLines; i++, y -= 16) {
	FGColors[i][0] = FGColors[i][1] = FGColors[i][2] = 1.0;
	FGColors[i][3] = (float)i * 0.0421 + 0.2;
	TextLineIds[i] = GfuiLabelCreateEx(MenuHandle, "", FGColors[i], GFUI_FONT_MEDIUM_C, 60, y, 
									   GFUI_ALIGN_HL_VB, 100);
	if (TextLines[i]) {
	    /* free old text */
	    free(TextLines[i]);
	    TextLines[i] = NULL;
	}
    }

    CurTextLineIdx = 0;
    
    if (bgimg) {
	GfuiScreenAddBgImg(MenuHandle, bgimg);
    }

    GfuiScreenActivate(MenuHandle);
    GfuiDisplay();
}

void
RmShutdownLoadingScreen(void)
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
 
