/***************************************************************************

    file                 : simuv41.cpp
    created              : Sun May 21 00:08:04 CET 2023
    copyright            : (C) 2000 by Eric Espie
    					   (C) 2023 by Xavier Bertaux
    email                : bertauxx@gmail.com
    version              : $Id: simuv41.cpp 3568 2011-05-15 15:55:24Z pouillot $

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "simuv41.h"

#include "sim.h"


// The Simuv4: singleton.
Simuv41* Simuv41::_pSelf = 0;

int openGfModule(const char* pszShLibName, void* hShLibHandle)
{
    // Instanciate the (only) module instance.
    Simuv41::_pSelf = new Simuv41(pszShLibName, hShLibHandle);

    // Register it to the GfModule module manager if OK.
    if (Simuv41::_pSelf)
        GfModule::register_(Simuv41::_pSelf);

    // Report about success or error.
    return Simuv41::_pSelf ? 0 : 1;
}

int closeGfModule()
{
    // Unregister it from the GfModule module manager.
    if (Simuv41::_pSelf)
        Simuv41::unregister(Simuv41::_pSelf);

    // Delete the (only) module instance.
    delete Simuv41::_pSelf;
    Simuv41::_pSelf = 0;

    // Report about success or error.
    return 0;
}

Simuv41& Simuv41::self()
{
    // Pre-condition : 1 successfull openGfModule call.
    return *_pSelf;
}

Simuv41::Simuv41(const std::string& strShLibName, void* hShLibHandle)
: GfModule(strShLibName, hShLibHandle)
{
}

Simuv41::~Simuv41()
{
}

// Implementation of IPhysicsEngine.
void Simuv41::initialize(int nCars, struct Track* pTrack)
{
    ::SimInit(nCars, pTrack);
}

void Simuv41::configureCar(struct CarElt* pCar)
{
    ::SimConfig(pCar);
}

void Simuv41::reconfigureCar(struct CarElt* pCar)
{
    ::SimReConfig(pCar);
}

void Simuv41::toggleCarTelemetry(int nCarIndex, bool bOn)
{
    ::SimCarTelemetry(nCarIndex, bOn);
}

void Simuv41::updateSituation(struct Situation *pSituation, double fDeltaTime)
{
    ::SimUpdate(pSituation, fDeltaTime);
}

void Simuv41::updateCar(struct Situation *pSituation, double fDeltaTime, int nCarIndex)
{
    ::SimUpdateSingleCar(nCarIndex, fDeltaTime, pSituation);
}

void Simuv41::setCar(const struct DynPt& dynGCG, int nCarIndex)
{
    ::UpdateSimCarTable(dynGCG, nCarIndex);
}

tDynPt* Simuv41::getCar(int nCarIndex)
{
    return ::GetSimCarTable(nCarIndex);
}

void Simuv41::shutdown()
{
    ::SimShutdown();
}
