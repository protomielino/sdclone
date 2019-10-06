#ifndef _PARAMETRICCUBIC_H_
#define _PARAMETRICCUBIC_H_

#include "Cubic.h"
#include "Vec2d.h"

class ParametricCubic
{
public:
    ParametricCubic();
    ~ParametricCubic();

    void	Set( Vec2d p0, Vec2d p1, Vec2d v0, Vec2d v1 );
    void	SetPointsAndTangents( Vec2d p0, Vec2d v0, Vec2d p1, Vec2d v1 );
    void	SetHalitePointsAndTangents( Vec2d p0, Vec2d v0, Vec2d p1, Vec2d v1 );

    static ParametricCubic FromPoints( Vec2d p0, Vec2d p1, Vec2d p2, Vec2d p3 );
    static ParametricCubic FromPointsAndTangents( Vec2d p0, Vec2d v0, Vec2d p1, Vec2d v1 );
    static ParametricCubic HaliteFromPointsAndTangents( Vec2d p0, Vec2d v0, Vec2d p1, Vec2d v1 );

    Cubic&	GetCubic( int which );

    Vec2d	Calc( double t ) const;
    Vec2d	CalcGradient( double t ) const;
    double	CalcCurvature( double t ) const;

    bool    Calc1stLineCrossingPt( const Vec2d& linePoint, const Vec2d& lineTangent, double* t ) const;

private:
    Cubic	m_x;
    Cubic	m_y;
};

#endif
