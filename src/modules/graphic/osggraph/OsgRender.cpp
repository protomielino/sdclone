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
#include "OsgScenery.h"

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
enum {SUN = 0, MOON};	//Celestial bodies

tTrack 	 *grTrack;

unsigned grSkyDomeDistance = 0;
static unsigned grNbCloudLayers = 0;

static bool grDynamicSkyDome = false;
static int grBackgroundType = 0;
static float grMax_Visibility = 0.0f;

static const unsigned SkyDomeDistThresh = 12000; // No dynamic sky below that value.

static int RainBool = 0;
static int skydynamic = 0;
static int TimeDyn = 0;
static int WeatherDyn = 0;
static int BackgroundType = 0;
static float sd = 0.0f;
static float sd2 = 0.0f;
static GLuint BackgroundList = 0;
static GLuint BackgroundTex = 0;
static GLuint BackgroundList2;
static GLuint BackgroundTex2;

static bool grBGSky = false;
static bool grBGType = false;

//static ssgRoot *TheBackground = NULL;
//static ssgaSky *Sky = NULL;
//static ssgTransform *TheSun = NULL;

//static ssgaCelestialBody *bodies[MAX_BODIES] = { NULL };

/*static sgdVec3 *star_data = NULL;
static sgdVec3 *planet_data = NULL;

static sgVec4 black             = { 0.0f, 0.0f, 0.0f, 1.0f } ;
static sgVec4 white             = { 1.0f, 1.0f, 1.0f, 1.0f } ;
static sgVec4 translucent_white = { 1.0f, 1.0f, 1.0f, 0.8f } ;

static sgVec4 base_sky_color    = { 0.39f, 0.50f, 0.74f, 1.0f } ;
static sgVec4 base_fog_color    = { 0.84f, 0.87f, 1.00f, 1.0f } ;

static sgVec4 base_ambiant      = { 0.2f, 0.2f, 0.2f, 1.0f } ;
static sgVec4 base_diffuse      = { 0.8f, 0.8f, 0.8f, 1.0f } ;
static sgVec4 base_specular     = { 0.3f, 0.3f, 0.3f, 1.0f } ;

static sgVec4 sky_color;
static sgVec4 fog_color;
static sgVec4 cloud_color;*/

static sgVec4 scene_ambiant;
static sgVec4 scene_diffuse;
static sgVec4 scene_specular;

//Utility
static const double m_log01 = -log( 0.01 );
static const double sqrt_m_log01 = sqrt( m_log01 );
static char buf[1024];

/**
 * SDRender
 * Initialises a scene (ie a new view).
 * 
 * @return 0 if OK, -1 if something failed
 */
int SDRender::Init(void)
{
    osg::Light* myLight2 = new osg::Light;
    myLight2->setLightNum(1);
    myLight2->setPosition(osg::Vec4(900.0f, -3220.0f, 1543.0f,1.0f));
    myLight2->setAmbient(osg::Vec4(0.2f, 0.2f, 0.2f, 1.0f));
    myLight2->setDiffuse(osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f));
    myLight2->setSpecular(osg::Vec4(0.5f, 0.5f, 0.5f, 1.0f));
    myLight2->setConstantAttenuation(1.0f);    

    m_sceneViewer->setSceneData(m_sceneroot.get());
    m_sceneViewer->getCamera()->setCullingMode( m_sceneViewer->getCamera()->getCullingMode() & ~osg::CullStack::SMALL_FEATURE_CULLING);
    

    osg::Group *g = new osg::Group;
    m_carroot = g;

    GfOut("LE POINTEUR %d\n",m_carroot.get());

  	return 0;
}//SDRender::Init

