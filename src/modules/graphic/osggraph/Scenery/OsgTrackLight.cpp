/***************************************************************************

    file                 : OsgTrackLight.cpp
    created              : Sun Oct 05 20:13:56 CEST 2014
    copyright            : (C) 2014 Xavier Bertaux
    email                : bertauxx@yahoo.fr
    version              : $Id: OsgTrackLight.cpp 2436 2014-10-05 20:22:43Z torcs-ng $

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

SDTrackLights::SDTrackLights(void)
{
}

SDTrackLights::~SDTrackLights(void)
{
    _osgtracklight->removeChildren(0, _osgtracklight->getNumChildren());
    _osgtracklight = NULL;
}

void SDTrackLights::build(const std::string TrackPath)
{
        osg::ref_ptr<osg::StateSet> state;
}
