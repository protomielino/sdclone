#ifndef _AVOIDANCE_H_
#define _AVOIDANCE_H_

#include "Opponent.h"
#include "Driver.h"
#include "Span.h"

#include <car.h>

class TDriver;

class Avoidance
{
public:
    enum
    {
        PRI_IGNORE	= 0,
        PRI_GENERIC	= 1,
        PRI_TRAFFIC = 3,
        PRI_LAPPING	= 5,
        PRI_MAX		= 10,
    };

    struct Info
    {
        int				flags;
        int				avoidAhead;
        int				avoidToSide;
        int				avoidLapping;
        int				nearbyCars;

        double			k;
        double			nextK;
        double			spdL;
        double			spdR;
        double			spdF;
        double			accF;
        double			minLSideDist;
        double			minRSideDist;
        double			minLDist;
        double			minRDist;
        double			bestPathOffs;

        Opponent::Info*	pClosestAhead;

        Info()
        {
            Init();
        }

        void	Init()
        {
            flags = 0;
            avoidAhead = 0;
            avoidToSide = 0;
            avoidLapping = 0;
            nearbyCars = 0;
            k = 0;
            nextK = 0;
            spdL = 200;
            spdR = 200;
            spdF = 200;
            accF = 100;
            minLSideDist = INT_MAX;
            minRSideDist = INT_MAX;
            minLDist = INT_MAX;
            minRDist = INT_MAX;
            bestPathOffs = 0;
            pClosestAhead = 0;
        }
    };

public:
    Avoidance();
    virtual ~Avoidance();

    virtual Vec2d	calcTarget(const Info& ai, const CarElt* pCar, const TDriver& me );
};

#endif
