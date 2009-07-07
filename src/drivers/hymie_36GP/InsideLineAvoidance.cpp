// InsideLineAvoidance.cpp: implementation of the InsideLineAvoidance class.
//
//////////////////////////////////////////////////////////////////////

#include "InsideLineAvoidance.h"
#include "Opponent.h"
#include "MyRobot.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

InsideLineAvoidance::InsideLineAvoidance()
{
}

InsideLineAvoidance::~InsideLineAvoidance()
{
}

int		InsideLineAvoidance::priority( const Info& ai, const CarElt* /*pCar*/ ) const
{
	if( ai.avoidToSide	||
		ai.avoidLapping	||
		!ai.avoidAhead	||
		(ai.flags & Opponent::F_BEING_LAPPED) == 0 ||
		ai.avoidAhead == (Opponent::F_LEFT | Opponent::F_RIGHT) )
	{
		return PRI_IGNORE;
	}
	else
	{
		return PRI_LAPPING;
	}
}

Vec2d	InsideLineAvoidance::calcTarget( const Info& ai, const CarElt* /*pCar*/, const MyRobot& /*me*/ )
{
	double	target = ai.nextK > 0 ? -0.5 : 0.5;
	return Vec2d(target, 1);
}
