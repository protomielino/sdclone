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
#include <sstream>

#include <portability.h>
#include <tgf.hpp>
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


// Uncomment to activate FPS limiter at compile-time.
//#define UseFPSLimiter true

#ifdef UseFPSLimiter

// Forced max. graphics update rate (Hz) (0 means no maximum).
static double FPSLimMaxRate = 0;

// Last time a real graphics update was done (only used if FPSLimMaxRate).
static double FPSLimLastTime = 0;

#endif

static void
rmUpdateRaceEngine()
{
    LmRaceEngine().updateState();
}

/**************************************************************************
 * Normal race screen (3D animated scene mode = non "blind" mode)
 */

// Current values for the menu messages.
static std::string rmStrCurMsg;
static std::string rmStrCurBigMsg;

// Flag to know if the menu state has been changed (and thus needs a redraw+redisplay).
static bool rmbMenuChanged = false;

struct RmMovieCapture
{
    int		enabled;
    int		active;
    double	simuRate; // Hz
    double	frameRate; // Hz
    char*	outputBase;
    int		currentCapture;
    int		currentFrame;
};


static RmMovieCapture rmMovieCapture =
{
	false, // enabled
	false, // active
	0.0,   // simuRate
	0.0,   // frameRate
	0,     // outputBase
	0,     // currentCapture
	0      // currentFrame
};
				  
static void
rmInitMovieCapture()
{
	// Don't do it twice.
	if (rmMovieCapture.outputBase)
		return;

	// But do it the first time.
	char buf[256];
	snprintf(buf, sizeof(buf), "%s%s", GfLocalDir(), RACE_ENG_CFG);

	void* hparmRaceEng = GfParmReadFile(buf, GFPARM_RMODE_REREAD | GFPARM_RMODE_CREAT);

	rmMovieCapture.enabled =
		strcmp(GfParmGetStr(hparmRaceEng, RM_SECT_MOVIE_CAPTURE, RM_ATT_CAPTURE_ENABLE,
							RM_VAL_NO),
			   RM_VAL_NO) ? true : false;
	if (!rmMovieCapture.enabled)
	{
		rmMovieCapture.outputBase = 0;
		GfLogInfo("Movie capture disabled (see raceengine.xml)\n");
	}
	else
	{
		rmMovieCapture.active = false;
		rmMovieCapture.frameRate =
			GfParmGetNum(hparmRaceEng, RM_SECT_MOVIE_CAPTURE, RM_ATT_CAPTURE_FPS, NULL, 25.0);
		rmMovieCapture.simuRate = 1.0 / RCM_MAX_DT_SIMU;
		char pszDefOutputBase[256];
		snprintf(pszDefOutputBase, sizeof(pszDefOutputBase), "%s%s",
				 GfLocalDir(), GfParmGetStr(hparmRaceEng, RM_SECT_MOVIE_CAPTURE,
											RM_ATT_CAPTURE_OUT_DIR, "captures"));
		rmMovieCapture.outputBase = strdup(pszDefOutputBase);
		GfDirCreate(pszDefOutputBase); // In case not already done.
		GfLogInfo("Movie capture enabled (%.0f FPS, PNG frames in %s)\n", 
				  rmMovieCapture.frameRate, rmMovieCapture.outputBase);
	}
}

static void
rmCaptureScreen()
{
	char filename[256];
    
    snprintf(filename, sizeof(filename), "%s/sd-%4.4d-%8.8d.png", rmMovieCapture.outputBase,
			 rmMovieCapture.currentCapture, rmMovieCapture.currentFrame++);
	
    GfScrCaptureAsPNG(filename);
}

static void
rmUpdateRaceMessages()
{
	if (!rmScreenHandle)
		return;

	// Set the new text for the "message" label if it changed.
	const char *pszMsg = LmRaceEngine().outData()->_reMessage;
	if ((pszMsg && rmStrCurMsg != pszMsg) || (!pszMsg && !rmStrCurMsg.empty()))
	{
		rmStrCurMsg = pszMsg ? pszMsg : "";
		GfuiLabelSetText(rmScreenHandle, rmMsgId, rmStrCurMsg.c_str());
		
		// The menu changed.
		rmbMenuChanged = true;
	}
	
	// Set the new text for the "big message" label if it changed.
	const char *pszBigMsg = LmRaceEngine().outData()->_reBigMessage;
	if ((pszBigMsg && rmStrCurBigMsg != pszBigMsg) || (!pszBigMsg && !rmStrCurBigMsg.empty()))
	{
		rmStrCurBigMsg = pszBigMsg ? pszBigMsg : "";
		GfuiLabelSetText(rmScreenHandle, rmBigMsgId, rmStrCurBigMsg.c_str());
		
		// The menu changed.
		rmbMenuChanged = true;
	}
}

static void
rmRedisplay()
{
	// Process any pending (human) pit request.
	const bool bPitRequested = RmCheckPitRequest();
	
#ifdef UseFPSLimiter
	// Auto FPS limitation if specified and if not capturing frames.
	if (FPSLimMaxRate > 0 && !rmMovieCapture.active)
	{
		// If too early to refresh graphics, do nothing more than wait a little.
		const double dCurrentTime = GfTimeClock();
		if (dCurrentTime < FPSLimLastTime + 1.0 / FPSLimMaxRate)
		{
			// Wait a little, to let the CPU take breath.
			// Note : Theorical resolution is 1ms, but actual one is from far more
			//        (10-15ms under Windows, even worse under Linux ?)
			//        which explains a lower than expected actual FPS mean.
			GfSleep(0.001);

			// Only giving back control to the scheduler gives good results
			// as for the actual mean FPS, but keeps the CPU 100 % (not very cool).
			//GfSleep(0.0);
			
			// Request an update in the next event loop though.
			GfuiApp().eventLoop().postRedisplay();
			
			return;
		}

		// Otherwise, last update time is now : go on with graphics update.
		FPSLimLastTime = dCurrentTime;
	}
#endif

	// Redraw the graphics part of the GUI if requested.
	const bool bUpdateGraphics =
		LmRaceEngine().outData()->_displayMode == RM_DISP_MODE_NORMAL
		&& !bPitRequested && LmGraphicsEngine();
	
	if (bUpdateGraphics)
	{
		//GfSchedBeginEvent("raceupdate", "graphics");
		LmGraphicsEngine()->redrawView(LmRaceEngine().outData()->s);
		//GfSchedEndEvent("raceupdate", "graphics");
	}

	// Synchronize the menu with the race messages if any changed.
	rmUpdateRaceMessages();

	// Redraw the menu part of the GUI
	// (always necessary if the graphics were redrawn first).
	if (bUpdateGraphics || rmbMenuChanged)
		GfuiRedraw();

	// Really do the display work.
	if (bUpdateGraphics || rmbMenuChanged)
		GfuiSwapBuffers();

	// The menu changes has now been taken into account.
	rmbMenuChanged = false;

	// Capture the newly displayed frame if movie capture mode.
	if (rmMovieCapture.active)
		rmCaptureScreen();

	// Request an redisplay in the next event loop.
	GfuiApp().eventLoop().postRedisplay();
}

static void
rmScreenActivate(void * /* dummy */)
{
	// Configure the FPS limiter if active.
#ifdef UseFPSLimiter
	
	// Get the max. refresh rate from the screen config params file.
	std::ostringstream ossConfFile;
	ossConfFile << GfLocalDir() << GFSCR_CONF_FILE;
	void* hparmScrConf = GfParmReadFile(ossConfFile.str().c_str(), GFPARM_RMODE_STD);
	FPSLimLastTime = 0.0;
	FPSLimMaxRate =
		GfParmGetNum(hparmScrConf, GFSCR_SECT_VALIDPROPS, GFSCR_ATT_MAXREFRESH, NULL, 0.0);
	if (FPSLimMaxRate)
		GfLogInfo("FPS limiter is on (%.1f Hz).\n", FPSLimMaxRate);
	else
		GfLogInfo("FPS limiter is off.\n");
	
	GfParmReleaseHandle(hparmScrConf);
	
#endif

	// Deactivate the movie capture mode in any case.
	rmMovieCapture.active = false;
	
	// Configure the event loop.
	GfuiApp().eventLoop().setRecomputeCB(rmUpdateRaceEngine);
    GfuiApp().eventLoop().setRedisplayCB(rmRedisplay);

	// Resynchronize the race engine.
	LmRaceEngine().start();

	// Request a redisplay for the next event loop.
    GfuiApp().eventLoop().postRedisplay();
	
	// The menu changed.
	rmbMenuChanged = true;
}

static void
rmRacePause(void * /* vboard */)
{
    if (LmRaceEngine().outData()->s->_raceState & RM_RACE_PAUSED) {
		LmRaceEngine().start();
		GfuiVisibilitySet(rmScreenHandle, rmPauseId, GFUI_INVISIBLE);
    } else {
		LmRaceEngine().stop();
		GfuiVisibilitySet(rmScreenHandle, rmPauseId, GFUI_VISIBLE);
    }
	
	// The menu changed.
	rmbMenuChanged = true;
}

static void
rmSkipPreStart(void * /* dummy */)
{
	// TODO: move this to a new LmRaceEngine().skipRacePreStart() ...
	tRmInfo* reInfo = LmRaceEngine().inData();
    if (reInfo->s->currentTime < -1.0) {
		reInfo->s->currentTime = -1.0;
		reInfo->_reLastRobTime = -1.0;
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
	LmRaceEngine().accelerateTime(fMultFactor);
}

static void
rmToggleMovieCapture(void * /* dummy */)
{
    if (!rmMovieCapture.enabled) 
    {
		GfLogWarning("Movie capture is not enabled : command ignored\n");
		return;
    }
    
    if (LmRaceEngine().outData()->_displayMode != RM_DISP_MODE_NORMAL)
    {
		GfLogWarning("Movie capture is available only in normal display mode : command ignored\n");
		return;
    }
    
    rmMovieCapture.active = !rmMovieCapture.active;
    if (rmMovieCapture.active)
	{
		// Try and change the race engine scheduling scheme for movie capture.
		if (LmRaceEngine().setSchedulingSpecs(rmMovieCapture.simuRate, rmMovieCapture.frameRate))
		{
			rmMovieCapture.currentFrame = 0;
			rmMovieCapture.currentCapture++;
			GfLogInfo("Starting movie capture\n");
		}
		else
		{
			// Not supported (multi-threaded mode).
			rmMovieCapture.active = false;
			GfLogWarning("Movie capture not supported in multi-threaded mode : command ignored\n");
		}
    }
	else
	{
		GfLogInfo("Stopping movie capture\n");
		LmRaceEngine().setSchedulingSpecs(1.0 / RCM_MAX_DT_SIMU);
		LmRaceEngine().start(); // Resynchronize the race engine.
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
    LmRaceEngine().applyState((int)(long)pvState);
}

// Not used : see below the commented-out call.
// static void
// rmOneStep(void *pvState)
// {
//     LmRaceEngine().step((int)(long)pvState);
// }

static void
rmAddKeys()
{
    GfuiAddKey(rmScreenHandle, GFUIK_F1,  "Help", rmScreenHandle, GfuiHelpScreen, NULL);
    GfuiAddKey(rmScreenHandle, GFUIK_F12, "Screen Shot", NULL, GfuiScreenShot, NULL);

    GfuiAddKey(rmScreenHandle, '-', "Slow down Time",    (void*)-1, rmTimeMod, NULL);
    GfuiAddKey(rmScreenHandle, '+', "Accelerate Time",   (void*)+1, rmTimeMod, NULL);
    GfuiAddKey(rmScreenHandle, '.', "Restore Real Time", (void*)0, rmTimeMod, NULL);
	
    GfuiAddKey(rmScreenHandle, 'p', "Pause Race",        (void*)0, rmRacePause, NULL);
    GfuiAddKey(rmScreenHandle, GFUIK_ESCAPE,  "Stop Current Race", (void*)RE_STATE_RACE_STOP, rmApplyState, NULL);
    GfuiAddKey(rmScreenHandle, 'q', "Quit Game, Save Nothing",    (void*)RE_STATE_EXIT, rmApplyState, NULL);
    GfuiAddKey(rmScreenHandle, ' ', "Skip Pre-start",    (void*)0, rmSkipPreStart, NULL);
	
	// WARNING: Sure this won't work with multi-threading On/Auto ...
    //GfuiAddKey(rmScreenHandle, '0', "One step simulation",    (void*)1, rmOneStep, NULL);
	
    GfuiAddKey(rmScreenHandle, 'c', "Movie Capture (if enabled)", (void*)0, rmToggleMovieCapture, NULL);
    GfuiAddKey(rmScreenHandle, 'o', "Hide / Show mouse cursor",   (void*)0, rmHideShowMouseCursor, NULL);
}

void *
RmScreenInit()
{
	// Initialize the movie capture system.
	rmInitMovieCapture();
	
    // Release screen if was initialized.
    RmScreenShutdown();

    // Create screen, load menu XML descriptor and create static controls.
    rmScreenHandle = GfuiScreenCreate(black, 0, rmScreenActivate, 0, 0, 0);
    void *menuXMLDescHdle = GfuiMenuLoad("raceglscreen.xml");
    GfuiMenuCreateStaticControls(rmScreenHandle, menuXMLDescHdle);

    // Create Message, BigMessage and Pause labels.
    rmMsgId = GfuiMenuCreateLabelControl(rmScreenHandle, menuXMLDescHdle, "message");
    rmBigMsgId = GfuiMenuCreateLabelControl(rmScreenHandle, menuXMLDescHdle, "bigmessage");
    rmPauseId = GfuiMenuCreateLabelControl(rmScreenHandle, menuXMLDescHdle, "pause");

    // Close menu XML descriptor.
    GfParmReleaseHandle(menuXMLDescHdle);
    
    // Register keyboard shortcuts.
    rmAddKeys();

    // Hide the Pause label for the moment.
    GfuiVisibilitySet(rmScreenHandle, rmPauseId, 0);

    return rmScreenHandle;
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
    rmUpdateRaceEngine();
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

static const char *aSessionTypeNames[3] = {"Practice", "Qualifications", "Race"};

static void	*rmResScreenHdle = 0;

static int	rmResMainTitleId;
static int	rmResTitleId;
static int*	rmResMsgId = 0;

static int*	rmResMsgClr = 0;
static char** rmResMsg = 0;

static int	rmCurLine;

static int rmNMaxResLines = 0; // Initialized at menu-load time.

// Flag to know if the menu state has been changed (and thus needs a redraw+redisplay).
static bool rmbResMenuChanged = false;

static void
rmResRedisplay()
{
	// Redraw the menu part of the GUI if necessary.
	if (rmbResMenuChanged)
		GfuiRedraw();

	// Really do the display work.
	if (rmbResMenuChanged)
		GfuiSwapBuffers();

	// The menu changes has now been taken into account.
	rmbResMenuChanged = false;
	
	// Request an redisplay in the next event loop.
	GfuiApp().eventLoop().postRedisplay();
}

static void
rmResScreenActivate(void * /* dummy */)
{
	// Configure the event loop.
    GfuiApp().eventLoop().setRecomputeCB(rmUpdateRaceEngine);
    GfuiApp().eventLoop().setRedisplayCB(rmResRedisplay);

	// Request a redisplay for the next event loop.
	GfuiApp().eventLoop().postRedisplay();
	
	// The menu changed.
	rmbResMenuChanged = true;
}

static void
rmContDisplay()
{
    GfuiDisplay();
	
    GfuiApp().eventLoop().postRedisplay();
}

static void
rmResCont(void * /* dummy */)
{
    rmUpdateRaceEngine();
}

static void
rmResScreenShutdown(void * /* dummy */)
{
    for (int i = 1; i < rmNMaxResLines; i++)
		FREEZ(rmResMsg[i]);
}

static void
rmAddResKeys()
{
    GfuiAddKey(rmResScreenHdle, GFUIK_F1,  "Help", rmResScreenHdle, GfuiHelpScreen, NULL);
    GfuiAddKey(rmResScreenHdle, GFUIK_F12, "Screen Shot", NULL, GfuiScreenShot, NULL);

    GfuiAddKey(rmResScreenHdle, GFUIK_ESCAPE,  "Stop Current Race", (void*)RE_STATE_RACE_STOP, rmApplyState, NULL);
    GfuiAddKey(rmResScreenHdle, 'q', "Quit Game, Save Nothing", (void*)RE_STATE_EXIT, rmApplyState, NULL);
}

void*
RmResScreenInit()
{
	if (rmResScreenHdle)
		GfuiScreenRelease(rmResScreenHdle);

	tRmInfo* reInfo = LmRaceEngine().inData();

    // Create screen, load menu XML descriptor and create static controls.
    rmResScreenHdle = GfuiScreenCreate(black, 0, rmResScreenActivate, 0, rmResScreenShutdown, 0);
    void *hmenu = GfuiMenuLoad("raceblindscreen.xml");
    GfuiMenuCreateStaticControls(rmResScreenHdle, hmenu);

    // Create variable main title (race type/stage) label.
    rmResMainTitleId = GfuiMenuCreateLabelControl(rmResScreenHdle, hmenu, "Title");
    GfuiLabelSetText(rmResScreenHdle, rmResMainTitleId, aSessionTypeNames[reInfo->s->_raceType]);

    // Create background image if any specified.
    const char* img = GfParmGetStr(reInfo->params, RM_SECT_HEADER, RM_ATTR_RUNIMG, 0);
    if (img)
		GfuiScreenAddBgImg(rmResScreenHdle, img);
    
    // Create variable subtitle (driver and race name, lap number) label.
    rmResTitleId = GfuiMenuCreateLabelControl(rmResScreenHdle, hmenu, "SubTitle");

	// Get layout properties, except for nMaxResultLines (see below).
    const int yTopLine = (int)GfuiMenuGetNumProperty(hmenu, "yTopLine", 400);
    const int yLineShift = (int)GfuiMenuGetNumProperty(hmenu, "yLineShift", 20);

	// Allocate line info arrays once and for all, if not already done.
	if (!rmResMsgId)
	{
		// Load nMaxResultLines only the first time (ignore any later change,
		// otherwize, we'd have to realloc the line info arrays).
		rmNMaxResLines = (int)GfuiMenuGetNumProperty(hmenu, "nMaxResultLines", 20);
		
		rmResMsgId = (int*)calloc(rmNMaxResLines, sizeof(int));
		rmResMsg = (char**)calloc(rmNMaxResLines, sizeof(char*));
		rmResMsgClr = (int*)calloc(rmNMaxResLines, sizeof(int));
	}

    // Create result lines (1 label for each).
    // TODO: Get layout, color, ... info from hmenu when available.
    int	y = yTopLine;
    for (int i = 0; i < rmNMaxResLines; i++)
	{
		FREEZ(rmResMsg[i]);
		rmResMsgClr[i] = 0;
		rmResMsgId[i] =
			GfuiMenuCreateLabelControl(rmResScreenHdle, hmenu, "Line", true, // from template
									   "", GFUI_TPL_X, y, GFUI_TPL_FONTID, GFUI_TPL_ALIGN,
									   GFUI_TPL_MAXLEN);
		y -= yLineShift;
    }

    // Close menu XML descriptor.
    GfParmReleaseHandle(hmenu);
    
    // Register keyboard shortcuts.
    rmAddResKeys();

    // Initialize current result line.
    rmCurLine = 0;

    return rmResScreenHdle;
}

void
RmResScreenSetTrackName(int nSessionType, const char *pszTrackName)
{
    if (!rmResScreenHdle)
		return;
	
	char pszTitle[128];
	snprintf(pszTitle, sizeof(pszTitle), "%s at %s",
			 aSessionTypeNames[nSessionType], pszTrackName);
	GfuiLabelSetText(rmResScreenHdle, rmResMainTitleId, pszTitle);
	
	// The menu changed.
	rmbResMenuChanged = true;
}

void
RmResScreenSetTitle(const char *pszTitle)
{
    if (!rmResScreenHdle)
		return;
	
	GfuiLabelSetText(rmResScreenHdle, rmResTitleId, pszTitle);
	
	// The menu changed.
	rmbResMenuChanged = true;
}

void
RmResScreenAddText(const char *text)
{
    if (!rmResScreenHdle)
		return;
	
    if (rmCurLine == rmNMaxResLines)
	{
		free(rmResMsg[0]);
		for (int i = 1; i < rmNMaxResLines; i++)
		{
			rmResMsg[i - 1] = rmResMsg[i];
			GfuiLabelSetText(rmResScreenHdle, rmResMsgId[i - 1], rmResMsg[i]);
		}
		rmCurLine--;
    }
    rmResMsg[rmCurLine] = strdup(text);
    GfuiLabelSetText(rmResScreenHdle, rmResMsgId[rmCurLine], rmResMsg[rmCurLine]);
    rmCurLine++;
	
	// The menu changed.
	rmbResMenuChanged = true;
}

void
RmResScreenSetText(const char *text, int line, int clr)
{
    if (!rmResScreenHdle)
		return;
	
    if (line < rmNMaxResLines)
	{
		FREEZ(rmResMsg[line]);
		rmResMsg[line] = strdup(text);
		rmResMsgClr[line] = (clr >= 0 && clr < 2) ? clr : 0;
		GfuiLabelSetText(rmResScreenHdle, rmResMsgId[line], rmResMsg[line]);
		GfuiLabelSetColor(rmResScreenHdle, rmResMsgId[line], rmColor[rmResMsgClr[line]]);

		// The menu changed.
		rmbResMenuChanged = true;
    }
}

int
RmResGetLines()
{
    return rmNMaxResLines;
}

void
RmResEraseScreen()
{
    if (!rmResScreenHdle)
		return;
	
    for (int i = 0; i < rmNMaxResLines; i++)
		RmResScreenSetText("", i, 0);
	
	// The menu changed.
	rmbResMenuChanged = true;
}


void
RmResScreenRemoveText(int line)
{
    if (!rmResScreenHdle)
		return;
	
    if (line < rmNMaxResLines)
	{
		FREEZ(rmResMsg[line]);
		GfuiLabelSetText(rmResScreenHdle, rmResMsgId[line], "");
		
		// The menu changed.
		rmbResMenuChanged = true;
    }
}

void
RmResShowCont()
{
    GfuiButtonCreate(rmResScreenHdle, "Continue", GFUI_FONT_LARGE_C,
					 320, 15, GFUI_BTNSZ, GFUI_ALIGN_HC_VB, 0, 0, rmResCont,
					 NULL, (tfuiCallback)NULL, (tfuiCallback)NULL);
    GfuiAddKey(rmResScreenHdle, GFUIK_RETURN,  "Continue", 0, rmResCont, NULL);
    GfuiAddKey(rmResScreenHdle, GFUIK_ESCAPE,  "Continue", 0, rmResCont, NULL);

    GfuiApp().eventLoop().setRedisplayCB(rmContDisplay);
	
    GfuiApp().eventLoop().postRedisplay();
}

//************************************************************
void
RmGameScreen()
{
	GfuiScreenActivate(LmRaceEngine().inData()->_reGameScreen);
}
