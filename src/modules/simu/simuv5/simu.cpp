/***************************************************************************

    file                 : simu.cpp
    created              : Sun Mar 19 00:07:53 CET 2000
    copyright            : (C) 2000 by Eric Espie
    email                : torcs@free.fr
    version              : $Id: simu.cpp 3945 2011-10-07 13:38:15Z wdbee $

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <cstdlib>
#include <cstdio>
#include <memory.h>
#include <cmath>

#include <portability.h>
#include <tgf.h>
#include <robottools.h>

#include "sim.h"

tCar   *SimCarTable = 0;

tdble  SimDeltaTime;

int    SimTelemetry = -1;

t3Dd   vectStart[16];
t3Dd   vectEnd[16];

tdble  SimRain = 0;
tdble  SimWater = 0;
tdble  SimTimeOfDay = 0;
int    SimClouds = 0;
int    lastCompoundSet = 0;

double Tair = 273.15;
tdble  Ttrack = 220.0;

float  SimAirPressure = 101300.0f;
float  SimAirDensity = 1.290f;

static tTrack *PTrack = 0;

static int SimNbCars = 0;

#define MEANNB 0
#define MEANW  1

/*
 * Check the input control from robots
 */
static void
ctrlCheck(tCar *car)
{
    tTransmission	*trans = &(car->transmission);
    tClutch		*clutch = &(trans->clutch);

    /* sanity check */
#ifndef WIN32
    if (isnan(car->ctrl->accelCmd) || isinf(car->ctrl->accelCmd)) car->ctrl->accelCmd = 0;
    if (isnan(car->ctrl->brakeCmd) || isinf(car->ctrl->brakeCmd)) car->ctrl->brakeCmd = 0;
    if (isnan(car->ctrl->clutchCmd) || isinf(car->ctrl->clutchCmd)) car->ctrl->clutchCmd = 0;
    if (isnan(car->ctrl->steer) || isinf(car->ctrl->steer)) car->ctrl->steer = 0;
    if (isnan(car->ctrl->wingFrontCmd) || isinf(car->ctrl->wingFrontCmd)) car->ctrl->wingFrontCmd = 0;
    if (isnan(car->ctrl->wingRearCmd) || isinf(car->ctrl->wingRearCmd)) car->ctrl->wingRearCmd = 0;
    if (isnan(car->ctrl->brakeFrontLeftCmd) || isinf(car->ctrl->brakeFrontLeftCmd)) car->ctrl->brakeFrontLeftCmd = 0;
    if (isnan(car->ctrl->brakeFrontRightCmd) || isinf(car->ctrl->brakeFrontRightCmd)) car->ctrl->brakeFrontRightCmd = 0;
    if (isnan(car->ctrl->brakeRearLeftCmd) || isinf(car->ctrl->brakeRearLeftCmd)) car->ctrl->brakeRearLeftCmd = 0;
    if (isnan(car->ctrl->brakeRearRightCmd) || isinf(car->ctrl->brakeRearRightCmd)) car->ctrl->brakeRearRightCmd = 0;
#else
    if (isnan(car->ctrl->accelCmd)) car->ctrl->accelCmd = 0;
    if (isnan(car->ctrl->brakeCmd)) car->ctrl->brakeCmd = 0;
    if (isnan(car->ctrl->clutchCmd)) car->ctrl->clutchCmd = 0;
    if (isnan(car->ctrl->steer)) car->ctrl->steer = 0;
    if (isnan(car->ctrl->wingFrontCmd)) car->ctrl->wingFrontCmd = 0;
    if (isnan(car->ctrl->wingRearCmd)) car->ctrl->wingRearCmd = 0;
    if (isnan(car->ctrl->brakeFrontLeftCmd)) car->ctrl->brakeFrontLeftCmd = 0;
    if (isnan(car->ctrl->brakeFrontRightCmd)) car->ctrl->brakeFrontRightCmd = 0;
    if (isnan(car->ctrl->brakeRearLeftCmd)) car->ctrl->brakeRearLeftCmd = 0;
    if (isnan(car->ctrl->brakeRearRightCmd)) car->ctrl->brakeRearRightCmd = 0;
#endif

    /* When the car is broken try to send it on the track side */
    if (car->carElt->_state & RM_CAR_STATE_BROKEN)
    {
        car->ctrl->accelCmd = 0.0f;
        car->ctrl->brakeCmd = 0.1f;
        car->ctrl->gear = 0;

        if (car->trkPos.toRight >  car->trkPos.seg->width / 2.0)
        {
            car->ctrl->steer = 0.1f;
        }
        else
        {
            car->ctrl->steer = -0.1f;
        }
    }
    else if (car->carElt->_state & RM_CAR_STATE_ELIMINATED)
    {
        car->ctrl->accelCmd = 0.0f;
        car->ctrl->brakeCmd = 0.1f;
        car->ctrl->gear = 0;
        if (car->trkPos.toRight >  car->trkPos.seg->width / 2.0)
        {
            car->ctrl->steer = 0.1f;
        }
        else
        {
            car->ctrl->steer = -0.1f;
        }
    }
    else if (car->carElt->_state & RM_CAR_STATE_FINISH)
    {
        /* when the finish line is passed, continue at "slow" pace */
        car->ctrl->accelCmd = (tdble) MIN(car->ctrl->accelCmd, 0.20);
        if (car->DynGC.vel.x > 30.0)
        {
            car->ctrl->brakeCmd = (tdble) MAX(car->ctrl->brakeCmd, 0.05);
        }
    }

    /* check boundaries */
    if (car->ctrl->accelCmd > 1.0)
    {
        car->ctrl->accelCmd = 1.0;
    }
    else if (car->ctrl->accelCmd < 0.0)
    {
        car->ctrl->accelCmd = 0.0;
    }
    if (car->ctrl->brakeCmd > 1.0)
    {
        car->ctrl->brakeCmd = 1.0;
    }
    else if (car->ctrl->brakeCmd < 0.0)
    {
        car->ctrl->brakeCmd = 0.0;
    }

    if (car->ctrl->clutchCmd > 1.0)
    {
        car->ctrl->clutchCmd = 1.0;
    }
    else if (car->ctrl->clutchCmd < 0.0)
    {
        car->ctrl->clutchCmd = 0.0;
    }

    if (car->ctrl->steer > 1.0)
    {
        car->ctrl->steer = 1.0;
    }
    else if (car->ctrl->steer < -1.0)
    {
        car->ctrl->steer = -1.0;
    }

    clutch->transferValue = 1.0f - car->ctrl->clutchCmd;

    if (car->ctrl->wingFrontCmd > (float) (PI_2))
    {
        car->ctrl->wingFrontCmd = (float) (PI_2);
    }
    else if (car->ctrl->wingFrontCmd < 0.0)
    {
        car->ctrl->wingFrontCmd = 0.0;
    }

    if (car->ctrl->wingRearCmd > (float) (PI_2))
    {
        car->ctrl->wingRearCmd = (float) (PI_2);
    }
    else if (car->ctrl->wingRearCmd < 0.0)
    {
        car->ctrl->wingRearCmd = 0.0;
    }

    if (car->ctrl->brakeFrontLeftCmd < 0)
    {
        car->ctrl->brakeFrontLeftCmd = 0;
    }
    else if (car->ctrl->brakeFrontLeftCmd > 1.0f)
    {
        car->ctrl->brakeFrontLeftCmd = 1.0f;
    }

    if (car->ctrl->brakeFrontRightCmd < 0)
    {
        car->ctrl->brakeFrontRightCmd = 0;
    }
    else if (car->ctrl->brakeFrontRightCmd > 1.0f)
    {
        car->ctrl->brakeFrontRightCmd = 1.0f;
    }

    if (car->ctrl->brakeRearLeftCmd < 0) {
        car->ctrl->brakeRearLeftCmd = 0;
    }
    else if (car->ctrl->brakeRearLeftCmd > 1.0f)
    {
        car->ctrl->brakeRearLeftCmd = 1.0f;
    }

    if (car->ctrl->brakeRearRightCmd < 0) {
        car->ctrl->brakeRearRightCmd = 0;
    }
    else if (car->ctrl->brakeRearRightCmd > 1.0f)
    {
        car->ctrl->brakeRearRightCmd = 1.0f;
    }
}

/* Initial configuration */
void
SimConfig(tCarElt *carElt)
{
    tCar *car = &(SimCarTable[carElt->index]);

    memset(car, 0, sizeof(tCar));

    car->carElt = carElt;
    car->DynGCg = car->DynGC = carElt->_DynGC;
    car->trkPos = carElt->_trkPos;
    car->ctrl   = &carElt->ctrl;
    car->params = carElt->_carHandle;

    SimCarConfig(car);

    SimCarCollideConfig(car, PTrack);
    sgMakeCoordMat4(carElt->pub.posMat, carElt->_pos_X, carElt->_pos_Y, carElt->_pos_Z - carElt->_statGC_z,
            (float) RAD2DEG(carElt->_yaw), (float) RAD2DEG(carElt->_roll), (float) RAD2DEG(carElt->_pitch));
}

/* After pit stop */
void
SimReConfig(tCarElt *carElt)
{
    int i;

    tCar *car = &(SimCarTable[carElt->index]);

    if (carElt->pitcmd.fuel > 0)
    {
        car->fuel += carElt->pitcmd.fuel;

        if (car->fuel > car->tank)
            car->fuel = car->tank;
    }

    if (carElt->pitcmd.repair > 0)
    {
        car->dammage -= carElt->pitcmd.repair;
        for(i=0; i<4; i++)
        {
            car->wheel[i].susp.damper.efficiency = 1.0;
        }

        if (car->dammage < 0)
            car->dammage = 0;
    }

    if (car->engine.temp_water > car->engine.max_temp_water - 1.0)
        car->engine.temp_water = car->engine.temp_water - 3.0;

    carElt->setup.reqRepair.desired_value = 0.0;
    carElt->_airtemp = Tair;

    if (carElt->pitcmd.tireChange == tCarPitCmd::ALL)
    {
        for(i=0; i<4; i++)
        {
            GfLogInfo("# Reset Tyre in simu !\n");
            car->wheel[i].treadDepth = 1.01f;
            car->wheel[i].currentWear = 0.0;
            car->wheel[i].currentGraining = 0.0;
            car->wheel[i].currentGripFactor = 1.0;

            if (car->options->tyre_temperature)
                car->wheel[i].Ttire = car->wheel[i].Tinit;
            else
                car->wheel[i].Ttire = car->wheel[i].Topt;

            if(car->features & FEAT_COMPOUNDS && car->options->compounds && carElt->pitcmd.tiresetChange)
            {
                car->wheel[i].tireSet = carElt->pitcmd.tiresetChange;
                GfLogInfo("# Tireset value simu = %d\n", car->wheel[i].tireSet);
                //setupCompound->value = wheel->tireSet;
                //setupCompound->changed = false;
                car->wheel[i].mu = car->wheel[i].muC[car->wheel[i].tireSet];
                car->wheel[i].Topt = car->wheel[i].ToptC[car->wheel[i].tireSet];
                car->wheel[i].hysteresisFactor = car->wheel[i].hysteresisFactorC[car->wheel[i].tireSet];
				car->wheel[i].coolingFactor = car->wheel[i].coolingFactorC[car->wheel[i].tireSet];
				car->wheel[i].latHeatFactor = car->wheel[i].latHeatFactorC[car->wheel[i].tireSet];
				car->wheel[i].longHeatFactor = car->wheel[i].longHeatFactorC[car->wheel[i].tireSet];
				car->wheel[i].tireSpeedCoolFactor = car->wheel[i].tireSpeedCoolFactorC[car->wheel[i].tireSet];
				car->wheel[i].wearFactor = car->wheel[i].wearFactorC[car->wheel[i].tireSet];
				car->wheel[i].tireTreadDrainFactor = car->wheel[i].tireTreadDrainFactorC[car->wheel[i].tireSet];
				car->wheel[i].muWet = car->wheel[i].muWetC[car->wheel[i].tireSet];
                car->carElt->_tyreT_opt(i) = car->wheel[i].Topt;
                GfLogInfo("# SimuV5 tire compound changed mu = %.3f - hysteresis = %.2f - wear factor = %.7f\n", car->wheel[i].mu,
                          car->wheel[i].hysteresisFactor, car->wheel[i].wearFactor);
            }
        }
    }

    SimWingReConfig(car, 0);
    SimWingReConfig(car, 1);
}

/* deal with dashboardInstant changes */
void
SimInstantReConfig(tCar *car)
{
    tCarSetupItem *setup;

    if (car->ctrl->setupChangeCmd)
    {
        setup = (car->ctrl->setupChangeCmd->setup);
    }
    else
        return;

    switch (car->ctrl->setupChangeCmd->type)
    {
        case DI_BRAKE_REPARTITION:
            SimBrakeSystemReConfig(car);
            break;
        case DI_FRONT_ANTIROLLBAR:
            SimArbReConfig(car, 0);
            break;
        case DI_REAR_ANTIROLLBAR:
            SimArbReConfig(car, 1);
            break;
        case DI_FRONT_DIFF_MAX_SLIP_BIAS:
            car->transmission.differential[TRANS_FRONT_DIFF].dSlipMax = setup->desired_value;
            setup->value = car->transmission.differential[TRANS_FRONT_DIFF].dSlipMax;
            setup->changed = FALSE;
            break;
        case DI_FRONT_DIFF_COAST_MAX_SLIP_BIAS:
            car->transmission.differential[TRANS_FRONT_DIFF].dCoastSlipMax = setup->desired_value;
            setup->value = car->transmission.differential[TRANS_FRONT_DIFF].dCoastSlipMax;
            setup->changed = FALSE;
            break;
        case DI_REAR_DIFF_MAX_SLIP_BIAS:
            car->transmission.differential[TRANS_REAR_DIFF].dSlipMax = setup->desired_value;
            setup->value = car->transmission.differential[TRANS_REAR_DIFF].dSlipMax;
            setup->changed = FALSE;
            break;
        case DI_REAR_DIFF_COAST_MAX_SLIP_BIAS:
            car->transmission.differential[TRANS_REAR_DIFF].dCoastSlipMax = setup->desired_value;
            setup->value = car->transmission.differential[TRANS_REAR_DIFF].dCoastSlipMax;
            setup->changed = FALSE;
            break;
        case DI_CENTRAL_DIFF_MAX_SLIP_BIAS:
            car->transmission.differential[TRANS_CENTRAL_DIFF].dSlipMax = setup->desired_value;
            setup->value = car->transmission.differential[TRANS_CENTRAL_DIFF].dSlipMax;
            setup->changed = FALSE;
            break;
        case DI_CENTRAL_DIFF_COAST_MAX_SLIP_BIAS:
            car->transmission.differential[TRANS_CENTRAL_DIFF].dCoastSlipMax = setup->desired_value;
            setup->value = car->transmission.differential[TRANS_CENTRAL_DIFF].dCoastSlipMax;
            setup->changed = FALSE;
            break;
    }

    car->ctrl->setupChangeCmd = NULL;
}

static void
RemoveCar(tCar *car, tSituation *s)
{
    int i;
    tCarElt *carElt;
    tTrkLocPos trkPos;
    int trkFlag;
    tdble travelTime;
    tdble dang;

    static tdble PULL_Z_OFFSET = 3.0;
    static tdble PULL_SPD = 0.5;

    carElt = car->carElt;

    if (carElt->_state & RM_CAR_STATE_PULLUP)
    {
        carElt->_pos_Z += car->restPos.vel.z * SimDeltaTime;
        carElt->_yaw += car->restPos.vel.az * SimDeltaTime;
        carElt->_roll += car->restPos.vel.ax * SimDeltaTime;
        carElt->_pitch += car->restPos.vel.ay * SimDeltaTime;
        sgMakeCoordMat4(carElt->pub.posMat, carElt->_pos_X, carElt->_pos_Y, carElt->_pos_Z - carElt->_statGC_z,
            (float) RAD2DEG(carElt->_yaw), (float) RAD2DEG(carElt->_roll), (float) RAD2DEG(carElt->_pitch));

        if (carElt->_pos_Z > (car->restPos.pos.z + PULL_Z_OFFSET))
        {
            carElt->_state &= ~RM_CAR_STATE_PULLUP;
            carElt->_state |= RM_CAR_STATE_PULLSIDE;

            // Moved pullside velocity computation down due to floating point error accumulation.
        }

        return;
    }

    if (carElt->_state & RM_CAR_STATE_PULLSIDE)
    {
        // Recompute speed to avoid missing the parking point due to error accumulation (the pos might be
        // in the 0-10000 range, depending on the track and vel*dt is around 0-0.001, so basically all
        // but the most significant digits are lost under bad conditions, happens e.g on e-track-4).
        // Should not lead to a division by zero because the pullside process stops if the car is within
        // [0.5, 0.5]. Do not move it back.
        travelTime = DIST(car->restPos.pos.x, car->restPos.pos.y, carElt->_pos_X, carElt->_pos_Y) / PULL_SPD;
        car->restPos.vel.x = (car->restPos.pos.x - carElt->_pos_X) / travelTime;
        car->restPos.vel.y = (car->restPos.pos.y - carElt->_pos_Y) / travelTime;

        carElt->_pos_X += car->restPos.vel.x * SimDeltaTime;
        carElt->_pos_Y += car->restPos.vel.y * SimDeltaTime;
        sgMakeCoordMat4(carElt->pub.posMat, carElt->_pos_X, carElt->_pos_Y, carElt->_pos_Z - carElt->_statGC_z,
            (float) RAD2DEG(carElt->_yaw), (float) RAD2DEG(carElt->_roll), (float) RAD2DEG(carElt->_pitch));

        if ((fabs(car->restPos.pos.x - carElt->_pos_X) < 0.5) && (fabs(car->restPos.pos.y - carElt->_pos_Y) < 0.5))
        {
            carElt->_state &= ~RM_CAR_STATE_PULLSIDE;
            carElt->_state |= RM_CAR_STATE_PULLDN;
        }

        return;
    }

    if (carElt->_state & RM_CAR_STATE_PULLDN)
    {
        carElt->_pos_Z -= car->restPos.vel.z * SimDeltaTime;
        sgMakeCoordMat4(carElt->pub.posMat, carElt->_pos_X, carElt->_pos_Y, carElt->_pos_Z - carElt->_statGC_z,
            (float) RAD2DEG(carElt->_yaw), (float) RAD2DEG(carElt->_roll), (float) RAD2DEG(carElt->_pitch));

        if (carElt->_pos_Z < car->restPos.pos.z)
        {
            carElt->_state &= ~RM_CAR_STATE_PULLDN;
            carElt->_state |= RM_CAR_STATE_OUT;
        }

        return;
    }

    if (carElt->_state & (RM_CAR_STATE_NO_SIMU & ~RM_CAR_STATE_PIT))
    {
        return;
    }

    if (carElt->_state & RM_CAR_STATE_PIT)
    {
        if ((s->_maxDammage) && (car->dammage > s->_maxDammage))
        {
            // Broken during pit stop.
            carElt->_state &= ~RM_CAR_STATE_PIT;
            carElt->_pit->pitCarIndex = TR_PIT_STATE_FREE;
        }
        else
        {
            return;
        }
    }

    if ((s->_maxDammage) && (car->dammage > s->_maxDammage))
    {
        carElt->_state |= RM_CAR_STATE_BROKEN;
    }
    else
    {
        carElt->_state |= RM_CAR_STATE_OUTOFGAS;
    }

    carElt->_gear = car->transmission.gearbox.gear = 0;
    carElt->_enginerpm = car->engine.rads = 0;

    if (!(carElt->_state & RM_CAR_STATE_DNF))
    {
        if (fabs(carElt->_speed_x) > 1.0)
        {
            return;
        }
    }

    carElt->_state |= RM_CAR_STATE_PULLUP;
    // RM_CAR_STATE_NO_SIMU evaluates to > 0 from here, so we remove the car from the
    // collision detection.
    SimCollideRemoveCar(car, s->_ncars);

    carElt->priv.collision = car->collision = 0;

    for(i = 0; i < 4; i++)
    {
        carElt->_skid[i] = 0;
        carElt->_wheelSpinVel(i) = 0;
        carElt->_brakeTemp(i) = 0;
    }

    carElt->pub.DynGC = car->DynGC;
    carElt->_speed_x = 0;

    // Compute the target zone for the wrecked car.
    trkPos = car->trkPos;
    if (trkPos.toRight >  trkPos.seg->width / 2.0)
    {
        while (trkPos.seg->lside != 0)
        {
            trkPos.seg = trkPos.seg->lside;
        }

        trkPos.toLeft = -3.0;
        trkFlag = TR_TOLEFT;
    }
    else
    {
        while (trkPos.seg->rside != 0)
        {
            trkPos.seg = trkPos.seg->rside;
        }

        trkPos.toRight = -3.0;
        trkFlag = TR_TORIGHT;
    }

    trkPos.type = TR_LPOS_SEGMENT;
    RtTrackLocal2Global(&trkPos, &(car->restPos.pos.x), &(car->restPos.pos.y), trkFlag);
    car->restPos.pos.z = RtTrackHeightL(&trkPos) + carElt->_statGC_z;
    car->restPos.pos.az = RtTrackSideTgAngleL(&trkPos);
    car->restPos.pos.ax = 0;
    car->restPos.pos.ay = 0;

    car->restPos.vel.z = PULL_SPD;
    travelTime = (car->restPos.pos.z + PULL_Z_OFFSET - carElt->_pos_Z) / car->restPos.vel.z;
    dang = car->restPos.pos.az - carElt->_yaw;
    FLOAT_NORM_PI_PI(dang);
    car->restPos.vel.az = dang / travelTime;
    dang = car->restPos.pos.ax - carElt->_roll;
    FLOAT_NORM_PI_PI(dang);
    car->restPos.vel.ax = dang / travelTime;
    dang = car->restPos.pos.ay - carElt->_pitch;
    FLOAT_NORM_PI_PI(dang);
    car->restPos.vel.ay = dang / travelTime;
}

void
SimCarTelemetry(int nCarIndex, bool bOn)
{
    SimTelemetry = bOn ? nCarIndex : -1;
}

void
SimUpdate(tSituation *s, double deltaTime)
{
    int i;
    int ncar;
    tCarElt *carElt;
    tCar *car;

    SimDeltaTime = (tdble) deltaTime;

    SimAtmosphereUpdate(s);

    for (ncar = 0; ncar < s->_ncars; ncar++)
    {
        SimCarTable[ncar].collision = 0;
        SimCarTable[ncar].blocked = 0;
    }

    for (ncar = 0; ncar < s->_ncars; ncar++)
    {
        car = &(SimCarTable[ncar]);
        carElt = car->carElt;

        if (carElt->_state & RM_CAR_STATE_NO_SIMU)
        {
            RemoveCar(car, s);
            continue;
        }
        else if (((s->_maxDammage) && (car->dammage > s->_maxDammage)) ||
            (car->fuel == 0) ||
            (car->carElt->_state & RM_CAR_STATE_ELIMINATED))
        {
            RemoveCar(car, s);

            if (carElt->_state & RM_CAR_STATE_NO_SIMU)
            {
                continue;
            }
        }

        if (s->_raceState & RM_RACE_PRESTART &&
                (car->carElt->_skillLevel < PRO || !(s->_features & RM_FEATURE_PENALTIES)))
        {
            car->ctrl->brakeCmd = 1.0;
            car->ctrl->clutchCmd = 1.0;
        }

        CHECK(car);

        ctrlCheck(car);
        CHECK(car);

        SimInstantReConfig(car);
        CHECK(car);

        SimSteerUpdate(car);
        CHECK(car);

        SimGearboxUpdate(car);
        CHECK(car);

        SimEngineUpdateTq(car);
        CHECK(car);

        /*SimEngineUpdateWater(car);
        CHECK(car);*/

        if (!(s->_raceState & RM_RACE_PRESTART) || car->carElt->_skillLevel == PRO)
        {
            SimCarUpdateWheelPos(car);
            CHECK(car);

            SimBrakeSystemUpdate(car);
            CHECK(car);

            SimAeroUpdate(car, s);
            CHECK(car);

            for (i = 0; i < 2; i++)
            {
                SimWingUpdate(car, i, s);
            }
            CHECK(car);

            for (i = 0; i < 4; i++)
            {
                SimWheelUpdateRide(car, i);
            }
            CHECK(car);

            for (i = 0; i < 2; i++)
            {
                SimAxleUpdate(car, i);
            }
            CHECK(car);

            for (i = 0; i < 4; i++)
            {
                SimWheelUpdateForce(car, i);
            }
            CHECK(car);

            SimTransmissionUpdate(car);
            CHECK(car);

            SimWheelUpdateRotation(car);
            CHECK(car);

            SimCarUpdate(car, s);
            CHECK(car);
        }
        else
        {
            SimTransmissionUpdate(car);
            SimEngineUpdateRpm(car, 0.0);
        }
    }

    SimCarCollideCars(s);

    /* printf ("%f - ", s->currentTime); */

    for (ncar = 0; ncar < s->_ncars; ncar++)
    {
        car = &(SimCarTable[ncar]);
        CHECK(car);
        carElt = car->carElt;

        if (carElt->_state & RM_CAR_STATE_NO_SIMU)
        {
            continue;
        }

        CHECK(car);
        SimCarUpdate2(car, s); /* telemetry */

        /* copy back the data to carElt */

        carElt->pub.DynGC = car->DynGC;
        carElt->pub.DynGCg = car->DynGCg;
        sgMakeCoordMat4(carElt->pub.posMat, carElt->_pos_X, carElt->_pos_Y, carElt->_pos_Z - carElt->_statGC_z,
                (float) RAD2DEG(carElt->_yaw), (float) RAD2DEG(carElt->_roll), (float) RAD2DEG(carElt->_pitch));
        carElt->_trkPos = car->trkPos;

        for (i = 0; i < 4; i++)
        {
            carElt->priv.wheel[i].relPos = car->wheel[i].relPos;
            carElt->_wheelSeg(i) = car->wheel[i].trkPos.seg;
            carElt->_brakeTemp(i) = car->wheel[i].brake.temp;
            carElt->pub.corner[i] = car->corner[i].pos;
        }

        carElt->_gear = car->transmission.gearbox.gear;
        carElt->_gearNext = car->transmission.gearbox.gearNext;
        carElt->_enginerpm = car->engine.rads;
        carElt->_fuel = car->fuel;
        carElt->_engineTempWater = car->engine.temp_water;
        carElt->priv.collision |= car->collision;
        carElt->_dammage = car->dammage;
        carElt->_airtemp = Tair;
        GfLogDebug(" # SimuV4 air temperature = %.8f - Engine Temperature Water = %.8f\n", K2C(carElt->_airtemp), carElt->_engineTempWater);
        carElt->_airPressure = SimAirPressure;

        carElt->_steerTqCenter = -car->ctrl->steer;
        carElt->_steerTqAlign = car->wheel[FRNT_RGT].torqueAlign + car->wheel[FRNT_LFT].torqueAlign;
    }
}

void
SimInit(int nbcars, tTrack* track)
{
    SimNbCars = nbcars;
    SimCarTable = (tCar*)calloc(nbcars, sizeof(tCar));
    PTrack = track;
    SimAtmospherePreConfig(PTrack);
    GfLogInfo("Tair in Simu = %3f - Air Pressure in Simu = %3f - Air Density in Simu = %3f\n", Tair, SimAirPressure, SimAirDensity);
    SimCarCollideInit(PTrack);
}

void
SimShutdown(void)
{
    tCar *car;
    int	 ncar;

    SimCarCollideShutdown(SimNbCars);
    if (SimCarTable) {
    for (ncar = 0; ncar < SimNbCars; ncar++) {
        car = &(SimCarTable[ncar]);
        SimEngineShutdown(car);
	delete car->options;
    }
    free(SimCarTable);
    SimCarTable = 0;
    }

    PTrack = 0;
}

/* Used for network games to update client physics */
void
UpdateSimCarTable(tDynPt DynGCG,int index)
{
    tCar *pCar = SimCarTable;
    pCar[index].DynGCg = DynGCG;
}

/* Used for network games get current physics values*/
tDynPt *
GetSimCarTable(int index)
{
    tCar *pCar = SimCarTable;
    return &pCar[index].DynGCg;
}


void
SimUpdateSingleCar(int index, double deltaTime,tSituation *s)
{
    int i;
    //int ncar;
    tCarElt *carElt;
    tCar *car;

    SimDeltaTime = (tdble) deltaTime;
    SimCarTable[index].collision = 0;
    SimCarTable[index].blocked = 0;

    car = &(SimCarTable[index]);
    carElt = car->carElt;

    CHECK(car);
    ctrlCheck(car);
    CHECK(car);
    SimInstantReConfig(car);
    CHECK(car);
    SimSteerUpdate(car);
    CHECK(car);
    SimGearboxUpdate(car);
    CHECK(car);
    SimEngineUpdateTq(car);
    CHECK(car);

    SimCarUpdateWheelPos(car);
    CHECK(car);
    SimBrakeSystemUpdate(car);
    CHECK(car);
    SimAeroUpdate(car, s);
    CHECK(car);
    for (i = 0; i < 2; i++){
        SimWingUpdate(car, i, s);
    }
    CHECK(car);
    for (i = 0; i < 4; i++){
        SimWheelUpdateRide(car, i);
    }
    CHECK(car);
    for (i = 0; i < 2; i++){
        SimAxleUpdate(car, i);
    }
    CHECK(car);
    for (i = 0; i < 4; i++){
        SimWheelUpdateForce(car, i);
    }
    CHECK(car);
    SimTransmissionUpdate(car);
    CHECK(car);
    SimWheelUpdateRotation(car);
    CHECK(car);
    SimCarUpdate(car, s);
    CHECK(car);

    /* copy back the data to carElt */

    carElt->pub.DynGC = car->DynGC;
    carElt->pub.DynGCg = car->DynGCg;
    sgMakeCoordMat4(carElt->pub.posMat, carElt->_pos_X, carElt->_pos_Y, carElt->_pos_Z - carElt->_statGC_z,
                    (float) RAD2DEG(carElt->_yaw), (float) RAD2DEG(carElt->_roll), (float) RAD2DEG(carElt->_pitch));
    carElt->_trkPos = car->trkPos;
    for (i = 0; i < 4; i++) {
        carElt->priv.wheel[i].relPos = car->wheel[i].relPos;
        carElt->_wheelSeg(i) = car->wheel[i].trkPos.seg;
        carElt->_brakeTemp(i) = car->wheel[i].brake.temp;
        carElt->pub.corner[i] = car->corner[i].pos;
    }
    carElt->_gear = car->transmission.gearbox.gear;
    carElt->_gearNext = car->transmission.gearbox.gearNext;
    carElt->_enginerpm = car->engine.rads;
    carElt->_fuel = car->fuel;
    carElt->priv.collision |= car->collision;
    carElt->_dammage = car->dammage;
    carElt->_airtemp = Tair;
    carElt->_airPressure = SimAirPressure;

    carElt->_steerTqCenter = -car->ctrl->steer;
    carElt->_steerTqAlign = car->wheel[FRNT_RGT].torqueAlign + car->wheel[FRNT_LFT].torqueAlign;
}
