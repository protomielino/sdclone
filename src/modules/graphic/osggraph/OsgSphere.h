/***************************************************************************

    file        : OsgSphere.h
    copyright   : (C) 2012 by Xavier Bertaux (based on SimGear code)
    web         : bertauxx@yahoo.fr
    version     : $Id: OsgSphere.h 3162 2012-12-03 13:11:22Z torcs-ng $

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include <osg/Node>

osg::Node* SDMakeSphere(double radius, int slices, int stacks);
