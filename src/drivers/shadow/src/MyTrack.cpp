/***************************************************************************

    file        : MyTrack.cpp
    created     : 9 Apr 2006
    copyright   : (C) 2006 Tim Foden

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// MyTrack.cpp: implementation of the MyTrack class.
//
//////////////////////////////////////////////////////////////////////

#include <robottools.h>

#include "MyTrack.h"
#include "Utils.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

MyTrack::MyTrack()
:	NSEG(0),
    m_delta(3),
    m_pSegs(0),
    m_pCurTrack(0)
{
}

MyTrack::~MyTrack()
{
    delete [] m_pSegs;
}

void MyTrack::Clear()
{
    delete [] m_pSegs;
    NSEG = 0;
    m_pSegs = 0;
    m_pCurTrack = 0;
    m_innerMod.clear();
    m_nBends = 0;
}

void MyTrack::NewTrack( tTrack* pNewTrack, const std::vector<double>* pInnerMod, bool pit, SideMod* pSideMod )
{
    if( m_pCurTrack != pNewTrack )
    {
        delete [] m_pSegs;
        m_pSegs = 0;
        NSEG = 0;
    }

    m_pCurTrack = pNewTrack;

    if( pInnerMod )
        m_innerMod = *pInnerMod;
    else
    {
        m_innerMod.clear();
    }


    if( pSideMod )
        m_sideMod = *pSideMod;

    if( m_pSegs == 0 )
    {
        // make new segs ... roughly every NOMINAL_SEG_LEN metres apart.
        const double	NOMINAL_SEG_LEN = 3;//10;
        NSEG = int(floor(pNewTrack->length / NOMINAL_SEG_LEN));
        m_pSegs = new Seg[NSEG];
        m_delta = pNewTrack->length / NSEG;

        tTrackSeg*	pseg = pNewTrack->seg;

        while( pseg->lgfromstart > pNewTrack->length / 2 )
            pseg = pseg->next;

        double		tsend = pseg->lgfromstart + pseg->length;

        int	pitEntry = -1;
        int	pitExit  = -1;
        int pitSide  = pNewTrack->pits.side == TR_LFT ? TR_SIDE_LFT : TR_SIDE_RGT;

        for( int i = 0; i < NSEG; i++ )
        {
            double	segDist = i * m_delta;

            while( segDist >= tsend )
            {
                pseg = pseg->next;
                tsend = pseg->lgfromstart + pseg->length;
            }

            m_pSegs[i].segDist = segDist;
            m_pSegs[i].pSeg = pseg;
            m_pSegs[i].wl = pseg->width / 2;
            m_pSegs[i].wr = pseg->width / 2;
            m_pSegs[i].midOffs = 0;

            if( pitEntry < 0 && (pseg->raceInfo & TR_PITENTRY) )
                pitEntry = i;

            if( (pseg->raceInfo & TR_PITEXIT) )
                pitExit  = i;
        }

        for( int i = 0; i < NSEG; i++ )
        {
            pseg = m_pSegs[i].pSeg;

            double	segDist = m_pSegs[i].segDist;
            double	t = (segDist - pseg->lgfromstart) / pseg->length;

            bool	inPit = (pitEntry < (pitExit && pitEntry <= i && i <= pitExit) ||
                            (pitEntry > pitExit && i <= pitExit) || (i >= pitEntry));

            const double	MIN_MU = pseg->surface->kFriction * 0.8;
            const double	MAX_ROUGH = MX(0.005, pseg->surface->kRoughness * 1.2);
            const double	MAX_RESIST = MX(0.02, pseg->surface->kRollRes * 1.2);
            const double	SLOPE = pseg->Kzw;

            for( int s = 0; s < 2; s++ )
            {
                tTrackSeg*	pSide = pseg->side[s];

                if( pSide == 0 )
                    continue;

                double	extraW = 0;

                bool	done = false;

                while( !done && pSide )
                {
                    double	w = pSide->startWidth + (pSide->endWidth - pSide->startWidth) * t;

                    if( pSide->style == TR_CURB )
                    {
                        if( s == m_sideMod.side && i >= m_sideMod.start && i <= m_sideMod.end );
                        else
                        {
                        // always keep 1 wheel on main track.
                        w = MN(w, 1.5);
                        done = true;

                        if( (s == TR_SIDE_LFT && pseg->type == TR_RGT) ||
                             ((s == TR_SIDE_RGT && pseg->type == TR_LFT) &&
                            pSide->surface->kFriction  < pseg->surface->kFriction ))
                            // keep a wheel on the good stuff.
                            w = 0;

                        // don't go too far up raised curbs (max 2cm).
                        if( pSide->height > 0 )
                            w = MN(w, 0.6);
                        }
                    }
                    else if( pSide->style == TR_PLAN )
                    {
                        if( (inPit && pitSide == s)	|| (pSide->raceInfo & (TR_SPEEDLIMIT | TR_PITLANE)) )
                        {
                            w = 0;
                            done = true;
                        }

                        if( s == m_sideMod.side &&
                            i >= m_sideMod.start &&
                            i <= m_sideMod.end )
                        {
                            if( w > 0.5 )
                                { w = 0.5; done = true; }
                        }
                        else
                        if( pSide->surface->kFriction  < MIN_MU		||
                            pSide->surface->kRoughness > MAX_ROUGH	||
                            pSide->surface->kRollRes   > MAX_RESIST	||
                            fabs(pSide->Kzw - SLOPE) > 0.005 )
                        {
                            w = 0;
                            done = true;
                        }

                        if( (s == TR_SIDE_LFT && pseg->type == TR_RGT) ||
                            (( s == TR_SIDE_RGT && pseg->type == TR_LFT) &&
                            pSide->surface->kFriction  < pseg->surface->kFriction ))
                        {
                            // keep a wheel on the good stuff.
                            w = 0;
                            done = true;
                        }
                    }
                    else
                    {
                        // wall of some sort.
                        w = pSide->style == TR_WALL ? -0.5 : 0;
                        done = true;
                    }

                    extraW += w;
                    pSide = pSide->side[s];
                }

                if( s == TR_SIDE_LFT )
                    m_pSegs[i].wl += extraW;
                else
                    m_pSegs[i].wr += extraW;
            }

            CalcPtAndNormal( pseg, segDist - pseg->lgfromstart,
                                m_pSegs[i].t,
                                m_pSegs[i].pt, m_pSegs[i].norm );

        }
    }
}

tTrack*	MyTrack::GetTrack()
{
    return m_pCurTrack;
}

const tTrack* MyTrack::GetTrack() const
{
    return m_pCurTrack;
}

double MyTrack::GetLength() const
{
    return m_pCurTrack->length;
}

int	MyTrack::GetSize() const
{
    return NSEG;
}

double MyTrack::GetWidth() const
{
    return m_pCurTrack->width;
}

double MyTrack::NormalisePos( double trackPos ) const
{
    while( trackPos < 0 )
        trackPos += m_pCurTrack->length;
    while( trackPos >= m_pCurTrack->length )
        trackPos -= m_pCurTrack->length;
    return trackPos;
}

int	MyTrack::IndexFromPos( double trackPos ) const
{
    int	idx = int(floor(trackPos / m_delta)) % NSEG;
    if( idx < 0 )
        idx += NSEG;
    else if( idx >= NSEG )
        idx -= NSEG;
    return idx;
}

const Seg& MyTrack::operator[]( int index ) const
{
    return m_pSegs[index];
}

const Seg& MyTrack::GetAt( int index ) const
{
    return m_pSegs[index];
}

double MyTrack::GetDelta() const
{
    return m_delta;
}

double MyTrack::CalcPos( tTrkLocPos& trkPos, double offset ) const
{
    double	pos = RtGetDistFromStart2(&trkPos) + offset;
    return NormalisePos(pos);
}

double MyTrack::CalcPos( tCarElt* car, double offset ) const
{
    double	pos = RtGetDistFromStart(car) + offset;
    return NormalisePos(pos);
}

double MyTrack::CalcPos( double x, double y, const Seg* hint, bool sides ) const
{
    tTrackSeg*	pTrackSeg = m_pSegs[0].pSeg;
    if( hint != 0 )
        pTrackSeg = hint->pSeg;

    tTrkLocPos	pos;
    RtTrackGlobal2Local( pTrackSeg, (tdble) x, (tdble)y, &pos, sides );
    double	dist = RtGetDistFromStart2(&pos);
    return dist;
}

double	MyTrack::CalcHeightAbovePoint( const Vec3d& start_point, const Vec3d& direction, const Seg* hint ) const
{
    tTrkLocPos	pos;
    pos.seg = hint ? hint->pSeg : m_pSegs[0].pSeg;

    Vec3d	point(start_point);

    for(int i = 0; i < 10; i++ )
    {
        RtTrackGlobal2Local( pos.seg, tdble(point.x), tdble(point.y), &pos, TR_LPOS_MAIN );
        double h = RtTrackHeightL(&pos);
        double delta_h = h - point.z;

        if( fabs(delta_h) < 0.0001 )
            break;

        point.x += direction.x * delta_h;
        point.y += direction.y * delta_h;
        point.z += direction.z * delta_h;
    }

    double dot = (point - start_point) * direction;

    return dot;
}

double MyTrack::CalcForwardAngle( double trackPos ) const
{
    int					idx = IndexFromPos(trackPos);
    const tTrackSeg*	pSeg = m_pSegs[idx].pSeg;

    double	t;
    Vec3d	pt;
    Vec3d	norm;
    CalcPtAndNormal( pSeg, trackPos - pSeg->lgfromstart, t, pt, norm );

    return Utils::VecAngXY(norm) + PI / 2;
}

Vec2d MyTrack::CalcNormal( double trackPos ) const
{
    int					idx = IndexFromPos(trackPos);
    const tTrackSeg*	pSeg = m_pSegs[idx].pSeg;

    double	t;
    Vec3d	pt;
    Vec3d	norm;
    CalcPtAndNormal( pSeg, trackPos - pSeg->lgfromstart, t, pt, norm );

    return norm.GetXY();
}

double MyTrack::GetFriction( int index, double offset ) const
{
    const tTrackSeg*	pSeg = m_pSegs[index].pSeg;
    double	friction = pSeg->surface->kFriction;
//	if( pSeg->surface->kRoughness > 0.005 )
//		friction *= 0.9;

/*
    double	w = pSeg->width / 2;
    if( offset < -w && pSeg->lside )
    {
        // on side to left
        friction = pSeg->lside->surface->kFriction;
    }
    else if( offset > w )
    {
        // on side to right
        friction = pSeg->rside->surface->kFriction;
    }
*/
    return friction;
}

void MyTrack::CalcPtAndNormal( const tTrackSeg*	pSeg, double toStart, double& t, Vec3d&	pt, Vec3d& norm ) const
{
    if( pSeg->type == TR_STR )
    {
        Vec3d	s = (Vec3d(pSeg->vertex[TR_SL]) + Vec3d(pSeg->vertex[TR_SR])) / 2;
        Vec3d	e = (Vec3d(pSeg->vertex[TR_EL]) + Vec3d(pSeg->vertex[TR_ER])) / 2;
        t = toStart / pSeg->length;
        pt = s + (e - s) * t;

        double hl = pSeg->vertex[TR_SL].z +
                    (pSeg->vertex[TR_EL].z - pSeg->vertex[TR_SL].z) * t;
        double hr = pSeg->vertex[TR_SR].z +
                    (pSeg->vertex[TR_ER].z - pSeg->vertex[TR_SR].z) * t;
        norm = -Vec3d(pSeg->rgtSideNormal);
        norm.z = (hr - hl) / pSeg->width;
    }
    else
    {
        double d = pSeg->type == TR_LFT ? 1 : -1;
        double deltaAng = d * toStart / pSeg->radius;
        double ang = pSeg->angle[TR_ZS] - PI / 2 + deltaAng;
        double c = cos(ang);
        double s = sin(ang);
        double r = d * pSeg->radius;
        t = toStart / pSeg->length;
        double hl = pSeg->vertex[TR_SL].z +
                    (pSeg->vertex[TR_EL].z - pSeg->vertex[TR_SL].z) * t;
        double hr = pSeg->vertex[TR_SR].z +
                    (pSeg->vertex[TR_ER].z - pSeg->vertex[TR_SR].z) * t;
        pt = Vec3d(pSeg->center.x + c * r, pSeg->center.y + s * r, (hl + hr) / 2);
        norm = Vec3d(c, s, (hr - hl) / pSeg->width);
    }
}
