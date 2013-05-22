/***************************************************************************

    file                 : OsgCar.h
    created              : Mon Aug 21 18:24:02 CEST 2012
    copyright            : (C) 2012 by Gaétan André
    email                : gaetan.andré@gmail.com
    version              : $Id$

***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _OSGCAR_H_
#define _OSGCAR_H_

#include <car.h>
#include <raceman.h>
#include <vector>

#include "OsgWheel.h"

class SDCarShader;

class SDCar
{
    private :
        osg::ref_ptr<osg::MatrixTransform> car_branch;
        tCarElt *car;
        SDWheels wheels;
        SDCarShader * shader;
        osg::ref_ptr<osg::Vec3Array> shadowVertices;
     //   osg::ref_ptr<osg::MatrixTransform> wheels[4];
       // osg::ref_ptr<osg::MatrixTransform> initWheel(int wheelIndec, const char *wheel_mod_name);
        osg::ref_ptr<osg::Node> initOcclusionQuad(tCarElt *car);

    public :
        osg::ref_ptr<osg::Node> loadCar(tCarElt *car);
        void updateCar();
        void updateShadingParameters(osg::Matrixf modelview);
};

class SDCars
{
    private :
        std::vector<SDCar *> the_cars;
        osg::ref_ptr<osg::Group> cars_branch;
        tSituation * situation;

        void addSDCar(SDCar * car);

    public :
		SDCars(void);
		~SDCars(void);

        void loadCars(tSituation * pSituation);
        void updateCars();
        void unLoad();
        void updateShadingParameters(osg::Matrixf modelview);
        osg::Node* getCarsNode(){ return cars_branch.get(); }
};

#endif /* _OSGCAR_H_ */
