// -*- Mode: c++ -*-
/***************************************************************************
    file                 : network.cpp
    created              : July 2009
    copyright            : (C) 2009 Brian Gavin
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

#include <cstdio>
#include <SDL/SDL.h>
#include "network.h"
#include "robotxml.h"

Client::Client()
{
	if (enet_initialize () != 0)
    {
        GfLogError ("An error occurred while initializing ENet.\n");
    }

	m_strClass = "client";
	m_pServer = NULL;
	m_pClient = NULL;
	m_pHost = NULL;
	m_eClientAccepted = PROCESSINGCLIENT;
}


Client::~Client()
{
	ResetNetwork();
	SetClient(false);


}

 void Client::Disconnect()
 {
	m_bConnected = false;

	ResetNetwork();
	SetClient(false);
 }

void Client::ResetNetwork()
{

	if (m_pClient == NULL)
		return;

	if (m_pServer == NULL)
		return;

	ENetEvent event;
    
    enet_peer_disconnect (m_pServer, 0);

	bool bDisconnect = false;

    /* Allow up to 3 seconds for the disconnect to succeed
	 and drop any packets received packets.
     */
    while (enet_host_service (m_pClient, & event, 3000) > 0)
    {
        switch (event.type)
        {
        case ENET_EVENT_TYPE_RECEIVE:
            enet_packet_destroy (event.packet);
            break;

        case ENET_EVENT_TYPE_DISCONNECT:
            GfLogTrace ("Network disconnection succeeded.");
			bDisconnect=true;
            break;
			
		case ENET_EVENT_TYPE_NONE:
		case ENET_EVENT_TYPE_CONNECT:
			// Do nothing.
			break;
        }
    }
    
    /* We've arrived here, so the disconnect attempt didn't */
    /* succeed yet.  Force the connection down.             */
    if (!bDisconnect)
		enet_peer_reset (m_pServer);

	SetClient(false);

    ENetPeer * pCurrentPeer1;

	if (m_pHost ==NULL)
		return;

	for (pCurrentPeer1 = m_pHost-> peers;
		 pCurrentPeer1 < & m_pHost->peers [m_pHost->peerCount];
		 ++ pCurrentPeer1)
	{
	   if (pCurrentPeer1->state != ENET_PEER_STATE_CONNECTED)
		 continue;

	   enet_peer_reset (pCurrentPeer1);
	}

	enet_host_destroy(m_pHost);
	m_pHost = NULL;

}

bool Client::ConnectToServer(const char *pAddress,int port, Driver *pDriver)
{
	m_bTimeSynced = false;
	m_bPrepareToRace = false;
	m_bBeginRace = false;
	m_timePhysics = -2.0;
	m_servertimedifference = 0.0;
	m_sendCarDataTime = 0.0;
	m_sendCtrlTime = 0.0;
	m_bPrepareToRace = false;
	m_bBeginRace = false;
	m_bConnected = false;
	m_pClient = NULL;
	m_pHost = NULL;

	#if (ENET_VERSION >= 0x010300)
          m_pClient = enet_host_create (NULL /* create a client host */,
                  MAXNETWORKPLAYERS, 
                  0, /*channel limit*/
                  0/* downstream bandwidth */,
                  0/* upstream bandwidth */);
        #else
          m_pClient = enet_host_create (NULL /* create a client host */,
                  MAXNETWORKPLAYERS, 
                  0/* downstream bandwidth */,
                  0/* upstream bandwidth */);
        #endif

    if (m_pClient == NULL)
    {
        GfLogError ("An error occurred while trying to create an ENet client host.\n");
		ResetNetwork();
		return false;
    }

    ENetAddress caddress;
    caddress.host = ENET_HOST_ANY;
    /* Bind the server to port*/
    caddress.port = SPEEDDREAMSPEERPORT;

    #if (ENET_VERSION >= 0x010300)
        m_pHost = enet_host_create (&caddress /* create a peer host */,
                    MAXNETWORKPLAYERS, 
                    0, /*channel limit*/
                    0/* downstream bandwidth */,
                    0/* upstream bandwidth */);
    #else
        m_pHost = enet_host_create (&caddress /* create a peer host */,
                    MAXNETWORKPLAYERS, 
                    0/* downstream bandwidth */,
                    0/* upstream bandwidth */);
    #endif
    if(m_pHost==NULL)
    {
	//try the other ports
	for (int i=1;i<5;i++)
	{
		caddress.port++;
                #if (ENET_VERSION >= 0x010300)
    		    m_pHost = enet_host_create (&caddress,MAXNETWORKPLAYERS,0,0,0);
                #else
    		    m_pHost = enet_host_create (&caddress,MAXNETWORKPLAYERS,0,0);
                #endif
		if(m_pHost)
			break;

	}

		if (m_pHost == NULL)
		{
			GfLogError("Unable to setup client listener\n");
			return false;
		}
    }

    ENetAddress address;
    ENetEvent event;

    enet_address_set_host (& address, pAddress);
    address.port = port;
	
    /* Initiate the connection, allocating the two channels 0 and 1. */
	GfLogError ("Initiating network connection to host %s:%d ...\n", pAddress, port);
    #if (ENET_VERSION >= 0x010300)
        m_pServer = enet_host_connect (m_pClient, & address, 2, 0);
    #else
        m_pServer = enet_host_connect (m_pClient, & address, 2);
    #endif

    if (m_pServer == NULL)
    {
       GfLogInfo ("No available peers for initiating an ENet connection.\n");
	   ResetNetwork();
       return false;
    }
    
    /* Wait up to 5 seconds for the connection attempt to succeed. */
    if (enet_host_service (m_pClient, & event, 5000) > 0 &&
        event.type == ENET_EVENT_TYPE_CONNECT)
    {
		m_address.host = m_pClient->address.host;
		m_address.port = m_pClient->address.port;
		m_bConnected = true;
		GfLogInfo ("Network connection accepted.\n");
    }
    else
    {
		m_bConnected = false;
		ResetNetwork();
    }

	m_eClientAccepted = PROCESSINGCLIENT;
	SendDriverInfoPacket(pDriver);

	//Wait for server to accept or reject 
	GfLogInfo ("Sent local driver info to the network server : waiting ...\n");
	while(m_eClientAccepted == PROCESSINGCLIENT)
	{
		SDL_Delay(50);
	}

	if (m_eClientAccepted == CLIENTREJECTED)
	{
		m_bConnected = false;
		ResetNetwork();
		return false;
	}
	else
		GfLogInfo ("Driver info accepted by the network server.\n");

	return m_bConnected;
}

bool Client::IsConnected()
{
	return m_bConnected;
}

void Client::SetDriverReady(bool bReady)
{
	// Get local driver index in the race driver list
	int idx = GetDriverIdx();

	MutexData *pNData = LockNetworkData();
	pNData->m_vecReadyStatus[idx-1] = bReady;
	UnlockNetworkData();

	int packetSize = 1+sizeof(idx)+sizeof(bReady);
    unsigned char packetId = DRIVERREADY_PACKET;

	unsigned char *pData = new unsigned char[packetSize];
	unsigned char *pDataStart = pData;

	memcpy(pData,&packetId,1);
	pData++;

	memcpy(pData,&idx,sizeof(idx));
	pData+=sizeof(idx);
	memcpy(pData,&bReady,sizeof(bReady));

	ENetPacket * pPacket = enet_packet_create (pDataStart, 
                                              packetSize, 
                                              ENET_PACKET_FLAG_RELIABLE);

	delete [] pDataStart;
	if (enet_peer_send (m_pServer, RELIABLECHANNEL, pPacket)==0)
		return;
}

bool Client::SendDriverInfoPacket(Driver *pDriver)
{
	SetDriverName(pDriver->name);
	pDriver->address.port = m_pHost->address.port;

	unsigned char  packetId = PLAYERINFO_PACKET;
	int datasize = sizeof(Driver)+1;

	unsigned char *pData = new unsigned char[datasize];
	memcpy(&pData[0],&packetId,1);
	memcpy(&pData[1],pDriver,sizeof(Driver));

	ENetPacket * pPacket = enet_packet_create (pData, 
                                              datasize, 
                                              ENET_PACKET_FLAG_RELIABLE);

	delete [] pData;
 
	if (enet_peer_send (m_pServer, RELIABLECHANNEL, pPacket)==0)
		return true;

	return false;
}

void Client::SendReadyToStartPacket()
{
	
	std::string strDName = GetDriverName();
	GfLogTrace("Sending ready to start packet\n");
	int l = strDName.size();
	int datasize = 1+sizeof(l)+l*sizeof(char);
	unsigned char *pData = new unsigned char[datasize];
	
	unsigned char packetId = CLIENTREADYTOSTART_PACKET;
	unsigned char *pCurData = pData;
	memcpy(pCurData,&packetId,1);
	pCurData++;
	memcpy(pCurData,&l,sizeof(l));
	pCurData+=sizeof(l);
	memcpy(pCurData,strDName.c_str(),l);

	ENetPacket * pPacket = enet_packet_create (pData, 
                                              datasize, 
                                              ENET_PACKET_FLAG_RELIABLE);

	if (enet_peer_send (m_pServer, RELIABLECHANNEL, pPacket))
		GfLogError("SendReadyToStartPacket : enet_peer_send failed\n");
}


void Client::SendServerTimeRequest()
{
	m_packetsendtime = GfTimeClock(); 
	unsigned char packetId = SERVER_TIME_REQUEST_PACKET;
	ENetPacket * pPacket = enet_packet_create (&packetId, 
                                              1, 
                                              ENET_PACKET_FLAG_UNSEQUENCED);

	if (enet_peer_send (m_pServer, UNRELIABLECHANNEL, pPacket))
		GfLogError("SendServerTimeRequest : enet_peer_send failed\n");
}

double Client::WaitForRaceStart()
{
	while(!m_bBeginRace)
	{
		SDL_Delay(20);
	}

	return GfTimeClock()-m_racestarttime;
}


void Client::ReadStartTimePacket(ENetPacket *pPacket)
{
	GfLogTrace("Recieved the start race Packet\n");
	unsigned char *pData = &pPacket->data[1];
	memcpy(&m_racestarttime,pData,sizeof(m_racestarttime));
	//double time = GfTimeClock();

	//Adjust start time based on client clock
	m_racestarttime= m_racestarttime+m_servertimedifference;
	m_bBeginRace = true;
	
}

void Client::ReadPlayerRejectedPacket(ENetPacket *pPacket)
{
	m_eClientAccepted = CLIENTREJECTED;
	GfLogWarning ("Server rejected connection.\n");
}

void Client::ReadPlayerAcceptedPacket(ENetPacket *pPacket)
{
	m_eClientAccepted = CLIENTACCEPTED;
	GfLogTrace ("Server accepted connection.\n");
}

bool Client::listenHost(ENetHost * pHost)
{
	if (pHost == NULL)
		return false;

	bool bHasPacket = false;

	ENetEvent event;
    
     while (enet_host_service(pHost, & event, 0) > 0)
    {
        switch (event.type)
        {
        case ENET_EVENT_TYPE_CONNECT:
			char hostName[256];
			enet_address_get_host_ip (&event.peer->address,hostName,256);

            GfLogTrace ("A new client connected from %s\n",hostName); 

            /* Store any relevant client information here. */
            event.peer -> data = (void*)"Client information";

            break;

        case ENET_EVENT_TYPE_RECEIVE:
            //printf ("A packet of length %u containing %s was received from %s on channel %u.\n",
            //        event.packet -> dataLength,
            //        event.packet -> data,
            //        event.peer -> data,
            //        event.channelID);
			ReadPacket(event);        
			bHasPacket = true;
            break;
           
        case ENET_EVENT_TYPE_DISCONNECT:
			if(event.peer == m_pServer)
			{
				m_bConnected = false;
				/* Reset the peer's client information. */
				GfLogTrace("Server disconnected\n");
			}
			
            event.peer -> data = NULL;
			break;
			
		case ENET_EVENT_TYPE_NONE:
			// Do nothing.
			break;
        }
    }

	return bHasPacket;

}

bool Client::listen()
{
	if (!m_bConnected)
		return false;
	
	listenHost(m_pClient);
	listenHost(m_pHost);

	return true;
}


void Client::ReadPacket(ENetEvent event)
{
	ENetPacket *pPacket = event.packet;
	assert(pPacket->dataLength>=1);
	unsigned char packetId;
	memcpy(&packetId,&pPacket->data[0],1);
	//unsigned char *pData = &pPacket->data[1];
	//int datasize = pPacket->dataLength-1;
	
		switch (packetId)
		{
		case RACEINFOCHANGE_PACKET:
			ReadRaceSetupPacket(event.packet);
			break;
		case PREPARETORACE_PACKET:
			ReadPrepareToRacePacket(event.packet);
			break;
		case RACESTARTTIME_PACKET:
			ReadStartTimePacket(event.packet);
			break;
		case CARCONTROLS_PACKET:
			ReadCarControlsPacket(event.packet);
			break;
		case FILE_PACKET:
			ReadFilePacket(event.packet);
			break;
		case SERVER_TIME_SYNC_PACKET:
			ReadTimePacket(event.packet);
			break;
		case WEATHERCHANGE_PACKET:
			ReadWeatherPacket(event.packet);
			break;
		case CARSTATUS_PACKET:
			ReadCarStatusPacket(event.packet);
			break;
		case LAPSTATUS_PACKET:
			ReadLapStatusPacket(event.packet);
			break;
		case FINISHTIME_PACKET:
			ReadFinishTimePacket(event.packet);
			break;
		case ALLDRIVERREADY_PACKET:
			ReadAllDriverReadyPacket(event.packet);
			break;
		case PLAYERREJECTED_PACKET:
			ReadPlayerRejectedPacket(event.packet);
			break;
		case PLAYERACCEPTED_PACKET:
			ReadPlayerAcceptedPacket(event.packet);
			break;
	default:
			assert(false);
			GfLogDebug ("A packet of length %u containing %s was received from %s on channel %u.\n",
                    event.packet -> dataLength,
                    event.packet -> data,
                    (char*)event.peer -> data,
                    event.channelID);

	}

    enet_packet_destroy (event.packet);
}

void Client::ReadPrepareToRacePacket(ENetPacket *pPacket)
{
	GfLogTrace("Recieved the start race Packet\n");

	//unsigned char packetId = pPacket->data[0];
	
	

	m_bPrepareToRace = true;


}
void Client::ReadRaceSetupPacket(ENetPacket *pPacket)
{
	GfLogTrace("\nRecieving race setup\n");

	SetRaceInfoChanged(true);
}

void Client::ConnectToDriver(Driver driver)
{
	char hostName[256];
	enet_address_get_host_ip (&driver.address,hostName,256);

	if (!driver.client)
	{
		GfLogTrace("Skipping server: %s Address: %s\n",driver.name,hostName);
		return;
	}
	
	if (strcmp(driver.name,GetDriverName())==0)
	{
		GfLogTrace("Skipping ourself: %s Address:  %s\n",driver.name,hostName);
		return;
	}

    	ENetPeer * pCurrentPeer;

    	for (pCurrentPeer = m_pClient-> peers;
        	 pCurrentPeer < & m_pClient->peers [m_pClient->peerCount];
         	++ pCurrentPeer)
    	{
       		if (pCurrentPeer->state == ENET_PEER_STATE_CONNECTED)
		{
         		if ((pCurrentPeer->address.host == driver.address.host)&&
				(pCurrentPeer->address.port == driver.address.port))
			{
				GfLogTrace("Already connected to driver: %s Address: %s\n",driver.name,hostName);
				return;
			}
		}

    	}

	GfLogTrace("connecting to driver: %s Address: %s\n",driver.name,hostName);

	//Connect to peer player
	//ENetPeer *pPeer = enet_host_connect (m_pClient, &driver.address, 2);



	ENetEvent event;
  
    if (enet_host_service (m_pClient, & event, 5000) > 0 &&
        event.type == ENET_EVENT_TYPE_CONNECT)
    {
		GfLogTrace("Successfully connected to peer\n");
		return;
    }
    else
    {
		GfLogWarning("Failed to connect to peer!\n");
		return;
    }

}

void Client::ReadWeatherPacket(ENetPacket *pPacket)
{
	//TODO Xavier read in weather data
}
void Client::ReadAllDriverReadyPacket(ENetPacket *pPacket)
{
	unsigned char *pData = &pPacket->data[1];
	int rsize;
	memcpy(&rsize,pData,sizeof(rsize));
	pData+=sizeof(rsize);
	MutexData *pNData = LockNetworkData();
	pNData->m_vecReadyStatus.clear();
	pNData->m_vecReadyStatus.resize(rsize);
	bool *pReady = (bool*)pData;
	for (int i=0;i<rsize;i++)
		pNData->m_vecReadyStatus[i] = pReady[i];

	UnlockNetworkData();
	SetRaceInfoChanged(true);

	GfLogTrace("Recieved All Driver Ready Packet\n");
}

void Client::ReadFinishTimePacket(ENetPacket *pPacket)
{
	unsigned char *pData = &pPacket->data[1];
	
	MutexData *pNData = LockNetworkData();
	memcpy(&pNData->m_finishTime,pData,sizeof(pNData->m_finishTime));
	UnlockNetworkData();
	GfOut("Recieved finish time packet\n");
}

void Client::ReadTimePacket(ENetPacket *pPacket)
{
	double curTime = GfTimeClock();
	m_lag = (curTime-m_packetsendtime)/2.0;
	GfLogTrace ("Connection lag is %lf seconds\n",m_lag);

	unsigned char *pData = &pPacket->data[1];
	double time;
	memcpy(&time,pData,sizeof(double));
	m_servertimedifference = curTime-time;
	m_bTimeSynced = true;
}
void Client::ReadFilePacket(ENetPacket *pPacket)
{

	unsigned char *pData = &pPacket->data[1];
	short len;
	memcpy(&len,pData,sizeof(short));
	pData+=sizeof(short);
	char file[255];
	memset(&file[0],0,255);
	memcpy(file,pData,len);
	pData+=len;
	unsigned int filesize;
	memcpy(&filesize,pData,sizeof(unsigned int));
	pData+=sizeof(unsigned int);
	GfLogTrace("Client file size %u\n",filesize);
	
	char filepath[255];
	sprintf(filepath, "%s%s", GfLocalDir(), file);
	FILE *pFile = fopen(filepath,"w+b");
	GfLogTrace("Reading file packet: File- %s\n",filepath);
	fwrite(pData,filesize,1,pFile);
	fclose(pFile);

}

void Client::BroadcastPacket(ENetPacket *pPacket,enet_uint8 channel)
{
	ENetPacket * pHostPacket = enet_packet_create (pPacket->data, 
                                              pPacket->dataLength, 
                                              pPacket->flags);

	//Send to connected clients
	enet_host_broadcast (m_pHost, channel, pPacket);

	//Send to server
	enet_peer_send (m_pServer, channel, pHostPacket);

	m_activeNetworkTime = GfTimeClock();
}

void Client::SetCarInfo(const char *pszName)
{
	std::vector<Driver> vecDrivers;

	RobotXml robotxml;
	robotxml.ReadRobotDrivers(NETWORKROBOT,vecDrivers);

	for (unsigned int i=0;i<vecDrivers.size();i++)
	{
		if (vecDrivers[i].name == m_strDriverName)
		{
			strncpy(vecDrivers[i].car,pszName,64);
			SendDriverInfoPacket(&vecDrivers[i]);
		}
	}
}

void Client::ConnectToClients()
{
	std::vector<Driver> vecDrivers;

	RobotXml robotxml;
	robotxml.ReadRobotDrivers(NETWORKROBOT,vecDrivers);

	for(unsigned int i=0;i<vecDrivers.size();i++)
	{
		ConnectToDriver(vecDrivers[i]);
	}

}

void Client::SetLocalDrivers()
{
	m_setLocalDrivers.clear();
	m_driverIdx = GetDriverIdx();
	m_setLocalDrivers.insert(m_driverIdx-1);
	GfLogTrace("Adding Human start rank: %i\n",m_driverIdx-1);
}

