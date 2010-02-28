// -*- Mode: c++ -*-
/***************************************************************************
    file                 : network.h
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

#ifndef NETWORK_H
#define NETWORK_H


#ifdef _WIN32
	#ifndef DLLEXPORT
		#define DLLEXPORT __declspec(dllexport)
	#endif
#pragma warning (disable: 4251)
#else
       #ifndef DLLEXPORT
               #define DLLEXPORT
       #endif
#endif



#include <string>
#include <vector>
#include <set>
#include <map>

#ifdef __APPLE__
#include <enet.h>
#else
#include <enet/enet.h>
#endif

#include <SDL/SDL_thread.h>

#define MAXNETWORKPLAYERS 3
//Port used for network play
#define TORCSNGPORT 28500
#define TORCSNGPEERPORT 28501

#define UNRELIABLECHANNEL 0
#define RELIABLECHANNEL 1

#define NETWORKROBOT "networkhuman"
#define HUMANROBOT "human"
//Network update rates
#define CAR_CONTROL_UPDATE 0.1
#define CAR_DATA_UPDATE 5.0
#define RACESTARTDELEAY 10.0
#define FINISHDELAY 10.0

//Packet definitions
#define CHAT_PACKET 1
#define PLAYERINFO_PACKET 2
#define RACEINFOCHANGE_PACKET 3
#define PREPARETORACE_PACKET 4
#define CLIENTREADYTOSTART_PACKET 5
#define RACESTARTTIME_PACKET 6
#define CARCONTROLS_PACKET 7
#define FILE_PACKET 8
#define SERVER_TIME_SYNC_PACKET 9
#define SERVER_TIME_REQUEST_PACKET 10
#define WEATHERCHANGE_PACKET 11
#define CARSTATUS_PACKET 12
#define LAPSTATUS_PACKET 13
#define FINISHTIME_PACKET 14
#define DRIVERREADY_PACKET 15
#define ALLDRIVERREADY_PACKET 16

#include <track.h>
#include <raceman.h>

//Use a structure to pass as a network ENetPacket sent 
//Packed / Compressed to reduce internet bandwidth requirements
struct CarControlsPacked
{
	unsigned char	startRank;
    tDynPt	DynGCg;		/* GC global data */

	short steering;//Fixed point between -2,2
	short throttle;//Fixed point between -2,2
	short brake;//Fixed point between -2,2
	short clutch;//Fixed point between -2,2
	unsigned char gear;
};

//Uncompressed car controls pack
struct CarControlsData
{
	unsigned char	startRank;
    tDynPt	DynGCg;		/* GC global data */

	float steering;
	float throttle;
	float brake;
	float clutch;
	unsigned char gear;
	double time;
};


struct LapStatus
{
	float bestLapTime;
	float bestSplitTime;
	unsigned short laps;
	unsigned char startRank;
};

struct CarStatus
{
	float topSpeed;
	short state;
	double time;
	float fuel;
	int dammage;
	unsigned char startRank;
};

struct CarStatusPacked
{
	float topSpeed;
	short state;
	float fuel;
	int dammage;
	unsigned char startRank;
};


//Holds driver values 
class DLLEXPORT Driver
{
public:
	Driver();
	ENetAddress address;
	unsigned short hostPort;
	
	int idx;
	char name[64];
	char car[64];
	char team[64];
	char author[64];
	int racenumber;
	char skilllevel[64];
	float red,green,blue;
	char module[64];
	char type[64];
	bool client;
};

//Holds car setup values
struct CarSetup
{
	//TODO
};

//Put data here that is read by the network thread and the main thread
class DLLEXPORT MutexData
{
public:
	MutexData()
	{
		m_networkMutex=  SDL_CreateMutex();
	}
	virtual ~MutexData()
	{
		SDL_DestroyMutex (m_networkMutex );
	}
	
	void Lock() 
	{
		SDL_mutexP ( m_networkMutex );
	}

	void Unlock() 
	{
		SDL_mutexV ( m_networkMutex );
	}

	void Init()
	{
		m_vecCarCtrls.clear();
		m_vecCarStatus.clear();
		m_vecLapStatus.clear();
		m_finishTime = 0.0f;

	}

	SDL_mutex *m_networkMutex;
	std::vector<CarControlsData> m_vecCarCtrls;
	std::vector<CarStatus> m_vecCarStatus;
	std::vector<LapStatus> m_vecLapStatus;
	std::vector<bool> m_vecReadyStatus;
	double m_finishTime;
};

//Put data here that is read by the network thread and the main thread
class DLLEXPORT ServerMutexData 
{
public:
	void Init()
	{
		m_vecNetworkPlayers.clear();
	}

	ServerMutexData()
	{
		m_networkMutex=  SDL_CreateMutex();
	}
	virtual ~ServerMutexData()
	{
		SDL_DestroyMutex (m_networkMutex );
	}
	
	void Lock() 
	{
		SDL_mutexP ( m_networkMutex );
	}

	void Unlock() 
	{
		SDL_mutexV ( m_networkMutex );
	}
	
	SDL_mutex *m_networkMutex;
	std::vector<Driver> m_vecNetworkPlayers;
};

class DLLEXPORT Network
{
public:
	Network()
	{
		m_strClass = "network";
		m_bRaceInfoChanged = false;
		m_bRefreshDisplay = false;
		
		m_sendCtrlTime = 0.0;
		m_sendCarDataTime = 0.0;
		m_pHost = NULL;
		m_currentTime = 0.0;
	}

	virtual ~Network()
	{
	}


	void SetCurrentTime(double time) {m_currentTime = time;}
	bool IsServerMode(); 
	bool IsClientMode(); 
	bool SetCurrentDriver();
	int GetNetworkHumanIdx();
	int	 GetDriverIdx();
	int GetCarIndex(int startRank,tSituation *s);
	virtual void ReadLapStatusPacket(ENetPacket *pPacket);
	virtual void SendCarControlsPacket(tSituation *s);
	virtual void SendCarStatusPacket(tSituation *s,bool bForce);
	virtual void SendLapStatusPacket(tCarElt *pCar);
	virtual void SetDriverReady(bool bReady) {};
	virtual bool IsConnected() { return false;}
	virtual bool IsRaceActive() { return m_bRaceActive;}
	virtual void SetRaceActive(bool bStatus) {m_bRaceActive = bStatus;}
	virtual void RaceInit(tSituation *s)
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

	virtual void RaceDone()
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

	};


	int GetDriverStartRank(int idx) 
	{
		std::map<int,int>::iterator p;
		p = m_mapRanks.find(idx);

		return p->second;
	}

	virtual bool listen(){ return false;};

	virtual void Disconnect() {};
	virtual void ResetNetwork() {};

	void ReadCarControlsPacket(ENetPacket *pPacket);
	void ReadCarStatusPacket(ENetPacket *pPacket);
	void PackCarControl(tCarCtrl *pCtrl,int &size,char *&pBuffer);
	void UnPackCarControl(tCarCtrl *&pCtrl,int size,char *pBuffer);
	bool PrepareToRace(){return m_bPrepareToRace;}
	void SetRaceInfoChanged(bool bStatus);
	void SetRefreshDisplay(bool bStatus);
	bool GetRefreshDisplay() {return m_bRefreshDisplay;}
	bool GetRaceInfoChanged(){return m_bRaceInfoChanged;};
	double GetRaceStartTime(){return m_racestarttime;}
	std::string GetNetworkDriverName();
	void SetRaceXMLFile(char const *pXMLFile);
	void ReadDriverData(Driver &player,int index,void *param);
	void WriteDriverData(Driver player,int index,void *param);
	int GetPlayerCarIndex(tSituation *s);

	void ClearLocalDrivers(){m_setLocalDrivers.clear();}

	void SetDriverName(char *pName)
	{
		m_strDriverName = pName;
	printf("\nSetting driver name: %s\n",m_strDriverName.c_str());
	}
	const char *GetDriverName(){return m_strDriverName.c_str();}
	virtual void SetLocalDrivers(){};
	void GetHostSettings(std::string &strCarCat,bool &bCollisions);
	virtual void SetCarInfo(const char *pszName) {};

	virtual bool FinishRace(double time) 
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


	MutexData * LockNetworkData() 
	{
		m_NetworkData.Lock();
		return & m_NetworkData;
	}

	void UnlockNetworkData()
	{
		m_NetworkData.Unlock();
	}


protected:
	std::string m_strDriverName;


	ENetHost * m_pHost;
	virtual void BroadcastPacket(ENetPacket *pPacket,enet_uint8 channel){};

	int m_driverIdx;
	bool m_bBeginRace;
	bool m_bRaceInfoChanged;
	bool m_bRefreshDisplay;
	double m_racestarttime;	
	bool m_bPrepareToRace;
	bool m_bTimeSynced;
	bool m_bRaceActive;

	//time when packet was sent or recieved
	double m_activeNetworkTime;
	ENetAddress m_address;

	FILE *m_pFile;
	
	double m_sendCtrlTime;
	double m_sendCarDataTime;
	double m_currentTime;

	MutexData m_NetworkData;


	std::map<int,int>	m_mapRanks;

	std::set<int> m_setLocalDrivers;

	double m_timePhysics;

	std::string m_strClass;
	std::string m_strRaceXMLFile;

};

class DLLEXPORT Client: public Network
{
public:
	Client();
	~Client();

	virtual void Disconnect();
	virtual void ResetNetwork();
	virtual bool IsConnected();

	bool ConnectToServer(const char *pAddress,int port,const char *pClientName);
	virtual bool listen();

	//Packets
	bool SendDriverInfoPacket(Driver *pDriver);
	virtual void SendDriverReadyPacket(){};
	void SendReadyToStartPacket();
	double WaitForRaceStart();
	void SendServerTimeRequest();
	virtual void SetDriverReady(bool bReady);

	bool TimeSynced(){return m_bTimeSynced;}
	int  LookUpDriverIdx() { return m_driverIdx;}
	bool listenHost(ENetHost * pHost);
	virtual void SetLocalDrivers()
	{
		m_setLocalDrivers.clear();
		m_driverIdx = GetDriverIdx();
		m_setLocalDrivers.insert(m_driverIdx-1);
		printf("Adding Human start rank: %i\n",m_driverIdx-1);
	};
	
	void ConnectToClients();
	void SetCarInfo(const char *pszName);

protected:
	//Packets
	void ReadRaceSetupPacket(ENetPacket *pPacket);
	void ReadPrepareToRacePacket(ENetPacket *pPacket);
	void ReadStartTimePacket(ENetPacket *pPacket);
	void ReadFilePacket(ENetPacket *pPacket);
	void ReadPacket(ENetEvent event);
	void ReadTimePacket(ENetPacket *pPacket);
	void ReadFinishTimePacket(ENetPacket *pPacket);
	void ReadAllDriverReadyPacket(ENetPacket *pPacket);
	void ReadWeatherPacket(ENetPacket *pPacket);
	void ConnectToDriver(Driver driver);

	virtual void BroadcastPacket(ENetPacket *pPacket,enet_uint8 channel);

	bool m_bConnected;



	double m_lag;
	double m_servertimedifference;
	double m_packetsendtime;



	ENetHost * m_pClient;
	
	ENetPeer *m_pServer;	

};

class DLLEXPORT Server : public Network
{
public:
	Server();
	~Server();

	virtual void Disconnect();
	virtual void ResetNetwork();
	virtual bool IsConnected();

	bool Start(int port);
	virtual bool listen();
	
	//Network Packets
	void SendRaceSetupPacket();
	void SendPrepareToRacePacket();
	void SendFilePacket(const char *pszFile);
	void WaitForClientsStartPacket();
	void SendStartTimePacket(int &startTime);
	void SendTimePacket(ENetPacket *pPacket, ENetPeer * pPeer);
	void SendFinishTimePacket();
	void SendWeatherPacket();
	void SendDriversReadyPacket();
	void PingClients();
	virtual void SetDriverReady(bool bReady);
	void OverrideDriverReady(int idx,bool bReady);
	
	bool ClientsReadyToRace();
	double WaitForRaceStart();
	void UpdateClientCarInfo(tSituation *s);

	void UpdateDriver(Driver & player);

	int  NumberofPlayers() { 
		int n = LockServerData()->m_vecNetworkPlayers.size();
		UnlockServerData();
		return n;
	};
	Driver GetPlayer(int i);
	void ClearDrivers();
	void RemoveDriver(ENetEvent event);
	void CreateNetworkRobotFile();
	//virtual void SendCarControlsPacket(tSituation *s);
	virtual void SetLocalDrivers();
	void SetHostSettings(const char *pszCarCat,bool bCollisions);

	void SetCarInfo(const char *pszName);
	void SetFinishTime(double time);
	void RemovePlayerFromRace(unsigned int idx);

	ServerMutexData * LockServerData() 
	{
		m_ServerData.Lock();
		return & m_ServerData;
	}

	void UnlockServerData()
	{
		m_ServerData.Unlock();
	}

protected:
	void GenerateDriversForXML();
	//Packets
	void ReadDriverInfoPacket(ENetPacket *ENetPacket, ENetPeer * pPeer);
	void ReadDriverReadyPacket(ENetPacket *pPacket);
	void ReadPacket(ENetEvent event);
	

	ServerMutexData m_ServerData;
	virtual void BroadcastPacket(ENetPacket *pPacket,enet_uint8 channel);


	std::vector<Driver> m_vecWaitForPlayers;

    ENetHost * m_pServer;


};


bool AddNetworkTimer();
bool RemoveNetworkTimer();

extern DLLEXPORT void SetServer(bool bStatus);
extern DLLEXPORT void SetClient(bool bStatus);
extern DLLEXPORT bool IsServer();
extern DLLEXPORT bool IsClient();

extern DLLEXPORT bool NetworkInit();
extern DLLEXPORT Server *GetServer();
extern DLLEXPORT Client *GetClient();
extern DLLEXPORT Network *GetNetwork();
void NetworkListen();

bool AddressMatch(ENetAddress &a1,ENetAddress &a2);

#endif // NETWORK_H
