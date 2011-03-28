/***************************************************************************

    file        : racerunningmenus.cpp
    created     : Sat Nov 16 18:22:00 CET 2002
    copyright   : (C) 2002 by Eric Espie
    email       : eric.espie@torcs.org   
    version     : $Id$                                  

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
    		The menus for when the race is running
    @author	<a href=mailto:eric.espie@torcs.org>Eric Espie</a>
    @version	$Id$
*/
#include <cstdlib>
#include <cstdio>

#include <portability.h>
#include <tgfclient.h>

#include <raceman.h>
#include <robot.h>

#include "legacymenu.h"
#include "racescreens.h"


static void	*rmScreenHandle = 0;
static int	rmPauseId;
static int	rmMsgId;
static int	rmBigMsgId;

static float black[4] = {0.0, 0.0, 0.0, 0.0};

/**************************************************************************
 * Normal race screen (3D animated scene mode = non "blind" mode)
 */
static void
rmIdle()
{
    // Do nothing.
}

static void
rmDisplay()
{
    LegacyMenu::self().raceEngine().updateState();
}

static void
rmScreenActivate(void * /* dummy */)
{
    // Set a real idle function (other than 0) doing nothing, 
    // otherwise GfelMainLoop will replace it by 1ms idle wait
    GfelSetIdleCB(rmIdle);

    GfelSetDisplayCB(rmDisplay);

	// Resync race engine if it is not paused or stopped
	tRmInfo* reInfo = LegacyMenu::self().raceEngine().data();
    if (!(reInfo->s->_raceState & RM_RACE_PAUSED)) {
		LegacyMenu::self().raceEngine().start(); 			/* resynchro */
    }

    GfelPostRedisplay();
}

static void
RmBoardInfo(void * /* vboard */)
{
	tRmInfo* reInfo = LegacyMenu::self().raceEngine().data();
    if (reInfo->s->_raceState & RM_RACE_PAUSED) {
	reInfo->s->_raceState &= ~RM_RACE_PAUSED;
	LegacyMenu::self().raceEngine().start();
	GfuiVisibilitySet(rmScreenHandle, rmPauseId, 0);
    } else {
	reInfo->s->_raceState |= RM_RACE_PAUSED;
	LegacyMenu::self().raceEngine().stop();
	GfuiVisibilitySet(rmScreenHandle, rmPauseId, 1);
    }
}

static void
rmSkipPreStart(void * /* dummy */)
{
	tRmInfo* reInfo = LegacyMenu::self().raceEngine().data();
    if (reInfo->s->currentTime < -1.0) {
	reInfo->s->currentTime = -1.0;
	reInfo->_reLastTime = -1.0;
    }
}

static void
rmTimeMod (void *pvCmd)
{
	double fMultFactor = 0.0; // The mult. factor for resetting "real time" simulation step.
	if ((long)pvCmd > 0)
		fMultFactor = 0.5; // Accelerate time means reduce the simulation time step.
	else if ((long)pvCmd < 0)
		fMultFactor = 2.0; // Slow-down time means increase the simulation time step.
	LegacyMenu::self().raceEngine().accelerateTime(fMultFactor);
}

static void
rmMovieCapture(void * /* dummy */)
{
	tRmInfo* reInfo = LegacyMenu::self().raceEngine().data();
	tRmMovieCapture	*capture = &(reInfo->movieCapture);

    if (!capture->enabled || reInfo->_displayMode == RM_DISP_MODE_NONE || reInfo->_displayMode == RM_DISP_MODE_SIMU_SIMU) 
    {
	GfLogWarning("Movie capture is not enabled : command ignored\n");
	return;
    }
    
    capture->state = 1 - capture->state;
    if (capture->state) {
	GfLogInfo("Starting movie capture\n");
	capture->currentFrame = 0;
	capture->currentCapture++;
	capture->lastFrame = GfTimeClock() - capture->deltaFrame;
	reInfo->_displayMode = RM_DISP_MODE_CAPTURE;
    } else {
	GfLogInfo("Stopping movie capture\n");
	reInfo->_displayMode = RM_DISP_MODE_NORMAL;
	LegacyMenu::self().raceEngine().start();
    }

}

static void
rmHideShowMouseCursor(void * /* dummy */)
{
    GfuiMouseToggleVisibility();
}

static void
rmApplyState(void *pvState)
{
    LegacyMenu::self().raceEngine().applyState((int)(long)pvState);
}

#ifdef DEBUG
static void
rmOneStep(void *pvState)
{
    LegacyMenu::self().raceEngine().step((int)(long)pvState);
}
#endif

static void
rmAddKeys()
{
    GfuiAddKey(rmScreenHandle, GFUIK_F1,  "Help", rmScreenHandle, GfuiHelpScreen, NULL);
    GfuiAddKey(rmScreenHandle, GFUIK_F12, "Screen Shot", NULL, GfuiScreenShot, NULL);

    GfuiAddKey(rmScreenHandle, '-', "Slow down Time",    (void*)-1, rmTimeMod, NULL);
    GfuiAddKey(rmScreenHandle, '+', "Accelerate Time",   (void*)+1, rmTimeMod, NULL);
    GfuiAddKey(rmScreenHandle, '.', "Restore Real Time", (void*)0, rmTimeMod, NULL);
	
    GfuiAddKey(rmScreenHandle, 'p', "Pause Race",        (void*)0, RmBoardInfo, NULL);
    GfuiAddKey(rmScreenHandle, GFUIK_ESCAPE,  "Stop Current Race", (void*)RE_STATE_RACE_STOP, rmApplyState, NULL);
    GfuiAddKey(rmScreenHandle, 'q', "Quit Game, Save Nothing",    (void*)RE_STATE_EXIT, rmApplyState, NULL);
    GfuiAddKey(rmScreenHandle, ' ', "Skip Pre-start",    (void*)0, rmSkipPreStart, NULL);
	
#ifdef DEBUG
	// WARNING: Sure this won't work with multi-threading On/Auto ...
    //GfuiAddKey(rmScreenHandle, '0', "One step simulation",    (void*)1, rmOneStep, NULL);
#endif
	
    GfuiAddKey(rmScreenHandle, 'c', "Movie Capture (if enabled)", (void*)0, rmMovieCapture, NULL);
    GfuiAddKey(rmScreenHandle, 'o', "Hide / Show mouse cursor",   (void*)0, rmHideShowMouseCursor, NULL);
}


void
RmSetRaceMsg(const char *msg)
{
    static char *curMsg = 0;

	// If nothing to change, don't change anything.
	if ((!curMsg && !msg) || (curMsg && msg && !strcmp(curMsg, msg)))
		return;

	// Otherwise, set the new text for the label.
    if (curMsg)
		free(curMsg);
    if (msg) {
		curMsg = strdup(msg);
		GfuiLabelSetText(rmScreenHandle, rmMsgId, curMsg);
    } else {
		curMsg = 0;
		GfuiLabelSetText(rmScreenHandle, rmMsgId, "");
    }
}

void
RmSetRaceBigMsg(const char *msg)
{
    static char *curMsg = 0;
    
	// If nothing to change, don't change anything.
	if ((!curMsg && !msg) || (curMsg && msg && !strcmp(curMsg, msg)))
		return;
	
    if (curMsg)
		free(curMsg);
    if (msg) {
		curMsg = strdup(msg);
		GfuiLabelSetText(rmScreenHandle, rmBigMsgId, curMsg);
    } else {
		curMsg = 0;
		GfuiLabelSetText(rmScreenHandle, rmBigMsgId, "");
    }
}

void *
RmScreenInit()
{
    // Release screen if was initialized.
    RmScreenShutdown();

    // Create screen, load menu XML descriptor and create static controls.
    rmScreenHandle = GfuiScreenCreateEx(black, 0, rmScreenActivate, 0, 0, 0);
    void *menuXMLDescHdle = LoadMenuXML("raceglscreen.xml");
    CreateStaticControls(menuXMLDescHdle, rmScreenHandle);

    // Create Message, BigMessage and Pause labels.
    rmMsgId = CreateLabelControl(rmScreenHandle, menuXMLDescHdle, "message");
    rmPauseId = CreateLabelControl(rmScreenHandle, menuXMLDescHdle, "pause");
    rmBigMsgId = CreateLabelControl(rmScreenHandle, menuXMLDescHdle, "bigmessage");

    // Close menu XML descriptor.
    GfParmReleaseHandle(menuXMLDescHdle);
    
    // Register keyboard shortcuts.
    rmAddKeys();

    // Hide the Pause label for the moment.
    GfuiVisibilitySet(rmScreenHandle, rmPauseId, 0);

    return rmScreenHandle;
}

void
RmScreenCapture(const char* pszTargetFilename)
{
    GfScrCaptureAsPNG(pszTargetFilename);
}

void
RmScreenShutdown()
{
    if (rmScreenHandle) {
		GfuiScreenRelease(rmScreenHandle);
		rmScreenHandle = 0;
    }
}


static void
rmHookActivate(void * /* dummy */)
{
    LegacyMenu::self().raceEngine().updateState();
}

void *
RmHookInit()
{
	static void	*rmHookHandle = 0;
	
    if (!rmHookHandle)
		rmHookHandle = GfuiHookCreate(0, rmHookActivate);

    return rmHookHandle;
}

/**************************************************************************
 * Result only screen (blind mode)
 */
static const int NMaxResultLines = 21;

static float	white[4]   = {1.0, 1.0, 1.0, 1.0};
static float	red[4]     = {1.0, 0.0, 0.0, 1.0};
static float	*rmColor[] = {white, red};

static const char *aRaceTypeNames[3] = {"Practice", "Qualifications", "Race"};

static void	*rmResScreenHdle = 0;

static int	rmResMainTitleId;
static int	rmResTitleId;
static int	rmResMsgId[NMaxResultLines];

static int	rmResMsgClr[NMaxResultLines];
static char	*rmResMsg[NMaxResultLines];

static int	rmCurLine;

static void
rmAddResKeys()
{
    GfuiAddKey(rmResScreenHdle, GFUIK_F1,  "Help", rmScreenHandle, GfuiHelpScreen, NULL);
    GfuiAddKey(rmResScreenHdle, GFUIK_F12, "Screen Shot", NULL, GfuiScreenShot, NULL);

    GfuiAddKey(rmResScreenHdle, GFUIK_ESCAPE,  "Stop Current Race", (void*)RE_STATE_RACE_STOP, rmApplyState, NULL);
    GfuiAddKey(rmResScreenHdle, 'q', "Quit Game, Save Nothing", (void*)RE_STATE_EXIT, rmApplyState, NULL);
}

static void
rmResScreenActivate(void * /* dummy */)
{
    // Set a real idle function (other than 0) doing nothing, 
    // otherwise GfelMainLoop will replace it by a 1ms idle wait
    GfelSetIdleCB(rmIdle);

    GfelSetDisplayCB(rmDisplay);
    GfuiDisplay();
    GfelPostRedisplay();
}


static void
rmContDisplay()
{
    GfuiDisplay();
    GfelPostRedisplay();
}


static void
rmResCont(void * /* dummy */)
{
    LegacyMenu::self().raceEngine().updateState();
}

static void
rmResScreenShutdown(void * /* dummy */)
{
    for (int i = 1; i < NMaxResultLines; i++)
		FREEZ(rmResMsg[i]);
}

void *
RmResScreenInit()
{
    int		i;
    int		y, dy;
    const char	*img;

    if (rmResScreenHdle) {
	GfuiScreenRelease(rmResScreenHdle);
    }

	tRmInfo* reInfo = LegacyMenu::self().raceEngine().data();

    // Create screen, load menu XML descriptor and create static controls.
    rmResScreenHdle = GfuiScreenCreateEx(black, 0, rmResScreenActivate, 0, rmResScreenShutdown, 0);
    void *menuXMLDescHdle = LoadMenuXML("raceblindscreen.xml");
    CreateStaticControls(menuXMLDescHdle, rmResScreenHdle);

    // Create variable main title (race type/stage) label.
    rmResMainTitleId = CreateLabelControl(rmResScreenHdle, menuXMLDescHdle, "title");
    GfuiLabelSetText(rmResScreenHdle, rmResMainTitleId, aRaceTypeNames[reInfo->s->_raceType]);

    // Create background image if any specified.
    img = GfParmGetStr(reInfo->params, RM_SECT_HEADER, RM_ATTR_RUNIMG, 0);
    if (img) {
	GfuiScreenAddBgImg(rmResScreenHdle, img);
    }
    
    // Create variable subtitle (driver and race name, lap number) label.
    rmResTitleId = CreateLabelControl(rmResScreenHdle, menuXMLDescHdle, "subtitle");

    // Create result lines (1 label for each).
    // TODO: Get layout, color, ... info from menuXMLDescHdle when available.
    y = 400;
    dy = 378 / NMaxResultLines;
    for (i = 0; i < NMaxResultLines; i++) {
	FREEZ(rmResMsg[i]);
	rmResMsgClr[i] = 0;
	rmResMsgId[i] = GfuiLabelCreate(rmResScreenHdle, "", GFUI_FONT_MEDIUM_C, 20, y, 
									GFUI_ALIGN_HL_VB, 120, white);
	y -= dy;
    }

    // Close menu XML descriptor.
    GfParmReleaseHandle(menuXMLDescHdle);
    
    // Register keyboard shortcuts.
    rmAddResKeys();

    // Initialize current result line.
    rmCurLine = 0;

    return rmResScreenHdle;
}

void
RmResScreenSetTrackName(const char *pszTrackName)
{
    if (rmResScreenHdle) {
		char pszTitle[128];
		tRmInfo* reInfo = LegacyMenu::self().raceEngine().data();
		snprintf(pszTitle, sizeof(pszTitle), "%s on %s",
				 aRaceTypeNames[reInfo->s->_raceType], pszTrackName);
		GfuiLabelSetText(rmResScreenHdle, rmResMainTitleId, pszTitle);
    }
}

void
RmResScreenSetTitle(const char *pszTitle)
{
    if (rmResScreenHdle) {
		GfuiLabelSetText(rmResScreenHdle, rmResTitleId, pszTitle);
    }
}

void
RmResScreenAddText(const char *text)
{
    int		i;

    if (rmCurLine == NMaxResultLines) {
	free(rmResMsg[0]);
	for (i = 1; i < NMaxResultLines; i++) {
	    rmResMsg[i - 1] = rmResMsg[i];
	    GfuiLabelSetText(rmResScreenHdle, rmResMsgId[i - 1], rmResMsg[i]);
	}
	rmCurLine--;
    }
    rmResMsg[rmCurLine] = strdup(text);
    GfuiLabelSetText(rmResScreenHdle, rmResMsgId[rmCurLine], rmResMsg[rmCurLine]);
    rmCurLine++;
}

void
RmResScreenSetText(const char *text, int line, int clr)
{
    if (line < NMaxResultLines) {
	FREEZ(rmResMsg[line]);
	rmResMsg[line] = strdup(text);
	if ((clr >= 0) && (clr < 2)) {
	    rmResMsgClr[line] = clr;
	} else {
	    rmResMsgClr[line] = 0;
	}
	GfuiLabelSetText(rmResScreenHdle, rmResMsgId[line], rmResMsg[line]);
	GfuiLabelSetColor(rmResScreenHdle, rmResMsgId[line], rmColor[rmResMsgClr[line]]);
    }
}

int
RmResGetLines()
{
    return NMaxResultLines;
}

void
RmResEraseScreen()
{
    int i;

    for (i = 0; i < NMaxResultLines; i++) {
	RmResScreenSetText("", i, 0);
    }
}


void
RmResScreenRemoveText(int line)
{
    if (line < NMaxResultLines) {
	FREEZ(rmResMsg[line]);
	GfuiLabelSetText(rmResScreenHdle, rmResMsgId[line], "");
    }
}

void
RmResShowCont()
{

    GfuiButtonCreate(rmResScreenHdle,
		     "Continue",
		     GFUI_FONT_LARGE_C,
		     320, 15, GFUI_BTNSZ,
		     GFUI_ALIGN_HC_VB,
		     0, 0, rmResCont,
		     NULL, (tfuiCallback)NULL,
		     (tfuiCallback)NULL);
    GfuiAddKey(rmResScreenHdle, GFUIK_RETURN,  "Continue", 0, rmResCont, NULL);
    GfuiAddKey(rmResScreenHdle, GFUIK_ESCAPE,  "Continue", 0, rmResCont, NULL);

    GfelSetDisplayCB(rmContDisplay);
    GfelPostRedisplay();
}

//************************************************************
void
RmGameScreen()
{
	GfuiScreenActivate(LegacyMenu::self().raceEngine().data()->_reGameScreen);
}
