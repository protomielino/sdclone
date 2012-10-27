/***************************************************************************

    file                 : OsgScene.h
    created              : Mon Aug 21 20:09:40 CEST 2012
    copyright            : (C) 2012 Xavier Bertaux
    email                : bertauxx@yahoo.fr
    version              : $Id: OsgScene.h 1813 2012-11-10 13:45:43Z torcs-ng $

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _OSGSCENE_H_
#define _OSGSCENE_H_

#include <string>

#define SKYBIN 1
#define TRACKBIN 2

//#include "OsgLoader.h"

//TODO: What is this??? kilo
#ifdef GUIONS
#include <glib.h> 
#endif //GUIONS

#include <track.h>	//tTrack
#include <raceman.h> // tSituation

extern int grWrldX;
extern int grWrldY;
extern int grWrldZ;
extern int grWrldMaxSize;
extern tTrack *grTrack;

extern osg::ref_ptr<osg::Group> m_sceneroot;
extern osg::ref_ptr<osg::Group> m_carroot;

//class cGrCamera;
//class cGrBackgroundCam;


//!Public interface
extern int OsgInitScene(void);
extern int grLoadScene(tTrack *track);
extern void grDrawScene(float speedcar, tSituation *s);
extern void SetTexturePaths(const char *pszPath);
//extern void grDrawScene(void);
extern void grShutdownScene(void);
extern void grCustomizePits(void);
//extern void grDrawBackground(class cGrCamera *, class cGrBackgroundCam *bgCam);
extern void grUpdateTime(tSituation *s);
extern void setViewer(osg::ref_ptr<osgViewer::Viewer> msV);
extern void ClearScene(void);
extern void setSceneRoot(osg::ref_ptr<osg::Group> root);

//TODO: What is this??? kilo
#ifdef GUIONS
class cDoV 
{
public:
  tdble FrontLevelGroupGlobal; /* the distance for the end of the front scene */
  tdble FrontLevelGroup1;      /* the distance for the end of the front scene for group type 1*/
  tdble FrontLevelGroup2;      /* the distance for the end of the front scene for group type 2*/
  tdble FrontLevelGroup3;      /* the distance for the end of the front scene for group type 3*/

  tdble RearLevelGroupGlobal; /* the distance for the end of the front scene */
  tdble RearLevelGroup1;
  tdble RearLevelGroup2;
  tdble RearLevelGroup3;
  
  tdble FrontLevelMap1;      /* the distance for the end of the front scene with only one mapping*/
  tdble FrontLevelMap2;      /* the distance for the end of the front scene with two mapping*/
  tdble FrontLevelMap3;      /* the distance for the end of the front scene with three mapping*/
  tdble RearLevelMap1;
  tdble RearLevelMap2;
  tdble RearLevelMap3;
};

class cHashMapElement 
{
  char	*name;
  int		numberOfMapToApply;
};

class cDistanceOfViewHashing
{
public:
  char				*name;				//segment name
  GHashTable	*ViewGroup;		//all object to display group1+group2+group3 for this segment */
  int ViewGroup_num;				//number of object */
  int ViewGroupMap1_num;
  int ViewGroupMap2_num;
  int ViewGroupMap3_num;
};


extern cDistanceOfViewHashing_t *SceneHashing;
extern cDoV	*currentDistanceOfView;
extern cDoV	PlayableDistanceOfView;
extern cDoV	UnPlayableDistanceOfView;
#endif //GUIONS

#endif //_GRSCENE_H_
