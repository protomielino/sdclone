/***************************************************************************

    file                 : raceman.h
    created              : Sun Jan 30 22:59:17 CET 2000
    copyright            : (C) 2000,2002 by Eric Espie
    email                : torcs@free.fr
    version              : $Id: raceman.h,v 1.28 2006/02/20 20:17:43 berniw Exp $

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
    		This is the race information structures.
    @author	<a href=mailto:torcs@free.fr>Eric Espie</a>
    @version	$Id: raceman.h,v 1.28 2006/02/20 20:17:43 berniw Exp $
    @ingroup	raceinfo
*/
 
#ifndef _RACEMANV1_H_
#define _RACEMANV1_H_

#include <tgf.h>
#include <car.h>
#include <track.h>
#include <graphic.h>
#include <simu.h>

#define RCM_IDENT 0

#define RACE_ENG_CFG	"config/raceengine.xml"

// Comment-out to inhibit multi-threaded race engine (get back to old nearly unmodified code).
#define ReMultiThreaded 1

struct RmInfo;

typedef int (*tfRmRunState) (struct RmInfo *);

#define RM_SYNC			0x00000001
#define RM_ASYNC		0x00000002

#define RM_END_RACE		0x00000010
#define RM_CONTINUE_RACE	0x00000020

#define RM_NEXT_STEP		0x00000100
#define RM_NEXT_RACE		0x00000200
#define RM_NEXT_EVENT		0x00000400

#define RM_ACTIVGAMESCR		0x01000000
#define RM_QUIT			0x40000000


#define RCM_MAX_DT_SIMU		0.002
#define RCM_MAX_DT_ROBOTS	0.02

/** General info on current race */
typedef struct {
    int			ncars;		/**< number of cars */
    int			totLaps;	/**< total laps */
    int			extraLaps;	/**< number of laps after the time finishes */
    double		totTime;	/**< total time */
    int			state;
#define RM_RACE_RUNNING		0X00000001
#define RM_RACE_FINISHING	0X00000002
#define RM_RACE_ENDED		0X00000004
#define RM_RACE_STARTING	0X00000008
#define RM_RACE_PRESTART	0X00000010
#define RM_RACE_PAUSED		0X40000000
    int			type;		/**< Race type */
#define RM_TYPE_PRACTICE	0 /* Please keep the order */
#define RM_TYPE_QUALIF		1
#define RM_TYPE_RACE		2
    int                 maxDammage;
    unsigned long	fps;
    int			features;	/**< A list of features a race will have (all the robots in the race will have these features) */
#define RM_FEATURE_PENALTIES	0x01	/**< A robot with this feature implemented penalties */
#define RM_FEATURE_TIMEDSESSION 0x02	/**< A robot with this feature has implemented timed sessions */
} tRaceAdmInfo;

#define _ncars		raceInfo.ncars
#define _totLaps	raceInfo.totLaps
#define _extraLaps	raceInfo.extraLaps
#define _totTime	raceInfo.totTime
#define _raceState	raceInfo.state
#define _raceType	raceInfo.type
#define _maxDammage	raceInfo.maxDammage
#define _features	raceInfo.features

/** cars situation used to inform the GUI and the drivers */
typedef struct Situation {
    tRaceAdmInfo	raceInfo;
    double		deltaTime;
    double		currentTime;	/**< current time in sec since the beginning of the simulation */
    int			nbPlayers;	/**< number of human player in local (splitted screen) */
    tCarElt		**cars;		/**< list of cars */ 
} tSituation;

/** Race Engine */
typedef struct 
{
    tTrackItf	trackItf;
    tGraphicItf	graphicItf;
    tSimItf	simItf;
} tRaceModIft;

#define RE_STATE_CONFIG			0
#define RE_STATE_EVENT_INIT		1
#define RE_STATE_PRE_RACE		3
#define RE_STATE_RACE_START		5
#define RE_STATE_NETWORK_WAIT   6
#define RE_STATE_RACE			7
#define RE_STATE_RACE_STOP		8
#define RE_STATE_RACE_END		9
#define RE_STATE_POST_RACE		10
#define RE_STATE_EVENT_SHUTDOWN		11
#define RE_STATE_SHUTDOWN		12
#define RE_STATE_ERROR			13
#define RE_STATE_EXIT			14

/** Race Engine Car Information about the race */
typedef struct 
{
    tTrkLocPos	prevTrkPos;
    tdble	sTime;
    int		lapFlag;
    char	*raceMsg;
    double	totalPitTime;
    double	startPitTime;
    tdble	topSpd;
    tdble	botSpd;
    tdble	fuel;
} tReCarInfo;

/** Race Engine Information.
   @image	html raceenginestate.gif
 */
typedef struct
{
    int			state;
    void		*param;
    tRaceModIft		itf;
    void		*gameScreen;
    void		*menuScreen;
    char		*filename;
    const char		*name;
    const char		*raceName;
    tReCarInfo		*carInfo;
    double		curTime;
    double		lastTime;
    double		timeMult;
    int			running;
#define RM_DISP_MODE_NORMAL	0
#define RM_DISP_MODE_CAPTURE	1
#define RM_DISP_MODE_NONE	2
#define RM_DISP_MODE_SIMU_SIMU	3
    int			displayMode;
    int			refreshDisplay;
#ifdef ReMultiThreaded
    tCarElt		*inPitMenuCar;
	char		*message;
    double		messageEnd;
	char		*bigMessage;
    double		bigMessageEnd;
#endif
} tRaceEngineInfo;

#define _reState	raceEngineInfo.state
#define _reParam	raceEngineInfo.param
#define _reTrackItf	raceEngineInfo.itf.trackItf
#define _reGraphicItf	raceEngineInfo.itf.graphicItf
#define _reSimItf	raceEngineInfo.itf.simItf
#define _reGameScreen	raceEngineInfo.gameScreen
#define _reMenuScreen	raceEngineInfo.menuScreen
#define _reFilename	raceEngineInfo.filename
#define _reName		raceEngineInfo.name
#define _reRaceName	raceEngineInfo.raceName
#define _reCarInfo	raceEngineInfo.carInfo
#define _reCurTime	raceEngineInfo.curTime
#define _reTimeMult	raceEngineInfo.timeMult
#define _reRunning	raceEngineInfo.running
#define _reLastTime	raceEngineInfo.lastTime
#define _displayMode	raceEngineInfo.displayMode
#define _refreshDisplay	raceEngineInfo.refreshDisplay
#ifdef ReMultiThreaded
# define _reInPitMenuCar	raceEngineInfo.inPitMenuCar
# define _reMessage	raceEngineInfo.message
# define _reMessageEnd	raceEngineInfo.messageEnd
# define _reBigMessage	raceEngineInfo.bigMessage
# define _reBigMessageEnd	raceEngineInfo.bigMessageEnd
#endif

#define RM_PNST_DRIVETHROUGH	0x00000001
#define RM_PNST_STOPANDGO	0x00000002
#define RM_PNST_STOPANDGO_OK	0x00000004
#define RM_PNST_SPD		0x00010000
#define RM_PNST_STNGO		0x00020000

typedef struct RmCarRules
{
    int			ruleState;
} tRmCarRules;

typedef struct RmMovieCapture
{
    int		enabled;
    int		state;
    double	deltaSimu;
    double	deltaFrame;
    double	lastFrame;
    const char	*outputBase;
    int		currentCapture;
    int		currentFrame;
} tRmMovieCapture;


/**
 * Race Manager General Info
 */
typedef struct RmInfo
{
    tCarElt		*carList;	/**< List of all the cars racing */
    tSituation		*s;		/**< Situation during race */
    tTrack		*track;		/**< Current track */
    void		*params;	/**< Raceman parameters */
    void		*mainParams;    /**< Stays the same even if params change because of more xml-files per raceman */
    void		*results;	/**< Race results */
    void		*mainResults;   /**< Stays the same even if params change because of more xml-files per raceman */
    tModList		**modList;	/**< drivers loaded */
    tRmCarRules		*rules;		/**< by car rules */
    tRaceEngineInfo	raceEngineInfo;
    tRmMovieCapture	movieCapture;
} tRmInfo;

/*
 * Parameters name definitions for Race Managers
 */
#define RM_SECT_HEADER		"Header"
#define RM_SECT_DRIVERS		"Drivers"
#define RM_SECT_STARTINGGRID	"Starting Grid"
#define RM_SECT_RACES		"Races"
#define RM_SECT_TRACKS		"Tracks"
#define RM_SECT_CONF		"Configuration"
#define RM_SECT_OPTIONS		"Options"
#define RM_SECT_POINTS		"Points"
#define RM_SECT_CLASSPOINTS	"Class Points"
#define RM_SECT_RACECARS	"RaceConfig/Cars"


#define RM_SECT_DRIVERS_RACING	"Drivers Start List"

#define RM_SECT_GROUPS		"Groups"
#define RM_SECT_TEAMS		"Teams"
#define RM_SECT_CLASSES		"Classes"
#define RM_SECT_ALLOWEDTRACKS	"Allowed Tracks"

#define RM_ATTR_CUR_CONF	"current configuration"
#define RM_ATTR_START_ORDER	"starting order"
#define RM_ATTR_ALLOW_RESTART	"restart"
#define RM_ATTR_ENABLED		"enabled"
#define RM_ATTR_MUST_COMPLETE	"must complete"
#define RM_ATTR_SPLASH_MENU	"splash menu"
#define RM_ATTR_DISP_START_GRID	"display starting grid"

#define RM_ATTR_MAXNUM		"maximum number"
#define RM_ATTR_MINNUM		"minimum number"
#define RM_ATTR_TOTALNUM	"total number"
#define RM_ATTR_MAX_DRV		"maximum drivers"
#define RM_ATTR_NUMBER		"number"
#define RM_ATTR_CAR		"car"
#define RM_ATTR_NBGROUPS	"number of groups"
#define RM_ATTR_CAR_CATEGORY "Car Category"

#define RM_ATTR_PRIO		"priority"
#define RM_ATTR_NAME		"name"
#define RM_ATTR_FULLNAME		"full name"
#define RM_ATTR_DRVNAME		"driver name"
#define RM_ATTR_CATEGORY	"category"
#define RM_ATTR_DESCR		"description"
#define RM_ATTR_BGIMG		"menu image"
#define RM_ATTR_RUNIMG		"run image"
#define RM_ATTR_STARTIMG	"start image"

#define RM_ATTR_MODULE		"module"
#define RM_ATTR_IDX		"idx"
#define RM_ATTR_EXTENDED	"extended"
#define RM_ATTR_SKILLLEVEL	"skill level"
#define RM_ATTR_FOCUSED		"focused module"
#define RM_ATTR_FOCUSEDIDX	"focused idx"
#define RM_ATTR_DISPMODE	"display mode"
#define RM_ATTR_DISPRES		"display results"

#define RM_ATTR_TIMESTEP	"time step"

#define RM_ATTR_TYPE		"type"
#define RM_ATTR_RACE		"race"
#define RM_ATTR_ROWS		"rows"
#define RM_ATTR_TOSTART		"distance to start"
#define RM_ATTR_COLDIST		"distance between columns"
#define RM_ATTR_COLOFFSET	"offset within a column"
#define RM_ATTR_INITSPEED	"initial speed"
#define RM_ATTR_INITHEIGHT	"initial height"
#define RM_ATTR_SHOW_RACE	"show race"
#define RM_ATTR_MAX_DMG		"maximum dammage"
#define RM_ATTR_DISTANCE	"distance"
#define RM_ATTR_LAPS		"laps"
#define RM_ATTR_SESSIONTIME	"sessiontime"
#define RM_ATTR_WEATHER		"weather"
#define RM_ATTR_WEATHER_CLOUDS	"weather clouds"
#define RM_ATTR_WEATHER_RAIN    "weather rain"
#define RM_ATTR_TIME		"Time"
#define RM_ATTR_QUAL_LAPS	"Qualification laps"
#define RM_ATTR_POLE		"pole position side"
#define RM_ATTR_CARSPERPIT	"cars per pit"

#define RM_ATTR_POINTS		"points"

#define RM_VAL_TRACKSEL		"track select"
#define RM_VAL_DRVSEL		"drivers select"
#define RM_VAL_RACECONF		"race config"
#define RM_VAL_CONFRACELEN	"race length"
#define RM_VAL_CONFDISPMODE	"display mode"

#define RM_VAL_DRV_LIST_ORDER	"drivers list"
#define RM_VAL_LAST_RACE_ORDER	"last race"
#define RM_VAL_LAST_RACE_RORDER	"last race reversed"

#define RM_VAL_RACE		"race"
#define RM_VAL_QUALIF		"qualifications"
#define RM_VAL_PRACTICE		"practice"

#define RM_VAL_YES		"yes"
#define RM_VAL_NO		"no"

#define RM_VAL_VISIBLE		"normal"
#define RM_VAL_INVISIBLE	"results only"
#define RM_VAL_SIMUSIMU		"simulation simulation"

#define RM_VAL_NO_CLOUDS	"no clouds"
#define RM_VAL_SCARCE_CLOUDS	"scarce clouds"
#define RM_VAL_MORE_CLOUDS	"more clouds"
#define RM_VAL_OVERCAST_CLOUDS	"overcast clouds"

/* Movie capture */

#define RM_SECT_MOVIE_CAPTURE	"Movie Capture"

#define RM_ATT_CAPTURE_ENABLE	"enable capture"
#define RM_ATT_CAPTURE_FPS	"fps"
#define RM_ATT_CAPTURE_OUT_DIR	"output directory"

#define RM_SECT_SUBFILES	"Header/Subfiles"

#define RM_ATTR_HASSUBFILES	"has subfiles"
#define RM_ATTR_FIRSTSUBFILE	"first subfile"
#define RM_ATTR_SUFFIX		"suffix"
#define RM_ATTR_SUBFILE_SUFFIX	"subfile suffix"
#define RM_ATTR_LASTSUBFILE	"islast"
#define RM_ATTR_NEXTSUBFILE	"next subfile"
#define RM_ATTR_PREVSUBFILE	"prev subfile"
#define RM_ATTR_RESULTSUBFILE	"result subfile"
#define RM_ATTR_SUBFILE		"subfile"

#define RM_SECT_ENDOFSEASON	"End-Of-Season"
#define RM_SECT_ENDOFSEASON_CLASSPOINTS	"End-Of-Season/Class Points"

#define RM_SECT_FIRSTNAME	"Names/First Name"
#define RM_SECT_LASTNAME	"Names/Last Name"
#define RM_SECT_DRIVERINFO	"Driver Info"

/* Race Engine modules */

#define RM_SECT_MODULES	"Modules"

#define RM_ATTR_MOD_TRACK		"track"
#define RM_ATTR_MOD_SIMU		"simu"
#define RM_ATTR_MOD_GRAPHIC		"graphic"

#define RM_VAL_MOD_SIMU_V2		"simuv2"
#define RM_VAL_MOD_SIMU_V3		"simuv3"
#define RM_VAL_MOD_TRACK		"track"
#define RM_VAL_MOD_SSGRAPH		"ssggraph"

/* Race Engine itself */

#define RM_SECT_RACE_ENGINE	"Race Engine"

#define RM_ATTR_MULTI_THREADING		"multi-threading"

#define RM_VAL_AUTO		"auto"
#define RM_VAL_ON		"on"
#define RM_VAL_OFF		"off"

/* Results */

#define RE_SECT_HEADER		"Header"
#define RE_ATTR_DATE		"date"
#define RE_ATTR_TYPE		"race"

#define RE_SECT_CURRENT		"Current"
#define RE_ATTR_CUR_RACE	"current race"
#define RE_ATTR_CUR_TRACK	"current track"
#define RE_ATTR_CUR_DRIVER	"current driver"
#define RE_ATTR_CUR_FILE	"current file"
#define RE_ATTR_CUR_SEASON	"current season"

#define RE_SECT_DRIVERS		"Drivers"
#define RE_SECT_DRIVER		"Driver"
#define RE_ATTR_DLL_NAME	"dll name"
#define RE_ATTR_INDEX		"index"

#define RE_SECT_STANDINGS	"Standings"

#define RE_SECT_RESULTS		"Results"
#define RE_SECT_STARTINGGRID	"Starting Grid"

#define RE_SECT_QUALIF		"Qualifications"

#define RE_SECT_FINAL		"Final"

#define RE_SECT_RANK		"Rank"

#define RE_SECT_TEAMINFO	"Team Info"		

#define RE_ATTR_NAME		"name"
#define RE_ATTR_CAR		"car"
#define RE_ATTR_MODULE		"module"
#define RE_ATTR_IDX		"idx"
#define RE_ATTR_LAPS		"laps"
#define RE_ATTR_BEST_LAP_TIME	"best lap time"
#define RE_ATTR_TIME		"time"
#define RE_ATTR_SESSIONTIME	"session time"
#define RE_ATTR_TOP_SPEED	"top speed"
#define RE_ATTR_BOT_SPEED	"bottom speed"
#define RE_ATTR_DAMMAGES	"dammages"
#define RE_ATTR_NB_PIT_STOPS	"pits stops"
#define RE_ATTR_POINTS		"points"
#define RE_SECT_CLASSPOINTS	"Class Points"

#endif /* _RACEMANV1_H_ */ 



