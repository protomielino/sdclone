/****************************************************************************

    file                 : venus.h
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

#ifndef _VENUS_H_
#define _VENUS_H_

#include "celestialbody.h"
#include "star.h"

class ePhVenus : public ePhCelestialBody
{
public:
  ePhVenus (double mjd);
  ePhVenus ();
  void updatePosition(double mjd, ePhStar *ourSun);
};

#endif // _VENUS_H_
