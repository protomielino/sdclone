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
    void        *param;		/* Race manager parameters where to set the selected track */
    void        *prevScreen;	/* Race manager screen to go back */
    void        *nextScreen;	/* Race manager screen to go after select */
    tTrackItf	trackItf;	/* Track module interface */
} tRmTrackSelect;

typedef struct RmDriverSelect
{
    void        *param;
    void        *prevScreen;	/* Race manager screen to go back */
    void        *nextScreen;	/* Race manager screen to go after select */
} tRmDriverSelect;

typedef struct RmRaceParam
{
    void        	*param;
    void        	*prevScreen;
    void        	*nextScreen;	/* Race manager screen to go after select */
    const char		*title;
    unsigned int	confMask;	/* Tell what to configure */
#define RM_CONF_RACE_LEN	0x00000001
#define RM_CONF_DISP_MODE	0x00000002
} tRmRaceParam;

typedef void (*tfSelectFile) (const char *);

enum RmFileSelectMode { RmFSModeLoad, RmFSModeSave };

typedef struct RmFileSelect
{
    const char		*title;
    const char		*path;
    void        	*prevScreen;
    tfSelectFile	select;
	RmFileSelectMode mode;
} tRmFileSelect;

RACESCREENS_API void RmTrackSelect(void * /* vs */);

RACESCREENS_API void RmDriversSelect(void * /* vs */);
extern void RmDriverSelect(void * /* vs */);

RACESCREENS_API void RmPitMenuStart(tCarElt * /* car */, tfuiCallback /* callback */);

RACESCREENS_API void RmLoadingScreenStart(const char * /* text */, const char * /* bgimg */);
RACESCREENS_API void RmLoadingScreenSetText(const char * /* text */);
RACESCREENS_API void RmShutdownLoadingScreen(void);

RACESCREENS_API void RmShowResults(void * /* prevHdle */, tRmInfo * /* info */);

RACESCREENS_API void *RmTwoStateScreen(const char *title,
									   const char *label1, const char *tip1, void *screen1,
									   const char *label2, const char *tip2, void *screen2);

RACESCREENS_API void *RmTriStateScreen(const char *title,
									   const char *label1, const char *tip1, void *screen1,
									   const char *label2, const char *tip2, void *screen2,
									   const char *label3, const char *tip3, void *screen3);

RACESCREENS_API void *RmFourStateScreen(const char *title,
										const char *label1, const char *tip1, void *screen1,
										const char *label2, const char *tip2, void *screen2,
										const char *label3, const char *tip3, void *screen3,
										const char *label4, const char *tip4, void *screen4);

RACESCREENS_API void *RmFiveStateScreen(char const *title,
										char const *label1, char const *tip1, void *screen1,
										char const *label2, char const *tip2, void *screen2,
										char const *label3, char const *tip3, void *screen3,
										char const *label4, char const *tip4, void *screen4,
										char const *label5, char const *tip5, void *screen5);

RACESCREENS_API void RmDisplayStartRace(tRmInfo *info, void *startScr, void *abortScr);


RACESCREENS_API void RmRaceParamMenu(void *vrp);

RACESCREENS_API void RmShowStandings(void *prevHdle, tRmInfo *info);

RACESCREENS_API void* RmFileSelect(void *vs);

RACESCREENS_API int RmGetFeaturesList( void* param );

#endif /* __RACESCREENS_H__ */

