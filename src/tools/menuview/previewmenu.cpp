/***************************************************************************

    file                 : mainmenu.cpp
    created              : Sat Mar 18 23:42:38 CET 2000
    copyright            : (C) 2000 by Eric Espie
    email                : torcs@free.fr
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


#include <cstdio>

#include <tgfclient.h>

#include "previewmenu.h"


void *menuHandle = NULL;

std::string g_strFile;

void LoadMenuScreen();

static void 
onQuit(void * /* dummy */)
{
    GfuiApp().eventLoop().postQuit();
}

static void
PreviewMenuActivate(void * /* dummy */)
{
}

int ReadControl(void *param,std::string strType,const char *pControlName)
{
	if ((strType == "textbutton")||(strType =="imagebutton"))
		return CreateButtonControl(menuHandle,param,pControlName,0,NULL);
	else if (strType == "editbox")
		return CreateEditControl(menuHandle,param,pControlName,0,NULL,NULL);
	else if (strType == "label")
		return CreateLabelControl(menuHandle,param,pControlName);
	else if (strType == "staticimage")
		return CreateStaticImageControl(menuHandle,param,pControlName);
	else if (strType == "combobox")
	{
		int id = CreateComboboxControl(menuHandle,param,pControlName,0,NULL);
		return id;
	}
	else if (strType == "scrolllist")
		return CreateScrollListControl(menuHandle,param,pControlName,0,NULL);
	else if (strType == "checkbox")
		return  CreateCheckboxControl(menuHandle,param,pControlName,0,NULL);
	else if (strType == "progressbar")
		return  CreateProgressbarControl(menuHandle,param,pControlName);

	return -1;
}

void ShowDynamicControls(void *param)
{

	if (GfParmListSeekFirst(param,"dynamiccontrols") == 0)
		{
			do
			{
				std::string strControlName = GfParmListGetCurEltName (param,"dynamiccontrols");
				std::string strType = GfParmGetCurStr(param,"dynamiccontrols","type","");
				ReadControl(param,strType,strControlName.c_str());
			} while (GfParmListSeekNext(param,"dynamiccontrols") == 0);
		}


}

void
ReloadMenuScreen(void *)
{
	GfuiScreenDeactivate();
	LoadMenuScreen();
	PreviewMenuRun();
}

void
LoadMenuScreen()
{
    menuHandle = GfuiScreenCreateEx((float*)NULL, 
				    NULL, PreviewMenuActivate, 
				    NULL, (tfuiCallback)NULL, 
				    1);

	void *param = GfParmReadFile(g_strFile.c_str(), GFPARM_RMODE_REREAD);
	if (param == NULL)
		param = GfParmReadFileLocal(g_strFile.c_str(), GFPARM_RMODE_REREAD);

    CreateStaticControls(param,menuHandle);
	ShowDynamicControls(param);
    GfuiAddKey(menuHandle, GFUIK_F5, "reload", NULL, ReloadMenuScreen, NULL);
    GfuiAddKey(menuHandle, 'Q', "Quit", 0, onQuit, NULL);
    GfuiAddKey(menuHandle, 'q', "Quit", 0, onQuit, NULL);
    GfParmReleaseHandle(param);

}

int
PreviewMenuInit(const char* pFile)
{
	g_strFile = pFile;
	LoadMenuScreen();
    return 0;
}
/*
 * Function
 *	
 *
 * Description
 *	
 *
 * Parameters
 *	
 *
 * Return
 *	
 *
 * Remarks
 *	
 */
int
PreviewMenuRun(void)
{
    GfuiScreenActivate(menuHandle);
    return 0;
}
