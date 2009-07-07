// OutsideLineAvoidance.cpp: implementation of the OutsideLineAvoidance class.
//
//////////////////////////////////////////////////////////////////////

#include "OutsideLineAvoidance.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

OutsideLineAvoidance::OutsideLineAvoidance()
{
}

OutsideLineAvoidance::~OutsideLineAvoidance()
{
}

int		OutsideLineAvoidance::priority( const Info& ai, const CarElt* /*pCar*/ ) const
{
	if( ai.avoidToSide	||
		ai.avoidLapping	||
		!ai.avoidAhead	||
		ai.avoidAhead == (Opponent::F_LEFT | Opponent::F_RIGHT) )
	{
		return PRI_IGNORE;
	}
	else
	{
		return PRI_TRAFFIC;
	}
}

Vec2d	OutsideLineAvoidance::calcTarget( const Info& ai, const CarElt* /*pCar*/, const MyRobot& /*me*/ )
{
	double	target = ai.nextK > 0 ? 1 : -1;
	return Vec2d(target, 1);
}
