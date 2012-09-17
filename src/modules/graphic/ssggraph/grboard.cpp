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

#include "src/modules/graphic/ssggraph/grboard.h"

#include <plib/ssg.h>
#include <portability.h>    // snprintf
#include <glfeatures.h>
#include <robottools.h>     // RELAXATION

#include <algorithm>        // remove
#include <sstream>
#include <vector>
#include <string>

#include "src/modules/graphic/ssggraph/grmain.h"       // grWinX, grHandle, grMaxDamage
#include "src/modules/graphic/ssggraph/grtrackmap.h"   // cGrTrackMap
#include "src/modules/graphic/ssggraph/grcar.h"        // grCarInfo
#include "src/modules/graphic/ssggraph/grutil.h"       // grWriteTime
#include "src/modules/graphic/ssggraph/grloadac.h"     // grssgSetCurrentOptions

using std::string;

static float grWhite[4] = {1.0, 1.0, 1.0, 1.0};
static float grRed[4] = {1.0, 0.0, 0.0, 1.0};
static float grBlue[4] = {0.0, 0.0, 1.0, 1.0};
static float grGreen[4] = {0.0, 1.0, 0.0, 1.0};
static float grBlack[4] = {0.0, 0.0, 0.0, 1.0};
static float grPink[4] = {1.0, 0.0, 1.0, 1.0};
static float grGrey[4] = {0.3, 0.3, 0.3, 1.0};
static float grYellow[4] = {1.0, 0.878, 0.0, 1.0};      // ffe000
static float grCyan[4] = {0.31, 0.968, 0.933, 1.0};     // 4ff7ee
static float grDefaultClr[4] = {0.9, 0.9, 0.15, 1.0};
// darkblue to fit the menu style: 0a162f
static float grBackground[4] = {0.039, 0.086, 0.184, 0.8};

static const int NB_BOARDS = 3;
static const int NB_LBOARDS = 5;    // # of leaderboard states
static const int NB_DEBUG = 4;

// Boards work on a OrthoCam with fixed height of 600, width flows
// with split screen(s) and can be limited to 'board width' % of screen
static const int TOP_ANCHOR = 600;
static const int BOTTOM_ANCHOR = 0;
static const int DEFAULT_WIDTH = 800;

static const int BUFSIZE = 256;


cGrBoard::cGrBoard(int myid)
{
  id = myid;
  trackMap = 0;

  // Scrolling leaderboard variables
  iStart = 0;
  iTimer = 0.0;
  iStringStart = 0;
}


cGrBoard::~cGrBoard()
{
  delete trackMap;
  trackMap = 0;
}


void
cGrBoard::loadDefaults(const tCarElt *curCar)
{
  char path[1024];
  snprintf(path, sizeof(path), "%s/%d", GR_SCT_DISPMODE, id);

  debugFlag = (int)GfParmGetNum(grHandle, path, GR_ATT_DEBUG, NULL, 1);
  boardFlag = (int)GfParmGetNum(grHandle, path, GR_ATT_BOARD, NULL, 2);
  leaderFlag  = (int)GfParmGetNum(grHandle, path, GR_ATT_LEADER, NULL, 1);
  leaderNb  = (int)GfParmGetNum(grHandle, path, GR_ATT_NBLEADER, NULL, 10);
  counterFlag = (int)GfParmGetNum(grHandle, path, GR_ATT_COUNTER, NULL, 1);
  GFlag = (int)GfParmGetNum(grHandle, path, GR_ATT_GGRAPH, NULL, 1);
  arcadeFlag  = (int)GfParmGetNum(grHandle, path, GR_ATT_ARCADE, NULL, 0);
  boardWidth  = (int)GfParmGetNum(grHandle, path, GR_ATT_BOARDWIDTH, NULL, 100);
  speedoRise  = (int)GfParmGetNum(grHandle, path, GR_ATT_SPEEDORISE, NULL, 0);

  trackMap->setViewMode((int)GfParmGetNum(grHandle, path, GR_ATT_MAP,
                                NULL, trackMap->getDefaultViewMode()));

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
    trackMap->setViewMode((int)GfParmGetNum(grHandle, path, GR_ATT_MAP,
                                NULL, trackMap->getViewMode()));
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
  char path[1024];
  snprintf(path, sizeof(path), "%s/%d", GR_SCT_DISPMODE, id);

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
 * 1 - Display the mean and instant FPS
 * 2 - Like 2 + the absolute frame counter
 * 3 - Like 2 + the segment the car is on, car's distance from startline, current camera
 *
 * @param s The current situation
 * @param car The current car
 * @param frame Frame info to display
 */
void
cGrBoard::grDispDebug(const tSituation *s, const tCarElt *car,
                        const cGrFrameInfo* frame)
{
  int x = rightAnchor - 100;
  int y = TOP_ANCHOR - 15;
  int dy = GfuiFontHeight(GFUI_FONT_SMALL_C);

  // Display frame rates (instant and average)
  char buf[BUFSIZE];
  snprintf(buf, sizeof(buf), "FPS: %.1f(%.1f)",
            frame->fInstFps, frame->fAvgFps);
  GfuiDrawString(buf, grWhite, GFUI_FONT_SMALL_C, x, y);

  if (debugFlag == 2) {
    //Only display detailed information in Debug Mode > 1
    // Display frame counter
    y -= dy;
    snprintf(buf, sizeof(buf),  "Frm: %u", frame->nTotalFrames);
    GfuiDrawString(buf, grWhite, GFUI_FONT_SMALL_C, x, y);

    // Display simulation time
    y -= dy;
    snprintf(buf, sizeof(buf),  "Time: %.f", s->currentTime);
    GfuiDrawString(buf, grWhite, GFUI_FONT_SMALL_C, x, y);

  } else if (debugFlag == 3) {
    // Only display detailed information in Debug Mode > 1
    // Display segment name
    y -= dy;
    snprintf(buf, sizeof(buf),  "Seg: %s", car->_trkPos.seg->name);
    GfuiDrawString(buf, grWhite, GFUI_FONT_SMALL_C, x, y);

    // Display distance from start
    y -= dy;
    snprintf(buf, sizeof(buf), "DfS: %5.0f", car->_distFromStartLine);
    GfuiDrawString(buf, grWhite, GFUI_FONT_SMALL_C, x, y);

    // Display current camera
    y -= dy;
    tRoadCam *curCam = car->_trkPos.seg->cam;
    if (curCam) {
      snprintf(buf, sizeof(buf), "Cam: %s", curCam->name);
      GfuiDrawString(buf, grWhite, GFUI_FONT_SMALL_C, x, y);
    }
  }
}  // grDispDebug


void
cGrBoard::grDispGGraph(const tCarElt *car)
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
  static const tdble THNSS = 2.0f;

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
    if (fabs(car->_speed_x)
        - fabs(car->_wheelSpinVel(xx) * car->_wheelRadius(xx)) >  5.0f)
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
  glVertex2f(X1 - car->ctrl.steer * 50.0f, Y1 + THNSS);
  glVertex2f(X1 - car->ctrl.steer * 50.0f, Y1 - THNSS);

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
cGrBoard::grDrawGauge(tdble X1, tdble Y1, tdble H,
                        float *clr1, float *clr2,
                        tdble val, const char *title)
{
  tdble curH = MAX(MIN(val, 1.0), 0.0);
  curH *= H;

  static const tdble THNSSBG = 2.0;
  static const tdble THNSSFG = 2.0;

  glBegin(GL_QUADS);
  // set F and D column to: 404040
  glColor4f(0.25, 0.25, 0.25, 0.8);
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
  GfuiDrawString((char *)title, grYellow, GFUI_FONT_MEDIUM,
                    (int)(X1 - (THNSSBG + THNSSFG)),
                    (int)(Y1 - THNSSBG - GfuiFontHeight(GFUI_FONT_MEDIUM)),
                    2*(THNSSBG + THNSSFG), GFUI_ALIGN_HC);
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


//
// grDispCarBoard1
//
// Displays driver-specific info (version 1)
//
// @param s[in] A pointer to the current situation
// @param car[in] A pointer to the current driver's car
// @return void
//
void
cGrBoard::grDispCarBoard1(const tCarElt *car, const tSituation *s)
{
  char buf[BUFSIZE];

  // Populate buf to get the width of the drawing area
  snprintf(buf, sizeof(buf), "%s: %d/%d", car->_name, car->_pos, s->_ncars);

  int dy = GfuiFontHeight(GFUI_FONT_MEDIUM_C);
  static const int dy2 = GfuiFontHeight(GFUI_FONT_SMALL_C);
  int dx = GfuiFontWidth(GFUI_FONT_MEDIUM_C, buf);
  static const int dxc = 60;

  int x = leftAnchor + 15;                      // constant text left pos.
  int y = BOTTOM_ANCHOR + dy2 * 8 + dy + 5;     // first row top pos.
  // (counting from the bottom, we have 8+1 rows to display)

  const int x2 = x + 40;    // volatile text left pos.
  const int xr = x2 + dxc;
  dx = MAX(dx, (xr - x));

  // Display board
  // We have 8 rows with small font and 1 with middle
  grSetupDrawingArea(x - 5, y + dy + 5, x + dx + 5, y - dy2 * 8 - dy + 5);

  // Display driver name and race position (in medium font size)
  GfuiDrawString(buf, grYellow, GFUI_FONT_MEDIUM_C, x, y);
  y -= dy;

  // From now on we use small font
  dy = GfuiFontHeight(GFUI_FONT_SMALL_C);

  // Display fuel
  GfuiDrawString("Fuel:", grWhite, GFUI_FONT_SMALL_C, x, y);
  float *clr = (car->_fuel < 5.0) ? grRed : grWhite;  // Display low fuel in red
  snprintf(buf, sizeof(buf), "%.1f l", car->_fuel);
  GfuiDrawString(buf, clr, GFUI_FONT_SMALL_C, x2, y, dxc, GFUI_ALIGN_HR);
  y -= dy;

  // Display damage
  clr = (car->_state & RM_CAR_STATE_BROKEN) ? grRed: grWhite;
  GfuiDrawString("Damage:", clr, GFUI_FONT_SMALL_C, x, y);
  snprintf(buf, sizeof(buf),  "%d", car->_dammage);
  GfuiDrawString(buf, clr, GFUI_FONT_SMALL_C, x2, y, dxc, GFUI_ALIGN_HR);
  y -= dy;

  // Display lap counter
  clr = grWhite;
  char const* lapsTimeLabel;
  grGetLapsTime(s, car, buf, &lapsTimeLabel);
  GfuiDrawString(lapsTimeLabel, clr, GFUI_FONT_SMALL_C, x, y);
  GfuiDrawString(buf, clr, GFUI_FONT_SMALL_C, x2, y, dxc, GFUI_ALIGN_HR);
  y -= dy;

  // Display total race time
  GfuiDrawString("Total:", clr, GFUI_FONT_SMALL_C, x, y);
  grWriteTime(clr, GFUI_FONT_SMALL_C, x2, y, dxc, s->currentTime, 0);
  y -= dy;

  // Display current lap time
  GfuiDrawString("Curr:", clr, GFUI_FONT_SMALL_C, x, y);
  grWriteTime(clr, GFUI_FONT_SMALL_C, x2, y, dxc, car->_curLapTime, 0);
  y -= dy;

  // Display last lap time
  GfuiDrawString("Last:", clr, GFUI_FONT_SMALL_C, x, y);
  grWriteTime(clr, GFUI_FONT_SMALL_C, x2, y, dxc, car->_lastLapTime, 0);
  y -= dy;

  // Display best lap time
  GfuiDrawString("Best:", clr, GFUI_FONT_SMALL_C, x, y);
  grWriteTime(clr, GFUI_FONT_SMALL_C, x2, y, dxc, car->_bestLapTime, 0);
  y -= dy;

  // Display top speed
  GfuiDrawString("Top Speed:", clr, GFUI_FONT_SMALL_C, x, y);
  snprintf(buf, sizeof(buf), "%d", (int)(car->_topSpeed * 3.6));
  GfuiDrawString(buf, clr, GFUI_FONT_SMALL_C, x2, y, dxc, GFUI_ALIGN_HR);
  y -= dy;
}  // grDispCarBoard1


//
// grDispCarBoard2
//
// Displays driver-specific info (version 2)
//
// @param s[in] A pointer to the current situation
// @param car[in] A pointer to the current driver's car
// @return void
//
void
cGrBoard::grDispCarBoard2(const tCarElt *car, const tSituation *s)
{
  // Font sizes
  int dy = GfuiFontHeight(GFUI_FONT_MEDIUM_C);
  static const int dy2 = GfuiFontHeight(GFUI_FONT_SMALL_C);
  static const int dxc = 60;

  const int x = leftAnchor + 15;    // constant text 1 left pos.
  const int x2 = x + 40;            // volatile text 1 left pos.
  const int x3 = x2 + dxc;          // constant text 2 left pos.
  const int xr = x3 + dxc;          // volatile text 2 left pos.
  int y = BOTTOM_ANCHOR + 8 * dy2 + dy + 5;      // first row top pos.
  // (counting from the bottom, we have 6+2 rows to display)

  // Populate buf to get the width of the drawing area
  char buf[BUFSIZE];
  snprintf(buf, sizeof(buf), "%s: %d/%d", car->_name, car->_pos, s->_ncars);
  const int dx = MAX(GfuiFontWidth(GFUI_FONT_MEDIUM_C, buf), (xr - x));

  // Display board
  // We have 8 rows with small font and 1 with middle
  grSetupDrawingArea(x - 5, y + dy + 5, x + dx + 5, y - dy2 * 8 - dy + 5);

  // Display driver name and race position (in medium font size)
  GfuiDrawString(buf, grYellow, GFUI_FONT_MEDIUM_C, x, y);
  y -= dy;

  // From now on we use small font
  dy = GfuiFontHeight(GFUI_FONT_SMALL_C);

  // Display fuel
  GfuiDrawString("Fuel:", grWhite, GFUI_FONT_SMALL_C, x, y);
  float *clr = (car->_fuel < 5.0) ? grRed : grWhite;
  snprintf(buf, sizeof(buf),  "%.1f l", car->_fuel);
  GfuiDrawString(buf, clr, GFUI_FONT_SMALL_C, x2, y, dxc, GFUI_ALIGN_HR);
  y -= dy;

  clr = grWhite;

  // Display lap counter
  char const *lapsTimeLabel;
  grGetLapsTime(s, car, buf, &lapsTimeLabel);
  GfuiDrawString(lapsTimeLabel, clr, GFUI_FONT_SMALL_C, x, y);
  GfuiDrawString(buf, clr, GFUI_FONT_SMALL_C, x2, y, dxc, GFUI_ALIGN_HR);
  y -= dy;

  // Display best lap time and diff of last and best lap
  GfuiDrawString("Best:", clr, GFUI_FONT_SMALL_C, x, y);
  grWriteTime(clr, GFUI_FONT_SMALL_C, x2, y, dxc, car->_bestLapTime, 0);
  grWriteTime(clr, GFUI_FONT_SMALL_C, x3, y, dxc, car->_deltaBestLapTime, 1);
  y -= dy;

  // Display current lap time and split times
  GfuiDrawString("Time:", clr, GFUI_FONT_SMALL_C, x, y);
  grWriteTime(clr, GFUI_FONT_SMALL_C, x2, y, dxc, car->_curLapTime, 0);
  double time;
  if (grGetSplitTime(s, car, false, time, NULL, &clr))
    grWriteTime(clr, GFUI_FONT_SMALL_C, x3, y, dxc, time, 1);
  clr = grWhite;
  y -= dy;
  y -= dy;

  // Display car ahead and diff
  clr = grCyan;
  if (car->_pos != 1) {
    snprintf(buf, sizeof(buf), "%s", s->cars[car->_pos - 2]->_name);
    GfuiDrawString(buf, clr, GFUI_FONT_SMALL_C, x, y);
    if (s->_raceType == RM_TYPE_RACE) {
      if (s->cars[car->_pos - 2]->_laps == car->_laps) {
        grWriteTime(clr, GFUI_FONT_SMALL_C, x3, y, dxc,
                        s->cars[car->_pos - 2]->_curTime-car->_curTime, 1);
      } else {
        GfuiDrawString("--:---", clr, GFUI_FONT_SMALL_C, x3, y, dxc,
                        GFUI_ALIGN_HR);
      }
    } else {
      if (car->_bestLapTime > 0.0f) {
        grWriteTime(clr, GFUI_FONT_SMALL_C, x3, y, dxc,
                        car->_bestLapTime - s->cars[car->_pos - 2]->_bestLapTime, 1);
      } else {
        GfuiDrawString("--:---", clr, GFUI_FONT_SMALL_C, x3, y, dxc,
                        GFUI_ALIGN_HR);
      }
    }
  } else {
    GfuiDrawString(" ", clr, GFUI_FONT_SMALL_C, x, y);
    GfuiDrawString("--:---", clr, GFUI_FONT_SMALL_C, x3, y, dxc, GFUI_ALIGN_HR);
  }
  y -= dy;

  // Display car behind and diff
  clr = grWhite;
  if (car->_pos != s->_ncars) {
    snprintf(buf, sizeof(buf), "%s", s->cars[car->_pos]->_name);
    GfuiDrawString(buf, clr, GFUI_FONT_SMALL_C, x, y);
    if (s->_raceType == RM_TYPE_RACE) {
      if (s->cars[car->_pos]->_laps == car->_laps) {
        grWriteTime(clr, GFUI_FONT_SMALL_C, x3, y, dxc,
                        s->cars[car->_pos]->_curTime-car->_curTime, 1);
      } else {
        GfuiDrawString("--:---", clr, GFUI_FONT_SMALL_C, x3, y, dxc,
                        GFUI_ALIGN_HR);
      }
    } else {
      if (s->cars[car->_pos]->_bestLapTime > 0.0f) {
        grWriteTime(clr, GFUI_FONT_SMALL_C, x3, y, dxc,
                        s->cars[car->_pos]->_bestLapTime - car->_bestLapTime, 1);
      } else {
        GfuiDrawString("--:---", clr, GFUI_FONT_SMALL_C, x3, y, dxc,
                        GFUI_ALIGN_HR);
      }
    }
  } else {
    GfuiDrawString(" ", clr, GFUI_FONT_SMALL_C, x, y);
    GfuiDrawString("--:---", clr, GFUI_FONT_SMALL_C, x3, y, dxc, GFUI_ALIGN_HR);
  }
  y -= dy;

  // Display control messages
  // (lines 3 and 4 only, ABS TCS SPD goes to separate indicator)
  for (int i = 2; i < 4; ++i) {
    if (car->ctrl.msg[i]) {
      GfuiDrawString(car->ctrl.msg[i], (float*)(car->ctrl.msgColor),
                        GFUI_FONT_SMALL_C, x, y);
      y -= dy;
    }
  }
}  // grDispCarBoard2


void
cGrBoard::grDispCarBoard(const tCarElt *car, const tSituation *s)
{
  switch (boardFlag) {
    case 0:
      break;

    case 1:
      grDispCarBoard1(car, s);
      if (true)
        grDispIndicators(car);
      break;

    case 2:
      grDispCarBoard2(car, s);
      if (true)
        grDispIndicators(car);
      break;

    default:
      break;
  }
}


#define ALIGN_CENTER  0
#define ALIGN_LEFT  1
#define ALIGN_RIGHT 2

void
cGrBoard::grDispEngineLeds(const tCarElt *car, int X, int Y, int align, bool bg)
{
  // Green LED
  GLfloat ledcolg[2][3] = {
    {0.0, 0.2, 0.0},
    {0.0, 1.0, 0.0}
  };

  // Red LED
  GLfloat ledcolr[2][3] = {
    {0.2, 0.0, 0.0},
    {1.0, 0.0, 0.0}
  };

  int ledNb     = 20;
  int ledHeight = 10;
  int ledWidth  = 5;
  int ledSpace  = 2;
  int ledRed    = (int)((car->_enginerpmRedLine * 0.9 / car->_enginerpmMax) * (tdble)ledNb);
  int ledLit    = (int)((car->_enginerpm / car->_enginerpmMax) * (tdble)ledNb);

  int x;
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

  int y = Y;
  glBegin(GL_QUADS);

  // Draw background?
  if (bg) {
    glColor3f(0.1, 0.1, 0.1);
    glVertex2f(x - ledSpace, y + ledHeight + ledSpace);
    glVertex2f(x + ledNb * (ledWidth+ ledSpace), y + ledHeight + ledSpace);
    glVertex2f(x + ledNb * (ledWidth+ ledSpace), BOTTOM_ANCHOR);
    glVertex2f(x - ledSpace, BOTTOM_ANCHOR);
  }

  const int xref = x;
  glColor3fv(ledcolg[0]);
  for (int i = 0; i < ledRed; ++i) {
    glVertex2f(x, y);
    glVertex2f(x + ledWidth, y);
    glVertex2f(x + ledWidth, y + ledHeight);
    glVertex2f(x, y + ledHeight);
    x += ledWidth + ledSpace;
  }

  glColor3fv(ledcolr[0]);
  for (int i = ledRed; i < ledNb; ++i) {
    glVertex2f(x, y);
    glVertex2f(x + ledWidth, y);
    glVertex2f(x + ledWidth, y + ledHeight);
    glVertex2f(x, y + ledHeight);
    x += ledWidth + ledSpace;
  }
  x = xref;

#define DD  1
  glColor3fv(ledcolg[1]);
  for (int i = 0; i < ledNb; ++i) {
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
}  // grDispEngineLeds


/**
 * grDispLeaderBoard
 *
 * Displays the leader board (in the lower left corner of the screen)
 * If [drawLaps] is on, writes the lap counter on the top of the list.
 * @param car[in] the car currently being viewed
 * @param s[in] situation provided by the sim
 */
void
cGrBoard::grDispLeaderBoard(const tCarElt *car, const tSituation *s)
{
  double time_left;
  if (leaderFlag == 4) {
    // Scrolling line in the top of the screen
    grDispLeaderBoardScrollLine(car, s);
  } else if (leaderFlag == 3 && leaderNb < s->_ncars) {
    // This mode is only reasonable if there are more drivers
    // than the leaderboard can display at once.
    // Scrolling results in the top left corner
    grDispLeaderBoardScroll(car, s);
  } else {
    // Static leaderboard
    char buf[BUFSIZE];

    int current = 0;  // Position of the currently displayed car
    for (int i = 0; i < s->_ncars; ++i) {
      if (car == s->cars[i]) {
        current = i;
        break;
      }
    }   // for i

    // Coords, limits
    const int x = leftAnchor + 10;
    const int x2 = x + 100;
    const int dxc = 60;
    const int xr = x2 + dxc;

    const int dy = GfuiFontHeight(GFUI_FONT_SMALL_C);
    //Max # of lines to display (# of cars in race or max 10 by default)
    const int maxLines = MIN(leaderNb, s->_ncars);
    // Display # of laps on top of the list?
    const int drawLaps = MIN(1, leaderFlag - 1);

    int y = TOP_ANCHOR - 10;
    int y2 = y - 5 - dy * (maxLines + drawLaps);

    grSetupDrawingArea(x, y, xr + 5, y2);
    y = y2;

    // Display current car in last line (ie is its position >= 10)?
    bool drawCurrent = (current >= maxLines) ? true : false;

    // The board is built from bottom up:
    // driver position #10/current is drawn first,
    // then from #9 onto #1, then the text 'Laps' if drawLaps requires.
    for (int j = maxLines; j > 0; --j) {
      int i;  // index of driver to be displayed
      if (drawCurrent) {
        i = current;
        drawCurrent = 0;
      } else {
        i = j - 1;
      }  // if drawCurrent

      // Set colour of the drivers to that
      // defined in the drivers' XML file.
      // Current driver is yellow.
      float *clr = (i == current)
        ? grYellow : grCarInfo[s->cars[i]->index].iconColor;

      if (i == current)
        clr = grYellow;
      else if (i < current)
        clr = grCyan;
      else
        clr = grWhite;

      // Driver position + name
      snprintf(buf, sizeof(buf), "%3d: %s", i + 1, s->cars[i]->_name);
      GfuiDrawString(buf, clr, GFUI_FONT_SMALL_C, x, y);

      // Display driver time / time behind leader / laps behind leader
      string sEntry = grGenerateLeaderBoardEntry(s->cars[i], s, (i == 0));
      if (s->cars[i]->_state & RM_CAR_STATE_DNF
        || s->cars[i]->_state & RM_CAR_STATE_PIT) {
        // driver DNF or in pit, show 'out' in red
        clr = grRed;
      }
      GfuiDrawString(sEntry.c_str(), clr, GFUI_FONT_SMALL_C, x2, y, dxc,
                        GFUI_ALIGN_HR);

      y += dy;  // 'Line feed'
    }   // for j

    // Write 'Lap X/Y' on top of the leader board
    if (drawLaps) {
      if (s->_raceType == RM_TYPE_RACE) {
        if (s->_totTime > s->currentTime) {
          GfuiDrawString(" Laps:", grYellow, GFUI_FONT_SMALL_C, x, y);
          snprintf(buf, sizeof(buf), "%d", MAX(s->cars[0]->_laps-1, 0));
        } else {
          GfuiDrawString(" Lap:", grYellow, GFUI_FONT_SMALL_C, x, y);
          snprintf(buf, sizeof(buf), "%d / %d", s->cars[0]->_laps, s->_totLaps);
        }
        GfuiDrawString(buf, grYellow, GFUI_FONT_SMALL_C, x2, y, dxc,
                        GFUI_ALIGN_HR);
      } else {
        if (s->_totTime > 0.0f) {
          time_left = MAX(MIN(s->_totTime, s->_totTime - s->currentTime), 0);
          GfuiDrawString(" Time left:", grYellow, GFUI_FONT_SMALL_C, x, y);
          snprintf(buf, sizeof(buf), "%d:%02d:%02d",
                    (int)floor(time_left / 3600.0f),
                    (int)floor(time_left/60.0f) % 60,
                    (int)floor(time_left) % 60);
        } else {
          GfuiDrawString(" Lap:", grYellow, GFUI_FONT_SMALL_C, x, y);
          snprintf(buf, sizeof(buf), "%d / %d", s->cars[0]->_laps, s->_totLaps);
        }
        GfuiDrawString(buf, grYellow, GFUI_FONT_SMALL_C, x2, y, dxc,
                        GFUI_ALIGN_HR);
      }
    }   // if drawLaps
  }   // else
}   // grDispLeaderBoard


void
cGrBoard::grDispCounterBoard2(const tCarElt *car)
{
  // RPM
  tgrCarInstrument *curInst = &(grCarInfo[car->index].instrument[0]);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_TEXTURE_2D);
  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
  glTranslatef(centerAnchor,
                BOTTOM_ANCHOR + (speedoRise * TOP_ANCHOR / 100), 0);
  if (curInst->texture) {
    glBindTexture(GL_TEXTURE_2D, curInst->texture->getTextureHandle());
  }
  glCallList(curInst->CounterList);
  glBindTexture(GL_TEXTURE_2D, 0);

  tdble val = (*(curInst->monitored) - curInst->minValue) / curInst->maxValue;
  val = MIN(1.0, MAX(0.0, val));    // val between 0.0 and 1.0
  val = curInst->minAngle + val * curInst->maxAngle;

  RELAXATION(val, curInst->prevVal, 30);

  glPushMatrix();
  glTranslatef(curInst->needleXCenter, curInst->needleYCenter, 0);
  glRotatef(val, 0, 0, 1);
  glCallList(curInst->needleList);
  glPopMatrix();

  // Show gear
  char buf[32];
  if (car->_gear <= 0)
    snprintf(buf, sizeof(buf), "%s", car->_gear == 0 ? "N" : "R");
  else
    snprintf(buf, sizeof(buf), "%d", car->_gear);
  GfuiDrawString(buf, curInst->needleColor, GFUI_FONT_LARGE_C,
                    (int)curInst->digitXCenter - 30,
                    (int)curInst->digitYCenter,
                    60, GFUI_ALIGN_HC);

  glTranslatef(-centerAnchor, -BOTTOM_ANCHOR, 0);

  // Speedo
  curInst = &(grCarInfo[car->index].instrument[1]);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_TEXTURE_2D);
  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
  glTranslatef(centerAnchor, BOTTOM_ANCHOR, 0);
  if (curInst->texture) {
    glBindTexture(GL_TEXTURE_2D, curInst->texture->getTextureHandle());
  }
  glCallList(curInst->CounterList);
  glBindTexture(GL_TEXTURE_2D, 0);

  // Reverse speed should show on needle, too
  val = (*(curInst->monitored) - curInst->minValue) / curInst->maxValue;
  if (val < 0.0)
    val *= -1.0;
  val = MIN(1.0, val);
  val = curInst->minAngle + val * curInst->maxAngle;

  RELAXATION(val, curInst->prevVal, 30);

  glPushMatrix();
  glTranslatef(curInst->needleXCenter, curInst->needleYCenter, 0);
  glRotatef(val, 0, 0, 1);
  glCallList(curInst->needleList);
  glPopMatrix();

  // Digital speedo
  if (curInst->digital) {
    // Do not add "%3d" or something, because the digital font
    // DOES NOT SUPPORT BLANKS!!!!
    snprintf(buf, sizeof(buf), "%d", abs((int)(car->_speed_x * 3.6)));
    GfuiDrawString(buf, curInst->needleColor, GFUI_FONT_LARGE_C,
                    (int)curInst->digitXCenter - 30,
                    (int)curInst->digitYCenter,
                    60, GFUI_ALIGN_HC);
  }

  glTranslatef(-centerAnchor, -BOTTOM_ANCHOR, 0);

  // Fuel and damage meter
  if (counterFlag == 1) {
    float *clr;
    if (car->_fuel < 5.0f) {
      clr = grRed;
    } else {
      clr = grYellow;
    }

    grDrawGauge(centerAnchor + 140, BOTTOM_ANCHOR + 25, 100, clr,
                grBackground, car->_fuel / car->_tank, "F");
    grDrawGauge(centerAnchor + 155, BOTTOM_ANCHOR + 25, 100, grRed,
                grBackground, (tdble)(car->_dammage) / grMaxDammage, "D");
  }

  glTranslatef(0, -(speedoRise * TOP_ANCHOR / 100), 0);
}  // grDispCounterBoard2


void
cGrBoard::initBoard(void)
{
  if (!trackMap) {
    trackMap = new cGrTrackMap();
  }
}


void
cGrBoard::shutdown(void)
{
  delete trackMap;  // 'delete 0' is safe.
  trackMap = 0;
}


void
cGrBoard::grDispArcade(const tCarElt *car, const tSituation *s)
{
#define XM  15  // X margin
#define YM  10  // Y margin

  int dy = GfuiFontHeight(GFUI_FONT_BIG_C);
  int dxc = 100;

  int x = leftAnchor + XM;
  int x2 = x + 50;
  int y = TOP_ANCHOR - YM - dy;

  char buf[BUFSIZE];
  snprintf(buf, sizeof(buf), "%d/%d", car->_pos, s->_ncars);
  GfuiDrawString(buf, grDefaultClr, GFUI_FONT_BIG_C, x, y);

  dy = GfuiFontHeight(GFUI_FONT_LARGE_C);
  y -= dy;
  GfuiDrawString("Time:", grDefaultClr, GFUI_FONT_LARGE_C, x, y);
  grWriteTime(grDefaultClr, GFUI_FONT_LARGE_C, x2, y, dxc, car->_curLapTime, 0);

  y -= dy;
  GfuiDrawString("Best:", grDefaultClr, GFUI_FONT_LARGE_C, x, y);
  grWriteTime(grDefaultClr, GFUI_FONT_LARGE_C, x2, y, dxc, car->_bestLapTime, 0);

  y = TOP_ANCHOR - YM - dy;
  grGetLapsTime (s, car, buf, NULL);
  GfuiDrawString(buf, grDefaultClr, GFUI_FONT_LARGE_C, x, y, rightAnchor - leftAnchor - 2*XM, GFUI_ALIGN_HR);

  snprintf(buf, sizeof(buf), "%s", car->_name);
  GfuiDrawString(buf, grDefaultClr, GFUI_FONT_LARGE_C, x, y, rightAnchor - leftAnchor - 2*XM, GFUI_ALIGN_HC);

  float *clr = (car->_fuel < 5.0) ? grRed : grWhite;
  grDrawGauge(leftAnchor + XM, BOTTOM_ANCHOR + 25, 100, clr, grBlack, car->_fuel / car->_tank, "F");
  grDrawGauge(leftAnchor + XM + 15, BOTTOM_ANCHOR + 25, 100, grRed, grGreen, (tdble)(car->_dammage) / grMaxDammage, "D");

  dy = GfuiFontHeight(GFUI_FONT_LARGE_C);
  y = YM + dy;
  snprintf(buf, sizeof(buf), "%3d km/h", abs((int)(car->_speed_x * 3.6)));
  GfuiDrawString(buf, grDefaultClr, GFUI_FONT_BIG_C, x, y, rightAnchor - leftAnchor - 2*XM, GFUI_ALIGN_HR);
  y = YM;
  if (car->_gear <= 0)
    snprintf(buf, sizeof(buf), "%s", car->_gear == 0 ? "N" : "R");
  else
    snprintf(buf, sizeof(buf), "%d", car->_gear);
  GfuiDrawString(buf, grDefaultClr, GFUI_FONT_LARGE_C, x, y, rightAnchor - leftAnchor - 2*XM, GFUI_ALIGN_HR);

  grDispEngineLeds(car, rightAnchor - XM, YM + dy + GfuiFontHeight (GFUI_FONT_BIG_C), ALIGN_RIGHT, false);
}  // grDispArcade


/**
 * This function calculates if the split time must be displayed, and if so what the
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
bool cGrBoard::grGetSplitTime(const tSituation *s, const tCarElt *car,
                                bool gap_inrace, double &time,
                                int *laps_different, float **color)
{
  tdble curSplit;
  tdble bestSplit;
  tdble bestSessionSplit;
  const tCarElt *ocar = car;
  const tCarElt *fcar = car;
  int sign = 1;
  int laps;

  if (laps_different)
    *laps_different = 0;

  if (s->_raceType != RM_TYPE_RACE || s->_ncars == 1) {
    if (car->_currentSector == 0)
      return false;

    curSplit = car->_curSplitTime[car->_currentSector - 1];
    bestSplit = car->_bestSplitTime[car->_currentSector - 1];

    if (car->_curLapTime - curSplit > 5.0f)
      return false; /* Only display split for five seconds */

    if (s->_ncars > 1) {
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
    } else {
      if (bestSplit < 0.0f)
        return false;

      time = curSplit - bestSplit;

      if (time < 0.0f)
        *color = grGreen;
      else
        *color = grWhite;
    }
  } else if (gap_inrace) {
    if (car->_pos == 1) {
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
    if (ocar->_currentSector < fcar->_currentSector
        || (ocar->_currentSector == fcar->_currentSector
            && fcar->_curTime + curSplit < ocar->_curTime + bestSplit))
      --laps;

    if (!laps_different && laps != 0)
      return false;

    if (laps_different)
      *laps_different = sign * laps;

    time = ocar->_curTime + bestSplit - (fcar->_curTime + curSplit);
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
 * @param result[out] An already existing string of len BUFSIZE which will contain the text
 * @param label[out] The label (Lap: or Time: ) If zero, then the label is added to @p result.
 */
void cGrBoard::grGetLapsTime(const tSituation *s, const tCarElt *car,
                              char* result, char const **label) const
{
  bool time = true;
  double cur_left;
  char const *loc_label;

  // Don't show time data if race haven't started yet or is already finished
  if (s->_totTime < 0.0f
        || (s->_totTime < s->currentTime && s->_extraLaps > 0))
    time = false;

  if (label) {
    *label = time ? "Time: " : "Lap: ";
    loc_label = "";
  } else {
    loc_label = time ? "Time: " : "Lap: ";
  }

  // Show only lap counts before start or after race
  if (!time) {
    snprintf(result, BUFSIZE, "%s%d/%d", loc_label, car->_laps, s->_totLaps);
  } else {
    cur_left = s->_totTime - s->currentTime;
    if (s->currentTime < 0.0f)
      cur_left = s->_totTime;
    if (cur_left < 0.0f)
      cur_left = 0.0f;

    snprintf(result, BUFSIZE, "%s%d:%02d:%02d", loc_label,
                (int)floor(cur_left / 3600.0f),
                (int)floor(cur_left / 60.0f) % 60,
                (int)floor(cur_left) % 60);
  }
}

void cGrBoard::refreshBoard(tSituation *s, const cGrFrameInfo* frameInfo,
                            const tCarElt *currCar, bool isCurrScreen)
{
  grDispMisc(isCurrScreen);

  if (arcadeFlag) {
    grDispArcade(currCar, s);
  } else {
    if (debugFlag)
      grDispDebug(s, currCar, frameInfo);
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


// TODO(?): clean solution for cleanup.
static ssgSimpleState* cleanup[1024];
static int nstate = 0;


void grInitBoardCar(tCarElt *car)
{
  static const int nMaxTexPathSize = 4096;
  const bool bMasterModel = strlen(car->_masterModel) != 0;

  grssgLoaderOptions options;
  grssgSetCurrentOptions(&options);

  int index = car->index; /* current car's index */
  tgrCarInfo *carInfo = &grCarInfo[index];
  void *handle = car->_carHandle;

  /* Set tachometer/speedometer textures search path :
   1) driver level specified, in the user settings,
   2) driver level specified,
   3) car level specified,
   4) common textures */
  grFilePath = (char*)malloc(nMaxTexPathSize);
  int lg = 0;
  lg += snprintf(grFilePath + lg, nMaxTexPathSize - lg, "%sdrivers/%s/%d/%s;",
                    GfLocalDir(), car->_modName, car->_driverIndex, car->_carName);
  if (bMasterModel)
    lg += snprintf(grFilePath + lg, nMaxTexPathSize - lg, "%sdrivers/%s/%d/%s;",
                    GfLocalDir(), car->_modName, car->_driverIndex, car->_masterModel);

  lg += snprintf(grFilePath + lg, nMaxTexPathSize - lg, "%sdrivers/%s/%d;",
                    GfLocalDir(), car->_modName, car->_driverIndex);

  lg += snprintf(grFilePath + lg, nMaxTexPathSize - lg, "%sdrivers/%s/%s;",
                    GfLocalDir(), car->_modName, car->_carName);
  if (bMasterModel)
    lg += snprintf(grFilePath + lg, nMaxTexPathSize - lg, "%sdrivers/%s/%s;",
                    GfLocalDir(), car->_modName, car->_masterModel);

  lg += snprintf(grFilePath + lg, nMaxTexPathSize - lg, "%sdrivers/%s;",
                    GfLocalDir(), car->_modName);

  lg += snprintf(grFilePath + lg, nMaxTexPathSize - lg, "drivers/%s/%d/%s;",
                    car->_modName, car->_driverIndex, car->_carName);
  if (bMasterModel)
    lg += snprintf(grFilePath + lg, nMaxTexPathSize - lg, "drivers/%s/%d/%s;",
                    car->_modName, car->_driverIndex, car->_masterModel);

  lg += snprintf(grFilePath + lg, nMaxTexPathSize - lg, "drivers/%s/%d;",
                    car->_modName, car->_driverIndex);

  lg += snprintf(grFilePath + lg, nMaxTexPathSize - lg, "drivers/%s/%s;",
                    car->_modName, car->_carName);
  if (bMasterModel)
    lg += snprintf(grFilePath + lg, nMaxTexPathSize - lg, "drivers/%s/%s;",
                    car->_modName, car->_masterModel);

  lg += snprintf(grFilePath + lg, nMaxTexPathSize - lg, "drivers/%s;", car->_modName);

  lg += snprintf(grFilePath + lg, nMaxTexPathSize - lg, "cars/%s;", car->_carName);
  if (bMasterModel)
    lg += snprintf(grFilePath + lg, nMaxTexPathSize - lg, "cars/%s;", car->_masterModel);

  lg += snprintf(grFilePath + lg, nMaxTexPathSize - lg, "data/textures");

  /* Tachometer --------------------------------------------------------- */
  tgrCarInstrument *curInst = &(carInfo->instrument[0]);

  /* Load the Tachometer texture */
  const char *param = GfParmGetStr(handle, SECT_GROBJECTS,
                                    PRM_TACHO_TEX, "rpm8000.png");

  curInst->texture = (ssgSimpleState*)grSsgLoadTexState(param);
  if (curInst->texture == 0)
    curInst->texture = (ssgSimpleState*)grSsgLoadTexState("rpm8000.rgb");

  cleanup[nstate] = curInst->texture;
  nstate++;

  /* Load the instrument placement */
  tdble xSz = GfParmGetNum(handle, SECT_GROBJECTS, PRM_TACHO_XSZ, (char*)NULL, 128);
  tdble ySz = GfParmGetNum(handle, SECT_GROBJECTS, PRM_TACHO_YSZ, (char*)NULL, 128);

  // position are delta from center of screen
  tdble xpos = GfParmGetNum(handle, SECT_GROBJECTS, PRM_TACHO_XPOS, (char*)NULL, 0 - xSz);
  tdble ypos = GfParmGetNum(handle, SECT_GROBJECTS, PRM_TACHO_YPOS, (char*)NULL, 0);
  tdble needlexSz = GfParmGetNum(handle, SECT_GROBJECTS, PRM_TACHO_NDLXSZ, (char*)NULL, 50);
  tdble needleySz = GfParmGetNum(handle, SECT_GROBJECTS, PRM_TACHO_NDLYSZ, (char*)NULL, 2);
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
  curInst->needleColor[0] = GfParmGetNum(handle, SECT_GROBJECTS,
                                PRM_NEEDLE_RED, (char*)NULL, 1.0);
  curInst->needleColor[1] = GfParmGetNum(handle, SECT_GROBJECTS,
                                PRM_NEEDLE_GREEN, (char*)NULL, 0.0);
  curInst->needleColor[2] = GfParmGetNum(handle, SECT_GROBJECTS,
                                PRM_NEEDLE_BLUE, (char*)NULL, 0.0);
  curInst->needleColor[3] = GfParmGetNum(handle, SECT_GROBJECTS,
                                PRM_NEEDLE_ALPHA, (char*)NULL, 1.0);

  curInst->CounterList = glGenLists(1);
  glNewList(curInst->CounterList, GL_COMPILE);
  glBegin(GL_TRIANGLE_STRIP);
  {
    glColor4f(1.0, 1.0, 1.0, 0.0);
    glTexCoord2f(0.0, 0.0);
    glVertex2f(xpos, ypos);
    glTexCoord2f(0.0, 1.0);
    glVertex2f(xpos, ypos + ySz);
    glTexCoord2f(1.0, 0.0);
    glVertex2f(xpos + xSz, ypos);
    glTexCoord2f(1.0, 1.0);
    glVertex2f(xpos + xSz, ypos + ySz);
  }
  glEnd();
  glEndList();

  curInst->needleList = glGenLists(1);
  glNewList(curInst->needleList, GL_COMPILE);
  glBegin(GL_TRIANGLE_STRIP);
  {
    glColor4f(curInst->needleColor[0], curInst->needleColor[1],
                curInst->needleColor[2], curInst->needleColor[3]);
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
  if (curInst->texture == 0)
    curInst->texture = (ssgSimpleState*)grSsgLoadTexState("speed360.rgb");

  free(grFilePath);

  cleanup[nstate] = curInst->texture;
  nstate++;

  /* Load the instrument placement */
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
  curInst->needleColor[0] = GfParmGetNum(handle, SECT_GROBJECTS,
                                PRM_NEEDLE_RED, (char*)NULL, 1.0);
  curInst->needleColor[1] = GfParmGetNum(handle, SECT_GROBJECTS,
                                PRM_NEEDLE_GREEN, (char*)NULL, 0.0);
  curInst->needleColor[2] = GfParmGetNum(handle, SECT_GROBJECTS,
                                PRM_NEEDLE_BLUE, (char*)NULL, 0.0);
  curInst->needleColor[3] = GfParmGetNum(handle, SECT_GROBJECTS,
                                PRM_NEEDLE_ALPHA, (char*)NULL, 1.0);

  curInst->CounterList = glGenLists(1);
  glNewList(curInst->CounterList, GL_COMPILE);
  glBegin(GL_TRIANGLE_STRIP);
  {
    glColor4f(1.0, 1.0, 1.0, 0.0);
    glTexCoord2f(0.0, 0.0);
    glVertex2f(xpos, ypos);
    glTexCoord2f(0.0, 1.0);
    glVertex2f(xpos, ypos + ySz);
    glTexCoord2f(1.0, 0.0);
    glVertex2f(xpos + xSz, ypos);
    glTexCoord2f(1.0, 1.0);
    glVertex2f(xpos + xSz, ypos + ySz);
  }
  glEnd();
  glEndList();

  curInst->needleList = glGenLists(1);
  glNewList(curInst->needleList, GL_COMPILE);
  glBegin(GL_TRIANGLE_STRIP);
  {
    glColor4f(curInst->needleColor[0], curInst->needleColor[1],
                curInst->needleColor[2], curInst->needleColor[3]);
    glVertex2f(0, -needleySz);
    glVertex2f(0, needleySz);
    glVertex2f(needlexSz, -needleySz / 2.0);
    glVertex2f(needlexSz, needleySz / 2.0);
  }
  glEnd();
  glEndList();
}   // grInitBoardCar


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
 * @param car[in] pointer to the currently displayed car
 * @param s[in] current situation, provided by the sim
*/
void
cGrBoard::grDispLeaderBoardScroll(const tCarElt *car, const tSituation *s)
{
  // Scrolling
  if (iTimer == 0 || s->currentTime < iTimer)
    iTimer = s->currentTime;
  if (s->currentTime >= iTimer + LEADERBOARD_SCROLL_TIME) {
    iTimer = s->currentTime;
    ++iStart;
    // Limit: number of cars + one separator line
    iStart = iStart % (s->_ncars + 1);
  }

  int current = 0;  // Position of the currently displayed car
  for (int i = 0; i < s->_ncars; ++i) {
    if (car == s->cars[i]) {
      current = i;
      break;
    }
  }  // for i

  // Coords, limits
  const int x = leftAnchor + 10;
  const int x2 = x + 100;
  static const int dxc = 60;

  const int dy = GfuiFontHeight(GFUI_FONT_SMALL_C);
  // Max # of lines to display (# of cars in race or max 10 by default)
  const int maxLines = MIN(leaderNb, s->_ncars);
  int y = TOP_ANCHOR - 10;
  int y2 = y - 5 - dy * (maxLines + 1);

  grSetupDrawingArea(x, y, x2 + dxc + 5, y2);
  y = y2;

  // The board is built from bottom up:
  // driver position #10/current is drawn first,
  // then from #9 onto #1, then the text 'Laps' if drawLaps requires.
  char buf[BUFSIZE];
  for (int j = maxLines - 1; j >= 0; j--) {
    int i = j + iStart;  // index of driver to be displayed

    if (i == s->_ncars) {
      // print empty line after last car
    } else {
      i = i % (s->_ncars + 1);

      // Set colour of the drivers to that
      // defined in the drivers' XML file.
      // Current driver is yellow...
      float *clr = (i == current)
        ? grDefaultClr : grCarInfo[s->cars[i]->index].iconColor;

      // Driver position + name
      snprintf(buf, sizeof(buf), "%3d: %s", i + 1, s->cars[i]->_name);
      GfuiDrawString(buf, clr, GFUI_FONT_SMALL_C, x, y);

      // Display driver time / time behind leader / laps behind leader
      string sEntry = grGenerateLeaderBoardEntry(s->cars[i], s, (i == 0));
      if (s->cars[i]->_state & RM_CAR_STATE_DNF
        || s->cars[i]->_state & RM_CAR_STATE_PIT) {
        // driver DNF or in pit, show 'out' in red
        clr = grRed;
      }
      GfuiDrawString(sEntry.c_str(), clr, GFUI_FONT_SMALL_C, x2, y, dxc,
                        GFUI_ALIGN_HR);
    }   // else i
    y += dy;  // 'Line feed'
  }  // for j

  // Write 'Lap X/Y' on top of the leader board
  if (s->currentTime < s->_totTime) {
    GfuiDrawString(" Laps:", grWhite, GFUI_FONT_SMALL_C, x, y);
    snprintf(buf, sizeof(buf), "%d", s->cars[0]->_laps);
  } else {
    GfuiDrawString(" Lap:", grWhite, GFUI_FONT_SMALL_C, x, y);
    snprintf(buf, sizeof(buf), "%d / %d", s->cars[0]->_laps, s->_totLaps);
  }
  GfuiDrawString(buf, grWhite, GFUI_FONT_SMALL_C, x2, y, dxc, GFUI_ALIGN_HR);
}   // grDispLeaderBoardScroll


#define LEADERBOARD_LINE_SCROLL_RATE 80     // pixels per second
#define LEADERBOARD_LINE_SCROLL_DELAY 5     // seconds
/**
 * grDispLeaderBoardScrollLine
 *
 * Scrolls the leaderboard on the bottom line, as seen on TV broadcasts.
 *
 * @param car[in] pointer to the currently displayed car
 * @param s[in] current situation, provided by the sim
 */
void
cGrBoard::grDispLeaderBoardScrollLine(const tCarElt *car, const tSituation *s)
{
  // At the start of the race or when first engaging this mode,
  // we generate the 3-letter names to be used
  if (iTimer == 0 || iStringStart == 0 || sShortNames.size() == 0)
    grMakeThreeLetterNames(s);

  // At first, get the current time and rebuild the ScrollLine text
  if (iTimer == 0 || s->currentTime < iTimer) {
    iTimer = s->currentTime;
    st.clear();
    /*!The roster holds the driver's position, name and difference
     * *at the time* the leader starts a new lap.
     * So it can happen it is somewhat mixed up, it will settle down
     * in the next lap.
    */

    std::ostringstream osRoster;
    // Add the track name as separator, embedded with 3 spaces each side.
    osRoster << "   " << grTrack->name << "   ";
    // Add # of laps
    osRoster << "Lap " << s->cars[0]->race.laps << " | ";
    for (int i = 0; i < s->_ncars; i++) {
      // Driver position + name
      osRoster.width(3);
      osRoster << (i + 1);
      osRoster << ": " << sShortNames[i];

      // Display driver time / time behind leader / laps behind leader
      string sEntry = grGenerateLeaderBoardEntry(s->cars[i], s, (i == 0));

      // get rid of leading spaces
      size_t iCut = sEntry.find_first_not_of(' ');
      if (iCut != string::npos && iCut != 0) {
        sEntry = sEntry.substr(iCut - 1);
      }  // if iCut
      // Add to roster, then separate it from next one
      osRoster << sEntry << "   ";
    }   // for i

    st.assign(osRoster.str());
  }

  int offset = (s->currentTime - iTimer - LEADERBOARD_LINE_SCROLL_DELAY)
                * LEADERBOARD_LINE_SCROLL_RATE;
  if (offset < 0)
    offset = 0;

  int dy = GfuiFontHeight(GFUI_FONT_MEDIUM_C);
  int dx = GfuiFontWidth(GFUI_FONT_SMALL_C, "W") * st.size();

  // Set up drawing area
  grSetupDrawingArea(leftAnchor, TOP_ANCHOR, rightAnchor, TOP_ANCHOR - dy);

  // Check if scrolling is completed
  if (offset > dx + 5)
    iTimer = 0;
  else
    // Display the line
    GfuiDrawString(st.c_str(), grWhite, GFUI_FONT_MEDIUM_C,
                    leftAnchor + 5 - offset, TOP_ANCHOR - dy);
}   // grDispLeaderBoardScrollLine


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
cGrBoard::grGenerateLeaderBoardEntry(const tCarElt* car,
                                        const tSituation* s,
                                        const bool isLeader) const
{
  char buf[BUFSIZE];

  // Display driver time / time behind leader / laps behind leader
  if (car->_state & RM_CAR_STATE_DNF)  {
    snprintf(buf, sizeof(buf), "       out");
    return buf;
  }

  if (car->_state & RM_CAR_STATE_PIT) {
    snprintf(buf, sizeof(buf), "       PIT");
    return buf;
  }

  // This is the leader, put out his time
  if (isLeader) {
    if (car->_bestLapTime == 0) {
      snprintf(buf, sizeof(buf), "       --:---");
    } else {
      if (s->_raceType == RM_TYPE_RACE || s->_ncars <= 1)
        grWriteTimeBuf(buf, car->_curTime, 0);
      else
        grWriteTimeBuf(buf, car->_bestLapTime, 0);
    }
    return buf;
  }

  // This is not the leader
  int lapsBehindLeader = car->_lapsBehindLeader;

  if (car->_laps < s->cars[0]->_laps - 1) {
    // need to do a little math as
    // car->_lapsBehindLeader is only updated at finish line
    lapsBehindLeader = s->cars[0]->_laps - car->_laps;

    if (s->cars[0]->_distFromStartLine < car->_distFromStartLine)
      lapsBehindLeader--;
  }

  switch (lapsBehindLeader) {
    case 0:  // Driver in same lap as leader or on first lap
      if (car->_bestLapTime == 0 || car->_laps < s->cars[0]->_laps) {
        snprintf(buf, sizeof(buf), "       --:---");
      } else {
        grWriteTimeBuf(buf, car->_timeBehindLeader, 1);
      }
      break;

    case 1:  // 1 lap behind leader
      snprintf(buf, sizeof(buf), "+%3d Lap", lapsBehindLeader);
      break;

    default:  // N laps behind leader
      snprintf(buf, sizeof(buf), "+%3d Laps", lapsBehindLeader);
      break;
  }

  return buf;
}


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
  // Build up two arrays of the original and the short names, same order.
  sShortNames.clear();
  std::vector<string> origNames;
  for (int i = 0; i < s->_ncars; i++) {  // Loop over each name in the race
    string st(s->cars[i]->_name);
    origNames.push_back(st);
    remove(st.begin(), st.end(), ' ');  // Remove spaces
    sShortNames.push_back(st.substr(0, 3));  // Cut to 3 characters
  }  // for i
  // Copy to hold original 3-letter names, for search
  std::vector<string> origShortNames(sShortNames);

  // Check for matching names
  for (unsigned int i = 0; i < sShortNames.size(); i++) {
    string sSearch(origShortNames[i]);
    for (unsigned int j = i + 1; j < sShortNames.size(); j++) {
      if (sSearch == origShortNames[j]) {  // Same 3-letter name found
        // Let's find the first mismatching character
        unsigned int k;
        for (k = 0;
                k < std::min(origNames[i].size(), origNames[j].size());
                ++k) {
          if (origNames[i][k] != origNames[j][k]) break;
        }   // for k
        // Set 3rd char of the short name to the mismatching char (or last one).
        // It is the driver designer's responsibility from now on
        // to provide some unsimilarities between driver names.
        sShortNames[i][2] = origNames[i][k];
        sShortNames[j][2] = origNames[j][k];
      }  // if sSearch
    }   // for j
  }     // for i
    // 3-letter name array ready to use!
}   // grMakeThreeLetterName


/**
 * Set up a drawing area to put textual info there.
 *
 * Draws a dark quadrangle on the given coords.
 *
 * @param xl X left
 * @param yb Y bottom
 * @param xr X right
 * @param yt Y top
 */
void
cGrBoard::grSetupDrawingArea(int xl, int yb, int xr, int yt) const
{
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glBegin(GL_QUADS);
  // darkblue to fit the menu style: 0a162f
  glColor4f(0.039, 0.086, 0.184, 0.8);
  glVertex2f(xl, yb);
  glVertex2f(xr, yb);
  glVertex2f(xr, yt);
  glVertex2f(xl, yt);
  glEnd();
  glDisable(GL_BLEND);
}


void
cGrBoard::grDispIndicators(const tCarElt* car)
{
  // Only useful for humans - maybe robots should show that, too?
  if (car->_driverType == RM_DRV_HUMAN) {
    bool abs = false;   // Show ABS indicator?
    bool tcs = false;   // Show TCS indicator?
    bool spd = false;   // Show speed limiter indicator?

    // Parse control messages if they include ABS / TCS / SPD
    for (int i = 0; i < 4; i++) {
      if (car->ctrl.msg[i]) {
        abs = abs || strstr(car->ctrl.msg[i], "ABS");
        tcs = tcs || strstr(car->ctrl.msg[i], "TCS");
        spd = spd || strstr(car->ctrl.msg[i], "Speed Limiter On");
      }
    }

    // Setup drawing area
    int dy = GfuiFontHeight(GFUI_FONT_MEDIUM_C);
    int dy2 = GfuiFontHeight(GFUI_FONT_SMALL_C);
    int dx = GfuiFontWidth(GFUI_FONT_MEDIUM_C, "SPD");

    int x = centerAnchor - 200;                 // constant text left pos.
    int y = BOTTOM_ANCHOR + dy2 * 8 + dy + 5;   // first row top pos.

    // Display board
    grSetupDrawingArea(x - 5, y + dy + 5, x + dx + 5, y - dy2 * 8 - dy + 5);

    // Display strings (until we are more advanced graphically)
    if (abs)
      GfuiDrawString("ABS", grYellow, GFUI_FONT_MEDIUM_C, x, y);
    else
      GfuiDrawString("ABS", grGrey, GFUI_FONT_MEDIUM_C, x, y);
    y -= dy;

    if (tcs)
      GfuiDrawString("TCS", grYellow, GFUI_FONT_MEDIUM_C, x, y);
    else
      GfuiDrawString("TCS", grGrey, GFUI_FONT_MEDIUM_C, x, y);
    y -= dy;

    if (spd)
      GfuiDrawString("SPD", grYellow, GFUI_FONT_MEDIUM_C, x, y);
    else
      GfuiDrawString("SPD", grGrey, GFUI_FONT_MEDIUM_C, x, y);
  }  // if human
}  // grDispIndicators
