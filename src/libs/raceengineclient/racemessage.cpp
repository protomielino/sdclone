/***************************************************************************

    file        : racemessage.cpp
    created     : Sat Nov 23 09:05:23 CET 2002
    copyright   : (C) 2002 by Eric Espie
    email       : eric.espie@torcs.org 
    version     : $Id: racemessage.cpp,v 1.19 2007/11/06 20:43:32 torcs Exp $

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
    		
    @author	<a href=mailto:eric.espie@torcs.org>Eric Espie</a>
    @version	$Id: racemessage.cpp,v 1.19 2007/11/06 20:43:32 torcs Exp $
*/

#include <cstdlib>

#include <portability.h>
#include <tgfclient.h>
#include <racescreens.h>

#include "racegl.h"

#include "racemessage.h"


static char	buf[1024];
static double	msgDisp;
static double	bigMsgDisp;

void
ReRaceMsgUpdate(tRmInfo* pReInfo)
{
	if (pReInfo->_reMessage)
	{
		ReSetRaceMsg(pReInfo->_reMessage);
	}		
	else
		ReSetRaceMsg(0);
	
	if (pReInfo->_reBigMessage)
	{
		ReSetRaceMsg(pReInfo->_reBigMessage);
	}		
	else
		ReSetRaceBigMsg(0);
}

void
ReRaceMsgManage(tRmInfo* pReInfo)
{
	if (pReInfo->_reMessage && pReInfo->_reCurTime > pReInfo->_reMessageEnd)
	{
		free(pReInfo->_reMessage);
		pReInfo->_reMessage = 0;
	}
	
	if (pReInfo->_reBigMessage && pReInfo->_reCurTime > pReInfo->_reBigMessageEnd)
	{
		free(pReInfo->_reBigMessage);
		pReInfo->_reBigMessage = 0;
	}
}

void
ReRaceMsgSet(tRmInfo* pReInfo, const char *msg, double life)
{
    if (pReInfo->_reMessage)
		free(pReInfo->_reMessage);
    if (msg)
		pReInfo->_reMessage = strdup(msg);
    else
		pReInfo->_reMessage = 0;
	pReInfo->_reMessageEnd = pReInfo->_reCurTime + life;
}

void
ReRaceMsgSetBig(tRmInfo* pReInfo, const char *msg, double life)
{
    if (pReInfo->_reBigMessage)
		free(pReInfo->_reBigMessage);
    if (msg)
		pReInfo->_reBigMessage = strdup(msg);
    else
		pReInfo->_reBigMessage = 0;
	pReInfo->_reBigMessageEnd = pReInfo->_reCurTime + life;
}
