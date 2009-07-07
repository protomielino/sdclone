// InsideLineAvoidance.h: interface for the InsideLineAvoidance class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_INSIDELINEAVOIDANCE_H__39A35869_AB40_4D97_A336_DCA2C1765590__INCLUDED_)
#define AFX_INSIDELINEAVOIDANCE_H__39A35869_AB40_4D97_A336_DCA2C1765590__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Avoidance.h"

class InsideLineAvoidance : public Avoidance
{
public:
	InsideLineAvoidance();
	virtual ~InsideLineAvoidance();

	virtual int		priority( const Info& ai, const CarElt* pCar ) const;
	virtual Vec2d	calcTarget( const Info& ai, const CarElt* pCar,
								const MyRobot& me );
};

#endif // !defined(AFX_INSIDELINEAVOIDANCE_H__39A35869_AB40_4D97_A336_DCA2C1765590__INCLUDED_)
