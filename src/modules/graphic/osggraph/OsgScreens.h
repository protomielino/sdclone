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




#include <osg/Group>
#include <osgViewer/Viewer>
#include <raceman.h>        //tSituation
#include <vector>
#include "OsgView.h"


#define GR_SPLIT_ADD	0
#define GR_SPLIT_REM	1
#define GR_SPLIT_ARR	2

#define GR_NEXT_SCREEN	0
#define GR_PREV_SCREEN	1

#define GR_NB_MAX_SCREEN 6

class SDScreens
{
    protected:
        osgViewer::Viewer *viewer;
        std::vector<SDView *> grScreens;
        osg::ref_ptr<osg::Group> root;

        int grWinx;
        int grWiny;
        int grWinw;
        int grWinh;

        int grNbActiveScreens;
        int grNbArrangeScreens;
        bool grSpanSplit;
        int nCurrentScreenIndex;

        void grAdaptScreenSize();
//		int mirrorFlag;
//		void loadParams(tSituation *s);			// Load from parameters files.

	public:
        SDScreens();
        ~SDScreens();


        void Init(int x, int y, int width, int height, osg::ref_ptr<osg::Group> m_sceneroot);
        void InitCars(tSituation *s);
        void update(tSituation *s,SDFrameInfo* fi);
        void splitScreen(long p);
        void changeScreen(long p);
        void changeCamera(long p);

        inline SDView * getActiveView(){return grScreens[nCurrentScreenIndex];}


//		void activate(int x, int y, int w, int h, float v);
//		inline void deactivate(void) { active = false; }

	
//		inline void setCurrentCar(tCarElt *newCurCar) { curCar = newCurCar; }
};

#endif //_OSGSCREENS_H_
