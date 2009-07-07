// Avoidance.h: interface for the Avoidance class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_AVOIDANCE_H__45D11C2F_08B6_4048_99F6_2B0BED60F84D__INCLUDED_)
#define AFX_AVOIDANCE_H__45D11C2F_08B6_4048_99F6_2B0BED60F84D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Opponent.h"
#include "Span.h"
#include <car.h>

class MyRobot;

class Avoidance  
{
public:
	enum
	{
		PRI_IGNORE	= 0,
		PRI_GENERIC	= 1,
		PRI_TRAFFIC = 3,
		PRI_LAPPING	= 5,
		PRI_MAX		= 10,
	};

	struct Info
	{
		int				flags;
		int				avoidAhead;
//		int				nAvoidAhead;
		int				avoidToSide;
		int				avoidLapping;
		double			k;
		double			nextK;
		double			spdL;
		double			spdR;
		double			spdF;
		double			minLSideDist;
		double			minRSideDist;
		double			minLDist;
		double			minRDist;
		double			bestPathOffs;
		Span			aheadSpan;
		Span			sideSpan;
		Opponent::Info*	pClosestAhead;

		Info()
		{
			Init();
		}

		void	Init()
		{
			flags = 0;
			avoidAhead = 0;
//			nAvoidAhead = 0;
			avoidToSide = 0;
			avoidLapping = 0;
			k = 0;
			nextK = 0;
			spdL = 200;
			spdR = 200;
			spdF = 200;
			minLSideDist = INT_MAX;
			minRSideDist = INT_MAX;
			minLDist = INT_MAX;
			minRDist = INT_MAX;
			bestPathOffs = 0;
			aheadSpan = Span(-999, 999);
			sideSpan = Span(-999, 999);
			pClosestAhead = 0;
		}
	};

public:
	Avoidance();
	virtual ~Avoidance();

	virtual int		priority( const Info& ai, const CarElt* pCar ) const = 0;
	virtual Vec2d	calcTarget( const Info& ai, const CarElt* pCar,
								const MyRobot& me ) = 0;
};

#endif // !defined(AFX_AVOIDANCE_H__45D11C2F_08B6_4048_99F6_2B0BED60F84D__INCLUDED_)
