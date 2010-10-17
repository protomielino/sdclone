/***************************************************************************

    file                 : grboard.h
    created              : Thu Aug 17 23:55:47 CEST 2000
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

#ifndef _GRBOARD_H_
#define _GRBOARD_H_

#include <car.h>			//tCarElt
#include <raceman.h>	//tSituation

class cGrTrackMap;

#include <string>
#include <vector>

class cGrBoard
{
 protected:
    int	id;		/* Board Id */

    int	boardFlag;
    int leaderFlag;
    int debugFlag;
    int leaderNb;
    int counterFlag;
    int GFlag;
    int arcadeFlag;
    int boardWidth;
    int leftAnchor;
    int centerAnchor;
    int rightAnchor;
    int speedoRise;
    std::vector<std::string> sShortNames;
		
 private:
    void grDispDebug(float instFps, float avgFps, tCarElt *car);
    void grDispGGraph(tCarElt *car);
    void grDispCarBoard1(tCarElt *car, tSituation *s);
    void grDispMisc(bool bCurrentScreen);
    void grDrawGauge(tdble X1, tdble Y1, tdble H, float *clr1, float *clr2, tdble val, const char *title);
    void grDispCarBoard2(tCarElt *car, tSituation *s);
    void grDispCarBoard(tCarElt *car, tSituation *s);
    void grDispCounterBoard(tCarElt *car);
    void grDispLeaderBoard(const tCarElt *car, const tSituation *s);
    void grDispLeaderBoardScroll(const tCarElt *car, const tSituation *s) const;
    void grDispLeaderBoardScrollLine(const tCarElt *car, const tSituation *s);
    void grDispCounterBoard2(tCarElt *car);
    void grDispArcade(tCarElt *car, tSituation *s);
    std::string grGenerateLeaderBoardEntry(const tCarElt *car, const tSituation *s, const bool isLeader) const;
    // Track overview object
    cGrTrackMap *trackMap;

    bool grGetSplitTime(tSituation *s, tCarElt *car, bool gap_inrace, double &time, int *laps_different, float **color);
    void grGetLapsTime(tSituation *s, tCarElt *car, char* result, char const** label) const;
    void grMakeThreeLetterNames(const tSituation *s);
		
 public:
    cGrBoard(int myid);
    ~cGrBoard();

    void initBoard(void);
    void shutdown(void);
    void selectBoard(int brd);
    void setWidth(int width);
    void initBoardCar(tCarElt *car);
    inline cGrTrackMap *getTrackMap() { return trackMap; }

    void refreshBoard(tSituation *s, float instFps, float avgFps,
					  bool forceArcade, tCarElt *currCar, bool isCurrScreen);
    void loadDefaults(tCarElt *curCar);
};

extern void grInitBoardCar(tCarElt *car);
extern void grShutdownBoardCar(void);

#endif /* _GRBOARD_H_ */ 
