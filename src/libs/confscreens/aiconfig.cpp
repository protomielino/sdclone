/***************************************************************************

    file        : aiconfig.cpp
    created     : Sat Dec  26 12:00:00 CET 2009
	copyright   : (C) 2009 The Speed Dreams Team
	web         : speed-dreams.sourceforge.net
    version     : $Id: aiconfig.cpp 2063 2009/12/26 12:00:00 pouillot $

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
    		
    @version	$Id: aiconfig.cpp 2063 2009/12/26 12:00:00 pouillot $
*/

#include <stdio.h>
#include <stdlib.h>
//#include <cstring>
#include <tgfclient.h>
#include <robot.h>
#include <portability.h>

#include "aiconfig.h"

static const char* AIGlobalSkillFilePathName = "config/raceman/extra/skill.xml";

/* Available skill level names and associated values */
static const char *SkillLevels[] = { ROB_VAL_ROOKIE, ROB_VAL_AMATEUR, ROB_VAL_SEMI_PRO, ROB_VAL_PRO };
static const tdble SkillLevelValues[] = { 10.0, 7.0, 3.0, 0.0};
static const int NSkillLevels = sizeof(SkillLevels) / sizeof(SkillLevels[0]);
static int CurSkillLevelIndex = 0;

/* GUI label ids */
static int SkillLevelId;

/* GUI screen handles */
static void	*ScrHandle = NULL;
static void	*PrevHandle = NULL;


/* Load the parameter values from the corresponding parameter file */
static void ReadAICfg(void)
{
	tdble aiSkillValue;
	int i;

	char buf[256];
	snprintf(buf, 256, "%s%s", GetLocalDir(), AIGlobalSkillFilePathName);

	void *paramHandle = GfParmReadFile(buf, GFPARM_RMODE_REREAD | GFPARM_RMODE_CREAT);
	aiSkillValue = GfParmGetNum(paramHandle, "skill", "level", 0, SkillLevelValues[0]);

	CurSkillLevelIndex = NSkillLevels-1; // In case aiSkillValue < 0.
	for (i = 0; i < NSkillLevels; i++) {
		if (aiSkillValue >= SkillLevelValues[i]) {
			CurSkillLevelIndex = i;
			break;
		}
	}

	GfParmReleaseHandle(paramHandle);

	GfuiLabelSetText(ScrHandle, SkillLevelId, SkillLevels[CurSkillLevelIndex]);
}


/* Save the choosen values into the corresponding parameter file */
static void SaveSkillLevel(void * /* dummy */)
{
	char buf[256];
	snprintf(buf, 256, "%s%s", GetLocalDir(), AIGlobalSkillFilePathName);

	void *paramHandle = GfParmReadFile(buf, GFPARM_RMODE_REREAD | GFPARM_RMODE_CREAT);
	GfParmSetNum(paramHandle, "skill", "level", 0, SkillLevelValues[CurSkillLevelIndex]);
	GfParmWriteFile(NULL, paramHandle, "Skill");
	GfParmReleaseHandle(paramHandle);
	
	/* return to previous screen */
	GfuiScreenActivate(PrevHandle);

	return;
}

/* Change the global AI skill level */
static void
ChangeSkillLevel(void *vp)
{
    const int delta = ((long)vp < 0) ? -1 : 1;

	CurSkillLevelIndex = (CurSkillLevelIndex + delta + NSkillLevels) % NSkillLevels;

    GfuiLabelSetText(ScrHandle, SkillLevelId, SkillLevels[CurSkillLevelIndex]);
}


static void onActivate(void * /* dummy */)
{
    ReadAICfg();
}


/* Menu creation */
void *
AIMenuInit(void *prevMenu)
{
    /* screen already created */
    if (ScrHandle) {
        return ScrHandle;
    }
    PrevHandle = prevMenu;

    ScrHandle = GfuiScreenCreateEx((float*)NULL, NULL, onActivate, NULL, (tfuiCallback)NULL, 1);

	void *param = LoadMenuXML("aiconfigmenu.xml");
    CreateStaticControls(param,ScrHandle);

    CreateButtonControl(ScrHandle,param,"skillleftarrow",(void*)-1,ChangeSkillLevel);
    CreateButtonControl(ScrHandle,param,"skillrightarrow",(void*)1,ChangeSkillLevel);

    SkillLevelId = CreateLabelControl(ScrHandle,param,"skilllabel");
    CreateButtonControl(ScrHandle,param,"accept",prevMenu,SaveSkillLevel);
    CreateButtonControl(ScrHandle,param,"cancel",prevMenu,GfuiScreenActivate);


    GfParmReleaseHandle(param);
    
    GfuiAddKey(ScrHandle, 13, "Save", NULL, SaveSkillLevel, NULL);
    GfuiAddKey(ScrHandle, 27, "Cancel Selection", prevMenu, GfuiScreenActivate, NULL);
    GfuiAddSKey(ScrHandle, GLUT_KEY_F1, "Help", ScrHandle, GfuiHelpScreen, NULL);
    GfuiAddSKey(ScrHandle, GLUT_KEY_F12, "Screen-Shot", NULL, GfuiScreenShot, NULL);
    GfuiAddSKey(ScrHandle, GLUT_KEY_LEFT, "Previous Skill Level", (void*)-1, ChangeSkillLevel, NULL);
    GfuiAddSKey(ScrHandle, GLUT_KEY_RIGHT, "Next Skill Level", (void*)+1, ChangeSkillLevel, NULL);

    return ScrHandle;  
}
