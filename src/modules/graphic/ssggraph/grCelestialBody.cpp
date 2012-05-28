/***************************************************************************

    file        : grCelestiaBody.cpp
    copyright   : (C) 2009 by Xavier Bertaux (based on ssgasky plib code)
    web         : http://www.speed-dreams.org
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

#include <math.h>
#include "grSky.h"
#include "grSphere.h"

static double sun_exp2_punch_through;
static double visibility;

static int grCelestialBodyOrbPreDraw( ssgEntity *e )
{
  ssgLeaf *f = (ssgLeaf *)e;
  if ( f -> hasState () ) f->getState()->apply() ;

  glPushAttrib( GL_COLOR_BUFFER_BIT | GL_ENABLE_BIT );

  glDisable( GL_FOG );
  glBlendFunc ( GL_SRC_ALPHA, GL_ONE ) ;

  return true;
}


static int grCelestialBodyOrbPostDraw( ssgEntity *e )
{
  glPopAttrib();

  return true;
}


static int grCelestialBodyHaloPreDraw( ssgEntity *e )
{
  ssgLeaf *f = (ssgLeaf *)e;
  if ( f -> hasState () ) f->getState()->apply() ;

  glPushAttrib( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_FOG_BIT );

  glDisable( GL_DEPTH_TEST );
  glDisable( GL_FOG );
  glBlendFunc ( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA ) ;

  return true;
}


static int grCelestialBodyHaloPostDraw( ssgEntity *e )
{
  glPopAttrib();

  return true;
}


cGrCelestialBody::cGrCelestialBody( void )
{
  transform = 0;
  body_angle = 0;
  body_rotation = 0;
  body_right_ascension = 0;
  body_declination = 0;
  body_dist = 0;
}


cGrCelestialBody::~cGrCelestialBody( void )
{
  ssgDeRefDelete( transform );
}


ssgBranch * cGrCelestialBody::build( const char* body_tex_path, const char* innerhalo_path, const char *outerhalo_path, double body_size )
{
  ssgSimpleState *orb_state = NULL;
  ssgSimpleState *ihalo_state = NULL;
  ssgSimpleState *ohalo_state = NULL;

  // set up the orb state
  orb_state = new ssgSimpleState();
  if (body_tex_path) 
  {
    orb_state->setTexture( body_tex_path );
    orb_state->setShadeModel( GL_SMOOTH );
    orb_state->enable( GL_LIGHTING );
    orb_state->enable( GL_CULL_FACE );
    orb_state->enable( GL_TEXTURE_2D );
    orb_state->enable( GL_COLOR_MATERIAL );
    orb_state->setColourMaterial( GL_DIFFUSE );
    orb_state->setMaterial( GL_AMBIENT, 0, 0, 0, 1 );
    orb_state->setMaterial( GL_EMISSION, 0, 0, 0, 1 );
    orb_state->setMaterial( GL_SPECULAR, 0, 0, 0, 1 );
    orb_state->enable( GL_BLEND );
    orb_state->enable( GL_ALPHA_TEST );
    orb_state->setAlphaClamp( 0.01f );
  }
  else 
  {
    orb_state->setShadeModel( GL_SMOOTH );
    orb_state->disable( GL_LIGHTING );
    orb_state->enable( GL_CULL_FACE );
    orb_state->disable( GL_TEXTURE_2D );
    orb_state->enable( GL_COLOR_MATERIAL );
    orb_state->setColourMaterial( GL_AMBIENT_AND_DIFFUSE );
    orb_state->setMaterial( GL_EMISSION, 0, 0, 0, 1 );
    orb_state->setMaterial( GL_SPECULAR, 0, 0, 0, 1 );
    orb_state->disable( GL_BLEND );
    orb_state->disable( GL_ALPHA_TEST );
  }

  if (innerhalo_path) 
  {
    ihalo_state = new ssgSimpleState();
    ihalo_state->setTexture( innerhalo_path );
    ihalo_state->enable( GL_TEXTURE_2D );
    ihalo_state->disable( GL_LIGHTING );
    ihalo_state->setShadeModel( GL_SMOOTH );
    ihalo_state->disable( GL_CULL_FACE );
    ihalo_state->enable( GL_COLOR_MATERIAL );
    ihalo_state->setColourMaterial( GL_AMBIENT_AND_DIFFUSE );
    ihalo_state->setMaterial( GL_EMISSION, 0, 0, 0, 1 );
    ihalo_state->setMaterial( GL_SPECULAR, 0, 0, 0, 1 );
    ihalo_state->enable( GL_ALPHA_TEST );
    ihalo_state->setAlphaClamp(0.01f);
    ihalo_state->enable ( GL_BLEND );

	ohalo_state = new ssgSimpleState();
    ohalo_state->setTexture( outerhalo_path );
    ohalo_state->enable( GL_TEXTURE_2D );
    ohalo_state->disable( GL_LIGHTING );
    ohalo_state->setShadeModel( GL_SMOOTH );
    ohalo_state->disable( GL_CULL_FACE );
    ohalo_state->enable( GL_COLOR_MATERIAL );
    ohalo_state->setColourMaterial( GL_AMBIENT_AND_DIFFUSE );
    ohalo_state->setMaterial( GL_EMISSION, 0, 0, 0, 1 );
    ohalo_state->setMaterial( GL_SPECULAR, 0, 0, 0, 1 );
    ohalo_state->enable( GL_ALPHA_TEST );
    ohalo_state->setAlphaClamp(0.01f);
    ohalo_state->enable ( GL_BLEND );
  }


  return build( orb_state, ihalo_state, ohalo_state, body_size );
}


ssgBranch * cGrCelestialBody::build( ssgSimpleState *orb_state, ssgSimpleState *ihalo_state, ssgSimpleState *ohalo_state, double body_size )
{
  ssgVertexArray *ihalo_vl;
  ssgTexCoordArray *ihalo_tl;
  ssgVertexArray *ohalo_vl;
  ssgTexCoordArray *ohalo_tl;

  // clean-up previous
  ssgDeRefDelete( transform );

  // build the ssg scene graph sub tree for the sky and connected
  // into the provide scene graph branch
  transform = new ssgTransform;
  transform->ref();

  cl = new ssgColourArray( 1 );
  ihalo_cl = new ssgColourArray( 1 );
  ohalo_cl = new ssgColourArray( 1 );
  sgVec4 color;
  sgSetVec4( color, 1.0, 1.0, 1.0, 1.0 );
  cl->add( color );
  ihalo_cl->add( color );
  ohalo_cl->add( color );

  ssgBranch *orb = grMakeSphere( orb_state, cl, (float)body_size, 15, 15, 
    grCelestialBodyOrbPreDraw, grCelestialBodyOrbPostDraw );

  transform->addKid( orb );

  // force a repaint of the colors with arbitrary defaults
  repaint( 0.0 );

  if (ihalo_state) 
  {
    // Build ssg structure
    float size = float(body_size * 8.0);
    sgVec3 v3;
    ihalo_vl = new ssgVertexArray;
    sgSetVec3( v3, -size, 0.0, -size );
    ihalo_vl->add( v3 );
    sgSetVec3( v3, size, 0.0, -size );
    ihalo_vl->add( v3 );
    sgSetVec3( v3, -size, 0.0,  size );
    ihalo_vl->add( v3 );
    sgSetVec3( v3, size, 0.0,  size );
    ihalo_vl->add( v3 );

    sgVec2 v2;
    ihalo_tl = new ssgTexCoordArray;
    sgSetVec2( v2, 0.0f, 0.0f );
    ihalo_tl->add( v2 );
    sgSetVec2( v2, 1.0, 0.0 );
    ihalo_tl->add( v2 );
    sgSetVec2( v2, 0.0, 1.0 );
    ihalo_tl->add( v2 );
    sgSetVec2( v2, 1.0, 1.0 );
    ihalo_tl->add( v2 );

    ssgLeaf *ihalo = new ssgVtxTable ( GL_TRIANGLE_STRIP, ihalo_vl, NULL, ihalo_tl, ihalo_cl );
    ihalo->setState( ihalo_state );

    ihalo->setCallback( SSG_CALLBACK_PREDRAW, grCelestialBodyHaloPreDraw );
    ihalo->setCallback( SSG_CALLBACK_POSTDRAW, grCelestialBodyHaloPostDraw );

    transform->addKid( ihalo );
    //transform->addKid(new ssgaLensFlare);

	size = float(body_size * 2.0);
    sgVec3 v4;
    ohalo_vl = new ssgVertexArray;
    sgSetVec3( v4, -size, 0.0, -size );
    ohalo_vl->add( v4 );
    sgSetVec3( v4, size, 0.0, -size );
    ohalo_vl->add( v4 );
    sgSetVec3( v4, -size, 0.0,  size );
    ohalo_vl->add( v4 );
    sgSetVec3( v4, size, 0.0,  size );
    ohalo_vl->add( v4 );

    sgVec2 v5;
    ohalo_tl = new ssgTexCoordArray;
    sgSetVec2( v5, 0.0f, 0.0f );
    ohalo_tl->add( v5 );
    sgSetVec2( v5, 1.0, 0.0 );
    ohalo_tl->add( v5 );
    sgSetVec2( v5, 0.0, 1.0 );
    ohalo_tl->add( v5 );
    sgSetVec2( v5, 1.0, 1.0 );
    ohalo_tl->add( v5 );

    ssgLeaf *ohalo = new ssgVtxTable ( GL_TRIANGLE_STRIP, ohalo_vl, NULL, ohalo_tl, ohalo_cl );
    ohalo->setState( ohalo_state );

    ohalo->setCallback( SSG_CALLBACK_PREDRAW, grCelestialBodyHaloPreDraw );
    ohalo->setCallback( SSG_CALLBACK_POSTDRAW, grCelestialBodyHaloPostDraw );

    transform->addKid( ohalo );
  }

  return transform;
}


bool cGrCelestialBody::reposition( sgVec3 p, double angle, double rightAscension, double declination, double sol_dist )
{
  sgMat4 T1, T2, GST, RA, DEC;
  sgVec3 axis;
  sgVec3 v;
  static double prev_sun_angle;

  sgMakeTransMat4( T1, p );

  sgSetVec3( axis, 0.0, 0.0, -1.0 );
  sgMakeRotMat4( GST, (float)angle, axis );

  sgSetVec3( axis, 0.0, 0.0, 1.0 );
  sgMakeRotMat4( RA, (float)((rightAscension * SGD_RADIANS_TO_DEGREES) - 90.0), axis );

  sgSetVec3( axis, 1.0, 0.0, 0.0 );
  sgMakeRotMat4( DEC, (float)(declination * SGD_RADIANS_TO_DEGREES), axis );

  sgSetVec3( v, 0.0, (float)sol_dist, 0.0 );
  sgMakeTransMat4( T2, v );

  sgMat4 TRANSFORM;
  sgCopyMat4( TRANSFORM, T1 );
  sgPreMultMat4( TRANSFORM, GST );
  sgPreMultMat4( TRANSFORM, RA );
  sgPreMultMat4( TRANSFORM, DEC );
  sgPreMultMat4( TRANSFORM, T2 );

  sgCoord skypos;
  sgSetCoord( &skypos, TRANSFORM );

  transform->setTransform( &skypos );

  if ( prev_sun_angle != angle ) 
  {
      if ( angle == 0 ) angle = 0.1;
      const double r_earth_pole = 6356752.314;
      const double r_tropo_pole = 6356752.314 + 8000;
      const double epsilon_earth2 = 6.694380066E-3;
      const double epsilon_tropo2 = 9.170014946E-3;

      double r_tropo = r_tropo_pole / sqrt ( 1 - ( epsilon_tropo2 * pow ( 0.0, 2 )));
      double r_earth = r_earth_pole / sqrt ( 1 - ( epsilon_earth2 * pow ( 0.0, 2 )));
 
      double position_radius = r_earth;

      double gamma =  SG_PI - angle;
      double sin_beta =  ( position_radius * sin ( gamma )  ) / r_tropo;
      double alpha =  SG_PI - gamma - asin( sin_beta );
      
	  double path_distance = sqrt( pow( position_radius, 2 ) + pow( r_tropo, 2 )
                        - ( 2 * position_radius * r_tropo * cos( alpha ) ));

      double alt_half = sqrt( pow ( r_tropo, 2 ) + pow( path_distance / 2, 2 ) - r_tropo * path_distance * cos( asin( sin_beta )) ) - r_earth;

      if ( alt_half < 0.0 ) alt_half = 0.0;
  }

  return true;
}


bool cGrCelestialBody::repaint( double angle )
{
	 static double prev_angle = 9999.0;
	 static double path_distance = getDist ();
	 //double effective_visibility = getVisibility();
	 //static int visibility = 0;

	if ( visibility != effective_visibility ) 
	{
		visibility = effective_visibility;

        static const double sqrt_m_log01 = sqrt( -log( 0.01 ) );
        sun_exp2_punch_through = sqrt_m_log01 / ( visibility * 15 );
    }

  if (prev_angle != angle) 
  {
    prev_angle = angle;

    double factor = 4*cos(angle);
	//double factor;

	if ( visibility < 100 )
	{
		factor = 8000;
	}
	else 
	{
		float vis2 = visibility / 100;
        factor = 80.5 / log( vis2 );
	}

    if (factor > 1) factor = 1.0;
    if (factor < -1) factor = -1.0;
    factor = factor / 2 + 0.5f;

	double rel_humidity = 0.5;
	double density_avg = 0.7;

    //sgVec4 color;
	sgVec4 i_halo_color, o_halo_color, sun_color;
    sun_color[0] = (float)pow(factor, 0.25);
    sun_color[1] = (float)pow(factor, 0.50);
    sun_color[2] = (float)pow(factor, 4.0);
    sun_color[3] = 1.0;

	double red_scat_f = ( factor * path_distance * density_avg ) / 5E+07;
	sun_color[0] = 1 - red_scat_f;
	i_halo_color[0] = 1 - ( 1.1 * red_scat_f );
	o_halo_color[0] = 1 - ( 1.4 * red_scat_f );

	// Green - 546.1 nm
	double green_scat_f = ( factor * path_distance * density_avg ) / 8.8938E+06;
	sun_color[1] = 1 - green_scat_f;
	i_halo_color[1] = 1 - ( 1.1 * green_scat_f );
	o_halo_color[1] = 1 - ( 1.4 * green_scat_f );
 
	// Blue - 435.8 nm
	double blue_scat_f = ( factor * path_distance * density_avg ) / 3.607E+06;
	sun_color[2] = 1 - blue_scat_f;
	i_halo_color[2] = 1 - ( 1.1 * blue_scat_f );
	o_halo_color[2] = 1 - ( 1.4 * blue_scat_f );

	// Alpha
	sun_color[3] = 1;
	i_halo_color[3] = 1;

	o_halo_color[3] = blue_scat_f; 
	if ( ( visibility < 10000 ) &&  ( blue_scat_f > 1 ))
	{
		o_halo_color[3] = 2 - blue_scat_f; 
	}

	double saturation = 1 - ( rel_humidity / 200 );
	sun_color[1] += (( 1 - saturation ) * ( 1 - sun_color[1] ));
	sun_color[2] += (( 1 - saturation ) * ( 1 - sun_color[2] ));

	i_halo_color[1] += (( 1 - saturation ) * ( 1 - i_halo_color[1] ));
	i_halo_color[2] += (( 1 - saturation ) * ( 1 - i_halo_color[2] )); 

	o_halo_color[1] += (( 1 - saturation ) * ( 1 - o_halo_color[1] )); 
	o_halo_color[2] += (( 1 - saturation ) * ( 1 - o_halo_color[2] ));

	// just to make sure we're in the limits
	if ( sun_color[0] < 0 ) sun_color[0] = 0;
	else if ( sun_color[0] > 1) sun_color[0] = 1;
	if ( i_halo_color[0] < 0 ) i_halo_color[0] = 0;
	else if ( i_halo_color[0] > 1) i_halo_color[0] = 1;
	if ( o_halo_color[0] < 0 ) o_halo_color[0] = 0;
	else if ( o_halo_color[0] > 1) o_halo_color[0] = 1;

	if ( sun_color[1] < 0 ) sun_color[1] = 0;
	else if ( sun_color[1] > 1) sun_color[1] = 1;
	if ( i_halo_color[1] < 0 ) i_halo_color[1] = 0;
	else if ( i_halo_color[1] > 1) i_halo_color[1] = 1;
	if ( o_halo_color[1] < 0 ) o_halo_color[1] = 0;
	else if ( o_halo_color[1] > 1) o_halo_color[1] = 1;

	if ( sun_color[2] < 0 ) sun_color[2] = 0;
	else if ( sun_color[2] > 1) sun_color[2] = 1;
	if ( i_halo_color[2] < 0 ) i_halo_color[2] = 0;
	else if ( i_halo_color[2] > 1) i_halo_color[2] = 1;
	if ( o_halo_color[2] < 0 ) o_halo_color[2] = 0;
	else if ( o_halo_color[2] > 1) o_halo_color[2] = 1;
	if ( o_halo_color[3] < 0 ) o_halo_color[2] = 0;
	else if ( o_halo_color[3] > 1) o_halo_color[3] = 1;

        
	grGammaCorrectRGB( i_halo_color );
	grGammaCorrectRGB( o_halo_color );
    grGammaCorrectRGB( sun_color );

    float *ptr;
    ptr = cl->get( 0 );
    sgCopyVec4( ptr, sun_color );

	ptr = ihalo_cl->get( 0 );
	sgCopyVec4( ptr, i_halo_color );
	ptr = ohalo_cl->get( 0 );
	sgCopyVec4( ptr, o_halo_color );
  }

  return true;
}
