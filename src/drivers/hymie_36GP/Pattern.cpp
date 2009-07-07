// Pattern.cpp: implementation of the Pattern class.
//
//////////////////////////////////////////////////////////////////////

#include "Pattern.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Pattern::Pattern()
{
}

Pattern::Pattern( const Pattern& pattern )
{
	m_spans = pattern.m_spans;
}

Pattern::Pattern( const Span& span )
{
	m_spans.Add( span );
}

Pattern::Pattern( double a, double b )
{
	m_spans.Add( Span(a, b) );
}

Pattern::~Pattern()
{
}

bool	Pattern::Overlaps( const Span& span ) const
{
	if( span.IsNull() )
		return false;

	for( int i = 0; i < m_spans.GetSize(); i++ )
	{
		if( m_spans[i].Overlaps(span) )
			return true;
	}

	return false;
}

void	Pattern::InsertAt( int index, const Span& span )
{
	m_spans.InsertAt( index, span );
}

void	Pattern::RemoveAt( int index )
{
	m_spans.RemoveAt( index );
}

void	Pattern::Remove( const Span& span )
{
	if( span.IsNull() )
		return;

	for( int i = m_spans.GetSize() - 1; i >= 0; i-- )
	{
		if( m_spans[i].a >= span.b )
			continue;

		if( m_spans[i].Overlaps(span) )
		{
			if( m_spans[i].Contains(span) )
			{
				InsertAt( i + 1, Span(span.b, m_spans[i].b) );
				m_spans[i].b = span.a;
				break;
			}
			else if( span.Contains(m_spans[i]) )
			{
				RemoveAt( i );
			}
			else
			{
				if( m_spans[i].a < span.a )
					m_spans[i].b = span.a;
				else
					m_spans[i].a = span.b;
			}
		}
		else if( m_spans[i].b <= span.a )
		{
			break;
		}
	}

//	ASSERT( !Overlaps(span) );
}

void	Pattern::Add( const Span& span )
{
	if( span.IsNull() )
		return;

	if( m_spans.IsEmpty() )
	{
		m_spans.Add( span );
		return;
	}

	if( span.b < m_spans[0].a )
	{
		m_spans.InsertAt( 0, span );
		return;
	}

	if( span.a > m_spans[m_spans.GetSize() - 1].b )
	{
		m_spans.Add( span );
		return;
	}

	for( int i = m_spans.GetSize() - 1; i >= 0; i-- )
	{
		if( m_spans[i].Overlaps(span) )
		{
			if( m_spans[i].Contains(span) )
				return;

//			if( m_spans[i].a < span.a )
//			else
		}
	}
}
