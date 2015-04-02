/***************************************************************************

    file                     : OsgOptions.h
    created                  : Thu Mar 31 00:00:41 CEST 2015
    copyright                : (C) 2015 by Xavier Bertaux
    email                    : bertauxx@yahoo.fr
    version                  : $Id: OsgOptions.h 5940 2015-04-01 03:12:09Z torcs-ng $

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <stdlib.h>

#ifndef _OSGOPTIONS_H_
#define _OSGOPTIONS_H_

class SDOptions
{
public:
    SDOptions( void );
    ~SDOptions( void );

private:
    unsigned int _SmokeValue;
    unsigned int _SmokeDuration;
    unsigned int _SmokeInterval;

    unsigned int _SkidValue;
    unsigned int _SkidLength;
    unsigned int _SkidInterval;

    unsigned int _LOD;

    bool         _bgsky;
    unsigned int _DynamicSkyDome;
    unsigned int _SkyDomeDistance;
    unsigned int _Max_Visibility;
    unsigned int _PrecipitationDensity;
    unsigned int _Rain;

    unsigned int _CloudLayer;

    unsigned int _SceneLOD;
    bool         _Cockpit3D;

    unsigned int _ShadowType;
    int          _ShadowSize;
    unsigned int _ShadowQuality;

    unsigned int _Shaders;
    int          _ShadersQuality;
    int          _ShadersSize;

    bool         _NormalMap;
};

#endif /* _OSGOPTIONS_H_ */
