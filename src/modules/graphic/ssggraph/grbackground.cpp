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
#include <glfeatures.h> // snprintf

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

static const sgVec4 BaseAmbiant      = { 0.2f, 0.2f, 0.2f, 1.0f } ;
static const sgVec4 BaseDiffuse      = { 0.8f, 0.8f, 0.8f, 1.0f } ;
static const sgVec4 BaseSpecular     = { 0.3f, 0.3f, 0.3f, 1.0f } ;

static int NStars = 0;
static int NPlanets = 0;

// Some exported global variables.
ssgStateSelector *grEnvSelector = NULL;
grMultiTexState	*grEnvState = NULL;
grMultiTexState	*grEnvShadowState = NULL;
grMultiTexState	*grEnvShadowStateOnCars = NULL;

ssgBranch *SunAnchor = NULL;

unsigned grSkyDomeDistance = 0;

// Some private global variables.
//static int grDynamicWeather = 0;
static bool grDynamicTime = false;
static int grBackgroundType = 0;
static float grSunDeclination = 0.0f;
static float grMoonDeclination = 0.0f;

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
	
	/* If no dynamic sky is set, or the track skyversion doesn't support dynamic sky,
	   we set up a static one */
	if (!grSkyDomeDistance || grTrack->skyversion < 1) 
	{
		GfLogInfo("Setting up static sky\n");

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
		
		/*if (!SUN) 
		  {
		  ssgaLensFlare      *sun_obj      = NULL ;
		  sun_obj  = new ssgaLensFlare () ;
		  TheSun   = new ssgTransform ;
		  TheSun-> setTransform( solposn ) ;
		  TheSun-> addKid( sun_obj  ) ;
		  SunAnchor-> addKid(TheSun) ;
		  }*/
	}
	
	/** If dynamic sky is needed, we create the Sun, the Moon, some stars and the clouds */
	else 
	{
		GfLogInfo("Setting up dynamic sky :\n");

		static const int CloudsTextureIndices[TR_CLOUDS_FULL+1] = { 1, 3, 5, 7, 8 };
		static const int NCloudsTextureIndices = sizeof(CloudsTextureIndices) / sizeof(int);

		int div = 80000 / grSkyDomeDistance; //grSkyDomeDistance > 0 so cannot div0
		ssgSetNearFar(1, grSkyDomeDistance);

		// Determine time of day (seconds since 00:00).
		const int timeOfDay = (int)grTrack->local.timeofday;

		// Add random stars when relevant.
		/*if (timeOfDay < 7 * 3600 || timeOfDay > 17 * 3600)
		{
			if (timeOfDay < 5 * 3600 || timeOfDay > 19 * 3600)
				NStars = NMaxStars;
			else
				NStars = NMaxStars / 2;
		}
		else
			NStars = 0;*/
		NStars = NMaxStars; // Stars must be initialized same in day not only in the night
			
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
        TheCelestBodies[eCBSun] = TheSky->addBody(NULL, "data/textures/halo.rgba", (2500 / div), grSkyDomeDistance, true);
		GLfloat sunAscension = grTrack->local.sunascension;
		grSunDeclination = (float)(15 * (double)timeOfDay / 3600 - 90.0);

		TheCelestBodies[eCBSun]->setDeclination ( DEG2RAD(grSunDeclination));
		TheCelestBodies[eCBSun]->setRightAscension ( sunAscension );

		GfLogInfo("  Sun : time of day = %02d:%02d:%02d (declination = %.1f deg), "
		          "ascension = %.1f deg\n", 
				  timeOfDay / 3600, (timeOfDay % 3600) / 60, timeOfDay % 60,
				  grSunDeclination, RAD2DEG(sunAscension));

		// Add the Moon
		TheCelestBodies[eCBMoon] = TheSky->addBody ( "data/textures/moon.rgba",NULL, (2500 / div), grSkyDomeDistance);
		if ( grSunDeclination < 0 )
			grMoonDeclination = 3.0 + (rand() % 25);
		else
			grMoonDeclination = -(rand() % 45) + 10;

		const float moonAscension = (float)(rand() % 240);
		
		TheCelestBodies[eCBMoon]->setDeclination ( DEG2RAD(grMoonDeclination) );
		TheCelestBodies[eCBMoon]->setRightAscension ( DEG2RAD(moonAscension) );

		GfLogInfo("  Moon : declination = %.1f deg, ascension = %.1f deg\n",
				  grMoonDeclination, moonAscension);

		// Add the cloud layers
		if (grTrack->local.clouds < 0)
			grTrack->local.clouds = 0;
		else if(grTrack->local.clouds >= NCloudsTextureIndices)
			grTrack->local.clouds = NCloudsTextureIndices - 1;
		const int cloudsTextureIndex = CloudsTextureIndices[grTrack->local.clouds];

		cGrCloudLayer *cloudLayers[NMaxCloudLayers];
		snprintf(buf, sizeof(buf), "data/textures/scattered%d.rgba", cloudsTextureIndex);
		if (grTrack->local.rain > 0) // TODO: More/different cloud layers for each rain strength value ?
			cloudLayers[0] = TheSky->addCloud(buf, grSkyDomeDistance, 650, 400 * div, 400 * div);
		else
			cloudLayers[0] = TheSky->addCloud(buf, grSkyDomeDistance, 2550, 100 * div, 100 * div);
		cloudLayers[0]->setSpeed(60);
		cloudLayers[0]->setDirection(45);
		GfLogInfo("  Cloud cover : 1 layer, texture=%s, speed=60, direction=45\n", buf);
    
		// Set up the light source to the Sun position?
    	sgVec3 solposn;
    	sgSetVec3(solposn, 0, 0, 0);
				  
    	light->setPosition(solposn);
		static ulClock ck;
		double dt = ck.getDeltaTime();
    	TheSky->repositionFlat(solposn, 0, dt);    

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
		
		TheSky->modifyVisibility( visibility, (float)dt);
    
		//Setup overall light level according to rain if any
		double sol_angle = TheCelestBodies[eCBSun]->getAngle();
		double sky_brightness = (1.0 + cos(sol_angle)) / 2.0;
		double scene_brightness = pow(sky_brightness, 0.5);
        
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
    
		SkyColor[0] = BaseSkyColor[0] * (float)sky_brightness;
		SkyColor[1] = BaseSkyColor[1] * (float)sky_brightness;
		SkyColor[2] = BaseSkyColor[2] * (float)sky_brightness;
		SkyColor[3] = BaseSkyColor[3];
		
		CloudsColor[0] = FogColor[0] = BaseFogColor[0] * (float)sky_brightness;
		CloudsColor[1] = FogColor[1] = BaseFogColor[1] * (float)sky_brightness;
		CloudsColor[2] = FogColor[2] = BaseFogColor[2] * (float)sky_brightness;
		CloudsColor[3] = FogColor[3] = BaseFogColor[3];
	
		TheSky->repaint(SkyColor, FogColor, CloudsColor, sol_angle, NPlanets, APlanetsData, NStars, AStarsData);
	
		sgCoord solpos;
		TheCelestBodies[eCBSun]->getPosition(&solpos);
		light->setPosition(solpos.xyz);	
	
		SceneAmbiant[0] = BaseAmbiant[0] * (float)scene_brightness;
		SceneAmbiant[1] = BaseAmbiant[1] * (float)scene_brightness;
		SceneAmbiant[2] = BaseAmbiant[2] * (float)scene_brightness;
		SceneAmbiant[3] = 1.0;
	
		SceneDiffuse[0] = BaseDiffuse[0] * (float)scene_brightness;
		SceneDiffuse[1] = BaseDiffuse[1] * (float)scene_brightness;
		SceneDiffuse[2] = BaseDiffuse[2] * (float)scene_brightness;
		SceneDiffuse[3] = 1.0;
	
		SceneSpecular[0] = BaseSpecular[0] * (float)scene_brightness;
		SceneSpecular[1] = BaseSpecular[1] * (float)scene_brightness;
		SceneSpecular[2] = BaseSpecular[2] * (float)scene_brightness;
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

	// Load graphic options for the background.
	grSkyDomeDistance =
		(unsigned)GfParmGetNum(grHandle, GR_SCT_GRAPHIC, GR_ATT_SKYDOMEDISTANCE, (char*)NULL, grSkyDomeDistance);
	if (grSkyDomeDistance < grSkyDomeDistThresh)
		grSkyDomeDistance = grSkyDomeDistThresh; // If user enabled it (>0), must be at least the threshold.

	grDynamicTime = grSkyDomeDistance > 0 && strcmp(GfParmGetStr(grHandle, GR_SCT_GRAPHIC, GR_ATT_DYNAMICTIME, GR_ATT_DYNAMICTIME_DISABLED), GR_ATT_DYNAMICTIME_ENABLED) == 0; 

	//grDynamicWeather = GfParmGetNum(grHandle, GR_SCT_GRAPHIC, GR_ATT_grDynamicWeather, (char*)NULL, grDynamicWeather);
			
	GfLogInfo("Graphic options : Sky dome distance = %d m, dynamic time = %s\n",
			  grSkyDomeDistance, grDynamicTime ? "true" : "false");


	snprintf(buf, sizeof(buf), "tracks/%s/%s;data/img;data/textures;.",
			grTrack->category, grTrack->internalname);
	grFilePath = buf;
	grGammaValue = 1.8;
	grMipMap = 0;
	bool UseEnvPng = false;   // Avoid crash with missing env.rgb files (i.e. Wheel-1)
	bool DoNotUseEnv = false; // Avoid crash with missing env.png

	const tTrackGraphicInfo *graphic = &grTrack->graphic;
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

	/* Lens Flares */
	SunAnchor = new ssgBranch;
	TheScene->addKid(SunAnchor);

	//Environment Mapping Settings
	grEnvSelector = new ssgStateSelector(graphic->envnb);
	for (i = 0; i < graphic->envnb; i++) {
		GfLogTrace("Loading %d Environment Mapping Image %s\n", i, graphic->env[i]);
		envst = (ssgSimpleState*)grSsgLoadTexState(graphic->env[i]);
        // Avoid crash with missing env.rgb files (i.e. Wheel-1)
		if (envst == NULL) {
			GfLogWarning("Failed : trying fallback env.png\n");
			envst = (ssgSimpleState*)grSsgLoadTexState("env.png");
			if (envst == NULL) {
				GfLogError("No usable Environment Mapping Image for #%d : stop displaying graphics!\n", i);
				DoNotUseEnv = true;
				break;
			}
			else
				UseEnvPng = true;
		}
		envst->enable(GL_BLEND);
		grEnvSelector->setStep(i, envst);
		envst->deRef();
	}//for i

	grEnvSelector->selectStep(0); //mandatory !!!
  
	// Avoid crash with missing env.rgb files (i.e. Wheel-1)
	if (UseEnvPng)
		grEnvState=(grMultiTexState*)grSsgEnvTexState("env.png");
	else {
		if (DoNotUseEnv)
			GfLogError("No env.png found!\n");
		else
			grEnvState=(grMultiTexState*)grSsgEnvTexState(graphic->env[0]);
	}
	grEnvShadowState=(grMultiTexState*)grSsgEnvTexState("envshadow.png");
	if (grEnvShadowState == NULL) {
		ulSetError ( UL_WARNING, "grscene:initBackground Failed to open envshadow.png for reading") ;
		ulSetError ( UL_WARNING, "        mandatory for top env mapping (should be in the .xml !!) ") ;
		ulSetError ( UL_WARNING, "        copy the envshadow.png from 'chemisay' to the track you selected ") ;
		ulSetError ( UL_WARNING, "        (sorry for exiting, but it would have actually crashed).") ;
		GfScrShutdown();
		exit(-1);
	}//if grEnvShadowState
    
	grEnvShadowStateOnCars = (grMultiTexState*)grSsgEnvTexState("shadow2.png");
	if(grEnvShadowStateOnCars == NULL)
		grEnvShadowStateOnCars = (grMultiTexState*)grSsgEnvTexState("shadow2.rgb");
  
	if(grEnvShadowStateOnCars == NULL) {
		ulSetError ( UL_WARNING, "grscene:initBackground Failed to open shadow2.png/rgb for reading") ;
		ulSetError ( UL_WARNING, "        no shadow mapping on cars for this track ") ;
	}//if grEnvShadowStateOnCars
}//grLoadBackground

void
grPreDrawSky(tSituation* s, float fogStart, float fogEnd) 
{
	static const double m_log01 = -log( 0.01 );
	static const double sqrt_m_log01 = sqrt( m_log01 );

	if (grSkyDomeDistance && grTrack->skyversion > 0) 
	{
		// Commenting these 2 lines fixes #266 (Splitscreen completely white with Skydome enabled)
		//glClearColor(FogColor[0], FogColor[1], FogColor[2], FogColor[3]);
		//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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
	{
		TheSky->postDraw(grSkyDomeDistance);
	}
}//grPostDrawSky

// Update the sky when time changes
void
grUpdateSky(double currentTime)
{
	if (!grDynamicTime)
		return;

	// Update last update time.
	static int lastchecked = -100;
	static double lastTime = -10.0f;
	static ulClock ck ;
	ck.update () ;
	double dt1 = ck.getDeltaTime();
	if( currentTime < lastTime ) 
	{
		lastchecked = -100;
		lastTime = currentTime;
		return;
	}

	// Update sun position (?)
	int current = (int)floor( ( currentTime + 10.0f ) / 60.0f );
	
	sgVec3 solposn;
	//sgSetVec3(solposn, 0, 0, 0);				  
	//ssgGetLight(0)->setPosition(solposn);
	//TheSky->repositionFlat(solposn, 0, dt1 );
	TheSky->repositionFlat(solposn , 0, dt1);
	//TheSky->modifyVisibility(solposn, (float)dt1);
	lastTime = currentTime;
	if( current == lastchecked )
		return;
	lastchecked = current;

	GfLogDebug("Updating sky (dynamic time)\n");
	
	// Update sun position
	grSunDeclination += 0.25f; // TODO: Is this delta value realistic ?
	if (grSunDeclination >= 360.0f)
		grSunDeclination = 0.0f;
	
	TheCelestBodies[eCBSun]->setDeclination ( DEG2RAD(grSunDeclination) );

	// Update moon position
	grMoonDeclination += 0.25f; // TODO: Is this delta value realistic ?
	if (grMoonDeclination >= 360.0f)
		grMoonDeclination = 0.0f;
	
	TheCelestBodies[eCBMoon]->setDeclination ( DEG2RAD(grMoonDeclination) );

	// Update scene color and light
	double sol_angle = TheCelestBodies[eCBSun]->getAngle();
	double sky_brightness = (1.0 + cos(sol_angle)) / 2.0;
	double scene_brightness = pow(sky_brightness, 0.5);
	
	SkyColor[0] = BaseSkyColor[0] * (float)sky_brightness;
	SkyColor[1] = BaseSkyColor[1] * (float)sky_brightness;
	SkyColor[2] = BaseSkyColor[2] * (float)sky_brightness;
	SkyColor[3] = BaseSkyColor[3];

	/* set cloud and fog color */
	CloudsColor[0] = FogColor[0] = BaseFogColor[0] * (float)sky_brightness;
	CloudsColor[1] = FogColor[1] = BaseFogColor[1] * (float)sky_brightness;
	CloudsColor[2] = FogColor[2] = BaseFogColor[2] * (float)sky_brightness;
	CloudsColor[3] = FogColor[3] = BaseFogColor[3];

	/* repaint the sky (simply update geometrical, color, ... state, no actual redraw) */
	TheSky->repaint(SkyColor, FogColor, CloudsColor, sol_angle, NPlanets, APlanetsData, NStars, AStarsData);

	sgCoord solpos;
	TheCelestBodies[eCBSun]-> getPosition(&solpos);
	ssgGetLight(0)-> setPosition(solpos.xyz);	

	SceneAmbiant[0] = BaseAmbiant[0] * (float)scene_brightness;
	SceneAmbiant[1] = BaseAmbiant[1] * (float)scene_brightness;
	SceneAmbiant[2] = BaseAmbiant[2] * (float)scene_brightness;
	SceneAmbiant[3] = 1.0;
	
	SceneDiffuse[0] = BaseDiffuse[0] * (float)scene_brightness;
	SceneDiffuse[1] = BaseDiffuse[1] * (float)scene_brightness;
	SceneDiffuse[2] = BaseDiffuse[2] * (float)scene_brightness;
	SceneDiffuse[3] = 1.0;
	
	SceneSpecular[0] = BaseSpecular[0] * (float)scene_brightness;
	SceneSpecular[1] = BaseSpecular[1] * (float)scene_brightness;
	SceneSpecular[2] = BaseSpecular[2] * (float)scene_brightness;
	SceneSpecular[3] = 1.0;
}//grUpdateSky

void
grShutdownBackground(void)
{
	if (TheSky) {
		delete TheSky;
		TheSky = 0;
	}
	
	if (TheSun) {
		delete TheSun;
		TheSun = 0;
	}
	
	if (grEnvState) {
		ssgDeRefDelete(grEnvState);
		grEnvState = NULL;
	}
	
	if (grEnvShadowState) {
		ssgDeRefDelete(grEnvShadowState);
		grEnvShadowState = NULL;
	}
	
	if (grEnvShadowStateOnCars) {
		ssgDeRefDelete(grEnvShadowStateOnCars);
		grEnvShadowStateOnCars = NULL;
	}
	
	if(grEnvSelector) {
		delete grEnvSelector;
		grEnvSelector = NULL;
	}

}//grShutdownBackground
