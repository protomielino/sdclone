/***************************************************************************

    file         : track.cpp
    created      : Sun Dec 24 12:14:18 CET 2000
    copyright    : (C) 2000 by Eric Espie
    email        : eric.espie@torcs.org
    version      : $Id$

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/** @file

    @author	<a href=mailto:eric.espie@torcs.org>Eric Espie</a>
    @version	$Id$
*/

#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#ifndef WIN32
#include <unistd.h>
#endif
#include <cmath>

#include <robottools.h>
#include <tgfclient.h>
#include <track.h>

#include "ac3d.h"
#include "util.h"

#include "trackgen.h"

struct tTexElt
{
    char *name;
    char *namebump;
    char *nameraceline;
    int mipmap; // Not yet used.
    unsigned int texid;
    tTexElt *next;
};

struct tDispElt
{
    int start;
    int nb;
    int surfType;
    char *name;
    int id;
    tTexElt *texture;
    tDispElt *next;
};

struct tGroup
{
    int nb;
    tDispElt *dispList;
};

static tGroup *Groups;
static int ActiveGroups;
static int GroupNb;

#define LMAX TrackStep

static float *trackvertices;
static float *tracktexcoord;
static unsigned int *trackindices;

static tdble TrackStep = 10.0;

#ifndef sqr
#define sqr(x) ((x) * (x))
#endif

static void initPits(tTrack *theTrack, void *TrackHandle, tTrackPitInfo *pits)
{
    tTrackSeg *curMainSeg;
    tTrackSeg *curPitSeg = nullptr;
    tTrackSeg *pitBuildingsStart = nullptr;
    const char *segName;
    tTrkLocPos curPos;
    int i;
    bool found = false;
    bool changeSeg = false;
    tdble offset = 0;
    tdble toStart = 0;
    static char path2[256];

    sprintf(path2, "%s/%s", TRK_SECT_MAIN, TRK_SECT_PITS);

    // In TR_PIT_NO_BUILDING, the pit positions are located exactly the same,
    // as in the TR_PIT_ON_TRACK_SIDE. It differs only later, showing/not showing
    // the buildings.
    switch (pits->type)
    {
    case TR_PIT_ON_TRACK_SIDE:
    case TR_PIT_NO_BUILDING:
        pits->driversPitsNb = MIN(pits->nPitSeg, pits->nMaxPits);
        pits->driversPits = (tTrackOwnPit *)calloc(pits->driversPitsNb, sizeof(tTrackOwnPit));

        curPos.type = TR_TOMIDDLE;
        // TR_LPOS_MAIN; //NB: TR_LPOS_MAIN not handled by RtTrackLocal2Global!

        segName = GfParmGetStr(TrackHandle, path2, TRK_ATT_BUILDINGS_START, nullptr);
        if (segName != 0)
        {
            pitBuildingsStart = theTrack->seg;
            found = false;
            for (i = 0; i <= theTrack->nseg; i++)
            {
                if (!strcmp(segName, pitBuildingsStart->name))
                {
                    found = true;
                }
                else if (found)
                {
                    pitBuildingsStart = pitBuildingsStart->next;
                    break;
                }
                pitBuildingsStart = pitBuildingsStart->prev;
            }

            if (!found)
            {
                pitBuildingsStart = nullptr;
            }
        }

        if (pitBuildingsStart == nullptr)
        {
            if (pits->pitStart == nullptr)
            {
                GfLogError("Can't find pit start\n");
                exit(1);
            }
            pitBuildingsStart = pits->pitStart;
        }

        curMainSeg = pitBuildingsStart->prev;

        changeSeg = true;
        offset = 0;
        toStart = 0;
        i = 0;
        while (i < pits->driversPitsNb)
        {
            if (changeSeg)
            {
                changeSeg = false;
                offset = 0;
                curMainSeg = curMainSeg->next;

                switch (pits->side)
                {
                case TR_RGT:
                    curPitSeg = curMainSeg->rside;
                    if (curPitSeg->rside)
                    {
                        offset = curPitSeg->width;
                        curPitSeg = curPitSeg->rside;
                    }
                    break;

                case TR_LFT:
                    curPitSeg = curMainSeg->lside;
                    if (curPitSeg->lside)
                    {
                        offset = curPitSeg->width;
                        curPitSeg = curPitSeg->lside;
                    }
                    break;
                }

                curPos.seg = curMainSeg;
                if (toStart >= curMainSeg->length)
                {
                    toStart -= curMainSeg->length;
                    changeSeg = true;
                    continue;
                }
            } // if changeSeg

            /* Not the real position but the start and border one
             * instead of center */
            // RtTrackLocal2Global expects toStart as a length in meters for straight,
            // and as an angle in radian for curves
            // TODO(kilo)
            // Proper handling of this should enable non-linear pitlanes.
            // Postponed after 2.0
#if 0
                tdble pitCenter = toStart + pits->len / 2.0;
                switch(curMainSeg->type) {
                    case TR_STR:
                        pits->driversPits[i].pos.toStart = pitCenter;
                        break;

                    case TR_LFT:
                    case TR_RGT:
                        pits->driversPits[i].pos.toStart = pitCenter / curMainSeg->radius;
                        break;
                }
#endif
            // TODO(kilo) get rid of following line when above feature is ready
            curPos.toStart = toStart;

            switch (pits->side)
            {
            case TR_RGT:
                curPos.toRight = -offset - RtTrackGetWidth(curPitSeg, toStart);
                curPos.toLeft = curMainSeg->width - curPos.toRight;
                curPos.toMiddle = -curMainSeg->width / 2.0 + curPos.toRight;
                break;

            case TR_LFT:
                curPos.toLeft = -offset - RtTrackGetWidth(curPitSeg, toStart);
                curPos.toRight = curMainSeg->width - curPos.toLeft;
                curPos.toMiddle = curMainSeg->width / 2.0 - curPos.toLeft;
                break;
            }

            memcpy(&(pits->driversPits[i].pos), &curPos, sizeof(curPos));
            toStart += pits->len;
            if (toStart >= curMainSeg->length)
            {
                toStart -= curMainSeg->length;
                changeSeg = true;
            }

            i++;

        } // while i
        break;

    case TR_PIT_ON_SEPARATE_PATH:
        break;

    case TR_PIT_NONE:
        break;
    }
}

/** Bug fix:
    If width of border decreased, additional vertices are needed to avoid missing surfaces
    Implementation:
    Compares old end vertices and new start vertices, set startNeeded if one is different
**/
int InitScene(tTrack *Track, void *TrackHandle, bool bump, bool raceline, bool bridge)
{
    int i;
    tTrackSeg *seg;
    tTrackSeg *lastSeg = nullptr;
    tTrackSeg *mseg;
    int nbvert;
    tdble width;
    tdble anz, ts = 0;
    tdble radiusr, radiusl;
    tdble step;
    tTrkLocPos trkpos;
    tdble x, y, z;
    tdble x2, y2, z2;
    tdble x3, y3, z3;
    bool startNeeded;

    tdble xprev = 0;
    tdble yprev = 0;

    tDispElt *aDispElt = nullptr;
    unsigned int prevTexId;
    unsigned int curTexId = 0;
    int curTexType = 0;
    int curTexLink = 0;
    tdble curTexOffset = 0;
    tdble curTexSeg;
    tdble curTexSize = 0;
    tdble curHeight;
    int prevSurfType = 0x10;
    int curSurfType = 0x10; // shaded single sided polygon
    tTexElt *texList = nullptr;
    tTexElt *curTexElt = nullptr;
    tTrackBarrier *curBarrier;
    tdble texLen;
    tdble texStep;
    tdble texMaxT = 0;
    tTrackPitInfo *pits;
    static int GenTexId = 1;
    tdble runninglentgh;

    tdble tmWidth = Track->graphic.turnMarksInfo.width;
    tdble tmHeight = Track->graphic.turnMarksInfo.height;
    tdble tmVSpace = Track->graphic.turnMarksInfo.vSpace;
    tdble tmHSpace = Track->graphic.turnMarksInfo.hSpace;
    char buf[256];
    bool hasBorder;
    tDispElt *theCurDispElt = nullptr;
    char sname[256];
    //	float		dmax = 0;

#define LG_STEP_MAX 50.0

    printf("++++++++++++ Track ++++++++++++\n");
    printf("name      = %s\n", Track->name);
    printf("authors   = %s\n", Track->authors);
    printf("filename  = %s\n", Track->filename);
    printf("nseg      = %d\n", Track->nseg);
    printf("version   = %d\n", Track->version);
    printf("length    = %f\n", Track->length);
    printf("width     = %f\n", Track->width);
    printf("pits segs = %d\n", Track->pits.nPitSeg);
    printf("pits      = %d\n", Track->pits.nMaxPits);
    printf("XSize     = %f\n", Track->max.x);
    printf("YSize     = %f\n", Track->max.y);
    printf("ZSize     = %f\n", Track->max.z);

    tdble delatx = Track->seg->next->vertex[TR_SL].x - Track->seg->vertex[TR_EL].x;
    tdble delaty = Track->seg->next->vertex[TR_SL].y - Track->seg->vertex[TR_EL].y;
    tdble delatz = Track->seg->next->vertex[TR_SL].z - Track->seg->vertex[TR_EL].z;
    tdble delata = Track->seg->next->angle[TR_ZS] - Track->seg->angle[TR_ZE];
    NORM_PI_PI(delata);

    printf("Delta X   = %f\n", delatx);
    printf("Delta Y   = %f\n", delaty);
    printf("Delta Z   = %f\n", delatz);
    printf("Delta Ang = %f (%f)\n", delata, RAD2DEG(delata));

    /*     if ((fabs(delatx) > 1.0) || (fabs(delaty) > 1.0) || (fabs(delatz) > 1.0)) { */
    /* 	exit(0); */
    /*     } */

    Groups = (tGroup *)calloc(Track->nseg, sizeof(tGroup));
    ActiveGroups = 0;
    GroupNb = Track->nseg;

    width = Track->width;
    // wi2 = width / 2.0; // Never used.

    double rlWidthScale = 1.0;
    double rlOffset = 0.0;

    if (raceline)
    {
        double SideDistExt = GfParmGetNum(TrackHandle, TRK_SECT_MAIN, TRK_ATT_RLEXT, nullptr, 2.0);
        double SideDistInt = GfParmGetNum(TrackHandle, TRK_SECT_MAIN, TRK_ATT_RLINT, nullptr, 2.0);
        rlWidthScale = GfParmGetNum(TrackHandle, TRK_SECT_MAIN, TRK_ATT_RLWIDTHSCALE, nullptr, 1.0);
        rlOffset = (1.0 - 1.0 / rlWidthScale) / 2.0;
        generateRaceLine(Track, SideDistExt, SideDistInt);
    }

    trkpos.type = TR_LPOS_MAIN;

#define SETTEXTURE(texname, texnamebump, texnameraceline, _mipmap)                                                  \
    do                                                                                                              \
    {                                                                                                               \
        int found = 0;                                                                                              \
        curTexElt = texList;                                                                                        \
        if (curTexElt == nullptr)                                                                                   \
        {                                                                                                           \
            curTexElt = (tTexElt *)calloc(1, sizeof(tTexElt));                                                      \
            curTexElt->next = curTexElt;                                                                            \
            texList = curTexElt;                                                                                    \
            curTexElt->name = strdup(texname);                                                                      \
            curTexElt->namebump = strdup(texnamebump);                                                              \
            curTexElt->nameraceline = strdup(texnameraceline);                                                      \
            curTexElt->mipmap = _mipmap;                                                                            \
            curTexElt->texid = GenTexId++;                                                                          \
        }                                                                                                           \
        else                                                                                                        \
        {                                                                                                           \
            do                                                                                                      \
            {                                                                                                       \
                curTexElt = curTexElt->next;                                                                        \
                if (strcmp(texname, curTexElt->name) == 0)                                                          \
                {                                                                                                   \
                    found = 1;                                                                                      \
                    break;                                                                                          \
                }                                                                                                   \
            } while (curTexElt != texList);                                                                         \
            if (!found)                                                                                             \
            {                                                                                                       \
                curTexElt = (tTexElt *)calloc(1, sizeof(tTexElt));                                                  \
                curTexElt->next = texList->next;                                                                    \
                texList->next = curTexElt;                                                                          \
                texList = curTexElt;                                                                                \
                curTexElt->name = strdup(texname);                                                                  \
                curTexElt->namebump = strdup(texnamebump);                                                          \
                curTexElt->nameraceline = strdup(texnameraceline);                                                  \
                curTexElt->mipmap = _mipmap;                                                                        \
                curTexElt->texid = GenTexId++;                                                                      \
            }                                                                                                       \
        }                                                                                                           \
        curTexId = curTexElt->texid;                                                                                \
    } while (0)

#define NBELTLIST 1

#define NEWDISPLIST(texchange, _name, _id)                                                                          \
    do                                                                                                              \
    {                                                                                                               \
        if (!bump || (*(curTexElt->namebump) != 0))                                                                 \
        {                                                                                                           \
            if (theCurDispElt != nullptr)                                                                           \
            {                                                                                                       \
                startNeeded = texchange;                                                                            \
                if (theCurDispElt->start != nbvert)                                                                 \
                {                                                                                                   \
                    theCurDispElt->nb = nbvert - theCurDispElt->start;                                              \
                    theCurDispElt = aDispElt = (tDispElt *)malloc(sizeof(tDispElt));                                \
                    aDispElt->start = nbvert;                                                                       \
                    aDispElt->nb = 0;                                                                               \
                    aDispElt->name = strdup(_name);                                                                 \
                    aDispElt->id = _id;                                                                             \
                    aDispElt->texture = curTexElt;                                                                  \
                    aDispElt->surfType = curSurfType;                                                               \
                    if (Groups[_id].nb == 0)                                                                        \
                    {                                                                                               \
                        ActiveGroups++;                                                                             \
                        aDispElt->next = aDispElt;                                                                  \
                        Groups[_id].dispList = aDispElt;                                                            \
                    }                                                                                               \
                    else                                                                                            \
                    {                                                                                               \
                        aDispElt->next = Groups[_id].dispList->next;                                                \
                        Groups[_id].dispList->next = aDispElt;                                                      \
                        Groups[_id].dispList = aDispElt;                                                            \
                    }                                                                                               \
                    Groups[_id].nb++;                                                                               \
                }                                                                                                   \
                else                                                                                                \
                {                                                                                                   \
                    aDispElt->texture = curTexElt;                                                                  \
                    aDispElt->surfType = curSurfType;                                                               \
                }                                                                                                   \
            }                                                                                                       \
            else                                                                                                    \
            {                                                                                                       \
                theCurDispElt = aDispElt = (tDispElt *)malloc(sizeof(tDispElt));                                    \
                aDispElt->start = nbvert;                                                                           \
                aDispElt->nb = 0;                                                                                   \
                aDispElt->name = strdup(_name);                                                                     \
                aDispElt->id = _id;                                                                                 \
                aDispElt->texture = curTexElt;                                                                      \
                aDispElt->surfType = curSurfType;                                                                   \
                aDispElt->next = aDispElt;                                                                          \
                Groups[_id].dispList = aDispElt;                                                                    \
                Groups[_id].nb++;                                                                                   \
                ActiveGroups++;                                                                                     \
            }                                                                                                       \
            /* fprintf(stderr, "%s %d (%s)\n", _name, _id, curTexElt->name); */                                     \
        }                                                                                                           \
    } while (0)

#define CHECKDISPLIST(mat, name, id, off)                                                                           \
    do                                                                                                              \
    {                                                                                                               \
        const char *texname;                                                                                        \
        const char *texnamebump;                                                                                    \
        const char *texnameraceline;                                                                                \
        int mipmap;                                                                                                 \
        static char path_[256];                                                                                     \
        if (Track->version < 4)                                                                                     \
        {                                                                                                           \
            sprintf(path_, "%s/%s/%s", TRK_SECT_SURFACES, TRK_LST_SURF, mat);                                       \
        }                                                                                                           \
        else                                                                                                        \
        {                                                                                                           \
            sprintf(path_, "%s/%s", TRK_SECT_SURFACES, mat);                                                        \
        }                                                                                                           \
        texnamebump = GfParmGetStr(TrackHandle, path_, TRK_ATT_BUMPNAME, "");                                       \
        texnameraceline = GfParmGetStr(TrackHandle, path_, TRK_ATT_RACELINENAME, "raceline.png");                   \
        texname = GfParmGetStr(TrackHandle, path_, TRK_ATT_TEXTURE, "tr-asphalt.png");                              \
        mipmap = (int)GfParmGetNum(TrackHandle, path_, TRK_ATT_TEXMIPMAP, nullptr, 0);                              \
        SETTEXTURE(texname, texnamebump, texnameraceline, mipmap);                                                  \
        if ((curTexId != prevTexId) || (curSurfType != prevSurfType) || startNeeded)                                \
        {                                                                                                           \
            const char *textype;                                                                                    \
            if (bump)                                                                                               \
            {                                                                                                       \
                curTexType = 1;                                                                                     \
                curTexLink = 1;                                                                                     \
                curTexOffset = -off;                                                                                \
                curTexSize = GfParmGetNum(TrackHandle, path_, TRK_ATT_BUMPSIZE, nullptr, 20.0);                     \
            }                                                                                                       \
            else                                                                                                    \
            {                                                                                                       \
                textype = GfParmGetStr(TrackHandle, path_, TRK_ATT_TEXTYPE, "continuous");                          \
                if (strcmp(textype, "continuous") == 0)                                                             \
                    curTexType = 1;                                                                                 \
                else                                                                                                \
                    curTexType = 0;                                                                                 \
                textype = GfParmGetStr(TrackHandle, path_, TRK_ATT_TEXLINK, TRK_VAL_YES);                           \
                if (strcmp(textype, TRK_VAL_YES) == 0)                                                              \
                    curTexLink = 1;                                                                                 \
                else                                                                                                \
                    curTexLink = 0;                                                                                 \
                textype = GfParmGetStr(TrackHandle, path_, TRK_ATT_TEXSTARTBOUNDARY, TRK_VAL_NO);                   \
                if (strcmp(textype, TRK_VAL_YES) == 0)                                                              \
                    curTexOffset = -off;                                                                            \
                else                                                                                                \
                    curTexOffset = 0;                                                                               \
                curTexSize = GfParmGetNum(TrackHandle, path_, TRK_ATT_TEXSIZE, nullptr, 20.0);                      \
            }                                                                                                       \
            prevTexId = curTexId;                                                                                   \
            prevSurfType = curSurfType;                                                                             \
            NEWDISPLIST(true, name, id);                                                                            \
        }                                                                                                           \
    } while (0)

#define CHECKDISPLIST2(texture, mipmap, name, id)                                                                   \
    do                                                                                                              \
    {                                                                                                               \
        char texname[300];                                                                                          \
        sprintf(texname, "%s.png", texture);                                                                        \
        SETTEXTURE(texname, "", "", mipmap);                                                                        \
        if (curTexId != prevTexId)                                                                                  \
        {                                                                                                           \
            prevTexId = curTexId;                                                                                   \
            NEWDISPLIST(true, name, id);                                                                            \
        }                                                                                                           \
    } while (0)

#define CHECKDISPLIST3(texture, mipmap, name, id)                                                                   \
    do                                                                                                              \
    {                                                                                                               \
        SETTEXTURE(texture, "", "", mipmap);                                                                        \
        if (curTexId != prevTexId)                                                                                  \
        {                                                                                                           \
            prevTexId = curTexId;                                                                                   \
            NEWDISPLIST(true, name, id);                                                                            \
        }                                                                                                           \
    } while (0)

#define CLOSEDISPLIST()                                                                                             \
    do                                                                                                              \
    {                                                                                                               \
        theCurDispElt->nb = nbvert - theCurDispElt->start;                                                          \
    } while (0)

#define SETPOINT(t1, t2, x, y, z)                                                                                   \
    do                                                                                                              \
    {                                                                                                               \
        if (*(curTexElt->name) != 0)                                                                                \
        {                                                                                                           \
            tracktexcoord[2 * nbvert] = t1;                                                                         \
            tracktexcoord[2 * nbvert + 1] = t2;                                                                     \
            trackvertices[3 * nbvert] = x;                                                                          \
            trackvertices[3 * nbvert + 1] = y;                                                                      \
            trackvertices[3 * nbvert + 2] = z;                                                                      \
            trackindices[nbvert] = nbvert;                                                                          \
            ++nbvert;                                                                                               \
        }                                                                                                           \
    } while (0)

#define SETPOINT2(t1, t2, x, y, z)                                                                                  \
    do                                                                                                              \
    {                                                                                                               \
        if (*(curTexElt->name) != 0)                                                                                \
        {                                                                                                           \
            tracktexcoord[2 * nbvert] = t1;                                                                         \
            tracktexcoord[2 * nbvert + 1] = t2;                                                                     \
            trackvertices[3 * nbvert] = x;                                                                          \
            trackvertices[3 * nbvert + 1] = y;                                                                      \
            trackvertices[3 * nbvert + 2] = z;                                                                      \
            trackindices[nbvert] = nbvert;                                                                          \
            ++nbvert;                                                                                               \
            printf("x=%f y=%f z=%f  u=%f v=%f\n", (tdble)(x), (tdble)(y), (tdble)(z), (tdble)(t1), (tdble)(t2));    \
        }                                                                                                           \
    } while (0)

    /* Count the number of vertice to allocate */
    nbvert = 0;
    for (i = 0, seg = Track->seg->next; i < Track->nseg; i++, seg = seg->next)
    {
        nbvert++;
        switch (seg->type)
        {
        case TR_STR:
            nbvert += (int)(seg->length / LMAX);
            break;
        case TR_LFT:
            nbvert += (int)(seg->arc * (seg->radiusr) / LMAX);
            break;
        case TR_RGT:
            nbvert += (int)(seg->arc * (seg->radiusl) / LMAX);
            break;
        }
    }

    nbvert++;
    nbvert *= 30;
    nbvert += 58; /* start bridge */
    nbvert += 12 + 10 * Track->pits.driversPitsNb;
    // TODO: Investigate if this is a feasible solution, nbvert was perviously += 1000.
    nbvert += 10000; /* safety margin */
    printf("=== Indices Array Size   = %d\n", nbvert);
    printf("=== Vertex Array Size    = %d\n", nbvert * 3);
    printf("=== Tex Coord Array Size = %d\n", nbvert * 2);
    trackindices = (unsigned int *)malloc(sizeof(unsigned int) * nbvert);
    trackvertices = (float *)malloc(sizeof(GLfloat) * (nbvert * 3));
    tracktexcoord = (float *)malloc(sizeof(GLfloat) * (nbvert * 2));

    nbvert = 0;

    /* Main track */
    prevTexId = 0;
    texLen = 0;
    startNeeded = true;
    runninglentgh = 0;
    for (i = 0, seg = Track->seg->next; i < Track->nseg; i++, seg = seg->next)
    {
        CHECKDISPLIST(seg->surface->material, "tkMn", i, seg->lgfromstart);
        if (!curTexLink)
        {
            curTexSeg = 0;
        }
        else
        {
            curTexSeg = seg->lgfromstart;
        }
        curTexSeg += curTexOffset;

        if (raceline)
        {
            // Required if some smaller than width tiled texture is used.
            curTexSize = seg->width;
        }

        texLen = curTexSeg / curTexSize;
        if (startNeeded || (runninglentgh > LG_STEP_MAX))
        {
            NEWDISPLIST(false, "tkMn", i);
            runninglentgh = 0;
            ts = 0;
            if (raceline)
            {
                // Required if some smaller than width tiled texture is used.
                texMaxT = 1.0;
            }
            else
            {
                // Normal case
                texMaxT = (curTexType == 1 ? width / curTexSize : 1.0 + floor(width / curTexSize));
            }

            double rlto = getTexureOffset(seg->lgfromstart) / rlWidthScale;

            SETPOINT(texLen, texMaxT - rlOffset + rlto, seg->vertex[TR_SL].x, seg->vertex[TR_SL].y, seg->vertex[TR_SL].z);
            SETPOINT(texLen, 0.0 + rlOffset + rlto, seg->vertex[TR_SR].x, seg->vertex[TR_SR].y, seg->vertex[TR_SR].z);
        }

        switch (seg->type)
        {
        case TR_STR:
            ts = LMAX;
            texStep = LMAX / curTexSize;
            trkpos.seg = seg;
            while (ts < seg->length)
            {
                texLen += texStep;
                trkpos.toStart = ts;

                trkpos.toRight = width;
                RtTrackLocal2Global(&trkpos, &x, &y, TR_TORIGHT);

                double rlto = getTexureOffset(seg->lgfromstart + ts) / rlWidthScale;

                SETPOINT(texLen, texMaxT - rlOffset + rlto, x, y, RtTrackHeightL(&trkpos));

                trkpos.toRight = 0;
                RtTrackLocal2Global(&trkpos, &x, &y, TR_TORIGHT);

                SETPOINT(texLen, 0.0 + rlOffset + rlto, x, y, RtTrackHeightL(&trkpos));

                ts += LMAX;
            }
            ts = seg->length;
            break;
        case TR_LFT:
            step = LMAX / (seg->radiusr);
            texStep = step * seg->radius / curTexSize;
            anz = seg->angle[TR_ZS] + step;
            ts = step;
            radiusr = seg->radiusr;
            radiusl = seg->radiusl;
            trkpos.seg = seg;
            while (anz < seg->angle[TR_ZE])
            {
                texLen += texStep;
                trkpos.toStart = ts;

                double rlto = getTexureOffset(seg->lgfromstart + ts * seg->radius) / rlWidthScale;
                /* left */
                trkpos.toRight = width;
                SETPOINT(texLen, texMaxT - rlOffset + rlto, seg->center.x + radiusl * sin(anz), seg->center.y - radiusl * cos(anz), RtTrackHeightL(&trkpos));

                /* right */
                trkpos.toRight = 0;
                SETPOINT(texLen, 0.0 + rlOffset + rlto, seg->center.x + radiusr * sin(anz), seg->center.y - radiusr * cos(anz), RtTrackHeightL(&trkpos));

                ts += step;
                anz += step;
            }
            ts = seg->arc;
            break;
        case TR_RGT:
            step = LMAX / (seg->radiusl);
            texStep = step * seg->radius / curTexSize;
            anz = seg->angle[TR_ZS] - step;
            ts = step;
            radiusr = seg->radiusr;
            radiusl = seg->radiusl;
            trkpos.seg = seg;
            while (anz > seg->angle[TR_ZE])
            {
                texLen += texStep;
                trkpos.toStart = ts;

                double rlto = getTexureOffset(seg->lgfromstart + ts * seg->radius) / rlWidthScale;
                /* left */
                trkpos.toRight = width;
                SETPOINT(texLen, texMaxT - rlOffset + rlto, seg->center.x - radiusl * sin(anz), seg->center.y + radiusl * cos(anz), RtTrackHeightL(&trkpos));

                /* right */
                trkpos.toRight = 0;
                SETPOINT(texLen, 0 + rlOffset + rlto, seg->center.x - radiusr * sin(anz), seg->center.y + radiusr * cos(anz), RtTrackHeightL(&trkpos));

                ts += step;
                anz -= step;
            }
            ts = seg->arc;
            break;
        }
        texLen = (curTexSeg + seg->length) / curTexSize;

        double rlto = getTexureOffset(seg->lgfromstart + seg->length) / rlWidthScale;
        SETPOINT(texLen, texMaxT - rlOffset + rlto, seg->vertex[TR_EL].x, seg->vertex[TR_EL].y, seg->vertex[TR_EL].z);
        SETPOINT(texLen, 0 + rlOffset + rlto, seg->vertex[TR_ER].x, seg->vertex[TR_ER].y, seg->vertex[TR_ER].z);

        startNeeded = false;
        runninglentgh += seg->length;
    }

    if (raceline)
    {
        CLOSEDISPLIST();
        printf("=== Indices really used = %d\n", nbvert);
        return 0;
    }

    /* Right Border */
    for (int j = 0; j < 3; j++)
    {
        prevTexId = 0;
        texLen = 0;
        startNeeded = true;
        runninglentgh = 0;
        sprintf(sname, "t%dRB", j);
        for (i = 0, mseg = Track->seg->next; i < Track->nseg; i++, mseg = mseg->next)
        {
            if ((mseg->rside != nullptr) && (mseg->rside->type2 == TR_RBORDER))
            {
                seg = mseg->rside;
                CHECKDISPLIST(seg->surface->material, sname, i, mseg->lgfromstart);
                if (!curTexLink)
                {
                    curTexSeg = 0;
                }
                else
                {
                    curTexSeg = mseg->lgfromstart;
                }
                curTexSeg += curTexOffset;
                texLen = curTexSeg / curTexSize;
                if (startNeeded || (runninglentgh > LG_STEP_MAX))
                {
                    NEWDISPLIST(false, sname, i);
                    runninglentgh = 0;
                    ts = 0;

                    width = RtTrackGetWidth(seg, ts);
                    texMaxT = (curTexType == 1 ? width / curTexSize : 1.0 + floor(width / curTexSize));

                    switch (seg->style)
                    {
                    case TR_PLAN:
                        if (j == 0)
                        {
                            SETPOINT(texLen, 0, seg->vertex[TR_SL].x, seg->vertex[TR_SL].y, seg->vertex[TR_SL].z);
                            SETPOINT(texLen, texMaxT, seg->vertex[TR_SR].x, seg->vertex[TR_SR].y, seg->vertex[TR_SR].z);
                        }
                        break;
                    case TR_CURB:
                        switch (j)
                        {
                        case 0:
                            if (!mseg->prev->rside || (mseg->prev->rside->type2 != TR_RBORDER) || (mseg->prev->rside->style != TR_CURB))
                            {
                                SETPOINT(texLen, 0, seg->vertex[TR_SL].x, seg->vertex[TR_SL].y, seg->vertex[TR_SL].z - 0.1);
                                SETPOINT(texLen, texMaxT, seg->vertex[TR_SR].x, seg->vertex[TR_SR].y, seg->vertex[TR_SR].z);
                            }
                            SETPOINT(texLen, 0, seg->vertex[TR_SL].x, seg->vertex[TR_SL].y, seg->vertex[TR_SL].z);
                            SETPOINT(texLen, texMaxT, seg->vertex[TR_SR].x, seg->vertex[TR_SR].y, seg->vertex[TR_SR].z + seg->height);
                            break;
                        case 1:
                            SETPOINT(texLen, texMaxT, seg->vertex[TR_SR].x, seg->vertex[TR_SR].y, seg->vertex[TR_SR].z + seg->height);
                            SETPOINT(texLen, texMaxT, seg->vertex[TR_SR].x, seg->vertex[TR_SR].y, seg->vertex[TR_SR].z);
                            break;
                        case 2:
                            break;
                        }
                        break;
                    case TR_WALL:
                        switch (j)
                        {
                        case 0:
                            SETPOINT(texLen, 0, seg->vertex[TR_SL].x, seg->vertex[TR_SL].y, seg->vertex[TR_SL].z);
                            SETPOINT(texLen, .33, seg->vertex[TR_SL].x, seg->vertex[TR_SL].y, seg->vertex[TR_SL].z + seg->height);
                            break;
                        case 1:
                            SETPOINT(texLen, 0.33, seg->vertex[TR_SL].x, seg->vertex[TR_SL].y, seg->vertex[TR_SL].z + seg->height);
                            SETPOINT(texLen, 0.66, seg->vertex[TR_SR].x, seg->vertex[TR_SR].y, seg->vertex[TR_SR].z + seg->height);
                            break;
                        case 2:
                            if (!mseg->prev->rside || (mseg->prev->rside->type2 != TR_RBORDER) || (mseg->prev->rside->style != TR_WALL))
                            {
                                SETPOINT(texLen - seg->width / curTexSize, 0.66, seg->vertex[TR_SL].x, seg->vertex[TR_SL].y, seg->vertex[TR_SL].z + seg->height);
                                SETPOINT(texLen - seg->width / curTexSize, 1.00, seg->vertex[TR_SL].x, seg->vertex[TR_SL].y, seg->vertex[TR_SL].z);
                            }
                            SETPOINT(texLen, 0.66, seg->vertex[TR_SR].x, seg->vertex[TR_SR].y, seg->vertex[TR_SR].z + seg->height);
                            SETPOINT(texLen, 1.0, seg->vertex[TR_SR].x, seg->vertex[TR_SR].y, seg->vertex[TR_SR].z);
                            break;
                        }
                        break;
                    case TR_FENCE:
                    case TR_PITBUILDING:
                        // not supported
                        break;
                    }
                }

                switch (seg->type)
                {
                case TR_STR:
                    ts = LMAX;
                    texStep = LMAX / curTexSize;
                    texLen += texStep;
                    trkpos.seg = seg;
                    while (ts < seg->length)
                    {
                        trkpos.toStart = ts;
                        width = RtTrackGetWidth(seg, ts);
                        texMaxT = (curTexType == 1 ? width / curTexSize : 1.0 + floor(width / curTexSize));

                        switch (seg->style)
                        {
                        case TR_PLAN:
                            if (j == 0)
                            {
                                trkpos.toRight = width;
                                RtTrackLocal2Global(&trkpos, &x, &y, TR_TORIGHT);
                                SETPOINT(texLen, 0, x, y, RtTrackHeightL(&trkpos));
                                trkpos.toRight = 0;
                                RtTrackLocal2Global(&trkpos, &x, &y, TR_TORIGHT);
                                SETPOINT(texLen, texMaxT, x, y, RtTrackHeightL(&trkpos));
                            }
                            break;
                        case TR_CURB:
                            switch (j)
                            {
                            case 0:
                                trkpos.toRight = width;
                                RtTrackLocal2Global(&trkpos, &x, &y, TR_TORIGHT);
                                SETPOINT(texLen, 0, x, y, RtTrackHeightL(&trkpos));
                                trkpos.toRight = 0;
                                RtTrackLocal2Global(&trkpos, &x, &y, TR_TORIGHT);
                                SETPOINT(texLen, texMaxT, x, y, RtTrackHeightL(&trkpos) + seg->height);
                                break;
                            case 1:
                                trkpos.toRight = 0;
                                RtTrackLocal2Global(&trkpos, &x, &y, TR_TORIGHT);
                                SETPOINT(texLen, texMaxT, x, y, RtTrackHeightL(&trkpos) + seg->height);
                                SETPOINT(texLen, texMaxT, x, y, RtTrackHeightL(&trkpos));
                                break;
                            case 2:
                                break;
                            }
                            break;
                        case TR_WALL:
                            switch (j)
                            {
                            case 0:
                                trkpos.toRight = width;
                                RtTrackLocal2Global(&trkpos, &x, &y, TR_TORIGHT);
                                SETPOINT(texLen, 0, x, y, RtTrackHeightL(&trkpos));
                                SETPOINT(texLen, 0.33, x, y, RtTrackHeightL(&trkpos) + seg->height);
                                break;
                            case 1:
                                trkpos.toRight = width;
                                RtTrackLocal2Global(&trkpos, &x, &y, TR_TORIGHT);
                                SETPOINT(texLen, 0.33, x, y, RtTrackHeightL(&trkpos) + seg->height);
                                trkpos.toRight = 0;
                                RtTrackLocal2Global(&trkpos, &x, &y, TR_TORIGHT);
                                SETPOINT(texLen, 0.66, x, y, RtTrackHeightL(&trkpos) + seg->height);
                                break;
                            case 2:
                                trkpos.toRight = 0;
                                RtTrackLocal2Global(&trkpos, &x, &y, TR_TORIGHT);
                                SETPOINT(texLen, 0.66, x, y, RtTrackHeightL(&trkpos) + seg->height);
                                SETPOINT(texLen, 1.0, x, y, RtTrackHeightL(&trkpos));
                                break;
                            }
                            break;
                        case TR_FENCE:
                        case TR_PITBUILDING:
                            // not supported
                            break;
                        }

                        ts += LMAX;
                        texLen += texStep;
                    }
                    ts = seg->length;
                    break;
                case TR_LFT:
                    step = LMAX / (mseg->radiusr);
                    texStep = step * mseg->radius / curTexSize;
                    anz = seg->angle[TR_ZS] + step;
                    ts = step;
                    texLen += texStep;
                    radiusr = seg->radiusr;
                    radiusl = seg->radiusl;
                    trkpos.seg = seg;
                    while (anz < seg->angle[TR_ZE])
                    {
                        trkpos.toStart = ts;
                        width = RtTrackGetWidth(seg, ts);
                        texMaxT = (curTexType == 1 ? width / curTexSize : 1.0 + floor(width / curTexSize));

                        switch (seg->style)
                        {
                        case TR_PLAN:
                            if (j == 0)
                            {
                                /* left */
                                trkpos.toRight = width;
                                RtTrackLocal2Global(&trkpos, &x, &y, TR_TORIGHT);
                                SETPOINT(texLen, 0, x, y, RtTrackHeightL(&trkpos));
                                /* right */
                                trkpos.toRight = 0;
                                RtTrackLocal2Global(&trkpos, &x, &y, TR_TORIGHT);
                                SETPOINT(texLen, texMaxT, x, y, RtTrackHeightL(&trkpos));
                            }
                            break;
                        case TR_CURB:
                            switch (j)
                            {
                            case 0:
                                /* left */
                                trkpos.toRight = width;
                                RtTrackLocal2Global(&trkpos, &x, &y, TR_TORIGHT);
                                SETPOINT(texLen, 0, x, y, RtTrackHeightL(&trkpos));
                                /* right */
                                trkpos.toRight = 0;
                                RtTrackLocal2Global(&trkpos, &x, &y, TR_TORIGHT);
                                SETPOINT(texLen, texMaxT, x, y, RtTrackHeightL(&trkpos) + seg->height);
                                break;
                            case 1:
                                trkpos.toRight = 0;
                                RtTrackLocal2Global(&trkpos, &x, &y, TR_TORIGHT);
                                SETPOINT(texLen, texMaxT, x, y, RtTrackHeightL(&trkpos) + seg->height);
                                SETPOINT(texLen, texMaxT, x, y, RtTrackHeightL(&trkpos));
                                break;
                            case 2:
                                break;
                            }
                            break;
                        case TR_WALL:
                            switch (j)
                            {
                            case 0:
                                trkpos.toRight = width;
                                RtTrackLocal2Global(&trkpos, &x, &y, TR_TORIGHT);
                                SETPOINT(texLen, 0, x, y, RtTrackHeightL(&trkpos));
                                SETPOINT(texLen, 0.33, x, y, RtTrackHeightL(&trkpos) + seg->height);
                                break;
                            case 1:
                                /* left */
                                trkpos.toRight = width;
                                RtTrackLocal2Global(&trkpos, &x, &y, TR_TORIGHT);
                                SETPOINT(texLen, 0.33, x, y, RtTrackHeightL(&trkpos) + seg->height);
                                /* right */
                                trkpos.toRight = 0;
                                RtTrackLocal2Global(&trkpos, &x, &y, TR_TORIGHT);
                                SETPOINT(texLen, 0.66, x, y, RtTrackHeightL(&trkpos) + seg->height);
                                break;
                            case 2:
                                trkpos.toRight = 0;
                                RtTrackLocal2Global(&trkpos, &x, &y, TR_TORIGHT);
                                SETPOINT(texLen, 0.66, x, y, RtTrackHeightL(&trkpos) + seg->height);
                                SETPOINT(texLen, 1.0, x, y, RtTrackHeightL(&trkpos));
                                break;
                            }
                            break;
                        case TR_FENCE:
                        case TR_PITBUILDING:
                            // not supported
                            break;
                        }

                        ts += step;
                        texLen += texStep;
                        anz += step;
                    }
                    ts = seg->arc;
                    break;
                case TR_RGT:
                    step = LMAX / (mseg->radiusl);
                    texStep = step * mseg->radius / curTexSize;
                    anz = seg->angle[TR_ZS] - step;
                    ts = step;
                    texLen += texStep;
                    radiusr = seg->radiusr;
                    radiusl = seg->radiusl;
                    trkpos.seg = seg;
                    while (anz > seg->angle[TR_ZE])
                    {
                        trkpos.toStart = ts;
                        width = RtTrackGetWidth(seg, ts);
                        texMaxT = (curTexType == 1 ? width / curTexSize : 1.0 + floor(width / curTexSize));

                        switch (seg->style)
                        {
                        case TR_PLAN:
                            if (j == 0)
                            {
                                /* left */
                                trkpos.toRight = width;
                                RtTrackLocal2Global(&trkpos, &x, &y, TR_TORIGHT);
                                SETPOINT(texLen, 0, x, y, RtTrackHeightL(&trkpos));
                                /* right */
                                trkpos.toRight = 0;
                                RtTrackLocal2Global(&trkpos, &x, &y, TR_TORIGHT);
                                SETPOINT(texLen, texMaxT, x, y, RtTrackHeightL(&trkpos));
                            }
                            break;
                        case TR_CURB:
                            switch (j)
                            {
                            case 0:
                                /* left */
                                trkpos.toRight = width;
                                RtTrackLocal2Global(&trkpos, &x, &y, TR_TORIGHT);
                                SETPOINT(texLen, 0, x, y, RtTrackHeightL(&trkpos));
                                /* right */
                                trkpos.toRight = 0;
                                RtTrackLocal2Global(&trkpos, &x, &y, TR_TORIGHT);
                                SETPOINT(texLen, texMaxT, x, y, RtTrackHeightL(&trkpos) + seg->height);
                                break;
                            case 1:
                                trkpos.toRight = 0;
                                RtTrackLocal2Global(&trkpos, &x, &y, TR_TORIGHT);
                                SETPOINT(texLen, texMaxT, x, y, RtTrackHeightL(&trkpos) + seg->height);
                                SETPOINT(texLen, texMaxT, x, y, RtTrackHeightL(&trkpos));
                                break;
                            case 2:
                                break;
                            }
                            break;
                        case TR_WALL:
                            switch (j)
                            {
                            case 0:
                                trkpos.toRight = width;
                                RtTrackLocal2Global(&trkpos, &x, &y, TR_TORIGHT);
                                SETPOINT(texLen, 0, x, y, RtTrackHeightL(&trkpos));
                                SETPOINT(texLen, 0.33, x, y, RtTrackHeightL(&trkpos) + seg->height);
                                break;
                            case 1:
                                /* left */
                                trkpos.toRight = width;
                                RtTrackLocal2Global(&trkpos, &x, &y, TR_TORIGHT);
                                SETPOINT(texLen, 0.33, x, y, RtTrackHeightL(&trkpos) + seg->height);
                                /* right */
                                trkpos.toRight = 0;
                                RtTrackLocal2Global(&trkpos, &x, &y, TR_TORIGHT);
                                SETPOINT(texLen, 0.66, x, y, RtTrackHeightL(&trkpos) + seg->height);
                                break;
                            case 2:
                                trkpos.toRight = 0;
                                RtTrackLocal2Global(&trkpos, &x, &y, TR_TORIGHT);
                                SETPOINT(texLen, 0.66, x, y, RtTrackHeightL(&trkpos) + seg->height);
                                SETPOINT(texLen, 1.0, x, y, RtTrackHeightL(&trkpos));
                                break;
                            }
                            break;
                        case TR_FENCE:
                        case TR_PITBUILDING:
                            // not supported
                            break;
                        }

                        ts += step;
                        texLen += texStep;
                        anz -= step;
                    }
                    ts = seg->arc;
                    break;
                }
                texLen = (curTexSeg + mseg->length) / curTexSize;

                switch (seg->style)
                {
                case TR_PLAN:
                    if (j == 0)
                    {
                        width = RtTrackGetWidth(seg, ts);
                        texMaxT = (curTexType == 1 ? width / curTexSize : 1.0 + floor(width / curTexSize));
                        SETPOINT(texLen, 0, seg->vertex[TR_EL].x, seg->vertex[TR_EL].y, seg->vertex[TR_EL].z);
                        SETPOINT(texLen, texMaxT, seg->vertex[TR_ER].x, seg->vertex[TR_ER].y, seg->vertex[TR_ER].z);
                    }
                    break;
                case TR_CURB:
                    switch (j)
                    {
                    case 0:
                        width = RtTrackGetWidth(seg, ts);
                        texMaxT = (curTexType == 1 ? width / curTexSize : 1.0 + floor(width / curTexSize));
                        SETPOINT(texLen, 0, seg->vertex[TR_EL].x, seg->vertex[TR_EL].y, seg->vertex[TR_EL].z);
                        SETPOINT(texLen, texMaxT, seg->vertex[TR_ER].x, seg->vertex[TR_ER].y, seg->vertex[TR_ER].z + seg->height);
                        if (mseg->next->rside && ((mseg->next->rside->type2 != TR_RBORDER) || (mseg->next->rside->style != TR_CURB)))
                        {
                            SETPOINT(texLen, 0, seg->vertex[TR_EL].x, seg->vertex[TR_EL].y, seg->vertex[TR_EL].z - 0.1);
                            SETPOINT(texLen, texMaxT, seg->vertex[TR_ER].x, seg->vertex[TR_ER].y, seg->vertex[TR_ER].z);
                        }
                        break;
                    case 1:
                        width = RtTrackGetWidth(seg, ts);
                        texMaxT = (curTexType == 1 ? width / curTexSize : 1.0 + floor(width / curTexSize));
                        SETPOINT(texLen, texMaxT, seg->vertex[TR_ER].x, seg->vertex[TR_ER].y, seg->vertex[TR_ER].z + seg->height);
                        SETPOINT(texLen, texMaxT, seg->vertex[TR_ER].x, seg->vertex[TR_ER].y, seg->vertex[TR_ER].z);
                        break;
                    case 2:
                        break;
                    }
                    break;
                case TR_WALL:
                    switch (j)
                    {
                    case 0:
                        SETPOINT(texLen, 0, seg->vertex[TR_EL].x, seg->vertex[TR_EL].y, seg->vertex[TR_EL].z);
                        SETPOINT(texLen, 0.33, seg->vertex[TR_EL].x, seg->vertex[TR_EL].y, seg->vertex[TR_EL].z + seg->height);
                        break;
                    case 1:
                        SETPOINT(texLen, 0.33, seg->vertex[TR_EL].x, seg->vertex[TR_EL].y, seg->vertex[TR_EL].z + seg->height);
                        SETPOINT(texLen, 0.66, seg->vertex[TR_ER].x, seg->vertex[TR_ER].y, seg->vertex[TR_ER].z + seg->height);
                        break;
                    case 2:
                        SETPOINT(texLen, 0.66, seg->vertex[TR_ER].x, seg->vertex[TR_ER].y, seg->vertex[TR_ER].z + seg->height);
                        SETPOINT(texLen, 1.0, seg->vertex[TR_ER].x, seg->vertex[TR_ER].y, seg->vertex[TR_ER].z);
                        if (mseg->next->rside && ((mseg->next->rside->type2 != TR_RBORDER) || (mseg->next->rside->style != TR_WALL)))
                        {
                            SETPOINT(texLen + seg->width / curTexSize, 0.66, seg->vertex[TR_EL].x, seg->vertex[TR_EL].y, seg->vertex[TR_EL].z + seg->height);
                            SETPOINT(texLen + seg->width / curTexSize, 1.00, seg->vertex[TR_EL].x, seg->vertex[TR_EL].y, seg->vertex[TR_EL].z);
                        }
                        break;
                    }
                    break;
                case TR_FENCE:
                case TR_PITBUILDING:
                    // not supported
                    break;
                }

                startNeeded = false;
                runninglentgh += seg->length;
            }
            else
            {
                startNeeded = true;
            }
        }
    }

    /* Right Side */
    prevTexId = 0;
    texLen = 0;
    startNeeded = true;
    runninglentgh = 0;
    hasBorder = false;
    for (i = 0, mseg = Track->seg->next; i < Track->nseg; i++, mseg = mseg->next)
    {
        if ((mseg->rside != nullptr) && ((mseg->rside->type2 == TR_RSIDE) || (mseg->rside->rside != nullptr)))
        {
            seg = mseg->rside;
            if (seg->rside != nullptr)
            {
                seg = seg->rside;
                if (!hasBorder)
                {
                    startNeeded = true;
                    hasBorder = true;
                }
            }
            else
            {
                if (hasBorder)
                {
                    startNeeded = true;
                    hasBorder = false;
                }
            }

            if (!startNeeded)
            {
                tdble d0 = Distance(lastSeg->vertex[TR_EL].x, lastSeg->vertex[TR_EL].y, lastSeg->vertex[TR_EL].z, seg->vertex[TR_SL].x, seg->vertex[TR_SL].y, seg->vertex[TR_SL].z);
                tdble d1 = Distance(lastSeg->vertex[TR_ER].x, lastSeg->vertex[TR_ER].y, lastSeg->vertex[TR_ER].z, seg->vertex[TR_SR].x, seg->vertex[TR_SR].y, seg->vertex[TR_SR].z);
                if ((d0 > 0.01) || (d1 > 0.01))
                    startNeeded = true;
            }

            CHECKDISPLIST(seg->surface->material, "tkRS", i, mseg->lgfromstart);
            if (!curTexLink)
            {
                curTexSeg = 0;
            }
            else
            {
                curTexSeg = mseg->lgfromstart;
            }
            curTexSeg += curTexOffset;
            texLen = curTexSeg / curTexSize;
            if (startNeeded || (runninglentgh > LG_STEP_MAX))
            {
                NEWDISPLIST(false, "tkRS", i);
                runninglentgh = 0;
                ts = 0;

                width = RtTrackGetWidth(seg, ts);
                texMaxT = (curTexType == 1 ? width / curTexSize : 1.0 + floor(width / curTexSize));

                SETPOINT(texLen, 0, seg->vertex[TR_SL].x, seg->vertex[TR_SL].y, seg->vertex[TR_SL].z);
                SETPOINT(texLen, texMaxT, seg->vertex[TR_SR].x, seg->vertex[TR_SR].y, seg->vertex[TR_SR].z);
            }

            switch (seg->type)
            {
            case TR_STR:
                ts = LMAX;
                texStep = LMAX / curTexSize;
                texLen += texStep;
                trkpos.seg = seg;
                while (ts < seg->length)
                {
                    trkpos.toStart = ts;

                    width = RtTrackGetWidth(seg, ts);
                    texMaxT = (curTexType == 1 ? width / curTexSize : 1.0 + floor(width / curTexSize));

                    trkpos.toRight = width;
                    RtTrackLocal2Global(&trkpos, &x, &y, TR_TORIGHT);
                    SETPOINT(texLen, 0, x, y, RtTrackHeightL(&trkpos));

                    trkpos.toRight = 0;
                    RtTrackLocal2Global(&trkpos, &x, &y, TR_TORIGHT);
                    SETPOINT(texLen, texMaxT, x, y, RtTrackHeightL(&trkpos));

                    ts += LMAX;
                    texLen += texStep;
                }
                ts = seg->length;
                break;
            case TR_LFT:
                step = LMAX / (mseg->radiusr);
                texStep = step * mseg->radius / curTexSize;
                anz = seg->angle[TR_ZS] + step;
                ts = step;
                texLen += texStep;
                radiusr = seg->radiusr;
                radiusl = seg->radiusl;
                trkpos.seg = seg;
                while (anz < seg->angle[TR_ZE])
                {
                    trkpos.toStart = ts;
                    width = RtTrackGetWidth(seg, ts);
                    texMaxT = (curTexType == 1 ? width / curTexSize : 1.0 + floor(width / curTexSize));

                    /* left */
                    trkpos.toRight = width;
                    RtTrackLocal2Global(&trkpos, &x, &y, TR_TORIGHT);
                    SETPOINT(texLen, 0, x, y, RtTrackHeightL(&trkpos));

                    /* right */
                    trkpos.toRight = 0;
                    RtTrackLocal2Global(&trkpos, &x, &y, TR_TORIGHT);
                    SETPOINT(texLen, texMaxT, x, y, RtTrackHeightL(&trkpos));

                    ts += step;
                    texLen += texStep;
                    anz += step;
                }
                ts = seg->arc;
                break;
            case TR_RGT:
                step = LMAX / (mseg->radiusl);
                texStep = step * mseg->radius / curTexSize;
                anz = seg->angle[TR_ZS] - step;
                ts = step;
                texLen += texStep;
                radiusr = seg->radiusr;
                radiusl = seg->radiusl;
                trkpos.seg = seg;
                while (anz > seg->angle[TR_ZE])
                {
                    trkpos.toStart = ts;
                    width = RtTrackGetWidth(seg, ts);
                    texMaxT = (curTexType == 1 ? width / curTexSize : 1.0 + floor(width / curTexSize));

                    /* left */
                    trkpos.toRight = width;
                    RtTrackLocal2Global(&trkpos, &x, &y, TR_TORIGHT);
                    SETPOINT(texLen, 0, x, y, RtTrackHeightL(&trkpos));

                    /* right */
                    trkpos.toRight = 0;
                    RtTrackLocal2Global(&trkpos, &x, &y, TR_TORIGHT);
                    SETPOINT(texLen, texMaxT, x, y, RtTrackHeightL(&trkpos));

                    ts += step;
                    texLen += texStep;
                    anz -= step;
                }
                ts = seg->arc;
                break;
            }
            texLen = (curTexSeg + mseg->length) / curTexSize;

            width = RtTrackGetWidth(seg, ts);
            texMaxT = (curTexType == 1 ? width / curTexSize : 1.0 + floor(width / curTexSize));

            SETPOINT(texLen, 0, seg->vertex[TR_EL].x, seg->vertex[TR_EL].y, seg->vertex[TR_EL].z);
            SETPOINT(texLen, texMaxT, seg->vertex[TR_ER].x, seg->vertex[TR_ER].y, seg->vertex[TR_ER].z);

            startNeeded = false;
            runninglentgh += seg->length;
            lastSeg = seg;
        }
        else
        {
            startNeeded = true;
        }
    }

    /* Left Border */
    for (int j = 0; j < 3; j++)
    {
        prevTexId = 0;
        texLen = 0;
        startNeeded = true;
        runninglentgh = 0;
        sprintf(sname, "t%dLB", j);
        for (i = 0, mseg = Track->seg->next; i < Track->nseg; i++, mseg = mseg->next)
        {
            if ((mseg->lside != nullptr) && (mseg->lside->type2 == TR_LBORDER))
            {
                seg = mseg->lside;
                CHECKDISPLIST(seg->surface->material, sname, i, mseg->lgfromstart);
                if (!curTexLink)
                {
                    curTexSeg = 0;
                }
                else
                {
                    curTexSeg = mseg->lgfromstart;
                }
                curTexSeg += curTexOffset;
                texLen = curTexSeg / curTexSize;
                if (startNeeded || (runninglentgh > LG_STEP_MAX))
                {
                    NEWDISPLIST(false, sname, i);
                    runninglentgh = 0;
                    ts = 0;
                    width = RtTrackGetWidth(seg, ts);
                    texMaxT = (curTexType == 1 ? width / curTexSize : 1.0 + floor(width / curTexSize));
                    switch (seg->style)
                    {
                    case TR_PLAN:
                        if (j == 0)
                        {
                            SETPOINT(texLen, texMaxT, seg->vertex[TR_SL].x, seg->vertex[TR_SL].y, seg->vertex[TR_SL].z);
                            SETPOINT(texLen, 0, seg->vertex[TR_SR].x, seg->vertex[TR_SR].y, seg->vertex[TR_SR].z);
                        }
                        break;
                    case TR_CURB:
                        switch (j)
                        {
                        case 0:
                            SETPOINT(texLen, texMaxT, seg->vertex[TR_SL].x, seg->vertex[TR_SL].y, seg->vertex[TR_SL].z);
                            SETPOINT(texLen, texMaxT, seg->vertex[TR_SL].x, seg->vertex[TR_SL].y, seg->vertex[TR_SL].z + seg->height);
                            break;
                        case 1:
                            if (!mseg->prev->rside || (mseg->prev->rside->type2 != TR_RBORDER) || (mseg->prev->rside->style != TR_CURB))
                            {
                                SETPOINT(texLen, texMaxT, seg->vertex[TR_SL].x, seg->vertex[TR_SL].y, seg->vertex[TR_SL].z);
                                SETPOINT(texLen, 0, seg->vertex[TR_SR].x, seg->vertex[TR_SR].y, seg->vertex[TR_SR].z - 0.1);
                            }
                            SETPOINT(texLen, texMaxT, seg->vertex[TR_SL].x, seg->vertex[TR_SL].y, seg->vertex[TR_SL].z + seg->height);
                            SETPOINT(texLen, 0, seg->vertex[TR_SR].x, seg->vertex[TR_SR].y, seg->vertex[TR_SR].z);
                            break;
                        case 2:
                            break;
                        }
                        break;
                    case TR_WALL:
                        switch (j)
                        {
                        case 0:
                            if (!mseg->prev->rside || (mseg->prev->rside->type2 != TR_RBORDER) || (mseg->prev->rside->style != TR_WALL))
                            {
                                SETPOINT(texLen - seg->width / curTexSize, 1.00, seg->vertex[TR_SR].x, seg->vertex[TR_SR].y, seg->vertex[TR_SR].z);
                                SETPOINT(texLen - seg->width / curTexSize, 0.66, seg->vertex[TR_SR].x, seg->vertex[TR_SR].y, seg->vertex[TR_SR].z + seg->height);
                            }
                            SETPOINT(texLen, 1.0, seg->vertex[TR_SL].x, seg->vertex[TR_SL].y, seg->vertex[TR_SL].z);
                            SETPOINT(texLen, 0.66, seg->vertex[TR_SL].x, seg->vertex[TR_SL].y, seg->vertex[TR_SL].z + seg->height);
                            break;
                        case 1:
                            SETPOINT(texLen, 0.66, seg->vertex[TR_SL].x, seg->vertex[TR_SL].y, seg->vertex[TR_SL].z + seg->height);
                            SETPOINT(texLen, 0.33, seg->vertex[TR_SR].x, seg->vertex[TR_SR].y, seg->vertex[TR_SR].z + seg->height);
                            break;
                        case 2:
                            SETPOINT(texLen, 0.33, seg->vertex[TR_SR].x, seg->vertex[TR_SR].y, seg->vertex[TR_SR].z + seg->height);
                            SETPOINT(texLen, 0.0, seg->vertex[TR_SR].x, seg->vertex[TR_SR].y, seg->vertex[TR_SR].z);
                            break;
                        }
                        break;
                    case TR_FENCE:
                    case TR_PITBUILDING:
                        // not supported
                        break;
                    }
                }

                switch (seg->type)
                {
                case TR_STR:
                    ts = LMAX;
                    texStep = LMAX / curTexSize;
                    texLen += texStep;
                    trkpos.seg = seg;
                    while (ts < seg->length)
                    {
                        trkpos.toStart = ts;
                        width = RtTrackGetWidth(seg, ts);
                        texMaxT = (curTexType == 1 ? width / curTexSize : 1.0 + floor(width / curTexSize));

                        switch (seg->style)
                        {
                        case TR_PLAN:
                            if (j == 0)
                            {
                                trkpos.toRight = width;
                                RtTrackLocal2Global(&trkpos, &x, &y, TR_TORIGHT);
                                SETPOINT(texLen, texMaxT, x, y, RtTrackHeightL(&trkpos));
                                trkpos.toRight = 0;
                                RtTrackLocal2Global(&trkpos, &x, &y, TR_TORIGHT);
                                SETPOINT(texLen, 0, x, y, RtTrackHeightL(&trkpos));
                            }
                            break;
                        case TR_CURB:
                            switch (j)
                            {
                            case 0:
                                trkpos.toRight = width;
                                RtTrackLocal2Global(&trkpos, &x, &y, TR_TORIGHT);
                                SETPOINT(texLen, texMaxT, x, y, RtTrackHeightL(&trkpos));
                                SETPOINT(texLen, texMaxT, x, y, RtTrackHeightL(&trkpos) + seg->height);
                                break;
                            case 1:
                                trkpos.toRight = width;
                                RtTrackLocal2Global(&trkpos, &x, &y, TR_TORIGHT);
                                SETPOINT(texLen, texMaxT, x, y, RtTrackHeightL(&trkpos) + seg->height);
                                trkpos.toRight = 0;
                                RtTrackLocal2Global(&trkpos, &x, &y, TR_TORIGHT);
                                SETPOINT(texLen, 0, x, y, RtTrackHeightL(&trkpos));
                                break;
                            case 2:
                                break;
                            }
                            break;
                        case TR_WALL:
                            switch (j)
                            {
                            case 0:
                                trkpos.toRight = width;
                                RtTrackLocal2Global(&trkpos, &x, &y, TR_TORIGHT);
                                SETPOINT(texLen, 1.0, x, y, RtTrackHeightL(&trkpos));
                                SETPOINT(texLen, 0.66, x, y, RtTrackHeightL(&trkpos) + seg->height);
                                break;
                            case 1:
                                trkpos.toRight = width;
                                RtTrackLocal2Global(&trkpos, &x, &y, TR_TORIGHT);
                                SETPOINT(texLen, 0.66, x, y, RtTrackHeightL(&trkpos) + seg->height);
                                trkpos.toRight = 0;
                                RtTrackLocal2Global(&trkpos, &x, &y, TR_TORIGHT);
                                SETPOINT(texLen, 0.33, x, y, RtTrackHeightL(&trkpos) + seg->height);
                                break;
                            case 2:
                                trkpos.toRight = 0;
                                RtTrackLocal2Global(&trkpos, &x, &y, TR_TORIGHT);
                                SETPOINT(texLen, 0.33, x, y, RtTrackHeightL(&trkpos) + seg->height);
                                SETPOINT(texLen, 0.0, x, y, RtTrackHeightL(&trkpos));
                                break;
                            }
                            break;
                        case TR_FENCE:
                        case TR_PITBUILDING:
                            // not supported
                            break;
                        }

                        ts += LMAX;
                        texLen += texStep;
                    }
                    ts = seg->length;
                    break;
                case TR_LFT:
                    step = LMAX / (mseg->radiusr);
                    texStep = step * mseg->radius / curTexSize;
                    anz = seg->angle[TR_ZS] + step;
                    ts = step;
                    texLen += texStep;
                    radiusr = seg->radiusr;
                    radiusl = seg->radiusl;
                    trkpos.seg = seg;
                    while (anz < seg->angle[TR_ZE])
                    {
                        trkpos.toStart = ts;
                        width = RtTrackGetWidth(seg, ts);
                        texMaxT = (curTexType == 1 ? width / curTexSize : 1.0 + floor(width / curTexSize));

                        switch (seg->style)
                        {
                        case TR_PLAN:
                            if (j == 0)
                            {
                                /* left */
                                trkpos.toRight = width;
                                RtTrackLocal2Global(&trkpos, &x, &y, TR_TORIGHT);
                                SETPOINT(texLen, texMaxT, x, y, RtTrackHeightL(&trkpos));
                                /* right */
                                trkpos.toRight = 0;
                                RtTrackLocal2Global(&trkpos, &x, &y, TR_TORIGHT);
                                SETPOINT(texLen, 0, x, y, RtTrackHeightL(&trkpos));
                            }
                            break;
                        case TR_CURB:
                            switch (j)
                            {
                            case 0:
                                trkpos.toRight = width;
                                RtTrackLocal2Global(&trkpos, &x, &y, TR_TORIGHT);
                                SETPOINT(texLen, texMaxT, x, y, RtTrackHeightL(&trkpos));
                                SETPOINT(texLen, texMaxT, x, y, RtTrackHeightL(&trkpos) + seg->height);
                                break;
                            case 1:
                                /* left */
                                trkpos.toRight = width;
                                RtTrackLocal2Global(&trkpos, &x, &y, TR_TORIGHT);
                                SETPOINT(texLen, texMaxT, x, y, RtTrackHeightL(&trkpos) + seg->height);
                                /* right */
                                trkpos.toRight = 0;
                                RtTrackLocal2Global(&trkpos, &x, &y, TR_TORIGHT);
                                SETPOINT(texLen, 0, x, y, RtTrackHeightL(&trkpos));
                                break;
                            case 2:
                                break;
                            }
                            break;
                        case TR_WALL:
                            switch (j)
                            {
                            case 0:
                                trkpos.toRight = width;
                                RtTrackLocal2Global(&trkpos, &x, &y, TR_TORIGHT);
                                SETPOINT(texLen, 1.0, x, y, RtTrackHeightL(&trkpos));
                                SETPOINT(texLen, 0.66, x, y, RtTrackHeightL(&trkpos) + seg->height);
                                break;
                            case 1:
                                /* left */
                                trkpos.toRight = width;
                                RtTrackLocal2Global(&trkpos, &x, &y, TR_TORIGHT);
                                SETPOINT(texLen, 0.66, x, y, RtTrackHeightL(&trkpos) + seg->height);
                                /* right */
                                trkpos.toRight = 0;
                                RtTrackLocal2Global(&trkpos, &x, &y, TR_TORIGHT);
                                SETPOINT(texLen, 0.33, x, y, RtTrackHeightL(&trkpos) + seg->height);
                                break;
                            case 2:
                                trkpos.toRight = 0;
                                RtTrackLocal2Global(&trkpos, &x, &y, TR_TORIGHT);
                                SETPOINT(texLen, 0.33, x, y, RtTrackHeightL(&trkpos) + seg->height);
                                SETPOINT(texLen, 0.0, x, y, RtTrackHeightL(&trkpos));
                                break;
                            }
                            break;
                        case TR_FENCE:
                        case TR_PITBUILDING:
                            // not supported
                            break;
                        }

                        ts += step;
                        texLen += texStep;
                        anz += step;
                    }
                    ts = seg->arc;
                    break;
                case TR_RGT:
                    step = LMAX / (mseg->radiusl);
                    texStep = step * mseg->radius / curTexSize;
                    anz = seg->angle[TR_ZS] - step;
                    ts = step;
                    texLen += texStep;
                    radiusr = seg->radiusr;
                    radiusl = seg->radiusl;
                    trkpos.seg = seg;
                    while (anz > seg->angle[TR_ZE])
                    {
                        trkpos.toStart = ts;
                        width = RtTrackGetWidth(seg, ts);
                        texMaxT = (curTexType == 1 ? width / curTexSize : 1.0 + floor(width / curTexSize));

                        switch (seg->style)
                        {
                        case TR_PLAN:
                            if (j == 0)
                            {
                                /* left */
                                trkpos.toRight = width;
                                RtTrackLocal2Global(&trkpos, &x, &y, TR_TORIGHT);
                                SETPOINT(texLen, texMaxT, x, y, RtTrackHeightL(&trkpos));
                                /* right */
                                trkpos.toRight = 0;
                                RtTrackLocal2Global(&trkpos, &x, &y, TR_TORIGHT);
                                SETPOINT(texLen, 0, x, y, RtTrackHeightL(&trkpos));
                            }
                            break;
                        case TR_CURB:
                            switch (j)
                            {
                            case 0:
                                trkpos.toRight = width;
                                RtTrackLocal2Global(&trkpos, &x, &y, TR_TORIGHT);
                                SETPOINT(texLen, texMaxT, x, y, RtTrackHeightL(&trkpos));
                                SETPOINT(texLen, texMaxT, x, y, RtTrackHeightL(&trkpos) + seg->height);
                                break;
                            case 1:
                                /* left */
                                trkpos.toRight = width;
                                RtTrackLocal2Global(&trkpos, &x, &y, TR_TORIGHT);
                                SETPOINT(texLen, texMaxT, x, y, RtTrackHeightL(&trkpos) + seg->height);
                                /* right */
                                trkpos.toRight = 0;
                                RtTrackLocal2Global(&trkpos, &x, &y, TR_TORIGHT);
                                SETPOINT(texLen, 0, x, y, RtTrackHeightL(&trkpos));
                                break;
                            case 2:
                                break;
                            }
                            break;
                        case TR_WALL:
                            switch (j)
                            {
                            case 0:
                                trkpos.toRight = width;
                                RtTrackLocal2Global(&trkpos, &x, &y, TR_TORIGHT);
                                SETPOINT(texLen, 1.0, x, y, RtTrackHeightL(&trkpos));
                                SETPOINT(texLen, 0.66, x, y, RtTrackHeightL(&trkpos) + seg->height);
                                break;
                            case 1:
                                /* left */
                                trkpos.toRight = width;
                                RtTrackLocal2Global(&trkpos, &x, &y, TR_TORIGHT);
                                SETPOINT(texLen, 0.66, x, y, RtTrackHeightL(&trkpos) + seg->height);
                                /* right */
                                trkpos.toRight = 0;
                                RtTrackLocal2Global(&trkpos, &x, &y, TR_TORIGHT);
                                SETPOINT(texLen, 0.33, x, y, RtTrackHeightL(&trkpos) + seg->height);
                                break;
                            case 2:
                                trkpos.toRight = 0;
                                RtTrackLocal2Global(&trkpos, &x, &y, TR_TORIGHT);
                                SETPOINT(texLen, 0.33, x, y, RtTrackHeightL(&trkpos) + seg->height);
                                SETPOINT(texLen, 0.0, x, y, RtTrackHeightL(&trkpos));
                                break;
                            }
                            break;
                        case TR_FENCE:
                        case TR_PITBUILDING:
                            // not supported
                            break;
                        }

                        ts += step;
                        texLen += texStep;
                        anz -= step;
                    }
                    ts = seg->arc;
                    break;
                }
                texLen = (curTexSeg + mseg->length) / curTexSize;

                width = RtTrackGetWidth(seg, ts);
                texMaxT = (curTexType == 1 ? width / curTexSize : 1.0 + floor(width / curTexSize));

                switch (seg->style)
                {
                case TR_PLAN:
                    if (j == 0)
                    {
                        SETPOINT(texLen, texMaxT, seg->vertex[TR_EL].x, seg->vertex[TR_EL].y, seg->vertex[TR_EL].z);
                        SETPOINT(texLen, 0, seg->vertex[TR_ER].x, seg->vertex[TR_ER].y, seg->vertex[TR_ER].z);
                    }
                    break;
                case TR_CURB:
                    switch (j)
                    {
                    case 0:
                        SETPOINT(texLen, texMaxT, seg->vertex[TR_EL].x, seg->vertex[TR_EL].y, seg->vertex[TR_EL].z);
                        SETPOINT(texLen, texMaxT, seg->vertex[TR_EL].x, seg->vertex[TR_EL].y, seg->vertex[TR_EL].z + seg->height);
                        break;
                    case 1:
                        SETPOINT(texLen, texMaxT, seg->vertex[TR_EL].x, seg->vertex[TR_EL].y, seg->vertex[TR_EL].z + seg->height);
                        SETPOINT(texLen, 0, seg->vertex[TR_ER].x, seg->vertex[TR_ER].y, seg->vertex[TR_ER].z);
                        if (mseg->next->lside && ((mseg->next->lside->type2 != TR_LBORDER) || (mseg->next->lside->style != TR_CURB)))
                        {
                            SETPOINT(texLen, texMaxT, seg->vertex[TR_EL].x, seg->vertex[TR_EL].y, seg->vertex[TR_EL].z);
                            SETPOINT(texLen, 0, seg->vertex[TR_ER].x, seg->vertex[TR_ER].y, seg->vertex[TR_ER].z - 0.1);
                        }
                        break;
                    case 2:
                        break;
                    }
                    break;
                case TR_WALL:
                    switch (j)
                    {
                    case 0:
                        SETPOINT(texLen, 1.0, seg->vertex[TR_EL].x, seg->vertex[TR_EL].y, seg->vertex[TR_EL].z);
                        SETPOINT(texLen, 0.66, seg->vertex[TR_EL].x, seg->vertex[TR_EL].y, seg->vertex[TR_EL].z + seg->height);
                        if (mseg->next->lside && ((mseg->next->lside->type2 != TR_LBORDER) || (mseg->next->lside->style != TR_WALL)))
                        {
                            SETPOINT(texLen + seg->width / curTexSize, 1.00, seg->vertex[TR_ER].x, seg->vertex[TR_ER].y, seg->vertex[TR_ER].z);
                            SETPOINT(texLen + seg->width / curTexSize, 0.66, seg->vertex[TR_ER].x, seg->vertex[TR_ER].y, seg->vertex[TR_ER].z + seg->height);
                        }
                        break;
                    case 1:
                        SETPOINT(texLen, 0.66, seg->vertex[TR_EL].x, seg->vertex[TR_EL].y, seg->vertex[TR_EL].z + seg->height);
                        SETPOINT(texLen, 0.33, seg->vertex[TR_ER].x, seg->vertex[TR_ER].y, seg->vertex[TR_ER].z + seg->height);
                        break;
                    case 2:
                        SETPOINT(texLen, 0.33, seg->vertex[TR_ER].x, seg->vertex[TR_ER].y, seg->vertex[TR_ER].z + seg->height);
                        SETPOINT(texLen, 0.0, seg->vertex[TR_ER].x, seg->vertex[TR_ER].y, seg->vertex[TR_ER].z);
                        break;
                    }
                    break;
                case TR_FENCE:
                case TR_PITBUILDING:
                    // not supported
                    break;
                }

                startNeeded = false;
                runninglentgh += seg->length;
            }
            else
            {
                startNeeded = true;
            }
        }
    }

    /* Left Side */
    prevTexId = 0;
    texLen = 0;
    startNeeded = true;
    runninglentgh = 0;
    hasBorder = false;
    for (i = 0, mseg = Track->seg->next; i < Track->nseg; i++, mseg = mseg->next)
    {
        if ((mseg->lside != nullptr) && ((mseg->lside->type2 == TR_LSIDE) || (mseg->lside->lside != nullptr)))
        {
            seg = mseg->lside;
            if (seg->lside)
            {
                seg = seg->lside;
                if (!hasBorder)
                {
                    startNeeded = true;
                    hasBorder = true;
                }
            }
            else
            {
                if (hasBorder)
                {
                    startNeeded = true;
                    hasBorder = false;
                }
            }

            if (!startNeeded)
            {
                tdble d0 = Distance(lastSeg->vertex[TR_EL].x, lastSeg->vertex[TR_EL].y, lastSeg->vertex[TR_EL].z, seg->vertex[TR_SL].x, seg->vertex[TR_SL].y, seg->vertex[TR_SL].z);
                tdble d1 = Distance(lastSeg->vertex[TR_ER].x, lastSeg->vertex[TR_ER].y, lastSeg->vertex[TR_ER].z, seg->vertex[TR_SR].x, seg->vertex[TR_SR].y, seg->vertex[TR_SR].z);
                if ((d0 > 0.01) || (d1 > 0.01))
                    startNeeded = true;
            }

            CHECKDISPLIST(seg->surface->material, "tkLS", i, mseg->lgfromstart);
            if (!curTexLink)
            {
                curTexSeg = 0;
            }
            else
            {
                curTexSeg = mseg->lgfromstart;
            }
            curTexSeg += curTexOffset;
            texLen = curTexSeg / curTexSize;
            if (startNeeded || (runninglentgh > LG_STEP_MAX))
            {
                NEWDISPLIST(false, "tkLS", i);
                runninglentgh = 0;

                ts = 0;
                width = RtTrackGetWidth(seg, ts);
                texMaxT = (curTexType == 1 ? width / curTexSize : 1.0 + floor(width / curTexSize));
                SETPOINT(texLen, texMaxT, seg->vertex[TR_SL].x, seg->vertex[TR_SL].y, seg->vertex[TR_SL].z);
                SETPOINT(texLen, 0, seg->vertex[TR_SR].x, seg->vertex[TR_SR].y, seg->vertex[TR_SR].z);
            }

            switch (seg->type)
            {
            case TR_STR:
                ts = LMAX;
                texStep = LMAX / curTexSize;
                texLen += texStep;
                trkpos.seg = seg;
                while (ts < seg->length)
                {
                    trkpos.toStart = ts;

                    width = RtTrackGetWidth(seg, ts);
                    texMaxT = (curTexType == 1 ? width / curTexSize : 1.0 + floor(width / curTexSize));

                    trkpos.toRight = width;
                    RtTrackLocal2Global(&trkpos, &x, &y, TR_TORIGHT);
                    SETPOINT(texLen, texMaxT, x, y, RtTrackHeightL(&trkpos));

                    trkpos.toRight = 0;
                    RtTrackLocal2Global(&trkpos, &x, &y, TR_TORIGHT);
                    SETPOINT(texLen, 0, x, y, RtTrackHeightL(&trkpos));

                    ts += LMAX;
                    texLen += texStep;
                }
                ts = seg->length;
                break;
            case TR_LFT:
                step = LMAX / (mseg->radiusr);
                texStep = step * mseg->radius / curTexSize;
                anz = seg->angle[TR_ZS] + step;
                ts = step;
                texLen += texStep;
                radiusr = seg->radiusr;
                radiusl = seg->radiusl;
                trkpos.seg = seg;
                while (anz < seg->angle[TR_ZE])
                {
                    trkpos.toStart = ts;
                    width = RtTrackGetWidth(seg, ts);
                    texMaxT = (curTexType == 1 ? width / curTexSize : 1.0 + floor(width / curTexSize));

                    /* left */
                    trkpos.toRight = width;
                    RtTrackLocal2Global(&trkpos, &x, &y, TR_TORIGHT);
                    SETPOINT(texLen, texMaxT, x, y, RtTrackHeightL(&trkpos));

                    /* right */
                    trkpos.toRight = 0;
                    RtTrackLocal2Global(&trkpos, &x, &y, TR_TORIGHT);
                    SETPOINT(texLen, 0, x, y, RtTrackHeightL(&trkpos));

                    ts += step;
                    texLen += texStep;
                    anz += step;
                }
                ts = seg->arc;
                break;
            case TR_RGT:
                step = LMAX / (mseg->radiusl);
                texStep = step * mseg->radius / curTexSize;
                anz = seg->angle[TR_ZS] - step;
                ts = step;
                texLen += texStep;
                radiusr = seg->radiusr;
                radiusl = seg->radiusl;
                trkpos.seg = seg;
                while (anz > seg->angle[TR_ZE])
                {
                    trkpos.toStart = ts;
                    width = RtTrackGetWidth(seg, ts);
                    texMaxT = (curTexType == 1 ? width / curTexSize : 1.0 + floor(width / curTexSize));

                    /* left */
                    trkpos.toRight = width;
                    RtTrackLocal2Global(&trkpos, &x, &y, TR_TORIGHT);
                    SETPOINT(texLen, texMaxT, x, y, RtTrackHeightL(&trkpos));

                    /* right */
                    trkpos.toRight = 0;
                    RtTrackLocal2Global(&trkpos, &x, &y, TR_TORIGHT);
                    SETPOINT(texLen, 0, x, y, RtTrackHeightL(&trkpos));

                    ts += step;
                    texLen += texStep;
                    anz -= step;
                }
                ts = seg->arc;
                break;
            }
            texLen = (curTexSeg + mseg->length) / curTexSize;

            width = RtTrackGetWidth(seg, ts);
            texMaxT = (curTexType == 1 ? width / curTexSize : 1.0 + floor(width / curTexSize));

            SETPOINT(texLen, texMaxT, seg->vertex[TR_EL].x, seg->vertex[TR_EL].y, seg->vertex[TR_EL].z);
            SETPOINT(texLen, 0, seg->vertex[TR_ER].x, seg->vertex[TR_ER].y, seg->vertex[TR_ER].z);

            startNeeded = false;
            runninglentgh += seg->length;
            lastSeg = seg;
        }
        else
        {
            startNeeded = true;
        }
    }

    /* Right barrier */
    curSurfType = 0x10;
    for (int j = 0; j < 3; j++)
    {
        prevTexId = 0;
        texLen = 0;
        startNeeded = true;
        runninglentgh = 0;
        sprintf(sname, "B%dRt", j);
        for (i = 0, mseg = Track->seg->next; i < Track->nseg; i++, mseg = mseg->next)
        {
            // Find last side segment
            seg = mseg;
            while (seg->rside)
                seg = seg->rside;

            if (seg->raceInfo & TR_PITBUILD)
            {
                startNeeded = true;
                runninglentgh = 0;
                continue;
            }
            
            curBarrier = mseg->barrier[0];

            // fence only has one side (j = 0)
            if (curBarrier->style == TR_FENCE && j != 0)
                continue;

            curSurfType = curBarrier->style == TR_FENCE ? 0x30 : 0x10;
            CHECKDISPLIST(curBarrier->surface->material, sname, i, 0);
            if (!curTexLink)
            {
                texLen = 0; // edited
            }
            trkpos.seg = seg;
            if (startNeeded || (runninglentgh > LG_STEP_MAX))
            {
                NEWDISPLIST(false, sname, i);
                if (curTexType == 0)
                    texLen = 0;
                runninglentgh = 0;
                xprev = seg->vertex[TR_SR].x; // edited
                yprev = seg->vertex[TR_SR].y; // edited

                switch (curBarrier->style)
                {
                case TR_FENCE:
                    if (j == 0)
                    {
                        SETPOINT(texLen, 0, seg->vertex[TR_SR].x, seg->vertex[TR_SR].y, seg->vertex[TR_SR].z);
                        SETPOINT(texLen, 1.0, seg->vertex[TR_SR].x, seg->vertex[TR_SR].y, seg->vertex[TR_SR].z + curBarrier->height);
                    }
                    break;
                case TR_WALL:
                    switch (j)
                    {
                    case 0:
                        SETPOINT(texLen, 0, seg->vertex[TR_SR].x, seg->vertex[TR_SR].y, seg->vertex[TR_SR].z);
                        SETPOINT(texLen, 0.33, seg->vertex[TR_SR].x, seg->vertex[TR_SR].y, seg->vertex[TR_SR].z + curBarrier->height);
                        break;
                    case 1:
                        trkpos.toStart = 0;
                        trkpos.toRight = -curBarrier->width;
                        RtTrackLocal2Global(&trkpos, &x, &y, TR_TORIGHT);
                        SETPOINT(texLen, 0.33, seg->vertex[TR_SR].x, seg->vertex[TR_SR].y, seg->vertex[TR_SR].z + curBarrier->height);
                        SETPOINT(texLen, 0.66, x, y, seg->vertex[TR_SR].z + curBarrier->height);
                        break;
                    case 2:
                        trkpos.toStart = 0;
                        trkpos.toRight = -curBarrier->width;
                        RtTrackLocal2Global(&trkpos, &x, &y, TR_TORIGHT);
                        if ((mseg->prev->barrier[0]->style != TR_WALL) || (mseg->prev->barrier[0]->height != curBarrier->height))
                        {
                            SETPOINT(texLen - curBarrier->width / curTexSize, 0.66, seg->vertex[TR_SR].x, seg->vertex[TR_SR].y, seg->vertex[TR_SR].z + curBarrier->height);
                            SETPOINT(texLen - curBarrier->width / curTexSize, 1.00, seg->vertex[TR_SR].x, seg->vertex[TR_SR].y, seg->vertex[TR_SR].z);
                        }
                        SETPOINT(texLen, 0.66, x, y, seg->vertex[TR_SR].z + curBarrier->height);
                        SETPOINT(texLen, 1.0, x, y, seg->vertex[TR_SR].z);
                        break;
                    }
                    break;
                case TR_CURB:
                case TR_PLAN:
                    // not supported
                    break;
                case TR_PITBUILDING:
                    // nothing to do
                    break;
                }
            }
            switch (seg->type)
            {
            case TR_STR:
                ts = LMAX;

                trkpos.seg = seg;
                while (ts < seg->length)
                {
                    trkpos.toStart = ts;
                    trkpos.toRight = 0;
                    RtTrackLocal2Global(&trkpos, &x, &y, TR_TORIGHT);
                    curHeight = RtTrackHeightL(&trkpos);
                    texStep = sqrt(pow((y - yprev), 2.0f) + pow((x - xprev), 2.0f)) / curTexSize; // new
                    texLen += texStep;                                                            // new
                    xprev = x;                                                                    // new
                    yprev = y;                                                                    // new
                    switch (curBarrier->style)
                    {
                    case TR_FENCE:
                        if (j == 0)
                        {
                            SETPOINT(texLen, 0, x, y, curHeight);
                            SETPOINT(texLen, 1.0, x, y, curHeight + curBarrier->height);
                        }
                        break;
                    case TR_WALL:
                        switch (j)
                        {
                        case 0:
                            SETPOINT(texLen, 0.0, x, y, curHeight);
                            SETPOINT(texLen, 0.33, x, y, curHeight + curBarrier->height);
                            break;
                        case 1:
                            SETPOINT(texLen, 0.33, x, y, curHeight + curBarrier->height);
                            trkpos.toRight = -curBarrier->width;
                            RtTrackLocal2Global(&trkpos, &x, &y, TR_TORIGHT);
                            SETPOINT(texLen, 0.66, x, y, curHeight + curBarrier->height);
                            break;
                        case 2:
                            trkpos.toRight = -curBarrier->width;
                            RtTrackLocal2Global(&trkpos, &x, &y, TR_TORIGHT);
                            SETPOINT(texLen, 0.66, x, y, curHeight + curBarrier->height);
                            SETPOINT(texLen, 1.0, x, y, curHeight);
                            break;
                        }
                        break;
                    case TR_CURB:
                    case TR_PLAN:
                        // not supported
                        break;
                    case TR_PITBUILDING:
                        // nothing to do
                        break;
                    }

                    ts += LMAX;
                }
                ts = seg->length;
                break;
            case TR_LFT:
                step = LMAX / (mseg->radiusr);
                // texStep = step * mseg->radius / curTexSize; //edited
                anz = seg->angle[TR_ZS] + step;
                ts = step;
                // texLen += texStep;
                radiusr = seg->radiusr;
                trkpos.seg = seg;
                while (anz < seg->angle[TR_ZE])
                {
                    trkpos.toStart = ts;
                    trkpos.toRight = 0;
                    RtTrackLocal2Global(&trkpos, &x, &y, TR_TORIGHT);

                    curHeight = RtTrackHeightL(&trkpos);
                    texStep = sqrt(pow((y - yprev), 2.0f) + pow((x - xprev), 2.0f)) / curTexSize; // new
                    texLen += texStep;                                                            // new
                    xprev = x;                                                                    // new
                    yprev = y;                                                                    // new
                    switch (curBarrier->style)
                    {
                    case TR_FENCE:
                        if (j == 0)
                        {
                            SETPOINT(texLen, 0.0, x, y, curHeight);
                            SETPOINT(texLen, 1.0, x, y, curHeight + curBarrier->height);
                        }
                        break;
                    case TR_WALL:
                        switch (j)
                        {
                        case 0:
                            SETPOINT(texLen, 0.0, x, y, curHeight);
                            SETPOINT(texLen, 0.33, x, y, curHeight + curBarrier->height);
                            break;
                        case 1:
                            SETPOINT(texLen, 0.33, x, y, curHeight + curBarrier->height);
                            trkpos.toRight = -curBarrier->width;
                            RtTrackLocal2Global(&trkpos, &x, &y, TR_TORIGHT);
                            SETPOINT(texLen, 0.66, x, y, curHeight + curBarrier->height);
                            break;
                        case 2:
                            trkpos.toRight = -curBarrier->width;
                            RtTrackLocal2Global(&trkpos, &x, &y, TR_TORIGHT);
                            SETPOINT(texLen, 0.66, x, y, curHeight + curBarrier->height);
                            SETPOINT(texLen, 1.0, x, y, curHeight);
                            break;
                        }
                        break;
                    case TR_CURB:
                    case TR_PLAN:
                        // not supported
                        break;
                    case TR_PITBUILDING:
                        // nothing to do
                        break;
                    }

                    ts += step;
                    // removed
                    anz += step;
                }
                ts = seg->arc;
                break;
            case TR_RGT:
                step = LMAX / (mseg->radiusl);
                // removed
                anz = seg->angle[TR_ZS] - step;
                ts = step;
                // removed
                radiusr = seg->radiusr;
                trkpos.seg = seg;
                while (anz > seg->angle[TR_ZE])
                {
                    trkpos.toStart = ts;
                    trkpos.toRight = 0;
                    RtTrackLocal2Global(&trkpos, &x, &y, TR_TORIGHT);

                    curHeight = RtTrackHeightL(&trkpos);
                    texStep = sqrt(pow((y - yprev), 2.0f) + pow((x - xprev), 2.0f)) / curTexSize; // new
                    texLen += texStep;                                                            // new
                    xprev = x;                                                                    // new
                    yprev = y;                                                                    // new
                    switch (curBarrier->style)
                    {
                    case TR_FENCE:
                        if (j == 0)
                        {
                            SETPOINT(texLen, 0.0, x, y, curHeight);
                            SETPOINT(texLen, 1.0, x, y, curHeight + curBarrier->height);
                        }
                        break;
                    case TR_WALL:
                        switch (j)
                        {
                        case 0:
                            SETPOINT(texLen, 0.0, x, y, curHeight);
                            SETPOINT(texLen, 0.33, x, y, curHeight + curBarrier->height);
                            break;
                        case 1:
                            SETPOINT(texLen, 0.33, x, y, curHeight + curBarrier->height);
                            trkpos.toRight = -curBarrier->width;
                            RtTrackLocal2Global(&trkpos, &x, &y, TR_TORIGHT);
                            SETPOINT(texLen, 0.66, x, y, curHeight + curBarrier->height);
                            break;
                        case 2:
                            trkpos.toRight = -curBarrier->width;
                            RtTrackLocal2Global(&trkpos, &x, &y, TR_TORIGHT);
                            SETPOINT(texLen, 0.66, x, y, curHeight + curBarrier->height);
                            SETPOINT(texLen, 1.0, x, y, curHeight);
                            break;
                        }
                        break;
                    case TR_CURB:
                    case TR_PLAN:
                        // not supported
                        break;
                    case TR_PITBUILDING:
                        // nothing to do
                        break;
                    }
                    ts += step;
                    anz -= step;
                }
                ts = seg->arc;
                break;
            }
            texStep = sqrt(pow((seg->vertex[TR_ER].y - yprev), 2.0f) + pow((seg->vertex[TR_ER].x - xprev), 2.0f)) / curTexSize; // new
            texLen += texStep;                                                                                                  // edited
            xprev = seg->vertex[TR_ER].x;                                                                                       // new
            yprev = seg->vertex[TR_ER].y;                                                                                       // new
            switch (curBarrier->style)
            {
            case TR_FENCE:
                if (j == 0)
                {
                    SETPOINT(texLen, 0.0, seg->vertex[TR_ER].x, seg->vertex[TR_ER].y, seg->vertex[TR_ER].z);
                    SETPOINT(texLen, 1.0, seg->vertex[TR_ER].x, seg->vertex[TR_ER].y, seg->vertex[TR_ER].z + curBarrier->height);
                }
                break;
            case TR_WALL:
                switch (j)
                {
                case 0:
                    SETPOINT(texLen, 0.0, seg->vertex[TR_ER].x, seg->vertex[TR_ER].y, seg->vertex[TR_ER].z);
                    SETPOINT(texLen, 0.33, seg->vertex[TR_ER].x, seg->vertex[TR_ER].y, seg->vertex[TR_ER].z + curBarrier->height);
                    break;
                case 1:
                    SETPOINT(texLen, 0.33, seg->vertex[TR_ER].x, seg->vertex[TR_ER].y, seg->vertex[TR_ER].z + curBarrier->height);
                    trkpos.toStart = ts;
                    trkpos.toRight = -curBarrier->width;
                    RtTrackLocal2Global(&trkpos, &x, &y, TR_TORIGHT);
                    SETPOINT(texLen, 0.66, x, y, seg->vertex[TR_ER].z + curBarrier->height);
                    break;
                case 2:
                    trkpos.toStart = ts;
                    trkpos.toRight = -curBarrier->width;
                    RtTrackLocal2Global(&trkpos, &x, &y, TR_TORIGHT);
                    SETPOINT(texLen, 0.66, x, y, seg->vertex[TR_ER].z + curBarrier->height);
                    SETPOINT(texLen, 1.00, x, y, seg->vertex[TR_ER].z);
                    if ((mseg->next->barrier[0]->style != TR_WALL) || (mseg->next->barrier[0]->height != curBarrier->height))
                    {
                        SETPOINT(texLen + curBarrier->width / curTexSize, 0.66, seg->vertex[TR_ER].x, seg->vertex[TR_ER].y, seg->vertex[TR_ER].z + curBarrier->height);
                        SETPOINT(texLen + curBarrier->width / curTexSize, 1.00, seg->vertex[TR_ER].x, seg->vertex[TR_ER].y, seg->vertex[TR_ER].z);
                    }
                    break;
                }
                break;
            case TR_CURB:
            case TR_PLAN:
                // not supported
                break;
            case TR_PITBUILDING:
                // nothing to do
                break;
            }
            startNeeded = false;
            runninglentgh += seg->length;
        }
    }

    /* Left Barrier */
    curSurfType = 0x10;
    for (int j = 0; j < 3; j++)
    {
        prevTexId = 0;
        texLen = 0;
        startNeeded = true;
        runninglentgh = 0;
        sprintf(sname, "B%dLt", j);
        for (i = 0, mseg = Track->seg->next; i < Track->nseg; i++, mseg = mseg->next)
        {
            // Find last side segment
            seg = mseg;
            while (seg->lside)
                seg = seg->lside;

            if (seg->raceInfo & TR_PITBUILD)
            {
                runninglentgh = 0;
                startNeeded = true;
                continue;
            }
            
            curBarrier = mseg->barrier[1];
            
            // fence only has one side (j = 0)
            if (curBarrier->style == TR_FENCE && j != 0)
                continue;
            
            curSurfType = curBarrier->style == TR_FENCE ? 0x30 : 0x10;
            CHECKDISPLIST(curBarrier->surface->material, sname, i, 0);
            if (!curTexLink)
            {
                texLen = 0;
            }
            trkpos.seg = seg;
            if (startNeeded || (runninglentgh > LG_STEP_MAX))
            {
                NEWDISPLIST(false, sname, i);
                runninglentgh = 0;
                if (curTexType == 0)
                    texLen = 0;
                xprev = seg->vertex[TR_SR].x; // edited
                yprev = seg->vertex[TR_SR].y; // edited
            
                switch (curBarrier->style)
                {
                case TR_FENCE:
                    if (j == 0)
                    {
                        SETPOINT(texLen, 1.0, seg->vertex[TR_SL].x, seg->vertex[TR_SL].y, seg->vertex[TR_SL].z + curBarrier->height);
                        SETPOINT(texLen, 0.0, seg->vertex[TR_SL].x, seg->vertex[TR_SL].y, seg->vertex[TR_SL].z);
                    }
                    break;
                case TR_WALL:
                    switch (j)
                    {
                    case 0:
                        trkpos.toStart = 0;
                        trkpos.toRight = curBarrier->width + RtTrackGetWidth(seg, 0);
                        trkpos.seg = seg;
                        RtTrackLocal2Global(&trkpos, &x, &y, TR_TORIGHT);
                        if ((mseg->prev->barrier[1]->style != TR_WALL) || (mseg->prev->barrier[1]->height != curBarrier->height))
                        {
                            SETPOINT(texLen - curBarrier->width / curTexSize, 1.00, seg->vertex[TR_SL].x, seg->vertex[TR_SL].y, seg->vertex[TR_SL].z);
                            SETPOINT(texLen - curBarrier->width / curTexSize, 0.66, seg->vertex[TR_SL].x, seg->vertex[TR_SL].y, seg->vertex[TR_SL].z + curBarrier->height);
                        }
                        SETPOINT(texLen, 1.0, x, y, seg->vertex[TR_SL].z);
                        SETPOINT(texLen, 0.66, x, y, seg->vertex[TR_SL].z + curBarrier->height);
                        break;
                    case 1:
                        trkpos.toStart = 0;
                        trkpos.toRight = curBarrier->width + RtTrackGetWidth(seg, 0);
                        trkpos.seg = seg;
                        RtTrackLocal2Global(&trkpos, &x, &y, TR_TORIGHT);
                        SETPOINT(texLen, 0.66, x, y, seg->vertex[TR_SL].z + curBarrier->height);
                        SETPOINT(texLen, 0.33, seg->vertex[TR_SL].x, seg->vertex[TR_SL].y, seg->vertex[TR_SL].z + curBarrier->height);
                        break;
                    case 2:
                        SETPOINT(texLen, 0.33, seg->vertex[TR_SL].x, seg->vertex[TR_SL].y, seg->vertex[TR_SL].z + curBarrier->height);
                        SETPOINT(texLen, 0.0, seg->vertex[TR_SL].x, seg->vertex[TR_SL].y, seg->vertex[TR_SL].z);
                        break;
                    }
                    break;
                case TR_CURB:
                case TR_PLAN:
                    // not supported
                    break;
                case TR_PITBUILDING:
                    // nothing to do
                    break;
                }
            }
            
            switch (seg->type)
            {
            case TR_STR:
                ts = LMAX;
            
                trkpos.seg = seg;
                while (ts < seg->length)
                {
                    trkpos.toStart = ts;
                    trkpos.toRight = RtTrackGetWidth(seg, ts);
                    RtTrackLocal2Global(&trkpos, &x, &y, TR_TORIGHT);
                    curHeight = RtTrackHeightL(&trkpos);
                    texStep = sqrt(pow((y - yprev), 2.0f) + pow((x - xprev), 2.0f)) / curTexSize; // new
                    texLen += texStep;                                                            // new
                    xprev = x;                                                                    // new
                    yprev = y;                                                                    // new
                    switch (curBarrier->style)
                    {
                    case TR_FENCE:
                        if (j == 0)
                        {
                            SETPOINT(texLen, 1.0, x, y, curHeight + curBarrier->height);
                            SETPOINT(texLen, 0, x, y, curHeight);
                        }
                        break;
                    case TR_WALL:
                        switch (j)
                        {
                        case 0:
                            trkpos.toRight = curBarrier->width + RtTrackGetWidth(seg, ts);
                            RtTrackLocal2Global(&trkpos, &x, &y, TR_TORIGHT);
                            SETPOINT(texLen, 1.0, x, y, curHeight);
                            SETPOINT(texLen, 0.66, x, y, curHeight + curBarrier->height);
                            break;
                        case 1:
                            trkpos.toRight = curBarrier->width + RtTrackGetWidth(seg, ts);
                            RtTrackLocal2Global(&trkpos, &x, &y, TR_TORIGHT);
                            SETPOINT(texLen, 0.66, x, y, curHeight + curBarrier->height);
                            trkpos.toRight = RtTrackGetWidth(seg, ts);
                            RtTrackLocal2Global(&trkpos, &x, &y, TR_TORIGHT);
                            SETPOINT(texLen, 0.33, x, y, curHeight + curBarrier->height);
                            break;
                        case 2:
                            SETPOINT(texLen, 0.33, x, y, curHeight + curBarrier->height);
                            SETPOINT(texLen, 0, x, y, curHeight);
                            break;
                        }
                        break;
                    case TR_CURB:
                    case TR_PLAN:
                        // not supported
                        break;
                    case TR_PITBUILDING:
                        // nothing to do
                        break;
                    }
            
                    ts += LMAX;
                }
                ts = seg->length;
                break;
            case TR_LFT:
                step = LMAX / (mseg->radiusr);
            
                anz = seg->angle[TR_ZS] + step;
                ts = step;
            
                radiusl = seg->radiusl;
                trkpos.seg = seg;
                while (anz < seg->angle[TR_ZE])
                {
                    trkpos.toStart = ts;
                    trkpos.toRight = RtTrackGetWidth(seg, ts);
                    RtTrackLocal2Global(&trkpos, &x, &y, TR_TORIGHT);
                    curHeight = RtTrackHeightL(&trkpos);
                    texStep = sqrt(pow((y - yprev), 2.0f) + pow((x - xprev), 2.0f)) / curTexSize; // new
                    texLen += texStep;                                                            // new
                    xprev = x;                                                                    // new
                    yprev = y;                                                                    // new
            
                    switch (curBarrier->style)
                    {
                    case TR_FENCE:
                        if (j == 0)
                        {
                            SETPOINT(texLen, 1.0, x, y, curHeight + curBarrier->height);
                            SETPOINT(texLen, 0, x, y, curHeight);
                        }
                        break;
                    case TR_WALL:
                        switch (j)
                        {
                        case 0:
                            trkpos.toRight = curBarrier->width + RtTrackGetWidth(seg, ts);
                            RtTrackLocal2Global(&trkpos, &x, &y, TR_TORIGHT);
                            SETPOINT(texLen, 1.0, x, y, curHeight);
                            SETPOINT(texLen, 0.66, x, y, curHeight + curBarrier->height);
                            break;
                        case 1:
                            trkpos.toRight = curBarrier->width + RtTrackGetWidth(seg, ts);
                            RtTrackLocal2Global(&trkpos, &x, &y, TR_TORIGHT);
                            SETPOINT(texLen, 0.66, x, y, curHeight + curBarrier->height);
                            trkpos.toRight = RtTrackGetWidth(seg, ts);
                            RtTrackLocal2Global(&trkpos, &x, &y, TR_TORIGHT);
                            SETPOINT(texLen, 0.33, x, y, curHeight + curBarrier->height);
                            break;
                        case 2:
                            SETPOINT(texLen, 0.33, x, y, curHeight + curBarrier->height);
                            SETPOINT(texLen, 0, x, y, curHeight);
                            break;
                        }
                        break;
                    case TR_CURB:
                    case TR_PLAN:
                        // not supported
                        break;
                    case TR_PITBUILDING:
                        // nothing to do
                        break;
                    }
            
                    ts += step;
            
                    anz += step;
                }
                ts = seg->arc;
                break;
            case TR_RGT:
                step = LMAX / (mseg->radiusl);
            
                anz = seg->angle[TR_ZS] - step;
                ts = step;
            
                radiusl = seg->radiusl;
                trkpos.seg = seg;
                while (anz > seg->angle[TR_ZE])
                {
                    trkpos.toStart = ts;
                    trkpos.toRight = RtTrackGetWidth(seg, ts);
                    RtTrackLocal2Global(&trkpos, &x, &y, TR_TORIGHT);
                    curHeight = RtTrackHeightL(&trkpos);
                    texStep = sqrt(pow((y - yprev), 2.0f) + pow((x - xprev), 2.0f)) / curTexSize; // new
                    texLen += texStep;                                                            // new
                    xprev = x;                                                                    // new
                    yprev = y;                                                                    // new
                    switch (curBarrier->style)
                    {
                    case TR_FENCE:
                        if (j == 0)
                        {
                            SETPOINT(texLen, 1.0, x, y, curHeight + curBarrier->height);
                            SETPOINT(texLen, 0, x, y, curHeight);
                        }
                        break;
                    case TR_WALL:
                        switch (j)
                        {
                        case 0:
                            trkpos.toRight = curBarrier->width + RtTrackGetWidth(seg, ts);
                            RtTrackLocal2Global(&trkpos, &x, &y, TR_TORIGHT);
                            SETPOINT(texLen, 1.0, x, y, curHeight);
                            SETPOINT(texLen, 0.66, x, y, curHeight + curBarrier->height);
                            break;
                        case 1:
                            trkpos.toRight = curBarrier->width + RtTrackGetWidth(seg, ts);
                            RtTrackLocal2Global(&trkpos, &x, &y, TR_TORIGHT);
                            SETPOINT(texLen, 0.66, x, y, curHeight + curBarrier->height);
                            trkpos.toRight = RtTrackGetWidth(seg, ts);
                            RtTrackLocal2Global(&trkpos, &x, &y, TR_TORIGHT);
                            SETPOINT(texLen, 0.33, x, y, curHeight + curBarrier->height);
                            break;
                        case 2:
                            SETPOINT(texLen, 0.33, x, y, curHeight + curBarrier->height);
                            SETPOINT(texLen, 0, x, y, curHeight);
                            break;
                        }
                        break;
                    case TR_CURB:
                        case TR_PLAN:
                        // not supported
                        break;
                    case TR_PITBUILDING:
                        // nothing to do
                        break;
                    }
            
                    ts += step;
            
                    anz -= step;
                }
                ts = seg->arc;
                break;
            }
            texStep = sqrt(pow((seg->vertex[TR_ER].y - yprev), 2.0f) + pow((seg->vertex[TR_ER].x - xprev), 2.0f)) / curTexSize; // new
            texLen += texStep;                                                                                                  // edited
            xprev = seg->vertex[TR_ER].x;                                                                                       // new
            yprev = seg->vertex[TR_ER].y;
            
            switch (curBarrier->style)
            {
            case TR_FENCE:
                if (j == 0)
                {
                    SETPOINT(texLen, 1.0, seg->vertex[TR_EL].x, seg->vertex[TR_EL].y, seg->vertex[TR_EL].z + curBarrier->height);
                    SETPOINT(texLen, 0, seg->vertex[TR_EL].x, seg->vertex[TR_EL].y, seg->vertex[TR_EL].z);
                }
                break;
            case TR_WALL:
                switch (j)
                {
                case 0:
                    trkpos.toStart = ts;
                    trkpos.toRight = curBarrier->width + RtTrackGetWidth(seg, ts);
                    RtTrackLocal2Global(&trkpos, &x, &y, TR_TORIGHT);
                    SETPOINT(texLen, 1.0, x, y, seg->vertex[TR_EL].z);
                    SETPOINT(texLen, 0.66, x, y, seg->vertex[TR_EL].z + curBarrier->height);
                    if ((mseg->next->barrier[1]->style != TR_WALL) || (mseg->next->barrier[1]->height != curBarrier->height))
                    {
                        SETPOINT(texLen + curBarrier->width / curTexSize, 1.00, seg->vertex[TR_EL].x, seg->vertex[TR_EL].y, seg->vertex[TR_EL].z);
                        SETPOINT(texLen + curBarrier->width / curTexSize, 0.66, seg->vertex[TR_EL].x, seg->vertex[TR_EL].y, seg->vertex[TR_EL].z + curBarrier->height);
                    }
                    break;
                case 1:
                    trkpos.toStart = ts;
                    trkpos.toRight = curBarrier->width + RtTrackGetWidth(seg, ts);
                    RtTrackLocal2Global(&trkpos, &x, &y, TR_TORIGHT);
                    SETPOINT(texLen, 0.66, x, y, seg->vertex[TR_EL].z + curBarrier->height);
                    SETPOINT(texLen, 0.33, seg->vertex[TR_EL].x, seg->vertex[TR_EL].y, seg->vertex[TR_EL].z + curBarrier->height);
                    break;
                case 2:
                    SETPOINT(texLen, 0.33, seg->vertex[TR_EL].x, seg->vertex[TR_EL].y, seg->vertex[TR_EL].z + curBarrier->height);
                    SETPOINT(texLen, 0.0, seg->vertex[TR_EL].x, seg->vertex[TR_EL].y, seg->vertex[TR_EL].z);
                    break;
                }
                break;
            case TR_CURB:
            case TR_PLAN:
                // not supported
                break;
            case TR_PITBUILDING:
                // nothing to do
                break;
            }
            
            startNeeded = false;
            runninglentgh += seg->length;
        }
    }

    if (!bump)
    {
        curSurfType = 0x10;

        /* Turn Marks */
        for (i = 0, seg = Track->seg->next; i < Track->nseg; i++, seg = seg->next)
        {
            if (seg->ext)
            {
                t3Dd normvec;
                int nbMarks = seg->ext->nbMarks;
                int *marks = seg->ext->marks;
                int j, k;

                for (j = 0; j < nbMarks; j++)
                {
                    /* find the segment */
                    tdble lgfs = seg->lgfromstart - (tdble)marks[j];
                    if (lgfs < 0)
                    {
                        lgfs += Track->length;
                    }
                    for (k = 0, mseg = Track->seg->next; k < Track->nseg; k++, mseg = mseg->next)
                    {
                        if ((lgfs >= mseg->lgfromstart) && (lgfs < (mseg->lgfromstart + mseg->length)))
                        {
                            break;
                        }
                    }

                    if (seg->type == TR_RGT)
                    {
                        sprintf(buf, "turn%dR", marks[j]);
                        trkpos.toRight = Track->width + tmHSpace + tmWidth;
                    }
                    else
                    {
                        sprintf(buf, "turn%dL", marks[j]);
                        trkpos.toRight = -tmHSpace;
                    }
                    trkpos.toStart = lgfs - mseg->lgfromstart;
                    if (mseg->type != TR_STR)
                    {
                        trkpos.toStart = trkpos.toStart / mseg->radius;
                    }
                    trkpos.seg = mseg;
                    RtTrackLocal2Global(&trkpos, &x, &y, TR_TORIGHT);
                    z = tmVSpace + RtTrackHeightL(&trkpos);
                    if (seg->type == TR_LFT)
                    {
                        RtTrackSideNormalG(mseg, x, y, TR_RGT, &normvec);
                        normvec.x = -normvec.x;
                        normvec.y = -normvec.y;
                    }
                    else
                    {
                        RtTrackSideNormalG(mseg, x, y, TR_LFT, &normvec);
                    }
                    CHECKDISPLIST2(buf, 0, "TuMk", mseg->id);

                    SETPOINT(0.0, 0.0, x, y, z);
                    SETPOINT(1.0, 0.0, x + tmWidth * normvec.x, y + tmWidth * normvec.y, z);
                    SETPOINT(0.0, 1.0, x, y, z + tmHeight);
                    SETPOINT(1.0, 1.0, x + tmWidth * normvec.x, y + tmWidth * normvec.y, z + tmHeight);

                    CHECKDISPLIST2("back-sign", 0, "TuMk", mseg->id);

                    SETPOINT(0.0, 0.0, x + tmWidth * normvec.x, y + tmWidth * normvec.y, z);
                    SETPOINT(1.0, 0.0, x, y, z);
                    SETPOINT(0.0, 1.0, x + tmWidth * normvec.x, y + tmWidth * normvec.y, z + tmHeight);
                    SETPOINT(1.0, 1.0, x, y, z + tmHeight);

                    printf("(%f, %f, %f), (%f, %f, %f)\n", x, y, z, x + tmWidth * normvec.x, y + tmWidth * normvec.y, z + tmHeight);
                }
            }
        }

        /* Start Bridge */
        if (bridge)
        {
            curSurfType = 0x10;
            CHECKDISPLIST2("pylon1", 4, "S0Bg", 0);
#define BR_HEIGHT_1 8.0
#define BR_HEIGHT_2 6.0
#define BR_WIDTH_0 2.0
#define BR_WIDTH_1 2.0
            mseg = Track->seg->next;
            if (mseg->rside)
            {
                seg = mseg->rside;
                if (seg->rside)
                {
                    seg = seg->rside;
                }
            }
            else
            {
                seg = mseg;
            }

            x = seg->vertex[TR_SR].x;
            y = seg->vertex[TR_SR].y - 0.1;
            z = seg->vertex[TR_SR].z;

            SETPOINT(0, 0, x, y, z);
            SETPOINT(0, 1, x, y, z + BR_HEIGHT_2);

            x += BR_WIDTH_0;

            SETPOINT(1, 0, x, y, z);
            SETPOINT(1, 1, x, y, z + BR_HEIGHT_2);

            y -= BR_WIDTH_1;

            SETPOINT(2, 0, x, y, z);
            SETPOINT(2, 1, x, y, z + BR_HEIGHT_2);

            x -= BR_WIDTH_0;

            SETPOINT(3, 0, x, y, z);
            SETPOINT(3, 1, x, y, z + BR_HEIGHT_2);

            y += BR_WIDTH_1;

            SETPOINT(4, 0, x, y, z);
            SETPOINT(4, 1, x, y, z + BR_HEIGHT_2);

            NEWDISPLIST(false, "S1Bg", 0);

            if (mseg->lside)
            {
                seg = mseg->lside;
                if (seg->lside)
                {
                    seg = seg->lside;
                }
            }
            else
            {
                seg = mseg;
            }
            x2 = seg->vertex[TR_SL].x;
            y2 = seg->vertex[TR_SL].y + 0.1;
            z2 = seg->vertex[TR_SL].z;

            SETPOINT(0, 1, x2, y2, z + BR_HEIGHT_2);
            SETPOINT(0, 0, x2, y2, z2);

            x2 += BR_WIDTH_0;

            SETPOINT(1, 1, x2, y2, z + BR_HEIGHT_2);
            SETPOINT(1, 0, x2, y2, z2);

            y2 += BR_WIDTH_1;

            SETPOINT(2, 1, x2, y2, z + BR_HEIGHT_2);
            SETPOINT(2, 0, x2, y2, z2);

            x2 -= BR_WIDTH_0;

            SETPOINT(3, 1, x2, y2, z + BR_HEIGHT_2);
            SETPOINT(3, 0, x2, y2, z2);

            y2 -= BR_WIDTH_1;

            SETPOINT(4, 1, x2, y2, z + BR_HEIGHT_2);
            SETPOINT(4, 0, x2, y2, z2);

            CHECKDISPLIST2("pylon2", 4, "S2Bg", 0);

            SETPOINT(0, 1, x, y, z + BR_HEIGHT_1);
            SETPOINT(0, 0, x, y, z + BR_HEIGHT_2);

            y -= BR_WIDTH_1;

            SETPOINT(1, 1, x, y, z + BR_HEIGHT_1);
            SETPOINT(1, 0, x, y, z + BR_HEIGHT_2);

            x += BR_WIDTH_0;

            SETPOINT(2, 1, x, y, z + BR_HEIGHT_1);
            SETPOINT(2, 0, x, y, z + BR_HEIGHT_2);

            y += BR_WIDTH_1;

            SETPOINT(3, 1, x, y, z + BR_HEIGHT_1);
            SETPOINT(3, 0, x, y, z + BR_HEIGHT_2);

            x -= BR_WIDTH_0;

            SETPOINT(3, 1, x + BR_WIDTH_0, y, z + BR_HEIGHT_1);
            SETPOINT(3, 0, x, y, z + BR_HEIGHT_1);

            y -= BR_WIDTH_1;

            SETPOINT(4, 1, x + BR_WIDTH_0, y, z + BR_HEIGHT_1);
            SETPOINT(4, 0, x, y, z + BR_HEIGHT_1);

            y += BR_WIDTH_1; /* back to origin */

            NEWDISPLIST(false, "S3Bg", 0);

            y2 += BR_WIDTH_1;

            SETPOINT(0, 1, x2 + BR_WIDTH_0, y2, z + BR_HEIGHT_1);
            SETPOINT(0, 0, x2, y2, z + BR_HEIGHT_1);

            y2 -= BR_WIDTH_1;

            SETPOINT(1, 1, x2 + BR_WIDTH_0, y2, z + BR_HEIGHT_1);
            SETPOINT(1, 0, x2, y2, z + BR_HEIGHT_1);

            x2 += BR_WIDTH_0;

            SETPOINT(1, 1, x2, y2, z + BR_HEIGHT_1);
            SETPOINT(1, 0, x2, y2, z + BR_HEIGHT_2);

            y2 += BR_WIDTH_1;

            SETPOINT(2, 1, x2, y2, z + BR_HEIGHT_1);
            SETPOINT(2, 0, x2, y2, z + BR_HEIGHT_2);

            x2 -= BR_WIDTH_0;

            SETPOINT(3, 1, x2, y2, z + BR_HEIGHT_1);
            SETPOINT(3, 0, x2, y2, z + BR_HEIGHT_2);

            y2 -= BR_WIDTH_1;

            SETPOINT(4, 1, x2, y2, z + BR_HEIGHT_1);
            SETPOINT(4, 0, x2, y2, z + BR_HEIGHT_2);

            /* Middle on the bridge */
            CHECKDISPLIST2("pylon3", 4, "S4Bg", 2);

            SETPOINT(0, 0, x2, y2, z + BR_HEIGHT_2);
            SETPOINT(1, 0, x, y, z + BR_HEIGHT_2);
            SETPOINT(0, 0.25, x2, y2, z + BR_HEIGHT_1);
            SETPOINT(1, 0.25, x, y, z + BR_HEIGHT_1);

            x += BR_WIDTH_0;
            x2 += BR_WIDTH_0;

            SETPOINT(0, 0.5, x2, y2, z + BR_HEIGHT_1);
            SETPOINT(1, 0.5, x, y, z + BR_HEIGHT_1);

            SETPOINT(0, 0.75, x2, y2, z + BR_HEIGHT_2);
            SETPOINT(1, 0.75, x, y, z + BR_HEIGHT_2);

            x -= BR_WIDTH_0;
            x2 -= BR_WIDTH_0;

            SETPOINT(0, 1, x2, y2, z + BR_HEIGHT_2);
            SETPOINT(1, 1, x, y, z + BR_HEIGHT_2);
        }

        /* draw the pits */
#define PIT_HEIGHT 5.0
#define PIT_DEEP 10.0
#define PIT_TOP 0.2

        pits = &(Track->pits);
        initPits(Track, TrackHandle, pits);
        curSurfType = 0x30; // this must be 2 sided

        switch (pits->type)
        {
        case TR_PIT_ON_TRACK_SIDE: {
            static int uid = 1;
            t3Dd normvec;

            startNeeded = true;
            sprintf(sname, "P%dts", uid++);
            CHECKDISPLIST3("concrete2.png", 4, sname, pits->driversPits[0].pos.seg->id);

            RtTrackLocal2Global(&(pits->driversPits[0].pos), &x, &y, pits->driversPits[0].pos.type);
            RtTrackSideNormalG(pits->driversPits[0].pos.seg, x, y, pits->side, &normvec);
            z2 = RtTrackHeightG(pits->driversPits[0].pos.seg, x, y);

            x2 = x + PIT_TOP * normvec.x;
            y2 = y + PIT_TOP * normvec.y;
            SETPOINT(1.0 - PIT_TOP, PIT_HEIGHT - PIT_TOP, x2, y2, z2 + PIT_HEIGHT - PIT_TOP);
            SETPOINT(1.0 - PIT_TOP, PIT_HEIGHT, x2, y2, z2 + PIT_HEIGHT);

            x2 = x;
            y2 = y;
            SETPOINT(1.0, PIT_HEIGHT - PIT_TOP, x2, y2, z2 + PIT_HEIGHT - PIT_TOP);

            x2 = x - PIT_DEEP * normvec.x;
            y2 = y - PIT_DEEP * normvec.y;
            SETPOINT(1.0 + PIT_DEEP, PIT_HEIGHT, x2, y2, z2 + PIT_HEIGHT);

            x2 = x;
            y2 = y;
            SETPOINT(1.0, 0, x2, y2, z2);

            x2 = x - PIT_DEEP * normvec.x;
            y2 = y - PIT_DEEP * normvec.y;
            SETPOINT(1.0 + PIT_DEEP, 0, x2, y2, z2);

            for (i = 0; i < pits->driversPitsNb; i++)
            {
                startNeeded = true;
                sprintf(sname, "P%dts", uid++);
                CHECKDISPLIST3("concrete.png", 4, sname, pits->driversPits[i].pos.seg->id);

                RtTrackLocal2Global(&(pits->driversPits[i].pos), &x, &y, pits->driversPits[i].pos.type);
                RtTrackSideNormalG(pits->driversPits[i].pos.seg, x, y, pits->side, &normvec);
                x2 = x;
                y2 = y;
                z2 = RtTrackHeightG(pits->driversPits[i].pos.seg, x2, y2);

                if (pits->side == TR_RGT)
                {
                    x3 = x + pits->len * normvec.y;
                    y3 = y - pits->len * normvec.x;
                }
                else
                {
                    x3 = x - pits->len * normvec.y;
                    y3 = y + pits->len * normvec.x;
                }

                z3 = RtTrackHeightG(pits->driversPits[i].pos.seg, x3, y3);
                SETPOINT(pits->len, 0, x2, y2, z2 + PIT_HEIGHT - PIT_TOP);
                SETPOINT(0, 0, x3, y3, z3 + PIT_HEIGHT - PIT_TOP);

                tdble dx = PIT_TOP * normvec.x;
                tdble dy = PIT_TOP * normvec.y;
                SETPOINT(pits->len, PIT_TOP, x2 + dx, y2 + dy, z2 + PIT_HEIGHT - PIT_TOP);
                SETPOINT(0, PIT_TOP, x3 + dx, y3 + dy, z3 + PIT_HEIGHT - PIT_TOP);
                SETPOINT(pits->len, 2 * PIT_TOP, x2 + dx, y2 + dy, z2 + PIT_HEIGHT);
                SETPOINT(0, 2 * PIT_TOP, x3 + dx, y3 + dy, z3 + PIT_HEIGHT);

                dx = -PIT_DEEP * normvec.x;
                dy = -PIT_DEEP * normvec.y;

                SETPOINT(pits->len, 2 * PIT_TOP + PIT_DEEP, x2 + dx, y2 + dy, z2 + PIT_HEIGHT);
                SETPOINT(0, 2 * PIT_TOP + PIT_DEEP, x3 + dx, y3 + dy, z3 + PIT_HEIGHT);
                SETPOINT(pits->len, 2 * PIT_TOP + PIT_DEEP + PIT_HEIGHT, x2 + dx, y2 + dy, z2);
                SETPOINT(0, 2 * PIT_TOP + PIT_DEEP + PIT_HEIGHT, x3 + dx, y3 + dy, z3);
            } // for i

            startNeeded = true;
            i--;
            sprintf(sname, "P%dts", uid++);
            CHECKDISPLIST3("concrete2.png", 4, sname, pits->driversPits[i].pos.seg->id);

            RtTrackLocal2Global(&(pits->driversPits[i].pos), &x, &y, pits->driversPits[i].pos.type);
            RtTrackSideNormalG(pits->driversPits[i].pos.seg, x, y, pits->side, &normvec);

            if (pits->side == TR_RGT)
            {
                x += pits->len * normvec.y;
                y -= pits->len * normvec.x;
            }
            else
            {
                x -= pits->len * normvec.y;
                y += pits->len * normvec.x;
            }

            z2 = RtTrackHeightG(pits->driversPits[i].pos.seg, x, y);
            x2 = x + PIT_TOP * normvec.x;
            y2 = y + PIT_TOP * normvec.y;
            SETPOINT(1.0 - PIT_TOP, PIT_HEIGHT, x2, y2, z2 + PIT_HEIGHT);
            SETPOINT(1.0 - PIT_TOP, PIT_HEIGHT - PIT_TOP, x2, y2, z2 + PIT_HEIGHT - PIT_TOP);

            x2 = x - PIT_DEEP * normvec.x;
            y2 = y - PIT_DEEP * normvec.y;
            SETPOINT(1.0 + PIT_DEEP, PIT_HEIGHT, x2, y2, z2 + PIT_HEIGHT);

            x2 = x;
            y2 = y;
            SETPOINT(1.0, PIT_HEIGHT - PIT_TOP, x2, y2, z2 + PIT_HEIGHT - PIT_TOP);

            x2 = x - PIT_DEEP * normvec.x;
            y2 = y - PIT_DEEP * normvec.y;
            SETPOINT(1.0 + PIT_DEEP, 0, x2, y2, z2);

            x2 = x;
            y2 = y;
            SETPOINT(1.0, 0, x2, y2, z2);
            break; // TR_PIT_ON_TRACK_SIDE
        }
        case TR_PIT_NO_BUILDING:
            break;

        case TR_PIT_ON_SEPARATE_PATH:
            break;

        case TR_PIT_NONE:
            break;

        } // switch pits->type
    }
    CLOSEDISPLIST();
    printf("=== Indices really used = %d\n", nbvert);

    return 0;
} // InitScene

static void saveObject(FILE *curFd, int nb, int start, char *texture, char *name, int surfType)
{
    int i, index;

    fprintf(curFd, "OBJECT poly\n");
    fprintf(curFd, "name \"%s\"\n", name);
    fprintf(curFd, "texture \"%s\"\n", texture);
    fprintf(curFd, "numvert %d\n", nb);

    for (i = 0; i < nb; i++)
    {
        index = 3 * (start + i);
        fprintf(curFd, "%f %f %f\n", trackvertices[index], trackvertices[index + 2], -trackvertices[index + 1]);
    }

    fprintf(curFd, "numsurf %d\n", nb - 2);
    fprintf(curFd, "SURF 0x%02x\n", surfType);
    fprintf(curFd, "mat 0\n");
    fprintf(curFd, "refs 3\n");
    fprintf(curFd, "%d %f %f\n", 0, tracktexcoord[2 * start], tracktexcoord[2 * start + 1]);
    fprintf(curFd, "%d %f %f\n", 1, tracktexcoord[2 * (start + 1)], tracktexcoord[2 * (start + 1) + 1]);
    fprintf(curFd, "%d %f %f\n", 2, tracktexcoord[2 * (start + 2)], tracktexcoord[2 * (start + 2) + 1]);

    /* triangle strip conversion to triangles */
    for (i = 2; i < nb - 1; i++)
    {
        fprintf(curFd, "SURF 0x%02x\n", surfType);
        fprintf(curFd, "mat 0\n");
        fprintf(curFd, "refs 3\n");
        if ((i % 2) == 0)
        {
            index = i;
            fprintf(curFd, "%d %f %f\n", index, tracktexcoord[2 * (start + index)], tracktexcoord[2 * (start + index) + 1]);
            index = i - 1;
            fprintf(curFd, "%d %f %f\n", index, tracktexcoord[2 * (start + index)], tracktexcoord[2 * (start + index) + 1]);
            index = i + 1;
            fprintf(curFd, "%d %f %f\n", index, tracktexcoord[2 * (start + index)], tracktexcoord[2 * (start + index) + 1]);
        }
        else
        {
            index = i - 1;
            fprintf(curFd, "%d %f %f\n", index, tracktexcoord[2 * (start + index)], tracktexcoord[2 * (start + index) + 1]);
            index = i;
            fprintf(curFd, "%d %f %f\n", index, tracktexcoord[2 * (start + index)], tracktexcoord[2 * (start + index) + 1]);
            index = i + 1;
            fprintf(curFd, "%d %f %f\n", index, tracktexcoord[2 * (start + index)], tracktexcoord[2 * (start + index) + 1]);
        }
    }
    fprintf(curFd, "kids 0\n");
}

static void SaveMainTrack(FILE *curFd, bool bump, bool raceline)
{
    tDispElt *aDispElt;
    char buf[256];
    int i;

    for (i = 0; i < GroupNb; i++)
    {
        if (Groups[i].nb != 0)
        {
            aDispElt = Groups[i].dispList;
            sprintf(buf, "TKMN%d", i);
            Ac3dGroup(curFd, buf, Groups[i].nb);
            do
            {
                aDispElt = aDispElt->next;
                if (aDispElt->nb != 0)
                {
                    sprintf(buf, "%s%d", aDispElt->name, aDispElt->id);

                     // skip bad display lists
                    if (aDispElt->nb > 0 && aDispElt->nb < 3)
                    {
                        GfLogWarning("Bug: Display list %s has %d vertices!\n", buf, aDispElt->nb);
                        continue;
                    }

                    if (bump)
                    {
                        saveObject(curFd, aDispElt->nb, aDispElt->start, aDispElt->texture->namebump, buf, aDispElt->surfType);
                    }
                    else if (raceline)
                    {
                        saveObject(curFd, aDispElt->nb, aDispElt->start, aDispElt->texture->nameraceline, buf, aDispElt->surfType);
                    }
                    else
                    {
                        saveObject(curFd, aDispElt->nb, aDispElt->start, aDispElt->texture->name, buf, aDispElt->surfType);
                    }
                }
            } while (aDispElt != Groups[i].dispList);
        }
    }
}

/** Calculate track parameters and exit without any file creation
    It is for information only, mainly for use from TrackEditor.
    @param	Track	track structure
    @param	TrackHandle	handle on the track description
    @return	none
*/
void CalculateTrack(tTrack *Track, void *TrackHandle, bool bump, bool raceline, bool bridge)
{
    TrackStep = GfParmGetNum(TrackHandle, TRK_SECT_TERRAIN, TRK_ATT_TSTEP, nullptr, TrackStep);
    printf("Track step: %.2f ", TrackStep);

    InitScene(Track, TrackHandle, bump, raceline, bridge);

    printf("Calculation finished\n");
}

/** Generate the track AC3D file(s).
    @param	Track	track structure
    @param	TrackHandle	handle on the track description
    @param	outFile	output file name for track only
    @param	AllFd	fd of the merged file
    @return	none
*/
void GenerateTrack(tTrack *Track, void *TrackHandle, const std::string &outFile, FILE *AllFd, bool bump, bool raceline, bool bridge)
{
    TrackStep = GfParmGetNum(TrackHandle, TRK_SECT_TERRAIN, TRK_ATT_TSTEP, nullptr, TrackStep);
    printf("Track step: %.2f ", TrackStep);

    InitScene(Track, TrackHandle, bump, raceline, bridge);

    if (!outFile.empty())
    {
        FILE *curFd = Ac3dOpen(outFile, 1);
        Ac3dGroup(curFd, "track", ActiveGroups);
        SaveMainTrack(curFd, bump, raceline);
        Ac3dClose(curFd);
    }

    if (AllFd)
    {
        Ac3dGroup(AllFd, "track", ActiveGroups);
        SaveMainTrack(AllFd, bump, raceline);
    }
}
