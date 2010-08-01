// grSky.h

#ifndef _GRSKY_H_
#define _GRSKY_H_

#include "plib/ssg.h"
//#include "plib/ssgAux.h"

class grCelestialBody;
class grCelestialBodyList;
class grCloudLayer;
class grCloudLayerList;
class grStars;
class grSkyDome;
class grSky;
class grMoon;
class grSun;


class grCelestialBody
{
private:

  ssgTransform *transform;
  ssgColourArray *cl;

  // used by repaint for rise/set effects
  double body_angle;
  double body_rotation;

  // used by reposition
  double body_right_ascension;
  double body_declination;
  double body_dist;

public:

  grCelestialBody( void );
  ~grCelestialBody( void );

  ssgBranch *build( const char* body_tex_path, const char* halo_tex_path, double body_size );
  ssgBranch *build( ssgSimpleState *orb_state, ssgSimpleState *halo_state, double body_size );

  bool reposition( sgVec3 p, double angle ) 
  {
    return reposition ( p, angle, body_right_ascension, body_declination, body_dist ); 
  }
  bool reposition( sgVec3 p, double angle, double rightAscension, double declination, double dist );

  bool repaint() { return repaint ( body_angle ); }
  bool repaint( double angle );

  void getPosition ( sgCoord* p )
  {
	sgMat4 Xform;
	transform->getTransform(Xform);
	sgSetCoord(p, Xform);
  }

  void setAngle ( double angle ) { body_angle = angle; }
  double getAngle () { return body_angle; }

  void setRotation ( double rotation ) { body_rotation = rotation; }
  double getRotation () { return body_rotation; }

  void setRightAscension ( double ra ) { body_right_ascension = ra; }
  double getRightAscension () { return body_right_ascension; }

  void setDeclination ( double decl ) { body_declination = decl; }
  double getDeclination () { return body_declination; }

  void setDist ( double dist ) { body_dist = dist; }
  double getDist () { return body_dist; }

  inline float *getColor() { return  cl->get( 0 ); }
} ;


class grCelestialBodyList : private ssgSimpleList
{
public:

  grCelestialBodyList ( int init = 3 )
	  : ssgSimpleList ( sizeof(grCelestialBody*), init ) { }

  ~grCelestialBodyList () { removeAll(); }

  int getNum (void) { return total ; }

  grCelestialBody* get ( unsigned int n )
  {
    assert(n<total);
    return *( (grCelestialBody**) raw_get ( n ) ) ;
  }

  void add ( grCelestialBody* item ) { raw_add ( (char *) &item ) ;}

  void removeAll ()
  {
    for ( int i = 0; i < getNum (); i++ )
      delete get (i) ;
    ssgSimpleList::removeAll () ;
  }
} ;


class grCloudLayer
{
private:

  ssgRoot *layer_root;
  ssgTransform *layer_transform;
  ssgLeaf *layer[4];

  ssgColourArray *cl[4]; 
  ssgVertexArray *vl[4];
  ssgTexCoordArray *tl[4];

  bool enabled;
  float layer_span;
  float layer_asl;
  float layer_thickness;
  float layer_transition;
  float scale;
  float speed;
  float direction;

  double last_lon, last_lat;
  double last_x, last_y;

public:

  grCloudLayer( void );
  ~grCloudLayer( void );

  void build( const char *cloud_tex_path, float span, float elevation, float thickness, float transition );
  void build( ssgSimpleState *cloud_state, float span, float elevation, float thickness, float transition );

  bool repositionFlat( sgVec3 p, double dt );
  bool reposition( sgVec3 p, sgVec3 up, double lon, double lat, double alt, double dt );

  bool repaint( sgVec3 fog_color );

  void draw();

  void enable() { enabled = true; }
  void disable() { enabled = false; }
  bool isEnabled() { return enabled; }

  float getElevation () { return layer_asl; }
  void  setElevation ( float elevation ) { layer_asl = elevation; }

  float getThickness () { return layer_thickness; }
  void  setThickness ( float thickness ) { layer_thickness = thickness; }

  float getTransition () { return layer_transition; }
  void  setTransition ( float transition ) { layer_transition = transition; }

  float getSpeed () { return speed; }
  void  setSpeed ( float val ) { speed = val; }

  float getDirection () { return direction; }
  void  setDirection ( float val ) { direction = val; }
};


class grCloudLayerList : private ssgSimpleList
{
public:

  grCloudLayerList ( int init = 3 )
	  : ssgSimpleList ( sizeof(grCloudLayer*), init ) { }

  ~grCloudLayerList () { removeAll(); }

  int getNum (void) { return total ; }

  grCloudLayer* get ( unsigned int n )
  {
    assert(n<total);
    return *( (grCloudLayer**) raw_get ( n ) ) ;
  }

  void add ( grCloudLayer* item ) { raw_add ( (char *) &item ) ;}

  void removeAll ()
  {
    for ( int i = 0; i < getNum (); i++ )
      delete get (i) ;
    ssgSimpleList::removeAll () ;
  }
} ;


class grStars
{
private:

  ssgTransform *stars_transform;
  ssgSimpleState *state;

  ssgColourArray *cl;
  ssgVertexArray *vl;

  int old_phase;  // data for optimization

public:

  grStars( void );
  ~grStars( void );

  ssgBranch *build( int num, sgdVec3 *star_data, double star_dist );

  bool reposition( sgVec3 p, double angle );

  bool repaint( double sol_angle, int num, sgdVec3 *star_data );
};


class grSkyDome
{
private:

  ssgTransform *dome_transform;
  ssgSimpleState *dome_state;

  ssgVertexArray *center_disk_vl;
  ssgColourArray *center_disk_cl;

  ssgVertexArray *upper_ring_vl;
  ssgColourArray *upper_ring_cl;

  ssgVertexArray *middle_ring_vl;
  ssgColourArray *middle_ring_cl;

  ssgVertexArray *lower_ring_vl;
  ssgColourArray *lower_ring_cl;
  float asl;

public:

  grSkyDome( void );
  ~grSkyDome( void );

  ssgBranch *build( double hscale = 80000.0, double vscale = 80000.0 );

  bool repositionFlat( sgVec3 p, double spin );
  bool reposition( sgVec3 p, double lon, double lat, double spin );

  bool repaint( sgVec3 sky_color, sgVec3 fog_color, double sol_angle, double vis );
};


class grSky
{
private:

  // components of the sky
  grSkyDome *dome;
  grCelestialBody* sol_ref;
  grCelestialBodyList bodies;
  grCloudLayerList clouds;
  grStars *planets;
  grStars *stars;

  ssgRoot *pre_root, *post_root;

  ssgSelector *pre_selector, *post_selector;
  ssgTransform *pre_transform, *post_transform;
  ssgTransform *bodies_transform, *stars_transform;

  // visibility
  float visibility;
  float effective_visibility;

  // near cloud visibility state variables
  bool in_puff;
  double puff_length;       // in seconds
  double puff_progression;  // in seconds
  double ramp_up;           // in seconds
  double ramp_down;         // in seconds

public:

  grSky( void );
  ~grSky( void );

  void build( double h_radius, double v_radius,
	  int nplanets, sgdVec3 *planet_data,
	  int nstars, sgdVec3 *star_data);

  grCelestialBody* addBody( const char *body_tex_path, const char *halo_tex_path, double size, double dist, bool sol = false );
  grCelestialBody* addBody( ssgSimpleState *orb_state, ssgSimpleState *halo_state, double size, double dist, bool sol = false );
  grCelestialBody* getBody(int i) { return bodies.get(i); }
  int getBodyCount() { return bodies.getNum(); }

  grCloudLayer* addCloud( const char *cloud_tex_path, float span, float elevation, float thickness, float transition );
  grCloudLayer* addCloud( ssgSimpleState *cloud_state, float span, float elevation, float thickness, float transition );
  grCloudLayer* getCloud(int i) { return clouds.get(i); }
  int getCloudCount() { return clouds.getNum(); }

  bool repositionFlat( sgVec3 view_pos, double spin, double dt );
  bool reposition( sgVec3 view_pos, sgVec3 zero_elev, sgVec3 view_up, double lon, double lat, double alt, double spin, double gst, double dt );

  bool repaint( sgVec4 sky_color, sgVec4 fog_color, sgVec4 cloud_color, double sol_angle,
	  int nplanets, sgdVec3 *planet_data,
	  int nstars, sgdVec3 *star_data );

  // modify visibility based on cloud layers, thickness, transition range, and simulated "puffs".
  void modifyVisibility( float alt, float time_factor );

  // draw background portions of sky (do this before you draw rest of your scene).
  void preDraw();

  // draw translucent clouds (do this after you've drawn all oapaque elements of your scene).
  void postDraw( float alt );

  // enable the sky
  inline void enable() {
    pre_selector->select( 1 );
    post_selector->select( 1 );
  }

  // disable the sky
  inline void disable() {
    pre_selector->select( 0 );
    post_selector->select( 0 );
  }

  // current effective visibility
  inline float getVisibility() const { return effective_visibility; }
  inline void setVisibility( float v ) {
    effective_visibility = visibility = v;
  }
} ;


// return a random number between [0.0, 1.0)
inline double grRandom(void)
{
  return(rand() / (double)RAND_MAX);
}

//#if defined( macintosh )
//const float system_gamma = 1.4;
//#elif defined (sgi)
//const float system_gamma = 1.7;
//#else	// others
const float system_gamma = 2.5;
//#endif

// simple architecture independant gamma correction function.
inline void grGammaCorrectRGB(float *color, float reff = 2.5, float system = system_gamma)
{
  color[0] = (float)pow(color[0], reff/system);
  color[1] = (float)pow(color[1], reff/system);
  color[2] = (float)pow(color[2], reff/system);
};

inline void grGammaCorrectC(float *color, float reff = 2.5, float system = system_gamma)
{
  *color = (float)pow(*color, reff/system);
};

inline void grGammaRestoreRGB(float *color, float reff = 2.5, float system = system_gamma)
{
  color[0] = (float)pow(color[0], system/reff);
  color[1] = (float)pow(color[1], system/reff);
  color[2] = (float)pow(color[2], system/reff);
};

inline void grGammaRestoreC(float *color, float reff = 2.5, float system = system_gamma)
{
  *color = (float)pow(*color, system/reff);
};

class grMoon 
{
    ssgTransform *moon_transform;
    ssgSimpleState *orb_state;
    ssgSimpleState *halo_state;

    ssgColourArray *cl;

    ssgVertexArray *halo_vl;
    ssgTexCoordArray *halo_tl;

    double prev_moon_angle;
    double moon_angle;
    double moon_rotation;

    // used by reposition
    double moon_right_ascension;
    double moon_declination;
    double moon_dist;

public:

    // Constructor
    grMoon( void );

    // Destructor
    ~grMoon( void );

    // build the moon object
    ssgBranch *build( const char* body_tex_path, double moon_size );

    // repaint the moon colors based on current value of moon_anglein
    // degrees relative to verticle
    // 0 degrees = high noon
    // 90 degrees = moon rise/set
    // 180 degrees = darkest midnight

    //bool repaint(double moon_angle);

    /*bool reposition( sgVec3 p, double angle,
		     double rightAscension, double declination,
		     double moon_dist  );*/

    /*bool reposition(sgVec3 p, double moon_angle) 
    {
       return reposition (p, moon_angle, rightAscension, declination, moon_dist); 
    }*/

    bool reposition(sgVec3 p, double moon_angle, double rightAscension, double declination, double moon_dist);

    /*bool repaint() 
    { 
	return repaint (moon_angle); 
    }*/
    bool repaint(double moon_angle);

    void getPosition (sgCoord* p)
    {
	sgMat4 Xform;
	moon_transform->getTransform(Xform);
	sgSetCoord(p, Xform);
    }

    void setAngle (double angle) 
    { 
	moon_angle = angle; 
    }
    double getAngle () 
    { 
	return moon_angle; 
    }

    void setRotation (double rotation) 
    { 
	moon_rotation = rotation; 
    }
    double getRotation () 
    { 
	return moon_rotation; 
    }

    void setRightAscension (double ra) 
    { 
	moon_right_ascension = ra; 
    }
    double getRightAscension () 
    { 
	return moon_right_ascension; 
    }

    void setDeclination ( double decl ) 
    { 
	moon_declination = decl; 
    }
    double getDeclination () 
    { 
	return moon_declination; 
    }

    void setDist ( double dist ) 
    { 
 	moon_dist = dist; 
    }
    double getDist () 
    { 
	return moon_dist; 
    }

    inline float *getColor() 
    { 
       return  cl->get( 0 ); 
    }
};

/*class grSun 
{
    ssgTransform *sun_transform;
    ssgSimpleState *sun_state; 
    ssgSimpleState *ihalo_state;
    ssgSimpleState *ohalo_state;

    ssgColourArray *sun_cl;
    ssgColourArray *ihalo_cl;
    ssgColourArray *ohalo_cl;

    ssgVertexArray *sun_vl;
    ssgVertexArray *ihalo_vl;
    ssgVertexArray *ohalo_vl;

    ssgTexCoordArray *sun_tl;
    ssgTexCoordArray *ihalo_tl;
    ssgTexCoordArray *ohalo_tl;

    GLuint sun_texid;
    GLubyte *sun_texbuf;

    double visibility;
    double prev_sun_angle;
    // distance of light traveling through the atmosphere
    double path_distance;

public:

    // Constructor
    grSun( void );

    // Destructor
    ~grSun( void );

    // return the sun object
    ssgBranch *build( const char* body_tex_path, const char* halo_tex_path, double sun_size);

    // repaint the sun colors based on current value of sun_anglein
    // degrees relative to verticle
    // 0 degrees = high noon
    // 90 degrees = sun rise/set
    // 180 degrees = darkest midnight
    bool repaint( double sun_angle, double new_visibility );

    // reposition the sun at the specified right ascension and
    // declination, offset by our current position (p) so that it
    // appears fixed at a great distance from the viewer.  Also add in
    // an optional rotation (i.e. for the current time of day.)

    /*bool reposition( sgVec3 p, double angle,
		     double rightAscension, double declination,
		     double sun_dist, double lat, double alt_asl, double sun_angle );*/

 /*   bool reposition( sgVec3 p, double sun_angle ) 
    {
       return reposition ( p, angle, sun_right_ascension, sun_declination, sun_body_dist ); 
    }

    bool reposition( sgVec3 p, double sun_angle, double rightAscension, double declination, double dist );

    // retrun the current color of the sun
    inline float *get_color() { return  ohalo_cl->get( 0 ); }

    // return the texture id of the sun halo texture
    inline GLuint get_texture_id() { return ohalo_state->getTextureHandle(); }
};*/


#endif
