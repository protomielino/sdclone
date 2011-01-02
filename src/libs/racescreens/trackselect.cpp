/***************************************************************************
                  trackselect.cpp -- interactive track selection
                             -------------------
    created              : Mon Aug 16 21:43:00 CEST 1999
    copyright            : (C) 1999 by Eric Espie
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
    @ingroup    racemantools
    @author <a href=mailto:torcs@free.fr>Eric Espie</a>
    @version    $Id$
*/


#include <sstream>
#include <iterator>
#include <algorithm>
#include <iomanip>

#include <raceman.h>

#include <tgfclient.h>
#include <portability.h>
#include <tracks.h>

#include "racescreens.h"


// Screen handle.
static void *ScrHandle;

// 
static tRmTrackSelect *TrackSelect;

// The currently selected track.
GfTrack* PCurTrack;

// Menu controls.
static int CategoryEditId;
static int NameEditId;
static int OutlineImageId;
static int AuthorLabelId;
static int LengthLabelId;
static int WidthLabelId;
static int DescLine1LabelId;
static int DescLine2LabelId;
static int MaxPitsLabelId;

/** 
 * rmtsWordWrap
 * 
 * Cuts the input string into two, according to the line length given.
 * 
 * @param   str Input string
 * @param   str1    First line should be placed in here
 * @param   str2    Second line should be placed in here (if needed)
 * @param   length  Line length limit where wrapping should occur
 */
static void
rmtsWordWrap(const std::string str, std::string &str1, std::string &str2, unsigned int length)
{
	//istream_iterator iterates through the container
    //using whitespaces as delimiters, so it is an ideal tool
    //for cutting strings into separate words.
    std::istringstream istr(str);
    std::istream_iterator<std::string> it(istr);
    std::istream_iterator<std::string> end;

    //str1 + next word still below line length limit?
    while (it != end && (str1.size() + (*it).size()) < length) {
        str1 += *it;    //concatenate next word and a space to str1
        str1 += " ";    //as the iterator eats the whitespace...
        it++;
    }//while
    
    if(str.size() >= length)    //If input string was longer than required,
        str2 = str.substr(str1.size()); //put the rest in str2.
}//rmtsWordWrap

/** 
 * rmtsGetFirstUsableTrack
 * 
 * Retrieve the first usable track in the given category, searching in the given direction
 * and skipping the first found if specified
 * 
 * @param   strCatId       Id of the category to search inside of.
 * @param   strFromTrackId Id of the track from which to start the search.
 * @param   nSearchDir     <0 = previous, >0 = next.
 * @param   bSkipFrom      If true, skip the first found track.
 */
static GfTrack*
rmtsGetFirstUsableTrack(const std::string& strCatId, const std::string& strFromTrackId = "",
						int nSearchDir = +1, bool bSkipFrom = false)
{
	//GfLogDebug("rmtsGetFirstUsableTrack(c=%s, ft=%s, %d)\n",
	//		   strCatId.c_str(), strFromTrackId.c_str(), nSearchDir);

	GfTrack* pTrack = 0;

	if (nSearchDir == 0)
		nSearchDir = +1;
	
	// Check category.
	const std::vector<std::string>& vecCatIds = GfTracks::self()->getCategoryIds();
	if (std::find(vecCatIds.begin(), vecCatIds.end(), strCatId) == vecCatIds.end())
		return 0;

	// Retrieve tracks in this category.
	const std::vector<GfTrack*> vecTracksInCat =
		GfTracks::self()->getTracksInCategory(strCatId);
	if (vecTracksInCat.size() == 0)
		return 0;
	
	// Retrieve the index of the specified track to start from, if any.
	int nCurTrackInd = 0;
	if (!strFromTrackId.empty())
	{
		std::vector<GfTrack*>::const_iterator itTrack = vecTracksInCat.begin();
		while (itTrack != vecTracksInCat.end())
		{
			if ((*itTrack)->getId() == strFromTrackId)
			{
				nCurTrackInd = itTrack - vecTracksInCat.begin();
				break;
			}
			itTrack++;
		}
	}
	
	int nTrackInd = nCurTrackInd;
	if (bSkipFrom || !vecTracksInCat[nTrackInd]->isUsable())
	{
		const int nPrevTrackInd = nCurTrackInd;
		do
		{
			nTrackInd =
				(nTrackInd + nSearchDir + vecTracksInCat.size()) % vecTracksInCat.size();
			//GfLogDebug("xxx nTrackInd=%d\n", nTrackInd);
		}
		while (nTrackInd != nPrevTrackInd && !vecTracksInCat[nTrackInd]->isUsable());
	}

	if (vecTracksInCat[nTrackInd]->isUsable())
		pTrack = vecTracksInCat[nTrackInd];

	return pTrack;
}
				  
/** 
 * rmtsGetFirstUsableTrack
 * 
 * Retrieve the first usable track among all categories, searching in the given direction
 * from the given category, but skipping it if specified
 * 
 * @param   strFromCatId   Id of the category to search inside of.
 * @param   nSearchDir     <0 = previous, >0 = next.
 * @param   bSkipFrom      If true, skip the first found track.
 */
static GfTrack*
rmtsGetFirstUsableTrack(const std::string& strFromCatId, int nSearchDir, bool bSkipFrom = false)
{
	//GfLogDebug("rmtsGetFirstUsableTrack(fc=%s, %d)\n", strFromCatId.c_str(), nSearchDir);

	GfTrack* pTrack = 0;

	if (nSearchDir == 0)
		nSearchDir = +1;
	
	// Retrieve and check category.
	const std::vector<std::string>& vecCatIds = GfTracks::self()->getCategoryIds();
	std::vector<std::string>::const_iterator itFromCat =
		std::find(vecCatIds.begin(), vecCatIds.end(), strFromCatId);
	if (itFromCat == vecCatIds.end())
		return 0;

	int nCatInd = itFromCat - vecCatIds.begin();

	if (bSkipFrom || !(pTrack = rmtsGetFirstUsableTrack(vecCatIds[nCatInd])))
	{
		const int nPrevCatInd = nCatInd;
		//GfLogDebug("xxx nPrevCatInd=%d\n", nPrevCatInd);
		do
		{
			nCatInd =
				(nCatInd + nSearchDir + vecCatIds.size()) % vecCatIds.size();
			pTrack = rmtsGetFirstUsableTrack(vecCatIds[nCatInd]);
			//GfLogDebug("xxx cat[%d]=%s, p=%p\n", nCatInd, vecCatIds[nCatInd].c_str(), pTrack);
		}
		while (nCatInd != nPrevCatInd && !pTrack);
	}

	return pTrack;
}
	
static void
rmtsUpdateTrackInfo(void)
{
	static const int nMaxLinesLength = 30;  //Line length for track description (chars)

    // Update GUI with track info.
	// 0) Track category and name.
	GfuiLabelSetText(ScrHandle, CategoryEditId, PCurTrack->getCategoryName().c_str());
	GfuiLabelSetText(ScrHandle, NameEditId, PCurTrack->getName().c_str());
	
	// 1) Track description, optionally wrapped in 2 lines
    std::string strDescLine1, strDescLine2;
    rmtsWordWrap(PCurTrack->getDescription(), strDescLine1, strDescLine2, nMaxLinesLength);
    GfuiLabelSetText(ScrHandle, DescLine1LabelId, strDescLine1.c_str());
    GfuiLabelSetText(ScrHandle, DescLine2LabelId, strDescLine2.c_str());

    // 2) Authors
    GfuiLabelSetText(ScrHandle, AuthorLabelId, PCurTrack->getAuthors().c_str());

    // 3) Width.
	std::ostringstream ossData;
	ossData << std::fixed << std::setprecision(0) << PCurTrack->getWidth() << " m";
    GfuiLabelSetText(ScrHandle, WidthLabelId, ossData.str().c_str());
	
    // 4) Length.
	ossData.str("");
	ossData << PCurTrack->getLength() << " m";
    GfuiLabelSetText(ScrHandle, LengthLabelId, ossData.str().c_str());

    // 5) Max number of pits slots.
	ossData.str("");
	if (PCurTrack->getMaxNumOfPitSlots())
		ossData << PCurTrack->getMaxNumOfPitSlots();
	else
		ossData << "None";
	GfuiLabelSetText(ScrHandle, MaxPitsLabelId, ossData.str().c_str());

	// 6) Outline image.
    GfuiStaticImageSet(ScrHandle, OutlineImageId, PCurTrack->getOutlineFile().c_str());

	// 7) Preview image (background).
	GfuiScreenAddBgImg(ScrHandle, PCurTrack->getPreviewFile().c_str());
}

static void
rmtsDeactivate(void *screen)
{
    GfuiScreenRelease(ScrHandle);

    if (screen) {
        GfuiScreenActivate(screen);
    }
}

static void
rmtsActivate(void * /* dummy */)
{
	GfLogTrace("Entering Track Select menu\n");

	// Update GUI (current track).
    rmtsUpdateTrackInfo();
}

/* Select next/previous track from currently selected track category */
static void
rmtsTrackPrevNext(void *vsel)
{
 	const int nSearchDir = vsel ? (int)(long)vsel : +1;

	PCurTrack =
		rmtsGetFirstUsableTrack(PCurTrack->getCategoryId(), PCurTrack->getId(), nSearchDir, true);



	/* Get next/previous usable track
       Note: Here, we assume there's at least one usable track in current category,
       which is guaranteed by CategoryList initialization in RmTrackSelect,
       and by rmtsTrackCatPrevNext */
// 	const int nDeltaInd = (int)(long)(vsel ? vsel : 1);
// 	const std::string& strCurCat = GfTracks::self()->getCategoryIds()[NCurCatIndex];
// 	const std::vector<GfTrack*> vecTracksInCat =
// 		GfTracks::self()->getTracksInCategory(strCurCat);
// 	const int nPrevTrackIndex = NCurTrackInCatIndex;
//     do
// 	{
// 		NCurTrackInCatIndex =
// 			(NCurTrackInCatIndex + nDeltaInd + vecTracksInCat.size()) % vecTracksInCat.size();
// 	}
// 	while (nPrevTrackIndex != NCurTrackInCatIndex && vecTracksInCat[NCurTrackInCatIndex]->isUsable());

	// If any next/previous usable track in the current category, select it.
	//if (nPrevTrackIndex != NCurTrackInCatIndex)
	if (PCurTrack)
	{
		// Set new currently selected track.
		//PCurTrack = vecTracksInCat[NCurTrackCatIndex];
		
		// Update GUI
		rmtsUpdateTrackInfo();
	}
}


/* Select next/previous track category */
static void
rmtsTrackCatPrevNext(void *vsel)
{
 	const int nSearchDir = vsel ? (int)(long)vsel : +1;

	//
	PCurTrack = rmtsGetFirstUsableTrack(PCurTrack->getCategoryId(), +1, true);

	//
	if (PCurTrack)
	{
		// Set new currently selected track.
		//PCurTrack = vecTracksInCat[NCurTrackCatIndex];
		
		// Update GUI
		rmtsUpdateTrackInfo();
	}

// 	/* Get next/previous usable track category
//        Note: Here, we assume there's at least one,
//              which is guaranteed by CategoryList initialization in RmTrackSelect */
// 	const int nDeltaInd = (int)(long)(vsel ? vsel : 1);
// 	const std::string& strCurCat = GfTracks::self()->getCategoryIds()[NCurCatIndex];
// 	const std::vector<GfTrack*> vecTracksInCat =
// 		GfTracks::self()->getTracksInCategory(strCurCat);
// 	const int nPrevTrackIndex = NCurTrackCatIndex;
//     do
// 	{
// 		NCurTrackInCatIndex =
// 			(NCurTrackInCatIndex + nDeltaInd + GfTracks::self()->getCategoryIds())
// 			% GfTracks::self()->getCategoryIds();
// 	}
// 	while (nPrevTrackIndex != NCurTrackInCatIndex
// 		   && vecTracksInCat[NCurTrackCatIndex]->isUsable());


// 	tFList *curCat = CategoryList;
//     do {
    
//     /* Next/previous category */
//     curCat = vsel ? curCat->next : curCat->prev;
    
//     /* Try and get the category display name if not already done */
//     if (curCat->userData && !curCat->dispName) {
//         curCat->dispName = rmtsGetCategoryName(curCat->name);
//         if (!curCat->dispName || strlen(curCat->dispName) == 0) {
// 	    GfError("rmtsTrackCatPrevNext: No definition for track category %s\n", curCat->name);
//         }
//     }
    
//     /* If the category is loadable and not empty : */
//     if (curCat->dispName && curCat->userData) {
        
//         /* Select the first usable track in the category */
//         tFList *curTr = (tFList*)(curCat->userData);
//         do {
    
//         /* Try and get the track display name if not already done */
//         if (!curTr->dispName) {
//             curTr->dispName = rmtsGetTrackName(curCat->name, curTr->name);
//             if (!curTr->dispName || strlen(curTr->dispName) == 0) {
// 		GfError("rmtsTrackCatPrevNext: No definition for track %s\n", curTr->name);
//             }
//         }

//         /* That's all if the track is usable*/
//         if (curTr->dispName)
//             break;
    
//         /* Next track */
//         curTr = curTr->next;
    
//         } while (curTr != (tFList*)(curCat->userData));
//         curCat->userData = (void*)curTr;
//     }

//     } while ((!curCat->dispName || !curCat->userData || !((tFList*)(curCat->userData))->dispName) 
// 	     && curCat != CategoryList);
//     CategoryList = curCat;
    
//     /* Update GUI */
//     GfuiLabelSetText(ScrHandle, CategoryEditId, CategoryList->dispName);
//     GfuiLabelSetText(ScrHandle, NameEditId, ((tFList*)curCat->userData)->dispName);
//     GfuiStaticImageSet(ScrHandle, OutlineImageLabelId, rmtsGetOutlineFileName(PathBuf, MaxPathSize));
//     GfuiScreenAddBgImg(ScrHandle, rmtsGetPreviewFileName(PathBuf, MaxPathSize));
//     rmtsUpdateTrackInfo();
}


static void
rmtsSelect(void * /* dummy */)
{
	// Save currently slected track category and LabelId to the race params file.
    const int nCurTrackInd =
		(int)GfParmGetNum(TrackSelect->param, RM_SECT_TRACKS, RE_ATTR_CUR_TRACK, NULL, 1);
	std::ostringstream ossParamPath;
	ossParamPath << RM_SECT_TRACKS << '/' << nCurTrackInd;
	
    GfParmSetStr(TrackSelect->param, ossParamPath.str().c_str(), RM_ATTR_CATEGORY,
				 PCurTrack->getCategoryId().c_str());
    GfParmSetStr(TrackSelect->param, ossParamPath.str().c_str(), RM_ATTR_NAME,
				 PCurTrack->getId().c_str());
    GfParmSetStr(TrackSelect->param, ossParamPath.str().c_str(), RM_ATTR_FULLNAME,
				 PCurTrack->getId().c_str());

    rmtsDeactivate(TrackSelect->nextScreen);
}


static void
rmtsAddKeys(void)
{
    GfuiAddKey(ScrHandle, GFUIK_RETURN, "Select Track", NULL, rmtsSelect, NULL);
    GfuiAddKey(ScrHandle, GFUIK_ESCAPE, "Cancel Selection", TrackSelect->prevScreen, rmtsDeactivate, NULL);
    GfuiAddKey(ScrHandle, GFUIK_LEFT, "Previous Track", (void*)0, rmtsTrackPrevNext, NULL);
    GfuiAddKey(ScrHandle, GFUIK_RIGHT, "Next Track", (void*)1, rmtsTrackPrevNext, NULL);
    GfuiAddKey(ScrHandle, GFUIK_F1, "Help", ScrHandle, GfuiHelpScreen, NULL);
    GfuiAddKey(ScrHandle, GFUIK_F12, "Screen-Shot", NULL, GfuiScreenShot, NULL);
    GfuiAddKey(ScrHandle, GFUIK_UP, "Previous Track Category", (void*)0, rmtsTrackCatPrevNext, NULL);
    GfuiAddKey(ScrHandle, GFUIK_DOWN, "Next Track Category", (void*)1, rmtsTrackCatPrevNext, NULL);
}


/** Interactive track selection
    @param  vs  Pointer on a tRmTrackSelect structure (cast to void *)
    @warning    The race manager's parameters are updated but not saved to the file.
    @ingroup    racemantools
 */
void
RmTrackSelect(void *vs)
{
    TrackSelect = (tRmTrackSelect*)vs;

    // Get currently selected track for the current race type
	// (or the first usable one in the selected category).
    const int nCurTrackInd =
		(int)GfParmGetNum(TrackSelect->param, RM_SECT_TRACKS, RE_ATTR_CUR_TRACK, NULL, 1);
	std::ostringstream ossSectionPath;
	ossSectionPath << RM_SECT_TRACKS << '/' << nCurTrackInd;
    const char* pszCurTrackId =
		GfParmGetStr(TrackSelect->param, ossSectionPath.str().c_str(), RM_ATTR_NAME, 0);
	const char* pszCurTrackCatId =
		GfParmGetStr(TrackSelect->param, ossSectionPath.str().c_str(), RM_ATTR_CATEGORY, 0);

	PCurTrack = rmtsGetFirstUsableTrack(pszCurTrackCatId, pszCurTrackId);
	if (PCurTrack && PCurTrack->getId() != pszCurTrackId)
        GfLogWarning("Could not find selected track %s (%s) ; using %s (%s)\n",
					 pszCurTrackId, pszCurTrackCatId,
					 PCurTrack->getId().c_str(), PCurTrack->getCategoryId().c_str());

    // If not usable, try and get the first usable track going ahead in categories
	if (!PCurTrack)
	{
		PCurTrack = rmtsGetFirstUsableTrack(pszCurTrackCatId, +1, true);
		if (PCurTrack)
			GfLogWarning("Could not find selected track %s and category %s unusable"
						 " ; using %s (%s)\n",
						 pszCurTrackId, pszCurTrackCatId,
						 PCurTrack->getId().c_str(), PCurTrack->getCategoryId().c_str());
	}
	
    // If no usable category/track found, ... return
	if (!PCurTrack)
	{
        GfLogError("No available track for any category ; quitting Track Select menu\n");
        return; // or exit(1) abruptly ?
    }

	// Create screen menu and controls.
    ScrHandle =
		GfuiScreenCreateEx((float*)NULL, NULL, rmtsActivate, NULL, (tfuiCallback)NULL, 1);

    void *hparmMenu = LoadMenuXML("trackselectmenu.xml");
    CreateStaticControls(hparmMenu, ScrHandle);

    CreateButtonControl(ScrHandle, hparmMenu, "trackcatleftarrow",(void*)-1, rmtsTrackCatPrevNext);
    CreateButtonControl(ScrHandle, hparmMenu, "trackcatrightarrow",(void*)1, rmtsTrackCatPrevNext);
    CategoryEditId = CreateLabelControl(ScrHandle, hparmMenu, "trackcatlabel");

    CreateButtonControl(ScrHandle, hparmMenu, "trackleftarrow", (void*)-1, rmtsTrackPrevNext);
    CreateButtonControl(ScrHandle, hparmMenu, "trackrightarrow", (void*)1, rmtsTrackPrevNext);
    NameEditId = CreateLabelControl(ScrHandle, hparmMenu, "tracklabel");

    OutlineImageId = CreateStaticImageControl(ScrHandle, hparmMenu, "outlineimage");

    CreateButtonControl(ScrHandle, hparmMenu, "nextbutton", NULL, rmtsSelect);
    CreateButtonControl(ScrHandle, hparmMenu, "previousbutton", TrackSelect->prevScreen, rmtsDeactivate);

    DescLine1LabelId = CreateLabelControl(ScrHandle, hparmMenu, "descriptionlabel");
    DescLine2LabelId = CreateLabelControl(ScrHandle, hparmMenu, "descriptionlabel2");
    LengthLabelId = CreateLabelControl(ScrHandle, hparmMenu, "lengthlabel");
    WidthLabelId = CreateLabelControl(ScrHandle, hparmMenu, "widthlabel");
    MaxPitsLabelId = CreateLabelControl(ScrHandle, hparmMenu, "pitslabel");
    AuthorLabelId = CreateLabelControl(ScrHandle, hparmMenu, "authorlabel");

    GfParmReleaseHandle(hparmMenu);

    rmtsAddKeys();

    GfuiScreenActivate(ScrHandle);
}
