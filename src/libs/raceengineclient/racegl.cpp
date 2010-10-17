/***************************************************************************

    file        : racegl.cpp
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
    		
    @author	<a href=mailto:eric.espie@torcs.org>Eric Espie</a>
    @version	$Id$
*/
#include <cstdlib>
#include <cstdio>

#include <tgfclient.h>
#include <raceman.h>
#include <robot.h>

#include "racesituation.h"
#include "racemessage.h"
#include "racemain.h"
#include "raceinit.h"
#include "racestate.h"
#include "raceupdate.h"

#include "racegl.h"


static void	*reScreenHandle = 0;
static void	*reHookHandle = 0;
static int	rePauseId;
static int	reMsgId;
static int	reBigMsgId;

static float black[4] = {0.0, 0.0, 0.0, 0.0};


/**************************************************************************
 * Normal race screen (3D animated scene mode = non "blind" mode)
 */
static void
reIdle(void)
{
    // Do nothing.
}

static void
reDisplay(void)
{
    ReStateManage();
}

static void
reScreenActivate(void * /* dummy */)
{
    // Set a real idle function (other than 0) doing nothing, 
    // otherwise GfelMainLoop will replace it by 1ms idle wait
    GfelSetIdleCB(reIdle);

    GfelSetDisplayCB(reDisplay);

	// Resync race engine if it is not paused or stopped
    if (!(ReInfo->s->_raceState & RM_RACE_PAUSED)) {
	ReStart(); 			/* resynchro */
    }

    GfelPostRedisplay();
}

static void
ReBoardInfo(void * /* vboard */)
{
    if (ReInfo->s->_raceState & RM_RACE_PAUSED) {
	ReInfo->s->_raceState &= ~RM_RACE_PAUSED;
	ReStart();
	GfuiVisibilitySet(reScreenHandle, rePauseId, 0);
    } else {
	ReInfo->s->_raceState |= RM_RACE_PAUSED;
	ReStop();
	GfuiVisibilitySet(reScreenHandle, rePauseId, 1);
    }
}

static void
reSkipPreStart(void * /* dummy */)
{
    if (ReInfo->s->currentTime < -1.0) {
	ReInfo->s->currentTime = -1.0;
	ReInfo->_reLastTime = -1.0;
    }
}

static void
reTimeMod (void *vcmd)
{
	char buf[32];
    long cmd = (long)vcmd;
    
    switch ((int)cmd) {
    case 0:
	ReInfo->_reTimeMult *= 2.0;
	if (ReInfo->_reTimeMult > 64.0) {
	    ReInfo->_reTimeMult = 64.0;
	}
	break;
    case 1:
	ReInfo->_reTimeMult *= 0.5;
	if (ReInfo->_reTimeMult < 0.25) {
	    ReInfo->_reTimeMult = 0.25;
	}
	break;
    case 2:
    default:
	ReInfo->_reTimeMult = 1.0;
	break;
    }
    sprintf(buf, "Time x%.2f", 1.0 / ReInfo->_reTimeMult);
    ReRaceMsgSet(ReInfo, buf, 5); // TODO: Thread-safe access to ReInfo in multi-threaded mode !
}

static void
reMovieCapture(void * /* dummy */)
{
	tRmMovieCapture	*capture = &(ReInfo->movieCapture);

    if (!capture->enabled || ReInfo->_displayMode == RM_DISP_MODE_NONE || ReInfo->_displayMode == RM_DISP_MODE_SIMU_SIMU) 
    {
	GfLogWarning("Movie capture is not enabled : command ignored\n");
	return;
    }
    
    capture->state = 1 - capture->state;
    if (capture->state) {
	GfOut("Starting movie capture\n");
	capture->currentFrame = 0;
	capture->currentCapture++;
	capture->lastFrame = GfTimeClock() - capture->deltaFrame;
	ReInfo->_displayMode = RM_DISP_MODE_CAPTURE;
    } else {
	GfOut("Stopping movie capture\n");
	ReInfo->_displayMode = RM_DISP_MODE_NORMAL;
	ReStart();
    }

}

static void
reHideShowMouseCursor(void * /* dummy */)
{
    GfuiMouseToggleVisibility();
}

static void
reAddKeys(void)
{
    GfuiAddKey(reScreenHandle, GFUIK_F1,  "Help", reScreenHandle, GfuiHelpScreen, NULL);
    GfuiAddKey(reScreenHandle, GFUIK_F12, "Screen Shot", NULL, GfuiScreenShot, NULL);

    GfuiAddKey(reScreenHandle, '-', "Slow down Time",    (void*)0, reTimeMod, NULL);
    GfuiAddKey(reScreenHandle, '+', "Accelerate Time",   (void*)1, reTimeMod, NULL);
    GfuiAddKey(reScreenHandle, '.', "Restore Real Time", (void*)2, reTimeMod, NULL);
    GfuiAddKey(reScreenHandle, 'p', "Pause Race",        (void*)0, ReBoardInfo, NULL);
    GfuiAddKey(reScreenHandle, GFUIK_ESCAPE,  "Stop Current Race", (void*)RE_STATE_RACE_STOP, ReStateApply, NULL);
    /* GfuiAddKey(reScreenHandle, 'q', "Exit from Game",     (void*)RE_STATE_EXIT, ReStateApply, NULL); */
    GfuiAddKey(reScreenHandle, ' ', "Skip Pre-start",    (void*)0, reSkipPreStart, NULL);
#ifdef DEBUG
	// WARNING: Certainly won't work with multi-threading On/Auto ...
    //GfuiAddKey(reScreenHandle, '0', "One step simulation",    (void*)1, reOneStep, NULL);
#endif
    GfuiAddKey(reScreenHandle, 'c', "Movie Capture (if enabled)", (void*)0, reMovieCapture, NULL);
    GfuiAddKey(reScreenHandle, 'o', "Hide / Show mouse cursor",   (void*)0, reHideShowMouseCursor, NULL);
}


void
ReSetRaceMsg(const char *msg)
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
		GfuiLabelSetText(reScreenHandle, reMsgId, curMsg);
    } else {
		curMsg = 0;
		GfuiLabelSetText(reScreenHandle, reMsgId, "");
    }
}

void
ReSetRaceBigMsg(const char *msg)
{
    static char *curMsg = 0;
    
	// If nothing to change, don't change anything.
	if ((!curMsg && !msg) || (curMsg && msg && !strcmp(curMsg, msg)))
		return;
	
    if (curMsg)
		free(curMsg);
    if (msg) {
		curMsg = strdup(msg);
		GfuiLabelSetText(reScreenHandle, reBigMsgId, curMsg);
    } else {
		curMsg = 0;
		GfuiLabelSetText(reScreenHandle, reBigMsgId, "");
    }
}

void *
ReScreenInit(void)
{
    // Release screen if was initialized.
    ReScreenShutdown();

    // Create screen, load menu XML descriptor and create static controls.
    reScreenHandle = GfuiScreenCreateEx(black, 0, reScreenActivate, 0, 0, 0);
    void *menuXMLDescHdle = LoadMenuXML("raceglscreen.xml");
    CreateStaticControls(menuXMLDescHdle, reScreenHandle);

    // Create Message, BigMessage and Pause labels.
    reMsgId = CreateLabelControl(reScreenHandle, menuXMLDescHdle, "message");
    rePauseId = CreateLabelControl(reScreenHandle, menuXMLDescHdle, "pause");
    reBigMsgId = CreateLabelControl(reScreenHandle, menuXMLDescHdle, "bigmessage");

    // Close menu XML descriptor.
    GfParmReleaseHandle(menuXMLDescHdle);
    
    // Register keyboard shortcuts.
    reAddKeys();

    // Hide the Pause label for the moment.
    GfuiVisibilitySet(reScreenHandle, rePauseId, 0);

    return reScreenHandle;
}


void
ReScreenShutdown(void)
{
    if (reScreenHandle) {
	GfuiScreenRelease(reScreenHandle);
	reScreenHandle = 0;
    }
}


static void
reHookActivate(void * /* dummy */)
{
    ReStateManage();
}

void *
ReHookInit(void)
{
    if (reHookHandle) {
	return reHookHandle;
    }
    
    reHookHandle = GfuiHookCreate(0, reHookActivate);

    return reHookHandle;
}


void
ReHookShutdown(void)
{
    if (reHookHandle) {
	GfuiHookRelease(reHookHandle);
	reHookHandle = 0;
    }
}

/**************************************************************************
 * Result only screen (blind mode)
 */
static const int NMaxResultLines = 21;

static float	white[4]   = {1.0, 1.0, 1.0, 1.0};
static float	red[4]     = {1.0, 0.0, 0.0, 1.0};
static float	*reColor[] = {white, red};

static void	*reResScreenHdle = 0;

static int	reResTitleId;
static int	reResMsgId[NMaxResultLines];

static int	reResMsgClr[NMaxResultLines];
static char	*reResMsg[NMaxResultLines];

static int	reCurLine;

static void
reAddResKeys(void)
{
    GfuiAddKey(reResScreenHdle, GFUIK_F1,  "Help", reScreenHandle, GfuiHelpScreen, NULL);
    GfuiAddKey(reResScreenHdle, GFUIK_F12, "Screen Shot", NULL, GfuiScreenShot, NULL);

    GfuiAddKey(reResScreenHdle, GFUIK_ESCAPE,  "Stop Current Race", (void*)RE_STATE_RACE_STOP, ReStateApply, NULL);
    /* GfuiAddKey(reResScreenHdle, 'q', "Exit from Game",     (void*)RE_STATE_EXIT, ReStateApply, NULL); */
}

static void
reResScreenActivate(void * /* dummy */)
{
    // Set a real idle function (other than 0) doing nothing, 
    // otherwise GfelMainLoop will replace it by a 1ms idle wait
    GfelSetIdleCB(reIdle);

    GfelSetDisplayCB(reDisplay);
    GfuiDisplay();
    GfelPostRedisplay();
}


static void
reContDisplay(void)
{
    GfuiDisplay();
    GfelPostRedisplay();
}


static void
reResCont(void * /* dummy */)
{
    ReStateManage();
}

static void
reResScreenShutdown(void * /* dummy */)
{
    int		i;

    for (i = 1; i < NMaxResultLines; i++) {
	FREEZ(reResMsg[i]);
    }
}

void *
ReResScreenInit(void)
{
    int		i;
    int		y, dy;
    const char	*img;
    static const char *title[3] = {"Practice", "Qualifications", "Race"};

    if (reResScreenHdle) {
	GfuiScreenRelease(reResScreenHdle);
    }

    // Create screen, load menu XML descriptor and create static controls.
    reResScreenHdle = GfuiScreenCreateEx(black, 0, reResScreenActivate, 0, reResScreenShutdown, 0);
    void *menuXMLDescHdle = LoadMenuXML("raceblindscreen.xml");
    CreateStaticControls(menuXMLDescHdle, reResScreenHdle);

    // Create variable main title (race type/stage) label.
    int mainTitleId = CreateLabelControl(reResScreenHdle, menuXMLDescHdle, "title");
    GfuiLabelSetText(reResScreenHdle, mainTitleId, title[ReInfo->s->_raceType]);

    // Create background image if any specified.
    img = GfParmGetStr(ReInfo->params, RM_SECT_HEADER, RM_ATTR_RUNIMG, 0);
    if (img) {
	GfuiScreenAddBgImg(reResScreenHdle, img);
    }
    
    // Create variable subtitle (driver and race name, lap number) label.
    reResTitleId = CreateLabelControl(reResScreenHdle, menuXMLDescHdle, "subtitle");

    // Create result lines (1 label for each).
    // TODO: Get layout, color, ... info from menuXMLDescHdle when available.
    y = 400;
    dy = 378 / NMaxResultLines;
    for (i = 0; i < NMaxResultLines; i++) {
	FREEZ(reResMsg[i]);
	reResMsgClr[i] = 0;
	reResMsgId[i] = GfuiLabelCreateEx(reResScreenHdle,
					  "",
					  white,
					  GFUI_FONT_MEDIUM_C,
					  20, y, 
					  GFUI_ALIGN_HL_VB, 120);
	y -= dy;
    }

    // Close menu XML descriptor.
    GfParmReleaseHandle(menuXMLDescHdle);
    
    // Register keyboard shortcuts.
    reAddResKeys();

    // Initialize current result line.
    reCurLine = 0;

    return reResScreenHdle;
}

void
ReResScreenSetTitle(const char *title)
{
    if (reResScreenHdle) {
	GfuiLabelSetText(reResScreenHdle, reResTitleId, title);
    }
}

void
ReResScreenAddText(const char *text)
{
    int		i;

    if (reCurLine == NMaxResultLines) {
	free(reResMsg[0]);
	for (i = 1; i < NMaxResultLines; i++) {
	    reResMsg[i - 1] = reResMsg[i];
	    GfuiLabelSetText(reResScreenHdle, reResMsgId[i - 1], reResMsg[i]);
	}
	reCurLine--;
    }
    reResMsg[reCurLine] = strdup(text);
    GfuiLabelSetText(reResScreenHdle, reResMsgId[reCurLine], reResMsg[reCurLine]);
    reCurLine++;
}

void
ReResScreenSetText(const char *text, int line, int clr)
{
    if (line < NMaxResultLines) {
	FREEZ(reResMsg[line]);
	reResMsg[line] = strdup(text);
	if ((clr >= 0) && (clr < 2)) {
	    reResMsgClr[line] = clr;
	} else {
	    reResMsgClr[line] = 0;
	}
	GfuiLabelSetText(reResScreenHdle, reResMsgId[line], reResMsg[line]);
	GfuiLabelSetColor(reResScreenHdle, reResMsgId[line], reColor[reResMsgClr[line]]);
    }
}

int
ReResGetLines(void)
{
    return NMaxResultLines;
}

void
ReResEraseScreen(void)
{
    int i;

    for (i = 0; i < NMaxResultLines; i++) {
	ReResScreenSetText("", i, 0);
    }
}


void
ReResScreenRemoveText(int line)
{
    if (line < NMaxResultLines) {
	FREEZ(reResMsg[line]);
	GfuiLabelSetText(reResScreenHdle, reResMsgId[line], "");
    }
}

void
ReResShowCont(void)
{

    GfuiButtonCreate(reResScreenHdle,
		     "Continue",
		     GFUI_FONT_LARGE_C,
		     320, 15, GFUI_BTNSZ,
		     GFUI_ALIGN_HC_VB,
		     0, 0, reResCont,
		     NULL, (tfuiCallback)NULL,
		     (tfuiCallback)NULL);
    GfuiAddKey(reResScreenHdle, GFUIK_RETURN,  "Continue", 0, reResCont, NULL);
    GfuiAddKey(reResScreenHdle, GFUIK_ESCAPE,  "Continue", 0, reResCont, NULL);

    GfelSetDisplayCB(reContDisplay);
    GfelPostRedisplay();
}

