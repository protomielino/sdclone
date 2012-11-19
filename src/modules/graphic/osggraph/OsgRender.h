/***************************************************************************

    file                 : OsgRender.h
    created              : Mon Aug 21 20:09:40 CEST 2012
    copyright            : (C) 2012 Xavier Bertaux
    email                : bertauxx@yahoo.fr
    version              : $Id: OsgRender.h 1813 2012-11-10 13:45:43Z torcs-ng $

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _OSGRENDER_H_
#define _OSGRENDER_H_

#define SKYBIN 1
#define TRACKBIN 2

#include <track.h>	//tTrack
#include <raceman.h> // tSituation

extern int grWrldX;
extern int grWrldY;
extern int grWrldZ;
extern int grWrldMaxSize;
extern tTrack *grTrack;

extern osg::ref_ptr<osg::Group> m_sceneroot;
extern osg::ref_ptr<osg::Group> m_carroot;

class SDRender
{
private:
	osg::ref_ptr<osg::Group> m_sceneroot;
	osg::ref_ptr<osg::Group> m_carroot;
	osg::ref_ptr<osg::Viewer> m_viewer;
	
public:
	SDRender(void);
	~SDRender(void);
	
	void Init(osg::Group *root);
	void Update(float speedcar, tSituation *s);
	void UpdateTime(tSituation *s);
}

#endif //_OSGRENDER_H_
