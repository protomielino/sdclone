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

#include "network.h"
#include <stdio.h>
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

#include <math.h>
#ifndef WIN32
#include <unistd.h>
#else
#include <process.h>
#endif /* WIN32 */

#include <SDL/SDL.h>
#include "tgfclient.h"
#include <portability.h>
#include "gui.h"

#include "glfeatures.h"

#ifdef HAVE_CONFIG_H
#include "version.h"
#endif

#if defined(WIN32) || defined(__APPLE__)
#undef USE_RANDR_EXT
#endif // WIN32

#ifdef USE_RANDR_EXT
#include <GL/glx.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>
#include <X11/extensions/Xrandr.h>

//Set HAS_RANDR_1_2 if we have xrandr version 1.2 or higher
#if RANDR_MAJOR > 1 || (RANDR_MAJOR == 1 && RANDR_MINOR >= 2)
#define HAS_RANDR_1_2 1
#endif
#endif // USE_RANDR_EXT

static int GfScrWidth;
static int GfScrHeight;
static int GfViewWidth;
static int GfViewHeight;
static int GfScrCenX;
static int GfScrCenY;

SDL_Surface *screenSurface = NULL;
static void	*scrHandle = NULL;
static char 	buf[1024];

/* Default list of resolutions in case no RANDR_EXT (Windows)
   or something went wrong during resolution detection */
static const char *DefRes[] =
  { "320x200",
	"320x240",
	"400x300",
	"416x312",
	"480x360",
	"512x384",
	"576x384",
	"576x432",
	"640x384",
	"640x400",
	"640x480",
	"640x512",
	"700x525",
	"720x450",
	"800x512",
	"800x512",
	"800x600",
	"832x624",
	"840x525",
	"896x672",
	"928x696",
	"960x600",
	"960x720",
	"960x1200",
	"1024x768",
	"1152x768",
	"1152x864",
	"1280x720",
	"1280x768",
	"1280x800",
	"1280x960",
	"1280x1024",
	"1400x1050",
	"1440x900",
	"1600x900",
	"1600x1024",
	"1680x1050",
	"1792x1344",
	"1800x1440",
	"1920x1200" };
static const int NbDefRes = sizeof(DefRes) / sizeof(DefRes[0]);

#ifdef USE_RANDR_EXT
static char	**Res = NULL;
static int nbRes = 0;
#else // USE_RANDR_EXT
static const char	**Res = DefRes;
static int nbRes = NbDefRes;
#endif // USE_RANDR_EXT

static const char	*Mode[] = {"Full-screen mode", "Window mode"};
static const char	*VInit[] = {GFSCR_VAL_VINIT_COMPATIBLE, GFSCR_VAL_VINIT_BEST};
static const char	*Depthlist[] = {"24", "32", "16"};

static const int nbMode = sizeof(Mode) / sizeof(Mode[0]);
static const int nbVInit = sizeof(VInit) / sizeof(VInit[0]);
static const int nbDepth = sizeof(Depthlist) / sizeof(Depthlist[0]);

static int	curRes = 0;
static int	curMode = 0;
static int	curDepth = 0;
static int curVInit = 0;

static int	curMaxFreq = 75;
#ifdef WIN32
static int	MaxFreqId;
#endif

static int	ResLabelId;
static int	DepthLabelId;
static int	ModeLabelId;
static int VInitLabelId;

static void	*paramHdle;


void
gfScreenInit(void)
{
#ifdef USE_RANDR_EXT
	// Get display, screen and root window handles.
	const char *displayname = getenv("DISPLAY");
	if (displayname == 0) {
		displayname = ":0.0";
	}

	Display *display = XOpenDisplay(displayname);

	if( display) {
		// If we have a display fill in the resolutions advertised by Xrandr.
		int screen = DefaultScreen(display);
    	Window root = RootWindow(display, screen);

	//Default: we cannot find the screenconfig
	XRRScreenConfiguration *screenconfig = NULL;
#if HAS_RANDR_1_2
	//If we have randr 1.2, we can query xrandr is actually enabled. On older versions, we will just assume that
	if (XRRQueryVersion (display, NULL, NULL))
	{
#endif
		//Load the screenconfig
		screenconfig = XRRGetScreenInfo (display, root);
#if HAS_RANDR_1_2
	}
#endif

		if (screenconfig) {
			int i, j, nsize;
			XRRScreenSize *sizes = XRRConfigSizes(screenconfig, &nsize);

			GfOut("X Supported resolutions :");
			if (nsize > 0) {
				// Force 320x200, 640x480, 800x600 to be available to the user,
				// even if X did not report about.
				int check_resx[] = {320, 640, 800};
				int check_resy[] = {240, 480, 600};
				bool mode_in_list[] = {false, false, false};
				int add_modes = sizeof(check_resx)/sizeof(check_resx[0]);

				for (i = 0; i < nsize; i++) {
					GfOut(" %dx%d", sizes[i].width, sizes[i].height);
					for (j = 0; j < 3; j++) {
						if (!mode_in_list[j] && sizes[i].width == check_resx[j]) {
							if (sizes[i].height == check_resy[j]) {
								// Mode already in list.
								mode_in_list[j] = true;
								add_modes--;
							}
						}
					}
				}
				GfOut("\n");

				// Build the mode list, adding "forced" resolutions at the end if necessary.
				const int bufsize = 20;
				char buffer[bufsize];
				Res = (char**) malloc(sizeof(char *)*(nsize+add_modes));
				int resx[nsize+add_modes];
				int resy[nsize+add_modes];
				GfOut("Available resolutions :\n");
				for (i = 0; i < nsize+add_modes; i++) {
					if (i < nsize) {
						// Add mode from screenconfig (system).
						snprintf(buffer, bufsize, "%dx%d", sizes[i].width, sizes[i].height);
						Res[i] = strndup(buffer, bufsize);
						resx[i] = sizes[i].width;
						resy[i] = sizes[i].height;
						GfOut("  %dx%d  \t(detected)\n", resx[i], resy[i]);
					} else {
						// Add mode from wish list.
						unsigned int j;
						for (j = 0; j < sizeof(check_resx)/sizeof(check_resx[0]); j++) {
							if (mode_in_list[j] == false) {
								mode_in_list[j] = true;
								snprintf(buffer, bufsize, "%dx%d", check_resx[j], check_resy[j]);
								Res[i] = strndup(buffer, bufsize);
								resx[i] = check_resx[j];
								resy[i] = check_resy[j];
								GfOut("  %dx%d  \t(forced)\n", resx[i], resy[i]);
								break;
							}
						}
					}

					// Stupid sorting (not much elements, don't worry).
					int j;
					for (j = i; j > 0; j--) {
						if (resx[j] < resx[j-1]
							|| (resx[j] == resx[j-1] && resy[j] < resy[j-1]))
						{
							int tx, ty;
							char *tc;
							tx = resx[j-1];
							ty = resy[j-1];
							resx[j-1] = resx[j];
							resy[j-1] = resy[j];
							resx[j] = tx;
							resy[j] = ty;
							tc = Res[j-1];
							Res[j-1] = Res[j];
							Res[j] = tc;
						} else {
							break;
						}
					}
				}

				nbRes = nsize + add_modes;

			} else {
				GfOut(" None !");
			}
			GfOut("\n");

			XRRFreeScreenConfigInfo(screenconfig);
		}
		XCloseDisplay(display);
	}

	if (Res == NULL || nbRes == 0) {
		// We failed to get a handle to the display, so fill in some defaults.
		GfOut("Failed to initialize resolutions for display '%s' ; using defaults", 
			  XDisplayName(displayname));
		nbRes = NbDefRes;
		Res = (char **) malloc(sizeof(char *)*nbRes);
		int i;
		for (i = 0; i < nbRes; i++) {
			Res[i] = strdup(DefRes[i]);
		}
	}
#endif // USE_RANDR_EXT
}

static void Reshape(int width, int height)
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
    int		xw, yw;
    int		winX, winY;
    void	*handle;
    const char	*fscr;
    const char	*vinit;
    SDL_Surface *icon;

    int		maxfreq;
    int		depth;

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

    // Initialize SDL.
    if ( SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0 ) {
        printf("Couldn't initialize SDL: %s\n", SDL_GetError());
        return;
    }

	NetworkInit();

    // Initialize game interface to SDL.
    sdlInitCallbacks();
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
	screenSurface = 0;
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
		screenSurface = SDL_SetVideoMode( winX, winY, depth, videomode);

		// Failed : try without anti-aliasing.
		if (!screenSurface)
		{
			SDL_GL_SetAttribute( SDL_GL_MULTISAMPLEBUFFERS, 0 );
			screenSurface = SDL_SetVideoMode( winX, winY, depth, videomode);
		}

		// Failed : try with anti-aliasing, but without alpha channel.
		if (!screenSurface)
		{
			SDL_GL_SetAttribute( SDL_GL_MULTISAMPLEBUFFERS, 1 );
			SDL_GL_SetAttribute( SDL_GL_ALPHA_SIZE, 0 );
			screenSurface = SDL_SetVideoMode( winX, winY, depth, videomode);
		}

		// Failed : try without anti-aliasing, and without alpha channel.
		if (!screenSurface)
		{
			SDL_GL_SetAttribute( SDL_GL_MULTISAMPLEBUFFERS, 0 );
			screenSurface = SDL_SetVideoMode( winX, winY, depth, videomode);
		}

		// Failed : try 16 bit z-buffer and back with alpha channel and anti-aliasing.
		if (!screenSurface)
		{
			SDL_GL_SetAttribute( SDL_GL_MULTISAMPLEBUFFERS, 1 );
			SDL_GL_SetAttribute( SDL_GL_ALPHA_SIZE, 8 );
			SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 16 );
			screenSurface = SDL_SetVideoMode( winX, winY, depth, videomode);
		}

		// Failed : try without anti-aliasing.
		if (!screenSurface)
		{
			SDL_GL_SetAttribute( SDL_GL_MULTISAMPLEBUFFERS, 0 );
			screenSurface = SDL_SetVideoMode( winX, winY, depth, videomode);
		}

		// Failed : try with anti-aliasing, but without alpha channel.
		if (!screenSurface)
		{
			SDL_GL_SetAttribute( SDL_GL_MULTISAMPLEBUFFERS, 1 );
			SDL_GL_SetAttribute( SDL_GL_ALPHA_SIZE, 0 );
			screenSurface = SDL_SetVideoMode( winX, winY, depth, videomode);
		}

		// Failed : try without anti-aliasing, and without alpha channel.
		if (!screenSurface)
		{
			SDL_GL_SetAttribute( SDL_GL_MULTISAMPLEBUFFERS, 0 );
			screenSurface = SDL_SetVideoMode( winX, winY, depth, videomode);
		}

		// Failed : Give up but say why.
		if (!screenSurface)
		{
		  GfTrace("The minimum display requirements are not fulfilled.\n");
		  GfTrace("We need a double buffered RGB visual with a 16 bit depth buffer at least.\n");
		}
	}

	// Video initialization with generic compatible settings.
	if (!screenSurface)
	{
		GfTrace("Trying generic video initialization with requested resolution, fallback.\n\n");
		screenSurface = SDL_SetVideoMode( winX, winY, depth, videomode);
	}

	// Failed : Try with a lower fallback resolution that should be supported everywhere ...
	if (!screenSurface)
	{
	  GfError("Unable to get compatible video mode with requested resolution\n\n");
		sscanf(DefRes[0], "%dx%d", &winX, &winY);
		GfTrace("Trying generic video initialization with fallback resolution %dx%d.\n\n",
				winX, winY);
		screenSurface = SDL_SetVideoMode( winX, winY, depth, videomode);
	}

	// Failed : No way ... no more ideas !
	if (!screenSurface)
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
	sdlPostRedisplay();
	sdlInitWindowPosition(0, 0);
	sdlInitWindowSize(winX, winY);
	Reshape(winX,winY);
	sdlReshapeFunc( Reshape );

	GfParmReleaseHandle(handle);

	checkGLFeatures();
}

/** Shutdown the screen
    @ingroup	screen
    @return	none
*/
void GfScrShutdown(void)
{
#ifdef USE_RANDR_EXT
	int i;
	for (i = 0; i < nbRes; i++) {
		free(Res[i]);
	}
	free(Res);
#endif // USE_RANDR_EXT
}


/** Get the screen and viewport sizes.
    @ingroup	screen
    @param	scrw	address of screen with
    @param	scrh	address of screen height
    @param	vieww	address of viewport with
    @param	viewh	address of viewport height
    @return	none
 */
void GfScrGetSize(int *scrw, int *scrh, int *vieww, int *viewh)
{
    *scrw = GfScrWidth;
    *scrh = GfScrHeight;
    *vieww = GfViewWidth;
    *viewh = GfViewHeight;
}

// Save graphical settings to XML file.
static void
saveParams(void)
{
	int x, y, bpp;

	sscanf(Res[curRes], "%dx%d", &x, &y);
	sscanf(Depthlist[curDepth], "%d", &bpp);

	GfParmSetNum(paramHdle, GFSCR_SECT_PROP, GFSCR_ATT_X, (char*)NULL, x);
	GfParmSetNum(paramHdle, GFSCR_SECT_PROP, GFSCR_ATT_Y, (char*)NULL, y);
	GfParmSetNum(paramHdle, GFSCR_SECT_PROP, GFSCR_ATT_WIN_X, (char*)NULL, x);
	GfParmSetNum(paramHdle, GFSCR_SECT_PROP, GFSCR_ATT_WIN_Y, (char*)NULL, y);
	GfParmSetNum(paramHdle, GFSCR_SECT_PROP, GFSCR_ATT_BPP, (char*)NULL, bpp);
	GfParmSetNum(paramHdle, GFSCR_SECT_PROP, GFSCR_ATT_MAXREFRESH, (char*)NULL, curMaxFreq);

	GfParmSetStr(paramHdle, GFSCR_SECT_PROP, GFSCR_ATT_VINIT, (char*)VInit[curVInit]);

	if (curMode == 0) {
		GfParmSetStr(paramHdle, GFSCR_SECT_PROP, GFSCR_ATT_FSCR, "yes");
	} else {
		GfParmSetStr(paramHdle, GFSCR_SECT_PROP, GFSCR_ATT_FSCR, "no");
	}
	GfParmWriteFile(NULL, paramHdle, "Screen");
}

// In-place convert internal file or dir path to an OS compatible path
void makeOSPath(char* path)
{
#ifdef WIN32
  size_t i;
  for (i = 0; i < strlen(path); i++)
	if (path[i] == '/')
	  path[i] = '\\';
#endif //WIN32 
}


// Build a new path string compatible with current OS and usable as a command line arg.
char* buildPathArg(const char *path)
{
#ifdef WIN32
  char *osPath = (char*)malloc(strlen(path)+3);
  sprintf(osPath, "\"%s", path);
  if (osPath[strlen(osPath)-1] == '/')
    osPath[strlen(osPath)-1] = 0; // Remove trailing '/' for command line
  strcat(osPath, "\"");
#else
  char *osPath = strdup(path);
#endif //WIN32 
  makeOSPath(osPath);
  return osPath;
}

// Re-init screen to take new graphical settings into account (implies process restart).
void
GfScrReinit(void * /* dummy */)
{
    int retcode = 0;
    static const int CMDSIZE = 1024;
    char cmd[CMDSIZE];

    char** args;
    int	i, nArgs;
    int	argInd;

    // Force current edit to loose focus (if one has it) and update associated variable.
    GfuiUnSelectCurrent();

    // Save graphical settings.
    saveParams();

    // Release screen allocated resources.
    GfScrShutdown();

    // Command name.
    sprintf(cmd, "%sspeed-dreams", GetBinDir());
#ifdef WIN32
    strcat(cmd, ".exe");
#endif
    makeOSPath(cmd);

    // Compute number of args.
    nArgs = 1; // Executable is always the first arg.
    
    if (GfuiMouseHW)
	  nArgs += 1;
    if (GetLocalDir() && strlen(GetLocalDir()))
	  nArgs += 2;
    if (GetBinDir() && strlen(GetBinDir()))
	  nArgs += 2;
    if (GetLibDir() && strlen(GetLibDir()))
	  nArgs += 2;
    if (GetDataDir() && strlen(GetDataDir()))
	  nArgs += 2;

    nArgs++; // Last arg must be a null pointer.

    // Allocate args array.
    args = (char**)malloc(sizeof(char*)*nArgs);
	
    // First arg is the executable path-name.
    argInd = 0;
    args[argInd++] = buildPathArg(cmd);

    // Then add subsequent args.
    if (GfuiMouseHW)
        args[argInd++] = strdup("-m");
    
    if (GetLocalDir() && strlen(GetLocalDir()))
    {
        args[argInd++] = strdup("-l");
	args[argInd++] = buildPathArg(GetLocalDir());
    }

    if (GetBinDir() && strlen(GetBinDir()))
    {
        args[argInd++] = strdup("-B");
	args[argInd++] = buildPathArg(GetBinDir());
    }
	
    if (GetLibDir() && strlen(GetLibDir()))
    {
        args[argInd++] = strdup("-L");
	args[argInd++] = buildPathArg(GetLibDir());
    }
	
    if (GetDataDir() && strlen(GetDataDir()))
    {
        args[argInd++] = strdup("-D");
	args[argInd++] = buildPathArg(GetDataDir ());
    }
	
    // Finally, last null arg.
    args[argInd++] = 0;
	  
    // Exec the command : restart the game (simply replacing current process)
    GfOut("Restarting ");
    for (i = 0; i < nArgs && args[i]; i++)
        printf("%s%s ", args[i], nArgs < nArgs-2 ? " " : "");
    GfOut(" ...\n\n");
    retcode = execvp (cmd, args);

    // If successfull, we never get here ... but if failed ...
    GfOut("Failed to restart Speed Dreams through command line '");
    for (i = 0; i < nArgs && args[i]; i++)
    {
        GfOut("%s%s ", args[i], nArgs < nArgs-2 ? " " : "");
	free(args[i]);
    }
    GfOut("' (exit code %d)\n", retcode);
    free(args);
    
    perror("Speed Dreams");
    exit(1);
}

static void
updateLabelText(void)
{
    GfuiLabelSetText (scrHandle, ResLabelId, Res[curRes]);
    GfuiLabelSetText (scrHandle, DepthLabelId, Depthlist[curDepth]);
    GfuiLabelSetText (scrHandle, ModeLabelId, Mode[curMode]);
#ifdef WIN32
    sprintf(buf, "%d", curMaxFreq);
    GfuiEditboxSetString(scrHandle, MaxFreqId, buf);
#endif
    GfuiLabelSetText(scrHandle, VInitLabelId, VInit[curVInit]);
}

static void
ResPrevNext(void *vdelta)
{
    long delta = (long)vdelta;
    curRes += (int)delta;
    if (curRes < 0) {
	curRes = nbRes - 1;
    } else {
	if (curRes >= nbRes) {
	    curRes = 0;
	}
    }
    updateLabelText();
}

static void
DepthPrevNext(void *vdelta)
{
    long delta = (long)vdelta;

    curDepth += (int)delta;
    if (curDepth < 0) {
	curDepth = nbDepth - 1;
    } else {
	if (curDepth >= nbDepth) {
	    curDepth = 0;
	}
    }
    updateLabelText();
}

static void
ModePrevNext(void *vdelta)
{
    long delta = (long)vdelta;

    curMode += (int)delta;
    if (curMode < 0) {
	curMode = nbMode - 1;
    } else {
	if (curMode >= nbMode) {
	    curMode = 0;
	}
    }
    updateLabelText();
}


static void
VInitPrevNext(void *vdelta)
{
	long delta = (long)vdelta;

	curVInit += (int)delta;
	if (curVInit < 0) {
		curVInit = nbVInit - 1;
	} else {
		if (curVInit >= nbVInit) {
			curVInit = 0;
		}
	}
	updateLabelText();
}


static void
initFromConf(void)
{
	int x, y, bpp;
	int i;

	x = (int)GfParmGetNum(paramHdle, GFSCR_SECT_PROP, GFSCR_ATT_X, NULL, 640);
	y = (int)GfParmGetNum(paramHdle, GFSCR_SECT_PROP, GFSCR_ATT_Y, NULL, 480);

	sprintf(buf, "%dx%d", x, y);
	for (i = 0; i < nbRes; i++) {
		if (!strcmp(buf, Res[i])) {
			curRes = i;
			break;
		}
	}

	if (!strcmp("yes", GfParmGetStr(paramHdle, GFSCR_SECT_PROP, GFSCR_ATT_FSCR, "yes"))) {
		curMode = 0;
	} else {
		curMode = 1;
	}

	curVInit = 0;
	const char *tmp = GfParmGetStr(paramHdle, GFSCR_SECT_PROP, GFSCR_ATT_VINIT, GFSCR_VAL_VINIT_COMPATIBLE);
	for (i = 0; i < nbVInit; i++) {
		if (strcmp(VInit[i], tmp) == 0) {
			curVInit = i;
			break;
		}
	}

	bpp = (int)GfParmGetNum(paramHdle, GFSCR_SECT_PROP, GFSCR_ATT_BPP, NULL, 24);
	sprintf(buf, "%d", bpp);
	for (i = 0; i < nbDepth; i++) {
		if (!strcmp(buf, Depthlist[i])) {
			curDepth = i;
			break;
		}
	}

	curMaxFreq = (int)GfParmGetNum(paramHdle, GFSCR_SECT_PROP, GFSCR_ATT_MAXREFRESH, NULL, curMaxFreq);
}

#ifdef WIN32
static void
ChangeMaxFreq(void * /* dummy */)
{
    char	*val;
    
    val = GfuiEditboxGetString(scrHandle, MaxFreqId);
    curMaxFreq = (int)strtol(val, (char **)NULL, 0);
    sprintf(buf, "%d", curMaxFreq);
    GfuiEditboxSetString(scrHandle, MaxFreqId, buf);
}
#endif

static void
onActivate(void * /* dummy */)
{
    initFromConf();
    updateLabelText();
}


/** Create and activate the video options menu screen.
    @ingroup	screen
    @param	precMenu	previous menu to return to
*/
void *
GfScrMenuInit(void *prevMenu)
{
	sprintf(buf, "%s%s", GetLocalDir(), GFSCR_CONF_FILE);
	paramHdle = GfParmReadFile(buf, GFPARM_RMODE_STD | GFPARM_RMODE_CREAT);
	
	if (scrHandle)
		return scrHandle;

	scrHandle = GfuiScreenCreateEx((float*)NULL, NULL, onActivate, NULL, (tfuiCallback)NULL, 1);
	void *param = LoadMenuXML("screenconfigmenu.xml");
	CreateStaticControls(param,scrHandle);

	CreateButtonControl(scrHandle,param,"resleftarrow",(void*)-1,ResPrevNext);
	CreateButtonControl(scrHandle,param,"resrightarrow",(void*)1,ResPrevNext);
	ResLabelId = CreateLabelControl(scrHandle,param,"reslabel");

	CreateButtonControl(scrHandle, param, "accept", NULL, GfScrReinit);
	CreateButtonControl(scrHandle, param, "cancel", prevMenu, GfuiScreenActivate);

	CreateButtonControl(scrHandle,param,"depthleftarrow",(void*)-1,DepthPrevNext);
	CreateButtonControl(scrHandle,param,"depthrightarrow",(void*)1,DepthPrevNext);
	DepthLabelId = CreateLabelControl(scrHandle,param,"depthlabel");

	CreateButtonControl(scrHandle,param,"displeftarrow",(void*)-1,ModePrevNext);
	CreateButtonControl(scrHandle,param,"disprightarrow",(void*)1,ModePrevNext);
	ModeLabelId = CreateLabelControl(scrHandle,param,"displabel");

#ifdef WIN32
	CreateLabelControl(scrHandle,param,"maxfreqlabel");
	MaxFreqId = CreateEditControl(scrHandle,param,"freqedit",NULL,NULL,ChangeMaxFreq);
#endif

	CreateButtonControl(scrHandle,param,"vmleftarrow",(void*)-1, VInitPrevNext);
	CreateButtonControl(scrHandle,param,"vmrightarrow",(void*)1, VInitPrevNext);
	VInitLabelId = CreateLabelControl(scrHandle,param,"vmlabel");
	GfParmReleaseHandle(param);

	GfuiAddKey(scrHandle, GFUIK_RETURN, "Accept", NULL, GfScrReinit, NULL);
	GfuiAddKey(scrHandle, GFUIK_ESCAPE, "Cancel", prevMenu, GfuiScreenActivate, NULL);
	GfuiAddSKey(scrHandle, GFUIK_LEFT, "Previous Resolution", (void*)-1, ResPrevNext, NULL);
	GfuiAddSKey(scrHandle, GFUIK_RIGHT, "Next Resolution", (void*)1, ResPrevNext, NULL);
	GfuiAddSKey(scrHandle, GFUIK_F1, "Help", scrHandle, GfuiHelpScreen, NULL);
	GfuiAddSKey(scrHandle, GFUIK_F12, "Screen-Shot", NULL, GfuiScreenShot, NULL);
    

	return scrHandle;
}


