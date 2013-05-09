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

#include <osg/Group>

#include <track.h>	 //tTrack
#include <raceman.h> // tSituation

class SDSky;
class SDScenery;

class SDRender
{
private:
    osg::ref_ptr<osg::Group> m_scene;
    //osg::ref_ptr<osg::Group> m_carroot;


    osg::Vec3f BaseSkyColor;
    osg::Vec3f BaseFogColor;
    osg::Vec3f SkyColor;
    osg::Vec3f FogColor;
    osg::Vec3f CloudsColor;

    osg::Vec4f SceneAmbiant;
    osg::Vec4f SceneDiffuse;
    osg::Vec4f SceneSpecular;
    osg::Vec4f SceneFog;

    unsigned SDSkyDomeDistance;
    unsigned SDNbCloudLayers;
    unsigned SDSkyDomeDistThresh;
    int SDDynamicWeather;
    bool SDDynamicSkyDome;
    float SDSunDeclination;
    float SDMoonDeclination;
    float SDMax_Visibility;
    double SDVisibility;

    osg::Vec3d *AStarsData;
    osg::Vec3d *APlanetsData;
    int NStars;
    int NPlanets;
    float sol_angle;
    float moon_angle;

public:
	SDRender(void);
	~SDRender(void);

    void Init(tTrack *track);
    osg::ref_ptr< osg::StateSet> setFogState();
    void Update(float speedcar, tSituation *s);
    void UpdateTime(tSituation *s);
    void UpdateLight(void);
    void UpdateFogColor(double angle);

    SDSky * getSky();
    osg::Node* getRoot() { return m_scene.get(); }
};

#endif //_OSGRENDER_H_
