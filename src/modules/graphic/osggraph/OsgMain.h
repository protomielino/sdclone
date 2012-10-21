/***************************************************************************

    file                     : OsgMain.h
    created                  : Fri Aug 18 00:00:41 CEST 2012
    copyright                : (C) 2012 by Xavier Bertaux
    email                    : bertauxx@yahoo.fr
    version                  : $Id: OsgMain.h 4693 2012-04-13 03:12:09Z torcs-ng $

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _OSGGRMAIN_H_
#define _OSGMAIN_H_

#include <osgViewer/Viewer>
#include <osg/Camera>

#include <graphic.h>
#include <raceman.h>	//tSituation

//#include "grcam.h"



extern int grWinx, grWiny, grWinw, grWinh;
extern int grVectFlag;
extern int grVectDispFlag[];

extern double grCurTime;

extern void *grHandle;
extern void *grTrackHandle;

extern int grNbCars;

extern int  initTrack(tTrack *track);
extern int  initCars(tSituation *s);
#define GR_VIEW_STD  0 /* full screen view mode */
#define GR_VIEW_PART 1 /* partial screen view mode (scissor test) */
extern int  initView(int x, int y, int width, int height, int mode, void *screen);
extern int  refresh(tSituation *s);
extern void shutdownCars(void);
extern void shutdownTrack(void);
extern void shutdownView(void);
//extern cGrCamera * grGetCurCamera(void);
//extern void bendCar (int index, sgVec3 poc, sgVec3 force, int cnt);

extern int grMaxTextureUnits;
extern tdble grMaxDammage;

// Number of active screens.
extern int grNbActiveScreens;
//extern osg::ref_ptr<osgViewer::Viewer> m_sceneViewer;

//extern class cGrScreen *grScreens[];
//extern class cGrScreen* grGetCurrentScreen(void);
extern osg::ref_ptr<osgViewer::Viewer> m_sceneViewer;
extern	osg::Timer m_timer;
extern	osg::Timer_t m_start_tick;

//static 

#define GR_SPLIT_ADD	0
#define GR_SPLIT_REM	1
#define GR_SPLIT_ARR	2

#define GR_NEXT_SCREEN	0
#define GR_PREV_SCREEN	1

#define GR_NB_MAX_SCREEN 6

extern tdble grLodFactorValue;

class cGrFrameInfo
{
 public:
	double fInstFps;        // "Instant" frame rate (average along a 1 second shifting window).
	double fAvgFps;         // Average frame rate (since the beginning of the race).
	unsigned nInstFrames;   // Nb of frames since the last "instant" FPS refresh time
	unsigned nTotalFrames;  // Total nb of frames since initView
};

#endif /* _OSGMAIN_H_ */ 
