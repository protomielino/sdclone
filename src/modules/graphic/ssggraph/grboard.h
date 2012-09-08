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
class cGrFrameInfo;

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
    void grDispDebug(const tSituation *s, const tCarElt *car, const cGrFrameInfo* frame);
    void grDispGGraph(tCarElt *car);
    void grDispMisc(bool bCurrentScreen);
    void grDrawGauge(tdble X1, tdble Y1, tdble H, float *clr1, float *clr2, tdble val, const char *title);

    void grDispCarBoard(const tCarElt *car, const tSituation *s);
    void grDispCarBoard1(const tCarElt *car, const tSituation *s);
    void grDispCarBoard2(const tCarElt *car, const tSituation *s);

    void grDispCounterBoard(tCarElt *car);

    void grDispLeaderBoard(const tCarElt *car, const tSituation *s);
    void grDispCounterBoard2(tCarElt *car);
    void grDispLeaderBoardScroll(const tCarElt *car, const tSituation *s) const;
    void grDispLeaderBoardScrollLine(const tCarElt *car, const tSituation *s);

    void grDispArcade(tCarElt *car, tSituation *s);
    std::string grGenerateLeaderBoardEntry(const tCarElt *car, const tSituation *s, const bool isLeader) const;
    // Track overview object
    cGrTrackMap *trackMap;

    bool grGetSplitTime(const tSituation *s, const tCarElt *car, bool gap_inrace, double &time, int *laps_different, float **color);
    void grGetLapsTime(const tSituation *s, const tCarElt *car, char* result, char const** label) const;
    void grMakeThreeLetterNames(const tSituation *s);
    void grSetupDrawingArea(int xl, int yb, int xr, int yt) const;
		
 public:
    cGrBoard(int myid);
    ~cGrBoard();

    void initBoard(void);
    void shutdown(void);
    void selectBoard(int brd);
    void setWidth(int width);
    void initBoardCar(tCarElt *car);
    inline cGrTrackMap *getTrackMap() { return trackMap; }

    void refreshBoard(tSituation *s, const cGrFrameInfo* frameInfo,
					  bool forceArcade, tCarElt *currCar, bool isCurrScreen);
    void loadDefaults(tCarElt *curCar);
};

extern void grInitBoardCar(tCarElt *car);
extern void grShutdownBoardCar(void);

#endif /* _GRBOARD_H_ */ 
