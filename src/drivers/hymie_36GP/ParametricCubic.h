// ParametricCubic.h: interface for the ParametricCubic class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PARAMETRICCUBIC_H__EF867A55_5F05_4790_B3C0_DA5740F7135F__INCLUDED_)
#define AFX_PARAMETRICCUBIC_H__EF867A55_5F05_4790_B3C0_DA5740F7135F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Cubic.h"
#include "Vec2d.h"

class ParametricCubic  
{
public:
	ParametricCubic();
	~ParametricCubic();

	void	Set( Vec2d p0, Vec2d p1, Vec2d v0, Vec2d v1 );

	Vec2d	Calc( double t ) const;
	Vec2d	CalcGradient( double t ) const;
	double	CalcCurvature( double t ) const;

private:
	Cubic	m_x;
	Cubic	m_y;
};

#endif // !defined(AFX_PARAMETRICCUBIC_H__EF867A55_5F05_4790_B3C0_DA5740F7135F__INCLUDED_)
