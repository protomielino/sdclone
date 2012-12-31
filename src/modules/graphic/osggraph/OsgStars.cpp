/***************************************************************************

    file        : SDStars.cpp
    copyright   : (C) 2012 by Xavier Bertaux (based on SimGear code)
    web         : http://www.speed-dreams.org
    version     : $Id: SDStars.cpp 3162 2010-12-05 13:11:22Z pouillot $

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <stdio.h>
#include <iostream>

#include <osg/AlphaFunc>
#include <osg/BlendFunc>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Image>
#include <osg/Material>
#include <osg/Point>
#include <osg/ShadeModel>
#include <osg/Node>

#include "SDSky.h"

#define SD_2PI   6.28318530717958647692
#define SD_PI_2  1.57079632679489661923

SDStars::SDStars( void ) :
old_phase(-1)
{
}

// Destructor
SDStars::~SDStars( void ) 
{
}

osg::Node* SDStars::build( int num, const osg::Vec3 star_data[], double star_dist ) 
{
    osg::Geode* geode = new osg::Geode;
    osg::StateSet* stateSet = geode->getOrCreateStateSet();
    stateSet->setRenderBinDetails(-9, "RenderBin");

    osg::BlendFunc* blendFunc = new osg::BlendFunc;
    blendFunc->setFunction(osg::BlendFunc::SRC_ALPHA,
                           osg::BlendFunc::ONE_MINUS_SRC_ALPHA);
    stateSet->setAttributeAndModes(blendFunc);
    stateSet->setMode(GL_FOG, osg::StateAttribute::OFF);
    stateSet->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
    stateSet->setMode(GL_CULL_FACE, osg::StateAttribute::OFF);
    stateSet->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);
    stateSet->setMode(GL_ALPHA_TEST, osg::StateAttribute::OFF);
    
    cl = new osg::Vec4Array;
    osg::Vec3Array* vl = new osg::Vec3Array;

    for ( int i = 0; i < num; ++i ) 
    {
        vl->push_back(osg::Vec3(star_dist * cos( star_data[i][0])
                                * cos( star_data[i][1] ),
                                star_dist * sin( star_data[i][0])
                                * cos( star_data[i][1] ),
                                star_dist * sin( star_data[i][1])));

        cl->push_back(osg::Vec4(1, 1, 1, 1));
    }

    osg::Geometry* geometry = new osg::Geometry;
    geometry->setUseDisplayList(false);
    geometry->setVertexArray(vl);
    geometry->setColorArray(cl.get());
    geometry->setColorBinding(osg::Geometry::BIND_PER_VERTEX);
    geometry->setNormalBinding(osg::Geometry::BIND_OFF);
    geometry->addPrimitiveSet(new osg::DrawArrays(GL_POINTS, 0, vl->size()));
    geode->addDrawable(geometry);

    return geode;
}

bool SDStars::reposition( osg::Vec3f& p, double angle )
{
  sgMat4 T1, GST;
  sgVec3 axis;

  sgMakeTransMat4( T1, p );

  sgSetVec3( axis, 0.0, 0.0, -1.0 );
  sgMakeRotMat4( GST, (float)angle, axis );

  sgMat4 TRANSFORM;
  sgCopyMat4( TRANSFORM, T1 );
  sgPreMultMat4( TRANSFORM, GST );

  sgCoord skypos;
  sgSetCoord( &skypos, TRANSFORM );

  stars_transform->setTransform( &skypos );

  return true;
}

bool SDStars::repaint( double sun_angle, int num, const osg::Vec3 star_data[] ) 
{
    double mag, nmag, alpha, factor, cutoff;

    int phase;

    if ( sun_angle > (SGD_PI_2 + 10.0 * SGD_DEGREES_TO_RADIANS ) ) 
    {
        factor = 1.0;
        cutoff = 4.5;
        phase = 0;
    } 
    else if ( sun_angle > (SGD_PI_2 + 8.8 * SGD_DEGREES_TO_RADIANS ) ) 
    {
        factor = 1.0;
        cutoff = 3.8;
        phase = 1;
    } 
    else if ( sun_angle > (SGD_PI_2 + 7.5 * SGD_DEGREES_TO_RADIANS ) ) 
    {
        factor = 0.95;
        cutoff = 3.1;
        phase = 2;
    } 
    else if ( sun_angle > (SGD_PI_2 + 7.0 * SGD_DEGREES_TO_RADIANS ) ) 
    {
        factor = 0.9;
        cutoff = 2.4;
        phase = 3;
    } 
    else if ( sun_angle > (SGD_PI_2 + 6.5 * SGD_DEGREES_TO_RADIANS ) ) 
    {
        factor = 0.85;
        cutoff = 1.8;
        phase = 4;
    } 
    else if ( sun_angle > (SGD_PI_2 + 6.0 * SGD_DEGREES_TO_RADIANS ) ) 
    {
        factor = 0.8;
        cutoff = 1.2;
        phase = 5;
    } 
    else if ( sun_angle > (SGD_PI_2 + 5.5 * SGD_DEGREES_TO_RADIANS ) ) 
    {
        factor = 0.75;
        cutoff = 0.6;
        phase = 6;
    } else 
    {
        factor = 0.7;
        cutoff = 0.0;
        phase = 7;
    }

    if( phase != old_phase ) 
    {
        old_phase = phase;
        for ( int i = 0; i < num; ++i ) 
	{
            mag = star_data[i][2];
            if ( mag < cutoff ) 
	    {
                nmag = ( 4.5 - mag ) / 5.5;
                alpha = nmag * 0.85 + 0.15;
                alpha *= factor;
            } else 
	    {
                alpha = 0.0;
            }

            if (alpha > 1.0) { alpha = 1.0; }
            if (alpha < 0.0) { alpha = 0.0; }

            (*cl)[i] = osg::Vec4(1, 1, 1, alpha);
        }
        cl->dirty();
    } 
    else 
    {
    }

    return true;
}
