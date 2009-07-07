// LinearAttractor.cpp: implementation of the LinearAttractor class.
//
//////////////////////////////////////////////////////////////////////

#include "LinearAttractor.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

LinearAttractor::LinearAttractor()
:	m_value(0),
	m_velocity(0),
//	m_accel(0),
	m_maxVelocity(0.01),
	m_maxAccel(0.01)
//	m_min(-1),
//	m_max(1)
{
}

LinearAttractor::~LinearAttractor()
{
}

double	LinearAttractor::GetValue() const
{
	return m_value;
}

void	LinearAttractor::SetValue( double value )
{
	m_value = value;
}

double	LinearAttractor::GetVelocity() const
{
	return m_velocity;
}

void	LinearAttractor::SetVelocity( double velocity )
{
	m_velocity = velocity;
}
/*
double	LinearAttractor::GetAccel() const
{
	return m_accel;
}

void	LinearAttractor::SetAccel( double accel )
{
	m_accel = accel;
}
*/
double	LinearAttractor::GetMaxVelocity() const
{
	return m_maxVelocity;
}

void	LinearAttractor::SetMaxVelocity( double velocity )
{
	m_maxVelocity = velocity;
}

double	LinearAttractor::GetMaxAccel() const
{
	return m_maxAccel;
}

void	LinearAttractor::SetMaxAccel( double accel )
{
	m_maxAccel = accel;
}
/*
double	LinearAttractor::GetMin() const
{
	return m_min;
}

void	LinearAttractor::SetMin( double min )
{
	m_min = min;
}

double	LinearAttractor::GetMax() const
{
	return m_max;
}

void	LinearAttractor::SetMax( double max )
{
	m_max = max;
}
*/
void	LinearAttractor::Update( double target )
{
	double	delta = target - m_value;

	double	targetVelocity = delta * 0.05;
	if( targetVelocity > m_maxVelocity )
		targetVelocity = m_maxVelocity;
	else if( targetVelocity < -m_maxVelocity )
		targetVelocity = -m_maxVelocity;

	double	velocityDelta = targetVelocity - m_velocity;

	double	targetAccel = velocityDelta * 0.5;
	if( targetAccel > m_maxAccel )
		targetAccel = m_maxAccel;
	else if( targetAccel < -m_maxAccel )
		targetAccel = -m_maxAccel;

	m_velocity += targetAccel;
	m_value += m_velocity;
}
