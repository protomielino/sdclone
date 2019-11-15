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

SDScenery::SDScenery(void) :
    m_background(NULL),
    _scenery(NULL),
    SDTrack(NULL)
{
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
    _speedWay = false;
    _speedWayLong = false;
}

SDScenery::~SDScenery(void)
{
    delete	m_background;
    delete  m_pit;
    delete SDTrack;

    if(_scenery != NULL)
    {
        _scenery->removeChildren(0, _scenery->getNumChildren());
        _scenery = NULL;
    }

    SDTrack = NULL;
}

void SDScenery::LoadScene(tTrack *track)
{
    void		*hndl = grTrackHandle;
    const char	*acname;
    char 		buf[256];

    GfOut("Initialisation class SDScenery\n");

    m_background = new SDBackground;
    m_pit = new SDPit;
    _scenery = new osg::Group;
    SDTrack = track;

    // Load graphics options.
    LoadGraphicsOptions();

    if(grHandle == NULL)
    {
        snprintf(buf, 256, "%s%s", GetLocalDir(), GR_PARAM_FILE);
        grHandle = GfParmReadFile(buf, GFPARM_RMODE_STD | GFPARM_RMODE_REREAD);
    }//if grHandle

    /* Determine the world limits */
    grWrldX = (int)(SDTrack->max.x - SDTrack->min.x + 1);
    grWrldY = (int)(SDTrack->max.y - SDTrack->min.y + 1);
    grWrldZ = (int)(SDTrack->max.z - SDTrack->min.z + 1);
    grWrldMaxSize = (int)(MAX(MAX(grWrldX, grWrldY), grWrldZ));

    if (strcmp(SDTrack->category, "speedway") == 0)
    {
        _speedWay = true;
        if (strcmp(SDTrack->subcategory, "long") == 0)
            _speedWayLong = true;
        else
            _speedWayLong = false;
    }
    else
        _speedWay = false;

    GfOut("SpeedWay = %d - SubCategorie = %d\n", _speedWay, _speedWayLong);

    acname = GfParmGetStr(hndl, TRK_SECT_GRAPH, TRK_ATT_3DDESC, "track.ac");

    GfOut("ACname = %s\n", acname);

    if (strlen(acname) == 0)
    {
        GfLogError("No specified track 3D model file\n");
    }

    std::string PathTmp = GetDataDir();

        _bgsky = strcmp(GfParmGetStr(grHandle, GR_SCT_GRAPHIC, GR_ATT_BGSKY, GR_ATT_BGSKY_DISABLED), GR_ATT_BGSKY_ENABLED) == 0;
        if (_bgsky)
        {
            _bgtype = strcmp(GfParmGetStr(grHandle, GR_SCT_GRAPHIC, GR_ATT_BGSKYTYPE, GR_ATT_BGSKY_RING), GR_ATT_BGSKY_LAND) == 0;
            std::string strPath = PathTmp;
            snprintf(buf, 256, "tracks/%s/%s/", SDTrack->category, SDTrack->internalname);
            strPath += buf;
            m_background->build(_bgtype, grWrldX, grWrldY, grWrldZ, strPath);
            GfOut("Background loaded\n");
        }

    std::string strPath = GetDataDir();
    snprintf(buf, 256, "tracks/%s/%s/", SDTrack->category, SDTrack->internalname);

    std::string ext = osgDB::getFileExtension(acname);

    if (ext == "acc")
    {
        GfOut("Load 3D Model Scene ACC\n");
        strPath+=buf;
        _strTexturePath = strPath;
        strPath+=acname;

        LoadTrack(strPath);
    }
    else
    {
        strPath+=buf;

        std::string strTPath = GetDataDir();
        snprintf(buf, 256, "data/textures/");
        strTPath += buf;
        osgDB::FilePathList pathList = osgDB::Registry::instance()->getDataFilePathList();
        pathList.push_back(strPath);
        pathList.push_back(strTPath+"data/objects/");
        pathList.push_back(strTPath+"data/textures/");
        osgDB::Registry::instance()->setDataFilePathList(pathList);
        osg::ref_ptr<osg::Node> pTrack = osgDB::readNodeFile(acname);

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
        else
        {
            _scenery->addChild(pTrack.get());
        }
    }
    
    m_pit->build(track);
	_scenery->addChild(m_pit->getPit());

    osgDB::Registry::instance()->setDataFilePathList( osgDB::FilePathList() );
    osgDB::Registry::instance()->clearObjectCache();
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

    _max_visibility = (unsigned)(GfParmGetNum(grHandle, GR_SCT_GRAPHIC, GR_ATT_VISIBILITY, 0, 0));
}

void SDScenery::LoadGraphicsOptions()
{
    char buf[256];

    if (!grHandle)
    {
        snprintf(buf, 256, "%s%s", GfLocalDir(), GR_PARAM_FILE);
        grHandle = GfParmReadFile(buf, GFPARM_RMODE_STD | GFPARM_RMODE_REREAD);
    }//if grHandle

    LoadSkyOptions();
}

void SDScenery::ShutdownScene(void)
{
    //delete loader;
    _scenery->removeChildren(0, _scenery->getNumChildren());
    _scenery = NULL;

    SDTrack = NULL;
}

bool SDScenery::LoadTrack(std::string& strTrack)
{
    char buf[256];
    std::string name = "";
    GfOut("Chemin Track : %s\n", strTrack.c_str());
    osgLoader loader;
    GfOut("Chemin Textures : %s\n", _strTexturePath.c_str());
    loader.AddSearchPath(_strTexturePath);

    std::string strTPath = GetDataDir();
    snprintf(buf, 256, "data/textures/");
    strTPath += buf;
    loader.AddSearchPath(strTPath);

    osg::Node *pTrack = loader.Load3dFile(strTrack, false, name);

    if (pTrack)
    {
        pTrack->getOrCreateStateSet()->setRenderBinDetails(TRACKBIN,"RenderBin");
        _scenery->addChild(pTrack);
#if 0
        std::string Tpath = GetLocalDir();
        Tpath = Tpath+"/track.osg";
        osgDB::writeNodeFile( *pTrack, Tpath);
#endif
    }
    else
        return false;

    return true;
}

void SDScenery::reposition(double X, double Y, double Z)
{
	m_background->reposition(X, Y, getWorldZ() / 2);
}
