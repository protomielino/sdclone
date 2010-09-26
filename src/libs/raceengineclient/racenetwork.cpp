/***************************************************************************

    file        : racenetwork.cpp
    copyright   : (C) 2009 by Brian Gavin 
    web         : www.speed-dreams.org 
    version     : $Id$

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/** @file   
    		
    @author	    Brian Gavin
    @version	$Id$
*/

#include <network.h>
#include <tgfclient.h>

#include "racesituation.h"
#include "racegl.h"
#include "racenetwork.h"


static void
reNetworkSetCarPhysics(double timeDelta,CarControlsData *pCt)
{
	tDynPt *pDynCG = NULL;
	pDynCG = ReInfo->_reSimItf.getsimcartable(pCt->startRank);

	double errX = pDynCG->pos.x-pCt->DynGCg.pos.x;
	double errY = pDynCG->pos.y-pCt->DynGCg.pos.y;
	double errZ = pDynCG->pos.z-pCt->DynGCg.pos.z;

	int idx = GetNetwork()->GetCarIndex(pCt->startRank,ReInfo->s);
	
	//Car controls (steering,gas,brake, gear
	tCarElt *pCar = ReInfo->s->cars[idx];
	pCar->ctrl.accelCmd = pCt->throttle;
	pCar->ctrl.brakeCmd = pCt->brake;
	pCar->ctrl.clutchCmd = pCt->clutch;
	pCar->ctrl.gear = pCt->gear;
	pCar->ctrl.steer = pCt->steering;

	pDynCG->pos = pCt->DynGCg.pos;
	pDynCG->acc = pCt->DynGCg.acc;
	pDynCG->vel = pCt->DynGCg.vel;

	double step = 0.0;
	if (timeDelta>0.0)
	{
		//predict car position
		while(timeDelta>0.0)
		{
			if (timeDelta>RCM_MAX_DT_SIMU)
			{
				step = RCM_MAX_DT_SIMU;
			}
			else
				step = timeDelta;

			timeDelta-=step;
			ReInfo->_reSimItf.singleupdate(pCt->startRank,step,ReInfo->s);
		}
	}

	//GfLogTrace("Network position error is %lf %lf %lf and delta is %lf\n",errX,errY,errZ,timeDelta);

	//Car physics
//	ReInfo->_reSimItf.updatesimcartable(pCt->DynGCg,pCt->startRank);

}

static void
reNetworkSetCarStatus(CarStatus *pStatus)
{
	int idx = GetNetwork()->GetCarIndex(pStatus->startRank,ReInfo->s);

	tCarElt *pCar = ReInfo->s->cars[idx];

	if (pStatus->dammage > 0.0)
		pCar->priv.dammage = pStatus->dammage;
	if (pStatus->fuel >0.0)
		pCar->priv.fuel = pStatus->fuel;
	if (pStatus->topSpeed >0.0)
		pCar->race.topSpeed = pStatus->topSpeed;

	pCar->pub.state = pStatus->state;
	

}

static void
reNetworkSetLapStatus(LapStatus *pStatus)
{
	int idx = GetNetwork()->GetCarIndex(pStatus->startRank,ReInfo->s);

	tCarElt *pCar = ReInfo->s->cars[idx];
	pCar->race.bestLapTime = pStatus->bestLapTime;
	*pCar->race.bestSplitTime = (double)pStatus->bestSplitTime;
	pCar->race.laps = pStatus->laps;
	GfLogTrace("Setting network lap status\n");
}

void
ReNetworkOneStep()
{
	tSituation *s = ReInfo->s;

	//Do network updates if needed
	//CarControlsData *pControls = NULL;
	int numCars = 0;
	double time = 0.0f;
	
	MutexData *pNData = GetNetwork()->LockNetworkData();

	numCars = pNData->m_vecCarCtrls.size();
	if (numCars>0)
	{
		for (int i=0;i<numCars;i++)
		{
			double timeDelta = s->currentTime-pNData->m_vecCarCtrls[i].time;
			if (timeDelta >= 0)
			{
				reNetworkSetCarPhysics(timeDelta,&pNData->m_vecCarCtrls[i]);
			}
			else if (timeDelta <= -1.0)
			{
				GfLogTrace("Ignoring physics packet (delta is %lf)\n", timeDelta);
			}
		}
	}

	GetNetwork()->SetCurrentTime(s->currentTime);
	pNData->m_vecCarCtrls.clear();

	//do car status updates if needed
	CarStatus *pStatus = NULL;
	numCars = pNData->m_vecCarStatus.size();
	time = 0.0f;

	if (numCars>0)
	{
		for (int i=0;i<numCars;i++)
		{
			double delta = s->currentTime-pNData->m_vecCarStatus[i].time;
			if (delta>=0)
				reNetworkSetCarStatus(&pNData->m_vecCarStatus[i]);
		}
	}

	std::vector<CarControlsData>::iterator p = pNData->m_vecCarCtrls.begin();
	while(p!=pNData->m_vecCarCtrls.end())
	{
		if(p->time<s->currentTime)
			p = pNData->m_vecCarCtrls.erase(p);
		else 
			p++;
	}

	//do lap status updates if needed
	LapStatus *pLapStatus = NULL;
	numCars = 0;
	time = 0.0f;
	numCars = pNData->m_vecLapStatus.size();
	if (numCars>0)
	{
		for (int i=0;i<numCars;i++)
		{
			reNetworkSetLapStatus(&pNData->m_vecLapStatus[i]);
		}
	}

	pNData->m_vecLapStatus.clear();

	GetNetwork()->UnlockNetworkData();
}

int
ReNetworkWaitReady()
{
	bool bWaitFinished = false;
	
	if (!GetNetwork())
		bWaitFinished = true;

	// If network race wait for other players and start when the server says too
	else if (GetClient())
	{
		GetClient()->SendReadyToStartPacket();
		ReInfo->s->currentTime = GetClient()->WaitForRaceStart();
		GfLogInfo("Client beginning race in %lf seconds!\n", ReInfo->s->currentTime);
		bWaitFinished = true;
	}
	
	else if (GetServer())
	{
		if (GetServer()->ClientsReadyToRace())
		{
			ReInfo->s->currentTime = GetServer()->WaitForRaceStart();
			GfLogInfo("Server beginning race in %lf seconds!\n", ReInfo->s->currentTime);
			bWaitFinished = true;
		}
	}

	int mode = RM_SYNC;
	if (bWaitFinished)
	{
		ReSetRaceBigMsg("");
		mode |= RM_NEXT_STEP;
	}
	else
	{
		ReSetRaceBigMsg("Waiting for online players");
		GfuiDisplay();
		ReInfo->_reGraphicItf.refresh(ReInfo->s);
		GfelPostRedisplay();	/* Callback -> reDisplay */
	}

	return mode;
}

void
ReNetworkCheckEndOfRace()
{
	// Check for end of online race.
	if (GetNetwork() && GetNetwork()->FinishRace(ReInfo->s->currentTime))
		ReInfo->s->_raceState = RM_RACE_ENDED;
}
