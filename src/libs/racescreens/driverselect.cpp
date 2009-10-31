/***************************************************************************
                  driverselect.cpp -- drivers interactive selection                              
                             -------------------                                         
    created              : Mon Aug 16 20:40:44 CEST 1999
    copyright            : (C) 1999 by Eric Espie                         
    email                : torcs@free.fr   
    version              : $Id: driverselect.cpp,v 1.5 2005/02/01 15:55:52 berniw Exp $                                  
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
    		This is a set of tools useful for race managers.
    @ingroup	racemantools
    @author	<a href=mailto:torcs@free.fr>Eric Espie</a>
    @version	$Id: driverselect.cpp,v 1.5 2005/02/01 15:55:52 berniw Exp $
*/


#include <stdlib.h>
#include <stdio.h>
#include <cstring>
#include <sys/stat.h>
#include <string>
#include <list>
#include <vector>
#include <algorithm>
#include <tgfclient.h>
#include <track.h>
#include <car.h>
#include <raceman.h>
#include <robot.h>
#include "racescreens.h"
#include "driver.h"

// Uncomment to re-activate focus managment (what for ?)
//#define FOCUS on

static const float	RedColor[] = {1.0, 0.0, 0.0, 1.0};
static const float	PurpleColor[] = {1.0, 0.0, 1.0, 1.0};

static void		*ScrHandle;
static tRmDrvSelect	*DrvSel;
static int		SelectedScrollList, UnSelectedScrollList;
static int		CarCatEditId;
static int		DrvTypEditId;
#ifdef FOCUS
static int		FocDrvLabelId;
#endif
static int		PickDrvTypLabelId;
static int		PickDrvCarLabelId;
static int		PickDrvCategoryLabelId;
static int		NbTotDrivers;
static int		NbSelectedDrivers;
static int		NbMaxSelectedDrivers;

// Car category list
static const char* AnyCarCat = "--- All ---";
static std::vector<std::string> CarCatList;
static size_t CurCarCatIndex = 0;

// Driver type list
static const char* AnyDrvTyp = "--- All ---";
static std::vector<std::string> DrvTypList;
static size_t CurDrvTypIndex = 0;

// Driver full list
GF_TAILQ_HEAD(DrvListHead, trmdDrvElt);
static tDrvListHead DrvList;


// Local functions.
static void rmdsFreeDrvList(void);
static void rmdsFilterDrvScrollList(const char* carCat, const char* drvTyp);


// Screen activation call-back.
static void
rmdsActivate(void * /* notused */)
{
}

// Screen de-activation call-back.
static void
rmdsDeactivate(void *screen)
{
    rmdsFreeDrvList();    
    GfuiScreenRelease(ScrHandle);
    
    if (screen) {
	GfuiScreenActivate(screen);
    }
}

static void
rmdsChangeCarCat(void *vp)
{
    if (vp)
	CurCarCatIndex = (CurCarCatIndex + 1) % CarCatList.size();
    else
 	CurCarCatIndex = (CurCarCatIndex + CarCatList.size() - 1) % CarCatList.size();

    GfuiLabelSetText(ScrHandle, CarCatEditId, CarCatList[CurCarCatIndex].c_str());

    rmdsFilterDrvScrollList(CarCatList[CurCarCatIndex].c_str(), DrvTypList[CurDrvTypIndex].c_str());
}

static void
rmdsChangeDrvTyp(void *vp)
{
    if (vp)
	CurDrvTypIndex = (CurDrvTypIndex + 1) % DrvTypList.size();
    else
 	CurDrvTypIndex = (CurDrvTypIndex + DrvTypList.size() - 1) % DrvTypList.size();

    GfuiLabelSetText(ScrHandle, DrvTypEditId, DrvTypList[CurDrvTypIndex].c_str());

    rmdsFilterDrvScrollList(CarCatList[CurCarCatIndex].c_str(), DrvTypList[CurDrvTypIndex].c_str());
}

#ifdef FOCUS
static void
rmdsSetFocus(void * /* dummy */)
{
    const char	*name;
    trmdDrvElt	*curDrv;

    name = GfuiScrollListGetSelectedElement(ScrHandle, SelectedScrollList, (void**)&curDrv);
    if (name) {
	GfParmSetStr(DrvSel->param, RM_SECT_DRIVERS, RM_ATTR_FOCUSED, curDrv->moduleName);
	GfParmSetNum(DrvSel->param, RM_SECT_DRIVERS, RM_ATTR_FOCUSEDIDX, (char*)NULL, curDrv->interfaceIndex);
	GfuiLabelSetText(ScrHandle, FocDrvLabelId, curDrv->name);
    }
}
#endif

static void
rmdsAccept(void * /* dummy */)
{
    char         drvSec[256];
    const char	*name;
    trmdDrvElt	*curDrv;
    int		index;
    
    GfParmListClean(DrvSel->param, RM_SECT_DRIVERS);
    name = GfuiScrollListExtractElement(ScrHandle, SelectedScrollList, 0, (void**)&curDrv);
    index = 1;
    while (name) {
	sprintf(drvSec, "%s/%d", RM_SECT_DRIVERS, index);
	GfParmSetNum(DrvSel->param, drvSec, RM_ATTR_IDX, (char*)NULL, curDrv->interfaceIndex);
	GfParmSetStr(DrvSel->param, drvSec, RM_ATTR_MODULE, curDrv->moduleName);
	index++;
	name = GfuiScrollListExtractElement(ScrHandle, SelectedScrollList, 0, (void**)&curDrv);
    }
    rmdsDeactivate(DrvSel->nextScreen);
}

static void
rmdsMove(void *vd)
{
    GfuiScrollListMoveSelectedElement(ScrHandle, SelectedScrollList, (long)vd);
}

static void
rmdsClickOnDriver(void * /* dummy */)
{
    static const unsigned maxBufSize = 64;
    char         buf[maxBufSize];
    const char	*name;
    trmdDrvElt	*curDrv;

    name = GfuiScrollListGetSelectedElement(ScrHandle, SelectedScrollList, (void**)&curDrv);
    if (!name) {
	name = GfuiScrollListGetSelectedElement(ScrHandle, UnSelectedScrollList, (void**)&curDrv);
    }
    
    // Get selected driver infos
    if (name) {
	rmdGetDriverType(curDrv->moduleName, buf, maxBufSize);
	GfuiLabelSetText(ScrHandle, PickDrvTypLabelId, buf);
	GfuiLabelSetText(ScrHandle, PickDrvCarLabelId, GfParmGetName(curDrv->carParmHdle));
	GfuiLabelSetText(ScrHandle, PickDrvCategoryLabelId, GfParmGetStr(curDrv->carParmHdle, SECT_CAR, PRM_CATEGORY, ""));
    }
}

static void
rmdsSelectDeselect(void * /* dummy */ )
{
    const char	*name;
    int		src, dst;
    trmdDrvElt	*curDrv;
    int		sel;

    // If the selected driver is in the Unselected scroll-list,
    // and if the max number of selected drivers has not been reached,
    // remove the driver from the Unselected scroll-list, and add him to the Selected scroll-list
    sel = 0;
    name = 0;
    if (NbSelectedDrivers < NbMaxSelectedDrivers) {
	src = UnSelectedScrollList;
	name = GfuiScrollListExtractSelectedElement(ScrHandle, src, (void**)&curDrv);
	if (name) {
	    dst = SelectedScrollList;
	    GfuiScrollListInsertElement(ScrHandle, dst, name, NbTotDrivers, (void*)curDrv);
	    NbSelectedDrivers++;
	    curDrv->isSelected = 1; // Now selected.
	}
    }

    // Otherwise, if the selected driver is in the Selected scroll-list,
    // remove the driver from the Selected scroll-list, and add him to the Unselected scroll-list
    // (if it matches the Unselected scroll-list filtering criteria)
    if (!name) {
	sel = 1;
	src = SelectedScrollList;
	name = GfuiScrollListExtractSelectedElement(ScrHandle, src, (void**)&curDrv);
	if (name) {
	    if (rmdDriverMatchesFilters(curDrv, CarCatList[CurCarCatIndex].c_str(), DrvTypList[CurDrvTypIndex].c_str(), AnyCarCat, AnyDrvTyp)) {
		dst = UnSelectedScrollList;
		if (curDrv->isHuman) {
		    GfuiScrollListInsertElement(ScrHandle, dst, name, 0, (void*)curDrv);
		} else {
		    GfuiScrollListInsertElement(ScrHandle, dst, name, NbTotDrivers, (void*)curDrv);
		}
	    }
	    NbSelectedDrivers--;
	    curDrv->isSelected = 0; // No more selected.
	} else {
	    return;
	}
    }

    // Focused driver management
    const char *modName = GfParmGetStr(DrvSel->param, RM_SECT_DRIVERS, RM_ATTR_FOCUSED, "");
    int robotIdx = (int)GfParmGetNum(DrvSel->param, RM_SECT_DRIVERS, RM_ATTR_FOCUSEDIDX, (char*)NULL, 0);
    if (sel) {
	modName = GfParmGetStr(DrvSel->param, RM_SECT_DRIVERS, RM_ATTR_FOCUSED, "");
	robotIdx = (int)GfParmGetNum(DrvSel->param, RM_SECT_DRIVERS, RM_ATTR_FOCUSEDIDX, (char*)NULL, 0);
	if (curDrv->interfaceIndex == robotIdx && !strcmp(curDrv->moduleName, modName)) {
	    /* the focused element was deselected : select a new one */
	    name = GfuiScrollListGetElement(ScrHandle, SelectedScrollList, 0, (void**)&curDrv);
	    if (name) {
		GfParmSetStr(DrvSel->param, RM_SECT_DRIVERS, RM_ATTR_FOCUSED, curDrv->moduleName);
		GfParmSetNum(DrvSel->param, RM_SECT_DRIVERS, RM_ATTR_FOCUSEDIDX, (char*)NULL, curDrv->interfaceIndex);
#ifdef FOCUS
		GfuiLabelSetText(ScrHandle, FocDrvLabelId, curDrv->name);
#endif
	    } else {
		GfParmSetStr(DrvSel->param, RM_SECT_DRIVERS, RM_ATTR_FOCUSED, "");
		GfParmSetNum(DrvSel->param, RM_SECT_DRIVERS, RM_ATTR_FOCUSEDIDX, (char*)NULL, 0);
#ifdef FOCUS
		GfuiLabelSetText(ScrHandle, FocDrvLabelId, "");
#endif
	    }
	}
    } else {
	if (strlen(modName) == 0 || curDrv->isHuman) {
	    GfParmSetStr(DrvSel->param, RM_SECT_DRIVERS, RM_ATTR_FOCUSED, curDrv->moduleName);
	    GfParmSetNum(DrvSel->param, RM_SECT_DRIVERS, RM_ATTR_FOCUSEDIDX, (char*)NULL, curDrv->interfaceIndex);
#ifdef FOCUS
	    GfuiLabelSetText(ScrHandle, FocDrvLabelId, curDrv->name);
#endif
	}
    }

    // Update selected driver displayed info
    rmdsClickOnDriver(NULL);
}

static void
rmdsAddKeys(void)
{
    GfuiAddKey(ScrHandle, 27, "Cancel Selection", DrvSel->prevScreen, rmdsDeactivate, NULL);
    GfuiAddKey(ScrHandle, 13, "Accept Selection", NULL, rmdsAccept, NULL);
    GfuiAddSKey(ScrHandle, GLUT_KEY_F1, "Help", ScrHandle, GfuiHelpScreen, NULL);
    GfuiAddSKey(ScrHandle, GLUT_KEY_F12, "Screen-Shot", NULL, GfuiScreenShot, NULL);
    GfuiAddKey(ScrHandle, '-', "Move Up", (void*)-1, rmdsMove, NULL);
    GfuiAddKey(ScrHandle, '+', "Move Down", (void*)1, rmdsMove, NULL);
    GfuiAddKey(ScrHandle, ' ', "Select/Deselect", NULL, rmdsSelectDeselect, NULL);
#ifdef FOCUS
    GfuiAddKey(ScrHandle, 'f', "Set Focus", NULL, rmdsSetFocus, NULL);
#endif    
}

/** Interactive Drivers list selection
    @param	vs	Pointer on tRmDrvSelect structure (cast to void)
    @warning	The race manager params are modified but not saved.
 */
void
RmDriversSelect(void *vs)
{
    tModList	*list;
    tModList	*curmod;
    const unsigned drvTypeMaxSize = 64;
    char	drvType[drvTypeMaxSize];
    const unsigned modNameMaxSize = 64;
    char	modName[modNameMaxSize];
    char        buf[512];
    char        path[256];
    char	*sp;
    const char	*moduleName;
    int		i, index;
    trmdDrvElt	*curDrv;
    int		nDrivers, robotIdx;
    void	*robhdle;
    struct stat st;
    const char	*carName;
    void	*carhdle;
    int		human;
    const char* initCarCat;

    // Initialize drivers selection
    DrvSel = (tRmDrvSelect*)vs;

    // Create screen, background image and title
    ScrHandle = GfuiScreenCreateEx((float*)NULL, NULL, rmdsActivate, NULL, (tfuiCallback)NULL, 1);
    
    void *param = LoadMenuXML("driverselectmenu.xml");
    CreateStaticControls(param,ScrHandle);

    SelectedScrollList = CreateScrollListControl(ScrHandle,param,"selectedscrolllist",NULL,rmdsClickOnDriver);
    UnSelectedScrollList = CreateScrollListControl(ScrHandle,param,"notselectedscrolllist",NULL,rmdsClickOnDriver);

    CreateButtonControl(ScrHandle,param,"carleftarrow",(void*)0,rmdsChangeCarCat);
    CreateButtonControl(ScrHandle,param,"carrightarrow",(void*)1,rmdsChangeCarCat);
    CarCatEditId = CreateLabelControl(ScrHandle,param,"cartext");
    

    // Driver type and associated "combobox" (left arrow, label, right arrow)
    CreateButtonControl(ScrHandle,param,"drvleftarrow",(void*)0,rmdsChangeDrvTyp);
    CreateButtonControl(ScrHandle,param,"drvrightarrow",(void*)1,rmdsChangeDrvTyp);
    DrvTypEditId = CreateLabelControl(ScrHandle,param,"drvtext");

    // Scroll-lists manipulation buttons
    CreateButtonControl(ScrHandle,param,"moveup",(void*)-1,rmdsMove);
    CreateButtonControl(ScrHandle,param,"movedown",(void*)1,rmdsMove);

    CreateButtonControl(ScrHandle,param,"select",(void*)0,rmdsSelectDeselect);

    // Load driver full list, driver type list and driven car category list
    GF_TAILQ_INIT(&DrvList);

    CarCatList.push_back(AnyCarCat);
    DrvTypList.push_back(AnyDrvTyp);

    list = 0;
    sprintf(buf, "%sdrivers", GetLibDir ());
    GfModInfoDir(CAR_IDENT, buf, 1, &list);

    NbTotDrivers = 0;
    curmod = list;
    if (curmod) {
	do {
	    curmod = curmod->next;
	    sp = strrchr(curmod->sopath, '/');
	    sp = sp ? sp+1 : curmod->sopath;
	    strncpy(modName, sp, modNameMaxSize);
	    modName[strlen(modName) - strlen(DLLEXT) - 1] = 0; /* cut .so or .dll */
	    sprintf(buf, "%sdrivers/%s/%s.xml", GetLocalDir(), modName, modName);
	    robhdle = GfParmReadFile(buf, GFPARM_RMODE_STD);
	    if (!robhdle) {
		sprintf(buf, "drivers/%s/%s.xml", modName, modName);
		robhdle = GfParmReadFile(buf, GFPARM_RMODE_STD);
	    }
	    if (!robhdle) {
		GfOut("No driver '%s' selected because no readable '%s.xml' found\n", modName, modName);
		break;
	    }
	    for (i = 0; i < curmod->modInfoSize; i++) {
		if (curmod->modInfo[i].name) {
		    sprintf(path, "%s/%s/%d", ROB_SECT_ROBOTS, ROB_LIST_INDEX, curmod->modInfo[i].index);
		    carName = GfParmGetStr(robhdle, path, ROB_ATTR_CAR, "");
		    human = strcmp(GfParmGetStr(robhdle, path, ROB_ATTR_TYPE, ROB_VAL_ROBOT), ROB_VAL_ROBOT);
		    sprintf(path, "cars/%s/%s.xml", carName, carName);
		    if (!stat(path, &st)) {
			carhdle = GfParmReadFile(path, GFPARM_RMODE_STD);
			if (carhdle) {
			    curDrv = (trmdDrvElt*)calloc(1, sizeof(trmdDrvElt));
			    curDrv->interfaceIndex = curmod->modInfo[i].index;
			    curDrv->moduleName = strdup(modName);
			    curDrv->name = strdup(curmod->modInfo[i].name);
			    curDrv->carParmHdle = carhdle;
			    const char* carCat = GfParmGetStr(carhdle, SECT_CAR, PRM_CATEGORY, "");
			    if (std::find(CarCatList.begin(), CarCatList.end(), carCat) == CarCatList.end()) {
				CarCatList.push_back(carCat);
			    }
			    rmdGetDriverType(modName, drvType, drvTypeMaxSize);
			    if (std::find(DrvTypList.begin(), DrvTypList.end(), drvType) == DrvTypList.end()) {
			      DrvTypList.push_back(drvType);
			    }
			    
			    if (human) {
				curDrv->isHuman = 1;
				GF_TAILQ_INSERT_HEAD(&DrvList, curDrv, link);
			    } else {
				curDrv->isHuman = 0;
				GF_TAILQ_INSERT_TAIL(&DrvList, curDrv, link);
			    }
			    NbTotDrivers++;
			} else {
			    GfOut("Driver '%s' not selected because car '%s' is not readable\n", curmod->modInfo[i].name, carName);
			}
		    } else {
			GfOut("Driver '%s' not selected because car '%s' is not present\n", curmod->modInfo[i].name, carName);
		    }
		}
	    }
	    if (robhdle)
		GfParmReleaseHandle(robhdle);
	} while (curmod != list);
    }

    GfModFreeInfoList(&list);

    // Load selected drivers scroll-list from file
    // and initialize unselected scrolllist car category filter criteria to the car category
    // of the last selected human driver or else to the first selected robot. 
    NbSelectedDrivers = 0;
    NbMaxSelectedDrivers = (int)GfParmGetNum(DrvSel->param, RM_SECT_DRIVERS, RM_ATTR_MAXNUM, NULL, 0);
    nDrivers = GfParmGetEltNb(DrvSel->param, RM_SECT_DRIVERS);
    initCarCat = 0;
    index = 1;
    for (i = 1; i < nDrivers+1; i++) {
	sprintf(path, "%s/%d", RM_SECT_DRIVERS, i);
	moduleName = GfParmGetStr(DrvSel->param, path, RM_ATTR_MODULE, "");
	robotIdx = (int)GfParmGetNum(DrvSel->param, path, RM_ATTR_IDX, (char*)NULL, 0);
	if ((curDrv = GF_TAILQ_FIRST(&DrvList))) {
	    do {
		if (curDrv->interfaceIndex == robotIdx && !strcmp(curDrv->moduleName, moduleName)) {
		    if (NbSelectedDrivers < NbMaxSelectedDrivers) {
			GfuiScrollListInsertElement(ScrHandle, SelectedScrollList, curDrv->name, index, (void*)curDrv);
			if (!initCarCat || !strcmp(moduleName, "human"))
			    initCarCat = GfParmGetStr(curDrv->carParmHdle, SECT_CAR, PRM_CATEGORY, "");
			curDrv->isSelected = index++;
			NbSelectedDrivers++;
		    }
		    break;
		}
	    } while ((curDrv = GF_TAILQ_NEXT(curDrv, link)));
	}
    }

    // Initialize unselected scroll-list filter criteria and fill-in the scroll-list.
    CurDrvTypIndex = 0;
    if (!initCarCat)
	initCarCat = AnyCarCat;
    CurCarCatIndex = std::find(CarCatList.begin(), CarCatList.end(), initCarCat) - CarCatList.begin();
    CurCarCatIndex = CurCarCatIndex % CarCatList.size();
    rmdsFilterDrvScrollList(initCarCat, AnyDrvTyp);
    GfuiLabelSetText(ScrHandle, DrvTypEditId, AnyDrvTyp);
    GfuiLabelSetText(ScrHandle, CarCatEditId, initCarCat);

    // Picked Driver Info
    PickDrvTypLabelId = CreateLabelControl(ScrHandle,param,"pickdrvtyplabel");
    PickDrvCategoryLabelId = CreateLabelControl(ScrHandle,param,"pickdrvcatlabel");
    PickDrvCarLabelId = CreateLabelControl(ScrHandle,param,"pickdrvcarlabel");
    
    // Accept and Cancel buttons
    CreateButtonControl(ScrHandle,param,"accept",NULL,rmdsAccept);
    CreateButtonControl(ScrHandle,param,"cancel",DrvSel->prevScreen,rmdsDeactivate);

    GfParmReleaseHandle(param);

    // Keyboard shortcuts
    GfuiMenuDefaultKeysAdd(ScrHandle);
    rmdsAddKeys();

    GfuiScreenActivate(ScrHandle);
}


static void
rmdsFreeDrvList(void)
{
    trmdDrvElt	*cur;

    CarCatList.clear();
    DrvTypList.clear();

    while ((cur = GF_TAILQ_FIRST(&DrvList))) {
	GF_TAILQ_REMOVE(&DrvList, cur, link);
	free(cur->name);
	free(cur->moduleName);
	GfParmReleaseHandle(cur->carParmHdle);
	free(cur);
    }
}

static void
rmdsFilterDrvScrollList(const char* carCat, const char* drvTyp)
{
    trmdDrvElt	*curDrv;

    // Empty the unselected scroll-list
    while (GfuiScrollListExtractElement(ScrHandle, UnSelectedScrollList, 0, (void**)&curDrv));

    // Fill it with drivers that match the filter criteria
    curDrv = GF_TAILQ_FIRST(&DrvList);
    if (curDrv) {
	do {
	    if (!curDrv->isSelected && rmdDriverMatchesFilters(curDrv, carCat, drvTyp, AnyCarCat, AnyDrvTyp))
		GfuiScrollListInsertElement(ScrHandle, UnSelectedScrollList, curDrv->name, NbTotDrivers, (void*)curDrv);
	} while ((curDrv = GF_TAILQ_NEXT(curDrv, link)));
    }

    // Show first element if any
    GfuiScrollListShowElement(ScrHandle, UnSelectedScrollList, 0);
}
