/***************************************************************************

    file                 : torcs.h
    created              : Sat Mar 18 23:49:33 CET 2000
    copyright            : (C) 2000 by Eric Espie
    email                : torcs@free.fr
    version              : $Id: client.h,v 1.2 2003/06/24 21:02:23 torcs Exp $

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
 
 
#ifndef _CLIENT_H_
#define _CLIENT_H_

// DLL exported symbols declarator for Windows.
#ifdef WIN32
# ifdef CLIENT_DLL
#  define CLIENT_API __declspec(dllexport)
# else
#  define CLIENT_API __declspec(dllimport)
# endif
#else
# define CLIENT_API
#endif

CLIENT_API bool MenuEntry(void);

#endif /* _CLIENT_H_ */ 



