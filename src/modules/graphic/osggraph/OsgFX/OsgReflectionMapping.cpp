/***************************************************************************

    file        : OsgScreens.h
    created     : Sat Feb 2013 15:52:19 CEST 2013
    copyright   : (C) 2013 by Gaëtan André
    email       : gaetan.andre@gmail.com
    version     : $Id$
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ****************************************************************************/

#include <osg/Group>
#include <osg/Texture2D>
#include <osg/Camera>
#include <osgViewer/Viewer>

#include "OsgView/OsgScreens.h"

#include "OsgReflectionMapping.h"

#include <car.h>




SDReflectionMapping::SDReflectionMapping(SDScreens *s, osg::ref_ptr<osg::Node> m_sceneroot){

    screens=s;

    reflectionMap = new osg::TextureCubeMap;
    reflectionMap->setTextureSize( 256, 256 );
    reflectionMap->setInternalFormat( GL_RGB );

    camerasRoot = new osg::Group;

    for(int i=0;i<6;i++){
        map = new osg::Texture2D;
        map->setTextureSize( 256, 256 );
        map->setInternalFormat( GL_RGB );


        osg::ref_ptr<osg::Camera> camera = new osg::Camera;
        camera->setViewport( 0, 0, 256, 256 );
        camera->setClearColor( osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f) );
        camera->setClearMask( GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT );

        camera->setRenderOrder( osg::Camera::PRE_RENDER );
        camera->setRenderTargetImplementation(
        osg::Camera::FRAME_BUFFER_OBJECT );
        camera->attach( osg::Camera::COLOR_BUFFER, reflectionMap,0,i );

        camera->setReferenceFrame( osg::Camera::ABSOLUTE_RF );
        camera->addChild( m_sceneroot );


        //camera->setProjectionMatrixAsOrtho(-1,1,-1,1,0,1000000);

        camera->setProjectionMatrixAsPerspective(45.0,1.0,1.0,1000000.0);
        camerasRoot->addChild(camera);
        cameras.push_back(camera);

        reflectionMap->setImage(i,map->getImage());

  }



}


void SDReflectionMapping::update(){

    tCarElt * car = screens->getActiveView()->getCurrentCar();
    sgVec3 P, p;
    osg::Vec3 eye,center,up;

    float offset = 0;

    p[0] = car->_drvPos_x;
    p[1] = car->_bonnetPos_y;
    p[2] = car->_drvPos_z;
    sgXformPnt3(p, car->_posMat);

    eye[0] = p[0];
    eye[1] = p[1];
    eye[2] = p[2];


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

    for(unsigned int i=0;i<cameras.size();i++){
        cameras[i]->setViewMatrixAsLookAt(eye,center,up);
    }
}

SDReflectionMapping::~SDReflectionMapping(){
}
