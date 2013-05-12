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

#include <osg/MatrixTransform>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgDB/Registry>
#include <osg/Light>
#include <osg/LightSource>
#include <osg/Camera>

#include "OsgMain.h"
#include "OsgScenery.h"
#include "OsgLoader.h"

#include <glfeatures.h>	//gluXXX
#include <robottools.h>	//RtXXX()
#include <portability.h>

static 	tTrack *grTrack;

SDScenery::SDScenery(void)
{
	_grWrldX = 0;
	_grWrldY = 0;
	_grWrldZ = 0;
	_grWrldMaxSize = 0;
    grWrldX = 0;
    grWrldY = 0;
    grWrldZ = 0;
    grWrldMaxSize = 0;
    _max_visibility = 0;
    _nb_cloudlayer = 0;
    _DynamicSkyDome = 0;
    _SkyDomeDistance = 0;
    _SkyDomeDistThresh = 12000;

	_bgtype = false;
	_bgsky =  false;

    _scenery = NULL;
    _background = NULL;
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
    _background = NULL;
    _scenery = NULL;
}

void SDScenery::LoadScene(tTrack *track)
{
	void		*hndl = grTrackHandle;
	const char	*acname;
	char 		buf[256];

	GfOut("Initialisation class SDScenery\n");

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

    grWrldX = _grWrldX;
    grWrldY = _grWrldY;
    grWrldZ = _grWrldZ;
    grWrldMaxSize = _grWrldMaxSize;

        acname = GfParmGetStr(hndl, TRK_SECT_GRAPH, TRK_ATT_3DDESC, "track.ac");
        GfOut("ACname = %s\n", acname);
        if (strlen(acname) == 0)
        {
                GfLogError("No specified track 3D model file\n");
        }

        std::string PathTmp = GetDataDir();

	if (_SkyDomeDistance > 0 && grTrack->skyversion > 0)
	{
		_bgsky = strcmp(GfParmGetStr(grHandle, GR_SCT_GRAPHIC, GR_ATT_BGSKY, GR_ATT_BGSKY_DISABLED), GR_ATT_BGSKY_ENABLED) == 0;
		if (_bgsky)
		{
			_bgtype = strcmp(GfParmGetStr(grHandle, GR_SCT_GRAPHIC, GR_ATT_BGSKYTYPE, GR_ATT_BGSKY_RING), GR_ATT_BGSKY_LAND) == 0;
			std::string strPath = PathTmp;
			sprintf(buf, "tracks/%s/%s", grTrack->category, grTrack->internalname);
			strPath += buf;
            osg::ref_ptr<osg::Node> bg= m_background->build(_bgtype, _grWrldX, _grWrldY, _grWrldZ, strPath);
            osg::ref_ptr<osg::StateSet> bgstate = bg->getOrCreateStateSet();
            bgstate->setRenderBinDetails(-1, "RenderBin");
            //bg->getOrCreateStateSet()->setRenderingHint( osg::StateSet::OPAQUE_BIN );
            _scenery->addChild(bg.get());
			GfOut("Background loaded\n");
		}
	}

	std::string strPath = GetDataDir();
	sprintf(buf, "tracks/%s/%s", grTrack->category, grTrack->internalname);
	
	std::string ext = osgDB::getFileExtension(acname);
	
	if (ext == "acc")
	{
		strPath+=buf;
        _strTexturePath = strPath;
		strPath+="/";	
		strPath+=acname;

		LoadTrack(strPath);
	}
	else
	{
		strPath+=buf;
        osg::ref_ptr<osgDB::Options> options = new osgDB::Options();
        options->getDatabasePathList().push_back(strPath);
        std::string strTPath = GetDataDir();
        snprintf(buf, 4096, "data/textures/");
        strTPath += buf;
        options->getDatabasePathList().push_back(strTPath);
        osg::ref_ptr<osg::Node> pTrack = osgDB::readNodeFile(acname, options);
        	
        if (ext =="ac")
        {
            osg::ref_ptr<osg::MatrixTransform> rot = new osg::MatrixTransform;
            osg::Matrix mat( 1.0f,  0.0f, 0.0f, 0.0f,
                     			 0.0f,  0.0f, 1.0f, 0.0f,
                     			 0.0f, -1.0f, 0.0f, 0.0f,
                     			 0.0f,  0.0f, 0.0f, 1.0f);
            rot->setMatrix(mat);
            rot->addChild(pTrack);
            _scenery->addChild(rot.get());
        }
    		
        _scenery->addChild(pTrack.get());
	}
}

void SDScenery::LoadSkyOptions()
{
	// Sky dome / background.
	_SkyDomeDistance = (unsigned)(GfParmGetNum(grHandle, GR_SCT_GRAPHIC, GR_ATT_SKYDOMEDISTANCE, 0, 0) + 0.5);
	if (_SkyDomeDistance > 0 && _SkyDomeDistance < _SkyDomeDistThresh)
		_SkyDomeDistance = _SkyDomeDistThresh; // If user enabled it (>0), must be at least the threshold.

	_DynamicSkyDome = _SkyDomeDistance > 0 && strcmp(GfParmGetStr(grHandle, GR_SCT_GRAPHIC, GR_ATT_DYNAMICSKYDOME, GR_ATT_DYNAMICSKYDOME_DISABLED),
						GR_ATT_DYNAMICSKYDOME_ENABLED) == 0;

	GfLogInfo("Graphic options : Sky dome : distance = %u m, dynamic = %s\n",
			  _SkyDomeDistance, _DynamicSkyDome ? "true" : "false");

	// Cloud layers.
	//grNbCloudLayers = (unsigned)(GfParmGetNum(grHandle, GR_SCT_GRAPHIC, GR_ATT_CLOUDLAYER, 0, 0) + 0.5);

	//GfLogInfo("Graphic options : Number of cloud layers : %u\n", grNbCloudLayers);

	_max_visibility = (unsigned)(GfParmGetNum(grHandle, GR_SCT_GRAPHIC, GR_ATT_VISIBILITY, 0, 0));
}

void SDScenery::LoadGraphicsOptions()
{
	char buf[256];

	if (!grHandle)
	{
		sprintf(buf, "%s%s", GfLocalDir(), GR_PARAM_FILE);
		grHandle = GfParmReadFile(buf, GFPARM_RMODE_STD | GFPARM_RMODE_REREAD);
	}//if grHandle

	LoadSkyOptions();
}

void SDScenery::ShutdownScene(void)
{
	_scenery->removeChildren(0, _scenery->getNumChildren());
    _scenery = NULL;
}

bool SDScenery::LoadTrack(std::string strTrack)
{
	char buf[4096];
	GfOut("Chemin Track : %s\n", strTrack.c_str());
	osgLoader loader;
	GfOut("Chemin Textures : %s\n", _strTexturePath.c_str());
	loader.AddSearchPath(_strTexturePath);
	
	std::string strTPath = GetDataDir();
	snprintf(buf, 4096, "data/textures/");
    strTPath += buf;
    loader.AddSearchPath(strTPath);
    	
    osg::ref_ptr<osg::Node> pTrack = loader.Load3dFile(strTrack, false);

	if (pTrack)
	{
        pTrack->getOrCreateStateSet()->setRenderBinDetails(TRACKBIN,"RenderBin");
        _scenery->addChild(pTrack.get());
	}
	else
		return false;

	return true;
}
