/****************************************************************************

    file                 : neptune.h
    created              : Sun Aug 02 22:54:56 CET 2012
    copyright            : (C) 1997  Durk Talsma - (C)2012 Xavier Bertaux
    email                : bertauxx@yahoo.fr

 ****************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef _NEPTUNE_H_
#define _NEPTUNE_H_

#include "celestialbody.h"
#include "star.h"

class ePhNeptune : public ePhCelestialBody
{
public:
  ePhNeptune (double mjd);
  ePhNeptune ();
  void updatePosition(double mjd, ePhStar *ourSun);
};

#endif // _NEPTUNE_H_
