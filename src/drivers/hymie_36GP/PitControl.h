/***************************************************************************

    file        : PitControl.h
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

// PitControl.h: interface for the PitControl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PITCONTROL_H__81207FE5_EA2F_4DC9_B253_1625C2B60DBF__INCLUDED_)
#define AFX_PITCONTROL_H__81207FE5_EA2F_4DC9_B253_1625C2B60DBF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "MyTrack.h"
#include "PitPath.h"

class PitControl  
{
public:
	PitControl( const MyTrack& track, const PitPath& pitPath );
	~PitControl();

	int	Process( CarElt* pCar, int pitInUse, CarElt *pOCar );

	bool	WantToPit() const;

private:
	enum
	{
		PIT_NONE,
		PIT_BEFORE,
		PIT_ENTER,
		PIT_ASKED,
		PIT_EXIT,
	};

private:
	const MyTrack&	m_track;
	const PitPath&	m_pitPath;

	int				m_state;

	double			m_lastFuel;
	double			m_totalFuel;
	double			m_lastDamage;
	double			m_totalDamage;
	int				m_lastLap;
	int				m_totalLaps;
};

#endif // !defined(AFX_PITCONTROL_H__81207FE5_EA2F_4DC9_B253_1625C2B60DBF__INCLUDED_)
