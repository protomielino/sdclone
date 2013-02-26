/***************************************************************************

    file                 : OsgCamera.cpp
    created              : Tue Feb 26 12:24:02 CEST 2013
    copyright            : (C) 2013 by Gaëtan André
    email                : gaetan.andre@gmail.com
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

#include <osg/Camera>
#include <osg/Matrix>

#include <car.h>
#include <raceman.h>
#include <robottools.h>
#include <tgf.h>
#include <guiscreen.h>
#include <graphic.h>

#include "OsgCamera.h"
#include "OsgView.h"
#include "OsgScenery.h"

//static char path[1024];

static float spanfovy;
static float bezelComp;
static float screenDist;
static float arcRatio;
static float spanaspect;
//void *grHandle;
static tdble spanA;

static double lastTime;


SDCamera::SDCamera(SDView  * c, int myid, int mydrawCurrent, int mydrawdrv, int mydrawBackground, int mymirrorAllowed){
    screen = c;
    id = myid;
    drawCurrent = mydrawCurrent;
    drawDriver = mydrawdrv;
    drawBackground = mydrawBackground;
    mirrorAllowed = mymirrorAllowed;
    speed[0] = speed[1] = speed[2] = 0.0;
    eye[0] = eye[1] = eye[2] = 0.0;
    center[0] = center[1] = center[2] = 0.0;
    up[0] = up[1] = 0.0; up[2] = 1.0;
}

Camera * SDCamera::getGenericCamera(){
    Camera * c = new Camera;
    c->Centerv = &center._v;
    c->Posv = &eye._v;
    c->Upv = &up._v;
    c->Speedv = &speed._v;
    return c;
}

void SDCamera::update(tCarElt * car, tSituation * s){

    osg::Vec3 P, p;
    float offset = 0;
   // int Speed = 0;

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

    //Speed = car->_speed_x * 3.6;

   // osg::Camera * osgCam = screen->getOsgCam();

   // osgCam->setComputeNearFarMode(osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR);

    //osgCam->setViewMatrixAsLookAt( eye, center, up);
}

// SdPerspCamera ================================================================

SDPerspCamera::SDPerspCamera(SDView *myscreen, int id, int drawCurr, int drawDrv, int drawBG, int mirrorAllowed,
                   float myfovy, float myfovymin, float myfovymax,
                   float myfnear, float myffar, float myfogstart, float myfogend)
    : SDCamera(myscreen, id, drawCurr, drawDrv, drawBG, mirrorAllowed)
{
    fovy     = myfovy;
    fovymin  = myfovymin;
    fovymax  = myfovymax;
    fnear    = myfnear;
    ffar     = myffar;
    fovydflt = myfovy;
    fogstart = myfogstart;
    fogend   = myfogend;


    viewOffset = 0;
    spanOffset = 0;
}

void SDPerspCamera::setProjection(void)
{



    screen->getOsgCam()->setProjectionMatrixAsPerspective(fovy,screen->getViewRatio(),fnear,ffar);

    // PLib takes the field of view as angles in degrees. However, the
    // aspect ratio really aplies to lengths in the projection
    // plane. So we have to transform the fovy angle to a length in
    // the projection plane, apply the aspect ratio and transform the
    // result back to an angle. Care needs to be taken to because the
    // tan and atan functions operate on angles in radians. Also,
    // we're only interested in half the viewing angle.

   /* float fovx = atan(getAspectRatio() / spanaspect * tan(fovy * M_PI / 360.0)) * 360.0 / M_PI;
    grContext.setFOV(fovx, fovy);
    grContext.setNearFar(fnear, ffar);

    // correct view for split screen spanning
    if (viewOffset != 0 && spanOffset != 0) {
    float dist, left, right;

        sgFrustum * frus = grContext.getFrustum();

    //=($A$2/$B$2)-((($A$2/$B$2)-$A$2)*cos(B10))
    if (spanAngle)
        dist = (screenDist / arcRatio) - (((screenDist / arcRatio) - screenDist) * cos(spanAngle));
    else
        dist = screenDist;

    if (dist !=0) {
        left = frus->getLeft() + (spanOffset * frus->getNear()/dist);
        right = frus->getRight() + (spanOffset * frus->getNear()/dist);
#if 0
        GfLogInfo("Adjusting ViewOffset %f : Frustum %f : dist %f : left %f -> %1.12f, Right %f -> %1.12f, near %f\n",
            viewOffset, spanOffset, dist,
            frus->getLeft(), left, //frus->getLeft() + spanOffset,
            frus->getRight(), right, //frus->getRight() + spanOffset,
            frus->getNear());
#endif
            frus->setFrustum(left, right,
            frus->getBot(), frus->getTop(),
            frus->getNear(), frus->getFar());
        }
    }*/
}

void SDPerspCamera::setModelView(void)
{
  screen->getOsgCam()->setViewMatrixAsLookAt(eye,center,up);
}

void SDPerspCamera::loadDefaults(char *attr)
{
   /* sprintf(path, "%s/%d", GR_SCT_DISPMODE, screen->getId());
    fovy = (float)GfParmGetNum(grHandle, path,
                   attr, (char*)NULL, fovydflt);
    limitFov();*/
}


/* Give the height in pixels of 1 m high object on the screen at this point */
float SDPerspCamera::getLODFactor(float x, float y, float z) {
    tdble	dx, dy, dz, dd;
    float	ang;
    int		scrh;//, dummy;
    float	res;

    dx = x - eye[0];
    dy = y - eye[1];
    dz = z - eye[2];

    dd = sqrt(dx*dx+dy*dy+dz*dz);

    ang = DEG2RAD(fovy / 2.0);
   // GfScrGetSize(&dummy, &scrh, &dummy, &dummy);

    res = (float)scrh / 2.0 / dd / tan(ang);
    if (res < 0) {
    res = 0;
    }
    return res;
}

float SDPerspCamera::getSpanAngle(void)
{
    float angle = 0;

    // check if already computed
    if (fovy == spanfovy)
    return spanAngle;

    fovy = spanfovy;

    //PreCalculate the spanOffset
    if (viewOffset) {
    //=2*$A$2*$D$2*tan(radians($C$2)/2)
    float width = 2 * (bezelComp / 100) * screenDist * tan(spanfovy * M_PI / 360.0) * screen->getViewRatio() / spanaspect;

#if 1
    // New method
    if (arcRatio > 0) {
        //=if($B$2=0,0,2*atan($A$5*$B$2/(2*$A$2)))
            float fovxR = 2 * atan(width * arcRatio / (2 * screenDist));

        //=A10*$B$5
            angle = (viewOffset - 10) * fovxR;

        //=if($B$2=0,A10*$A$5,abs($A$2/$B$2)-$A$2)/sqrt(tan(radians(90)-B10)^2+1)*if(A10>0,-1,1)
        spanOffset = fabs((screenDist / arcRatio) - screenDist) / sqrt((tan((M_PI/2) - angle) * tan((M_PI/2) - angle)) + 1);

        if (viewOffset < 10) spanOffset *= -1;
        if (arcRatio > 1) spanOffset *= -1;
    } else {
        // monitors mounted flat on wall
        angle = 0;
        spanOffset = (viewOffset - 10) * width;
    }
#else
    // Old method
    angle = (viewOffset - 10 + (int((viewOffset - 10) * 2) * (bezelComp - 100)/200)) *
        atan(screen->getViewRatio() / spanaspect * tan(spanfovy * M_PI / 360.0)) * 2;

    spanOffset = 0;
#endif
    spanAngle = angle;

    GfLogInfo("ViewOffset %f : fovy %f, arcRatio %f => width %f, angle %f, SpanOffset %f\n", viewOffset, fovy, arcRatio, width, angle, spanOffset);
    }

    return angle;
}

void SDPerspCamera::setViewOffset(float newOffset)
{
    viewOffset = newOffset;

    //PreCalculate the spanAngle and spanOffset
    if (newOffset) {
    spanfovy = fovy;
    fovy = 0;
        spanAngle = getSpanAngle();
    } else {
    //spanAngle = 0;
    spanOffset = 0;
    }
}

void SDPerspCamera::setZoom(int cmd)
{
    //char	buf[256];


    switch(cmd) {
    case GR_ZOOM_IN:
    if (fovy > 2) {
        fovy--;
    } else {
        fovy /= 2.0;
    }
    if (fovy < fovymin) {
        fovy = fovymin;
    }
    break;

    case GR_ZOOM_OUT:
    fovy++;
    if (fovy > fovymax) {
        fovy = fovymax;
    }
    break;

    case GR_ZOOM_MIN:
    fovy = fovymax;
    break;

    case GR_ZOOM_MAX:
    fovy = fovymin;
    break;

    case GR_ZOOM_DFLT:
    fovy = fovydflt;
    break;
    }

    limitFov();

    if (viewOffset) {
    spanfovy = fovy;
    fovy = 0;
        spanAngle = getSpanAngle();
    } else {
    //spanAngle = 0;
    spanOffset = 0;
    }

   this->setProjection();
   // sprintf(buf, "%s-%d-%d", GR_ATT_FOVY, screen->getCameras()->getIntSelectedCamera(), getId());
    //sprintf(path, "%s/%d", GR_SCT_DISPMODE, screen->getId());
    //GfParmSetNum(grHandle, path, buf, (char*)NULL, (tdble)fovy);
    //GfParmWriteFile(NULL, grHandle, "Graph");
}


class SDCarCamInsideDriverEye : public SDPerspCamera
{
 public:
    SDCarCamInsideDriverEye(SDView *myscreen, int id, int drawCurr, int drawBG,
            float myfovy, float myfovymin, float myfovymax,
            float myfnear, float myffar = 1500.0,
            float myfogstart = 1400.0, float myfogend = 1500.0)
    : SDPerspCamera(myscreen, id, drawCurr, 0, drawBG, 1,
             myfovy, myfovymin, myfovymax,
             myfnear, myffar, myfogstart, myfogend) {
    }

    void update(tCarElt *car, tSituation *s) {
    sgVec3 P, p;
    float offset = 0;

    p[0] = car->_drvPos_x;
    p[1] = car->_bonnetPos_y;
    p[2] = car->_drvPos_z;
    sgXformPnt3(p, car->_posMat);

    eye[0] = p[0];
    eye[1] = p[1];
    eye[2] = p[2];

    // Compute offset angle and bezel compensation)
    if (viewOffset) {
        offset += getSpanAngle();
    }

    P[0] = car->_drvPos_x + 30.0 * cos(2*PI/3 * car->_glance + offset);
    P[1] = car->_bonnetPos_y - 30.0 * sin(2*PI/3 * car->_glance + offset);
    P[2] = car->_drvPos_z;
    sgXformPnt3(P, car->_posMat);

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
    }
};


// cGrCarCamInsideDynDriverEye ====================================================
// Change define value to choose desired dynamic behaviour of the CamInsideDriverEye cameras
// * 1 = Torcs's one : strange rotation of the camera around speed vector axis
// * 2 = Use attenuated car yaw to translate camera center (by Andrew)
#define CamDriverEyeDynamicBehaviour 3

class SDCarCamInsideDynDriverEye : public SDCarCamInsideDriverEye
{
#if (CamDriverEyeDynamicBehaviour != 1)
 private:
    tdble PreA;
#endif

 public:
    SDCarCamInsideDynDriverEye(SDView *myscreen, int id, int drawCurr, int drawBG,
            float myfovy, float myfovymin, float myfovymax,
            float myfnear, float myffar = 1500.0,
            float myfogstart = 1400.0, float myfogend = 1500.0)
    : SDCarCamInsideDriverEye(myscreen, id, drawCurr, drawBG,
             myfovy, myfovymin, myfovymax,
             myfnear, myffar, myfogstart, myfogend) {
#if (CamDriverEyeDynamicBehaviour == 1)
    up[0] = 0;
    up[1] = 0;
    up[2] = 1;
#else
    PreA = 0.0f;
#endif
    }

    void update(tCarElt *car, tSituation *s) {
    sgVec3 P, p;
    float offset = 0;

    p[0] = car->_drvPos_x;
    p[1] = car->_drvPos_y;
    p[2] = car->_drvPos_z;
    sgXformPnt3(p, car->_posMat);

    eye[0] = p[0];
    eye[1] = p[1];
    eye[2] = p[2];

    // Compute offset angle and bezel compensation)
    if (viewOffset) {
        offset += getSpanAngle();
    }

    P[0] = car->_drvPos_x + 30.0 * cos(2*PI/3 * car->_glance + offset);
    P[1] = car->_drvPos_y - 30.0 * sin(2*PI/3 * car->_glance + offset);
    P[2] = car->_drvPos_z;

#if (CamDriverEyeDynamicBehaviour == 3)
    tdble A = 0;

    // We want uniform movement across split screens when 'spanning'
    if (viewOffset && lastTime == s->currentTime) {
        A = spanA;
    } else {
        A = car->_yaw;
        if (fabs(PreA - A) > fabs(PreA - A + 2*PI)) {
            PreA += 2*PI;
        } else if (fabs(PreA - A) > fabs(PreA - A - 2*PI)) {
            PreA -= 2*PI;
        }
        RELAXATION(A, PreA, 8.0f);
        spanA = A;
    }
    lastTime = s->currentTime;

    // ignore head movement if glancing left/right
    if (car->_glance == 0) {
        tdble headTurn = (A - car->_yaw)/2;

        if (headTurn > PI/3) headTurn = PI/3;
        if (headTurn < -PI/3) headTurn = -PI/3;

        P[0] = car->_drvPos_x + 30.0 * cos(2*PI/3 * car->_glance + offset + headTurn);
        P[1] = car->_drvPos_y - 30.0 * sin(2*PI/3 * car->_glance + offset + headTurn);
    }
#endif

    sgXformPnt3(P, car->_posMat);

#if (CamDriverEyeDynamicBehaviour == 2)
    tdble A = 0;

    // We want uniform movement across split screens when 'spanning'
    if (viewOffset && lastTime == s->currentTime) {
        A = spanA;
    } else {
        A = car->_yaw;
        if (fabs(PreA - A) > fabs(PreA - A + 2*PI)) {
            PreA += 2*PI;
        } else if (fabs(PreA - A) > fabs(PreA - A - 2*PI)) {
            PreA -= 2*PI;
        }
        RELAXATION(A, PreA, 4.0f);
        spanA = A;
    }
    lastTime = s->currentTime;

    // ignore if glancing left/right
    if (car->_glance != 0)
        A = 0;

    const tdble CosA = cos(A);
    const tdble SinA = sin(A);

    //tdble brake = 0.0f;
    //if (car->_accel_x < 0.0)
    //	brake = MIN(2.0, fabs(car->_accel_x) / 20.0);

    center[0] = P[0] - (10 - 1) * CosA;
    center[1] = P[1] - (10 - 1) * SinA;
    center[2] = P[2]; // - brake;  // this does not work yet

#else
    center[0] = P[0];
    center[1] = P[1];
    center[2] = P[2];
#endif

#if (CamDriverEyeDynamicBehaviour != 1)
    up[0] = car->_posMat[2][0];
    up[1] = car->_posMat[2][1];
    up[2] = car->_posMat[2][2];
#endif

    speed[0] = car->pub.DynGCg.vel.x;
    speed[1] = car->pub.DynGCg.vel.y;
    speed[2] = car->pub.DynGCg.vel.z;

    Speed = car->_speed_x * 3.6;
    }
};

// SDCarCamBehind ================================================================

class SDCarCamBehind : public SDPerspCamera
{
    tdble PreA;

 protected:
    float dist;
    float height;
    float relax;

 public:
    SDCarCamBehind(SDView *myscreen, int id, int drawCurr, int drawBG,
            float fovy, float fovymin, float fovymax,
            float mydist, float myHeight, float fnear, float ffar = 1500.0,
            float myfogstart = 1400.0, float myfogend = 1500.0, float relaxation = 10.0f)
    : SDPerspCamera(myscreen, id, drawCurr, 1, drawBG, 0, fovy, fovymin,
             fovymax, fnear, ffar, myfogstart, myfogend) {
    dist = mydist;
    height = myHeight;
    relax = relaxation;
    PreA = 0.0;
    up[0] = 0;
    up[1] = 0;
    up[2] = 1;
    }

    void update(tCarElt *car, tSituation *s) {
    tdble A;
    float offset = 0;

    // We want uniform movement across split screens when 'spanning'
    if (viewOffset && lastTime == s->currentTime) {
        A = spanA;
    } else {
        A = car->_yaw;
        if (fabs(PreA - A) > fabs(PreA - A + 2*PI)) {
            PreA += 2*PI;
        } else if (fabs(PreA - A) > fabs(PreA - A - 2*PI)) {
            PreA -= 2*PI;
        }
        if (relax > 0.1)
            RELAXATION(A, PreA, relax);
        spanA = A;
    }
    lastTime = s->currentTime;

    eye[0] = car->_pos_X - dist * cos(A + PI * car->_glance);
    eye[1] = car->_pos_Y - dist * sin(A + PI * car->_glance);
    eye[2] = RtTrackHeightG(car->_trkPos.seg, eye[0], eye[1]) + height;

    // Compute offset angle and bezel compensation)
    if (viewOffset) {
        offset += getSpanAngle();
    }

    center[0] = car->_pos_X - dist * cos(A + PI * car->_glance) + dist * cos(A + PI * car->_glance - offset);
    center[1] = car->_pos_Y - dist * sin(A + PI * car->_glance) + dist * sin(A + PI * car->_glance - offset);
    center[2] = car->_pos_Z;

    speed[0] = car->pub.DynGCg.vel.x;
    speed[1] = car->pub.DynGCg.vel.y;
    speed[2] = car->pub.DynGCg.vel.z;

    Speed = car->_speed_x * 3.6;

    //grRain.drawPrecipitation(1, up[2], 0.0, 0.0, eye[2], eye[1], eye[0], Speed);

    }
};

// SDCarCamInsideFixedCar ================================================================
class SDCarCamInsideFixedCar : public SDPerspCamera
{
 public:
    SDCarCamInsideFixedCar(SDView *myscreen, int id, int drawCurr, int drawBG,
                float myfovy, float myfovymin, float myfovymax,
                float myfnear, float myffar = 1500.0,
                float myfogstart = 1400.0, float myfogend = 1500.0)
    : SDPerspCamera(myscreen, id, drawCurr, 0, drawBG, 1,
             myfovy, myfovymin, myfovymax,
             myfnear, myffar, myfogstart, myfogend) {
    }

    void update(tCarElt *car, tSituation *s) {
    sgVec3 P, p;
    float offset = 0;

    p[0] = car->_bonnetPos_x;
    p[1] = car->_bonnetPos_y;
    p[2] = car->_bonnetPos_z;
    sgXformPnt3(p, car->_posMat);

    eye[0] = p[0];
    eye[1] = p[1];
    eye[2] = p[2];

    // Compute offset angle and bezel compensation)
    if (viewOffset) {
        offset += getSpanAngle();
    }

    P[0] = car->_bonnetPos_x + 30.0 * cos(2*PI/3 * car->_glance + offset);
    P[1] = car->_bonnetPos_y - 30.0 * sin(2*PI/3 * car->_glance + offset);
    P[2] = car->_bonnetPos_z;
    sgXformPnt3(P, car->_posMat);

    center[0] = P[0];
    center[1] = P[1];
    center[2] = P[2];

    up[0] = car->_posMat[2][0];
    up[1] = car->_posMat[2][1];
    up[2] = car->_posMat[2][2];

    speed[0] =car->pub.DynGCg.vel.x;
    speed[1] =car->pub.DynGCg.vel.y;
    speed[2] =car->pub.DynGCg.vel.z;

    Speed = car->_speed_x * 3.6;
    }
};

// SDCarCamInfrontFixedCar ================================================================

class SDCarCamInfrontFixedCar : public SDPerspCamera
{
 public:
    SDCarCamInfrontFixedCar(SDView *myscreen, int id, int drawCurr, int drawBG,
                float myfovy, float myfovymin, float myfovymax,
                float myfnear, float myffar = 1500.0,
                float myfogstart = 1400.0, float myfogend = 1500.0)
    : SDPerspCamera(myscreen, id, drawCurr, 0, drawBG, 1,
             myfovy, myfovymin, myfovymax,
             myfnear, myffar, myfogstart, myfogend) {
    }

    void update(tCarElt *car, tSituation *s) {
    sgVec3 P, p;
    float offset = 0;

    p[0] = car->_dimension_x / 2;
    p[1] = car->_bonnetPos_y;
    p[2] = car->_statGC_z;
    sgXformPnt3(p, car->_posMat);

    eye[0] = p[0];
    eye[1] = p[1];
    eye[2] = p[2];

    // Compute offset angle and bezel compensation)
    if (viewOffset) {
        offset += getSpanAngle();
    }

#if 0 // SDW test
    spanOffset = car->_glance * (viewOffset - 10) / 5;

    P[0] = (car->_dimension_x / 2) + 30.0 * cos(offset);
    P[1] = car->_bonnetPos_y - 30.0 * sin(offset);
#else
    P[0] = (car->_dimension_x / 2) + 30.0 * cos(2*PI/3 * car->_glance + offset);
    P[1] = car->_bonnetPos_y - 30.0 * sin(2*PI/3 * car->_glance + offset);
#endif
    P[2] = car->_statGC_z;

    sgXformPnt3(P, car->_posMat);

    center[0] = P[0];
    center[1] = P[1];
    center[2] = P[2];

    up[0] = car->_posMat[2][0];
    up[1] = car->_posMat[2][1];
    up[2] = car->_posMat[2][2];

    speed[0] =car->pub.DynGCg.vel.x;
    speed[1] =car->pub.DynGCg.vel.y;
    speed[2] =car->pub.DynGCg.vel.z;

    Speed = car->_speed_x * 3.6;
    }
};

// cGrCarCamBehindReverse ================================================================

class SDCarCamBehindReverse : public SDPerspCamera
{
 public:
    SDCarCamBehindReverse (SDView *myscreen, int id, int drawCurr, int drawBG,
                float myfovy, float myfovymin, float myfovymax,
                float myfnear, float myffar = 1500.0,
                float myfogstart = 1400.0, float myfogend = 1500.0)
    : SDPerspCamera(myscreen, id, drawCurr, 0, drawBG, 0,
             myfovy, myfovymin, myfovymax,
             myfnear, myffar, myfogstart, myfogend) {
    }

    void setModelView(void)
    {
      //screen->getOsgCam()->setViewMatrixAsLookAt(eye,center,up);

      //osg::Matrix m= screen->getOsgCam()->getViewMatrix();
       osg::Matrix m;
       m.makeLookAt(eye,center,up);
      //float mirror[4][4];

/*#define M(row,col)  mirror[row][col]
      M(0,0) = 1.0;  M(0,1) = 0.0;  M(0,2) = 0.0;  M(0,3) = 0.0;
      M(1,0) = 0.0;  M(1,1) =-1.0;  M(1,2) = 0.0;  M(1,3) = 0.0;
      M(2,0) = 0.0;  M(2,1) = 0.0;  M(2,2) = 1.0;  M(2,3) = 0.0;
      M(3,0) = 0.0;  M(3,1) = 0.0;  M(3,2) = 0.0;  M(3,3) = 1.0;
#undef M*/
      osg::Matrix mir(1,0,0,0,
                      0,-1,0,0,
                      0,0,1,0,
                      0,0,0,1);
      osg::Matrix res = m*mir;

      screen->getOsgCam()->setViewMatrix(res);

      //glFrontFace( GL_CW );
    }

    void update(tCarElt *car, tSituation *s)
    {
    sgVec3 P, p;
    float offset = 0;

    p[0] = car->_bonnetPos_x - (car->_dimension_x/2);
    p[1] = car->_bonnetPos_y;
    p[2] = car->_bonnetPos_z;
    sgXformPnt3(p, car->_posMat);

    eye[0] = p[0];
    eye[1] = p[1];
    eye[2] = p[2];

    // Compute offset angle and bezel compensation)
    if (viewOffset) {
        offset += getSpanAngle();
    }

    P[0] = car->_bonnetPos_x - (car->_dimension_x/2) + 30.0 * cos(offset);
    P[1] = car->_bonnetPos_y + 30.0 * sin(offset);
    P[2] = car->_bonnetPos_z;
    sgXformPnt3(P, car->_posMat);

    center[0] = P[0];
    center[1] = P[1];
    center[2] = P[2];

    up[0] = car->_posMat[2][0];
    up[1] = car->_posMat[2][1];
    up[2] = car->_posMat[2][2];

    speed[0] =car->pub.DynGCg.vel.x;
    speed[1] =car->pub.DynGCg.vel.y;
    speed[2] =car->pub.DynGCg.vel.z;
    }
};

// cGrCarCamFront ================================================================

class SDCarCamFront : public SDPerspCamera
{
 protected:
    float dist;

 public:
    SDCarCamFront(SDView *myscreen, int id, int drawCurr, int drawBG,
           float fovy, float fovymin, float fovymax,
           float mydist, float fnear, float ffar = 1500.0,
           float myfogstart = 1400.0, float myfogend = 1500.0)
    : SDPerspCamera(myscreen, id, drawCurr, 1, drawBG, 0, fovy, fovymin,
             fovymax, fnear, ffar, myfogstart, myfogend) {
    dist = mydist;
    up[0] = 0;
    up[1] = 0;
    up[2] = 1;
    }

    void update(tCarElt *car, tSituation *s) {
    float offset = 0;

    eye[0] = car->_pos_X + dist * cos(car->_yaw + PI * car->_glance);
    eye[1] = car->_pos_Y + dist * sin(car->_yaw + PI * car->_glance);
    eye[2] = RtTrackHeightG(car->_trkPos.seg, eye[0], eye[1]) + 0.5;

    // Compute offset angle and bezel compensation)
    if (viewOffset) {
        offset += getSpanAngle();
    }

    center[0] = car->_pos_X + dist * cos(car->_yaw + PI * car->_glance) - dist * cos(car->_yaw + PI * car->_glance - offset);
    center[1] = car->_pos_Y + dist * sin(car->_yaw + PI * car->_glance) - dist * sin(car->_yaw + PI * car->_glance - offset);
    center[2] = car->_pos_Z;

    speed[0] = car->pub.DynGCg.vel.x;
    speed[1] = car->pub.DynGCg.vel.y;
    speed[2] = car->pub.DynGCg.vel.z;

    Speed = car->_speed_x * 3.6;
    }
};

// SCarCamSide ================================================================

class SDCarCamSide : public SDPerspCamera
{
protected:
    float distx;
    float disty;
    float distz;

 public:
    SDCarCamSide(class SDView *myscreen, int id, int drawCurr, int drawBG,
          float fovy, float fovymin, float fovymax,
          float mydistx, float mydisty, float mydistz,
          float fnear, float ffar = 1500.0,
          float myfogstart = 1400.0, float myfogend = 1500.0)
    : SDPerspCamera(myscreen, id, drawCurr, 1, drawBG, 0, fovy, fovymin,
             fovymax, fnear, ffar, myfogstart, myfogend) {
    distx = mydistx;
    disty = mydisty;
    distz = mydistz;

    up[0] = 0;
    up[1] = 0;
    up[2] = 1;
    }

    void update(tCarElt *car, tSituation *s) {
    tdble x = car->_pos_X + distx;
    tdble y = car->_pos_Y + disty;
    tdble z = car->_pos_Z + distz;

    eye[0] = x;
    eye[1] = y;
    eye[2] = z;
    center[0] = car->_pos_X;
    center[1] = car->_pos_Y;
    center[2] = car->_pos_Z;

    speed[0] = car->pub.DynGCg.vel.x;
    speed[1] = car->pub.DynGCg.vel.y;
    speed[2] = car->pub.DynGCg.vel.z;

    Speed = car->_speed_x * 3.6;
    }
};

// cGrCarCamUp ================================================================

class SDCarCamUp : public SDPerspCamera
{
 protected:
    float distz;

 public:
    SDCarCamUp(SDView *myscreen, int id, int drawCurr, int drawBG,
        float fovy, float fovymin, float fovymax,
        float mydistz, int axis,
        float fnear, float ffar = 1500.0,
        float myfogstart = 1600.0, float myfogend = 1700.0)
    : SDPerspCamera(myscreen, id, drawCurr, 1, drawBG, 0, fovy, fovymin,
             fovymax, fnear, ffar, myfogstart, myfogend) {
    distz = mydistz;
    up[2] = 0;
    switch (axis) {
    case 0:
        up[0] = 0;
        up[1] = 1;
        break;
    case 1:
        up[0] = 0;
        up[1] = -1;
        break;
    case 2:
        up[0] = 1;
        up[1] = 0;
        break;
    case 3:
        up[0] = -1;
        up[1] = 0;
        break;
    default:
        up[0] = 0;
        up[1] = 1;
        break;
    }
    }

    void update(tCarElt *car, tSituation *s) {
    tdble x = car->_pos_X;
    tdble y = car->_pos_Y;
    tdble z = car->_pos_Z + distz;

    eye[0] = x;
    eye[1] = y;
    eye[2] = z;
    center[0] = x;
    center[1] = y;
    center[2] = car->_pos_Z;


    speed[0] = car->pub.DynGCg.vel.x;
    speed[1] = car->pub.DynGCg.vel.y;
    speed[2] = car->pub.DynGCg.vel.z;

    Speed = car->_speed_x * 3.6;
    }
};

// cGrCarCamCenter ================================================================

class SDCarCamCenter : public SDPerspCamera
{
 protected:
    float distz;
    float locfar;
    float locfovy;

 public:
    SDCarCamCenter(SDView *myscreen, int id, int drawCurr, int drawBG,
            float fovy, float fovymin, float fovymax,
            float mydistz,
            float fnear, float ffar = 1500.0,
            float myfogstart = 1400.0, float myfogend = 1500.0)
    : SDPerspCamera(myscreen, id, drawCurr, 1, drawBG, 0, fovy, fovymin,
             fovymax, fnear, ffar, myfogstart, myfogend) {
    distz = mydistz;
    locfar = ffar;
    locfovy = fovy;

    eye[0] = grWrldX * 0.5;
    eye[1] = grWrldY * 0.6;
    eye[2] = distz;

    up[0] = 0;
    up[1] = 0;
    up[2] = 1;
    }

    void loadDefaults(char *attr) {
  /*  sprintf(path, "%s/%d", GR_SCT_DISPMODE, screen->getId());
    locfovy = (float)GfParmGetNum(grHandle, path,
                   attr, (char*)NULL, fovydflt);*/
    }

    void setZoom(int cmd) {
    fovy = locfovy;
    SDPerspCamera::setZoom(cmd);
    locfovy = fovy;
    }

    void update(tCarElt *car, tSituation *s) {
    tdble	dx, dy, dz, dd;

    center[0] = car->_pos_X;
    center[1] = car->_pos_Y;
    center[2] = car->_pos_Z;

    dx = center[0] - eye[0];
    dy = center[1] - eye[1];
    dz = center[2] - eye[2];

    dd = sqrt(dx*dx+dy*dy+dz*dz);

    fnear = dz - 5;
    if (fnear < 1) {
        fnear = 1;
    }
    ffar  = dd + locfar;

    fovy = RAD2DEG(atan2(locfovy, dd));

    speed[0] = 0;
    speed[1] = 0;
    speed[2] = 0;

    Speed = car->_speed_x * 3.6;
    }
};

// SDCarCamLookAt ================================================================

class SDCarCamLookAt : public SDPerspCamera
{
 protected:

 public:
    SDCarCamLookAt(SDView *myscreen, int id, int drawCurr, int drawBG,
            float fovy, float fovymin, float fovymax,
            int axis,
            float eyex, float eyey, float eyez,
            float centerx, float centery, float centerz,
            float fnear, float ffar = 1500.0,
            float myfogstart = 1600.0, float myfogend = 1700.0)
    : SDPerspCamera(myscreen, id, drawCurr, 1, drawBG, 0, fovy, fovymin,
             fovymax, fnear, ffar, myfogstart, myfogend) {

    eye[0] = eyex;
    eye[1] = eyey;
    eye[2] = eyez;

    center[0] = centerx;
    center[1] = centery;
    center[2] = centerz;

    switch (axis) {
    case 0:
        up[0] = 0;
        up[1] = 1;
        up[2] = 0;
        break;
    case 1:
        up[0] = 0;
        up[1] = -1;
        up[2] = 0;
        break;
    case 2:
        up[0] = 1;
        up[1] = 0;
        up[2] = 0;
        break;
    case 3:
        up[0] = -1;
        up[1] = 0;
        up[2] = 0;
        break;
    case 4:
        up[0] = 0;
        up[1] = 0;
        up[2] = 1;
        break;
    case 5:
        up[0] = 0;
        up[1] = 0;
        up[2] = -1;
        break;
    default:
        up[0] = 0;
        up[1] = 0;
        up[2] = 1;
        break;
    }
    }

    void update(tCarElt *car, tSituation *s) {
    }
};

// cGrCarCamGoPro1 ================================================================

class SDCarCamGoPro1 : public SDPerspCamera
{
 protected:

 public:
    SDCarCamGoPro1(SDView *myscreen, int id, int drawCurr, int drawBG,
            float fovy, float fovymin, float fovymax,
            float fnear, float ffar = 1500.0,
            float myfogstart = 1400.0, float myfogend = 1500.0)
    : SDPerspCamera(myscreen, id, drawCurr, 1, drawBG, 0, fovy, fovymin,
             fovymax, fnear, ffar, myfogstart, myfogend) {
    }

    void update(tCarElt *car, tSituation *s) {
    sgVec3 P, p;
    float offset = 0;

    p[0] = car->_drvPos_x;
    p[1] = (car->_dimension_y / 2) + 0.1;
    p[2] = car->_statGC_z;
    sgXformPnt3(p, car->_posMat);

    eye[0] = p[0];
    eye[1] = p[1];
    eye[2] = p[2];

    // Compute offset angle and bezel compensation)
    if (viewOffset) {
        offset += getSpanAngle();
    }

    P[0] = 30 * cos(offset);
    P[1] = (car->_dimension_y / 2) + 0.1 - 30.0 * sin(offset);
    P[2] = car->_statGC_z;

    sgXformPnt3(P, car->_posMat);

    center[0] = P[0];
    center[1] = P[1];
    center[2] = P[2];

    up[0] = car->_posMat[2][0];
    up[1] = car->_posMat[2][1];
    up[2] = car->_posMat[2][2];

    speed[0] =car->pub.DynGCg.vel.x;
    speed[1] =car->pub.DynGCg.vel.y;
    speed[2] =car->pub.DynGCg.vel.z;

    Speed = car->_speed_x * 3.6;
    }
};

// cGrCarCamGoPro2 ================================================================

class SDCarCamGoPro2 : public SDPerspCamera
{
 protected:

 public:
    SDCarCamGoPro2(SDView*myscreen, int id, int drawCurr, int drawBG,
            float fovy, float fovymin, float fovymax,
            float fnear, float ffar = 1500.0,
            float myfogstart = 1400.0, float myfogend = 1500.0)
    : SDPerspCamera(myscreen, id, drawCurr, 1, drawBG, 0, fovy, fovymin,
             fovymax, fnear, ffar, myfogstart, myfogend) {
    }

    void update(tCarElt *car, tSituation *s) {
    sgVec3 P, p;
    float offset = 0;

    p[0] = car->_drvPos_x;
    p[1] = 0 - (car->_dimension_y / 2) - 0.1;
    p[2] = car->_statGC_z;
    sgXformPnt3(p, car->_posMat);

    eye[0] = p[0];
    eye[1] = p[1];
    eye[2] = p[2];

    // Compute offset angle and bezel compensation)
    if (viewOffset) {
        offset += getSpanAngle();
    }

    P[0] = 30 * cos(offset);
    P[1] = 0 - (car->_dimension_y / 2) - 0.1 - 30.0 * sin(offset);
    P[2] = car->_statGC_z;

    sgXformPnt3(P, car->_posMat);

    center[0] = P[0];
    center[1] = P[1];
    center[2] = P[2];

    up[0] = car->_posMat[2][0];
    up[1] = car->_posMat[2][1];
    up[2] = car->_posMat[2][2];

    speed[0] =car->pub.DynGCg.vel.x;
    speed[1] =car->pub.DynGCg.vel.y;
    speed[2] =car->pub.DynGCg.vel.z;

    Speed = car->_speed_x * 3.6;
    }
};




SDCamera::~SDCamera( void ){
}






SDCameras::SDCameras(SDView *c){



    // Get the factor of visibiity from the graphics settings and from the track.
    //tdble fovFactor = GfParmGetNum(grHandle, GR_SCT_GRAPHIC, GR_ATT_FOVFACT, (char*)NULL, 1.0);
    //fovFactor *= GfParmGetNum(grTrackHandle, TRK_SECT_GRAPH, TRK_ATT_FOVFACT, (char*)NULL, 1.0);

    tdble fovFactor =1;
    tdble fixedFar =0;

    // If sky dome is enabled, we have a "fixed far" cut plane.
    // Warning: In theory, 2*grSkyDomeDistance+1 should be enough, but it is not (why ?).
    //const tdble fixedFar = grSkyDomeDistance ? (2.1f * grSkyDomeDistance + 1.0f) : 0;

    //GfLogTrace("Screen #%d : FOV = %.2f, Far=%.0f\n", id, fovFactor, fixedFar);


    screen = c;
    SDView * myscreen = screen;

    int id=0;

    /* F2 - First Person Views  - cameras index 0*/
    /* cam F2 = inside, from the driver's eye, with head movements (driver's view) */
    cameras[0].insert(cameras[0].end(),new SDCarCamInsideDynDriverEye(myscreen,
                                                                 id,
                                                                 1,	/* drawCurr */
                                                                 1,	/* drawBG  */
                                                                 75.5,	/* fovy */
                                                                 10.0,	/* fovymin */
                                                                 95.0,	/* fovymax */
                                                                 0.03,	/* near */
                                                                 fixedFar ? fixedFar : 600.0 * fovFactor,	/* far */
                                                                 fixedFar ? fixedFar/2 : 300.0 * fovFactor,	/* fogstart */
                                                                 fixedFar ? fixedFar : 600.0 * fovFactor	/* fogend */
                                                                 ));
   id++;

   /* cam F2 = inside, from the driver's eye, without head movements (driver's view) */
    cameras[0].insert(cameras[0].end(),new SDCarCamInsideDriverEye(myscreen,
                                                              id,
                                                              1,	/* drawCurr */
                                                              1,	/* drawBG  */
                                                              75.5,	/* fovy */
                                                              10.0,	/* fovymin */
                                                              95.0,	/* fovymax */
                                                              0.03,	/* near */
                                                              fixedFar ? fixedFar : 600.0 * fovFactor,	/* far */
                                                              fixedFar ? fixedFar/2 : 300.0 * fovFactor,	/* fogstart */
                                                              fixedFar ? fixedFar : 600.0 * fovFactor	/* fogend */
                                                              ));
    id++;
    /* cam F2 = inside, from the board (bonnet view), fixed to the car */
    cameras[0].insert(cameras[0].end(),new SDCarCamInsideFixedCar(myscreen,
                                                                   id,
                                                                   1,	/* drawCurr */
                                                                   1,	/* drawBG  */
                                                                   67.5,	/* fovy */
                                                                   10.0,	/* fovymin */
                                                                   95.0,	/* fovymax */
                                                                   0.3,	/* near */
                                                                   fixedFar ? fixedFar : 600.0 * fovFactor,	/* far */
                                                                   fixedFar ? fixedFar/2 : 300.0 * fovFactor,	/* fogstart */
                                                                   fixedFar ? fixedFar : 600.0 * fovFactor	/* fogend */
                                                                   ));
    id++;
    /* cam F2 = inside, from the board (bonnet view), fixed to the car */
    cameras[0].insert(cameras[0].end(),new SDCarCamInfrontFixedCar(myscreen,
                                                                   id,
                                                                   1,	/* drawCurr */
                                                                   1,	/* drawBG  */
                                                                   67.5,	/* fovy */
                                                                   10.0,	/* fovymin */
                                                                   95.0,	/* fovymax */
                                                                   0.3,	/* near */
                                                                   fixedFar ? fixedFar : 600.0 * fovFactor,	/* far */
                                                                   fixedFar ? fixedFar/2 : 300.0 * fovFactor,	/* fogstart */
                                                                   fixedFar ? fixedFar : 600.0 * fovFactor	/* fogend */
                                                                   ));
    id++;
    /* TODO BUG F2 = just behind the car; camera looking back  */
    cameras[0].insert(cameras[0].end(),new SDCarCamBehindReverse(myscreen,
                                                                   id,
                                                                   0,	/* drawCurr */
                                                                   2,	/* drawBG  */
                                                                   67.5,	/* fovy */
                                                                   10.0,	/* fovymin */
                                                                   95.0,	/* fovymax */
                                                                   0.3,	/* near */
                                                                   fixedFar ? fixedFar : 600.0 * fovFactor,	/* far */
                                                                   fixedFar ? fixedFar/2 : 300.0 * fovFactor,	/* fogstart */
                                                                   fixedFar ? fixedFar : 600.0 * fovFactor	/* fogend */
                                                                   ));

    /* F3 - 3rd Person Views - cameras index 1*/
    id=0;

    /* cam F2 = behind the car, near, looking forward */
    cameras[1].insert(cameras[1].end(),new SDCarCamBehind(myscreen,
                                                          id,
                                                          1,	/* drawCurr */
                                                          1,	/* drawBG  */
                                                          40.0,	/* fovy */
                                                          5.0,	/* fovymin */
                                                          95.0,	/* fovymax */
                                                          10.0,	/* dist */
                                                          2.0,	/* height */
                                                          1.0,	/* near */
                                                          fixedFar ? fixedFar : 600.0 * fovFactor,	/* far */
                                                          fixedFar ? fixedFar/2 : 300.0 * fovFactor,	/* fogstart */
                                                          fixedFar ? fixedFar : 600.0 * fovFactor,	/* fogend */
                                                          25.0	/* relaxation */
                                                          ));
    id++;
    /* cam F3 = car behind */
    cameras[1].insert(cameras[1].end(),new SDCarCamBehind(myscreen,
                                                           id,
                                                           1,	/* drawCurr */
                                                           1,	/* drawBG  */
                                                           40.0,	/* fovy */
                                                           5.0,	/* fovymin */
                                                           95.0,	/* fovymax */
                                                           8.0,	/* dist */
                                                           .50,	/* height */
                                                           .50,	/* near */
                                                           fixedFar ? fixedFar : 600.0 * fovFactor,	/* far */
                                                           fixedFar ? fixedFar/2 : 300.0 * fovFactor,	/* fogstart */
                                                           fixedFar ? fixedFar : 600.0 * fovFactor	/* fogend */
                                                           ));

    id++;
    /* cam F3 = car reverse */
    cameras[1].insert(cameras[1].end(),new SDCarCamFront(myscreen,
                                                          id,
                                                          1,	/* drawCurr */
                                                          1,	/* drawBG  */
                                                          40.0,	/* fovy */
                                                          5.0,	/* fovymin */
                                                          95.0,	/* fovymax */
                                                          8.0,	/* dist */
                                                          0.5,	/* near */
                                                          fixedFar ? fixedFar : 1000.0 * fovFactor,	/* far */
                                                          fixedFar ? fixedFar/2 : 500.0 * fovFactor,	/* fogstart */
                                                          fixedFar ? fixedFar : 1000.0 * fovFactor	/* fogend */
                                                          ));

    id++;
   /* cam F3 = behind the car, very near, looking forward */
    cameras[1].insert(cameras[1].end(),new SDCarCamBehind(myscreen,
                                                           id,
                                                           1,	/* drawCurr */
                                                           1,	/* drawBG  */
                                                           40.0,	/* fovy */
                                                           5.0,	/* fovymin */
                                                           95.0,	/* fovymax */
                                                           8.0,	/* dist */
                                                           2.5,	/* height */
                                                           1.0,	/* near */
                                                           fixedFar ? fixedFar : 600.0 * fovFactor,	/* far */
                                                           fixedFar ? fixedFar/2 : 300.0 * fovFactor,	/* fogstart */
                                                           fixedFar ? fixedFar : 600.0 * fovFactor,	/* fogend */
                                                           25.0	/* relaxation */
                                                           ));

    /* F4 - Tracking side/front/back shots -index list 2*/
    id=0;

    /* cam F4 = car side 1 */
    cameras[2].insert(cameras[2].end(),new SDCarCamSide(myscreen,
                                                         id,
                                                         1,	/* drawCurr */
                                                         1,	/* drawBG  */
                                                         30.0,	/* fovy */
                                                         5.0,	/* fovymin */
                                                         60.0,	/* fovymax */
                                                         0.0,	/* distx */
                                                         -20.0,	/* disty */
                                                         3.0,	/* distz */
                                                         1.0,	/* near */
                                                         fixedFar ? fixedFar : 1000.0 * fovFactor,	/* far */
                                                         fixedFar ? fixedFar/2 : 500.0 * fovFactor,	/* fogstart */
                                                         fixedFar ? fixedFar : 1000.0 * fovFactor	/* fogend */
                                                         ));
    id++;
    /* cam F4 = car side 2 */
    cameras[2].insert(cameras[2].end(),new SDCarCamSide(myscreen,
                                                         id,
                                                         1,	/* drawCurr */
                                                         1,	/* drawBG  */
                                                         30.0,	/* fovy */
                                                         5.0,	/* fovymin */
                                                         60.0,	/* fovymax */
                                                         0.0,	/* distx */
                                                         20.0,	/* disty */
                                                         3.0,	/* distz */
                                                         1.0,	/* near */
                                                         fixedFar ? fixedFar : 1000.0 * fovFactor,	/* far */
                                                         fixedFar ? fixedFar/2 : 500.0 * fovFactor,	/* fogstart */
                                                         fixedFar ? fixedFar : 1000.0 * fovFactor	/* fogend */
                                                         ));
    id++;

    /* cam F4 = car side 3 */
    cameras[2].insert(cameras[2].end(),new SDCarCamSide(myscreen,
                                                         id,
                                                         1,	/* drawCurr */
                                                         1,	/* drawBG  */
                                                         30.0,	/* fovy */
                                                         5.0,	/* fovymin */
                                                         60.0,	/* fovymax */
                                                         -20.0,	/* distx */
                                                         0.0,	/* disty */
                                                         3.0,	/* distz */
                                                         1.0,	/* near */
                                                         fixedFar ? fixedFar : 1000.0 * fovFactor,	/* far */
                                                         fixedFar ? fixedFar/2 : 500.0 * fovFactor,	/* fogstart */
                                                         fixedFar ? fixedFar : 1000.0 * fovFactor	/* fogend */
                                                         ));
    id++;

    /* cam F4 = car side 4 */
    cameras[2].insert(cameras[2].end(),new SDCarCamSide(myscreen,
                                                         id,
                                                         1,	/* drawCurr */
                                                         1,	/* drawBG  */
                                                         30.0,	/* fovy */
                                                         5.0,	/* fovymin */
                                                         60.0,	/* fovymax */
                                                         20.0,	/* distx */
                                                         0.0,	/* disty */
                                                         3.0,	/* distz */
                                                         1.0,	/* near */
                                                         fixedFar ? fixedFar : 1000.0 * fovFactor,	/* far */
                                                         fixedFar ? fixedFar/2 : 500.0 * fovFactor,	/* fogstart */
                                                         fixedFar ? fixedFar : 1000.0 * fovFactor	/* fogend */
                                                         ));
    id++;

    /* cam F4 = car side 5 */
    cameras[2].insert(cameras[2].end(),new SDCarCamSide(myscreen,
                                                         id,
                                                         1,	/* drawCurr */
                                                         1,	/* drawBG  */
                                                         30.0,	/* fovy */
                                                         5.0,	/* fovymin */
                                                         60.0,	/* fovymax */
                                                         0.0,	/* distx */
                                                         -40.0,	/* disty */
                                                         6.0,	/* distz */
                                                         1.0,	/* near */
                                                         fixedFar ? fixedFar : 1000.0 * fovFactor,	/* far */
                                                         fixedFar ? fixedFar/2 : 500.0 * fovFactor,	/* fogstart */
                                                         fixedFar ? fixedFar : 1000.0 * fovFactor	/* fogend */
                                                         ));
    id++;
    /* cam F4 = car side 6 */
    cameras[2].insert(cameras[2].end(),new SDCarCamSide(myscreen,
                                                         id,
                                                         1,	/* drawCurr */
                                                         1,	/* drawBG  */
                                                         30.0,	/* fovy */
                                                         5.0,	/* fovymin */
                                                         60.0,	/* fovymax */
                                                         0.0,	/* distx */
                                                         40.0,	/* disty */
                                                         6.0,	/* distz */
                                                         1.0,	/* near */
                                                         fixedFar ? fixedFar : 1000.0 * fovFactor,	/* far */
                                                         fixedFar ? fixedFar/2 : 500.0 * fovFactor,	/* fogstart */
                                                         fixedFar ? fixedFar : 1000.0 * fovFactor	/* fogend */
                                                         ));
    id++;

    /* cam F4 = car side 7 */
    cameras[2].insert(cameras[2].end(),new SDCarCamSide(myscreen,
                                                        id,
                                                        1,	/* drawCurr */
                                                        1,	/* drawBG  */
                                                        30.0,	/* fovy */
                                                        5.0,	/* fovymin */
                                                        60.0,	/* fovymax */
                                                        -40.0,	/* distx */
                                                        0.0,	/* disty */
                                                        6.0,	/* distz */
                                                        1.0,	/* near */
                                                        fixedFar ? fixedFar : 1000.0 * fovFactor,	/* far */
                                                        fixedFar ? fixedFar/2 : 500.0 * fovFactor,	/* fogstart */
                                                        fixedFar ? fixedFar : 1000.0 * fovFactor	/* fogend */
                                                        ));
    id++;

    /* cam F4 = car side 8 */
    cameras[2].insert(cameras[2].end(),new SDCarCamSide(myscreen,
                                                        id,
                                                        1,	/* drawCurr */
                                                        1,	/* drawBG  */
                                                        30.0,	/* fovy */
                                                        5.0,	/* fovymin */
                                                        60.0,	/* fovymax */
                                                        40.0,	/* distx */
                                                        0.0,	/* disty */
                                                        6.0,	/* distz */
                                                        1.0,	/* near */
                                                        fixedFar ? fixedFar : 1000.0 * fovFactor,	/* far */
                                                        fixedFar ? fixedFar/2 : 500.0 * fovFactor,	/* fogstart */
                                                        fixedFar ? fixedFar : 1000.0 * fovFactor	/* fogend */
                                                        ));
    id++;

    /* F5 - Views from above - list index 3 */
    id=0;

    /* cam F5 = car up 1 */
    cameras[3].insert(cameras[3].end(),new SDCarCamUp(myscreen,
                                                       id,
                                                       1,	/* drawCurr */
                                                       1,	/* drawBG  */
                                                       //12.0,	/* fovy */
                                                       37.5,	/* fovy */
                                                       1.0,	/* fovymin */
                                                       90.0,	/* fovymax */
                                                       //300.0,	/* distz */
                                                       200.0,	/* distz */
                                                       0,		/* axis */
                                                       //200.0,	/* near */
                                                       100.0,	/* near */
                                                       fixedFar ? fixedFar : 1000.0 * fovFactor,/* far */
                                                       fixedFar ? fixedFar/2 : 500.0 * fovFactor,	/* fogstart */
                                                       fixedFar ? fixedFar : 1000.0 * fovFactor	/* fogend */
                                                       ));
    id++;

    /* cam F5 = car up 2 */
    cameras[3].insert(cameras[3].end(),new SDCarCamUp(myscreen,
                                                      id,
                                                      1,	/* drawCurr */
                                                      1,	/* drawBG  */
                                                      //12.0,	/* fovy */
                                                      37.5,	/* fovy */
                                                      1.0,	/* fovymin */
                                                      90.0,	/* fovymax */
                                                      //300.0,	/* distz */
                                                      250.0,	/* distz */
                                                      1,		/* axis */
                                                      200.0,	/* near */
                                                      fixedFar ? fixedFar : 1000.0 * fovFactor,/* far */
                                                      fixedFar ? fixedFar/2 : 500.0 * fovFactor,	/* fogstart */
                                                      fixedFar ? fixedFar : 1000.0 * fovFactor	/* fogend */
                                                      ));
    id++;

    /* cam F5 = car up 3 */
    cameras[3].insert(cameras[3].end(),new SDCarCamUp(myscreen,
                                                      id,
                                                      1,	/* drawCurr */
                                                      1,	/* drawBG  */
                                                      //12.0,	/* fovy */
                                                      37.5,	/* fovy */
                                                      1.0,	/* fovymin */
                                                      90.0,	/* fovymax */
                                                      //300.0,	/* distz */
                                                      350.0,	/* distz */
                                                      2,		/* axis */
                                                      200.0,	/* near */
                                                      fixedFar ? fixedFar : 1000.0 * fovFactor,/* far */
                                                      fixedFar ? fixedFar/2 : 500.0 * fovFactor,	/* fogstart */
                                                      fixedFar ? fixedFar : 1000.0 * fovFactor	/* fogend */
                                                      ));
    id++;

    /* cam F5 = car up 4 */
    cameras[3].insert(cameras[3].end(),new SDCarCamUp(myscreen,
                                                      id,
                                                      1,	/* drawCurr */
                                                      1,	/* drawBG  */
                                                      //12.0,	/* fovy */
                                                      37.5,	/* fovy */
                                                      1.0,	/* fovymin */
                                                      90.0,	/* fovymax */
                                                      //300.0,	/* distz */
                                                      400.0,	/* distz */
                                                      3,		/* axis */
                                                      200.0,	/* near */
                                                      fixedFar ? fixedFar : 1000.0 * fovFactor,/* far */
                                                      fixedFar ? fixedFar/2 : 500.0 * fovFactor,	/* fogstart */
                                                      fixedFar ? fixedFar : 1000.0 * fovFactor	/* fogend */
                                                      ));
    id++;

    /* F6 - index 4*/
    id=0;

    /* BUG TODO cam F6 = car from circuit centre */
    cameras[4].insert(cameras[4].end(),new SDCarCamCenter(myscreen,
                                                          id,
                                                          1,	/* drawCurr */
                                                          1,	/* drawBG  */
                                                          21.0,	/* fovy */
                                                          2.0,	/* fovymin */
                                                          60.0,	/* fovymax */
                                                          120.0,	/* distz */
                                                          100.0,	/* near */
                                                          fixedFar ? fixedFar : 1000.0 * fovFactor,/* far */
                                                          fixedFar ? fixedFar/2 : 500.0 * fovFactor,/* fogstart */
                                                          fixedFar ? fixedFar : 1000.0 * fovFactor	/* fogend */
                                                          ));
    id++;


    /* F7 -index 5*/
    id =0;
    /* BUGGY too cam F7 = panoramic */
    cameras[5].insert(cameras[5].end(),new SDCarCamLookAt(myscreen,
                                                           id,
                                                           1,		/* drawCurr */
                                                           0,		/* drawBG  */
                                                           74.0,		/* fovy */
                                                           1.0,		/* fovymin */
                                                           110.0,		/* fovymax */
                                                           0,		/* up axis */
                                                           grWrldX/2,	/* eyex */
                                                           grWrldY/2,	/* eyey */
                                                           MAX(grWrldX/2, grWrldY*4/3/2) + grWrldZ, /* eyez */
                                                           grWrldX/2,	/* centerx */
                                                           grWrldY/2,	/* centery */
                                                           0,		/* centerz */
                                                           10.0,		/* near */
                                                           grWrldMaxSize * 2.0,	/* far */
                                                           grWrldMaxSize * 10.0,	/* fogstart */
                                                           grWrldMaxSize * 20.0	/* fogend */
                                                           ));

    /* F8 - GoPro like views-index 6*/
    id=0;

    cameras[6].insert(cameras[6].end(),new SDCarCamGoPro1(myscreen,
                                                          id,
                                                          1,	/* drawCurr */
                                                          1,	/* drawBG  */
                                                          67.5,	/* fovy */
                                                          10.0,	/* fovymin */
                                                          95.0,	/* fovymax */
                                                          0.05,	/* near */
                                                          fixedFar ? fixedFar : 600.0 * fovFactor,	/* far */
                                                          fixedFar ? fixedFar : 300.0 * fovFactor,	/* fogstart */
                                                          fixedFar ? fixedFar : 600.0 * fovFactor	/* fogend */
                                                          ));
    id++;
    cameras[6].insert(cameras[6].end(),new SDCarCamGoPro2(myscreen,
                                                           id,
                                                           1,	/* drawCurr */
                                                           1,	/* drawBG  */
                                                           67.5,	/* fovy */
                                                           10.0,	/* fovymin */
                                                           95.0,	/* fovymax */
                                                           0.05,	/* near */
                                                           fixedFar ? fixedFar : 600.0 * fovFactor,	/* far */
                                                           fixedFar ? fixedFar : 300.0 * fovFactor,	/* fogstart */
                                                           fixedFar ? fixedFar : 600.0 * fovFactor	/* fogend */
                                                           ));


    selectedCamera =0;
    selectedList=0;

}

SDCamera * SDCameras::getSelectedCamera(){
    return cameras[selectedList][selectedCamera];
}

void SDCameras::nextCamera(int list){
    if(list == selectedList){
        selectedCamera = (selectedCamera +1)%cameras[selectedList].size();
    }else{
        selectedCamera =0;
        selectedList = list;
    }

    cameras[selectedList][selectedCamera]->setProjection();

}

void SDCameras::update(tCarElt * car, tSituation * s){
    cameras[selectedList][selectedCamera]->update(car,s);

    cameras[selectedList][selectedCamera]->setModelView();
}

SDCameras::~SDCameras(){
}

