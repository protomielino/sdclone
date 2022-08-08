/***************************************************************************

    file                 : OsgMain.cpp
    created              : Thu Aug 17 23:23:49 CEST 2000
    copyright            : (C)2013 by Xavier Bertaux
    email                : bertauxx@yahoo.fr
    version              : $Id: OsgMain.cpp 4712 2012-05-10 06:02:49Z mungewell $

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <osgDB/Registry>
#include <osg/Group>
#include <osgViewer/View>
#include <osgViewer/CompositeViewer>
#include <osgViewer/Viewer>

#include <glfeatures.h> // GfglFeatures
#include <robot.h>	//ROB_SECT_ARBITRARY
#include <graphic.h>

#include "OsgMain.h"
//#include "OsgOptions.h"
#include "OsgCar.h"
#include "OsgCarLight.h"
#include "OsgScenery.h"
#include "OsgRender.h"
#include "OsgMath.h"
#include "OsgScreens.h"
#include "OsgHUD.h"

//extern	osg::Timer m_timer;
//extern	osg::Timer_t m_start_tick;

//SDOptions *Options = 0;
SDCarLights *carLights = 0;
SDCars *cars = 0;
SDScenery *scenery = 0;
SDRender *render = 0;
SDScreens *screens = 0;
SDCamera *cam = 0;

SDHUD hud;

/*oid *getOptions()
{
    return Options;
}*/

SDScreens *getScreens()
{
    return screens;
}

SDRender *getRender()
{
    return render;
}

SDCars * getCars()
{
    return cars;
}

SDCarLights * getCarLights()
{
    return carLights;
}

SDScenery * getScenery()
{
    return scenery;
}

static osg::Timer m_timer;

int grMaxTextureUnits = 0;

tdble grMaxDammage = 10000.0;

void *grHandle = 0;
void *grTrackHandle = 0;

int m_Winx, m_Winy, m_Winw, m_Winh;

tdble grLodFactorValue = 1.0;

// Frame/FPS info.
static SDFrameInfo frameInfo;
static double fFPSPrevInstTime;   // Last "instant" FPS refresh time
static unsigned nFPSTotalSeconds; // Total duration since initView

// Mouse coords graphics backend to screen ratios.
static float fMouseRatioX, fMouseRatioY;

// Number of active screens.
int m_NbActiveScreens = 1;
int m_NbArrangeScreens = 0;

// Current screen index.
static int m_CurrentScreenIndex = 0;

static void SDPrevCar(void * /* dummy */)
{
    screens->getActiveView()->selectPrevCar();
}

static void SDNextCar(void * /* dummy */)
{
    screens->getActiveView()->selectNextCar();
}

void SDSelectCamera(void *vp)
{
    long t = (long)vp;
    screens->changeCamera(t);
}

void SDSetZoom(void *vp)
{
    long t = (long)vp;
    screens->getActiveView()->getCameras()->getSelectedCamera()->setZoom(t);
}

void SDSwitchMirror(void *vp)
{
    screens->getActiveView()->switchMirror();
}

/*OSG hud widgets toggle*/
void SDToggleHUD(void *vp)
{
    screens->toggleHUD();
}

void SDToggleHUDwidget(void *vp)
{
    screens->toggleHUDwidget(static_cast<const char *>(vp));
}

void SDToggleHUDwidgets(void *vp)
{
    screens->toggleHUDwidgets(static_cast<const char *>(vp));
}

void SDToggleHUDdriverinput(void *vp)
{
    screens->toggleHUDdriverinput();
}

/*Driver position change*/
void SDMoveSeatUpDown(void *move)
{
    //if positive up if negative down
    float movement = 0.005;
    if((long)move == 1){
        movement = movement*1;
    }
    if((long)move == -1){
        movement = movement*-1;
    }
    tCarElt* curCar = screens->getActiveView()->getCurrentCar();
    curCar->_drvPos_z+=movement;
}
void SDMoveSeatLeftRight(void *move)
{
    //if positive left if negative right
    float movement = 0.005;
    if((long)move == 1){
        movement = movement*1;
    }
    if((long)move == -1){
        movement = movement*-1;
    }
    tCarElt* curCar = screens->getActiveView()->getCurrentCar();
    curCar->_drvPos_y+=movement;
}
void SDMoveSeatForwardBackward(void *move)
{
    //if positive forward if negative backward
    float movement = 0.005;
    if((long)move == 1){
        movement = movement*1;
    }
    if((long)move == -1){
        movement = movement*-1;
    }
    tCarElt* curCar = screens->getActiveView()->getCurrentCar();
    curCar->_drvPos_x+=movement;
}



int initView(int x, int y, int width, int height, int /* flag */, void *screen)
{
    screens = new SDScreens();

    m_Winx = x;
    m_Winy = y;
    m_Winw = width;
    m_Winh = height;

    fMouseRatioX = width / 640.0;
    fMouseRatioY = height / 480.0;

    frameInfo.fInstFps = 0.0;
    frameInfo.fAvgFps = 0.0;
    frameInfo.nInstFrames = 0;
    frameInfo.nTotalFrames = 0;
    fFPSPrevInstTime = GfTimeClock();
    nFPSTotalSeconds = 0;

    screens->Init(x,y,width,height, render->getRoot(), render->getFogColor());

    GfuiAddKey(screen, GFUIK_END,      "Zoom Minimum", (void*)GR_ZOOM_MIN,	SDSetZoom, NULL);
    GfuiAddKey(screen, GFUIK_HOME,     "Zoom Maximum", (void*)GR_ZOOM_MAX,	SDSetZoom, NULL);
    GfuiAddKey(screen, '*',            "Zoom Default", (void*)GR_ZOOM_DFLT,	SDSetZoom, NULL);

    GfuiAddKey( screen, GFUIK_PAGEUP,   "Select Previous Car", (void*)0, SDPrevCar, NULL);
    GfuiAddKey( screen, GFUIK_PAGEDOWN, "Select Next Car",     (void*)0, SDNextCar, NULL);

    GfuiAddKey(screen, GFUIK_F2,       "Driver Views",      (void*)0, SDSelectCamera, NULL);
    GfuiAddKey(screen, GFUIK_F3,       "Car Views",         (void*)1, SDSelectCamera, NULL);
    GfuiAddKey(screen, GFUIK_F4,       "Side Car Views",    (void*)2, SDSelectCamera, NULL);
    GfuiAddKey(screen, GFUIK_F5,       "Up Car View",       (void*)3, SDSelectCamera, NULL);
    GfuiAddKey(screen, GFUIK_F6,       "Persp Car View",    (void*)4, SDSelectCamera, NULL);
    GfuiAddKey(screen, GFUIK_F7,       "All Circuit Views", (void*)5, SDSelectCamera, NULL);
    GfuiAddKey(screen, GFUIK_F8,       "Track View",        (void*)6, SDSelectCamera, NULL);
    GfuiAddKey(screen, GFUIK_F9,       "Track View Zoomed", (void*)7, SDSelectCamera, NULL);
    GfuiAddKey(screen, GFUIK_F10,      "Follow Car Zoomed", (void*)8, SDSelectCamera, NULL);
    GfuiAddKey(screen, GFUIK_F11,      "TV Director View",  (void*)9, SDSelectCamera, NULL);

    GfuiAddKey(screen, '1',            "Toggle HUD laptimeWidget",     (void*)"laptimeWidget",     SDToggleHUDwidget, NULL);
    GfuiAddKey(screen, '2',            "Toggle HUD carinfoWidget",     (void*)"carinfoWidget,carstatusWidget", SDToggleHUDwidgets, NULL);
    GfuiAddKey(screen, '3',            "Toggle HUD boardWidget",       (void*)"boardWidget",       SDToggleHUDwidget, NULL);
    GfuiAddKey(screen, '4',            "Toggle HUD driverinputWidget", (void*)0, SDToggleHUDdriverinput, NULL);
    GfuiAddKey(screen, '5',            "Toggle HUD deltaWidget",       (void*)"deltaWidget",       SDToggleHUDwidget, NULL);
    GfuiAddKey(screen, '6',            "Toggle HUD dashitemsWidget",   (void*)"dashitemsWidget",   SDToggleHUDwidget, NULL);
    GfuiAddKey(screen, '7',            "Toggle HUD raceinfoWidget",    (void*)"racepositionWidget,racelapsWidget", SDToggleHUDwidgets, NULL);
    GfuiAddKey(screen, '8',            "Toggle HUD",                   (void*)0, SDToggleHUD, NULL);

    GfuiAddKey(screen, '1', GFUIM_CTRL, "Toggle HUD textFPSWidget",     (void*)"textFPSWidget",     SDToggleHUDwidget, NULL, GFUI_HELP_RIGHT);
    GfuiAddKey(screen, '2', GFUIM_CTRL, "Toggle HUD tiresWidget",       (void*)"tiresWidget",       SDToggleHUDwidget, NULL, GFUI_HELP_RIGHT);
    GfuiAddKey(screen, '3', GFUIM_CTRL, "Toggle HUD gforceWidget",      (void*)"gforceWidget",      SDToggleHUDwidget, NULL, GFUI_HELP_RIGHT);
    GfuiAddKey(screen, '4', GFUIM_CTRL, "Toggle HUD graphFPSWidget",    (void*)"graphFPSWidget",    SDToggleHUDwidget, NULL, GFUI_HELP_RIGHT);
    GfuiAddKey(screen, '5', GFUIM_CTRL, "Toggle HUD graphSpeedWidget",  (void*)"graphSpeedWidget",  SDToggleHUDwidget, NULL, GFUI_HELP_RIGHT);
    GfuiAddKey(screen, '6', GFUIM_CTRL, "Toggle HUD graphFFBWidget",    (void*)"graphFFBWidget",    SDToggleHUDwidget, NULL, GFUI_HELP_RIGHT);
    GfuiAddKey(screen, '7', GFUIM_CTRL, "Toggle HUD graphInputsWidget", (void*)"graphInputsWidget", SDToggleHUDwidget, NULL, GFUI_HELP_RIGHT);

    /*GfuiAddKey(screen, '5',            "Debug Info",        (void*)3, grSelectBoard, NULL);
    GfuiAddKey(screen, '4',            "G/Cmd Graph",       (void*)4, grSelectBoard, NULL);
    GfuiAddKey(screen, '3',            "Leaders Board",     (void*)2, grSelectBoard, NULL);
    GfuiAddKey(screen, '2',            "Driver Counters",   (void*)1, grSelectBoard, NULL);
    GfuiAddKey(screen, '1',            "Driver Board",      (void*)0, grSelectBoard, NULL);*/
    GfuiAddKey(screen, '9',            "Mirror",            (void*)0, SDSwitchMirror, NULL);
    //GfuiAddKey(screen, '0',            "Arcade Board",      (void*)5, grSelectBoard, NULL);*/
    GfuiAddKey(screen, '+', GFUIM_CTRL, "Zoom In",           (void*)GR_ZOOM_IN,	 SDSetZoom, NULL);
    GfuiAddKey(screen, '=', GFUIM_CTRL, "Zoom In",           (void*)GR_ZOOM_IN,	 SDSetZoom, NULL);
    GfuiAddKey(screen, '-', GFUIM_CTRL, "Zoom Out",          (void*)GR_ZOOM_OUT, SDSetZoom, NULL);
    //GfuiAddKey(screen, '>',             "Zoom In",           (void*)GR_ZOOM_IN,	 SDSetZoom, NULL);
    //GfuiAddKey(screen, '<',             "Zoom Out",          (void*)GR_ZOOM_OUT, SDSetZoom, NULL);
    //GfuiAddKey(screen, '(',            "Split Screen",   (void*)SD_SPLIT_ADD, SDSplitScreen, NULL);
    //GfuiAddKey(screen, ')',            "UnSplit Screen", (void*)SD_SPLIT_REM, SDSplitScreen, NULL);
    //GfuiAddKey(screen, '_',            "Split Screen Arrangement", (void*)SD_SPLIT_ARR, SDSplitScreen, NULL);
    //GfuiAddKey(screen, GFUIK_TAB,      "Next (split) Screen", (void*)SD_NEXT_SCREEN, SDChangeScreen, NULL);
    /*GfuiAddKey(screen, 'm',            "Track Maps",          (void*)0, grSelectTrackMap, NULL);*/

    GfuiAddKey(screen, '+',        GFUIM_ALT,   "Move seat up",         (void*)1,  SDMoveSeatUpDown, NULL);
    GfuiAddKey(screen, '-',        GFUIM_ALT,   "Move seat down",       (void*)-1, SDMoveSeatUpDown, NULL);
    GfuiAddKey(screen, GFUIK_LEFT, GFUIM_ALT,   "Move seat left",       (void*)1,  SDMoveSeatLeftRight, NULL);
    GfuiAddKey(screen, GFUIK_RIGHT,GFUIM_ALT,   "Move seat right",      (void*)-1, SDMoveSeatLeftRight, NULL);
    GfuiAddKey(screen, GFUIK_UP,   GFUIM_ALT,   "Move seat forward",    (void*)1,  SDMoveSeatForwardBackward, NULL);
    GfuiAddKey(screen, GFUIK_DOWN, GFUIM_ALT,   "Move seat backward",   (void*)-1, SDMoveSeatForwardBackward, NULL);

    if(GfScrUsingResizableWindow())
        GfuiAddKey(screen, GFUIK_RETURN, GFUIM_ALT, "Toggle Full-screen", (void*)0, GfScrToggleFullScreen, NULL);

    GfLogInfo("Current screen is #%d (out of %d)\n", m_CurrentScreenIndex, m_NbActiveScreens);

    return 0; // true;
}

void adaptScreenSize()
{
    // TODO need to resize 'screens'
    //GfScrGetSize(&grWinx, &grWiny, &grWinw, &grWinh);
}

int refresh(tSituation *s)
{
    // Compute F/S indicators every second.
    frameInfo.nInstFrames++;
    frameInfo.nTotalFrames++;
    const double dCurTime = GfTimeClock();
    const double dDeltaTime = dCurTime - fFPSPrevInstTime;

    if (dDeltaTime > 1.0)
    {
        ++nFPSTotalSeconds;
        fFPSPrevInstTime = dCurTime;
        frameInfo.fInstFps = frameInfo.nInstFrames / dDeltaTime;
        frameInfo.nInstFrames = 0;
        frameInfo.fAvgFps = (double)frameInfo.nTotalFrames / nFPSTotalSeconds;

        // Trace F/S every 5 seconds.
        if (nFPSTotalSeconds % 5 == 2)
            GfLogDebug("Frame rate (F/s) : Instant = %.1f (Average %.1f)\n",
                      frameInfo.fInstFps, frameInfo.fAvgFps);
    }
    adaptScreenSize();
    cam = screens->getActiveView()->getCameras()->getSelectedCamera();
    osg::Vec3d eye = cam->getCameraPosition();
    double X = eye[0];
    double Y = eye[1];
    double Z = eye[2];
    render->UpdateSky(s->currentTime, s->accelTime, X, Y);

    tCarElt* curCar = screens->getActiveView()->getCurrentCar();
    int drawDriver = cam->getDrawDriver();
    int drawCurrent = cam->getDrawCurrent();
    cars->updateCars(s, curCar, drawCurrent, drawDriver);

    scenery->reposition(X, Y, Z );
    scenery->update_tracklights(s->currentTime, s->_totTime, s->_raceType);

    screens->update(s, &frameInfo, osg::Vec4f(render->getFogColor(),1.0f));

    //refresh the hud
    //tCarElt* curCar = screens->getActiveView()->getCurrentCar();
    hud.Refresh(s, &frameInfo, curCar);

    return 0;
}

void shutdownCars(void)
{
    if (cars)
    {
        cars->unLoad();
        delete cars;
        cars = NULL;
        GfLogInfo("Delete cars in OsgMain\n");
    }

    if (carLights)
    {
        delete carLights;
        carLights = NULL;
        GfLogInfo("Delete carLights in OsgMain\n");
    }

    // Trace final mean F/s.
    if (nFPSTotalSeconds > 0)
        GfLogTrace("Average frame rate: %.2f F/s\n",
                   (double)frameInfo.nTotalFrames/((double)nFPSTotalSeconds + GfTimeClock() - fFPSPrevInstTime));
}

int initTrack(tTrack *track)
{
    // The inittrack does as well init the context, that is highly inconsistent, IMHO.
    // TODO: Find a solution to init the graphics first independent of objects.

    // Now, do the real track loading job.
    grTrackHandle = GfParmReadFile(track->filename, GFPARM_RMODE_STD | GFPARM_RMODE_CREAT);

    //Options = new SDOptions;

    scenery = new SDScenery;
    render = new SDRender;
    //cam = new SDCamera;

    scenery->LoadScene(track);
    render->Init(track);

    return 0;
}

int  initCars(tSituation *s)
{
    GfLogInfo("InitCars\n");
    carLights = new SDCarLights;
    cars = new SDCars;
    carLights->loadStates();
    cars->loadCars(s, scenery->getSpeedWay(), scenery->getSpeedWayLong());
    render->addCars(cars->getCarsNode(), carLights->getLightsRoot());
    GfLogInfo("All cars loaded\n");

    screens->InitCars(s);

    if (!grHandle)
    {
        grHandle = GfParmReadFileLocal(GR_PARAM_FILE, GFPARM_RMODE_STD | GFPARM_RMODE_CREAT);
    }

    return 0;
}

void shutdownTrack(void)
{
    // Do the real track termination job.
    osgDB::Registry::instance()->clearObjectCache();

    if (grTrackHandle)
    {
        GfParmReleaseHandle(grTrackHandle);
        grTrackHandle = 0;
    }

    if (scenery)
    {
        scenery->ShutdownScene();
        delete scenery;
        scenery = NULL;
        GfLogInfo("Delete scenery in OsgMain\n");
    }
}

void shutdownView(void)
{
    if (screens)
    {
        delete screens;
        screens = NULL;
        GfLogInfo("Delete screens in OsgMain\n");
    }

    if (render)
    {
        delete render;
        render = NULL;
        GfLogInfo("Delete render in OsgMain\n");
    }
}

Camera * getCamera(void)
{
    return screens->getActiveView()->getCamera();
}
