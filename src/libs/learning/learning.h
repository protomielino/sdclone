/***************************************************************************

    file        : learning.h
    copyright   : (C) 2010 by Jean-Philippe Meuret
    email       : pouillot@users.sourceforge.net

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

    @author	<a href=mailto:pouillot@users.sourceforge.net>Jean-Philippe Meuret</a>
*/

#ifndef _LEARNING_H_
#define _LEARNING_H_

// DLL exported symbols declarator for Windows.
#ifdef _WIN32
# ifdef LEARNING_DLL
#  define LEARNING_API __declspec(dllexport)
# else
#  define LEARNING_API __declspec(dllimport)
# endif
#else
# define LEARNING_API
#endif

#endif /* _LEARNING_H_ */
