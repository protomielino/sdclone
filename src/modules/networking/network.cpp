// -*- Mode: c++ -*-
/***************************************************************************
    file                 : network.cpp
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

/*
Overview
The network file is used for networked game play.  The server 
computer handles all of the physics and robot AI.  The server
sends car control(steering,brake,gas) frequently in unreliable
packets.  The client uses these values to figure out car position and
does its own physics calculation until the server sends the position
information.  

Every CAR_CONTROL_UPDATE seconds the server sends out detailed position 
information in a reliable ENetPacket.  All cars position information is updated
based on the server values.
*/ 

// Warning this code is VERY rough and unfinished

// TODO: Make a real SD module (dynamically loadable, like simuvx, human, ssggraph ...).


#include <cstdio>
#include <SDL/SDL.h>
#include <SDL/SDL_thread.h>

#include "network.h"


bool g_bInit = false;

bool g_bClient = false;
bool g_bServer = false;
Server g_server;
Client g_client;
SDL_TimerID g_timerId;


MutexData::MutexData()
{
	m_networkMutex =  SDL_CreateMutex();
}

MutexData::~MutexData()
{
	SDL_DestroyMutex ( m_networkMutex );
}
	
void MutexData::Lock() 
{
	SDL_mutexP ( m_networkMutex );
}

void MutexData::Unlock() 
{
	SDL_mutexV ( m_networkMutex );
}

void MutexData::Init()
{
	m_vecCarCtrls.clear();
	m_vecCarStatus.clear();
	m_vecLapStatus.clear();
	m_finishTime = 0.0f;
}

//============================================================

void ServerMutexData::Init()
{
	m_vecNetworkPlayers.clear();
}

ServerMutexData::ServerMutexData()
{
	m_networkMutex=  SDL_CreateMutex();
}

ServerMutexData::~ServerMutexData()
{
	SDL_DestroyMutex (m_networkMutex );
}
	
void ServerMutexData::Lock() 
{
	SDL_mutexP ( m_networkMutex );
}

void ServerMutexData::Unlock() 
{
	SDL_mutexV ( m_networkMutex );
}

//============================================================

Network::Network()
{
	m_strClass = "network";
	m_bRaceInfoChanged = false;
	m_bRefreshDisplay = false;
	
	m_sendCtrlTime = 0.0;
	m_sendCarDataTime = 0.0;
	m_pHost = NULL;
	m_currentTime = 0.0;
}

Network::~Network()
{
}

void Network::RaceInit(tSituation *s)
{
	m_sendCtrlTime = 0.0;
	m_sendCarDataTime = 0.0;
	m_timePhysics = 0.0;
	m_currentTime = 0.0;

	m_mapRanks.clear();
	for (int i = 0; i < s->_ncars; i++) 
	{
		tCarElt *pCar = s->cars[i];
		m_mapRanks[i] = pCar->info.startRank;
	}
		
	m_NetworkData.Init();

}

void Network::RaceDone()
{
	m_bRaceActive = false;
	m_bBeginRace = false;
	m_bPrepareToRace = false;
	m_bRaceInfoChanged = false;
	m_bTimeSynced = false;
	m_sendCtrlTime = 0.0;
	m_sendCarDataTime = 0.0;
	m_timePhysics = -2.0;

	m_mapRanks.clear();

}


int Network::GetDriverStartRank(int idx) 
{
	std::map<int,int>::iterator p;
	p = m_mapRanks.find(idx);

	return p->second;
}

//============================================================

Driver::Driver()
{
	//Initialize values
	idx = -1;
	memset(name,0,sizeof(name));
	memset(car,0,sizeof(car));
	memset(team,0,sizeof(team));
	memset(author,0,sizeof(author));
	racenumber = 1;
	memset(skilllevel,0,sizeof(skilllevel));
	red = 1.0;
	green = 1.0;
	blue = 1.0;
	hostPort = 0;
	client = false;
	memset(module,0,sizeof(module));
	memset(type,0,sizeof(type));
}

bool NetworkInit();

Network *GetNetwork()
{
	if (!g_bInit)
		NetworkInit();

	if (g_bServer)
		return &g_server;

	if (g_bClient)
		return &g_client;

	return NULL;
}


int Network::GetCarIndex(int startRank,tSituation *s)
{
	for (int i=0;i<s->_ncars;i++)
	{
		if (startRank == s->cars[i]->info.startRank)
			return i;
	}

	assert(false);
	return -1;
}

bool Network::IsServerMode() 
{
	if (m_strClass == (char*)"server")
		return true;

	return false;
}

void Network::SetRefreshDisplay(bool bStatus)
{
	m_bRefreshDisplay = bStatus;
	if (!bStatus)
		GfOut("refreshdisplay false\n");
}

void Network::SetRaceInfoChanged(bool bStatus)
{
	m_bRaceInfoChanged = bStatus;
	if (bStatus)
		m_bRefreshDisplay = true;

	if (!bStatus)
		GfOut("raceinfo false\n");
}

bool Network::IsClientMode() 
{
	if (m_strClass == (char*)"client")
		return true;

	return false;
}

int Network::GetNetworkHumanIdx()
{
	
	assert(m_strDriverName!="");
	int idx = 1;

	char buf[255];
	sprintf(buf,"drivers/networkhuman/networkhuman.xml");
	void *params = GfParmReadFileLocal(buf,GFPARM_RMODE_REREAD);
	assert(params);
	char path2[256];

	int i=0;
	const char *pName = NULL;


	do
	{
		i++;
		sprintf(path2, "Robots/index/%d",i);
		pName = GfParmGetStr(params, path2, "name",NULL);
		if (pName)
		{		
			if (strcmp(m_strDriverName.c_str(),pName)==0)
			{
				idx = i;
				break;
			}
		}	
	}
	while(pName!=NULL);
	GfParmReleaseHandle(params);

	return idx;
}

void Network::SetRaceXMLFile(char const*pXmlFile)
{
	m_strRaceXMLFile=pXmlFile;
}

int Network::GetPlayerCarIndex(tSituation *s)
{
	int i=0;
	while (s->cars[i]->info.startRank != (m_driverIdx-1))
		i++;

	return i;
}

void Network::ClearLocalDrivers()
{
	m_setLocalDrivers.clear();
}

void Network::SetDriverName(char *pName)
{
	m_strDriverName = pName;
	printf("\nSetting driver name: %s\n", m_strDriverName.c_str());
}

const char *Network::GetDriverName()
{
	return m_strDriverName.c_str();
}

void Network::SetLocalDrivers()
{
}

void Network::SetCarInfo(const char *pszName)
{
}

bool Network::FinishRace(double time) 
{
	MutexData *pNData = LockNetworkData();
	double finishTime = pNData->m_finishTime;
	UnlockNetworkData();

	if (finishTime<=0.0)
		return false;

	if (time<finishTime)
		return false;

	GfOut("Finishing network race\n");
	return true;	
}

MutexData * Network::LockNetworkData() 
{
	m_NetworkData.Lock();
	return & m_NetworkData;
}

void Network::UnlockNetworkData()
{
	m_NetworkData.Unlock();
}

void Network::BroadcastPacket(ENetPacket *pPacket,enet_uint8 channel)
{
}

int	Network::GetDriverIdx()
{

	int nhidx = GetNetworkHumanIdx();

	assert(m_strRaceXMLFile!="");

	void *params = GfParmReadFileLocal(m_strRaceXMLFile.c_str(),GFPARM_RMODE_STD);
	assert(params);

    int nCars = GfParmGetEltNb(params, RM_SECT_DRIVERS);	
	//Gather vector of all non human drivers
	std::vector<Driver> vecRDrivers;
	for (int i=1;i<=nCars;i++)
	{
		Driver driver;
		ReadDriverData(driver,i,params);
		if ((driver.idx == nhidx)&&(strcmp(NETWORKROBOT,driver.module)==0))
		{
			GfParmReleaseHandle(params);
			return i;
		}
	}

	GfOut("\n\n\nUnable to GetDriverIdx %s\n",m_strDriverName.c_str());
	return -1;
}

void Network::ReadDriverData(Driver &driver,int index,void *params)
{
	char path2[256];
	sprintf(path2, "%s/%d", RM_SECT_DRIVERS, index);
	const char *pMod = GfParmGetStr(params, path2, RM_ATTR_MODULE,NULL);
	strncpy(&driver.module[0],pMod,64);
	driver.idx = (int)GfParmGetNum(params, path2, RM_ATTR_IDX, NULL,-1);
	
	
}

void Network::WriteDriverData(Driver driver,int index,void *params)
{
	char path2[256];
	sprintf(path2, "%s/%d", RM_SECT_DRIVERS, index);
	GfParmSetStr(params, path2, RM_ATTR_MODULE,driver.module);
	GfParmSetNum(params, path2, RM_ATTR_IDX, NULL,(tdble)driver.idx);

}

std::string Network::GetNetworkDriverName()
{
	return m_strDriverName;
}

bool Network::SetCurrentDriver()
{

	void *params = GfParmReadFileLocal("config/graph.xml",GFPARM_RMODE_REREAD);
	assert(params);

	const char *pName =GfParmGetStr(params, RM_SECT_HEADER, RM_ATTR_NAME, "");
	

	std::string strDriver = GetNetworkDriverName();
	if (strDriver =="")
		return false;

	char path[255];
	sprintf(path, "%s/%d", GR_SCT_DISPMODE, 0);
	GfParmSetStr(params, path, GR_ATT_CUR_DRV, strDriver.c_str());

	//Save our changes
	GfParmWriteFileLocal("config/graph.xml", params, pName);

	GfParmReleaseHandle(params);
	return true;
}

void Network::SendLapStatusPacket(tCarElt *pCar)
{

	LapStatus status;

	status.bestLapTime = (float)pCar->race.bestLapTime;
	status.bestSplitTime = (float)*pCar->race.bestSplitTime;
	status.laps = pCar->race.laps;
	status.startRank =  pCar->info.startRank;

	int packetSize = 1+(sizeof(LapStatus));
    unsigned char packetId = LAPSTATUS_PACKET;

	unsigned char *pData = new unsigned char[packetSize];
	unsigned char *pDataStart = pData;

	memcpy(pData,&packetId,1);
	pData++;
	memcpy(pData,&status,sizeof(status));

	ENetPacket * pPacket = enet_packet_create (pDataStart, 
                                              packetSize, 
                                              ENET_PACKET_FLAG_RELIABLE);

	BroadcastPacket(pPacket,RELIABLECHANNEL);
	
	delete [] pDataStart;
}


void Network::SendCarStatusPacket(tSituation *s,bool bForce)
{
	if (s->currentTime<0.0)
		return;

	//Send carinfo packet when enough time has passed(CAR_DATA_UPDATE)
	if (((m_sendCarDataTime+CAR_DATA_UPDATE)>s->currentTime)&&(!bForce))
	{
		return;
	}

	std::vector<CarStatusPacked> vecCarStatus;
	double time = 0.0;


	//Pack controls values to reduce data size of packet
	for (int i = 0; i < s->_ncars; i++) 
	{
		tCarElt *pCar = s->cars[i];
		//Only transmit local drivers to other clients
		if (m_setLocalDrivers.find(pCar->info.startRank)!=m_setLocalDrivers.end())
		{
			GfOut("Sending car info: %s,startRank=%i\n",pCar->info.name,pCar->info.startRank);
			CarStatusPacked status;
			status.topSpeed = pCar->race.topSpeed;
			status.state = pCar->pub.state;
			status.startRank = pCar->info.startRank;
			status.dammage = pCar->priv.dammage;
			status.fuel = pCar->priv.fuel;

			vecCarStatus.push_back(status);
		}

	}

	time = s->currentTime;
	m_sendCarDataTime = s->currentTime;


	int iNumCars = vecCarStatus.size();
	int packetSize = 1+sizeof(time)+iNumCars*sizeof(iNumCars)+iNumCars*(sizeof(CarStatusPacked));
    	unsigned char packetId = CARSTATUS_PACKET;
	unsigned char *pData = new unsigned char[packetSize];
	unsigned char *pDataStart = pData;

	memcpy(pData,&packetId,1);
	pData++;
	memcpy(pData,&time,sizeof(time));
	pData+=sizeof(time);
	memcpy(pData,&iNumCars,sizeof(iNumCars));
	pData+=sizeof(iNumCars);
	for (int i=0;i<iNumCars;i++)
	{
		memcpy(pData,(unsigned char*)&vecCarStatus[i],sizeof(CarStatusPacked));
		pData+=sizeof(CarStatusPacked);
	}

	ENetPacket * pPacket = enet_packet_create (pDataStart, 
                                              packetSize, 
                                              ENET_PACKET_FLAG_RELIABLE);

	BroadcastPacket(pPacket,RELIABLECHANNEL);
	
	delete [] pDataStart;
}

void Network::SendCarControlsPacket(tSituation *s)
{
	if (s->currentTime<0.0)
		return;

	SendCarStatusPacket(s,false);

	//Send carinfo packet when enough time has passed(CAR_CONTROL_UPDATE)
	if ((m_sendCtrlTime+CAR_CONTROL_UPDATE)>s->currentTime)
	{
		return;
	}

	std::vector<CarControlsPacked> vecPackedCtrls;
	double time = 0.0;


	//Pack controls values to reduce data size of packet
	for (int i = 0; i < s->raceInfo.ncars; i++) 
	{
		tCarElt *pCar = s->cars[i];
		//Only transmit local drivers to other clients
		if (m_setLocalDrivers.find(pCar->info.startRank)!=m_setLocalDrivers.end())
		{
			CarControlsPacked ctrl;
			ctrl.gear = pCar->ctrl.gear;
			ctrl.brake = (short)(pCar->ctrl.brakeCmd*256);
			ctrl.steering = (short)(pCar->ctrl.steer*256);
			ctrl.throttle = (short)(pCar->ctrl.accelCmd*256);
			ctrl.clutch = (short)(pCar->ctrl.clutchCmd*256);
			
			memcpy(&ctrl.DynGCg,&pCar->pub.DynGCg,sizeof(tDynPt));
			
			ctrl.startRank = pCar->info.startRank;
			vecPackedCtrls.push_back(ctrl);
		}

	}
	time = s->currentTime;
	m_sendCtrlTime = s->currentTime;


	int iNumCars = vecPackedCtrls.size();
	int packetSize = 1+sizeof(time)+iNumCars*sizeof(iNumCars)+iNumCars*(sizeof(CarControlsPacked));
    unsigned char packetId = CARCONTROLS_PACKET;
	unsigned char *pData = new unsigned char[packetSize];
	unsigned char *pDataStart = pData;

	memcpy(pData,&packetId,1);
	pData++;
	memcpy(pData,&time,sizeof(time));
	pData+=sizeof(time);
	memcpy(pData,&iNumCars,sizeof(iNumCars));
	pData+=sizeof(iNumCars);
	for (int i=0;i<iNumCars;i++)
	{
		memcpy(pData,(unsigned char*)&vecPackedCtrls[i],sizeof(CarControlsPacked));
		pData+=sizeof(CarControlsPacked);
	}

	ENetPacket * pPacket = enet_packet_create (pDataStart, 
                                              packetSize, 
                                              ENET_PACKET_FLAG_UNSEQUENCED);

	BroadcastPacket(pPacket,UNRELIABLECHANNEL);
	
	delete [] pDataStart;

}

void Network::ReadLapStatusPacket(ENetPacket *pPacket)
{
	unsigned char *pData = &pPacket->data[1];
	
	LapStatus lstatus;
	//time
	double packettime=0;
	memcpy(&lstatus,pData,sizeof(lstatus));
	
	MutexData *pNData = LockNetworkData();
	bool bFound = false;
	for (unsigned int i=0;i<pNData->m_vecLapStatus.size();i++)
	{
		if (pNData->m_vecLapStatus[i].startRank = lstatus.startRank)
		{
			bFound = true;
			pNData->m_vecLapStatus[i] = lstatus;
		}
	}

	if (!bFound)
		pNData->m_vecLapStatus.push_back(lstatus);
	
	UnlockNetworkData();
}


void Network::ReadCarStatusPacket(ENetPacket *pPacket)
{

	unsigned char *pData = &pPacket->data[1];
	
	//time
	double packettime=0;
	memcpy(&packettime,pData,sizeof(packettime));
	pData+=sizeof(packettime);


	int iNumCars = 0;
	memcpy(&iNumCars,pData,sizeof(iNumCars));
	pData+=sizeof(iNumCars);

	MutexData *pNData = LockNetworkData();
		
	//Car conrols values (steering,brake,gas,and etc
	for (int i=0;i<iNumCars;i++)
	{
		CarStatusPacked statusPacked;
		memcpy(&statusPacked,pData,sizeof(CarStatusPacked));
		
		//Unpack values
		CarStatus status;

		status.state = statusPacked.state;
		status.startRank =statusPacked.startRank;

		status.topSpeed = statusPacked.topSpeed;
		status.fuel = statusPacked.fuel;
		status.dammage = statusPacked.dammage;
		
		status.time = packettime;
	
		
		bool bFound = false;
		for (unsigned int i=0;i<pNData->m_vecCarStatus.size();i++)
		{
			if (pNData->m_vecCarStatus[i].startRank == status.startRank)
			{
				bFound = true;
				//Only use the data if the time is newer.  Prevent out of order packet
				if (pNData->m_vecCarStatus[i].time < status.time)
				{
					pNData->m_vecCarStatus[i] = status;
				}
			}
		}

		if (!bFound)
			pNData->m_vecCarStatus.push_back(status);

		pData+=sizeof(CarStatusPacked);
	}

	UnlockNetworkData();


}

void Network::GetHostSettings(std::string &strCarCat,bool &bCollisions)
{
	assert(m_strRaceXMLFile!="");

	void *params = GfParmReadFileLocal(m_strRaceXMLFile.c_str(),GFPARM_RMODE_STD);
	assert(params);

	strCarCat = GfParmGetStr(params, RM_SECT_HEADER,RM_ATTR_CAR_CATEGORY,"All");
	//TODO
	bCollisions = true;
}

void Network::ReadCarControlsPacket(ENetPacket *pPacket)
{
	unsigned char *pData = &pPacket->data[1];
	
	//time
	double packettime=0;
	memcpy(&packettime,pData,sizeof(packettime));
	pData+=sizeof(packettime);


	int iNumCars = 0;
	memcpy(&iNumCars,pData,sizeof(iNumCars));
	pData+=sizeof(iNumCars);


	MutexData *pNData = LockNetworkData();
	
	//Car conrols values (steering,brake,gas,and etc
	for (int i=0;i<iNumCars;i++)
	{
		CarControlsPacked ctrlPacked;
		memcpy(&ctrlPacked,pData,sizeof(CarControlsPacked));
		
		//Unpack values
		CarControlsData ctrl;	
		ctrl.throttle = (float)(ctrlPacked.throttle/256.0);
		ctrl.brake = (float)(ctrlPacked.brake/256.0);
		ctrl.clutch = (float)(ctrlPacked.clutch/256.0);
		ctrl.gear = ctrlPacked.gear;
		ctrl.steering = (float)(ctrlPacked.steering/256.0);
		ctrl.DynGCg = ctrlPacked.DynGCg;
		ctrl.startRank = ctrlPacked.startRank;
		ctrl.time = packettime;
	
		
		bool bFound = false;
		for (unsigned int i=0;i<pNData->m_vecCarCtrls.size();i++)
		{
			if (pNData->m_vecCarCtrls[i].startRank == ctrl.startRank)
			{
				bFound = true;
				//Only use the data if the time is newer.  Prevent out of order packet
				if (pNData->m_vecCarCtrls[i].time < ctrl.time)
				{
					pNData->m_vecCarCtrls[i] = ctrl;
				}
			}
		}

		if (!bFound)
			pNData->m_vecCarCtrls.push_back(ctrl);

		pData+=sizeof(ctrlPacked);
	}

	UnlockNetworkData();
}

//==========================================================

Uint32 network_callbackfunc(Uint32 interval, void *param)
{
	if (GetNetwork())
	{
		GetNetwork()->listen();
	}
	
	return(interval);
}

bool NetworkInit()
{
	// Initialize SDL.
	if ( SDL_Init(SDL_INIT_TIMER) < 0 ) {
		GfOut("NetworkInit : Couldn't initialize SDL: %s\n", SDL_GetError());
		return false;
	}

	g_bInit = true;

	return true;
}

bool RemoveNetworkTimer()
{
	return SDL_RemoveTimer(g_timerId) == SDL_TRUE ? true : false;
}

bool AddNetworkTimer()
{
	//Create a timer callback to listen to the network
	g_timerId = SDL_AddTimer(40, network_callbackfunc,0);

	return true;
}

Server *GetServer()
{
	if (!g_bServer)
		return NULL;
	
	return &g_server;
}

Client *GetClient()
{
	if (!g_bClient)
		return NULL;

	return &g_client;
}

void SetServer(bool bStatus)
{
	if (bStatus == g_bServer)
	return;

	g_bServer = bStatus;
	if (g_bServer)
		AddNetworkTimer();
	else
		RemoveNetworkTimer();
}

void SetClient(bool bStatus)
{
	if (bStatus == g_bClient)
	return;

	g_bClient = bStatus;
	if (g_bClient)
		AddNetworkTimer();
	else
		RemoveNetworkTimer();

}

bool IsServer()
{
	return g_bServer;
}
bool IsClient()
{
	return g_bClient;
}



void NetworkListen()
{
	if (GetNetwork())
	{
		GetNetwork()->listen();
	}
}

bool AddressMatch(ENetAddress &a1,ENetAddress &a2)
{
	if ((a1.host == a2.host)&&(a1.port == a2.port))
		return true;

	return false;
};

static int
networkInit(int /* idx */, void *pt)
{
    return 0;
}


