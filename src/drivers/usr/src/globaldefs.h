/***************************************************************************

    file                 : globaldefs.h
    created              : Sat Aug 08 21:20:19 CET 2015
    copyright            : (C) 2015 Xavier Bertaux
    email                : bertauxx@yahoo.fr
    version              : $Id: globaldefs.h 5902 2015-03-17 22:44:51Z torcs-ng $

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _GLOBALDEFS_H_
#define _GLOBALDEFS_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include <tgf.h>     // TORCS
#include <track.h>   // TORCS
#include <car.h>     // TORCS
#include <raceman.h> // TORCS

// The "USR" logger instance.
extern GfLogger* PLogUSR;
#define LogUSR (*PLogUSR)

static const int MAX_NBBOTS = 100;               // Number of drivers/robots

#define SECT_PRIVATE          "usr private"

// raceline values
#define PRV_ITERATIONS         "iterations"
#define PRV_SIDE_ITERATIONS    "side iterations"
#define PRV_INTMARGIN          "intmargin"
#define PRV_EXTMARGIN          "extmargin"
#define PRV_AVOID_INTMARGIN    "avoidintmargin"
#define PRV_AVOID_EXTMARGIN    "avoidextmargin"
#define PRV_MINTURNINVERSE     "min turn inverse"
#define PRV_CURVE_FACTOR       "curvefactor"
#define PRV_ACCEL_CURVE        "accel curve"
#define PRV_BRAKE_CURVE        "brake curve"
#define PRV_ACCEL_CURVE_LIMIT  "accel curve limit"
#define PRV_BRAKE_CURVE_LIMIT  "brake curve limit"
#define PRV_SECURITY           "security"
#define PRV_STEER_GAIN         "steer gain"
#define PRV_STEER_SKID         "steer skid"
#define PRV_STEER_SKID_OFFLINE "steer skid offline"
#define PRV_STEER_SMOOTH       "steer smooth"
#define PRV_ACCEL_SMOOTH       "accel smooth"
#define PRV_OUTSTEER_REDUCER   "outsteer speed reducer"
#define PRV_LOOKAHEAD          "lookahead"
#define PRV_LOOKAHEAD_OUT      "lookahead outside"
#define PRV_LOOKAHEAD_IN       "lookahead inside"
#define PRV_LOOKAHEAD_EMPTY    "lookahead empty"
#define PRV_LOOKAHEAD_LEFT     "lookahead left"
#define PRV_LOOKAHEAD_RIGHT    "lookahead right"
#define PRV_LOOKAHEAD_CFACTOR  "lookahead cold factor"
#define PRV_SKID_ACCEL         "skid accel"
#define PRV_HTT                "htt"
#define PRV_SHIFT              "gear shift"
#define PRV_SHIFT_UP           "gear shift up"
#define PRV_SHIFT_DOWN         "gear shift down"
#define PRV_CTFACTOR           "ctfactor"
#define PRV_OVERTAKE_CAUTION   "overtake caution"
#define PRV_SKID_CORRECTION    "skid correction"
#define PRV_MIN_CORNER_INV     "min corner inverse"
#define PRV_INC_CORNER_INV     "increase corner inverse"
#define PRV_INC_CORNER_FACTOR  "increase corner factor"
#define PRV_BASE_SPEED         "base speed"
#define PRV_BASE_SPEED_X       "base speed factor"
#define PRV_AVOID_SPEED        "add avoid speed"
#define PRV_AVOID_SPEED_X      "avoid speed factor"
#define PRV_AVOID_BRAKE        "add avoid brake"
#define PRV_AVOID_BRAKE_X      "avoid brake factor"
#define PRV_INT_MARGIN         "int margin"
#define PRV_EXT_MARGIN         "ext margin"
#define PRV_RL_RIGHT_MARGIN    "rl right margin"
#define PRV_RL_LEFT_MARGIN     "rl left margin"
#define PRV_BASE_BRAKE         "base brake"
#define PRV_BRAKE_MOD          "brake mod"
#define PRV_APEX               "apex"
#define PRV_CORRECT_FACTOR     "correct factor"
#define PRV_BRAKE_POWER        "brake power"
#define PRV_SPEED_LIMIT        "speed limit"
#define PRV_RACELINE_DEBUG     "raceline debug"
#define PRV_ACCEL_EXIT         "accel exit"
#define PRV_BUMP_CAUTION       "bump caution"
#define PRV_LEFT_BUMP_CAUTION  "left bump caution"
#define PRV_RIGHT_BUMP_CAUTION "right bump caution"
#define PRV_MAX_SPEED          "max speed"
#define PRV_LEFT_MAX_SPEED     "left max speed"
#define PRV_RIGHT_MAX_SPEED    "right max speed"
#define PRV_SLOPE_FACTOR       "slope factor"
#define PRV_STEER_MOD          "steer mod"
#define PRV_EXIT_BOOST         "exit boost"
#define PRV_EXIT_BOOST_X       "exit boost factor"
#define PRV_AV_EXIT_BOOST      "avoid exit boost"
#define PRV_AV_EXIT_BOOST_X    "avoid exit boost factor"
#define PRV_ACCEL_REDUX_X      "accel redux factor"
#define PRV_FUEL_MASS_FACTOR   "fuel mass factor"
#define PRV_COAST_ACCEL        "coast accel"
#define PRV_BEGIN              "bgn"
#define PRV_END                "end"
#define PRV_WTT                "wtt"
#define PRV_STOP_UPDATE_DIST   "stop update dist"
#define PRV_RESUME_UPDATE_DIST "resume update dist"
#define PRV_LAST_UPDATE_DIST   "last update dist"
#define PRV_SPINDIST           "spin dist"
#define PRV_SPINDIR            "spin dir"
#define PRV_OFFTRACK_ALLOWED   "offtrack allowed"
#define PRV_OFFTRACK_RLIMIT    "rough limit"
#define PRV_EDGE_LINE_MARGIN   "edge line margin"
#define PRV_SAVE_TRACK         "save track"
#define PRV_LOAD_TRACK         "load track"
#define PRV_ERROR_CORRECTION   "error correction factor"
#define PRV_TEST_RACELINE      "test raceline"
#define PRV_USE_MERGED_SPEED   "merged speed"
#define PRV_TRANSITION_INC     "transitionincrement"
#define PRV_REC_TRANSITION_INC "recoverytransitionincrement"
#define PRV_LFT_TRANS_INC      "lefttransitionincrement"
#define PRV_RGT_TRANS_INC      "righttransitionincrement"
#define PRV_CORNERSTEER        "cornersteer"
#define PRV_STEERTIMEFACTOR    "steer time factor"
#define PRV_HOLDMIDDIST        "hold mid distance"
#define PRV_SPEEDERROR         "speed error factor"

// raceline override value
#define PRV_CORNERSPEED        "cornerspeed"
#define PRV_CORNERSPEED_MID    "cornerspeed mid"
#define PRV_CORNERSPEED_SLOW   "cornerspeed slow"
#define PRV_CORNERSPEED_COLD   "cornerspeed cold"
#define PRV_CORNERSPEED_FACTOR "cornerspeed factor"
#define PRV_SPEEDADJUST        "speedadjust"
#define PRV_OFFLINE_BRAKEDELAY "offlinebrakedelay"
#define PRV_BRAKEDELAY         "brakedelay"
#define PRV_BRAKEDELAY_MID     "brakedelay mid"
#define PRV_BRAKEDELAY_SLOW    "brakedelay slow"
#define PRV_BRAKEDELAY_COLD    "brakedelay cold"
#define PRV_LEFT_BRAKEDELAY    "left brakedelay"
#define PRV_RIGHT_BRAKEDELAY   "right brakedelay"
#define PRV_LEFT_BRAKEDELAY_COLD    "left brakedelay cold"
#define PRV_RIGHT_BRAKEDELAY_COLD   "right brakedelay cold"
#define PRV_OUTSIDECORNERSPEED "outsidecornerspeed"
#define PRV_INSIDECORNERSPEED  "insidecornerspeed"
#define PRV_LEFTCORNERSPEED_COLD    "leftcornerspeed cold"
#define PRV_RIGHTCORNERSPEED_COLD   "rightcornerspeed cold"
#define PRV_LEFTCORNERSPEED    "leftcornerspeed"
#define PRV_RIGHTCORNERSPEED   "rightcornerspeed"
#define PRV_AVOIDBRAKEDELAY    "avoidbrakedelay"
#define PRV_RACELINECURVE      "racelinecurve"
#define PRV_COLDTYREFACTOR     "coldtyrefactor"
#define PRV_TYREWEARDANGER     "tyreweardanger"
#define PRV_TYREGRAINDANGER    "tyregraindanger"
#define PRV_AVOID_BUMPCAUTION  "avoid bump caution"
#define PRV_AVOID_SLOPE        "avoid slope factor"
#define PRV_WB4DIST            "twb4d"
#define PRV_WB4COUNT           "twb4count"
#define PRV_LEFT_MARGIN        "left margin"
#define PRV_LEFT_MARGIN_RL     "left margin rl"
#define PRV_RIGHT_MARGIN       "right margin"
#define PRV_RIGHT_MARGIN_RL    "right margin rl"
#define PRV_LEFT_MARGIN_MID    "left margin mid"
#define PRV_RIGHT_MARGIN_MID   "right margin mid"
#define PRV_LEFT_MARGIN_SLOW   "left margin slow"
#define PRV_RIGHT_MARGIN_SLOW  "right margin slow"
#define PRV_PREFERRED_SIDE     "preferred side"
#define PRV_MIN_LANE           "min lane"
#define PRV_MAX_LANE           "max lane"
#define PRV_BRAKEZONE          "brake zone"

// driver values
#define PRV_VERBOSE            "modeverbose"
#define PRV_STRATEGY_VERBOSE   "strategyverbose"
#define PRV_STEER_VERBOSE      "steerverbose"
#define PRV_DISPLAYSETTING     "showsetting"
#define PRV_QUALIF_TEST        "chkqualiftime"
#define PRV_PIT_TEST           "chkpitstop"
#define PRV_PIT_START_OVERRIDE0 "pit start entry" // at this dist from start the car will straighten ready for pit entry
#define PRV_PIT_START_OVERRIDE1 "pit start low speed" // by this distance the car will be at pit speed
#define PRV_PIT_START_OVERRIDE2 "pit start turn in"   // where the spline curves towards pits
#define PRV_PIT_EXIT_OVERRIDE  "pit exit override"
#define PRV_PIT_ENTRY_OFFSET   "pit entry offset"
#define PRV_PIT_EXIT_OFFSET   "pit exit offset"
#define PRV_PIT_TRANS_OFFSET  "pit transition offset"
#define PRV_PIT_OFFSET         "pit offset"
#define PRV_PIT_EXIT_SPEED     "pit exit speed"
#define PRV_PIT_MAX_SPEED      "pit max speed"
#define PRV_PIT_MAX_SPEED_OFFSET "pit max speed offset"
#define PRV_PIT_SPEED_MARGIN   "pit speed margin"
#define PRV_PIT_LAP_BUFFER     "pit lap buffer"
#define PRV_PIT_EXIT_TI        "pit exit ti"
#define PRV_NO_PIT             "no pit"
#define PRV_FORCE_PIT          "force pit"
#define PRV_PIT_STRATEGY       "pitstrat"
#define PRV_PITSTOP_TIME       "pitstop time"
#define PRV_LINE_INDEX         "lineindex"
#define PRV_FUEL_FACTOR        "fueltankfactor"
#define PRV_MAXFUEL            "max fuel"
#define PRV_INITIAL_FUEL       "initial fuel"
#define PRV_LETPASS            "LetPass"
#define PRV_TCL_SLIP           "tcl_slip"
#define PRV_ABS_SLIP           "abs_slip"
#define PRV_TCL_RANGE          "tcl_range"
#define PRV_ABS_RANGE          "abs_range"
#define PRV_TURN_SPEED         "turnspeed"
#define PRV_OFFLINE_TURNSPEED  "offlineturnspeed"
#define PRV_BRAKE_MARGIN       "brakemargin"
#define PRV_COLLBRAKE_TIMPACT  "coll brake timpact"
#define PRV_COLLBRAKE_BOOST    "coll brake boost"
#define PRV_PIT_EXTRA_LENGTH   "pit start extra length"
#define PRV_AVOID_OFFSET       "avoid offset"
#define PRV_FULLTANK_PERCENT   "full tank percentage"
#define PRV_MIDTANK_PERCENT    "mid tank percentage"
#define PRV_SPEED_MULTIPLIER   "speed multiplier"
#define PRV_WARN_MULTIPLIER    "brake warn multiplier"
#define PRV_BRAKE_MULTIPLIER   "brake multiplier"
#define PRV_BRAKE_COEFFICIENT  "brake coefficient"
#define PRV_OUTSIDE_OVERTAKE   "outside overtake inhibitor"
#define PRV_OUTSIDE_DAMPENER   "outside steering dampener"           // the smaller value the more it dampens
#define PRV_OUTSIDE_DAMPENER_O "outside steering dampener overlap"   // steering to the outside. Values > 1 will
#define PRV_OUTSIDE_DAMPENER_A "outside steering dampener accel"     // increase outside steering.
#define PRV_SPEED_ADVANCE      "speed advance"
#define PRV_SPEED_DIVISOR      "speed divisor"
#define PRV_LEFT_OVERTAKE      "left overtake"
#define PRV_RIGHT_OVERTAKE     "right overtake"
#define PRV_OVERTAKE           "overtake"
#define PRV_RL_FOR_OVERTAKE    "rl for overtake"
#define PRV_OVERTAKE_SPD_DIFF  "speed diff for overtake"
#define PRV_STAY_INSIDE        "stay inside"
#define PRV_DEFEND_LINE		   "defend line" // (1=lft, 2=rgt, 3=mid)

#define PRV_REVS_UP            "revs change up"
#define PRV_REVS_DOWN          "revs change down"
#define PRV_REVS_DOWN_MAX      "revs change down max"
#define PRV_NO_TEAM_WAITING    "no team waiting"
#define PRV_TEAM_WAIT_TIME     "team wait time"

#define SECT_SKILL            "skill"
#define PRV_SKILL_LEVEL       "level"
#define PRV_SKILL_AGGRO       "aggression"

#define BT_SECT_PRIV          "private"
#define BT_ATT_FUELPERMETER   "fuelpermeter"
#define BT_ATT_FUELPERLAP     "fuelperlap"
#define BT_ATT_FUELPERSECOND  "fuelpersecond"
#define BT_ATT_MUFACTOR       "mufactor"
#define BT_ATT_BRAKEDIST      "brakedelay"
#define BT_ATT_PITTIME        "pittime"
#define BT_ATT_PITOFFSET      "pit offset"
#define BT_ATT_BESTLAP        "bestlap"
#define BT_ATT_WORSTLAP       "worstlap"
#define BT_ATT_TEAMMATE       "teammate"
#define BT_ATT_MAXDAMAGE      "max damages"

enum { no_mode=0, normal=1, correcting=2, pitting=4, avoiding=8, avoidright=16, avoidside=32, avoidleft=64 };

/*=========================================================================*
                  global constants definition
 *=========================================================================*/

// Status flags
                const int OPP_IGNORE			= 0x1;
                const int OPP_FRONT				= 0x2; // Opponent are in front of my car
                const int OPP_BACK				= 0x4; // Opponent are behind my car
                const int OPP_SIDE				= 0x8; // Looking to a side
                const int OPP_LEFT				= 0x10; // Opponent are at my left side
                const int OPP_RIGHT				= 0x20; // Opponent are at my right side
                const int OPP_COLL				= 0x40;
                const int OPP_COLL_WARNING		= 0x80;
                const int OPP_LETPASS			= 0x100;
                const int OPP_FRONT_FAST		= 0x200; // Drives faster than we
                const int OPP_FRONT_FOLLOW		= 0x400; // Drives slower than we
                const int OPP_OFF_TRACK			= 0x800; // don't worry about this one
                const int OPP_COLL_LINEAR   	= 0x1000; // don't worry about this one
                const int OPP_BACK_CATCHING		= 0x2000;
                const int OPP_BACK_THREAT  		= 0x4000;
                const int OPP_BACK_SLOW         = 0x8000;

#endif