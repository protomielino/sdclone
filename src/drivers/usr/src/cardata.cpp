/***************************************************************************

    file                 : cardata.cpp
    created              : Thu Sep 23 12:31:37 CET 2004
    copyright            : (C) 2004 Bernhard Wymann
    email                : berniw@bluewin.ch
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

#include "cardata.h"


void SingleCardata::update()
{
    trackangle = RtTrackSideTgAngleL(&(car->_trkPos));
    speed = getSpeed(car, trackangle);
    //evalTrueSpeed();
    angle = trackangle - car->_yaw;
    FLOAT_NORM_PI_PI(angle);
    width = MAX(car->_dimension_y, fabs(car->_dimension_x*sin(angle) + car->_dimension_y*cos(angle))) + 0.1f;
    length = MAX(car->_dimension_x, fabs(car->_dimension_y*sin(angle) + car->_dimension_x*cos(angle))) + 0.1f;

    for (int i=0; i<4; i++)
    {
        corner2[i].ax = corner1[i].ax;
        corner2[i].ay = corner1[i].ay;
        corner1[i].ax = car->_corner_x(i);
        corner1[i].ay = car->_corner_y(i);
    }

    t_m_f = MIN(car->priv.wheel[0].condition, car->priv.wheel[1].condition);
    t_m_r = MIN(car->priv.wheel[2].condition, car->priv.wheel[3].condition);
    TyreMu = MIN(t_m * t_m_f, t_m * t_m_r);

    evalTrueSpeed();

    lastspeed[2].ax = lastspeed[1].ax;
    lastspeed[2].ay = lastspeed[1].ay;
    lastspeed[1].ax = lastspeed[0].ax;
    lastspeed[1].ay = lastspeed[0].ay;
    lastspeed[0].ax = car->_speed_X;
    lastspeed[0].ay = car->_speed_Y;
}

void SingleCardata::updateWalls()
{
    tolftwall = torgtwall = 1000.0f;

    tTrackSeg *lseg = car->_trkPos.seg->lside;
    tTrackSeg *rseg = car->_trkPos.seg->rside;

    // get wall/fence segments on each side
    if (lseg)
        while (lseg->style == TR_PLAN || lseg->style == TR_CURB)
        {
            if (!lseg->lside) break;
            lseg = lseg->lside;
        }

    if (rseg)
        while (rseg->style == TR_PLAN && rseg->style == TR_CURB)
        {
            if (!rseg->rside) break;
            rseg = rseg->rside;
        }

    if (lseg && rseg)
    {
        // make a line along the wall
        straight2f lftWallLine( lseg->vertex[TR_SL].x, lseg->vertex[TR_SL].y,
                                lseg->vertex[TR_EL].x - lseg->vertex[TR_SL].x,
                                lseg->vertex[TR_EL].y - lseg->vertex[TR_SL].y);
        straight2f rgtWallLine( rseg->vertex[TR_SR].x, rseg->vertex[TR_SR].y,
                                rseg->vertex[TR_EL].x - rseg->vertex[TR_SL].x,
                                rseg->vertex[TR_EL].y - rseg->vertex[TR_SL].y);

        for (int i=0; i<4; i++)
        {
            // get minimum distance to each wall
            vec2f corner(car->_corner_x(i), car->_corner_y(i));
            tolftwall = MIN(tolftwall, lftWallLine.dist( corner ));
            torgtwall = MIN(torgtwall, rgtWallLine.dist( corner ));
        }
    }
    else
    {
        tolftwall = car->_trkPos.toLeft;
        torgtwall = car->_trkPos.toRight;
    }
}


static double getDistance2D( double x1, double y1, double x2, double y2 )
{
    double dx = x1 - x2;
    double dy = y1 - y2;

    return sqrt(dx*dx + dy*dy);
}

void SingleCardata::evalTrueSpeed()
{
    tTrackSeg *seg = car->_trkPos.seg;
    truespeed = speed;

    if (seg->type == TR_STR)
        return;

    double lengthlft = getDistance2D( seg->vertex[TR_SL].x, seg->vertex[TR_SL].y, seg->vertex[TR_EL].x, seg->vertex[TR_EL].y );
    double lengthrgt = getDistance2D( seg->vertex[TR_SR].x, seg->vertex[TR_SR].y, seg->vertex[TR_ER].x, seg->vertex[TR_ER].y );

    double ratio;
    if (seg->type == TR_LFT)
        ratio = MAX(0.0, MIN(1.0, car->_trkPos.toLeft / (seg->width-3.0)));
    else
        ratio = MAX(0.0, MIN(1.0, (1.0 - car->_trkPos.toRight / (seg->width-3.0))));

    double ourlength = lengthlft * ratio + lengthrgt * (1.0-ratio);
    double avglength = lengthlft/2 + lengthrgt/2;
    double change = MIN(1.0, MAX(0.85, ourlength / avglength));

    truespeed *= (tdble) change;
    truespeed *= TyreMu;
}

// compute speed component parallel to the track.
float SingleCardata::getSpeed(tCarElt *car, float ltrackangle)
{
    v2d speed, dir;
    speed.x = car->_speed_X;
    speed.y = car->_speed_Y;
    dir.x = cos(ltrackangle);
    dir.y = sin(ltrackangle);

    return (tdble) (speed*dir);
}

void SingleCardata::init( CarElt *pcar )
{
    car = pcar;
    for (int i=0; i<4; i++)
    {
        corner1[i].ax = corner2[i].ax = car->_corner_x(i);
        corner1[i].ay = corner2[i].ay = car->_corner_y(i);
    }
    lastspeed[0].ax = lastspeed[1].ax = lastspeed[2].ax = car->_speed_X;
    lastspeed[0].ay = lastspeed[1].ay = lastspeed[2].ay = car->_speed_Y;

    t_m = t_m_f = t_m_r = 9999999.0f;
    //lTT = GfParmGetNum(car->_carHandle, SECT_PRIVATE, PRV_WTT, (char *)NULL, BT_ATT_WT);
    //hTT = GfParmGetNum(car->_carHandle, SECT_PRIVATE, PRV_HTT, (char *)NULL, iT() + (iT() - lTT)*0.80);
    fuelMassFactor = GfParmGetNum(car->_carHandle, SECT_PRIVATE, PRV_FUEL_MASS_FACTOR, (char *)NULL, 1.0f);

    RH = 0.0f;

    for (int i=0; i<4; i++)
    {
        static const char *WheelSect[4] = {SECT_FRNTRGTWHEEL, SECT_FRNTLFTWHEEL,
                                           SECT_REARRGTWHEEL, SECT_REARLFTWHEEL};
        double mu = GfParmGetNum(car->_carHandle, WheelSect[i], PRM_MU, (char *)NULL, 1.0);
        t_m = MIN(t_m, mu);

        RH += GfParmGetNum(car->_carHandle, WheelSect[i], (char *)PRM_RIDEHEIGHT, (char *)NULL, 0.20f);
    }

    t_m_f = MIN(car->priv.wheel[0].condition, car->priv.wheel[1].condition);
    t_m_r = MIN(car->priv.wheel[2].condition, car->priv.wheel[3].condition);
    TyreMu = MIN(t_m * t_m_f, t_m * t_m_r);

    // Aerodynamics
    RH *= 1.5f; RH = RH*RH; RH = 2.0 * exp(-3.0 * RH);
    tdble CL = GfParmGetNum(car->_carHandle, SECT_AERODYNAMICS, PRM_FCL, (char *)NULL, 0.0f) +
            GfParmGetNum(car->_carHandle, SECT_AERODYNAMICS, PRM_RCL, (char *)NULL, 0.0f);
    float fwingarea = GfParmGetNum(car->_carHandle, SECT_FRNTWING, PRM_WINGAREA, (char*) NULL, 0.0f);
    float fwingangle = GfParmGetNum(car->_carHandle, SECT_FRNTWING, PRM_WINGANGLE, (char*) NULL, 0.0f);
    float rwingarea = GfParmGetNum(car->_carHandle, SECT_REARWING, PRM_WINGAREA, (char*) NULL, 0.0f);
    float rwingangle = GfParmGetNum(car->_carHandle, SECT_REARWING, PRM_WINGANGLE, (char*) NULL, 0.0f);
    float rwingArea = rwingarea * sin(rwingangle);
    float fwingArea = fwingarea * sin(fwingangle);
    float wingCA = 1.23f * (fwingArea + rwingArea);
    CA = RH * CL + 4.0f * wingCA;
    CA_FW = 4 * 1.23f * fwingArea;
    CA_RW = 4 * 1.23f * rwingArea;
    CA_GE = RH * CL;

    baseMass = GfParmGetNum(car->_carHandle, SECT_CAR, PRM_MASS, NULL, 1000.0f);
    fuel = 0.0f;
    //mFTT = 0.0f;
    //aFTT = 0.0f;
    //aTT = 0.0f;
    carMu = fuelCarMu = offlineFuelCarMu = 1.0f;
    lmTT = rmTT = 0.0;

    baseCarMu = (CA_FW * t_m_f + CA_RW * t_m_r + CA_GE * t_m) / baseMass;

    CTFactor = GfParmGetNum(car->_carHandle, SECT_PRIVATE, PRV_CTFACTOR, NULL, 1.0f);
}

void SingleCardata::updateModel()
{
    //aTT = 0.0f; aFTT = 0.0f; mFTT = 0.0f;
    float mLTT = 0.0f, mRTT = 0.0f;
    tdble mG = 0.0f;
    int i;

    /*for (i=0; i<4; i++)
    {
        double ct = car->priv.wheel[i].condition;

        aTT += ct;
        mG = MAX(mG, ct);
        if (i < 2)
        {
            aFTT += ct;
            mFTT = MAX(mFTT, ct);

            if (i == FRNT_LFT || i == REAR_LFT)
                mLTT = MAX(mLTT, ct);
            else
                mRTT = MAX(mRTT, ct);
        }
    }*/

    //aTT /= 4;
    //aFTT /= 2;
    fuel = car->_fuel;
    damage = car->_dammage;

    tdble cTM = t_m, cTMF = t_m_f, cTMR = t_m_r;
    for (i=0; i<4; i++)
    {
        double ct = car->priv.wheel[i].condition;
        tdble gF = 1.0f - /*mG/10;*/ct;

        //if (ct > hTT)
        //	gF -= MIN(0.20f, ((ct - hTT) / 15.0f) / 5.0f);
        /*if (ct < lTT)
            gF -= MIN(0.75f, ((lTT - ct) / (45.0f*CTFactor)) / 15.0f);*/

        if (i < 2)
            cTMF = MIN(cTMF, t_m_f * ct);
        else
            cTMR = MIN(cTMR, t_m_r * ct);
    }

    //lftOH = MAX(0.0, MAX(lmTT, mLTT) - hTT);
    //rgtOH = MAX(0.0, MAX(rmTT, mRTT) - hTT);
    lmTT = rmTT = 0.0;

    cTM = MIN(cTMF, cTMR);

    fuelCarMu = ((CA_FW * t_m_f + CA_RW * t_m_r + CA_GE * t_m) / (baseMass+fuel * fuelMassFactor)) / baseCarMu;
    offlineFuelCarMu = ((CA_FW * t_m_f + CA_RW * t_m_r + CA_GE * t_m) / (baseMass+fuel)) / baseCarMu;
    carMu = ((CA_FW * cTMF + CA_RW * cTMR + CA_GE * cTM) / (baseMass+fuel * fuelMassFactor)) / baseCarMu;
}

Cardata::Cardata(tSituation *s)
{
    ncars = s->_ncars;
    data = new SingleCardata[ncars];
    int i;

    for (i = 0; i < ncars; i++)
    {
        data[i].init(s->cars[i]);
    }
}

Cardata::~Cardata()
{
    delete [] data;
}

void Cardata::update()
{
    int i;
    for (i = 0; i < ncars; i++)
    {
        data[i].update();
    }
}

SingleCardata *Cardata::findCar(tCarElt *car)
{
    int i;

    for (i = 0; i < ncars; i++)
    {
        if (data[i].thisCar(car))
        {
            return &data[i];
        }
    }
    return NULL;
}
