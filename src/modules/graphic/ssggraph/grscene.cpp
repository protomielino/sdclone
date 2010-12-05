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

#include <ctime>

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
#include "grbackground.h"
#include "grmain.h"
#include "grcam.h"	//cGrBackgroundCam
#include "grloadac.h"
#include "grutil.h"
#include "grssgext.h"
#include "grrain.h"
#include "grSky.h"


// Some public global variables.
int grWrldX;
int grWrldY;
int grWrldZ;
int grWrldMaxSize;
tTrack 	 *grTrack;

// TheScene
ssgRoot *TheScene = NULL;

// TheScene kids order (but some others in background.cpp)
ssgBranch *LandAnchor = NULL;
ssgBranch *CarsAnchor = NULL;
ssgBranch *ShadowAnchor = NULL;
ssgBranch *PitsAnchor = NULL;
ssgBranch *SmokeAnchor = NULL;
ssgBranch *SkidAnchor = NULL;
ssgBranch *CarlightAnchor = NULL;
ssgBranch *TrackLightAnchor = NULL;
ssgBranch *ThePits = NULL;

// Must have (Question: What for ?)
int preScene(ssgEntity *e)
{
	return TRUE;
}


/**
 * grInitScene
 * Initialises a scene (ie a new view).
 * 
 * @return 0 if OK, -1 if something failed
 */
int
grInitScene(void)
{
	char buf[256];
	void *hndl = grTrackHandle;
	ssgLight *light = ssgGetLight(0);
	
	// Load graphic options if not already done.
	if(!grHandle) 
	{
		sprintf(buf, "%s%s", GetLocalDir(), GR_PARAM_FILE);
		grHandle = GfParmReadFile(buf, GFPARM_RMODE_STD | GFPARM_RMODE_CREAT);
	}//if grHandle

	// Initialize the background, sky ...
	grInitBackground();
	
	// Initialize the rain renderer
	const float precipitationDensity =
		GfParmGetNum(grHandle, GR_SCT_GRAPHIC, GR_ATT_PRECIPDENSITY, "%", 100);
	grRain.initialize(grTrack->rain, precipitationDensity);
	
	/* GUIONS GL_TRUE */
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);

#ifdef GL_SEPARATE_SPECULAR_COLOR 
	GfLogTrace("Using GL_SEPARATE_SPECULAR_COLOR light model control\n");
	glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SEPARATE_SPECULAR_COLOR);
#else
#ifdef GL_SEPARATE_SPECULAR_COLOR_EXT
	GfLogTrace("Using GL_SEPARATE_SPECULAR_COLOR_EXT light model control\n");
	glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL_EXT, GL_SEPARATE_SPECULAR_COLOR_EXT);
#endif
#endif

  return 0;
}//grInitScene


static grssgLoaderOptions options(/*bDoMipMap*/true);

int
grLoadScene(tTrack *track)
{
	char buf[256];
	void		*hndl = grTrackHandle;
	const char		*acname;
	ssgEntity		*desc;

	if (grMaxTextureUnits == 0) {
		grInitMultiTex();
	}

	grssgSetCurrentOptions(&options);

	// Load graphic options if not already done.
	if(!grHandle) {
		sprintf(buf, "%s%s", GetLocalDir(), GR_PARAM_FILE);
		grHandle = GfParmReadFile(buf, GFPARM_RMODE_STD | GFPARM_RMODE_REREAD);
	}//if grHandle

	grTrack = track;

	// Build scene.
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

	/* Anchor for track lights */
	TrackLightAnchor = new ssgBranch;
	TheScene->addKid(TrackLightAnchor);

	/* Load the background (horizon and sky) */
	grLoadBackground();

	/* Determine the world limits */
	grWrldX = (int)(track->max.x - track->min.x + 1);
	grWrldY = (int)(track->max.y - track->min.y + 1);
	grWrldZ = (int)(track->max.z - track->min.z + 1);
	grWrldMaxSize = (int)(MAX(MAX(grWrldX, grWrldY), grWrldZ));

	/* The track itself, and its landscape */
	acname = GfParmGetStr(hndl, TRK_SECT_GRAPH, TRK_ATT_3DDESC, "track.ac");
	if (strlen(acname) == 0) 
	{
		GfLogError("No specified track 3D model file\n");
		return -1;
	}

	snprintf(buf, sizeof(buf), "tracks/%s/%s;data/textures;data/img;.", grTrack->category, grTrack->internalname);
	ssgTexturePath(buf);
	snprintf(buf, sizeof(buf), "tracks/%s/%s", grTrack->category, grTrack->internalname);
	ssgModelPath(buf);

	desc = grssgLoadAC3D(acname, NULL);
	LandAnchor->addKid(desc);

	return 0;
}//grLoadScene

void
grDrawScene() 
{
	TRACE_GL("refresh: ssgCullAndDraw start");
	ssgCullAndDraw(TheScene);
	TRACE_GL("refresh: ssgCullAndDraw");
}//grDrawScene


void
grShutdownScene(void)
{
	if (TheScene) {
		delete TheScene;
		TheScene = 0;
	}

	grShutdownBackground();

	options.endLoad();
}//grShutdownScene

void
grCustomizePits(void)
{
	char buf[512];
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
