/***************************************************************************

    file        : grMoon.cpp
    copyright   : (C) 2009 by Xavier Bertaux (based on ssgasky plib code)
    web         : http://www.speed-dreams.org
    version     : $Id: grSky.cpp 3162 2010-12-05 13:11:22Z pouillot $

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/



#include "grSky.h"
#include "grSphere.h"

static int grMoonOrbPreDraw( ssgEntity *e ) 
{
    ssgLeaf *f = (ssgLeaf *)e;
    if ( f -> hasState () ) f->getState()->apply() ;

    glPushAttrib( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_ENABLE_BIT );
    glDisable( GL_DEPTH_TEST );
    glDisable( GL_FOG );
    glBlendFunc ( GL_SRC_ALPHA, GL_ONE );
    return true;
}

static int grMoonOrbPostDraw( ssgEntity *e ) 
{
    glBlendFunc ( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    glPopAttrib();
    return true;
}

// Constructor
cGrMoon::cGrMoon( void ) :
    prev_moon_angle(-1)
{
}

// Destructor
cGrMoon::~cGrMoon( void ) 
{
}

// build the moon object
ssgBranch * cGrMoon::build( const char *moon_path, double moon_size ) 
{
    orb_state = new ssgSimpleState();
    orb_state->setTexture( moon_path );
    orb_state->setShadeModel( GL_SMOOTH );
    orb_state->enable( GL_LIGHTING );
    orb_state->enable( GL_CULL_FACE );
    orb_state->enable( GL_TEXTURE_2D );
    orb_state->enable( GL_COLOR_MATERIAL );
    orb_state->setColourMaterial( GL_DIFFUSE );
    orb_state->setMaterial( GL_AMBIENT, 0, 0, 0, 1.0 );
    orb_state->setMaterial( GL_EMISSION, 0.0, 0.0, 0.0, 1 );
    orb_state->setMaterial( GL_SPECULAR, 0, 0, 0, 1 );
    orb_state->enable( GL_BLEND );
    orb_state->enable( GL_ALPHA_TEST );
    orb_state->setAlphaClamp( 0.01 );

    cl = new ssgColourArray( 1 );
    sgVec4 color;
    sgSetVec4( color, 1.0, 1.0, 1.0, 1.0 );
    cl->add( color );

    ssgBranch *orb = grMakeSphere( orb_state, cl, moon_size, 15, 15,
				    grMoonOrbPreDraw, grMoonOrbPostDraw );

    repaint( 0.0 );

    moon_transform = new ssgTransform;
    moon_transform->addKid( orb );

    return moon_transform;
}

bool cGrMoon::repaint( double moon_angle ) 
{
    if (prev_moon_angle != moon_angle) 
	{
        prev_moon_angle = moon_angle;

        float moon_factor = 4*cos(moon_angle);

        if (moon_factor > 1) moon_factor = 1.0;
        if (moon_factor < -1) moon_factor = -1.0;
        moon_factor = moon_factor/2 + 0.5;

        sgVec4 color;
        color[1] = sqrt(moon_factor);
        color[0] = sqrt(color[1]);
        color[2] = moon_factor * moon_factor;
        color[2] *= color[2];
        color[3] = 1.0;

        grGammaCorrectRGB( color );

        float *ptr;
        ptr = cl->get( 0 );
        sgCopyVec4( ptr, color );
    }

    return true;
}

bool cGrMoon::reposition( sgVec3 p, double angle, double rightAscension, double declination, double moon_dist ) 
{
    sgMat4 T1, T2, GST, RA, DEC;
    sgVec3 axis;
    sgVec3 v;

    sgMakeTransMat4( T1, p );

    sgSetVec3( axis, 0.0, 0.0, -1.0 );
    sgMakeRotMat4( GST, (float)angle, axis );
    sgSetVec3( axis, 0.0, 0.0, 1.0 );
    sgMakeRotMat4( RA, ((float)rightAscension * SGD_RADIANS_TO_DEGREES) - 90.0, axis );
    sgSetVec3( axis, 1.0, 0.0, 0.0 );
    sgMakeRotMat4( DEC, (float)declination * SGD_RADIANS_TO_DEGREES, axis );
    sgSetVec3( v, 0.0, moon_dist, 0.0 );
    sgMakeTransMat4( T2, v );

    sgMat4 TRANSFORM;
    sgCopyMat4( TRANSFORM, T1 );
    sgPreMultMat4( TRANSFORM, GST );
    sgPreMultMat4( TRANSFORM, RA );
    sgPreMultMat4( TRANSFORM, DEC );
    sgPreMultMat4( TRANSFORM, T2 );

    sgCoord skypos;
    sgSetCoord( &skypos, TRANSFORM );

    moon_transform->setTransform( &skypos );

    return true;
}