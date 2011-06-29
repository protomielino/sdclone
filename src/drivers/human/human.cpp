/***************************************************************************

    file                 : human.cpp
    created              : Sat Mar 18 23:16:38 CET 2000
    copyright            : (C) 2000 by Eric Espie
    email                : torcs@free.fr
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

/** @file   
    		
    @author	<a href=mailto:torcs@free.fr>Eric Espie</a>
    @version	$Id$
*/

// Flags for selecting the "binary control steering code" variant :
// * Old variant
#define OLD 0
// * Jepz variant
#define JEPZ 1
// * JPM variant
#define JPM 2
// * if neither JEPZ or JPM variant are selected, the old code is used.
#define BINCTRL_STEERING JPM

#include <map>
#include <vector>
#include <string>

#include <portability.h>
#include <tgfclient.h>
#include <robottools.h>	//Rt*
#include <robot.h>
#include <playerpref.h>

#include "pref.h"
#include "human.h"


static const int FuelReserve = 5;
static const tdble MaxFuelPerMeter = 0.0008;	// [kg/m] fuel consumption.

static void initTrack(int index, tTrack* track, void *carHandle, void **carParmHandle, tSituation *s);
static void drive_mt(int index, tCarElt* car, tSituation *s);
static void drive_at(int index, tCarElt* car, tSituation *s);
static void newrace(int index, tCarElt* car, tSituation *s);
static int  pitcmd(int index, tCarElt* car, tSituation *s);
static void SetFuelAtRaceStart(tTrack *track, void **carParmHandle, tSituation *s, int idx);
static char	sstring[1024];
static char	buf[1024];

static tTrack *curTrack;

static float color[] = {0.0, 0.0, 1.0, 1.0};

bool joyPresent = false;
static tCtrlJoyInfo	*joyInfo = NULL;
static tCtrlMouseInfo	*mouseInfo = NULL;
static int ControlsUpdaterIndex = -1;

std::vector<tHumanContext*> HCtx;

static bool speedLimiter = false;
static tdble speedLimit;

typedef struct
{
	int state;
	int edgeDn;
	int edgeUp;
} tKeyInfo;

// Keyboard map for all players
// (key code => index of the associated command in keyInfo / lastReadKeyState).
typedef std::map<int,int> tKeyMap;
static tKeyMap mapKeys;
static int keyIndex = 0;

// Last read state for each possible player command key.
static int lastReadKeyState[GFUIK_MAX+1];

// Up-to-date info for each possible player command key (state, edge up, edge down)
static tKeyInfo keyInfo[GFUIK_MAX+1];

static bool firstTime = false;
static tdble lastKeyUpdate = -10.0;

void *PrefHdle = NULL;

// Human drivers names.
static std::vector<std::string> VecNames;

// Number of human drivers (initialized by moduleWelcome).
static int NbDrivers = -1;

// List of permited gear changes with hbox transmission
// prevents mis-selection with thumbstick
// Note 'N' selectable from any gear from ..... 654321NR ... to :
const static int hboxChanges[] = {   0x02, // 0b00000010,  // R
                                     0x2B, // 0b00101011,  // 1
                                     0x57, // 0b01010111,  // 2
                                     0xAA, // 0b10101010,  // 3
                                     0x52, // 0b01010010,  // 4
                                     0xA2, // 0b10100010,  // 5
                                     0x52  // 0b01010010   // 6
                                };

#ifdef _WIN32
/* Must be present under MS Windows */
BOOL WINAPI DllEntryPoint (HINSTANCE hDLL, DWORD dwReason, LPVOID Reserved)
{
    return TRUE;
}
#endif


static void
shutdown(const int index)
{
	int idx = index - 1;

	VecNames.erase(VecNames.begin() + idx);

	free (HCtx[idx]);
	HCtx[idx] = 0;

	if (firstTime) {
		GfParmReleaseHandle(PrefHdle);
		GfctrlJoyRelease(joyInfo);
		GfctrlMouseRelease(mouseInfo);
		GfuiKeyEventRegisterCurrent(0);
		firstTime = false;
	}//if firstTime
}//shutdown


/**
 * 
 *	InitFuncPt
 *
 *	Robot functions initialisation.
 *
 *	@param pt	pointer on functions structure
 *  @return 0
 */
static int
InitFuncPt(int index, void *pt)
{
	tRobotItf *itf = (tRobotItf *)pt;
	const int idx = index - 1;

	// Choose this driver as the one who will exclusively read the controls state
	// (if no other was choosen in this race).
	if (ControlsUpdaterIndex < 0)
		ControlsUpdaterIndex = index;

	// Initialize mouse and joystick controls backend if not already done.
	if (!firstTime) {
		firstTime = true;
		joyInfo = GfctrlJoyInit();
		if (joyInfo) {
			joyPresent = true;
		}//if joyInfo
		mouseInfo = GfctrlMouseInit();
	}//if !firstTime

	/* Allocate a new context for that player */
	if ((int)HCtx.size() < idx + 1)
	  HCtx.resize(idx + 1);
	HCtx[idx] = (tHumanContext *) calloc (1, sizeof (tHumanContext));

	HCtx[idx]->antiLock = 1.0;
	HCtx[idx]->antiSlip = 1.0;

	itf->rbNewTrack = initTrack;	/* give the robot the track view called */
	/* for every track change or new race */
	itf->rbNewRace  = newrace;

	HmReadPrefs(index);

	/* drive during race */
	itf->rbDrive = (HCtx[idx]->transmission == eTransAuto) ? drive_at : drive_mt;
	itf->rbShutdown = shutdown;
	itf->rbPitCmd   = pitcmd;
	itf->index      = index;

	return 0;
}//InitFuncPt


/**
 * 
 * moduleWelcome
 *
 * First function of the module called at load time :
 *  - the caller gives the module some information about its run-time environment
 *  - the module gives the caller some information about what he needs
 * MUST be called before moduleInitialize()
 *
 * @param	welcomeIn Run-time info given by the module loader at load time
 * @param welcomeOut Module run-time information returned to the called
 * @return 0 if no error occured, not 0 otherwise
 */
extern "C" int
moduleWelcome(const tModWelcomeIn* welcomeIn, tModWelcomeOut* welcomeOut)
{
	// Open and load the human drivers params file
	sprintf(buf, "%sdrivers/human/human.xml", GfLocalDir());
	void *drvInfo = GfParmReadFile(buf, GFPARM_RMODE_REREAD | GFPARM_RMODE_CREAT);

	// Count the number of human drivers registered in the params 
	NbDrivers = -1;
	if (drvInfo) {
		const char *driver;
		do {
			NbDrivers++;
			sprintf(sstring, "Robots/index/%d", NbDrivers+1);
			driver = GfParmGetStr(drvInfo, sstring, "name", "");
		} while (strlen(driver) > 0);

		GfParmReleaseHandle(drvInfo);	// Release in case we got it.
	}//if drvInfo

	welcomeOut->maxNbItf = NbDrivers;

	return 0;
}//moduleWelcome


/**
 * 
 * moduleInitialize
 *
 * Module entry point
 *
 * @param modInfo	administrative info on module
 * @return 0 if no error occured, -1 if any error occured 
 */
extern "C" int
moduleInitialize(tModInfo *modInfo)
{
	if (NbDrivers <= 0) {
		GfOut("human : No human driver registered, or moduleMaxInterfaces() was not called (NbDrivers=%d)\n", NbDrivers);
		return -1;
	}

	// Reset module interfaces info.
	memset(modInfo, 0, NbDrivers*sizeof(tModInfo));

	// Clear the local driver name vector
	VecNames.clear();
    
	// Open and load the human drivers params file
	sprintf(buf, "%sdrivers/human/human.xml", GfLocalDir());
	void *drvInfo = GfParmReadFile(buf, GFPARM_RMODE_REREAD | GFPARM_RMODE_CREAT);

	if (drvInfo) {
		// Fill the module interfaces info : each driver is associated to 1 interface.
		for (int i = 0; i < NbDrivers; i++) {
			sprintf(sstring, "Robots/index/%d", i+1);
			std::string driver =GfParmGetStr(drvInfo, sstring, "name", "");
			if (driver.size() > 0) {
				VecNames.push_back(driver); // Don't rely on GfParm allocated data
				char *pName = new char[VecNames[i].size()+1];
				memset((void*)pName,0,VecNames[i].size()+1);
				strncpy(pName,VecNames[i].c_str(),VecNames[i].size());
				modInfo->name = pName;
				//modInfo->name    = VecNames[i].c_str();	/* name of the module (short) */
				modInfo->desc    = "Joystick controlable driver";	/* description of the module (can be long) */
				modInfo->fctInit = InitFuncPt;	/* init function */
				modInfo->gfId    = ROB_IDENT;	/* supported framework version */
				modInfo->index   = i+1;
				modInfo++;
			}//if strlen
		}//for i
	
		GfParmReleaseHandle(drvInfo);	// Release in case we got it.
	}//if drvInfo
   
	return 0;
}//moduleInitialize


/**
 * moduleTerminate
 *
 * Module exit point
 *
 * @return 0
 */
extern "C" int
moduleTerminate()
{
	VecNames.clear();	//Free local copy of driver names
	return 0;
}//moduleTerminate


/**
 * initTrack
 *
 * Search under drivers/human/cars/<carname>/<trackname>.xml
 *
 * @param index
 * @param track
 * @param carHandle
 * @param carParmHandle
 * @param s situation provided by the sim
 *
 */
static void
initTrack(int index, tTrack* track, void *carHandle, void **carParmHandle, tSituation *s)
{
	char trackname[256];

	const int idx = index - 1;

	curTrack = track;
	char *s1 = strrchr(track->filename, '/') + 1;
	char *s2 = strchr(s1, '.');
	strncpy(trackname, s1, s2 - s1);
	trackname[s2 - s1] = 0;
	sprintf(sstring, "Robots/index/%d", index);

	sprintf(buf, "%sdrivers/human/human.xml", GfLocalDir());
	void *drvInfo = GfParmReadFile(buf, GFPARM_RMODE_REREAD | GFPARM_RMODE_CREAT);
	std::string carname = (drvInfo != NULL)
		? GfParmGetStrNC(drvInfo, sstring, "car name", NULL)
		: "";

	sprintf(sstring, "%sdrivers/curcarnames.xml", GfLocalDir());
	void *curCars = GfParmReadFile(sstring, GFPARM_RMODE_REREAD);
	if (curCars) {
		sprintf(sstring, "drivers/human/%d", index + NbDrivers + 1);
		carname = GfParmGetStr(curCars, sstring, "car name", carname.c_str());
	}//if curCars

	sprintf(sstring, "%sdrivers/human/cars/%s/default.xml", GfLocalDir(), carname.c_str());
	*carParmHandle = GfParmReadFile(sstring, GFPARM_RMODE_REREAD);

	if (!*carParmHandle) {
		sprintf(sstring, "%s/drivers/human/car.xml", GfLocalDir());
		*carParmHandle = GfParmReadFile(sstring, GFPARM_RMODE_REREAD);
	}

	sprintf(sstring, "%sdrivers/human/cars/%s/%s.xml", GfLocalDir(), carname.c_str(), trackname);
	void *newhandle = GfParmReadFile(sstring, GFPARM_RMODE_REREAD);
	if (newhandle) {
		*carParmHandle = (*carParmHandle)
			? GfParmMergeHandles(*carParmHandle, newhandle,
				(GFPARM_MMODE_SRC|GFPARM_MMODE_DST|GFPARM_MMODE_RELSRC|GFPARM_MMODE_RELDST))
			: newhandle;

		if (*carParmHandle) {
			GfOut("Player: %s Loaded\n", sstring);
		}
	} else {
		if (*carParmHandle) {
			GfOut("Player: %s Default Setup Loaded\n", sstring);
		}
	}//if-else newhandle

	if (curTrack->pits.type != TR_PIT_NONE) {
		sprintf(sstring, "%s/%s/%d", HM_SECT_PREF, HM_LIST_DRV, index);
		HCtx[idx]->nbPitStopProg = (int)GfParmGetNum(PrefHdle, sstring, HM_ATT_NBPITS, (char*)NULL, 0);
		GfOut("Player: index %d , Pit stops %d\n", index, HCtx[idx]->nbPitStopProg);
	} else {
		HCtx[idx]->nbPitStopProg = 0;
	}//if-else curTrack->pits

	//Initial fuel fill computation
	SetFuelAtRaceStart(track, carParmHandle, s, idx);
	
	speedLimit = curTrack->pits.speedLimit;
	
	if(drvInfo) {
		GfParmReleaseHandle(drvInfo);
	}
}//initTrack


/**
 *
 * newrace
 *
 * @param index
 * @param car
 * @param s situation provided by the sim
 * 
 */
void
newrace(int index, tCarElt* car, tSituation *s)
{
	const int idx = index - 1;

	// Initialize engine RPM shifting threshold table for automatic shifting mode.
	for (int i = 0; i < MAX_GEARS; i++) {
		if (car->_gearRatio[i] != 0) {
			HCtx[idx]->shiftThld[i] = car->_enginerpmRedLine * car->_wheelRadius(2) * 0.85 / car->_gearRatio[i];
			GfOut("Gear %d: Spd %f\n", i, HCtx[idx]->shiftThld[i] * 3.6);
		} else {
			HCtx[idx]->shiftThld[i] = 10000.0;
		}
	}//for i

	// Center the mouse.
	if (HCtx[idx]->mouseControlUsed) {
		GfctrlMouseCenter();
	}

	// Initialize key state table
	memset(keyInfo, 0, sizeof(keyInfo));
	memset(lastReadKeyState, 0, sizeof(lastReadKeyState));

#ifndef WIN32
#ifdef TELEMETRY
	if (s->_raceType == RM_TYPE_PRACTICE) {
		RtTelemInit(-10, 10);
		RtTelemNewChannel("Dist", &HCtx[idx]->distToStart, 0, 0);
		RtTelemNewChannel("Ax", &car->_accel_x, 0, 0);
		RtTelemNewChannel("Ay", &car->_accel_y, 0, 0);
		RtTelemNewChannel("Steer", &car->ctrl->steer, 0, 0);
		RtTelemNewChannel("Throttle", &car->ctrl->accelCmd, 0, 0);
		RtTelemNewChannel("Brake", &car->ctrl->brakeCmd, 0, 0);
		RtTelemNewChannel("Gear", &HCtx[idx]->gear, 0, 0);
		RtTelemNewChannel("Speed", &car->_speed_x, 0, 0);
	}
#endif
#endif

	const std::string traintype = GfParmGetStr(car->_carHandle, SECT_DRIVETRAIN, PRM_TYPE, VAL_TRANS_RWD);
	if (traintype == VAL_TRANS_RWD) {
		HCtx[idx]->driveTrain = eRWD;
	} else if (traintype == VAL_TRANS_FWD) {
		HCtx[idx]->driveTrain = eFWD;
	} else if (traintype == VAL_TRANS_4WD) {
		HCtx[idx]->driveTrain = e4WD;
	}//if traintype

	// Determine cluch mode : auto or "manual" (footual ;-?).
	tControlCmd	*cmd = HCtx[idx]->cmdControl;
	if (cmd[CMD_CLUTCH].type != GFCTRL_TYPE_JOY_AXIS && 
			cmd[CMD_CLUTCH].type != GFCTRL_TYPE_MOUSE_AXIS)
		HCtx[idx]->autoClutch = true;
	else
		HCtx[idx]->autoClutch = false;

	//GfOut("SteerCmd : Left : sens=%4.1f, spSens=%4.2f, deadZ=%4.2f\n",
	//	  cmd[CMD_LEFTSTEER].sens, cmd[CMD_LEFTSTEER].spdSens, cmd[CMD_LEFTSTEER].deadZone);
	//GfOut("SteerCmd : Right: sens=%4.1f, spSens=%4.2f, deadZ=%4.2f\n",
	//	  cmd[CMD_RIGHTSTEER].sens, cmd[CMD_RIGHTSTEER].spdSens, cmd[CMD_RIGHTSTEER].deadZone);

	// Setup Keyboard map (key code => index of the associated command in keyInfo / lastReadKeyState).
	for (int i = 0; i < NbCmdControl; i++)
	{
		if (cmd[i].type == GFCTRL_TYPE_KEYBOARD)
		{
			if (mapKeys.find(cmd[i].val) == mapKeys.end())
			{
				mapKeys[cmd[i].val] = keyIndex;
				keyIndex++;
			}
		}//KEYBOARD
		
	}//for i

}//newrace


static int
lookUpKeyMap(int key)
{
	const tKeyMap::const_iterator p = mapKeys.find(key);

	if (p != mapKeys.end())
		return p->second;

	return -1;
}

static void
updateKeys(void)
{
	int i;
	int nKeyInd;
	int idx;
	tControlCmd *cmd;

	for (idx = 0; idx < (int)HCtx.size(); idx++) {
		if (HCtx[idx]) {
			cmd = HCtx[idx]->cmdControl;
			for (i = 0; i < NbCmdControl; i++) {
				if (cmd[i].type == GFCTRL_TYPE_KEYBOARD) {
					nKeyInd = lookUpKeyMap(cmd[i].val);
					if (lastReadKeyState[nKeyInd] == GFUI_KEY_DOWN) {
						if (keyInfo[nKeyInd].state == GFUI_KEY_UP) {
							keyInfo[nKeyInd].edgeDn = 1;
						} else {
							keyInfo[nKeyInd].edgeDn = 0;
						}
					} else {
						if (keyInfo[nKeyInd].state == GFUI_KEY_DOWN) {
							keyInfo[nKeyInd].edgeUp = 1;
						} else {
							keyInfo[nKeyInd].edgeUp = 0;
						}
					}
					keyInfo[nKeyInd].state = lastReadKeyState[nKeyInd];
				}
			}
		}
	}
}//updateKeys


static int
onKeyAction(int key, int modifier, int state)
{
	// Update key state only if the key is assigned to a player command.
	const int nKeyInd = lookUpKeyMap(key);
	if (nKeyInd >= 0)
		lastReadKeyState[lookUpKeyMap(key)] = state;

	return 0;
}//onKeyAction


static void
common_drive(const int index, tCarElt* car, tSituation *s)
{
	tdble slip;
	tdble ax0;
	tdble brake;
	tdble clutch;
	tdble throttle;
	tdble leftSteer;
	tdble rightSteer;
	tdble newGlance;;
#if (BINCTRL_STEERING == JEPZ || BINCTRL_STEERING == JPM)
	tdble sensFrac, speedFrac;
#endif
	int scrw, scrh, dummy;
	
	const int idx = index - 1;
	tControlCmd *cmd = HCtx[idx]->cmdControl;
	static bool firstTime = true;

	if (!GfuiScreenIsActive(0) && firstTime)
	{
		if (HCtx[idx]->mouseControlUsed) {
			GfuiMouseShow();
			GfctrlMouseCenter();
			GfctrlMouseInitCenter();
		}
		GfuiKeyEventRegisterCurrent(onKeyAction);
		firstTime = false;
	}

	HCtx[idx]->distToStart = RtGetDistFromStart(car);
	HCtx[idx]->gear = (tdble)car->_gear;	/* telemetry */

	GfScrGetSize(&scrw, &scrh, &dummy, &dummy);

	memset(&(car->ctrl), 0, sizeof(tCarCtrl));

	car->_lightCmd = HCtx[idx]->lightCmd;

	if (car->_laps != HCtx[idx]->lastPitStopLap) {
		car->_raceCmd = RM_CMD_PIT_ASKED;
	}

	// Update the controls at most once per "robots time slice" (RCM_MAX_DT_ROBOTS s)
	// (i.e. keyboard/joystick/mouse values read for all players simultaneously).
	if (lastKeyUpdate != s->currentTime && index == ControlsUpdaterIndex) {
		updateKeys();

		if (joyPresent) {
			GfctrlJoyGetCurrent(joyInfo);
		}

		GfctrlMouseGetCurrent(mouseInfo);
		lastKeyUpdate = s->currentTime;
	}

	if ((cmd[CMD_ABS].type == GFCTRL_TYPE_JOY_BUT && joyInfo->edgeup[cmd[CMD_ABS].val])
	    || (cmd[CMD_ABS].type == GFCTRL_TYPE_MOUSE_BUT && mouseInfo->edgeup[cmd[CMD_ABS].val])
	    || (cmd[CMD_ABS].type == GFCTRL_TYPE_KEYBOARD && keyInfo[lookUpKeyMap(cmd[CMD_ABS].val)].edgeUp))
	{
		HCtx[idx]->paramAbs = !HCtx[idx]->paramAbs;
		sprintf(sstring, "%s/%s/%d", HM_SECT_PREF, HM_LIST_DRV, index);
		GfParmSetStr(PrefHdle, sstring, HM_ATT_ABS, Yn[!HCtx[idx]->paramAbs].c_str());
		GfParmWriteFile(NULL, PrefHdle, "Human");
	}

	if ((cmd[CMD_ASR].type == GFCTRL_TYPE_JOY_BUT && joyInfo->edgeup[cmd[CMD_ASR].val])
	    || (cmd[CMD_ASR].type == GFCTRL_TYPE_MOUSE_BUT && mouseInfo->edgeup[cmd[CMD_ASR].val])
	    || (cmd[CMD_ASR].type == GFCTRL_TYPE_KEYBOARD && keyInfo[lookUpKeyMap(cmd[CMD_ASR].val)].edgeUp))
	{
		HCtx[idx]->paramAsr = !HCtx[idx]->paramAsr;
		sprintf(sstring, "%s/%s/%d", HM_SECT_PREF, HM_LIST_DRV, index);
		GfParmSetStr(PrefHdle, sstring, HM_ATT_ASR, Yn[!HCtx[idx]->paramAsr].c_str());
		GfParmWriteFile(NULL, PrefHdle, "Human");
	}

	sprintf(car->_msgCmd[0], "%s %s", (HCtx[idx]->paramAbs ? "ABS" : ""), (HCtx[idx]->paramAsr ? "TCS" : ""));
	memcpy(car->_msgColorCmd, color, sizeof(car->_msgColorCmd));

	if ((cmd[CMD_SPDLIM].type == GFCTRL_TYPE_JOY_BUT && joyInfo->edgeup[cmd[CMD_SPDLIM].val])
	    || (cmd[CMD_SPDLIM].type == GFCTRL_TYPE_MOUSE_BUT && mouseInfo->edgeup[cmd[CMD_SPDLIM].val])
	    || (cmd[CMD_SPDLIM].type == GFCTRL_TYPE_KEYBOARD && keyInfo[lookUpKeyMap(cmd[CMD_SPDLIM].val)].edgeUp))
	{
		speedLimiter = !speedLimiter;
	}

	sprintf(car->_msgCmd[1], "Speed Limiter %s", (speedLimiter ? "On" : "Off"));

	if ((cmd[CMD_LIGHT1].type == GFCTRL_TYPE_JOY_BUT && joyInfo->edgeup[cmd[CMD_LIGHT1].val])
	    || (cmd[CMD_LIGHT1].type == GFCTRL_TYPE_MOUSE_BUT && mouseInfo->edgeup[cmd[CMD_LIGHT1].val])
	    || (cmd[CMD_LIGHT1].type == GFCTRL_TYPE_KEYBOARD && keyInfo[lookUpKeyMap(cmd[CMD_LIGHT1].val)].edgeUp))
	{
		if (HCtx[idx]->lightCmd & RM_LIGHT_HEAD1) {
			HCtx[idx]->lightCmd &= ~(RM_LIGHT_HEAD1 | RM_LIGHT_HEAD2);
		} else {
			HCtx[idx]->lightCmd |= RM_LIGHT_HEAD1 | RM_LIGHT_HEAD2;
		}
	}

	switch (cmd[CMD_LEFTSTEER].type) {
		case GFCTRL_TYPE_JOY_AXIS:
			ax0 = joyInfo->ax[cmd[CMD_LEFTSTEER].val] + cmd[CMD_LEFTSTEER].deadZone;
			if (ax0 > cmd[CMD_LEFTSTEER].max) {
				ax0 = cmd[CMD_LEFTSTEER].max;
			} else if (ax0 < cmd[CMD_LEFTSTEER].min) {
				ax0 = cmd[CMD_LEFTSTEER].min;
			}
			leftSteer = -SIGN(ax0) * cmd[CMD_LEFTSTEER].pow * pow(fabs(ax0), 1.0f / cmd[CMD_LEFTSTEER].sens) / (1.0 + cmd[CMD_LEFTSTEER].spdSens * car->_speed_x / 100.0);
			break;
		case GFCTRL_TYPE_MOUSE_AXIS:
			ax0 = mouseInfo->ax[cmd[CMD_LEFTSTEER].val] - cmd[CMD_LEFTSTEER].deadZone;
			if (ax0 > cmd[CMD_LEFTSTEER].max) {
				ax0 = cmd[CMD_LEFTSTEER].max;
			} else if (ax0 < cmd[CMD_LEFTSTEER].min) {
				ax0 = cmd[CMD_LEFTSTEER].min;
			}
			ax0 = ax0 * cmd[CMD_LEFTSTEER].pow;
			leftSteer = pow(fabs(ax0), 1.0f / cmd[CMD_LEFTSTEER].sens) / (1.0f + cmd[CMD_LEFTSTEER].spdSens * car->_speed_x / 1000.0);
			break;
		case GFCTRL_TYPE_KEYBOARD:
		case GFCTRL_TYPE_JOY_BUT:
		case GFCTRL_TYPE_MOUSE_BUT:
			if (cmd[CMD_LEFTSTEER].type == GFCTRL_TYPE_KEYBOARD) {
				ax0 = keyInfo[lookUpKeyMap(cmd[CMD_LEFTSTEER].val)].state;
			} else if (cmd[CMD_LEFTSTEER].type == GFCTRL_TYPE_MOUSE_BUT) {
				ax0 = mouseInfo->button[cmd[CMD_LEFTSTEER].val];
			} else {
				ax0 = joyInfo->levelup[cmd[CMD_LEFTSTEER].val];
			}
#if (BINCTRL_STEERING == JEPZ)
			if (ax0 == 0) {
				leftSteer = HCtx[idx]->prevLeftSteer - s->deltaTime * 5.0;
			} else {
				ax0 = 2 * ax0 - 1;
				sensFrac = 10.0 * cmd[CMD_LEFTSTEER].sens;
				speedFrac = fabs(car->_speed_x * cmd[CMD_LEFTSTEER].spdSens);
				if (speedFrac < 1.0) speedFrac = 1.0;
				leftSteer = HCtx[idx]->prevLeftSteer + s->deltaTime * ax0 * sensFrac / speedFrac;
			}
			if (leftSteer > 1.0) leftSteer = 1.0;
			if (leftSteer < 0.0) leftSteer = 0.0;
#elif (BINCTRL_STEERING == JPM)
			// ax should be 0 or 1 here (to be checked) => -1 (zero steer) or +1 (full steer left).
			ax0 = 2 * ax0 - 1;
			sensFrac = 1.0 + (3.5 - 1.5 * ax0) * cmd[CMD_LEFTSTEER].sens;
			speedFrac = 1.0 + car->_speed_x * car->_speed_x * cmd[CMD_LEFTSTEER].spdSens / 300.0;
			leftSteer = HCtx[idx]->prevLeftSteer + s->deltaTime * ax0 * sensFrac / speedFrac;
			//GfOut("Left : ax=%4.1f, ws=%4.2f, ss=%4.2f, prev=%5.2f, new=%5.2f (spd=%6.2f)\n",
			//	  ax0, sensFrac, speedFrac, HCtx[idx]->prevLeftSteer, leftSteer, car->_speed_x);
			if (leftSteer > 1.0)
				leftSteer = 1.0;
			else if (leftSteer < 0.0)
				leftSteer = 0.0;
#else
			if (ax0 == 0) {
				leftSteer = 0;
			} else {
				ax0 = 2 * ax0 - 1;
				leftSteer = HCtx[idx]->prevLeftSteer + ax0 * s->deltaTime / cmd[CMD_LEFTSTEER].sens / (1.0 + cmd[CMD_LEFTSTEER].spdSens * car->_speed_x / 1000.0);
				if (leftSteer > 1.0) leftSteer = 1.0;
				if (leftSteer < 0.0) leftSteer = 0.0;
			}
#endif			
			HCtx[idx]->prevLeftSteer = leftSteer;
			break;
		default:
			leftSteer = 0;
			break;
	}

	switch (cmd[CMD_RIGHTSTEER].type) {
		case GFCTRL_TYPE_JOY_AXIS:
			ax0 = joyInfo->ax[cmd[CMD_RIGHTSTEER].val] - cmd[CMD_RIGHTSTEER].deadZone;
			if (ax0 > cmd[CMD_RIGHTSTEER].max) {
				ax0 = cmd[CMD_RIGHTSTEER].max;
			} else if (ax0 < cmd[CMD_RIGHTSTEER].min) {
				ax0 = cmd[CMD_RIGHTSTEER].min;
			}
			rightSteer = -SIGN(ax0) * cmd[CMD_RIGHTSTEER].pow * pow(fabs(ax0), 1.0f / cmd[CMD_RIGHTSTEER].sens) / (1.0 + cmd[CMD_RIGHTSTEER].spdSens * car->_speed_x / 100.0);
			break;
		case GFCTRL_TYPE_MOUSE_AXIS:
			ax0 = mouseInfo->ax[cmd[CMD_RIGHTSTEER].val] - cmd[CMD_RIGHTSTEER].deadZone;
			if (ax0 > cmd[CMD_RIGHTSTEER].max) {
				ax0 = cmd[CMD_RIGHTSTEER].max;
			} else if (ax0 < cmd[CMD_RIGHTSTEER].min) {
				ax0 = cmd[CMD_RIGHTSTEER].min;
			}
			ax0 = ax0 * cmd[CMD_RIGHTSTEER].pow;
			rightSteer = - pow(fabs(ax0), 1.0f / cmd[CMD_RIGHTSTEER].sens) / (1.0f + cmd[CMD_RIGHTSTEER].spdSens * car->_speed_x / 1000.0);
			break;
		case GFCTRL_TYPE_KEYBOARD:
		case GFCTRL_TYPE_JOY_BUT:
		case GFCTRL_TYPE_MOUSE_BUT:
			if (cmd[CMD_RIGHTSTEER].type == GFCTRL_TYPE_KEYBOARD) {
				ax0 = keyInfo[lookUpKeyMap(cmd[CMD_RIGHTSTEER].val)].state;
			} else if (cmd[CMD_RIGHTSTEER].type == GFCTRL_TYPE_MOUSE_BUT) {
				ax0 = mouseInfo->button[cmd[CMD_RIGHTSTEER].val];
			} else {
				ax0 = joyInfo->levelup[cmd[CMD_RIGHTSTEER].val];
			}
#if (BINCTRL_STEERING == JEPZ)
			if (ax0 == 0) {
				rightSteer = HCtx[idx]->prevRightSteer + s->deltaTime * 5.0;
			} else {
				ax0 = 2 * ax0 - 1;
				sensFrac = 10.0 * cmd[CMD_RIGHTSTEER].sens;
				speedFrac = fabs(car->_speed_x * cmd[CMD_RIGHTSTEER].spdSens);
				if (speedFrac < 1.0) speedFrac = 1.0;
				rightSteer = HCtx[idx]->prevRightSteer - s->deltaTime * ax0 * sensFrac / speedFrac;
			}
			if (rightSteer > 0.0) rightSteer = 0.0;
			if (rightSteer < -1.0) rightSteer = -1.0;
#elif (BINCTRL_STEERING == JPM)
			// ax should be 0 or 1 here (to be checked) => -1 (zero steer) or +1 (full steer left).
			ax0 = 2 * ax0 - 1;
			sensFrac = 1.0 + (3.5 - 1.5 * ax0) * cmd[CMD_RIGHTSTEER].sens;
			speedFrac = 1.0 + car->_speed_x * car->_speed_x * cmd[CMD_RIGHTSTEER].spdSens / 300.0;
			rightSteer = HCtx[idx]->prevRightSteer - s->deltaTime * ax0 * sensFrac / speedFrac;
			//GfOut("Right: ax=%4.1f, ws=%4.2f, ss=%4.2f, prev=%5.2f, new=%5.2f (spd=%6.2f)\n",
			//	  ax0, sensFrac, speedFrac, HCtx[idx]->prevRightSteer, rightSteer, car->_speed_x);
			if (rightSteer < -1.0)
				rightSteer = -1.0;
			else if (rightSteer > 0.0)
				rightSteer = 0.0;
#else
			if (ax0 == 0) {
				rightSteer = 0;
			} else {
				ax0 = 2 * ax0 - 1;
				rightSteer = HCtx[idx]->prevRightSteer - ax0 * s->deltaTime / cmd[CMD_RIGHTSTEER].sens / (1.0 + cmd[CMD_RIGHTSTEER].spdSens * car->_speed_x / 1000.0);
				if (rightSteer > 0.0) rightSteer = 0.0;
				if (rightSteer < -1.0) rightSteer = -1.0;
			}
#endif
			HCtx[idx]->prevRightSteer = rightSteer;
			break;
		default:
			rightSteer = 0;
			break;
	}

	car->_steerCmd = leftSteer + rightSteer;

#define GLANCERATE 12 	// speed at which the driver turns his head, radians per sec -> ~1/3s to full glance
	newGlance = car->_glance;

	if ((cmd[CMD_LEFTGLANCE].type == GFCTRL_TYPE_JOY_BUT && joyInfo->levelup[cmd[CMD_LEFTGLANCE].val])
	    || (cmd[CMD_LEFTGLANCE].type == GFCTRL_TYPE_MOUSE_BUT && mouseInfo->button[cmd[CMD_LEFTGLANCE].val])
	    || (cmd[CMD_LEFTGLANCE].type == GFCTRL_TYPE_KEYBOARD && keyInfo[lookUpKeyMap(cmd[CMD_LEFTGLANCE].val)].state))
	{
		newGlance = newGlance - GLANCERATE * s->deltaTime;
	} else if ((cmd[CMD_RIGHTGLANCE].type == GFCTRL_TYPE_JOY_BUT && joyInfo->levelup[cmd[CMD_RIGHTGLANCE].val])
	    || (cmd[CMD_RIGHTGLANCE].type == GFCTRL_TYPE_MOUSE_BUT && mouseInfo->button[cmd[CMD_RIGHTGLANCE].val])
	    || (cmd[CMD_RIGHTGLANCE].type == GFCTRL_TYPE_KEYBOARD && keyInfo[lookUpKeyMap(cmd[CMD_RIGHTGLANCE].val)].state))
	{
		newGlance = newGlance + GLANCERATE * s->deltaTime;
	} else {
		// return view to center
		if (newGlance > 0) {
			newGlance = newGlance - GLANCERATE * s->deltaTime;
			if (newGlance < 0) newGlance = 0;
		}
		if (newGlance < 0) {
			newGlance = newGlance + GLANCERATE * s->deltaTime;
			if (newGlance > 0) newGlance = 0;
		}
	}

	// limit glance to 120 degrees either way
	if (newGlance > 2*PI/3) newGlance=2*PI/3;
	if (newGlance < -2*PI/3) newGlance=-2*PI/3;

	car->_glance = newGlance;

	switch (cmd[CMD_BRAKE].type) {
		case GFCTRL_TYPE_JOY_AXIS:
			brake = joyInfo->ax[cmd[CMD_BRAKE].val];
			if (brake > cmd[CMD_BRAKE].max) {
				brake = cmd[CMD_BRAKE].max;
			} else if (brake < cmd[CMD_BRAKE].min) {
				brake = cmd[CMD_BRAKE].min;
			}
			car->_brakeCmd = fabs(cmd[CMD_BRAKE].pow *
						pow(fabs((brake - cmd[CMD_BRAKE].minVal) /
							(cmd[CMD_BRAKE].max - cmd[CMD_BRAKE].min)),
						1.0f / cmd[CMD_BRAKE].sens));
			break;
		case GFCTRL_TYPE_MOUSE_AXIS:
			ax0 = mouseInfo->ax[cmd[CMD_BRAKE].val] - cmd[CMD_BRAKE].deadZone;
			if (ax0 > cmd[CMD_BRAKE].max) {
				ax0 = cmd[CMD_BRAKE].max;
			} else if (ax0 < cmd[CMD_BRAKE].min) {
				ax0 = cmd[CMD_BRAKE].min;
			}
			ax0 = ax0 * cmd[CMD_BRAKE].pow;
			car->_brakeCmd =  pow(fabs(ax0), 1.0f / cmd[CMD_BRAKE].sens) / (1.0 + cmd[CMD_BRAKE].spdSens * car->_speed_x / 1000.0);
			break;
		case GFCTRL_TYPE_JOY_BUT:
			car->_brakeCmd = joyInfo->levelup[cmd[CMD_BRAKE].val];
			break;
		case GFCTRL_TYPE_MOUSE_BUT:
			car->_brakeCmd = mouseInfo->button[cmd[CMD_BRAKE].val];
			break;
		case GFCTRL_TYPE_KEYBOARD:
			car->_brakeCmd = keyInfo[lookUpKeyMap(cmd[CMD_BRAKE].val)].state;
			break;
		default:
			car->_brakeCmd = 0;
			break;
	}

	switch (cmd[CMD_CLUTCH].type) {
		case GFCTRL_TYPE_JOY_AXIS:
			clutch = joyInfo->ax[cmd[CMD_CLUTCH].val];
			if (clutch > cmd[CMD_CLUTCH].max) {
				clutch = cmd[CMD_CLUTCH].max;
			} else if (clutch < cmd[CMD_CLUTCH].min) {
				clutch = cmd[CMD_CLUTCH].min;
			}
			car->_clutchCmd = fabs(cmd[CMD_CLUTCH].pow *
						pow(fabs((clutch - cmd[CMD_CLUTCH].minVal) /
							(cmd[CMD_CLUTCH].max - cmd[CMD_CLUTCH].min)),
						1.0f / cmd[CMD_CLUTCH].sens));
			break;
		case GFCTRL_TYPE_MOUSE_AXIS:
			ax0 = mouseInfo->ax[cmd[CMD_CLUTCH].val] - cmd[CMD_CLUTCH].deadZone;
			if (ax0 > cmd[CMD_CLUTCH].max) {
				ax0 = cmd[CMD_CLUTCH].max;
			} else if (ax0 < cmd[CMD_CLUTCH].min) {
				ax0 = cmd[CMD_CLUTCH].min;
			}
			ax0 = ax0 * cmd[CMD_CLUTCH].pow;
			car->_clutchCmd =  pow(fabs(ax0), 1.0f / cmd[CMD_CLUTCH].sens) / (1.0 + cmd[CMD_CLUTCH].spdSens * car->_speed_x / 1000.0);
			break;
		case GFCTRL_TYPE_JOY_BUT:
			car->_clutchCmd = joyInfo->levelup[cmd[CMD_CLUTCH].val];
			break;
		case GFCTRL_TYPE_MOUSE_BUT:
			car->_clutchCmd = mouseInfo->button[cmd[CMD_CLUTCH].val];
			break;
		case GFCTRL_TYPE_KEYBOARD:
			car->_clutchCmd = keyInfo[lookUpKeyMap(cmd[CMD_CLUTCH].val)].state;
			break;
		default:
			car->_clutchCmd = 0;
			break;
	}

	// if player's used the clutch manually then we dispense with autoClutch
	if (car->_clutchCmd != 0.0f)
		HCtx[idx]->autoClutch = false;

	// Ebrake here so that it can override the clutch control
	if ((cmd[CMD_EBRAKE].type == GFCTRL_TYPE_JOY_BUT && joyInfo->levelup[cmd[CMD_EBRAKE].val])
	    || (cmd[CMD_EBRAKE].type == GFCTRL_TYPE_MOUSE_BUT && mouseInfo->button[cmd[CMD_EBRAKE].val])
	    || (cmd[CMD_EBRAKE].type == GFCTRL_TYPE_KEYBOARD && keyInfo[lookUpKeyMap(cmd[CMD_EBRAKE].val)].state == GFUI_KEY_DOWN))
	{
		car->_ebrakeCmd = 1;
		if (HCtx[idx]->autoClutch)
			car->_clutchCmd = 1;
	} else {
		car->_ebrakeCmd = 0;
	}

	switch (cmd[CMD_THROTTLE].type) {
		case GFCTRL_TYPE_JOY_AXIS:
			throttle = joyInfo->ax[cmd[CMD_THROTTLE].val];
			if (throttle > cmd[CMD_THROTTLE].max) {
				throttle = cmd[CMD_THROTTLE].max;
			} else if (throttle < cmd[CMD_THROTTLE].min) {
				throttle = cmd[CMD_THROTTLE].min;
			}
			car->_accelCmd = fabs(cmd[CMD_THROTTLE].pow *
						pow(fabs((throttle - cmd[CMD_THROTTLE].minVal) /
								(cmd[CMD_THROTTLE].max - cmd[CMD_THROTTLE].min)),
							1.0f / cmd[CMD_THROTTLE].sens));
			break;
		case GFCTRL_TYPE_MOUSE_AXIS:
			ax0 = mouseInfo->ax[cmd[CMD_THROTTLE].val] - cmd[CMD_THROTTLE].deadZone;
			if (ax0 > cmd[CMD_THROTTLE].max) {
				ax0 = cmd[CMD_THROTTLE].max;
			} else if (ax0 < cmd[CMD_THROTTLE].min) {
				ax0 = cmd[CMD_THROTTLE].min;
			}
			ax0 = ax0 * cmd[CMD_THROTTLE].pow;
			car->_accelCmd =  pow(fabs(ax0), 1.0f / cmd[CMD_THROTTLE].sens) / (1.0 + cmd[CMD_THROTTLE].spdSens * car->_speed_x / 1000.0);
			if (isnan (car->_accelCmd)) {
				car->_accelCmd = 0;
			}
			/* printf("  axO:%f  accelCmd:%f\n", ax0, car->_accelCmd); */
			break;
		case GFCTRL_TYPE_JOY_BUT:
			car->_accelCmd = joyInfo->levelup[cmd[CMD_THROTTLE].val];
			break;
		case GFCTRL_TYPE_MOUSE_BUT:
			car->_accelCmd = mouseInfo->button[cmd[CMD_THROTTLE].val];
			break;
		case GFCTRL_TYPE_KEYBOARD:
			car->_accelCmd = keyInfo[lookUpKeyMap(cmd[CMD_THROTTLE].val)].state;
			break;
		default:
			car->_accelCmd = 0;
			break;
	}

	// thanks Christos for the following: gradual accel/brake changes for on/off controls.
	if (cmd[CMD_BRAKE].type == GFCTRL_TYPE_JOY_BUT
		|| cmd[CMD_BRAKE].type == GFCTRL_TYPE_MOUSE_BUT
		|| cmd[CMD_BRAKE].type == GFCTRL_TYPE_KEYBOARD)
	{
		if (s->currentTime > 1.0)
		{
			static const tdble inc_rate = 0.2f;
		
			tdble d_brake = car->_brakeCmd - HCtx[idx]->pbrake;
			if (fabs(d_brake) > inc_rate && car->_brakeCmd > HCtx[idx]->pbrake)
				car->_brakeCmd =
					MIN(car->_brakeCmd, HCtx[idx]->pbrake + inc_rate*d_brake/fabs(d_brake));
		}
		HCtx[idx]->pbrake = car->_brakeCmd;
	}
	
	if (cmd[CMD_THROTTLE].type == GFCTRL_TYPE_JOY_BUT
		|| cmd[CMD_THROTTLE].type == GFCTRL_TYPE_MOUSE_BUT
		|| cmd[CMD_THROTTLE].type == GFCTRL_TYPE_KEYBOARD)
	{
		if (s->currentTime > 1.0)
		{
			static const tdble inc_rate = 0.2f;
			
			tdble d_accel = car->_accelCmd - HCtx[idx]->paccel;
			if (fabs(d_accel) > inc_rate && car->_accelCmd > HCtx[idx]->paccel)
				car->_accelCmd =
					MIN(car->_accelCmd, HCtx[idx]->paccel + inc_rate*d_accel/fabs(d_accel));
		}
		HCtx[idx]->paccel = car->_accelCmd;
	}

	if (HCtx[idx]->autoReverseEngaged) {
		/* swap brake and throttle */
		brake = car->_brakeCmd;
		car->_brakeCmd = car->_accelCmd;
		car->_accelCmd = brake;
	}

	if (HCtx[idx]->paramAbs) 
	{
		if (fabs(car->_speed_x) > 10.0 && car->_brakeCmd > 0.0)
		{
			tdble brake1 = car->_brakeCmd, brake2 = car->_brakeCmd, brake3 = car->_brakeCmd;
			//tdble rearskid = MAX(0.0, MAX(car->_skid[2], car->_skid[3]) - MAX(car->_skid[0], car->_skid[1]));
			int i;

			// reduce brake if car sliding sideways
			tdble skidAng = atan2(car->_speed_Y, car->_speed_X) - car->_yaw;
			NORM_PI_PI(skidAng);

			if (car->_speed_x > 5 && fabs(skidAng) > 0.2)
				brake1 = MIN(car->_brakeCmd, 0.10 + 0.70 * cos(skidAng));

#if 0
			// reduce brake if car steering sharply
			if (fabs(car->_steerCmd) > 0.1)
			{
				tdble decel = ((fabs(car->_steerCmd)-0.1) * (1.0 + fabs(car->_steerCmd)) * 0.2);
				brake2 = MIN(car->_brakeCmd, MAX(0.35, 1.0 - decel));
			}
#endif

			const tdble abs_slip = 1.0;
			const tdble abs_range = 9.0;

			// reduce brake if wheels are slipping
			slip = 0;
			for (i = 0; i < 4; i++) {
				slip = MAX(slip, car->_speed_x - (car->_wheelSpinVel(i) * car->_wheelRadius(i)));
			}

			if (slip > abs_slip)
				brake3 = MAX(MIN(0.35, car->_brakeCmd), car->_brakeCmd - MIN(car->_brakeCmd*0.8, (slip - abs_slip) / abs_range));

			car->_brakeCmd = MIN(brake1, MIN(brake2, brake3));
		}
	}


	if (HCtx[idx]->paramAsr) 
	{
		tdble origaccel = car->_accelCmd;

		tdble drivespeed = 0.0;
		switch (HCtx[idx]->driveTrain)
		{
			case e4WD:
				drivespeed = ((car->_wheelSpinVel(FRNT_RGT) + car->_wheelSpinVel(FRNT_LFT)) *
				              car->_wheelRadius(FRNT_LFT) +
				              (car->_wheelSpinVel(REAR_RGT) + car->_wheelSpinVel(REAR_LFT)) *
				              car->_wheelRadius(REAR_LFT)) / 4.0; 
				break;
			case eFWD:
				drivespeed = (car->_wheelSpinVel(FRNT_RGT) + car->_wheelSpinVel(FRNT_LFT)) *
				              car->_wheelRadius(FRNT_LFT) / 2.0;
				break;
			case eRWD:
				// ADJUSTMENTS TO RWD Asr:-
				// Originally this purely returned the speed of the wheels, which when the speed of
				// the car is subtracted below provides the degree of slip, which is then used to
				// reduce the amount of accelerator.
				//
				// The new calculation below reduces the degree to which the difference between wheel
				// and car speed affects slip, and instead looks at the SlipAccel and SlipSide values,
				// which are more important as they signify an impending loss of control.  The SlipSide
				// value is reduced the faster the car's travelling, as usually it matters most in the
				// low to mid speed ranges.  We also take into account the difference between the
				// player's steer command and the actual yaw rate of the vehicle - where the player
				// is steering against the yaw rate, we decrease the amount of acceleration to stop
				// tirespin sending the rear wheels into a spinout.

				tdble friction = MIN(car->_wheelSeg(REAR_RGT)->surface->kFriction, car->_wheelSeg(REAR_LFT)->surface->kFriction) - 0.2;
				if (friction < 1.0) friction *= MAX(0.6, friction);

				bool  steer_correct = (fabs(car->_yaw_rate) > fabs(car->_steerCmd * MAX(4.0, car->_speed_x/12.0) * friction) ||
				                       (car->_yaw_rate < 0.0 && car->_steerCmd > 0.0) ||
				                       (car->_yaw_rate > 0.0 && car->_steerCmd < 0.0));
				tdble steer_diff    = fabs(car->_yaw_rate - car->_steerCmd);

				tdble slipf = (steer_correct ? 8 * friction : 15 * friction);

				drivespeed = (((car->_wheelSpinVel(REAR_RGT) + car->_wheelSpinVel(REAR_LFT)) - (20 * friction)) *
				              car->_wheelRadius(REAR_LFT) +
				              (steer_correct ? (steer_diff * fabs(car->_yaw_rate) * (8 / friction)) : 0.0) +
				              MAX(0.0, (-(car->_wheelSlipAccel(REAR_RGT)) - friction)) +
				              MAX(0.0, (-(car->_wheelSlipAccel(REAR_LFT)) - friction)) +
				              fabs(car->_wheelSlipSide(REAR_RGT) * MAX(4, 80-fabs(car->_speed_x))/slipf) +
				              fabs(car->_wheelSlipSide(REAR_LFT) * MAX(4, 80-fabs(car->_speed_x))/slipf))
				             / 2.0;
				break;
		}

		tdble slip = drivespeed - fabs(car->_speed_x);
		if (slip > 2.5)
			car->_accelCmd = MIN(car->_accelCmd, origaccel - MIN(origaccel-0.2, ((slip - 2.5)/20.0)));
	}

	if (speedLimiter) {
		if (speedLimit != 0) {
			tdble dv = speedLimit - car->_speed_x;
			if (dv > 0.0) {
				car->_accelCmd = MIN(car->_accelCmd, fabs(dv/6.0));
			} else {
				car->_brakeCmd = MAX(car->_brakeCmd, fabs(dv/5.0));
				car->_accelCmd = 0;
			}//if-else dv
		}//if speedLimit
	}//if speedLimiter


#ifndef WIN32
#ifdef TELEMETRY
	if ((car->_laps > 1) && (car->_laps < 5)) {
		if (HCtx[idx]->lap == 1) {
			RtTelemStartMonitoring("Player");
		}
		RtTelemUpdate(car->_curLapTime);
	}
	if (car->_laps == 5) {
		if (HCtx[idx]->lap == 4) {
			RtTelemShutdown();
		}
	}
#endif
#endif

	HCtx[idx]->lap = car->_laps;
}//common_drive


static tdble
getAutoClutch(const int idx, int gear, int newGear, tCarElt *car)
{
	tdble ret = 0.0f;
	
	if (newGear != 0 && newGear < car->_gearNb) {
		if (newGear != gear)
			HCtx[idx]->clutchtime = 0.332f - ((tdble) newGear / 65.0f);

		if (HCtx[idx]->clutchtime > 0.0f)
			HCtx[idx]->clutchtime -= RCM_MAX_DT_ROBOTS;
			
		ret = 2.0f * HCtx[idx]->clutchtime;
	}//if newGear
	
	return ret;
}//getAutoClutch


/*
 * Function
 *
 *
 * Description
 *
 *
 * Parameters
 *
 *
 * Return
 *
 *
 * Remarks
 *	
 */
static void
drive_mt(int index, tCarElt* car, tSituation *s)
{
	const int idx = index - 1;

	tControlCmd *cmd = HCtx[idx]->cmdControl;

	common_drive(index, car, s);

	car->_gearCmd = car->_gear;
	/* manual shift sequential */
	if (HCtx[idx]->transmission == eTransSeq)
	{
		/* Up shifting command */
		if ((cmd[CMD_UP_SHFT].type == GFCTRL_TYPE_JOY_BUT && joyInfo->edgeup[cmd[CMD_UP_SHFT].val])
		    || (cmd[CMD_UP_SHFT].type == GFCTRL_TYPE_MOUSE_BUT && mouseInfo->edgeup[cmd[CMD_UP_SHFT].val])
		    || (cmd[CMD_UP_SHFT].type == GFCTRL_TYPE_KEYBOARD && keyInfo[lookUpKeyMap(cmd[CMD_UP_SHFT].val)].edgeUp))
		{
			if (HCtx[idx]->seqShftAllowNeutral || car->_gearCmd > -1)
				car->_gearCmd++;
		}

		/* Down shifting command */
		if ((cmd[CMD_DN_SHFT].type == GFCTRL_TYPE_JOY_BUT && joyInfo->edgeup[cmd[CMD_DN_SHFT].val])
		    || (cmd[CMD_DN_SHFT].type == GFCTRL_TYPE_MOUSE_BUT && mouseInfo->edgeup[cmd[CMD_DN_SHFT].val])
		    || (cmd[CMD_DN_SHFT].type == GFCTRL_TYPE_KEYBOARD && keyInfo[lookUpKeyMap(cmd[CMD_DN_SHFT].val)].edgeUp))
		{
			if (HCtx[idx]->seqShftAllowNeutral || car->_gearCmd > 1)
				car->_gearCmd--;
		}

		/* Neutral gear command */
		if ((cmd[CMD_GEAR_N].type == GFCTRL_TYPE_JOY_BUT && joyInfo->edgeup[cmd[CMD_GEAR_N].val])
		    || (cmd[CMD_GEAR_N].type == GFCTRL_TYPE_MOUSE_BUT && mouseInfo->edgeup[cmd[CMD_GEAR_N].val])
		    || (cmd[CMD_GEAR_N].type == GFCTRL_TYPE_KEYBOARD && keyInfo[lookUpKeyMap(cmd[CMD_GEAR_N].val)].edgeUp))
		{
			car->_gearCmd = 0;
		}

		/* Reverse gear command */
		if ((cmd[CMD_GEAR_R].type == GFCTRL_TYPE_JOY_BUT && joyInfo->edgeup[cmd[CMD_GEAR_R].val])
		    || (cmd[CMD_GEAR_R].type == GFCTRL_TYPE_MOUSE_BUT && mouseInfo->edgeup[cmd[CMD_GEAR_R].val])
		    || (cmd[CMD_GEAR_R].type == GFCTRL_TYPE_KEYBOARD && keyInfo[lookUpKeyMap(cmd[CMD_GEAR_R].val)].edgeUp))
		{
			/* Only allow Reverse to be selected at low speed (~40kmph) or from neutral */
			if (car->_speed_x < 10 || car->_gear == 0) 
				car->_gearCmd = -1;
		}
	}

	/* manual shift direct (button for each gear) */
	else if (HCtx[idx]->transmission == eTransGrid)
	{
		/* Go to neutral gear if any gear command released (edge down) */
		if (HCtx[idx]->relButNeutral) {
			for (int i = CMD_GEAR_R; i <= CMD_GEAR_6; i++) {
				if ((cmd[i].type == GFCTRL_TYPE_JOY_BUT && joyInfo->edgedn[cmd[i].val])
				    || (cmd[i].type == GFCTRL_TYPE_MOUSE_BUT && mouseInfo->edgedn[cmd[i].val])
				    || (cmd[i].type == GFCTRL_TYPE_KEYBOARD && keyInfo[lookUpKeyMap(cmd[i].val)].edgeDn))
				{
					car->_gearCmd = 0;
				}
			}
		}

		/* Select the right gear if any gear command activated (edge up) */
		for (int i = CMD_GEAR_R; i <= CMD_GEAR_6; i++) {
			if ((cmd[i].type == GFCTRL_TYPE_JOY_BUT && joyInfo->edgeup[cmd[i].val])
			    || (cmd[i].type == GFCTRL_TYPE_MOUSE_BUT && mouseInfo->edgeup[cmd[i].val])
			    || (cmd[i].type == GFCTRL_TYPE_KEYBOARD && keyInfo[lookUpKeyMap(cmd[i].val)].edgeUp))
			{
				car->_gearCmd = i - CMD_GEAR_N;
			}
		}
	}

	/* H-Box selector using XY axis of joy/thumbstick */
	else if (HCtx[idx]->transmission == eTransHbox)
	{
		// Used to test bitfield of allowable changes
		int hboxGearTest = 1 << (car->_gear + 1);
		float ax0, ay0;

		ax0 = joyInfo->ax[cmd[CMD_HBOX_X].val];
		ay0 = joyInfo->ax[cmd[CMD_HBOX_Y].val];

		if (ax0 > 0.33) {
			if (ay0 < -0.66 && hboxChanges[5] & hboxGearTest)
				car->_gearCmd = 5;

			if (car->_speed_x < 10) {
				/* 'R' Only selectable at low speed */
				if (ay0 > 0.66 && hboxChanges[0] & hboxGearTest)
					car->_gearCmd = -1;
			} else {
				/* '6' Only selectable at high speed */
				if (ay0 > 0.66 && hboxChanges[6] & hboxGearTest)
					car->_gearCmd = 6;
			}
		} else if (ax0 < -0.33) {
			if (ay0 < -0.66 && hboxChanges[1] & hboxGearTest)
				car->_gearCmd = 1;
			if (ay0 > 0.66 && hboxChanges[2] & hboxGearTest)
				car->_gearCmd = 2;
		} else {
			if (ay0 < -0.66 && hboxChanges[3] & hboxGearTest)
				car->_gearCmd = 3;
			if (ay0 > 0.66 && hboxChanges[4] & hboxGearTest)
				car->_gearCmd = 4;
		}

		/* 'N' selectable from any gear */
		if (ay0 < 0.33 && ay0 > -0.33 && ax0 > -0.5 && ax0 < -0.33)
			car->_gearCmd = 0;
		/* Extended 'N' area when using clutch to allow 'jumping' gears */
		if (ay0 < 0.33 && ay0 > -0.33 && ax0 > -0.5 && ax0 < 0.5 && HCtx[idx]->autoClutch == 0)
			car->_gearCmd = 0;
	}

	if (HCtx[idx]->autoClutch && car->_clutchCmd == 0.0f)
		car->_clutchCmd = getAutoClutch(idx, car->_gear, car->_gearCmd, car);
}//drive_mt


/*
 * Function
 *
 *
 * Description
 *
 *
 * Parameters
 *
 *
 * Return
 *	
 *
 * Remarks
 *	
 */
static void
drive_at(int index, tCarElt* car, tSituation *s)
{
	const int idx = index - 1;

	tControlCmd *cmd = HCtx[idx]->cmdControl;

	common_drive(index, car, s);

	/* shift */
	int gear = car->_gear;
	if (gear > 0)	//return to auto-shift
		HCtx[idx]->manual = false;
	gear += car->_gearOffset;
	car->_gearCmd = car->_gear;

    if (!HCtx[idx]->autoReverse) {
		/* manual shift */
		if ((cmd[CMD_UP_SHFT].type == GFCTRL_TYPE_JOY_BUT && joyInfo->edgeup[cmd[CMD_UP_SHFT].val])
		    || (cmd[CMD_UP_SHFT].type == GFCTRL_TYPE_MOUSE_BUT && mouseInfo->edgeup[cmd[CMD_UP_SHFT].val])
		    || (cmd[CMD_UP_SHFT].type == GFCTRL_TYPE_KEYBOARD && keyInfo[lookUpKeyMap(cmd[CMD_UP_SHFT].val)].edgeUp))
		{
			car->_gearCmd++;
			HCtx[idx]->manual = true;
		}//CMD_UP_SHFT

		if ((cmd[CMD_DN_SHFT].type == GFCTRL_TYPE_JOY_BUT && joyInfo->edgeup[cmd[CMD_DN_SHFT].val])
		    || (cmd[CMD_DN_SHFT].type == GFCTRL_TYPE_MOUSE_BUT && mouseInfo->edgeup[cmd[CMD_DN_SHFT].val])
		    || (cmd[CMD_DN_SHFT].type == GFCTRL_TYPE_KEYBOARD && keyInfo[lookUpKeyMap(cmd[CMD_DN_SHFT].val)].edgeUp))
		{
			car->_gearCmd--;
			HCtx[idx]->manual = true;
		}//CMD_DN_SHFT

		/* manual shift direct */
		if (HCtx[idx]->relButNeutral) {
			for (int i = CMD_GEAR_R; i < CMD_GEAR_2; i++) {
				if ((cmd[i].type == GFCTRL_TYPE_JOY_BUT && joyInfo->edgedn[cmd[i].val])
				    || (cmd[i].type == GFCTRL_TYPE_MOUSE_BUT && mouseInfo->edgedn[cmd[i].val])
				    || (cmd[i].type == GFCTRL_TYPE_KEYBOARD && keyInfo[lookUpKeyMap(cmd[i].val)].edgeDn))
				{
					car->_gearCmd = 0;
					HCtx[idx]->manual = false;	//return to auto-shift
				}//
			}//for i
		}//relButNeutral

		for (int i = CMD_GEAR_R; i < CMD_GEAR_2; i++) {
			if ((cmd[i].type == GFCTRL_TYPE_JOY_BUT && joyInfo->edgeup[cmd[i].val])
			    || (cmd[i].type == GFCTRL_TYPE_MOUSE_BUT && mouseInfo->edgeup[cmd[i].val])
			    || (cmd[i].type == GFCTRL_TYPE_KEYBOARD && keyInfo[lookUpKeyMap(cmd[i].val)].edgeUp))
			{
				car->_gearCmd = i - CMD_GEAR_N;
				HCtx[idx]->manual = false;
    }
		}//for i
	}//if !autoReverse

	/* auto shift */
	if (!HCtx[idx]->manual && !HCtx[idx]->autoReverseEngaged) {
		if (car->_speed_x > HCtx[idx]->shiftThld[gear]) {
			car->_gearCmd++;
		} else if ((car->_gearCmd > 1) && (car->_speed_x < (HCtx[idx]->shiftThld[gear-1] - 4.0))) {
			car->_gearCmd--;
		}
		if (car->_gearCmd <= 0)
			car->_gearCmd++;
	}//if !manual && !autoReverse

	/* Automatic Reverse Gear Mode */
	if (HCtx[idx]->autoReverse) {
		if (!HCtx[idx]->autoReverseEngaged) {	//currently not in autoReverse
			if ((car->_brakeCmd > car->_accelCmd) && (car->_speed_x < 1.0)) {
				HCtx[idx]->autoReverseEngaged = true;
				car->_gearCmd = CMD_GEAR_R - CMD_GEAR_N;
			}
		} else {	//currently in autoreverse mode
			if ((car->_brakeCmd > car->_accelCmd) && (car->_speed_x > -1.0) && (car->_speed_x < 1.0)) {
				HCtx[idx]->autoReverseEngaged = false;
				car->_gearCmd = CMD_GEAR_1 - CMD_GEAR_N;
			} else {
				car->_gearCmd = CMD_GEAR_R - CMD_GEAR_N;
			}
		}//if-else autoReverseEngaged
	}//if autoReverse

	/* Automatic clutch mode */
	if (HCtx[idx]->autoClutch && car->_clutchCmd == 0.0f)
	  car->_clutchCmd = getAutoClutch(idx, car->_gear, car->_gearCmd, car);
}//drive_at


static int
pitcmd(int index, tCarElt* car, tSituation *s)
{
	const int idx = index - 1;

	HCtx[idx]->nbPitStops++;  //Yet another pitstop
	tdble curr_fuel = car->_tank - car->_fuel;  //Can receive max. this fuel

	tdble planned_stops = 1.0
      + MAX(HCtx[idx]->nbPitStopProg - HCtx[idx]->nbPitStops, 0);  //Planned pitstops still ahead
	
  //Need this amount of extra fuel to finish the race
  tdble fuel = ( MaxFuelPerMeter
      * (curTrack->length * car->_remainingLaps + car->_trkPos.seg->lgfromstart)
      + 2.7f / 60.0f * MAX(s->_totTime, 0) )
    / planned_stops
    - car->_fuel;

  //No need to check for limits as curr_fuel cannot be bigger
  //than the tank capacity
	car->_pitFuel = MAX(MIN(curr_fuel, fuel), 0);

	HCtx[idx]->lastPitStopLap = car->_laps;

	car->_pitRepair = (int)car->_dammage;

	if (HCtx[idx]) {
		const tControlCmd *cmd = HCtx[idx]->cmdControl;
		for (int i = 0; i < NbCmdControl; i++) {
			if (cmd[i].type == GFCTRL_TYPE_KEYBOARD) {
				const int key = lookUpKeyMap(cmd[i].val);
				keyInfo[key].state = GFUI_KEY_UP;
				keyInfo[key].edgeDn = 0;
				keyInfo[key].edgeUp = 0;
				lastReadKeyState[key] = GFUI_KEY_UP;
			}
		}//for i
	}//if HCtx

	return ROB_PIT_MENU; /* The player is able to modify the value by menu */
}//pitcmd


// Trivial strategy:
// fill in as much fuel as required for the whole race,
// or if the tank is too small, fill the tank completely.
static void SetFuelAtRaceStart(tTrack* track, void **carParmHandle,
                                tSituation *s, int idx) {
  tdble fuel_requested;
  const tdble initial_fuel = GfParmGetNum(*carParmHandle, SECT_CAR,
                                            PRM_FUEL, NULL, 0.0f);

  if (initial_fuel) {
    // If starting fuel is set up explicitely,
    // no use computing anything...
    fuel_requested = initial_fuel;
  } else {
    // We must load and calculate parameters.
    tdble fuel_per_lap = track->length * MaxFuelPerMeter;
    tdble fuel_for_race = fuel_per_lap * (s->_totLaps + 1.0f);
    // aimed at timed sessions:
    fuel_for_race +=  fuel_per_lap / 60.0 * MAX(s->_totTime, 0);
    // divide qty by planned pitstops:
    fuel_for_race /= (1.0 + ((tdble)HCtx[idx]->nbPitStopProg));
    // add some reserve:
    //fuel_for_race += FuelReserve;

    const tdble tank_capacity = GfParmGetNum(*carParmHandle, SECT_CAR,
                                              PRM_TANK, NULL, 100.0f);
    fuel_requested = MIN(fuel_for_race, tank_capacity);
  }

  GfParmSetNum(*carParmHandle, SECT_CAR, PRM_FUEL, NULL, fuel_requested);
}  // SetFuelAtRaceStart
