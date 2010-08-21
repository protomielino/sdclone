/***************************************************************************

    file        : racemessage.h
    created     : Sat Nov 23 09:35:21 CET 2002
    copyright   : (C) 2002 by Eric Espie                        
    email       : eric.espie@torcs.org   
    version     : $Id: racemessage.h,v 1.4 2004/04/05 18:25:00 olethros Exp $                                  

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
    @version	$Id: racemessage.h,v 1.4 2004/04/05 18:25:00 olethros Exp $
*/

#ifndef _RACEMESSAGE_H_
#define _RACEMESSAGE_H_

struct RmInfo;

extern void ReRaceMsgUpdate(struct RmInfo* pReInfo);
extern void ReRaceMsgManage(struct RmInfo* pReInfo);
extern void ReRaceMsgSet(struct RmInfo* pReInfo, const char *msg, double life);
extern void ReRaceMsgSetBig(struct RmInfo* pReInfo, const char *msg, double life);


#endif /* _RACEMESSAGE_H_ */ 



