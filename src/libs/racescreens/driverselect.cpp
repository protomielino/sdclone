/***************************************************************************
                  driverselect.cpp -- drivers interactive selection                              
                             -------------------                                         
    created              : Mon Aug 16 20:40:44 CEST 1999
    copyright            : (C) 1999 by Eric Espie, 2009 Jean-Philippe Meuret
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

/** @file
    		Driver selection menu.
    @ingroup	racemantools
    @author	<a href=mailto:torcs@free.fr>Eric Espie</a>
    @version	$Id$
*/


#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <algorithm>

#include <tgfclient.h>
#include <track.h>
#include <car.h>
#include <raceman.h>
#include <robot.h>

#include <cars.h>
#include <drivers.h>
#include <race.h>

#include "racescreens.h"
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
static int		RemoveAllButtonId, SelectRandomButtonId, ShuffleButtonId;
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

// Car categories
static const char* AnyCarCategory = "--- All car categories ---";
static std::vector<std::string> VecCarCategoryIds; // Category folder/file names
static std::vector<std::string> VecCarCategoryNames; // Category real/displayed names
static size_t CurCarCategoryIndex = 0;

// Driver types
static const char* AnyDriverType = "--- All driver types ---";
static std::vector<std::string> VecDriverTypes;
static size_t CurDriverTypeIndex = 0;

// Skin names, targets and associated preview files for the currently selected driver. 
static std::vector<GfDriverSkin> VecCurDriverPossSkins;
static size_t CurSkinIndex = 0;

// The current race.
GfRace TheRace;

GfDriver* PPickedDriver;

// Local functions.
static void rmdsCleanup(void);
static void rmdsFilterCandidatesScrollList(const std::string& strCarCatId,
											const std::string& strType);
static GfDriver* rmdsGetHighlightedDriver();
static bool rmdsIsAnyCompetitorHighlighted();
static void rmdsClickOnDriver(void * /* dummy */);


static bool
rmdsIsAnyCompetitorHighlighted()
{
	GfDriver *pDriver;

    const char* name =
		GfuiScrollListGetSelectedElement(ScrHandle, CompetitorsScrollListId, (void**)&pDriver);

	return name != 0;
}

static GfDriver*
rmdsGetHighlightedDriver()
{
	GfDriver *pDriver;
	
	const char *name =
		GfuiScrollListGetSelectedElement(ScrHandle, CompetitorsScrollListId, (void**)&pDriver);
    if (!name)
		name = GfuiScrollListGetSelectedElement(ScrHandle, CandidatesScrollListId, (void**)&pDriver);
    if (!name)
		pDriver = 0;

	return pDriver;
}

static void
rmdsHighlightDriver(const GfDriver* pDriver)
{
	if (!pDriver)
		return;

	// Search first in the competitors scroll-list.
	GfDriver* pCompetitor;
	int index = 0;
	while (GfuiScrollListGetElement(ScrHandle, CompetitorsScrollListId, index, (void**)&pCompetitor))
	{
		if (pCompetitor == pDriver)
		{
			//GfLogDebug("Selecting competitor #%d '%s'\n", curDrvIndex, pCompetitor->getName().c_str());
			GfuiScrollListSetSelectedElement(ScrHandle, CompetitorsScrollListId, index);
			rmdsClickOnDriver(0);
			return;
		}
		index++;
	}
	
	// Then in the candidates scroll-list.
	GfDriver* pCandidate;
	index = 0;
	while (GfuiScrollListGetElement(ScrHandle, CandidatesScrollListId, index, (void**)&pCandidate))
	{
		if (pCandidate == pDriver)
		{
			//GfLogDebug("Selecting candidate #%d '%s'\n", curDrvIndex, name);
			GfuiScrollListSetSelectedElement(ScrHandle, CandidatesScrollListId, index);
			rmdsClickOnDriver(0);
			return;
		}
		index++;
	}
}

static void
rmdsReloadCompetitorsScrollList()
{
	GfuiScrollListClear(ScrHandle, CompetitorsScrollListId);

	// For each competitor in the race :
	std::vector<GfDriver*> vecCompetitors = TheRace.getCompetitors();
	std::vector<GfDriver*>::iterator itComp;
	for (itComp = vecCompetitors.begin(); itComp != vecCompetitors.end(); itComp++)
		// Add its name to the Competitors scroll list.
		GfuiScrollListInsertElement(ScrHandle, CompetitorsScrollListId, (*itComp)->getName().c_str(),
									TheRace.getCompetitorsCount(), (void*)(*itComp));
}

// Screen activation call-back.
static void
rmdsActivate(void * /* notused */)
{
	GfLogTrace("Entering Driver Select menu\n");

	// Update GUI : picked driver info, car preview.
	rmdsHighlightDriver(PPickedDriver);
	
    // Initialize the driver type filter criteria to "any driver".
    CurDriverTypeIndex =
		(std::find(VecDriverTypes.begin(), VecDriverTypes.end(), AnyDriverType)
		 - VecDriverTypes.begin()) % VecDriverTypes.size();

	// Initialize the car category filter criteria : use the one of the current driver if any.
	const std::string strCurCarCatId =
		PPickedDriver ? PPickedDriver->getCar()->getCategoryId() : AnyCarCategory;
    CurCarCategoryIndex =
		(std::find(VecCarCategoryIds.begin(), VecCarCategoryIds.end(), strCurCarCatId)
		 - VecCarCategoryIds.begin()) % VecCarCategoryIds.size();

	// Update GUI (candidate list, filter criteria).
    GfuiLabelSetText(ScrHandle, DriverTypeEditId, VecDriverTypes[CurDriverTypeIndex].c_str());
    GfuiLabelSetText(ScrHandle, CarCategoryEditId, VecCarCategoryNames[CurCarCategoryIndex].c_str());
    rmdsFilterCandidatesScrollList(strCurCarCatId, VecDriverTypes[CurDriverTypeIndex]);
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
		(CurCarCategoryIndex + VecCarCategoryIds.size() + (int)(long)vp) % VecCarCategoryIds.size();

    GfuiLabelSetText(ScrHandle, CarCategoryEditId, VecCarCategoryNames[CurCarCategoryIndex].c_str());

    rmdsFilterCandidatesScrollList(VecCarCategoryIds[CurCarCategoryIndex],
							   VecDriverTypes[CurDriverTypeIndex]);

	if (rmdsIsAnyCompetitorHighlighted())
		GfuiEnable(ScrHandle, ChangeCarButtonId, GFUI_ENABLE);
}

static void
rmdsChangeDriverType(void *vp)
{
 	CurDriverTypeIndex =
		(CurDriverTypeIndex + VecDriverTypes.size() + (int)(long)vp) % VecDriverTypes.size();

    GfuiLabelSetText(ScrHandle, DriverTypeEditId, VecDriverTypes[CurDriverTypeIndex].c_str());

    rmdsFilterCandidatesScrollList(VecCarCategoryIds[CurCarCategoryIndex],
							   VecDriverTypes[CurDriverTypeIndex]);

	if (rmdsIsAnyCompetitorHighlighted())
		GfuiEnable(ScrHandle, ChangeCarButtonId, GFUI_ENABLE);
}

static void
rmdsChangeSkin(void *vp)
{
	if (VecCurDriverPossSkins.empty())
		return; // Should never happens (see GfDriver::getPossibleSkins).

	// Update skin combo-box.
 	CurSkinIndex = (CurSkinIndex + VecCurDriverPossSkins.size()
					+ (int)(long)vp) % VecCurDriverPossSkins.size();

	const GfDriverSkin& curSkin = VecCurDriverPossSkins[CurSkinIndex];
	const std::string strCurSkinDispName =
		curSkin.getName().empty() ? "standard " : curSkin.getName();
    GfuiLabelSetText(ScrHandle, SkinEditId, strCurSkinDispName.c_str());

	// Load associated preview image (or "no preview" panel if none available).
	if (GfFileExists(curSkin.getCarPreviewFileName().c_str()))
		GfuiStaticImageSet(ScrHandle, CarImageId,
						   curSkin.getCarPreviewFileName().c_str(),
						   /* index= */ 0, /* canDeform= */false);
	else
		GfuiStaticImageSet(ScrHandle, CarImageId, "data/img/nocarpreview.png");

	// Update highlighted driver skin.
	GfDriver *pDriver = rmdsGetHighlightedDriver();
	if (pDriver)
		pDriver->setSkin(curSkin); // TODO: Can we do this for the whole game session (not only this race if it ever starts) ?
}

#ifdef FOCUS
static void
rmdsSetFocus(void * /* dummy */)
{
    const char	*name;
    GfDriver	*pDriver;

    name = GfuiScrollListGetSelectedElement(ScrHandle, CompetitorsScrollListId, (void**)&pDriver);
    if (name) {
		GfParmSetStr(MenuData->param, RM_SECT_DRIVERS, RM_ATTR_FOCUSED,
					 pDriver->getModuleName().c_str());
		GfParmSetNum(MenuData->param, RM_SECT_DRIVERS, RM_ATTR_FOCUSEDIDX,
					 (char*)NULL, pDriver->getInterfaceIndex());
		GfuiLabelSetText(ScrHandle, FocusedDriverLabelId, pDriver->getName().c_str());
    }
}
#endif

static void
rmdsNextMenu(void * /* dummy */)
{
	// Save the race data to its params file
	TheRace.save();

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
	if (PPickedDriver)
	{
		CarSelectMenu.SetPreviousMenuHandle(pPreviousMenu);
		CarSelectMenu.RunMenu(PPickedDriver);
	}
}

static void
rmdsMoveCompetitor(void *vd)
{
    GfuiScrollListMoveSelectedElement(ScrHandle, CompetitorsScrollListId, (long)vd);
}

static void
rmdsClickOnDriver(void * /* dummy */)
{
	// Determine which list and which driver were clicked on.
	GfDriver* pDriver = 0;
    if (GfuiScrollListGetSelectedElement(ScrHandle, CompetitorsScrollListId, (void**)&pDriver))
	{
		// A driver from the Competitors scroll-list.
		GfuiEnable(ScrHandle, SelectButtonId, GFUI_DISABLE);
		GfuiEnable(ScrHandle, DeselectButtonId, GFUI_ENABLE);
		GfuiEnable(ScrHandle, ChangeCarButtonId, GFUI_ENABLE);
		GfuiVisibilitySet(ScrHandle, SkinEditId, GFUI_VISIBLE);
	}
    else if (GfuiScrollListGetSelectedElement(ScrHandle, CandidatesScrollListId, (void**)&pDriver))
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
    if (pDriver)
	{
		//GfLogDebug("rmdsClickOnDriver: '%s'\n", pDriver->getName().c_str());
		
		// The selected driver is the new picked one.
		PPickedDriver = pDriver;

		// Update picked driver info.
		GfuiLabelSetText(ScrHandle, PickedDriverTypeLabelId, pDriver->getType().c_str());
		const GfCar* pCar = pDriver->getCar();
		GfuiLabelSetText(ScrHandle, PickedDriverCarLabelId, pCar->getName().c_str());
		GfuiLabelSetText(ScrHandle, PickedDriverCarCategoryLabelId, pCar->getCategoryId().c_str());
		
		// Get really available skins (the user may have changed some file somewhere
		// since last time we got here for this driver).
		VecCurDriverPossSkins = pDriver->getPossibleSkins();
		
		// Determine the index of the currently selected skin for this driver.
		CurSkinIndex = 0;
		std::vector<GfDriverSkin>::iterator itSkin =
			GfDriver::findSkin(VecCurDriverPossSkins, pDriver->getSkin().getName());
		if (itSkin != VecCurDriverPossSkins.end())
			CurSkinIndex = itSkin - VecCurDriverPossSkins.begin();

		const int skinButtonsEnabled =
			VecCurDriverPossSkins.size() > 1 ? GFUI_ENABLE : GFUI_DISABLE;
		GfuiEnable(ScrHandle, SkinRightButtonId, skinButtonsEnabled);
		GfuiEnable(ScrHandle, SkinLeftButtonId, skinButtonsEnabled);

		// Update driver skin and show it in the GUI.
		rmdsChangeSkin(0);
    }
}

static void
rmdsSelectDeselectDriver(void * /* dummy */ )
{
    const char* name;
    int	src, dst;
    GfDriver *pDriver;
    int	sel;

    // If the selected driver is in the Candidate scroll-list,
    // and if the max number of selected drivers has not been reached,
    // remove the driver from the Candidate scroll-list,
	// and add him to the Competitors scroll-list and to the race competitors.
    sel = 0;
    name = 0;
    if (TheRace.acceptsMoreCompetitors()) {
		src = CandidatesScrollListId;
		name = GfuiScrollListExtractSelectedElement(ScrHandle, src, (void**)&pDriver);
		if (name) {
			dst = CompetitorsScrollListId;
			GfuiScrollListInsertElement(ScrHandle, dst, name, GfDrivers::self()->getCount(), (void*)pDriver);
			TheRace.appendCompetitor(pDriver); // Now selected.
		}
    }

    // Otherwise, if the selected driver is in the Competitors scroll-list,
    // remove the driver from the Competitors scroll-list and from the race competitors,
	// and add him to the Candidate scroll-list
    // (if it matches the Candidate scroll-list filtering criteria)
    if (!name) {
		sel = 1;
		src = CompetitorsScrollListId;
		name = GfuiScrollListExtractSelectedElement(ScrHandle, src, (void**)&pDriver);
		if (name) {
			const std::string strCarCatIdFilter =
				(VecCarCategoryIds[CurCarCategoryIndex] == AnyCarCategory
				 ? "" : VecCarCategoryIds[CurCarCategoryIndex]);
			const std::string strTypeFilter =
				(VecDriverTypes[CurDriverTypeIndex] == AnyDriverType
				 ? "" : VecDriverTypes[CurDriverTypeIndex]);
			if (pDriver->matchesTypeAndCategory(strTypeFilter, strCarCatIdFilter)) {
				dst = CandidatesScrollListId;
				GfuiScrollListInsertElement(ScrHandle, dst, name,
											pDriver->isHuman() ? 0 : GfDrivers::self()->getCount(), (void*)pDriver);
			}
			TheRace.removeCompetitor(pDriver); // No more selected.
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
		if (pDriver->getInterfaceIndex() == robotIdx && pDriver->getModuleName() == modName) {
			/* the focused element was deselected : select a new one */
			name = GfuiScrollListGetElement(ScrHandle, CompetitorsScrollListId, 0, (void**)&pDriver);
			if (name) {
				GfParmSetStr(MenuData->param, RM_SECT_DRIVERS, RM_ATTR_FOCUSED, pDriver->getModuleName().c_str());
				GfParmSetNum(MenuData->param, RM_SECT_DRIVERS, RM_ATTR_FOCUSEDIDX, (char*)NULL, pDriver->getInterfaceIndex());
#ifdef FOCUS
				GfuiLabelSetText(ScrHandle, FocusedDriverLabelId, pDriver->getName.c_str());
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
		if (strlen(modName) == 0 || pDriver->isHuman()) {
			GfParmSetStr(MenuData->param, RM_SECT_DRIVERS, RM_ATTR_FOCUSED, pDriver->getModuleName().c_str());
			GfParmSetNum(MenuData->param, RM_SECT_DRIVERS, RM_ATTR_FOCUSEDIDX, (char*)NULL, pDriver->getInterfaceIndex());
#ifdef FOCUS
			GfuiLabelSetText(ScrHandle, FocusedDriverLabelId, pDriver->getName().c_str());
#endif
		}
    }

    // Update selected driver displayed info.
    rmdsClickOnDriver(0);

    // Don't allow user to Accept 0 drivers, this would cause a crash.
    GfuiEnable(ScrHandle, NextButtonId, TheRace.getCompetitorsCount() > 0 ? GFUI_ENABLE : GFUI_DISABLE);

	// For a smart display refresh, when automatically called multiple time.
    GfuiDisplay();
}

static void
rmdsRemoveAllCompetitors(void * /* dummy */ )
{
	// Take care that no candidate is selected (see why in rmdsSelectDeselectDriver).
	GfuiScrollListClearSelection(ScrHandle, CandidatesScrollListId);
	
	// Deselect the first competitors until there's none left.
	int nCompetitors;
	while ((nCompetitors = GfuiScrollListGetNumberOfElements(ScrHandle, CompetitorsScrollListId)) > 0)
	{
		// Select the firstd element of the Competitors scroll-list.
		GfuiScrollListSetSelectedElement(ScrHandle, CompetitorsScrollListId, 0);

		// Do the normal deselection job (just as if the user had manually done it).
		rmdsSelectDeselectDriver(0);
	}
}

static void
rmdsSelectRandomCandidates(void * /* dummy */ )
{
	// Max number of randomly selected candidates.
	static const unsigned nRandomCompetitors = 5;

	// Take care that no competitor is selected (see why in rmdsSelectDeselectDriver).
	GfuiScrollListClearSelection(ScrHandle, CompetitorsScrollListId);
	
	// Select as many random candidates as possible.
	unsigned nCount = 1;
	int nCandidates;
	while (nCount <= nRandomCompetitors
		   && TheRace.acceptsMoreCompetitors()
		   && (nCandidates = GfuiScrollListGetNumberOfElements(ScrHandle, CandidatesScrollListId)) > 0)
	{
		// Pick-up a random candidate from the candidate scroll-list.
		const unsigned nPickedCandInd = rand() % nCandidates;

		// Make it the selected element of the Candidate scroll-list.
		GfuiScrollListSetSelectedElement(ScrHandle, CandidatesScrollListId, nPickedCandInd);

		// Do the normal selection job (just as if the user had manually done it).
		rmdsSelectDeselectDriver(0);
		
		// Next random candidate.
		nCount++;
	}
}

static void
rmdsShuffleCompetitors(void * /* dummy */ )
{
	// Save currently highlighted driver.
	const GfDriver* pDriver = rmdsGetHighlightedDriver();
	
	// Shuffle the race competitor list and reload the scroll-list.
	TheRace.shuffleCompetitors();
	rmdsReloadCompetitorsScrollList();

    // Re-highlight the previously highlighted driver.
	rmdsHighlightDriver(pDriver);
}

static void
rmdsAddKeys(void)
{
    GfuiAddKey(ScrHandle, GFUIK_ESCAPE, "Previous menu", MenuData->prevScreen, rmdsPreviousMenu, NULL);
    GfuiAddKey(ScrHandle, GFUIK_RETURN, "Next menu", NULL, rmdsNextMenu, NULL);
    GfuiAddKey(ScrHandle, GFUIK_F1, "Help", ScrHandle, GfuiHelpScreen, NULL);
    GfuiAddKey(ScrHandle, GFUIK_F12, "Screen-Shot", NULL, GfuiScreenShot, NULL);
    GfuiAddKey(ScrHandle, '-', "Move Up", (void*)-1, rmdsMoveCompetitor, NULL);
    GfuiAddKey(ScrHandle, '+', "Move Down", (void*)+1, rmdsMoveCompetitor, NULL);
    GfuiAddKey(ScrHandle, ' ', "Select/Deselect", NULL, rmdsSelectDeselectDriver, NULL);
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
    // Initialize drivers selection
    MenuData = (tRmDriverSelect*)vs;

    // Create screen, background image and title
    ScrHandle = GfuiScreenCreateEx((float*)NULL, NULL, rmdsActivate, NULL, (tfuiCallback)NULL, 1);
    
    void *menuDescHdle = LoadMenuXML("driverselectmenu.xml");
    CreateStaticControls(menuDescHdle, ScrHandle);

    CompetitorsScrollListId = CreateScrollListControl(ScrHandle, menuDescHdle, "competitorsscrolllist", NULL, rmdsClickOnDriver);
    CandidatesScrollListId = CreateScrollListControl(ScrHandle, menuDescHdle, "candidatesscrolllist", NULL, rmdsClickOnDriver);

	
    // Car category filtering "combobox" (left arrow, label, right arrow)
    CreateButtonControl(ScrHandle, menuDescHdle, "carcategoryleftarrow", (void*)-1, rmdsChangeCarCategory);
    CreateButtonControl(ScrHandle, menuDescHdle, "carcategoryrightarrow", (void*)1, rmdsChangeCarCategory);
    CarCategoryEditId = CreateLabelControl(ScrHandle, menuDescHdle, "carcategorytext");
    
    // Driver type filtering "combobox" (left arrow, label, right arrow)
    CreateButtonControl(ScrHandle, menuDescHdle, "drivertypeleftarrow", (void*)-1, rmdsChangeDriverType);
    CreateButtonControl(ScrHandle, menuDescHdle, "drivertyperightarrow", (void*)1, rmdsChangeDriverType);
    DriverTypeEditId = CreateLabelControl(ScrHandle, menuDescHdle, "drivertypetext");

    // Scroll-lists manipulation buttons
    CreateButtonControl(ScrHandle, menuDescHdle, "moveupbutton", (void*)-1, rmdsMoveCompetitor);
    CreateButtonControl(ScrHandle, menuDescHdle, "movedownbutton", (void*)1, rmdsMoveCompetitor);
	
    SelectButtonId = CreateButtonControl(ScrHandle, menuDescHdle, "selectbutton", 0, rmdsSelectDeselectDriver);
    DeselectButtonId = CreateButtonControl(ScrHandle, menuDescHdle, "deselectbutton", 0, rmdsSelectDeselectDriver);
    RemoveAllButtonId = CreateButtonControl(ScrHandle, menuDescHdle, "removeallbutton", 0, rmdsRemoveAllCompetitors);
    SelectRandomButtonId = CreateButtonControl(ScrHandle, menuDescHdle, "selectrandombutton", 0, rmdsSelectRandomCandidates);
    ShuffleButtonId = CreateButtonControl(ScrHandle, menuDescHdle, "shufflebutton", 0, rmdsShuffleCompetitors);

    // Skin selection "combobox" (left arrow, label, right arrow)
    SkinLeftButtonId = CreateButtonControl(ScrHandle, menuDescHdle, "skinleftarrow", (void*)-1, rmdsChangeSkin);
    SkinRightButtonId = CreateButtonControl(ScrHandle, menuDescHdle, "skinrightarrow", (void*)1, rmdsChangeSkin);
    SkinEditId = CreateLabelControl(ScrHandle, menuDescHdle, "skintext");

    // Car preview image
    CarImageId = CreateStaticImageControl(ScrHandle, menuDescHdle, "carpreviewimage");
    GfuiStaticImageSet(ScrHandle, CarImageId, "data/img/nocarpreview.png");

	// Initialize the car category Ids, names and driver types lists for the driver filter system.
	VecCarCategoryIds = GfCars::self()->getCategoryIds();
    VecCarCategoryIds.push_back(AnyCarCategory);
	
	VecCarCategoryNames = GfCars::self()->getCategoryNames();
    VecCarCategoryNames.push_back(AnyCarCategory);

	VecDriverTypes = GfDrivers::self()->getTypes();
    VecDriverTypes.push_back(AnyDriverType);
	
    // Picked Driver Info
    PickedDriverTypeLabelId =
		CreateLabelControl(ScrHandle, menuDescHdle, "pickeddrivertypelabel");
    PickedDriverCarCategoryLabelId =
		CreateLabelControl(ScrHandle, menuDescHdle, "pickeddrivercarcategorylabel");
    PickedDriverCarLabelId =
		CreateLabelControl(ScrHandle, menuDescHdle, "pickeddrivercarlabel");
    
    // Next, Previous and Change Car buttons
    NextButtonId =
		CreateButtonControl(ScrHandle, menuDescHdle, "nextmenubutton", NULL, rmdsNextMenu);
    CreateButtonControl(ScrHandle, menuDescHdle, "previousmenubutton",
						MenuData->prevScreen, rmdsPreviousMenu);
    ChangeCarButtonId = CreateButtonControl(ScrHandle, menuDescHdle, "carselectbutton",
											ScrHandle, rmdsCarSelectMenu);
	GfuiEnable(ScrHandle, ChangeCarButtonId, GFUI_DISABLE);

    GfParmReleaseHandle(menuDescHdle);

    // Keyboard shortcuts
    GfuiMenuDefaultKeysAdd(ScrHandle);
    rmdsAddKeys();

    // Load the race data from the params file.
	TheRace.load(MenuData->param);

	// Fill-in the competitors scroll-list.
	rmdsReloadCompetitorsScrollList();
	
	// Initialize the currently highlighted driver.
	PPickedDriver = 0;
	std::vector<GfDriver*> vecCompetitors = TheRace.getCompetitors();
	std::vector<GfDriver*>::iterator itComp;
	for (itComp = vecCompetitors.begin(); itComp != vecCompetitors.end(); itComp++)
		// Initialize the picked driver (the last human driver, or else of the last driver).
		if (!PPickedDriver || (*itComp)->isHuman())
			PPickedDriver = *itComp;

	// Display the menu.
    GfuiScreenActivate(ScrHandle);
}


static void
rmdsCleanup(void)
{
    VecCarCategoryIds.clear();
    VecCarCategoryNames.clear();
    VecDriverTypes.clear();
    VecCurDriverPossSkins.clear();

    TheRace.clear();
}

static void
rmdsFilterCandidatesScrollList(const std::string& strCarCatId, const std::string& strType)
{

    // Empty the unselected scroll-list
    GfuiScrollListClear(ScrHandle, CandidatesScrollListId);
	GfuiEnable(ScrHandle, SelectButtonId, GFUI_DISABLE);

    // Fill it with drivers that match the filter criteria and are not among competitors.
	const std::vector<GfDriver*>& vecCompetitors = TheRace.getCompetitors();
	const std::string strCarCatIdFilter = (strCarCatId == AnyCarCategory ? "" : strCarCatId);
	const std::string strTypeFilter = (strType == AnyDriverType ? "" : strType);
	const std::vector<GfDriver*> vecCandidates =
		GfDrivers::self()->getDriversWithTypeAndCategory(strTypeFilter, strCarCatIdFilter);
	std::vector<GfDriver*>::const_iterator itCandidate;
	int index = 1;
	for (itCandidate = vecCandidates.begin(); itCandidate != vecCandidates.end(); itCandidate++)
	{
		if (std::find(vecCompetitors.begin(), vecCompetitors.end(), *itCandidate)
			== vecCompetitors.end())
			GfuiScrollListInsertElement(ScrHandle, CandidatesScrollListId,
										(*itCandidate)->getName().c_str(), index++,
										(void*)(*itCandidate));
    }

    // Show first element if any
    GfuiScrollListShowElement(ScrHandle, CandidatesScrollListId, 0);
}
