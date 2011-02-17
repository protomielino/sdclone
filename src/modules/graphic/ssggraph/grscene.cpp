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
#include <string>

#ifdef WIN32
#include <windows.h>
#include <GL/gl.h>
#include <GL/glext.h>
#endif

#include <glfeatures.h>
#include <robottools.h>	//RtXXX()
#include <portability.h> // snprintf

#include "grscene.h"
#include "grbackground.h"
#include "grmain.h"
#include "grloadac.h"
#include "grutil.h"
#include "grrain.h"


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
		sprintf(buf, "%s%s", GfLocalDir(), GR_PARAM_FILE);
		grHandle = GfParmReadFile(buf, GFPARM_RMODE_STD | GFPARM_RMODE_CREAT);
	}//if grHandle

	// Initialize the background, sky ...
	grInitBackground();
	
	// Initialize the rain renderer
	const float precipitationDensity =
		GfParmGetNum(grHandle, GR_SCT_GRAPHIC, GR_ATT_PRECIPDENSITY, "%", 100);
	grRain.initialize(grTrack->local.rain, precipitationDensity);
	
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
		sprintf(buf, "%s%s", GfLocalDir(), GR_PARAM_FILE);
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


void grCustomizePits(void) {
  char buf[512];
  ThePits = new ssgBranch();
  PitsAnchor->addKid(ThePits);

  tTrackPitInfo *pits = &(grTrack->pits);

  /* draw the pit identification */
  GfOut("GrScene:: pits->type: %d\n", pits->type);
  switch (pits->type) {
    case TR_PIT_ON_TRACK_SIDE:
      for (int i = 0; i < pits->nMaxPits; i++) {
        // GfLogDebug("Pit Nbr: %d\n", i);
        ssgVertexArray *pit_vtx = new ssgVertexArray(4);
        ssgTexCoordArray *pit_tex = new ssgTexCoordArray(4);
        ssgColourArray *pit_clr = new ssgColourArray(1);
        ssgNormalArray *pit_nrm = new ssgNormalArray(1);

        sgVec4 clr = {0, 0, 0, 1};
        pit_clr->add(clr);

        std::string strLogoFileName("logo");
        // Default driver logo file name (pit door).

        if (pits->driversPits[i].car[0]) {
          // If we have more than one car in the pit,
          // use the team pit logo of driver 0.
          snprintf(buf, sizeof(buf),
            "%sdrivers/%s/%d;%sdrivers/%s;drivers/%s/%d;drivers/%s;data/textures",
            GfLocalDir(),
            pits->driversPits[i].car[0]->_modName,
            pits->driversPits[i].car[0]->_driverIndex,
            GfLocalDir(),
            pits->driversPits[i].car[0]->_modName,
            pits->driversPits[i].car[0]->_modName,
            pits->driversPits[i].car[0]->_driverIndex,
            pits->driversPits[i].car[0]->_modName);

          // If a custom skin was selected, and it can apply to the pit door,
          // update the logo file name accordingly
          if (strlen(pits->driversPits[i].car[0]->_skinName) != 0
            && pits->driversPits[i].car[0]->_skinTargets & RM_CAR_SKIN_TARGET_PIT_DOOR) {
            strLogoFileName += '-';
            strLogoFileName += pits->driversPits[i].car[0]->_skinName;
            GfLogTrace("Using skinned pit door logo %s\n",
                          strLogoFileName.c_str());
          }
        } else {
          snprintf(buf, sizeof(buf), "data/textures");
        }  // if pits->driverPits[i].car[0]

        // Load logo texture (.rgb first, for backwards compatibility,
        // then .png)
        const std::string strRGBLogoFileName = strLogoFileName + ".rgb";
        ssgState *st = grSsgLoadTexStateEx(strRGBLogoFileName.c_str(), buf,
                                    FALSE, FALSE, FALSE);
        if (!st) {
          const std::string strPNGLogoFileName = strLogoFileName + ".png";
          st = grSsgLoadTexStateEx(strPNGLogoFileName.c_str(), buf,
                                    FALSE, FALSE);
        }
        reinterpret_cast<ssgSimpleState*>(st)->setShininess(50);

        tdble x, y;
        t3Dd normalvector;
        RtTrackLocal2Global(&(pits->driversPits[i].pos), &x, &y,
                            pits->driversPits[i].pos.type);
        RtTrackSideNormalG(pits->driversPits[i].pos.seg, x, y,
                            pits->side, &normalvector);
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

        ssgVtxTable *pit = new ssgVtxTable(GL_TRIANGLE_STRIP, pit_vtx,
                                            pit_nrm, pit_tex, pit_clr);
        pit->setState(st);
        pit->setCullFace(0);
        ThePits->addKid(pit);
      }  // for i
      break;

    case TR_PIT_NO_BUILDING:
      {
      // This mode draws a white, low wall (about 3ft high) with logos

      // First we load the low wall's texture, as it is the same
      // for all the drivers
      snprintf(buf, sizeof(buf), "data/textures");
      const std::string strWallFileName = "tr-bar-gr.png";
      ssgState *stWall = grSsgLoadTexStateEx(strWallFileName.c_str(), buf,
                                              FALSE, FALSE, FALSE);
      reinterpret_cast<ssgSimpleState*>(stWall)->setShininess(100);

      // Loop over all the available pits
      for (int i = 0; i < pits->nMaxPits; i++) {
        tTrackOwnPit *act_pit = &(pits->driversPits[i]);

        // Get this pit's center coords
        tdble x0, y0;
        t3Dd normalvector;
        RtTrackLocal2Global(&(act_pit->pos), &x0, &y0, act_pit->pos.type);
        RtTrackSideNormalG(act_pit->pos.seg, x0, y0, pits->side, &normalvector);
        // This offset needed so the pit walls start at the correct place
        x0 = x0 - pits->width / 2.0 * normalvector.x
          - pits->len / 4.0 * normalvector.y;

        // Load this pit's own logo
        // Default driver logo filename (pit door)
        std::string strLogoFileName("logo");

        if (act_pit->car[0]) {
          // If we have more than one car in the pit,
          // use the team pit logo of driver 0.
          snprintf(buf, sizeof(buf),
            "%sdrivers/%s/%d;%sdrivers/%s;drivers/%s/%d;\\drivers/%s;data/textures",
            GfLocalDir(),
            act_pit->car[0]->_modName,
            act_pit->car[0]->_driverIndex,
            GfLocalDir(),
            act_pit->car[0]->_modName,
            act_pit->car[0]->_modName,
            act_pit->car[0]->_driverIndex,
            act_pit->car[0]->_modName);

          // If a custom skin was selected, and it can apply to the pit door,
          // update the logo file name accordingly
          if (strlen(act_pit->car[0]->_skinName) != 0
              && act_pit->car[0]->_skinTargets & RM_CAR_SKIN_TARGET_PIT_DOOR) {
            strLogoFileName += '-';
            strLogoFileName += act_pit->car[0]->_skinName;
            GfLogTrace("Using skinned pit door logo %s\n",
                        strLogoFileName.c_str());
          }
        } else {
          snprintf(buf, sizeof(buf), "data/textures");
        }  // if act_pit->car[0]

        // Let's draw the low wall
        // It is drawn in 3 parts.
        // First some small wall, then the logo, then some more wall.
        // Small wall bounds: (x1, y1) - (x2, y2)
        // Logo part bounds:  (x2, y2) - (x3, y3)
        // Small wall bounds: (x3, y3) - (x4, y4)

        tdble x1 = x0 - pits->width / 2.0 * normalvector.x
          + pits->len / 2.0 * normalvector.y;
        tdble y1 = y0 - pits->width / 2.0 * normalvector.y
          - pits->len / 2.0 * normalvector.x;
        tdble z1 = RtTrackHeightG(act_pit->pos.seg, x1, y1);

        tdble x2 = x0 - pits->width / 2.0 * normalvector.x
          + pits->len / 4.0 * normalvector.y;
        tdble y2 = y0 - pits->width / 2.0 * normalvector.y
          - pits->len / 4.0 * normalvector.x;
        tdble z2 = RtTrackHeightG(act_pit->pos.seg, x2, y2);

        tdble x3 = x0 - pits->width / 2.0 * normalvector.x
          - pits->len / 4.0 * normalvector.y;
        tdble y3 = y0 - pits->width / 2.0 * normalvector.y
          + pits->len / 4.0 * normalvector.x;
        tdble z3 = RtTrackHeightG(act_pit->pos.seg, x3, y3);

        tdble x4 = x0 - pits->width / 2.0 * normalvector.x
          - pits->len / 2.0 * normalvector.y;
        tdble y4 = y0 - pits->width / 2.0 * normalvector.y
          + pits->len / 2.0 * normalvector.x;
        tdble z4 = RtTrackHeightG(act_pit->pos.seg, x4, y4);

        ssgVertexArray *pit_vtx1 = new ssgVertexArray(4);
        ssgTexCoordArray *pit_tex1 = new ssgTexCoordArray(4);
        ssgColourArray *pit_clr1 = new ssgColourArray(1);
        ssgNormalArray *pit_nrm1 = new ssgNormalArray(1);
        sgVec4 clr1 = { 1, 1, 1, 1 };
        pit_clr1->add(clr1);
        sgVec3 nrm1 = { normalvector.x, normalvector.y, 0.0 };
        pit_nrm1->add(nrm1);

        // First, bottom vertex
        {
          sgVec2 tex = { 0.0, 0.0 };
          sgVec3 vtx = { x1, y1, z1 };
          pit_tex1->add(tex);
          pit_vtx1->add(vtx);
        }

        // First, top vertex
        {
          sgVec2 tex = { 0.0, 0.25 };
          sgVec3 vtx = { x1, y1, z1 + 0.9 };
          pit_tex1->add(tex);
          pit_vtx1->add(vtx);
        }

        // Second, bottom vertex
        {
          sgVec2 tex = { 1.0, 0.0 };
          sgVec3 vtx = { x2, y2, z2 };
          pit_tex1->add(tex);
          pit_vtx1->add(vtx);
        }

        // Second, top vertex
        {
          sgVec2 tex = { 1.0, 0.25 };
          sgVec3 vtx = { x2, y2, z2 + 0.9 };
          pit_tex1->add(tex);
          pit_vtx1->add(vtx);
        }

        ssgVtxTable *pit = new ssgVtxTable(GL_TRIANGLE_STRIP, pit_vtx1,
                                              pit_nrm1, pit_tex1, pit_clr1);
        pit->setState(stWall);
        pit->setCullFace(0);
        ThePits->addKid(pit);

        // Let's draw the logo
        // Load logo texture (.rgb first, for backwards compatibility,
        // then .png)
        const std::string strRGBLogoFileName = strLogoFileName + ".rgb";
        ssgState *stLogo = grSsgLoadTexStateEx(strRGBLogoFileName.c_str(),
                                        buf, FALSE, FALSE, FALSE);
        if (!stLogo) {
          const std::string strPNGLogoFileName = strLogoFileName + ".png";
          stLogo = grSsgLoadTexStateEx(strPNGLogoFileName.c_str(), buf,
                                        FALSE, FALSE);
        }
        reinterpret_cast<ssgSimpleState*>(stLogo)->setShininess(50);

        ssgVertexArray *pit_vtx2 = new ssgVertexArray(4);
        ssgTexCoordArray *pit_tex2 = new ssgTexCoordArray(4);
        ssgColourArray *pit_clr2 = new ssgColourArray(1);
        ssgNormalArray *pit_nrm2 = new ssgNormalArray(1);
        sgVec4 clr2 = { 1, 1, 1, 1 };
        pit_clr2->add(clr2);
        sgVec3 nrm2 = { normalvector.x, normalvector.y, 0.0 };
        pit_nrm2->add(nrm2);
        // First, bottom vertex
        {
          sgVec2 tex = { 0.0, 0.0 };
          sgVec3 vtx = { x2, y2, z2 };
          pit_tex2->add(tex);
          pit_vtx2->add(vtx);
        }

        // First, top vertex
        {
          sgVec2 tex = { 0.0, 0.33 };
          sgVec3 vtx = { x2, y2, z2 + 0.9 };
          pit_tex2->add(tex);
          pit_vtx2->add(vtx);
        }

        // Second, bottom vertex
        {
          sgVec2 tex = { 1.0, 0.0 };
          sgVec3 vtx = { x3, y3, z3 };
          pit_tex2->add(tex);
          pit_vtx2->add(vtx);
        }

        // Second, top vertex
        {
          sgVec2 tex = { 1.0, 0.33 };
          sgVec3 vtx = { x3, y3, z3 + 0.9 };
          pit_tex2->add(tex);
          pit_vtx2->add(vtx);
        }

        ssgVtxTable *pit2 = new ssgVtxTable(GL_TRIANGLE_STRIP, pit_vtx2,
                                              pit_nrm2, pit_tex2, pit_clr2);
        pit2->setState(stLogo);
        pit2->setCullFace(0);
        ThePits->addKid(pit2);

        // Draw 2nd wall
        ssgVertexArray *pit_vtx3 = new ssgVertexArray(4);
        ssgTexCoordArray *pit_tex3 = new ssgTexCoordArray(4);
        ssgColourArray *pit_clr3 = new ssgColourArray(1);
        ssgNormalArray *pit_nrm3 = new ssgNormalArray(1);
        sgVec4 clr3 = { 1, 1, 1, 1 };
        pit_clr3->add(clr3);
        sgVec3 nrm3 = { normalvector.x, normalvector.y, 0.0 };
        pit_nrm3->add(nrm3);
        // First, bottom vertex
        {
          sgVec2 tex = { 0.0, 0.0 };
          sgVec3 vtx = { x3, y3, z3 };
          pit_tex3->add(tex);
          pit_vtx3->add(vtx);
        }

        // First, top vertex
        {
          sgVec2 tex = { 0.0, 0.25 };
          sgVec3 vtx = { x3, y3, z3 + 0.9 };
          pit_tex3->add(tex);
          pit_vtx3->add(vtx);
        }

        // Second, bottom vertex
        {
          sgVec2 tex = { 1.0, 0.0 };
          sgVec3 vtx = { x4, y4, z4 };
          pit_tex3->add(tex);
          pit_vtx3->add(vtx);
        }

        // Second, top vertex
        {
          sgVec2 tex = { 1.0, 0.25 };
          sgVec3 vtx = { x4, y4, z4 + 0.9 };
          pit_tex3->add(tex);
          pit_vtx3->add(vtx);
        }

        ssgVtxTable *pit3 = new ssgVtxTable(GL_TRIANGLE_STRIP, pit_vtx3,
                                              pit_nrm3, pit_tex3, pit_clr3);
        pit3->setState(stWall);
        pit3->setCullFace(0);
        ThePits->addKid(pit3);
      }  // for i
      }
      break;

    case TR_PIT_ON_SEPARATE_PATH:
      // Not implemented yet
      break;

    case TR_PIT_NONE:
      // Nothing to do
      break;
  }  // switch pit->type
}  // grCustomizePits
