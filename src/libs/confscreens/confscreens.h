/***************************************************************************

    file                 : confscreens.h
    created              : Sat Mar 18 23:33:01 CET 2000
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
 
/**
    @defgroup	confscreens	Configuration screens.
    Menu screens for configuration.
*/

#ifndef __CONFSCREENS_H__
#define __CONFSCREENS_H__

// DLL exported symbols declarator for Windows.
#ifdef WIN32
# ifdef CONFSCREENS_DLL
#  define CONFSCREENS_API __declspec(dllexport)
# else
#  define CONFSCREENS_API __declspec(dllimport)
# endif
#else
# define CONFSCREENS_API
#endif

#endif /* __CONFSCREENS_H__ */

