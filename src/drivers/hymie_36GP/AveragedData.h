// AveragedData.h: interface for the AveragedData class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_AVERAGEDDATA_H__CAC1F03C_8AE5_4886_8E2A_38F14A3E03EB__INCLUDED_)
#define AFX_AVERAGEDDATA_H__CAC1F03C_8AE5_4886_8E2A_38F14A3E03EB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class AveragedData  
{
public:
	AveragedData( int nBinsX, double minX, double maxX,
				  int nBinsY, double minY, double maxY );
	~AveragedData();

	int		GetXSize() const;
	int		GetYSize() const;
	double	GetAxisValue( int axis, int index ) const;
	double	GetValueAt( int x, int y ) const;
	void	AddValue( double x, double y, double value );

private:
	struct Avg
	{
		Avg() : value(0), count(0) {}

		double	value;
		int		count;
	};

private:
	int		m_xSize;
	double	m_xMin;
	double	m_xSpan;
	int		m_ySize;
	double	m_yMin;
	double	m_ySpan;
	Avg*	m_pData;
};

#endif // !defined(AFX_AVERAGEDDATA_H__CAC1F03C_8AE5_4886_8E2A_38F14A3E03EB__INCLUDED_)
