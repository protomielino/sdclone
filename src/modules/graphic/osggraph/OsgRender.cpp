/***************************************************************************

    file                 : OsgRender.cpp
    created              : Mon Aug 21 20:13:56 CEST 2012
    copyright            : (C) 2012 Xavier Bertaux
    email                : bertauxx@yahoo.fr
    version              : $Id: OsgRender.cpp 2436 2010-05-08 14:22:43Z torcs-ng $

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <osgUtil/Optimizer>
#include <osg/MatrixTransform>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgDB/Registry>
#include <osg/Light>
#include <osg/LightSource>
#include <osg/Camera>

#include "OsgMain.h"
#include "OsgRender.h"

#include <glfeatures.h>	//gluXXX
#include <robottools.h>	//RtXXX()

#define MAX_BODIES	2
#define MAX_CLOUDS	3
#define NSTARS			1000
#define NPLANETS		0	//No planets displayed
#define NB_BG_FACES	36	//Background faces
#define BG_DIST			1.0f
#define SKYDYNAMIC_THR	12000	//Skydynamic setting threshold. No dynamic sky below that.
#define CLEAR_CLOUD 1
#define MORE_CLOUD 6
#define SCARCE_CLOUD 5
#define COVERAGE_CLOUD 8

SDRender::SDRender(void)
{
}

SDRender::~SDRender(void)
{
}

/**
 * SDRender
 * Initialises a scene (ie a new view).
 * 
 * @return 0 if OK, -1 if something failed
 */
void SDRender::Init(osg::ref_ptr<osg::Group> scene, osg::ref_ptr<osgViewer::Viewer> viewer)
{
    osg::Light* myLight2 = new osg::Light;
    myLight2->setLightNum(1);
    myLight2->setPosition(osg::Vec4(900.0f, -3220.0f, 1543.0f,1.0f));
    myLight2->setAmbient(osg::Vec4(0.2f, 0.2f, 0.2f, 1.0f));
    myLight2->setDiffuse(osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f));
    myLight2->setSpecular(osg::Vec4(0.5f, 0.5f, 0.5f, 1.0f));
    myLight2->setConstantAttenuation(1.0f);    

    viewer->setSceneData(scene.get());
    viewer->getCamera()->setCullingMode( viewer->getCamera()->getCullingMode() & ~osg::CullStack::SMALL_FEATURE_CULLING);
    

    osg::Group *g = new osg::Group;
    m_carroot = g;

    GfOut("LE POINTEUR %d\n",m_carroot.get());

  	//return 0;
}//SDRender::Init

