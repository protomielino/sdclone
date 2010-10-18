/***************************************************************************

    file                 : grboard.cpp
    created              : Thu Aug 17 23:52:20 CEST 2000
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

#include <algorithm>  //remove
#include <sstream>
#include <iostream>

#include <plib/ssg.h>
#include <glfeatures.h>

#include <robottools.h> //RELAXATION
#include <portability.h> // snprintf

#include "grboard.h"
#include "grmain.h" //grWinX, grHandle, grMaxDamage
#include "grtrackmap.h" //cGrTrackMap
#include "grcar.h"  //grCarInfo
#include "grutil.h" //grWriteTime
#include "grloadac.h" //grssgSetCurrentOptions

using namespace std;

static float grWhite[4] = {1.0, 1.0, 1.0, 1.0};
static float grRed[4] = {1.0, 0.0, 0.0, 1.0};
static float grBlue[4] = {0.0, 0.0, 1.0, 1.0};
static float grGreen[4] = {0.0, 1.0, 0.0, 1.0};
static float grBlack[4] = {0.0, 0.0, 0.0, 1.0};
static float grPink[4] = {1.0, 0.0, 1.0, 1.0};
static float grDefaultClr[4] = {0.9, 0.9, 0.15, 1.0};

#define NB_BOARDS 3
#define NB_LBOARDS  5 //# of leaderboard states
#define NB_DEBUG  3

// Boards work on a OrthoCam with fixed height of 600, width flows
// with split screen(s) and can be limited to 'board width' % of screen
#define TOP_ANCHOR	600
#define BOTTOM_ANCHOR	0
#define DEFAULT_WIDTH	800

static char path[1024];

//Scrolling leaderboard variables
static int iStart = 0;
static double iTimer = 0.0;
static int iStringStart = 0;
static string st; //This is the line we will display in the bottom
  

cGrBoard::cGrBoard (int myid) {
  id = myid;
  trackMap = NULL;
}


cGrBoard::~cGrBoard () {
  if(trackMap != NULL)
    delete trackMap;
  trackMap = NULL;
}


void
cGrBoard::loadDefaults(tCarElt *curCar)
{
  snprintf (path, sizeof(path), "%s/%d", GR_SCT_DISPMODE, id);
  
  debugFlag = (int)GfParmGetNum(grHandle, path, GR_ATT_DEBUG, NULL, 1);
  boardFlag = (int)GfParmGetNum(grHandle, path, GR_ATT_BOARD, NULL, 2);
  leaderFlag  = (int)GfParmGetNum(grHandle, path, GR_ATT_LEADER, NULL, 1);
  leaderNb  = (int)GfParmGetNum(grHandle, path, GR_ATT_NBLEADER, NULL, 10);
  counterFlag = (int)GfParmGetNum(grHandle, path, GR_ATT_COUNTER, NULL, 1);
  GFlag = (int)GfParmGetNum(grHandle, path, GR_ATT_GGRAPH, NULL, 1);
  arcadeFlag  = (int)GfParmGetNum(grHandle, path, GR_ATT_ARCADE, NULL, 0);
  boardWidth  = (int)GfParmGetNum(grHandle, path, GR_ATT_BOARDWIDTH, NULL, 100);
  speedoRise  = (int)GfParmGetNum(grHandle, path, GR_ATT_SPEEDORISE, NULL, 0);
  
  trackMap->setViewMode((int) GfParmGetNum(grHandle, path, GR_ATT_MAP, NULL, trackMap->getDefaultViewMode()));
  
  if (curCar->_driverType == RM_DRV_HUMAN) {
    snprintf(path, sizeof(path), "%s/%s", GR_SCT_DISPMODE, curCar->_name);
    debugFlag = (int)GfParmGetNum(grHandle, path, GR_ATT_DEBUG, NULL, debugFlag);
    boardFlag = (int)GfParmGetNum(grHandle, path, GR_ATT_BOARD, NULL, boardFlag);
    leaderFlag  = (int)GfParmGetNum(grHandle, path, GR_ATT_LEADER, NULL, leaderFlag);
    leaderNb  = (int)GfParmGetNum(grHandle, path, GR_ATT_NBLEADER, NULL, leaderNb);
    counterFlag   = (int)GfParmGetNum(grHandle, path, GR_ATT_COUNTER, NULL, counterFlag);
    GFlag   = (int)GfParmGetNum(grHandle, path, GR_ATT_GGRAPH, NULL, GFlag);
    arcadeFlag  = (int)GfParmGetNum(grHandle, path, GR_ATT_ARCADE, NULL, arcadeFlag);
    boardWidth  = (int)GfParmGetNum(grHandle, path, GR_ATT_BOARDWIDTH, NULL, boardWidth);
    speedoRise  = (int)GfParmGetNum(grHandle, path, GR_ATT_SPEEDORISE, NULL, speedoRise);
    trackMap->setViewMode((int) GfParmGetNum(grHandle, path, GR_ATT_MAP, NULL, trackMap->getViewMode()));
  }

  if (boardWidth < 0 || boardWidth > 100)
	  boardWidth = 100;
  this->setWidth(DEFAULT_WIDTH);

  if (speedoRise < 0 || speedoRise > 100)
	  speedoRise = 0;
}

void
cGrBoard::setWidth(int val)
{
  // Setup the margins according to percentage of screen width
  centerAnchor = (val / 2);
  leftAnchor = (val / 2) - val * boardWidth / 200;
  rightAnchor = (val / 2) + val * boardWidth / 200;
}

void
cGrBoard::selectBoard(int val)
{
  snprintf (path, sizeof(path), "%s/%d", GR_SCT_DISPMODE, id);
  
  switch (val) {
    case 0:
      boardFlag = (boardFlag + 1) % NB_BOARDS;
      GfParmSetNum(grHandle, path, GR_ATT_BOARD, (char*)NULL, (tdble)boardFlag);
      break;
    case 1:
      counterFlag = (counterFlag + 1) % NB_BOARDS;
      GfParmSetNum(grHandle, path, GR_ATT_COUNTER, (char*)NULL, (tdble)counterFlag);
      break;
    case 2:
      leaderFlag = (leaderFlag + 1) % NB_LBOARDS;
      GfParmSetNum(grHandle, path, GR_ATT_LEADER, (char*)NULL, (tdble)leaderFlag);
      break;
    case 3:
      debugFlag = (debugFlag + 1) % NB_DEBUG;
      GfParmSetNum(grHandle, path, GR_ATT_DEBUG, (char*)NULL, (tdble)debugFlag);
      break;  
    case 4:
      GFlag = 1 - GFlag;
      GfParmSetNum(grHandle, path, GR_ATT_GGRAPH, (char*)NULL, (tdble)GFlag);
      break;  
    case 5:
      arcadeFlag = 1 - arcadeFlag;
      GfParmSetNum(grHandle, path, GR_ATT_ARCADE, (char*)NULL, (tdble)arcadeFlag);
      break;  
  }
  GfParmWriteFile(NULL, grHandle, "graph");
}


/** 
 * grDispDebug
 * 
 * Displays debug information in the top right corner.
 * It is a 3-state display, states as follows:
 * 0 - Don't display any info
 * 1 - Display the FPS only
 * 2 - Display FPS, the segment the car is on, car's distance from startline, current camera
 * 
 * @param instFps Instant frame rate value to display
 * @param avgFps Average frame rate value to display
 * @param car The current car
 */
void
cGrBoard::grDispDebug(float instFps, float avgFps, tCarElt *car)
{
  char buf[256];
  int x, y, dy;

  x = rightAnchor - 100;
  y = TOP_ANCHOR - 15;
  
  dy = GfuiFontHeight(GFUI_FONT_SMALL_C);

  //Display frame rates (instant and average)
  snprintf(buf, sizeof(buf), "FPS: %.1f(%.1f)", instFps, avgFps);
  GfuiPrintString(buf, grWhite, GFUI_FONT_SMALL_C, x, y, GFUI_ALIGN_HL_VB);

  if(debugFlag == 2) {  //Only display detailed information in Debug Mode 2
    //Display segment name
    y -= dy;
    snprintf(buf, sizeof(buf),  "Seg: %s", car->_trkPos.seg->name);
    GfuiPrintString(buf, grWhite, GFUI_FONT_SMALL_C, x, y, GFUI_ALIGN_HL_VB);

    //Display distance from start
    y -= dy;
    snprintf(buf, sizeof(buf), "DfS: %5.0f", car->_distFromStartLine);
    GfuiPrintString(buf, grWhite, GFUI_FONT_SMALL_C, x, y, GFUI_ALIGN_HL_VB);

    //Display current camera
    y -= dy;
    tRoadCam *curCam = car->_trkPos.seg->cam;
    if (curCam) {
      snprintf(buf, sizeof(buf), "Cam: %s", curCam->name);
      GfuiPrintString(buf, grWhite, GFUI_FONT_SMALL_C, x, y, GFUI_ALIGN_HL_VB);
    }
  }
}


void
cGrBoard::grDispGGraph(tCarElt *car)
{
  // Position the graph
  const tdble X1 = (tdble)(rightAnchor - 100);
  const tdble Y1 = (tdble)(BOTTOM_ANCHOR + 70);
  const tdble XC = (tdble)(rightAnchor - 30);
  const tdble YC = (tdble)(Y1 - 50);

  // Draw the static blue thin cross and vertical segment.
  glBegin(GL_LINES);
  glColor4f(1.0, 1.0, 1.0, 1.0);
  glVertex2f(X1-50, Y1);
  glVertex2f(X1+50, Y1);
  glVertex2f(X1, Y1-50);
  glVertex2f(X1, Y1+50);
  glVertex2f(XC, YC);
  glVertex2f(XC, YC+100);
  glEnd();

  // Draw the throttle gauge (vertical thick segment, starting in X1,Y1,
  // going upwards, length proportional to the throttle command).
  static const tdble THNSS = 2.0f; // static => the assignment is only executed once ever.

  glBegin(GL_QUADS);
  glColor4f(0.0, 0.0, 1.0, 1.0);
  glVertex2f(X1 - THNSS, Y1);
  glVertex2f(X1 + THNSS, Y1);
  glVertex2f(X1 + THNSS, Y1 + car->ctrl.accelCmd * 50.0f);
  glVertex2f(X1 - THNSS, Y1 + car->ctrl.accelCmd * 50.0f);

  // Draw the brake gauge (vertical thick segment, starting in X1,Y1,
  // going downward, length proportional to the brake command,
  // red if at least one wheel is blocking/blocked, blue otherwise).
  // a) Detect wheel blocking, and change current color to red if so.
  for (int xx = 0; xx < 4; ++xx) 
  {
    if (car->_wheelSpinVel(xx) < car->_speed_x - 5.0f)
    {
      glColor4f(1.0, 0.0, 0.0, 1.0);
      break;
    }
  }

  // b) Draw the gauge.
  glVertex2f(X1 - THNSS, Y1);
  glVertex2f(X1 + THNSS, Y1);
  glVertex2f(X1 + THNSS, Y1 - car->ctrl.brakeCmd * 50.0f);
  glVertex2f(X1 - THNSS, Y1 - car->ctrl.brakeCmd * 50.0f);

  // Back to normal blue color.
  glColor4f(0.0, 0.0, 1.0, 1.0);

  // Draw the steer gauge (horizontal thick segment, starting in X1,Y1,
  // going left or right according to the direction,
  // length proportional to the steer command).
  glVertex2f(X1, Y1 - THNSS);
  glVertex2f(X1, Y1 + THNSS);
  glVertex2f(X1 - car->ctrl.steer * 100.0f, Y1 + THNSS);
  glVertex2f(X1 - car->ctrl.steer * 100.0f, Y1 - THNSS);

  // Draw the clutch gauge (vertical thick segment, starting in xc, yc,
  // going upwards, length proportional to the clutch command).
  glVertex2f(XC - THNSS, YC);
  glVertex2f(XC + THNSS, YC);
  glVertex2f(XC + THNSS, YC + car->ctrl.clutchCmd * 100.0f);
  glVertex2f(XC - THNSS, YC + car->ctrl.clutchCmd * 100.0f);

  glEnd();

  // Draw the acceleration gauge (thin segment, starting in X1, Y1,
  // going in the direction of the acceleration vector).
  const tdble X2 = -car->_DynGC.acc.y / 9.81f * 25.0f + X1;
  const tdble Y2 = car->_DynGC.acc.x / 9.81f * 25.0f + Y1;

  glBegin(GL_LINES);
  glColor4f(1.0, 0.0, 0.0, 1.0);
  glVertex2f(X1, Y1);
  glVertex2f(X2, Y2);
  glEnd();
}


void
cGrBoard::grDrawGauge(tdble X1, tdble Y1, tdble H, float *clr1, float *clr2, tdble val, const char *title)
{
  tdble curH;

  curH = MIN(val, 1.0);
  curH = MAX(curH, 0.0);
  curH *= H;
  
  static const tdble THNSSBG = 2.0; // static => the assignment is only executed once ever.
  static const tdble THNSSFG = 2.0; // static => the assignment is only executed once ever.

  glBegin(GL_QUADS);
  glColor4fv(grBlack);
  glVertex2f(X1 - (THNSSBG + THNSSFG), Y1 - THNSSBG);
  glVertex2f(X1 + (THNSSBG + THNSSFG), Y1 - THNSSBG);
  glVertex2f(X1 + (THNSSBG + THNSSFG), Y1 + H + THNSSBG);
  glVertex2f(X1 - (THNSSBG + THNSSFG), Y1 + H + THNSSBG);

  glColor4fv(clr2);
  glVertex2f(X1 - THNSSFG, Y1 + curH);
  glVertex2f(X1 + THNSSFG, Y1 + curH);
  glVertex2f(X1 + THNSSFG, Y1 + H);
  glVertex2f(X1 - THNSSFG, Y1 + H);

  glColor4fv(clr1);
  glVertex2f(X1 - THNSSFG, Y1);
  glVertex2f(X1 + THNSSFG, Y1);
  glVertex2f(X1 + THNSSFG, Y1 + curH);
  glVertex2f(X1 - THNSSFG, Y1 + curH);
  glEnd();
  GfuiPrintString((char *)title, grBlue, GFUI_FONT_MEDIUM, (int)X1, (int)(Y1 - THNSSBG - GfuiFontHeight(GFUI_FONT_MEDIUM)), GFUI_ALIGN_HC_VB);
}

void 
cGrBoard::grDispMisc(bool bCurrentScreen)
{
  if (bCurrentScreen)
  {
    // Draw the current screen indicator (only in split screen mode)
    // (a green-filled square on the bottom right corner of the screen).
    static const float h = 10.0f;
    const float w = h;

    const float x = rightAnchor - w - 5;
    const float y = BOTTOM_ANCHOR + 5;
    
    glBegin(GL_QUADS);
    glColor4f(0.0, 1.0, 0.0, 1.0);
    glVertex2f(x,     y);
    glVertex2f(x + w, y);
    glVertex2f(x + w, y + h);
    glVertex2f(x,     y + h);
    glEnd();
  }
}

void
cGrBoard::grDispCarBoard1(tCarElt *car, tSituation *s)
{
  int  x, x2, y;
  char buf[256];
  char const* lapsTimeLabel;
  float *clr;
  int dy, dy2, dx;
  
  snprintf(buf, sizeof(buf), "%d/%d - %s", car->_pos, s->_ncars, car->_name);
  
  dy = GfuiFontHeight(GFUI_FONT_MEDIUM_C);
  dy2 = GfuiFontHeight(GFUI_FONT_SMALL_C);
  dx = GfuiFontWidth(GFUI_FONT_MEDIUM_C, buf);

  x = leftAnchor + 10;
  y = TOP_ANCHOR - dy - 5;

  x2 = x + 100;
  dx = MAX(dx, (x2-x));

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA) ;
  glBegin(GL_QUADS);
  glColor4f(0.1, 0.1, 0.1, 0.8);
  glVertex2f(x-5, y + dy);
  glVertex2f(x+dx+5, y + dy);
  glVertex2f(x+dx+5, y-5 - dy2 * 8 /* lines */);
  glVertex2f(x-5, y-5 - dy2 * 8 /* lines */);
  glEnd();
  glDisable(GL_BLEND);
  
  GfuiPrintString(buf, grCarInfo[car->index].iconColor, GFUI_FONT_MEDIUM_C, x, y, GFUI_ALIGN_HL_VB);
  y -= dy;
  
  dy = GfuiFontHeight(GFUI_FONT_SMALL_C);
  
  GfuiPrintString("Fuel:", grWhite, GFUI_FONT_SMALL_C, x, y, GFUI_ALIGN_HL_VB);
  if (car->_fuel < 5.0) {
    clr = grRed;
  } else {
    clr = grWhite;
  }
  snprintf(buf, sizeof(buf),  "%.1f l", car->_fuel);
  GfuiPrintString(buf, clr, GFUI_FONT_SMALL_C, x2, y, GFUI_ALIGN_HR_VB);
  y -= dy;
  
  if (car->_state & RM_CAR_STATE_BROKEN) {
    clr = grRed;
  } else {
    clr = grWhite;
  }
  
  GfuiPrintString("Damage:", clr, GFUI_FONT_SMALL_C, x, y, GFUI_ALIGN_HL_VB);
  snprintf(buf, sizeof(buf),  "%d", car->_dammage);
  GfuiPrintString(buf, clr, GFUI_FONT_SMALL_C, x2, y, GFUI_ALIGN_HR_VB);
  y -= dy;
  clr = grWhite;
  
  grGetLapsTime (s, car, buf, &lapsTimeLabel);
  GfuiPrintString(lapsTimeLabel, clr, GFUI_FONT_SMALL_C, x, y, GFUI_ALIGN_HL_VB);
  GfuiPrintString(buf, clr, GFUI_FONT_SMALL_C, x2, y, GFUI_ALIGN_HR_VB);
  y -= dy;
  
  GfuiPrintString("Total:", clr, GFUI_FONT_SMALL_C, x, y, GFUI_ALIGN_HL_VB);
  grWriteTime(clr, GFUI_FONT_SMALL_C, x2, y, s->currentTime, 0);
  y -= dy;
  
  GfuiPrintString("Curr:", clr, GFUI_FONT_SMALL_C, x, y, GFUI_ALIGN_HL_VB);
  grWriteTime(clr, GFUI_FONT_SMALL_C, x2, y, car->_curLapTime, 0);
  y -= dy;
  
  GfuiPrintString("Last:", clr, GFUI_FONT_SMALL_C, x, y, GFUI_ALIGN_HL_VB);
  grWriteTime(clr, GFUI_FONT_SMALL_C, x2, y, car->_lastLapTime, 0);
  y -= dy;
  
  GfuiPrintString("Best:", clr, GFUI_FONT_SMALL_C, x, y, GFUI_ALIGN_HL_VB);
  grWriteTime(clr, GFUI_FONT_SMALL_C, x2, y, car->_bestLapTime, 0);
  y -= dy;
  
  GfuiPrintString("Top Speed:", clr, GFUI_FONT_SMALL_C, x, y, GFUI_ALIGN_HL_VB);
  snprintf(buf, sizeof(buf), "%d", (int)(car->_topSpeed * 3.6));
  GfuiPrintString(buf, clr, GFUI_FONT_SMALL_C, x2, y, GFUI_ALIGN_HR_VB);
  y -= dy;
}

void
cGrBoard::grDispCarBoard2(tCarElt *car, tSituation *s)
{
  int  x, x2, x3, y;
  char buf[256];
  char const *lapsTimeLabel;
  float *clr;
  double time;
  int dy, dy2, dx;
  int lines, i;
  
  snprintf(buf, sizeof(buf),  "%d/%d - %s", car->_pos, s->_ncars, car->_name);

  dy = GfuiFontHeight(GFUI_FONT_MEDIUM_C);
  dy2 = GfuiFontHeight(GFUI_FONT_SMALL_C);
  dx = GfuiFontWidth(GFUI_FONT_MEDIUM_C, buf);
  
  x = leftAnchor + 10;
  x2 = x + 100;
  x3 = x + 160;
  y = TOP_ANCHOR - dy - 5;
  
  dx = MAX(dx, (x3-x));

  lines = 6;
  for (i = 0; i < 4; i++) {
    if (car->ctrl.msg[i]) {
      lines++;
    }
  }
  
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA) ;
  glBegin(GL_QUADS);
  glColor4f(0.1, 0.1, 0.1, 0.8);
  glVertex2f(x - 5, y + dy);
  glVertex2f(x + dx + 5, y + dy);
  glVertex2f(x + dx + 5, y - 5 - dy2 * lines);
  glVertex2f(x - 5, y - 5 - dy2 * lines);
  glEnd();
  glDisable(GL_BLEND);
  
  GfuiPrintString(buf, grCarInfo[car->index].iconColor, GFUI_FONT_MEDIUM_C, x, y, GFUI_ALIGN_HL_VB);
  y -= dy;
  
  dy = GfuiFontHeight(GFUI_FONT_SMALL_C);
  
  GfuiPrintString("Fuel:", grWhite, GFUI_FONT_SMALL_C, x, y, GFUI_ALIGN_HL_VB);
  if (car->_fuel < 5.0) {
    clr = grRed;
  } else {
    clr = grWhite;
  }
  snprintf(buf, sizeof(buf),  "%.1f l", car->_fuel);
  GfuiPrintString(buf, clr, GFUI_FONT_SMALL_C, x2, y, GFUI_ALIGN_HR_VB);
  y -= dy;
  
  clr = grWhite;
  
  grGetLapsTime (s, car, buf, &lapsTimeLabel);
  GfuiPrintString(lapsTimeLabel, clr, GFUI_FONT_SMALL_C, x, y, GFUI_ALIGN_HL_VB);
  GfuiPrintString(buf, clr, GFUI_FONT_SMALL_C, x2, y, GFUI_ALIGN_HR_VB);
  y -= dy;
  
  GfuiPrintString("Best:", clr, GFUI_FONT_SMALL_C, x, y, GFUI_ALIGN_HL_VB);
  grWriteTime(clr, GFUI_FONT_SMALL_C, x2, y, car->_bestLapTime, 0);
  grWriteTime(clr, GFUI_FONT_SMALL_C, x3, y, car->_deltaBestLapTime, 1);
  y -= dy;
  
  GfuiPrintString("Time:", clr, GFUI_FONT_SMALL_C, x, y, GFUI_ALIGN_HL_VB);
  grWriteTime(clr, GFUI_FONT_SMALL_C, x2, y, car->_curLapTime, 0);
  if (grGetSplitTime(s, car, false, time, NULL, &clr))
    grWriteTime(clr, GFUI_FONT_SMALL_C, x3, y, time, 1);
  clr = grWhite;
  y -= dy;
  
  if (car->_pos != 1) {
    snprintf(buf, sizeof(buf),  "<- %s", s->cars[car->_pos - 2]->_name);
    GfuiPrintString(buf, clr, GFUI_FONT_SMALL_C, x, y, GFUI_ALIGN_HL_VB);
    if (s->_raceType == RM_TYPE_RACE)
    {
      if (s->cars[car->_pos - 2]->_laps == car->_laps) {
        grWriteTime(clr, GFUI_FONT_SMALL_C, x3, y, s->cars[car->_pos - 2]->_curTime-car->_curTime, 1);
      } else {
        GfuiPrintString("       --:--", clr, GFUI_FONT_SMALL_C, x3, y, GFUI_ALIGN_HR_VB);
      }
    }
    else
    {
      if (car->_bestLapTime > 0.0f)
        grWriteTime(clr, GFUI_FONT_SMALL_C, x3, y, car->_bestLapTime - s->cars[car->_pos - 2]->_bestLapTime, 1);
      else
        GfuiPrintString("       --:--", clr, GFUI_FONT_SMALL_C, x3, y, GFUI_ALIGN_HR_VB);
    }
  } else {
    GfuiPrintString("<- ", clr, GFUI_FONT_SMALL_C, x, y, GFUI_ALIGN_HL_VB);
    GfuiPrintString("       --:--", clr, GFUI_FONT_SMALL_C, x3, y, GFUI_ALIGN_HR_VB);
  }
  y -= dy;
  
  if (car->_pos != s->_ncars) {
    snprintf(buf, sizeof(buf),  "-> %s", s->cars[car->_pos]->_name);
    GfuiPrintString(buf, clr, GFUI_FONT_SMALL_C, x, y, GFUI_ALIGN_HL_VB);
    if (s->_raceType == RM_TYPE_RACE)
    {
      if (s->cars[car->_pos]->_laps == car->_laps) {
        grWriteTime(clr, GFUI_FONT_SMALL_C, x3, y, s->cars[car->_pos]->_curTime-car->_curTime, 1);    
      } else {
        GfuiPrintString("       --:--", clr, GFUI_FONT_SMALL_C, x3, y, GFUI_ALIGN_HR_VB);
      }
    }
    else
    {
      if (s->cars[car->_pos]->_bestLapTime > 0.0f)
        grWriteTime(clr, GFUI_FONT_SMALL_C, x3, y, s->cars[car->_pos]->_bestLapTime - car->_bestLapTime, 1);
      else
        GfuiPrintString("       --:--", clr, GFUI_FONT_SMALL_C, x3, y, GFUI_ALIGN_HR_VB);
    }
  } else {
    GfuiPrintString("-> ", clr, GFUI_FONT_SMALL_C, x, y, GFUI_ALIGN_HL_VB);
    GfuiPrintString("       --:--", clr, GFUI_FONT_SMALL_C, x3, y, GFUI_ALIGN_HR_VB);
  }
  y -= dy;
  for (i = 0; i < 4; i++) {
    if (car->ctrl.msg[i]) {
      GfuiPrintString(car->ctrl.msg[i], car->ctrl.msgColor, GFUI_FONT_SMALL_C, x, y, GFUI_ALIGN_HL_VB);
      y -= dy;
    }
  }
}

void
cGrBoard::grDispCarBoard(tCarElt *car, tSituation *s)
{
  switch(boardFlag) {
    case 0:
      break;
    case 1:
      grDispCarBoard1(car, s);
      break;
    case 2:
      grDispCarBoard2(car, s);
      break;
    default:
      break;
  }
}

#define ALIGN_CENTER  0
#define ALIGN_LEFT  1
#define ALIGN_RIGHT 2

static void
grDispEngineLeds (tCarElt *car, int X, int Y, int align, int bg)
{
  int  x, y;
  int  xref;
  GLfloat ledcolg[2][3] = { 
    {0.0, 0.2, 0.0},
    {0.0, 1.0, 0.0}
  };

  GLfloat ledcolr[2][3] = { 
    {0.2, 0.0, 0.0},
    {1.0, 0.0, 0.0}
  };

  int i;
  int ledNb     = 20;
  int ledHeight = 10;
  int ledWidth  = 5;
  int ledSpace  = 2;
  int ledRed    = (int)((car->_enginerpmRedLine * .9 / car->_enginerpmMax) * (tdble)ledNb);
  int ledLit    = (int)((car->_enginerpm / car->_enginerpmMax) * (tdble)ledNb);

  switch (align) {
    case ALIGN_CENTER:
      x = X - ((ledNb * ledWidth) + (ledNb - 1) * ledSpace) / 2;
      break;
    case ALIGN_LEFT:
      x = X;
      break;
    case ALIGN_RIGHT:
      x = X - ((ledNb * ledWidth) + (ledNb - 1) * ledSpace);
      break;
    default:
      x = X - ((ledNb * ledWidth) + (ledNb - 1) * ledSpace) / 2;
      break;
  }

  y = Y;
  glBegin(GL_QUADS);
  
  if (bg) {
    glColor3f(0.1, 0.1, 0.1);
    glVertex2f(x - ledSpace, y + ledHeight + ledSpace);
    glVertex2f(x + ledNb * (ledWidth+ ledSpace), y + ledHeight + ledSpace);
    glVertex2f(x + ledNb * (ledWidth+ ledSpace), BOTTOM_ANCHOR);
    glVertex2f(x - ledSpace, BOTTOM_ANCHOR);
  }

  xref = x;
  glColor3fv(ledcolg[0]);
  for (i = 0; i < ledRed; i++) {
    glVertex2f(x, y);
    glVertex2f(x + ledWidth, y);
    glVertex2f(x + ledWidth, y + ledHeight);
    glVertex2f(x, y + ledHeight);
    x += ledWidth + ledSpace;
  }

  glColor3fv(ledcolr[0]);
  for (i = ledRed; i < ledNb; i++) {
    glVertex2f(x, y);
    glVertex2f(x + ledWidth, y);
    glVertex2f(x + ledWidth, y + ledHeight);
    glVertex2f(x, y + ledHeight);
    x += ledWidth + ledSpace;
  }
  x = xref;
#define DD  1
  glColor3fv(ledcolg[1]);
  for (i = 0; i < ledNb; i++) {
    if (i == ledRed) {
      glColor3fv(ledcolr[1]);
    }
    if (i <= ledLit) {
      glVertex2f(x + DD, y + DD);
      glVertex2f(x + ledWidth - DD, y + DD);
      glVertex2f(x + ledWidth - DD, y + ledHeight - DD);
      glVertex2f(x + DD, y + ledHeight - DD);
      x += ledWidth + ledSpace;
    } else {
      break;
    }
  }
  glEnd();
}

void
cGrBoard::grDispCounterBoard(tCarElt *car)
{
  int  x, y;
  char buf[256];
  
  grDispEngineLeds (car, centerAnchor,  BOTTOM_ANCHOR + MAX(GfuiFontHeight(GFUI_FONT_BIG_C), GfuiFontHeight(GFUI_FONT_DIGIT)), ALIGN_CENTER, 1);
  
  x = centerAnchor;
  y = BOTTOM_ANCHOR;

  if (car->_gear <= 0)
    snprintf(buf, sizeof(buf), " kph %s", car->_gear == 0 ? "N" : "R");
  else
    snprintf(buf, sizeof(buf), " kph %d", car->_gear);
  GfuiPrintString(buf, grBlue, GFUI_FONT_BIG_C, x, y, GFUI_ALIGN_HL_VB);
  
  x = centerAnchor;
  snprintf(buf, sizeof(buf), "%3d", abs((int)(car->_speed_x * 3.6)));
  GfuiPrintString(buf, grBlue, GFUI_FONT_BIG_C, x, y, GFUI_ALIGN_HR_VB);
}

/** 
 * grDispLeaderBoard
 * 
 * Displayes the leader board (in the lower left corner of the screen)
 * If [drawLaps] is on, writes the lap counter on the top of the list.
 * @param car the car currently being viewed
 * @param s situation provided by the sim
 */
void
cGrBoard::grDispLeaderBoard(const tCarElt *car, const tSituation *s)
{
  double time_left;
  if(leaderFlag == 4) 
    {
      //Scrolling line in the bottom of the screen
      grDispLeaderBoardScrollLine(car, s);
    }
  //This mode is only reasonable if there are more drivers
  //than the leaderboard can display at once.
  else if(leaderFlag == 3 && leaderNb < s->_ncars)
    {
      //Scrolling results in the bottom left corner
      grDispLeaderBoardScroll(car, s);
    }
  else
    { //Static leaderboard
    char buf[256];

    int current = 0;  //Position of the currently displayed car
    for(int i = 0; i < s->_ncars; i++) {
      if (car == s->cars[i]) {
        current = i;
        break;
      }
    }//for i
    
    //Coords, limits
    int x = leftAnchor + 5;
    int x2 = x + 165;
    int y = BOTTOM_ANCHOR + 10;

    int dy = GfuiFontHeight(GFUI_FONT_SMALL_C);
    int maxLines = MIN(leaderNb, s->_ncars);  //Max # of lines to display (# of cars in race or max 10 by default)
    int drawLaps = MIN(1, leaderFlag - 1);  //Display # of laps on top of the list?

    //Set up drawing area
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA) ;
    glBegin(GL_QUADS);
    glColor4f(0.1, 0.1, 0.1, 0.8);
    glVertex2f(x, y - 5);
    glVertex2f(x + 175, y - 5);
    glVertex2f(x + 175, y + dy * (maxLines + drawLaps));
    glVertex2f(x, y + dy * (maxLines + drawLaps));
    glEnd();
    glDisable(GL_BLEND);
    
    //Display current car in last line (ie is its position >= 10)?
    int drawCurrent = (current >= maxLines) ? 1 : 0;

    //The board is built from bottom up:
    //driver position #10/current is drawn first,
    //then from #9 onto #1, then the text 'Laps' if drawLaps requires.
    for(int j = maxLines; j > 0; j--) {
      int i;  //index of driver to be displayed
      if (drawCurrent) {
        i = current;
        drawCurrent = 0;
      } else {
        i = j - 1;
      }//if drawCurrent

      //Set colour of the currently displayed driver to that
      //defined in the driver's XML file
      float *clr; //colour to display driver name
      if (i == current) {
        clr = grCarInfo[car->index].iconColor;
        drawCurrent = 0;
      } else {
        clr = grWhite;
      }//if i

      //Driver position + name
      snprintf(buf, sizeof(buf), "%3d: %s", i + 1, s->cars[i]->_name);
      GfuiPrintString(buf, clr, GFUI_FONT_SMALL_C, x, y, GFUI_ALIGN_HL_VB);

      //Display driver time / time behind leader / laps behind leader
      string sEntry = grGenerateLeaderBoardEntry(s->cars[i], s, (i==0));
      if(s->cars[i]->_state & RM_CAR_STATE_DNF
        || s->cars[i]->_state & RM_CAR_STATE_PIT) { //driver DNF or in pit, show 'out' in red
        clr = grRed;
      }
      GfuiPrintString(sEntry.c_str(), clr, GFUI_FONT_SMALL_C, x2, y, GFUI_ALIGN_HR_VB);
      
      y += dy;  //'Line feed'
    }//for j

    //Write 'Lap X/Y' on top of the leader board
    if (drawLaps) {
      if (s->_raceType == RM_TYPE_RACE)
      {
        if (s->_totTime > s->currentTime)
  {
          GfuiPrintString(" Laps:", grWhite, GFUI_FONT_SMALL_C, x, y, GFUI_ALIGN_HL_VB);
          snprintf(buf, sizeof(buf), "%d", MAX(s->cars[0]->_laps-1,0));
          GfuiPrintString(buf, grWhite, GFUI_FONT_SMALL_C, x2, y, GFUI_ALIGN_HR_VB);
  } else {
          GfuiPrintString(" Lap:", grWhite, GFUI_FONT_SMALL_C, x, y, GFUI_ALIGN_HL_VB);
          snprintf(buf, sizeof(buf), "%d / %d", s->cars[0]->_laps, s->_totLaps);
          GfuiPrintString(buf, grWhite, GFUI_FONT_SMALL_C, x2, y, GFUI_ALIGN_HR_VB);
  }
      } else {
        if (s->_totTime > 0.0f)
  {
    time_left = MAX(MIN(s->_totTime,s->_totTime - s->currentTime),0);
          GfuiPrintString(" Time left:", grWhite, GFUI_FONT_SMALL_C, x, y, GFUI_ALIGN_HL_VB);
          snprintf(buf, sizeof(buf), "%d:%02d:%02d", (int)floor(time_left / 3600.0f), (int)floor(time_left/60.0f) % 60, (int)floor(time_left) % 60);
          GfuiPrintString(buf, grWhite, GFUI_FONT_SMALL_C, x2, y, GFUI_ALIGN_HR_VB);
  } else {
          GfuiPrintString(" Lap:", grWhite, GFUI_FONT_SMALL_C, x, y, GFUI_ALIGN_HL_VB);
          snprintf(buf, sizeof(buf), "%d / %d", s->cars[0]->_laps, s->_totLaps);
          GfuiPrintString(buf, grWhite, GFUI_FONT_SMALL_C, x2, y, GFUI_ALIGN_HR_VB);
  }
      }
    }//if drawLaps
  }//else
}//grDispLeaderBoard


void
cGrBoard::grDispCounterBoard2(tCarElt *car)
{
  int index;
  tgrCarInstrument *curInst;
  tdble val;
  char buf[32];
  
  index = car->index; /* current car's index */
  curInst = &(grCarInfo[index].instrument[0]);
  
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA) ;
  glEnable(GL_TEXTURE_2D);
  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
  glTranslatef(centerAnchor, BOTTOM_ANCHOR + (speedoRise * TOP_ANCHOR / 100), 0);
  if (curInst->texture) {
    glBindTexture(GL_TEXTURE_2D, curInst->texture->getTextureHandle());
  }
  glCallList(curInst->CounterList);
  glBindTexture(GL_TEXTURE_2D, 0);
  
  val = (*(curInst->monitored) - curInst->minValue) / curInst->maxValue;
  if (val > 1.0) {
    val = 1.0;
  } else if (val < 0.0) {
    val = 0.0;
  }
  
  val = curInst->minAngle + val * curInst->maxAngle;
  
  RELAXATION(val, curInst->prevVal, 30);
  
  glPushMatrix();
  glTranslatef(curInst->needleXCenter, curInst->needleYCenter, 0);
  glRotatef(val, 0, 0, 1);
  glCallList(curInst->needleList);
  glPopMatrix();
  
  if (car->_gear <= 0)
    snprintf(buf, sizeof(buf), "%s", car->_gear == 0 ? "N" : "R");
  else
    snprintf(buf, sizeof(buf), "%d", car->_gear);
  GfuiPrintString(buf, curInst->needleColor, GFUI_FONT_LARGE_C,
      (int)curInst->digitXCenter, (int)curInst->digitYCenter, GFUI_ALIGN_HC_VB);
  
  glTranslatef(-centerAnchor, -BOTTOM_ANCHOR, 0);

  curInst = &(grCarInfo[index].instrument[1]);
  
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA) ;
  glEnable(GL_TEXTURE_2D);
  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
  glTranslatef(centerAnchor, BOTTOM_ANCHOR, 0);
  if (curInst->texture) {
    glBindTexture(GL_TEXTURE_2D, curInst->texture->getTextureHandle());
  }
  glCallList(curInst->CounterList);
  glBindTexture(GL_TEXTURE_2D, 0);
  
  val = (*(curInst->monitored) - curInst->minValue) / curInst->maxValue;
  if (val > 1.0) {
    val = 1.0;
  } else if (val < 0.0) {
    val = 0.0;
  }
  val = curInst->minAngle + val * curInst->maxAngle;
  
  RELAXATION(val, curInst->prevVal, 30);
  
  glPushMatrix();
  glTranslatef(curInst->needleXCenter, curInst->needleYCenter, 0);
  glRotatef(val, 0, 0, 1);
  glCallList(curInst->needleList);
  glPopMatrix();
  
  if (curInst->digital) {
    // Do not add "%3d" or something, because the digital font DOES NOT SUPPORT BLANKS!!!!
    snprintf(buf, sizeof(buf), "%d", abs((int)(car->_speed_x * 3.6)));
    GfuiPrintString(buf, curInst->needleColor, GFUI_FONT_LARGE_C,
      (int)curInst->digitXCenter, (int)curInst->digitYCenter, GFUI_ALIGN_HC_VB);
  }

  glTranslatef(-centerAnchor, -BOTTOM_ANCHOR, 0);
  
  if (counterFlag == 1){
    float *clr;
    if (car->_fuel < 5.0f) {
      clr = grRed;
    } else {
      clr = grWhite;
    }
    
    grDrawGauge(centerAnchor + 140, BOTTOM_ANCHOR + 25, 100, clr, grBlack, car->_fuel / car->_tank, "F");
    grDrawGauge(centerAnchor + 155, BOTTOM_ANCHOR + 25, 100, grRed, grGreen, (tdble)(car->_dammage) / grMaxDammage, "D");
  }

  glTranslatef(0, -(speedoRise * TOP_ANCHOR / 100), 0);
}

void
cGrBoard::initBoard(void)
{
  if (trackMap == NULL) {
    trackMap = new cGrTrackMap();
  }
}

void
cGrBoard::shutdown(void)
{                                                                                                                                                                                                                                                                         
  if (trackMap != NULL) {
    delete trackMap;
    trackMap = NULL;
  }
  
  //Resetting scrolling leaderboard variables
  sShortNames.clear();
  st.clear();
  iStart = 0;
  iTimer = 0.0;
  iStringStart = 0;
}

void
cGrBoard::grDispArcade(tCarElt *car, tSituation *s)
{
  int  x, y;
  int  dy;
  char buf[256];
  float *clr;

#define XM  15
#define YM  10

  dy = GfuiFontHeight(GFUI_FONT_BIG_C);

  x = leftAnchor + XM;
  y = TOP_ANCHOR - YM - dy;

  snprintf(buf, sizeof(buf), "%d/%d", car->_pos, s->_ncars);
  GfuiPrintString(buf, grDefaultClr, GFUI_FONT_BIG_C, x, y, GFUI_ALIGN_HL_VB);

  dy = GfuiFontHeight(GFUI_FONT_LARGE_C);
  y -= dy;
  GfuiPrintString("Time:", grDefaultClr, GFUI_FONT_LARGE_C, x, y, GFUI_ALIGN_HL_VB);
  grWriteTime(grDefaultClr, GFUI_FONT_LARGE_C, x + 150, y, car->_curLapTime, 0);

  y -= dy;
  GfuiPrintString("Best:", grDefaultClr, GFUI_FONT_LARGE_C, x, y, GFUI_ALIGN_HL_VB);
  grWriteTime(grDefaultClr, GFUI_FONT_LARGE_C, x + 150, y, car->_bestLapTime, 0);

  x = rightAnchor - XM;
  y = TOP_ANCHOR - YM - dy;
  grGetLapsTime (s, car, buf, NULL);
  GfuiPrintString(buf, grDefaultClr, GFUI_FONT_LARGE_C, x, y, GFUI_ALIGN_HR_VB);
  

  x = centerAnchor;
  snprintf(buf, sizeof(buf), "%s", car->_name);
  GfuiPrintString(buf, grDefaultClr, GFUI_FONT_LARGE_C, x, y, GFUI_ALIGN_HC_VB);


  if (car->_fuel < 5.0) {
    clr = grRed;
  } else {
    clr = grWhite;
  }
  grDrawGauge(leftAnchor + XM, BOTTOM_ANCHOR + 25, 100, clr, grBlack, car->_fuel / car->_tank, "F");
  grDrawGauge(leftAnchor + XM + 15, BOTTOM_ANCHOR + 25, 100, grRed, grGreen, (tdble)(car->_dammage) / grMaxDammage, "D");

  x = rightAnchor - XM;
  dy = GfuiFontHeight(GFUI_FONT_LARGE_C);
  y = YM + dy;
  snprintf(buf, sizeof(buf), "%3d km/h", abs((int)(car->_speed_x * 3.6)));
  GfuiPrintString(buf, grDefaultClr, GFUI_FONT_BIG_C, x, y, GFUI_ALIGN_HR_VB);
  y = YM;
  if (car->_gear <= 0)
    snprintf(buf, sizeof(buf), "%s", car->_gear == 0 ? "N" : "R");
  else
    snprintf(buf, sizeof(buf), "%d", car->_gear);
  GfuiPrintString(buf, grDefaultClr, GFUI_FONT_LARGE_C, x, y, GFUI_ALIGN_HR_VB);

  grDispEngineLeds (car, rightAnchor - XM, YM + dy + GfuiFontHeight (GFUI_FONT_BIG_C), ALIGN_RIGHT, 0);
}

/**
 * This function calculates if the split time must be displaded, and if so what the
 * split time is.
 *
 * @param s[in] A pointer to the current situation
 * @param car[in] A pointer to the car to calculate the split time for
 * @param gap_inrace[in] True if it must display the gap during races, false if compares the current lap with the personal best lap
 * @param time[out] The split difference time
 * @param laps_different[out] Contains the number of laps behind / for at the split point
 * @param color[out] The color which can be used to display the split time
 * @return true if there is a split time to be displayed, false otherwise
 */
bool cGrBoard::grGetSplitTime(tSituation *s, tCarElt *car, bool gap_inrace, double &time, int *laps_different, float **color)
{
  tdble curSplit;
  tdble bestSplit;
  tdble bestSessionSplit;
  tCarElt *ocar = car;
  tCarElt *fcar = car;
  int sign = 1;
  int laps;

  if (laps_different)
    *laps_different = 0;

  if (s->_raceType != RM_TYPE_RACE || s->_ncars == 1)
  {
    if (car->_currentSector == 0)
      return false;

    curSplit = car->_curSplitTime[car->_currentSector - 1];
    bestSplit = car->_bestSplitTime[car->_currentSector - 1];

    if (car->_curLapTime - curSplit > 5.0f)
      return false; /* Only display split for five seconds */

    if (s->_ncars > 1)
    {
      bestSessionSplit = s->cars[0]->_bestSplitTime[car->_currentSector - 1];

      if (bestSessionSplit <= 0.0f)
        return false;

      time = curSplit - bestSessionSplit;
      if (time < 0.0f)
        *color = grPink;
      else if (curSplit < bestSplit)
        *color = grGreen;
      else
        *color = grWhite;
    }
    else
    {
      if (bestSplit < 0.0f)
        return false;

      time = curSplit - bestSplit;

      if (time < 0.0f)
        *color = grGreen;
      else
        *color = grWhite;
    }
  }
  else if (gap_inrace)
  {
    if (car->_pos == 1)
    {
      fcar = s->cars[1];
      sign = -1;
    }

    ocar = s->cars[fcar->_pos-2];

    if (fcar->_currentSector == 0)
      return false;

    curSplit = fcar->_curSplitTime[fcar->_currentSector - 1];
    bestSplit = ocar->_curSplitTime[fcar->_currentSector - 1];

    if (fcar->_curLapTime - curSplit > 5.0f)
      return false;

    laps = ocar->_laps - fcar->_laps;
    if (ocar->_currentSector < fcar->_currentSector ||
         (ocar->_currentSector == fcar->_currentSector && fcar->_curTime + curSplit < ocar->_curTime + bestSplit ) )
      --laps;

    if (!laps_different && laps != 0)
      return false;

    if (laps_different)
      *laps_different = sign * laps;

    time = ocar->_curTime + bestSplit - ( fcar->_curTime + curSplit );
    if (sign < 0)
      time *= -1.0f;

    *color = grWhite;
  } else {
    if (car->_currentSector == 0)
      return false;

    curSplit = car->_curSplitTime[car->_currentSector - 1];
    bestSplit = car->_bestSplitTime[car->_currentSector - 1];

    if (bestSplit < 0.0f)
      return false;

    if (car->_curLapTime - curSplit > 5.0f)
      return false;

    time = curSplit - bestSplit;
    if (time < 0.0f)
      *color = grGreen;
    else
      *color = grWhite;
  }

  return true;
}

/**
 * This function gives back the information about the remaining laps / time
 *
 * @param s[in] The current situation
 * @param car[in] The current car
 * @param result[out] An already existing string which will contain the text
 * @param label[out] The label (Lap: or Time: ) If zero, then the label is added to @p result.
 */
void cGrBoard::grGetLapsTime(tSituation *s, tCarElt *car, char* result, char const **label) const
{
  char time = TRUE;
  double cur_left;
  char const *loc_label;

  if (s->_totTime < 0.0f || (s->_totTime < s->currentTime && s->_extraLaps > 0) )
    time = FALSE;
  
  if (label)
  {
    *label = time ? "Time: " : "Lap: ";
    loc_label = "";
  }
  else
  {
    loc_label = time ? "Time: " : "Lap: ";
  }

  if (!time)
  {
    snprintf(result, sizeof(result), "%s%d/%d", loc_label, car->_laps, s->_totLaps );
  }
  else 
  {
    cur_left = s->_totTime - s->currentTime;
    if (s->currentTime < 0.0f)
      cur_left = s->_totTime;
    if (cur_left < 0.0f)
      cur_left = 0.0f;

    snprintf(result, sizeof(result), "%s%d:%02d:%02d", loc_label, (int)floor( cur_left / 3600.0f ), (int)floor( cur_left / 60.0f ) % 60, (int)floor( cur_left ) % 60 );
  }
}

void cGrBoard::refreshBoard(tSituation *s, float instFps, float avgFps,
              bool forceArcade, tCarElt *currCar, bool isCurrScreen)
{
  grDispMisc(isCurrScreen);
  
  if (arcadeFlag || forceArcade) {
    grDispArcade(currCar, s);
  } else {
    if (debugFlag)
      grDispDebug(instFps, avgFps, currCar);
    if (GFlag)
      grDispGGraph(currCar);
    if (boardFlag)
      grDispCarBoard(currCar, s);
    if (leaderFlag)
      grDispLeaderBoard(currCar, s);
    if (counterFlag)
      grDispCounterBoard2(currCar);
  }

  trackMap->display(currCar, s, 0, 0, rightAnchor, TOP_ANCHOR);
}


// TODO: clean solution for cleanup.
static ssgSimpleState* cleanup[1024];
static int nstate = 0;


void grInitBoardCar(tCarElt *car)
{
  char    buf[4096];
  int     index;
  void    *handle;
  const char  *param;
  grssgLoaderOptions  options;
  tgrCarInfo    *carInfo;
  tgrCarInstrument  *curInst;
  tdble   xSz, ySz, xpos, ypos;
  tdble   needlexSz, needleySz;
  int lg;
  const bool bTemplate = strlen(car->_carTemplate) != 0;
  
  grssgSetCurrentOptions ( &options ) ;
  
  index = car->index; /* current car's index */
  carInfo = &grCarInfo[index];
  handle = car->_carHandle;
  
  /* Set tachometer/speedometer textures search path :
	 1) driver level specified, in the user settings,
     2) driver level specified,
	 3) car level specified,
	 4) common textures */
  param = GfParmGetStr(handle, SECT_GROBJECTS, PRM_TACHO_TEX, "rpm8000.png");
  grFilePath = (char*)malloc(4096);
  lg = 0;
  lg += snprintf(grFilePath + lg, 4096 - lg, "%sdrivers/%s/%s;", GetLocalDir(), car->_modName, car->_carName);
  if (bTemplate)
    lg += snprintf(grFilePath + lg, 4096 - lg, "%sdrivers/%s/%s;", GetLocalDir(), car->_modName, car->_carTemplate);
  lg += snprintf(grFilePath + lg, 4096 - lg, "drivers/%s/%d/%s;", car->_modName, car->_driverIndex, car->_carName);
  if (bTemplate)
    lg += snprintf(grFilePath + lg, 4096 - lg, "drivers/%s/%d/%s;", car->_modName, car->_driverIndex, car->_carTemplate);
  lg += snprintf(grFilePath + lg, 4096 - lg, "drivers/%s/%d;", car->_modName, car->_driverIndex);
  lg += snprintf(grFilePath + lg, 4096 - lg, "drivers/%s/%s;", car->_modName, car->_carName);
  if (bTemplate)
    lg += snprintf(grFilePath + lg, 4096 - lg, "drivers/%s/%s;", car->_modName, car->_carTemplate);
  lg += snprintf(grFilePath + lg, 4096 - lg, "drivers/%s;", car->_modName);
  lg += snprintf(grFilePath + lg, 4096 - lg, "cars/%s;", car->_carName);
  if (bTemplate)
    lg += snprintf(grFilePath + lg, 4096 - lg, "cars/%s;", car->_carTemplate);
  lg += snprintf(grFilePath + lg, 4096 - lg, "data/textures");

  /* Tachometer --------------------------------------------------------- */
  curInst = &(carInfo->instrument[0]);
  
  /* Load the Tachometer texture */
  curInst->texture = (ssgSimpleState*)grSsgLoadTexState(param);
  if (curInst->texture == 0) {
    curInst->texture = (ssgSimpleState*)grSsgLoadTexState("rpm8000.rgb");
  }
  
  cleanup[nstate] = curInst->texture;
  nstate++;
  
  /* Load the instrument placement */
  xSz = GfParmGetNum(handle, SECT_GROBJECTS, PRM_TACHO_XSZ, (char*)NULL, 128);
  ySz = GfParmGetNum(handle, SECT_GROBJECTS, PRM_TACHO_YSZ, (char*)NULL, 128);

  // position are delta from center of screen
  xpos = GfParmGetNum(handle, SECT_GROBJECTS, PRM_TACHO_XPOS, (char*)NULL, 0 - xSz);
  ypos = GfParmGetNum(handle, SECT_GROBJECTS, PRM_TACHO_YPOS, (char*)NULL, 0);
  needlexSz = GfParmGetNum(handle, SECT_GROBJECTS, PRM_TACHO_NDLXSZ, (char*)NULL, 50);
  needleySz = GfParmGetNum(handle, SECT_GROBJECTS, PRM_TACHO_NDLYSZ, (char*)NULL, 2);
  curInst->needleXCenter = GfParmGetNum(handle, SECT_GROBJECTS, PRM_TACHO_XCENTER, (char*)NULL, xSz / 2.0) + xpos;
  curInst->needleYCenter = GfParmGetNum(handle, SECT_GROBJECTS, PRM_TACHO_YCENTER, (char*)NULL, ySz / 2.0) + ypos;
  curInst->digitXCenter = GfParmGetNum(handle, SECT_GROBJECTS, PRM_TACHO_XDIGITCENTER, (char*)NULL, xSz / 2.0) + xpos;
  curInst->digitYCenter = GfParmGetNum(handle, SECT_GROBJECTS, PRM_TACHO_YDIGITCENTER, (char*)NULL, 10) + ypos;
  curInst->minValue = GfParmGetNum(handle, SECT_GROBJECTS, PRM_TACHO_MINVAL, (char*)NULL, 0);
  curInst->maxValue = GfParmGetNum(handle, SECT_GROBJECTS, PRM_TACHO_MAXVAL, (char*)NULL, RPM2RADS(10000)) - curInst->minValue;
  curInst->minAngle = GfParmGetNum(handle, SECT_GROBJECTS, PRM_TACHO_MINANG, "deg", 225);
  curInst->maxAngle = GfParmGetNum(handle, SECT_GROBJECTS, PRM_TACHO_MAXANG, "deg", -45) - curInst->minAngle;
  curInst->monitored = &(car->_enginerpm);
  curInst->prevVal = curInst->minAngle;

  /* Get colour to use for needle, default is Red */
  curInst->needleColor[0] = GfParmGetNum(handle, SECT_GROBJECTS, PRM_NEEDLE_RED, (char*)NULL, 1.0);
  curInst->needleColor[1] = GfParmGetNum(handle, SECT_GROBJECTS, PRM_NEEDLE_GREEN, (char*)NULL, 0.0);
  curInst->needleColor[2] = GfParmGetNum(handle, SECT_GROBJECTS, PRM_NEEDLE_BLUE, (char*)NULL, 0.0);
  curInst->needleColor[3] = GfParmGetNum(handle, SECT_GROBJECTS, PRM_NEEDLE_ALPHA, (char*)NULL, 1.0);
  
  curInst->CounterList = glGenLists(1);
  glNewList(curInst->CounterList, GL_COMPILE);
  glBegin(GL_TRIANGLE_STRIP);
  {
    glColor4f(1.0, 1.0, 1.0, 0.0);
    glTexCoord2f(0.0, 0.0); glVertex2f(xpos, ypos);
    glTexCoord2f(0.0, 1.0); glVertex2f(xpos, ypos + ySz);
    glTexCoord2f(1.0, 0.0); glVertex2f(xpos + xSz, ypos);
    glTexCoord2f(1.0, 1.0); glVertex2f(xpos + xSz, ypos + ySz);
  }
  glEnd();
  glEndList();
  
  curInst->needleList = glGenLists(1);
  glNewList(curInst->needleList, GL_COMPILE);
  glBegin(GL_TRIANGLE_STRIP);
  {
    glColor4f(curInst->needleColor[0], curInst->needleColor[1], curInst->needleColor[2], curInst->needleColor[3]);
    glVertex2f(0, -needleySz);
    glVertex2f(0, needleySz);
    glVertex2f(needlexSz, -needleySz / 2.0);
    glVertex2f(needlexSz, needleySz / 2.0);
  }
  glEnd();
  glEndList();
  
  
  /* Speedometer ----------------------------------------------------------- */
  curInst = &(carInfo->instrument[1]);
  
  /* Load the Speedometer texture */
  param = GfParmGetStr(handle, SECT_GROBJECTS, PRM_SPEEDO_TEX, "speed360.png");

  curInst->texture = (ssgSimpleState*)grSsgLoadTexState(param);
  if (curInst->texture == 0) {
    curInst->texture = (ssgSimpleState*)grSsgLoadTexState("speed360.rgb");
  }

  free(grFilePath);
  
  cleanup[nstate] = curInst->texture;
  nstate++;
  
  /* Load the intrument placement */
  xSz = GfParmGetNum(handle, SECT_GROBJECTS, PRM_SPEEDO_XSZ, (char*)NULL, 128);
  ySz = GfParmGetNum(handle, SECT_GROBJECTS, PRM_SPEEDO_YSZ, (char*)NULL, 128);

  // position are delta from center of screen
  xpos = GfParmGetNum(handle, SECT_GROBJECTS, PRM_SPEEDO_XPOS, (char*)NULL, 0);
  ypos = GfParmGetNum(handle, SECT_GROBJECTS, PRM_SPEEDO_YPOS, (char*)NULL, 0);
  needlexSz = GfParmGetNum(handle, SECT_GROBJECTS, PRM_SPEEDO_NDLXSZ, (char*)NULL, 50);
  needleySz = GfParmGetNum(handle, SECT_GROBJECTS, PRM_SPEEDO_NDLYSZ, (char*)NULL, 2);
  curInst->needleXCenter = GfParmGetNum(handle, SECT_GROBJECTS, PRM_SPEEDO_XCENTER, (char*)NULL, xSz / 2.0) + xpos;
  curInst->needleYCenter = GfParmGetNum(handle, SECT_GROBJECTS, PRM_SPEEDO_YCENTER, (char*)NULL, ySz / 2.0) + ypos;
  curInst->digitXCenter = GfParmGetNum(handle, SECT_GROBJECTS, PRM_SPEEDO_XDIGITCENTER, (char*)NULL, xSz / 2.0) + xpos;
  curInst->digitYCenter = GfParmGetNum(handle, SECT_GROBJECTS, PRM_SPEEDO_YDIGITCENTER, (char*)NULL, 10) + ypos; 
  curInst->minValue = GfParmGetNum(handle, SECT_GROBJECTS, PRM_SPEEDO_MINVAL, (char*)NULL, 0);
  curInst->maxValue = GfParmGetNum(handle, SECT_GROBJECTS, PRM_SPEEDO_MAXVAL, (char*)NULL, 100) - curInst->minValue;
  curInst->minAngle = GfParmGetNum(handle, SECT_GROBJECTS, PRM_SPEEDO_MINANG, "deg", 225);
  curInst->maxAngle = GfParmGetNum(handle, SECT_GROBJECTS, PRM_SPEEDO_MAXANG, "deg", -45) - curInst->minAngle;
  curInst->monitored = &(car->_speed_x);
  curInst->prevVal = curInst->minAngle;
  if (strcmp(GfParmGetStr(handle, SECT_GROBJECTS, PRM_SPEEDO_DIGITAL, "yes"), "yes") == 0) {
    curInst->digital = 1;
  }
  
  /* Get colour to use for needle, default is Red */
  curInst->needleColor[0] = GfParmGetNum(handle, SECT_GROBJECTS, PRM_NEEDLE_RED, (char*)NULL, 1.0);
  curInst->needleColor[1] = GfParmGetNum(handle, SECT_GROBJECTS, PRM_NEEDLE_GREEN, (char*)NULL, 0.0);
  curInst->needleColor[2] = GfParmGetNum(handle, SECT_GROBJECTS, PRM_NEEDLE_BLUE, (char*)NULL, 0.0);
  curInst->needleColor[3] = GfParmGetNum(handle, SECT_GROBJECTS, PRM_NEEDLE_ALPHA, (char*)NULL, 1.0);
  
  curInst->CounterList = glGenLists(1);
  glNewList(curInst->CounterList, GL_COMPILE);
  glBegin(GL_TRIANGLE_STRIP);
  {
    glColor4f(1.0, 1.0, 1.0, 0.0);
    glTexCoord2f(0.0, 0.0); glVertex2f(xpos, ypos);
    glTexCoord2f(0.0, 1.0); glVertex2f(xpos, ypos + ySz);
    glTexCoord2f(1.0, 0.0); glVertex2f(xpos + xSz, ypos);
    glTexCoord2f(1.0, 1.0); glVertex2f(xpos + xSz, ypos + ySz);
  }
  glEnd();
  glEndList();
  
  curInst->needleList = glGenLists(1);
  glNewList(curInst->needleList, GL_COMPILE);
  glBegin(GL_TRIANGLE_STRIP);
  {
    glColor4f(curInst->needleColor[0], curInst->needleColor[1], curInst->needleColor[2], curInst->needleColor[3]);
    glVertex2f(0, -needleySz);
    glVertex2f(0, needleySz);
    glVertex2f(needlexSz, -needleySz / 2.0);
    glVertex2f(needlexSz, needleySz / 2.0);
  }
  glEnd();
  glEndList();
  
}

void grShutdownBoardCar(void)
{
  /*int i;
  for (i = 0; i < nstate; i++) {
    printf("%d\n", i);
    if (cleanup[i]->getRef() > 0) {
      ssgDeRefDelete(cleanup[i]);
    } else {
      delete cleanup[i];
    }
  }
  nstate = 0;*/
}

#define LEADERBOARD_SCROLL_TIME 2.0
/**
 * grDispLeaderBoardScroll
 * 
 * Displays the leaderboard in a vertical scrolled fashion,
 * if there are more than 10 names to display.
 * 
 * @param car pointer to the currently displayed car
 * @param s current situation, provided by the sim
*/
void
cGrBoard::grDispLeaderBoardScroll(const tCarElt *car, const tSituation *s) const
{
  //Scrolling
  if(iTimer == 0 || s->currentTime < iTimer) iTimer = s->currentTime;
  if(s->currentTime >= iTimer + LEADERBOARD_SCROLL_TIME)
    {
      iTimer = s->currentTime;
      ++iStart;
      iStart = iStart % (s->_ncars + 1);  //Limit: number of cars + one separator line
    }
    
  char buf[256];

  int current = 0;  //Position of the currently displayed car
  for(int i = 0; i < s->_ncars; i++) {
    if (car == s->cars[i]) {
      current = i;
      break;
    }
  }//for i

  //Coords, limits
  int x = leftAnchor + 5;
  int x2 = x + 165;
  int y = BOTTOM_ANCHOR + 10;

  int dy = GfuiFontHeight(GFUI_FONT_SMALL_C);
  int maxLines = MIN(leaderNb, s->_ncars);  //Max # of lines to display (# of cars in race or max 10 by default)
  
  //Set up drawing area
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA) ;
  glBegin(GL_QUADS);
  glColor4f(0.1, 0.1, 0.1, 0.8);
  glVertex2f(x, y + 5);
  glVertex2f(x + 175, y - 5);
  glVertex2f(x + 175, y + dy * (maxLines + 1));
  glVertex2f(x, y + dy * (maxLines + 1));
  glEnd();
  glDisable(GL_BLEND);


  //The board is built from bottom up:
  //driver position #10/current is drawn first,
  //then from #9 onto #1, then the text 'Laps' if drawLaps requires.
  for(int j = maxLines - 1; j >= 0; j--) {
    int i = j + iStart; //index of driver to be displayed

    if(i == s->_ncars) {
      //print empty line after last car
    } else {
      i = i % (s->_ncars + 1);

      //Set colour of the drivers to that
      //defined in the drivers' XML file.
      //Current driver is yellow...
      float *clr = (i == current)
        ? grDefaultClr : grCarInfo[s->cars[i]->index].iconColor;

      //Driver position + name
      snprintf(buf, sizeof(buf), "%3d: %s", i + 1, s->cars[i]->_name);
      GfuiPrintString(buf, clr, GFUI_FONT_SMALL_C, x, y, GFUI_ALIGN_HL_VB);
      
      //Display driver time / time behind leader / laps behind leader
      string sEntry = grGenerateLeaderBoardEntry(s->cars[i], s, (i==0));
      if(s->cars[i]->_state & RM_CAR_STATE_DNF
        || s->cars[i]->_state & RM_CAR_STATE_PIT) { //driver DNF or in pit, show 'out' in red
        clr = grRed;
      }
      GfuiPrintString(sEntry.c_str(), clr, GFUI_FONT_SMALL_C, x2, y, GFUI_ALIGN_HR_VB);
    }//else i
    y += dy;  //'Line feed'
  }//for j

  //Write 'Lap X/Y' on top of the leader board
  if (s->currentTime < s->_totTime)
  {
    GfuiPrintString(" Laps:", grWhite, GFUI_FONT_SMALL_C, x, y, GFUI_ALIGN_HL_VB);
    snprintf(buf, sizeof(buf), "%d", s->cars[0]->_laps);
    GfuiPrintString(buf, grWhite, GFUI_FONT_SMALL_C, x2, y, GFUI_ALIGN_HR_VB);
  }
  else
  {
    GfuiPrintString(" Lap:", grWhite, GFUI_FONT_SMALL_C, x, y, GFUI_ALIGN_HL_VB);
    snprintf(buf, sizeof(buf), "%d / %d", s->cars[0]->_laps, s->_totLaps);
    GfuiPrintString(buf, grWhite, GFUI_FONT_SMALL_C, x2, y, GFUI_ALIGN_HR_VB);
  }
}//grDispLeaderBoardScroll


#define LEADERBOARD_LINE_SCROLL_TIME 0.07
/** 
 * grDispLeaderBoardScrollLine
 * 
 * Scrolls the leaderboard on the bottom line, as seen on TV broadcasts.
 * 
 * @param car pointer to the currently displayed car
 * @param s current situation, provided by the sim
 */
void
cGrBoard::grDispLeaderBoardScrollLine(const tCarElt *car, const tSituation *s)
{
  //At the start of the race or when first engaging this mode,
  //we generate the 3-letter names to be used
  if(iTimer == 0 || iStringStart == 0 || sShortNames.size() == 0)
    grMakeThreeLetterNames(s);
  
  //At first, get the current time
  if(iTimer == 0 || s->currentTime < iTimer)
    iTimer = s->currentTime;
    
  //Scrolling needed?
  if(s->currentTime >= iTimer + LEADERBOARD_LINE_SCROLL_TIME) {
    //When in initial position, show it fixed (no scroll) for some secs.
    if((iStringStart == 0 && s->currentTime >= iTimer + LEADERBOARD_LINE_SCROLL_TIME * 20)
      || (iStringStart > 0)) {
        iTimer = s->currentTime;
        ++iStringStart;
      }//if iStringStart
  }//if currentTime
    
  //Coords, limits
  int x = leftAnchor + 5;
  int x2 = rightAnchor - 5;
  int y = BOTTOM_ANCHOR;

  int dy = GfuiFontHeight(GFUI_FONT_MEDIUM_C);
  //int dx = GfuiFontWidth(GFUI_FONT_SMALL_C, "W");
  //int iLen = (x2 - x) / dx; //# of chars to display
  
  //Set up drawing area
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA) ;
  glBegin(GL_QUADS);
  glColor4f(0.1, 0.1, 0.1, 0.8);
  glVertex2f(x, y);
  glVertex2f(x2, y);
  glVertex2f(x2, y + dy);
  glVertex2f(x, y + dy);
  glEnd();
  glDisable(GL_BLEND);

  //Are we at the end of the scrolled string? If yes, let's regenerate it
  if(st.empty() || (iStringStart == (int)st.size())) {
    st.clear();
    /*!The roster holds the driver's position, name and difference
     * *at the time* the leader starts a new lap.
     * So it can happen it is somewhat mixed up, it will settle down
     * in the next lap.
    */ 
    
    ostringstream osRoster;
    //Add the track name as separator, embedded with 3 spaces each side.
    osRoster << "   " << grTrack->name << "   ";
    //Add # of laps
    osRoster << "Lap " << s->cars[0]->race.laps << " | ";
    for(int i = 0; i < s->_ncars; i++) {
      //Driver position + name
      osRoster.width(3);
      osRoster << (i + 1);
      osRoster << ": " << sShortNames[i];

      //Display driver time / time behind leader / laps behind leader
      string sEntry = grGenerateLeaderBoardEntry(s->cars[i], s, (i==0));

      //get rid of leading spaces
      size_t iCut = sEntry.find_first_not_of(' ');
      if(iCut != string::npos && iCut != 0) {
        sEntry = sEntry.substr(iCut - 1);
      }//if iCut
      //Add to roster, then separate it from next one
      osRoster << sEntry << "   ";
    }//for i

    st.assign(osRoster.str());
  }//if st.empty || iStringStart > size
  
  //Display the line
  GfuiPrintString(st.c_str() + iStringStart, grWhite, GFUI_FONT_MEDIUM_C, x, y, GFUI_ALIGN_HL_VB);
  iStringStart = iStringStart % st.size();
}//grDispLeaderBoardScrollLine


/** 
 * name: grGenerateLeaderBoardEntry
 * 
 * Generates one leaderboard entry,
 * this time only the time behind/laps behind part.
 * Optimally it would make up the whole string that can be shown
 * in the line, but colour handling and positioning
 * is not available then.
 * 
 * @param car
 * @param isLeader
 * @return string
 */
string
cGrBoard::grGenerateLeaderBoardEntry(const tCarElt* car, const tSituation* s, const bool isLeader) const
{
  string ret;
  char buf[256];
  
  //Display driver time / time behind leader / laps behind leader
  if(car->_state & RM_CAR_STATE_DNF)  {
    //driver DNF
    snprintf(buf, sizeof(buf), "       out");
  }
  else if(car->_state & RM_CAR_STATE_PIT) {
    //driver in pit
    snprintf(buf, sizeof(buf), "       PIT");
  } else {
    //no DNF nor in pit
    if(isLeader) {
      //This is the leader, put out his time
      if (s->_raceType == RM_TYPE_RACE || s->_ncars <= 1)
        grWriteTimeBuf(buf, car->_curTime, 0);
      else
      {
        if (car->_bestLapTime > 0)
          grWriteTimeBuf(buf, car->_bestLapTime, 0);
        else
          snprintf(buf, sizeof(buf), "       --:--");
      }
    } else {
      //This is not the leader
      switch(car->_lapsBehindLeader) {
        case 0: //Driver in same lap as leader
          if (car->_timeBehindLeader == 0 && (s->_raceType == RM_TYPE_RACE || car->_bestLapTime <= 0.0f)) {
            //Cannot decide time behind, first lap or passed leader
            snprintf(buf, sizeof(buf), "       --:--");
          }
          else {
            //Can decide time behind
            grWriteTimeBuf(buf, car->_timeBehindLeader, 1);
          }
          break;
          
        case 1: //1 lap behind leader
          snprintf(buf, sizeof(buf), "+%3d Lap", car->_lapsBehindLeader);
          break;
          
        default:  //N laps behind leader
          snprintf(buf, sizeof(buf), "+%3d Laps", car->_lapsBehindLeader);
          break;
      }//switch 
    }//not leader
  }//driver not DNF or in pit
  ret.assign(buf);
  
  return ret;
}//grGenerateLeaderBoardEntry


/** 
 * grMakeThreeLetterNames
 * 
 * Let's build an array of 3-letter name abbreviations (sShortNames).
 * As we follow original name order, this array can be indexed
 * the same as the original names' array.
 * 
 * @param s situation provided by SD
 */
void
cGrBoard::grMakeThreeLetterNames(const tSituation *s)
{
  //Build up two arrays of the original and the short names, same order.
  sShortNames.clear();
  vector<string> origNames;
  for(int i = 0; i < s->_ncars; i++) {  //Loop over each name in the race
    string st(s->cars[i]->_name);
    origNames.push_back(st);
    remove(st.begin(), st.end(), ' ');  //Remove spaces
    sShortNames.push_back(st.substr(0, 3)); //Cut to 3 characters
  }//for i
  vector<string> origShortNames(sShortNames); //Copy to hold original 3-letter names, for search

  //Check for matching names
  for(unsigned int i = 0; i < sShortNames.size(); i++) {
    string sSearch(origShortNames[i]);
    for(unsigned int j = i + 1; j < sShortNames.size(); j++) {
      if(sSearch == origShortNames[j]) {  //Same 3-letter name found
        //Let's find the first mismatching character
        unsigned int k;
        for(k = 0; k < min(origNames[i].size(), origNames[j].size()); k++) {
          if(origNames[i][k] != origNames[j][k]) break;
        }//for k
        //Set 3rd char of the short name to the mismatching char (or last one).
        //It is the driver designer's responsibility from now on
        //to provide some unsimilarities between driver names.
        sShortNames[i][2] = origNames[i][k];
        sShortNames[j][2] = origNames[j][k];
      }//if sSearch
    }//for j
  }//for i
  //3-letter name array ready to use!
}//grMakeThreeLetterName
