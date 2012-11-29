/***************************************************************************

    file                 : OsgBackground.cpp
    created              : Mon Aug 21 20:13:56 CEST 2012
    copyright            : (C) 2012 Xavier Bertaux
    email                : bertauxx@yahoo.fr
    version              : $Id: OsgBackground.cpp 2436 2010-05-08 14:22:43Z torcs-ng $

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
//#include <osg/Node>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgDB/Registry>

#include "OsgScenery.h"

SDBackground::SDBackground(void)
{
}

SDBackground::~SDBackground(void)
{
}

osg::Node *SDBackground::build(bool type, int grWrldX, int grWrldY, const std::string TrackPath)
{
	_sceneX = grWrldX;
	_sceneY = grWrldY;
	int land = type;
	
	std::string LocalPath = GetDataDir();
	
	osgDB::FilePathList pathList = osgDB::Registry::instance()->getDataFilePathList();
   	pathList.push_back(TrackPath);
    	pathList.push_back(LocalPath+"data/objects");
    	pathList.push_back(LocalPath+"data/textures");
    	osgDB::Registry::instance()->setDataFilePathList(pathList);
	
	osg::Node *m_background = osgDB::readNodeFile("background-sky.ac");
	
	if (!type)
	{
		_background_transform = new osg::MatrixTransform;
		_background_transform->addChild( m_background );
	}
}
	
	
