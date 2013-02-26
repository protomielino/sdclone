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


#include <osgViewer/Viewer>
#include <osgViewer/GraphicsWindow>


#include "OsgScreens.h"


SDScreens::SDScreens()
{
    viewer = new osgViewer::Viewer();
}

void SDScreens::Init(int x,int y, int width, int height, osg::ref_ptr<osg::Group> m_sceneroot){


    //int grWinx = x;
    //int grWiny = y;
    int grWinw = width;
    int grWinh = height;









    view = new SDViewer(viewer->getCamera());
    viewer->setThreadingModel(osgViewer::Viewer::CullThreadPerCameraDrawThreadPerContext);
    osg::ref_ptr<osgViewer::GraphicsWindowEmbedded> gw = viewer->setUpViewerAsEmbeddedInWindow(0, 0, grWinw, grWinh);
    //view->getOsgView()->setUpViewInWindow(0, 0, grWinw, grWinh,0);
    viewer->getCamera()->setName("Cam one");
    viewer->getCamera()->setViewport(new osg::Viewport(0, 0, grWinw, grWinh));
    viewer->getCamera()->setGraphicsContext(gw);
    viewer->getCamera()->setProjectionMatrixAsPerspective(67.5f, static_cast<double>((float)grWinw / (float)grWinh), 0.1f, 12000.0f);
    viewer->setSceneData(m_sceneroot.get());
    //viewer->realize();





}

void SDScreens::InitCars(tSituation *s){
    view->Init(s);
}

void SDScreens::update(tSituation * s,SDFrameInfo* fi){
    view->update(s,fi);

    if (!viewer->done())
        viewer->frame();
}

SDScreens::~SDScreens()
{
}




