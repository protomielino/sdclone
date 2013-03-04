/***************************************************************************

    file                 : OsgSky.h
    created              : Mon Aug 21 18:24:02 CEST 2012
    copyright            : (C) 2012 by Xavier Bertaux
    email                : bertauxx@yahoo.fr
    version              : $Id$

***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _OSGSKY_H
#define _OSGSKY_H

#include <vector>
#include <string>
#include <osg/ref_ptr>
#include <osg/MatrixTransform>
#include <osg/Node>
#include <osg/Switch>

#include "OsgDome.h"
#include "OsgMoon.h"
#include "OsgSun.h"
#include "OsgStars.h"
//#include "OsgSharedPtr.h"

using std::vector;

//class SDCloudLayer;
//class SDCloudLayerList;
class SDSun;
class SDMoon;
class SDStars;
class SDSkyDome;
class SDSky;

typedef struct 
{
  osg::Vec3d pos;
  double spin;
  double gst;
  double sun_dist;
  double moon_dist;
  double sun_angle;
} SDSkyState;

typedef struct 
{
  osg::Vec3f sky_color;
  osg::Vec3f adj_sky_color;
  osg::Vec3f fog_color;
  osg::Vec3f cloud_color;
  double sun_angle, moon_angle;
} SDSkyColor;

enum NodeMask 
{
    BACKGROUND_BIT = (1 << 11),
    MODEL_BIT = (1 << 12),
};

class SDSky
{
private:
    //typedef std::vector<SDSharedPtr<SDCloudLayer> > layer_list_type;
    //typedef layer_list_type::iterator layer_list_iterator;
    //typedef layer_list_type::const_iterator layer_list_const_iterator;

    // components of the sky
    SDSkyDome* dome;
    SDSun* sun;
    SDMoon* moon;
    SDStars* planets;
    SDStars* stars;
    //layer_list_type cloud_layers;

    osg::ref_ptr<osg::Group> pre_root, cloud_root;
    osg::ref_ptr<osg::Switch> pre_selector;
    osg::ref_ptr<osg::Group> pre_transform;
    //osg::ref_ptr<osg::MatrixTransform> _ephTransform;

    // visibility
    float visibility;
    float effective_visibility;

    int in_cloud;
    int cur_layer_pos;

    bool in_puff;
    double puff_length;
    double puff_progression;
    double ramp_up;
    double ramp_down;

    // 3D clouds enabled
    //bool clouds_3d_enabled;

    // 3D cloud density
    //double clouds_3d_density;

public:

    /** Constructor */
    SDSky( void );

    /** Destructor */
    ~SDSky( void );

    void build( std::string tex_path, double h_radius, double v_radius, double sun_size, double sun_dist,
          double moon_size, double moon_dist, int nplanets, osg::Vec3d *planet_data,
          int nstars, osg::Vec3d *star_data );

    /*void build( double h_radius_m, double v_radius_m,
                double sun_size, double moon_size,
                const SGEphemeris& eph);*/
    
    //bool repaint( const SDSkyColor &sc/*, const SDEphemeris& eph*/ );
    bool repaint (osg::Vec4d sky_color, osg::Vec4d fog_color, osg::Vec4d cloud_color, double sol_angle,
                  double moon_angle, int nplanets, osg::Vec3d *planet_data,
                  int nstars, osg::Vec3d *star_data);
    //bool reposition( const SDSkyState &st/*, const SDEphemeris& eph*/, double dt = 0.0 );
    bool reposition(osg::Vec3& view_pos, osg::Vec3 zero_elev, osg::Vec3 view_up, double lon, double lat, double alt, double spin, double gst, double dt);
 
    void modify_vis( float alt, float time_factor );

    osg::Node* getPreRoot() { return pre_root.get(); }
    osg::Node* getCloudRoot() { return cloud_root.get(); }
    
    void texture_path( const std::string& path );

    inline void enable() {  pre_selector->setValue(0, 1); }   
    inline void disable() { pre_selector->setValue(0, 0); }   
    inline osg::Vec4f get_sun_color() { return sun->get_color(); }
    inline osg::Vec4f get_scene_color() { return sun->get_scene_color(); }
    
    //void add_cloud_layer (SGCloudLayer * layer);   
    //const SGCloudLayer * get_cloud_layer (int i) const;    
    //SGCloudLayer * get_cloud_layer (int i);  
    //int get_cloud_layer_count () const;

    void setMA(double angle) { moon->setMoonAngle(angle); }
    double getMA() { return moon->getMoonAngle(); }

    void setMR(double rotation) { moon->setMoonRotation( rotation); }
    double getMR() { return moon->getMoonRotation(); }
    void setMRA( double ra ) { moon->setMoonRightAscension( ra ); }
    double getMRA() { return moon->getMoonRightAscension(); }

    void setMD( double decl ) { moon->setMoonDeclination( decl ); }
    double getMD() { return moon->getMoonDeclination(); }

    void setMDist( double dist ) { moon->setMoonDist(dist); }
    double getMDist() { return moon->getMoonDist(); }

    void setSA(double angle) { sun->setSunAngle(angle); }
    double getSA() { return sun->getSunAngle(); }

    void setSR(double rotation) { sun->setSunRotation( rotation ); }
    double getSR() { return sun->getSunRotation(); }

    void setSRA(double ra) { sun->setSunRightAscension( ra ); }
    double getSRA() { return sun->getSunRightAscension(); }

    void setSD( double decl ) { sun->setSunDeclination( decl ); }
    double getSD() { return sun->getSunDeclination(); }

    void setSDistance( double dist ) { sun->setSunDistance( dist ); }
    double getSDistance() { return sun->getSunDistance(); }

    inline float get_visibility() const { return effective_visibility; }
    inline void set_visibility( float v ) {	effective_visibility = visibility = (v <= 25.0) ? 25.0 : v; }

    //virtual double get_3dCloudDensity() const;
    //virtual void set_3dCloudDensity(double density);
    //virtual float get_3dCloudVisRange() const;
    //virtual void set_3dCloudVisRange(float vis);
};

#endif // _OSGSKY_H
