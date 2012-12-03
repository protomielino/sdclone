/***************************************************************************

    file        : OsgSun.cpp
    copyright   : (C) 2012 by Xavier Bertaux (based on SimGear code)
    web         : http://www.speed-dreams.org
    version     : $Id: OsgSun.cpp 3162 2012-12-03 13:11:22Z torcs-ng $

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <osg/AlphaFunc>
#include <osg/BlendFunc>
#include <osg/Fog>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Material>
#include <osg/ShadeModel>
#include <osg/TexEnv>
#include <osg/Texture2D>
#include <osgDB/ReadFile>

#include "OsgSky.h"
#include "OsgColors.h"
#include "OsgMath.h"

// Constructor
SDSun::SDSun( void ) :
    visibility(-9999.0), prev_sun_angle(-9999.0), path_distance(60000.0),
    sun_exp2_punch_through(7.0e-06)
{

}


// Destructor
SDSun::~SDSun( void ) 
{
}


// initialize the sun object and connect it into our scene graph root
osg::Node* SDSun::build( const std::string path, double sun_size ) 
{
    osg::ref_ptr<osgDB::ReaderWriter::Options> options
        = makeOptionsFromPath(path);

    sun_transform = new osg::MatrixTransform;
    osg::StateSet* stateSet = sun_transform->getOrCreateStateSet();

    osg::TexEnv* texEnv = new osg::TexEnv;
    texEnv->setMode(osg::TexEnv::MODULATE);
    stateSet->setTextureAttribute(0, texEnv, osg::StateAttribute::ON);
 
    osg::Material* material = new osg::Material;
    material->setColorMode(osg::Material::AMBIENT_AND_DIFFUSE);
    material->setEmission(osg::Material::FRONT_AND_BACK, osg::Vec4(0,0,0,1));
    material->setSpecular(osg::Material::FRONT_AND_BACK, osg::Vec4(0,0,0,1));
    stateSet->setAttribute(material);

    osg::ShadeModel* shadeModel = new osg::ShadeModel;
    shadeModel->setMode(osg::ShadeModel::SMOOTH);
    stateSet->setAttributeAndModes(shadeModel);

    osg::AlphaFunc* alphaFunc = new osg::AlphaFunc;
    alphaFunc->setFunction(osg::AlphaFunc::ALWAYS);
    stateSet->setAttributeAndModes(alphaFunc);

    osg::BlendFunc* blendFunc = new osg::BlendFunc;
    blendFunc->setSource(osg::BlendFunc::SRC_ALPHA);
    blendFunc->setDestination(osg::BlendFunc::ONE_MINUS_SRC_ALPHA);
    stateSet->setAttributeAndModes(blendFunc);

    stateSet->setMode(GL_FOG, osg::StateAttribute::OFF);
    stateSet->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
    stateSet->setMode(GL_CULL_FACE, osg::StateAttribute::OFF);
    stateSet->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);

    osg::Geode* geode = new osg::Geode;
    stateSet = geode->getOrCreateStateSet();

    stateSet->setRenderBinDetails(-6, "RenderBin");

    osg::Texture2D* texture = SGLoadTexture2D("sun.png", options.get());
    stateSet->setTextureAttributeAndModes(0, texture);

    // Build scenegraph
    sun_cl = new osg::Vec4Array;
    sun_cl->push_back(osg::Vec4(1, 1, 1, 1));

    scene_cl = new osg::Vec4Array;
    scene_cl->push_back(osg::Vec4(1, 1, 1, 1));

    osg::Vec3Array* sun_vl = new osg::Vec3Array;
    sun_vl->push_back(osg::Vec3(-sun_size, 0, -sun_size));
    sun_vl->push_back(osg::Vec3(sun_size, 0, -sun_size));
    sun_vl->push_back(osg::Vec3(-sun_size, 0, sun_size));
    sun_vl->push_back(osg::Vec3(sun_size, 0, sun_size));

    osg::Vec2Array* sun_tl = new osg::Vec2Array;
    sun_tl->push_back(osg::Vec2(0, 0));
    sun_tl->push_back(osg::Vec2(1, 0));
    sun_tl->push_back(osg::Vec2(0, 1));
    sun_tl->push_back(osg::Vec2(1, 1));

    osg::Geometry* geometry = new osg::Geometry;
    geometry->setUseDisplayList(false);
    geometry->setVertexArray(sun_vl);
    geometry->setColorArray(sun_cl.get());
    geometry->setColorBinding(osg::Geometry::BIND_OVERALL);
    geometry->setNormalBinding(osg::Geometry::BIND_OFF);
    geometry->setTexCoordArray(0, sun_tl);
    geometry->addPrimitiveSet(new osg::DrawArrays(GL_TRIANGLE_STRIP, 0, 4));
    geode->addDrawable(geometry);

    sun_transform->addChild( geode );

    // set up the inner-halo state
    geode = new osg::Geode;
    stateSet = geode->getOrCreateStateSet();
    stateSet->setRenderBinDetails(-7, "RenderBin");
    
    texture = SGLoadTexture2D("inner_halo.png", options.get());
    stateSet->setTextureAttributeAndModes(0, texture);

    ihalo_cl = new osg::Vec4Array;
    ihalo_cl->push_back(osg::Vec4(1, 1, 1, 1));

    float ihalo_size = sun_size * 2.0;
    osg::Vec3Array* ihalo_vl = new osg::Vec3Array;
    ihalo_vl->push_back(osg::Vec3(-ihalo_size, 0, -ihalo_size));
    ihalo_vl->push_back(osg::Vec3(ihalo_size, 0, -ihalo_size));
    ihalo_vl->push_back(osg::Vec3(-ihalo_size, 0, ihalo_size));
    ihalo_vl->push_back(osg::Vec3(ihalo_size, 0, ihalo_size));

    osg::Vec2Array* ihalo_tl = new osg::Vec2Array;
    ihalo_tl->push_back(osg::Vec2(0, 0));
    ihalo_tl->push_back(osg::Vec2(1, 0));
    ihalo_tl->push_back(osg::Vec2(0, 1));
    ihalo_tl->push_back(osg::Vec2(1, 1));

    geometry = new osg::Geometry;
    geometry->setUseDisplayList(false);
    geometry->setVertexArray(ihalo_vl);
    geometry->setColorArray(ihalo_cl.get());
    geometry->setColorBinding(osg::Geometry::BIND_OVERALL);
    geometry->setNormalBinding(osg::Geometry::BIND_OFF);
    geometry->setTexCoordArray(0, ihalo_tl);
    geometry->addPrimitiveSet(new osg::DrawArrays(GL_TRIANGLE_STRIP, 0, 4));
    geode->addDrawable(geometry);

    sun_transform->addChild( geode );
    
    geode = new osg::Geode;
    stateSet = geode->getOrCreateStateSet();
    stateSet->setRenderBinDetails(-8, "RenderBin");

    texture = SGLoadTexture2D("outer_halo.png", options.get());
    stateSet->setTextureAttributeAndModes(0, texture);

    ohalo_cl = new osg::Vec4Array;
    ohalo_cl->push_back(osg::Vec4(1, 1, 1, 1));

    double ohalo_size = sun_size * 8.0;
    osg::Vec3Array* ohalo_vl = new osg::Vec3Array;
    ohalo_vl->push_back(osg::Vec3(-ohalo_size, 0, -ohalo_size));
    ohalo_vl->push_back(osg::Vec3(ohalo_size, 0, -ohalo_size));
    ohalo_vl->push_back(osg::Vec3(-ohalo_size, 0, ohalo_size));
    ohalo_vl->push_back(osg::Vec3(ohalo_size, 0, ohalo_size));

    osg::Vec2Array* ohalo_tl = new osg::Vec2Array;
    ohalo_tl->push_back(osg::Vec2(0, 0));
    ohalo_tl->push_back(osg::Vec2(1, 0));
    ohalo_tl->push_back(osg::Vec2(0, 1));
    ohalo_tl->push_back(osg::Vec2(1, 1));

    geometry = new osg::Geometry;
    geometry->setUseDisplayList(false);
    geometry->setVertexArray(ohalo_vl);
    geometry->setColorArray(ohalo_cl.get());
    geometry->setColorBinding(osg::Geometry::BIND_OVERALL);
    geometry->setNormalBinding(osg::Geometry::BIND_OFF);
    geometry->setTexCoordArray(0, ohalo_tl);
    geometry->addPrimitiveSet(new osg::DrawArrays(GL_TRIANGLE_STRIP, 0, 4));
    geode->addDrawable(geometry);

    sun_transform->addChild( geode );

    repaint( 0.0, 1.0 );

    return sun_transform.get();
}

bool SDSun::repaint( double sun_angle, double new_visibility ) 
{
    if ( visibility != new_visibility ) 
    {
        if (new_visibility < 100.0) new_visibility = 100.0;
        else if (new_visibility > 45000.0) new_visibility = 45000.0;
        visibility = new_visibility;
        sun_exp2_punch_through = 2.0/log(visibility);
    }

    if ( prev_sun_angle != sun_angle ) 
    {
        prev_sun_angle = sun_angle;

        double aerosol_factor;
        if ( visibility < 100 ) 
        {
            aerosol_factor = 8000;
        }
        else 
        {
            aerosol_factor = 80.5 / log( visibility / 99.9 );
        }

        double rel_humidity, density_avg;

        if ( !env_node ) 
        {
            rel_humidity = 0.5;
            density_avg = 0.7;
        }
        else 
        {
            rel_humidity = env_node->getFloatValue( "relative-humidity" ); 
            density_avg =  env_node->getFloatValue( "atmosphere/density-tropo-avg" );
        }

        osg::Vec4 i_halo_color, o_halo_color, scene_color, sun_color;

        double red_scat_f, red_scat_corr_f, green_scat_f, blue_scat_f;
        
        red_scat_f = (aerosol_factor * path_distance * density_avg)/5E+07;
        red_scat_corr_f = sun_exp2_punch_through / (1 - red_scat_f);
        sun_color[0] = 1;
        scene_color[0] = 1 - red_scat_f;

        green_scat_f = (aerosol_factor * path_distance * density_avg)/8.8938E+06;
        sun_color[1] = 1 - green_scat_f * red_scat_corr_f;
        scene_color[1] = 1 - green_scat_f;

        blue_scat_f = (aerosol_factor * path_distance * density_avg)/3.607E+06;
        sun_color[2] = 1 - blue_scat_f * red_scat_corr_f;
        scene_color[2] = 1 - blue_scat_f;

        sun_color[3] = 1;
        scene_color[3] = 1;

        double saturation = 1 - ( rel_humidity / 200 );
        scene_color[1] += (( 1 - saturation ) * ( 1 - scene_color[1] ));
        scene_color[2] += (( 1 - saturation ) * ( 1 - scene_color[2] ));

        if (sun_color[0] > 1.0) sun_color[0] = 1.0;
        if (sun_color[0] < 0.0) sun_color[0] = 0.0;
        if (sun_color[1] > 1.0) sun_color[1] = 1.0;
        if (sun_color[1] < 0.0) sun_color[1] = 0.0;
        if (sun_color[2] > 1.0) sun_color[2] = 1.0;
        if (sun_color[2] < 0.0) sun_color[2] = 0.0;

        if (scene_color[0] > 1.0) scene_color[0] = 1.0;
        if (scene_color[0] < 0.0) scene_color[0] = 0.0;
        if (scene_color[1] > 1.0) scene_color[1] = 1.0;
        if (scene_color[1] < 0.0) scene_color[1] = 0.0;
        if (scene_color[2] > 1.0) scene_color[2] = 1.0;
        if (scene_color[2] < 0.0) scene_color[2] = 0.0;

        double scene_f = 0.5 * (1 / (1 - red_scat_f));
        double sun_f = 1.0 - scene_f;
        i_halo_color[0] = sun_f * sun_color[0] + scene_f * scene_color[0];
        i_halo_color[1] = sun_f * sun_color[1] + scene_f * scene_color[1];
        i_halo_color[2] = sun_f * sun_color[2] + scene_f * scene_color[2];
        i_halo_color[3] = 1;

        o_halo_color[0] = 0.2 * sun_color[0] + 0.8 * scene_color[0];
        o_halo_color[1] = 0.2 * sun_color[1] + 0.8 * scene_color[1];
        o_halo_color[2] = 0.2 * sun_color[2] + 0.8 * scene_color[2];
        o_halo_color[3] = blue_scat_f;
        if ((visibility < 10000) && (blue_scat_f > 1)) 
        {
            o_halo_color[3] = 2 - blue_scat_f;
        }
        
        if (o_halo_color[3] > 1) o_halo_color[3] = 1;
        if (o_halo_color[3] < 0) o_halo_color[3] = 0;

        sd_gamma_correct_rgb( i_halo_color._v );
        sd_gamma_correct_rgb( o_halo_color._v );
        sd_gamma_correct_rgb( scene_color._v );    
        sd_gamma_correct_rgb( sun_color._v );

        (*sun_cl)[0] = sun_color;
        sun_cl->dirty();
        (*scene_cl)[0] = scene_color;
        scene_cl->dirty();
        (*ihalo_cl)[0] = i_halo_color;
        ihalo_cl->dirty();
        (*ohalo_cl)[0] = o_halo_color;
        ohalo_cl->dirty();
    }

    return true;
}

bool SDSun::reposition( double rightAscension, double declination, 
                        double sun_dist, double lat, double alt_asl, double sun_angle)
{
    // GST - GMT sidereal time 
    osg::Matrix T2, RA, DEC;

    // xglRotatef( ((SGD_RADIANS_TO_DEGREES * rightAscension)- 90.0),
    //             0.0, 0.0, 1.0);
    RA.makeRotate(rightAscension - 90*SGD_DEGREES_TO_RADIANS, osg::Vec3(0, 0, 1));

    // xglRotatef((SGD_RADIANS_TO_DEGREES * declination), 1.0, 0.0, 0.0);
    DEC.makeRotate(declination, osg::Vec3(1, 0, 0));

    // xglTranslatef(0,sun_dist);
    T2.makeTranslate(osg::Vec3(0, sun_dist, 0));

    sun_transform->setMatrix(T2*DEC*RA);

    // Suncolor related things:
    if ( prev_sun_angle != sun_angle ) 
    {
      	if ( sun_angle == 0 ) sun_angle = 0.1;
      	
        const double r_earth_pole = 6356752.314;
        const double r_tropo_pole = 6356752.314 + 8000;
        const double epsilon_earth2 = 6.694380066E-3;
        const double epsilon_tropo2 = 9.170014946E-3;

        double r_tropo = r_tropo_pole / sqrt ( 1 - ( epsilon_tropo2 * pow ( cos( lat ), 2 )));
        double r_earth = r_earth_pole / sqrt ( 1 - ( epsilon_earth2 * pow ( cos( lat ), 2 )));
 
        double position_radius = r_earth + alt_asl;

        double gamma =  SG_PI - sun_angle;
        double sin_beta =  ( position_radius * sin ( gamma )  ) / r_tropo;
        
        if (sin_beta > 1.0) sin_beta = 1.0;
        
        double alpha =  SG_PI - gamma - asin( sin_beta );

        // OK, now let's calculate the distance the light travels
        path_distance = sqrt( pow( position_radius, 2 ) + pow( r_tropo, 2 )
                        - ( 2 * position_radius * r_tropo * cos( alpha ) ));

        double alt_half = sqrt( pow ( r_tropo, 2 ) + pow( path_distance / 2, 2 ) - r_tropo * path_distance * cos( asin( sin_beta )) ) - r_earth;

        if ( alt_half < 0.0 ) alt_half = 0.0;

        // Push the data to the property tree, so it can be used in the enviromental code
        if ( env_node )
        {
            env_node->setDoubleValue( "atmosphere/altitude-troposphere-top", r_tropo - r_earth );
            env_node->setDoubleValue( "atmosphere/altitude-half-to-sun", alt_half );
      	}
    }

    return true;
}

SGVec4f
SDSun::get_color()
{
    return SGVec4f((*sun_cl)[0][0], (*sun_cl)[0][1], (*sun_cl)[0][2], (*sun_cl)[0][3]);
}

SGVec4f
SDSun::get_scene_color()
{
    return SGVec4f((*scene_cl)[0][0], (*scene_cl)[0][1], (*scene_cl)[0][2], (*scene_cl)[0][3]);
}
