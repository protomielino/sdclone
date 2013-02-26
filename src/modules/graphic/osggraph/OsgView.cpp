/***************************************************************************

    file        : OsgViewer.cpp
    created     : Sun Jan 13 22:11:03 CEST 2013
    copyright   : (C) 2013 by Xavier Bertaux
    email       : bertauxx@yahoo.fr
    version     : $Id$

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/



#include <osg/Camera>
#include <tgfclient.h>



#include "OsgMain.h"
#include "OsgView.h"
//#include "OsgCar.h"

static char buf[1024];
static char path[1024];
static char path2[1024];

osg::Vec3 eye, center, up, speed;


SDViewer::SDViewer(osg::Camera * c)
{
    cam = c;
    id = 0;
	curCar = NULL;
    //curCam = NULL;
    //mirrorCam = NULL;
    //dispCam = NULL;
    //boardCam = NULL;
    //bgCam = NULL;
    //board = NULL;
	curCamHead = 0;
	drawCurrent = 0;
	active = false;
	selectNextFlag = false;
	selectPrevFlag = false;
	mirrorFlag = 1;
    //memset(cams, 0, sizeof(cams));
    //viewRatio = 1.33;
	cars = 0;
	
	scrx = 0;
	scry = 0;
	scrw = 800;
	scrh = 600;
}

SDViewer::~SDViewer()
{
    //int i;
	FREEZ(cars);
}

void SDViewer::switchMirror(void)
{
	mirrorFlag = 1 - mirrorFlag;
	sprintf(path, "%s/%d", GR_SCT_DISPMODE, id);
	GfParmSetNum(grHandle, path, GR_ATT_MIRROR, NULL, (tdble)mirrorFlag);

	if (curCar->_driverType == RM_DRV_HUMAN) 
	{
		sprintf(path2, "%s/%s", GR_SCT_DISPMODE, curCar->_name);
		GfParmSetNum(grHandle, path2, GR_ATT_MIRROR, NULL, (tdble)mirrorFlag);
	}
	
	GfParmWriteFile(NULL, grHandle, "Graph");
}

void SDViewer::Init(tSituation *s)
{
    loadParams(s);
}



void SDViewer::camDraw(tSituation *s)
{
    GfProfStartProfile("dispCam->beforeDraw*");
    //dispCam->beforeDraw();
    GfProfStopProfile("dispCam->beforeDraw*");

    //glDisable(GL_COLOR_MATERIAL);
	
	GfProfStartProfile("dispCam->update*");
    //dispCam->update(curCar, s);
	GfProfStopProfile("dispCam->update*");


}

/* Update screen display */
void SDViewer::update(tSituation *s, const SDFrameInfo* frameInfo)
{
    /*if (!active)
	{
		return;
    }*/
	
	int carChanged = 0;
	if (selectNextFlag) 
	{
		for (int i = 0; i < (s->_ncars - 1); i++) 
		{
			if (curCar == s->cars[i]) 
			{
				curCar = s->cars[i + 1];
				carChanged = 1;
                GfOut("Car Next\n");
				break;
			}
		}
		
		selectNextFlag = false;
	}

	if (selectPrevFlag) 
	{
		for (int i = 1; i < s->_ncars; i++) 
		{
			if (curCar == s->cars[i]) 
			{
				curCar = s->cars[i - 1];
				carChanged = 1;
                GfOut("Car Previous\n");
				break;
			}
		}
		
		selectPrevFlag = false;
	}
	
	if (carChanged) 
	{
		sprintf(path, "%s/%d", GR_SCT_DISPMODE, id);
		GfParmSetStr(grHandle, path, GR_ATT_CUR_DRV, curCar->_name);
		loadParams (s);
        //board->setWidth(fakeWidth);
		GfParmWriteFile(NULL, grHandle, "Graph");
        //curCam->onSelect(curCar, s);
	}

    //int	i;
    int nb = s->_ncars;
    //viewer->update(s, &frameInfo);
    tCarElt *car = getCurrentCar();



    osg::Vec3 P, p;
    float offset = 0;
    int Speed = 0;

    p[0] = car->_drvPos_x;
    p[1] = car->_drvPos_y;
    p[2] = car->_drvPos_z;

    float t0 = p[0];
    float t1 = p[1];
    float t2 = p[2];

    p[0] = t0*car->_posMat[0][0] + t1*car->_posMat[1][0] + t2*car->_posMat[2][0] + car->_posMat[3][0];
    p[1] = t0*car->_posMat[0][1] + t1*car->_posMat[1][1] + t2*car->_posMat[2][1] + car->_posMat[3][1];
    p[2] = t0*car->_posMat[0][2] + t1*car->_posMat[1][2] + t2*car->_posMat[2][2] + car->_posMat[3][2];

        //GfOut("Car X = %f - P0 = %f\n", car->_pos_X, P[0]);

    eye[0] = p[0];
    eye[1] = p[1];
    eye[2] = p[2];


    // Compute offset angle and bezel compensation)
    /*if (spansplit && viewOffset) {
        offset += (viewOffset - 10 + (int((viewOffset - 10) * 2) * (bezelcomp - 100)/200)) *
            atan(screen->getViewRatio() / spanaspect * tan(spanfovy * M_PI / 360.0)) * 2;
        fovy = spanfovy;
    }*/

    P[0] = (car->_pos_X + 30.0 * cos(car->_glance + offset + car->_yaw));
    P[1] = (car->_pos_Y + 30.0 * sin(car->_glance + offset + car->_yaw));
    P[2] = car->_pos_Z + car->_yaw;
        //osgXformPnt3(P, car->_posMat);

    center[0] = P[0];
    center[1] = P[1];
    center[2] = P[2];

    up[0] = car->_posMat[2][0];
    up[1] = car->_posMat[2][1];
    up[2] = car->_posMat[2][2];

    speed[0] = car->pub.DynGCg.vel.x;
    speed[1] = car->pub.DynGCg.vel.y;
    speed[2] = car->pub.DynGCg.vel.z;

    Speed = car->_speed_x * 3.6;

    //osg::Camera * camera = m_sceneViewer->getCamera();

    cam->setComputeNearFarMode(osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR);

    cam->setViewMatrixAsLookAt( eye, center, up);

}

Camera* SDViewer::getCamera(){
    Camera * c = new Camera;
    c->Centerv = &center._v;
    c->Posv = &eye._v;
    c->Upv = &up._v;
    c->Speedv = &speed._v;
}

void SDViewer::loadParams(tSituation *s)
{
	int camNum;
	int i;
	//class cGrCamera *cam;
	const char *carName;

	// Initialize the screen "current car" if not already done.
	sprintf(path, "%s/%d", GR_SCT_DISPMODE, id);
	
	if (!curCar) 
	{
		// Load the name of the "current driver", and search it in the race competitors.
		carName = GfParmGetStr(grHandle, path, GR_ATT_CUR_DRV, "");
		for (i = 0; i < s->_ncars; i++) 
		{
			if (!strcmp(s->cars[i]->_name, carName)) 
			{
				break;
			}
		}

		// Found : this is the "current driver".
		if (i < s->_ncars) 
		{
			curCar = s->cars[i];
		} else if (id < s->_ncars) 
		{
			curCar = s->cars[id];
		} else 
		{
			curCar = s->cars[0];
		}
		
		GfLogTrace("Screen #%d : Assigned to %s\n", id, curCar->_name);
	}

	// Load "current camera" settings (attached to the "current car").
	sprintf(path2, "%s/%s", GR_SCT_DISPMODE, curCar->_name);
    GfOut("Driver Name Camera = %s\n", curCar->_name);
	curCamHead	= (int)GfParmGetNum(grHandle, path, GR_ATT_CAM_HEAD, NULL, 9);
	camNum	= (int)GfParmGetNum(grHandle, path, GR_ATT_CAM, NULL, 0);
	mirrorFlag	= (int)GfParmGetNum(grHandle, path, GR_ATT_MIRROR, NULL, (tdble)mirrorFlag);
	curCamHead	= (int)GfParmGetNum(grHandle, path2, GR_ATT_CAM_HEAD, NULL, (tdble)curCamHead);
	camNum	= (int)GfParmGetNum(grHandle, path2, GR_ATT_CAM, NULL, (tdble)camNum);
	mirrorFlag	= (int)GfParmGetNum(grHandle, path2, GR_ATT_MIRROR, NULL, (tdble)mirrorFlag);
}
