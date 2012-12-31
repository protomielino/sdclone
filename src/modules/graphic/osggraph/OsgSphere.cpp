/***************************************************************************

    file        : OsgSphere.cpp
    copyright   : (C) 2012 by Xavier Bertaux (based on SimGear code)
    web         : bertauxx@yahoo.fr
    version     : $Id: OsgSphere.cpp 3162 2012-12-03 13:11:22Z torcs-ng $

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <iostream>
#include <cstdlib>

#include <osg/Node>
#include <osg/Geometry>
#include <osg/Geode>
#include <osg/Array>

#include "OsgMath.h"

osg::Node* SDMakeSphere(double radius, int slices, int stacks)
{
	float rho, drho, dtheta;
    	float s, t, ds, dt;
    	int i, j, imin, imax;
    	float nsign = 1.0;
    	osg::Geode* geode = new osg::Geode;

    	drho = SGD_PI / (float) stacks;
    	dtheta = SGD_2PI / (float) slices;

    	ds = 1.0 / slices;
    	dt = 1.0 / stacks;
    	t = 1.0;  
    	imin = 0;
    	imax = stacks;

    	for ( i = imin; i < imax; i++ ) 
    	{
		osg::Geometry* geometry = new osg::Geometry;
        	osg::Vec3Array* vl = new osg::Vec3Array;
        	osg::Vec3Array* nl = new osg::Vec3Array;
        	osg::Vec2Array* tl = new osg::Vec2Array;

		rho = i * drho;
		s = 0.0;
		for ( j = 0; j <= slices; j++ ) 
		{
	    		double theta = (j == slices) ? 0.0 : j * dtheta;
	    		double x = -sin(theta) * sin(rho);
	    		double y = cos(theta) * sin(rho);
	    		double z = nsign * cos(rho);

            		osg::Vec3 normal(x*nsign, y*nsign, z*nsign);
	    		normal.normalize();
	    		
	    		nl->push_back(normal);
	    		tl->push_back(osg::Vec2(s, t));
	    		vl->push_back(osg::Vec3(x*radius, y*radius, z*radius));

	    		x = -sin(theta) * sin(rho+drho);
	    		y = cos(theta) * sin(rho+drho);
	   		z = nsign * cos(rho+drho);

            		normal = osg::Vec3(x*nsign, y*nsign, z*nsign);
	    		normal.normalize();
	    		
	    		nl->push_back(normal);
	    		tl->push_back(osg::Vec2(s, t-dt));
	    		
	    		s += ds;

	    		vl->push_back(osg::Vec3(x*radius, y*radius, z*radius));
		}

		if ( vl->size() != nl->size() ) 
		{
            		GfOut("bad sphere1\n");
	    		exit(-1);
		}
		
		if ( vl->size() != tl->size() ) 
		{
           		GfOut("bad sphere2\n");
	    		exit(-1);
		}

        	// colors
        	osg::Vec4Array* cl = new osg::Vec4Array;
        	cl->push_back(osg::Vec4(1, 1, 1, 1));

        	geometry->setUseDisplayList(false);
        	geometry->setVertexArray(vl);
        	geometry->setNormalArray(nl);
        	geometry->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
        	geometry->setTexCoordArray(0, tl);
        	geometry->setColorArray(cl);
        	geometry->setColorBinding(osg::Geometry::BIND_OVERALL);
        	geometry->addPrimitiveSet(new osg::DrawArrays(GL_TRIANGLE_STRIP, 0, vl->size()));
        	geode->addDrawable(geometry);

		t -= dt;
    }

    return geode;
}
