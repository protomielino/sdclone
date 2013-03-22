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
#include <osg/Fog>
#include <osg/Light>
#include <osg/LightSource>
#include <osg/Camera>
#include <osgViewer/Viewer>

#include "OsgMain.h"
#include "OsgRender.h"
#include "OsgSky.h"
#include "OsgScenery.h"
#include "OsgMath.h"
#include "OsgColor.h"

#include <glfeatures.h>	//gluXXX
#include <robottools.h>	//RtXXX()


static const double m_log01 = -log( 0.01 );
static const double sqrt_m_log01 = sqrt( m_log01 );
const GLfloat fog_exp2_density = sqrt_m_log01 / 11000;

static osg::ref_ptr<osg::Group> mRealRoot = new osg::Group;
static osg::ref_ptr<osg::Group> mRoot = new osg::Group;

SDSky *thesky = NULL;
static tTrack *grTrack;

unsigned SDSkyDomeDistance = 0;
//static unsigned SDNbCloudLayers = 0;

// Some private global variables.
//static int grDynamicWeather = 0;
//static bool SDDynamicSkyDome = false;
static float SDSunDeclination = 0.0f;
static float SDMoonDeclination = 0.0f;
//static float SDMax_Visibility = 0.0f;
static double SDVisibility = 0.0f;

#define MAX_BODIES	2
#define MAX_CLOUDS	3
#define NMaxStars	3000
#define NPLANETS		0	//No planets displayed
#define NB_BG_FACES	36	//Background faces
#define BG_DIST			1.0f
#define SKYDYNAMIC_THR	12000	//Skydynamic setting threshold. No dynamic sky below that.
#define CLEAR_CLOUD 1
#define MORE_CLOUD 6
#define SCARCE_CLOUD 5
#define COVERAGE_CLOUD 8

static const osg::Vec4 BaseSkyColor ( 0.31f, 0.43f, 0.69f, 1.0f );

static osg::Vec3d *AStarsData = NULL;
static osg::Vec3d *APlanetsData = NULL;
static int NStars;
static int NPlanets;
static float sol_angle;
static float moon_angle;

static osg::ref_ptr<osg::Group> RealRoot = new osg::Group;

SDRender::SDRender(void)
{
}

SDRender::~SDRender(void)
{
}

SDSky * SDRender::getSky(){
    return thesky;
}

/**
 * SDRender
 * Initialises a scene (ie a new view).
 *
 * @return 0 if OK, -1 if something failed
 */
osg::Node* SDRender::Init(osg::Group *m_sceneroot, tTrack *track)
{
    //char buf[256];
    //void *hndl = grTrackHandle;
    grTrack = track;

    std::string datapath = GetDataDir();
    //datapath +="/";
    thesky = new SDSky;
    GfOut("SDSky class\n");
    int SDSkyDomeDistance = 12000;

    NStars = NMaxStars;
    if (AStarsData)
        delete [] AStarsData;

    AStarsData = new osg::Vec3d[NStars];

    for(int i= 0; i < NStars; i++)
    {
        AStarsData[i][0] = SDRandom() * PI;
        AStarsData[i][1] = SDRandom() * PI;
        AStarsData[i][2] = SDRandom();
    }

    GfLogInfo("  Stars (random) : %d\n", NStars);

    NPlanets = 0;
    APlanetsData = NULL;

    GfLogInfo("  Planets : %d\n", NPlanets);

    const int timeOfDay = (int)grTrack->local.timeofday;
    const double domeSizeRatio = SDSkyDomeDistance / 80000.0;

    GfLogInfo("  domeSizeRation : %d\n", domeSizeRatio);

    thesky->build(datapath, SDSkyDomeDistance, SDSkyDomeDistance, 2000*domeSizeRatio,
                  SDSkyDomeDistance, 2000*domeSizeRatio, SDSkyDomeDistance, NPlanets,
                  APlanetsData, NStars, AStarsData );
    GfOut("Build SKY\n");
    GLfloat sunAscension = grTrack->local.sunascension;
    SDSunDeclination = (float)(15 * (double)timeOfDay / 3600 - 90.0);

    thesky->setSD( DEG2RAD(SDSunDeclination));
    thesky->setSRA( sunAscension );

    GfLogInfo("  Sun : time of day = %02d:%02d:%02d (declination = %.1f deg), "
              "ascension = %.1f deg\n", timeOfDay / 3600, (timeOfDay % 3600) / 60, timeOfDay % 60,
              SDSunDeclination, RAD2DEG(sunAscension));

    if ( SDSunDeclination > 180 )
        SDMoonDeclination = 3.0 + (rand() % 40);
    else
        SDMoonDeclination = (rand() % 270);

    //SDMoonDeclination = grUpdateMoonPos(timeOfDay);
    //SDMoonDeclination = 22.0; /*(rand() % 270);*/

    const float moonAscension = grTrack->local.sunascension;

    thesky->setMD( DEG2RAD(SDMoonDeclination) );
    thesky->setMRA( DEG2RAD(moonAscension) );

    GfLogInfo("  Moon : declination = %.1f deg, ascension = %.1f deg\n",
              SDMoonDeclination, moonAscension);

    // Set up the light source to the Sun position.
    //sgCoord sunPosition;
    //TheSky->getSunPos(&sunPosition);
    //light->setPosition(sunPosition.xyz);

    // Initialize the whole sky dome.
    double r_WrldX = SDScenery::getWorldX();
    double r_WrldY = SDScenery::getWorldY();
    osg::Vec3 viewPos(r_WrldX / 2, r_WrldY/ 2, 0.0);

    thesky->reposition( viewPos, 0, 0);
    UpdateLight();
    thesky->repaint(SkyColor, FogColor, CloudsColor, sol_angle, moon_angle, NPlanets,
                    APlanetsData, NStars, AStarsData);

    osg::Group* sceneGroup = new osg::Group;
    sceneGroup->addChild(m_sceneroot);
    //sceneGroup->setNodeMask(~simgear::BACKGROUND_BIT);
    osg::StateSet* stateSet = sceneGroup->getOrCreateStateSet();
    stateSet->setMode(GL_DEPTH_TEST, osg::StateAttribute::ON);

    osg::LightSource* lightSource = new osg::LightSource;
    lightSource->getLight()->setDataVariance(osg::Object::DYNAMIC);
    // relative because of CameraView being just a clever transform node
    lightSource->setReferenceFrame(osg::LightSource::RELATIVE_RF);
    lightSource->setLocalStateSetModes(osg::StateAttribute::ON);
    //lightSource->setUpdateCallback(new FGLightSourceUpdateCallback);

    // we need a white diffuse light for the phase of the moon
    osg::LightSource* sunLight = new osg::LightSource;
    sunLight->getLight()->setDataVariance(osg::Object::DYNAMIC);
    sunLight->getLight()->setLightNum(1);
    sunLight->setReferenceFrame(osg::LightSource::RELATIVE_RF);
    sunLight->setLocalStateSetModes(osg::StateAttribute::ON);

    osg::Group* skyGroup = new osg::Group;
    osg::StateSet* skySS = skyGroup->getOrCreateStateSet();
    skySS->setMode(GL_LIGHT0, osg::StateAttribute::OFF);
    skyGroup->addChild(thesky->getPreRoot());
    sunLight->addChild(skyGroup);
    mRoot->addChild(sceneGroup);
    mRoot->addChild(sunLight);

    // Clouds are added to the scene graph later
    stateSet = m_sceneroot->getOrCreateStateSet();
    stateSet->setMode(GL_ALPHA_TEST, osg::StateAttribute::ON);
    stateSet->setMode(GL_LIGHTING, osg::StateAttribute::ON);
    stateSet->setMode(GL_DEPTH_TEST, osg::StateAttribute::ON);

    GfOut("LE POINTEUR %d\n",mRoot.get());

    return mRoot;
}//SDRender::Init

void SDRender::UpdateLight( void )
{
    sol_angle = (float)thesky->getSA();
    moon_angle = (float)thesky->getMA();
    float sky_brightness = (float)(1.0 + cos(sol_angle)) / 2.0f;

    GfOut("Sun Angle in Render = %f\n", sol_angle);

    if (grTrack->local.rain > 0) // TODO: Different values for each rain strength value ?
    {
        BaseFogColor[0] = 0.42f;
        BaseFogColor[1] = 0.44f;
        BaseFogColor[2] = 0.50f;

        sky_brightness = (float)pow(sky_brightness, 0.5f);
    }
    else
    {
        BaseFogColor[0] = 0.84f;
        BaseFogColor[1] = 0.84f;
        BaseFogColor[2] = 1.00f;
    }

    SkyColor[0] = BaseSkyColor[0] * sky_brightness;
    SkyColor[1] = BaseSkyColor[1] * sky_brightness;
    SkyColor[2] = BaseSkyColor[2] * sky_brightness;
    SkyColor[3] = BaseSkyColor[3];
    UpdateFogColor(sol_angle);

    sd_gamma_correct_rgb( SkyColor._v );

    // 3a)cloud and fog color
    CloudsColor[0] = FogColor[0] = BaseFogColor[0] * sky_brightness;
    CloudsColor[1] = FogColor[1] = BaseFogColor[1] * sky_brightness;
    CloudsColor[2] = FogColor[2] = BaseFogColor[2] * sky_brightness;
    CloudsColor[3] = FogColor[3] = BaseFogColor[3];

    //grUpdateFogColor(sol_angle);
    sd_gamma_correct_rgb( CloudsColor._v );


    osg::Vec4f sun_color = thesky->get_sun_color();
    //float *sun_color = suncolor[0][0];

    if (sol_angle > 1.0)
    {
        if (SDVisibility > 1000 /*&& cloudsTextureIndex < 8*/)
        {
            CloudsColor[0] = CloudsColor[0] * sun_color[0];
            CloudsColor[1] = CloudsColor[1] * sun_color[1];
            CloudsColor[2] = CloudsColor[2] * sun_color[2];
        }
        else
        {
            CloudsColor[0] = CloudsColor[0] * sun_color[0];
            CloudsColor[1] = CloudsColor[1] * sun_color[0];
            CloudsColor[2] = CloudsColor[2] * sun_color[0];
        }
    }

    sd_gamma_correct_rgb( CloudsColor._v );

    // 3b) repaint the sky (simply update geometrical, color, ... state, no actual redraw)
    thesky->repaint(SkyColor, FogColor, CloudsColor, sol_angle, moon_angle,
                    NPlanets, APlanetsData, NStars, AStarsData);

    // 3c) update the main light position (it's at the sun position !)
    /*sgCoord solpos;
    TheSky->getSunPos(&solpos);
    ssgGetLight(0)-> setPosition(solpos.xyz);*/

    // 3c) update scene colors.
    if (SDVisibility > 1000 /*&& cloudsTextureIndex < 8*/)
    {
        SceneAmbiant[0] = (sun_color[0]*0.25f + CloudsColor[0]*0.75f) * sky_brightness;
        SceneAmbiant[1] = (sun_color[1]*0.25f + CloudsColor[1]*0.75f) * sky_brightness;
        SceneAmbiant[2] = (sun_color[2]*0.25f + CloudsColor[2]*0.75f) * sky_brightness;
        SceneAmbiant[3] = 1.0;

        SceneDiffuse[0] = (sun_color[0]*0.25f + FogColor[0]*0.75f) * sky_brightness;
        SceneDiffuse[1] = (sun_color[1]*0.25f + FogColor[1]*0.75f) * sky_brightness;
        SceneDiffuse[2] = (sun_color[2]*0.25f + FogColor[2]*0.75f) * sky_brightness;
        SceneDiffuse[3] = 1.0;

        SceneSpecular[0] = sun_color[0] * sky_brightness;
        SceneSpecular[1] = sun_color[1] * sky_brightness;
        SceneSpecular[2] = sun_color[2] * sky_brightness;
        SceneSpecular[3] = 1.0;
    }
    else
    {
        SceneAmbiant[0] = (sun_color[0]*0.25f + CloudsColor[0]*0.75f) * sky_brightness;
        SceneAmbiant[1] = (sun_color[0]*0.25f + CloudsColor[1]*0.75f) * sky_brightness;
        SceneAmbiant[2] = (sun_color[0]*0.25f + CloudsColor[2]*0.75f) * sky_brightness;
        SceneAmbiant[3] = 1.0;

        SceneDiffuse[0] = (sun_color[0]*0.25f + FogColor[0]*0.75f) * sky_brightness;
        SceneDiffuse[1] = (sun_color[0]*0.25f + FogColor[1]*0.75f) * sky_brightness;
        SceneDiffuse[2] = (sun_color[0]*0.25f + FogColor[2]*0.75f) * sky_brightness;
        SceneDiffuse[3] = 1.0;

        SceneSpecular[0] = sun_color[0] * sky_brightness;
        SceneSpecular[1] = sun_color[0] * sky_brightness;
        SceneSpecular[2] = sun_color[0] * sky_brightness;
        SceneSpecular[3] = 1.0;
    }
}//grUpdateLight

void SDRender::UpdateFogColor(double sol_angle)
{
    double rotation;

    // first determine the difference between our view angle and local
    // direction to the sun
    rotation = -(thesky->getSR() + SGD_PI);
    while ( rotation < 0 )
    {
        rotation += SD_2PI;
    }
    while ( rotation > SD_2PI )
    {
        rotation -= SD_2PI;
    }

    // revert to unmodified values before usign them.
    //
    osg::Vec4f sun_color = thesky->get_sun_color();

    sd_gamma_correct_rgb( BaseFogColor._v );

    // Calculate the fog color in the direction of the sun for
    // sunrise/sunset effects.
    //
    float s_red =   (BaseFogColor[0] + 2 * sun_color[0] * sun_color[0]) / 3;
    float s_green = (BaseFogColor[1] + 2 * sun_color[1] * sun_color[1]) / 3;
    float s_blue =  (BaseFogColor[2] + 2 * sun_color[2] * sun_color[2]) / 3;

    // interpolate beween the sunrise/sunset color and the color
    // at the opposite direction of this effect. Take in account
    // the current visibility.
    //
    float av = thesky->get_visibility();
    if (av > 45000)
       av = 45000;

    float avf = 0.87 - (45000 - av) / 83333.33;
    float sif = 0.5 - cos( sol_angle * 2)/2;

    if (sif < 1e-4)
       sif = 1e-4;

    float rf1 = fabs((rotation - SD_PI) / SD_PI);             // 0.0 .. 1.0
    float rf2 = avf * pow(rf1 * rf1, 1 /sif);
    float rf3 = 0.94 - rf2;

    FogColor[0] = rf3 * BaseFogColor[0] + rf2 * s_red;
    FogColor[1] = rf3 * BaseFogColor[1] + rf2 * s_green;
    FogColor[2] = rf3 * BaseFogColor[2] + rf2 * s_blue;
    sd_gamma_correct_rgb( FogColor._v );

    // make sure the colors have their original value before they are being
    // used by the rest of the program.
    //
    sd_gamma_correct_rgb( BaseFogColor._v );
}
