/***************************************************************************

file                 : robotxml.h
created              : July 2009
copyright            : (C) 2009 Brian Gavin
web                  : speed-dreams.sourceforge.net

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef CSROBOTXML_H
#define CSROBOTXML_H

#include <vector>

#include "csnetwork.h"

class RobotXml
{
public:
    RobotXml();

    bool CreateRobotFile(const char*pRobotName,std::vector<NetDriver> &vecDriver);
    bool ReadRobotDrivers(const char*pRobotName,std::vector<NetDriver> &vecDrivers);
};

#endif // CSROBOTXML_H
