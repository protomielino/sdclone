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
#include <osg/GraphicsContext>

#include "OsgScreens.h"
#include "OsgMain.h"

SDScreens::SDScreens()
{
    viewer = new osgViewer::Viewer();
}

void SDScreens::Init(int x,int y, int width, int height, osg::ref_ptr<osg::Group> m_sceneroot){


    grWinx = x;
    grWiny = y;
    grWinw = width;
    grWinh = height;


    //intialising main screen
    osg::Camera * mirrorCam = new osg::Camera;

    SDView * view = new SDView(viewer->getCamera(),0,0,grWinw,grWinh,mirrorCam);

    viewer->setThreadingModel(osgViewer::Viewer::CullThreadPerCameraDrawThreadPerContext);
    osg::ref_ptr<osgViewer::GraphicsWindowEmbedded> gw = viewer->setUpViewerAsEmbeddedInWindow(0, 0, grWinw, grWinh);
    //viewer->getCamera()->setName("Cam one");
    viewer->getCamera()->setViewport(new osg::Viewport(0, 0, grWinw, grWinh));
    viewer->getCamera()->setGraphicsContext(gw);
    viewer->getCamera()->setComputeNearFarMode(osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR);

    mirrorCam->setGraphicsContext(gw);
    mirrorCam->setClearMask( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    mirrorCam->setReferenceFrame( osg::Camera::ABSOLUTE_RF );

    grScreens.insert(grScreens.end(),view);

    root = new osg::Group;
    root->addChild(m_sceneroot);
    root->addChild(mirrorCam);
    mirrorCam->addChild(m_sceneroot);


    //adding all otherer cams
    osg::Camera * screenCam;
    for(int i=1;i<GR_NB_MAX_SCREEN;i++){
        screenCam = new osg::Camera;
        screenCam->setGraphicsContext(gw);
        screenCam->setClearMask( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
        screenCam->setReferenceFrame( osg::Camera::ABSOLUTE_RF );
        screenCam->setComputeNearFarMode(osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR);
        screenCam->addChild(m_sceneroot);
        screenCam->setNodeMask(0);

        mirrorCam = new osg::Camera;
        mirrorCam->setGraphicsContext(gw);
        mirrorCam->setClearMask( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
        mirrorCam->setReferenceFrame( osg::Camera::ABSOLUTE_RF );
        mirrorCam->addChild(m_sceneroot);
        mirrorCam->setNodeMask(0);

        view = new SDView(screenCam,0,0,grWinw,grWinh,mirrorCam);

        grScreens.insert(grScreens.end(),view);

        root->addChild(screenCam);
        root->addChild(mirrorCam);

    }




    viewer->setSceneData(root);
    //viewer->realize();
}

void SDScreens::InitCars(tSituation *s)
{

    for (int i=0; i<grScreens.size();i++){
        grScreens[i]->Init(s);
    }

}

void SDScreens::update(tSituation * s,SDFrameInfo* fi)
{
    for (int i=0;i< grScreens.size();i++){
        grScreens[i]->update(s,fi);
    }


    if (!viewer->done())
        viewer->frame();
}

void SDScreens::splitScreen(long p){
    switch (p) {
        case GR_SPLIT_ADD:
            if (grNbActiveScreens < GR_NB_MAX_SCREEN)
                grNbActiveScreens++;
                if (grSpanSplit)
                    grNbArrangeScreens=1;
                else
                    grNbArrangeScreens=0;
            break;
        case GR_SPLIT_REM:
            if (grNbActiveScreens > 1)
                grNbActiveScreens--;
                if (grSpanSplit)
                    grNbArrangeScreens=1;
                else
                    grNbArrangeScreens=0;
            break;
        case GR_SPLIT_ARR:
            grNbArrangeScreens++;
    }

    // Ensure current screen index stays in the righ range.
    if (nCurrentScreenIndex >= grNbActiveScreens)
        nCurrentScreenIndex = grNbActiveScreens - 1;

    // Save nb of active screens to user settings.
    GfParmSetNum(grHandle, GR_SCT_DISPMODE, GR_ATT_NB_SCREENS, NULL, grNbActiveScreens);
    GfParmSetNum(grHandle, GR_SCT_DISPMODE, GR_ATT_ARR_SCREENS, NULL, grNbArrangeScreens);
    GfParmWriteFile(NULL, grHandle, "Graph");
    grAdaptScreenSize();
}

void SDScreens::grAdaptScreenSize(){



    int i;

    switch (grNbActiveScreens)
    {
        default:
        case 1:
            // Hack to allow span-split to function OK
            if (grNbArrangeScreens > 1) grNbArrangeScreens = 0;

            // Always Full window.
            grScreens[0]->activate(grWinx, grWiny, grWinw, grWinh, 0.0);
            for (i=1; i < GR_NB_MAX_SCREEN; i++)
                grScreens[i]->deactivate();
            break;
        case 2:
            switch (grNbArrangeScreens) {
            default:
                grNbArrangeScreens = 0;
            case 0:
                // Top & Bottom half of the window
                grScreens[0]->activate(grWinx, grWiny + grWinh / 2, grWinw, grWinh / 2, 0.0);
                grScreens[1]->activate(grWinx, grWiny,              grWinw, grWinh / 2, 0.0);
                break;
            case 1:
                // Left & Right half of the window
                grScreens[0]->activate(grWinx,              grWiny, grWinw / 2, grWinh, grSpanSplit * (-0.5 + 10));
                grScreens[1]->activate(grWinx + grWinw / 2, grWiny, grWinw / 2, grWinh, grSpanSplit * (0.5 + 10));
                break;
            case 2:
                // 33/66% Left/Right
                grScreens[0]->activate(grWinx,              grWiny, grWinw / 3,   grWinh, 0.0);
                grScreens[1]->activate(grWinx + grWinw / 3, grWiny, grWinw * 2/3, grWinh, 0.0);
                break;
            case 3:
                // 66/33% Left/Right
                grScreens[0]->activate(grWinx,                grWiny, grWinw * 2/3, grWinh, 0.0);
                grScreens[1]->activate(grWinx + grWinw * 2/3, grWiny, grWinw / 3,   grWinh, 0.0);
                break;
            }

            for (i=2; i < GR_NB_MAX_SCREEN; i++)
                grScreens[i]->deactivate();
            break;
        case 3:
            switch (grNbArrangeScreens) {
            default:
                grNbArrangeScreens = 0;
            case 0:
                // Left/Right below wide
                grScreens[0]->activate(grWinx,              grWiny + grWinh / 2, grWinw,     grWinh / 2, 0.0);
                grScreens[1]->activate(grWinx,              grWiny,              grWinw / 2, grWinh / 2, 0.0);
                grScreens[2]->activate(grWinx + grWinw / 2, grWiny,              grWinw / 2, grWinh / 2, 0.0);
                break;
            case 1:
                // All side by side
                grScreens[0]->activate(grWinx,                grWiny, grWinw / 3,   grWinh, grSpanSplit * (-1 + 10));
                grScreens[1]->activate(grWinx + grWinw / 3,   grWiny, grWinw / 3,   grWinh, grSpanSplit * (0.0 + 10));
                grScreens[2]->activate(grWinx + grWinw * 2/3, grWiny, grWinw / 3,   grWinh, grSpanSplit * (1 + 10));
                break;
            case 2:
                // Left/Right above wide
                grScreens[0]->activate(grWinx,              grWiny + grWinh / 2, grWinw / 2, grWinh / 2, 0.0);
                grScreens[1]->activate(grWinx + grWinw / 2, grWiny + grWinh / 2, grWinw / 2, grWinh / 2, 0.0);
                grScreens[2]->activate(grWinx,              grWiny,              grWinw,     grWinh / 2, 0.0);
                break;
            case 3:
                // 50/50% Left plus Top/Bottom on Right
                grScreens[0]->activate(grWinx,              grWiny,              grWinw / 2, grWinh, 0.0);
                grScreens[1]->activate(grWinx + grWinw / 2, grWiny + grWinh / 2, grWinw / 2, grWinh / 2, 0.0);
                grScreens[2]->activate(grWinx + grWinw / 2, grWiny,              grWinw / 2, grWinh / 2, 0.0);
                break;
            case 5:
                // 50/50% Top/Bottom on Left plus Right
                grScreens[0]->activate(grWinx,              grWiny + grWinh / 2, grWinw / 2, grWinh / 2, 0.0);
                grScreens[1]->activate(grWinx + grWinw / 2, grWiny,              grWinw / 2, grWinh, 0.0);
                grScreens[2]->activate(grWinx,              grWiny,              grWinw / 2, grWinh / 2, 0.0);
                break;
            case 6:
                // 66/33% Left plus Top/Bottom on Right
                grScreens[0]->activate(grWinx,                grWiny,              grWinw * 2/3, grWinh, 0.0);
                grScreens[1]->activate(grWinx + grWinw * 2/3, grWiny + grWinh / 2, grWinw / 3,   grWinh / 2, 0.0);
                grScreens[2]->activate(grWinx + grWinw * 2/3, grWiny,              grWinw / 3,   grWinh / 2, 0.0);
                break;
            case 7:
                // 33/66% Top/Bottom on Left plus Right
                grScreens[0]->activate(grWinx,              grWiny + grWinh / 2, grWinw / 3,   grWinh / 2, 0.0);
                grScreens[1]->activate(grWinx + grWinw / 3, grWiny,              grWinw * 2/3, grWinh, 0.0);
                grScreens[2]->activate(grWinx,              grWiny,              grWinw / 3,   grWinh / 2, 0.0);
                break;
            }
            for (i=3; i < GR_NB_MAX_SCREEN; i++)
                grScreens[i]->deactivate();
            break;
        case 4:
            switch (grNbArrangeScreens) {
            case 8:
                // 3 side by side + 1 for rear view - only when spanning splits
                if (grSpanSplit) {
                    grScreens[0]->activate(grWinx,                grWiny, grWinw / 4,   grWinh, grSpanSplit * (-1 + 10));
                    grScreens[1]->activate(grWinx + grWinw / 4,   grWiny, grWinw / 4,   grWinh, grSpanSplit * (0.0 + 10));
                    grScreens[2]->activate(grWinx + grWinw * 2/4, grWiny, grWinw / 4,   grWinh, grSpanSplit * (1 + 10));
                    grScreens[3]->activate(grWinx + grWinw * 3/4, grWiny, grWinw / 4,   grWinh, 0.0);
                    break;
                }
            default:
                grNbArrangeScreens = 0;
            case 0:
                // Top/Bottom Left/Rigth Quarters
                grScreens[0]->activate(grWinx,              grWiny + grWinh / 2, grWinw / 2, grWinh / 2, 0.0);
                grScreens[1]->activate(grWinx + grWinw / 2, grWiny + grWinh / 2, grWinw / 2, grWinh / 2, 0.0);
                grScreens[2]->activate(grWinx,              grWiny,              grWinw / 2, grWinh / 2, 0.0);
                grScreens[3]->activate(grWinx + grWinw / 2, grWiny,              grWinw / 2, grWinh / 2, 0.0);
                break;
            case 1:
                // All side by side
                grScreens[0]->activate(grWinx,                grWiny, grWinw / 4,   grWinh, grSpanSplit * (-1.5 + 10));
                grScreens[1]->activate(grWinx + grWinw / 4,   grWiny, grWinw / 4,   grWinh, grSpanSplit * (-0.5 + 10));
                grScreens[2]->activate(grWinx + grWinw * 2/4, grWiny, grWinw / 4,   grWinh, grSpanSplit * (0.5 + 10));
                grScreens[3]->activate(grWinx + grWinw * 3/4, grWiny, grWinw / 4,   grWinh, grSpanSplit * (1.5 + 10));
                break;
            case 2:
                // Left/Middle/Right above wide
                grScreens[0]->activate(grWinx,                grWiny + grWinh / 2, grWinw / 3, grWinh / 2, 0.0);
                grScreens[1]->activate(grWinx + grWinw / 3,   grWiny + grWinh / 2, grWinw / 3, grWinh / 2, 0.0);
                grScreens[2]->activate(grWinx + grWinw * 2/3, grWiny + grWinh / 2, grWinw / 3, grWinh / 2, 0.0);
                grScreens[3]->activate(grWinx,                grWiny,              grWinw,     grWinh / 2, 0.0);
                break;
            case 3:
                // Left/Middle/Right below wide
                grScreens[0]->activate(grWinx,                grWiny + grWinh / 2, grWinw,     grWinh / 2, 0.0);
                grScreens[1]->activate(grWinx,                grWiny,              grWinw / 3, grWinh / 2, 0.0);
                grScreens[2]->activate(grWinx + grWinw / 3,   grWiny,              grWinw / 3, grWinh / 2, 0.0);
                grScreens[3]->activate(grWinx + grWinw * 2/3, grWiny,              grWinw / 3, grWinh / 2, 0.0);
                break;
            case 4:
                // 50/50% Left plus Top/Middle/Bottom on Right
                grScreens[0]->activate(grWinx,              grWiny,                grWinw / 2, grWinh, 0.0);
                grScreens[1]->activate(grWinx + grWinw / 2, grWiny + grWinh * 2/3, grWinw / 2, grWinh / 3, 0.0);
                grScreens[2]->activate(grWinx + grWinw / 2, grWiny + grWinh / 3,   grWinw / 2, grWinh / 3, 0.0);
                grScreens[3]->activate(grWinx + grWinw / 2, grWiny,                grWinw / 2, grWinh / 3, 0.0);
                break;
            case 5:
                // 50/50% Top/Middle/Bottom on Left plus Right
                grScreens[0]->activate(grWinx,              grWiny + grWinh * 2/3, grWinw / 2, grWinh / 3, 0.0);
                grScreens[1]->activate(grWinx + grWinw / 2, grWiny,                grWinw / 2, grWinh, 0.0);
                grScreens[2]->activate(grWinx,              grWiny + grWinh / 3  , grWinw / 2, grWinh / 3, 0.0);
                grScreens[3]->activate(grWinx,              grWiny,                grWinw / 2, grWinh / 3, 0.0);
                break;
            case 6:
                // 66/33% Left plus Top/Middle/Bottom on Right
                grScreens[0]->activate(grWinx,                grWiny,                grWinw * 2/3, grWinh, 0.0);
                grScreens[1]->activate(grWinx + grWinw * 2/3, grWiny + grWinh * 2/3, grWinw / 3,   grWinh / 3, 0.0);
                grScreens[2]->activate(grWinx + grWinw * 2/3, grWiny + grWinh / 3,   grWinw / 3,   grWinh / 3, 0.0);
                grScreens[3]->activate(grWinx + grWinw * 2/3, grWiny,                grWinw / 3,   grWinh / 3, 0.0);
                break;
            case 7:
                // 33/66% Top/Middle/Bottom on Left plus Right
                grScreens[0]->activate(grWinx,              grWiny + grWinh * 2/3, grWinw / 3,   grWinh / 3, 0.0);
                grScreens[1]->activate(grWinx + grWinw / 3, grWiny,                grWinw * 2/3, grWinh, 0.0);
                grScreens[2]->activate(grWinx,              grWiny + grWinh / 3  , grWinw / 3,   grWinh / 3, 0.0);
                grScreens[3]->activate(grWinx,              grWiny,                grWinw / 3,   grWinh / 3, 0.0);
                break;
            }
            for (i=4; i < GR_NB_MAX_SCREEN; i++)
                grScreens[i]->deactivate();
            break;
        case 5:
            switch (grNbArrangeScreens) {
            default:
                grNbArrangeScreens = 0;
            case 0:
                // Top/Bottom Left/Middle/Rigth Matrix
                grScreens[0]->activate(grWinx,                grWiny + grWinh / 2, grWinw / 2, grWinh / 2, 0.0);
                grScreens[1]->activate(grWinx + grWinw / 2  , grWiny + grWinh / 2, grWinw / 2, grWinh / 2, 0.0);
                grScreens[2]->activate(grWinx,                grWiny,              grWinw / 3, grWinh / 2, 0.0);
                grScreens[3]->activate(grWinx + grWinw / 3,   grWiny,              grWinw / 3, grWinh / 2, 0.0);
                grScreens[4]->activate(grWinx + grWinw * 2/3, grWiny,              grWinw / 3, grWinh / 2, 0.0);
                break;
            case 1:
                // All side by side
                grScreens[0]->activate(grWinx,                grWiny, grWinw / 5,   grWinh, grSpanSplit * (-2.0 + 10));
                grScreens[1]->activate(grWinx + grWinw / 5,   grWiny, grWinw / 5,   grWinh, grSpanSplit * (-1.0 + 10));
                grScreens[2]->activate(grWinx + grWinw * 2/5, grWiny, grWinw / 5,   grWinh, grSpanSplit * (0.0 + 10));
                grScreens[3]->activate(grWinx + grWinw * 3/5, grWiny, grWinw / 5,   grWinh, grSpanSplit * (1.0 + 10));
                grScreens[4]->activate(grWinx + grWinw * 4/5, grWiny, grWinw / 5,   grWinh, grSpanSplit * (2.0 + 10));
                break;
            }
            for (i=5; i < GR_NB_MAX_SCREEN; i++)
                grScreens[i]->deactivate();
            break;
        case 6:
            switch (grNbArrangeScreens) {
            case 2:
                if (grSpanSplit) {
                    // five side by side + 1 for rear view - only when spanning splits
                    grScreens[0]->activate(grWinx,                grWiny, grWinw / 6,   grWinh, grSpanSplit * (-2.0 + 10));
                    grScreens[1]->activate(grWinx + grWinw / 6,   grWiny, grWinw / 6,   grWinh, grSpanSplit * (-1.0 + 10));
                    grScreens[2]->activate(grWinx + grWinw * 2/6, grWiny, grWinw / 6,   grWinh, grSpanSplit * (0.0 + 10));
                    grScreens[3]->activate(grWinx + grWinw * 3/6, grWiny, grWinw / 6,   grWinh, grSpanSplit * (1.0 + 10));
                    grScreens[4]->activate(grWinx + grWinw * 4/6, grWiny, grWinw / 6,   grWinh, grSpanSplit * (2.0 + 10));
                    grScreens[5]->activate(grWinx + grWinw * 5/6, grWiny, grWinw / 6,   grWinh, 0.0);
                    break;
                }
            default:
                grNbArrangeScreens = 0;
            case 0:
                // Top/Bottom Left/Middle/Rigth Matrix
                grScreens[0]->activate(grWinx,                grWiny + grWinh / 2, grWinw / 3, grWinh / 2, 0.0);
                grScreens[1]->activate(grWinx + grWinw / 3,   grWiny + grWinh / 2, grWinw / 3, grWinh / 2, 0.0);
                grScreens[2]->activate(grWinx + grWinw * 2/3, grWiny + grWinh / 2, grWinw / 3, grWinh / 2, 0.0);
                grScreens[3]->activate(grWinx,                grWiny,              grWinw / 3, grWinh / 2, 0.0);
                grScreens[4]->activate(grWinx + grWinw / 3,   grWiny,              grWinw / 3, grWinh / 2, 0.0);
                grScreens[5]->activate(grWinx + grWinw * 2/3, grWiny,              grWinw / 3, grWinh / 2, 0.0);
                break;
            case 1:
                // All side by side
                grScreens[0]->activate(grWinx,                grWiny, grWinw / 6,   grWinh, grSpanSplit * (-2.5 + 10));
                grScreens[1]->activate(grWinx + grWinw / 6,   grWiny, grWinw / 6,   grWinh, grSpanSplit * (-1.5 + 10));
                grScreens[2]->activate(grWinx + grWinw * 2/6, grWiny, grWinw / 6,   grWinh, grSpanSplit * (-0.5 + 10));
                grScreens[3]->activate(grWinx + grWinw * 3/6, grWiny, grWinw / 6,   grWinh, grSpanSplit * (0.5 + 10));
                grScreens[4]->activate(grWinx + grWinw * 4/6, grWiny, grWinw / 6,   grWinh, grSpanSplit * (1.5 + 10));
                grScreens[5]->activate(grWinx + grWinw * 5/6, grWiny, grWinw / 6,   grWinh, grSpanSplit * (2.5 + 10));
                break;
            }
            for (i=6; i < GR_NB_MAX_SCREEN; i++)
                grScreens[i]->deactivate();
            break;
        }

}

void SDScreens::changeScreen(long p){

    switch (p) {
        case GR_NEXT_SCREEN:
            nCurrentScreenIndex = (nCurrentScreenIndex + 1) % grNbActiveScreens;
            break;
        case GR_PREV_SCREEN:
            nCurrentScreenIndex = (nCurrentScreenIndex - 1 + grNbActiveScreens) % grNbActiveScreens;
            break;
    }
    GfLogInfo("Changing current screen to #%d (out of %d)\n", nCurrentScreenIndex, grNbActiveScreens);

}
SDScreens::~SDScreens()
{
    for (int i=0;i< grScreens.size();i++){
        delete grScreens[i];
    }
    //root.release();

    //viewer->getSceneData();
    //delete viewer->getSceneData();

    delete viewer;
}




