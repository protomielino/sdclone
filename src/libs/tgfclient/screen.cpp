/***************************************************************************
                           screen.cpp -- screen init
                             -------------------
    created              : Fri Aug 13 22:29:56 CEST 1999
    copyright            : (C) 1999, 2004 by Eric Espie, Bernhard Wymann
    email                : torcs@free.fr
    version              : $Id: screen.cpp,v 1.23 2008/02/21 09:27:38 torcs Exp $
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
    @version	$Id: screen.cpp,v 1.23 2008/02/21 09:27:38 torcs Exp $
    @ingroup	screen
*/

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

#include <GL/glut.h>
#include <math.h>
#ifndef WIN32
#include <unistd.h>
#else
#include <process.h>
#endif /* WIN32 */

#include <tgfclient.h>
#include <portability.h>
#include "gui.h"
#include "fg_gm.h"
#include "glfeatures.h"

//#ifndef WIN32
//#define USE_RANDR_EXT
//#endif // WIN32

#ifdef USE_RANDR_EXT
#include <GL/glx.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>
#include <X11/extensions/Xrandr.h>
#endif // USE_RANDR_EXT

static int GfScrWidth;
static int GfScrHeight;
static int GfViewWidth;
static int GfViewHeight;
static int GfScrCenX;
static int GfScrCenY;

static void	*scrHandle = NULL;
static char 	buf[1024];

static int usedGM = 0;
#if !defined(FREEGLUT) && !defined(WIN32)
static int usedFG = 0;
#endif

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

		XRRScreenConfiguration *screenconfig = XRRGetScreenInfo (display, root);
		if (screenconfig) {
			int i, j, nsize;
			XRRScreenSize *sizes = XRRConfigSizes(screenconfig, &nsize);

			if (nsize > 0) {
				// Force 320x200, 640x480, 800x600 to be available to the user,
				// even if X did not report about.
				int check_resx[] = {320, 640, 800};
				int check_resy[] = {240, 480, 600};
				bool mode_in_list[] = {false, false, false};
				int add_modes = sizeof(check_resx)/sizeof(check_resx[0]);

				for (i = 0; i < nsize; i++) {
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

			}

			XRRFreeScreenConfigInfo(screenconfig);
		}
		XCloseDisplay(display);
	}

	if (Res == NULL || nbRes == 0) {
		// We failed to get a handle to the display, so fill in some defaults.
		GfError("Failed to initialize resolutions for display '%s' ; using defaults", 
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
    int		Window;
    int		xw, yw;
    int		winX, winY;
    void	*handle;
    const char	*fscr;
    const char	*vinit;
    int		fullscreen;
    int		maxfreq;
    int		i, depth;

    sprintf(buf, "%s%s", GetLocalDir(), GFSCR_CONF_FILE);
    handle = GfParmReadFile(buf, GFPARM_RMODE_STD | GFPARM_RMODE_CREAT);
    xw = (int)GfParmGetNum(handle, GFSCR_SECT_PROP, GFSCR_ATT_X, (char*)NULL, 640);
    yw = (int)GfParmGetNum(handle, GFSCR_SECT_PROP, GFSCR_ATT_Y, (char*)NULL, 480);
    winX = (int)GfParmGetNum(handle, GFSCR_SECT_PROP, GFSCR_ATT_WIN_X, (char*)NULL, xw);
    winY = (int)GfParmGetNum(handle, GFSCR_SECT_PROP, GFSCR_ATT_WIN_Y, (char*)NULL, yw);
    depth = (int)GfParmGetNum(handle, GFSCR_SECT_PROP, GFSCR_ATT_BPP, (char*)NULL, 32);
    maxfreq = (int)GfParmGetNum(handle, GFSCR_SECT_PROP, GFSCR_ATT_MAXREFRESH, (char*)NULL, 160);
    GfViewWidth = xw;
    GfViewHeight = yw;
    GfScrCenX = xw / 2;
    GfScrCenY = yw / 2;

	// The fullscreen hack must be run before glutInit, such that glut gets the right screen size, etc.
	fscr = GfParmGetStr(handle, GFSCR_SECT_PROP, GFSCR_ATT_FSCR, GFSCR_VAL_NO);
	fullscreen = 0;
#if !defined(FREEGLUT) && !defined(WIN32)
	if (strcmp(fscr, GFSCR_VAL_YES) == 0) {	// Resize the screen
		GfOut ("Freeglut not detected...\n");
		for (i = maxfreq; i > 59; i--) {
			sprintf(buf, "%dx%d:%d@%d", winX, winY, depth, i);
			GfOut("Trying %s video mode\n", buf);
			fglutGameModeString(buf);
			if (fglutEnterGameMode()) {
				GfTrace("OK for %s video mode\n", buf);
				usedFG = 1;
				break;
			}
		}
		if (!usedFG)
		  GfError("Could not find any usable video mode\n");
	}
#endif

	vinit = GfParmGetStr(handle, GFSCR_SECT_PROP, GFSCR_ATT_VINIT, GFSCR_VAL_VINIT_COMPATIBLE);

	glutInit(&argc, argv);

	// Depending on "video mode init" settings, try to get the best mode or try to get a mode in a safe way...
	// This is a workaround for driver/glut/glx bug, which lies about the capabilites of the visual.

	GfTrace("Visual Properties Report\n");
	GfTrace("------------------------\n");

	if (strcmp(vinit, GFSCR_VAL_VINIT_BEST) == 0) {

		// Try to get "best" videomode, z-buffer >= 24bit, visual with alpha channel,
		// antialiasing support.

		int visualDepthBits = 24;
		bool visualSupportsMultisample = true;
		bool visualSupportsAlpha = true;

		glutInitDisplayString("rgba double depth>=24 samples alpha");

		if (!glutGet(GLUT_DISPLAY_MODE_POSSIBLE)) {
			// Failed, try without antialiasing support.
			visualDepthBits = 24;
			visualSupportsMultisample = false;
			visualSupportsAlpha = true;
			glutInitDisplayString("rgba double depth>=24 alpha");
		}

		if (!glutGet(GLUT_DISPLAY_MODE_POSSIBLE)) {
			// Failed, try without alpha channel.
			visualDepthBits = 24;
			visualSupportsMultisample = true;
			visualSupportsAlpha = false;
			glutInitDisplayString("rgb double depth>=24 samples");
		}

		if (!glutGet(GLUT_DISPLAY_MODE_POSSIBLE)) {
			// Failed, try without antialiasing and alpha support.
			visualDepthBits = 24;
			visualSupportsMultisample = false;
			visualSupportsAlpha = false;
			glutInitDisplayString("rgb double depth>=24");
		}

		if (!glutGet(GLUT_DISPLAY_MODE_POSSIBLE)) {
			// Failed, try without 24 bit z-Buffer and without antialiasing.
			visualDepthBits = 16;
			visualSupportsMultisample = false;
			visualSupportsAlpha = true;
			glutInitDisplayString("rgba double depth>=16 alpha");
		}

		if (!glutGet(GLUT_DISPLAY_MODE_POSSIBLE)) {
			// Failed, try without 24 bit z-Buffer, without antialiasing and without alpha.
			visualDepthBits = 16;
			visualSupportsMultisample = false;
			visualSupportsAlpha = false;
			glutInitDisplayString("rgb double depth>=16");
		}

		if (!glutGet(GLUT_DISPLAY_MODE_POSSIBLE)) {
			// All failed.
			GfTrace("The minimum display requirements are not fulfilled.\n");
			GfTrace("We need a double buffered RGB visual with a 16 bit depth buffer at least.\n");
			// Try fallback as last resort.
			GfTrace("Trying generic initialization, fallback.\n");
			glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
		} else {
			// We have got a mode, report the properties.
			GfTrace("View size: %dx%d\n", winX, winY);
			GfTrace("z-buffer depth: %d (%s)\n", visualDepthBits, visualDepthBits < 24 ? "bad" : "good");
			GfTrace("multisampling : %s\n", visualSupportsMultisample ? "available" : "no");
			GfTrace("alpha bits    : %s\n", visualSupportsAlpha ? "available" : "no");
			if (visualDepthBits < 24) {
				// Show a hint if the z-buffer depth is not optimal.
				GfTrace("The z-buffer resolution is below 24 bit, you will experience rendering\n");
				GfTrace("artefacts. Try to improve the setup of your graphics board or look\n");
				GfTrace("for an alternate driver.\n");
			}
		}
	} else {
		// Compatibility mode.
		glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
		GfTrace("View size: %dx%d\n", winX, winY);
		GfTrace("Compatibility mode, other properties unknown.\n");
	}


	if (strcmp(fscr, GFSCR_VAL_YES) == 0) {
		for (i = maxfreq; i > 59; i--) {
			sprintf(buf, "%dx%d:%d@%d", winX, winY, depth, i);
			glutGameModeString(buf);
			GfOut("2 - Trying %s video mode\n", buf);
			if (glutGameModeGet(GLUT_GAME_MODE_POSSIBLE)) {
				GfOut("2- %s video mode available\n", buf);
				glutEnterGameMode();
				if (glutGameModeGet(GLUT_GAME_MODE_DISPLAY_CHANGED)) {
					GfOut("OK for %s video mode\n", buf);
					usedGM = 1;
					fullscreen = 1;
					break;
				} else {
					glutLeaveGameMode();
				}
			}
		}
		if (!usedGM)
		  GfError("Could not find any usable video mode\n");

	}

	if (!fullscreen) {
		/* Give an initial size and position so user doesn't have to place window */
		glutInitWindowPosition(0, 0);
		glutInitWindowSize(winX, winY);

		/* Create the window (with a smart versioned title) */
		sprintf(buf, "Speed Dreams %s", VERSION);
		Window = glutCreateWindow(buf);
		if (!Window) {
			GfError("Error, couldn't open window\n");
			GfScrShutdown();
			exit(1);
		}

		/* Set a versioned title for iconified window */
		glutSetIconTitle(buf);
	}

	if ((strcmp(fscr, GFSCR_VAL_YES) == 0) && (!fullscreen)) {
		/* glutVideoResize(0, 0, winX, winY); */
		glutFullScreen();
	}

    GfParmReleaseHandle(handle);

    glutReshapeFunc( Reshape );

	checkGLFeatures();
}

/** Shutdown the screen
    @ingroup	screen
    @return	none
*/
void GfScrShutdown(void)
{
    if (usedGM) {
	glutLeaveGameMode();
    }
#if !defined(FREEGLUT) && !defined(WIN32)
    if (usedFG) {
	fglutLeaveGameMode();
    }
#endif

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

	GfParmSetStr(paramHdle, GFSCR_SECT_PROP, GFSCR_ATT_VINIT, VInit[curVInit]);

	if (curMode == 0) {
		GfParmSetStr(paramHdle, GFSCR_SECT_PROP, GFSCR_ATT_FSCR, "yes");
	} else {
		GfParmSetStr(paramHdle, GFSCR_SECT_PROP, GFSCR_ATT_FSCR, "no");
	}
	GfParmWriteFile(NULL, paramHdle, "Screen");
}


void
GfScrReinit(void * /* dummy */)
{
    int retcode = 0;
    static const int CMDSIZE = 1024;
    char cmd[CMDSIZE];

#ifndef WIN32
    const char	*arg[8];
    int		curArg;
#endif

    saveParams();

#ifdef WIN32
	snprintf(cmd, CMDSIZE, "%sspeed-dreams.exe", GetDataDir());
	int i;
	for (i = 0; i < CMDSIZE && cmd[i] != NULL; i++) {
		if (cmd[i] == '/') {
			cmd[i] = '\\';
		}
	}
	
	char cmdarg[CMDSIZE];
	strcpy(cmdarg, "speed-dreams.exe");
	//snprintf(cmdarg, CMDSIZE, "speed-dreams.exe", GetDataDir());
	for (i = 0; i < CMDSIZE && cmdarg[i] != NULL; i++) {
		if (cmdarg[i] == '/') {
			cmdarg[i] = '\\';
		}
	}

	retcode = execlp(cmd, cmdarg, (const char *)NULL);
#else
    GfScrShutdown();

    sprintf (cmd, "%sspeed-dreams-bin", GetLibDir ());
    memset (arg, 0, sizeof (arg));
    curArg = 0;
    if (GfuiMouseHW) {
	arg[curArg++] = "-m";
    }
    
    if (strlen(GetLocalDir ())) {
	arg[curArg++] = "-l";
	arg[curArg++] = GetLocalDir ();
    }

    if (strlen(GetLibDir ())) {
	arg[curArg++] = "-L";
	arg[curArg++] = GetLibDir ();
    }

    if (strlen(GetDataDir ())) {
	arg[curArg++] = "-D";
	arg[curArg++] = GetDataDir ();
    }

    switch (curArg) {
    case 0:
	retcode = execlp (cmd, cmd, (const char *)NULL);
	break;
    case 1:
	retcode = execlp (cmd, cmd, arg[0], (const char *)NULL);
	break;
    case 2:
	retcode = execlp (cmd, cmd, arg[0], arg[1], (const char *)NULL);
	break;
    case 3:
	retcode = execlp (cmd, cmd, arg[0], arg[1], arg[2], (const char *)NULL);
	break;
    case 4:
	retcode = execlp (cmd, cmd, arg[0], arg[1], arg[2], arg[3], (const char *)NULL);
	break;
    case 5:
	retcode = execlp (cmd, cmd, arg[0], arg[1], arg[2], arg[3], arg[4], (const char *)NULL);
	break;
    case 6:
	retcode = execlp (cmd, cmd, arg[0], arg[1], arg[2], arg[3], arg[4], arg[5], (const char *)NULL);
	break;
    case 7:
	retcode = execlp (cmd, cmd, arg[0], arg[1], arg[2], arg[3], arg[4], arg[5], arg[6], (const char *)NULL);
	break;
    case 8:
	retcode = execlp (cmd, cmd, arg[0], arg[1], arg[2], arg[3], arg[4], arg[5], arg[6], arg[7], (const char *)NULL);
	break;
    }


#endif
    if (retcode) {
	perror(cmd);
	exit(1);
    }
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
	GfuiLabelSetText (scrHandle, VInitLabelId, VInit[curVInit]);
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
GfScrMenuInit(void *precMenu)
{
    sprintf(buf, "%s%s", GetLocalDir(), GFSCR_CONF_FILE);
    paramHdle = GfParmReadFile(buf, GFPARM_RMODE_STD | GFPARM_RMODE_CREAT);

    if (scrHandle) return scrHandle;

    scrHandle = GfuiScreenCreateEx((float*)NULL, NULL, onActivate, NULL, (tfuiCallback)NULL, 1);
     void *param = LoadMenuXML("screenconfigmenu.xml");
     CreateStaticControls(param,scrHandle);

	CreateButtonControl(scrHandle,param,"resleftarrow",(void*)-1,ResPrevNext);
	CreateButtonControl(scrHandle,param,"resrightarrow",(void*)1,ResPrevNext);
	ResLabelId = CreateLabelControl(scrHandle,param,"reslabel");

	GfuiAddSKey(scrHandle, GLUT_KEY_LEFT, "Previous Resolution", (void*)-1, ResPrevNext, NULL);
    GfuiAddSKey(scrHandle, GLUT_KEY_RIGHT, "Next Resolution", (void*)1, ResPrevNext, NULL);
    GfuiAddKey(scrHandle, 13, "Apply Mode", NULL, GfScrReinit, NULL);
    GfuiButtonCreate(scrHandle, "Apply", GFUI_FONT_LARGE, 210, 40, 150, GFUI_ALIGN_HC_VB, GFUI_MOUSE_UP,
		     NULL, GfScrReinit, NULL, (tfuiCallback)NULL, (tfuiCallback)NULL);
    GfuiAddKey(scrHandle, 27, "Cancel", precMenu, GfuiScreenActivate, NULL);
    GfuiButtonCreate(scrHandle, "Back", GFUI_FONT_LARGE, 430, 40, 150, GFUI_ALIGN_HC_VB, GFUI_MOUSE_UP,
		     precMenu, GfuiScreenActivate, NULL, (tfuiCallback)NULL, (tfuiCallback)NULL);

	CreateButtonControl(scrHandle,param,"depthleftarrow",(void*)-1,DepthPrevNext);
	CreateButtonControl(scrHandle,param,"depthrightarrow",(void*)1,DepthPrevNext);
	DepthLabelId = CreateLabelControl(scrHandle,param,"depthlabel");

	CreateButtonControl(scrHandle,param,"displeftarrow",(void*)-1,ModePrevNext);
	CreateButtonControl(scrHandle,param,"disprightarrow",(void*)1,ModePrevNext);
	ModeLabelId = CreateLabelControl(scrHandle,param,"displabel");

#ifdef WIN32
	CreateLabelControl(scrHandle,param,"maxfreqlabel");
	MaxFreqId = CreateEditControl(scrHandle,param,"freqedit",NULL,ChangeMaxFreq,NULL);
#endif

	CreateButtonControl(scrHandle,param,"vmleftarrow",(void*)-1, VInitPrevNext);
	CreateButtonControl(scrHandle,param,"vmrightarrow",(void*)1, VInitPrevNext);
	VInitLabelId = CreateLabelControl(scrHandle,param,"vmlabel");

    return scrHandle;
}



int GfuiGlutExtensionSupported(char const *str)
{
    return glutExtensionSupported(str);
}
