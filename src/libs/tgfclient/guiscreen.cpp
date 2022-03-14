/***************************************************************************
                           screen.cpp -- screen init
                             -------------------
    created              : Fri Aug 13 22:29:56 CEST 1999
    copyright            : (C) 1999, 2004 by Eric Espie, Bernhard Wymann
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
    Screen management.
    @author	<a href=mailto:torcs@free.fr>Eric Espie</a>
    @version	$Id$
    @ingroup	screen
*/

#include <cstdio>
#include <cstring>
#include <cmath>
#include <sstream>
#include <algorithm>

#ifdef WIN32
#include <windows.h>
#include <process.h>
#else
#include <unistd.h>
#endif

#include <SDL.h>
#include <SDL_video.h>

#include <portability.h>

#include "tgfclient.h"
#include "gui.h"

#include "glfeatures.h"

// The screen properties.
static int GfScrWidth;
static int GfScrHeight;
static int GfViewWidth;
static int GfViewHeight;
static int GfScrCenX;
static int GfScrCenY;
static SDL_GLContext GLContext = NULL;

static int GfScrNumDisplays = 0;
static int GfScrStartDisplayId = 0;

// The screen surface.
static SDL_Surface *PScreenSurface = NULL;

/* Default list of screen color depths (bits per pixel, alpha included) in case
   something went wrong during hardware / driver capabilities detection */
static int ADefScreenColorDepths[] = { 16, 24, 32 };
static const int NDefScreenColorDepths =
    sizeof(ADefScreenColorDepths) / sizeof(ADefScreenColorDepths[0]);

/* Default list of screen sizes ("resolutions") in case
   something went wrong during hardware / driver capabilities detection */
static tScreenSize ADefScreenSizes[] =
{
    {  320,  200 },
    {  320,  240 },
    {  400,  300 },
    {  416,  312 },
    {  480,  360 },
    {  512,  384 },
    {  576,  384 },
    {  576,  432 },
    {  640,  384 },
    {  640,  400 },
    {  640,  480 },
    {  640,  512 },
    {  700,  525 },
    {  720,  450 },
    {  800,  512 },
    {  800,  600 },
    {  832,  624 },
    {  840,  525 },
    {  896,  672 },
    {  928,  696 },
    {  960,  600 },
    {  960,  720 },
    { 1024,  600 },
    { 1024,  768 },
    { 1152,  768 },
    { 1152,  864 },
    { 1280,  600 },
    { 1280,  720 },
    { 1280,  768 },
    { 1280,  800 },
    { 1280,  960 },
    { 1280, 1024 },
    { 1366,  768 },
    { 1400, 1050 },
    { 1440,  900 },
    { 1600,  900 },
    { 1600, 1024 },
    { 1680, 1050 },
    { 1792, 1344 },
    { 1800, 1440 },
    { 1920, 1080 },
    { 1920, 1200 },
    { 2560, 1080 },
    { 2560, 1440 },
    { 3840, 2160 }
};
static const int NDefScreenSizes =
    sizeof(ADefScreenSizes) / sizeof(ADefScreenSizes[0]);

/** Get a list of (common?) window sizes (pixels)
    @ingroup	screen
    @return	ScreenSizeVector of sizes
 */
ScreenSizeVector GfScrGetWindowSizes()
{
    ScreenSizeVector vecSizes;
    for(int i = 0; i < NDefScreenSizes; i++)
    {
        vecSizes.push_back(ADefScreenSizes[i]);
    }
    ScreenSizeVector custSizes = GfScrGetCustomWindowSizes();

    for (unsigned i = 0; i < custSizes.size(); i++)
    {
       vecSizes.push_back(custSizes[i]);
    }
    return vecSizes;
}

/** Get any custom screen / window sizes (pixels) from screen.xml
    @ingroup	screen
    @param	none
    @return	ScreenSizeVector of custom sizes
 */
ScreenSizeVector GfScrGetCustomWindowSizes()
{
   ScreenSizeVector vecSizes;

   void* hparmScreen =
      GfParmReadFileLocal(GFSCR_CONF_FILE, GFPARM_RMODE_STD | GFPARM_RMODE_CREAT);

   if (GfParmExistsSection(hparmScreen, GFSCR_SECT_WIN_MODES))
   {
      tScreenSize last;
      last.width = 0;
      last.height = 0;

      GfParmListSeekFirst(hparmScreen, GFSCR_SECT_WIN_MODES);
      do
      {
         last.width = GfParmGetCurNum(hparmScreen, GFSCR_SECT_WIN_MODES, GFSCR_ATT_WIN_X, NULL, 0);
         last.height = GfParmGetCurNum(hparmScreen, GFSCR_SECT_WIN_MODES, GFSCR_ATT_WIN_Y, NULL, 0);

         if ((last.height != 0) && (last.width != 0))
            vecSizes.push_back(last);
      }
      while (GfParmListSeekNext(hparmScreen, GFSCR_SECT_WIN_MODES) == 0);
   }
   return vecSizes;
}

/** Get the supported screen / window sizes (pixels) for the current display mode.
    @ingroup	screen
    @param	nDisplayIndex	Index of the display from which to get the sizes
    @return	ScreenSizeVector of detected supported sizes
 */
ScreenSizeVector GfScrGetSupportedSizes(int nDisplayIndex)
{
    /* Build list of available screen sizes */
    int numModes;
    SDL_DisplayMode mode;
    SDL_Rect bounds;
    ScreenSizeVector vecSizes;
    tScreenSize last;
    last.width = 0;
    last.height = 0;

    bounds.w = 0;
    bounds.h = 0;

    // make sure nDisplayIndex is valid (less than Number of displays)
    if(nDisplayIndex < GfScrGetAttachedDisplays())
    {
        if(SDL_GetDesktopDisplayMode(nDisplayIndex, &mode) == 0)
        {
            bounds.w = mode.w;
            bounds.h = mode.h;
            GfLogInfo("Display %d : %d x %d x %d @ %d hz\n", nDisplayIndex + 1, mode.w,mode.h,SDL_BITSPERPIXEL(mode.format),mode.refresh_rate);
        }
        else
        {
            GfLogError("Could not get the Display mode for Display %d \n", nDisplayIndex + 1);
            bounds.w = 0;
            bounds.h = 0;
        }

        numModes = SDL_GetNumDisplayModes(nDisplayIndex);
        GfLogInfo("Display %d : modes available %d\n", nDisplayIndex + 1, numModes);

        for(int i = 0; i < numModes; i++)
        {
            if(SDL_GetDisplayMode(nDisplayIndex, i, &mode) == 0)
            {
                // Don't allow duplicate entries
                if((mode.w != last.width) || (mode.h != last.height))
                {
                    GfLogDebug("  %d x %d x %d @ %d hz\n",mode.w,mode.h,SDL_BITSPERPIXEL(mode.format),mode.refresh_rate);
                    last.width = mode.w;
                    last.height = mode.h;
                    vecSizes.push_back(last);
                }
            }
        }
        // Reverse the order for combobox GUI
        std::reverse(vecSizes.begin(), vecSizes.end());
    }
    else
    {
        GfLogError("Invalid Display index passed to GfScrGetSupportedSizes()\n");
    }

    if(vecSizes.empty())
    {
        GfLogInfo("No supported sizes for Display .\n");

        // Desperation stick the Display Bounds into the vector
        last.width = bounds.w;
        last.height = bounds.h;
        vecSizes.push_back(last);
    }

    return vecSizes;
}

/** Get the screen dimensions 
    @ingroup	screen
    @param	nDisplayIndex	Index of the display from which to get the size
    @return	tScreenSize containing the current size
 */
tScreenSize GfScrGetCurrentDisplaySize(int nDisplayIndex)
{
    tScreenSize size;

    size.width = 0;
    size.height = 0;

    SDL_DisplayMode mode;
    if(SDL_GetCurrentDisplayMode(nDisplayIndex, &mode) == 0)
    {
        size.width = mode.w;
        size.height = mode.h;
    }
    return size;
}

/** Get the default / fallback screen / window color depths (bits per pixels, alpha included).
    @ingroup	screen
    @param	pnColorDepths	Address of number of default sizes (output).
    @return	Array of detected supported sizes (static data, never free).
 */
int* GfScrGetDefaultColorDepths(int* pnColorDepths)
{
    *pnColorDepths = NDefScreenColorDepths;

    return ADefScreenColorDepths;
}

/** Get the supported color depths as supported by the underlying hardware/driver.
    @ingroup	screen
    @param	pnDepths	Address of number of detected color depths (output)
    @return	Array of detected supported color depths (allocated on the heap, must use free at the end)
 */
int* GfScrGetSupportedColorDepths(int* pnDepths)
{
    // Need to completely re-write this function
    *pnDepths = NDefScreenColorDepths;
    return ADefScreenColorDepths;
}

static void gfScrReshapeViewport(int width, int height)
{
    GfViewWidth = GfScrWidth = width;
    GfViewHeight = GfScrHeight = height;
    GfScrCenX = width / 2;
    GfScrCenY = height / 2;

    glViewport((width-GfViewWidth)/2, (height-GfViewHeight)/2, GfViewWidth,  GfViewHeight);
    glMatrixMode(GL_PROJECTION );
    glLoadIdentity();
    glOrtho(0.0, 640.0, 0.0, 480.0, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

static void gfuiInitialWindowedPosition(int displayId, SDL_Window* window )
{
    int top = 0, left = 0, bottom = 0, right = 0, x = 0, y = 0;
    SDL_Rect rect;
    SDL_GetDisplayBounds(displayId, &rect);

    SDL_GetWindowPosition(window, &x, &y);

    SDL_GetWindowBordersSize(window, &top, &left, &bottom, &right);

    if(y < rect.y + top)
        y = rect.y + top;
    if(x < rect.x)
        x = rect.x;

    SDL_SetWindowPosition(window, x, y);
}

SDL_Surface* gfScrCreateWindow(int nWinWidth, int nWinHeight, int nTotalDepth,int bfVideoMode)
{
    if(GfuiWindow)
    {
        SDL_DestroyWindow(GfuiWindow);
        GfuiWindow = NULL;
    }
    if(PScreenSurface)
    {
        SDL_FreeSurface(PScreenSurface);
        PScreenSurface = NULL;
    }
    // Set window/icon captions
    std::ostringstream ossCaption;
    ossCaption << GfuiApp().name() << ' ' << GfuiApp().version();

    GfuiWindow = SDL_CreateWindow(ossCaption.str().c_str(),
        SDL_WINDOWPOS_CENTERED_DISPLAY(GfScrStartDisplayId), SDL_WINDOWPOS_CENTERED_DISPLAY(GfScrStartDisplayId),
        nWinWidth, nWinHeight, SDL_WINDOW_HIDDEN | SDL_WINDOW_OPENGL);


#if !defined(__APPLE__)
    // Set window icon (MUST be a 32x32 icon for Windows, and with black pixels as alpha ones,
    // as BMP doesn't support transparency).
    std::ostringstream ossIconFilename;
    ossIconFilename << GfDataDir() << "data/icons/icon.bmp";
    SDL_Surface* surfIcon = SDL_LoadBMP(ossIconFilename.str().c_str());
    if (surfIcon)
    {
        SDL_SetColorKey(surfIcon, SDL_TRUE, SDL_MapRGB(surfIcon->format, 0, 0, 0));
        SDL_SetWindowIcon(GfuiWindow, surfIcon);
        SDL_FreeSurface(surfIcon);
    }
#endif
    /* Create OpenGL context */
    GLContext = SDL_GL_CreateContext(GfuiWindow);

    // If specified, try best possible settings.
    PScreenSurface = SDL_CreateRGBSurface(0, nWinWidth, nWinHeight, nTotalDepth,
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
        0x00FF0000, 0x0000FF00, 0x000000FF,
#else
        0x000000FF, 0x0000FF00, 0x00FF0000,
#endif
        0x00000000);

    if (bfVideoMode & SDL_WINDOW_FULLSCREEN)
    {
        SDL_Rect bounds;

        /* Work around SDL2 bug */
        if (SDL_GetDisplayBounds(GfScrStartDisplayId, &bounds) == 0) {
            if (bounds.w == nWinWidth && bounds.h == nWinHeight)
                SDL_SetWindowFullscreen(GfuiWindow, SDL_WINDOW_FULLSCREEN_DESKTOP);
            else SDL_SetWindowFullscreen(GfuiWindow, SDL_WINDOW_FULLSCREEN);
        } else SDL_SetWindowFullscreen(GfuiWindow, SDL_WINDOW_FULLSCREEN);
    }
    return PScreenSurface;
}

int GfScrGetAttachedDisplays()
{
    SDL_Rect rect;
    int nDisplays = SDL_GetNumVideoDisplays();

    for(int i = 0;i < nDisplays;i++)
    {
        if(SDL_GetDisplayBounds(i,&rect) == 0)
        {
            printf("   Display (%s) : %d\n", SDL_GetDisplayName(i), i);
            printf("   %-8.8s: %d\n","left", rect.x);
            printf("   %-8.8s: %d\n","top", rect.y);
            printf("   %-8.8s: %d\n","width", rect.w);
            printf("   %-8.8s: %d\n","height", rect.h);
        }
    }
    return nDisplays;
}

bool GfScrInitSDL2(int nWinWidth, int nWinHeight, int nFullScreen)
{
#ifdef __APPLE__
    // Version d'OpenGL
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
#else
    // Version d'OpenGL
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
#endif

    // Prepare video mode.
    int bfVideoMode = SDL_WINDOW_OPENGL;

    // Initialize SDL video subsystem (and exit if not supported).
    if (SDL_InitSubSystem(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
    {
        GfLogError("Couldn't initialize SDL audio/video sub-system (%s)\n", SDL_GetError());
        return false;
    }
#if ((SDL_MAJOR_VERSION >= 2) && (SDL_PATCHLEVEL >= 5))
    SDL_SetHint(SDL_HINT_MOUSE_FOCUS_CLICKTHROUGH, "1");
#endif
    SDL_SetHint(SDL_HINT_VIDEO_MINIMIZE_ON_FOCUS_LOSS, "0");

    GfScrNumDisplays = GfScrGetAttachedDisplays();

    // Get selected frame buffer specs from config file
    // 1) Load the config file
    void* hparmScreen =
        GfParmReadFileLocal(GFSCR_CONF_FILE, GFPARM_RMODE_STD | GFPARM_RMODE_CREAT);

    // 2) Check / update test state of any 'in-test' specs.
    if (GfParmExistsSection(hparmScreen, GFSCR_SECT_INTESTPROPS))
    {
        // Remove the 'in-test' specs if the test failed (we are still in the 'in progress'
        // test state because the game crashed during the test).
        if (std::string(GfParmGetStr(hparmScreen, GFSCR_SECT_INTESTPROPS, GFSCR_ATT_TESTSTATE,
                                     GFSCR_VAL_INPROGRESS)) == GFSCR_VAL_INPROGRESS)
        {
            GfLogInfo("Reverting to last validated screen specs, as last test failed.\n");
            GfParmRemoveSection(hparmScreen, GFSCR_SECT_INTESTPROPS);
        }

        // If the test has not yet been done, mark it as 'in progress'
        else
        {
            GfLogInfo("Testing new screen specs : let's see what's happening ...\n");
            GfParmSetStr(hparmScreen, GFSCR_SECT_INTESTPROPS, GFSCR_ATT_TESTSTATE,
                         GFSCR_VAL_INPROGRESS);
        }

        // Write the config file to disk (in case the forthcoming test makes the game crash,
        // or in order the Options / Display menu shows the actual current settings).
        GfParmWriteFile(NULL, hparmScreen, "Screen");
    }

    // 3) Select the 'in-test' specs if present, otherwise the 'validated' ones.
    const char* pszScrPropSec =
        GfParmExistsSection(hparmScreen, GFSCR_SECT_INTESTPROPS)
        ? GFSCR_SECT_INTESTPROPS : GFSCR_SECT_VALIDPROPS;

    // 4) Get/Read the specs.
    if (nWinWidth < 0)
        nWinWidth =
            (int)GfParmGetNum(hparmScreen, pszScrPropSec, GFSCR_ATT_WIN_X, (char*)NULL, 800);
    if (nWinHeight < 0)
        nWinHeight =
            (int)GfParmGetNum(hparmScreen, pszScrPropSec, GFSCR_ATT_WIN_Y, (char*)NULL, 600);
    int nTotalDepth =
        (int)GfParmGetNum(hparmScreen, pszScrPropSec, GFSCR_ATT_BPP, (char*)NULL, 32);
    GfScrStartDisplayId =
        (int)GfParmGetNum(hparmScreen, pszScrPropSec, GFSCR_ATT_STARTUPDISPLAY, (char*)NULL, 0);
    if(GfScrStartDisplayId >= GfScrNumDisplays)
    {
        GfScrStartDisplayId = 0;
    }

    bool bFullScreen;
    if (nFullScreen < 0)
        bFullScreen =
            std::string(GfParmGetStr(hparmScreen, pszScrPropSec, GFSCR_ATT_FSCR, GFSCR_VAL_NO))
            == GFSCR_VAL_YES;
    else
        bFullScreen = nFullScreen ? true : false;

    if(bFullScreen)
        bfVideoMode |= SDL_WINDOW_FULLSCREEN;

//* TODO : move and re-implement these? 
    bool bAlphaChannel =
        std::string(GfParmGetStr(hparmScreen, pszScrPropSec, GFSCR_ATT_ALPHACHANNEL,
        GFSCR_VAL_YES))
        == GFSCR_VAL_YES;

    bool bBumpMap =
        std::string(GfParmGetStr(hparmScreen, pszScrPropSec, GFSCR_ATT_BUMPMAPPING,
                                 GFSCR_VAL_NO))
        == GFSCR_VAL_YES;

    int nAniFilt =
        (int)GfParmGetNum(hparmScreen, pszScrPropSec, GFSCR_ATT_ANISOTROPICFILTERING, (char*)NULL, 0);

    bool bStereo =
        std::string(GfParmGetStr(hparmScreen, pszScrPropSec, GFSCR_ATT_STEREOVISION,
                                 GFSCR_VAL_NO))
        == GFSCR_VAL_YES;
    bool bTryBestVInitMode =
        std::string(GfParmGetStr(hparmScreen, pszScrPropSec, GFSCR_ATT_VINIT,
                                 GFSCR_VAL_VINIT_BEST))
        == GFSCR_VAL_VINIT_BEST;


    // TODO ?
    // Add new values to the config OpenGL Major and Minor
    // and setup GL Major/Minor before window creation
    // SDL_GL_SetSwapInterval(1) for for vsync (may have to go AFTER window creation)

        if (bTryBestVInitMode)
    {
        GfLogInfo("Trying 'best possible mode' for video initialization.\n");

        // Detect best supported features for the specified frame buffer specs.
        // Warning: Restarts the game if the frame buffer specs changed since last call.
        // If specified and possible, setup the best possible settings.
        if (GfglFeatures::self().checkBestSupport(nWinWidth, nWinHeight, nTotalDepth,
                                                  bAlphaChannel, bFullScreen, bBumpMap, bStereo,nAniFilt,hparmScreen))
        {
            // Load Open GL user settings from the config file.
            GfglFeatures::self().loadSelection();

            // Setup the video mode parameters.
            const int nColorDepth =
                GfglFeatures::self().getSelected(GfglFeatures::ColorDepth);
            SDL_GL_SetAttribute(SDL_GL_RED_SIZE, nColorDepth/3);
            SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, nColorDepth/3);
            SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, nColorDepth/3);
            SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, nColorDepth);

            const int nAlphaDepth =
                GfglFeatures::self().getSelected(GfglFeatures::AlphaDepth);
            SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, nAlphaDepth);

            const int nDoubleBuffer =
                GfglFeatures::self().isSelected(GfglFeatures::DoubleBuffer) ? 1 : 0;
            SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, nDoubleBuffer);

            const int nMultiSampling =
                GfglFeatures::self().isSelected(GfglFeatures::MultiSampling) ? 1 : 0;
            SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, nMultiSampling);
            if (nMultiSampling)
            {
                const int nMaxMultiSamples =
                    GfglFeatures::self().getSelected(GfglFeatures::MultiSamplingSamples);
                SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, nMaxMultiSamples);
            }

            const int nStereoVision =
                GfglFeatures::self().isSelected(GfglFeatures::StereoVision) ? 1 : 0;
            SDL_GL_SetAttribute(SDL_GL_STEREO, nStereoVision);

            // Try the video mode with these parameters : should always work
            // (unless you downgraded you hardware / OS and didn't clear your config file).
            PScreenSurface = gfScrCreateWindow(nWinWidth, nWinHeight, nTotalDepth,bfVideoMode);
        }

        // If best mode not supported, or test actually failed,
        // revert to a supported mode (restart the game).
        if (!PScreenSurface)
        {
            GfLogWarning("Failed to setup best supported video mode "
                         "whereas previously detected !\n");
            GfLogWarning("Tip: You should remove your %s%s file and restart,\n",
                         GfLocalDir(), GFSCR_CONF_FILE);
            GfLogWarning("     if something changed in your OS"
                         " or video hardware/driver configuration.\n");

            // If testing new screen specs, remember that the test failed
            // in order to revert to the previous validated specs on restart.
            if (std::string(pszScrPropSec) == GFSCR_SECT_INTESTPROPS)
            {
                GfParmSetStr(hparmScreen, pszScrPropSec, GFSCR_ATT_TESTSTATE,
                             GFSCR_VAL_FAILED);
            }

            // Force compatible video init. mode if not testing a new video mode.
            else
            {
                GfLogWarning("Falling back to a more compatible default mode ...\n");
                GfParmSetStr(hparmScreen, pszScrPropSec, GFSCR_ATT_VINIT,
                             GFSCR_VAL_VINIT_COMPATIBLE);
            }
            GfParmWriteFile(NULL, hparmScreen, "Screen");
            GfParmReleaseHandle(hparmScreen);

            // And restart the game.
            GfuiApp().restart(); // Never returns.
        }
    }

    // Video initialization with generic compatible settings.
    if (!PScreenSurface)
    {
        GfLogInfo("Trying 'default compatible' mode for video initialization.\n");

        // cancel StereoVision
        SDL_GL_SetAttribute(SDL_GL_STEREO, 0);

        //SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
        //SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 8);

        PScreenSurface = gfScrCreateWindow(nWinWidth, nWinHeight, nTotalDepth,bfVideoMode);
        if (!PScreenSurface)
            GfLogTrace("Can't get a %s%dx%dx%d compatible video mode\n",
                       bFullScreen ? "full-screen " : "", nWinWidth, nWinHeight, nTotalDepth);
    }

    // Failed : Try and remove the full-screen requirement if present ...
    if (!PScreenSurface && bFullScreen)
    {
        bfVideoMode &= ~SDL_WINDOW_FULLSCREEN;
        PScreenSurface = gfScrCreateWindow(nWinWidth, nWinHeight, nTotalDepth,bfVideoMode);
        if (!PScreenSurface)
            GfLogTrace("Can't get a non-full-screen %dx%dx%d compatible video mode\n",
                       nWinWidth, nWinHeight, nTotalDepth);

        // Update screen specs.
        GfParmSetStr(hparmScreen, pszScrPropSec, GFSCR_ATT_FSCR, GFSCR_VAL_NO);
        GfParmWriteFile(NULL, hparmScreen, "Screen");
    }

    // Failed : Try with a lower fallback size  : should be supported everywhere ...
    if (!PScreenSurface)
    {
        nWinWidth = ADefScreenSizes[0].width;
        nWinHeight = ADefScreenSizes[0].height;
        PScreenSurface = gfScrCreateWindow(nWinWidth, nWinHeight, nTotalDepth,bfVideoMode);
        if (!PScreenSurface)
            GfLogTrace("Can't get a %dx%dx%d compatible video mode\n",
                       nWinWidth, nWinHeight, nTotalDepth);

        // Update screen specs.
        GfParmSetNum(hparmScreen, pszScrPropSec, GFSCR_ATT_WIN_X, 0, (tdble)nWinWidth);
        GfParmSetNum(hparmScreen, pszScrPropSec, GFSCR_ATT_WIN_Y, 0, (tdble)nWinHeight);
        GfParmWriteFile(NULL, hparmScreen, "Screen");
    }

    // Failed : Try with a lower fallback color depth : should be supported everywhere ...
    if (!PScreenSurface)
    {
        nTotalDepth = ADefScreenColorDepths[0];
        PScreenSurface = gfScrCreateWindow(nWinWidth, nWinHeight, nTotalDepth,bfVideoMode);
        if (!PScreenSurface)
            GfLogTrace("Can't get a %dx%dx%d compatible video mode\n",
                       nWinWidth, nWinHeight, nTotalDepth);

        // Update screen specs.
        GfParmSetNum(hparmScreen, pszScrPropSec, GFSCR_ATT_BPP, 0, (tdble)nTotalDepth);
        GfParmWriteFile(NULL, hparmScreen, "Screen");
    }

    // Close the config file.
    GfParmReleaseHandle(hparmScreen);

    // Failed : No way ... no more ideas !
    if (!PScreenSurface)
    {
        GfLogError("Unable to get any compatible video mode"
                   " (fallback resolution / color depth not supported) : giving up !\n\n");
        return false;
    }

    // If we get here, that's because we succeeded in getting a valid video mode :-)

    // If 'compatible mode' selected, detect only standard Open GL features
    // and load OpenGL settings from the config file.
    if (!bTryBestVInitMode)
    {
        GfglFeatures::self().detectStandardSupport();
        GfglFeatures::self().dumpSupport();
        GfglFeatures::self().loadSelection();
    }

    // Save view geometry and screen center.
    GfViewWidth = nWinWidth;
    GfViewHeight = nWinHeight;
    GfScrCenX = nWinWidth / 2;
    GfScrCenY = nWinHeight / 2;

    // Report about selected SDL video mode.
    GfLogInfo("Selected SDL video mode :\n");
    GfLogInfo("  Full screen : %s\n", (bfVideoMode & SDL_WINDOW_FULLSCREEN) ? "Yes" : "No");
    GfLogInfo("  Size        : %dx%d\n", nWinWidth, nWinHeight);
    GfLogInfo("  Color depth : %d bits\n", nTotalDepth);

    // Report about underlying hardware (needs a running frame buffer).
    GfglFeatures::self().dumpHardwareInfo();

    if(GfuiWindow)
    {
        SDL_SetWindowPosition(GfuiWindow, SDL_WINDOWPOS_CENTERED_DISPLAY(GfScrStartDisplayId), SDL_WINDOWPOS_CENTERED_DISPLAY(GfScrStartDisplayId));
        SDL_ShowWindow(GfuiWindow);
        SDL_RestoreWindow(GfuiWindow);
    }

    // Position Window if not full screen
    if (!(bfVideoMode & SDL_WINDOW_FULLSCREEN))
    {
        gfuiInitialWindowedPosition(GfScrStartDisplayId, GfuiWindow);
    }


    // Initialize the Open GL viewport.
    gfScrReshapeViewport(nWinWidth, nWinHeight);

    // Setup the event loop about the new display.
    GfuiApp().eventLoop().setReshapeCB(gfScrReshapeViewport);
    GfuiApp().eventLoop().postRedisplay();
    return true;
}
bool GfScrInit(int nWinWidth, int nWinHeight, int nFullScreen)
{
    return GfScrInitSDL2(nWinWidth,nWinHeight,nFullScreen);
}

/** Shutdown the screen
    @ingroup	screen
    @return	none
*/
void GfScrShutdown(void)
{
    GfLogTrace("Shutting down screen.\n");

    SDL_GL_MakeCurrent(GfuiWindow,GLContext);
    SDL_GL_DeleteContext(GLContext);
    GLContext = NULL;

    // Shutdown SDL video sub-system.
    SDL_QuitSubSystem(SDL_INIT_VIDEO | SDL_INIT_AUDIO);

    // If there's an 'in-test' screen properties section in the config file,
    // * if the test state is 'to do', do nothing (will be taken care of in next GfScrInit),
    // * if the test state is 'in progress', validate the new screen properties,
    // * if the test state is 'failed', revert to the validated screen properties.
    void* hparmScreen = GfParmReadFileLocal(GFSCR_CONF_FILE, GFPARM_RMODE_STD);

    if (GfParmExistsSection(hparmScreen, GFSCR_SECT_INTESTPROPS))
    {
        if (std::string(GfParmGetStr(hparmScreen, GFSCR_SECT_INTESTPROPS, GFSCR_ATT_TESTSTATE,
                                     GFSCR_VAL_INPROGRESS)) == GFSCR_VAL_INPROGRESS)
        {
            GfLogInfo("Validating new screen specs (test was successful).\n");

            // Copy the 'in test' props to the 'validated' ones.
            GfParmSetNum(hparmScreen, GFSCR_SECT_VALIDPROPS, GFSCR_ATT_WIN_X, 0,
                         GfParmGetNum(hparmScreen, GFSCR_SECT_INTESTPROPS, GFSCR_ATT_WIN_X, 0, 800));
            GfParmSetNum(hparmScreen, GFSCR_SECT_VALIDPROPS, GFSCR_ATT_WIN_Y, 0,
                         GfParmGetNum(hparmScreen, GFSCR_SECT_INTESTPROPS, GFSCR_ATT_WIN_Y, 0, 600));
            GfParmSetNum(hparmScreen, GFSCR_SECT_VALIDPROPS, GFSCR_ATT_BPP, 0,
                         GfParmGetNum(hparmScreen, GFSCR_SECT_INTESTPROPS, GFSCR_ATT_BPP, 0, 32));
            GfParmSetNum(hparmScreen, GFSCR_SECT_VALIDPROPS, GFSCR_ATT_STARTUPDISPLAY, 0,
                         GfParmGetNum(hparmScreen, GFSCR_SECT_INTESTPROPS, GFSCR_ATT_STARTUPDISPLAY, 0, 0));
            GfParmSetStr(hparmScreen, GFSCR_SECT_VALIDPROPS, GFSCR_ATT_VDETECT,
                         GfParmGetStr(hparmScreen, GFSCR_SECT_INTESTPROPS, GFSCR_ATT_VDETECT, GFSCR_VAL_VDETECT_AUTO));
            const  char* pszVInitMode =
                GfParmGetStr(hparmScreen, GFSCR_SECT_INTESTPROPS, GFSCR_ATT_VINIT,
                             GFSCR_VAL_VINIT_COMPATIBLE);
            GfParmSetStr(hparmScreen, GFSCR_SECT_VALIDPROPS, GFSCR_ATT_VINIT, pszVInitMode);
            GfParmSetStr(hparmScreen, GFSCR_SECT_VALIDPROPS, GFSCR_ATT_FSCR,
                         GfParmGetStr(hparmScreen, GFSCR_SECT_INTESTPROPS, GFSCR_ATT_FSCR, GFSCR_VAL_NO));
            // Store OpenGL settings if best video init mode selected
            // (because loadSelection can changed them).
            if (std::string(pszVInitMode) == GFSCR_VAL_VINIT_BEST)
                GfglFeatures::self().storeSelection(hparmScreen);
        }
        else if (std::string(GfParmGetStr(hparmScreen, GFSCR_SECT_INTESTPROPS, GFSCR_ATT_TESTSTATE,
                                          GFSCR_VAL_INPROGRESS)) == GFSCR_VAL_FAILED)
        {
            GfLogInfo("Canceling new screen specs, back to old ones (test failed).\n");
        }


        if (std::string(GfParmGetStr(hparmScreen, GFSCR_SECT_INTESTPROPS, GFSCR_ATT_TESTSTATE,
                                     GFSCR_VAL_INPROGRESS)) != GFSCR_VAL_TODO)
        {
            // Remove the 'in-test' section.
            GfParmRemoveSection(hparmScreen, GFSCR_SECT_INTESTPROPS);

            // Write the screen config file to disk.
            GfParmWriteFile(NULL, hparmScreen, "Screen");
        }
        else
        {
            GfLogInfo("New screen specs will be tested when restarting.\n");
        }
    }

    // Release screen config params file.
    GfParmReleaseHandle(hparmScreen);
}


/** Get the screen and viewport sizes.
    @ingroup	screen
    @param	scrw	address of screen with
    @param	scrh	address of screen height
    @param	vieww	address of viewport with
    @param	viewh	address of viewport height
    @return	none
 */
void GfScrGetSize(int *scrW, int *scrH, int *viewW, int *viewH)
{
    *scrW = GfScrWidth;
    *scrH = GfScrHeight;
    *viewW = GfViewWidth;
    *viewH = GfViewHeight;
}

bool GfScrToggleFullScreen()
{
    Uint32 flags = SDL_GetWindowFlags(GfuiWindow);

    if ((flags & SDL_WINDOW_FULLSCREEN) || (flags & SDL_WINDOW_FULLSCREEN_DESKTOP)) {
        SDL_SetWindowFullscreen(GfuiWindow, 0);
        return false;
    } else {
        SDL_Rect bounds;

        /* Work around SDL2 bug */
        if (SDL_GetDisplayBounds(GfScrStartDisplayId, &bounds) == 0) {
            if (bounds.w == GfScrWidth && bounds.h == GfScrHeight)
                SDL_SetWindowFullscreen(GfuiWindow, SDL_WINDOW_FULLSCREEN_DESKTOP);
            else SDL_SetWindowFullscreen(GfuiWindow, SDL_WINDOW_FULLSCREEN);
        } else SDL_SetWindowFullscreen(GfuiWindow, SDL_WINDOW_FULLSCREEN);

        return true;
    }
}

/** Capture screen pixels into an RGB buffer (caller must free the here-allocated buffer).
    @ingroup	screen
    @param	scrw	address of screen with
    @param	scrh	address of screen height
    @return	none
 */
unsigned char* GfScrCapture(int* viewW, int *viewH)
{
    unsigned char *img;
    int sW, sH;

    GfScrGetSize(&sW, &sH, viewW, viewH);
    img = (unsigned char*)malloc((*viewW) * (*viewH) * 3);
    if (img)
    {
        glPixelStorei(GL_PACK_ROW_LENGTH, 0);
        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        glReadBuffer(GL_FRONT);
        glReadPixels((sW-(*viewW))/2, (sH-(*viewH))/2, *viewW, *viewH,
                     GL_RGB, GL_UNSIGNED_BYTE, (GLvoid*)img);
    }

    return img;
}

/** Capture screen pixels into a PNG file
    @ingroup	screen
    @param	filename	filename of the png file
    @return	0 Ok
        <br>-1 Error
 */
int GfScrCaptureAsPNG(const char *filename)
{
    int viewW, viewH;

    // Capture screen to an RGB image (in memory) (and measure elapsed time).
    const double dCaptureBeginTime = GfTimeClock();

    unsigned char* img = GfScrCapture(&viewW, &viewH);

    const double dCaptureEndTime = GfTimeClock();

    const double dCaptureDuration = dCaptureEndTime - dCaptureBeginTime;

    // Write RGB image to the PNG file (and measure elapsed time).
    const int nStatus = GfTexWriteImageToPNG(img, filename, viewW, viewH);

    const double dFileWriteDuration = GfTimeClock() - dCaptureEndTime;

    // Free the image buffer.
    if (img)
        free(img);

    if (!nStatus)
        GfLogTrace("Captured screen to %s (capture=%.3f s, PNG=%.3f s)\n",
                   filename, dCaptureDuration, dFileWriteDuration);
    else
        GfLogError("Failed to capture screen to %s\n", filename);

    return nStatus;
}

SDL_Window* GfScrGetMainWindow()
{
    return GfuiWindow;
}
