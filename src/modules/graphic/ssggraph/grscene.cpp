/***************************************************************************

    file                 : grscene.cpp
    created              : Mon Aug 21 20:13:56 CEST 2000
    copyright            : (C) 2000 by Eric Espie
    email                : torcs@free.fr
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


#ifdef WIN32
#include <windows.h>
#include <GL/gl.h>
#include <GL/glext.h>
#endif

#include <plib/ssg.h>
#include <plib/ssgAux.h>
#include <glfeatures.h>	//gluXXX
#include <robottools.h>	//RtXXX()
#include <portability.h> // snprintf

#include "grscene.h"
#include "grmain.h"
#include "grcam.h"	//cGrBackgroundCam
#include "grloadac.h"
#include "grutil.h"
#include "grssgext.h"
#include "grrain.h"
#include "grSky.h"

#define NB_BG_FACES	36	//Background faces
#define BG_DIST			1.0f

const int grSkyDomeDistThresh = 12000; // No dynamic sky below that value.

static const int NMaxStars = 1000;
static const int NMaxPlanets = 0; //No planets displayed for the moment
static const int NMaxCloudLayers = 3;
enum {eCBSun = 0, eCBMoon, NMaxCelestianBodies};	// Celestial bodies

static int NStars = 0;
static int NPlanets = 0;

int grWrldX;
int grWrldY;
int grWrldZ;
int grWrldMaxSize;
tTrack 	 *grTrack;

ssgStateSelector *grEnvSelector = NULL;
grMultiTexState	*grEnvState = NULL;
grMultiTexState	*grEnvShadowState = NULL;
grMultiTexState	*grEnvShadowStateOnCars = NULL;

int grSkyDomeDistance = 0;

ssgRoot *TheScene = NULL;
//TheScene kid order
ssgBranch *SunAnchor = NULL;
ssgBranch *LandAnchor = NULL;
ssgBranch *CarsAnchor = NULL;
ssgBranch *ShadowAnchor = NULL;
ssgBranch *PitsAnchor = NULL;
ssgBranch *SmokeAnchor = NULL;
ssgBranch *SkidAnchor = NULL;
ssgBranch *CarlightAnchor = NULL;
ssgBranch *TrackLightAnchor = NULL;
ssgBranch *ThePits = NULL;

//static int DynamicWeather = 0;
static int BackgroundType = 0;
static float grSunDeclination = 0.0f;
static float grMoonDeclination = 0.0f;
static GLuint BackgroundList = 0;
static GLuint BackgroundTex = 0;
static GLuint BackgroundList2;
static GLuint BackgroundTex2;

static ssgRoot *TheBackground = NULL;
static grSky *Sky = NULL;
static ssgTransform *TheSun = NULL;

static grCelestialBody *bodies[NMaxCelestianBodies] = { NULL };
static grMoon *Moon = NULL;

static sgdVec3 *AStarsData = NULL;
static sgdVec3 *APlanetsData = NULL;

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
static sgVec4 cloud_color;

static sgVec4 scene_ambiant;
static sgVec4 scene_diffuse;
static sgVec4 scene_specular;

//Utility
static const double m_log01 = -log( 0.01 );
static const double sqrt_m_log01 = sqrt( m_log01 );
static char buf[1024];
static void initBackground(void);


//Must have
int preScene(ssgEntity *e) { return TRUE; }


/**
 * grInitScene
 * Initialises a scene (ie a new view).
 * 
 * @return 0 if OK, -1 if something failed
 */
int
grInitScene(void)
{
	void *hndl = grTrackHandle;
	ssgLight *light = ssgGetLight(0);
	
	if(grHandle == NULL) 
	{
		sprintf(buf, "%s%s", GetLocalDir(), GR_PARAM_FILE);
		grHandle = GfParmReadFile(buf, GFPARM_RMODE_STD | GFPARM_RMODE_CREAT);
	}//if grHandle

	/* If no dynamic sky is set, or the track skyversion doesn't support dynamic sky,
	   we set up a static one */
	if (grSkyDomeDistance < grSkyDomeDistThresh || grTrack->skyversion < 1) 
	{
		GfLogInfo("Setting up static sky\n");

		GLfloat mat_specular[]   = {0.3, 0.3, 0.3, 1.0};
		GLfloat mat_shininess[]  = {50.0};
		GLfloat light_position[] = {0, 0, 200, 0.0};
		GLfloat lmodel_ambient[] = {0.2, 0.2, 0.2, 1.0};
		GLfloat lmodel_diffuse[] = {0.8, 0.8, 0.8, 1.0};
		GLfloat fog_clr[]        = {1.0, 1.0, 1.0, 0.5};

		mat_specular[0] = GfParmGetNum(hndl, TRK_SECT_GRAPH, TRK_ATT_SPEC_R, NULL, mat_specular[0]);
		mat_specular[1] = GfParmGetNum(hndl, TRK_SECT_GRAPH, TRK_ATT_SPEC_G, NULL, mat_specular[1]);
		mat_specular[2] = GfParmGetNum(hndl, TRK_SECT_GRAPH, TRK_ATT_SPEC_B, NULL, mat_specular[2]);

		lmodel_ambient[0] = GfParmGetNum(hndl, TRK_SECT_GRAPH, TRK_ATT_AMBIENT_R, NULL, lmodel_ambient[0]);
		lmodel_ambient[1] = GfParmGetNum(hndl, TRK_SECT_GRAPH, TRK_ATT_AMBIENT_G, NULL, lmodel_ambient[1]);
		lmodel_ambient[2] = GfParmGetNum(hndl, TRK_SECT_GRAPH, TRK_ATT_AMBIENT_B, NULL, lmodel_ambient[2]);

		lmodel_diffuse[0] = GfParmGetNum(hndl, TRK_SECT_GRAPH, TRK_ATT_DIFFUSE_R, NULL, lmodel_diffuse[0]);
		lmodel_diffuse[1] = GfParmGetNum(hndl, TRK_SECT_GRAPH, TRK_ATT_DIFFUSE_G, NULL, lmodel_diffuse[1]);
		lmodel_diffuse[2] = GfParmGetNum(hndl, TRK_SECT_GRAPH, TRK_ATT_DIFFUSE_B, NULL, lmodel_diffuse[2]);

		mat_shininess[0] = GfParmGetNum(hndl, TRK_SECT_GRAPH, TRK_ATT_SHIN, NULL, mat_shininess[0]);

		light_position[0] = GfParmGetNum(hndl, TRK_SECT_GRAPH, TRK_ATT_LIPOS_X, NULL, light_position[0]);
		light_position[1] = GfParmGetNum(hndl, TRK_SECT_GRAPH, TRK_ATT_LIPOS_Y, NULL, light_position[1]);
		light_position[2] = GfParmGetNum(hndl, TRK_SECT_GRAPH, TRK_ATT_LIPOS_Z, NULL, light_position[2]);

		glShadeModel(GL_SMOOTH);

		light->setPosition(light_position[0],light_position[1],light_position[2]);
		light->setColour(GL_AMBIENT,lmodel_ambient);
		light->setColour(GL_DIFFUSE,lmodel_diffuse);
		light->setColour(GL_SPECULAR,mat_specular);
		light->setSpotAttenuation(0.0, 0.0, 0.0);

		sgCopyVec3 (fog_clr,  grTrack->graphic.bgColor);
		sgScaleVec3 (fog_clr, 0.8);
		glFogi(GL_FOG_MODE, GL_LINEAR);
		glFogfv(GL_FOG_COLOR, fog_clr);
		glFogf(GL_FOG_DENSITY, 0.05);
		glHint(GL_FOG_HINT, GL_DONT_CARE);

		glEnable(GL_LIGHTING);
		glEnable(GL_LIGHT0);
		glEnable(GL_DEPTH_TEST);
	}
	
	/** If dynamic sky is needed, we create the Sun, the Moon, some stars and the clouds */
	else 
	{
		GfLogInfo("Setting up dynamic sky :\n");

		static const int CloudsTextureIndices[] = { 1, 3, 5, 7, 8 };
		static const int NCloudsTextureIndices = sizeof(CloudsTextureIndices) / sizeof(int);

		//Query the time
		static ulClock ck;
		double dt = ck.getDeltaTime();

		int div = 80000 / grSkyDomeDistance;	//grSkyDomeDistance >= grSkyDomeDistThresh so cannot div0
		ssgSetNearFar(1, grSkyDomeDistance);

		// Add random stars at dawn, dusk or night.
		switch (grTrack->timeofday) 
		{
			case TR_TIME_DUSK:
			case TR_TIME_DAWN:
				NStars = NMaxStars / 2;
				break;

			case TR_TIME_NIGHT:
				NStars = NMaxStars;
				break;
					
			default:
				NStars = 0;
				break;
				
		}//switch timeofday

		if (AStarsData)
			delete AStarsData;
		AStarsData = new sgdVec3[NStars];
		for(int i= 0; i < NStars; i++) 
		{
			AStarsData[i][0] = grRandom() * SGD_PI;
			AStarsData[i][1] = grRandom() * SGD_PI;
			AStarsData[i][2] = grRandom();
		}//for i

		GfLogInfo("  Stars : %d random ones\n", NStars);
		
		//No planets
		NPlanets = 0;
		APlanetsData = NULL;

		GfLogInfo("  Planets : none\n");
		
		//Build the sky
		Sky	= new grSky;
		Sky->build(grSkyDomeDistance, grSkyDomeDistance, NPlanets, APlanetsData, NStars, AStarsData);
		
		//Add the Sun itself
        bodies[eCBSun] = Sky->addBody(NULL, "data/textures/halo.rgba", (2500 / div), grSkyDomeDistance, true);
		GLfloat sunAscension =
			(float)GfParmGetNum(hndl, TRK_SECT_GRAPH, TRK_ATT_SUN_H, (char*)NULL, 0.0f);

		switch (grTrack->timeofday) 
		{
			case TR_TIME_DAWN:
				grSunDeclination = 2.0f;
				break;
					
			case TR_TIME_MORNING:
				grSunDeclination = 30.0f;
				break;
					
			case TR_TIME_NOON:
				grSunDeclination = 85.0f;
				break;
					
			case TR_TIME_AFTERNOON:
				grSunDeclination = 135.0f;
				break;
					
			case TR_TIME_DUSK:
				grSunDeclination = 178.0f;
				break;
					
			case TR_TIME_NIGHT:
				grSunDeclination = -90.0f;
				break;
					
			case TR_TIME_DYNAMIC:
			{
				const float sunHeight =
					(float)GfParmGetNum(hndl, TRK_SECT_GRAPH, TRK_ATT_HOUR, (char*)NULL, 0.0f);
				GfLogDebug("User defined sun height = %f\n", sunHeight);
				grSunDeclination = ((sunHeight / 3600) * 15.0f) - 90.0f;
				break;
			}

			default:
				grSunDeclination = 135.0f;
				GfLogError("Unsupported value %d for grTrack->timeofday (assuming afternoon)\n",
						   grTrack->timeofday);
				break;
				
		}//switch timeofday

		bodies[eCBSun]->setDeclination ( grSunDeclination * SGD_DEGREES_TO_RADIANS);
		bodies[eCBSun]->setRightAscension ( sunAscension * SGD_DEGREES_TO_RADIANS);

		GfLogInfo("  Sun : declination = %4.1f deg, ascension = %4.1f deg\n",
				  grSunDeclination, sunAscension);

		//Add the Moon
		bodies[eCBMoon] = Sky->addBody ( "data/textures/moon.rgba",NULL, (2500 / div), grSkyDomeDistance);
		if ( grSunDeclination < 0 )
			grMoonDeclination = 3.0 + (rand() % 25);
		else
			grMoonDeclination = -(rand() % 45) + 10;

		const float moonAscension = (float)(rand() % 240);
		
		bodies[eCBMoon]->setDeclination (grMoonDeclination * SGD_DEGREES_TO_RADIANS );
		bodies[eCBMoon]->setRightAscension ( moonAscension * SGD_DEGREES_TO_RADIANS );

		GfLogInfo("  Moon : declination = %4.1f deg, ascension = %4.1f deg\n",
				  grMoonDeclination, moonAscension);

		// Add clouds
		if (grTrack->clouds < 0)
			grTrack->clouds = 0;
		else if(grTrack->clouds >= NCloudsTextureIndices)
			grTrack->clouds = NCloudsTextureIndices - 1;
		const int cloudsTextureIndex = CloudsTextureIndices[grTrack->clouds];

		grCloudLayer *cloudLayers[NMaxCloudLayers] = { NULL };
		snprintf(buf, sizeof(buf), "data/textures/scattered%d.rgba", cloudsTextureIndex);
		if (grTrack->rain > 0) // TODO: More/different cloud layers for each rain strength value ?
			cloudLayers[0] = Sky->addCloud(buf, grSkyDomeDistance, 650, 400 * div, 400 * div);
		else
			cloudLayers[0] = Sky->addCloud(buf, grSkyDomeDistance, 2550, 100 * div, 100 * div);
		cloudLayers[0]->setSpeed(60);
		cloudLayers[0]->setDirection(45);
		GfLogInfo("  Cloud cover : 1 layer, texture=%s, speed=60, direction=45\n", buf);
    
		//Set up the light source to the Sun position?
    	sgVec3 solposn;
    	sgSetVec3(solposn, 0, 0, 0);
				  
    	ssgGetLight(0)->setPosition(solposn);
    	Sky->repositionFlat(solposn, 0, dt);    

		//Setup visibility according to rain if any
		float visibility = 0.0f;
		switch (grTrack->rain)	
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
			case TR_RAIN_RANDOM:
				// Should never happens here (only used in the menu side).
			default:
				GfLogWarning("Unsupported rain strength value %d (assuming none)", grTrack->rain);
				visibility = 0.0f;
				break;
		}//switch Rain
		
		Sky->modifyVisibility( visibility, (float)dt);
    
		//Setup overall light level according to rain if any
		double sol_angle = bodies[eCBSun]->getAngle();
		double sky_brightness = (1.0 + cos(sol_angle)) / 2.0;
		double scene_brightness = pow(sky_brightness, 0.5);
        
		sky_color[0] = base_sky_color[0] * (float)sky_brightness;
		sky_color[1] = base_sky_color[1] * (float)sky_brightness;
		sky_color[2] = base_sky_color[2] * (float)sky_brightness;
		sky_color[3] = base_sky_color[3];
		
		if (grTrack->rain > 0) // TODO: Different values for each rain strength value ?
		{
			base_fog_color[0] = 0.40f;
			base_fog_color[1] = 0.43f;
			base_fog_color[2] = 0.45f;

			scene_brightness = scene_brightness / 2.0f;			
		}
		else
		{
			base_fog_color[0] = 0.84f;
			base_fog_color[1] = 0.87f;
			base_fog_color[2] = 1.00f;

			scene_brightness = scene_brightness;
		}
    
		cloud_color[0] = fog_color[0] = base_fog_color[0] * (float)sky_brightness;
		cloud_color[1] = fog_color[1] = base_fog_color[1] * (float)sky_brightness;
		cloud_color[2] = fog_color[2] = base_fog_color[2] * (float)sky_brightness;
		cloud_color[3] = fog_color[3] = base_fog_color[3];
	
		Sky->repaint(sky_color, fog_color, cloud_color, sol_angle, NPlanets, APlanetsData, NStars, AStarsData);
	
		sgCoord solpos;
		bodies[eCBSun]-> getPosition(&solpos);
		ssgGetLight(0)-> setPosition(solpos.xyz);	
	
		scene_ambiant[0] = base_ambiant[0] * (float)scene_brightness;
		scene_ambiant[1] = base_ambiant[1] * (float)scene_brightness;
		scene_ambiant[2] = base_ambiant[2] * (float)scene_brightness;
		scene_ambiant[3] = 1.0;
	
		scene_diffuse[0] = base_diffuse[0] * (float)scene_brightness;
		scene_diffuse[1] = base_diffuse[1] * (float)scene_brightness;
		scene_diffuse[2] = base_diffuse[2] * (float)scene_brightness;
		scene_diffuse[3] = 1.0;
	
		scene_specular[0] = base_specular[0] * (float)scene_brightness;
		scene_specular[1] = base_specular[1] * (float)scene_brightness;
		scene_specular[2] = base_specular[2] * (float)scene_brightness;
		scene_specular[3] = 1.0;
	
		glLightModelfv( GL_LIGHT_MODEL_AMBIENT, black);
		ssgGetLight(0) -> setColour( GL_AMBIENT, scene_ambiant);
		ssgGetLight(0) -> setColour( GL_DIFFUSE, scene_diffuse);
		ssgGetLight(0) -> setColour( GL_SPECULAR, scene_specular);
	}//else grSkyDomeDistance 

	// Initialize rain renderer
	if (grTrack->rain > 0)
	{
		grRain.initialize(grTrack->rain);
	}
	
	/*if (!SUN) 
	{
	ssgaLensFlare      *sun_obj      = NULL ;
	sun_obj  = new ssgaLensFlare () ;
	TheSun   = new ssgTransform ;
	TheSun-> setTransform( solposn ) ;
	TheSun-> addKid( sun_obj  ) ;
	SunAnchor-> addKid(TheSun) ;
	}*/

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
}//grInitScene


static grssgLoaderOptions options(/*bDoMipMap*/true);

int
grLoadScene(tTrack *track)
{
	void		*hndl = grTrackHandle;
	const char		*acname;
	ssgEntity		*desc;

	if (maxTextureUnits == 0) {
		InitMultiTex();
	}

	grssgSetCurrentOptions(&options);

	if(grHandle == NULL) {
		sprintf(buf, "%s%s", GetLocalDir(), GR_PARAM_FILE);
		grHandle = GfParmReadFile(buf, GFPARM_RMODE_STD | GFPARM_RMODE_REREAD);
	}//if grHandle

	grSkyDomeDistance =
		GfParmGetNum(grHandle, GR_SCT_GRAPHIC, GR_ATT_SKYDOMEDISTANCE, (char*)NULL, grSkyDomeDistance);
	if (grSkyDomeDistance > 0 && grSkyDomeDistance < grSkyDomeDistThresh)
		grSkyDomeDistance = grSkyDomeDistThresh; // If user enabled it (>0), must be over the threshold.
	//DynamicWeather = GfParmGetNum(grHandle, GR_SCT_GRAPHIC, GR_ATT_DYNAMICWEATHER, (char*)NULL, DynamicWeather);
			
	grTrack = track;
	TheScene = new ssgRoot;

	/* Landscape */
	LandAnchor = new ssgBranch;
	TheScene->addKid(LandAnchor);

	/* Pit stops walls */
	PitsAnchor = new ssgBranch;
	TheScene->addKid(PitsAnchor);

	/* Skid Marks */
	SkidAnchor = new ssgBranch;
	TheScene->addKid(SkidAnchor);

	/* Car shadows */
	ShadowAnchor = new ssgBranch;
	TheScene->addKid(ShadowAnchor);

	/* Car lights */
	CarlightAnchor = new ssgBranch;
	TheScene->addKid(CarlightAnchor);

	/* Cars */
	CarsAnchor = new ssgBranch;
	TheScene->addKid(CarsAnchor);

	/* Smoke */
	SmokeAnchor = new ssgBranch;
	TheScene->addKid(SmokeAnchor);

	/* Lens Flares */
	SunAnchor = new ssgBranch;
	TheScene->addKid(SunAnchor);

	/* Anchor for track lights */
	TrackLightAnchor = new ssgBranch;
	TheScene->addKid(TrackLightAnchor);

	initBackground();
    
	grWrldX = (int)(track->max.x - track->min.x + 1);
	grWrldY = (int)(track->max.y - track->min.y + 1);
	grWrldZ = (int)(track->max.z - track->min.z + 1);
	grWrldMaxSize = (int)(MAX(MAX(grWrldX, grWrldY), grWrldZ));

	acname = GfParmGetStr(hndl, TRK_SECT_GRAPH, TRK_ATT_3DDESC, "track.ac");
	if (strlen(acname) == 0) 
	{
		GfLogError("No specified track 3D model file\n");
		return -1;
	}

	sprintf(buf, "tracks/%s/%s;data/textures;data/img;.", grTrack->category, grTrack->internalname);
	ssgTexturePath(buf);
	sprintf(buf, "tracks/%s/%s", grTrack->category, grTrack->internalname);
	ssgModelPath(buf);

	desc = grssgLoadAC3D(acname, NULL);
	LandAnchor->addKid(desc);

	return 0;
}//grLoadScene

void
grDrawScene(float carSpeed, tSituation *s) 
{
	const bool bDrawSky = (grSkyDomeDistance >= grSkyDomeDistThresh && grTrack->skyversion > 0);
	if (bDrawSky) 
	{
		if (grTrack->timeofday == TR_TIME_DYNAMIC) {
			grUpdateTime(s);
		}
		
		glClearColor(fog_color[0], fog_color[1], fog_color[2], fog_color[3]);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		GLfloat fog_exp2_density = (float)sqrt_m_log01 / Sky->getVisibility();
		glEnable(GL_FOG);
		glFogi(GL_FOG_MODE, GL_EXP2);
		glFogfv(GL_FOG_COLOR, fog_color);
		glFogf(GL_FOG_DENSITY, fog_exp2_density);
		glHint(GL_FOG_HINT, GL_DONT_CARE);

		ssgGetLight(0)->setColour(GL_DIFFUSE, white);

		Sky->preDraw();

		glLightModelfv( GL_LIGHT_MODEL_AMBIENT, black);
		ssgGetLight(0)->setColour(GL_AMBIENT, scene_ambiant);
		ssgGetLight(0)->setColour(GL_DIFFUSE, scene_diffuse);
		ssgGetLight(0)->setColour(GL_SPECULAR, scene_specular);
	}
	
	TRACE_GL("refresh: ssgCullAndDraw start");
	ssgCullAndDraw(TheScene);
	TRACE_GL("refresh: ssgCullAndDraw");
	
	if (bDrawSky) 
	{
		Sky->postDraw(grSkyDomeDistance);
	}
	
	if (grTrack->rain > 0)
	{
		grRain.drawPrecipitation(grTrack->rain, 1.0, 0.0, 0.0, 0.0, carSpeed);
	}
	
}//grDrawScene


void
grShutdownScene(void)
{
	if (TheScene) {
		delete TheScene;
		TheScene = 0;
	}

	if (Sky) {
		delete Sky;
		Sky = 0;
	}
		
	if (BackgroundTex) {
		glDeleteTextures(1, &BackgroundTex);
		BackgroundTex = 0;
	}

	if (BackgroundList) {
		glDeleteLists(BackgroundList, 1);
		BackgroundList = 0;
	}

	if (BackgroundType > 2) {
		glDeleteTextures(1, &BackgroundTex2);
		glDeleteLists(BackgroundList2, 1);
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

	options.endLoad();
}//grShutdownScene


static void
initBackground(void) {
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

	sprintf(buf, "tracks/%s/%s;data/img;data/textures;.",
		grTrack->category, grTrack->internalname);
	grFilePath = buf;
	grGammaValue = 1.8;
	grMipMap = 0;
	bool UseEnvPng = false;   // Avoid chrash with missing env.rgb files (i.e. Wheel-1)
	bool DoNotUseEnv = false; // Avoid chrash with missing env.png

	const tTrackGraphicInfo *graphic = &grTrack->graphic;
	glClearColor(graphic->bgColor[0], graphic->bgColor[1], graphic->bgColor[2], 1.0);
	BackgroundTex = BackgroundTex2 = 0;

	TheBackground = new ssgRoot();
	clr[0] = clr[1] = clr[2] = 1.0;
	clr[3] = 1.0;
	nrm[0] = nrm[2] = 0.0;
	nrm[1] = 1.0;

	z1 = -0.5;
	z2 = 1.0;
	
	BackgroundType = graphic->bgtype;
	switch (BackgroundType) {
		case 0:
			bg_vtx = new ssgVertexArray(NB_BG_FACES + 1);
			bg_tex = new ssgTexCoordArray(NB_BG_FACES + 1);
			bg_clr = new ssgColourArray(1);
			bg_nrm = new ssgNormalArray(1);
			bg_clr->add(clr);
			bg_nrm->add(nrm);
	
			for (i = 0; i < NB_BG_FACES + 1; i++) {
				alpha = (float)i * 2 * PI / (float)NB_BG_FACES;
				texLen = (float)i / (float)NB_BG_FACES;

				x = BG_DIST * cos(alpha);
				y = BG_DIST * sin(alpha);
	    
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

    case 2:
			bg_vtx = new ssgVertexArray(NB_BG_FACES + 1);
			bg_tex = new ssgTexCoordArray(NB_BG_FACES + 1);
			bg_clr = new ssgColourArray(1);
			bg_nrm = new ssgNormalArray(1);
			bg_clr->add(clr);
			bg_nrm->add(nrm);

			for (i = 0; i < NB_BG_FACES / 4 + 1; i++) {
				alpha = (float)i * 2 * PI / (float)NB_BG_FACES;
				texLen = (float)i / (float)NB_BG_FACES;
	    
				x = BG_DIST * cos(alpha);
				y = BG_DIST * sin(alpha);
	    
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


			bg_vtx = new ssgVertexArray(NB_BG_FACES + 1);
			bg_tex = new ssgTexCoordArray(NB_BG_FACES + 1);
			bg_clr = new ssgColourArray(1);
			bg_nrm = new ssgNormalArray(1);
			bg_clr->add(clr);
			bg_nrm->add(nrm);

			for (i = NB_BG_FACES/4; i < NB_BG_FACES / 2 + 1; i++) {
				alpha = (float)i * 2 * PI / (float)NB_BG_FACES;
				texLen = (float)i / (float)NB_BG_FACES;
	    
				x = BG_DIST * cos(alpha);
				y = BG_DIST * sin(alpha);
				
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


			bg_vtx = new ssgVertexArray(NB_BG_FACES + 1);
			bg_tex = new ssgTexCoordArray(NB_BG_FACES + 1);
			bg_clr = new ssgColourArray(1);
			bg_nrm = new ssgNormalArray(1);

			bg_clr->add(clr);
			bg_nrm->add(nrm);

			for (i = NB_BG_FACES / 2; i < 3 * NB_BG_FACES / 4 + 1; i++) {
				alpha = (float)i * 2 * PI / (float)NB_BG_FACES;
				texLen = (float)i / (float)NB_BG_FACES;
	    
				x = BG_DIST * cos(alpha);
				y = BG_DIST * sin(alpha);
	    
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


			bg_vtx = new ssgVertexArray(NB_BG_FACES + 1);
			bg_tex = new ssgTexCoordArray(NB_BG_FACES + 1);
			bg_clr = new ssgColourArray(1);
			bg_nrm = new ssgNormalArray(1);

			bg_clr->add(clr);
			bg_nrm->add(nrm);

			for(i = 3 * NB_BG_FACES / 4; i < NB_BG_FACES + 1; i++) {
				alpha = (float)i * 2 * PI / (float)NB_BG_FACES;
				texLen = (float)i / (float)NB_BG_FACES;
				
				x = BG_DIST * cos(alpha);
				y = BG_DIST * sin(alpha);
				
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

  case 4:
		z1 = -1.0;
		z2 = 1.0;

		bg_vtx = new ssgVertexArray(NB_BG_FACES + 1);
		bg_tex = new ssgTexCoordArray(NB_BG_FACES + 1);
		bg_clr = new ssgColourArray(1);
		bg_nrm = new ssgNormalArray(1);
		bg_clr->add(clr);
		bg_nrm->add(nrm);

		for (i = 0; i < NB_BG_FACES + 1; i++) {
	    alpha = (double)i * 2 * PI / (double)NB_BG_FACES;
	    texLen = 1.0 - (float)i / (float)NB_BG_FACES;
	    
	    x = BG_DIST * cos(alpha);
	    y = BG_DIST * sin(alpha);
	    
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
		break;
	}//switch BackgroundType

	//Environment Mapping Settings
	grEnvSelector = new ssgStateSelector(graphic->envnb);
	for (i = 0; i < graphic->envnb; i++) {
		GfLogTrace("Loading %d Environment Mapping Image %s\n", i, graphic->env[i]);
		envst = (ssgSimpleState*)grSsgLoadTexState(graphic->env[i]);
        // Avoid chrash with missing env.rgb files (i.e. Wheel-1)
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
  // Avoid chrash with missing env.rgb files (i.e. Wheel-1)
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
}//initBackground


void
grDrawBackground(cGrCamera *cam, cGrBackgroundCam *bgCam)
{
	TRACE_GL("grDrawBackground: ssgCullAndDraw start");

	bgCam->update(cam);
	bgCam->action();
	ssgCullAndDraw(TheBackground);

	TRACE_GL("grDrawBackground: ssgCullAndDraw");
}//grDrawBackground


void
grCustomizePits(void)
{
	ThePits = new ssgBranch();
	PitsAnchor->addKid(ThePits);

	tTrackPitInfo *pits = &(grTrack->pits);
	/* draw the pit identification */

	switch (pits->type) {
		case TR_PIT_ON_TRACK_SIDE:
			for(int i = 0; i < pits->nMaxPits; i++) {
				//GfLogDebug("Pit Nbr: %d\n", i);        
				ssgVertexArray *pit_vtx = new ssgVertexArray(4);
				ssgTexCoordArray *pit_tex = new ssgTexCoordArray(4);
				ssgColourArray *pit_clr = new ssgColourArray(1);
				ssgNormalArray *pit_nrm = new ssgNormalArray(1);
			
				sgVec4 clr = {0, 0, 0, 1};
				pit_clr->add(clr);
		
				std::string strLogoFileName("logo"); // Default driver logo file name (pit door).

				if (pits->driversPits[i].car[0]) {
					
					// If we have more than one car in the pit use the team pit logo of driver 0. 
					snprintf(buf, sizeof(buf),
							 "%sdrivers/%s/%d;%sdrivers/%s;drivers/%s/%d;drivers/%s;data/textures",
							 GetLocalDir(),
							 pits->driversPits[i].car[0]->_modName,
							 pits->driversPits[i].car[0]->_driverIndex,
							 GetLocalDir(),
							 pits->driversPits[i].car[0]->_modName,
							 pits->driversPits[i].car[0]->_modName,
							 pits->driversPits[i].car[0]->_driverIndex,
							 pits->driversPits[i].car[0]->_modName);

					// If a custom skin was selected, and it can apply to the pit door,
					// update the logo file name accordingly
					if (strlen(pits->driversPits[i].car[0]->_skinName) != 0
						&& pits->driversPits[i].car[0]->_skinTargets & RM_CAR_SKIN_TARGET_PIT_DOOR)
					{
						strLogoFileName += '-';
						strLogoFileName += pits->driversPits[i].car[0]->_skinName;
						GfLogTrace("Using skinned pit door logo %s\n", strLogoFileName.c_str());
					}
					
				} else {
					snprintf(buf, sizeof(buf), "data/textures");
				}//if pits->driverPits[i].car[0]

				// Load logo texture (.rgb first, for backwards compatibility, then .png)
				const std::string strRGBLogoFileName = strLogoFileName + ".rgb";
				ssgState *st = grSsgLoadTexStateEx(strRGBLogoFileName.c_str(), buf, FALSE, FALSE, FALSE);
				if (!st)
				{
					const std::string strPNGLogoFileName = strLogoFileName + ".png";
					st = grSsgLoadTexStateEx(strPNGLogoFileName.c_str(), buf, FALSE, FALSE);
				}
				((ssgSimpleState*)st)->setShininess(50);
			
				tdble x, y;
				t3Dd normalvector;
				RtTrackLocal2Global(&(pits->driversPits[i].pos), &x, &y, pits->driversPits[i].pos.type);
				RtTrackSideNormalG(pits->driversPits[i].pos.seg, x, y, pits->side, &normalvector);
				tdble x2 = x - pits->width / 2.0 * normalvector.x
					+ pits->len / 2.0 * normalvector.y;
				tdble y2 = y - pits->width / 2.0 * normalvector.y
					- pits->len / 2.0 * normalvector.x;
				tdble z2 = RtTrackHeightG(pits->driversPits[i].pos.seg, x2, y2);
		
				sgVec3 nrm;
				nrm[0] = normalvector.x;
				nrm[1] = normalvector.y;
				nrm[2] = 0;
				pit_nrm->add(nrm);
		
				sgVec2 tex;
				sgVec3 vtx;
				tex[0] = -0.7;
				tex[1] = 0.33;
				vtx[0] = x2;
				vtx[1] = y2;
				vtx[2] = z2;
				pit_tex->add(tex);
				pit_vtx->add(vtx);
			
				tex[0] = -0.7;
				tex[1] = 1.1;
				vtx[0] = x2;
				vtx[1] = y2;
				vtx[2] = z2 + 4.8;
				pit_tex->add(tex);
				pit_vtx->add(vtx);
			
				x2 = x - pits->width / 2.0 * normalvector.x
					- pits->len / 2.0 * normalvector.y;
				y2 = y - pits->width / 2.0 * normalvector.y
					+ pits->len / 2.0 * normalvector.x;
				z2 = RtTrackHeightG(pits->driversPits[i].pos.seg, x2, y2);
		
				tex[0] = 1.3;
				tex[1] = 0.33;
				vtx[0] = x2;
				vtx[1] = y2;
				vtx[2] = z2;
				pit_tex->add(tex);
				pit_vtx->add(vtx);
			
				tex[0] = 1.3;
				tex[1] = 1.1;
				vtx[0] = x2;
				vtx[1] = y2;
				vtx[2] = z2 + 4.8;
				pit_tex->add(tex);
				pit_vtx->add(vtx);
		
				ssgVtxTable *pit = new ssgVtxTable(GL_TRIANGLE_STRIP, pit_vtx, pit_nrm, pit_tex, pit_clr);
				pit->setState(st);
				pit->setCullFace(0);
				ThePits->addKid(pit);
			}//for i
			break;
			
		case TR_PIT_ON_SEPARATE_PATH:
			break;
		
		case TR_PIT_NONE:
			break;	
	}//switch pit->type
}//grCustomizePits

// Update Time
void
grUpdateTime(tSituation *s)
{
	static int lastchecked = -100;
	static double lastTime = -10.0f;
	if( s->currentTime < lastTime ) {
		lastchecked = -100;
		lastTime = s->currentTime;
		return;
	}

	int current = (int)floor( ( s->currentTime + 10.0f ) / 60.0f );
	double dt = s->currentTime - lastTime;
	sgVec3 solposn;
	sgSetVec3(solposn, 0, 0, 0);				  
	ssgGetLight(0)->setPosition(solposn);
	Sky->repositionFlat(solposn , 0, dt);
	lastTime = s->currentTime;
	if( current == lastchecked )
		return;
	lastchecked = current;

	GfLogDebug("Updating time for dynamic time of day\n");
	
	/* Update */
	grSunDeclination += 0.25f; // TODO: Is this delta value realistic ?
	if (grSunDeclination >= 360.0f)
		grSunDeclination = 0.0f;
	
	bodies[eCBSun]->setDeclination ( grSunDeclination * SGD_DEGREES_TO_RADIANS );

	grMoonDeclination += 0.25f; // TODO: Is this delta value realistic ?
	if (grMoonDeclination >= 360.0f)
		grMoonDeclination = 0.0f;
	
	bodies[eCBMoon]->setDeclination ( grMoonDeclination * SGD_DEGREES_TO_RADIANS );

	double sol_angle = bodies[eCBSun]->getAngle();
	double sky_brightness = (1.0 + cos(sol_angle)) / 2.0;
	double scene_brightness = pow(sky_brightness, 0.5);
	
	sky_color[0] = base_sky_color[0] * (float)sky_brightness;
	sky_color[1] = base_sky_color[1] * (float)sky_brightness;
	sky_color[2] = base_sky_color[2] * (float)sky_brightness;
	sky_color[3] = base_sky_color[3];

	/* set cloud and fog color */
	cloud_color[0] = fog_color[0] = base_fog_color[0] * (float)sky_brightness;
	cloud_color[1] = fog_color[1] = base_fog_color[1] * (float)sky_brightness;
	cloud_color[2] = fog_color[2] = base_fog_color[2] * (float)sky_brightness;
	cloud_color[3] = fog_color[3] = base_fog_color[3];

	/* repaint the sky */			

	Sky->repaint(sky_color, fog_color, cloud_color, sol_angle, NPlanets, APlanetsData, NStars, AStarsData);

	sgCoord solpos;
	bodies[eCBSun]-> getPosition(&solpos);
	ssgGetLight(0)-> setPosition(solpos.xyz);	

	scene_ambiant[0] = base_ambiant[0] * (float)scene_brightness;
	scene_ambiant[1] = base_ambiant[1] * (float)scene_brightness;
	scene_ambiant[2] = base_ambiant[2] * (float)scene_brightness;
	scene_ambiant[3] = 1.0;
	
	scene_diffuse[0] = base_diffuse[0] * (float)scene_brightness;
	scene_diffuse[1] = base_diffuse[1] * (float)scene_brightness;
	scene_diffuse[2] = base_diffuse[2] * (float)scene_brightness;
	scene_diffuse[3] = 1.0;
	
	scene_specular[0] = base_specular[0] * (float)scene_brightness;
	scene_specular[1] = base_specular[1] * (float)scene_brightness;
	scene_specular[2] = base_specular[2] * (float)scene_brightness;
	scene_specular[3] = 1.0;
}
