/***************************************************************************

    file                 : mainmenu.cpp
    created              : Sat Mar 18 23:42:38 CET 2000
    copyright            : (C) 2000 by Eric Espie
    email                : torcs@free.fr
    version              : $Id: mainmenu.cpp,v 1.4.2.1 2008/08/16 23:59:54 berniw Exp $

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include <stdio.h>
#include <tgfclient.h>

#include "previewmenu.h"


void *menuHandle = NULL;
tModList *RacemanModLoaded = (tModList*)NULL;
std::string g_strFile;

void LoadMenuScreen();

static void 
endofprog(void * /* dummy */)
{
    GfScrShutdown();
    exit(0);
}

static void
PreviewMenuActivate(void * /* dummy */)
{
    if (RacemanModLoaded) {
	GfModUnloadList(&RacemanModLoaded);
    }
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
		int id = CreateComboboxControl(menuHandle,param,pControlName,NULL);
		//Add some place holder text
		GfuiComboboxAddText(menuHandle,id,"1");
		GfuiComboboxAddText(menuHandle,id,"2");
		return id;
	}
	else if (strType == "scrolllist")
		return CreateScrollListControl(menuHandle,param,pControlName,0,NULL);
	else if (strType == "checkbox")
		return  CreateCheckboxControl(menuHandle,param,pControlName,NULL);

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

    CreateStaticControls(param,menuHandle);
	ShowDynamicControls(param);
    GfuiAddSKey(menuHandle, GFUIK_F1, "reload", NULL, ReloadMenuScreen, NULL);
    GfuiAddKey(menuHandle, 'Q', "Quit", 0, endofprog, NULL);
    GfuiAddKey(menuHandle, 'q', "Quit", 0, endofprog, NULL);
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
