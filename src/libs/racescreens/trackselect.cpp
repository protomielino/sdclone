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


#include <stdlib.h>
#include <stdio.h>
#include <cstring>
#include <string>
#include <sstream>
#include <iterator>
#ifdef WIN32
#include <windows.h>
#endif
#include <portability.h>
#include <tgfclient.h>
#include <track.h>
#include <raceman.h>
#include "racescreens.h"

#define DESC_LINE_LENGTH    21  //Line length for track description

/* Tracks Categories */
static tFList *CategoryList;
static void *ScrHandle;
static int TrackLabelId;
static int CatLabelId;
//static int MapId;
static int AuthorId;
static int LengthId;
static int WidthId;
static int DescId;
static int Desc2Id;
static int PitsId;
static tRmTrackSelect *TrackSelect;


static void rmtsActivate(void * /* dummy */)
{
    /* call display function of graphic */
    //gfuiReleaseImage(MapId);
}


static void rmtsFreeLists(void *vl)
{
    GfDirFreeList((tFList*)(((tFList*)vl)->userData), NULL, true, true);
}


static char * rmtsGetMapName(char* mapNameBuf, unsigned mapNameBufSize)
{
    snprintf(mapNameBuf, mapNameBufSize, "tracks/%s/%s/%s.png", CategoryList->name,
         ((tFList*)CategoryList->userData)->name, ((tFList*)CategoryList->userData)->name);
    mapNameBuf[mapNameBufSize-1] = 0; /* snprinf manual is not clear about that ... */
    return mapNameBuf;
}


static void rmtsDeactivate(void *screen)
{
    GfuiScreenRelease(ScrHandle);

    GfDirFreeList(CategoryList, rmtsFreeLists, true, true);
    if (screen) {
        GfuiScreenActivate(screen);
    }
}

/** 
 * WordWrap
 * 
 * Cuts the input string into two, according to the line length given.
 * 
 * @param   str Input string
 * @param   str1    First line should be placed in here
 * @param   str2    Second line should be placed in here (if needed)
 * @param   length  Line length limit where wrapping should occur
 */
static void
WordWrap(const std::string str, std::string &str1, std::string &str2, unsigned int length)
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
}//WordWrap


static void rmtsUpdateTrackInfo(void)
{
    const unsigned bufSize = 256;
    static char buf[bufSize];

    void *trackHandle;
    float tmp;
    tTrack *trk;

    /* Try and load track XML file */
    sprintf(buf, "tracks/%s/%s/%s.%s", CategoryList->name, ((tFList*)CategoryList->userData)->name,
        ((tFList*)CategoryList->userData)->name, TRKEXT);
    trackHandle = GfParmReadFile(buf, GFPARM_RMODE_STD);
    if (!trackHandle) {
        GfError("Could not load track file %s\n", buf);
        return;
    }

    /* Try and load track 3D model (never fails ?) */
    trk = TrackSelect->trackItf.trkBuild(buf);
    if (!trk) {
        GfError("Could not load track 3D model\n");
        GfParmReleaseHandle(trackHandle);
        return;
    }

    /* Update GUI with track info */
    std::string strDesc = GfParmGetStr(trackHandle, TRK_SECT_HDR, TRK_ATT_DESCR, "");

    std::string str1, str2;
    WordWrap(strDesc, str1, str2, DESC_LINE_LENGTH);
    GfuiLabelSetText(ScrHandle, DescId, str1.c_str());
    GfuiLabelSetText(ScrHandle, Desc2Id, str2.c_str());

    GfuiLabelSetText(ScrHandle, AuthorId, GfParmGetStr(trackHandle, TRK_SECT_HDR, TRK_ATT_AUTHOR, ""));

    tmp = GfParmGetNum(trackHandle, TRK_SECT_MAIN, TRK_ATT_WIDTH, NULL, 0);
    sprintf(buf, "%.2f m", tmp);
    GfuiLabelSetText(ScrHandle, WidthId, buf);
    tmp = trk->length;
    sprintf(buf, "%.2f m", tmp);
    GfuiLabelSetText(ScrHandle, LengthId, buf);

    if (trk->pits.nMaxPits) {
        sprintf(buf, "%d", trk->pits.nMaxPits);
        GfuiLabelSetText(ScrHandle, PitsId, buf);
    } else {
        GfuiLabelSetText(ScrHandle, PitsId, "none");
    }

    /* Unload track 3D model */
    TrackSelect->trackItf.trkShutdown();

    /* Close track XML file */
    GfParmReleaseHandle(trackHandle);
}


/** Get the track name
    @param  category    track category directory
    @param  trackName   track name for file
    @return Long track name
    @ingroup    racemantools
 */
char *
RmGetTrackName(const char *category, const char *trackName)
{
    static char path[256];
    void *trackHandle;
    char *name;

    sprintf(path, "tracks/%s/%s/%s.%s", category, trackName, trackName, TRKEXT);
    trackHandle = GfParmReadFile(path, GFPARM_RMODE_STD);

    if (trackHandle) {
        name = strdup(GfParmGetStr(trackHandle, TRK_SECT_HDR, TRK_ATT_NAME, trackName));
    } else {
        GfTrace("Could not read file %s\n", path);
        return 0;
    }

    GfParmReleaseHandle(trackHandle);
    return name;
}


/** Get the track category name from the directory name
    @param  category    track category directory
    @return category display name
    @ingroup    racemantools
*/
char *
RmGetCategoryName(const char *category)
{
    static char path[256];
    void *categoryHandle;
    char *name;

    sprintf(path, "data/tracks/%s.%s", category, TRKEXT);
    categoryHandle = GfParmReadFile(path, GFPARM_RMODE_STD);

    if (categoryHandle) {
        name = strdup(GfParmGetStr(categoryHandle, TRK_SECT_HDR, TRK_ATT_NAME, category));
    } else {
        GfTrace("Could not read file %s\n", path);
        return 0;
    }

    GfParmReleaseHandle(categoryHandle);
    return name;
}


/* Select next/previous track from currently selected track category */
static void rmtsTrackPrevNext(void *vsel)
{
    const unsigned maxPathSize = 256;
    static char path[maxPathSize];

    /* Get next/previous usable track
       Note: Here, we assume there's at least one usable track in current category,
             which is guaranteed by CategoryList initialization in RmTrackSelect,
             and by rmtsTrackCatPrevNext */
    tFList *curTr = (tFList*)(CategoryList->userData);
    do {
        
        curTr = vsel ? curTr->next : curTr->prev;
        
        /* Try and get the track display name if not already done */
        if (!curTr->dispName) {
        curTr->dispName = RmGetTrackName(CategoryList->name, curTr->name);
        if (!curTr->dispName || strlen(curTr->dispName) == 0) {
            GfTrace("rmtsTrackPrevNext: No definition for track %s\n", curTr->name);
        }
        }
        
    } while (!curTr->dispName && curTr != (tFList*)(CategoryList->userData));
    CategoryList->userData = (void*)curTr;
    
    /* Update GUI */
    GfuiLabelSetText(ScrHandle, TrackLabelId, curTr->dispName);
    //GfuiStaticImageSet(ScrHandle, MapId, rmtsGetMapName(path, maxPathSize));
    GfuiScreenAddBgImg(ScrHandle,rmtsGetMapName(path, maxPathSize));
    rmtsUpdateTrackInfo();
}

/* Select next/previous track category */
static void rmtsTrackCatPrevNext(void *vsel)
{
    const unsigned maxPathSize = 256;
    static char path[maxPathSize];

    /* Get next/previous usable track category
       Note: Here, we assume there's at least one,
             which is guaranteed by CategoryList initialization in RmTrackSelect */
    tFList *curCat = CategoryList;
    do {
    
    /* Next/previous category */
    curCat = vsel ? curCat->next : curCat->prev;
    
    /* Try and get the category display name if not already done */
    if (curCat->userData && !curCat->dispName) {
        curCat->dispName = RmGetCategoryName(curCat->name);
        if (!curCat->dispName || strlen(curCat->dispName) == 0) {
        GfTrace("rmtsTrackCatPrevNext: No definition for track category %s\n", curCat->name);
        }
    }
    
    /* If the category is loadable and not empty : */
    if (curCat->dispName && curCat->userData) {
        
        /* Select the first usable track in the category */
        tFList *curTr = (tFList*)(curCat->userData);
        do {
    
        /* Try and get the track display name if not already done */
        if (!curTr->dispName) {
            curTr->dispName = RmGetTrackName(curCat->name, curTr->name);
            if (!curTr->dispName || strlen(curTr->dispName) == 0) {
            GfTrace("rmtsTrackCatPrevNext: No definition for track %s\n", curTr->name);
            }
        }

        /* That's all if the track is usable*/
        if (curTr->dispName)
            break;
    
        /* Next track */
        curTr = curTr->next;
    
        } while (curTr != (tFList*)(curCat->userData));
        curCat->userData = (void*)curTr;
    }

    } while ((!curCat->dispName || !curCat->userData || !((tFList*)(curCat->userData))->dispName) 
         && curCat != CategoryList);
    CategoryList = curCat;
    
    /* Update GUI */
    GfuiLabelSetText(ScrHandle, CatLabelId, CategoryList->dispName);
    GfuiLabelSetText(ScrHandle, TrackLabelId, ((tFList*)curCat->userData)->dispName);
    //GfuiStaticImageSet(ScrHandle, MapId, rmtsGetMapName(path, maxPathSize));
    GfuiScreenAddBgImg(ScrHandle,rmtsGetMapName(path, maxPathSize));
    rmtsUpdateTrackInfo();
}


void rmtsSelect(void * /* dummy */)
{
    static char path[256];
    int curTrkIdx;

    curTrkIdx = (int)GfParmGetNum(TrackSelect->param, RM_SECT_TRACKS, RE_ATTR_CUR_TRACK, NULL, 1);
    sprintf(path, "%s/%d", RM_SECT_TRACKS, curTrkIdx);
    GfParmSetStr(TrackSelect->param, path, RM_ATTR_CATEGORY, CategoryList->name);
    GfParmSetStr(TrackSelect->param, path, RM_ATTR_NAME, ((tFList*)CategoryList->userData)->name);

    rmtsDeactivate(TrackSelect->nextScreen);
}


static void rmtsAddKeys(void)
{
    GfuiAddKey(ScrHandle, 13, "Select Track", NULL, rmtsSelect, NULL);
    GfuiAddKey(ScrHandle, 27, "Cancel Selection", TrackSelect->prevScreen, rmtsDeactivate, NULL);
    GfuiAddSKey(ScrHandle, GLUT_KEY_LEFT, "Previous Track", (void*)0, rmtsTrackPrevNext, NULL);
    GfuiAddSKey(ScrHandle, GLUT_KEY_RIGHT, "Next Track", (void*)1, rmtsTrackPrevNext, NULL);
    GfuiAddSKey(ScrHandle, GLUT_KEY_F1, "Help", ScrHandle, GfuiHelpScreen, NULL);
    GfuiAddSKey(ScrHandle, GLUT_KEY_F12, "Screen-Shot", NULL, GfuiScreenShot, NULL);
    GfuiAddSKey(ScrHandle, GLUT_KEY_UP, "Previous Track Category", (void*)0, rmtsTrackCatPrevNext, NULL);
    GfuiAddSKey(ScrHandle, GLUT_KEY_DOWN, "Next Track Category", (void*)1, rmtsTrackCatPrevNext, NULL);
}


/** Interactive track selection
    @param  vs  Pointer on a tRmTrackSelect structure (cast to void *)
    @warning    The race manager's parameters are updated but not saved.
    @ingroup    racemantools
 */
void
RmTrackSelect(void *vs)
{
    const unsigned maxPathSize = 256;
    static char path[maxPathSize];

    const char *defaultTrack;
    const char *defaultCategory;
    tFList *curCat;
    tFList *trList, *curTr;
    int curTrkIdx;

    TrackSelect = (tRmTrackSelect*)vs;

    /* Get default category and track for the current race type */
    curTrkIdx = (int)GfParmGetNum(TrackSelect->param, RM_SECT_TRACKS, RE_ATTR_CUR_TRACK, NULL, 1);
    sprintf(path, "%s/%d", RM_SECT_TRACKS, curTrkIdx);
    defaultCategory = GfParmGetStr(TrackSelect->param, path, RM_ATTR_CATEGORY, 0);
    defaultTrack = GfParmGetStr(TrackSelect->param, path, RM_ATTR_NAME, 0);

    /* Get the list of track category directories */
    CategoryList = GfDirGetList("tracks");
    if (!CategoryList) {
        GfTrace("RmTrackSelect: No track category available\n");
        return;
    }

    /* For each track category directory, get the track sub-dirs in it 
       Note: We avoid loading every category and track XML file now, 
             for a faster screen display ; these files are loaded only when needed 
         (on the fly), only once, here and in rmtsTrackPrevNext and rmtsTrackPrevNext */
    curCat = CategoryList;
    do {
        
        /* Initialize the display name to 0, as a category "not yet loaded" indicator */
        curCat->dispName = 0;
        
        /* Get the tracks in the category directory */
        sprintf(path, "tracks/%s", curCat->name);
        trList = GfDirGetList(path);
        
        /* Attach the track dir list to the category if not empty */
        if (!trList) {
        GfTrace("RmTrackSelect: No track available in dir tracks/%s\n", curCat->name);
        } else {
        //trList = trList->next; /* get the first one */
        
        /* For each track dir in the category dir */
        curTr = trList;
        do {
            
            /* Initialize the display name to 0, as a track "not yet loaded" indicator */
            curTr->dispName = 0;
            
            /* Next track dir*/
            curTr = curTr->next;
            
          } while (curTr != trList);
        }

        curCat->userData = (void*)trList;
        
        /* Next category dir */
        curCat = curCat->next;

    } while (curCat != CategoryList);

    /* Initialize currently selected category and track from default ones, if any */
    if (defaultCategory) {

        /* For each track category directory : */
        curCat = CategoryList;
        
        do {

        /* If current category is not empty and is the default one, */
        if (curCat->userData && !strcmp(curCat->name, defaultCategory)) {
            
            /* Try and get the category display name, and exit loop if it failed */
            curCat->dispName = RmGetCategoryName(curCat->name);
            if (!curCat->dispName || strlen(curCat->dispName) == 0) {
            GfTrace("RmTrackSelect: No definition for default track category %s\n", 
                curCat->name);
            break;
            }
            
            /* Now, we have the currently selected category */
            CategoryList = curCat;
            
            /* Initialize currently selected track from default one, if any */
            if (defaultTrack) {
            
            /* For each track dir in the category dir */
            curTr = (tFList*)(curCat->userData);
            do {
                
                /* If current track is the default one, */
                if (!strcmp(curTr->name, defaultTrack)) {
                
                /* Try and get the track display name */
                curTr->dispName = RmGetTrackName(curCat->name, curTr->name);
                if (!curTr->dispName || strlen(curTr->dispName) == 0)
                    GfTrace("RmTrackSelect: No definition for default track %s\n",
                        curTr->name);
                
                /* Now, we have the default selected track 
                   in the default selected category ; but beware,
                   it may be not usable : dispName may be 0 */
                curCat->userData = (void*)curTr;
                break;
                }
                
                /* Next track dir*/
                curTr = curTr->next;
                
            } while (curTr != trList);
            }
            
            /* Nothing more to do, whatever happened */
            break;
        }
        
        /* Next category dir */
        curCat = curCat->next;
        
        } while (curCat != CategoryList);
    }

    /* If we don't have yet a usable currently selected category (empty dir or unreadable XML
       or empty name), or a usable currently selected track (unreadable XML or empty name),
       try and get the first usable category and track going ahead */
    if (!CategoryList->userData || !((tFList*)(CategoryList->userData))->dispName) {
        
        /* For each track category directory : */
        curCat = CategoryList;

        do {
        
        /* Try and get the category display name if not already done */
        if (curCat->userData && !curCat->dispName) {
            curCat->dispName = RmGetCategoryName(curCat->name);
            if (!curCat->dispName || strlen(curCat->dispName) == 0) {
            GfTrace("RmTrackSelect: No definition for track category %s\n", curCat->name);
            }
        }
            
        /* If current category is usable, and has a usable track inside,
           select them and exit loop */
        if (curCat->userData && curCat->dispName && strlen(curCat->dispName) > 0) {

            /* Select the category */
            CategoryList = curCat;

            /* For each track  directory : */
            curTr = (tFList*)(CategoryList->userData);
            do {
        
            /* Try and load the track file if we don't have yet the track disp name */
            if (!curTr->dispName) {
                curTr->dispName = RmGetTrackName(curCat->name, curTr->name);
                if (!curTr->dispName || strlen(curTr->dispName) == 0)
                GfTrace("RmTrackSelect: No definition for track %s\n", curTr->name);
            }
            if (curTr->dispName && strlen(curTr->dispName) > 0)
                break;
        
            curTr = curTr->next;
        
            } while (curTr != (tFList*)(curCat->userData));

            /* Select the track */
            CategoryList->userData = (void*)curTr;

            /* If a track was really selected, exit loop */
            if (((tFList*)(CategoryList->userData))->dispName)
            break;
        }
        
        /* Next category dir */
        curCat = curCat->next;
        
        } while (curCat != CategoryList);
    }

    /* If no usable category/track found, ... return */
    if (!CategoryList->userData || !((tFList*)(CategoryList->userData))->dispName) {
        
        GfError("RmTrackSelect: No available track for any category\n");
        return; // or exit(1) abruptly ?
    }

    GfOut("Cat=%s, Track=%s\n", CategoryList->dispName, ((tFList*)(CategoryList->userData))->dispName);

    /* Create the screen, set background image, title and keyboard shortcuts */
    ScrHandle = GfuiScreenCreateEx((float*)NULL, NULL, rmtsActivate, 
                       NULL, (tfuiCallback)NULL, 1);

    void *param = LoadMenuXML("trackselectmenu.xml");
        CreateStaticControls(param,ScrHandle);

    rmtsAddKeys();

    /* Create category and track selection combo-boxes 
       (initialized to currently selected category and track) */
    CreateButtonControl(ScrHandle,param,"trackcatleftarrow",(void*)0,rmtsTrackCatPrevNext);
    CreateButtonControl(ScrHandle,param,"trackcatrightarrow",(void*)1,rmtsTrackCatPrevNext);
    CatLabelId = CreateLabelControl(ScrHandle,param,"trackcatlabel");
    GfuiLabelSetText(ScrHandle,CatLabelId,CategoryList->dispName);

    CreateButtonControl(ScrHandle,param,"trackleftarrow",(void*)0,rmtsTrackPrevNext);
    CreateButtonControl(ScrHandle,param,"trackrightarrow",(void*)1,rmtsTrackPrevNext);
    TrackLabelId = CreateLabelControl(ScrHandle,param,"tracklabel");
    GfuiLabelSetText(ScrHandle,TrackLabelId,((tFList*)CategoryList->userData)->dispName);

    /* Create static preview/map for currently selected track */
    //MapId = CreateStaticImageControl(ScrHandle,param,"trackimage");
    //GfuiStaticImageSet(ScrHandle, MapId, rmtsGetMapName(path, maxPathSize));
    GfuiScreenAddBgImg(ScrHandle,rmtsGetMapName(path, maxPathSize));

    CreateButtonControl(ScrHandle,param,"accept",NULL,rmtsSelect);
    CreateButtonControl(ScrHandle,param,"back",TrackSelect->prevScreen,rmtsDeactivate);

    DescId = CreateLabelControl(ScrHandle,param,"descriptionlabel");
    Desc2Id = CreateLabelControl(ScrHandle,param,"descriptionlabel2");
    LengthId = CreateLabelControl(ScrHandle,param,"lengthlabel");
    WidthId = CreateLabelControl(ScrHandle,param,"widthlabel");
    PitsId = CreateLabelControl(ScrHandle,param,"pitslabel");
    AuthorId = CreateLabelControl(ScrHandle,param,"authorlabel");

    GfParmReleaseHandle(param);

    rmtsUpdateTrackInfo();

    GfuiScreenActivate(ScrHandle);
}
