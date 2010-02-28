/***************************************************************************

    file                 : grmain.cpp
    created              : Thu Aug 17 23:23:49 CEST 2000
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

#ifdef WIN32
#include <windows.h>
#endif


#include <plib/ssg.h>
#include <glfeatures.h>
#include <robot.h>	//ROB_SECT_ARBITRARY

#include "grmain.h"
#include "grshadow.h"
#include "grskidmarks.h"
#include "grsmoke.h"
#include "grcar.h"
#include "grscreen.h"
#include "grscene.h"
#include "grsound.h"
#include "grutil.h"
#include "grcarlight.h"
#include "grboard.h"
#include "grtracklight.h"

int maxTextureUnits = 0;
static double OldTime;
static int nFrame;
static int nTotalFrame;
static int nSeconds;
float grFps;
double grCurTime;
double grDeltaTime;
int segIndice = 0;

tdble grMaxDammage = 10000.0;
int grNbCars = 0;

void *grHandle = NULL;
void *grTrackHandle = NULL;

int grWinx, grWiny, grWinw, grWinh;

static float grMouseRatioX, grMouseRatioY;

tgrCarInfo *grCarInfo;
ssgContext grContext;
class cGrScreen *grScreens[GR_NB_MAX_SCREEN] = {NULL, NULL, NULL, NULL};
int grNbScreen = 1;
tdble grLodFactorValue = 1.0;


static char buf[1024];

#ifdef WIN32
#include <GL/glext.h>
PFNGLMULTITEXCOORD2FARBPROC glMultiTexCoord2fARB = NULL;
PFNGLMULTITEXCOORD2FVARBPROC glMultiTexCoord2fvARB = NULL;
PFNGLACTIVETEXTUREARBPROC glActiveTextureARB = NULL;
PFNGLCLIENTACTIVETEXTUREARBPROC glClientActiveTextureARB = NULL;
#endif


// InitMultiTex
// desc: sets up OpenGL for multitexturing support
bool InitMultiTex(void)
{
	if (GetSingleTextureMode ()) {
		maxTextureUnits = 1;
		return true;
    } else {
		// list of available extensions
		char *extensionStr = (char*)glGetString(GL_EXTENSIONS);
		if (extensionStr == NULL)
			return false;

		if (strstr(extensionStr, "GL_ARB_multitexture")) {
			// retrieve the maximum number of texture units allowed
			glGetIntegerv(GL_MAX_TEXTURE_UNITS_ARB, &maxTextureUnits);
#ifdef WIN32
			// retrieve addresses of multitexturing functions
			glMultiTexCoord2fARB = (PFNGLMULTITEXCOORD2FARBPROC) wglGetProcAddress("glMultiTexCoord2fARB");
			glActiveTextureARB = (PFNGLACTIVETEXTUREARBPROC) wglGetProcAddress("glActiveTextureARB");
			glClientActiveTextureARB = (PFNGLCLIENTACTIVETEXTUREARBPROC) wglGetProcAddress("glClientActiveTextureARB");
			glMultiTexCoord2fvARB = (PFNGLMULTITEXCOORD2FVARBPROC) wglGetProcAddress("glMultiTexCoord2fvARB");
#endif
			return true;
		} else {
			return false;
		}
    }
}


static void grAdaptScreenSize(void)
{
    switch (grNbScreen) {
    case 0:
    case 1:
	grScreens[0]->activate(grWinx, grWiny, grWinw, grWinh);
	grScreens[1]->deactivate();
	grScreens[2]->deactivate();
	grScreens[3]->deactivate();
	break;
    case 2:
	grScreens[0]->activate(grWinx, grWiny + grWinh / 2, grWinw, grWinh / 2);
	grScreens[1]->activate(grWinx, grWiny,              grWinw, grWinh / 2);
	grScreens[2]->deactivate();
	grScreens[3]->deactivate();
	break;
    case 3:
	grScreens[0]->activate(grWinx,              grWiny + grWinh / 2, grWinw / 2, grWinh / 2);
	grScreens[1]->activate(grWinx + grWinw / 2, grWiny + grWinh / 2, grWinw / 2, grWinh / 2);
	grScreens[2]->activate(grWinx + grWinw / 4, grWiny,              grWinw / 2, grWinh / 2);
	grScreens[3]->deactivate();
	break;
    case 4:
	grScreens[0]->activate(grWinx,              grWiny + grWinh / 2, grWinw / 2, grWinh / 2);
	grScreens[1]->activate(grWinx + grWinw / 2, grWiny + grWinh / 2, grWinw / 2, grWinh / 2);
	grScreens[2]->activate(grWinx,              grWiny,              grWinw / 2, grWinh / 2);
	grScreens[3]->activate(grWinx + grWinw / 2, grWiny,              grWinw / 2, grWinh / 2);
	break;
    }
}

static void
grSplitScreen(void *vp)
{
    long p = (long)vp;

    switch (p) {
    case GR_SPLIT_ADD:
	grNbScreen++;
	if (grNbScreen > GR_NB_MAX_SCREEN) {
	    grNbScreen = GR_NB_MAX_SCREEN;
	}
	break;
    case GR_SPLIT_REM:
	grNbScreen--;
	if (grNbScreen < 1) {
	    grNbScreen = 1;
	}
	break;
    }
    GfParmSetNum(grHandle, GR_SCT_DISPMODE, GR_ATT_NB_SCREENS, NULL, grNbScreen);
    GfParmWriteFile(NULL, grHandle, "Graph");
    grAdaptScreenSize();
}

static class cGrScreen *
grGetcurrentScreen(void)
{
    tMouseInfo	*mouse;
    int		i;
    int		x, y;

    mouse = GfuiMouseInfo();
    x = (int)(mouse->X * grMouseRatioX);
    y = (int)(mouse->Y * grMouseRatioY);
    for (i = 0; i < GR_NB_MAX_SCREEN; i++) {
	if (grScreens[i]->isInScreen(x, y)) {
	    return grScreens[i];
	}
    }
    return grScreens[0];
}

static void
grSetZoom(void *vp)
{
    grGetcurrentScreen()->setZoom((long)vp);
}

static void
grSelectCamera(void *vp)
{
    grGetcurrentScreen()->selectCamera((long)vp);
}

static void
grSelectBoard(void *vp)
{
    grGetcurrentScreen()->selectBoard((long)vp);
}

static void
grSelectTrackMap(void * /* vp */)
{
    grGetcurrentScreen()->selectTrackMap();
}

static void
grPrevCar(void * /* dummy */)
{
    grGetcurrentScreen()->selectPrevCar();
}

static void
grNextCar(void * /* dummy */)
{
    grGetcurrentScreen()->selectNextCar();
}

static void
grSwitchMirror(void * /* dummy */)
{
    grGetcurrentScreen()->switchMirror();
}

int
initView(int x, int y, int width, int height, int /* flag */, void *screen)
{
    int i;

    if (maxTextureUnits==0)
      {
	InitMultiTex();
      }
    
    grWinx = x;
    grWiny = y;
    grWinw = width;
    grWinh = height;

    grMouseRatioX = width / 640.0;
    grMouseRatioY = height / 480.0;

    OldTime = GfTimeClock();
    nFrame = 0;
    nTotalFrame = 0;
    nSeconds = 0;
    grFps = 0;

    if (!grHandle)
    {
    	sprintf(buf, "%s%s", GetLocalDir(), GR_PARAM_FILE);
    	grHandle = GfParmReadFile(buf, GFPARM_RMODE_STD | GFPARM_RMODE_CREAT);
    }

    for (i = 0; i < GR_NB_MAX_SCREEN; i++) {
	grScreens[i]->initBoard ();
    }

    GfuiAddSKey(screen, GFUIK_HOME,     "Zoom Maximum", (void*)GR_ZOOM_MAX,	grSetZoom, NULL);
    GfuiAddSKey(screen, GFUIK_END,      "Zoom Minimum", (void*)GR_ZOOM_MIN,	grSetZoom, NULL);
    GfuiAddKey(screen, '*',            "Zoom Default", (void*)GR_ZOOM_DFLT,	grSetZoom, NULL);

    GfuiAddSKey(screen, GFUIK_PAGEUP,   "Select Previous Car", (void*)0, grPrevCar, NULL);
    GfuiAddSKey(screen, GFUIK_PAGEDOWN, "Select Next Car",     (void*)0, grNextCar, NULL);

    GfuiAddSKey(screen, GFUIK_F2,       "Driver Views",      (void*)0, grSelectCamera, NULL);
    GfuiAddSKey(screen, GFUIK_F3,       "Car Views",         (void*)1, grSelectCamera, NULL);
    GfuiAddSKey(screen, GFUIK_F4,       "Side Car Views",    (void*)2, grSelectCamera, NULL);
    GfuiAddSKey(screen, GFUIK_F5,       "Up Car View",       (void*)3, grSelectCamera, NULL);
    GfuiAddSKey(screen, GFUIK_F6,       "Persp Car View",    (void*)4, grSelectCamera, NULL);
    GfuiAddSKey(screen, GFUIK_F7,       "All Circuit Views", (void*)5, grSelectCamera, NULL);
    GfuiAddSKey(screen, GFUIK_F8,       "Track View",        (void*)6, grSelectCamera, NULL);
    GfuiAddSKey(screen, GFUIK_F9,       "Track View Zoomed", (void*)7, grSelectCamera, NULL);
    GfuiAddSKey(screen, GFUIK_F10,      "Follow Car Zoomed", (void*)8, grSelectCamera, NULL);
    GfuiAddSKey(screen, GFUIK_F11,      "TV Director View",  (void*)9, grSelectCamera, NULL);

    GfuiAddKey(screen, '5',            "FPS Counter",       (void*)3, grSelectBoard, NULL);
    GfuiAddKey(screen, '4',            "G/Cmd Graph",       (void*)4, grSelectBoard, NULL);
    GfuiAddKey(screen, '3',            "Leaders Board",     (void*)2, grSelectBoard, NULL);
    GfuiAddKey(screen, '2',            "Driver Counters",   (void*)1, grSelectBoard, NULL);
    GfuiAddKey(screen, '1',            "Driver Board",      (void*)0, grSelectBoard, NULL);
    GfuiAddKey(screen, '9',            "Mirror",            (void*)0, grSwitchMirror, NULL);
    GfuiAddKey(screen, '0',            "Arcade Board",      (void*)5, grSelectBoard, NULL);
    GfuiAddKey(screen,  '>',           "Zoom In",           (void*)GR_ZOOM_IN,	grSetZoom, NULL);
    GfuiAddKey(screen,  '<',           "Zoom Out",          (void*)GR_ZOOM_OUT,	grSetZoom, NULL);
    GfuiAddKey(screen, '[',            "Split Screen",   (void*)GR_SPLIT_ADD, grSplitScreen, NULL);
    GfuiAddKey(screen, ']',            "UnSplit Screen", (void*)GR_SPLIT_REM, grSplitScreen, NULL);
    GfuiAddKey(screen, 'm',            "Track Maps",     (void*)0, grSelectTrackMap, NULL);

    grAdaptScreenSize();

    grInitScene();

    grLodFactorValue = GfParmGetNum(grHandle, GR_SCT_GRAPHIC, GR_ATT_LODFACTOR, NULL, 1.0);

    return 0;
}


int
refresh(tSituation *s)
{
    int			i;

    START_PROFILE("refresh");

    nFrame++;
    nTotalFrame++;
    grCurTime = GfTimeClock();
    grDeltaTime = grCurTime - OldTime;
    if ((grCurTime - OldTime) > 1.0) {
	/* The Frames Per Second (FPS) display is refreshed every second */
	grFps = (tdble)nFrame / (grCurTime - OldTime);
	nFrame = 0;
	++nSeconds;
	OldTime = grCurTime;
    }

    TRACE_GL("refresh: start");

    START_PROFILE("grRefreshSound*");
    grRefreshSound(s, grScreens[0]->getCurCamera());
    STOP_PROFILE("grRefreshSound*");

    START_PROFILE("grDrawBackground/glClear");
    glDepthFunc(GL_LEQUAL);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    STOP_PROFILE("grDrawBackground/glClear");

    for (i = 0; i < GR_NB_MAX_SCREEN; i++) {
	grScreens[i]->update(s, grFps);
    }

    grUpdateSmoke(s->currentTime);
    grTrackLightUpdate(s);

    STOP_PROFILE("refresh");
    return 0;
}

int
initCars(tSituation *s)
{
    char	idx[16];
    int		index;
    int		i;
    tCarElt 	*elt;
    void	*hdle;

    TRACE_GL("initCars: start");

    if (!grHandle)
    {
        sprintf(buf, "%s%s", GetLocalDir(), GR_PARAM_FILE);
        grHandle = GfParmReadFile(buf, GFPARM_RMODE_STD | GFPARM_RMODE_CREAT);
    }

    grInitCommonState();
    grInitCarlight(s->_ncars);
    grMaxDammage = (tdble)s->_maxDammage;
    grNbCars = s->_ncars;

    grCustomizePits();

    grCarInfo = (tgrCarInfo*)calloc(s->_ncars, sizeof(tgrCarInfo));

	for (i = 0; i < s->_ncars; i++) {
		elt = s->cars[i];
		/* Shadow init (Should be done before the cars for display order) */
		grInitShadow(elt);
		/* Skidmarks init */
		grInitSkidmarks(elt);
	}

    grNbScreen = 0;
    for (i = 0; i < s->_ncars; i++) {
	elt = s->cars[i];
	index = elt->index;
	hdle = elt->_paramsHandle;
	//sprintf(idx, "Robots/index/%d", elt->_driverIndex);
	//grCarInfo[index].iconColor[0] = GfParmGetNum(hdle, idx, "red",   (char*)NULL, 0);
	//grCarInfo[index].iconColor[1] = GfParmGetNum(hdle, idx, "green", (char*)NULL, 0);
	//grCarInfo[index].iconColor[2] = GfParmGetNum(hdle, idx, "blue",  (char*)NULL, 0);
	if (elt->_driverType == RM_DRV_HUMAN) 
	{
	if (elt->_driverIndex > 10)
		sprintf(idx, "Robots/index/%d", elt->_driverIndex - 11);
	else
		sprintf(idx, "Robots/index/%d", elt->_driverIndex);
	} else
	{
		sprintf(idx, "Robots/index/%d", elt->_driverIndex);
	}
	grCarInfo[index].iconColor[0] = GfParmGetNum(hdle, idx, "red",   (char*)NULL, GfParmGetNum(hdle, ROB_SECT_ARBITRARY, "red",   NULL, 0));
	grCarInfo[index].iconColor[1] = GfParmGetNum(hdle, idx, "green", (char*)NULL, GfParmGetNum(hdle, ROB_SECT_ARBITRARY, "green", NULL, 0));
	grCarInfo[index].iconColor[2] = GfParmGetNum(hdle, idx, "blue",  (char*)NULL, GfParmGetNum(hdle, ROB_SECT_ARBITRARY, "blue",  NULL, 0));
	grCarInfo[index].iconColor[3] = 1.0;
	grInitCar(elt);
	if ((elt->_driverType == RM_DRV_HUMAN) && (grNbScreen < GR_NB_MAX_SCREEN) &&(elt->_networkPlayer == 0)) 
	{
	    grScreens[grNbScreen]->setCurrentCar(elt);
	    grNbScreen++;
	}
    }

    if (grNbScreen == 0) {
	grNbScreen = (int)GfParmGetNum(grHandle, GR_SCT_DISPMODE, GR_ATT_NB_SCREENS, NULL, 1.0);
    }

    for (i = 0; i < GR_NB_MAX_SCREEN; i++) {
	grScreens[i]->initCams(s);
    }

    TRACE_GL("initCars: end");

    grInitSmoke(s->_ncars);
    grInitSound(s, s->_ncars);
    grTrackLightInit();

    grAdaptScreenSize();

    return 0;
    
}

void
shutdownCars(void)
{
	int i;

	GfOut("-- shutdownCars\n");
	grShutdownSound(grNbCars);
	if (grNbCars) {
		grShutdownBoardCar();
		grShutdownSkidmarks();
		grShutdownSmoke();
		grShutdownCarlight();
		grTrackLightShutdown();
		/* Delete ssg objects */
		CarsAnchor->removeAllKids();
		ShadowAnchor->removeAllKids();
		for (i = 0; i < grNbCars; i++) {
			ssgDeRefDelete(grCarInfo[i].envSelector);
			ssgDeRefDelete(grCarInfo[i].shadowBase);
			if (grCarInfo[i].driverSelectorinsg == false) {
				delete grCarInfo[i].driverSelector;
			}
		}

		PitsAnchor->removeAllKids();
		ThePits = 0;
		free(grCarInfo);
	}

	GfParmReleaseHandle(grHandle);
	grHandle = NULL;

	for (i = 0; i < GR_NB_MAX_SCREEN; i++) {
		grScreens[i]->setCurrentCar(NULL);
	}

	if (nSeconds > 0)
		printf( "Average FPS: %.2f\n", (double)nTotalFrame/((double)nSeconds+GfTimeClock()-OldTime) );}

int
initTrack(tTrack *track)
{
	int i;

	// The inittrack does as well init the context, that is highly inconsistent, IMHO.
	// TODO: Find a solution to init the graphics first independent of objects.
	grContext.makeCurrent();

	grTrackHandle = GfParmReadFile(track->filename, GFPARM_RMODE_STD | GFPARM_RMODE_CREAT);
	if (grNbScreen)
		grLoadScene(track);

	for (i = 0; i < GR_NB_MAX_SCREEN; i++) {
		grScreens[i] = new cGrScreen(i);
	}

	return 0;
}


void
shutdownTrack(void)
{
	int	i;

	grShutdownScene();
	grShutdownState();

	for (i = 0; i < GR_NB_MAX_SCREEN; i++) {
		if (grScreens[i] != NULL) {
			delete grScreens[i];
			grScreens[i] = NULL;
		}
	}
}

/*void bendCar (int index, sgVec3 poc, sgVec3 force, int cnt)
{
	if (grCarInfo) 
		grPropagateDamage (grCarInfo[index].carEntity, poc, force, cnt);
}*/
