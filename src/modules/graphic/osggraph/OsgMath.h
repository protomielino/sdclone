/***************************************************************************

    file                     : OsgMath.h
    created                  : Fri Aug 18 00:00:41 CEST 2012
    copyright                : (C) 2012 by Xavier Bertaux
    email                    : bertauxx@yahoo.fr
    version                  : $Id: OsgMath.h 4693 2012-04-13 03:12:09Z torcs-ng $

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _OSGMATH_H_
#define _OSGMATH_H_


typedef float osgMat3[3][3];
typedef float osgMat4[4][4];

struct osgCoord
{
	osg::Vec3 xyz ;
	osg::Vec3 hpr ;
};

extern void osgXformPnt3( osg::Vec3 dst, const osg::Vec3 src, const osgMat4 mat );

inline void osgXformPnt3( osg::Vec3 dst, const osgMat4 mat ) { osgXformPnt3 ( dst, dst, mat ); }

void osgXformPnt3(osg::Vec3 dst, const osg::Vec3 src, const osgMat4 mat);
/*{
	float t0 = src[0];
	float t1 = src[1];
	float t2 = src[2];
	
	dst[0] = t0*mat[0][0] + t1*mat[1][0] + t2*mat[2][0] + mat[3][0];	
	dst[1] = t0*mat[0][1] + t1*mat[1][1] + t2*mat[2][1] + mat[3][1];	
	dst[2] = t0*mat[0][2] + t1*mat[1][2] + t2*mat[2][2] + mat[3][2];
}*/

#endif /* _OSGMATH_H_ */
