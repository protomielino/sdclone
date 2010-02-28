// grRain.cpp
// (c)2009 Xavier Bertaux - xavier@torcs-ng.fr 
// Based on
// precipitation rendering code by Simgear Team - www.simgear.org

#ifdef WIN32
#include <windows.h>
#endif

#if defined(__APPLE__)
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#include <plib/ssg.h>
#include <plib/ssgAux.h>
#include <plib/ssgaSky.h>
#include "grrain.h"

#include <vector>

//#define MAX_RAIN_SLICE	200
#define MAX_RAIN_SLICE	1000
static float rainpos[MAX_RAIN_SLICE];
#define MAX_LT_TREE_SEG	400

#define DFL_CONE_BASE_RADIUS 10.0
#define DFL_CONE_HEIGHT 35.0
#define DFL_STREAK_BRIGHT_NEARMOST_LAYER 0.9
#define DFL_STREAK_BRIGHT_FARMOST_LAYER 0.4
#define DFL_STREAK_PERIOD_MAX 2.5
#define DFL_STREAK_PERIOD_CHANGE_KMS 0.005
#define DFL_STREAK_PERIOD_MIN 1.0
#define DFL_STREAK_LENGTH_MIN 0.03
#define DFL_STREAK_LENGTH_CHANGE_KMS 0.0005
#define DFL_STREAK_LENGTH_MAX 0.1
#define DFL_STREAK_COUNT_MIN 40
#define DFL_STREAK_COUNT_MAX 190
#define SG_MPH_TO_MPS       0.44704
#define SG_MPS_TO_KT        1.9438444924406046432

#define DFL_MIN_LIGHT 0.35
sgVec3 cGrRain::min_light = {DFL_MIN_LIGHT, DFL_MIN_LIGHT, DFL_MIN_LIGHT};

float cGrRain::streak_period_max = DFL_STREAK_PERIOD_MAX;
float cGrRain::streak_period_change_per_kms = DFL_STREAK_PERIOD_CHANGE_KMS;
float cGrRain::streak_period_min = DFL_STREAK_PERIOD_MIN;
float cGrRain::streak_length_min = DFL_STREAK_LENGTH_MIN;
float cGrRain::streak_length_change_per_kms = DFL_STREAK_LENGTH_CHANGE_KMS;
float cGrRain::streak_bright_nearmost_layer = DFL_STREAK_BRIGHT_NEARMOST_LAYER;
float cGrRain::streak_bright_farmost_layer = DFL_STREAK_BRIGHT_FARMOST_LAYER;
float cGrRain::streak_length_max = DFL_STREAK_LENGTH_MAX;
float cGrRain::cone_base_radius = DFL_CONE_BASE_RADIUS;
float cGrRain::cone_height = DFL_CONE_HEIGHT;
int cGrRain::streak_count_max = DFL_STREAK_COUNT_MAX;
int cGrRain::streak_count_min = DFL_STREAK_COUNT_MIN;

cGrRain grRain;

cGrRain::cGrRain() :
	precipitation_enable_state(true),
	precipitation_density(100.0),
	elapsed_time(5.0),
	dt(1.0),
	min_time_before_lt(0.0),
	fov_width(55.0),
	fov_height(55.0)
{
	for(int i = 0; i < MAX_RAIN_SLICE ; i++)
		rainpos[i] = ssgaRandom();
}


cGrRain::~cGrRain(void) 
{
	//destructor
}


void
cGrRain::DrawCone2(float baseRadius, float height, int slices, bool down, double rain_norm, double speed)
{
	sgVec3 light;
	sgAddVec3( light, fog_color, min_light );
	float da = SG_PI * 2.0f / (float) slices;
	// low number = faster
	float speedf = streak_period_max - speed * streak_period_change_per_kms;
	if( speedf < streak_period_min )
		speedf = streak_period_min;
	float lenf = streak_length_min + speed * streak_length_change_per_kms;
	if( lenf > streak_length_max )
		lenf = streak_length_max;
    float t = fmod((float) elapsed_time, speedf) / speedf;
	//\tab t = 0.1f;
	if( !down )
		t = 1.0f - t;
	float angle = 0.0f;
	glColor4f(1.0f, 0.6f, 0.6f, 0.9f); // XXX unneeded? overriden below
	glBegin(GL_LINES);
	if (slices >  MAX_RAIN_SLICE)
		slices = MAX_RAIN_SLICE; // should never happen
	for( int i = 0 ; i < slices ; i++ )
	{	
		float x = cos(angle) * (baseRadius + rand() % 10);
		float y = sin(angle) * (baseRadius + rand() % 10);
		angle += da;
		sgVec3 dir = {x, -height, y};

		// rain drops at 2 different speed to simulate depth\par
		float t1 = (i & 1 ? t : t + t) + rainpos[i];
		if(t1 > 1.0f)	
			t1 -= 1.0f;
		if(t1 > 1.0f)	
			t1 -= 1.0f;

		// distant raindrops are more transparent
		float c = t1 * (i & 1 ? streak_bright_farmost_layer : streak_bright_nearmost_layer);
		glColor4f(c * light[0], c * light[1], (c * light[2])+ 0.05f, c);
		sgVec3 p1, p2;
		sgScaleVec3(p1, dir, t1);
		// distant raindrops are shorter\par
		float t2 = t1 + (i & 1 ? lenf : lenf+lenf);
		sgScaleVec3(p2, dir, t2);

		glVertex3f(p1[0], p1[1] + height, p1[2]);
		glVertex3f(p2[0], p2[1] + height, p2[2]);
	}
	glEnd();
}

void
cGrRain::drawRain(double pitch, double roll, double heading, double hspeed, double rain_norm, int rain)
{

	#if 0
	static int debug_period = 0;
	if (debug_period++ == 50)
	{
		debug_period = 0;
		cout << "drawRain("
		<< pitch << ", "
		<< roll  << ", "
		<< heading << ", "
		<< hspeed << ", "
		<< rain_norm << ");"
		//" angle = " << angle
		//<< " raindrop(KTS) = " << raindrop_speed_kts
		<< endl;
	}
	#endif

	glBindTexture(GL_TEXTURE_2D, 0);

	glDisable(GL_DEPTH_TEST);
	glShadeModel(GL_SMOOTH);
	glEnable(GL_BLEND);
	glBlendFunc( GL_ONE, GL_ONE_MINUS_SRC_ALPHA );
	glDisable( GL_FOG );
	glDisable(GL_LIGHTING);

	int slice_count = static_cast<int>((streak_count_min + rain_norm * (streak_count_max - streak_count_min)) * (precipitation_density / 100.0) * rain);

	double raindrop_speed_kts = (5.0 + rain_norm * 15.0) * SG_MPH_TO_MPS * SG_MPS_TO_KT;

	float angle = atanf(hspeed / raindrop_speed_kts) * SG_RADIANS_TO_DEGREES;
	glPushMatrix();

	// the cone rotate with hspeed
	angle = -pitch - angle;
	glRotatef(roll, 0.0, 0.0, 1.0);
	glRotatef(heading, 0.0, 1.0, 0.0);
	glRotatef(angle, 1.0, 0.0, 0.0);

	// up cone
	DrawCone2(cone_base_radius, cone_height, slice_count, true, rain_norm, hspeed);

	// down cone (usually not visible)
	//if(angle > 0.0 || heading != 0.0)
	if(angle > 0.0)
		DrawCone2(cone_base_radius, -cone_height, slice_count, false, rain_norm, hspeed);

	glPopMatrix();

	glEnable(GL_LIGHTING);
	glBlendFunc ( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA ) ;
	glEnable( GL_FOG );
	glEnable(GL_DEPTH_TEST);
}


void cGrRain::drawPrecipitation(int rain, double rain_norm, double pitch, double roll, double heading, double hspeed)
{
	drawRain(pitch, roll, heading, hspeed, rain_norm, rain);
}
