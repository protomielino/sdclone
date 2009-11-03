/***************************************************************************

    file        : fileselect.cpp
    created     : Sun Feb 16 13:09:23 CET 2003
    copyright   : (C) 2003 by Eric Espi�                        
    email       : eric.espie@torcs.org   
    version     : $Id: fileselect.cpp,v 1.2 2003/06/24 21:02:24 torcs Exp $                                  

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
    		Files manipulation screens.
    @ingroup	racemantools
    @author	<a href=mailto:eric.espie@torcs.org>Eric Espie</a>
    @version	$Id: fileselect.cpp,v 1.2 2003/06/24 21:02:24 torcs Exp $
*/


#include <stdlib.h>
#include <stdio.h>
#include <cstring>
#include <sys/stat.h>
#ifdef WIN32
#include <windows.h>
#endif

#include <tgfclient.h>

#include "racescreens.h"

static void		*ScrHandle = NULL;
static int		FileScrollListId;
static tRmFileSelect	*RmFs;
static tFList		*FileList = NULL;
static tFList		*FileSelected;

static void
rmActivate(void * /* dummy */ )
{
}

static void
rmClickOnFile(void * /*dummy*/)
{
    GfuiScrollListGetSelectedElement(ScrHandle, FileScrollListId, (void**)&FileSelected);
}

static void
rmSelect(void * /* dummy */ )
{
    if (FileList) {
	RmFs->select(FileSelected->name);
	GfDirFreeList(FileList, NULL);
	FileList = NULL;
    } else {
	RmFs->select(NULL);
    }
}

static void
rmDeactivate(void * /* dummy */ )
{
    if (FileList) {
	GfDirFreeList(FileList, NULL);
	FileList = NULL;
    }
    GfuiScreenActivate(RmFs->prevScreen);
}


/** File selection
    @param	vs	Pointer on tRmFileSelect structure (cast to void)
    @return	none
*/
void
RmFileSelect(void *vs)
{
    tFList	*FileCur;

    RmFs = (tRmFileSelect*)vs;

    if (ScrHandle) {
	GfuiScreenRelease(ScrHandle);
    }

    // Create screen, load menu XML descriptor and create static controls.
    ScrHandle = GfuiScreenCreateEx(NULL, NULL, rmActivate, NULL, NULL, 1);

    void *menuXMLDescHdle = LoadMenuXML("fileselectmenu.xml");

    CreateStaticControls(menuXMLDescHdle, ScrHandle);

    // Create variable title label.
    int titleId = CreateLabelControl(ScrHandle, menuXMLDescHdle, "titlelabel");
    GfuiLabelSetText(ScrHandle, titleId, RmFs->title);
    
    /* Create and fill-in the Scroll List containing the File list */
    FileScrollListId = CreateScrollListControl(ScrHandle, menuXMLDescHdle, "filescrolllist",
					       NULL, rmClickOnFile);

    FileList = GfDirGetList(RmFs->path);
    if (FileList == NULL) {
	GfuiScreenActivate(RmFs->prevScreen);
	return;
    }
    FileSelected = FileList;
    FileCur = FileList;
    do {
	FileCur = FileCur->next;
	GfuiScrollListInsertElement(ScrHandle, FileScrollListId, FileCur->name, 1000, (void*)FileCur);
    } while (FileCur != FileList);

    // Create Back and Reset buttons.
    CreateButtonControl(ScrHandle, menuXMLDescHdle, "selectbutton", NULL, rmSelect);
    CreateButtonControl(ScrHandle, menuXMLDescHdle, "cancelbutton", NULL, rmDeactivate);

    // Close menu XML descriptor.
    GfParmReleaseHandle(menuXMLDescHdle);
    
    // Register keyboard shortcuts.
    GfuiAddKey(ScrHandle, (unsigned char)27, "Cancel", NULL, rmDeactivate, NULL);
    GfuiMenuDefaultKeysAdd(ScrHandle);

    GfuiScreenActivate(ScrHandle);
}
