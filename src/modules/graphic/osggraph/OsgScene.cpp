/***************************************************************************

    file                 : OsgScene.cpp
    created              : Mon Aug 21 20:13:56 CEST 2012
    copyright            : (C) 2012 Xavier Bertaux
    email                : bertauxx@yahoo.fr
    version              : $Id: OsgScene.cpp 2436 2010-05-08 14:22:43Z torcs-ng $

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


/*#ifdef WIN32
#include <windows.h>
#include <GL/gl.h>
#include <GL/glext.h>
#endif*/

#include <osgUtil/Optimizer>
#include <osg/MatrixTransform>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgDB/Registry>

#include "OsgLoader.h"
#include "OsgMain.h"
#include "OsgScene.h"


#include <glfeatures.h>	//gluXXX
#include <robottools.h>	//RtXXX()



//#include "grcam.h"	//cGrBackgroundCam
/*#include "grutil.h"
#include "grssgext.h"
#include "grrain.h"

#include "osgBackground.h"*/

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

osg::Node *Load3dFile(std::string strFile);
bool LoadTrack(std::string strTrack);

std::string m_strTexturePath;
osg::ref_ptr<osg::MatrixTransform> m_sceneroot;
osg::ref_ptr<osg::Group> m_carroot;
osg::ref_ptr<osgViewer::Viewer> m_sceneViewer;
osg::Timer m_timer;
osg::Timer_t m_start_tick;
osg::Matrix m_ModelMat;
osg::Matrix m_ProjMat;

/*ssgStateSelector *grEnvSelector = NULL;
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
ssgBranch *ThePits = NULL;*/

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
//static void initBackground(void);
//static void grDrawRain(void);

//extern ssgEntity *grssgLoadAC3D(const char *fname, const ssgLoaderOptions* options);


//Must have
//int preScene(ssgEntity *e) { return TRUE; }


/**
 * grInitScene
 * Initialises a scene (ie a new view).
 * 
 * @return 0 if OK, -1 if something failed
 */
int
OsgInitScene(void)
{
	void *hndl = grTrackHandle;

  	return 0;
}//grInitScene


//static ssgLoaderOptionsEx	options;

void setViewer(osg::ref_ptr<osgViewer::Viewer> msV)
{
    m_sceneViewer = msV;
}

bool LoadTrack(std::string strTrack)
{
	GfOut("Chemin Track : %s\n", strTrack.c_str());
	osgLoader loader;
	loader.AddSearchPath(m_strTexturePath);
	osg::Node *pTrack = loader.Load3dFile(strTrack);

 	//osgDB::writeNodeFile(*pTrack,"mytrack.osg");

	if (pTrack)
	{
		osgUtil::Optimizer optimizer;
		optimizer.optimize(pTrack); 
		osg::Matrixf mat;
		//Rotate track model 90 degrees to match the way plib loader works
		mat = mat.rotate(osg::inDegrees(90.0), osg::X_AXIS);
		osg::MatrixTransform *pTrans = new osg::MatrixTransform(mat);
		pTrans->addChild(pTrack);
		pTrans->getOrCreateStateSet()->setRenderBinDetails(TRACKBIN,"RenderBin");
		m_sceneroot->addChild(pTrans);
		
	}
	else
		return false;

	return true;
}

/*void grOSG::SetCamera(osg::Matrixf projMat,osg::Matrixf modelMat)
{
	m_pCamera->setProjectionMatrix(projMat);
	m_pCamera->setViewMatrix(modelMat);
}

void grOSG::SetCamera(osg::Vec3 center,osg::Vec3 eye,osg::Vec3 up,float fov,float aspect,float znear,float zfar)
{

	m_pCamera->setProjectionMatrixAsPerspective(fov,aspect,znear,zfar);
	m_pCamera->setViewMatrixAsLookAt(eye,center,up);
}*/

int
grLoadScene(tTrack *track)
{
	void		*hndl = grTrackHandle;
	const char		*acname;
	
	grTrack = track;
	//ssgEntity		*desc;

	/*if (maxTextureUnits == 0) {
		InitMultiTex();
	}*/

	if(grHandle == NULL) 
	{
		sprintf(buf, "%s%s", GetLocalDir(), GR_PARAM_FILE);
		grHandle = GfParmReadFile(buf, GFPARM_RMODE_STD | GFPARM_RMODE_REREAD);
	}//if grHandle

	//skydynamic = GfParmGetNum(grHandle, GR_SCT_GRAPHIC, GR_ATT_SKYDOME, (char*)NULL, skydynamic);
	//TimeDyn = GfParmGetNum(grHandle, GR_SCT_GRAPHIC, GR_ATT_DYNAMICTIME, (char*)NULL, TimeDyn);
	//WeatherDyn = GfParmGetNum(grHandle, GR_SCT_GRAPHIC, GR_ATT_DYNAMICWEATHER, (char*)NULL, WeatherDyn);
			
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

	std::string strTPath = GetDataDir();
	sprintf(buf, "tracks/%s/%s;data/textures;data/img;.", grTrack->category, grTrack->internalname);
	//GfOut("strPath = %s\n", acname);
	strTPath+=buf;
	//strTPath+="tracks/speedway/milky-five";
	SetTexturePaths(strTPath.c_str());
	
	//ssgTexturePath(buf);
	//sprintf(buf, "tracks/%s/%s", grTrack->category, grTrack->internalname);
	//ssgModelPath(buf);

	std::string strPath = GetDataDir();
	sprintf(buf, "tracks/%s/%s", grTrack->category, grTrack->internalname);
	strPath+=buf;
	//strPath+="tracks/speedway/milky-five";
	strPath+="/";
	strPath+=acname;
	//GetOSG()->ClearScene();
	//osgBackground back;
	//GetOSG()->SetBackGround(back.CreateBackground(grTrack));
	LoadTrack(strPath);
	//desc = grssgLoadAC3D(acname, NULL);
	//LandAnchor->addKid(desc);

	return 0;
}//grLoadScene

void
grDrawScene(float speedcar, tSituation *s) 
//grDrawScene(void)
{	
	
}//grDrawScene


void
grShutdownScene(void)
{

}//grShutdownScene

void SetTexturePaths(const char *pszPath)
{
	m_strTexturePath = pszPath;
}



/*static void
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
	
	/*if ((pits->nPitSeg < 1) && (pits->nMaxPits > 1)) 
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
	/*sd = sd + 0.25f;
	if (sd > 359.9)
		sd = 0.0;
	sd2 = sd2 + 0.25f;
	if (sd2 > 359.9)
		sd2 = 0.0;
	
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
	/*cloud_color[0] = fog_color[0] = base_fog_color[0] * (float)sky_brightness;
	cloud_color[1] = fog_color[1] = base_fog_color[1] * (float)sky_brightness;
	cloud_color[2] = fog_color[2] = base_fog_color[2] * (float)sky_brightness;
	cloud_color[3] = fog_color[3] = base_fog_color[3];

	/* repaint the sky */			

	/*Sky->repaint(sky_color, fog_color, cloud_color, sol_angle, NPLANETS, planet_data, NSTARS, star_data);

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
}*/
