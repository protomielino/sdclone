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

#include "grscene.h"
#include "grmain.h"
#include "grcam.h"	//cGrBackgroundCam
#include "grloadac.h"
#include "grutil.h"
#include "grssgext.h"
#include "grrain.h"
#include "grSky.h"

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

int grWrldX;
int grWrldY;
int grWrldZ;
int grWrldMaxSize;
tTrack 	 *grTrack;

ssgStateSelector *grEnvSelector = NULL;
grMultiTexState	*grEnvState = NULL;
grMultiTexState	*grEnvShadowState = NULL;
grMultiTexState	*grEnvShadowStateOnCars = NULL;

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

static ssgRoot *TheBackground = NULL;
static grSky *Sky = NULL;
static ssgTransform *TheSun = NULL;

static grCelestialBody *bodies[MAX_BODIES] = { NULL };
static grMoon *Moon = NULL;

static sgdVec3 *star_data = NULL;
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
static sgVec4 cloud_color;

static sgVec4 scene_ambiant;
static sgVec4 scene_diffuse;
static sgVec4 scene_specular;

//Utility
static const double m_log01 = -log( 0.01 );
static const double sqrt_m_log01 = sqrt( m_log01 );
static char buf[1024];
static void initBackground(void);
static void grDrawRain(void);


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

	/**If no dynamic sky is set,
	 * or the track skyversion doesn't support dynamic sky,
	 * we set up a static one */
	if((skydynamic < SKYDYNAMIC_THR) || (grTrack->skyversion < 1)) 
	{
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
	} else 
	{
		/** If dynamic sky is needed, we create the Sun, the Moon,
		 * some stars and clouds */
		//Query the time

		static ulClock ck;
		//float sd;
		//float sd2;
		double dt = ck.getDeltaTime();

		int div = 80000 / skydynamic;	//skydynamic >= SKYDYNAMIC_THR so cannot div0
		ssgSetNearFar(1, skydynamic);

		//Add random stars
		star_data = new sgdVec3[NSTARS];
		for(int i= 0; i < NSTARS; i++) 
		{
			star_data[i][0] = grRandom() * SGD_PI;
			star_data[i][1] = grRandom() * SGD_PI;
			star_data[i][2] = grRandom();
		}//for i

		//No planets
		//sgdVec3 *planet_data = NULL;

		//Build the sky
		Sky	= new grSky;
		Sky->build(skydynamic, skydynamic, NPLANETS, planet_data, NSTARS, star_data);
		
		//Add the Sun itself
		//ssgaCelestialBody *bodies[MAX_BODIES] = { NULL };
        bodies[SUN] = Sky->addBody(NULL, "data/textures/halo.rgba", (2500 / div), skydynamic, true);
		GLfloat	sunpos1 = 0.0f;
		GLfloat	sunpos2 = 0.0f;
		int cloudtype = 0;
		sunpos1 = (float)GfParmGetNum(hndl, TRK_SECT_GRAPH, TRK_ATT_HOUR, (char*)NULL, sunpos1);
		sunpos2 = (float)GfParmGetNum(hndl, TRK_SECT_GRAPH, TRK_ATT_SUN_H, (char*)NULL, sunpos2);

		int Timeday = grTrack->Timeday;
		if (Timeday > 0)
		{
			switch (grTrack->Timeday) 
			{
				case 1:
					sd = -90.0f;
					//bodies[SUN]->setDeclination ( -90.0 * SGD_DEGREES_TO_RADIANS);
					break;
					
				case 2:
					sd = 2.0f;
					//bodies[SUN]->setDeclination ( 2.0 * SGD_DEGREES_TO_RADIANS);
					break;
					
				case 3:
					sd = 30.0f;
					//bodies[SUN]->setDeclination ( 30.0 * SGD_DEGREES_TO_RADIANS);
					break;
					
				case 4:
					sd = 85.0f;
					//bodies[SUN]->setDeclination ( 85.0 * SGD_DEGREES_TO_RADIANS);
					break;
			}//switch Timeday
		} else 
		{ //if quickrace
			sd = ((sunpos1 / 3600) * 15.0f) - 90.0f;
			printf("Sunpos1 = %f - SD = %f\n", sunpos1, sd);
			//if sunpos
		}//else if quickrace

		bodies[SUN]->setDeclination ( sd * SGD_DEGREES_TO_RADIANS);
		bodies[SUN]->setRightAscension ( sunpos2 * SGD_DEGREES_TO_RADIANS);

		//Add the Moon
		bodies[MOON] = Sky->addBody ( "data/textures/moon.rgba",NULL, (2500 / div), skydynamic);
		if ( sd < 0 )
			sd2 = 3.0 + (rand() % 25);
		else
			sd2 = -(rand() % 45) + 10;

		bodies[MOON]->setDeclination (sd2 * SGD_DEGREES_TO_RADIANS );

		bodies[MOON]->setRightAscension ( ((rand() % 240)) * SGD_DEGREES_TO_RADIANS );

		//Add clouds

		if(Timeday > 0) 
		{
			switch (grTrack->weather) 
			{
				case 1 : cloudtype = CLEAR_CLOUD; break;
				case 2 : cloudtype = MORE_CLOUD; break;
				case 3 : cloudtype = SCARCE_CLOUD; break;
				case 4 : cloudtype = COVERAGE_CLOUD; break;
				case 5 :
				case 6 :
				case 7 : cloudtype = COVERAGE_CLOUD;
				default : cloudtype = CLEAR_CLOUD;
			}//switch cloud
		}
		else
			cloudtype = grTrack->weather;

		if (RainBool > 0)
			cloudtype = 8;

		printf("Cloud = %d", cloudtype);

		grCloudLayer *clouds[MAX_CLOUDS] = { NULL };
		sprintf(buf, "data/textures/scattered%d.rgba", cloudtype);//scattered1, scattered2, etc
		if (RainBool > 0)
			clouds[0] = Sky->addCloud(buf, skydynamic, 650, 400 * div, 400 * div);
		else
			clouds[0] = Sky->addCloud(buf, skydynamic, 2550, 100 * div, 100 * div);
		clouds[0]->setSpeed(60);
		clouds[0]->setDirection(45);
    
		/*clouds[1] = Sky->addCloud ("data/textures/scattered1.rgba", skydynamic, skydynamic-2000, 400 * div, 400 * div);
		clouds[1] -> setSpeed (20);
		clouds[1] -> setDirection (45);
    
		clouds[2] = Sky->addCloud ("data/textures/scattered.png", 20000, 2450, 400, 400);
		clouds[2] -> setSpeed (20);
		clouds[2] -> setDirection (45);*/
   		
		//Set up the light source to the Sun position?
    	sgVec3 solposn;
    	sgSetVec3(solposn, 0, 0, 0);
				  
    	ssgGetLight(0)->setPosition(solposn);
    	Sky->repositionFlat(solposn , 0, dt);    

		//If it rains, decrease visibility
    		if(RainBool > 0) 
    		{
			switch (RainBool)	
			{
				case 1 : Sky->modifyVisibility( 400.0f, (float)dt); break;
				case 2 : Sky->modifyVisibility( 500.0f, (float)dt); break;
				case 3 : Sky->modifyVisibility( 550.0f, (float)dt); break;

    			}//switch RainBool
	
			//grRain.drawPrecipitation(1, 1.0, 1.0, 1.0, 0.0, 0.0, 5.0, 10.0);
     		}
     		else 
     			Sky->modifyVisibility( 0.0f, (float)dt);
     		//if RainBool
    
    		double sol_angle = bodies[SUN]->getAngle();
    		double sky_brightness = (1.0 + cos(sol_angle)) / 2.0;
    		double scene_brightness = pow(sky_brightness, 0.5);
        
		sky_color[0] = base_sky_color[0] * (float)sky_brightness;
		sky_color[1] = base_sky_color[1] * (float)sky_brightness;
		sky_color[2] = base_sky_color[2] * (float)sky_brightness;
		sky_color[3] = base_sky_color[3];
		
		if (RainBool > 0)
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
	
		Sky->repaint(sky_color, fog_color, cloud_color, sol_angle, NPLANETS, planet_data, NSTARS, star_data);
	
		sgCoord solpos;
		bodies[SUN]-> getPosition(&solpos);
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
	}//else skydynamic 
	
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


static grssgLoaderOptions	options(/*bDoMipMap*/true);

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

	skydynamic = GfParmGetNum(grHandle, GR_SCT_GRAPHIC, GR_ATT_SKYDOME, (char*)NULL, skydynamic);
	TimeDyn = GfParmGetNum(grHandle, GR_SCT_GRAPHIC, GR_ATT_DYNAMICTIME, (char*)NULL, TimeDyn);
	WeatherDyn = GfParmGetNum(grHandle, GR_SCT_GRAPHIC, GR_ATT_DYNAMICWEATHER, (char*)NULL, WeatherDyn);
			
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

	RainBool = grTrack->Rain;
	printf("Rain = %d\n", RainBool);

	//acname = GfParmGetStr(hndl, TRK_SECT_GRAPH, TRK_ATT_3DDESC, "track.ac");
	/*if ((grTrack->Timeday == 1) && (grTrack->skyversion > 0)) // If night in quickrace, practice or network mode
		acname = GfParmGetStr(hndl, TRK_SECT_GRAPH, TRK_ATT_3DDESC3, "track.ac");
	else*/
		acname = GfParmGetStr(hndl, TRK_SECT_GRAPH, TRK_ATT_3DDESC, "track.ac");
  	GfOut("ACname = %s\n", acname);
	if (strlen(acname) == 0) 
	{
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
grDrawScene(float speedcar, tSituation *s) 
//grDrawScene(void)
{	
	TRACE_GL("refresh: ssgCullAndDraw start");    
	if ((skydynamic >= SKYDYNAMIC_THR) && (grTrack->skyversion > 0)) 
	{
		if(TimeDyn == 1) {grUpdateTime(s);}		
				
		glClearColor(fog_color[0], fog_color[1], fog_color[2], fog_color[3]);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		GLfloat fog_exp2_density = (float)sqrt_m_log01 / Sky->getVisibility();
		glEnable(GL_FOG);
		glFogi(GL_FOG_MODE, GL_EXP2);
		glFogfv(GL_FOG_COLOR, fog_color);
		glFogf(GL_FOG_DENSITY, fog_exp2_density);
		glHint(GL_FOG_HINT, GL_DONT_CARE);

		ssgGetLight(0)-> setColour(GL_DIFFUSE, white);

		Sky->preDraw();

		glLightModelfv( GL_LIGHT_MODEL_AMBIENT, black);
		ssgGetLight(0) -> setColour( GL_AMBIENT, scene_ambiant);
		ssgGetLight(0) -> setColour( GL_DIFFUSE, scene_diffuse);
		ssgGetLight(0) -> setColour( GL_SPECULAR, scene_specular);

		TRACE_GL("refresh: ssgCullAndDraw start");
		ssgCullAndDraw(TheScene);
		TRACE_GL("refresh: ssgCullAndDraw");

		Sky->postDraw(skydynamic);
		if(RainBool > 0)
		{
			grRain.drawPrecipitation(RainBool, 1.0, 0.0, 0.0, 0.0, speedcar);
		}
	
	} 
	else 
	{	//if (skydynamic==0 | grTrack->version < 5)
		TRACE_GL("refresh: ssgCullAndDraw start");
		ssgCullAndDraw(TheScene);
		TRACE_GL("refresh: ssgCullAndDraw");
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
		GfOut("Loading Environment Mapping Image %s\n", graphic->env[i]);
		envst = (ssgSimpleState*)grSsgLoadTexState(graphic->env[i]);
		envst->enable(GL_BLEND);
		grEnvSelector->setStep(i, envst);
		envst->deRef();
  }//for i

  grEnvSelector->selectStep(0); //mandatory !!!
	grEnvState=(grMultiTexState*)grSsgEnvTexState(graphic->env[0]);
	grEnvShadowState=(grMultiTexState*)grSsgEnvTexState("envshadow.png");
	if (grEnvShadowState == NULL) {
		ulSetError ( UL_WARNING, "grscene:initBackground Failed to open envshadow.png for reading") ;
		ulSetError ( UL_WARNING, "        mandatory for top env mapping ") ;
		ulSetError ( UL_WARNING, "        should be in the .xml !! ") ;
		ulSetError ( UL_WARNING, "        copy the envshadow.png from g-track-2 to the track you selected ") ;
		ulSetError ( UL_WARNING, "        c'est pas classe comme sortie, mais ca evite un crash ") ;
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
	
	if ((pits->nPitSeg < 1) && (pits->nMaxPits > 1)) 
		pits->nPitSeg = pits->nMaxPits;              

	switch (pits->type) {
		case TR_PIT_ON_TRACK_SIDE:
			for(int i = 0; i < pits->nPitSeg; i++) {
				GfOut("Pit Nbr: %d\n", i);        
				ssgVertexArray *pit_vtx = new ssgVertexArray(4);
				ssgTexCoordArray *pit_tex = new ssgTexCoordArray(4);
				ssgColourArray *pit_clr = new ssgColourArray(1);
				ssgNormalArray *pit_nrm = new ssgNormalArray(1);
			
				sgVec4 clr = {0, 0, 0, 1};
				pit_clr->add(clr);
		
				if (pits->driversPits[i].car[0]) {
					// If we have more than one car in the pit use the team pit logo of driver 0. 
					if (pits->driversPits[i].freeCarIndex == 1) { 
						// One car assigned to the pit.
						sprintf(buf, "drivers/%s/%d;drivers/%s;data/textures;data/img;.",
							pits->driversPits[i].car[0]->_modName,
							pits->driversPits[i].car[0]->_driverIndex,
							pits->driversPits[i].car[0]->_modName);
					} else {
						// Multiple cars assigned to the pit.
						sprintf(buf, "drivers/%s;data/textures;data/img;.",
							pits->driversPits[i].car[0]->_modName);
					}//if ...freeCarIndex == 1
				} else {
					sprintf(buf, "data/textures;data/img;.");
				}//if pits->driverPits[i].car[0]
			
				ssgState *st = grSsgLoadTexStateEx("logo.rgb", buf, FALSE, FALSE);
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
	if( s->currentTime < lastTime ) { lastchecked = -100; lastTime = s->currentTime; return; }
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

	/* Update */
	sd = sd + 0.25f;
	if (sd > 359.9f)
		sd = 0.0f;
	sd2 = sd2 + 0.25f;
	if (sd2 > 359.9f)
		sd2 = 0.0f;
	
	bodies[SUN]->setDeclination ( sd * SGD_DEGREES_TO_RADIANS);
	bodies[MOON]->setDeclination (	sd2 * SGD_DEGREES_TO_RADIANS );

	double sol_angle = bodies[SUN]->getAngle();
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

	Sky->repaint(sky_color, fog_color, cloud_color, sol_angle, NPLANETS, planet_data, NSTARS, star_data);

	sgCoord solpos;
	bodies[SUN]-> getPosition(&solpos);
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
