/***************************************************************************

    file        : PitPath.h
    created     : 18 Apr 2017
    copyright   : (C) 2017 Tim Foden

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

#include "Path.h"
#include "MyTrack.h"

class PitPath : public Path
{
public:
	PitPath();
	virtual ~PitPath();

	void	MakePath( const tTrackOwnPit* pPit, Path* pBasePath, const CarModel& cm,
					  int pitType, double entryOffset = 0, double exitOffset = 0 );

	//	CPath overrides.
//	virtual bool	ContainsPos( double trackPos ) const;
//	virtual bool	GetPtInfo( double trackPos, PtInfo& pi ) const;

	bool	InPitSection( double trackPos ) const;
	bool	CanStop( double trackPos ) const;
	double	EntryToPitDistance() const;

private:
	double	ToSplinePos( double trackPos ) const;
    void    LocalToGlobalXY( double dist, double offs, double slope, Vec2d* p, Vec2d* v );

private:
//	const MyTrack*	m_pTrack;
	double			m_pitEntryPos;
	double			m_pitExitPos;
	double			m_pitStartPos;
	double			m_pitEndPos;
	int				m_stopIdx;
	double			m_stopPos;
};

#endif // !defined(AFX_PITPATH_H__CA8B7BDD_A9C1_4B01_A44B_FF6965813B40__INCLUDED_)
