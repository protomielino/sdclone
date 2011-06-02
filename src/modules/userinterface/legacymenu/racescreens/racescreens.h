/***************************************************************************

    file                 : racescreens.h
    created              : Sat Mar 18 23:33:01 CET 2000
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
 
/**
    @defgroup	racescreens	Race menus.
    The race manager menus.
*/

#ifndef __RACESCREENS_H__
#define __RACESCREENS_H__

#include <raceman.h>

#include <itrackloader.h>

#include <tgfclient.h> // tfuiCallback


class GfRace;

typedef struct RmTrackSelect
{
	GfRace      *pRace; /* The race to update */
    void        *prevScreen;	/* Race manager screen to go back */
    void        *nextScreen;	/* Race manager screen to go after select */
    ITrackLoader	*piTrackLoader;	/* Track loader */
} tRmTrackSelect;

typedef struct RmDriverSelect
{
	GfRace      *pRace; /* The race to update */
    void        *prevScreen;	/* Race manager screen to go back */
    void        *nextScreen;	/* Race manager screen to go after select */
} tRmDriverSelect;

typedef struct RmRaceParam
{
	GfRace          *pRace; /* The race to update */
    std::string		session; /* The race session to configure (RM_VAL_ANYRACE for all of them) */
    void        	*prevScreen;	/* Race manager screen to go back */
    void        	*nextScreen;	/* Race manager screen to go after select */
} tRmRaceParam;

typedef void (*tfSelectFile) (const char *);

enum RmFileSelectMode { RmFSModeLoad, RmFSModeSave };

typedef struct RmFileSelect
{
    std::string		title;
    std::string		path;
    void        	*prevScreen;
    tfSelectFile	select;
	RmFileSelectMode mode;
} tRmFileSelect;

extern void RmTrackSelect(void * /* vs */);

extern void RmDriversSelect(void * /* vs */);

extern void RmPitMenuStart(tCarElt * /* car */, tfuiCallback /* callback */);

extern void RmLoadingScreenStart(const char * /* text */, const char * /* bgimg */);
extern void RmLoadingScreenSetText(const char * /* text */);
extern void RmLoadingScreenShutdown();

extern void RmGameScreen();

extern void RmShowResults(void * /* prevHdle */, tRmInfo * /* info */);

extern void RmStopRaceScreen();

extern void RmDisplayStartRace();

extern void RmRaceParamsMenu(void* vrp);

extern void RmShowStandings(void* prevHdle, tRmInfo *info, int start = 0);

extern void* RmFileSelect(void* vs);

// From racemanmenus.
extern int RmRacemanMenu();
extern int RmNextEventMenu();
extern void RmConfigureRace(void*  /* dummy */);
extern void RmSetRacemanMenuHandle(void*  handle);

extern void* RmGetRacemanMenuHandle();

extern void RmConfigRunState(bool bStart = false);

// From raceselectmenu.
extern void* RmRaceSelectInit(void* precMenu);

// From racerunningmenus.
extern void* RmScreenInit();
extern void RmScreenShutdown();
extern void* RmHookInit();
extern bool RmCheckPitRequest();

extern void* RmResScreenInit();
extern void RmResScreenSetTitles(const char *pszTitle, const char *pszSubTitle);
extern void RmResScreenSetHeader(const char *pszHeader);
extern void RmResScreenAddText(const char *pszText);
extern void RmResScreenSetText(const char *pszText, int nRowIndex, int nColorIndex);
extern void RmResScreenRemoveText(int nRowIndex);
//extern void RmResShowCont(); // Never used : remove ?
extern int  RmResGetRows();
extern void RmResEraseScreen();

// From networkingmenu.
extern void RmNetworkClientMenu(void* pVoid);
extern void RmNetworkMenu(void* /* dummy */);
extern void RmNetworkHostMenu(void* /* dummy */);



#endif /* __RACESCREENS_H__ */

