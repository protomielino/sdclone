/***************************************************************************

    file        : OsgScreens.h
    created     : Sat Feb 2013 15:52:19 CEST 2013
    copyright   : (C) 2013 by Gaëtan André
    email       : gaetan.andre@gmail.com
    version     : $Id$
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _OSGREFLECTIONMAPPING_H_
#define _OSGREFLECTIONMAPPING_H_

#include <osg/TextureCubeMap>
#include <osg/Texture2D>

class SDReflectionMapping
{
    private:
        osg::ref_ptr<osg::Group> camerasRoot;
        std::vector< osg::ref_ptr<osg::Camera> > cameras;
        osg::ref_ptr<osg::TextureCubeMap> reflectionMap;
        SDScreens * screens;

        osg::ref_ptr<osg::Texture2D> map;

    public:
        SDReflectionMapping(SDScreens *s, osg::ref_ptr<osg::Node> m_sceneroot);
        inline osg::ref_ptr<osg::Group> getCamerasRoot(){
            return camerasRoot;
        }
        inline osg::ref_ptr<osg::TextureCubeMap> getReflectionMap(){
            return reflectionMap;
        }
        inline osg::ref_ptr<osg::Texture2D> getMap(){
            return map;
        }

        void update();

        ~SDReflectionMapping();

};

#endif //_OSGREFLECTIONMAPPING_H_
