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

#ifdef WIN32
#ifndef HAVE_CONFIG_H
#define HAVE_CONFIG_H
#endif
#endif

#include <cstdio>
#include <cstring>
#include <cmath>
#ifdef WIN32
#include <windows.h>
#include <process.h>
#else
#include <unistd.h>
#endif

#include <SDL/SDL.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#include "version.h"
#endif

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

// The screen surface.
SDL_Surface *ScreenSurface = NULL;

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
	{  960, 1200 },
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
};
static const int NDefScreenSizes =
	sizeof(ADefScreenSizes) / sizeof(ADefScreenSizes[0]);

/** Get the default / fallback screen / window sizes (pixels).
    @ingroup	screen
    @param	pnSizes	Address of number of default sizes (output).
    @return	Array of detected supported sizes (static data, never free).
 */
tScreenSize* GfScrGetDefaultSizes(int* pnSizes)
{
	*pnSizes = NDefScreenSizes;
	
	return ADefScreenSizes;
}

/** Get the supported screen / window sizes (pixels) for the given color depth and display mode.
    @ingroup	screen
    @param	nColorDepth	Requested color depth (bits)
    @param	bFullScreen	Requested display mode : full-screeen mode if true, windowed otherwise.
    @param	pnSizes	Address of number of detected supported sizes (output) (-1 if any size is supported).
    @return	Array of detected supported sizes (allocated on the heap, must use free at the end), or 0 if no detected supported size, or -1 if any size is supported.
	@note   The vertical refresh rate is not taken into account as a parameter for detection here, due to SDL API not supporting this ; fortunately, when selecting a given video mode, SDL ensures to (silently) select a safe refresh rate for the selected mode, which may be of some importantce especially in full-screen modes.
 */
tScreenSize* GfScrGetSupportedSizes(int nColorDepth, bool bFullScreen, int* pnSizes)
{
	// Query system video capabilities.
	const SDL_VideoInfo* sdlVideoInfo = SDL_GetVideoInfo();
	if (!sdlVideoInfo)
	{
		GfLogWarning("Could not SDL_GetVideoInfo (%s)\n", SDL_GetError());
		*pnSizes = 0;
		return 0;
	}
	
	// Get best supported pixel format.
	SDL_PixelFormat sdlPixelFormat;
	memcpy(&sdlPixelFormat, &(sdlVideoInfo->vfmt), sizeof(SDL_PixelFormat));
		
	//sdlPixelFormat.palette = 0;
	//sdlPixelFormat.BitsPerPixel = ;
	//sdlPixelFormat.BytesPerPixel = ;
	//sdlPixelFormat.Rloss = ;
	//sdlPixelFormat.Gloss = ;
	//sdlPixelFormat.Bloss = ;
	//sdlPixelFormat.Aloss = ;
	//sdlPixelFormat.Rshift = ;
	//sdlPixelFormat.Gshift = ;
	//sdlPixelFormat.Bshift = ;
	//sdlPixelFormat.Ashift = ;
	//sdlPixelFormat.Rmask = ;
	//sdlPixelFormat.Gmask = ;
	//sdlPixelFormat.Bmask = ;
	//sdlPixelFormat.Amask = ;
	//sdlPixelFormat.colorkey = ;
	//sdlPixelFormat.alpha = ;

	// Update the pixel format to match the requested color depth.
	sdlPixelFormat.BitsPerPixel = nColorDepth;
	sdlPixelFormat.BytesPerPixel = nColorDepth / 8;

	// Select the requested display mode.
	Uint32 sdlDisplayMode = SDL_OPENGL;
	if (bFullScreen)
		sdlDisplayMode |= SDL_FULLSCREEN;
	
	// Get the supported sizes for this pixel format.
	SDL_Rect **asdlSuppSizes = SDL_ListModes(&sdlPixelFormat, sdlDisplayMode);

	GfLogInfo("Available %u-bit %s video sizes :",
			  sdlPixelFormat.BitsPerPixel, bFullScreen ? "full-screen" : "windowed");

	tScreenSize* aSuppSizes;
	if (asdlSuppSizes == (SDL_Rect**)0)
	{
		GfLogInfo(" None.\n");
		aSuppSizes = (tScreenSize*)0;
		*pnSizes = 0;
	}
	else if (asdlSuppSizes == (SDL_Rect**)-1)
	{
		GfLogInfo(" Any.\n");
		aSuppSizes = (tScreenSize*)-1;
		*pnSizes = -1;
	}
	else
	{
		// Count the supported sizes.
		*pnSizes = 0;
		while (asdlSuppSizes[*pnSizes])
			(*pnSizes)++;

		// Copy them into the output array.
		aSuppSizes = (tScreenSize*)malloc((*pnSizes)*sizeof(tScreenSize));
		for (int nSizeInd = 0; nSizeInd < *pnSizes; nSizeInd++)
		{
			aSuppSizes[nSizeInd].width  = asdlSuppSizes[*pnSizes - 1 - nSizeInd]->w;
			aSuppSizes[nSizeInd].height = asdlSuppSizes[*pnSizes - 1 - nSizeInd]->h;
			GfLogInfo(" %dx%d,", aSuppSizes[nSizeInd].width, aSuppSizes[nSizeInd].height);
		}
		GfLogInfo("\n");
	}
	
	return aSuppSizes;
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
	// Determine the maximum supported color depth (default to 32 in any case).
	const SDL_VideoInfo* sdlVideoInfo = SDL_GetVideoInfo();
	int nMaxColorDepth = 32;
	if (sdlVideoInfo)
	{
		nMaxColorDepth = sdlVideoInfo->vfmt->BitsPerPixel;
		if (nMaxColorDepth > 32)
			nMaxColorDepth = 32;
	}
	else
		GfLogWarning("Could not SDL_GetVideoInfo (%s)\n", SDL_GetError());

	// We support a minimum color depth of 16 bits.
	const int nMinColorDepth = 16;

	// So we can't have more than ... supported color depths.
	const int nMaxColorDepths = 1 + (nMaxColorDepth - nMinColorDepth) / 8;
	
	// Check video backend capabilities for each color depth between min and max,
	// and store in target array if OK.
	int nSuppSizes;
	tScreenSize* aSuppSizes;
	int* aSuppDepths = (int*)malloc(nMaxColorDepths*sizeof(int));
	*pnDepths = 0;
	for (int nDepthInd = 0; nDepthInd < nMaxColorDepths; nDepthInd++)
	{
		const int nCheckedColorDepth = nMinColorDepth + 8 * nDepthInd;

		// Windowed mode.
		aSuppSizes = GfScrGetSupportedSizes(nCheckedColorDepth, false, &nSuppSizes);
		const bool bWindowedOK = (aSuppSizes != 0);
		if (aSuppSizes && aSuppSizes != (tScreenSize*)-1)
			free(aSuppSizes);

		// Full-screen mode
		aSuppSizes = GfScrGetSupportedSizes(nCheckedColorDepth, true, &nSuppSizes);
		const bool bFullScreenOK = (aSuppSizes != 0);
		if (aSuppSizes && aSuppSizes != (tScreenSize*)-1)
			free(aSuppSizes);

		// Keep this color depth if one of the display modes work
		// TODO: Shouldn't we use "and" here ?
		if (bWindowedOK || bFullScreenOK)
		{
			aSuppDepths[*pnDepths] = nCheckedColorDepth;
			(*pnDepths)++;
		}
	}

	// Report supported depths.
	if (*pnDepths == 0)
	{
		// Fallback : assume at least 24 bit depth is supported.
		GfLogWarning("SDL reports no supported color depth : assuming 32 bit is OK");
		aSuppDepths[*pnDepths] = 32;
		(*pnDepths)++;
	}
	else
	{
		GfLogInfo("Supported color depths (bits) :");
		for (int nDepthInd = 0; nDepthInd < *pnDepths; nDepthInd++)
			GfLogInfo(" %d,", aSuppDepths[nDepthInd]);
		GfLogInfo("\n");
	}

	return aSuppDepths;
}

static void gfScrReshapeViewport(int width, int height)
{
    glViewport( (width-GfViewWidth)/2, (height-GfViewHeight)/2, GfViewWidth,  GfViewHeight);
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    glOrtho( 0.0, 640.0, 0.0, 480.0, -1.0, 1.0 );
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();

    GfScrWidth = width;
    GfScrHeight = height;
    GfScrCenX = width / 2;
    GfScrCenY = height / 2;
}

bool GfScrInit(void)
{
	char 	buf[512];
    int		xw, yw;
    int		winX, winY;
    void	*handle;
    const char	*fscr;
    const char	*vinit;
    SDL_Surface *icon;

    int		maxfreq;
    int		depth;

	// Initialize SDL video subsystem (and exit if not supported).
	if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0)
	{
		GfLogError("Couldn't initialize SDL video sub-system (%s)\n", SDL_GetError());
		return false;
	}
	
	// Enable unicode translation for SDL key press events, even if already done before
	// (SDL_InitSubSystem(SDL_INIT_VIDEO) seems to break it).
	SDL_EnableUNICODE(/*enable=*/1);
	
	// Query system video capabilities.
	// Note: Does not work very well as long as you don't force SDL to use
	//       a special hardware driver ... which we don't want at all (the default is the one).
//typedef struct{
//  Uint32 hw_available:1;
//  Uint32 wm_available:1;
//  Uint32 blit_hw:1;
//  Uint32 blit_hw_CC:1;
//  Uint32 blit_hw_A:1;
//  Uint32 blit_sw:1;
//  Uint32 blit_sw_CC:1;
//  Uint32 blit_sw_A:1;
//  Uint32 blit_fill;
//  Uint32 video_mem;
//  SDL_PixelFormat *vfmt;
//} SDL_VideoInfo;
	const SDL_VideoInfo* sdlVideoInfo = SDL_GetVideoInfo();
	
	if (!sdlVideoInfo)
	{
		GfLogError("Could not SDL_GetVideoInfo (%s)\n", SDL_GetError());
		return false;
	}
	
	char pszDriverName[32];
	GfLogTrace("SDL video driver : '%s'\n", SDL_VideoDriverName(pszDriverName, 32));
	//GfLogTrace("  Hardware acceleration : %s\n", sdlVideoInfo->hw_available ? "true" : "false");
	//GfLogTrace("  Total video memory    : %u Kb\n", sdlVideoInfo->video_mem);
	GfLogTrace("Maximum pixel depth : %d bits\n", sdlVideoInfo->vfmt->BitsPerPixel);

	// Get graphical settings from config file.
    sprintf(buf, "%s%s", GfLocalDir(), GFSCR_CONF_FILE);
    handle = GfParmReadFile(buf, GFPARM_RMODE_STD | GFPARM_RMODE_CREAT);
    xw = (int)GfParmGetNum(handle, GFSCR_SECT_PROP, GFSCR_ATT_X, (char*)NULL, 800);
    yw = (int)GfParmGetNum(handle, GFSCR_SECT_PROP, GFSCR_ATT_Y, (char*)NULL, 600);
    winX = (int)GfParmGetNum(handle, GFSCR_SECT_PROP, GFSCR_ATT_WIN_X, (char*)NULL, xw);
    winY = (int)GfParmGetNum(handle, GFSCR_SECT_PROP, GFSCR_ATT_WIN_Y, (char*)NULL, yw);
    depth = (int)GfParmGetNum(handle, GFSCR_SECT_PROP, GFSCR_ATT_BPP, (char*)NULL, 32);
    maxfreq = (int)GfParmGetNum(handle, GFSCR_SECT_PROP, GFSCR_ATT_MAXREFRESH, (char*)NULL, 160);

    fscr = GfParmGetStr(handle, GFSCR_SECT_PROP, GFSCR_ATT_FSCR, GFSCR_VAL_NO);

    vinit = GfParmGetStr(handle, GFSCR_SECT_PROP, GFSCR_ATT_VINIT, GFSCR_VAL_VINIT_COMPATIBLE);

	// Set window/icon captions
	sprintf(buf, "Speed Dreams %s", VERSION_LONG);
	SDL_WM_SetCaption(buf, buf);

	// Set window icon (MUST be a 32x32 icon for Windows, and with black pixels as alpha ones, 
	// as BMP doesn't support transparency).
	sprintf(buf, "%sdata/icons/icon.bmp", GfDataDir());
	if ((icon = SDL_LoadBMP(buf)))
	{
	    SDL_SetColorKey(icon, SDL_SRCCOLORKEY, SDL_MapRGB(icon->format, 0, 0, 0));
	    SDL_WM_SetIcon(icon, 0);
	    SDL_FreeSurface(icon);
	}

	// Set full screen mode if required.
	int videomode = SDL_OPENGL; // What about SDL_DOUBLEBUFFER ?
	if (!strcmp(fscr, "yes"))
		videomode |= SDL_FULLSCREEN;
 
	// Video initialization with best possible settings.
	// TODO: Shouldn't we use only SDL_SetVideoMode here, and move SDL_GL_SetAttribute
	//       into the graphics engine ? Do we need multi-sampling in menus ?
	ScreenSurface = 0;
	if (strcmp(vinit, GFSCR_VAL_VINIT_BEST) == 0) 
	{
		// TODO: Check for better than default multi-sampling level
		//       through SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, level);
		// Could not get it automatically detected till now.

		/* Set the minimum requirements for the OpenGL window */
		SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);

		// Try to get "best" videomode with default anti-aliasing support :
		// 24 bit z-buffer with alpha channel.
		SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
		ScreenSurface = SDL_SetVideoMode(winX, winY, depth, videomode);

		// Failed : try without anti-aliasing.
		if (!ScreenSurface)
		{
			SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
			ScreenSurface = SDL_SetVideoMode(winX, winY, depth, videomode);
		}

		// Failed : try with anti-aliasing, but without alpha channel.
		if (!ScreenSurface)
		{
			SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
			SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 0);
			ScreenSurface = SDL_SetVideoMode(winX, winY, depth, videomode);
		}

		// Failed : try without anti-aliasing, and without alpha channel.
		if (!ScreenSurface)
		{
			SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
			ScreenSurface = SDL_SetVideoMode(winX, winY, depth, videomode);
		}

		// Failed : try 16 bit z-buffer and back with alpha channel and anti-aliasing.
		if (!ScreenSurface)
		{
			SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
			SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
			SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
			ScreenSurface = SDL_SetVideoMode(winX, winY, depth, videomode);
		}

		// Failed : try without anti-aliasing.
		if (!ScreenSurface)
		{
			SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
			ScreenSurface = SDL_SetVideoMode(winX, winY, depth, videomode);
		}

		// Failed : try with anti-aliasing, but without alpha channel.
		if (!ScreenSurface)
		{
			SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
			SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 0);
			ScreenSurface = SDL_SetVideoMode(winX, winY, depth, videomode);
		}

		// Failed : try without anti-aliasing, and without alpha channel.
		if (!ScreenSurface)
		{
			SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
			ScreenSurface = SDL_SetVideoMode(winX, winY, depth, videomode);
		}

		// Failed : Give up but say why.
		if (!ScreenSurface)
		{
		  GfLogWarning("The minimum display requirements are not fulfilled :\n");
		  GfLogWarning("We need a double buffered RGB visual with a 16 bit depth buffer at least.\n");
		}
	}

	// Video initialization with generic compatible settings.
	if (!ScreenSurface)
	{
		GfLogInfo("Trying generic video initialization with requested size and color depth, fallback.\n\n");
		ScreenSurface = SDL_SetVideoMode(winX, winY, depth, videomode);
	}

	// Failed : Try with a lower fallback size  : should be supported everywhere ...
	if (!ScreenSurface)
	{
		GfLogError("Unable to get compatible video mode with requested size and color depth\n\n");
		winX = ADefScreenSizes[0].width;
		winY = ADefScreenSizes[0].height;
		GfLogInfo("Trying generic video initialization with fallback size %dx%d and requested color depth.\n\n",
				  winX, winY);
		ScreenSurface = SDL_SetVideoMode(winX, winY, depth, videomode);
	}

	// Failed : Try with a lower fallback color depth : should be supported everywhere ...
	if (!ScreenSurface)
	{
		GfLogError("Unable to get compatible video mode with fallback size and requested color depth\n\n");
		depth = ADefScreenColorDepths[0];
		GfLogInfo("Trying generic video initialization with fallback size %dx%d and color depth %d.\n\n",
				  winX, winY, depth);
		ScreenSurface = SDL_SetVideoMode(winX, winY, depth, videomode);
	}

	// Failed : No way ... no more ideas !
	if (!ScreenSurface)
	{
		GfLogError("Unable to get any compatible video mode"
				   " (fallback resolution not supported) : giving up !\n\n");
		return false;
	}

	GfParmReleaseHandle(handle);

	// Save view geometry and screen center.
    GfViewWidth = xw;
    GfViewHeight = yw;
    GfScrCenX = xw / 2;
    GfScrCenY = yw / 2;

	// Report video mode and OpenGL features finally obtained.
	GfLogInfo("Current video mode :\n");
 	GfLogInfo("  Full screen : %s\n", (videomode & SDL_FULLSCREEN) ? "Yes" : "No");
 	GfLogInfo("  Size        : %dx%d\n", winX, winY);
 	GfLogInfo("  Color depth : %d\n", depth);
	
	int glAlpha, glDepth, glDblBuff, glMSamp, glMSampLevel;
	SDL_GL_GetAttribute(SDL_GL_ALPHA_SIZE, &glAlpha);
	SDL_GL_GetAttribute(SDL_GL_DEPTH_SIZE, &glDepth);
	SDL_GL_GetAttribute(SDL_GL_DOUBLEBUFFER, &glDblBuff);
	SDL_GL_GetAttribute(SDL_GL_MULTISAMPLEBUFFERS, &glMSamp);
	SDL_GL_GetAttribute(SDL_GL_MULTISAMPLESAMPLES, &glMSampLevel);

	// TODO: Is anti-aliasing really related to video mode initialization ?
	//       What about moving this to gfglCheckGLFeatures and enable it later ?
	GfLogInfo("Enabled OpenGL features :\n");
 	GfLogInfo("  Double-buffer : %s", glDblBuff ? "Yes" : "No\n");
	if (glDblBuff)
		GfLogInfo(" (%d bits)\n", glDepth);
 	GfLogInfo("  Alpha-channel : %s\n", glAlpha ? "Yes" : "No");
 	GfLogInfo("  Anti-aliasing : %s", glMSamp ? "Yes" : "No\n");
	if (glMSamp)
		GfLogInfo("   (multi-sampling level %d)\n", glMSampLevel);

#ifdef WIN32
	// Under Windows, give an initial position to the window if not full-screen mode
	// (under Linux/Mac OS X, no need, the window manager smartly takes care of this).
	if (!(videomode & SDL_FULLSCREEN))
	{
		// Try to center the game Window on the desktop, but keep the title bar visible if any.
		const HWND hDesktop = GetDesktopWindow();
		RECT rectDesktop;
		GetWindowRect(hDesktop, &rectDesktop);
		const int nWMWinXPos = winX >= rectDesktop.right ? 0 : (rectDesktop.right - winX) / 2;
		const int nWMWinYPos = winY >= rectDesktop.bottom ? 0 : (rectDesktop.bottom - winY) / 2;
		GfuiInitWindowPositionAndSize(nWMWinXPos, nWMWinYPos, winX, winY);
	}
#endif

	// Initialize the Open GL viewport.
	gfScrReshapeViewport(winX, winY);

	// Setup the event loop about the new display.
	GfuiApp().eventLoop().setReshapeCB(gfScrReshapeViewport);
	GfuiApp().eventLoop().postRedisplay();

	// Initialize Open GL feature management layer
	//TODO:gfglCheckGLFeatures();

	return true;
}

/** Shutdown the screen
    @ingroup	screen
    @return	none
*/
void GfScrShutdown(void)
{
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
		GfLogDebug("Captured screen to %s (capture=%.3f s, PNG=%.3f s)\n",
				   filename, dCaptureDuration, dFileWriteDuration);
	else
		GfLogError("Failed to capture screen to %s\n", filename);

	return nStatus;
}

// Accessor to the SDL screen surface.
SDL_Surface* gfScrGetScreenSurface()
{
	return ScreenSurface;
}
