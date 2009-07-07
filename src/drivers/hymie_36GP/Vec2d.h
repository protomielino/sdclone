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

// Vec2d.h: interface for the Vec2d class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_VEC2D_H__178BE43A_3B15_4998_A853_97EF3DE7E1FC__INCLUDED_)
#define AFX_VEC2D_H__178BE43A_3B15_4998_A853_97EF3DE7E1FC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <tmath/v2_t.h>
#include <tgf.h>

class Vec2d : public v2t<double>
{
public:
	Vec2d() {}
	Vec2d( const v2t<double>& v ) : v2t<double>(v) {}
	Vec2d( double x, double y ) : v2t<double>(x, y) {};

	Vec2d&	operator=( const v2t<double>& v )
	{
		v2t<double>::operator=(v);
		return *this;
	}
};

#endif // !defined(AFX_VEC2D_H__178BE43A_3B15_4998_A853_97EF3DE7E1FC__INCLUDED_)
