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
 ********************************* ******************************************/
#include <cstdio>

#include "network.h"
#include "robotxml.h" 
#include <SDL/SDL.h>


Server::Server()
{

	if (enet_initialize () != 0)
    {
        GfOut ("An error occurred while initializing ENet.\n");
		assert(false);
        
    }

	m_strClass = "server";
	

}

Server::~Server()
{
	ResetNetwork();
	SetServer(false);
	

}

void Server::Disconnect()
 {
	ResetNetwork();
	SetServer(false);
 }

void Server::ResetNetwork()
{

	if (m_pServer)
	{

	    ENetPeer * pCurrentPeer;

		for (pCurrentPeer = m_pServer-> peers;
			 pCurrentPeer < & m_pServer->peers [m_pServer->peerCount];
			 ++ pCurrentPeer)
		{
		   if (pCurrentPeer->state != ENET_PEER_STATE_CONNECTED)
			 continue;

		   enet_peer_disconnect (pCurrentPeer, 0);
		}

		ENetEvent event;
		bool bDisconnect = false;
	    /* Allow up to 3 seconds for the disconnect to succeed
		and drop any packets received packets.
		*/
		while (enet_host_service (m_pServer, & event, 3000) > 0)
		{
			switch (event.type)
			{
			case ENET_EVENT_TYPE_RECEIVE:
				enet_packet_destroy (event.packet);
				break;

			case ENET_EVENT_TYPE_DISCONNECT:
				puts ("Disconnection succeeded.");
				bDisconnect=true;
				break;
			}
		}

	    /* We've arrived here, so the disconnect attempt didn't */
		/* succeed yet.  Force the connection down.             */
		if (!bDisconnect)
		{
		    ENetPeer * pCurrentPeer1;

			for (pCurrentPeer1 = m_pServer-> peers;
				 pCurrentPeer1 < & m_pServer->peers [m_pServer->peerCount];
				 ++ pCurrentPeer1)
			{
			   if (pCurrentPeer1->state != ENET_PEER_STATE_CONNECTED)
				 continue;

			   enet_peer_reset (pCurrentPeer1);
			}
		}

		enet_host_destroy(m_pServer);
		m_pServer = NULL;
	}

}

bool Server::IsConnected()
{
	if (m_pServer)
		return true;

	return false;
}

bool Server::Start(int port)
{
	SetRaceInfoChanged(true);
	m_bPrepareToRace = false;
	m_bBeginRace = false;

	m_timePhysics = -2.0;
	m_sendCarDataTime = 0.0;
	m_sendCtrlTime = 0.0;


    /* Bind the server to the default localhost.     */
    /* A specific host address can be specified by   */
    /* enet_address_set_host (& address, "x.x.x.x"); */

    m_address.host = ENET_HOST_ANY;
    /* Bind the server to port*/
    m_address.port = port;

	assert(m_pServer ==NULL);

    m_pServer = enet_host_create (& m_address /* the address to bind the server host to */, 
                                 MAXNETWORKPLAYERS,
                                  0      /* assume any amount of incoming bandwidth */,
                                  0      /* assume any amount of outgoing bandwidth */);
    if (m_pServer == NULL)
    {
        GfOut ("An error occurred while trying to create an ENet server host.\n");
		return false;
    }

	m_pHost = m_pServer;
	return true;

}


bool Server::ClientsReadyToRace()
{
	return m_bBeginRace;
}

void Server::WaitForClientsStartPacket()
{
	while (!m_bBeginRace)
	{
		SDL_Delay(20);
	}
	
}

void Server::SendStartTimePacket(int &startTime)
{
	unsigned char packetId = RACESTARTTIME_PACKET;

	//Wait RACESTARTDELEAY seconds to start race
	m_racestarttime = GfTimeClock()+RACESTARTDELEAY;
	int datasize = sizeof(m_racestarttime)+1;
	unsigned char *pData = new unsigned char[datasize];
	pData[0] = packetId;
	memcpy(&pData[1],&m_racestarttime,sizeof(m_racestarttime));


	ENetPacket * pPacket = enet_packet_create (pData, 
                                              datasize, 
                                              ENET_PACKET_FLAG_RELIABLE);

	BroadcastPacket(pPacket,RELIABLECHANNEL);

	delete [] pData;
	GfOut("Server Start time is %lf\n",m_racestarttime);
}
double Server::WaitForRaceStart()
{
	int startTime;
	SendStartTimePacket(startTime);
	GfOut("Server waiting to start the race\n");


	double time = GfTimeClock()-m_racestarttime;

	return time;

}

void Server::ClearDrivers()
{
	LockServerData()->m_vecNetworkPlayers.clear();
	LockServerData()->Unlock();
	GenerateDriversForXML();
}


void Server::SetHostSettings(const char *pszCarCat,bool bCollisions)
{
	assert(m_strRaceXMLFile!="");

	void *params = GfParmReadFileLocal(m_strRaceXMLFile.c_str(),GFPARM_RMODE_STD);
	assert(params);
	const char *pName =GfParmGetStr(params, RM_SECT_HEADER, RM_ATTR_NAME, "");


	GfParmSetStr(params, RM_SECT_HEADER,RM_ATTR_CAR_CATEGORY, pszCarCat);
	GfParmWriteFileLocal(m_strRaceXMLFile.c_str(), params, pName);
}


void Server::GenerateDriversForXML()
{


	assert(m_strRaceXMLFile!="");

	void *params = GfParmReadFileLocal(m_strRaceXMLFile.c_str(),GFPARM_RMODE_STD);
	assert(params);

	const char *pName =GfParmGetStr(params, RM_SECT_HEADER, RM_ATTR_NAME, "");

    int nCars = GfParmGetEltNb(params, RM_SECT_DRIVERS);	
	//Gather vector of all non human drivers
	std::vector<Driver> vecRDrivers;
	for (int i=1;i<=nCars;i++)
	{
		Driver driver;
		ReadDriverData(driver,i,params);
		if ((strcmp(driver.module,NETWORKROBOT)!=0)
			&&(strcmp(driver.module,HUMANROBOT)!=0))
		{
			vecRDrivers.push_back(driver);
		}
	}


	//Recreate drivers section robots first
	char drvSec[256];
	GfParmListClean(params, RM_SECT_DRIVERS);
	for (int i=0;i<(int)vecRDrivers.size();i++)
	{
		int index = i+1;
		sprintf(drvSec, "%s/%d", RM_SECT_DRIVERS, index);
		GfParmSetNum(params, drvSec, RM_ATTR_IDX, (char*)NULL, (tdble)vecRDrivers[i].idx);
		GfParmSetStr(params, drvSec, RM_ATTR_MODULE, vecRDrivers[i].module);
    }

	ServerMutexData *pSData = LockServerData();
	for (int i=0;i<(int)pSData->m_vecNetworkPlayers.size();i++)
	{
		int index = i+1+vecRDrivers.size();
		sprintf(drvSec, "%s/%d", RM_SECT_DRIVERS, index);
		GfParmSetNum(params, drvSec, RM_ATTR_IDX, (char*)NULL,(tdble) pSData->m_vecNetworkPlayers[i].idx);
		GfParmSetStr(params, drvSec, RM_ATTR_MODULE, pSData->m_vecNetworkPlayers[i].module);
    }

	UnlockServerData();
	//Save our changes
	GfParmWriteFileLocal(m_strRaceXMLFile.c_str(), params, pName);
	
}

void Server::SetLocalDrivers()
{

	m_setLocalDrivers.clear();

	m_driverIdx = GetDriverIdx();
	GfOut("Adding Human start rank: %i\n",m_driverIdx);
	m_setLocalDrivers.insert(m_driverIdx-1);

	assert(m_strRaceXMLFile!="");

	void *params = GfParmReadFileLocal(m_strRaceXMLFile.c_str(),GFPARM_RMODE_STD);
	assert(params);

	//const char *pName =GfParmGetStr(params, RM_SECT_HEADER, RM_ATTR_NAME, "");

    int nCars = GfParmGetEltNb(params, RM_SECT_DRIVERS);	
	//Gather vector of all non human drivers
	std::vector<Driver> vecRDrivers;
	for (int i=1;i<=nCars;i++)
	{
		Driver driver;
		ReadDriverData(driver,i,params);
		if ((strcmp(driver.module,NETWORKROBOT)!=0)
			&&(strcmp(driver.module,HUMANROBOT)!=0))
		{
			m_setLocalDrivers.insert(i-1);
			GfOut("Adding driver start rank:%i\n",i);
		}
	}

}

void Server::OverrideDriverReady(int idx,bool bReady)
{
	MutexData *pNData = LockNetworkData();
	pNData->m_vecReadyStatus[idx-1] = bReady;
	UnlockNetworkData();

	SetRaceInfoChanged(true);
}
void Server::SetDriverReady(bool bReady)
{
	int idx = GetDriverIdx();

	MutexData *pNData = LockNetworkData();
	pNData->m_vecReadyStatus[idx-1] = bReady;
	UnlockNetworkData();

	SendDriversReadyPacket();
}


void Server::UpdateDriver(Driver & driver)
{
	assert(m_strRaceXMLFile!="");
	bool bNewDriver = true;

	ServerMutexData *pSData = LockServerData();

	for(unsigned int i=0;i<pSData->m_vecNetworkPlayers.size();i++)
	{
		if (strcmp(driver.name,pSData->m_vecNetworkPlayers[i].name)==0)
		{
			bNewDriver = false;
			strncpy(pSData->m_vecNetworkPlayers[i].car,driver.car,64);
		}
	}


	if (bNewDriver)
	{
		if (pSData->m_vecNetworkPlayers.size()==0)
			driver.idx = 1;
		else
			driver.idx = pSData->m_vecNetworkPlayers.size()+1;

		if (!driver.client)
		{
			driver.address = m_pServer->address;
		}


		pSData->m_vecNetworkPlayers.push_back(driver);
		MutexData *pNData = LockNetworkData();
		bool bReady = false;
		pNData->m_vecReadyStatus.push_back(bReady);
		UnlockNetworkData();
	}


	GenerateDriversForXML();
	RobotXml rXml;
	rXml.CreateRobotFile("networkhuman",pSData->m_vecNetworkPlayers);

	UnlockServerData();
	
	SetRaceInfoChanged(true);
}

void Server::SetCarInfo(const char *pszName)
{
	std::vector<Driver> vecDrivers;

	RobotXml robotxml;
	robotxml.ReadRobotDrivers(NETWORKROBOT,vecDrivers);

	for (unsigned int i=0;i<vecDrivers.size();i++)
	{
		if (vecDrivers[i].name == m_strDriverName)
		{
			strncpy(vecDrivers[i].car,pszName,64);
			UpdateDriver(vecDrivers[i]);
		}
	}

}

void Server::CreateNetworkRobotFile()
{
	RobotXml rXml;
	ServerMutexData *pSData = LockServerData();
	rXml.CreateRobotFile("networkhuman",pSData->m_vecNetworkPlayers);
	UnlockServerData();
}

void Server::RemoveDriver(ENetEvent event)
{
	int playerStartIndex;
	ENetAddress address = event.peer->address;

	char hostName[256];
	enet_address_get_host_ip (&address,hostName,256);

    GfOut ("Client Player Info disconnect from %s\n",hostName); 
	
	std::vector<Driver>::iterator p;

	if (m_vecWaitForPlayers.size()>0)
	{
		p = m_vecWaitForPlayers.begin();

		while(p!=m_vecWaitForPlayers.end())
		{

			if ((p->address.host == address.host)&&(p->hostPort == address.port))
			{
				m_vecWaitForPlayers.erase(p);
				break;
			}

			p++;
		}

		if (m_vecWaitForPlayers.size()==0)
			m_bBeginRace = true;
	}

	//look for driver id
	ServerMutexData *pSData = LockServerData();
	for (p = pSData->m_vecNetworkPlayers.begin();p!=pSData->m_vecNetworkPlayers.end();p++)
	{
		if (p->client)
		{
			if ((p->address.host == address.host)&&(p->hostPort == address.port))
			{
				if(m_bRaceActive)
				{
					playerStartIndex = p->idx-1;
					pSData->m_vecNetworkPlayers.erase(p);
					RemovePlayerFromRace(playerStartIndex);
					GenerateDriversForXML();
					RobotXml rXml;
					rXml.CreateRobotFile("networkhuman",pSData->m_vecNetworkPlayers);
					SetRaceInfoChanged(true);
				}
				else
				{
					pSData->m_vecNetworkPlayers.erase(p);
					GenerateDriversForXML();
					RobotXml rXml;
					rXml.CreateRobotFile("networkhuman",pSData->m_vecNetworkPlayers);
					SetRaceInfoChanged(true);
				}

				UnlockServerData();
				return;
			}
		}
	}

	UnlockServerData();

}

bool Server::SendPlayerAcceptedPacket(ENetPeer * pPeer)
{

	//Send to client requesting connection
	unsigned char  packetId = PLAYERACCEPTED_PACKET;
	int datasize = 1;

	unsigned char *pData = new unsigned char[datasize];
	unsigned char *pDataSpot = pData;
	memcpy(pDataSpot,&packetId,1);	

	ENetPacket * pPacket = enet_packet_create (pData, 
                                              datasize, 
                                              ENET_PACKET_FLAG_RELIABLE);

	delete [] pData;
 
	if (enet_peer_send (pPeer, RELIABLECHANNEL, pPacket)==0)
		return true;

	return false;
}

bool Server::SendPlayerRejectedPacket(ENetPeer * pPeer,std::string strReason)
{
	unsigned int l = strReason.length();
	
	//Send to client requesting connection
	unsigned char  packetId = PLAYERREJECTED_PACKET;
	int datasize = sizeof(l)+l+1;

	unsigned char *pData = new unsigned char[datasize];
	unsigned char *pDataSpot = pData;
	memcpy(pDataSpot,&packetId,1);	
	pDataSpot++;
	memcpy(pDataSpot,&l,sizeof(l));
	pDataSpot+=sizeof(l);
	memcpy(pDataSpot,strReason.c_str(),l);
	pDataSpot+=l;

	ENetPacket * pPacket = enet_packet_create (pData, 
                                              datasize, 
                                              ENET_PACKET_FLAG_RELIABLE);

	delete [] pData;
 
	if (enet_peer_send (pPeer, RELIABLECHANNEL, pPacket)==0)
		return true;

	return false;
}

void Server::SendDriversReadyPacket()
{
	
	unsigned char packetId = ALLDRIVERREADY_PACKET;
	MutexData *pNData = LockNetworkData();

	int rsize = pNData->m_vecReadyStatus.size();
	int datasize = 1+sizeof(rsize)+sizeof(bool)*rsize;

	unsigned char *pData = new unsigned char[datasize];
	unsigned char *pDataSpot = pData;
	memcpy(pDataSpot,&packetId,1);
	pDataSpot++;
	memcpy(pDataSpot,&rsize,sizeof(rsize));
	pDataSpot+=sizeof(rsize);
	bool *pReady = (bool*)pDataSpot;
	for (int i=0;i<rsize;i++)
	{
		pReady[i] = pNData->m_vecReadyStatus[i];
	}

	UnlockNetworkData();

	ENetPacket * pPacket = enet_packet_create (pData, 
                                              datasize, 
                                              ENET_PACKET_FLAG_RELIABLE);

	delete [] pData;
 
	BroadcastPacket(pPacket,RELIABLECHANNEL);
	m_bRefreshDisplay = true;

}

void Server::SendRaceSetupPacket()
{

	unsigned char packetId = RACEINFOCHANGE_PACKET;
	int datasize = 1;

	unsigned char *pData = new unsigned char[datasize];
	unsigned char *pDataSpot = pData;
	memcpy(pDataSpot,&packetId,1);

	ENetPacket * pPacket = enet_packet_create (pData, 
                                              datasize, 
                                              ENET_PACKET_FLAG_RELIABLE);

	delete [] pData;
 
	BroadcastPacket(pPacket,RELIABLECHANNEL);

	SetRaceInfoChanged(true);

}


void Server::ReadDriverReadyPacket(ENetPacket *pPacket)
{
    GfOut ("Read Driver Ready Packet\n"); 
	
	int idx;
	memcpy(&idx,&pPacket->data[1],sizeof(idx));
	int spot = sizeof(idx)+1;
	bool bReady;
	memcpy(&bReady,&pPacket->data[spot],sizeof(bReady));
	
	MutexData *pNData = LockNetworkData();
	pNData->m_vecReadyStatus[idx-1] = bReady;
	UnlockNetworkData();

	SendDriversReadyPacket();
}

void Server::ReadDriverInfoPacket(ENetPacket *pPacket, ENetPeer * pPeer)
{
	assert(pPacket->dataLength==(sizeof(Driver)+1));
	
	Driver driver;

	char hostName[256];
	enet_address_get_host_ip (&driver.address,hostName,256);

    GfOut ("Client Player Info connected from %s\n",hostName); 

	memcpy(&driver,&pPacket->data[1],sizeof(Driver));

	//Make sure player name is unique otherwise disconnect player
	ServerMutexData *pSData = LockServerData();
	for(unsigned int i=0;i<pSData->m_vecNetworkPlayers.size();i++)
	{
		if (strcmp(driver.name,pSData->m_vecNetworkPlayers[i].name)==0)
		{
			SendPlayerRejectedPacket(pPeer,"Player name already used.  Please choose a different name.");
			UnlockServerData();
			return;
		}
	}
	UnlockServerData();

	driver.address.host = pPeer->address.host;
	driver.hostPort = pPeer->address.port;

	SendPlayerAcceptedPacket(pPeer);
	UpdateDriver(driver);



	GfOut("Reading Driver Info Packet:  Driver: %s,Car: %s\n",driver.name,driver.car);

}


//Used to verify that all clients are still connected
void Server::PingClients()
{
    ENetPeer * pCurrentPeer;

    for (pCurrentPeer = m_pServer-> peers;
         pCurrentPeer < & m_pServer->peers [m_pServer->peerCount];
         ++ pCurrentPeer)
    {
       if (pCurrentPeer->state != ENET_PEER_STATE_CONNECTED)
         continue;

       enet_peer_ping (pCurrentPeer);
    }

}


//Here you are Xavier a dynamic weather packet
void Server::SendWeatherPacket()
{
		GfOut("Sending Weather Packet\n");

		unsigned char  packetId = WEATHERCHANGE_PACKET;
		//TODO add weather data here

		ENetPacket * pWeatherPacket = enet_packet_create (&packetId, 
                                              1, 
                                              ENET_PACKET_FLAG_RELIABLE);


		BroadcastPacket(pWeatherPacket,RELIABLECHANNEL);

}

void Server::SendTimePacket(ENetPacket *pPacketRec, ENetPeer * pPeer)
{
	GfOut("Sending Time Packet\n");
	int packetSize = 1+sizeof(double);
	unsigned char *pData = new unsigned char[packetSize];
	unsigned char *pDataStart = pData;

	unsigned char packetId = SERVER_TIME_SYNC_PACKET;
	memcpy(pData,&packetId,1);
	pData++;
	double time = GfTimeClock();
	GfOut("\nServer time is %lf",time);

	memcpy(pData,&time,sizeof(time));
	pData+=sizeof(time);

	//TODO change to peer send
	ENetPacket * pPacket = enet_packet_create (pDataStart, 
                                              packetSize, 
                                              ENET_PACKET_FLAG_UNSEQUENCED);

	enet_peer_send (pPeer, UNRELIABLECHANNEL, pPacket);
}

//Send a file to clients
//Do not use this to send large files
//In future maybe change to TCP based
//64k file size limit
void Server::SendFilePacket(const char *pszFile)
{
	char filepath[255];
	sprintf(filepath, "%s%s", GetLocalDir(), pszFile);
	
	GfOut("Sending file packet: File- %s\n",filepath);

	FILE *pFile = fopen(filepath,"rb");
	if (!pFile)
		return;


	char buf[0xffff];
	size_t size;
	size = fread( buf, 1, 0xffff, pFile );


	//File is to big
	if (!feof(pFile))
	{
		fclose(pFile);
		assert(false);
		return;
	}

	fclose(pFile);
	unsigned int filesize = size;
	int datasize = filesize+sizeof(short)+sizeof(unsigned int)+strlen(pszFile)+1;
	unsigned char *pDataPacket = new unsigned char[datasize];
	memset(pDataPacket,0,datasize);
	unsigned char *pData = pDataPacket;


	unsigned char packetId = FILE_PACKET;
	memcpy(&pData[0],&packetId,1);
	pData++;
	short namelen = strlen(pszFile);
	memcpy(pData,&namelen,sizeof(short));
	pData+=sizeof(short);
	memcpy(pData,pszFile,namelen);
	pData+=namelen;
	
	memcpy(pData,&filesize,sizeof(unsigned int));
	GfOut("Server file size %u\n",filesize);
	pData+=sizeof(unsigned int);
	
	memcpy(pData,buf,size);

	ENetPacket * pPacket = enet_packet_create (pDataPacket, 
                                              datasize, 
                                              ENET_PACKET_FLAG_RELIABLE);
	
	BroadcastPacket(pPacket,RELIABLECHANNEL);


}

bool Server::listen()
{

	if (!m_pServer)
		return false;

	bool bHasPacket = false;
	ENetEvent event;
	char hostName[256];    
    
    while (enet_host_service (m_pServer, & event, 0) > 0)
    {
        switch (event.type)
        {
        case ENET_EVENT_TYPE_CONNECT:

			enet_address_get_host_ip (&event.peer -> address,hostName,256);

            GfOut ("A new client connected from %s\n",hostName); 

            /* Store any relevant client information here. */
            event.peer -> data = (void*)"Client information";

            break;

        case ENET_EVENT_TYPE_RECEIVE:
			ReadPacket(event); 
			bHasPacket = true;
            break;
           
        case ENET_EVENT_TYPE_DISCONNECT:
			GfOut("\nA client lost the connection.\n");
			enet_address_get_host_ip (&event.peer -> address,hostName,256);
            GfOut ("A new client disconnected from %s\n",hostName); 

			RemoveDriver(event);
			SetRaceInfoChanged(true);

            GfOut ("%s disconected.\n", (char*)event.peer -> data);

            /* Reset the peer's client information. */

            event.peer -> data = NULL;
			break;
        }
    }

	if (bHasPacket)
		m_activeNetworkTime = GfTimeClock();
	
	return bHasPacket;
}

//Remove disconnected player from race track
void Server::RemovePlayerFromRace(unsigned int idx)
{
	GfOut("Removing disconnected player\n");
	std::vector<CarStatusPacked> vecCarStatus;
	double time = 0.0;

	int startRank = GetDriverStartRank(idx);
	CarStatus cstatus;
	cstatus.topSpeed = -1.0;
	cstatus.fuel = -1.0;
	cstatus.startRank = startRank;
	cstatus.dammage = -1;
	cstatus.state = RM_CAR_STATE_ELIMINATED;
	cstatus.time = m_currentTime;
	
	MutexData *pNData = LockNetworkData();
	pNData->m_vecCarStatus.push_back(cstatus);
	UnlockNetworkData();


	//Pack controls values to reduce data size of packet
	CarStatusPacked status;
	status.topSpeed = -1.0;
	status.state = RM_CAR_STATE_ELIMINATED;
	status.startRank = startRank;
	status.dammage = -1;
	status.fuel = -1.0;

	vecCarStatus.push_back(status);

	time = m_currentTime;

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

void Server::ReadPacket(ENetEvent event)
{
	ENetPacket *pPacket = event.packet;
	assert(pPacket->dataLength>=1);
	unsigned char packetId;
	memcpy(&packetId,&pPacket->data[0],1);
	unsigned char *pData = &pPacket->data[1];
	//int datasize = pPacket->dataLength-1;
	
		switch (packetId)
		{
		case PLAYERINFO_PACKET:
			GfOut("PlayerInfo Packet\n");
			ReadDriverInfoPacket(pPacket,event.peer);
			break;
		case CLIENTREADYTOSTART_PACKET:
			{
			int l;
			char name[256];
			memset(&name[0],0,256);
			memcpy(&l,pData,sizeof(l));
			pData+=sizeof(l);
			memcpy(name,pData,l);
			std::vector<Driver>::iterator p;
			p = m_vecWaitForPlayers.begin();
			while(p!=m_vecWaitForPlayers.end())
			{
				if (strcmp(p->name,name)==0)
				{
					GfOut("%s ready to start\n",&name[0]);
					m_vecWaitForPlayers.erase(p);
					break;
				}

				p++;
			}

			if (m_vecWaitForPlayers.size()==0)
				m_bBeginRace = true;

			}
			break;
		case SERVER_TIME_REQUEST_PACKET:
			SendTimePacket(pPacket,event.peer);
			break;
		case CARCONTROLS_PACKET:
			ReadCarControlsPacket(event.packet);
			break;
		case CARSTATUS_PACKET:
			ReadCarStatusPacket(event.packet);
			break;
		case LAPSTATUS_PACKET:
			ReadLapStatusPacket(event.packet);
			break;
		case DRIVERREADY_PACKET:
			ReadDriverReadyPacket(event.packet);
			break;

	default:
			GfOut ("A packet of length %u containing %s was received from %s on channel %u.\n",
                    event.packet -> dataLength,
                    event.packet -> data,
                    (char*)event.peer -> data,
                    event.channelID);

	}

    enet_packet_destroy (event.packet);
}

void Server::SendFinishTimePacket()
{
	GfOut("Sending finish Time Packet\n");
	int packetSize = 1+sizeof(double);
	unsigned char *pData = new unsigned char[packetSize];
	unsigned char *pDataStart = pData;

	unsigned char packetId = FINISHTIME_PACKET;
	memcpy(pData,&packetId,1);
	pData++;
	MutexData *pNData = LockNetworkData();
	double time = pNData->m_finishTime;
	UnlockNetworkData();

	GfOut("\nServer finish time is %lf",time);

	memcpy(pData,&time,sizeof(time));
	pData+=sizeof(time);

	ENetPacket * pPacket = enet_packet_create (pDataStart, 
                                              packetSize, 
                                              ENET_PACKET_FLAG_RELIABLE);
	BroadcastPacket(pPacket,RELIABLECHANNEL);
}

void Server::SendPrepareToRacePacket()
{
	//Add all players to list except the server player
	ServerMutexData *pSData = LockServerData();
	for (int i=0;i<(int)pSData->m_vecNetworkPlayers.size();i++)
	{
		if (pSData->m_vecNetworkPlayers[i].client)
		{
			m_vecWaitForPlayers.push_back(pSData->m_vecNetworkPlayers[i]);
		}
	}

	UnlockServerData();

	if (m_vecWaitForPlayers.size()==0)
		m_bBeginRace = true;

	////TODO send needed xml files to race
	unsigned char packetId = PREPARETORACE_PACKET;
	ENetPacket * pPacket = enet_packet_create (&packetId, 
                                              1, 
                                              ENET_PACKET_FLAG_RELIABLE);

	BroadcastPacket(pPacket,RELIABLECHANNEL);
}

void Server::BroadcastPacket(ENetPacket *pPacket,enet_uint8 channel)
{
	enet_host_broadcast (m_pHost, channel, pPacket);
	m_activeNetworkTime = GfTimeClock();
}

void Server::SetFinishTime(double time)
{
	MutexData *pNData = LockNetworkData();
	pNData->m_finishTime = time;
	UnlockNetworkData();
	SendFinishTimePacket();
}