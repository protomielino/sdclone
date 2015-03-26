#ifndef _LINEARATTRACTOR_H_
#define _LINEARATTRACTOR_H_

class LinearAttractor  
{
public:
	LinearAttractor();
	~LinearAttractor();

	double	GetValue() const;
	void	SetValue( double value );

	double	GetVelocity() const;
	void	SetVelocity( double vel );

	double	GetMaxVelocity() const;
	void	SetMaxVelocity( double velocity );

	double	GetMaxAccel() const;
	void	SetMaxAccel( double accel );

	void	Update( double target );

private:
	double	m_value;
	double	m_velocity;
	double	m_maxVelocity;
	double	m_maxAccel;
};

#endif
