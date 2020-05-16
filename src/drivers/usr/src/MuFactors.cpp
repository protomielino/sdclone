/***************************************************************************

    file        : MuFactors.cpp
    created     : 29 Apr 2017
    copyright   : (C) 2017 D.Schellhammer

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "MuFactors.h"

#include <fstream>
#include <iostream>

MuFactors::MuFactors()
{
}

void MuFactors::read(const std::string& datadir, const std::string& trackname)
{
    mSect.clear();
    MuFactorsSect sect;
    std::string datFile = datadir + trackname + "_mufactors.dat";
    std::ifstream myfile(datFile.c_str());

    if (myfile.is_open())
    {
        while (myfile >> sect.fromstart >> sect.mufactor)
        {
            mSect.push_back(sect);
        }

        mMinMuFactor = calcMinMuFactor();
        printMuFactors();
    }
    else
    {
        sect.fromstart = 0.0;
        sect.mufactor = 1.0;
        mSect.push_back(sect);
        mMinMuFactor = 1.0;
        printMuFactors();
        std::cout << "readMuFactors(): data file " << datFile << " not found, using default factor" << std::endl;
    }
}

double MuFactors::muFactor(double fromstart) const
{
    return mSect[sectIdx(fromstart)].mufactor;
}

int MuFactors::sectIdx(double fromstart) const
{
    int idx = -1;

    for (int i = 0; i < (int)mSect.size() - 1; i++)
    {
        if (fromstart > mSect[i].fromstart && fromstart < mSect[i+1].fromstart)
        {
            idx = i;
        }
    }

    if (idx == -1)
    {
        idx = (int)mSect.size() - 1;
    }

    return idx;
}

double MuFactors::calcMinMuFactor() const
{
    double minfactor = 1000.0;

    for (int i = 0; i < (int)mSect.size(); i++)
    {
        if (mSect[i].mufactor < minfactor)
        {
            minfactor = mSect[i].mufactor;
        }
    }

    return minfactor;
}

void MuFactors::printMuFactors() const
{
    std::cout << "Mu factors:" << std::endl;

    for (int i = 0; i < (int)mSect.size(); i++)
    {
        std::cout << "fs " << mSect[i].fromstart << " factor " << mSect[i].mufactor << std::endl;
    }
}
