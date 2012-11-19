/***************************************************************************

    file                 : OsgScenery.cpp
    created              : Mon Aug 21 20:13:56 CEST 2012
    copyright            : (C) 2012 Xavier Bertaux
    email                : bertauxx@yahoo.fr
    version              : $Id: OsgScenery.cpp 2436 2010-05-08 14:22:43Z torcs-ng $

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

#include "OsgLoader.h"
#include "OsgMain.h"
#include "OsgScenery.h"

#include <glfeatures.h>	//gluXXX
#include <robottools.h>	//RtXXX()

SDScenery::SDScenery(void)
{
	_grWrldX = 0;
	_grWrldY = 0;
	_grWrldZ = 0;
	_grWrldMaxSize = 0;
	_max_visibility = 0;
	_nb_cloudlayer = 0;
	_DynamicSkyDome = 0;
	_SkyDomeDistance = 0;
	
	_bgtype = false;
	
	_scenery = 0;
	_background = 0;
	//_spectators = 0;
	//_trees = 0;
	//_pits = 0;	
}

SDScenery::~SDScenery(void)
{
	delete	m_background;
	//delete	m_spectators;
	//delete	m_trees;
	//delete	m_pits;
}

void SDScenery::LoadScene(tTrack *track)
{	
	void		*hndl = grTrackHandle;
	const char	*acname;
	
	m_background = new SDBackground;
	_scenery = new osg::Group;
	grTrack = track;
	
	// Load graphics options.
	LoadGraphicsOptions();

	if(grHandle == NULL) 
	{
		sprintf(buf, "%s%s", GetLocalDir(), GR_PARAM_FILE);
		grHandle = GfParmReadFile(buf, GFPARM_RMODE_STD | GFPARM_RMODE_REREAD);
	}//if grHandle

	/* Determine the world limits */
	_grWrldX = (int)(track->max.x - track->min.x + 1);
	_grWrldY = (int)(track->max.y - track->min.y + 1);
	_grWrldZ = (int)(track->max.z - track->min.z + 1);
	_grWrldMaxSize = (int)(MAX(MAX(_grWrldX, _grWrldY), _grWrldZ));

	acname = GfParmGetStr(hndl, TRK_SECT_GRAPH, TRK_ATT_3DDESC, "track.ac");
  	GfOut("ACname = %s\n", acname);
	if (strlen(acname) == 0) 
	{
		GfLogError("No specified track 3D model file\n");
		return -1;
	}	
	
    osgDB::FilePathList filePathList = osgDB::Registry::instance()->getDataFilePathList();
    
    std::string PathTmp = GetDataDir();   
    //filePathList.push_back(path_list[i]);
	
	std::string PathTmp = GetDataDir();   
    //filePathList.push_back(path_list[i]);
	
	if (grSkyDomeDistance > 0 && grTrack->skyversion > 0)
	{
		grBGSky = strcmp(GfParmGetStr(grHandle, GR_SCT_GRAPHIC, GR_ATT_BGSKY, GR_ATT_BGSKY_DISABLED), GR_ATT_BGSKY_ENABLED) == 0;
		if (grBGSky)
		{
			grBGType = strcmp(GfParmGetStr(grHandle, GR_SCT_GRAPHIC, GR_ATT_BGSKYTYPE, GR_ATT_BGSKY_RING), GR_ATT_BGSKY_LAND) == 0;
			std::string strPath = PathTmp;
			sprintf(buf, "tracks/%s/%s", grTrack->category, grTrack->internalname);
			strPath += buf;
			_scenery = addkid(m_background->buid(grBGType, _grWrldX, _grWrldY, strPath));
		}
	}

	std::string strPath = GetDataDir();
	sprintf(buf, "tracks/%s/%s", grTrack->category, grTrack->internalname);
	strPath+=buf;
	strTPath = strPath;
	filePathList.push_back(strTPath);
	strPath+="/";	
	strPath+=acname;
	
	strTPath = PathTmp;
	strTPath+="data/textures";
	filePathList.push_back(strTPath);

	LoadTrack(strPath);

	return 0;
}
	
void SDScenery::LoadBackgroundGraphicsOptions()
{
	// Sky dome / background.
	grSkyDomeDistance =
		(unsigned)(GfParmGetNum(grHandle, GR_SCT_GRAPHIC, GR_ATT_SKYDOMEDISTANCE, 0, 0) + 0.5);
	if (grSkyDomeDistance > 0 && grSkyDomeDistance < SkyDomeDistThresh)
		grSkyDomeDistance = SkyDomeDistThresh; // If user enabled it (>0), must be at least the threshold.
	
	grDynamicSkyDome = grSkyDomeDistance > 0 && strcmp(GfParmGetStr(grHandle, GR_SCT_GRAPHIC, GR_ATT_DYNAMICSKYDOME, GR_ATT_DYNAMICSKYDOME_DISABLED), 
						GR_ATT_DYNAMICSKYDOME_ENABLED) == 0; 

	GfLogInfo("Graphic options : Sky dome : distance = %u m, dynamic = %s\n",
			  grSkyDomeDistance, grDynamicSkyDome ? "true" : "false");
			
	// Cloud layers.
	grNbCloudLayers = (unsigned)(GfParmGetNum(grHandle, GR_SCT_GRAPHIC, GR_ATT_CLOUDLAYER, 0, 0) + 0.5);

	GfLogInfo("Graphic options : Number of cloud layers : %u\n", grNbCloudLayers);

	grMax_Visibility =
		(unsigned)(GfParmGetNum(grHandle, GR_SCT_GRAPHIC, GR_ATT_VISIBILITY, 0, 0));
}

void SDScenery::LoadGraphicsOptions()
{
	char buf[256];
	
	if (!grHandle)
	{
		sprintf(buf, "%s%s", GfLocalDir(), GR_PARAM_FILE);
		grHandle = GfParmReadFile(buf, GFPARM_RMODE_STD | GFPARM_RMODE_REREAD);
	}//if grHandle

	LoadBackgroundGraphicsOptions();
}

void SDScenery::ClearScene(void)
{
	_scenery->removeChildren(0, _scenery->getNumChildren());	
}

bool SDScenery::LoadTrack(std::string strTrack)
{
	GfOut("Chemin Track : %s\n", strTrack.c_str());
	osgLoader loader;
	GfOut("Chemin Textures : %s\n", m_strTexturePath.c_str());
	loader.AddSearchPath(m_strTexturePath);
	osg::Node *pTrack = loader.Load3dFile(strTrack);

	if (pTrack)
	{
		osgUtil::Optimizer optimizer;
		optimizer.optimize(pTrack); 
		pTrack->getOrCreateStateSet()->setRenderBinDetails(TRACKBIN,"RenderBin");
		m_sceneroot->addChild(pTrack);		
	}
	else
		return false;

	return true;
}

void SDScenery::ShutdownScene(void)
{
}//grShutdownScene

void SDScenery::SetTexturePaths(const char *pszPath)
{
	m_strTexturePath = pszPath;
}