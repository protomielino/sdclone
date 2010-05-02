/***************************************************************************

    file        : human.h
    created     : Sat May 10 19:12:46 CEST 2003
    copyright   : (C) 2003 by Eric Espi�                        
    email       : eric.espie@torcs.org   
    version     : $Id: human.h,v 1.3 2003/11/08 16:37:17 torcs Exp $                                  

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
    @version	$Id: human.h,v 1.3 2003/11/08 16:37:17 torcs Exp $
*/

#ifndef _NETWORKHUMAN_H_
#define _NETWORKHUMAN_H_

#include <vector>

#include <car.h>


typedef struct HumanContext
{
    int		NbPitStops;
    int		LastPitStopLap;
    int 	AutoReverseEngaged;
    tdble	shiftThld[MAX_GEARS+1];
    tdble	Gear;
    tdble	distToStart;
    float	clutchtime;
    float	clutchdelay;
    float	ABS;
    float	AntiSlip;
    int		lap;
    float	prevLeftSteer;
    float	prevRightSteer;
		float paccel;
		float pbrake;
    int		manual;
    int		Transmission;
    int		NbPitStopProg;
    int		ParamAsr;
    int		ParamAbs;
    int		RelButNeutral;
    int		SeqShftAllowNeutral;
    int		AutoReverse;
    int		drivetrain;
		int   autoClutch;
    tControlCmd	*CmdControl;
    int		MouseControlUsed;
    int		lightCmd;
} tHumanContext;


extern std::vector<tHumanContext*> HCtx;

extern int joyPresent;

#endif /* _NETWORKHUMAN_H_ */ 



