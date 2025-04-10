/***************************************************************************

    file                 : engine.cpp
    created              : Sun Mar 19 00:06:55 CET 2000
    copyright            : (C) 2000 by Eric Espie
    email                : torcs@free.fr

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
#include "timeanalysis.h"

tdble CalculateTorque (tEngine* engine, tdble rads)
{
	double StartTimeStamp = RtTimeStamp(); // Profiling test

	tEngineCurve *curve = &(engine->curve);
	tdble Tq = curve->data[0].Tq;
	tdble Tmax = curve->data[0].Tq;
	tdble Tmin = curve->data[0].Tq * 0.5f;
	tdble rpm_max = curve->data[0].rads;
	tdble rpm_min = -1.0;
	tdble alpha = 0.0;
	for (int i = 0; i < curve->nbPts; i++) {
		if (rads > curve->data[i].rads) {
			Tmin = curve->data[i].Tq;
			rpm_min = curve->data[i].rads;
			if (i<engine->curve.nbPts) {
				Tmax = curve->data[i+1].Tq;
				rpm_max = curve->data[i+1].rads;
			}
		}
	}
	alpha = (rads - rpm_min) / (rpm_max - rpm_min);
	Tq = (float)((1.0-alpha) * Tmin + alpha * Tmax);

  	SimTicks += RtDuration(StartTimeStamp); // Profiling test

	return Tq;
}

tdble CalculateTorque2 (tEngine* engine, tdble rads)
{
	tEngineCurve *curve = &(engine->curve);

	int i = engine->lastInterval;

	tdble rpm_min = curve->data[i].rads;
	tdble rpm_max = curve->data[i+1].rads;

	if (rads > rpm_max) {
		if (i < curve->nbPts) {
			rpm_min = rpm_max;
			engine->lastInterval = ++i;
			rpm_max = curve->data[i+1].rads;
		}
	}
	else if (rads < rpm_min) {
		if (i > 0) {
			rpm_max = rpm_min;
			engine->lastInterval = --i;
			rpm_min = curve->data[i].rads;
		}
	}

	// In case of large changes of rads
	if ((rads < rpm_min) || (rads > rpm_max))
		return CalculateTorque2 (engine, rads);
	else {
	  tdble alpha = (rads - rpm_min) / (rpm_max - rpm_min);
	  return (float)((1.0-alpha) * curve->data[i].Tq + alpha * curve->data[i+1].Tq);
	}
}

/*
  For profiling tests:

  Call CalculateTorque3 instead of CalculateTorque2
  to get the times without overlapping
  caused by recursive calls to CalculateTorque2 */

tdble CalculateTorque3 (tEngine* engine, tdble rads)
{
	double StartTimeStamp = RtTimeStamp(); // Profiling test

	tEngineCurve *curve = &(engine->curve);

	int i = engine->lastInterval;

	tdble rpm_min = curve->data[i].rads;
	tdble rpm_max = curve->data[i+1].rads;

	if (rads > rpm_max) {
		if (i < curve->nbPts) {
			rpm_min = rpm_max;
			engine->lastInterval = ++i;
			rpm_max = curve->data[i+1].rads;
		}
	}
	else if (rads < rpm_min) {
		if (i > 0) {
			rpm_max = rpm_min;
			engine->lastInterval = --i;
			rpm_min = curve->data[i].rads;
		}
	}

	tdble Tq;

	// In case of large changes of rads
	if ((rads < rpm_min) || (rads > rpm_max))
		Tq = CalculateTorque2 (engine, rads);
	else {
	  tdble alpha = (rads - rpm_min) / (rpm_max - rpm_min);
	  Tq = (float)((1.0-alpha) * curve->data[i].Tq + alpha * curve->data[i+1].Tq);
	}

  	SimTicks2 += RtDuration(StartTimeStamp); // Profiling test

	return Tq;
}

void
SimEngineConfig(tCar *car)
{
    void	*hdle = car->params;
    int		i;
    tdble	maxTq;
    tdble	rpmMaxTq = 0;
    char	idx[64];
    tEngineCurveElem *data;
    struct tEdesc {
	    tdble rpm;
	    tdble tq;
    } *edesc;


	car->engine.lastInterval = 0; // Initialize interval
    car->engine.revsLimiter = GfParmGetNum(hdle, SECT_ENGINE, PRM_REVSLIM, (char*)NULL, 800);
    car->carElt->_enginerpmRedLine = car->engine.revsLimiter;
    car->engine.revsMax     = GfParmGetNum(hdle, SECT_ENGINE, PRM_REVSMAX, (char*)NULL, 1000);
    car->carElt->_enginerpmMax = car->engine.revsMax;
    car->engine.tickover    = GfParmGetNum(hdle, SECT_ENGINE, PRM_TICKOVER, (char*)NULL, 150);
    car->engine.I           = GfParmGetNum(hdle, SECT_ENGINE, PRM_INERTIA, (char*)NULL, 0.2423f);
    car->engine.fuelcons    = GfParmGetNum(hdle, SECT_ENGINE, PRM_FUELCONS, (char*)NULL, 0.0622f);
    car->engine.brakeCoeff  = GfParmGetNum(hdle, SECT_ENGINE, PRM_ENGBRKCOEFF, (char*)NULL, 0.05f);
	car->engine.pressure = 0.0f;
	car->engine.exhaust_pressure = 0.0f;
	car->engine.exhaust_refract = 0.1f;
	car->engine.Tq_response = 0.0f;
    car->engine.I_joint = car->engine.I;
    sprintf(idx, "%s/%s", SECT_ENGINE, ARR_DATAPTS);
    car->engine.curve.nbPts = GfParmGetEltNb(hdle, idx);
    edesc = (struct tEdesc*)malloc((car->engine.curve.nbPts + 1) * sizeof(struct tEdesc));

    for (i = 0; i < car->engine.curve.nbPts; i++) {
		sprintf(idx, "%s/%s/%d", SECT_ENGINE, ARR_DATAPTS, i+1);
		edesc[i].rpm = GfParmGetNum(hdle, idx, PRM_RPM, (char*)NULL, car->engine.revsMax);
		edesc[i].tq  = GfParmGetNum(hdle, idx, PRM_TQ, (char*)NULL, 0);
    }
    if (i > 0) {
        edesc[i].rpm = edesc[i - 1].rpm;
        edesc[i].tq = edesc[i - 1].tq;
    }

    maxTq = 0;
	car->engine.curve.maxPw = 0;
    car->engine.curve.data = (tEngineCurveElem *)malloc(car->engine.curve.nbPts * sizeof(tEngineCurveElem));

    for(i = 0; i < car->engine.curve.nbPts; i++) {
		data = &(car->engine.curve.data[i]);

		data->rads = edesc[i].rpm;
		if ((data->rads>=car->engine.tickover)
			&& (edesc[i].tq > maxTq)
			&& (data->rads < car->engine.revsLimiter)) {
			maxTq = edesc[i].tq;
			rpmMaxTq = data->rads;
		}
		if ((data->rads>=car->engine.tickover)
			&& (data->rads * edesc[i].tq > car->engine.curve.maxPw)
			&& (data->rads < car->engine.revsLimiter)) {
			car->engine.curve.TqAtMaxPw = edesc[i].tq;
			car->engine.curve.maxPw = data->rads * edesc[i].tq;
			car->engine.curve.rpmMaxPw = data->rads;
		}
		data->Tq = edesc[i].tq;
    }
    car->engine.curve.maxTq = maxTq;
    car->carElt->_engineMaxTq = maxTq;
    car->carElt->_enginerpmMaxTq = rpmMaxTq;
    car->carElt->_engineMaxPw = car->engine.curve.maxPw;
    car->carElt->_enginerpmMaxPw = car->engine.curve.rpmMaxPw;
	//printf ("%fNm@%frpm, %fKW@%f rpm\n",
	//  car->carElt->_engineMaxTq,
	//	car->carElt->_enginerpmMaxTq * (30.0 / M_PI),
	//	car->carElt->_engineMaxPw * 0.001,
	//	car->carElt->_enginerpmMaxPw * (30.0 / M_PI)
	//	);
	float X=urandom();
    car->engine.rads = X*car->engine.tickover+(1-X)*car->engine.revsMax;

    /*sanity check of rev limits*/
    if (car->engine.curve.nbPts > 0 && car->engine.revsMax > car->engine.curve.data[car->engine.curve.nbPts-1].rads) {
        car->engine.revsMax = car->engine.curve.data[car->engine.curve.nbPts-1].rads;
        GfLogWarning("Revs maxi bigger than the maximum RPM in the curve data.\nIt is set to %g.\n",car->engine.revsMax);
    }
    if (car->engine.revsLimiter > car->engine.revsMax) {
        car->engine.revsLimiter = car->engine.revsMax;
        GfLogWarning("Revs limiter is bigger than revs maxi.\nIt is set to %g.\n",car->engine.revsLimiter);
    }

#if 0
	// TEST TORQUE FUNCTION
	for (float rads=1.0; rads<car->engine.revsMax; rads+=1.0) {
		float Tq = CalculateTorque(&(car->engine), rads);
		float Tq2 = CalculateTorque2(&(car->engine), rads);
		printf ("%f %f %f %d #TORQUE\n", 30.0*rads/M_PI, Tq, Tq2, engine->lastInterval);
	}
#endif
    free(edesc);
}

/* Update torque output with engine rpm and accelerator command */
void
SimEngineUpdateTq(tCar *car)
{
    tEngine	*engine = &(car->engine);
    tEngineCurve *curve = &(engine->curve);
    tTransmission	*trans = &(car->transmission);
    tClutch		*clutch = &(trans->clutch);


	// set clutch on when engine revs too low
	if (engine->rads < engine->tickover) {
		clutch->state = CLUTCH_APPLIED;
		clutch->transferValue = 0.0f;
		//		engine->rads = engine->tickover;
	}

    if ((car->fuel <= 0.0) || (car->carElt->_state & (RM_CAR_STATE_BROKEN | RM_CAR_STATE_ELIMINATED))) {
		car->ctrl->accelCmd = 0.0;
    }

    engine->rads = MIN(engine->rads, engine->revsMax);
    const tdble static_friction = 0.1f;
	tdble EngBrkK = curve->TqAtMaxPw * engine->brakeCoeff * (static_friction + (1.0f - static_friction)*(engine->rads) / (engine->revsMax));

    if (engine->rads < engine->tickover) {
		engine->Tq = 0.0f;
		engine->rads = engine->tickover;
	} else {
		// tdble Tq_max = CalculateTorque2(engine, engine->rads); // Use this for release
		tdble Tq_max = CalculateTorque3(engine, engine->rads);  // Use this for profiling tests
		tdble alpha = car->ctrl->accelCmd;
        if (alpha < 1) {
            //tdble da = 1 /(1 - alpha); // flow
            alpha *= exp(MIN(0,alpha - engine->rads/engine->revsMax));
            if (alpha < 0) {
                alpha = 0;
            }
            if (alpha > 1) {
                alpha = 1;
            }
        }
        if (engine->rads > engine->revsLimiter) {
            alpha = 0.0;
        }
		tdble Tq_cur = (Tq_max + EngBrkK)* alpha;
		engine->Tq =  Tq_cur;
		if (engine->rads > engine->tickover) {
			engine->Tq -= EngBrkK;
		}

		tdble cons = Tq_cur * 0.75f;
		if (cons > 0) {
			car->fuel -= (float)(cons * engine->rads * engine->fuelcons * 0.0000001 * SimDeltaTime);
		}
        car->fuel = (tdble) (MAX(car->fuel, 0.0));
    }
}

/*
 * Function
 *	SimEngineUpdateRpm
 *
 * Description
 *	update engine RPM with wheels RPM
 *
 * Parameters
 *	car and axle RPM
 *
 * Return
 *	axle rpm for wheel update
 *	0 if no wheel update
 */
tdble
SimEngineUpdateRpm(tCar *car, tdble axleRpm)
{
    tTransmission	*trans = &(car->transmission);
    tClutch		*clutch = &(trans->clutch);
    tEngine		*engine = &(car->engine);
    float		freerads;
    float		transfer;

#if 0
    if (car->fuel <= 0.0) {
		clutch->state = CLUTCH_APPLIED;
		clutch->transferValue = 0.0;
    }
#endif

	freerads = engine->rads;
	freerads +=engine->Tq / (  engine->I) * SimDeltaTime;
	if (freerads > engine->revsMax) {
	  freerads = engine->revsMax;
        }
	tdble dp = engine->pressure;
	engine->pressure = (float)(engine->pressure*.9 + .1*engine->Tq);
	dp = (float)(0.01*fabs(engine->pressure - dp));
	dp = fabs(dp);
	tdble rth = urandom();
	if (dp>rth) {
		engine->exhaust_pressure += rth;
	}
	engine->exhaust_pressure*=.9f;


	car->carElt->priv.smoke += 5.0f*engine->exhaust_pressure;
	car->carElt->priv.smoke *= exp(-0.0001f * engine->rads);
#if 0
	if (engine->exhaust_pressure>(engine->exhaust_refract)) {
		//car->carElt->priv.smoke += engine->exhaust_pressure;//engine->exhaust_refract;
		engine->exhaust_pressure -= engine->exhaust_refract;
		engine->exhaust_refract = 0.05;
	}
	if (engine->exhaust_refract<0.01) {
		engine->exhaust_refract = 0.01;
		car->carElt->priv.smoke = 0.0;
	} else {
		engine->exhaust_refract*=0.99;
		car->carElt->priv.smoke *= 0.99;
	}
#endif

    transfer = 0.0;
    float alpha = 0.1f; // transition coefficient
    float ttq = 0.0f;
	float I_response = trans->differential[0].feedBack.I + trans->differential[1].feedBack.I;
	engine->Tq_response = 0.0;
	tdble dI = fabs(trans->curI - engine->I_joint);
	tdble sdI = dI;

	if (sdI>1.0) sdI = 1.0;

	engine->I_joint = (float)(engine->I_joint*(1.0-alpha) +  alpha*trans->curI);

    if ((clutch->transferValue > 0.01) && (trans->gearbox.gear)) {

		transfer = clutch->transferValue * clutch->transferValue * clutch->transferValue * clutch->transferValue;

		ttq = (float)(dI* tanh(0.01*(axleRpm * trans->curOverallRatio * transfer + freerads * (1.0-transfer) -engine->rads))*100.0);
		engine->rads = (float)((1.0-sdI) * (axleRpm * trans->curOverallRatio * transfer + freerads * (1.0-transfer)) + sdI *(engine->rads + ((ttq)*SimDeltaTime)/(engine->I)));
		if (engine->rads < 0.0) {
			engine->rads = 0;
            engine->Tq = 0.0;
		}
    } else {
		engine->rads = freerads;
    }
	if (engine->rads < engine->tickover) {
	    engine->rads = engine->tickover;
		engine->Tq = 0.0;
	} else if (engine->rads > engine->revsMax) {
            engine->rads = engine->revsMax;
            return engine->revsMax / trans->curOverallRatio;
        }

    if ((trans->curOverallRatio!=0.0) && (I_response > 0)) {
		return axleRpm - sdI * ttq * trans->curOverallRatio   * SimDeltaTime / ( I_response);
    } else {
		return 0.0;
    }
}

void
SimEngineShutdown(tCar *car)
{
    free(car->engine.curve.data);
    //GfOut("#Total Time CalculateTorque  used: %g sec\n",SimTicks/1000.0);
    GfOut("#Total Time CalculateTorque2 used: %g sec\n",SimTicks2/1000.0);  // Profiling test
}
