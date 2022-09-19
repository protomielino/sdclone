/***************************************************************************

    file                 : relief.cpp
    created              : Tue Mar  6 23:15:19 CET 2001
    copyright            : (C) 2000 by Eric Espie
    email                : eric.espie@torcs.org
    version              : $Id$

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
#else
#include <windows.h>
#endif
#include <cmath>
#include <plib/ssg.h>

#include "easymesh.h"
#include "trackgen.h"
#include <portability.h>
#include <tgfclient.h>
#include <track.h>

#include "relief.h"

typedef struct Line
{
    GF_TAILQ_ENTRY(struct Line) link;
    ssgBranch *branch;
} tLine;

GF_TAILQ_HEAD(RingListHead, tLine);

tRingListHead InteriorList;
tRingListHead ExteriorList;

static tdble GridStep;

static ssgEntity *Root = nullptr;

/*
 * Read the faces from AC3D file
 * separate between interior and exterior lines
 */
static ssgBranch *hookNode(char *s)
{
    tLine *line;

    line = reinterpret_cast<tLine *>(calloc(1, sizeof(tLine)));
    line->branch = new ssgBranch();

    if (strncmp(s, "interior", 8) == 0)
    {
        GF_TAILQ_INSERT_TAIL(&InteriorList, line, link);
    }
    else
    {
        GF_TAILQ_INSERT_TAIL(&ExteriorList, line, link);
    }
    return line->branch;
}

/*
  Load a simple database
*/
void LoadRelief(tTrack *track, void *TrackHandle, const char *reliefFile)
{
    GF_TAILQ_INIT(&InteriorList);
    GF_TAILQ_INIT(&ExteriorList);

    GridStep = GfParmGetNum(TrackHandle, TRK_SECT_TERRAIN, TRK_ATT_BSTEP, nullptr, GridStep);

    ssgLoaderOptions options;

    options.setCreateBranchCallback(hookNode);

    ssgSetCurrentOptions(&options);

    printf("\nLoading relief file %s\n", reliefFile);

    Root = ssgLoadAC(reliefFile);
}

static void countRec(ssgEntity *e, int *nb_vert, int *nb_seg)
{
    if (e->isAKindOf(_SSG_TYPE_BRANCH))
    {
        ssgBranch *br = dynamic_cast<ssgBranch *>(e);

        for (int i = 0; i < br->getNumKids(); i++)
        {
            countRec(br->getKid(i), nb_vert, nb_seg);
        }
    }
    else if (e->isAKindOf(_SSG_TYPE_VTXTABLE))
    {
        ssgVtxTable *vt = dynamic_cast<ssgVtxTable *>(e);

        *nb_vert += vt->getNumVertices();
        *nb_seg += vt->getNumLines();
    }
}

void CountRelief(bool interior, int *nb_vert, int *nb_seg)
{
    tLine *curLine;
    tRingListHead *curHead;

    *nb_vert = *nb_seg = 0;

    if (Root == nullptr)
    {
        return;
    }

    if (interior)
    {
        curHead = &InteriorList;
    }
    else
    {
        curHead = &ExteriorList;
    }

    curLine = GF_TAILQ_FIRST(curHead);
    while (curLine != nullptr)
    {
        ssgBranch *br = curLine->branch->getParent(0);
        ssgBranch *br2 = new ssgBranch();

        br2->addKid(br);
        ssgFlatten(br);
        curLine->branch = br2;

        countRec(dynamic_cast<ssgEntity *>(curLine->branch), nb_vert, nb_seg);

        curLine = GF_TAILQ_NEXT(curLine, link);
    }
}

static void genRec(ssgEntity *e)
{
    if (e->isAKindOf(_SSG_TYPE_BRANCH))
    {
        ssgBranch *br = dynamic_cast<ssgBranch *>(e);

        for (int i = 0; i < br->getNumKids(); i++)
        {
            genRec(br->getKid(i));
        }
    }
    else if (e->isAKindOf(_SSG_TYPE_VTXTABLE))
    {
        ssgVtxTable *vt = dynamic_cast<ssgVtxTable *>(e);

        int nv = vt->getNumVertices();
        int nl = vt->getNumLines();
        int sv = getPointCount();

        for (int i = 0; i < nv; i++)
        {
            float *vtx = vt->getVertex(i);

            addPoint(vtx[0], vtx[1], vtx[2], GridStep, 100000);
        }

        for (int i = 0; i < nl; i++)
        {
            short vv0, vv1;

            vt->getLine(i, &vv0, &vv1);
            addSegment(vv0 + sv, vv1 + sv, 100000);
        }
    }
}

void GenRelief(bool interior)
{
    tLine *curLine;
    tRingListHead *curHead;

    if (Root == nullptr)
    {
        return;
    }

    if (interior)
    {
        curHead = &InteriorList;
    }
    else
    {
        curHead = &ExteriorList;
    }

    curLine = GF_TAILQ_FIRST(curHead);
    while (curLine != nullptr)
    {
        genRec(dynamic_cast<ssgEntity *>(curLine->branch));

        curLine = GF_TAILQ_NEXT(curLine, link);
    }
}
