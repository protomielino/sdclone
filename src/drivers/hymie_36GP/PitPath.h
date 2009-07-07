/***************************************************************************

    file        : PitPath.h
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

// PitPath.h: interface for the PitPath class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PITPATH_H__CA8B7BDD_A9C1_4B01_A44B_FF6965813B40__INCLUDED_)
#define AFX_PITPATH_H__CA8B7BDD_A9C1_4B01_A44B_FF6965813B40__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "LinePath.h"
#include "MyTrack.h"
#include "CubicSpline.h"

class PitPath : public LinePath
{
public:
	PitPath();
	virtual ~PitPath();

	void	MakePath( CarElt* pCar, LinePath* pBasePath, const CarModel& cm,
						double entryOffset = 0, double exitOffset = 0 );

	//	CPath overrides.
//	virtual bool	ContainsPos( double trackPos ) const;
//	virtual bool	GetPtInfo( double trackPos, PtInfo& pi ) const;

	bool	InPitSection( double trackPos ) const;
	bool	CanStop( double trackPos ) const;

private:
	double	ToSplinePos( double trackPos ) const;

private:
//	const MyTrack*	m_pTrack;
	double			m_pitEntryPos;
	double			m_pitExitPos;
	double			m_pitStartPos;
	double			m_pitEndPos;
	int				m_stopIdx;
//	CubicSpline*	m_pSpline;
};

#endif // !defined(AFX_PITPATH_H__CA8B7BDD_A9C1_4B01_A44B_FF6965813B40__INCLUDED_)
