/***************************************************************************

    file        : raceengineclient.h
    copyright   : (C) 2010 by Jean-Philippe Meuret                        
    email       : pouillot@users.sourceforge.net   
    version     : $Id: raceengineclient.h,v 1.1 2010/06/10 18:25:00 pouillot Exp $                                  

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
    @version    $Id: raceengineclient.h,v 1.1 2010/06/10 18:25:00 pouillot Exp $
*/

#ifndef _RACEENGINECLIENT_H_
#define _RACEENGINECLIENT_H_

// DLL exported symbols declarator for Windows.
#ifdef WIN32
# ifdef RACEENGINECLIENT_DLL
#  define RACEENGINECLIENT_API __declspec(dllexport)
# else
#  define RACEENGINECLIENT_API __declspec(dllimport)
# endif
#else
# define RACEENGINECLIENT_API
#endif

#endif /* _RACEENGINECLIENT_H_ */ 
