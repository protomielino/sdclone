// Pattern.h: interface for the Pattern class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PATTERN_H__DAD78BFD_7ECF_40DB_B6D4_8156C8EA1FAF__INCLUDED_)
#define AFX_PATTERN_H__DAD78BFD_7ECF_40DB_B6D4_8156C8EA1FAF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Array.h"
#include "Span.h"

class Pattern  
{
public:
	Pattern();
	Pattern( const Pattern& pattern );
	Pattern( const Span& span );
	Pattern( double a, double b );
	~Pattern();

	bool	Overlaps( const Span& span ) const;

	void	InsertAt( int index, const Span& span );
	void	RemoveAt( int index );

	void	Remove( const Span& span );	// pattern &= ~span
	void	Add( const Span& span );	// pattern |= span;

//	void	Remove( const Pattern& p );	// pattern &= ~p;
//	void	Add( const Pattern& p );	// pattern |= p;

private:
	Array<Span>	m_spans;
};

#endif // !defined(AFX_PATTERN_H__DAD78BFD_7ECF_40DB_B6D4_8156C8EA1FAF__INCLUDED_)
