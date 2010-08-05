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


#include <sys/stat.h>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <algorithm>

#include <tgfclient.h>
#include <track.h>
#include <car.h>
#include <raceman.h>
#include <robot.h>

#include "racescreens.h"
#include "driver.h"
#include "carselect.h"


// Uncomment to re-activate focus managment (what for ?)
//#define FOCUS on

// Handle for the screen.
static void		*ScrHandle;

// Information about the current race type.
static tRmDriverSelect	*MenuData;

// GUI control Ids.
static int		CompetitorsScrollListId, CandidatesScrollListId;
static int		SelectButtonId, DeselectButtonId;
static int		CarCategoryEditId;
static int		DriverTypeEditId;
static int		SkinEditId;
static int		SkinLeftButtonId, SkinRightButtonId;
static int		CarImageId;
#ifdef FOCUS
static int		FocusedDriverLabelId;
#endif
static int		PickedDriverTypeLabelId;
static int		PickedDriverCarLabelId;
static int		PickedDriverCarCategoryLabelId;

static int      NextButtonId;
static int      ChangeCarButtonId;

// The car selection menu.
static RmCarSelectMenu CarSelectMenu;

// Information on drivers.
static int		NbTotDrivers;
static int		NbSelectedDrivers;
static int		NbMaxSelectedDrivers;

// Car categories
static const char* AnyCarCategory = "--- All ---";
static std::vector<std::string> VecCarCategories;
static size_t CurCarCategoryIndex = 0;

// Driver types
static const char* AnyDriverType = "--- All ---";
static std::vector<std::string> VecDriverTypes;
static size_t CurDriverTypeIndex = 0;

// Skin names and associated preview files 
static std::vector<std::string> VecSkinNames;
static std::map<std::string, std::string> MapSkins2PreviewFiles; // Key = skin name.
static size_t CurSkinIndex = 0;

// Driver full list
GF_TAILQ_HEAD(DriverListHead, trmdDrvElt);
static tDriverListHead DriverList;

// Local functions.
static void rmdsCleanup(void);
static void rmdsFilterDriverScrollList(const char* carCat, const char* driverType);
static trmdDrvElt* rmdsGetHighlightedDriver();
static bool rmdsIsAnyCompetitorHighlighted();
static void rmdsClickOnDriver(void * /* dummy */);


// Screen activation call-back.
static void
rmdsActivate(void * /* notused */)
{
    // Update selected driver displayed info
    rmdsClickOnDriver(NULL);
}

// Screen de-activation call-back.
static void
rmdsDeactivate(void *nextScreenHdle)
{
    rmdsCleanup();    
    GfuiScreenRelease(ScrHandle);
    
    if (nextScreenHdle)
	GfuiScreenActivate(nextScreenHdle);
}

static void
rmdsChangeCarCategory(void *vp)
{
	CurCarCategoryIndex =
		(CurCarCategoryIndex + VecCarCategories.size() + (int)(long)vp) % VecCarCategories.size();

    GfuiLabelSetText(ScrHandle, CarCategoryEditId, VecCarCategories[CurCarCategoryIndex].c_str());

    rmdsFilterDriverScrollList(VecCarCategories[CurCarCategoryIndex].c_str(),
							   VecDriverTypes[CurDriverTypeIndex].c_str());

	if (rmdsIsAnyCompetitorHighlighted())
		GfuiEnable(ScrHandle, ChangeCarButtonId, GFUI_ENABLE);
}

static void
rmdsChangeDriverType(void *vp)
{
 	CurDriverTypeIndex =
		(CurDriverTypeIndex + VecDriverTypes.size() + (int)(long)vp) % VecDriverTypes.size();

    GfuiLabelSetText(ScrHandle, DriverTypeEditId, VecDriverTypes[CurDriverTypeIndex].c_str());

    rmdsFilterDriverScrollList(VecCarCategories[CurCarCategoryIndex].c_str(),
							   VecDriverTypes[CurDriverTypeIndex].c_str());

	if (rmdsIsAnyCompetitorHighlighted())
		GfuiEnable(ScrHandle, ChangeCarButtonId, GFUI_ENABLE);
}

static void
rmdsChangeSkin(void *vp)
{
	if (VecSkinNames.empty())
		return;

	// Update GUI.
 	CurSkinIndex = (CurSkinIndex + VecSkinNames.size() + (int)(long)vp) % VecSkinNames.size();

	const char* pszCurSkinName = VecSkinNames[CurSkinIndex].c_str();
    GfuiLabelSetText(ScrHandle, SkinEditId, pszCurSkinName);

	// Load associated preview image (or "no preview panel" if none available).
	struct stat st;
	if (!stat(MapSkins2PreviewFiles[pszCurSkinName].c_str(), &st))
		GfuiStaticImageSet(ScrHandle, CarImageId, MapSkins2PreviewFiles[pszCurSkinName].c_str());
	else
		GfuiStaticImageSet(ScrHandle, CarImageId, "data/img/nocarpreview.png");

	// Update highlighted driver skin.
	trmdDrvElt *pDriver = rmdsGetHighlightedDriver();
	if (pDriver)
	{
		if (pDriver->skinName)
			free(pDriver->skinName);
		pDriver->skinName = strdup(pszCurSkinName);
	}
}

#ifdef FOCUS
static void
rmdsSetFocus(void * /* dummy */)
{
    const char	*name;
    trmdDrvElt	*curDrv;

    name = GfuiScrollListGetSelectedElement(ScrHandle, CompetitorsScrollListId, (void**)&curDrv);
    if (name) {
	GfParmSetStr(MenuData->param, RM_SECT_DRIVERS, RM_ATTR_FOCUSED,
				 curDrv->moduleName);
	GfParmSetNum(MenuData->param, RM_SECT_DRIVERS, RM_ATTR_FOCUSEDIDX,
				 (char*)NULL, curDrv->interfaceIndex);
	GfuiLabelSetText(ScrHandle, FocusedDriverLabelId, curDrv->name);
    }
}
#endif

static void
rmdsNextMenu(void * /* dummy */)
{
    char         drvSec[256];
    const char	*name;
    trmdDrvElt	*curDrv;
    int		index;

	// Clear the race starting grid.
    GfParmListClean(MenuData->param, RM_SECT_DRIVERS);

	// And then rebuild it from the current Competitors scroll list state
	// (for each competitor, module name, interface index, car name if human, skin name if any).
    index = 1;
    while ((name = GfuiScrollListExtractElement(ScrHandle, CompetitorsScrollListId,
												0, (void**)&curDrv))) {
		sprintf(drvSec, "%s/%d", RM_SECT_DRIVERS, index);
		GfParmSetNum(MenuData->param, drvSec, RM_ATTR_IDX, (char*)NULL, curDrv->interfaceIndex);
		GfParmSetStr(MenuData->param, drvSec, RM_ATTR_MODULE, curDrv->moduleName);
		if (curDrv->carName && curDrv->isHuman)
			GfParmSetStr(MenuData->param, drvSec, RM_ATTR_CARNAME, curDrv->carName);
			// TODO: Save chosen car as the default/prefered one in human.xml ?
		if ((curDrv->skinName && strcmp(curDrv->skinName, rmdStdSkinName))
			|| GfParmGetStr(MenuData->param, drvSec, RM_ATTR_SKINNAME, 0))
			GfParmSetStr(MenuData->param, drvSec, RM_ATTR_SKINNAME, curDrv->skinName);
		index++;
    }

	// Finally, go back to the caller menu.
    rmdsDeactivate(MenuData->nextScreen);
}

static void
rmdsPreviousMenu(void *screen)
{
	// Go back to the caller menu without any change to the starting grid.
	rmdsDeactivate(screen);
}

static void
rmdsCarSelectMenu(void *pPreviousMenu)
{
	trmdDrvElt *pDriver = rmdsGetHighlightedDriver();

	if (pDriver)
	{
		CarSelectMenu.SetPreviousMenuHandle(pPreviousMenu);
		CarSelectMenu.RunMenu(pDriver);
	}
}

static void
rmdsMoveDriver(void *vd)
{
    GfuiScrollListMoveSelectedElement(ScrHandle, CompetitorsScrollListId, (long)vd);
}

static void
rmdsClickOnDriver(void * /* dummy */)
{
    static const unsigned maxBufSize = 64;
    char         buf[maxBufSize];
    const char	*name;
	trmdDrvElt	*curDrv;

	// Determine which list and which driver were clicked on.
    if ((name = GfuiScrollListGetSelectedElement(ScrHandle, CompetitorsScrollListId, (void**)&curDrv)))
	{
		// A driver from the Competitors scroll-list.
		GfuiEnable(ScrHandle, SelectButtonId, GFUI_DISABLE);
		GfuiEnable(ScrHandle, DeselectButtonId, GFUI_ENABLE);
		GfuiEnable(ScrHandle, ChangeCarButtonId, GFUI_ENABLE);
		GfuiVisibilitySet(ScrHandle, SkinEditId, GFUI_VISIBLE);
	}
    else if ((name = GfuiScrollListGetSelectedElement(ScrHandle, CandidatesScrollListId, (void**)&curDrv)))
	{
		// A driver from the Candidates scroll-list.
		GfuiEnable(ScrHandle, SelectButtonId, GFUI_ENABLE);
		GfuiEnable(ScrHandle, DeselectButtonId, GFUI_DISABLE);
		GfuiEnable(ScrHandle, ChangeCarButtonId, GFUI_DISABLE);
		GfuiVisibilitySet(ScrHandle, SkinEditId, GFUI_VISIBLE);
	}
    else
	{
		// No driver from any scroll-list.
		GfuiEnable(ScrHandle, SelectButtonId, GFUI_DISABLE);
		GfuiEnable(ScrHandle, DeselectButtonId, GFUI_DISABLE);
		GfuiEnable(ScrHandle, ChangeCarButtonId, GFUI_DISABLE);
		GfuiVisibilitySet(ScrHandle, SkinEditId, GFUI_INVISIBLE);
		GfuiStaticImageSet(ScrHandle, CarImageId, "data/img/nocarpreview.png");
	}

    // Update selected driver (if any) displayed infos.
    if (name)
	{
		rmdGetDriverType(curDrv->moduleName, buf, maxBufSize);
		GfuiLabelSetText(ScrHandle, PickedDriverTypeLabelId, buf);
		GfuiLabelSetText(ScrHandle, PickedDriverCarLabelId, GfParmGetName(curDrv->carParmHdle));
		GfuiLabelSetText(ScrHandle, PickedDriverCarCategoryLabelId,
						 GfParmGetStr(curDrv->carParmHdle, SECT_CAR, PRM_CATEGORY, ""));
		
		// Get really available skins and previews for the driver's car.
		rmdGetCarSkinsInSearchPath(curDrv, 0, VecSkinNames, MapSkins2PreviewFiles);
		
		// Set currently selected skin for this driver.
		CurSkinIndex = 0;
		if (curDrv->skinName && strcmp(curDrv->skinName, rmdStdSkinName))
		{
			std::vector<std::string>::const_iterator iterSkin =
				std::find(VecSkinNames.begin(), VecSkinNames.end(), curDrv->skinName);
			if (iterSkin != VecSkinNames.end())
				CurSkinIndex = iterSkin - VecSkinNames.begin();
		}
		const char* pszCurSkinName = VecSkinNames[CurSkinIndex].c_str();
		GfuiLabelSetText(ScrHandle, SkinEditId, pszCurSkinName);
		
		// Load associated preview image (or "no preview" image if none available).
		struct stat st;
		if (!stat(MapSkins2PreviewFiles[pszCurSkinName].c_str(), &st))
			GfuiStaticImageSet(ScrHandle, CarImageId, MapSkins2PreviewFiles[pszCurSkinName].c_str());
		else
			GfuiStaticImageSet(ScrHandle, CarImageId, "data/img/nocarpreview.png");
    }
}

static trmdDrvElt*
rmdsGetHighlightedDriver()
{
	trmdDrvElt *curDrv;
	
	const char *name =
		GfuiScrollListGetSelectedElement(ScrHandle, CompetitorsScrollListId, (void**)&curDrv);
    if (!name)
		name = GfuiScrollListGetSelectedElement(ScrHandle, CandidatesScrollListId, (void**)&curDrv);
    if (!name)
		curDrv = 0;

	return curDrv;
}

static bool
rmdsIsAnyCompetitorHighlighted()
{
	trmdDrvElt	*curDrv;

    const char* name =
		GfuiScrollListGetSelectedElement(ScrHandle, CompetitorsScrollListId, (void**)&curDrv);

	return name != 0;
}

static void
rmdsSelectDeselect(void * /* dummy */ )
{
    const char	*name;
    int		src, dst;
    trmdDrvElt	*curDrv;
    int		sel;

    // If the selected driver is in the Candidate scroll-list,
    // and if the max number of selected drivers has not been reached,
    // remove the driver from the NotSelected scroll-list, and add him to the Selected scroll-list
    sel = 0;
    name = 0;
    if (NbSelectedDrivers < NbMaxSelectedDrivers) {
	src = CandidatesScrollListId;
	name = GfuiScrollListExtractSelectedElement(ScrHandle, src, (void**)&curDrv);
	if (name) {
	    dst = CompetitorsScrollListId;
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
	src = CompetitorsScrollListId;
	name = GfuiScrollListExtractSelectedElement(ScrHandle, src, (void**)&curDrv);
	if (name) {
	    if (rmdDriverMatchesFilters(curDrv, VecCarCategories[CurCarCategoryIndex].c_str(),
									VecDriverTypes[CurDriverTypeIndex].c_str(),
									AnyCarCategory, AnyDriverType)) {
		dst = CandidatesScrollListId;
		GfuiScrollListInsertElement(ScrHandle, dst, name,
									curDrv->isHuman ? 0 : NbTotDrivers, (void*)curDrv);
	    }
	    NbSelectedDrivers--;
	    curDrv->isSelected = 0; // No more selected.
	} else {
	    return;
	}
    }

    // Focused driver management (inhibited for the moment : what is it useful for ?)
    const char *modName = GfParmGetStr(MenuData->param, RM_SECT_DRIVERS, RM_ATTR_FOCUSED, "");
    int robotIdx = (int)GfParmGetNum(MenuData->param, RM_SECT_DRIVERS, RM_ATTR_FOCUSEDIDX, (char*)NULL, 0);
    if (sel) {
	modName = GfParmGetStr(MenuData->param, RM_SECT_DRIVERS, RM_ATTR_FOCUSED, "");
	robotIdx = (int)GfParmGetNum(MenuData->param, RM_SECT_DRIVERS, RM_ATTR_FOCUSEDIDX, (char*)NULL, 0);
	if (curDrv->interfaceIndex == robotIdx && !strcmp(curDrv->moduleName, modName)) {
	    /* the focused element was deselected : select a new one */
	    name = GfuiScrollListGetElement(ScrHandle, CompetitorsScrollListId, 0, (void**)&curDrv);
	    if (name) {
		GfParmSetStr(MenuData->param, RM_SECT_DRIVERS, RM_ATTR_FOCUSED, curDrv->moduleName);
		GfParmSetNum(MenuData->param, RM_SECT_DRIVERS, RM_ATTR_FOCUSEDIDX, (char*)NULL, curDrv->interfaceIndex);
#ifdef FOCUS
		GfuiLabelSetText(ScrHandle, FocusedDriverLabelId, curDrv->name);
#endif
	    } else {
		GfParmSetStr(MenuData->param, RM_SECT_DRIVERS, RM_ATTR_FOCUSED, "");
		GfParmSetNum(MenuData->param, RM_SECT_DRIVERS, RM_ATTR_FOCUSEDIDX, (char*)NULL, 0);
#ifdef FOCUS
		GfuiLabelSetText(ScrHandle, FocusedDriverLabelId, "");
#endif
	    }
	}
    } else {
	if (strlen(modName) == 0 || curDrv->isHuman) {
	    GfParmSetStr(MenuData->param, RM_SECT_DRIVERS, RM_ATTR_FOCUSED, curDrv->moduleName);
	    GfParmSetNum(MenuData->param, RM_SECT_DRIVERS, RM_ATTR_FOCUSEDIDX, (char*)NULL, curDrv->interfaceIndex);
#ifdef FOCUS
	    GfuiLabelSetText(ScrHandle, FocusedDriverLabelId, curDrv->name);
#endif
	}
    }

    // Update selected driver displayed info
    rmdsClickOnDriver(NULL);

	// Don't allow user to Accept 0 drivers this would cause a crash
	GfuiEnable(ScrHandle,NextButtonId, NbSelectedDrivers > 0 ? GFUI_ENABLE : GFUI_DISABLE);

	GfuiDisplay();
}

static void
rmdsAddKeys(void)
{
    GfuiAddKey(ScrHandle, GFUIK_ESCAPE, "Previous menu", MenuData->prevScreen, rmdsPreviousMenu, NULL);
    GfuiAddKey(ScrHandle, GFUIK_RETURN, "Next menu", NULL, rmdsNextMenu, NULL);
    GfuiAddKey(ScrHandle, GFUIK_F1, "Help", ScrHandle, GfuiHelpScreen, NULL);
    GfuiAddKey(ScrHandle, GFUIK_F12, "Screen-Shot", NULL, GfuiScreenShot, NULL);
    GfuiAddKey(ScrHandle, '-', "Move Up", (void*)-1, rmdsMoveDriver, NULL);
    GfuiAddKey(ScrHandle, '+', "Move Down", (void*)1, rmdsMoveDriver, NULL);
    GfuiAddKey(ScrHandle, ' ', "Select/Deselect", NULL, rmdsSelectDeselect, NULL);
#ifdef FOCUS
    GfuiAddKey(ScrHandle, 'f', "Set Focus", NULL, rmdsSetFocus, NULL);
#endif    
}

/** Interactive Drivers list selection
    @param	vs	Pointer on tRmDriverSelect structure (casted to void*)
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
    const char	*skinName;
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
    MenuData = (tRmDriverSelect*)vs;

    // Create screen, background image and title
    ScrHandle = GfuiScreenCreateEx((float*)NULL, NULL, rmdsActivate, NULL, (tfuiCallback)NULL, 1);
    
    void *menuDescHdle = LoadMenuXML("driverselectmenu.xml");
    CreateStaticControls(menuDescHdle,ScrHandle);

    CompetitorsScrollListId = CreateScrollListControl(ScrHandle,menuDescHdle,"competitorsscrolllist",NULL,rmdsClickOnDriver);
    CandidatesScrollListId = CreateScrollListControl(ScrHandle,menuDescHdle,"candidatesscrolllist",NULL,rmdsClickOnDriver);

	
    // Car category filtering "combobox" (left arrow, label, right arrow)
    CreateButtonControl(ScrHandle,menuDescHdle,"carcategoryleftarrow",(void*)-1,rmdsChangeCarCategory);
    CreateButtonControl(ScrHandle,menuDescHdle,"carcategoryrightarrow",(void*)1,rmdsChangeCarCategory);
    CarCategoryEditId = CreateLabelControl(ScrHandle,menuDescHdle,"carcategorytext");
    
    // Driver type filtering "combobox" (left arrow, label, right arrow)
    CreateButtonControl(ScrHandle,menuDescHdle,"drivertypeleftarrow",(void*)-1,rmdsChangeDriverType);
    CreateButtonControl(ScrHandle,menuDescHdle,"drivertyperightarrow",(void*)1,rmdsChangeDriverType);
    DriverTypeEditId = CreateLabelControl(ScrHandle,menuDescHdle,"drivertypetext");

    // Scroll-lists manipulation buttons
    CreateButtonControl(ScrHandle,menuDescHdle,"moveupbutton",(void*)-1,rmdsMoveDriver);
    CreateButtonControl(ScrHandle,menuDescHdle,"movedownbutton",(void*)1,rmdsMoveDriver);
	
    SelectButtonId = CreateButtonControl(ScrHandle,menuDescHdle,"selectbutton",0,rmdsSelectDeselect);
    DeselectButtonId = CreateButtonControl(ScrHandle,menuDescHdle,"deselectbutton",0,rmdsSelectDeselect);
	GfuiEnable(ScrHandle, SelectButtonId, GFUI_ENABLE);
	GfuiEnable(ScrHandle, DeselectButtonId, GFUI_DISABLE);

    // Skin selection "combobox" (left arrow, label, right arrow)
    SkinLeftButtonId = CreateButtonControl(ScrHandle,menuDescHdle,"skinleftarrow",(void*)-1,rmdsChangeSkin);
    SkinRightButtonId = CreateButtonControl(ScrHandle,menuDescHdle,"skinrightarrow",(void*)1,rmdsChangeSkin);
    SkinEditId = CreateLabelControl(ScrHandle,menuDescHdle,"skintext");

    // Car preview image
    CarImageId = CreateStaticImageControl(ScrHandle, menuDescHdle, "carpreviewimage");
    GfuiStaticImageSet(ScrHandle, CarImageId, "data/img/nocarpreview.png");

    // Load driver full list, driver type list and driven car category list
    GF_TAILQ_INIT(&DriverList);

    VecCarCategories.push_back(AnyCarCategory);
    VecDriverTypes.push_back(AnyDriverType);

    list = 0;
    sprintf(buf, "%sdrivers", GetLibDir());
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
				GfError("No driver '%s' selected because no readable '%s.xml' found\n", modName, modName);
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
							curDrv->carName = strdup(carName);
							curDrv->skinName = 0; // Initialized later if needed from race params.
							curDrv->name = strdup(curmod->modInfo[i].name);
							curDrv->carParmHdle = carhdle;
							const char* carCat = GfParmGetStr(carhdle, SECT_CAR, PRM_CATEGORY, "");
							if (std::find(VecCarCategories.begin(), VecCarCategories.end(), carCat) == VecCarCategories.end()) {
								VecCarCategories.push_back(carCat);
							}
							rmdGetDriverType(modName, drvType, drvTypeMaxSize);
							if (std::find(VecDriverTypes.begin(), VecDriverTypes.end(), drvType) == VecDriverTypes.end()) {
								VecDriverTypes.push_back(drvType);
							}
			    
							curDrv->isHuman = human ? 1 : 0;
							if (human) {
								GF_TAILQ_INSERT_HEAD(&DriverList, curDrv, link);
							} else {
								GF_TAILQ_INSERT_TAIL(&DriverList, curDrv, link);
							}
							NbTotDrivers++;
						} else {
							GfError("Ignoring '%s' (%s #%d) because '%s' is not readable\n",
									curmod->modInfo[i].name, modName, i, path);
						}
					} else {
						GfError("Ignoring '%s' (%s #%d) because '%s' was not found\n",
								curmod->modInfo[i].name, modName, i, path);
					}
				}
			}
			if (robhdle)
				GfParmReleaseHandle(robhdle);
		} while (curmod != list);
    }

    GfModFreeInfoList(&list);

    // Load Competitors scroll-list from the race params file
    // and initialize Candidates scroll-list car category filter criteria to the car category
    // of the last selected human driver or else to the first selected robot.
	// Also load chosen skin (if any) for each competitor,
	// and chosen car for human ones (for those, the car in human.xml is only
	// a default/prefered one, but can be changed when configuring each race).
    NbSelectedDrivers = 0;
    NbMaxSelectedDrivers =
		(int)GfParmGetNum(MenuData->param, RM_SECT_DRIVERS, RM_ATTR_MAXNUM, NULL, 0);
    nDrivers = GfParmGetEltNb(MenuData->param, RM_SECT_DRIVERS);
    initCarCat = 0;
    index = 1;
    for (i = 1; i < nDrivers+1; i++) {
		// Get driver infos from the the starting grid in the race params file
		sprintf(path, "%s/%d", RM_SECT_DRIVERS, i);
		moduleName = GfParmGetStr(MenuData->param, path, RM_ATTR_MODULE, "");
		robotIdx = (int)GfParmGetNum(MenuData->param, path, RM_ATTR_IDX, (char*)NULL, 0);
		skinName = GfParmGetStr(MenuData->param, path, RM_ATTR_SKINNAME, rmdStdSkinName);
		carName = GfParmGetStr(MenuData->param, path, RM_ATTR_CARNAME, 0);

		// Try and retrieve this driver in the full drivers list
		if ((curDrv = GF_TAILQ_FIRST(&DriverList))) {
			do {
				if (curDrv->interfaceIndex == robotIdx && !strcmp(curDrv->moduleName, moduleName)) {
					// We've got it : if we can keep it for the race
					// (there is a threshold on the number of competitors) :
					if (NbSelectedDrivers < NbMaxSelectedDrivers) {

						// The driver is selected for the race !
						curDrv->isSelected = index++;
						
						// Add its name to the Competitors scroll list.
						GfuiScrollListInsertElement(ScrHandle, CompetitorsScrollListId,
													curDrv->name, index, (void*)curDrv);

						// Try and determine the initial car category for the filtering combo-box
						// (the car category of the last human driver, or else of the last driver).
						if (!initCarCat || curDrv->isHuman)
							initCarCat = GfParmGetStr(curDrv->carParmHdle, SECT_CAR, PRM_CATEGORY, "");

						// Get the chosen car for the race if any specified (human only).
						if (curDrv->isHuman && carName)
						{
							sprintf(path, "cars/%s/%s.xml", carName, carName);
							if (!stat(path, &st)) {
								carhdle = GfParmReadFile(path, GFPARM_RMODE_STD);
								if (carhdle) {
									if (curDrv->carParmHdle)
										GfParmReleaseHandle(curDrv->carParmHdle);
									curDrv->carParmHdle = carhdle;
									if (curDrv->carName)
										free(curDrv->carName);
									curDrv->carName = strdup(carName);
								} else {
									GfError("Falling back to default car '%s' "
											"for %s because '%s' is not readable\n",
											curDrv->carName, curDrv->name, path);
								}
							} else {
								GfError("Falling back to default car '%s' "
										"for %s because '%s' was not found\n",
										curDrv->carName, curDrv->name, path);
							}
						}

						// Get the chosen car skin/livery if any specified.
						if (skinName && strcmp(skinName, rmdStdSkinName))
							curDrv->skinName = strdup(skinName);

						// Increment the current number of selected drivers.
						NbSelectedDrivers++;
					}
					break;
				}
			} while ((curDrv = GF_TAILQ_NEXT(curDrv, link)));
		}
    }

    // Initialize Candidates scroll-list filter criteria and fill-in the scroll-list.
    CurDriverTypeIndex = 0;
    if (!initCarCat)
		initCarCat = AnyCarCategory;
    CurCarCategoryIndex = std::find(VecCarCategories.begin(), VecCarCategories.end(), initCarCat) - VecCarCategories.begin();
    CurCarCategoryIndex = CurCarCategoryIndex % VecCarCategories.size();
    rmdsFilterDriverScrollList(initCarCat, AnyDriverType);
    GfuiLabelSetText(ScrHandle, DriverTypeEditId, AnyDriverType);
    GfuiLabelSetText(ScrHandle, CarCategoryEditId, initCarCat);

    // Picked Driver Info
    PickedDriverTypeLabelId = CreateLabelControl(ScrHandle,menuDescHdle,"pickeddrivertypelabel");
    PickedDriverCarCategoryLabelId = CreateLabelControl(ScrHandle,menuDescHdle,"pickeddrivercarcategorylabel");
    PickedDriverCarLabelId = CreateLabelControl(ScrHandle,menuDescHdle,"pickeddrivercarlabel");
    
    // Next, Previous and Change Car buttons
    NextButtonId = CreateButtonControl(ScrHandle,menuDescHdle,"nextmenubutton",NULL,rmdsNextMenu);
    CreateButtonControl(ScrHandle,menuDescHdle,"previousmenubutton",MenuData->prevScreen,rmdsPreviousMenu);
    ChangeCarButtonId = CreateButtonControl(ScrHandle,menuDescHdle,"carselectbutton",ScrHandle,rmdsCarSelectMenu);
	GfuiEnable(ScrHandle, ChangeCarButtonId, GFUI_DISABLE);

    GfParmReleaseHandle(menuDescHdle);

    // Keyboard shortcuts
    GfuiMenuDefaultKeysAdd(ScrHandle);
    rmdsAddKeys();

    GfuiScreenActivate(ScrHandle);
}


static void
rmdsCleanup(void)
{
    trmdDrvElt *curDrv;

    VecCarCategories.clear();
    VecDriverTypes.clear();
    VecSkinNames.clear();
    MapSkins2PreviewFiles.clear();

    while ((curDrv = GF_TAILQ_FIRST(&DriverList))) {
	GF_TAILQ_REMOVE(&DriverList, curDrv, link);
	free(curDrv->name);
	free(curDrv->moduleName);
	free(curDrv->carName);
	if (curDrv->skinName)
	    free(curDrv->skinName);
	GfParmReleaseHandle(curDrv->carParmHdle);
	free(curDrv);
    }
}

static void
rmdsFilterDriverScrollList(const char* carCategory, const char* driverType)
{
    trmdDrvElt	*curDrv;

    // Empty the unselected scroll-list
    GfuiScrollListClear(ScrHandle, CandidatesScrollListId);
	GfuiEnable(ScrHandle, SelectButtonId, GFUI_DISABLE);

    // Fill it with drivers that match the filter criteria
    curDrv = GF_TAILQ_FIRST(&DriverList);
    if (curDrv) {
	do {
	    if (!curDrv->isSelected && rmdDriverMatchesFilters(curDrv, carCategory, driverType,
														   AnyCarCategory, AnyDriverType))
		GfuiScrollListInsertElement(ScrHandle, CandidatesScrollListId, curDrv->name,
									NbTotDrivers, (void*)curDrv);
	} while ((curDrv = GF_TAILQ_NEXT(curDrv, link)));
    }

    // Show first element if any
    GfuiScrollListShowElement(ScrHandle, CandidatesScrollListId, 0);
}
