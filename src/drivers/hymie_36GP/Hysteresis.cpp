// Hysteresis.cpp: implementation of the Hysteresis class.
//
//////////////////////////////////////////////////////////////////////

#include "Hysteresis.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Hysteresis::Hysteresis( double hist )
:	m_hist(hist),
	m_oldTest(TEST_UNKNOWN)
{
}

Hysteresis::~Hysteresis()
{
}

bool	Hysteresis::IsLess( double value, double limit ) const
{
	bool	less =	value < limit ||
					m_oldTest == TEST_LESS && value < limit + m_hist;
	m_oldTest = less ? TEST_LESS : TEST_MORE;
	return less;
}

bool	Hysteresis::IsMore( double value, double limit ) const
{
	bool	more =	value > limit ||
					m_oldTest == TEST_MORE && value > limit - m_hist;
	m_oldTest = more ? TEST_MORE : TEST_LESS;
	return more;
}
