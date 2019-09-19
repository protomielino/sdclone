/***************************************************************************

    file        : Vec2d.h
    created     : 9 Apr 2006
    copyright   : (C) 2006 Tim Foden

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _VEC2D_H_
#define _VEC2D_H_

#include <v2_t.h>
#include <tgf.h>

class Vec2d : public v2t<double>
{
public:
    Vec2d() : v2t(0, 0) {}
    Vec2d( const v2t<double>& v ) : v2t<double>(v) {}
    explicit Vec2d( const v3t<double>& v ) : v2t<double>(v.x, v.y) {}
    Vec2d( const tPosd& p ) : v2t<double>(p.x, p.y) {}
    Vec2d( double x, double y ) : v2t<double>(x, y) {};
//	Vec2d( double angle ) : v2t<double>(cos(angle), sin(angle)) {};

    Vec2d&	operator=( const v2t<double>& v )
    {
        v2t<double>::operator=(v);
        return *this;
    }

    // cross product
    double	operator%( const v2t<double>& v ) const
    {
        return x * v.y - y * v.x;
    }

    double	GetAngle() const
    {
        return atan2(y, x);
    }

    Vec2d	GetUnit() const
    {
        if( x == 0 && y == 0 )
            return Vec2d();

        double	len = hypot(x, y);
        return Vec2d(x / len, y / len);
    }

    Vec2d	GetNormal() const
    {
        return Vec2d(-y, x);
    }

    double	DistSq( const Vec2d& other ) const
    {
        double dx = other.x - x;
        double dy = other.y - y;
        return dx * dx + dy * dy;
    }
};

#endif
