/***************************************************************************

    file                 : grmain.h
    created              : Fri Aug 18 00:00:41 CEST 2000
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

#ifndef _GRMAIN_H_
#define _GRMAIN_H_

#include <plib/ssg.h>	//ssgContect
#include <raceman.h>	//tSituation

#ifdef WIN32
#include <windows.h>
#include <GL/gl.h>
#include <GL/glext.h>
#endif


#ifdef WIN32
////// Multitexturing Info
extern PFNGLMULTITEXCOORD2FARBPROC glMultiTexCoord2fARB ;
extern PFNGLMULTITEXCOORD2FVARBPROC glMultiTexCoord2fvARB;
extern PFNGLACTIVETEXTUREARBPROC   glActiveTextureARB ;
extern PFNGLCLIENTACTIVETEXTUREARBPROC glClientActiveTextureARB ;
#endif

extern bool grInitMultiTex();

extern int grWinx, grWiny, grWinw, grWinh;
extern int grVectFlag;
extern int grVectDispFlag[];

extern double grCurTime;

extern void *grHandle;
extern void *grTrackHandle;

extern ssgContext grContext;
extern int grNbCars;

extern int  initView(int x, int y, int width, int height, int flag, void *screen);
extern int  initCars(tSituation *s);
extern int  refresh(tSituation *s);
extern void shutdownCars(void);
extern int  initTrack(tTrack *track);
extern void shutdownTrack(void);
//extern void bendCar (int index, sgVec3 poc, sgVec3 force, int cnt);

extern int grMaxTextureUnits;
extern tdble grMaxDammage;

// Number of active screens.
extern int grNbActiveScreens;

extern class cGrScreen *grScreens[];
extern class cGrScreen* grGetCurrentScreen(void);


#define GR_SPLIT_ADD	0
#define GR_SPLIT_REM	1

#define GR_NEXT_SCREEN	0
#define GR_PREV_SCREEN	1

#define GR_NB_MAX_SCREEN 4

extern tdble grLodFactorValue;

#endif /* _GRMAIN_H_ */ 
