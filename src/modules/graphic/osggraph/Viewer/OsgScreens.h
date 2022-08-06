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

#ifndef _OSGSCREENS_H_
#define _OSGSCREENS_H_

#include <raceman.h>        //tSituation
#include <vector>

#include <osg/Group>

#include "OsgView.h"

class SDDebugHUD;
class SDReflectionMapping;

class SDScreens
{
protected:
    osg::ref_ptr<osgViewer::Viewer> viewer;
    std::vector<SDView *> Screens;
    osg::ref_ptr<osg::Group> root;
    osg::ref_ptr<osg::Group> prerenderRoot;
    SDDebugHUD * debugHUD;

    osg::ref_ptr<osgViewer::GraphicsWindowEmbedded> m_gw;

    int  m_NbActiveScreens;
    int  m_NbArrangeScreens;
    bool m_SpanSplit;
    int  m_CurrentScreenIndex;

public:
    SDScreens();
    ~SDScreens();

    void Init(int x, int y, int width, int height, osg::ref_ptr<osg::Node> m_sceneroot, osg::Vec3f fogcolor);
    void InitCars(tSituation *s);
    void update(tSituation *s,SDFrameInfo* fi, osg::Vec4f(colorfog));
    void changeCamera(long p);
    void registerViewDependantPreRenderNode(osg::ref_ptr<osg::Node> node);
    void toggleHUD();
    void toggleHUDwidget(const std::string &widget);
    void toggleHUDwidgets(const std::string &widgets);

    void toggleHUDdriverinput();

    inline SDView * getActiveView(){return Screens[m_CurrentScreenIndex];}
};

#endif //_OSGSCREENS_H_
