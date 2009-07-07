/***************************************************************************

    file        : Path.h
    created     : 9 Apr 2006
    copyright   : (C) 2006 Tim Foden

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// Path.h: interface for the Path class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PATH_H__39CE30C6_B9EF_441F_84DD_C2ECA10CB65F__INCLUDED_)
#define AFX_PATH_H__39CE30C6_B9EF_441F_84DD_C2ECA10CB65F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "PtInfo.h"

class Path
{
public:
	Path();
	virtual ~Path();

	virtual bool	ContainsPos( double trackPos ) const = 0;
	virtual bool	GetPtInfo( double trackPos, PtInfo& pi ) const = 0;
};

#endif // !defined(AFX_PATH_H__39CE30C6_B9EF_441F_84DD_C2ECA10CB65F__INCLUDED_)
