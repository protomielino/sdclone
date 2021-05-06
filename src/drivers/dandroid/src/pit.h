/***************************************************************************

    file                 : pit.h
    created              : Thu Aug 31 01:21:49 UTC 2006
    copyright            : (C) 2006 Daniel Schellhammer

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _PIT_H_
#define _PIT_H_

#include "torcs_or_sd.h"
#include "globaldefinitions.h"

#ifdef DANDROID_TORCS
#include <track.h>
#include <car.h>
#include <raceman.h>
#include <robottools.h>
#endif

#include "spline.h"

#define NPOINTS  7 // pitting
#define DTPOINTS 4 // drive through

class Pit
{
public:
    Pit();
    ~Pit();

    void   init(PTrack t, PSituation s, PtCarElt c, int pitdamage, double pitentrymargin, bool HASTYC);
    void   setPitstop(bool pitstop);
    bool   getPitstop() { return pitstop; }
    bool   getPenalty() { return penalty; }
    void   setInPit(bool inpitl) { inpitlane = inpitl; }
    bool   getInPit() { return inpitlane; }
    double getPitOffset(double fromstart);
    bool   isBetween(double fromstart);
    bool   isPitlimit(double fromstart);
    double getPitEntry() { return pitentry; }
    double getLimitEntry() { return limitentry; }
    double getLimitExit() { return limitexit; }
    double getNPitStart() { return p[1].x; }
    double getNPitLoc() { return p[3].x; }
    double getNPitEnd() { return p[5].x; }
    double toSplineCoord(double x);
    void   update(double fromstart, double tyre);
    int    getRepair();
    bool   pitForPenalty();
    double getFuel();
    double getSpeedlimit();
    double getSpeedlimit(double fromstart);
    double getDist();
    double getSideDist();
    void   pitCommand();
    double avgWearPerMeter() const { return mAvgWearPerMeter; }


public:
    PTrack track;
    PtCarElt car;
    PtCarElt teamcar;
    tTrackOwnPit* mypit;     /* pointer to my pit */
    tTrackPitInfo* pitinfo;  /* general pit info */
    SplinePoint p[NPOINTS];  /* spline points for pitting */
    Spline spline;           /* spline  for pitting*/
    SplinePoint dt[DTPOINTS];/* spline for drive throught */
    Spline dtspline;         /* spline for drive throught */
    bool pitstop;            /* pitstop planned */
    bool penalty;
    bool pittyres;
    bool inpitlane;          /* we are still in the pit lane */
    double mFromStart;
    double pitentry;         /* distance to start line of the pit entry */
    double pitexit;          /* distance to the start line of the pit exit */
    double limitentry;       /* distance to start line of the pit entry */
    double limitexit;        /* distance to the start line of the pit exit */
    double speedlimit;       /* pit speed limit */
    bool fuelchecked;        /* fuel statistics updated */
    double lastfuel;         /* the fuel available when we cross the start lane */
    double lastpitfuel;      /* amount refueled, special case when we refuel */
    double maxfuelperlap;    /* the maximum amount of fuel we needed for a lap */
    double totalfuel;        /* the total amount of fuel we needed for the race */
    int fuellapscounted;     /* the total laps we counted, maybe we miss a lap */
    double avgfuelperlap;    /* the average amount of fuel we needed for a lap */
    double mAvgWearPerMeter; /* the average amount of tyre we needed for a lapv*/
    double mPitGripFactor;
    int PIT_DAMAGE;
    int MAX_DAMAGE;
    int MAX_DAMAGE_DIST;
    double ENTRY_MARGIN;
    double SPEED_LIMIT_MARGIN;
    double mHASTYC;
};

#endif // _PIT_H_
