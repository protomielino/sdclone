#ifndef _PATTERN_H_
#define _PATTERN_H_

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

#endif
