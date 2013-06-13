/***************************************************************************

    file        : OsgShader.h
    created     : Sat Feb 2013 15:52:19 CEST 2013
    copyright   : (C) 2013 by Gaëtan André
    email       : gaetan.andre@gmail.com
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

class SDCarShader{
private :

    osg::ref_ptr<osg::Program> program ;

    osg::ref_ptr<osg::Node> pCar;
    osg::ref_ptr<osg::StateSet> stateset;
    osg::ref_ptr<osg::Uniform> diffuseMap;
    osg::ref_ptr<osg::Uniform> reflectionMap;
    osg::ref_ptr<osg::Uniform> specularColor;
    osg::ref_ptr<osg::Uniform> lightVector;
    osg::ref_ptr<osg::Uniform> lightPower;
    osg::ref_ptr<osg::Uniform> ambientColor;
    osg::ref_ptr<osg::Uniform> shininess;


public :
    SDCarShader(osg::Node *car);
    void update(osg::Matrixf view);
};
