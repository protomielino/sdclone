/***************************************************************************

    file                 : grboard.cpp
    created              : Thu Aug 17 23:52:20 CEST 2000
    copyright            : (C) 2000 by Eric Espie
    email                : torcs@free.fr
    version              : $Id: grboard.cpp,v 1.27 2006/09/06 21:56:06 berniw Exp $

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include <plib/ssg.h>

#include "grboard.h"
#include "grshadow.h"
#include "grskidmarks.h"
#include "grcar.h"
#include "grutil.h"
#include <robottools.h>

#include <iostream>
using namespace std;

static float grWhite[4] = {1.0, 1.0, 1.0, 1.0};
static float grRed[4] = {1.0, 0.0, 0.0, 1.0};
static float grBlue[4] = {0.0, 0.0, 1.0, 1.0};
static float grGreen[4] = {0.0, 1.0, 0.0, 1.0};
static float grBlack[4] = {0.0, 0.0, 0.0, 1.0};
static float grDefaultClr[4] = {0.9, 0.9, 0.15, 1.0};

#define NB_BOARDS	3
#define NB_LBOARDS	5	//# of leaderboard states
#define NB_CBOARDS	3

static int	Winx	= 0;
static int	Winw	= 800;
static int	Winy	= 0;
static int	Winh	= 600;

static char	path[1024];

cGrBoard::cGrBoard (int myid) {
	id = myid;
	trackMap = NULL;
	Winw = grWinw*600/grWinh;
}


cGrBoard::~cGrBoard () {
	trackMap = NULL;
}


void
cGrBoard::loadDefaults(tCarElt *curCar)
{
	sprintf (path, "%s/%d", GR_SCT_DISPMODE, id);
	
	debugFlag	= (int)GfParmGetNum(grHandle, path, GR_ATT_DEBUG, NULL, 1);
	boardFlag	= (int)GfParmGetNum(grHandle, path, GR_ATT_BOARD, NULL, 2);
	leaderFlag	= (int)GfParmGetNum(grHandle, path, GR_ATT_LEADER, NULL, 1);
	leaderNb	= (int)GfParmGetNum(grHandle, path, GR_ATT_NBLEADER, NULL, 10);
	counterFlag = (int)GfParmGetNum(grHandle, path, GR_ATT_COUNTER, NULL, 1);
	GFlag	= (int)GfParmGetNum(grHandle, path, GR_ATT_GGRAPH, NULL, 1);
	arcadeFlag	= (int)GfParmGetNum(grHandle, path, GR_ATT_ARCADE, NULL, 0);
	
	trackMap->setViewMode((int) GfParmGetNum(grHandle, path, GR_ATT_MAP, NULL, trackMap->getDefaultViewMode()));
	
	if (curCar->_driverType == RM_DRV_HUMAN) {
		sprintf(path, "%s/%s", GR_SCT_DISPMODE, curCar->_name);
		debugFlag	= (int)GfParmGetNum(grHandle, path, GR_ATT_DEBUG, NULL, debugFlag);
		boardFlag	= (int)GfParmGetNum(grHandle, path, GR_ATT_BOARD, NULL, boardFlag);
		leaderFlag	= (int)GfParmGetNum(grHandle, path, GR_ATT_LEADER, NULL, leaderFlag);
		leaderNb	= (int)GfParmGetNum(grHandle, path, GR_ATT_NBLEADER, NULL, leaderNb);
		counterFlag 	= (int)GfParmGetNum(grHandle, path, GR_ATT_COUNTER, NULL, counterFlag);
		GFlag		= (int)GfParmGetNum(grHandle, path, GR_ATT_GGRAPH, NULL, GFlag);
		arcadeFlag	= (int)GfParmGetNum(grHandle, path, GR_ATT_ARCADE, NULL, arcadeFlag);
		trackMap->setViewMode((int) GfParmGetNum(grHandle, path, GR_ATT_MAP, NULL, trackMap->getViewMode()));
	}
}


void
cGrBoard::selectBoard(int val)
{
	sprintf (path, "%s/%d", GR_SCT_DISPMODE, id);
	
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
			debugFlag = 1 - debugFlag;
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


void
cGrBoard::grDispDebug(float fps, tCarElt *car)
{
	char buf[256];
	int  x, y;
	tRoadCam *curCam;
	
	curCam = car->_trkPos.seg->cam;
	
	x = Winx + Winw - 100;
	y = Winy + Winh - 30;
	
	sprintf(buf, "FPS: %.1f", fps);
	GfuiPrintString(buf, grWhite, GFUI_FONT_SMALL_C, x, y, GFUI_ALIGN_HL_VB);
	return;
	
	y -= 15;
	sprintf(buf, "Seg: %s", car->_trkPos.seg->name);
	GfuiPrintString(buf, grWhite, GFUI_FONT_SMALL_C, x, y, GFUI_ALIGN_HL_VB);
	y -= 15;
	if (curCam) {
		sprintf(buf, "Cam: %s", curCam->name);
		GfuiPrintString(buf, grWhite, GFUI_FONT_SMALL_C, x, y, GFUI_ALIGN_HL_VB);
		y -= 15;
	}
}

void
cGrBoard::grDispGGraph(tCarElt *car)
{
	tdble X1, Y1, X2, Y2, xc, yc;

	X1 = (tdble)(Winx + Winw - 100);
	Y1 = (tdble)(Winy + 100);
	xc = (tdble)(Winx + Winw - 30);
	yc = (tdble)(Y1 - 50);

	X2 = -car->_DynGC.acc.y / 9.81f * 25.0f + X1;
	Y2 = car->_DynGC.acc.x / 9.81f * 25.0f + Y1;
	glBegin(GL_LINES);
	glColor4f(1.0, 1.0, 1.0, 1.0);
	glVertex2f(X1-50, Y1);
	glVertex2f(X1+50, Y1);
	glVertex2f(X1, Y1-50);
	glVertex2f(X1, Y1+50);
	glVertex2f(xc, yc);
	glVertex2f(xc, yc+100);
	glEnd();

	const tdble THNSS = 2.0f;

	glBegin(GL_QUADS);
	glColor4f(0.0, 0.0, 1.0, 1.0);
	glVertex2f(X1 - THNSS, Y1);
	glVertex2f(X1 + THNSS, Y1);
	glVertex2f(X1 + THNSS, Y1 + car->ctrl.accelCmd * 50.0f);
	glVertex2f(X1 - THNSS, Y1 + car->ctrl.accelCmd * 50.0f);

	glVertex2f(X1 - THNSS, Y1);
	glVertex2f(X1 + THNSS, Y1);
	glVertex2f(X1 + THNSS, Y1 - car->ctrl.brakeCmd * 50.0f);
	glVertex2f(X1 - THNSS, Y1 - car->ctrl.brakeCmd * 50.0f);

	glVertex2f(X1, Y1 - THNSS);
	glVertex2f(X1, Y1 + THNSS);
	glVertex2f(X1 - car->ctrl.steer * 100.0f, Y1 + THNSS);
	glVertex2f(X1 - car->ctrl.steer * 100.0f, Y1 - THNSS);

	glVertex2f(xc - THNSS, yc);
	glVertex2f(xc + THNSS, yc);
	glVertex2f(xc + THNSS, yc + car->ctrl.clutchCmd * 100.0f);
	glVertex2f(xc - THNSS, yc + car->ctrl.clutchCmd * 100.0f);

	glEnd();

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
	
#define THNSSBG	2.0
#define THNSSFG	2.0
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
cGrBoard::grDispMisc(tCarElt *car)
{
	float *clr;
	
	if (car->_fuel < 5.0f) {
		clr = grRed;
	} else {
		clr = grWhite;
	}

	tdble fw = Winw/800.0f;

	grDrawGauge(545.0f*fw, 20.0f*fw, 80.0f, clr, grBlack, car->_fuel / car->_tank, "F");
	grDrawGauge(560.0f*fw, 20.0f*fw, 80.0f, grRed, grGreen, (tdble)(car->_dammage) / grMaxDammage, "D");
}

void
cGrBoard::grDispCarBoard1(tCarElt *car, tSituation *s)
{
	int  x, x2, y;
	char buf[256];
	float *clr;
	int dy, dy2, dx;
	
	x = 10;
	x2 = 110;
	dy = GfuiFontHeight(GFUI_FONT_MEDIUM_C);
	dy2 = GfuiFontHeight(GFUI_FONT_SMALL_C);
	y = Winy + Winh - dy - 5;
	sprintf(buf, "%d/%d - %s", car->_pos, s->_ncars, car->_name);
	dx = GfuiFontWidth(GFUI_FONT_MEDIUM_C, buf);
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
	sprintf(buf, "%.1f l", car->_fuel);
	GfuiPrintString(buf, clr, GFUI_FONT_SMALL_C, x2, y, GFUI_ALIGN_HR_VB);
	y -= dy;
	
	if (car->_state & RM_CAR_STATE_BROKEN) {
		clr = grRed;
	} else {
		clr = grWhite;
	}
	
	GfuiPrintString("Damage:", clr, GFUI_FONT_SMALL_C, x, y, GFUI_ALIGN_HL_VB);
	sprintf(buf, "%d", car->_dammage);
	GfuiPrintString(buf, clr, GFUI_FONT_SMALL_C, x2, y, GFUI_ALIGN_HR_VB);
	y -= dy;
	clr = grWhite;
	
	GfuiPrintString("Laps:", clr, GFUI_FONT_SMALL_C, x, y, GFUI_ALIGN_HL_VB);
	sprintf(buf, "%d / %d", car->_laps, s->_totLaps);
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
	sprintf(buf, "%d", (int)(car->_topSpeed * 3.6));
	GfuiPrintString(buf, clr, GFUI_FONT_SMALL_C, x2, y, GFUI_ALIGN_HR_VB);
	y -= dy;
}

void
cGrBoard::grDispCarBoard2(tCarElt *car, tSituation *s)
{
	int  x, x2, x3, y;
	char buf[256];
	float *clr;
	int dy, dy2, dx;
	int lines, i;
	
	x = 10;
	x2 = 110;
	x3 = 170;
	dy = GfuiFontHeight(GFUI_FONT_MEDIUM_C);
	dy2 = GfuiFontHeight(GFUI_FONT_SMALL_C);
	
	y = Winy + Winh - dy - 5;
	
	sprintf(buf, "%d/%d - %s", car->_pos, s->_ncars, car->_name);
	dx = GfuiFontWidth(GFUI_FONT_MEDIUM_C, buf);
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
	glVertex2f(x-5, y + dy);
	glVertex2f(x+dx+5, y + dy);
	glVertex2f(x+dx+5, y-5 - dy2 * lines);
	glVertex2f(x-5, y-5 - dy2 * lines);
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
	sprintf(buf, "%.1f l", car->_fuel);
	GfuiPrintString(buf, clr, GFUI_FONT_SMALL_C, x2, y, GFUI_ALIGN_HR_VB);
	y -= dy;
	
	clr = grWhite;
	
	GfuiPrintString("Laps:", clr, GFUI_FONT_SMALL_C, x, y, GFUI_ALIGN_HL_VB);
	sprintf(buf, "%d / %d", car->_laps, s->_totLaps);
	GfuiPrintString(buf, clr, GFUI_FONT_SMALL_C, x2, y, GFUI_ALIGN_HR_VB);
	y -= dy;
	
	GfuiPrintString("Best:", clr, GFUI_FONT_SMALL_C, x, y, GFUI_ALIGN_HL_VB);
	grWriteTime(clr, GFUI_FONT_SMALL_C, x2, y, car->_bestLapTime, 0);
	grWriteTime(clr, GFUI_FONT_SMALL_C, x3, y, car->_deltaBestLapTime, 1);
	y -= dy;
	
	GfuiPrintString("Time:", clr, GFUI_FONT_SMALL_C, x, y, GFUI_ALIGN_HL_VB);
	grWriteTime(clr, GFUI_FONT_SMALL_C, x2, y, car->_curLapTime, 0);    
	y -= dy;
	
	if (car->_pos != 1) {
		sprintf(buf, "<- %s", s->cars[car->_pos - 2]->_name);
		GfuiPrintString(buf, clr, GFUI_FONT_SMALL_C, x, y, GFUI_ALIGN_HL_VB);
		if (s->cars[car->_pos - 2]->_laps == car->_laps) {
			grWriteTime(clr, GFUI_FONT_SMALL_C, x3, y, s->cars[car->_pos - 2]->_curTime-car->_curTime, 1);
		} else {
			GfuiPrintString("       --:--", clr, GFUI_FONT_SMALL_C, x3, y, GFUI_ALIGN_HR_VB);
		}
	} else {
		GfuiPrintString("<- ", clr, GFUI_FONT_SMALL_C, x, y, GFUI_ALIGN_HL_VB);
		GfuiPrintString("       --:--", clr, GFUI_FONT_SMALL_C, x3, y, GFUI_ALIGN_HR_VB);
	}
	y -= dy;
	
	if (car->_pos != s->_ncars) {
		sprintf(buf, "-> %s", s->cars[car->_pos]->_name);
		GfuiPrintString(buf, clr, GFUI_FONT_SMALL_C, x, y, GFUI_ALIGN_HL_VB);
		if (s->cars[car->_pos]->_laps == car->_laps) {
			grWriteTime(clr, GFUI_FONT_SMALL_C, x3, y, s->cars[car->_pos]->_curTime-car->_curTime, 1);    
		} else {
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

#define ALIGN_CENTER	0
#define ALIGN_LEFT	1
#define ALIGN_RIGHT	2

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
	int	ledNb     = 20;
	int ledHeight = 10;
	int ledWidth  = 5;
	int ledSpace  = 2;
	int ledRed    = (int)((car->_enginerpmRedLine * .9 / car->_enginerpmMax) * (tdble)ledNb);
	int ledLit	  = (int)((car->_enginerpm / car->_enginerpmMax) * (tdble)ledNb);

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
		glVertex2f(x + ledNb * (ledWidth+ ledSpace), Winy);
		glVertex2f(x - ledSpace, Winy);
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
#define DD	1
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
	
	grDispEngineLeds (car, Winx + Winw / 2,  Winy + MAX(GfuiFontHeight(GFUI_FONT_BIG_C), GfuiFontHeight(GFUI_FONT_DIGIT)), ALIGN_CENTER, 1);
	
	x = Winx + Winw/2;
	y = Winy;
	if (car->_gear <= 0)
		sprintf(buf, " kph %s", car->_gear == 0 ? "N" : "R");
	else
		sprintf(buf, " kph %d", car->_gear);
	GfuiPrintString(buf, grBlue, GFUI_FONT_BIG_C, x, y, GFUI_ALIGN_HL_VB);
	
	x = Winx + Winw/2;
	sprintf(buf, "%3d", abs((int)(car->_speed_x * 3.6)));
	GfuiPrintString(buf, grBlue, GFUI_FONT_BIG_C, x, y, GFUI_ALIGN_HR_VB);
}

/*
 * 
 * name: grDispLeaderBoard
 * Displayes the leader board (in the lower left corner of the screen)
 * If [drawLaps] is on, writes the lap counter on the top of the list.
 * @param	tCarElt *car: the car currently being viewed
 * @param tSituation *s: situation
 * @return: void
 */
void
cGrBoard::grDispLeaderBoard(const tCarElt *car, const tSituation *s) const
{
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
		{	//Static leaderboard
    char buf[256];

    int current = 0;	//Position of the currently displayed car
    for(int i = 0; i < s->_ncars; i++) {
      if (car == s->cars[i]) {
        current = i;
        break;
      }
    }//for i
    
    //Coords, limits
    int x = Winx + 5;
    int x2 = Winx + 170;
    int y = Winy + 10;
    int dy = GfuiFontHeight(GFUI_FONT_SMALL_C);
    int maxLines = MIN(leaderNb, s->_ncars);	//Max # of lines to display (# of cars in race or max 10 by default)
    int drawLaps = MIN(1, leaderFlag - 1);	//Display # of laps on top of the list?

    //Set up drawing area
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA) ;
    glBegin(GL_QUADS);
    glColor4f(0.1, 0.1, 0.1, 0.8);
    glVertex2f(x, Winy + 5);
    glVertex2f(Winx + 180, Winy + 5);
    glVertex2f(Winx + 180, y + dy * (maxLines + drawLaps));
    glVertex2f(x, y + dy * (maxLines + drawLaps));
    glEnd();
    glDisable(GL_BLEND);
    
    //Display current car in last line (ie is its position >= 10)?
    int drawCurrent = (current >= maxLines) ? 1 : 0;

    //The board is built from bottom up:
    //driver position #10/current is drawn first,
    //then from #9 onto #1, then the text 'Laps' if drawLaps requires.
    for(int j = maxLines; j > 0; j--) {
      int i;	//index of driver to be displayed
      if (drawCurrent) {
        i = current;
        drawCurrent = 0;
      } else {
        i = j - 1;
      }//if drawCurrent

      //Set colour of the currently displayed driver to that
      //defined in the driver's XML file
      float *clr;	//colour to display driver name
      if (i == current) {
        clr = grCarInfo[car->index].iconColor;
        drawCurrent = 0;
      } else {
        clr = grWhite;
      }//if i

      //Driver position + name
      sprintf(buf, "%3d: %s", i + 1, s->cars[i]->_name);
      GfuiPrintString(buf, clr, GFUI_FONT_SMALL_C, x, y, GFUI_ALIGN_HL_VB);

      //Display driver time / time behind leader / laps behind leader
			string sEntry = grGenerateLeaderBoardEntry(s->cars[i], (i==0));
      if(s->cars[i]->_state & RM_CAR_STATE_DNF)	{	//driver DNF, show 'out' in red
				clr = grRed;
			}
      GfuiPrintString(sEntry.c_str(), clr, GFUI_FONT_SMALL_C, x2, y, GFUI_ALIGN_HR_VB);
      
      y += dy;	//'Line feed'
    }//for j

    //Write 'Lap X/Y' on top of the leader board
    if (drawLaps) {
      GfuiPrintString(" Lap:", grWhite, GFUI_FONT_SMALL_C, x, y, GFUI_ALIGN_HL_VB);
      sprintf(buf, "%d / %d", s->cars[0]->_laps, s->_totLaps);
      GfuiPrintString(buf, grWhite, GFUI_FONT_SMALL_C, x2, y, GFUI_ALIGN_HR_VB);
    }//if drawLaps
  }//else
}//grDispLeaderBoard


class myLoaderOptions : public ssgLoaderOptions
{
public:
  virtual void makeModelPath ( char* path, const char *fname ) const
  {
    ulFindFile ( path, model_dir, fname, NULL ) ;
  }

  virtual void makeTexturePath ( char* path, const char *fname ) const
  {
    ulFindFile ( path, texture_dir, fname, NULL ) ;
  }
} ;

void
cGrBoard::grDispCounterBoard2(tCarElt *car)
{
	int index;
	tgrCarInstrument *curInst;
	tdble val;
	char buf[32];
	
	index = car->index;	/* current car's index */
	curInst = &(grCarInfo[index].instrument[0]);
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA) ;
	glEnable(GL_TEXTURE_2D);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glBindTexture(GL_TEXTURE_2D, curInst->texture->getTextureHandle());
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
		strcpy(buf, car->_gear == 0 ? "N" : "R");
	else
		sprintf(buf, "%d", car->_gear);
	GfuiPrintString(buf, grRed, GFUI_FONT_LARGE_C,
			(int)curInst->digitXCenter, (int)curInst->digitYCenter, GFUI_ALIGN_HC_VB);
	
	
	curInst = &(grCarInfo[index].instrument[1]);
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA) ;
	glEnable(GL_TEXTURE_2D);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glBindTexture(GL_TEXTURE_2D, curInst->texture->getTextureHandle());
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
		sprintf(buf, "%d", abs((int)(car->_speed_x * 3.6)));
		GfuiPrintString(buf, grRed, GFUI_FONT_LARGE_C,
			(int)curInst->digitXCenter, (int)curInst->digitYCenter, GFUI_ALIGN_HC_VB);
	}
	
	if (counterFlag == 1){
		grDispMisc(car);
	}
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
}

void
cGrBoard::grDispArcade(tCarElt *car, tSituation *s)
{
	int  x, y;
	int  dy;
	char buf[256];
	float *clr;

#define XM	15
#define YM	10

	x = XM;
	dy = GfuiFontHeight(GFUI_FONT_BIG_C);
	y = Winy + Winh - YM - dy;
	sprintf(buf, "%d/%d", car->_pos, s->_ncars);
	GfuiPrintString(buf, grDefaultClr, GFUI_FONT_BIG_C, x, y, GFUI_ALIGN_HL_VB);

	dy = GfuiFontHeight(GFUI_FONT_LARGE_C);
	y -= dy;
	GfuiPrintString("Time:", grDefaultClr, GFUI_FONT_LARGE_C, x, y, GFUI_ALIGN_HL_VB);
	grWriteTime(grDefaultClr, GFUI_FONT_LARGE_C, x + 150, y, car->_curLapTime, 0);

	y -= dy;
	GfuiPrintString("Best:", grDefaultClr, GFUI_FONT_LARGE_C, x, y, GFUI_ALIGN_HL_VB);
	grWriteTime(grDefaultClr, GFUI_FONT_LARGE_C, x + 150, y, car->_bestLapTime, 0);

	x = Winx + Winw - XM;
	y = Winy + Winh - YM - dy;
	sprintf(buf, "Lap: %d/%d", car->_laps, s->_totLaps);
	GfuiPrintString(buf, grDefaultClr, GFUI_FONT_LARGE_C, x, y, GFUI_ALIGN_HR_VB);
	

	x = Winx + Winw / 2;
	sprintf(buf, "%s", car->_name);
	GfuiPrintString(buf, grDefaultClr, GFUI_FONT_LARGE_C, x, y, GFUI_ALIGN_HC_VB);


	if (car->_fuel < 5.0) {
		clr = grRed;
	} else {
		clr = grWhite;
	}
	grDrawGauge(XM, 20.0, 80.0, clr, grBlack, car->_fuel / car->_tank, "F");
	grDrawGauge(XM + 15, 20.0, 80.0, grRed, grGreen, (tdble)(car->_dammage) / grMaxDammage, "D");

	x = Winx + Winw - XM;
	dy = GfuiFontHeight(GFUI_FONT_LARGE_C);
	y = YM + dy;
	sprintf(buf, "%3d km/h", abs((int)(car->_speed_x * 3.6)));
	GfuiPrintString(buf, grDefaultClr, GFUI_FONT_BIG_C, x, y, GFUI_ALIGN_HR_VB);
	y = YM;
	if (car->_gear <= 0)
		sprintf(buf, "%s", car->_gear == 0 ? "N" : "R");
	else
		sprintf(buf, "%d", car->_gear);
	GfuiPrintString(buf, grDefaultClr, GFUI_FONT_LARGE_C, x, y, GFUI_ALIGN_HR_VB);

	grDispEngineLeds (car, Winx + Winw - XM, YM + dy + GfuiFontHeight (GFUI_FONT_BIG_C), ALIGN_RIGHT, 0);
}


void cGrBoard::refreshBoard(tSituation *s, float Fps, int forceArcade, tCarElt *curr)
{
	if (arcadeFlag || forceArcade) {
		grDispArcade(curr, s);
	} else {
		if (debugFlag) grDispDebug(Fps, curr);
		if (GFlag) grDispGGraph(curr);
		if (boardFlag) grDispCarBoard(curr, s);
		if (leaderFlag)	grDispLeaderBoard(curr, s);
		if (counterFlag) grDispCounterBoard2(curr);
	}

	trackMap->display(curr, s, Winx, Winy, Winw, Winh);
}


// TODO: clean solution for cleanup.
static ssgSimpleState* cleanup[1024];
static int nstate = 0;


void grInitBoardCar(tCarElt *car)
{
	char		buf[4096];
	int			index;
	void		*handle;
	const char	*param;
	myLoaderOptions	options ;
	tgrCarInfo		*carInfo;
	tgrCarInstrument	*curInst;
	tdble		xSz, ySz, xpos, ypos;
	tdble		needlexSz, needleySz;
	
	ssgSetCurrentOptions ( &options ) ;
	
	index = car->index;	/* current car's index */
	carInfo = &grCarInfo[index];
	handle = car->_carHandle;
	
	/* Tachometer */
	curInst = &(carInfo->instrument[0]);
	
	/* Load the Tachometer texture */
	param = GfParmGetStr(handle, SECT_GROBJECTS, PRM_TACHO_TEX, "rpm8000.rgb");
	sprintf(buf, "drivers/%s/%d;drivers/%s;cars/%s;data/textures", car->_modName, car->_driverIndex, car->_modName, car->_carName);
	grFilePath = strdup(buf);
	curInst->texture = (ssgSimpleState*)grSsgLoadTexState(param);
	free(grFilePath);
	cleanup[nstate] = curInst->texture;
	nstate++;
	
	/* Load the intrument placement */
	xSz = GfParmGetNum(handle, SECT_GROBJECTS, PRM_TACHO_XSZ, (char*)NULL, 128);
	ySz = GfParmGetNum(handle, SECT_GROBJECTS, PRM_TACHO_YSZ, (char*)NULL, 128);
	xpos = GfParmGetNum(handle, SECT_GROBJECTS, PRM_TACHO_XPOS, (char*)NULL, Winw / 2.0 - xSz);
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
		glColor4f(1.0, 0.0, 0.0, 1.0);
		glVertex2f(0, -needleySz);
		glVertex2f(0, needleySz);
		glVertex2f(needlexSz, -needleySz / 2.0);
		glVertex2f(needlexSz, needleySz / 2.0);
	}
	glEnd();
	glEndList();
	
	
	/* Speedometer */
	curInst = &(carInfo->instrument[1]);
	
	/* Load the Speedometer texture */
	param = GfParmGetStr(handle, SECT_GROBJECTS, PRM_SPEEDO_TEX, "speed360.rgb");
	sprintf(buf, "drivers/%s/%d;drivers/%s;cars/%s;data/textures", car->_modName, car->_driverIndex, car->_modName, car->_carName);
	grFilePath = strdup(buf);
	curInst->texture = (ssgSimpleState*)grSsgLoadTexState(param);
	free(grFilePath);
	cleanup[nstate] = curInst->texture;
	nstate++;
	
	/* Load the intrument placement */
	xSz = GfParmGetNum(handle, SECT_GROBJECTS, PRM_SPEEDO_XSZ, (char*)NULL, 128);
	ySz = GfParmGetNum(handle, SECT_GROBJECTS, PRM_SPEEDO_YSZ, (char*)NULL, 128);
	xpos = GfParmGetNum(handle, SECT_GROBJECTS, PRM_SPEEDO_XPOS, (char*)NULL, Winw / 2.0);
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
		glColor4f(1.0, 0.0, 0.0, 1.0);
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

static int iStart = 0;
static double iTimer = 0;
#define LEADERBOARD_SCROLL_TIME 2.0
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

  int current = 0;	//Position of the currently displayed car
  for(int i = 0; i < s->_ncars; i++) {
    if (car == s->cars[i]) {
      current = i;
      break;
    }
  }//for i

	//Coords, limits
	int x = Winx + 5;
	int x2 = Winx + 170;
	int y = Winy + 10;
	int dy = GfuiFontHeight(GFUI_FONT_SMALL_C);
	int maxLines = MIN(leaderNb, s->_ncars);	//Max # of lines to display (# of cars in race or max 10 by default)
	
	//Set up drawing area
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA) ;
	glBegin(GL_QUADS);
	glColor4f(0.1, 0.1, 0.1, 0.8);
	glVertex2f(x, Winy + 5);
	glVertex2f(Winx + 180, Winy + 5);
	glVertex2f(Winx + 180, y + dy * (maxLines + 1));
	glVertex2f(x, y + dy * (maxLines + 1));
	glEnd();
	glDisable(GL_BLEND);


	//The board is built from bottom up:
	//driver position #10/current is drawn first,
	//then from #9 onto #1, then the text 'Laps' if drawLaps requires.
	for(int j = maxLines - 1; j >= 0; j--) {
		int i = j + iStart;	//index of driver to be displayed

		if(i == s->_ncars) {
			//print empty line after last car
		}	else {
      i = i % (s->_ncars + 1);

			//Set colour of the drivers to that
			//defined in the drivers' XML file.
			//Current driver is yellow...
      float *clr = (i == current)
				? grDefaultClr : grCarInfo[s->cars[i]->index].iconColor;

			//Driver position + name
			sprintf(buf, "%3d: %s", i + 1, s->cars[i]->_name);
			GfuiPrintString(buf, clr, GFUI_FONT_SMALL_C, x, y, GFUI_ALIGN_HL_VB);
			
      //Display driver time / time behind leader / laps behind leader
			string sEntry = grGenerateLeaderBoardEntry(s->cars[i], (i==0));
      if(s->cars[i]->_state & RM_CAR_STATE_DNF)	{	//driver DNF, show 'out' in red
				clr = grRed;
			}
      GfuiPrintString(sEntry.c_str(), clr, GFUI_FONT_SMALL_C, x2, y, GFUI_ALIGN_HR_VB);
		}//else i
		y += dy;	//'Line feed'
	}//for j

	//Write 'Lap X/Y' on top of the leader board
	GfuiPrintString(" Lap:", grWhite, GFUI_FONT_SMALL_C, x, y, GFUI_ALIGN_HL_VB);
	sprintf(buf, "%d / %d", s->cars[0]->_laps, s->_totLaps);
	GfuiPrintString(buf, grWhite, GFUI_FONT_SMALL_C, x2, y, GFUI_ALIGN_HR_VB);
}//grDispLeaderBoardScroll


static int iStringStart = 0;
static int iRaceLaps = -1;
#define LEADERBOARD_LINE_SCROLL_TIME 0.1
/*
 * 
 * name: grDispLeaderBoardScrollLine
 * Scrolls the leaderboard on the bottom line, as seen on TV broadcasts.
 * @param:	tCarElt* car
 * @param:	tSituation *s
 * @return:	void
 */
void
cGrBoard::grDispLeaderBoardScrollLine(const tCarElt *car, const tSituation *s) const
{
	//Scrolling
	if(iTimer == 0 || s->currentTime < iTimer) iTimer = s->currentTime;
	if(s->currentTime >= iTimer + LEADERBOARD_LINE_SCROLL_TIME)
		{
			//When in initial position, show it fixed (no scroll) for 3 secs.
			if(
				(iStringStart == 0 && s->currentTime >= iTimer + LEADERBOARD_LINE_SCROLL_TIME * 1)//15)
				||
				(iStringStart > 0)
			)
				{
					iTimer = s->currentTime;
					++iStringStart;
				}
		}
		
	//Coords, limits
	int x = Winx + 5;
	int x2 = Winw - 5;
	int y = Winy;
	int dy = GfuiFontHeight(GFUI_FONT_MEDIUM_C);
	//int dx = GfuiFontWidth(GFUI_FONT_SMALL_C, "W");
	//int iLen = (x2 - x) / dx;	//# of chars to display
	
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

	static string st;	//This is the line we will display
	
	//Another lap gone by?
	if(s->cars[0]->race.laps > iRaceLaps) {
		//Let's regenerate the roster.
		//The roster holds the driver's position, name and difference
		//at the time the leader starts a new lap.
		//So it can happen it is somewhat mixed up, it will settle down
		//in the next lap.
		iRaceLaps++;
		
		char buf[256];
		string roster;

		sprintf(buf, "Lap %d | ", iRaceLaps);
		roster.append(buf);

		for(int i = 0; i < s->_ncars; i++) {
			//Driver position + name
			sprintf(buf, "%3d. %s", i + 1, s->cars[i]->_name);
			roster.append(buf);

      //Display driver time / time behind leader / laps behind leader
			string sEntry = grGenerateLeaderBoardEntry(s->cars[i], (i==0));

			//get rid of leading spaces
			size_t iCut = sEntry.find_first_not_of(' ');
			if(iCut != string::npos && iCut != 0)
				{
					sEntry = sEntry.substr(iCut - 1);
				}
			//Add to roster, then separate it from next one
			roster.append(sEntry);
			roster.append("   ");
		}//for i
	
		string sep("   TORCS-NG   ");	//separator
	
		if(st.empty())
			st.assign(roster + sep);
		else {
			st.append(roster + sep);
			st = st.substr(iStringStart);	//Let's make sure the string won't grow big
			iStringStart = 0;
		}//TODO!!! It can happen garbage remains at the beginning of st.
		//Somehow should get rid of it when it moves out of screen.
	}
	
	//Display the line
	GfuiPrintString(st.c_str() + iStringStart, grWhite, GFUI_FONT_MEDIUM_C, x, y, GFUI_ALIGN_HL_VB);
	//iStringStart = (iStringStart == st.size() ? 0 : iStringStart);
	iStringStart = iStringStart % st.size();
}//grDispLeaderBoardScrollLine


/*
 * 
 * name: grGenerateLeaderBoardEntry
 * 
 * Generates one leaderboard entry,
 * this time only the time behind/laps behind part.
 * Optimally it would make up the whole string that can be shown
 * in the line, but colour handling and positioning
 * is not available then.
 * 
 * @param:	tCarElt* car
 * @param:	bool isLeader
 * @return:	string
 */
string
cGrBoard::grGenerateLeaderBoardEntry(const tCarElt* car, const bool isLeader) const
{
	string ret;
	char buf[256];
	
	//Display driver time / time behind leader / laps behind leader
	if(car->_state & RM_CAR_STATE_DNF)	{
		//driver DNF
		sprintf(buf, "       out");
	} else {
		//no DNF
		if(isLeader) {
			//This is the leader, put out his time
			grWriteTimeBuf(buf, car->_curTime, 0);
		} else {
			//This is not the leader
			switch(car->_lapsBehindLeader) {
				case 0:	//Driver in same lap as leader
					if (car->_timeBehindLeader == 0) {
						//Cannot decide time behind, first lap or passed leader
						sprintf(buf, "       --:--");
					}
					else {
						//Can decide time behind
						grWriteTimeBuf(buf, car->_timeBehindLeader, 1);
					}
					break;
					
				case 1:	//1 lap behind leader
					sprintf(buf, "+%3d Lap", car->_lapsBehindLeader);
					break;
					
				default:	//N laps behind leader
					sprintf(buf, "+%3d Laps", car->_lapsBehindLeader);
					break;
			}//switch	
		}//not leader
	}//driver not DNF
	ret.assign(buf);
	
	return ret;
}//grGenerateLeaderBoardEntry
