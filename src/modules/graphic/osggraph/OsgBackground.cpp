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
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgDB/Registry>

#include "OsgBackground.h"

SDBackground::SDBackground(void)
{
	type = 0;
	sceneX = 0;
	sceneY = 0;
	m_background = new osg::Group;
}

SDBackground::~SDBackground(void)
{
}

osg::Node *SDBackground::build(int type, int grWrldX, int grWrldY, const std::string TrackPath)
{
	sceneX = grWrldX;
	sceneY = grWrldY;
	int land = type;
	
	std::string strTmpPath = GetDataDir();
	std::string strPath = strTmpPath +"data/objets";
	strPath = strPath+strTmpPath+"data/textures";
	strPath = strPath+TrackPath;
	
	m_background = osgDB::readNodeFile(strPath);
	
	if (!type)
	{
		pre_transform = new osg::MatrixTransform;
		
	
	
