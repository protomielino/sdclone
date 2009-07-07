// Hysteresis.h: interface for the Hysteresis class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_HYSTERESIS_H__4ABE8E9B_EE33_42DA_B566_3A54D9331F5C__INCLUDED_)
#define AFX_HYSTERESIS_H__4ABE8E9B_EE33_42DA_B566_3A54D9331F5C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class Hysteresis  
{
public:
	Hysteresis( double hist );
	~Hysteresis();

	bool	IsLess( double value, double limit ) const;
	bool	IsMore( double value, double limit ) const;

private:
	enum
	{
		TEST_LESS		= -1,
		TEST_UNKNOWN	= 0,
		TEST_MORE		= 1,
	};

private:
	double			m_hist;
	mutable int		m_oldTest;
};

#endif // !defined(AFX_HYSTERESIS_H__4ABE8E9B_EE33_42DA_B566_3A54D9331F5C__INCLUDED_)
