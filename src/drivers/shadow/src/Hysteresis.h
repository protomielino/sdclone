#ifndef _HYSTERESIS_H_
#define _HYSTERESIS_H_

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

#endif
