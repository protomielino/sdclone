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
#ifdef WIN32
#include <windows.h>
#ifndef HAVE_CONFIG_H
#define HAVE_CONFIG_H
#endif
#endif
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <cmath>
#ifndef WIN32
#include <unistd.h>
#else
#include <process.h>
#endif /* WIN32 */

#include <SDL/SDL.h>
#include <portability.h>

#include "tgfclient.h"
#include "gui.h"

#include "glfeatures.h"

#ifdef HAVE_CONFIG_H
#include "version.h"
#endif

static int GfScrWidth;
static int GfScrHeight;
static int GfViewWidth;
static int GfViewHeight;
static int GfScrCenX;
static int GfScrCenY;

SDL_Surface *ScreenSurface = NULL;
static void	*scrHandle = NULL;

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
	{ 1024,  768 },
	{ 1152,  768 },
	{ 1152,  864 },
	{ 1280,  720 },
	{ 1280,  768 },
	{ 1280,  800 },
	{ 1280,  960 },
	{ 1280, 1024 },
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
static const int NDefScreenSizes = sizeof(ADefScreenSizes) / sizeof(ADefScreenSizes[0]);

static tScreenSize* AScreenSizes = 0;
static int NScreenSizes = 0;

static const char* ADisplayModes[] = { "Full-screen", "Windowed" };
static const int NDisplayModes = sizeof(ADisplayModes) / sizeof(ADisplayModes[0]);

static const char* AVideoInitModes[] = { "Compatible", "Best possible" };
static const int NVideoInitModes = sizeof(AVideoInitModes) / sizeof(AVideoInitModes[0]);

static int* AColorDepths = 0;
static int NColorDepths = 0;

static int NCurScreenSize = 0;
static int NCurDisplayMode = 0;
static int NCurColorDepth = 0;
static int NCurVideoInitMode = 0;

static int	NCurMaxFreq = 75;

static int ScreenSizeLabelId;
static int ColorDepthLabelId;
static int DisplayModeLabelId;
static int VideoInitModeLabelId;
#ifdef WIN32
static int MaxFreqEditId;
#endif

static void	*paramHdle;


/** Get the possible screen / windows sizes (pixels) for the given color depth and display mode.
    @ingroup	screen
    @param	nColorDepth	Requested color depth (bits)
    @param	bFullScreen	Requested display mode : full-screeen mode if true, windowed otherwise.
    @param	pnSizes	Address of number of detected possible sizes (output) (-1 if any size is possible).
    @return	Array of detected possible sizes (allocated on the heap, must use free at the end), or 0 if no detected possible size, or -1 if any size is possible.
 */
tScreenSize* GfScrGetPossibleSizes(int nColorDepth, bool bFullScreen, int* pnSizes)
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
	
	// Get the possible sizes for this pixel format.
	SDL_Rect **asdlPossSizes = SDL_ListModes(&sdlPixelFormat, sdlDisplayMode);

	GfLogTrace("Available %u-bit %s video sizes :",
			   sdlPixelFormat.BitsPerPixel, bFullScreen ? "full-screen" : "windowed");

	tScreenSize* aPossSizes;
	if (asdlPossSizes == (SDL_Rect**)0)
	{
		GfLogInfo(" None.\n");
		aPossSizes = (tScreenSize*)0;
		*pnSizes = 0;
	}
	else if (asdlPossSizes == (SDL_Rect**)-1)
	{
		GfLogInfo(" Any.\n");
		aPossSizes = (tScreenSize*)-1;
		*pnSizes = -1;
	}
	else
	{
		// Count the possible sizes.
		*pnSizes = 0;
		while (asdlPossSizes[*pnSizes])
			(*pnSizes)++;

		// Copy them into the output array.
		aPossSizes = (tScreenSize*)malloc((*pnSizes)*sizeof(tScreenSize));
		for (int nSizeInd = 0; nSizeInd < *pnSizes; nSizeInd++)
		{
			aPossSizes[nSizeInd].width  = asdlPossSizes[*pnSizes - 1 - nSizeInd]->w;
			aPossSizes[nSizeInd].height = asdlPossSizes[*pnSizes - 1 - nSizeInd]->h;
			GfLogInfo(" %dx%d,", aPossSizes[nSizeInd].width, aPossSizes[nSizeInd].height);
		}
		GfLogInfo("\n");
	}
	
	return aPossSizes;
}

/** Get the possible color depths as supported bythe underlying hardware/driver.
    @ingroup	screen
    @param	pnDepths	Address of number of detected color depths (output)
    @return	Array of detected possible color depths (allocated on the heap, must use free at the end)
 */
int* GfScrGetPossibleColorDepths(int* pnDepths)
{
	// Determine the maximum supported color depth (default to 24 in any case).
	const SDL_VideoInfo* sdlVideoInfo = SDL_GetVideoInfo();
	int nMaxColorDepth = 24;
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
	int nPossSizes;
	tScreenSize* aPossSizes;
	int* aPossDepths = (int*)malloc(nMaxColorDepths*sizeof(int));
	*pnDepths = 0;
	for (int nDepthInd = 0; nDepthInd < nMaxColorDepths; nDepthInd++)
	{
		const int nCheckedColorDepth = nMinColorDepth + 8 * nDepthInd;

		// Windowed mode.
		aPossSizes = GfScrGetPossibleSizes(nCheckedColorDepth, false, &nPossSizes);
		const bool bWindowedOK = (aPossSizes != 0);
		if (aPossSizes && aPossSizes != (tScreenSize*)-1)
			free(aPossSizes);

		// Full-screen mode
		aPossSizes = GfScrGetPossibleSizes(nCheckedColorDepth, true, &nPossSizes);
		const bool bFullScreenOK = (aPossSizes != 0);
		if (aPossSizes && aPossSizes != (tScreenSize*)-1)
			free(aPossSizes);

		// Keep this color depth if one of the display modes work
		// TODO: Shouldn't we use "and" here ?
		if (bWindowedOK || bFullScreenOK)
		{
			aPossDepths[*pnDepths] = nCheckedColorDepth;
			(*pnDepths)++;
		}
	}

	// Fallback : assume at least 24 bit depth is supported.
	if (*pnDepths == 0)
	{
		GfLogWarning("SDL reports no supported color depth : assuming 24 bit is OK");
		aPossDepths[*pnDepths] = 24;
		(*pnDepths)++;
	}

	// Report supported depths.
	GfLogInfo("Supported color depths (bits) :");
	for (int nDepthInd = 0; nDepthInd < *pnDepths; nDepthInd++)
		GfLogInfo(" %d,", aPossDepths[nDepthInd]);
	GfLogInfo("\n");

	return aPossDepths;
}

static void gfScrReshape(int width, int height)
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

void GfScrInit(int argc, char *argv[])
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

	// Initialize SDL video subsystem (and exit if not possible).
	if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0)
		GfLogFatal("Couldn't initialize SDL video sub-system (%s)\n", SDL_GetError());

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
		GfLogWarning("Could not SDL_GetVideoInfo (%s)\n", SDL_GetError());
		return;
	}
	
	char pszDriverName[32];
	GfLogTrace("SDL video driver : '%s'\n", SDL_VideoDriverName(pszDriverName, 32));
	//GfLogTrace("  Hardware acceleration : %s\n", sdlVideoInfo->hw_available ? "true" : "false");
	//GfLogTrace("  Total video memory    : %u Kb\n", sdlVideoInfo->video_mem);
	GfLogTrace("Maximum pixel depth : %d bits\n", sdlVideoInfo->vfmt->BitsPerPixel);

	// Query supported color depths.
	if (AColorDepths)
		free(AColorDepths);
	AColorDepths = GfScrGetPossibleColorDepths(&NColorDepths);
	
	// Get graphical settings from config file.
    sprintf(buf, "%s%s", GetLocalDir(), GFSCR_CONF_FILE);
    handle = GfParmReadFile(buf, GFPARM_RMODE_STD | GFPARM_RMODE_CREAT);
    xw = (int)GfParmGetNum(handle, GFSCR_SECT_PROP, GFSCR_ATT_X, (char*)NULL, 640);
    yw = (int)GfParmGetNum(handle, GFSCR_SECT_PROP, GFSCR_ATT_Y, (char*)NULL, 480);
    winX = (int)GfParmGetNum(handle, GFSCR_SECT_PROP, GFSCR_ATT_WIN_X, (char*)NULL, xw);
    winY = (int)GfParmGetNum(handle, GFSCR_SECT_PROP, GFSCR_ATT_WIN_Y, (char*)NULL, yw);
    depth = (int)GfParmGetNum(handle, GFSCR_SECT_PROP, GFSCR_ATT_BPP, (char*)NULL, 32);
    maxfreq = (int)GfParmGetNum(handle, GFSCR_SECT_PROP, GFSCR_ATT_MAXREFRESH, (char*)NULL, 160);

    fscr = GfParmGetStr(handle, GFSCR_SECT_PROP, GFSCR_ATT_FSCR, GFSCR_VAL_NO);

    vinit = GfParmGetStr(handle, GFSCR_SECT_PROP, GFSCR_ATT_VINIT, GFSCR_VAL_VINIT_COMPATIBLE);

    // Initialize game interface to SDL.
    GfelInitialize();
    atexit(SDL_Quit);

	// Set window/icon captions
	sprintf(buf, "Speed Dreams %s", VERSION_LONG);
	SDL_WM_SetCaption(buf, buf);

	// Set window icon (MUST be a 32x32 icon for Windows, and with black pixels as alpha ones, 
	// as BMP doesn't support transparency).
	sprintf(buf, "%sdata/icons/icon.bmp", GetDataDir());
	if ((icon = SDL_LoadBMP(buf)))
	{
	    SDL_SetColorKey(icon, SDL_SRCCOLORKEY, SDL_MapRGB(icon->format, 0, 0, 0));
	    SDL_WM_SetIcon(icon, 0);
	    SDL_FreeSurface(icon);
	}

	// Set full screen mode if required.
	int videomode = SDL_OPENGL;
	if (strcmp(fscr,"yes")==0)
		videomode |= SDL_FULLSCREEN;
 
	// Video initialization with best possible settings.
	ScreenSurface = 0;
	if (strcmp(vinit, GFSCR_VAL_VINIT_BEST) == 0) 
	{
		// TODO: Check for better than default multi-sampling level
		//       through SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, level);
		// Could not get it automatically detected till now.

		/* Set the minimum requirements for the OpenGL window */
		SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 8 );
		SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 8 );
		SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 8 );

		// Try to get "best" videomode with default anti-aliasing support :
		// 24 bit z-buffer with alpha channel.
		SDL_GL_SetAttribute( SDL_GL_ALPHA_SIZE, 8 );
		SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 24 );
		SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
		SDL_GL_SetAttribute( SDL_GL_MULTISAMPLEBUFFERS, 1 );
		ScreenSurface = SDL_SetVideoMode( winX, winY, depth, videomode);

		// Failed : try without anti-aliasing.
		if (!ScreenSurface)
		{
			SDL_GL_SetAttribute( SDL_GL_MULTISAMPLEBUFFERS, 0 );
			ScreenSurface = SDL_SetVideoMode( winX, winY, depth, videomode);
		}

		// Failed : try with anti-aliasing, but without alpha channel.
		if (!ScreenSurface)
		{
			SDL_GL_SetAttribute( SDL_GL_MULTISAMPLEBUFFERS, 1 );
			SDL_GL_SetAttribute( SDL_GL_ALPHA_SIZE, 0 );
			ScreenSurface = SDL_SetVideoMode( winX, winY, depth, videomode);
		}

		// Failed : try without anti-aliasing, and without alpha channel.
		if (!ScreenSurface)
		{
			SDL_GL_SetAttribute( SDL_GL_MULTISAMPLEBUFFERS, 0 );
			ScreenSurface = SDL_SetVideoMode( winX, winY, depth, videomode);
		}

		// Failed : try 16 bit z-buffer and back with alpha channel and anti-aliasing.
		if (!ScreenSurface)
		{
			SDL_GL_SetAttribute( SDL_GL_MULTISAMPLEBUFFERS, 1 );
			SDL_GL_SetAttribute( SDL_GL_ALPHA_SIZE, 8 );
			SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 16 );
			ScreenSurface = SDL_SetVideoMode( winX, winY, depth, videomode);
		}

		// Failed : try without anti-aliasing.
		if (!ScreenSurface)
		{
			SDL_GL_SetAttribute( SDL_GL_MULTISAMPLEBUFFERS, 0 );
			ScreenSurface = SDL_SetVideoMode( winX, winY, depth, videomode);
		}

		// Failed : try with anti-aliasing, but without alpha channel.
		if (!ScreenSurface)
		{
			SDL_GL_SetAttribute( SDL_GL_MULTISAMPLEBUFFERS, 1 );
			SDL_GL_SetAttribute( SDL_GL_ALPHA_SIZE, 0 );
			ScreenSurface = SDL_SetVideoMode( winX, winY, depth, videomode);
		}

		// Failed : try without anti-aliasing, and without alpha channel.
		if (!ScreenSurface)
		{
			SDL_GL_SetAttribute( SDL_GL_MULTISAMPLEBUFFERS, 0 );
			ScreenSurface = SDL_SetVideoMode( winX, winY, depth, videomode);
		}

		// Failed : Give up but say why.
		if (!ScreenSurface)
		{
		  GfTrace("The minimum display requirements are not fulfilled.\n");
		  GfTrace("We need a double buffered RGB visual with a 16 bit depth buffer at least.\n");
		}
	}

	// Video initialization with generic compatible settings.
	if (!ScreenSurface)
	{
		GfTrace("Trying generic video initialization with requested resolution, fallback.\n\n");
		ScreenSurface = SDL_SetVideoMode( winX, winY, depth, videomode);
	}

	// Failed : Try with a lower fallback resolution that should be supported everywhere ...
	if (!ScreenSurface)
	{
		GfError("Unable to get compatible video mode with requested resolution\n\n");
		winX = ADefScreenSizes[0].width;
		winY = ADefScreenSizes[0].height;
		GfTrace("Trying generic video initialization with fallback resolution %dx%d.\n\n",
				winX, winY);
		ScreenSurface = SDL_SetVideoMode( winX, winY, depth, videomode);
	}

	// Failed : No way ... no more ideas !
	if (!ScreenSurface)
	{
		GfFatal("Unable to get any compatible video mode (fallback resolution not supported)\n\n");
		exit(1);
	}

	// Save view geometry and screen center.
    GfViewWidth = xw;
    GfViewHeight = yw;
    GfScrCenX = xw / 2;
    GfScrCenY = yw / 2;

	// Report video mode finally obtained.
	int glAlpha, glDepth, glDblBuff, glMSamp, glMSampLevel;
	SDL_GL_GetAttribute( SDL_GL_ALPHA_SIZE, &glAlpha );
	SDL_GL_GetAttribute( SDL_GL_DEPTH_SIZE, &glDepth );
	SDL_GL_GetAttribute( SDL_GL_DOUBLEBUFFER, &glDblBuff );
	SDL_GL_GetAttribute( SDL_GL_MULTISAMPLEBUFFERS, &glMSamp );
	SDL_GL_GetAttribute( SDL_GL_MULTISAMPLESAMPLES, &glMSampLevel );

	GfTrace("Visual Properties Report\n");
	GfTrace("------------------------\n");
 	GfTrace("Resolution    : %dx%dx%d\n", winX, winY, depth);
 	GfTrace("Double-buffer : %s", glDblBuff ? "Yes" : "No\n");
	if (glDblBuff)
		GfTrace(" (%d bits)\n", glDepth);
 	GfTrace("Alpha-channel : %s\n", glAlpha ? "Yes" : "No");
 	GfTrace("Anti-aliasing : %s", glMSamp ? "Yes" : "No\n");
	if (glMSamp)
		GfTrace(" (multi-sampling level %d)\n", glMSampLevel);

	/* Give an initial size and position*/
	GfelPostRedisplay();
	GfuiInitWindowPosition(0, 0);
	GfuiInitWindowSize(winX, winY);
	gfScrReshape(winX,winY);
	GfelSetReshapeCB(gfScrReshape);

	GfParmReleaseHandle(handle);

	gfglCheckGLFeatures();
}

/** Shutdown the screen
    @ingroup	screen
    @return	none
*/
void GfScrShutdown(void)
{
	if (AColorDepths)
	{
		free(AColorDepths);
		AColorDepths = 0;
	}
	
	if (AScreenSizes && AScreenSizes != ADefScreenSizes)
	{
		free(AScreenSizes);
		AScreenSizes = 0;
	}
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

// Save graphical settings to XML file.
static void
saveParams(void)
{
	const int w = AScreenSizes[NCurScreenSize].width;
	const int h = AScreenSizes[NCurScreenSize].height;
	
	GfParmSetNum(paramHdle, GFSCR_SECT_PROP, GFSCR_ATT_X, (char*)NULL, w);
	GfParmSetNum(paramHdle, GFSCR_SECT_PROP, GFSCR_ATT_Y, (char*)NULL, h);
	GfParmSetNum(paramHdle, GFSCR_SECT_PROP, GFSCR_ATT_WIN_X, (char*)NULL, w);
	GfParmSetNum(paramHdle, GFSCR_SECT_PROP, GFSCR_ATT_WIN_Y, (char*)NULL, h);
	GfParmSetNum(paramHdle, GFSCR_SECT_PROP, GFSCR_ATT_BPP, (char*)NULL, AColorDepths[NCurColorDepth]);
	GfParmSetNum(paramHdle, GFSCR_SECT_PROP, GFSCR_ATT_MAXREFRESH, (char*)NULL, NCurMaxFreq);

	const char* pszVInitMode =
		(NCurVideoInitMode == 0) ? GFSCR_VAL_VINIT_COMPATIBLE : GFSCR_VAL_VINIT_BEST;
	GfParmSetStr(paramHdle, GFSCR_SECT_PROP, GFSCR_ATT_VINIT, pszVInitMode);

	GfParmSetStr(paramHdle, GFSCR_SECT_PROP, GFSCR_ATT_FSCR, NCurDisplayMode == 0 ? "yes" : "no");
	
	GfParmWriteFile(NULL, paramHdle, "Screen");
}

// Re-init screen to take new graphical settings into account (implies process restart).
void
GfScrReinit(void * /* dummy */)
{
    // Force current edit to loose focus (if one has it) and update associated variable.
    GfuiUnSelectCurrent();

    // Save graphical settings.
    saveParams();

    // Release screen allocated resources.
    GfScrShutdown();

    // Restart the game.
	GfRestart(GfuiMouseHW != 0, GfglIsMultiTexturingEnabled());
}

static void
updateLabelText(void)
{
	char buf[32];
	
	sprintf(buf, "%dx%d", AScreenSizes[NCurScreenSize].width, AScreenSizes[NCurScreenSize].height);
    GfuiLabelSetText(scrHandle, ScreenSizeLabelId, buf);
	
	sprintf(buf, "%d", AColorDepths[NCurColorDepth]);
    GfuiLabelSetText(scrHandle, ColorDepthLabelId, buf);
	
    GfuiLabelSetText(scrHandle, DisplayModeLabelId, ADisplayModes[NCurDisplayMode]);
	
#ifdef WIN32
    sprintf(buf, "%d", NCurMaxFreq);
    GfuiEditboxSetString(scrHandle, MaxFreqEditId, buf);
#endif
	
    GfuiLabelSetText(scrHandle, VideoInitModeLabelId, AVideoInitModes[NCurVideoInitMode]);
}

static void
ResPrevNext(void *vdelta)
{
    NCurScreenSize = (NCurScreenSize + (int)(long)vdelta + NScreenSizes) % NScreenSizes;

    updateLabelText();
}

static void
updateScreenSizes(int nCurrWidth, int nCurrHeight)
{
	// Query possible screen sizes for the current display mode and color depth.
	if (AScreenSizes && AScreenSizes != ADefScreenSizes)
		free(AScreenSizes);
	AScreenSizes = GfScrGetPossibleSizes(AColorDepths[NCurColorDepth],
										 NCurDisplayMode == 0, &NScreenSizes);

	// If any size is possible :-) or none :-(, use default hard coded list (temporary).
	if (AScreenSizes == (tScreenSize*)-1 || AScreenSizes == 0)
	{
		AScreenSizes = ADefScreenSizes;
		NScreenSizes = NDefScreenSizes;
	}

	// Try and find the closest screen size to the current choice in the new list.
	// 1) Is there an exact match ?
	NCurScreenSize = -1;
	for (int nSizeInd = 0; nSizeInd < NScreenSizes; nSizeInd++)
	{
		if (nCurrWidth == AScreenSizes[nSizeInd].width
			&& nCurrHeight == AScreenSizes[nSizeInd].height)
		{
			NCurScreenSize = nSizeInd;
			break;
		}
	}

	// 2) Is there an approximative match ?
	if (NCurScreenSize < 0)
	{
		for (int nSizeInd = 0; nSizeInd < NScreenSizes; nSizeInd++)
		{
			if (nCurrWidth <= AScreenSizes[nSizeInd].width
				&& nCurrHeight <= AScreenSizes[nSizeInd].height)
			{
				NCurScreenSize = nSizeInd;
				break;
			}
		}
	}

	// 3) Not found : the closest is the biggest.
	if (NCurScreenSize < 0)
		NCurScreenSize = NScreenSizes - 1;
}

static void
DepthPrevNext(void *vdelta)
{
    NCurColorDepth = (NCurColorDepth + (int)(long)vdelta + NColorDepths) % NColorDepths;

	updateScreenSizes(AScreenSizes[NCurScreenSize].width, AScreenSizes[NCurScreenSize].height);
	
    updateLabelText();
}

static void
ModePrevNext(void *vdelta)
{
    NCurDisplayMode = (NCurDisplayMode + (int)(long)vdelta + NDisplayModes) % NDisplayModes;

	updateScreenSizes(AScreenSizes[NCurScreenSize].width, AScreenSizes[NCurScreenSize].height);
	
    updateLabelText();
}


static void
VInitPrevNext(void *vdelta)
{
    NCurVideoInitMode = (NCurVideoInitMode + (int)(long)vdelta + NVideoInitModes) % NVideoInitModes;

	updateLabelText();
}


static void
loadParams(void)
{
	int w, h, bpp;
	int i;

	// Color depth (bits per pixel).
	bpp = (int)GfParmGetNum(paramHdle, GFSCR_SECT_PROP, GFSCR_ATT_BPP, NULL, AColorDepths[NColorDepths-1]);
	NCurColorDepth = NColorDepths-1; // Defaults to max possible supported value.
	for (i = 0; i < NColorDepths; i++) {
		if (bpp <= AColorDepths[i]) {
			NCurColorDepth = i;
			break;
		}
	}

	// Display mode : Full-screen or Windowed.
	if (!strcmp("yes", GfParmGetStr(paramHdle, GFSCR_SECT_PROP, GFSCR_ATT_FSCR, "no"))) {
		NCurDisplayMode = 0;
	} else {
		NCurDisplayMode = 1;
	}

	// Screen / window size.
	w = (int)GfParmGetNum(paramHdle, GFSCR_SECT_PROP, GFSCR_ATT_X, NULL, 640);
	h = (int)GfParmGetNum(paramHdle, GFSCR_SECT_PROP, GFSCR_ATT_Y, NULL, 480);
	
	updateScreenSizes(w, h);

	// Video initialization mode : Compatible or Best.
	NCurVideoInitMode = 0;
	const char *tmp = GfParmGetStr(paramHdle, GFSCR_SECT_PROP, GFSCR_ATT_VINIT, GFSCR_VAL_VINIT_COMPATIBLE);
	if (strcmp(GFSCR_VAL_VINIT_COMPATIBLE, tmp) == 0) {
		NCurVideoInitMode = 0;
	} else {
		NCurVideoInitMode = 1;
	}

	// Max. refresh rate (Hz).
	NCurMaxFreq = (int)GfParmGetNum(paramHdle, GFSCR_SECT_PROP, GFSCR_ATT_MAXREFRESH, NULL, NCurMaxFreq);
}

#ifdef WIN32
static void
ChangeMaxFreq(void * /* dummy */)
{
	char buf[32];
    char	*val;
    
    val = GfuiEditboxGetString(scrHandle, MaxFreqEditId);
    NCurMaxFreq = (int)strtol(val, (char **)NULL, 0);
    sprintf(buf, "%d", NCurMaxFreq);
    GfuiEditboxSetString(scrHandle, MaxFreqEditId, buf);
}
#endif

static void
onActivate(void * /* dummy */)
{
    loadParams();
    updateLabelText();
}


/** Create and activate the video options menu screen.
    @ingroup	screen
    @param	precMenu	previous menu to return to
*/
void *
GfScrMenuInit(void *prevMenu)
{
	char buf[512];

	sprintf(buf, "%s%s", GetLocalDir(), GFSCR_CONF_FILE);
	paramHdle = GfParmReadFile(buf, GFPARM_RMODE_STD | GFPARM_RMODE_CREAT);
	
	if (scrHandle)
		return scrHandle;

	scrHandle = GfuiScreenCreateEx((float*)NULL, NULL, onActivate, NULL, (tfuiCallback)NULL, 1);
	void *param = LoadMenuXML("screenconfigmenu.xml");
	CreateStaticControls(param,scrHandle);

	CreateButtonControl(scrHandle,param,"resleftarrow",(void*)-1,ResPrevNext);
	CreateButtonControl(scrHandle,param,"resrightarrow",(void*)1,ResPrevNext);
	ScreenSizeLabelId = CreateLabelControl(scrHandle,param,"reslabel");

	CreateButtonControl(scrHandle, param, "accept", NULL, GfScrReinit);
	CreateButtonControl(scrHandle, param, "cancel", prevMenu, GfuiScreenActivate);

	CreateButtonControl(scrHandle,param,"depthleftarrow",(void*)-1,DepthPrevNext);
	CreateButtonControl(scrHandle,param,"depthrightarrow",(void*)1,DepthPrevNext);
	ColorDepthLabelId = CreateLabelControl(scrHandle,param,"depthlabel");

	CreateButtonControl(scrHandle,param,"displeftarrow",(void*)-1,ModePrevNext);
	CreateButtonControl(scrHandle,param,"disprightarrow",(void*)1,ModePrevNext);
	DisplayModeLabelId = CreateLabelControl(scrHandle,param,"displabel");

#ifdef WIN32
	CreateLabelControl(scrHandle,param,"maxfreqlabel");
	MaxFreqEditId = CreateEditControl(scrHandle,param,"freqedit",NULL,NULL,ChangeMaxFreq);
#endif

	CreateButtonControl(scrHandle,param,"vmleftarrow",(void*)-1, VInitPrevNext);
	CreateButtonControl(scrHandle,param,"vmrightarrow",(void*)1, VInitPrevNext);
	VideoInitModeLabelId = CreateLabelControl(scrHandle,param,"vmlabel");
	GfParmReleaseHandle(param);

	GfuiAddKey(scrHandle, GFUIK_RETURN, "Accept", NULL, GfScrReinit, NULL);
	GfuiAddKey(scrHandle, GFUIK_ESCAPE, "Cancel", prevMenu, GfuiScreenActivate, NULL);
	GfuiAddKey(scrHandle, GFUIK_LEFT, "Previous Resolution", (void*)-1, ResPrevNext, NULL);
	GfuiAddKey(scrHandle, GFUIK_RIGHT, "Next Resolution", (void*)1, ResPrevNext, NULL);
	GfuiAddKey(scrHandle, GFUIK_F1, "Help", scrHandle, GfuiHelpScreen, NULL);
	GfuiAddKey(scrHandle, GFUIK_F12, "Screen-Shot", NULL, GfuiScreenShot, NULL);
    

	return scrHandle;
}


