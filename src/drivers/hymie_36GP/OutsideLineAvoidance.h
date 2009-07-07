// OutsideLineAvoidance.h: interface for the OutsideLineAvoidance class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_OUTSIDELINEAVOIDANCE_H__3F095E62_0298_46C4_BBDC_2E85CAFE3706__INCLUDED_)
#define AFX_OUTSIDELINEAVOIDANCE_H__3F095E62_0298_46C4_BBDC_2E85CAFE3706__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Avoidance.h"

class OutsideLineAvoidance : public Avoidance
{
public:
	OutsideLineAvoidance();
	virtual ~OutsideLineAvoidance();

	virtual int		priority( const Info& ai, const CarElt* pCar ) const;
	virtual Vec2d	calcTarget( const Info& ai, const CarElt* pCar,
								const MyRobot& me );
};

#endif // !defined(AFX_OUTSIDELINEAVOIDANCE_H__3F095E62_0298_46C4_BBDC_2E85CAFE3706__INCLUDED_)
