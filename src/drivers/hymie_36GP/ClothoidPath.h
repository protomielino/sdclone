/***************************************************************************

    file        : ClothoidPath.h
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

// ClothoidPath.h: interface for the ClothoidPath class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CLOTHOIDPATH_H__E1689BA0_5D2E_4D10_954C_92DC51D23523__INCLUDED_)
#define AFX_CLOTHOIDPATH_H__E1689BA0_5D2E_4D10_954C_92DC51D23523__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "LinePath.h"
#include "MyTrack.h"
#include "Array.h"

class ClothoidPath : public LinePath
{
public:
	enum
	{
		FLAG_FLYING		= 0x01,
	};

	struct Options
	{
		int		bumpMod;
		double	maxL;
		double	maxR;

		Options() : bumpMod(0), maxL(999), maxR(999) {}
		Options( int bm, double ml = 999, double mr = 999 )
		:	bumpMod(bm), maxL(ml), maxR(mr) {}
	};

public:
	ClothoidPath();
	virtual ~ClothoidPath();

	void	ClearFactors();
	void	AddFactor( double factor );
	void	SetFactors( const Array<double>& factors );
	const Array<double>&	GetFactors() const;

	void	MakeSmoothPath( MyTrack* pTrack, const CarModel& cm,
				const Options& opts, int maxstep = 32 );

private:
	void	AnalyseBumps( const CarModel& cm, bool dumpInfo = false );
	void	SmoothBetween( int step );
	void	SetOffset( const CarModel& cm, double k, double t,
					   PathPt* l3, const PathPt* l2, const PathPt* l4 );
	void	OptimiseLine( const CarModel& cm, int idx, int step, double hLimit,
						  PathPt* l3, const PathPt* l2, const PathPt* l4 );
	void	Optimise(	const CarModel& cm, double factor,
						int idx, PathPt* l3,
						const PathPt* l0, const PathPt* l1,
						const PathPt* l2, const PathPt* l4,
						const PathPt* l5, const PathPt* l6,
						int	bumpMod );
	void	OptimisePath(	const CarModel& cm,
							int step, int nIterations, int bumpMod );

private:
	Array<double>	m_factors;
};

#endif // !defined(AFX_CLOTHOIDPATH_H__E1689BA0_5D2E_4D10_954C_92DC51D23523__INCLUDED_)
