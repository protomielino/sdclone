/***************************************************************************

    file                 : datalog.h
    created              : 2017-07-20 06:35:00 UTC
    copyright            : (C) Daniel Schellhammer

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _DATALOG_H_
#define _DATALOG_H_

#include "dataloghead.h"

#include <string>
#include <vector>

class DataLog
{
public:
    DataLog();
    void init(std::string dir, std::string carname);
    void add(std::string name, double* data, double scale);
    void update();
    void write();

private:
    std::string mDir;
    std::string mFile;
    std::vector<DataLogHead> mHead;
    std::vector<double> mData;
    unsigned mLogLine {};
    const unsigned mMaxLines {3000};
};

#endif // _DATALOG_H_
