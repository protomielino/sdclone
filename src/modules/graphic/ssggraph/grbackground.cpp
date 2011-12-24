/***************************************************************************

    file                 : grbackground.cpp
    created              : Thu Nov 25 21:09:40 CEST 2010
    copyright            : (C) 2010 by Jean-Philippe Meuret
    email                : http://www.speed-dreams.org
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

#include <ctime>

#include <robottools.h>	//RtXXX()
#include <portability.h> // snprintf
#include <glfeatures.h>

#include <plib/ssgAux.h>

#include "grscene.h"
#include "grmain.h"
#include "grcam.h"	//cGrBackgroundCam
#include "grutil.h"
#include "grSky.h"
#include "grbackground.h"


// Some exported constants.
const tdble grSkyDomeNeutralFOVDistance = 20000.0f; // Not the smallest, a medium one.

// Some private constants.
static const int NbBackgroundFaces = 36; //Background faces
static const float BackgroundDistance = 1.0f;

static const unsigned grSkyDomeDistThresh = 12000; // No dynamic sky below that value.

static const int NMaxStars = 1000;
static const int NMaxPlanets = 0; //No planets displayed for the moment
static const int NMaxCloudLayers = 3;

enum {eCBSun = 0, eCBMoon, NMaxCelestianBodies};	// Celestial bodies in the sky.

static const sgVec4 Black            = { 0.0f, 0.0f, 0.0f, 1.0f } ;
static const sgVec4 White            = { 1.0f, 1.0f, 1.0f, 1.0f } ;
static const sgVec4 TranslucentWhite = { 1.0f, 1.0f, 1.0f, 0.8f } ;

static const sgVec4 BaseSkyColor    = { 0.39f, 0.50f, 0.74f, 1.0f } ;

static const sgVec4 BaseAmbiant      = { 0.35f, 0.35f, 0.40f, 1.0f } ;
static const sgVec4 BaseDiffuse      = { 0.80f, 0.80f, 0.80f, 1.0f } ;
static const sgVec4 BaseSpecular     = { 0.33f, 0.33f, 0.30f, 1.0f } ;

static int NStars = 0;
static int NPlanets = 0;

static const int CloudsTextureIndices[TR_CLOUDS_FULL+1] = { 1, 3, 5, 7, 8 };
static const int NCloudsTextureIndices = sizeof(CloudsTextureIndices) / sizeof(int);

static const char* AEnvShadowKeys[] =
{ "no-cloud", "few-clouds", "scarce-clouds", "many-clouds", "full-cover",
  "full-cover-rain", "night" };
static const int NEnvShadowFullCoverRainIndex = 5; // Index in AEnvShadowKeys
static const int NEnvShadowNightIndex = 6; // Index in AEnvShadowKeys

// Some exported global variables.
ssgStateSelector* grEnvSelector = 0;
cgrMultiTexState* grEnvState = 0;
cgrMultiTexState* grEnvShadowState = 0;
cgrMultiTexState* grEnvShadowStateOnCars = 0;

unsigned grSkyDomeDistance = 0;
int grNBCloudfield = 0;

// Some private global variables.
//static int grDynamicWeather = 0;
static bool grDynamicSkyDome = false;
static int grBackgroundType = 0;
static float grSunDeclination = 0.0f;
static float grMoonDeclination = 0.0f;

static ssgBranch *SunAnchor = NULL;

static ssgRoot *TheBackground = NULL;
static ssgTransform *TheSun = NULL;

static cGrCelestialBody *TheCelestBodies[NMaxCelestianBodies] = { NULL, NULL };
static cGrSky *TheSky = NULL;

static sgdVec3 *AStarsData = NULL;
static sgdVec3 *APlanetsData = NULL;

static sgVec4 SkyColor;
static sgVec4 BaseFogColor;
static sgVec4 FogColor;
static sgVec4 CloudsColor;

static sgVec4 SceneAmbiant;
static sgVec4 SceneDiffuse;
static sgVec4 SceneSpecular;

/**
 * grInitBackground
 * Initialize the background (mainly the sky).
 * 
 * @return 0 if OK, -1 if something failed
 */
int
grInitBackground(void)
{
	char buf[256];
	void *hndl = grTrackHandle;
	ssgLight *light = ssgGetLight(0);
	
	// If no realistic sky dome requested, or if the track skyversion doesn't support it,
	// we set up a static - texture-based - background
	if (!grSkyDomeDistance || grTrack->skyversion < 1) 
	{
		GfLogInfo("Setting up static background (mono-texture sky and landscape)\n");

		GLfloat matSpecular[]       = {0.3, 0.3, 0.3, 1.0};
		GLfloat matShininess[]      = {50.0};
		GLfloat lightPosition[]     = {0, 0, 200, 0.0};
		GLfloat lightModelAmbient[] = {0.2, 0.2, 0.2, 1.0};
		GLfloat lightModelDiffuse[] = {0.8, 0.8, 0.8, 1.0};
		GLfloat fogColor[]        = {0.0, 0.0, 0.0, 0.5};

		matSpecular[0] = GfParmGetNum(hndl, TRK_SECT_GRAPH, TRK_ATT_SPEC_R, NULL, matSpecular[0]);
		matSpecular[1] = GfParmGetNum(hndl, TRK_SECT_GRAPH, TRK_ATT_SPEC_G, NULL, matSpecular[1]);
		matSpecular[2] = GfParmGetNum(hndl, TRK_SECT_GRAPH, TRK_ATT_SPEC_B, NULL, matSpecular[2]);

		lightModelAmbient[0] = GfParmGetNum(hndl, TRK_SECT_GRAPH, TRK_ATT_AMBIENT_R, NULL, lightModelAmbient[0]);
		lightModelAmbient[1] = GfParmGetNum(hndl, TRK_SECT_GRAPH, TRK_ATT_AMBIENT_G, NULL, lightModelAmbient[1]);
		lightModelAmbient[2] = GfParmGetNum(hndl, TRK_SECT_GRAPH, TRK_ATT_AMBIENT_B, NULL, lightModelAmbient[2]);

		lightModelDiffuse[0] = GfParmGetNum(hndl, TRK_SECT_GRAPH, TRK_ATT_DIFFUSE_R, NULL, lightModelDiffuse[0]);
		lightModelDiffuse[1] = GfParmGetNum(hndl, TRK_SECT_GRAPH, TRK_ATT_DIFFUSE_G, NULL, lightModelDiffuse[1]);
		lightModelDiffuse[2] = GfParmGetNum(hndl, TRK_SECT_GRAPH, TRK_ATT_DIFFUSE_B, NULL, lightModelDiffuse[2]);

		matShininess[0] = GfParmGetNum(hndl, TRK_SECT_GRAPH, TRK_ATT_SHIN, NULL, matShininess[0]);

		lightPosition[0] = GfParmGetNum(hndl, TRK_SECT_GRAPH, TRK_ATT_LIPOS_X, NULL, lightPosition[0]);
		lightPosition[1] = GfParmGetNum(hndl, TRK_SECT_GRAPH, TRK_ATT_LIPOS_Y, NULL, lightPosition[1]);
		lightPosition[2] = GfParmGetNum(hndl, TRK_SECT_GRAPH, TRK_ATT_LIPOS_Z, NULL, lightPosition[2]);

		glShadeModel(GL_SMOOTH);

		light->setPosition(lightPosition[0],lightPosition[1],lightPosition[2]);
		light->setColour(GL_AMBIENT,lightModelAmbient);
		light->setColour(GL_DIFFUSE,lightModelDiffuse);
		light->setColour(GL_SPECULAR,matSpecular);
		light->setSpotAttenuation(0.0, 0.0, 0.0);

		sgCopyVec3 (fogColor, grTrack->graphic.bgColor);
		sgScaleVec3 (fogColor, 0.8);
		glFogi(GL_FOG_MODE, GL_LINEAR);
		glFogfv(GL_FOG_COLOR, fogColor);
		glFogf(GL_FOG_DENSITY, 0.05);
		glHint(GL_FOG_HINT, GL_DONT_CARE);

		glEnable(GL_LIGHTING);
		glEnable(GL_LIGHT0);
		glEnable(GL_DEPTH_TEST);
		
		if (!TheSun && grTrack->local.rain == 0)
		{
			ssgaLensFlare *sun_obj = new ssgaLensFlare();
			TheSun = new ssgTransform;
			TheSun->setTransform(lightPosition);
			TheSun->addKid(sun_obj);
			SunAnchor->addKid(TheSun);
		}
	}
	
	// If realistic sky dome is requested,
	// we create the Sun, the Moon, some stars and the clouds
	else 
	{
		GfLogInfo("Setting up realistic %s sky dome :\n", grDynamicSkyDome ? "dynamic" : "static");

		//ssgSetNearFar(1, grSkyDomeDistance);

		// Determine time of day (seconds since 00:00).
		const int timeOfDay = (int)grTrack->local.timeofday;

		// Add random stars (can't optimize from real time of day,
		// in case grDynamicSkyDome - that is dynamic time of day -
		// and also simply because it'd be complicated to take care of actual lattitude).
		NStars = NMaxStars;
		if (AStarsData)
			delete [] AStarsData;
		AStarsData = new sgdVec3[NStars];
		for(int i= 0; i < NStars; i++) 
		{
			AStarsData[i][0] = grRandom() * PI;
			AStarsData[i][1] = grRandom() * PI;
			AStarsData[i][2] = grRandom();
		}//for i

		GfLogInfo("  Stars (random) : %d\n", NStars);
		
		//No planets
		NPlanets = 0;
		APlanetsData = NULL;

		GfLogInfo("  Planets : %d\n", NPlanets);
		
		//Build the sky
		TheSky	= new cGrSky;
		TheSky->build(grSkyDomeDistance, grSkyDomeDistance, NPlanets, APlanetsData, NStars, AStarsData);
		
		//Add the Sun itself
		const double domeSizeRatio = grSkyDomeDistance / 80000.0;
		TheCelestBodies[eCBSun] = TheSky->addBody(NULL, "data/textures/halo.rgba",
												  2500 * domeSizeRatio, grSkyDomeDistance, /*isSun=*/true);
		GLfloat sunAscension = grTrack->local.sunascension;
		grSunDeclination = (float)(15 * (double)timeOfDay / 3600 - 90.0);

		TheCelestBodies[eCBSun]->setDeclination ( DEG2RAD(grSunDeclination));
		TheCelestBodies[eCBSun]->setRightAscension ( sunAscension );

		GfLogInfo("  Sun : time of day = %02d:%02d:%02d (declination = %.1f deg), "
		          "ascension = %.1f deg\n", 
				  timeOfDay / 3600, (timeOfDay % 3600) / 60, timeOfDay % 60,
				  grSunDeclination, RAD2DEG(sunAscension));

		// Add the Moon (TODO: Find a better solution than this random positioning !)
		TheCelestBodies[eCBMoon] = TheSky->addBody ( "data/textures/moon.rgba",NULL,
													 2500 * domeSizeRatio, grSkyDomeDistance);
		if ( grSunDeclination > 180 )
			grMoonDeclination = 3.0 + (rand() % 40);
		else
			grMoonDeclination = (rand() % 270);

		const float moonAscension = (float)(rand() % 360);
		
		TheCelestBodies[eCBMoon]->setDeclination ( DEG2RAD(grMoonDeclination) );
		TheCelestBodies[eCBMoon]->setRightAscension ( DEG2RAD(moonAscension) );

		GfLogInfo("  Moon : declination = %.1f deg, ascension = %.1f deg\n",
				  grMoonDeclination, moonAscension);

		// Add the cloud layers
		// TODO :
		//  * Why does thickness and transition get greater as the sky dome distance decreases ?
		//  * More/different cloud layers for each rain strength value (only 2 as for now) ?
		const int cloudsTextureIndex = CloudsTextureIndices[grTrack->local.clouds];

		cGrCloudLayer *cloudLayers[NMaxCloudLayers];
		snprintf(buf, sizeof(buf), "data/textures/scattered%d.rgba", cloudsTextureIndex);
		if (grTrack->local.rain > 0 )
			cloudLayers[0] = TheSky->addCloud(buf, grSkyDomeDistance, 650,
											  400 / domeSizeRatio, 400 / domeSizeRatio);
		else if (grNBCloudfield == 1)
		{
			cloudLayers[0] = TheSky->addCloud(buf, grSkyDomeDistance, 2550,
											  100 / domeSizeRatio, 100 / domeSizeRatio);
		cloudLayers[0]->setSpeed(60);
		cloudLayers[0]->setDirection(45);
		GfLogInfo("  Cloud cover : 1 layer, texture=%s, speed=60, direction=45\n", buf);
		} else if (grNBCloudfield == 2)
		{
			snprintf(buf, sizeof(buf), "data/textures/scattered1.rgba", 1);
			cloudLayers[0] = TheSky->addCloud(buf, grSkyDomeDistance, 3000,
											  100 / domeSizeRatio, 100 / domeSizeRatio);
			cloudLayers[0]->setSpeed(30);
			cloudLayers[0]->setDirection(40);

			snprintf(buf, sizeof(buf), "data/textures/scattered%d.rgba", cloudsTextureIndex);
			cloudLayers[1] = TheSky->addCloud(buf, grSkyDomeDistance, 2000,
											  100 / domeSizeRatio, 100 / domeSizeRatio);
			cloudLayers[1]->setSpeed(60);
			cloudLayers[1]->setDirection(45);
			GfLogInfo("  Cloud cover : 1 layer, texture=01, speed=30, direction=40\n  Cloud cover : 2 layer, texture=%s, speed=60, direction=45\n", buf);
		} else if (grNBCloudfield == 3)
		{
			snprintf(buf, sizeof(buf), "data/textures/scattered1.rgba", 1);
			cloudLayers[0] = TheSky->addCloud(buf, grSkyDomeDistance, 3000,
											  100 / domeSizeRatio, 100 / domeSizeRatio);
			cloudLayers[0]->setSpeed(30);
			cloudLayers[0]->setDirection(40);

			snprintf(buf, sizeof(buf), "data/textures/scattered%d.rgba", cloudsTextureIndex);
			cloudLayers[1] = TheSky->addCloud(buf, grSkyDomeDistance, 2000,
											  100 / domeSizeRatio, 100 / domeSizeRatio);
			cloudLayers[1]->setSpeed(60);
			cloudLayers[1]->setDirection(45);

			snprintf(buf, sizeof(buf), "data/textures/scattered%d.rgba", cloudsTextureIndex);
			cloudLayers[2] = TheSky->addCloud(buf, grSkyDomeDistance, 1000,
											  100 / domeSizeRatio, 100 / domeSizeRatio);
			cloudLayers[2]->setSpeed(80);
			cloudLayers[2]->setDirection(45);
			GfLogInfo("  Cloud cover : 1 layer, texture=01, speed=30, direction=40\n  Cloud cover : 2 layer, texture=%s, speed=60, direction=45\n Cloud cover : 3 layer, texture=%s, speed=80, direction=45\n", buf);
		}

		// Set up the light source to the Sun position.
		sgCoord sunPosition;
		TheCelestBodies[eCBSun]->getPosition(&sunPosition);
    	light->setPosition(sunPosition.xyz);

		// Initialize the whole sky dome.
		sgVec3 viewPos;
		//sgSetVec3(viewPos, (track->max.x)/2, 0, (track->max.z)/2);
		sgSetVec3(viewPos,grWrldX/2, grWrldY/2, 0);
		TheSky->repositionFlat(viewPos, 0, 0);    

		//Setup visibility according to rain if any
		// TODO: Does visibility really decrease when rain gets heavier ????
		float visibility = 0.0f;
		switch (grTrack->local.rain)	
		{
			case TR_RAIN_NONE:
				visibility = 0.0f;
				break;
			case TR_RAIN_LITTLE:
				visibility = 400.0f;
				break;
			case TR_RAIN_MEDIUM:
				visibility = 500.0f;
				break;
			case TR_RAIN_HEAVY:
				visibility = 550.0f;
				break;
			default:
				GfLogWarning("Unsupported rain strength value %d (assuming none)",
							 grTrack->local.rain);
				visibility = 0.0f;
				break;
		}//switch Rain
		
		TheSky->modifyVisibility( visibility, 0);
    
		//Setup overall light level according to rain if any
		const float sol_angle = (float)TheCelestBodies[eCBSun]->getAngle();
		const float sky_brightness = (float)(1.0 + cos(sol_angle)) / 2.0f;
		float scene_brightness = (float)pow(sky_brightness, 0.5f);
        
		if (grTrack->local.rain > 0) // TODO: Different values for each rain strength value ?
		{
			BaseFogColor[0] = 0.40f;
			BaseFogColor[1] = 0.43f;
			BaseFogColor[2] = 0.45f;

			scene_brightness = scene_brightness / 2.0f;			
		}
		else
		{
			BaseFogColor[0] = 0.84f;
			BaseFogColor[1] = 0.87f;
			BaseFogColor[2] = 1.00f;

			scene_brightness = scene_brightness;
		}
    
		SkyColor[0] = BaseSkyColor[0] * sky_brightness;
		SkyColor[1] = BaseSkyColor[1] * sky_brightness;
		SkyColor[2] = BaseSkyColor[2] * sky_brightness;
		SkyColor[3] = BaseSkyColor[3];
		
		CloudsColor[0] = FogColor[0] = BaseFogColor[0] * sky_brightness;
		CloudsColor[1] = FogColor[1] = BaseFogColor[1] * sky_brightness;
		CloudsColor[2] = FogColor[2] = BaseFogColor[2] * sky_brightness;
		CloudsColor[3] = FogColor[3] = BaseFogColor[3];
	
		TheSky->repaint(SkyColor, FogColor, CloudsColor, sol_angle,
						NPlanets, APlanetsData, NStars, AStarsData);
	
		sgCoord solpos;
		TheCelestBodies[eCBSun]->getPosition(&solpos);
		light->setPosition(solpos.xyz);	
	
		SceneAmbiant[0] = BaseAmbiant[0] * scene_brightness;
		SceneAmbiant[1] = BaseAmbiant[1] * scene_brightness;
		SceneAmbiant[2] = BaseAmbiant[2] * scene_brightness;
		SceneAmbiant[3] = 1.0;
	
		SceneDiffuse[0] = BaseDiffuse[0] * scene_brightness;
		SceneDiffuse[1] = BaseDiffuse[1] * scene_brightness;
		SceneDiffuse[2] = BaseDiffuse[2] * scene_brightness;
		SceneDiffuse[3] = 1.0;
	
		SceneSpecular[0] = BaseSpecular[0] * scene_brightness;
		SceneSpecular[1] = BaseSpecular[1] * scene_brightness;
		SceneSpecular[2] = BaseSpecular[2] * scene_brightness;
		SceneSpecular[3] = 1.0;
	
		glLightModelfv( GL_LIGHT_MODEL_AMBIENT, Black);
		ssgGetLight(0) -> setColour( GL_AMBIENT, SceneAmbiant);
		ssgGetLight(0) -> setColour( GL_DIFFUSE, SceneDiffuse);
		ssgGetLight(0) -> setColour( GL_SPECULAR, SceneSpecular);
	}//else grSkyDomeDistance 

	/* GUIONS GL_TRUE */
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,GL_FALSE);

#ifdef GL_SEPARATE_SPECULAR_COLOR 
	glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL,GL_SEPARATE_SPECULAR_COLOR);
#else
	#ifdef GL_SEPARATE_SPECULAR_COLOR_EXT
	glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL_EXT,GL_SEPARATE_SPECULAR_COLOR_EXT);
	#	endif
#endif

  return 0;
}//grInitBackground


void
grLoadBackground(void)
{
	char buf[256];
	int			i;
	float		x, y, z1, z2;
	double		alpha;
	float		texLen;
	ssgSimpleState	*envst;
	sgVec3		vtx;
	sgVec4		clr;
	sgVec3		nrm;
	sgVec2		tex;
	ssgVtxTable 	*bg;
	ssgVertexArray	*bg_vtx;
	ssgTexCoordArray	*bg_tex;
	ssgColourArray	*bg_clr;
	ssgNormalArray	*bg_nrm;
	ssgSimpleState	*bg_st;

	// Load graphic options for the sky dome / background.
	grSkyDomeDistance =
		(unsigned)GfParmGetNum(grHandle, GR_SCT_GRAPHIC, GR_ATT_SKYDOMEDISTANCE, (char*)NULL, grSkyDomeDistance);
	if (grSkyDomeDistance > 0 && grSkyDomeDistance < grSkyDomeDistThresh)
		grSkyDomeDistance = grSkyDomeDistThresh; // If user enabled it (>0), must be at least the threshold.
	grDynamicSkyDome = grSkyDomeDistance > 0 && strcmp(GfParmGetStr(grHandle, GR_SCT_GRAPHIC, GR_ATT_DYNAMICSKYDOME, GR_ATT_DYNAMICSKYDOME_DISABLED), GR_ATT_DYNAMICSKYDOME_ENABLED) == 0; 

	//grDynamicWeather = GfParmGetNum(grHandle, GR_SCT_GRAPHIC, GR_ATT_grDynamicWeather, (char*)NULL, grDynamicWeather);
			
	GfLogInfo("Graphic options : Sky dome : distance = %d m, dynamic = %s\n",
			  grSkyDomeDistance, grDynamicSkyDome ? "true" : "false");

	grNBCloudfield =
		(unsigned)GfParmGetNum(grHandle, GR_SCT_GRAPHIC, GR_ATT_CLOUDLAYER, (char*)NULL, grNBCloudfield);

	GfLogInfo("Graphic options : CloudField Number : = %d\n", grNBCloudfield);

	snprintf(buf, sizeof(buf), "tracks/%s/%s;data/img;data/textures;.",
			grTrack->category, grTrack->internalname);
	grFilePath = buf;
	grGammaValue = 1.8;
	grMipMap = 0;

	// Load static background if no sky dome.
	const tTrackGraphicInfo *graphic = &grTrack->graphic;
	TheBackground = 0;
	if (!grSkyDomeDistance || grTrack->skyversion < 1)
	{
		GfLogInfo("Loading static background (type %d)\n", graphic->bgtype);

		glClearColor(graphic->bgColor[0], graphic->bgColor[1], graphic->bgColor[2], 1.0);

		TheBackground = new ssgRoot();
		clr[0] = clr[1] = clr[2] = 1.0 / (1.0 + 1.0 * grTrack->local.rain);
		clr[3] = 1.0;
		nrm[0] = nrm[2] = 0.0;
		nrm[1] = 1.0;

		z1 = -0.5;
		z2 = 1.0;
	
		grBackgroundType = graphic->bgtype;
		switch (grBackgroundType) {
			case TR_BACKGROUND_TYPE_0: //-----------------------------------------------------------
				bg_vtx = new ssgVertexArray(NbBackgroundFaces + 1);
				bg_tex = new ssgTexCoordArray(NbBackgroundFaces + 1);
				bg_clr = new ssgColourArray(1);
				bg_nrm = new ssgNormalArray(1);
				bg_clr->add(clr);
				bg_nrm->add(nrm);
	
				for (i = 0; i < NbBackgroundFaces + 1; i++) {
					alpha = (float)i * 2 * PI / (float)NbBackgroundFaces;
					texLen = (float)i / (float)NbBackgroundFaces;

					x = BackgroundDistance * cos(alpha);
					y = BackgroundDistance * sin(alpha);
	    
					vtx[0] = x;
					vtx[1] = y;
					vtx[2] = z1;
					bg_vtx->add(vtx);
					tex[0] = texLen*4.0;
					tex[1] = 0;
					bg_tex->add(tex);

					vtx[0] = x;
					vtx[1] = y;
					vtx[2] = z2;
					bg_vtx->add(vtx);
					tex[0] = texLen*4.0;
					tex[1] = 1.0;
					bg_tex->add(tex);
				}//for i
			
				bg = new ssgVtxTable(GL_TRIANGLE_STRIP, bg_vtx, bg_nrm, bg_tex, bg_clr);
				bg_st = (ssgSimpleState*)grSsgLoadTexState(graphic->background);
				bg_st->disable(GL_LIGHTING);
				bg->setState(bg_st);
				bg->setCullFace(0);
				TheBackground->addKid(bg);
				break;	//case 1

			case TR_BACKGROUND_TYPE_2: //-----------------------------------------------------------
				bg_vtx = new ssgVertexArray(NbBackgroundFaces + 1);
				bg_tex = new ssgTexCoordArray(NbBackgroundFaces + 1);
				bg_clr = new ssgColourArray(1);
				bg_nrm = new ssgNormalArray(1);
				bg_clr->add(clr);
				bg_nrm->add(nrm);

				for (i = 0; i < NbBackgroundFaces / 4 + 1; i++) {
					alpha = (float)i * 2 * PI / (float)NbBackgroundFaces;
					texLen = (float)i / (float)NbBackgroundFaces;
	    
					x = BackgroundDistance * cos(alpha);
					y = BackgroundDistance * sin(alpha);
	    
					vtx[0] = x;
					vtx[1] = y;
					vtx[2] = z1;
					bg_vtx->add(vtx);
					tex[0] = texLen*4.0;
					tex[1] = 0;
					bg_tex->add(tex);

					vtx[0] = x;
					vtx[1] = y;
					vtx[2] = z2;
					bg_vtx->add(vtx);
					tex[0] = texLen*4.0;
					tex[1] = 0.5;
					bg_tex->add(tex);
				}//for i
		
				bg = new ssgVtxTable(GL_TRIANGLE_STRIP, bg_vtx, bg_nrm, bg_tex, bg_clr);
				bg_st = (ssgSimpleState*)grSsgLoadTexState(graphic->background);
				bg_st->disable(GL_LIGHTING);
				bg->setState(bg_st);
				bg->setCullFace(0);
				TheBackground->addKid(bg);


				bg_vtx = new ssgVertexArray(NbBackgroundFaces + 1);
				bg_tex = new ssgTexCoordArray(NbBackgroundFaces + 1);
				bg_clr = new ssgColourArray(1);
				bg_nrm = new ssgNormalArray(1);
				bg_clr->add(clr);
				bg_nrm->add(nrm);

				for (i = NbBackgroundFaces/4; i < NbBackgroundFaces / 2 + 1; i++) {
					alpha = (float)i * 2 * PI / (float)NbBackgroundFaces;
					texLen = (float)i / (float)NbBackgroundFaces;
	    
					x = BackgroundDistance * cos(alpha);
					y = BackgroundDistance * sin(alpha);
				
					vtx[0] = x;
					vtx[1] = y;
					vtx[2] = z1;
					bg_vtx->add(vtx);
					tex[0] = texLen*4.0;
					tex[1] = 0.5;
					bg_tex->add(tex);

					vtx[0] = x;
					vtx[1] = y;
					vtx[2] = z2;
					bg_vtx->add(vtx);
					tex[0] = texLen*4.0;
					tex[1] = 1.0;
					bg_tex->add(tex);
				}//for i
			
				bg = new ssgVtxTable(GL_TRIANGLE_STRIP, bg_vtx, bg_nrm, bg_tex, bg_clr);
				bg_st = (ssgSimpleState*)grSsgLoadTexState(graphic->background);
				bg_st->disable(GL_LIGHTING);
				bg->setState(bg_st);
				bg->setCullFace(0);
				TheBackground->addKid(bg);


				bg_vtx = new ssgVertexArray(NbBackgroundFaces + 1);
				bg_tex = new ssgTexCoordArray(NbBackgroundFaces + 1);
				bg_clr = new ssgColourArray(1);
				bg_nrm = new ssgNormalArray(1);

				bg_clr->add(clr);
				bg_nrm->add(nrm);

				for (i = NbBackgroundFaces / 2; i < 3 * NbBackgroundFaces / 4 + 1; i++) {
					alpha = (float)i * 2 * PI / (float)NbBackgroundFaces;
					texLen = (float)i / (float)NbBackgroundFaces;
	    
					x = BackgroundDistance * cos(alpha);
					y = BackgroundDistance * sin(alpha);
	    
					vtx[0] = x;
					vtx[1] = y;
					vtx[2] = z1;
					bg_vtx->add(vtx);
					tex[0] = texLen*4.0;
					tex[1] = 0.0;
					bg_tex->add(tex);

					vtx[0] = x;
					vtx[1] = y;
					vtx[2] = z2;
					bg_vtx->add(vtx);
					tex[0] = texLen*4.0;
					tex[1] = 0.5;
					bg_tex->add(tex);
				}//for i
	
				bg = new ssgVtxTable(GL_TRIANGLE_STRIP, bg_vtx, bg_nrm, bg_tex, bg_clr);
				bg_st = (ssgSimpleState*)grSsgLoadTexState(graphic->background);
				bg_st->disable(GL_LIGHTING);
				bg->setState(bg_st);
				bg->setCullFace(0);
				TheBackground->addKid(bg);


				bg_vtx = new ssgVertexArray(NbBackgroundFaces + 1);
				bg_tex = new ssgTexCoordArray(NbBackgroundFaces + 1);
				bg_clr = new ssgColourArray(1);
				bg_nrm = new ssgNormalArray(1);

				bg_clr->add(clr);
				bg_nrm->add(nrm);

				for(i = 3 * NbBackgroundFaces / 4; i < NbBackgroundFaces + 1; i++) {
					alpha = (float)i * 2 * PI / (float)NbBackgroundFaces;
					texLen = (float)i / (float)NbBackgroundFaces;
				
					x = BackgroundDistance * cos(alpha);
					y = BackgroundDistance * sin(alpha);
				
					vtx[0] = x;
					vtx[1] = y;
					vtx[2] = z1;
					bg_vtx->add(vtx);
					tex[0] = texLen*4.0;
					tex[1] = 0.5;
					bg_tex->add(tex);

					vtx[0] = x;
					vtx[1] = y;
					vtx[2] = z2;
					bg_vtx->add(vtx);
					tex[0] = texLen*4.0;
					tex[1] = 1.0;
					bg_tex->add(tex);
				}//for i
		
				bg = new ssgVtxTable(GL_TRIANGLE_STRIP, bg_vtx, bg_nrm, bg_tex, bg_clr);
				bg_st = (ssgSimpleState*)grSsgLoadTexState(graphic->background);
				bg_st->disable(GL_LIGHTING);
				bg->setState(bg_st);
				bg->setCullFace(0);
				TheBackground->addKid(bg);

				break;	//case 2

			case TR_BACKGROUND_TYPE_4: //-----------------------------------------------------------
				z1 = -1.0;
				z2 = 1.0;

				bg_vtx = new ssgVertexArray(NbBackgroundFaces + 1);
				bg_tex = new ssgTexCoordArray(NbBackgroundFaces + 1);
				bg_clr = new ssgColourArray(1);
				bg_nrm = new ssgNormalArray(1);
				bg_clr->add(clr);
				bg_nrm->add(nrm);

				for (i = 0; i < NbBackgroundFaces + 1; i++) {
					alpha = (double)i * 2 * PI / (double)NbBackgroundFaces;
					texLen = 1.0 - (float)i / (float)NbBackgroundFaces;
	    
					x = BackgroundDistance * cos(alpha);
					y = BackgroundDistance * sin(alpha);
	    
					vtx[0] = x;
					vtx[1] = y;
					vtx[2] = z1;
					bg_vtx->add(vtx);
					tex[0] = texLen;
					tex[1] = 0;
					bg_tex->add(tex);

					vtx[0] = x;
					vtx[1] = y;
					vtx[2] = z2;
					bg_vtx->add(vtx);
					tex[0] = texLen;
					tex[1] = 1.0;
					bg_tex->add(tex);
				}//for i
	
				bg = new ssgVtxTable(GL_TRIANGLE_STRIP, bg_vtx, bg_nrm, bg_tex, bg_clr);
				bg_st = (ssgSimpleState*)grSsgLoadTexState(graphic->background);
				bg_st->disable(GL_LIGHTING);
				bg->setState(bg_st);
				bg->setCullFace(0);
				TheBackground->addKid(bg);
				break;//case 4

			default:
				GfLogError("Unsupported background type %d\n", graphic->bgtype);
				break;
		}//switch grBackgroundType
		
		if (!SunAnchor && grTrack->local.rain == 0) {
			// Lens Flares when no sky dome (realistic sky dome will use another system when ready).
			SunAnchor = new ssgBranch;
			TheScene->addKid(SunAnchor);
		}

	} //if (!grSkyDomeDistance || grTrack->skyversion < 1)
	else
	{
		// Check / fix the cloud cover index, in any case.
		if (grTrack->local.clouds < 0)
			grTrack->local.clouds = 0;
		else if(grTrack->local.clouds >= NCloudsTextureIndices)
			grTrack->local.clouds = NCloudsTextureIndices - 1;
	}
	
	// Environment Mapping Settings
	// 1) Horizontal reflexions of the track objects (env.png & co)
	bool bUseEnvPng = false;   // Avoid crash with missing env.rgb files (i.e. Wheel-1)
	bool bDoNotUseEnv = false; // Avoid crash with missing env.png
	grEnvSelector = new ssgStateSelector(graphic->envnb);
	for (i = 0; i < graphic->envnb; i++) {
		GfLogTrace("Loading #%d track-specific env. mapping image :\n", i+1);
		envst = (ssgSimpleState*)grSsgLoadTexState(graphic->env[i]);
        // Avoid crash with missing env.rgb files (i.e. Wheel-1)
		if (!envst) {
			GfLogWarning("Failed : trying fallback env.png\n");
			envst = (ssgSimpleState*)grSsgLoadTexState("env.png");
			if (!envst) {
				GfLogError("No usable Environment Mapping Image for #%d : stop displaying graphics!\n", i);
				bDoNotUseEnv = true;
				break;
			}
			else
				bUseEnvPng = true;
		}
		envst->enable(GL_BLEND);
		grEnvSelector->setStep(i, envst);
		envst->deRef();
	}//for i

	grEnvSelector->selectStep(0); //mandatory !!!
  
	// Avoid crash with missing env.rgb files (i.e. Wheel-1)
	GfLogTrace("Loading global env. mapping image :\n");
	if (bUseEnvPng)
		grEnvState = grSsgEnvTexState("env.png", cgrMultiTexState::modulate);
	else if (bDoNotUseEnv)
		GfLogError("No env.png found!\n");
	else
		grEnvState = grSsgEnvTexState(graphic->env[0], cgrMultiTexState::modulate);

	// 2) Sky shadows (vertical) (envshadow-xxx.png), according to the weather conditions
	GfLogTrace("Loading sky shadow mapping image :\n");
	grEnvShadowState = 0;
	int nEnvShadowIndex = -1; // Default = not depending on weather conds.
	if (!grSkyDomeDistance || grTrack->skyversion < 1)
	{
		// Static / texture-based sky case.
		if (grTrack->local.rain > 0) // Rain => full cloud cover.
			nEnvShadowIndex = NEnvShadowFullCoverRainIndex;
	}
	else
	{
		// Realistic sky dome case.
		// TODO: Find a solution for the "dynamic time" case (not correctly supported here).
		if (grTrack->local.timeofday < 6*60*60 || grTrack->local.timeofday > 18*60*60)
			nEnvShadowIndex = NEnvShadowNightIndex;
		else if (grTrack->local.rain > 0) // Rain => full cloud cover.
			nEnvShadowIndex = NEnvShadowFullCoverRainIndex;
		else
			nEnvShadowIndex = grTrack->local.clouds;
	}
	if (nEnvShadowIndex >= 0)
	{
		char pszEnvFile[64];
		snprintf(pszEnvFile, sizeof(pszEnvFile), "envshadow-%s.png", AEnvShadowKeys[nEnvShadowIndex]);
		grEnvShadowState = grSsgEnvTexState(pszEnvFile, cgrMultiTexState::addColorModulateAlpha);
		if (!grEnvShadowState)
			GfLogWarning("%s not found ; falling back to weather-independant sky shadows"
						 " from envshadow.png\n", pszEnvFile);
	}
	if (!grEnvShadowState)
		grEnvShadowState = grSsgEnvTexState("envshadow.png", cgrMultiTexState::addColorModulateAlpha);
	if (!grEnvShadowState) {
		GfLogError("envshadow.png not found ; exiting !\n");
		GfLogError("(mandatory for top env mapping (should be in <track>.xml or data/textures ;\n");
		GfLogError(" copy the envshadow.png from 'chemisay' to the track you selected ;\n");
		GfLogError(" sorry for exiting, but it would have actually crashed)\n");
		GfScrShutdown();
		exit(-1);
	}//if grEnvShadowState
    
	// 3) Vertical shadows of track objects on the cars (shadow2.png)
	GfLogTrace("Loading track shadows mapping image :\n");
	grEnvShadowStateOnCars = grSsgEnvTexState("shadow2.png", cgrMultiTexState::modulate);
	if(!grEnvShadowStateOnCars)
		grEnvShadowStateOnCars = grSsgEnvTexState("shadow2.rgb", cgrMultiTexState::modulate);
  
	if(!grEnvShadowStateOnCars)
		GfLogWarning("shadow2.png/rgb not found ; no shadow mapping on cars for this track\n");
}//grLoadBackground

void
grPreDrawSky(tSituation* s, float fogStart, float fogEnd) 
{
	static const double m_log01 = -log( 0.01 );
	static const double sqrt_m_log01 = sqrt( m_log01 );

	if (grSkyDomeDistance && grTrack->skyversion > 0) 
	{
		const GLfloat fog_exp2_density = (float)sqrt_m_log01 / TheSky->getVisibility();
		glEnable(GL_FOG);
		//glFogf(GL_FOG_START, fogStart);
		//glFogf(GL_FOG_END, fogEnd);
		//glFogi(GL_FOG_MODE, GL_LINEAR);
		glFogi(GL_FOG_MODE, GL_EXP2);
		glFogfv(GL_FOG_COLOR, FogColor);
		glFogf(GL_FOG_DENSITY, fog_exp2_density);
		glHint(GL_FOG_HINT, GL_DONT_CARE);

		ssgGetLight(0)->setColour(GL_DIFFUSE, White);

		TheSky->preDraw();

		glLightModelfv( GL_LIGHT_MODEL_AMBIENT, Black);
		ssgGetLight(0)->setColour(GL_AMBIENT, SceneAmbiant);
		ssgGetLight(0)->setColour(GL_DIFFUSE, SceneDiffuse);
		ssgGetLight(0)->setColour(GL_SPECULAR, SceneSpecular);
		
	}
}//grPreDrawSky

void
grDrawStaticBackground(cGrCamera *cam, cGrBackgroundCam *bgCam) 
{
	if (!TheBackground)
		return;
	
	TRACE_GL("grDrawStaticBackground: ssgCullAndDraw start");

	bgCam->update(cam);
	bgCam->action();
	ssgCullAndDraw(TheBackground);

	TRACE_GL("grDrawStaticBackground: ssgCullAndDraw");
}//grDrawStaticBackground

void
grPostDrawSky(void) 
{
	if (grSkyDomeDistance && grTrack->skyversion > 0)
		TheSky->postDraw(grSkyDomeDistance);
}//grPostDrawSky

// Update the sky when time changes
void
grUpdateSky(double currentTime)
{
	// Nothing to do if static sky dome, or race not started.
	if (!grDynamicSkyDome)
		return;

	if (currentTime < 0)
		return;
	
	// Detect first call (in order to initialize "last times").
	static bool bInitialized = false;
	static double lastTimeHighSpeed = 0;
	static int lastTimeLowSpeed = 0;
	if (!bInitialized)
	{
		lastTimeHighSpeed = currentTime;
		lastTimeLowSpeed = 60 * (int)floor(currentTime / 60.0);
		bInitialized = true;
		return;
	}

	// At each call, update possibly high speed objects of the sky dome : the clouds.
 	sgVec3 viewPos;
 	sgSetVec3(viewPos, grWrldX/2, grWrldY/2, 0);
	//sgSetVec3(viewPos, 0, 0, 0);
	TheSky->repositionFlat(viewPos, 0, currentTime - lastTimeHighSpeed);    

	// Now, we are done for high speed objects.
	lastTimeHighSpeed = currentTime;

	// Check if time to update low speed objects : sun and moon (once every minute).
	int nextTimeLowSpeed = 60 * (int)floor((currentTime + 60.0) / 60.0);
	if (nextTimeLowSpeed == lastTimeLowSpeed)
		return;
	const float deltaTimeLowSpeed = (float)(nextTimeLowSpeed - lastTimeLowSpeed);
	lastTimeLowSpeed = nextTimeLowSpeed;

	// Update sun and moon, and thus global lighting / coloring parameters of the scene.
	//GfLogDebug("%f : Updating slow objects of the dynamic sky dome (sun and moon)\n", currentTime);
	
	// 1) Update sun position
	const float deltaDecl = deltaTimeLowSpeed * 360.0f / (24.0f * 60.0f * 60.0f);
	grSunDeclination += deltaDecl;
	if (grSunDeclination >= 360.0f)
		grSunDeclination -= 360.0f;
	
	TheCelestBodies[eCBSun]->setDeclination ( DEG2RAD(grSunDeclination) );

	// 2) Update moon position
	grMoonDeclination += deltaDecl;
	if (grMoonDeclination >= 360.0f)
		grMoonDeclination -= 360.0f;
	
	TheCelestBodies[eCBMoon]->setDeclination ( DEG2RAD(grMoonDeclination) );

	// 3) Update scene color and light
	const float sol_angle = (float)TheCelestBodies[eCBSun]->getAngle();
	const float sky_brightness = (float)(1.0 + cos(sol_angle)) / 2.0f;
	const float scene_brightness = (float)pow(sky_brightness, 0.5f);
	
	SkyColor[0] = BaseSkyColor[0] * sky_brightness;
	SkyColor[1] = BaseSkyColor[1] * sky_brightness;
	SkyColor[2] = BaseSkyColor[2] * sky_brightness;
	SkyColor[3] = BaseSkyColor[3];

	// 3a)cloud and fog color
	CloudsColor[0] = FogColor[0] = BaseFogColor[0] * sky_brightness;
	CloudsColor[1] = FogColor[1] = BaseFogColor[1] * sky_brightness;
	CloudsColor[2] = FogColor[2] = BaseFogColor[2] * sky_brightness;
	CloudsColor[3] = FogColor[3] = BaseFogColor[3];

	// 3b) repaint the sky (simply update geometrical, color, ... state, no actual redraw)
	TheSky->repaint(SkyColor, FogColor, CloudsColor, sol_angle,
					NPlanets, APlanetsData, NStars, AStarsData);

	// 3c) update the main light position (it's at the sun position !)
	sgCoord solpos;
	TheCelestBodies[eCBSun]-> getPosition(&solpos);
	ssgGetLight(0)-> setPosition(solpos.xyz);	

	// 3c) update scene colors.
	SceneAmbiant[0] = BaseAmbiant[0] * scene_brightness;
	SceneAmbiant[1] = BaseAmbiant[1] * scene_brightness;
	SceneAmbiant[2] = BaseAmbiant[2] * scene_brightness;
	
	SceneDiffuse[0] = BaseDiffuse[0] * scene_brightness;
	SceneDiffuse[1] = BaseDiffuse[1] * scene_brightness;
	SceneDiffuse[2] = BaseDiffuse[2] * scene_brightness;
	
	SceneSpecular[0] = BaseSpecular[0] * scene_brightness;
	SceneSpecular[1] = BaseSpecular[1] * scene_brightness;
	SceneSpecular[2] = BaseSpecular[2] * scene_brightness;
}//grUpdateSky

void
grShutdownBackground(void)
{
	if (TheSky) {
		delete TheSky;
		TheSky = 0;
	}
	
	if (TheSun)
		TheSun = 0;
	
	if (SunAnchor)
		SunAnchor = 0;
	
	if (grEnvState) {
		ssgDeRefDelete(grEnvState);
		grEnvState = 0;
	}
	
	if (grEnvShadowState) {
		ssgDeRefDelete(grEnvShadowState);
		grEnvShadowState = 0;
	}
	
	if (grEnvShadowStateOnCars) {
		ssgDeRefDelete(grEnvShadowStateOnCars);
		grEnvShadowStateOnCars = 0;
	}
	
	if(grEnvSelector) {
		delete grEnvSelector;
		grEnvSelector = 0;
	}

}//grShutdownBackground
