/***************************************************************************

    file        : OsgViewer.cpp
    created     : Sun Jan 13 22:11:03 CEST 2013
    copyright   : (C) 2013 by Xavier Bertaux
    email       : bertauxx@yahoo.fr
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

#include <tgfclient.h>


#include <osgViewer/CompositeViewer>
#include <osgViewer/GraphicsWindow>
#include <osgViewer/Viewer>

#include "OsgScreens.h"


SDScreens::SDScreens()
{
    viewer = new osgViewer::CompositeViewer();
}

void SDScreens::Init(int x,int y, int width, int height, osg::ref_ptr<osg::Group> m_sceneroot){


    //int grWinx = x;
    //int grWiny = y;
    int grWinw = width;
    int grWinh = height;

    osgViewer::View *  osgView = new osgViewer::View;

    osgViewer::GraphicsWindow *gw = new osgViewer::GraphicsWindow();

    viewer->addView(osgView);



    view = new SDViewer(osgView);
    viewer->setThreadingModel(osgViewer::Viewer::CullThreadPerCameraDrawThreadPerContext);
    view->getOsgView()->setUpViewInWindow(0, 0, grWinw, grWinh,0);
    osgView->getCamera()->setName("Cam one");
    osgView->getCamera()->setViewport(new osg::Viewport(0, 0, grWinw, grWinh));
    osgView->getCamera()->setGraphicsContext(gw);
    osgView->getCamera()->setProjectionMatrixAsPerspective(67.5f, static_cast<double>((float)grWinw / (float)grWinh), 0.1f, 12000.0f);
    osgView->setSceneData(m_sceneroot.get());
    //viewer->realize();





}

void SDScreens::InitCars(tSituation *s){
    view->Init(s);
}

void SDScreens::update(tSituation * s,SDFrameInfo* fi){
    view->update(s,fi);

    //if (!viewer->done())
        viewer->frame();
}

SDScreens::~SDScreens()
{
}




