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

static void		*ScrHandle;
static tRmDriverSelect	*MenuData;
static int		CompetitorsScrollList, CandidatesScrollList;
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

static RmCarSelectMenu CarSelectMenu;

static int		NbTotDrivers;
static int		NbSelectedDrivers;
static int		NbMaxSelectedDrivers;

// Car category list
static const char* AnyCarCategory = "--- All ---";
static std::vector<std::string> CarCategoryList;
static size_t CurCarCategoryIndex = 0;

// Driver type list
static const char* AnyDriverType = "--- All ---";
static std::vector<std::string> DriverTypeList;
static size_t CurDriverTypeIndex = 0;

// Skin name and preview file lists
static std::vector<std::string> SkinNameList;
static std::vector<std::string> PreviewFileList;
static size_t CurSkinIndex = 0;

// Driver full list
GF_TAILQ_HEAD(DriverListHead, trmdDrvElt);
static tDriverListHead DriverList;

// Functions and constants that should be in driver.h/cpp (temporary workaround for MS shit)
static const char* pszSkinFileExt = ".png";
static const char* pszPreviewFileSuffix = "-preview.png";

static const char* apszExcludedSkinFileSuffixes[] =
{ "-rpm.png", "-speed.png", pszPreviewFileSuffix };
static const int nExcludedSkinFileSuffixes = sizeof(apszExcludedSkinFileSuffixes) / sizeof(char*);

void rmdGetCarSkinsInFolder(const trmdDrvElt *pDriver, const char* pszFolderPath,
							std::vector<std::string>& lstSkinNames,
							std::vector<std::string>& lstPreviewFiles)
{
	//struct stat st;
	tFList *pSkinFileList, *pCurSkinFile;

	GfOut("rmdGetCarSkinsInFolder(%s) :\n", pszFolderPath);

	pCurSkinFile = pSkinFileList =
		GfDirGetListFiltered(pszFolderPath, pDriver->carName, pszSkinFileExt);
		
	if (pSkinFileList)
		do
		{
			pCurSkinFile = pCurSkinFile->next;

			// Ignore files with an excluded suffix.
			int nExclSfxInd = 0;
			for (; nExclSfxInd < nExcludedSkinFileSuffixes; nExclSfxInd++)
				 if (strstr(pCurSkinFile->name, apszExcludedSkinFileSuffixes[nExclSfxInd]))
					 break;
			if (nExclSfxInd < nExcludedSkinFileSuffixes)
				continue;

			// Extract the skin name from the skin file name.
			const int nSkinNameLen = // Expecting "<car name>-<skin name>.png"
				strlen(pCurSkinFile->name) - strlen(pDriver->carName) - 1 - strlen(pszSkinFileExt);
			std::string strSkinName;
			if (nSkinNameLen > 0)
				strSkinName =
					std::string(pCurSkinFile->name)
					.substr(strlen(pDriver->carName) + 1, nSkinNameLen);
			else // Assuming default/standard "<car name>.png"
				strSkinName = rmdStdSkinName;
			
			// Ignore skins that are already in the list (path search priority).
			if (std::find(lstSkinNames.begin(), lstSkinNames.end(), strSkinName)
				== lstSkinNames.end())
			{
				// Add found skin in the list
				lstSkinNames.push_back(strSkinName);
				
				// Add associated preview image
				// (don't check file existence now, will be needed only at display time).
				std::ostringstream ossPreviewName;
				ossPreviewName << pszFolderPath << '/' << pDriver->carName;
				if (strSkinName != rmdStdSkinName)
					ossPreviewName << '-' << strSkinName;
				ossPreviewName << pszPreviewFileSuffix;
				//if (!stat(ossPreviewName.str().c_str(), &st))
				{
					lstPreviewFiles.push_back(ossPreviewName.str());
					GfOut("* found skin=%s, preview=%s\n",
						  strSkinName.c_str(), ossPreviewName.str().c_str());
				}
// 				else
// 					GfError("Ignoring '%s' skin for %s/%d/%s because preview file %s not found\n",
// 							strSkinName.c_str(), pDriver->moduleName, pDriver->interfaceIndex,
// 							pDriver->carName, ossPreviewName.str().c_str());
			}
				
		} while (pCurSkinFile != pSkinFileList);
		
	GfDirFreeList(pSkinFileList, NULL);
}

void rmdGetCarSkinsInSearchPath(const trmdDrvElt *pDriver,
								std::vector<std::string>& lstSkinNames,
								std::vector<std::string>& lstPreviewFiles)
{
	std::ostringstream ossDirPath;
	std::string strPreviewName;

	GfOut("rmdGetCarSkinsInSearchPath : module=%s, idx=%d, car=%s ...\n",
		  pDriver->moduleName, pDriver->interfaceIndex, pDriver->carName);

	// Clear the skin and preview lists.
	lstSkinNames.clear();
	lstPreviewFiles.clear();

	// Get skins/previews from the directories in the search path
	// (WARNING: Must be consistent with the search path passed to ssgTexturePath in grcar.cpp,
	//           at least for the car skin file search).
	ossDirPath.str("");
	ossDirPath << GetLocalDir() << "drivers/" << pDriver->moduleName
			   << '/' << pDriver->carName;
	rmdGetCarSkinsInFolder(pDriver, ossDirPath.str().c_str(),
						   lstSkinNames, lstPreviewFiles);

	ossDirPath.str("");
	ossDirPath << "drivers/" << pDriver->moduleName
			   << '/' << pDriver->interfaceIndex << '/' << pDriver->carName;
	rmdGetCarSkinsInFolder(pDriver, ossDirPath.str().c_str(),
						   lstSkinNames, lstPreviewFiles);

	ossDirPath.str("");
	ossDirPath << "drivers/" << pDriver->moduleName
			   << '/' << pDriver->interfaceIndex;
	rmdGetCarSkinsInFolder(pDriver, ossDirPath.str().c_str(),
						   lstSkinNames, lstPreviewFiles);

	ossDirPath.str("");
	ossDirPath << "drivers/" << pDriver->moduleName
			   << '/' << pDriver->carName;
	rmdGetCarSkinsInFolder(pDriver, ossDirPath.str().c_str(),
						   lstSkinNames, lstPreviewFiles);

	ossDirPath.str("");
	ossDirPath << "drivers/" << pDriver->moduleName;
	rmdGetCarSkinsInFolder(pDriver, ossDirPath.str().c_str(),
						   lstSkinNames, lstPreviewFiles);

	ossDirPath.str("");
	ossDirPath << "cars/" << pDriver->carName;
	rmdGetCarSkinsInFolder(pDriver, ossDirPath.str().c_str(),
						   lstSkinNames, lstPreviewFiles);

// 	// If no skin was found, add the car's standard one, should always be there !
// 	if (lstSkinNames.empty())
// 	{
// 		// Skin.
// 		lstSkinNames.push_back(rmdStdSkinName);
		
// 		// Associated preview image.
// 		std::ostringstream ossPreviewName;
// 		ossPreviewName << "cars/" << pDriver->carName << '/' << pDriver->carName
// 					   << pszPreviewFileSuffix;
// 		lstPreviewFiles.push_back(ossPreviewName.str());
		
// 		GfOut("... skin=<%s>, preview=%s\n",
// 			  rmdStdSkinName, ossPreviewName.str().c_str());
// 	}
	
	// If we have at least 1 skin, make sure that if the standard one is inside,
	// it is the first one.
	if (!lstSkinNames.empty())
	{
		std::vector<std::string>::iterator iterSkin =
			std::find(lstSkinNames.begin(), lstSkinNames.end(), rmdStdSkinName);
		if (iterSkin != lstSkinNames.end() && iterSkin != lstSkinNames.begin())
		{
			std::vector<std::string>::iterator iterPreview =
				lstPreviewFiles.begin() + (iterSkin - lstSkinNames.begin());
			strPreviewName = *iterPreview;
			lstSkinNames.erase(iterSkin);
			lstPreviewFiles.erase(iterPreview);
			
			lstSkinNames.insert(lstSkinNames.begin(), rmdStdSkinName);
			lstPreviewFiles.insert(lstPreviewFiles.begin(), strPreviewName);
		}
	}
}



// Local functions.
static void rmdsCleanup(void);
static void rmdsFilterDriverScrollList(const char* carCat, const char* driverType);
static trmdDrvElt* rmdsGetHighlightedDriver();
static bool rmdsIsAnyCompetitorHighlighted();


// Screen activation call-back.
static void
rmdsActivate(void * /* notused */)
{
// 	if (rmdsIsAnyCompetitorHighlighted())
// 		GfuiVisibilitySet(ScrHandle, ChangeCarButtonId, GFUI_INVISIBLE);
// 	GfuiVisibilitySet(ScrHandle, SkinEditId, GFUI_INVISIBLE);
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
		(CurCarCategoryIndex + CarCategoryList.size() + (int)(long)vp) % CarCategoryList.size();

    GfuiLabelSetText(ScrHandle, CarCategoryEditId, CarCategoryList[CurCarCategoryIndex].c_str());

    rmdsFilterDriverScrollList(CarCategoryList[CurCarCategoryIndex].c_str(),
							   DriverTypeList[CurDriverTypeIndex].c_str());

	if (rmdsIsAnyCompetitorHighlighted())
		GfuiVisibilitySet(ScrHandle, ChangeCarButtonId, GFUI_VISIBLE);
}

static void
rmdsChangeDriverType(void *vp)
{
 	CurDriverTypeIndex =
		(CurDriverTypeIndex + DriverTypeList.size() + (int)(long)vp) % DriverTypeList.size();

    GfuiLabelSetText(ScrHandle, DriverTypeEditId, DriverTypeList[CurDriverTypeIndex].c_str());

    rmdsFilterDriverScrollList(CarCategoryList[CurCarCategoryIndex].c_str(),
							   DriverTypeList[CurDriverTypeIndex].c_str());

	if (rmdsIsAnyCompetitorHighlighted())
		GfuiVisibilitySet(ScrHandle, ChangeCarButtonId, GFUI_VISIBLE);
}

static void
rmdsChangeSkin(void *vp)
{
	if (SkinNameList.empty())
		return;

	// Update GUI.
 	CurSkinIndex = (CurSkinIndex + SkinNameList.size() + (int)(long)vp) % SkinNameList.size();

    GfuiLabelSetText(ScrHandle, SkinEditId, SkinNameList[CurSkinIndex].c_str());

	// Load associated preview image (or "no preview panel" if none available).
	struct stat st;
	if (!stat(PreviewFileList[CurSkinIndex].c_str(), &st))
		GfuiStaticImageSet(ScrHandle, CarImageId, PreviewFileList[CurSkinIndex].c_str());
	else
		GfuiStaticImageSet(ScrHandle, CarImageId, "data/img/nocarpreview.png");

	// Update highlighted driver skin.
	trmdDrvElt *pDriver = rmdsGetHighlightedDriver();
	if (pDriver)
	{
		if (pDriver->skinName)
			free(pDriver->skinName);
		pDriver->skinName = strdup(SkinNameList[CurSkinIndex].c_str());
	}
}

#ifdef FOCUS
static void
rmdsSetFocus(void * /* dummy */)
{
    const char	*name;
    trmdDrvElt	*curDrv;

    name = GfuiScrollListGetSelectedElement(ScrHandle, CompetitorsScrollList, (void**)&curDrv);
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
    
    GfParmListClean(MenuData->param, RM_SECT_DRIVERS);
    name = GfuiScrollListExtractElement(ScrHandle, CompetitorsScrollList, 0, (void**)&curDrv);
    index = 1;
    while (name) {
	sprintf(drvSec, "%s/%d", RM_SECT_DRIVERS, index);
	GfParmSetNum(MenuData->param, drvSec, RM_ATTR_IDX, (char*)NULL, curDrv->interfaceIndex);
	GfParmSetStr(MenuData->param, drvSec, RM_ATTR_MODULE, curDrv->moduleName);
	GfOut("rmdsNextMenu : drv %s, idx=%d, mod=%s, skin=%s (was %s)\n",
		  drvSec, curDrv->interfaceIndex, curDrv->moduleName,
		  curDrv->skinName ? curDrv->skinName : "<null>",
		  GfParmGetStr(MenuData->param, drvSec, RM_ATTR_SKIN, 0)
		  ? GfParmGetStr(MenuData->param, drvSec, RM_ATTR_SKIN, 0) : "<null>");
	if ((curDrv->skinName && strcmp(curDrv->skinName, rmdStdSkinName))
		|| GfParmGetStr(MenuData->param, drvSec, RM_ATTR_SKIN, 0))
		GfParmSetStr(MenuData->param, drvSec, RM_ATTR_SKIN, curDrv->skinName);
	index++;
	name = GfuiScrollListExtractElement(ScrHandle, CompetitorsScrollList, 0, (void**)&curDrv);
    }
    rmdsDeactivate(MenuData->nextScreen);
}

static void
rmdsPreviousMenu(void *screen)
{
	rmdsDeactivate(screen);
}

static void
rmdsCarSelectMenu(void *pPreviousMenu)
{
	const trmdDrvElt *pDriver = rmdsGetHighlightedDriver();

	if (pDriver)
		CarSelectMenu.RunMenu(pDriver);
}

static void
rmdsMoveDriver(void *vd)
{
    GfuiScrollListMoveSelectedElement(ScrHandle, CompetitorsScrollList, (long)vd);
}

static void
rmdsClickOnDriver(void * /* dummy */)
{
    static const unsigned maxBufSize = 64;
    char         buf[maxBufSize];
    const char	*name;
	trmdDrvElt	*curDrv;

	// Determine which list and which driver were clicked on.
    if ((name = GfuiScrollListGetSelectedElement(ScrHandle, CompetitorsScrollList, (void**)&curDrv)))
	{
		GfuiVisibilitySet(ScrHandle, SelectButtonId, GFUI_INVISIBLE);
		GfuiVisibilitySet(ScrHandle, DeselectButtonId, GFUI_VISIBLE);
		GfuiVisibilitySet(ScrHandle, ChangeCarButtonId, GFUI_VISIBLE);
		GfuiVisibilitySet(ScrHandle, SkinEditId, GFUI_VISIBLE);
		GfuiVisibilitySet(ScrHandle, CarImageId, GFUI_VISIBLE);
	}
    else if ((name = GfuiScrollListGetSelectedElement(ScrHandle, CandidatesScrollList, (void**)&curDrv)))
	{
		GfuiVisibilitySet(ScrHandle, SelectButtonId, GFUI_VISIBLE);
		GfuiVisibilitySet(ScrHandle, DeselectButtonId, GFUI_INVISIBLE);
		GfuiVisibilitySet(ScrHandle, ChangeCarButtonId, GFUI_INVISIBLE);
		GfuiVisibilitySet(ScrHandle, SkinEditId, GFUI_VISIBLE);
		GfuiVisibilitySet(ScrHandle, CarImageId, GFUI_VISIBLE);
	}
    else
	{
		GfuiVisibilitySet(ScrHandle, SelectButtonId, GFUI_INVISIBLE);
		GfuiVisibilitySet(ScrHandle, DeselectButtonId, GFUI_INVISIBLE);
		GfuiVisibilitySet(ScrHandle, ChangeCarButtonId, GFUI_INVISIBLE);
		GfuiVisibilitySet(ScrHandle, SkinEditId, GFUI_INVISIBLE);
		GfuiVisibilitySet(ScrHandle, CarImageId, GFUI_INVISIBLE);
	}

    // Get selected driver infos
    if (name)
	{
		rmdGetDriverType(curDrv->moduleName, buf, maxBufSize);
		GfuiLabelSetText(ScrHandle, PickedDriverTypeLabelId, buf);
		GfuiLabelSetText(ScrHandle, PickedDriverCarLabelId, GfParmGetName(curDrv->carParmHdle));
		GfuiLabelSetText(ScrHandle, PickedDriverCarCategoryLabelId,
						 GfParmGetStr(curDrv->carParmHdle, SECT_CAR, PRM_CATEGORY, ""));
		
		// Get really available skins and previews for the driver's car.
		rmdGetCarSkinsInSearchPath(curDrv, SkinNameList, PreviewFileList);
		
		// Set currently selected skin for this driver.
		CurSkinIndex = 0;
		if (curDrv->skinName && strcmp(curDrv->skinName, rmdStdSkinName))
		{
			std::vector<std::string>::const_iterator iterSkin =
				std::find(SkinNameList.begin(), SkinNameList.end(), curDrv->skinName);
			if (iterSkin != SkinNameList.end())
				CurSkinIndex = iterSkin - SkinNameList.begin();
		}
		GfuiLabelSetText(ScrHandle, SkinEditId, SkinNameList[CurSkinIndex].c_str());
		
		// Load associated preview image (or "no preview" image if none available).
		struct stat st;
		if (!stat(PreviewFileList[CurSkinIndex].c_str(), &st))
			GfuiStaticImageSet(ScrHandle, CarImageId, PreviewFileList[CurSkinIndex].c_str());
		else
			GfuiStaticImageSet(ScrHandle, CarImageId, "data/img/nocarpreview.png");
    }
}

static trmdDrvElt*
rmdsGetHighlightedDriver()
{
	trmdDrvElt *curDrv;
	
	const char *name =
		GfuiScrollListGetSelectedElement(ScrHandle, CompetitorsScrollList, (void**)&curDrv);
    if (!name)
		name = GfuiScrollListGetSelectedElement(ScrHandle, CandidatesScrollList, (void**)&curDrv);
    if (!name)
		curDrv = 0;

	return curDrv;
}

static bool
rmdsIsAnyCompetitorHighlighted()
{
	trmdDrvElt	*curDrv;

    const char* name =
		GfuiScrollListGetSelectedElement(ScrHandle, CompetitorsScrollList, (void**)&curDrv);

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
	src = CandidatesScrollList;
	name = GfuiScrollListExtractSelectedElement(ScrHandle, src, (void**)&curDrv);
	if (name) {
	    dst = CompetitorsScrollList;
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
	src = CompetitorsScrollList;
	name = GfuiScrollListExtractSelectedElement(ScrHandle, src, (void**)&curDrv);
	if (name) {
	    if (rmdDriverMatchesFilters(curDrv, CarCategoryList[CurCarCategoryIndex].c_str(), DriverTypeList[CurDriverTypeIndex].c_str(), AnyCarCategory, AnyDriverType)) {
		dst = CandidatesScrollList;
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

    // Focused driver management (inhibited for the moment : what is it useful for ?)
    const char *modName = GfParmGetStr(MenuData->param, RM_SECT_DRIVERS, RM_ATTR_FOCUSED, "");
    int robotIdx = (int)GfParmGetNum(MenuData->param, RM_SECT_DRIVERS, RM_ATTR_FOCUSEDIDX, (char*)NULL, 0);
    if (sel) {
	modName = GfParmGetStr(MenuData->param, RM_SECT_DRIVERS, RM_ATTR_FOCUSED, "");
	robotIdx = (int)GfParmGetNum(MenuData->param, RM_SECT_DRIVERS, RM_ATTR_FOCUSEDIDX, (char*)NULL, 0);
	if (curDrv->interfaceIndex == robotIdx && !strcmp(curDrv->moduleName, modName)) {
	    /* the focused element was deselected : select a new one */
	    name = GfuiScrollListGetElement(ScrHandle, CompetitorsScrollList, 0, (void**)&curDrv);
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

    CompetitorsScrollList = CreateScrollListControl(ScrHandle,menuDescHdle,"competitorsscrolllist",NULL,rmdsClickOnDriver);
    CandidatesScrollList = CreateScrollListControl(ScrHandle,menuDescHdle,"candidatesscrolllist",NULL,rmdsClickOnDriver);

	
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
	GfuiVisibilitySet(ScrHandle, SelectButtonId, GFUI_INVISIBLE);
	GfuiVisibilitySet(ScrHandle, DeselectButtonId, GFUI_INVISIBLE);

    // Skin selection "combobox" (left arrow, label, right arrow)
    SkinLeftButtonId = CreateButtonControl(ScrHandle,menuDescHdle,"skinleftarrow",(void*)-1,rmdsChangeSkin);
    SkinRightButtonId = CreateButtonControl(ScrHandle,menuDescHdle,"skinrightarrow",(void*)1,rmdsChangeSkin);
    SkinEditId = CreateLabelControl(ScrHandle,menuDescHdle,"skintext");

    // Car preview image
    CarImageId = CreateStaticImageControl(ScrHandle, menuDescHdle, "carpreviewimage");
    GfuiStaticImageSet(ScrHandle, CarImageId, "data/img/nocarpreview.png");

    // Load driver full list, driver type list and driven car category list
    GF_TAILQ_INIT(&DriverList);

    CarCategoryList.push_back(AnyCarCategory);
    DriverTypeList.push_back(AnyDriverType);

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
		GfError("Warning: No driver '%s' selected because no readable '%s.xml' found\n", modName, modName);
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
			    if (std::find(CarCategoryList.begin(), CarCategoryList.end(), carCat) == CarCategoryList.end()) {
				CarCategoryList.push_back(carCat);
			    }
			    rmdGetDriverType(modName, drvType, drvTypeMaxSize);
			    if (std::find(DriverTypeList.begin(), DriverTypeList.end(), drvType) == DriverTypeList.end()) {
			      DriverTypeList.push_back(drvType);
			    }
			    
				curDrv->isHuman = human ? 1 : 0;
			    if (human) {
				GF_TAILQ_INSERT_HEAD(&DriverList, curDrv, link);
			    } else {
				GF_TAILQ_INSERT_TAIL(&DriverList, curDrv, link);
			    }
			    NbTotDrivers++;
			} else {
			    GfError("Warning: Driver '%s' not selected because car '%s' is not readable\n", curmod->modInfo[i].name, carName);
			}
		    } else {
			GfError("Warning: Driver '%s' not selected because car '%s' is not present\n", curmod->modInfo[i].name, carName);
		    }
		}
	    }
	    if (robhdle)
		GfParmReleaseHandle(robhdle);
	} while (curmod != list);
    }

    GfModFreeInfoList(&list);

    // Load Competitors scroll-list from file
    // and initialize Candidates scroll-list car category filter criteria to the car category
    // of the last selected human driver or else to the first selected robot. 
    NbSelectedDrivers = 0;
    NbMaxSelectedDrivers = (int)GfParmGetNum(MenuData->param,
											 RM_SECT_DRIVERS, RM_ATTR_MAXNUM, NULL, 0);
    nDrivers = GfParmGetEltNb(MenuData->param, RM_SECT_DRIVERS);
    initCarCat = 0;
    index = 1;
    for (i = 1; i < nDrivers+1; i++) {
	sprintf(path, "%s/%d", RM_SECT_DRIVERS, i);
	moduleName = GfParmGetStr(MenuData->param, path, RM_ATTR_MODULE, "");
	robotIdx = (int)GfParmGetNum(MenuData->param, path, RM_ATTR_IDX, (char*)NULL, 0);
	skinName = GfParmGetStr(MenuData->param, path, RM_ATTR_SKIN, rmdStdSkinName);
	if ((curDrv = GF_TAILQ_FIRST(&DriverList))) {
	    do {
		if (curDrv->interfaceIndex == robotIdx && !strcmp(curDrv->moduleName, moduleName)) {
		    if (NbSelectedDrivers < NbMaxSelectedDrivers) {
			GfuiScrollListInsertElement(ScrHandle, CompetitorsScrollList, curDrv->name, index, (void*)curDrv);
			if (!initCarCat || !strcmp(moduleName, "human"))
			    initCarCat = GfParmGetStr(curDrv->carParmHdle, SECT_CAR, PRM_CATEGORY, "");
			curDrv->isSelected = index++;
			if (skinName && strcmp(skinName, rmdStdSkinName))
				curDrv->skinName = strdup(skinName);
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
    CurCarCategoryIndex = std::find(CarCategoryList.begin(), CarCategoryList.end(), initCarCat) - CarCategoryList.begin();
    CurCarCategoryIndex = CurCarCategoryIndex % CarCategoryList.size();
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
	GfuiVisibilitySet(ScrHandle, ChangeCarButtonId, GFUI_INVISIBLE);

    GfParmReleaseHandle(menuDescHdle);

	// Initialize the Car selection menu.
	CarSelectMenu.Init(ScrHandle);
	
    // Keyboard shortcuts
    GfuiMenuDefaultKeysAdd(ScrHandle);
    rmdsAddKeys();

    GfuiScreenActivate(ScrHandle);
}


static void
rmdsCleanup(void)
{
    trmdDrvElt *curDrv;

    CarCategoryList.clear();
    DriverTypeList.clear();
    SkinNameList.clear();
    PreviewFileList.clear();

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
    GfuiScrollListClear(ScrHandle, CandidatesScrollList);

    // Fill it with drivers that match the filter criteria
    curDrv = GF_TAILQ_FIRST(&DriverList);
    if (curDrv) {
	do {
	    if (!curDrv->isSelected && rmdDriverMatchesFilters(curDrv, carCategory, driverType,
														   AnyCarCategory, AnyDriverType))
		GfuiScrollListInsertElement(ScrHandle, CandidatesScrollList, curDrv->name,
									NbTotDrivers, (void*)curDrv);
	} while ((curDrv = GF_TAILQ_NEXT(curDrv, link)));
    }

    // Show first element if any
    GfuiScrollListShowElement(ScrHandle, CandidatesScrollList, 0);
}
