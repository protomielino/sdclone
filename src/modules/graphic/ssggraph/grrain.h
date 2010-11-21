// Precipitation
//
// Written by Xavier Bertaux (based on the sourcecode of simgear), started March 2009.
//
// Copyright (C) 2009  Xavier Bertaux - xavier@torcs-ng.fr
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 2 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//
//	$Id$
//

#ifndef _GRRAIN_H
#define _GRRAIN_H

//SG_USING_STD(vector);
//SG_USING_STD(string);

#include <vector>

/**
 * Precipitation.
 */
class cGrRain
{
private:
	void drawCone(float baseRadius, float height, int slices, bool down, double rain_norm, double speed);
	void lt_update(void);

	bool precipitation_enable_state;
	float precipitation_density;
	double elapsed_time, dt;
	sgVec4	fog_color;
	sgMat4 transform;
	double last_lon, last_lat, last_alt;	
	double min_time_before_lt;

	float fov_width, fov_height;

	static sgVec3 min_light;
	static SGfloat streak_bright_nearmost_layer,
				   streak_bright_farmost_layer,
				   streak_period_max,
				   streak_period_change_per_kms,
				   streak_period_min,
				   streak_length_min,
				   streak_length_change_per_kms,
				   streak_length_max;
	static int streak_count_min, streak_count_max;
	static SGfloat cone_base_radius,
				   cone_height;

public:
	cGrRain();
	~cGrRain();

	void initialize(int rainStrength);
	
	void drawRain(double pitch, double roll, double heading, double hspeed, double rain_norm, int rain);
    /**
     * Draw rain or snow precipitation around the viewer.
     * @param rain_norm rain normalized intensity given by metar class
     * @param snow_norm snow normalized intensity given by metar class
     * @param hail_norm hail normalized intensity given by metar class
     * @param pitch pitch rotation of viewer
     * @param roll roll rotation of viewer
     * @param hspeed moving horizontal speed of viewer in kt
     */

	void drawPrecipitation(int rain, double rain_norm, double pitch, double roll, double heading, double hspeed);

    /**
     * Forward the fog color used by the rain rendering.
     * @param adj_fog_color color of the fog
     */
	
	// rain/snow
	//inline float get_precipitation_density(void) const
		//{ return precipitation_density; }

	//inline bool get_precipitation_enable_state(void) const
		//{ return precipitation_enable_state; }

	/** 
	 * Decrease the precipitation density to the given percentage.
	 * (Only show the given percentage of rain streaks etc.)
	 * Default precipitation density upon construction is 100.0.
	 * @param density 0.0 to 100.0
	 */
	//void set_precipitation_density(float density);
    /**
     * Enable or disable the rendering of precipitation around the viewer.
     * @param enable when true we will draw precipitation depending on metar data
     */
	//void set_precipitation_enable_state(bool enable);   

   	//void setFOV( float w, float h );
	//void getFOV( float &w, float &h );

	//sgMat4 *get_transform(void) { return &transform; }
};

extern cGrRain grRain;

#endif // _GRRAIN_H
