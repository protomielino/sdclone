/***************************************************************************

    file                 : networkingmenu.cpp
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


/* This file deals with the networking menus.  
   The server sees a list of players and the client sees a list of other players.
   Should also allow choosing IP address, track, etc ...
*/

#include <cstdlib>
#include <cstdio>
#include <string>

#include <portability.h>
#include <tgfclient.h>

#include <raceman.h>
#include <robot.h>
#include <cars.h>
#include <network.h>

#include <racescreens.h>
#include <playerconfig.h>
#include <playerpref.h>
#include <hostsettingsmenu.h>
#include <carsettingsmenu.h>

#include "racesituation.h"
#include "racemain.h"
#include "raceinit.h"
#include "racestate.h"
#include "raceresults.h"

#include "raceenginemenus.h"
#include "networkingmenu.h"



int g_readystatus[MAXNETWORKPLAYERS];
int g_playerNames[MAXNETWORKPLAYERS];
int g_carNames[MAXNETWORKPLAYERS];

int g_trackHd;
int g_lapsHd;
int g_catHd;
int g_OutlineId;

int g_CarSetupButtonId;
int g_HostSettingsButtonId;
int g_DisconnectButtonId;
int g_CancelButtonId;
int g_ReadyCheckboxId;
int g_RaceSetupId;

static int g_IPEditId;
static int g_NameId;


static char	buf[1024];
static void	*racemanMenuHdle = NULL;

//static char *g_pDriver = NULL;
//static char *g_pCar = NULL;
static std::string g_strDriver;
static std::string g_strCar;

//static char ip[1024];

static float green[] = {0.0, 1.0, 0.0, 1.0};
static float white[] = {1.0, 1.0, 1.0, 1.0};

static std::string g_strHostIP = "127.0.0.1";

HostSettingsMenu g_HostMenu;
CarSettingsMenu g_CarMenu;

static void GetHumanDriver(Driver &driver,int index);
static void ServerPrepareStartNetworkRace(void *pVoid);
static void NetworkClientMenu(void * /* dummy */);


static void SetReadyStatus(int index,bool bStatus)
{
	GfuiStaticImageSetActive(racemanMenuHdle,g_readystatus[index],bStatus ? 1 : 0);
}

static void EnableMenuHostButtons(bool bChecked)
{
	// Disable/enable menu selections
	const int isEnabled = bChecked ? GFUI_DISABLE : GFUI_ENABLE;
	GfuiEnable(racemanMenuHdle, g_CarSetupButtonId, isEnabled);
	GfuiEnable(racemanMenuHdle, g_HostSettingsButtonId, isEnabled);		
	GfuiEnable(racemanMenuHdle, g_CancelButtonId, isEnabled);
	GfuiEnable(racemanMenuHdle, g_RaceSetupId, isEnabled);
}

static void onHostPlayerReady(tCheckBoxInfo* pInfo)
{
	SetReadyStatus(GetNetwork()->GetDriverIdx()-1, pInfo->bChecked);
	GetNetwork()->SetDriverReady(pInfo->bChecked);
	EnableMenuHostButtons(pInfo->bChecked);
}

static void EnableMenuClientButtons(bool bChecked)
{
	// Disable/enable menu selections
	const int isEnabled = bChecked ? GFUI_ENABLE : GFUI_DISABLE;
	GfuiEnable(racemanMenuHdle, g_CarSetupButtonId,isEnabled);
	GfuiEnable(racemanMenuHdle, g_DisconnectButtonId,isEnabled);
}

static void onClientPlayerReady(tCheckBoxInfo* pInfo)
{
	SetReadyStatus(GetNetwork()->GetDriverIdx()-1,pInfo->bChecked);
	GetNetwork()->SetDriverReady(pInfo->bChecked);
	EnableMenuClientButtons(pInfo->bChecked);
}

static std::string 
GetTrackName(const char *category, const char *trackName)
{
    void *trackHandle;
	std::string name;

    sprintf(buf, "tracks/%s/%s/%s.%s", category, trackName, trackName, TRKEXT);
    trackHandle = GfParmReadFile(buf, GFPARM_RMODE_STD);

    if (trackHandle) {
        name = GfParmGetStr(trackHandle, TRK_SECT_HDR, TRK_ATT_NAME, trackName);
    } else {
        GfTrace("Could not read file %s\n", buf);
        return 0;
    }

    GfParmReleaseHandle(trackHandle);
    return name;
}

static void
onChangeCarCategory(void * pData)
{
	
}

static std::string
GetTrackPreviewFileName(const char *pszCategory, const char *pszTrack)
{
	char buf[256];

	// Try JPEG first.
    snprintf(buf, sizeof(buf), "tracks/%s/%s/%s.jpg", pszCategory, pszTrack, pszTrack);
    buf[255] = 0; /* snprinf manual is not clear about that ... */

	// Then PNG if not found.
	if (!GfFileExists(buf))
	{
		snprintf(buf, sizeof(buf), "tracks/%s/%s/%s.png", pszCategory, pszTrack, pszTrack);
		buf[255] = 0; /* snprinf manual is not clear about that ... */
	}

	// Then fallback.
	if (!GfFileExists(buf))
		strncpy(buf, "data/img/splash-networkrace.jpg", sizeof(buf));
	
	return std::string(buf);
}

static std::string
GetTrackOutlineFileName(const char *pszCategory,const char *pszTrack)
{
	char buf[256];

    snprintf(buf, sizeof(buf), "tracks/%s/%s/outline.png", pszCategory, pszTrack);
	
	if (!GfFileExists(buf))
		strncpy(buf, "data/img/transparent.png", sizeof(buf));
	
	return std::string(buf);
}

static void 
UpdateNetworkPlayers()
{
	Network *pNetwork = GetNetwork();

	if (pNetwork->GetRefreshDisplay() == false)
		return;

	//Set current driver that camera will look at
	pNetwork->SetCurrentDriver();

	//reload xml file
	GetNetwork()->SetRaceXMLFile("config/raceman/networkrace.xml");
	ReInfo->params = GfParmReadFileLocal("config/raceman/networkrace.xml",GFPARM_RMODE_REREAD);
	assert(ReInfo->params);

	ReInfo->_reName = GfParmGetStr(ReInfo->params, RM_SECT_HEADER, RM_ATTR_NAME, "");
	assert(ReInfo->_reName);

	//Update track info
	std::string strTrackPath = GfParmGetStr(ReInfo->params, "Tracks/1", RM_ATTR_NAME, "");
	std::string strCategory = GfParmGetStr(ReInfo->params, "Tracks/1", RM_ATTR_CATEGORY, "");

	std::string strTrackName = GetTrackName(strCategory.c_str(),strTrackPath.c_str());

	sprintf(buf, "%s", strTrackName.c_str());
	GfuiLabelSetText(racemanMenuHdle,g_trackHd,buf);
	
	int laps = (int)GfParmGetNum(ReInfo->params, ReInfo->_reName,"laps", "",0);
	sprintf(buf, "%i", laps);
	GfuiLabelSetText(racemanMenuHdle,g_lapsHd,buf);

	GfuiScreenAddBgImg(racemanMenuHdle,
					   GetTrackPreviewFileName(strCategory.c_str(),strTrackPath.c_str()).c_str());
	GfuiStaticImageSet(racemanMenuHdle, g_OutlineId,
					   GetTrackOutlineFileName(strCategory.c_str(),strTrackPath.c_str()).c_str());

	// Update category info
	std::string strCarCat;
	bool bCollisions;
	GetNetwork()->GetHostSettings(strCarCat,bCollisions);
	GfuiLabelSetText(racemanMenuHdle,g_catHd,strCarCat.c_str());

	//fill in player data
	int nCars = GfParmGetEltNb(ReInfo->params, RM_SECT_DRIVERS);

	char	dname[256];
	char    robpath[256];

	float *pColor = &green[0];

	bool bEveryoneReadyToRace = true;
	
	for (int i = 1; i < nCars+1; i++) 
	{
		sprintf(dname, "%s/%d", RM_SECT_DRIVERS, i);

		const char* robot = GfParmGetStr(ReInfo->params, dname, RM_ATTR_MODULE, "");

		//lookup playerName and car name
		sprintf(robpath,"drivers/%s/%s.xml",robot,robot);
		void *pMod = GfParmReadFileLocal(robpath,GFPARM_RMODE_REREAD);

		if (pMod == NULL)
		{
			//try again in other path
			sprintf(robpath,"drivers/%s/%s.xml",robot,robot);
			pMod = GfParmReadFile(robpath,GFPARM_RMODE_REREAD);
			if (pMod == NULL)
				continue;
		}

		assert(pMod);

		char ppname[256];
		int idx = GfParmGetNum(ReInfo->params, dname, RM_ATTR_IDX, "",0);
		sprintf(ppname,"Robots/index/%d",idx);
		const char* name = GfParmGetStr(pMod, ppname, RM_ATTR_NAME, "");

		const char* car = GfParmGetStr(pMod, ppname, "car name", "");
		std::string strRealCar = GfCars::self()->getCar(car)->getName();

		// WAIT : pNData->m_vecReadyStatus[i-1] ?!
		//        This can only work when _only_ networkhuman drivers in the race
		//        (that is _no_robot_driver_) ; because m_vecReadyStatus is indexed
		//        by the networkhuman drivers list.
		// TO fix this, 2 solutions:
		// 1) make the networking module take care of the robot drivers too
		//    (in m_vecReadyStatus, m_vecNetworkPlayers, ...)
		// 2) make the networking _menu_ only take care of the networkhuman drivers.
		MutexData *pNData = GetNetwork()->LockNetworkData();
		bool bReady = pNData->m_vecReadyStatus[i-1];
		GetNetwork()->UnlockNetworkData();

		int readyindex = 0;
		if (bReady)
			readyindex = 1;	
		else
			bEveryoneReadyToRace = false;

		bool bLocalPlayer = false;
		if (strcmp(GetNetwork()->GetDriverName(),name)==0)
		{
			bLocalPlayer = true;
			pColor = &green[0];
			g_strCar = strRealCar;
			//Make sure checkbox matches ready state
			GfuiCheckboxSetChecked(racemanMenuHdle, g_ReadyCheckboxId, bReady);
			if (GetClient())
				EnableMenuClientButtons(bReady);
			else
				EnableMenuHostButtons(bReady);
		}
		else
			pColor = &white[0];

		GfuiVisibilitySet(racemanMenuHdle,g_readystatus[i-1],true);
		GfuiStaticImageSetActive(racemanMenuHdle,g_readystatus[i-1],readyindex);
		GfuiLabelSetColor(racemanMenuHdle, g_playerNames[i-1], pColor);
		GfuiLabelSetText(racemanMenuHdle,g_playerNames[i-1],name);

		GfuiLabelSetColor(racemanMenuHdle, g_carNames[i-1], pColor);
		GfuiLabelSetText(racemanMenuHdle,g_carNames[i-1],strRealCar.c_str());
		GfParmReleaseHandle(pMod);
	}

	//Clear out
	for (int i=nCars;i<MAXNETWORKPLAYERS;i++)
	{
		GfuiVisibilitySet(racemanMenuHdle,g_readystatus[i],false);
		GfuiLabelSetText(racemanMenuHdle,g_playerNames[i],"");
		GfuiLabelSetText(racemanMenuHdle,g_carNames[i],"");
	}

	pNetwork->SetRefreshDisplay(false);
	GfelPostRedisplay();

	if (IsClient())
	{	
		GetClient()->ConnectToClients();

		if (!GetClient()->TimeSynced())
		{
			GetClient()->SendServerTimeRequest();
		}
	}

	if (IsServer())
	{
		if (bEveryoneReadyToRace && nCars > 1)
			ServerPrepareStartNetworkRace(NULL);
	}
}


static void
reNetworkClientDisconnect(void * /* dummy */)
{
	GfOut("Disconnecting from server\n");
	if (GetClient())
		GetClient()->Disconnect();

	GfuiScreenActivate(ReSituation()->_reMenuScreen);

}


static void
CheckDriversCategory()
{
	bool bDriversChange = false;
	std::string strCarCat;
	bool bCollisions;
	GetNetwork()->GetHostSettings(strCarCat,bCollisions);
	if (strCarCat =="All")
		return;

	const std::vector<std::string> vecCars =
		GfCars::self()->getCarIdsInCategory(strCarCat);

	//Make sure all cars are in the correct category or force change of car
	Driver *pDrivers = NULL;
	unsigned int count = 0;
	ServerMutexData *pSData = GetServer()->LockServerData();

	count = pSData->m_vecNetworkPlayers.size();
	for (unsigned int i=0;i<count;i++)
	{
		const GfCar* pCar = GfCars::self()->getCar(pSData->m_vecNetworkPlayers[i].car);
		if (pCar->getCategoryId() != strCarCat)
		{
			//Pick first car in categroy
			strncpy(pSData->m_vecNetworkPlayers[i].car,vecCars[0].c_str(),64);
			bDriversChange = true;
			GetServer()->OverrideDriverReady(pSData->m_vecNetworkPlayers[i].idx,false);
		}
	}

	if(bDriversChange)
	{
		GetServer()->CreateNetworkRobotFile();
	}
	
	//GetServer()->UnlockDrivers();
	GetServer()->UnlockServerData();
}

static void
HostServerIdle(void)
{
	GfuiIdle();
	if (IsServer())
	{
		if (GetServer()->GetRaceInfoChanged())
		{
			CheckDriversCategory();
			//Send to clients all of the xml files we modified and client needs to reload
			GetServer()->SendFilePacket("drivers/networkhuman/networkhuman.xml");
			GetServer()->SendFilePacket("config/raceman/networkrace.xml");
			GetServer()->SendRaceSetupPacket();
			GetServer()->SendDriversReadyPacket();
			GetServer()->SetRaceInfoChanged(false);
		}
		else
		{
			if (GetServer()->GetRefreshDisplay())
			{
				UpdateNetworkPlayers();
			}

		}

		GfelPostRedisplay();
	}
	
    /* Let CPU take breath (and fans stay at low and quiet speed) */
    GfSleep(0.001);
}


static void
ClientIdle(void)
{
	GfuiIdle();
	if (IsClient())
	{
		if (!GetClient()->TimeSynced())
		{
			GetClient()->SendServerTimeRequest();
		}

		if (GetClient()->GetRefreshDisplay())
		{
			//Update the screen
			UpdateNetworkPlayers();
			GfelPostRedisplay();
		}

		if (GetClient()->PrepareToRace())
		{
			GetClient()->SetLocalDrivers();
			ReStartNewRace(NULL);
		}

		if (!GetClient()->IsConnected())
		{
			reNetworkClientDisconnect(NULL);
		}

		GfelPostRedisplay();
	}
	
    /* Let CPU take breath (and fans stay at low and quiet speed) */
    GfSleep(0.001);
}

static void
NetworkRaceInfo()
{
	GetServer()->SetRaceXMLFile("config/raceman/networkrace.xml");

	Driver driver;
	std::string strName = GetServer()->GetDriverName();
	if (strName =="")
	{
		GetHumanDriver(driver,1);
		driver.client = false;
		GetServer()->UpdateDriver(driver);
		GetServer()->SetDriverName(driver.name);
	}

	//Look up race info
	void *params = ReInfo->params =
		GfParmReadFileLocal("config/raceman/networkrace.xml",GFPARM_RMODE_REREAD);
	assert(ReInfo->params);
	ReInfo->_reName = GfParmGetStr(ReInfo->params, RM_SECT_HEADER, RM_ATTR_NAME, "");
}

static void
NetworkDisplay(void)
{
}

static void OnActivateNetworkClient(void *)
{
	GfelSetIdleCB(ClientIdle);
}


static void
OnActivateNetworkHost(void *)
{
	MutexData *pNData = GetNetwork()->LockNetworkData();
	for (unsigned int i=0;i<pNData->m_vecReadyStatus.size();i++)
	{
		pNData->m_vecReadyStatus[i] = false;
	};

	GetNetwork()->UnlockNetworkData();

	GetServer()->SetRaceInfoChanged(true);
	ReInfo->params = GfParmReadFileLocal("config/raceman/networkrace.xml",GFPARM_RMODE_REREAD);
	assert(ReInfo->params);
	ReInfo->_reName = GfParmGetStr(ReInfo->params, RM_SECT_HEADER, RM_ATTR_NAME, "");
	GfelSetIdleCB(HostServerIdle);	
	GetServer()->SetRefreshDisplay(true);
}

static void
reNetworkServerDisconnect(void * /* dummy */)
{
	GfOut("Disconnecting all clients\n");
	if (GetServer())
		GetServer()->Disconnect();

	GfuiScreenActivate(ReInfo->_reMenuScreen);
}

static void
reCarSettingsMenu(void *pMenu)
{
	g_CarMenu.Init(pMenu,g_strCar.c_str());

	ReSetRacemanMenuHandle(g_CarMenu.GetMenuHandle());

	g_CarMenu.RunMenu();
}

static void
reNetworkHostSettingsMenu(void *pMenu)
{
	g_HostMenu.Init(pMenu);
	
	ReSetRacemanMenuHandle(g_HostMenu.GetMenuHandle());

	g_HostMenu.RunMenu();
}


// Host on-line race menu.
void
ReNetworkHostMenu(void * /* dummy */)
{
	if (!GetNetwork())
	{
		SetServer(true);
		SetClient(false);
		if (!GetServer()->Start(SPEEDDREAMSPORT))
		{
			SetServer(false);
			return;
		}
	}

    if (racemanMenuHdle) {
	GfuiScreenRelease(racemanMenuHdle);
    }

    racemanMenuHdle = GfuiScreenCreateEx(NULL, 
					 NULL, (tfuiCallback)OnActivateNetworkHost, 
					 NULL, (tfuiCallback)NULL, 
					 1);

    void *mparam = LoadMenuXML("networkhostmenu.xml");

    CreateStaticControls(mparam,racemanMenuHdle);

    GfuiMenuDefaultKeysAdd(racemanMenuHdle);

	ReSetRacemanMenuHandle(racemanMenuHdle);

	NetworkRaceInfo();

    g_trackHd = CreateLabelControl(racemanMenuHdle,mparam,"trackname");

	g_lapsHd = CreateLabelControl(racemanMenuHdle,mparam,"lapcountname");
	g_catHd = CreateLabelControl(racemanMenuHdle,mparam,"carcatname");
    
	g_OutlineId = CreateStaticImageControl(racemanMenuHdle,mparam,"outlineimage");
	//Show players
    for (int i = 0; i < MAXNETWORKPLAYERS; i++) 
	{
	    char buf[1024];
	    sprintf(buf,"ready%i",i);
	    g_readystatus[i] = CreateStaticImageControl(racemanMenuHdle,mparam,buf);
		GfuiVisibilitySet(racemanMenuHdle,g_readystatus[i],false);
	    sprintf(buf,"driver%i",i);
	    g_playerNames[i] = CreateLabelControl(racemanMenuHdle,mparam,buf);
		GfuiLabelSetText(racemanMenuHdle,g_playerNames[i],"");
	    sprintf(buf,"car%i",i);
	    g_carNames[i] =  CreateLabelControl(racemanMenuHdle,mparam,buf);
		GfuiLabelSetText(racemanMenuHdle,g_carNames[i],"");
    }

	g_ReadyCheckboxId = CreateCheckboxControl(racemanMenuHdle,mparam,"playerreadycheckbox",NULL,onHostPlayerReady);
	g_HostSettingsButtonId = CreateButtonControl(racemanMenuHdle,mparam,"networkhostsettings",racemanMenuHdle,reNetworkHostSettingsMenu);
	g_RaceSetupId = CreateButtonControl(racemanMenuHdle,mparam,"racesetup",racemanMenuHdle,ReConfigureRace);
	g_CarSetupButtonId = CreateButtonControl(racemanMenuHdle,mparam,"car",racemanMenuHdle,reCarSettingsMenu);

	CreateButtonControl(racemanMenuHdle,mparam,"start race",NULL,ServerPrepareStartNetworkRace);
	g_CancelButtonId = CreateButtonControl(racemanMenuHdle,mparam,"cancel",NULL,reNetworkServerDisconnect);

    GfParmReleaseHandle(mparam);
	
	UpdateNetworkPlayers();

    GfuiScreenActivate(racemanMenuHdle);
}

static void
ShowWaitingToConnectScreen()
{
	if (racemanMenuHdle) 
	{
		GfuiScreenRelease(racemanMenuHdle);
	}
	racemanMenuHdle = GfuiScreenCreateEx(NULL, 
						NULL, (tfuiCallback) NULL, 
						NULL, (tfuiCallback)NULL, 
						1);
	void *params = ReInfo->params;
	const char *str = GfParmGetStr(params, RM_SECT_HEADER, RM_ATTR_BGIMG, 0);
	if (str) 
	{
		GfuiScreenAddBgImg(racemanMenuHdle, str);
	}
	    
	GfuiMenuDefaultKeysAdd(racemanMenuHdle);

	GfuiTitleCreate(racemanMenuHdle, "Trying to Connect to Server...", 30);
	GfuiScreenActivate(racemanMenuHdle);
	GfelPostRedisplay();
}

void
ReNetworkClientConnectMenu(void * /* dummy */)
{
	ShowWaitingToConnectScreen();
	
	if (!GetClient())
	{
		SetServer(false);
		SetClient(true);

		Driver driver;
		GetHumanDriver(driver,1);
		driver.client = true;
		strcpy(driver.name,g_strDriver.c_str());
		if (!GetClient()->ConnectToServer((char*)g_strHostIP.c_str(),SPEEDDREAMSPORT,&driver))
		{
			//failed so back to connect menu
			NetworkClientMenu(NULL);
			return;
		}

		//GetClient()->SendDriverInfoPacket(&driver);
	}

	if (racemanMenuHdle) {
	GfuiScreenRelease(racemanMenuHdle);
	}

	racemanMenuHdle = GfuiScreenCreateEx(NULL, 
					NULL, (tfuiCallback)OnActivateNetworkClient, 
					NULL, (tfuiCallback)NULL, 
					1);

    void *mparam = LoadMenuXML("networkclientmenu.xml");
	CreateStaticControls(mparam,racemanMenuHdle);

	GfuiMenuDefaultKeysAdd(racemanMenuHdle);

	ReSetRacemanMenuHandle(racemanMenuHdle);

	g_trackHd = CreateLabelControl(racemanMenuHdle,mparam,"trackname");

	g_lapsHd = CreateLabelControl(racemanMenuHdle,mparam,"lapcountname");
	g_catHd = CreateLabelControl(racemanMenuHdle,mparam,"carcatname");

	g_OutlineId = CreateStaticImageControl(racemanMenuHdle,mparam,"outlineimage");
	//Show players
	for (int i = 0; i < MAXNETWORKPLAYERS; i++) 
	{
		char buf[1024];
		sprintf(buf,"ready%i",i);
		g_readystatus[i] = CreateStaticImageControl(racemanMenuHdle,mparam,buf);
		GfuiVisibilitySet(racemanMenuHdle,g_readystatus[i],false);

		sprintf(buf,"driver%i",i);
		g_playerNames[i] = CreateLabelControl(racemanMenuHdle,mparam,buf);
		GfuiLabelSetText(racemanMenuHdle,g_playerNames[i],"");
		sprintf(buf,"car%i",i);
		g_carNames[i] =  CreateLabelControl(racemanMenuHdle,mparam,buf);
		GfuiLabelSetText(racemanMenuHdle,g_carNames[i],"");
	}

	g_ReadyCheckboxId = CreateCheckboxControl(racemanMenuHdle,mparam,"playerreadycheckbox",NULL,onClientPlayerReady);
	g_CarSetupButtonId = CreateButtonControl(racemanMenuHdle,mparam,"car",racemanMenuHdle,reCarSettingsMenu);

	g_DisconnectButtonId = CreateButtonControl(racemanMenuHdle,mparam,"disconnect",NULL,reNetworkClientDisconnect);


	GfParmReleaseHandle(mparam);

	UpdateNetworkPlayers();
	GfuiScreenActivate(racemanMenuHdle);
	GfelSetIdleCB(ClientIdle);
}

static void 
ChangeName(void * /*dummy*/)
{
   	char	*val;

   	val = GfuiEditboxGetString(racemanMenuHdle, g_NameId);
	if (val!=NULL)
		g_strDriver = val;
}

static void
ChangeIP(void * /* dummy */)
{
   	char	*val;

   	val = GfuiEditboxGetString(racemanMenuHdle, g_IPEditId);
	if (val!=NULL)
		g_strHostIP = val;
}


static void
LookupPlayerSetup(std::string & strDriver,std::string & strCar)
{
    	void	*drvinfo;
	
	char buf[255];
	sprintf(buf, "%s", HM_DRV_FILE);

	drvinfo = GfParmReadFileLocal(buf, GFPARM_RMODE_REREAD);
		assert(drvinfo);
	if (drvinfo == NULL) {
		return;
	}

	char sstring[256];

	sprintf(sstring, "%s/%s/%d", ROB_SECT_ROBOTS, ROB_LIST_INDEX, 1);
	strDriver = GfParmGetStr(drvinfo, sstring, ROB_ATTR_NAME, "");
	strCar = GfParmGetStr(drvinfo, sstring, ROB_ATTR_CAR, "");
	GfParmReleaseHandle(drvinfo);
}

static void
NetworkClientMenu(void * /* dummy */)
{
	const char	*str;

	LookupPlayerSetup(g_strDriver,g_strCar);


	if (racemanMenuHdle) {
		GfuiScreenRelease(racemanMenuHdle);
	}
	racemanMenuHdle = GfuiScreenCreateEx(NULL, 
										 NULL, (tfuiCallback)NULL, 
										 NULL, (tfuiCallback)NULL, 
										 1);

	void *params = ReInfo->params;
	str = GfParmGetStr(params, RM_SECT_HEADER, RM_ATTR_BGIMG, 0);
	if (str) {
		GfuiScreenAddBgImg(racemanMenuHdle, str);
	}
    
	GfuiMenuDefaultKeysAdd(racemanMenuHdle);

	GfuiTitleCreate(racemanMenuHdle, "Connect to Host", 15);

	int x,x2,x3,x4,y,dy;
	x = 20;
	x2 = 170;
	x3 = x2 + 100;
	x4 = x2 + 200;
	y = 370;
	dy = 30;
	
	
	GfuiLabelCreate(racemanMenuHdle, "IP Address:", GFUI_FONT_MEDIUM, x, y, GFUI_ALIGN_HL_VB, 0);
	g_IPEditId = GfuiEditboxCreate(racemanMenuHdle, "", GFUI_FONT_MEDIUM_C,
					x2+10, y, 180, 16, NULL, (tfuiCallback)NULL, ChangeIP);

	y-=dy;

	char namebuf[255];
	sprintf(namebuf,"%s",g_strDriver.c_str());
	GfuiLabelCreate(racemanMenuHdle, "Name:", GFUI_FONT_MEDIUM, x, y, GFUI_ALIGN_HL_VB, 0);
	g_NameId = GfuiEditboxCreate(racemanMenuHdle, namebuf, GFUI_FONT_MEDIUM_C, x2+10, y, 180,16,NULL,(tfuiCallback)NULL,ChangeName);

	GfuiButtonCreate(racemanMenuHdle, "Connect", GFUI_FONT_LARGE, 210, 40, 150, GFUI_ALIGN_HC_VB, GFUI_MOUSE_UP,
	NULL, ReNetworkClientConnectMenu, NULL, (tfuiCallback)NULL, (tfuiCallback)NULL);
	
	GfuiButtonCreate(racemanMenuHdle, "Back", GFUI_FONT_LARGE, 430, 40, 150, GFUI_ALIGN_HC_VB, GFUI_MOUSE_UP,
	ReInfo->_reMenuScreen, GfuiScreenActivate, NULL, (tfuiCallback)NULL, (tfuiCallback)NULL);
	
	GfuiScreenActivate(racemanMenuHdle);

}

void ReNetworkMenu(void *)
{
    const char	*str;

	if (GetNetwork())
	{
		GetNetwork()->ResetNetwork();
	}

    racemanMenuHdle = GfuiScreenCreateEx(NULL, 
					 NULL, (tfuiCallback)NULL, 
					 NULL, (tfuiCallback)NULL, 
					 1);

	void *params = ReInfo->params;
    str = GfParmGetStr(params, RM_SECT_HEADER, RM_ATTR_BGIMG, 0);
    if (str) {
	GfuiScreenAddBgImg(racemanMenuHdle, str);
    }
    
    GfuiMenuDefaultKeysAdd(racemanMenuHdle);

    str = GfParmGetStr(params, RM_SECT_HEADER, RM_ATTR_NAME, 0);
    if (str) {
	GfuiTitleCreate(racemanMenuHdle, str, strlen(str));
	
    }

    GfuiMenuButtonCreate(racemanMenuHdle,
			 "Host Online Race", "Host a race",
			 NULL, GFUI_ALIGN_HL_VB, ReNetworkHostMenu);

    GfuiMenuButtonCreate(racemanMenuHdle,
			 "Connect to Online Race", "Connect to a race on Internet or Network",
			 NULL, GFUI_ALIGN_HL_VB, NetworkClientMenu);

    GfuiMenuBackQuitButtonCreate(racemanMenuHdle,
				 "Back", "Return to previous Menu",
				 ReInfo->_reMenuScreen, GfuiScreenActivate);

    GfuiScreenActivate(racemanMenuHdle);
}

static void
ServerPrepareStartNetworkRace(void *pVoid)
{
	GetServer()->SetLocalDrivers();
	//Tell all clients to prepare to race and wait for response from all clients
	GetServer()->SendPrepareToRacePacket();

	//restore the idle function
	GfelSetIdleCB(GfuiIdle);
	ReStartNewRace(pVoid);
}

// Retrieve the Driver instance with given index in the human module interface list
static void
GetHumanDriver(Driver &driver,int index)
{
	char buf[255];
	sprintf(buf,"drivers/human/human.xml");
	void *params = GfParmReadFileLocal(buf,GFPARM_RMODE_REREAD);
	assert(params);
	char path2[256];
	sprintf(path2, "Robots/index/%d",index);
	strncpy(driver.name,GfParmGetStr(params, path2, "name",NULL),64);
	strncpy(driver.car,GfParmGetStr(params, path2, "car name",NULL),64);
	strncpy(driver.type,GfParmGetStr(params, path2, "type",NULL),64);
	strncpy(driver.skilllevel,GfParmGetStr(params, path2, "skill level",NULL),64);

	driver.racenumber = GfParmGetNum(params, path2, "race number",NULL,1.0);
	driver.red = GfParmGetNum(params, path2, "red",NULL,1.0);
	driver.green = GfParmGetNum(params, path2, "green",NULL,1.0);
	driver.blue = GfParmGetNum(params, path2, "blue",NULL,1.0);

	strncpy(driver.module,NETWORKROBOT,64);
	GfParmReleaseHandle(params);
}
