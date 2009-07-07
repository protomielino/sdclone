// GenericAvoidance.h: interface for the GenericAvoidance class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GENERICAVOIDANCE_H__A766CF08_2B64_409F_8D41_E7322BE9D357__INCLUDED_)
#define AFX_GENERICAVOIDANCE_H__A766CF08_2B64_409F_8D41_E7322BE9D357__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Avoidance.h"

class GenericAvoidance : public Avoidance
{
public:
	GenericAvoidance();
	virtual ~GenericAvoidance();

	virtual int		priority( const Info& ai, const CarElt* pCar ) const;
	virtual Vec2d	calcTarget( const Info& ai, const CarElt* pCar,
								const MyRobot& me );
};

#endif // !defined(AFX_GENERICAVOIDANCE_H__A766CF08_2B64_409F_8D41_E7322BE9D357__INCLUDED_)
