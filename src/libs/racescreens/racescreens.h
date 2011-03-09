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

#include <tgfclient.h> // tfuiCallback

class GfRace;

// DLL exported symbols declarator for Windows.
#ifdef WIN32
# ifdef RACESCREENS_DLL
#  define RACESCREENS_API __declspec(dllexport)
# else
#  define RACESCREENS_API __declspec(dllimport)
# endif
#else
# define RACESCREENS_API
#endif


typedef struct RmTrackSelect
{
	GfRace      *pRace; /* The race to update */
    void        *prevScreen;	/* Race manager screen to go back */
    void        *nextScreen;	/* Race manager screen to go after select */
    tTrackItf	trackItf;	/* Track module interface */
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
extern void RmLoadingScreenShutdown(void);

extern void RmShowResults(void * /* prevHdle */, tRmInfo * /* info */);

extern void *RmStopRaceScreen(char const *title,
									   char const *label1, char const *tip1, void *screen1,
									   char const *label2, char const *tip2, void *screen2,
									   char const *label3 = 0, char const *tip3 = 0, void *screen3 = 0,
									   char const *label4 = 0, char const *tip4 = 0, void *screen4 = 0,
									   char const *label5 = 0, char const *tip5 = 0, void *screen5 = 0);

extern void RmDisplayStartRace(tRmInfo *info, void *startScr, void *abortScr);


extern void RmRaceParamsMenu(void *vrp);

extern void RmShowStandings(void *prevHdle, tRmInfo *info, int start = 0);

extern void* RmFileSelect(void *vs);

// From racemanmenus.
extern int ReRacemanMenu();
extern int ReNextEventMenu(void);
extern void ReConfigureRace(void * /* dummy */);
extern void ReSetRacemanMenuHandle(void * handle);

extern void* ReGetRacemanMenuHandle();

extern void ReConfigRunState(bool bStart = false);

// From raceselectmenu.
RACESCREENS_API void *ReRaceSelectInit(void *precMenu);

// From racegl.
extern void *ReScreenInit(void);
extern void  ReScreenShutdown(void);
extern void *ReHookInit(void);
extern void ReHookShutdown(void);
extern void ReSetRaceMsg(const char *msg);
extern void ReSetRaceBigMsg(const char *msg);

extern void *ReResScreenInit(void);
extern void ReResScreenSetTrackName(const char *trackName);
extern void ReResScreenSetTitle(const char *title);
extern void ReResScreenAddText(const char *text);
extern void ReResScreenSetText(const char *text, int line, int clr);
extern void ReResScreenRemoveText(int line);
extern void ReResShowCont(void);
extern int  ReResGetLines(void);
extern void ReResEraseScreen(void);

// From networkingmenu.
extern void ReNetworkClientConnectMenu(void *pVoid);
extern void ReNetworkMenu(void * /* dummy */);
extern void ReNetworkHostMenu(void * /* dummy */);



#endif /* __RACESCREENS_H__ */

