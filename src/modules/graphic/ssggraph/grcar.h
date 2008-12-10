
/***************************************************************************

    file                 : grcar.h
    created              : Mon Aug 21 18:21:15 CEST 2000
    copyright            : (C) 2000 by Eric Espie
    email                : torcs@free.fr
    version              : $Id: grcar.h,v 1.19 2005/02/01 19:08:18 berniw Exp $

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

 
#ifndef _GRCAR_H_
#define _GRCAR_H_

typedef struct
{
    ssgSimpleState	*texture;
    GLuint		CounterList;
    GLuint		needleList;
    tdble		needleXCenter, needleYCenter;
    tdble		digitXCenter, digitYCenter;
    tdble		minValue, maxValue;
    tdble		minAngle, maxAngle;
    tdble		*monitored;
    tdble		prevVal;
    tdble		rawPrev;
    int			digital;
} tgrCarInstrument;


typedef struct 
{
    float		iconColor[4];
    ssgTransform	*carTransform;
    ssgSelector		*LODSelector;
    ssgSelector		*DRMSelector;
    ssgEntity		*carEntity;
    int                 nSteer;
    int			LODSelectMask[32];
    float		LODThreshold[32];
    int			DRMSelectMask[32];
    int                 nDRM;
    float		DRMThreshold[32];
    float               lastDRMswitch;
    ssgSelector		*driverSelector;
    ssgSelector         *steerSelector;
    bool driverSelectorinsg;
    ssgStateSelector	*envSelector;
    ssgTransform	*wheelPos[4];
    ssgTransform	*wheelRot[4];
    ssgTransform	*steerRot[2];
    ssgColourArray	*brkColor[4];
    ssgSelector		*wheelselector[4];
    ssgState		*wheelTexture;
    ssgVtxTableShadow	*shadowBase;
    ssgVtxTableShadow	*shadowCurr;
    ssgBranch		*shadowAnchor;
    tgrSkidmarks        *skidmarks;
    sgMat4		carPos;
    tgrCarInstrument	instrument[2];
    tdble               distFromStart;
    tdble               envAngle;
    int			fireCount;
    tdble               steerMovt;
    tdble               px;
    tdble               py;
    tdble               sx;
    tdble               sy;
} tgrCarInfo;

extern tgrCarInfo	*grCarInfo;

extern void grInitCar(tCarElt *car);
extern void grDrawCar(tSituation *, tCarElt*, tCarElt *, int, int, double curTime, class cGrPerspCamera *curCam);
extern void grDrawShadow(tCarElt *car);
extern void grInitShadow(tCarElt *car);
extern tdble grGetDistToStart(tCarElt *car);
extern void grInitCommonState(void);
extern void grPropagateDamage (ssgEntity* l, sgVec3 poc, sgVec3 force, int cnt);
#endif /* _GRCAR_H_ */ 



