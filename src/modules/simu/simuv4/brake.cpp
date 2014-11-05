/***************************************************************************

    file                 : brake.cpp
    created              : Sun Mar 19 00:05:26 CET 2000
    copyright            : (C) 2000 by Eric Espie
    email                : torcs@free.fr
    version              : $Id: brake.cpp 3948 2011-10-08 07:27:25Z wdbee $

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "sim.h"

void 
SimBrakeConfig(void *hdle, const char *section, tBrake *brake)
{
    tdble diam, area, mu;
    
    diam     = GfParmGetNum(hdle, section, PRM_BRKDIAM, (char*)NULL, 0.2f);
    area     = GfParmGetNum(hdle, section, PRM_BRKAREA, (char*)NULL, 0.002f);
    mu       = GfParmGetNum(hdle, section, PRM_MU, (char*)NULL, 0.30f);

	// Option TCL ...
	//if (car->features & FEAT_TCLINSIMU)
	{
	    brake->TCL = 1.0f;
		brake->TCLMin = 1.0f;
	}
	// ... Option TCL
	// Option ABS ...
	//if (car->features & FEAT_ABSINSIMU)
	{
		brake->ABS = 1.0f;
		brake->EnableABS
		     = GfParmGetNum(hdle, section, PRM_ABSINSIMU, (char*)NULL, 0.0f) > 0;
/*
		if (brake->EnableABS)
			fprintf(stderr,"ABS: Enabled\n");
		else
			fprintf(stderr,"ABS: Disabled\n");
*/
	}
	// ... Option ABS

	brake->coeff = (tdble) (diam * 0.5 * area * mu);

    brake->I = GfParmGetNum(hdle, section, PRM_INERTIA, (char*)NULL, 0.13f);
    brake->radius = diam/2.0f;
}

void 
SimBrakeUpdate(tCar *car, tWheel *wheel, tBrake *brake)
{
    brake->Tq = brake->coeff * brake->pressure;

	// Option ABS ...
	if (car->features & FEAT_ABSINSIMU)
	{
		if (brake->EnableABS)
			brake->Tq *= brake->ABS;
	}
	// ... Option ABS
	// Option TCL ...
	if (car->features & FEAT_TCLINSIMU)
	{
		tdble TCL_BrakeScale = 400.0f; // Make it be a parameter later
		if ((brake->TCLMin < 1.0) && (brake->TCLMin == brake->TCL))
		  brake->Tq += TCL_BrakeScale/brake->TCL;
	}
	// ... Option TCL

    brake->temp -= (tdble) (fabs(car->DynGC.vel.x) * 0.0001 + 0.0002);
    if (brake->temp < 0 ) brake->temp = 0;
    brake->temp += (tdble) (brake->pressure * brake->radius * fabs(wheel->spinVel) * 0.00000000005);
    if (brake->temp > 1.0) brake->temp = 1.0;
}

void 
SimBrakeSystemConfig(tCar *car)
{
    void *hdle = car->params;
    
    car->brkSyst.rep   = GfParmGetNum(hdle, SECT_BRKSYST, PRM_BRKREP, (char*)NULL, 0.5);
    car->brkSyst.coeff = GfParmGetNum(hdle, SECT_BRKSYST, PRM_BRKPRESS, (char*)NULL, 1000000);
    car->brkSyst.ebrake_pressure = GfParmGetNum(hdle, SECT_BRKSYST, PRM_EBRKPRESS, (char*)NULL, 0.0);
    
}

void 
SimBrakeSystemUpdate(tCar *car)
{
    tBrakeSyst	*brkSyst = &(car->brkSyst);

	if (car->ctrl->singleWheelBrakeMode == 1)
	{
/*
		car->wheel[FRNT_RGT].brake.pressure = brkSyst->coeff * MIN(car->ctrl->brakeFrontRightCmd, brkSyst->rep); 
		car->wheel[FRNT_LFT].brake.pressure = brkSyst->coeff * MIN(car->ctrl->brakeFrontLeftCmd, brkSyst->rep);
		car->wheel[REAR_RGT].brake.pressure = brkSyst->coeff * MIN(car->ctrl->brakeRearRightCmd, (1-brkSyst->rep));
		car->wheel[REAR_LFT].brake.pressure = brkSyst->coeff * MIN(car->ctrl->brakeRearLeftCmd, (1-brkSyst->rep));
*/
		car->wheel[FRNT_RGT].brake.pressure = brkSyst->coeff * car->ctrl->brakeFrontRightCmd; 
		car->wheel[FRNT_LFT].brake.pressure = brkSyst->coeff * car->ctrl->brakeFrontLeftCmd;
		car->wheel[REAR_RGT].brake.pressure = brkSyst->coeff * car->ctrl->brakeRearRightCmd;
		car->wheel[REAR_LFT].brake.pressure = brkSyst->coeff * car->ctrl->brakeRearLeftCmd;
	}
	else
	{
	    tdble	ctrl = car->ctrl->brakeCmd;
		ctrl *= brkSyst->coeff;
		car->wheel[FRNT_RGT].brake.pressure = car->wheel[FRNT_LFT].brake.pressure = ctrl * brkSyst->rep;
		car->wheel[REAR_RGT].brake.pressure = car->wheel[REAR_LFT].brake.pressure = ctrl * (1 - brkSyst->rep);
	}

    if ( (car->ctrl->ebrakeCmd > 0) && (car->wheel[REAR_RGT].brake.pressure < brkSyst->ebrake_pressure) ) {
        car->wheel[REAR_RGT].brake.pressure = car->wheel[REAR_LFT].brake.pressure = brkSyst->ebrake_pressure;
    }
}
