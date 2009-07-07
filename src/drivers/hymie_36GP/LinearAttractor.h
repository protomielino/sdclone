// LinearAttractor.h: interface for the LinearAttractor class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LINEARATTRACTOR_H__9C9A0C2D_4935_48C8_BDCF_5D1FD495A208__INCLUDED_)
#define AFX_LINEARATTRACTOR_H__9C9A0C2D_4935_48C8_BDCF_5D1FD495A208__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class LinearAttractor  
{
public:
	LinearAttractor();
	~LinearAttractor();

	double	GetValue() const;
	void	SetValue( double value );
	double	GetVelocity() const;
	void	SetVelocity( double vel );
//	double	GetAccel() const;
//	void	SetAccel( double accel );
	double	GetMaxVelocity() const;
	void	SetMaxVelocity( double velocity );
	double	GetMaxAccel() const;
	void	SetMaxAccel( double accel );
//	double	GetMin() const;
//	void	SetMin( double min );
//	double	GetMax() const;
//	void	SetMax( double max );

	void	Update( double target );

private:
	double	m_value;
	double	m_velocity;
//	double	m_accel;
	double	m_maxVelocity;
	double	m_maxAccel;
//	double	m_min;
//	double	m_max;
};

#endif // !defined(AFX_LINEARATTRACTOR_H__9C9A0C2D_4935_48C8_BDCF_5D1FD495A208__INCLUDED_)
