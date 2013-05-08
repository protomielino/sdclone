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
#include <osg/LightModel>
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

SDSky *thesky = NULL;
static tTrack *grTrack;

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

SDRender::SDRender(void)
{
    BaseSkyColor = osg::Vec3f( 0.31f, 0.43f, 0.69f );
    BaseFogColor = osg::Vec3f( 0.84f, 0.84f, 1.0f );

    SDSkyDomeDistance = 0;
    SDSkyDomeDistThresh = 12000;
    SDNbCloudLayers = 0;
    SDDynamicWeather = 0;
    SDDynamicSkyDome = false;

    SDSunDeclination = 0.0f;
    SDMoonDeclination = 0.0f;
    SDMax_Visibility = 12000.0f;
    SDVisibility = 0.0f;
    NStars = 0;
    NPlanets = 0;
    sol_angle = 0.0;
    moon_angle = 0.0;
    m_scene = NULL;
}

SDRender::~SDRender(void)
{
    delete thesky;
    m_scene = NULL;
}

SDSky * SDRender::getSky()
{
    return thesky;
}

/**
 * SDRender
 * Initialises a scene (ie a new view).
 *
 * @return 0 if OK, -1 if something failed
 */
void SDRender::Init(tTrack *track)
{
    char buf[256];
    //void *hndl = grTrackHandle;
    grTrack = track;

    std::string datapath = GetDataDir();
    //datapath +="/";
    thesky = new SDSky;
    GfOut("SDSky class\n");

    // Sky dome / background.
    SDSkyDomeDistance =
        (unsigned)(GfParmGetNum(grHandle, GR_SCT_GRAPHIC, GR_ATT_SKYDOMEDISTANCE, 0, 0) + 0.5);
    if (SDSkyDomeDistance > 0 && SDSkyDomeDistance < SDSkyDomeDistThresh)
        SDSkyDomeDistance = SDSkyDomeDistThresh; // If user enabled it (>0), must be at least the threshold.

    SDDynamicSkyDome = strcmp(GfParmGetStr(grHandle, GR_SCT_GRAPHIC, GR_ATT_DYNAMICSKYDOME, GR_ATT_DYNAMICSKYDOME_DISABLED), GR_ATT_DYNAMICSKYDOME_ENABLED) == 0;

    GfLogInfo("Graphic options : Sky dome : distance = %u m, dynamic = %s\n",
              SDSkyDomeDistance, SDDynamicSkyDome ? "true" : "false");

    // Dynamic weather.
    //grDynamicWeather = GfParmGetNum(grHandle, GR_SCT_GRAPHIC, GR_ATT_grDynamicWeather, (char*)NULL, grDynamicWeather);

    // Cloud layers.
    SDNbCloudLayers =
        (unsigned)(GfParmGetNum(grHandle, GR_SCT_GRAPHIC, GR_ATT_CLOUDLAYER, 0, 0) + 0.5);

    GfLogInfo("Graphic options : Number of cloud layers : %u\n", SDNbCloudLayers);

    SDMax_Visibility =
        (unsigned)(GfParmGetNum(grHandle, GR_SCT_GRAPHIC, GR_ATT_VISIBILITY, 0, 0));

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

    //SDMax_Visibility = 20000.0f;

    GfLogInfo("  domeSizeRation : %d\n", domeSizeRatio);

    thesky->build(datapath, SDSkyDomeDistance, SDSkyDomeDistance, 800,
                  40000, 800, 30000, NPlanets,
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

    // Initialize the whole sky dome.
    SDScenery * scenery = (SDScenery *)getScenery();
    double r_WrldX = scenery->getWorldX();
    double r_WrldY = scenery->getWorldY();
    //double r_WrldZ = SDScenery::getWorldZ();
    osg::Vec3 viewPos(r_WrldX / 2, r_WrldY/ 2, 0.0 );

    switch (grTrack->local.rain)
    {
        case TR_RAIN_NONE:
            SDVisibility = SDMax_Visibility;
            break;
        case TR_RAIN_LITTLE:
            SDVisibility = 800.0;
            break;
        case TR_RAIN_MEDIUM:
            SDVisibility = 600.0;
            break;
        case TR_RAIN_HEAVY:
            SDVisibility = 200.0;
            break;
        default:
            GfLogWarning("Unsupported rain strength value %d (assuming none)",
                         grTrack->local.rain);
            SDVisibility = 12000.0;
            break;
    }//switch Rain

    thesky->set_visibility( SDVisibility ); // Visibility in meters

    thesky->reposition( viewPos, 0, 0);
    UpdateLight();
    thesky->repaint(SkyColor, FogColor, CloudsColor, sol_angle, moon_angle, NPlanets,
                    APlanetsData, NStars, AStarsData);

    osg::ref_ptr<osg::Group> sceneGroup = new osg::Group;
    osg::ref_ptr<osg::Group> mRoot = new osg::Group;
    sceneGroup->addChild(scenery->getScene());
    //sceneGroup->setNodeMask(~simgear::BACKGROUND_BIT);
    osg::ref_ptr<osg::StateSet> stateSet = sceneGroup->getOrCreateStateSet();
    stateSet->setMode(GL_DEPTH_TEST, osg::StateAttribute::ON);

    osg::ref_ptr<osg::Material> material = new osg::Material;
    material->setColorMode(osg::Material::OFF); // switch glColor usage off
    stateSet->setAttributeAndModes(material, osg::StateAttribute::OVERRIDE|osg::StateAttribute::ON);
    stateSet->setMode(GL_LIGHTING, osg::StateAttribute::OVERRIDE|osg::StateAttribute::ON);

    osg::ref_ptr<osg::LightSource> lightSource = new osg::LightSource;
    lightSource->getLight()->setDataVariance(osg::Object::DYNAMIC);
    // relative because of CameraView being just a clever transform node
    lightSource->setReferenceFrame(osg::LightSource::RELATIVE_RF);
    lightSource->setLocalStateSetModes(osg::StateAttribute::ON);
    lightSource->getLight()->setAmbient(osg::Vec4(0.0f, 0.0f, 0.0f, 0.0f));
    lightSource->getLight()->setDiffuse(osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f));
    lightSource->getLight()->setSpecular(osg::Vec4(0.0f, 0.0f, 0.0f, 0.0f));
    //sceneGroup->addChild(lightSource);

    //assert(dynamic_cast<osg::LightSource*>(mRoot));
    //osg::LightSource lightSource = static_cast<osg::LightSource*>(mRoot);

    // we need a white diffuse light for the phase of the moon
    osg::ref_ptr<osg::LightSource> sunLight = new osg::LightSource;
    sunLight->getLight()->setDataVariance(osg::Object::DYNAMIC);
    sunLight->getLight()->setLightNum(1);
    sunLight->setReferenceFrame(osg::LightSource::RELATIVE_RF);
    sunLight->setLocalStateSetModes(osg::StateAttribute::ON);
    sunLight->getLight()->setAmbient(SceneAmbiant);
    sunLight->getLight()->setDiffuse(SceneDiffuse);
    sunLight->getLight()->setSpecular(SceneSpecular);
    sunLight->setStateSetModes(*stateSet,osg::StateAttribute::ON);

    osg::Vec3f sun_position = thesky->sunposition();
    osg::Vec3f sun_direction = -sun_position;
    osg::Vec4f position(sun_position, 1.0f);
    sunLight->getLight()->setPosition(position);
    sunLight->getLight()->setDirection(sun_direction);

    osg::ref_ptr<osg::Group> skyGroup = new osg::Group;
    osg::ref_ptr<osg::StateSet> skySS = skyGroup->getOrCreateStateSet();
    skySS->setMode(GL_LIGHT0, osg::StateAttribute::OFF);
    skyGroup->addChild(thesky->getPreRoot());
    sunLight->addChild(skyGroup.get());
    mRoot->addChild(sceneGroup.get());
    mRoot->setStateSet(setFogState().get());
    mRoot->addChild(sunLight.get());
    //mRoot->addChild(lightSource);


    // Clouds are added to the scene graph later
    stateSet = mRoot->getOrCreateStateSet();
    stateSet->setMode(GL_ALPHA_TEST, osg::StateAttribute::ON);
    stateSet->setMode(GL_LIGHTING, osg::StateAttribute::ON);
    stateSet->setMode(GL_DEPTH_TEST, osg::StateAttribute::ON);

    m_scene = new osg::Group;
    m_scene->addChild(mRoot.get());

    GfOut("LE POINTEUR %d\n",mRoot.get());

    //return mRoot.get();
}//SDRender::Init

void SDRender::UpdateLight( void )
{
    sol_angle = (float)thesky->getSA();
    moon_angle = (float)thesky->getMA();
    float deg = sol_angle * SD_RADIANS_TO_DEGREES;
    float sky_brightness = (float)(1.0 + cos(sol_angle)) / 2.0f;

    GfOut("Sun Angle in Render = %f - sky brightness = %f\n", sol_angle, sky_brightness);


    if (grTrack->local.rain > 0) // TODO: Different values for each rain strength value ?
    {
        BaseFogColor = osg::Vec3f(0.42f, 0.44f, 0.50f);
        sky_brightness = (float)pow(sky_brightness, 0.5f);
    }
    else
    {
        BaseFogColor = osg::Vec3f(0.84f, 0.87f, 1.00f);
    }

    SkyColor = BaseSkyColor * sky_brightness;

    UpdateFogColor(sol_angle);

    sd_gamma_correct_rgb( SkyColor._v );

    // 3a)cloud and fog color
    CloudsColor = FogColor = BaseFogColor * sky_brightness;

    //grUpdateFogColor(sol_angle);
    sd_gamma_correct_rgb( CloudsColor._v );


    osg::Vec4f suncolor = thesky->get_sun_color();
    //osg::Vec3f sun_color = osg::Vec3f(suncolor._v[0], suncolor._v[1], suncolor._v[2]);
    osg::Vec3f sun_color = osg::Vec3f(0.7f, 0.7f, 0.7f); // For Test !!!
    GfOut("Sun Color in Render = %f R - %f V - %f B\n", sun_color._v[0], sun_color._v[1], sun_color._v[2]);

    if (sol_angle > 1.0)
    {
        if (SDVisibility > 1000 /*&& cloudsTextureIndex < 8*/)
        {
            CloudsColor = osg::componentMultiply(CloudsColor, sun_color);
        }
        else
        {
            CloudsColor = CloudsColor * sun_color[0];
        }
    }

    sd_gamma_correct_rgb( CloudsColor._v );

    // 3b) repaint the sky (simply update geometrical, color, ... state, no actual redraw)
    thesky->repaint(SkyColor, FogColor, CloudsColor, sol_angle, moon_angle,
                    NPlanets, APlanetsData, NStars, AStarsData);

    // 3c) update scene colors.
    if (SDVisibility > 1000 /*&& cloudsTextureIndex < 8*/)
    {
        SceneAmbiant = osg::Vec4f((sun_color * 0.25f) + (CloudsColor * 0.75f) * sky_brightness, 1.0f);
        SceneDiffuse = osg::Vec4f((sun_color * 0.25f) + (FogColor * 0.75f) * sky_brightness, 1.0f);
        SceneSpecular = osg::Vec4f(sun_color * sky_brightness, 1.0f);
    }
    else
    {
        SceneAmbiant = osg::Vec4f(((CloudsColor._v[0] * 0.75f) + (sun_color._v[0] * 0.25f)) * sky_brightness,
                ((CloudsColor._v[1] * 0.75f) + (sun_color._v[0] * 0.25f)) * sky_brightness,
                ((CloudsColor._v[2] * 0.75f) + (sun_color._v[0] * 0.25f)) * sky_brightness, 1.0f);
        SceneDiffuse = osg::Vec4f(((FogColor._v[0] * 0.75f) + (sun_color._v[0] * 0.25f)) * sky_brightness,
                ((FogColor._v[1] * 0.75f) + (sun_color._v[0] * 0.25f)) * sky_brightness,
                ((FogColor._v[2] * 0.75f) + (sun_color._v[0] * 0.25f)) * sky_brightness, 1.0f);
        SceneSpecular = osg::Vec4f(sun_color._v[0] * sky_brightness, sun_color._v[0] * sky_brightness,
                                   sun_color._v[0] * sky_brightness, 1.0f);
    }

}//grUpdateLight

osg::ref_ptr< osg::StateSet> SDRender::setFogState()
{
    static const double m_log01 = -log( 0.01 );
    static const double sqrt_m_log01 = sqrt( m_log01 );
    const GLfloat fog_exp2_density = sqrt_m_log01 / thesky->get_visibility();

    SceneFog = osg::Vec4f(FogColor, 1.0f);

    osg::ref_ptr<osg::Fog> fog = new osg::Fog();    //The fog object
    fog->setMode(osg::Fog::EXP2);                   //Fog type
    fog->setDensity(fog_exp2_density);              //Fog density
    fog->setColor(SceneFog);                        //Fog color
    fog->setFogCoordinateSource(osg::Fog::FRAGMENT_DEPTH);
    osg::ref_ptr< osg::StateSet> fogState (new osg::StateSet);
    fogState->setAttributeAndModes(fog.get(), osg::StateAttribute::ON);

    fogState->setMode(GL_FOG, osg::StateAttribute::ON);

    return fogState;

}

void SDRender::UpdateFogColor(double sol_angle)
{
    double rotation;

    // first determine the difference between our view angle and local
    // direction to the sun
    rotation = -(thesky->getSR() + SD_PI);
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
    float s_red =   (BaseFogColor._v[0] + 2 * sun_color._v[0] * sun_color._v[0]) / 3;
    float s_green = (BaseFogColor._v[1] + 2 * sun_color._v[1] * sun_color._v[1]) / 3;
    float s_blue =  (BaseFogColor._v[2] + 2 * sun_color._v[2] * sun_color._v[2]) / 3;

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

    FogColor._v[0] = rf3 * BaseFogColor._v[0] + rf2 * s_red;
    FogColor._v[1] = rf3 * BaseFogColor._v[1] + rf2 * s_green;
    FogColor._v[2] = rf3 * BaseFogColor._v[2] + rf2 * s_blue;
    sd_gamma_correct_rgb( FogColor._v );

    // make sure the colors have their original value before they are being
    // used by the rest of the program.
    //
    sd_gamma_correct_rgb( BaseFogColor._v );
}
