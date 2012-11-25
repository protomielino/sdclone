/***************************************************************************

    file                 : OsgScenery.h
    created              : Mon Aug 21 20:09:40 CEST 2012
    copyright            : (C) 2012 Xavier Bertaux
    email                : bertauxx@yahoo.fr
    version              : $Id: OsgScenery.h 1813 2012-11-10 13:45:43Z torcs-ng $

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _OSGSCENERY_H_
#define _OSGSCENERY_H_

#include <string>

#include <track.h>	//tTrack
#include <raceman.h> // tSituation

#include "OsgLoader.h"

class	SDBackground;
//class	SDSpectators;
//class	SDTrees;
class	SDScenery;

#define SKYBIN 1
#define TRACKBIN 2

class SDBackground
{
	osg::ref_ptr<osg::Group> _background;
	osg::ref_ptr<osg::MatrixTransform>	 _background_transform;
	
	double	_sceneX;
	double 	_sceneY;
	bool	_type;
	
public:
	
	// Constructor	
	SDBackground(void);
	
	// Destructor
	~SDBackground(void);
	
	osg::Node *build(bool type, int X, int Y, const std::string strTrack);
	void reposition(int X, int Y);	
};

/*class SDSpectators
{
private:

	osg::ref_prt<osg::Node>	_spectators;
	
	int	_number;

public:

	void SDSpectators(void);
	void ~SDSpectators(void);
	void build(int number, tTrack *track);
};

class SDTrees
{
private:

	osg::ref_ptr<osg::Node> _trees;
	
public:

	void SDTrees(void);
	void ~SDTrees(void);
	void build(tTrack *track);
};*/

class SDScenery
{
private:
	SDBackground	*m_background;
	//SDSpectators	*m_spectators;
	//SDTrees		*m_trees;
	osg::ref_ptr<osg::Group> _scenery;
	osg::ref_ptr<osg::Group> _background;
	
	int	_grWrldX;
	int _grWrldY;
	int _grWrldZ;
	int _grWrldMaxSize;
	int _max_visibility;
	int _nb_cloudlayer;
	int _DynamicSkyDome;
	int _SkyDomeDistance;
	
	bool _bgtype;

	//_spectators = 0;
	//_trees = 0;
	//_pits = 0;	
	
	tTrack 		*grTrack;
	osgLoader	*TheScene;
	
	void LoadGraphicsOptions();
	void CustomizePits(void);
	bool LoadTrack(std::string strTrack);
	
public:
	
	/* Constructor */
	SDScenery(void);
	
	/* Destructor */
	~SDScenery(void);
	
	void 	LoadScene(tTrack *track);
	void	CreatePit(tTrack *track);
	//void	addSpectators(SDSpectators->build(number, tTrack *track));
	//void	addTrees(SDTrees->build(tTrack *track));
	void 	ShutdownScene(void);
	//void	
	
	//osg::Node* getPreRoot() { return pre_root.get(); }

	//osg::ref_ptr<osg::Group>	getSDScenery { return _scenery };
	//osg::Group	getSDBackground { return SDBackground->getbackground; }
	osg::Node* getSceneroot() { return _scenery.get(); }
};

#endif //_OSGSCENERY_H_
