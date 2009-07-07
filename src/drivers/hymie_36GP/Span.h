// Span.h: interface for the Span class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SPAN_H__DA520979_A9C3_4DA0_BBC2_4CD0C2A03C60__INCLUDED_)
#define AFX_SPAN_H__DA520979_A9C3_4DA0_BBC2_4CD0C2A03C60__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class Span  
{
public:
	Span();
	Span( const Span& span );
	Span( double A, double B );
	~Span();

	bool	IsNull() const;
	double	GetSize() const;

	void	Set( double x, double y );

	bool	Overlaps( const Span& span ) const;
	bool	Contains( const Span& span ) const;
	bool	Contains( double x ) const;

	Span	Intersect( const Span& span ) const;
	Span	Intersect( double A, double B ) const;

	void	Extend( double x );
	void	ExcludeLeftOf( double x );
	void	ExcludeRightOf( double x );

public:
	double	a;
	double	b;
};

#endif // !defined(AFX_SPAN_H__DA520979_A9C3_4DA0_BBC2_4CD0C2A03C60__INCLUDED_)
