/***************************************************************************

    file                 : cars.cpp
    created              : July 2009
    copyright            : (C) 2009 Brian Gavin, 2010 Jean-Philippe Meuret
    web                  : speed-dreams.sourceforge.net
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

#include <cmath>
#include <map>
#include <sstream>
#include <algorithm>

#include <tgf.h>

#include <car.h>

#include "cars.h"


class GfCars::Private
{
public:
	
	// One GfCar structure for each car (order = sorted directory one).
	std::vector<GfCar*> vecCars;

	// Map for quick access to GfCar by id
	std::map<std::string, GfCar*> mapCarsById;

	// Vector of category Ids.
	std::vector<std::string> vecCatIds;
	
	// Vector of category names.
	std::vector<std::string> vecCatNames;
};


GfCars* GfCars::_pSelf = 0;

GfCars *GfCars::self()
{
	if (!_pSelf)
		_pSelf = new GfCars;
	
	return _pSelf;
}

GfCars::~GfCars()
{
	std::vector<GfCar*>::const_iterator itCar;
	for (itCar = _pPrivate->vecCars.begin(); itCar != _pPrivate->vecCars.end(); itCar++)
		delete *itCar;
}

GfCars::GfCars()
{
	_pPrivate = new Private;

	// Get the list of sub-dirs in the "cars" folder.
	tFList* lstFolders = GfDirGetList("cars");
	if (!lstFolders)
	{
		GfLogFatal("No car available in the 'cars' folder\n");
		return;
	}
	
	std::string strLastCatId("none");
	std::string strCatName;
	tFList* pFolder = lstFolders;
	do 
	{
		//GfLogDebug("GfCars::GfCars() : Examining %s\n", pFolder->name);
		
		// Ignore "." and ".." folders.
		if (pFolder->name[0] == '.') 
			continue;
			
		// Open the XML file of the car.
		const char* pszCarId = pFolder->name;
			
		std::ostringstream ossCarFileName;
		ossCarFileName << "cars/" << pszCarId << '/' << pszCarId << PARAMEXT;
		void* hparmCar = GfParmReadFile(ossCarFileName.str().c_str(), GFPARM_RMODE_STD);
		if (!hparmCar)
		{
			GfLogWarning("Ignoring car %s (file %s not %s)\n",
						 pszCarId, ossCarFileName.str().c_str(),
						 GfFileExists(ossCarFileName.str().c_str()) ? "readable" : "found");
			continue;
		}

		// Read car info.
		const std::string strCatId = GfParmGetStr(hparmCar, SECT_CAR, PRM_CATEGORY, "");
		if (strCatId != strLastCatId)
		{
			// Little optimization : don't load category file if same as the previous car's.
			std::ostringstream ossCatFileName;
			ossCatFileName << "categories/" << strCatId << PARAMEXT;
			void* hparmCat = GfParmReadFile(ossCatFileName.str().c_str(), GFPARM_RMODE_STD);
			if (!hparmCat)
			{
				GfLogWarning("Ignoring car %s (category file %s not %s)\n",
							 pszCarId, ossCatFileName.str().c_str(),
							 GfFileExists(ossCatFileName.str().c_str()) ? "readable" : "found");
				GfParmReleaseHandle(hparmCar);
				continue;
			}
			strLastCatId = strCatId;
			strCatName = GfParmGetName(hparmCat);
			GfParmReleaseHandle(hparmCat);
		}

		// Store it in a new GfCar instance.
		GfCar* pCar = new GfCar(pszCarId, strCatId, strCatName, hparmCar);

		// Update the GfCars singleton.
		_pPrivate->vecCars.push_back(pCar);
		_pPrivate->mapCarsById[pszCarId] = pCar;
		if (std::find(_pPrivate->vecCatIds.begin(), _pPrivate->vecCatIds.end(), strCatId)
			== _pPrivate->vecCatIds.end())
		{
			_pPrivate->vecCatIds.push_back(strCatId);
			_pPrivate->vecCatNames.push_back(strCatName);
		}

		// Close the XML file of the car and category.
		GfParmReleaseHandle(hparmCar);
	} 
	while ((pFolder = pFolder->next) != lstFolders);
	
	GfDirFreeList(lstFolders, NULL, true, true);
	
	// Trace what we got.
	print();
}

const std::vector<std::string>& GfCars::getCategoryIds() const
{
	return _pPrivate->vecCatIds;
}

const std::vector<std::string>& GfCars::getCategoryNames() const
{
	return _pPrivate->vecCatNames;
}

GfCar* GfCars::getCar(const std::string& strId) const
{
	std::map<std::string, GfCar*>::iterator itCar =
		_pPrivate->mapCarsById.find(strId);
	if (itCar != _pPrivate->mapCarsById.end())
		return itCar->second;
	
	return 0;
}

GfCar* GfCars::getCarWithName(const std::string& strName) const
{
	std::vector<GfCar*>::iterator itCar;
	for (itCar = _pPrivate->vecCars.begin(); itCar != _pPrivate->vecCars.end(); itCar++)
		if ((*itCar)->getName() == strName)
			return *itCar;

	return 0;
}

std::vector<GfCar*> GfCars::getCarsInCategory(const std::string& strCatId) const
{
	std::vector<GfCar*> vecCarsInCat;

	std::vector<GfCar*>::iterator itCar;
	for (itCar = _pPrivate->vecCars.begin(); itCar != _pPrivate->vecCars.end(); itCar++)
		if (strCatId.empty() || (*itCar)->getCategoryId() == strCatId)
			vecCarsInCat.push_back(*itCar);

	return vecCarsInCat;
}

std::vector<GfCar*> GfCars::getCarsInCategoryWithName(const std::string& strCatName) const
{
	std::vector<GfCar*> vecCarsInCat;

	std::vector<GfCar*>::iterator itCar;
	for (itCar = _pPrivate->vecCars.begin(); itCar != _pPrivate->vecCars.end(); itCar++)
		if (strCatName.empty() || (*itCar)->getCategoryName() == strCatName)
			vecCarsInCat.push_back(*itCar);

	return vecCarsInCat;
}

std::vector<std::string> GfCars::getCarIdsInCategory(const std::string& strCatId) const
{
	std::vector<std::string> vecCarIds;

	std::vector<GfCar*>::const_iterator itCar;
	for (itCar = _pPrivate->vecCars.begin(); itCar != _pPrivate->vecCars.end(); itCar++)
		if (strCatId.empty() || (*itCar)->getCategoryId() == strCatId)
			vecCarIds.push_back((*itCar)->getId());

	return vecCarIds;
}

std::vector<std::string> GfCars::getCarNamesInCategory(const std::string& strCatId) const
{
	std::vector<std::string> vecCarNames;

	std::vector<GfCar*>::const_iterator itCar;
	for (itCar = _pPrivate->vecCars.begin(); itCar != _pPrivate->vecCars.end(); itCar++)
		if (strCatId.empty() || (*itCar)->getCategoryId() == strCatId)
			vecCarNames.push_back((*itCar)->getName());

	return vecCarNames;
}

void GfCars::print() const
{
	GfLogTrace("Car base : %d categories, %d cars\n",
			   _pPrivate->vecCatIds.size(), _pPrivate->vecCars.size());
	std::vector<std::string>::const_iterator itCatName;
	for (itCatName = _pPrivate->vecCatNames.begin();
		 itCatName != _pPrivate->vecCatNames.end(); itCatName++)
	{
		GfLogTrace("  '%s' category :\n", itCatName->c_str());
		const std::vector<GfCar*> vecCarsInCat = getCarsInCategoryWithName(*itCatName);
		std::vector<GfCar*>::const_iterator itCar;
		for (itCar = vecCarsInCat.begin(); itCar != vecCarsInCat.end(); itCar++)
			GfLogTrace("    %-22s: %s\n", (*itCar)->getName().c_str(),
					   (*itCar)->getDescriptorFileName().c_str());
	}
}

// GfCar class ------------------------------------------------------------------

GfCar::GfCar(const std::string& strId, const std::string& strCatId,
			 const std::string& strCatName, void* hparmCar)
: _strId(strId), _strCatId(strCatId), _strCatName(strCatName)
{
	load(hparmCar);
}

void GfCar::load(void* hparmCar)
{
	static const char *pszGearName[MAX_GEARS] = {"r", "n", "1", "2", "3", "4", "5", "6", "7", "8"};

	// Name.
	_strName = GfParmGetName(hparmCar);

	// Descriptor file name (default setup).
	_strDescFile = GfParmGetFileName(hparmCar);

	// Mass and front/rear repartition.
	_fMass = GfParmGetNum(hparmCar, SECT_CAR, PRM_MASS, (char*)NULL, 1500);
	_fFrontRearMassRatio = GfParmGetNum(hparmCar, SECT_CAR, PRM_FRWEIGHTREP, (char*)NULL, .5);
	
	// Drive train.
	const std::string strDriveTrain =
		GfParmGetStr(hparmCar, SECT_DRIVETRAIN, PRM_TYPE, VAL_TRANS_RWD);
	if (strDriveTrain == VAL_TRANS_RWD)
		_eDriveTrain = eRWD;
	else if (strDriveTrain == VAL_TRANS_FWD)
		_eDriveTrain = eFWD;
	else if (strDriveTrain == VAL_TRANS_4WD)
		_eDriveTrain = e4WD;

	// Number of gears.
	std::ostringstream ossSpecPath;
	for (int nGearInd = MAX_GEARS - 1; nGearInd >= 0; nGearInd--)
	{
		ossSpecPath.str("");
		ossSpecPath << SECT_GEARBOX << '/' << ARR_GEARS << '/' << pszGearName[nGearInd];
		if (GfParmGetNum(hparmCar, ossSpecPath.str().c_str(), PRM_RATIO, (char*)NULL, 0.0f))
		{
			_nGears = nGearInd - 1;
			break;
		}
	}

	// Turbo.
	_bTurboCharged = strcmp(GfParmGetStr(hparmCar, SECT_ENGINE, PRM_TURBO, "false"), "true") == 0;

	// Max power and torque + associated engine speeds.
	_fMaxTorque = 0;
	_fMaxTorqueSpeed = 0;
	_fMaxPower = 0;
	_fMaxPowerSpeed = 0;
	const tdble fMaxSpeed = GfParmGetNum(hparmCar, SECT_ENGINE, PRM_REVSLIM, 0, 0);

	ossSpecPath.str("");
	ossSpecPath << SECT_ENGINE << '/' << ARR_DATAPTS;
	const int nEngineTqCurvePts = GfParmGetEltNb(hparmCar, ossSpecPath.str().c_str());
	for (int nPtInd = 2; nPtInd <= nEngineTqCurvePts; nPtInd++)
	{
		ossSpecPath.str("");
		ossSpecPath << SECT_ENGINE << '/' << ARR_DATAPTS << '/' << nPtInd;
		const tdble fSpeed = GfParmGetNum(hparmCar, ossSpecPath.str().c_str(), PRM_RPM, 0, 0);
		if (fSpeed > fMaxSpeed)
			break;
		const tdble fTorque = GfParmGetNum(hparmCar, ossSpecPath.str().c_str(), PRM_TQ, 0, 0);
		if (fTorque > _fMaxTorque)
		{
			_fMaxTorque = fTorque;
			_fMaxTorqueSpeed = fSpeed;
		}
		const tdble fPower = (tdble)(fTorque * fSpeed); 
		if (fPower > _fMaxPower)
		{
			_fMaxPower = fPower;
			_fMaxPowerSpeed = fSpeed;
		}
	}

	 // "Mechanical = Low speed" grip (~mu*g, but with front/rear mass repartition).
	const tdble fMuFront =
		(GfParmGetNum(hparmCar, SECT_FRNTRGTWHEEL, PRM_MU, (char*)NULL, 1.0)
		 + GfParmGetNum(hparmCar, SECT_FRNTLFTWHEEL, PRM_MU, (char*)NULL, 1.0)) / 2.0f;
	const tdble fMuRear =
		(GfParmGetNum(hparmCar, SECT_REARRGTWHEEL, PRM_MU, (char*)NULL, 1.0)
		 + GfParmGetNum(hparmCar, SECT_REARLFTWHEEL, PRM_MU, (char*)NULL, 1.0)) / 2.0f;
	_fLowSpeedGrip =
		(_fFrontRearMassRatio * fMuFront + (1.0f - _fFrontRearMassRatio) * fMuRear) * G;

	// "Aerodynamic = High speed" grip (same + with aero down-force).
	// TODO: Check formula (F/R Clift repartition "guessed" from Kristof's ; sure it's wrong ;-)
	const tdble fRefCarSpeed = 200 / 3.6f;
	const tdble fFrontWingArea =
		GfParmGetNum(hparmCar, SECT_FRNTWING, PRM_WINGAREA, (char*)NULL, 0.0);
	const tdble fRearWingArea =
		GfParmGetNum(hparmCar, SECT_REARWING, PRM_WINGAREA, (char*)NULL, 0.0);
	const tdble fFrontWingAngle =
		GfParmGetNum(hparmCar, SECT_FRNTWING, PRM_WINGANGLE, (char*)NULL, 0.0);
	const tdble fRearWingAngle =
		GfParmGetNum(hparmCar, SECT_REARWING, PRM_WINGANGLE, (char*)NULL, 0.0);
	const tdble fFrontClift =
		GfParmGetNum(hparmCar, SECT_AERODYNAMICS, PRM_FCL, (char*)NULL, 0.0);
	const tdble fRearClift =
		GfParmGetNum(hparmCar, SECT_AERODYNAMICS, PRM_RCL, (char*)NULL, 0.0);
	const double fTotalFrontClift = 2 * fFrontClift + 4.92 * fFrontWingArea * sin(fFrontWingAngle);
	const double fTotalRearClift = 2 * fRearClift + 4.92 * fRearWingArea * sin(fRearWingAngle);
// Work in progress.
 	_fHighSpeedGrip =
 		fRefCarSpeed * fRefCarSpeed
 		* (tdble)(_fFrontRearMassRatio * fTotalFrontClift * fMuFront
				  + (1.0 - _fFrontRearMassRatio) * fTotalRearClift * fMuRear) * G / _fMass;

	// Inverse of the inertia around the Z axis.
	const tdble fMassRepCoef = GfParmGetNum(hparmCar, SECT_CAR, PRM_CENTR, (char*)NULL, 1.0);
	const tdble fCarLength = GfParmGetNum(hparmCar, SECT_CAR, PRM_LEN, (char*)NULL, 4.7f);
	const tdble fCarWidth = GfParmGetNum(hparmCar, SECT_CAR, PRM_WIDTH, (char*)NULL, 1.9f);
	_fInvertedZAxisInertia = // Stolen from Simu V2.1, car.cpp, SimCarConfig()
		12.0f / (_fMass * fMassRepCoef * fMassRepCoef)
		/ (fCarWidth * fCarWidth + fCarLength * fCarLength);
	
	// Theorical top speed on a flat road, assuming the gears are tuned accordingly.
	const tdble fFrontArea =
		GfParmGetNum(hparmCar, SECT_AERODYNAMICS, PRM_FRNTAREA, (char*)NULL, 2.0f);
	const tdble fCx =
		GfParmGetNum(hparmCar, SECT_AERODYNAMICS, PRM_CX, (char*)NULL, 0.35f);
	const tdble muRollRes = 0.015f; // Average track.
	const tdble eff = 0.95f * 0.95f; // Average gear * differential efficiencies
	const tdble Cd =
		0.645f * fCx * fFrontArea
		+ 1.23f * (fFrontWingArea * sin(fFrontWingAngle) + fRearWingArea * sin(fRearWingAngle));
	const double pp =
		muRollRes * _fMass * G / (Cd + muRollRes * (tdble)(fTotalFrontClift + fTotalRearClift));
	const double q = eff * _fMaxPower / (Cd + muRollRes * (tdble)(fTotalFrontClift + fTotalRearClift));
// Work in progress.
	_fTopSpeed =
 		(tdble)pow(q/2+sqrt(q*q/4+pp*pp*pp/27), 1.0/3)
 		- (tdble)pow(-q/2+sqrt(q*q/4+pp*pp*pp/27), 1.0/3);
}

const std::string& GfCar::getId() const
{
	return _strId;
}

const std::string& GfCar::getName() const
{
	return _strName;
}

const std::string& GfCar::getCategoryId() const
{
	return _strCatId;
}

const std::string& GfCar::getCategoryName() const
{
	return _strCatName;
}

const std::string& GfCar::getDescriptorFileName() const
{
	return _strDescFile;
}

GfCar::EDriveTrain GfCar::getDriveTrain() const
{
	return _eDriveTrain;
}

unsigned GfCar::getGearsCount() const
{
	return _nGears;
}

bool GfCar::isTurboCharged() const
{
	return _bTurboCharged;
}

tdble GfCar::getMaxPower() const
{
	return _fMaxPower;
}

tdble GfCar::getMaxPowerSpeed() const
{
	return _fMaxPowerSpeed;
}

tdble GfCar::getMaxTorque() const
{
	return _fMaxTorque;
}

tdble GfCar::getMaxTorqueSpeed() const
{
	return _fMaxTorqueSpeed;
}

tdble GfCar::getMass() const
{
	return _fMass;
}

tdble GfCar::getFrontRearMassRatio() const
{
	return _fFrontRearMassRatio;
}

tdble GfCar::getTopSpeed() const
{
	return _fTopSpeed;
}

tdble GfCar::getLowSpeedGrip() const
{
	return _fLowSpeedGrip;
}

tdble GfCar::getHighSpeedGrip() const
{
	return _fHighSpeedGrip;
}

tdble GfCar::getInvertedZAxisInertia() const
{
	return _fInvertedZAxisInertia;
}

// void GfCar::setId(const std::string& strId)
// {
// 	_strId = strId;
// }

// void GfCar::setName(const std::string& strName)
// {
// 	_strName = strName;
// }

// void GfCar::setCategoryId(const std::string& strCatId)
// {
// 	_strCatId = strCatId;
// }

// void GfCar::setCategoryName(const std::string& strCatName)
// {
// 	_strCatName = strCatName;
// }

// void GfCar::setDescriptorFileName(const std::string& strDescFile)
// {
// 	_strDescFile = strDescFile;
// }
