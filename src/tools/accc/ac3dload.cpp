/***************************************************************************

 file        : ac3dload.cpp
 created     : Fri Apr 18 23:00:28 CEST 2003
 copyright   : (C) 2003 by Christophe Guionneau
 version     : $Id$

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

 @author Christophe Guionneau
 @version    $Id$
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <cstring>
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <math.h>
#include <float.h>
#include "accc.h"

#if _MSC_VER && _MSC_VER < 1700
#define to_string(x) to_string(static_cast<long long>(x))
#endif

#define AC3D     "AC3Db"
#define MATERIAL "MATERIAL"
#define OBJECT   "OBJECT"
#define KIDS     "Kids"
#define NAME     "name"
#define LOC      "loc"
#define DATA     "data"
#define TEXTURE  "texture"
#define TEXREP   "texrep"
#define NUMVERT  "numvert"
#define NUMSURF  "numsurf"
#define SURF     "SURF"
#define MAT      "mat"
#define REFS     "refs"
#define CREASE   "crease"

ob_t::ob_t() :
kids(0),
loc(0.0, 0.0, 0.0),
attrSurf(0),
attrMat(0),
texrep_x(0.0),
texrep_y(0.0),
numvert(0),
numsurf(0),
numvertice(0),
va(nullptr),
surfrefs(nullptr),
next(nullptr),
x_min(0.0),
y_min(0.0),
z_min(0.0),
x_max(0.0),
y_max(0.0),
z_max(0.0),
dist_min(0.0),
saved(false),
kids_o(0),
inkids_o(false)
{
}

ob_t::~ob_t()
{
    free(va);
    free(surfrefs);
}

ob_t * obAppend(ob_t * destob, ob_t * srcob)
{
    if(!destob)
        return srcob;
    if(!srcob)
        return destob;

    ob_t * curob = destob;

    while(curob->next != NULL)
        curob = curob->next;

    curob->next = srcob;

    return destob;
}

void obInitSpacialExtend(ob_t * ob)
{
    ob->x_min = ob->y_min = ob->z_min = DBL_MAX;
    ob->x_max = ob->y_max = ob->z_max = DBL_MIN;

    for (int v = 0; v < ob->numvertice; v++)
    {
        if (ob->vertex[v].x > ob->x_max)
            ob->x_max = ob->vertex[v].x;
        if (ob->vertex[v].x < ob->x_min)
            ob->x_min = ob->vertex[v].x;

        if (ob->vertex[v].y > ob->y_max)
            ob->y_max = ob->vertex[v].y;
        if (ob->vertex[v].y < ob->y_min)
            ob->y_min = ob->vertex[v].y;

        if (ob->vertex[v].z > ob->z_max)
            ob->z_max = ob->vertex[v].z;
        if (ob->vertex[v].z < ob->z_min)
            ob->z_min = ob->vertex[v].z;
    }
}

void obCreateTextArrays(ob_t * ob)
{
    if (!ob->vertexarray.empty())
    {
        ob->textarray.resize(ob->numvertice);
        for (int i = 0; i < ob->numsurf * 3; i++)
            ob->textarray[ob->vertexarray[i].indice] = ob->vertexarray[i].uv;
    }

    if (!ob->vertexarray1.empty())
    {
        ob->textarray1.resize(ob->numvertice);
        for (int i = 0; i < ob->numsurf * 3; i++)
            ob->textarray1[ob->vertexarray1[i].indice] = ob->vertexarray1[i].uv;
    }

    if (!ob->vertexarray2.empty())
    {
        ob->textarray2.resize(ob->numvertice);
        for (int i = 0; i < ob->numsurf * 3; i++)
            ob->textarray2[ob->vertexarray2[i].indice] = ob->vertexarray2[i].uv;
    }

    if (!ob->vertexarray3.empty())
    {
        ob->textarray3.resize(ob->numvertice);
        for (int i = 0; i < ob->numsurf * 3; i++)
            ob->textarray3[ob->vertexarray3[i].indice] = ob->vertexarray3[i].uv;
    }
}

void obCreateVertexArrays(ob_t * ob)
{
    int numEls = ob->numsurf * 3;

    if (ob->hasTexture())
        ob->vertexarray.resize(numEls);

    if (ob->hasTexture1())
        ob->vertexarray1.resize(numEls);

    if (ob->hasTexture2())
        ob->vertexarray2.resize(numEls);

    if (ob->hasTexture3())
        ob->vertexarray3.resize(numEls);
}

void obCopyTextureNames(ob_t * destob, const ob_t * srcob)
{
    destob->texture = srcob->texture;
    destob->texture1 = srcob->texture1;
    destob->texture2 = srcob->texture2;
    destob->texture3 = srcob->texture3;
}

void obSetVertexArraysIndex(ob_t * ob, int vaIdx, int newIndex)
{
    ob->vertexarray[vaIdx].indice = newIndex;

    if (!ob->vertexarray1.empty())
        ob->vertexarray1[vaIdx].indice = newIndex;
    if (!ob->vertexarray2.empty())
        ob->vertexarray2[vaIdx].indice = newIndex;
    if (!ob->vertexarray3.empty())
        ob->vertexarray3[vaIdx].indice = newIndex;
}

#ifndef M_PI
#define M_PI 3.14159267
#endif
void computeTriNorm(ob_t * object);
void smoothTriNorm(ob_t * object);
void computeObjectTriNorm(ob_t * object);
void smoothFaceTriNorm(ob_t * object);
void smoothObjectTriNorm(ob_t * object);
void normalize(point_t *t);
bool checkMustSmoothVector(point_t *n1, point_t *n2, point_t *t1, point_t *t2);
void mapNormalToSphere(ob_t * object);
void mapNormalToSphere2(ob_t * object);
void normalMap(ob_t * object);
void mapTextureEnv(ob_t * object);
point_t tmpPoint[100000];
tcoord_t tmpva[100000];
uv_t tmptexa[100000];
int tmpsurf[100000];
int refs = 0;
const char * const shadowtexture = "shadow2.png";

int numob = 0;
int nummaterial = 0;
int numvertex = 0;
int dataSize = 0;
int dataSizeRead = 0;
bool numvertFound = false;
bool numrefsFound = false;
bool dataFound = false;
int attrSurf = 0;
int attrMat = 0;
int numrefs = 0;
int numrefstotal = 0;
char tmpname[256];
int tmpIndice = 0;
int tmpIndice2 = 0;
int tmpIndice3 = 0;
int vert;
int numvertice = 0;
char tex[256][256];
int texnum = 0;
struct verbaction_t
{
    const char * verb;
    int (*doVerb)(char * Line, ob_t *object, std::vector<mat_t> &materials);
};

int doMaterial(char *Line, ob_t *object, std::vector<mat_t> &materials);
int doObject(char *Line, ob_t *object, std::vector<mat_t> &materials);
int doKids(char *Line, ob_t *object, std::vector<mat_t> &materials);
int doName(char *Line, ob_t *object, std::vector<mat_t> &materials);
int doLoc(char *Line, ob_t *object, std::vector<mat_t> &materials);
int doData(char *Line, ob_t *object, std::vector<mat_t> &materials);
int doTexture(char *Line, ob_t *object, std::vector<mat_t> &materials);
int doTexrep(char *Line, ob_t *object, std::vector<mat_t> &materials);
int doNumvert(char *Line, ob_t *object, std::vector<mat_t> &materials);
int doNumsurf(char *Line, ob_t *object, std::vector<mat_t> &materials);
int doSurf(char *Line, ob_t *object, std::vector<mat_t> &materials);
int doMat(char *Line, ob_t *object, std::vector<mat_t> &materials);
int doRefs(char *Line, ob_t *object, std::vector<mat_t> &materials);
int doCrease(char *Line, ob_t *object, std::vector<mat_t> &materials);

#ifdef _3DS
void saveObin3DS(const std::string & OutputFilename, ob_t * object, std::vector<mat_t> &materials);
#endif
void computeSaveAC3D(const std::string & OutputFilename, ob_t * object, const std::vector<mat_t> &materials);
void computeSaveOBJ(const std::string & OutputFilename, ob_t * object, const std::vector<mat_t> &materials);
void computeSaveAC3DM(const std::string & OutputFilename, ob_t * object, const std::vector<mat_t> &materials);
void computeSaveAC3DStrip(const std::string & OutputFilename, ob_t * object, const std::vector<mat_t> &materials);
void stripifyOb(FILE * ofile, ob_t * object, int writeit);

verbaction_t verbTab[] =
{
{ MATERIAL, doMaterial },
{ OBJECT, doObject },
{ KIDS, doKids },
{ NAME, doName },
{ LOC, doLoc },
{ DATA, doData },
{ TEXTURE, doTexture },
{ TEXREP, doTexrep },
{ NUMVERT, doNumvert },
{ NUMSURF, doNumsurf },
{ SURF, doSurf },
{ MAT, doMat },
{ REFS, doRefs },
{ CREASE, doCrease },
{ "END", NULL } };

void copyVertexArraysSurface(ob_t * destob, int destSurfIdx, ob_t * srcob, int srcSurfIdx)
{
    int firstDestIdx = destSurfIdx * 3;
    int firstSrcIdx = srcSurfIdx * 3;

    for (int off = 0; off < 3; off++)
    {
        destob->vertexarray[firstDestIdx + off] = srcob->vertexarray[firstSrcIdx + off];

        if (!srcob->vertexarray1.empty())
            destob->vertexarray1[firstDestIdx + off] = srcob->vertexarray1[firstSrcIdx + off];
        if (!srcob->vertexarray2.empty())
            destob->vertexarray2[firstDestIdx + off] = srcob->vertexarray2[firstSrcIdx + off];
        if (!srcob->vertexarray3.empty())
            destob->vertexarray3[firstDestIdx + off] = srcob->vertexarray3[firstSrcIdx + off];
    }
}

/** copy the (u,v) coords from srcidxarr to the corresponding position in destarr.
 *  destarr needs to have 2 * number of vertices entries.
 */
void createTexCoordArray(uv_t * destarr, tcoord_t * srcidxarr, int numidx)
{
    tcoord_t * curidxobj = NULL;

    for (int curidx = 0; curidx < numidx; curidx++)
    {
        curidxobj = &srcidxarr[curidx];

        destarr[curidxobj->indice] = curidxobj->uv;
    }
}

/** Creates a new object from the given data. This function is used during object splitting.
 *
 *  @param splitid the id of this split object
 *  @param srcobj the original object, which was split
 *  @param tmpob the temporary storage object using in splitting
 *
 *  In the tmpob the following should be set: numvertice, numsurf, vertex, snorm,
 *  vertexarray, textarray
 *
 *  @return the created split object
 */
ob_t * createObjectSplitCopy(int splitid, const ob_t * srcobj, const ob_t * tmpob)
{
    /* allocate space */
    ob_t * retob = new ob_t;

    /* special handling of name */
    retob->name = srcobj->name + "_s_" + std::to_string(splitid);

    retob->type = srcobj->type;
    retob->attrSurf = srcobj->attrSurf;
    retob->attrMat = srcobj->attrMat;
    retob->texture = srcobj->texture;
    retob->texture1 = srcobj->texture1;
    retob->texture2 = srcobj->texture2;
    retob->texture3 = srcobj->texture3;
    retob->data = srcobj->data;
    retob->numvert = tmpob->numvertice;
    retob->numsurf = tmpob->numsurf;
    retob->numvertice = tmpob->numvertice;
    retob->vertex = tmpob->vertex;
    retob->norm = tmpob->norm;
    retob->snorm = tmpob->snorm;
    retob->vertexarray = tmpob->vertexarray;
    retob->textarray = tmpob->textarray;
    retob->vertexarray1 = tmpob->vertexarray1;
    retob->textarray1 = tmpob->textarray1;
    retob->vertexarray2 = tmpob->vertexarray2;
    retob->textarray2 = tmpob->textarray2;
    retob->vertexarray3 = tmpob->vertexarray3;
    retob->textarray3 = tmpob->textarray3;

    return retob;
}

void copyTexChannel(std::vector<uv_t> & desttextarray, std::vector<tcoord_t> & destvertexarray, tcoord_t * srcvert,
    int storedptidx, int destptidx, int destvertidx)
{
    desttextarray[destptidx] = srcvert->uv;

    destvertexarray[destvertidx].set(storedptidx, srcvert->uv, 0);
}

void copySingleVertexData(ob_t * destob, ob_t * srcob,
    int storedptidx, int destptidx, int destvertidx, int srcvertidx)
{
    tcoord_t * srcvert;

    /* channel 0 */
    if (!destob->textarray.empty())
    {
        srcvert = &(srcob->vertexarray[srcvertidx]);

        copyTexChannel(destob->textarray, destob->vertexarray, srcvert,
                storedptidx, destptidx, destvertidx);
    }

    /* channel 1 */
    if (!destob->textarray1.empty())
    {
        srcvert = &(srcob->vertexarray1[srcvertidx]);

        copyTexChannel(destob->textarray1, destob->vertexarray1, srcvert,
                storedptidx, destptidx, destvertidx);
    }

    /* channel 2 */
    if (!destob->textarray2.empty())
    {
        srcvert = &(srcob->vertexarray2[srcvertidx]);

        copyTexChannel(destob->textarray2, destob->vertexarray2, srcvert,
                storedptidx, destptidx, destvertidx);
    }

    /* channel 3 */
    if (!destob->textarray3.empty())
    {
        srcvert = &(srcob->vertexarray3[srcvertidx]);

        copyTexChannel(destob->textarray3, destob->vertexarray3, srcvert,
                storedptidx, destptidx, destvertidx);
    }
}

void clearSavedInVertexArrayEntry(ob_t * object, int vertidx)
{
    object->vertexarray[vertidx].saved = false;
}

int computeNorm(point_t * pv1, point_t *pv2, point_t *pv3, point_t *norm)
{
    double p1, p2, p3, q1, q2, q3, dd;
    double x1, y1, z1, x2, y2, z2, x3, y3, z3;

    x1 = pv1->x;
    y1 = pv1->y;
    z1 = pv1->z;

    x2 = pv2->x;
    y2 = pv2->y;
    z2 = pv2->z;

    x3 = pv3->x;
    y3 = pv3->y;
    z3 = pv3->z;

    if (((x1 == x2) && (y1 == y2) && (z1 == z2))
            || ((x1 == x3) && (y1 == y3) && (z1 == z3))
            || ((x2 == x3) && (y2 == y3) && (z2 == z3)))
    {
        norm->x = 0;
        norm->y = 1.0;
        norm->z = 0;
        return 0;
    }

    p1 = x2 - x1;
    p2 = y2 - y1;
    p3 = z2 - z1;

    q1 = x3 - x1;
    q2 = y3 - y1;
    q3 = z3 - z1;

    dd = sqrt(
            (p2 * q3 - q2 * p3) * (p2 * q3 - q2 * p3)
                    + (p3 * q1 - q3 * p1) * (p3 * q1 - q3 * p1)
                    + (p1 * q2 - q1 * p2) * (p1 * q2 - q1 * p2));
    if (dd == 0.0)
    {
        norm->set(0, 1.0, 0);
        return 0;
    }

    norm->x = (p2 * q3 - q2 * p3) / dd;
    norm->y = (p3 * q1 - q3 * p1) / dd;
    norm->z = (p1 * q2 - q1 * p2) / dd;

    if (isnan(norm->x) || isnan(norm->y) || isnan(norm->z))
    {
        norm->set(0, 1.0, 0);
        return 0;
    }

    return 0;
}

void computeObSurfCentroid(const ob_t * object, int obsurf, point_t * out)
{
    int firstIdx = obsurf * 3;

    out->set(0, 0, 0);

    for (int curVert = 0; curVert < 3; curVert++)
        *out += object->vertex[object->vertexarray[firstIdx + curVert].indice];

    *out /= 3;
}

int doMaterial(char *Line, ob_t *object, std::vector<mat_t> &materials)
{
    char * p;
    char name[256] = { 0 };
    mat_t materialt;

    nummaterial++;

    p = strstr(Line, " ");
    if (p == NULL)
    {
        fprintf(stderr, "unknown MATERIAL format %s\n", Line);
        return (-1);
    }
    if (sscanf(p,
            "%255s rgb %lf %lf %lf amb %lf %lf %lf emis %lf %lf %lf spec %lf %lf %lf shi %lf trans %lf",
            name, &(materialt.rgb.r), &(materialt.rgb.g), &(materialt.rgb.b),
            &(materialt.amb.r), &(materialt.amb.g), &(materialt.amb.b),
            &(materialt.emis.r), &(materialt.emis.g), &(materialt.emis.b),
            &(materialt.spec.r), &(materialt.spec.g), &(materialt.spec.b),
            &(materialt.shi), &(materialt.trans)) != 15)
    {
        fprintf(stderr, "invalid MATERIAL format %s\n", p);
        return (-1);
    }

    materialt.name = name;

    // append to list
    materials.push_back(materialt); // use emplace_back someday

    return (0);
}

int doObject(char *Line, ob_t *object, std::vector<mat_t> &materials)
{
    char * p;
    char name[256] = { 0 };

    p = strstr(Line, " ");
    if (p == NULL)
    {
        fprintf(stderr, "unknown OBJECT format %s\n", Line);
        return (-1);
    }
    if (sscanf(p, "%255s", name) != 1)
    {
        fprintf(stderr, "invalid OBJECT format %s\n", p);
        return (-1);
    }

    ob_t *objectt = new ob_t;

    objectt->x_min = 1000000;
    objectt->y_min = 1000000;
    objectt->z_min = 1000000;
    objectt->type = name;
    objectt->texrep_x = 1.0;
    objectt->texrep_y = 1.0;

    ob_t *t1 = object->next;
    object->next = objectt;
    objectt->next = t1;

    numob++;
    numrefs = 0;
    numvertFound = false;
    dataFound = false;

    return (0);
}

int findIndice(int indice, const std::vector<int> &oldva, int n)
{
    for (int i = 0; i < n; i++)
    {
        if (oldva[i] == indice)
            return i;
    }
    return -1;
}

ob_t* terrainSplitOb(ob_t * object)
{
    ob_t * tob = NULL;
    ob_t * tob0 = NULL;

    printf("terrain splitting %s\n", object->name.c_str());
    if ((object->x_max - object->x_min) < 2 * distSplit)
        return 0;
    if ((object->y_max - object->y_min) < 2 * distSplit)
        return 0;
    printf("terrain splitting %s started\n", object->name.c_str());

    int numSurf = object->numsurf;
    std::vector<int> oldSurfToNewObjMap(numSurf, 0);

    int numNewObjs = 0;

    for (double curXPos = object->x_min; curXPos < object->x_max; curXPos += distSplit)
    {
        for (double curYPos = object->y_min; curYPos < object->y_max; curYPos += distSplit)
        {
            int numTriFound = 0;
            bool found_a_tri = false;

            for (int curObjSurf = 0; curObjSurf < object->numsurf; curObjSurf++)
            {
                point_t surfCentroid;
                computeObSurfCentroid(object, curObjSurf, &surfCentroid);

                if (surfCentroid.x >= curXPos && surfCentroid.x < curXPos + distSplit)
                {
                    if (surfCentroid.y >= curYPos && surfCentroid.y < curYPos + distSplit)
                    {
                        found_a_tri = true;
                        oldSurfToNewObjMap[curObjSurf] = numNewObjs;
                        numTriFound++;
                    }
                }
            }

            if (found_a_tri)
            {
                printf("surface num %d : numtri : %d\n", numNewObjs, numTriFound);
                numNewObjs++;
            }
        }
    }
    printf("found in %s : %d subsurfaces\n", object->name.c_str(), numNewObjs);

    for (int curNewObj = 0; curNewObj < numNewObjs; curNewObj++)
    {
        int numNewSurf = 0;
        /* find the number of surface */
        for (int curSurf = 0; curSurf < object->numsurf; curSurf++)
        {
            if (oldSurfToNewObjMap[curSurf] != curNewObj)
                continue;
            numNewSurf++;
        }

        /* initial creation of tob */

        tob = new ob_t;

        tob->numsurf = numNewSurf;
        tob->attrSurf = object->attrSurf;
        tob->attrMat = object->attrMat;
        tob->data = object->data;
        tob->type = object->type;

        /* special name handling */
        tob->name = object->name + "__split__" + std::to_string(curNewObj);

        obCopyTextureNames(tob, object);

        /* store the index data in tob's vertexarray */

        obCreateVertexArrays(tob);

        int curNewSurf = 0;
        for (int curSurf = 0; curSurf < object->numsurf; curSurf++)
        {
            if (oldSurfToNewObjMap[curSurf] != curNewObj)
                continue;

            copyVertexArraysSurface(tob, curNewSurf, object, curSurf);

            curNewSurf++;
        }

        /* create a list with temporal points and smoothed normals and store the index
         * to them in tob's vertexarray.indice property.
         */

        /* temporal storage for points and smoothed normals. Temporal because
         * we don't know the size, so we allocate the same number as in the
         * source object.
         */
        std::vector<point_t> pttmp(object->numvertice, point_t(0.0, 0.0, 0.0));
        std::vector<point_t> snorm(object->numvertice, point_t(0.0, 0.0, 0.0));

        /* storedPtIdxArr: keep a list of the indices of points stored in the new object.
         * If an index is contained in storedPtIdxArr we don't store the point itself,
         * but only the index in the vertexarray of the new object.
         */
        std::vector<int> storedPtIdxArr(object->numvertice, 0);

        int curNewPtIdx = 0;
        for (int curNewIdx = 0; curNewIdx < numNewSurf * 3; curNewIdx++)
        {
            int idx = tob->vertexarray[curNewIdx].indice;

            int storedIdx = findIndice(idx, storedPtIdxArr, curNewPtIdx);
            if (storedIdx == -1)
            {
                storedPtIdxArr[curNewPtIdx] = idx;
                storedIdx = curNewPtIdx;
                pttmp[curNewPtIdx] = object->vertex[idx];
                snorm[curNewPtIdx] = object->norm[idx];
                curNewPtIdx++;
            }

            obSetVertexArraysIndex(tob, curNewIdx, storedIdx);
        }

        int numNewPts = curNewPtIdx;

        tob->numvert = numNewPts;
        tob->numvertice = numNewPts;

        /* create and store tob's norm, snorm, vertex and textarray data */

        tob->vertex = pttmp;
        tob->norm = snorm;
        tob->snorm = snorm;

        obCreateTextArrays(tob);

        obInitSpacialExtend(tob);

        // prepend the new object to the list
        tob0 = obAppend(tob, tob0);
    }

    return tob0;
}

ob_t* splitOb(ob_t *object)
{
    int oldnumptstored = 0; /* temporary placeholder for numptstored */

    /* The object we use as storage during splitting.
     * Following attribs will be used: vertexarray, vertex, snorm, textarray
     */
    ob_t workob;

    tcoord_t curvertex[3];
    int curstoredidx[3];

    bool touse = false;
    int orignumtris = object->numsurf; /* number of surfaces/triangles in the source object */
    int orignumverts = orignumtris * 3; /* number of vertices in the source object: orignumtris * 3 */
    bool mustcontinue = true;
    ob_t * tob0 = NULL;
    int numobject = 0;
    int curvert = 0;

    std::vector<bool> tri(orignumtris, false);
    std::vector<int> oldva(orignumverts, 0);

    workob.vertex.resize(orignumverts);
    workob.norm.resize(orignumverts);
    workob.snorm.resize(orignumverts);
    workob.vertexarray = object->vertexarray;
    workob.textarray = object->textarray;
    workob.vertexarray1 = object->vertexarray1;
    workob.textarray1 = object->textarray1;
    workob.vertexarray2 = object->vertexarray2;
    workob.textarray2 = object->textarray2;
    workob.vertexarray3 = object->vertexarray3;
    workob.textarray3 = object->textarray3;

    while (mustcontinue)
    {
        mustcontinue = false;
		
        int numvertstored = 0; /* number of vertices stored in the object */
        int numtristored = 0; /* number of triangles stored in the object: numvertstored/3 */
        int numptstored = 0; /* number of vertices stored */
        bool firstTri = false;
        bool atleastone = true;
		
        while (atleastone)
        {
            atleastone = false;
            for (int curtri = 0; curtri < orignumtris; curtri++)
            {
                touse = false;
                if (tri[curtri])
                    continue;
                mustcontinue = 1;

                curvert = curtri * 3;

                /** find vertices of the triangle */
                for (int i = 0; i < 3; i++)
                {
                    curvertex[i] = object->vertexarray[curvert+i];

                    curstoredidx[i] = findIndice(curvertex[i].indice, oldva, numptstored);
                }

                if (curstoredidx[0] == -1 && curstoredidx[1] == -1 && curstoredidx[2] == -1)
                {
                    if (!firstTri)
                        touse = true;
                    else
                        touse = false;
                    /* triangle is ok */
                }
                else
                {
                    touse = true;

                    for (int i = 0; i < 3; i++)
                    {
                        if (curstoredidx[i] != -1)
                        {
                            if(workob.textarray[curstoredidx[i]] != curvertex[i].uv)
                            {
                                touse = false;
                                /* triangle is not ok */
                            }
                        }
                    }
                }

                if (touse)
                {
                    firstTri = true;
                    /* triangle is ok */

                    tri[curtri] = true; /* mark this triangle */

                    /* store the vertices of the triangle with new indice */
                    /* not yet in the array : store it at the current position */
                    for (int i = 0; i < 3; i++)
                    {
                        oldnumptstored = numptstored;

                        if (curstoredidx[i] == -1)
                        {
                            workob.vertex[numptstored] = object->vertex[curvertex[i].indice];
                            workob.norm[numptstored] = object->norm[curvertex[i].indice];
                            workob.snorm[numptstored] = object->snorm[curvertex[i].indice];

                            clearSavedInVertexArrayEntry(object, curvert+i);

                            oldva[numptstored] = curvertex[i].indice; /* remember the value of the vertice already saved */
                            curstoredidx[i] = numptstored;
                            numptstored++;
                        }

                        copySingleVertexData(&workob, object, curstoredidx[i],
                                oldnumptstored, numvertstored, curvert+i);

                        numvertstored++;
                    }

                    numtristored++;
                    atleastone = true;

                } // if (touse)

            } // for (curtri = 0; curtri < orignumtris; curtri++)
        } // while (atleastone)

        if (numtristored == 0)
            continue;

        /* must saved the object */
        workob.numvertice = numptstored;
        workob.numsurf = numvertstored/3;

        ob_t * tob = createObjectSplitCopy(numobject++, object, &workob);

        attrSurf = tob->attrSurf;
        attrMat = tob->attrMat;

        // prepend the new object to the list
        tob0 = obAppend(tob, tob0);

        printf("numtri = %d on orignumtris = %d\n", numtristored, orignumtris);

    } // while (mustcontinue == 1)

    return tob0;
}

int doKids(char* Line, ob_t* object, std::vector<mat_t> &materials)
{
    int kids;
    char *p = strstr(Line, " ");
    if (p == NULL)
    {
        fprintf(stderr, "unknown Kids format %s\n", Line);
        return (-1);
    }
    if (sscanf(p, "%d", &kids) != 1)
    {
        fprintf(stderr, "invalid Kids format %s\n", p);
        return (-1);
    }

    if (kids == 0)
    {
        object->next->vertexarray.resize(numrefstotal);
        object->next->textarray.resize(numrefstotal);
        object->next->surfrefs = (int *) malloc(sizeof(int) * numrefs);
        object->next->norm.assign(numrefstotal * 3, point_t(0.0, 0.0, 0.0));
        object->next->snorm.assign(numrefstotal * 3, point_t(0.0, 0.0, 0.0));
        object->next->attrSurf = attrSurf;
        object->next->attrMat = attrMat;
        attrSurf = 0x20;

        for (int i = 0; i < numrefstotal; i++)
        {
            object->next->vertexarray[i] = tmpva[i];
            object->next->textarray[i] = tmptexa[i];
        }

        memcpy(object->next->surfrefs, tmpsurf, numrefs * sizeof(int));
        object->next->numvertice = numvertice;

        if (!object->next->hasName())
        {
            object->next->name = tmpname + std::to_string(tmpIndice);

            tmpIndice++;
        }

        if ((typeConvertion == _AC3DTOAC3DS
                && (extendedStrips || extendedTriangles))
                || typeConvertion == _AC3DTOAC3DGROUP
                || (typeConvertion == _AC3DTOAC3D && extendedTriangles))
        {
            printf("Computing normals for %s\n", object->next->name.c_str());
            computeObjectTriNorm(object->next);
            //smoothObjectTriNorm(object->next );
        }

        numrefs = numrefstotal = 0;
        numvertFound = false;
        numrefsFound = false;
        dataFound = false;
        numvertex = 0;
        numvertice = 0;

    }
    else
        object->next->kids = kids;

    return (0);
}

int doName(char *Line, ob_t *object, std::vector<mat_t> &materials)
{
    char * p;
    char *q;
    char name[256];
    char name2[256];
    p = strstr(Line, "\"");
    if (p == NULL)
    {
        fprintf(stderr, "unknown name format %s\n", Line);
        return (-1);
    }
    else
        p++;
    sprintf(name, "%s", p);
    p = strstr(name, "\n");
    if (p != NULL)
        *p = '\0';

    if (!strcmp("\"n\"", name))
    {
        sprintf(name, "terrain%d", tmpIndice2++);
    }
    if (!strcmp("\"NoName\"", name))
    {
        sprintf(name, "ob%d", tmpIndice3++);
    }
    p = name;
    q = name2;
    while (*p)
    {
        if ((*p <= 'z' && *p >= 'a'))
        {
            *p = (*p - 'a') + 'A';
        }
        if ((*p >= 'A' && *p <= 'Z') || (*p >= '0' && *p <= '9'))
        {
            *q = *p;
            q++;
            *q = '\0';
        }
        p++;
    }
    sprintf(name, "%s", name2);

    /*sprintf(name,"terrain%d",tmpIndice2++);*/
    object->next->name = name;
    sprintf(tmpname, "%s", name);

    fprintf(stderr, "loading  %s object                             \r", name);
    printf("loading  %s object\n", name);
    tmpIndice = 0;
    dataFound = false;
    return (0);
}

int doLoc(char *Line, ob_t *object, std::vector<mat_t> &materials)
{
    char * p = strstr(Line, " ");
    if (p == NULL)
    {
        fprintf(stderr, "unknown Loc format %s\n", Line);
        return (-1);
    }
    if (sscanf(p, "%lf %lf %lf", &(object->next->loc.x), &(object->next->loc.y),
        &(object->next->loc.z)) != 3)
    {
        fprintf(stderr, "invalid Loc format %s\n", p);
        return (-1);
    }

    dataFound = false;
    return (0);
}

int doData(char *Line, ob_t *object, std::vector<mat_t> &materials)
{
    char * p = strstr(Line, " ");
    if (p == NULL)
    {
        fprintf(stderr, "unknown data format %s\n", Line);
        return (-1);
    }
    if (sscanf(p, "%d", &dataSize) != 1)
    {
        fprintf(stderr, "invalid data format %s\n", p);
        return (-1);
    }
    dataFound = true;
    dataSizeRead = 0;
    return (0);
}

int doGetData(char *Line, ob_t *object, std::vector<mat_t> &materials)
{
    int lineSize = (int)strlen(Line);
    // the '\n' of the last line is not included
    if ((dataSizeRead + lineSize) > dataSize && Line[lineSize - 1] == '\n')
    {
        Line[lineSize - 1] = 0;
        lineSize--;
    }
    dataSizeRead += lineSize;
    object->next->data += Line;
    return (0);
}

int doCrease(char *Line, ob_t *object, std::vector<mat_t> &materials)
{
    dataFound = false;
    return (0);
}

int doTexture(char *Line, ob_t *object, std::vector<mat_t> &materials)
{
    char name[256] = { 0 };
    char * p = strstr(Line, " ");
    if (p == NULL)
    {
        fprintf(stderr, "unknown texture format %s\n", Line);
        return (-1);
    }
    if (sscanf(p, "%255s", name) != 1)
    {
        fprintf(stderr, "invalid texture format %s\n", p);
        return (-1);
    }

    p = strstr(name, "\"");
    if (p != NULL)
    {
        p++;
        char *q = strstr(p, "\"");
        if (q != NULL)
            *q = '\0';

        object->next->texture = p;
    }
    else
        object->next->texture = name;
    dataFound = false;
    return (0);
}

int doTexrep(char *Line, ob_t *object, std::vector<mat_t> &materials)
{
    char * p = strstr(Line, " ");
    if (p == NULL)
    {
        fprintf(stderr, "unknown Texrep format %s\n", Line);
        return (-1);
    }
    if (sscanf(p, "%lf %lf", &(object->next->texrep_x), &(object->next->texrep_y)) != 2)
    {
        fprintf(stderr, "invalid Texrep format %s\n", p);
        return (-1);
    }

    dataFound = false;
    return (0);
}

int doNumvert(char *Line, ob_t *object, std::vector<mat_t> &materials)
{
    char * p = strstr(Line, " ");
    if (p == NULL)
    {
        fprintf(stderr, "unknown numvert format %s\n", Line);
        return (-1);
    }
    if (sscanf(p, "%d", &(object->next->numvert)) != 1)
    {
        fprintf(stderr, "invalid numvert format %s\n", p);
        return (-1);
    }
    object->next->vertex.resize(object->next->numvert);
    numvertex = 0;
    numvertFound = true;
    dataFound = false;
    return (0);
}

int doNumsurf(char *Line, ob_t *object, std::vector<mat_t> &materials)
{
    char * p = strstr(Line, " ");
    if (p == NULL)
    {
        fprintf(stderr, "unknown numsurf format %s\n", Line);
        return (-1);
    }
    if (sscanf(p, "%d", &(object->next->numsurf)) != 1)
    {
        fprintf(stderr, "invalid numsurf format %s\n", p);
        return (-1);
    }
    numvertFound = false;
    dataFound = false;
    return (0);
}

int doGetVertex(char *Line, ob_t *object, std::vector<mat_t> &materials)
{
    if (sscanf(Line, "%lf %lf %lf ", &(object->next->vertex[numvertex].x),
        &(object->next->vertex[numvertex].z),
        &(object->next->vertex[numvertex].y)) != 3)
    {
        fprintf(stderr, "invalid vertex format %s\n", Line);
        return (-1);
    }
    object->next->vertex[numvertex].x += object->next->loc.x;
    object->next->vertex[numvertex].y += object->next->loc.z;
    object->next->vertex[numvertex].z += object->next->loc.y;
    object->next->vertex[numvertex].y = -object->next->vertex[numvertex].y;
    /* compute min/max of the vertex for this object */
    if (object->next->vertex[numvertex].x > object->next->x_max)
        object->next->x_max = object->next->vertex[numvertex].x;
    if (object->next->vertex[numvertex].x < object->next->x_min)
        object->next->x_min = object->next->vertex[numvertex].x;

    if (object->next->vertex[numvertex].y > object->next->y_max)
        object->next->y_max = object->next->vertex[numvertex].y;
    if (object->next->vertex[numvertex].y < object->next->y_min)
        object->next->y_min = object->next->vertex[numvertex].y;

    if (object->next->vertex[numvertex].z > object->next->z_max)
        object->next->z_max = object->next->vertex[numvertex].z;
    if (object->next->vertex[numvertex].z < object->next->z_min)
        object->next->z_min = object->next->vertex[numvertex].z;

    numvertex++;
    /*fprintf(stderr,"numvertex = %d\n",numvertex);*/
    return (0);
}

int doGetSurf(char *Line, ob_t *object, std::vector<mat_t> &materials)
{
    /*  double u,v;*/

    if (sscanf(Line, "%d %lf %lf ", &(tmpva[numvertice].indice),
        &(tmpva[numvertice].uv.u), &(tmpva[numvertice].uv.v)) != 3)
    {
        fprintf(stderr, "invalid surf format %s\n", Line);
        return (-1);
    }
    /*fprintf(stderr,"numrefs = %d\n",numrefs);*/
    /*printf("%.2lf %.2lf\n",tmpva[numvertice].u,tmpva[numvertice].v);*/
    tmpva[numvertice].saved = false;
    tmptexa[tmpva[numvertice].indice].u = tmpva[numvertice].uv.u * object->next->texrep_x;
    tmptexa[tmpva[numvertice].indice].v = tmpva[numvertice].uv.v * object->next->texrep_y;
    numvertice++;
    return (0);
}

int doSurf(char *Line, ob_t *object, std::vector<mat_t> &materials)
{
    int surf = 0;
    char * p = strstr(Line, " ");
    if (p == NULL)
    {
        fprintf(stderr, "unknown SURF format %s\n", Line);
        return (-1);
    }
    if (sscanf(p, "%x", &surf) != 1)
    {
        fprintf(stderr, "invalid SURF format %s\n", p);
        return (-1);
    }
    // Check for an object with multiple surfaces with different SURF types.
    // Can't convert multiple triangles with different SURF types into a triangle strip with a single SURF type.
    if ((typeConvertion == _AC3DTOAC3DS || typeConvertion == _AC3DTOAC3DGROUP) && numrefs && surf != attrSurf)
    {
        fprintf(stderr, "multiple SURF in object 0x%x and 0x%x (OBJECT needs splitting by SURF type?)\n", surf, attrSurf);
        return (-1);
    }
    attrSurf = surf;
    numvertFound = false;
    dataFound = false;
    return (0);
}

int doMat(char *Line, ob_t *object, std::vector<mat_t> &materials)
{
    int mat = 0;
    char * p = strstr(Line, " ");
    if (p == NULL)
    {
        fprintf(stderr, "unknown mat format %s\n", Line);
        return (-1);
    }
    if (sscanf(p, "%d", &mat) != 1)
    {
        fprintf(stderr, "invalid mat format %s\n", p);
        return (-1);
    }
    // Check for an object with multiple surfaces with different material types.
    // Can't convert multiple triangles with different material types into a triangle strip with a single material type.
    if ((typeConvertion == _AC3DTOAC3DS || typeConvertion == _AC3DTOAC3DGROUP) && numrefs && mat != attrMat)
    {
        fprintf(stderr, "multiple mat in object %d and %d (OBJECT needs splitting by material type?)\n", mat, attrMat);
        return (-1);
    }
    attrMat = mat;
    numvertFound = false;
    return (0);
}

int doRefs(char *Line, ob_t *object, std::vector<mat_t> &materials)
{
    char * p = strstr(Line, " ");
    if (p == NULL)
    {
        fprintf(stderr, "unknown Refs format %s\n", Line);
        return (-1);
    }
    if (sscanf(p, "%d", &refs) != 1)
    {
        fprintf(stderr, "invalid Refs format %s\n", p);
        return (-1);
    }
    if (refs != 3)
    {
        fprintf(stderr, "invalid number of Refs %d\n", refs);
        return (-1);
    }

    numrefstotal += refs;
    numrefsFound = true;
    tmpsurf[numrefs] = refs;
    numrefs++;
    return (0);
}

/* We need to split an object face in more faces
 * if there are common points with different texture coordinates.
 */
bool isObjectSplit(ob_t* object)
{
    if (notexturesplit)
        return false;

    int numverts = object->numvertice;

    for (int i = 0; i < numverts; i++)
    {
        for (int j = i + 1; j < numverts; j++)
        {
            bool same_pt = (object->vertexarray[i].indice == object->vertexarray[j].indice);
            bool diff_u = (object->vertexarray[i].uv.u != object->vertexarray[j].uv.u);
            bool diff_v = (object->vertexarray[i].uv.v != object->vertexarray[j].uv.v);

            if (same_pt && (diff_u || diff_v))
                return true;
        }
    }

    if (collapseObject)
        return true;

    return false;
}

bool isTerrainSplit(ob_t* object)
{
    /* general showstoppers */
    if (typeConvertion == _AC3DTOAC3DS)
        return false;

    if (distSplit <= 0)
        return false;

    if (object->hasName())
    {
        /* denied prefixes */
        const int num_prefixes = 17;
        const char* denied_prefixes[num_prefixes] =
        { "tkrb", "tkmn", "tkrs", "tklb", "brlt", "brrt", "tkls", "t0RB",
          "t1RB", "t2RB", "tkRS", "t0LB", "t1LB", "t2LB", "tkLS", "BOLt",
          "BORt" };

        for (int i = 0; i < num_prefixes; i++)
        {
            if (object->nameStartsWith(denied_prefixes[i]))
                return false;
        }

        /* name contains terrain or ground */
        if (object->nameHasStr("terrain") || object->nameHasStr("TERRAIN") || 
            object->nameHasStr("GROUND") || object->nameHasStr("ground"))
            return true;
    }

    /* dimension within splitting distance */
    if (((object->x_max - object->x_min) > 1.5 * distSplit) ||
        ((object->y_max - object->y_min) > 1.5 * distSplit))
        return true;

    return false;
}

ob_t * splitObjects(ob_t* object)
{
    if (NULL == object)
        return NULL;

    /* The returned object. Contains all objects from the given
     * object, split and non-split objects.
     */
    ob_t* newob = NULL;

    ob_t* current_ob = object;
    while(current_ob != NULL)
    {
        ob_t* next_ob = current_ob->next; // remember next one, cuz we'll modify current_ob
        ob_t* splitob = NULL;

        if (isObjectSplit(current_ob))
        {
            printf("Found in %s, a duplicate coord with different u,v, split is required\n",
                   current_ob->name.c_str());

            splitob = splitOb(current_ob);
        }
        else if (isTerrainSplit(current_ob))
        {
            printf("Splitting surfaces of %s\n", current_ob->name.c_str());

            splitob = terrainSplitOb(current_ob);
        }

        if (splitob != NULL)
        {
            delete current_ob;
            newob = obAppend(newob, splitob);
        }
        else
        {
            printf("No split required for %s\n", current_ob->name.c_str());

            // Append only the single object, not the whole list.
            // The others will be appended in this function one by one.
            current_ob->next = NULL;
            newob = obAppend(newob, current_ob);
        }

        current_ob = next_ob;
    }

    return newob;
}

int loadAC(const std::string & inputFilename, ob_t** objects, std::vector<mat_t>& materials, const std::string & outputFilename)
{
    /* saveIn : 0= 3ds , 1= obj , 2=ac3d grouped (track) , 3 = ac3d strips (cars) */
    char Line[256];
    int ret = 0;
    int (*doVerb)(char * Line, ob_t *object, std::vector<mat_t> &materials);
    FILE * file;
    ob_t * current_ob;

    if ((file = fopen(inputFilename.c_str(), "r")) == NULL)
    {
        fprintf(stderr, "failed to open %s\n", inputFilename.c_str());
        return (-1);
    }
    if (fgets(Line, 256, file) == NULL)
    {
        fprintf(stderr, "failed to read first line of the file\n");
        fclose(file);
        return (-1);
    }
    if (strnicmp(Line, AC3D, strlen(AC3D)))
    {
        fprintf(stderr, "unknown format %s\n", Line);
        fclose(file);
        return (-1);
    }

    current_ob = new ob_t;
    current_ob->name = "root";
    *objects = current_ob;

    fprintf(stderr, "starting loading ...\n");

    while (fgets(Line, sizeof(Line), file))
    {
        int i = 0;
        /*fprintf(stderr,"parsing line: %s", Line);*/
        doVerb = NULL;
        while (1)
        {
            if (stricmp("END", verbTab[i].verb) == 0)
                break;
            if (strnicmp(Line, verbTab[i].verb, strlen(verbTab[i].verb)) == 0)
            {
                doVerb = verbTab[i].doVerb;
                break;
            }
            i++;
        }
        if (numvertFound && doVerb == NULL)
        {
            ret = doGetVertex(Line, current_ob, materials);
            if(ret != 0)
                break;
        }
        else if (numrefsFound && doVerb == NULL)
        {
            ret = doGetSurf(Line, current_ob, materials);
            if(ret != 0)
                break;
        }
        else if (dataFound && doVerb == NULL)
        {
            ret = doGetData(Line, current_ob, materials);
            if (ret != 0)
                break;
        }
        else
        {
            if (doVerb == NULL)
            {
                fprintf(stderr, " Unknown verb %s\n", Line);
                continue;
            }
            numvertFound = false;
            numrefsFound = false;
            dataFound = false;
            ret = doVerb(Line, current_ob, materials);
            if(ret != 0)
                break;
        }
    }
    fclose(file);
    if(ret != 0)
        return ret;

    if (splitObjectsDuringLoad != 0)
        current_ob = splitObjects(current_ob);

    // --- perform file output ---

    if(outputFilename.empty())
        return 0;

    if (typeConvertion == _AC3DTOOBJ)
    {
        computeSaveOBJ(outputFilename, current_ob, materials);
    }
    else if (typeConvertion == _AC3DTOAC3DM)
    {
        computeSaveAC3DM(outputFilename, current_ob, materials);
    }
    else if (typeConvertion == _AC3DTOAC3DS)
    {
        computeSaveAC3DStrip(outputFilename, current_ob, materials);
    }
    else if (typeConvertion == _AC3DTOAC3D)
    {
        computeSaveAC3D(outputFilename, current_ob, materials);
    }
#ifdef _3DS
    else if (typeConvertion == _AC3DTO3DS)
    {
        saveObin3DS(outputFilename, current_ob, current_material);
    }
#endif

    return 0;
}

#ifdef _3DS
void saveIn3DSsubObject(ob_t * object,database3ds *db)
{

    /*material3ds *matr     = NULL;*/
    mesh3ds *mobj = NULL;
    kfmesh3ds *kobj = NULL;

    if (object->name==NULL && (!stricmp("world",object->type)))
    {
        return;
    }
    else if (!stricmp("world",object->name))
    {
        return;
    }
    else if (!stricmp("root",object->name))
    {
        if (object->next!=NULL)
        {
            saveIn3DSsubObject(object->next,db);
            return;
        }
    }

    if (object->numvert!=0)
    {
        if (object->name==NULL)
        {
            sprintf(tmpname,"TMPNAME%d",tmpIndice);
            tmpIndice=0;
        }
        else
            sprintf(tmpname,"%s",object->name);

        printf("saving %s , numvert=%d , numsurf=%d\n",object->name,object->numvert,object->numsurf);

        InitMeshObj3ds(&mobj, object->numvert, object->numsurf,
                InitNoExtras3ds|InitTextArray3ds|InitVertexArray3ds|InitFaceArray3ds);

        for (int i = 0; i < object->numvert; i++)
        {
            mobj->vertexarray[i].x = object->vertex[i].x;
            mobj->vertexarray[i].y = object->vertex[i].y;
            mobj->vertexarray[i].z = object->vertex[i].z;
        }

        for (int i = 0; i < object->numvert; i++)
        {
            mobj->textarray[i].u = object->textarray[i*2];
            mobj->textarray[i].v = object->textarray[i*2+1];
        }

        for (int j = 0; j < object->numsurf; j++)
        {
            /* GUIONS */
            mobj->facearray[j].v1=object->vertexarray[j*3].indice;
            mobj->facearray[j].flag=FaceABVisable3ds|FaceBCVisable3ds|FaceCAVisable3ds;
            mobj->facearray[j].v2=object->vertexarray[j*3+1].indice;
            mobj->facearray[j].v3=object->vertexarray[j*3+2].indice;
        }

        ON_ERROR_RETURN;
        sprintf(mobj->name, "%s", tmpname);
        printf("generating object %s faces: %d vertex:%d             \r",
                mobj->name,object->numsurf,object->numvert);
        mobj->ntextverts =object->numvert;
        mobj->usemapinfo = True3ds;

        if (object->texture!=NULL)
        {
            mobj->nmats = 1;
            InitMeshObjField3ds (mobj, InitMatArray3ds);
            ON_ERROR_RETURN;

            InitMatArrayIndex3ds (mobj, 0, mobj->nfaces);
            ON_ERROR_RETURN;

            for (int i=0; i<texnum; i++)
            {
                if (tex[i]!=NULL)
                if (object->hasTexture())
                if (!strncmp(object->texture.c_str(),tex[i],13))
                sprintf(mobj->matarray[0].name,"texture%d",i);
            }

            for (int i=0; i<mobj->nfaces; i++)
                mobj->matarray[0].faceindex[i] = (ushort3ds)i;
            mobj->matarray[0].nfaces = mobj->nfaces;
        }

        FillMatrix3ds(mobj);
        PutMesh3ds(db, mobj);
        ON_ERROR_RETURN;
        InitObjectMotion3ds(&kobj, 1, 1, 1, 0, 0);
        ON_ERROR_RETURN;
        sprintf(kobj->name, "%s",tmpname);
        /* Set the pivot point to the mesh's center */
        SetPoint(kobj->pivot, 0.0F, 0.0F, 0.0F);
        SetBoundBox3ds (mobj, kobj);
        ON_ERROR_RETURN;
        kobj->pkeys[0].time = 0;
        kobj->pos[0].x = (kobj->boundmax.x - kobj->boundmin.x) / 2.0F + kobj->boundmin.x;
        kobj->pos[0].y = (kobj->boundmax.y - kobj->boundmin.y) / 2.0F + kobj->boundmin.y;
        kobj->pos[0].z = (kobj->boundmax.z - kobj->boundmin.z) / 2.0F + kobj->boundmin.z;
        kobj->rkeys[0].time = 0;
        kobj->rot[0].angle = 0.0F;
        kobj->rot[0].x = 0.0F;
        kobj->rot[0].y = 0.0F;
        kobj->rot[0].z = -1.0F;
        PutObjectMotion3ds(db, kobj);
        ON_ERROR_RETURN;
        ReleaseObjectMotion3ds (&kobj);
        ON_ERROR_RETURN;
        RelMeshObj3ds(&mobj);
    }

    if (object->next)
    saveIn3DSsubObject(object->next,db);

}

void saveObin3DS( const std::string & OutputFilename, ob_t * object, mat_t * material)
{
    database3ds *db = NULL;
    file3ds *file;
    material3ds *matr = NULL;

    meshset3ds *mesh = NULL;
    background3ds *bgnd = NULL;
    atmosphere3ds *atmo = NULL;
    char name2[256];
    char *p, *q;
    viewport3ds *view = NULL;
    ob_t * tmpob =NULL;

    /* Clear error list, not necessary but safe */
    ClearErrList3ds();
    file = OpenFile3ds(OutputFilename.c_str(), "w");
    PRINT_ERRORS_EXIT(stderr);
    InitDatabase3ds(&db);
    PRINT_ERRORS_EXIT(stderr);
    CreateNewDatabase3ds(db, MeshFile);
    PRINT_ERRORS_EXIT(stderr);
    /*--- MESH SETTINGS */
    InitMeshSet3ds(&mesh);
    ON_ERROR_RETURN;
    mesh->ambientlight.r = mesh->ambientlight.g = mesh->ambientlight.b = 0.3F;
    PutMeshSet3ds(db, mesh);
    ON_ERROR_RETURN;
    ReleaseMeshSet3ds(&mesh);
    ON_ERROR_RETURN;
    InitBackground3ds(&bgnd);
    ON_ERROR_RETURN;
    PutBackground3ds(db, bgnd);
    ON_ERROR_RETURN;
    ReleaseBackground3ds(&bgnd);
    ON_ERROR_RETURN;
    /*--- ATMOSPHERE */
    InitAtmosphere3ds(&atmo);
    ON_ERROR_RETURN;
    PutAtmosphere3ds(db, atmo);
    ON_ERROR_RETURN;
    ReleaseAtmosphere3ds(&atmo);
    ON_ERROR_RETURN;
    /*--- MATERIAL */
    InitMaterial3ds(&matr);
    ON_ERROR_RETURN;
    strcpy(matr->name, "RedSides");
    matr->ambient.r = 0.0F;
    matr->ambient.g = 0.0F;
    matr->ambient.b = 0.0F;
    matr->diffuse.r = 1.0F;
    matr->diffuse.g = 0.0F;
    matr->diffuse.b = 0.0F;
    matr->specular.r = 1.0F;
    matr->specular.g = 0.7F;
    matr->specular.b = 0.65F;
    matr->shininess = 0.0F;
    matr->shinstrength = 0.0F;
    matr->blur = 0.2F;
    matr->transparency = 0.0F;
    matr->transfalloff = 0.0F;
    matr->selfillumpct = 0.0F;
    matr->wiresize = 1.0F;
    matr->shading = Phong;
    matr->useblur = True3ds;
    PutMaterial3ds(db, matr);
    ON_ERROR_RETURN;
    ReleaseMaterial3ds(&matr);
    ON_ERROR_RETURN;

    texnum=0;
    tmpob=object;
    while (tmpob!=NULL)
    {
        int texnofound=0;
        if (tmpob->canSkip())
        {
            tmpob=tmpob->next;
            continue;
        }
        texnofound=1;
        for (int i=0; i<texnum; i++)
        {
            if (tmpob->texture==NULL)
            {
                texnofound=0;
                break;

            }
            if (!strncmp(tex[i],tmpob->texture.c_str(),13))
            {
                texnofound=0;
                break;

            }
            else
            {
                texnofound=1;
            }
        }
        if (texnofound==1)
        {
            if (tmpob->texture!=NULL)
            {
                strcpy(tex[texnum],tmpob->texture);
                tex[texnum][13]='\0';
            }
            texnum ++;
        }
        tmpob=tmpob->next;
    }

    for (int i=0; i<texnum; i++)
    {

        InitMaterial3ds(&matr);
        ON_ERROR_RETURN;
        sprintf(matr->name,"texture%d",i);
        /*sprintf(matr->texture.map.name,"texture%d",i);*/
        printf("analysing  %s\n",tex[i]);
        p=tex[i];
        q=name2;
        while (*p)
        {
            if ( (*p<='Z'&&*p>='A'))
            {
                *p=(*p-'A')+'a';
            }

            if ( (*p>='a' && *p<='z') || (*p>='0' && *p<='9') || (*p=='.'))
            {
                *q=*p; q++; *q='\0';
            }
            p++;
        }
        int j=0;
        while (name2[j]!='\0')
        {
            if (name2[j]=='.')
            {
                name2[j]='\0';
                break;
            }
            j++;
            if (j==8)
            {
                name2[j]='\0';
                break;
            }
        }

        printf("texture file %s will be stored as %s.png\n",tex[i],name2);
        sprintf(matr->texture.map.name,"%s.png",name2);

        if (material != NULL)
        {
            matr->ambient.r = material->amb.r;
            matr->ambient.g = material->amb.g;
            matr->ambient.b = material->amb.b;
            matr->specular.r = material->spec.r;
            matr->specular.g = material->spec.g;
            matr->specular.b = material->spec.b;
            matr->shininess = material->shi;
            matr->transparency = material->trans;
            matr->diffuse.r = material->rgb.r;
            matr->diffuse.g = material->rgb.g;
            matr->diffuse.b = material->rgb.b;
        }
        else
        {
            matr->ambient.r = 1.0F;
            matr->ambient.g = 1.0F;
            matr->ambient.b = 1.0F;
            matr->specular.r =1.0F;
            matr->specular.g =1.0F;
            matr->specular.b =1.0F;
            matr->shininess = 0.0F;
            matr->transparency = 0.0F;
            matr->diffuse.r = 1.0F;
            matr->diffuse.g = 1.0F;
            matr->diffuse.b = 1.0F;
        }

        matr->shinstrength = 0.0F;
        matr->blur = 0.2F;

        matr->transfalloff = 0.0F;
        matr->selfillumpct = 0.0F;
        matr->wiresize = 1.0F;
        matr->shading = Phong;
        matr->useblur = True3ds;
        matr->texture.map.percent = 1.0F;
        matr->texture.map.ignorealpha = False3ds;
        matr->texture.map.filter = (filtertype3ds)False3ds;
        matr->texture.map.mirror = False3ds;
        matr->texture.map.negative =False3ds;
        matr->texture.map.source = RGB;
        matr->texture.map.blur = 0.07F;
        PutMaterial3ds(db, matr);
        ON_ERROR_RETURN;
        ReleaseMaterial3ds(&matr);
        ON_ERROR_RETURN;
    }

    tmpIndice=0;
    /* do the job */
    saveIn3DSsubObject(object,db);
    printf("\nend\n");
    /* Always remember to release the memory used by a mesh3ds pointer prior
     to using the pointer again so that the values of the last mesh will
     not be carried over. */
    InitViewport3ds(&view);
    ON_ERROR_RETURN;
    PutViewport3ds(db, view);
    ON_ERROR_RETURN;
    ReleaseViewport3ds(&view);
    ON_ERROR_RETURN;
    WriteDatabase3ds(file, db);
    PRINT_ERRORS_EXIT(stderr);
    CloseAllFiles3ds();
    PRINT_ERRORS_EXIT(stderr);
    ReleaseDatabase3ds(&db);
    PRINT_ERRORS_EXIT(stderr);

}
#endif

int printOb(FILE *ofile, ob_t * object)
{
    int multitex = 0;

    if (object->numsurf == 0)
        return 0;
    if (!extendedStrips && !normalMapping)
        if (!(isobjectacar && collapseObject))
            stripifyOb(ofile, object, 0);
    object->saved = true;
    fprintf(ofile, "OBJECT poly\n");
    fprintf(ofile, "name \"%s\"\n", object->name.c_str());
    if (object->hasMultiTexture())
    {
        multitex = 1;
        fprintf(ofile, "texture \"%s\" base\n", object->texture.c_str());
        if (object->hasTexture1())
            fprintf(ofile, "texture \"%s\" tiled\n", object->texture1.c_str());
        else
            fprintf(ofile, "texture empty_texture_no_mapping tiled\n");
        if (object->hasTexture2())
            fprintf(ofile, "texture \"%s\" skids\n", object->texture2.c_str());
        else
            fprintf(ofile, "texture empty_texture_no_mapping skids\n");
        if (object->hasTexture3())
            fprintf(ofile, "texture \"%s\" shad\n", object->texture3.c_str());
        else
            fprintf(ofile, "texture empty_texture_no_mapping shad\n");
    }
    else
    {
        fprintf(ofile, "texture \"%s\"\n", object->texture.c_str());
    }
    fprintf(ofile, "numvert %d\n", object->numvert);
    for (int i = 0; i < object->numvert; i++)
    {
        if ((typeConvertion == _AC3DTOAC3DS
                && (extendedStrips || extendedTriangles))
                || typeConvertion == _AC3DTOAC3DGROUP
                || (typeConvertion == _AC3DTOAC3D && extendedTriangles))
        {
            fprintf(ofile, "%lf %lf %lf %lf %lf %lf\n", object->vertex[i].x,
                object->vertex[i].z, -object->vertex[i].y, object->snorm[i].x,
                object->snorm[i].z, -object->snorm[i].y);
        }
        else
        {
            fprintf(ofile, "%lf %lf %lf\n", object->vertex[i].x, object->vertex[i].z,
                    -object->vertex[i].y);
        }
    }
    if (!extendedStrips)
    {
        fprintf(ofile, "numsurf %d\n", object->numsurf);
        for (int i = 0; i < object->numsurf; i++)
        {
            if (object->attrSurf != 0)
                fprintf(ofile, "SURF 0x%2x\n", object->attrSurf);
            else
                fprintf(ofile, "SURF 0x20\n");
            fprintf(ofile, "mat %d\n", object->attrMat);
            fprintf(ofile, "refs 3\n");
            /* GUIONS */
            if (multitex == 0)
            {
                fprintf(ofile, "%d %.5f %.5f\n", object->vertexarray[i * 3].indice,
                        object->textarray[object->vertexarray[i * 3].indice].u,
                        object->textarray[object->vertexarray[i * 3].indice].v);
                fprintf(ofile, "%d %.5f %.5f\n",
                        object->vertexarray[i * 3 + 1].indice,
                        object->textarray[object->vertexarray[i * 3 + 1].indice].u,
                        object->textarray[object->vertexarray[i * 3 + 1].indice].v);
                fprintf(ofile, "%d %.5f %.5f\n",
                        object->vertexarray[i * 3 + 2].indice,
                        object->textarray[object->vertexarray[i * 3 + 2].indice].u,
                        object->textarray[object->vertexarray[i * 3 + 2].indice].v);
            }
            else
            {
                fprintf(ofile, "%d %.5f %.5f", object->vertexarray[i * 3].indice,
                        object->textarray[object->vertexarray[i * 3].indice].u,
                        object->textarray[object->vertexarray[i * 3].indice].v);

                if (object->hasTexture1())
                    fprintf(ofile, " %.5f %.5f",
                            object->textarray1[object->vertexarray[i * 3].indice].u,
                            object->textarray1[object->vertexarray[i * 3].indice].v);

                if (object->hasTexture2())
                    fprintf(ofile, " %.5f %.5f",
                            object->textarray2[object->vertexarray[i * 3].indice].u,
                            object->textarray2[object->vertexarray[i * 3].indice].v);
                if (object->hasTexture3())
                    fprintf(ofile, " %.5f %.5f",
                            object->textarray3[object->vertexarray[i * 3].indice].u,
                            object->textarray3[object->vertexarray[i * 3].indice].v);
                fprintf(ofile, "\n");

                fprintf(ofile, "%d %.5f %.5f",
                        object->vertexarray[i * 3 + 1].indice,
                        object->textarray[object->vertexarray[i * 3 + 1].indice].u,
                        object->textarray[object->vertexarray[i * 3 + 1].indice].v);
                if (object->hasTexture1())
                    fprintf(ofile, " %.5f %.5f",
                            object->textarray1[object->vertexarray[i * 3 + 1].indice].u,
                            object->textarray1[object->vertexarray[i * 3 + 1].indice].v);

                if (object->hasTexture2())
                    fprintf(ofile, " %.5f %.5f",
                            object->textarray2[object->vertexarray[i * 3 + 1].indice].u,
                            object->textarray2[object->vertexarray[i * 3 + 1].indice].v);
                if (object->hasTexture3())
                    fprintf(ofile, " %.5f %.5f",
                            object->textarray3[object->vertexarray[i * 3 + 1].indice].u,
                            object->textarray3[object->vertexarray[i * 3 + 1].indice].v);
                fprintf(ofile, "\n");

                fprintf(ofile, "%d %.5f %.5f",
                        object->vertexarray[i * 3 + 2].indice,
                        object->textarray[object->vertexarray[i * 3 + 2].indice].u,
                        object->textarray[object->vertexarray[i * 3 + 2].indice].v);
                if (object->hasTexture1())
                    fprintf(ofile, " %.5f %.5f",
                            object->textarray1[object->vertexarray[i * 3 + 2].indice].u,
                            object->textarray1[object->vertexarray[i * 3 + 2].indice].v);

                if (object->hasTexture2())
                    fprintf(ofile, " %.5f %.5f",
                            object->textarray2[object->vertexarray[i * 3 + 2].indice].u,
                            object->textarray2[object->vertexarray[i * 3 + 2].indice].v);
                if (object->hasTexture3())
                {
                    fprintf(ofile, " %.5f %.5f",
                            object->textarray3[object->vertexarray[i * 3 + 2].indice].u,
                            object->textarray3[object->vertexarray[i * 3 + 2].indice].v);
                    if (object->textarray3[object->vertexarray[i * 3 + 2].indice].u
                            != object->textarray1[object->vertexarray[i * 3 + 2].indice].u)
                    {
                        printf("error in text\n");
                    }
                }
                fprintf(ofile, "\n");
            }
        }
    }
    else
    {
        stripifyOb(ofile, object, 1);
    }
    fprintf(ofile, "kids 0\n");
    return 0;
}

int foundNear(FILE * ofile, ob_t * object, ob_t *allobjects, double dist, bool print)
{
    ob_t * tmpob;
    double x;
    double y;
    int numfound = 0;
    tmpob = allobjects;

    x = (object->x_max - object->x_min) / 2 + object->x_min;
    y = (object->y_max - object->y_min) / 2 + object->y_min;

    while (tmpob != NULL)
    {
        if (tmpob->canSkip())
        {
            tmpob = tmpob->next;
            continue;
        }
        if (tmpob->nameStartsWith("tkmn"))
        {
            tmpob = tmpob->next;
            continue;
        }

        if (tmpob->inkids_o)
        {
            tmpob = tmpob->next;
            continue;
        }
        if (tmpob->numsurf == 0)
        {
            tmpob = tmpob->next;
            continue;
        }
        if ((tmpob->x_min - x) * (tmpob->x_min - x)
                + (tmpob->y_min - y) * (tmpob->y_min - y) < dist * dist)
        {
            /*printf("object %s near object %s (dist=%d)\n", tmpob->name , object->name, dist);*/
            numfound++;
            tmpob->inkids_o = true;
            if (print)
            {
                printOb(ofile, tmpob);
            }
            tmpob = tmpob->next;
            continue;
        }
        if ((tmpob->x_max - x) * (tmpob->x_max - x)
                + (tmpob->y_max - y) * (tmpob->y_max - y) < dist * dist)
        {
            /*printf("object %s near object %s (dist=%d)\n", tmpob->name , object->name, dist);*/
            numfound++;
            tmpob->inkids_o = true;
            if (print)
            {
                printOb(ofile, tmpob);
            }
            tmpob = tmpob->next;
            continue;
        }
        if ((tmpob->x_min - x) * (tmpob->x_min - x)
                + (tmpob->y_max - y) * (tmpob->y_max - y) < dist * dist)
        {
            /*printf("object %s near object %s (dist=%d)\n", tmpob->name , object->name, dist);*/
            numfound++;
            tmpob->inkids_o = true;
            if (print)
            {
                printOb(ofile, tmpob);
            }
            tmpob = tmpob->next;
            continue;
        }
        if ((tmpob->x_max - x) * (tmpob->x_max - x)
                + (tmpob->y_min - y) * (tmpob->y_min - y) < dist * dist)
        {
            /*printf("object %s near object %s (dist=%d)\n", tmpob->name , object->name, dist);*/
            numfound++;
            tmpob->inkids_o = true;
            if (print)
            {
                printOb(ofile, tmpob);
            }
            tmpob = tmpob->next;
            continue;
        }
        tmpob = tmpob->next;
    }

    object->kids_o = numfound++;

    /*printf(" object %s (dist=%d) found %d objects\n", object->name, dist, numfound);*/
    return (0);

}

void normalize(point_t *t)
{
    double dd;
    dd = sqrt(t->x * t->x + t->y * t->y + t->z * t->z);
    if (dd != 0.0)
        *t /= dd;
    else
        t->set(0.0, 1.0, 0.0);
}

void computeTriNorm(ob_t * object)
{
    point_t norm;
    ob_t *tmpob = object;

    while (tmpob != NULL)
    {
        if (tmpob->canSkip())
        {
            tmpob = tmpob->next;
            continue;
        }
        for (int i = 0; i < tmpob->numsurf; i++)
        {
            /* compute the same normal for each points in the surface */
            computeNorm(&tmpob->vertex[tmpob->vertexarray[i * 3].indice],
                        &tmpob->vertex[tmpob->vertexarray[i * 3 + 1].indice],
                        &tmpob->vertex[tmpob->vertexarray[i * 3 + 2].indice],
                        &norm);
            tmpob->norm[tmpob->vertexarray[i * 3].indice] += norm;
            tmpob->norm[tmpob->vertexarray[i * 3 + 1].indice] += norm;
            tmpob->norm[tmpob->vertexarray[i * 3 + 2].indice] += norm;
        }
        for (int i = 0; i < tmpob->numsurf; i++)
        {
            normalize(&tmpob->norm[tmpob->vertexarray[i * 3].indice]);
            normalize(&tmpob->norm[tmpob->vertexarray[i * 3 + 1].indice]);
            normalize(&tmpob->norm[tmpob->vertexarray[i * 3 + 2].indice]);
        }

        tmpob = tmpob->next;
    }
    return;
}

void computeObjectTriNorm(ob_t * object)
{
    point_t norm;
    ob_t *tmpob = object;

    if (tmpob->canSkip())
        return;
    for (int i = 0; i < tmpob->numsurf; i++)
    {
        /* compute the same normal for each points in the surface */
        computeNorm(&tmpob->vertex[tmpob->vertexarray[i * 3].indice],
                    &tmpob->vertex[tmpob->vertexarray[i * 3 + 1].indice],
                    &tmpob->vertex[tmpob->vertexarray[i * 3 + 2].indice], &norm);

        tmpob->norm[tmpob->vertexarray[i * 3].indice] += norm;
        tmpob->norm[tmpob->vertexarray[i * 3 + 1].indice] += norm;
        tmpob->norm[tmpob->vertexarray[i * 3 + 2].indice] += norm;
    }
    for (int i = 0; i < tmpob->numsurf; i++)
    {
        normalize(&tmpob->norm[tmpob->vertexarray[i * 3].indice]);
        normalize(&tmpob->norm[tmpob->vertexarray[i * 3 + 1].indice]);
        normalize(&tmpob->norm[tmpob->vertexarray[i * 3 + 2].indice]);
    }

    return;
}

bool checkMustSmoothVector(point_t *n1, point_t *n2, point_t *t1, point_t *t2)
{
    return false;
#if 0
    double dot, cos_angle;
    cos_angle = cos(smooth_angle * M_PI / 180.0);
    if (fabs(t1->x - t2->x) <= 0.05 && fabs(t1->y - t2->y) <= 0.05
            && fabs(t1->z - t2->z) <= 0.05)
    {
        // GUIONS
        dot = n1->x * n2->x + n1->y * n2->y + n1->z * n2->z;
        if (dot > cos_angle)
        {
            return true;
        }

    }
    return false;
#endif
}

void smoothTriNorm(ob_t * object)
{
    ob_t * tmpob = NULL;
    ob_t * tmpob1 = NULL;
    double dd;
    double nx, ny, nz;

    printf("Smooth called on %s\n", object->name.c_str());
    tmpob = object;
    while (tmpob != NULL)
    {
        if (tmpob->canSkip())
        {
            tmpob = tmpob->next;
            continue;
        }
        for (int i = 0; i < tmpob->numvert; i++)
        {
            /* compute the same normal for each points in the surface */
            tmpob->snorm[i] = tmpob->norm[i];
        }
        tmpob = tmpob->next;
    }

    tmpob = object;
    tmpob1 = object;

    while (tmpob != NULL)
    {
        if (tmpob->canSkip() || tmpob->hasNoSmooth())
        {
            tmpob = tmpob->next;
            continue;
        }
        tmpob1 = object;
        while (tmpob1 != NULL)
        {
            if (tmpob1->canSkip() || tmpob1->hasNoSmooth())
            {
                tmpob1 = tmpob1->next;
                continue;
            }
            for (int i = 0; i < tmpob->numvert; i++)
            {
                for (int j = 0; j < tmpob1->numvert; j++)
                {
                    if (checkMustSmoothVector(&tmpob->norm[i], &tmpob1->norm[j],
                            &tmpob->vertex[i], &tmpob1->vertex[j]))
                    {
                        point_t p = tmpob1->norm[j] + tmpob->norm[i];
                        normalize(&p);

                        tmpob->snorm[i] = p;
                        tmpob1->snorm[j] = p;

                        /* tmpob->snorm[i] += tmpob1->norm[j]; */
                        /* tmpob1->snorm[j] += tmpob->norm[i]; */
                    }
                }
            }

            tmpob1 = tmpob1->next;
        }
        tmpob = tmpob->next;
    }

    tmpob = object;
    while (tmpob != NULL)
    {
        if (tmpob->canSkip())
        {
            tmpob = tmpob->next;
            continue;
        }
        for (int i = 0; i < tmpob->numvert; i++)
        {
            /* compute the same normal for each points in the surface */
            nx = tmpob->snorm[i].x;
            ny = tmpob->snorm[i].y;
            nz = tmpob->snorm[i].z;
            dd = sqrt(nx * nx + ny * ny + nz * nz);
            if (dd != 0.0)
            {
                tmpob->snorm[i].x = nx / dd;
                tmpob->snorm[i].y = ny / dd;
                tmpob->snorm[i].z = nz / dd;
            }
            else
                tmpob->snorm[i].set(0, 0, 1);
        }
        tmpob = tmpob->next;
    }

    tmpob = object;
    while (tmpob != NULL)
    {
        if (tmpob->canSkip() || tmpob->hasNoSmooth())
        {
            tmpob = tmpob->next;
            continue;
        }
        tmpob1 = object;
        while (tmpob1 != NULL)
        {
            if (tmpob1->canSkip() || tmpob1->hasNoSmooth())
            {
                tmpob1 = tmpob1->next;
                continue;
            }
            tmpob1 = tmpob1->next;
        }
        tmpob = tmpob->next;
    }

    return;
}

void smoothFaceTriNorm(ob_t * object)
{
    ob_t * tmpob = object;

    if (tmpob->canSkip())
        return;

    for (int i = 0; i < tmpob->numvert; i++)
    {
        /* compute the same normal for each points in the surface */
        for (int j = 0; j < tmpob->numvert; j++)
        {
            if ((tmpob->vertex[i].x - tmpob->vertex[j].x) <= 0.01
                    && (tmpob->vertex[i].y - tmpob->vertex[j].y) <= 0.01
                    && (tmpob->vertex[i].z - tmpob->vertex[j].z) <= 0.1)
            {
                /*same point */
                tmpob->snorm[i] += tmpob->norm[j];
                tmpob->snorm[j] = tmpob->snorm[i];
            }
        }
    }

    for (int i = 0; i < tmpob->numvert; i++)
    {
        normalize(&tmpob->snorm[i]);
    }
    return;
}

void smoothObjectTriNorm(ob_t * object)
{
    ob_t * tmpob = object;

    for (int i = 0; i < tmpob->numvert; i++)
    {
        /* compute the same normal for each points in the surface */
        for (int j = 0; j < tmpob->numvert; j++)
        {
            if ((tmpob->vertex[i].x - tmpob->vertex[j].x) <= 0.001
                    && (tmpob->vertex[i].y - tmpob->vertex[j].y) <= 0.001
                    && (tmpob->vertex[i].z - tmpob->vertex[j].z) <= 0.001)
            {
                /*same point */
                tmpob->snorm[i] += tmpob->norm[j];
                tmpob->snorm[j] = tmpob->snorm[i];
            }
        }
    }
    for (int i = 0; i < tmpob->numvert; i++)
    {
        normalize(&tmpob->snorm[i]);
    }
    return;
}

void computeSaveAC3D(const std::string & OutputFilename, ob_t * object, const std::vector<mat_t> &materials)
{
    char name2[256];
    char *p, *q;
    ob_t * tmpob = NULL;
    int numg = 0;
    bool lastpass = false;
    int nborder = 0;
    bool ordering = false;
    FILE * ofile = NULL;

    if (normalMapping)
        normalMap(object);

    if ((ofile = fopen(OutputFilename.c_str(), "w")) == NULL)
    {
        fprintf(stderr, "failed to open %s\n", OutputFilename.c_str());
        return;
    }
    if (extendedTriangles)
    {
        smoothTriNorm(object);
        if (isobjectacar)
        {
            mapNormalToSphere2(object);
            if (extendedEnvCoord)
                mapTextureEnv(object);
        }
        if (collapseObject)
            mergeSplitted(&object);
    }

    fprintf(ofile, "AC3Db\n");
    printMaterials(ofile, materials);
    fprintf(ofile, "OBJECT world\n");

    if (OrderString && isobjectacar)
    {
        fprintf(stderr, "ordering objects according to  %s\n", OrderString);
        p = OrderString;
        ordering = true;
        nborder = 1;
        while (true)
        {
            q = strstr(p, ";");
            if (q != NULL)
                nborder++;
            else
                break;
            p = q + 1;
            if (*p == '\0')
            {
                nborder--;
                break;
            }
        }
    }
    else
    {
        ordering = false;
        nborder = 0;
    }

    tmpob = object;
    while (tmpob != NULL)
    {
        if (!tmpob->hasName())
        {
            tmpob = tmpob->next;
            continue;
        }
        if (!isobjectacar)
        {
            if (tmpob->nameStartsWith("tkmn"))
            {
                tmpob = tmpob->next;
                numg++;
                continue;
            }
        }
        else
        {
            if (tmpob->type == "group" || tmpob->name == "root")
            {
                tmpob = tmpob->next;
                continue;
            }
            numg++;
        }
        tmpob = tmpob->next;
    }

    fprintf(ofile, "kids %d\n", numg);

    texnum = 0;
    tmpob = object;
    while (tmpob != NULL)
    {
        int texnofound = 0;
        if (tmpob->canSkip())
        {
            tmpob = tmpob->next;
            continue;
        }
        texnofound = 1;
        for (int i = 0; i < texnum; i++)
        {
            if (!tmpob->hasTexture())
            {
                texnofound = 0;
                break;
            }
            if (!strncmp(tex[i], tmpob->texture.c_str(), 13))
            {
                texnofound = 0;
                break;
            }
            else
                texnofound = 1;
        }
        if (texnofound == 1)
        {
            if (tmpob->hasTexture())
            {
                strcpy(tex[texnum], tmpob->texture.c_str());
                tex[texnum][13] = '\0';
                /*sprintf(tex[texnum],"%s",tmpob->texture);*/
            }
            texnum++;
        }
        tmpob->saved = false;
        printf("name=%s x_min=%.1f y_min=%.1f x_max=%.1f y_max=%.1f\n",
                tmpob->name.c_str(), tmpob->x_min, tmpob->y_min, tmpob->x_max,
                tmpob->y_max);

        tmpob = tmpob->next;
    }

    tmpob = object;
    tmpob->kids_o = 0;
    while (tmpob != NULL)
    {
        tmpob->kids_o = 0;
        if (tmpob->canSkip())
        {
            tmpob = tmpob->next;
            continue;
        }
        if (tmpob->nameStartsWith("tkmn"))
        {
            foundNear(ofile, tmpob, object, far_dist, false);
            printf("object =%s num kids_o=%d\n", tmpob->name.c_str(), tmpob->kids_o);
        }

        tmpob = tmpob->next;
    }

    tmpob = object;
    while (tmpob != NULL)
    {
        tmpob->inkids_o = false;
        tmpob = tmpob->next;
    }

    p = OrderString;
    q = OrderString;
    nborder++;
    for (int ik = 0; ik < nborder; ik++)
    {
        if (ordering)
        {
            /* look to the current object name to save */
            if (p == NULL)
                lastpass = true;
            else
            {
                q = p;
                p = strstr(p, ";");
                if (p != NULL)
                {
                    *p = '\0';
                    p++;
                }
            }
        }
        tmpob = object;
        while (tmpob != NULL)
        {
            if (tmpob->canSkip())
            {
                tmpob = tmpob->next;
                continue;
            }
            if (!isobjectacar)
            {
                if (tmpob->nameStartsWith("tkmn"))
                {
                    fprintf(ofile, "OBJECT group\n");
                    fprintf(ofile, "name \"%s_g\"\n", tmpob->name.c_str());
                    fprintf(ofile, "kids %d\n", tmpob->kids_o + 1);
                    printOb(ofile, tmpob);
                    foundNear(ofile, tmpob, object, far_dist, true);
                    printf("object =%s num kids_o=%d\n", tmpob->name.c_str(), tmpob->kids_o);
                }
            }
            else
            {
                if (!tmpob->saved)
                {
                    if (ordering && !lastpass)
                    {
                        if (tmpob->name == q)
                        {
                            printOb(ofile, tmpob);
                            printf("object =%s num kids_o=%d test with %s\n", tmpob->name.c_str(), tmpob->kids_o, q);
                        }
                        else
                        {
                            char nameBuf[1024];
                            sprintf(nameBuf, "%ss", q);
                            if (tmpob->name == nameBuf)
                            {
                                printOb(ofile, tmpob);
                                printf("object =%s num kids_o=%d\n", tmpob->name.c_str(), tmpob->kids_o);
                            }

                        }
                    }
                    else
                    {
                        printOb(ofile, tmpob);
                        printf("object =%s num kids_o=%d\n", tmpob->name.c_str(), tmpob->kids_o);
                    }
                }
            }

            tmpob = tmpob->next;
        }
    }

    for (int i = 0; i < texnum; i++)
    {
        printf("analysing  %s\n", tex[i]);
        p = tex[i];
        q = name2;
        while (*p)
        {
            if ((*p <= 'Z' && *p >= 'A'))
            {
                *p = (*p - 'A') + 'a';
            }

            if ((*p >= 'a' && *p <= 'z') || (*p >= '0' && *p <= '9')
                    || (*p == '.'))
            {
                *q = *p;
                q++;
                *q = '\0';
            }
            p++;
        }
        int j = 0;
        while (name2[j] != '\0')
        {
            if (name2[j] == '.')
            {
                name2[j] = '\0';
                break;
            }
            j++;
            if (j == 8)
            {
                name2[j] = '\0';
                break;
            }
        }

        printf("texture file %s will be stored as %s.png\n", tex[i], name2);
    }

    tmpIndice = 0;
    /* do the job */
    printf("\nend\n");

    fclose(ofile);
}

void computeSaveOBJ(const std::string & OutputFilename, ob_t * object, const std::vector<mat_t> &materials)
{
    char name2[256];
    ob_t * tmpob = NULL;
    int deltav = 1;
    int ind = 0;
    char tname[256];
    FILE * ofile;
    FILE * tfile;

    if ((ofile = fopen(OutputFilename.c_str(), "w")) == NULL)
    {
        fprintf(stderr, "failed to open %s\n", OutputFilename.c_str());
        return;
    }

    fprintf(ofile, "mtllib ./%s.mtl\n", OutputFilename.c_str());
    sprintf(tname, "%s.mtl", OutputFilename.c_str());

    if ((tfile = fopen(tname, "w")) == NULL)
    {
        fprintf(stderr, "failed to open %s\n", tname);
        fclose(ofile);
        return;
    }

    for (size_t i = 0, end = materials.size(); i < end; ++i)
    {
        tmpob = object;
        while (tmpob != NULL)
        {
            if (tmpob->canSkip())
            {
                tmpob = tmpob->next;
                continue;
            }

            if (tmpob->hasTexture())
            {
                fprintf(tfile, "newmtl default\n");
                fprintf(tfile, "Ka %lf %lf %lf\n", materials[i].amb.r,
                        materials[i].amb.g, materials[i].amb.b);
                fprintf(tfile, "Kd %lf %lf %lf\n", materials[i].emis.r,
                        materials[i].emis.g, materials[i].emis.b);
                fprintf(tfile, "Ks %lf %lf %lf\n", materials[i].spec.r,
                        materials[i].spec.g, materials[i].spec.b);
                fprintf(tfile, "Ns %d\n", (int)materials[i].shi);
                fprintf(tfile, "map_kd %s\n", tmpob->texture.c_str());
                break;
            }

            tmpob = tmpob->next;
        }
    }

    texnum = 0;
    tmpob = object;
    while (tmpob != NULL)
    {
        int texnofound = 0;
        if (tmpob->canSkip())
        {
            tmpob = tmpob->next;
            continue;
        }
        texnofound = 1;
        for (int i = 0; i < texnum; i++)
        {
            if (!tmpob->hasTexture())
            {
                texnofound = 0;
                break;
            }
            if (!strncmp(tex[i], tmpob->texture.c_str(), 13))
            {
                texnofound = 0;
                break;
            }
            else
                texnofound = 1;
        }
        if (texnofound == 1)
        {
            if (tmpob->hasTexture())
            {
                strcpy(tex[texnum], tmpob->texture.c_str());
                tex[texnum][13] = '\0';
            }
            texnum++;
        }
        printf("name=%s x_min=%.1f y_min=%.1f x_max=%.1f y_max=%.1f\n",
                tmpob->name.c_str(), tmpob->x_min, tmpob->y_min, tmpob->x_max,
                tmpob->y_max);

        tmpob = tmpob->next;
    }

    for (int i = 0; i < texnum; i++)
    {
        printf("analysing  %s\n", tex[i]);
        char *p = tex[i];
        char *q = name2;
        while (*p)
        {
            if ((*p <= 'Z' && *p >= 'A'))
            {
                *p = (*p - 'A') + 'a';
            }

            if ((*p >= 'a' && *p <= 'z') || (*p >= '0' && *p <= '9')
                    || (*p == '.'))
            {
                *q = *p;
                q++;
                *q = '\0';
            }
            p++;
        }
        int j = 0;
        while (name2[j] != '\0')
        {
            if (name2[j] == '.')
            {
                name2[j] = '\0';
                break;
            }
            j++;
            if (j == 8)
            {
                name2[j] = '\0';
                break;
            }
        }

        printf("texture file %s will be stored as %s.png\n", tex[i], name2);
    }

    tmpob = object;
    computeTriNorm(tmpob);
    tmpob = object;
    smoothTriNorm(tmpob);
    fprintf(ofile, "g\n");

    tmpob = object;
    while (tmpob != NULL)
    {
        if (tmpob->canSkip())
        {
            tmpob = tmpob->next;
            continue;
        }
        for (int i = 0; i < tmpob->numvert; i++)
        {
            fprintf(ofile, "v %lf %lf %lf\n", tmpob->vertex[i].x, tmpob->vertex[i].y, tmpob->vertex[i].z);
        }
        tmpob = tmpob->next;
    }

    tmpob = object;
    while (tmpob != NULL)
    {
        if (tmpob->canSkip())
        {
            tmpob = tmpob->next;
            continue;
        }
        for (int i = 0; i < tmpob->numvert; i++)
        {
            fprintf(ofile, "vt %lf %lf 0.0\n", tmpob->textarray[i].u, tmpob->textarray[i].v);
        }
        tmpob = tmpob->next;
    }

    tmpob = object;
    while (tmpob != NULL)
    {
        if (tmpob->canSkip())
        {
            tmpob = tmpob->next;
            continue;
        }
        for (int i = 0; i < tmpob->numvert; i++)
        {
            fprintf(ofile, "vn %lf %lf %lf\n", tmpob->snorm[i].x, tmpob->snorm[i].y, tmpob->snorm[i].z);
        }
        tmpob = tmpob->next;
    }
    fprintf(ofile, "g OB1\n");
    fprintf(ofile, "s 1\n");
    fprintf(ofile, "usemtl default\n");
    tmpob = object;
    while (tmpob != NULL)
    {
        if (tmpob->canSkip())
        {
            tmpob = tmpob->next;
            continue;
        }
        ind = tmpob->numvert;
        printf("making obj face for %s\n", tmpob->name.c_str());

        for (int i = 0; i < tmpob->numsurf; i++)
        {
            int v1, v2, v3;
            v1 = tmpob->vertexarray[i * 3].indice;
            v2 = tmpob->vertexarray[i * 3 + 1].indice;
            v3 = tmpob->vertexarray[i * 3 + 2].indice;
            fprintf(ofile, "f %d/%d/%d %d/%d/%d %d/%d/%d\n", v1 + deltav,
                    v1 + deltav, v1 + deltav, v2 + deltav, v2 + deltav,
                    v2 + deltav, v3 + deltav, v3 + deltav, v3 + deltav);
        }
        deltav += ind;
        tmpob = tmpob->next;
    }
    fprintf(ofile, "end\n");

    fclose(ofile);
	fclose(tfile);
}

void stripifyOb(FILE * ofile, ob_t * object, int writeit)
{
    FILE *stripeout, *stripein;
    char line[256];
    char filename[50];
    char command[256];
    unsigned int NumStrips;
    unsigned int NumStripPoints;
    unsigned int * StripPoint;
    unsigned int * StripStart;
    unsigned int * StripLength;
    unsigned int CurrentStripNumber;
    unsigned int CurrentStripStart;
    unsigned int CurrentStripLength;
    unsigned int CurrentStripPoint;
    int debj = 0;
    int dege = 0;
    int k, v1, v2, v0;
    k = 0;
    int tri = 0;
    int tritotal = 0;
    int multitex = 0;

    if (object->numsurf < 3 && writeit == 0)
        return;
    fprintf(stderr, "stripifying %s                    \r", object->name.c_str());
    sprintf(filename, "temp.obj");
    stripeout = fopen(filename, "w");
    for (int i = 0; i < object->numvert; i++)
        fprintf(stripeout, "v 0.0 0.0 0.0\n");

    for (int i = 0; i < object->numsurf; i++)
    {
        fprintf(stripeout, "f %d %d %d\n",
                object->vertexarray[i * 3].indice + 1,
                object->vertexarray[i * 3 + 1].indice + 1,
                object->vertexarray[i * 3 + 2].indice + 1);
    }

    fclose(stripeout);
#ifdef WIN32
    sprintf(command, "stripe.exe %s >shudup", filename);
    system(command);
    sprintf(command, "erase shudup");
    system(command);
    strcat(filename, "f");
    stripein = fopen(filename,"r");
#else
    sprintf(command, "stripe %s >/dev/null", filename);
    if (system(command) < 0)
        printf("Calling stripe failed");
    sprintf(command, "rm %s", filename);
    if (system(command) < 0)
        printf("Calling stripe failed");
    strcat(filename, "f");
    stripein = fopen("temp.objf", "r");
#endif

    /* Count the number of strip points and initialize PolyList */
    NumStrips = 0;
    NumStripPoints = 0;

    while (fscanf(stripein, "%255s", line) != EOF)
    {
        switch (line[0])
        {
        case '#':
        case 'v':
            /* # is a comment, v is a vertex, we ignore both */
            while (fgetc(stripein) != '\n')
                ;
            break;

        case 't':
            /* t is the start of a new triangle strip */
            NumStrips++;
            break;

        case 'q':
        case '%':
            /* q is another entry in the current strip, % is a STRIPE type */
            break;

        default:
            /* Anything else is a point in the current triangle strip */
            NumStripPoints++;
            break;
        }
    }

    if (object->hasName())
        printf("name=%s stripnumber =%u\n", object->name.c_str(), NumStrips);
    /* Allocate enough memory for what we just read */
    if ((StripPoint = (unsigned int*)malloc(sizeof(unsigned int) * NumStripPoints)) == 0)
    {
        printf("Problem mallocing while stripifying\n");
        fclose(stripein);
        exit(-1);
    }
    if ((StripStart = (unsigned int*)malloc(sizeof(unsigned int) * NumStrips)) == 0)
    {
        printf("Problem mallocing while stripifying\n");
        fclose(stripein);
        exit(-1);
    }
    if ((StripLength = (unsigned int*)malloc(sizeof(unsigned int) * NumStrips)) == 0)
    {
        printf("Problem mallocing while stripifying\n");
        fclose(stripein);
        exit(-1);
    }

    /* Fill the triangle strip lists with the STRIPE data */
    rewind(stripein);

    CurrentStripNumber = 0;
    CurrentStripStart = 0;
    CurrentStripLength = 0;
    CurrentStripPoint = 0;

    for (unsigned int j = 0; j < NumStrips; j++)
    {
        StripStart[j] = 0;
        StripLength[j] = 0;
    }

    while (fscanf(stripein, "%255s", line) != EOF)
    {
        switch (line[0])
        {
        case '#':
        case 'v':
            /* # is a comment, v is a vertex, we ignore both */
            while (fgetc(stripein) != '\n')
                ;
            break;

        case 't':
            /* t is the start of a new triangle strip */
            if (CurrentStripNumber > 0)
            {
                StripStart[CurrentStripNumber - 1] = CurrentStripStart;
                StripLength[CurrentStripNumber - 1] = CurrentStripLength;
                printf("striplength %u\n",
                        StripLength[CurrentStripNumber - 1]);
            }
            CurrentStripNumber++;
            CurrentStripStart = CurrentStripPoint;
            CurrentStripLength = 0;
            printf("new strip\n");
            break;

        case 'q':
        case '%':
            /* q is another entry in the current strip, % is a STRIPE type */
            break;

        default:
            /* Anything else is a point in the current triangle strip */
            StripPoint[CurrentStripPoint] = (unsigned int) (atoi(line) - 1);
            CurrentStripPoint++;
            CurrentStripLength++;
            break;
        }
    }
    if (CurrentStripNumber > 0)
    {
        StripStart[CurrentStripNumber - 1] = CurrentStripStart;
        StripLength[CurrentStripNumber - 1] = CurrentStripLength;
    }

    fclose(stripein);

    std::vector<tcoord_t> stripvertexarray(object->numvertice * 10);
    k = 0;
    dege = 0;
    if (writeit == 1)
    {
        fprintf(ofile, "numsurf %u\n", NumStrips);

    }
    if (object->hasMultiTexture())
        multitex = 1;
    else
        multitex = 0;

    for (unsigned int i = 0; i < NumStrips; i++)
    {
        /* get the first triangle */
        v1 = StripPoint[StripStart[i]];
        v2 = StripPoint[StripStart[i] + 1];
        debj = 2;
        tri = 0;
        if (writeit == 1)
        {
            /** For some reason the surf attribute is modified for the output.
             *  The surfaces are made double-sided, although stripification doesn't
             *  introduce this property.
             *  Thus, instead of the whole if-condition the actual code for outputting
             *  this attribute should simply be:
             *
             *  fprintf(ofile, "SURF 0x%2x\n", object->attrSurf)
             *
             *  However this causes huge artifacts in the generated tracks. Thus, the
             *  following is not correct behavior but works for whatever reason.
             *  It is a legacy from TORCS and no details are known why it is done.
             *
             *  A proper solution would be to remove the attribute modification and
             *  rework all tracks to fit the correct behavior.
             */
            if (object->attrSurf)
            {
                fprintf(ofile, "SURF 0x%2x\n",
                        (object->attrSurf & 0xF0) | 0x04);
            }
            else
            {
                fprintf(ofile, "SURF 0x24\n");
            }
            fprintf(ofile, "mat %d\n", object->attrMat);
            fprintf(ofile, "refs %u\n", StripLength[i]);
            if (multitex == 0)
            {
                fprintf(ofile, "%d %.5f %.5f\n", v1, object->textarray[v1].u, object->textarray[v1].v);
            }
            else
            {
                fprintf(ofile, "%d %.5f %.5f", v1, object->textarray[v1].u, object->textarray[v1].v);
                if (object->hasTexture1())
                    fprintf(ofile, " %.5f %.5f", object->textarray1[v1].u, object->textarray1[v1].v);
                if (object->hasTexture2())
                    fprintf(ofile, " %.5f %.5f", object->textarray2[v1].u, object->textarray2[v1].v);
                if (object->hasTexture3())
                    fprintf(ofile, " %.5f %.5f", object->textarray3[v1].u, object->textarray3[v1].v);
                fprintf(ofile, "\n");
            }
            if (multitex == 0)
            {
                fprintf(ofile, "%d %.5f %.5f\n", v2, object->textarray[v2].u, object->textarray[v2].v);
            }
            else
            {
                fprintf(ofile, "%d %.5f %.5f", v2, object->textarray[v2].u, object->textarray[v2].v);
                if (object->hasTexture1())
                     fprintf(ofile, " %.5f %.5f", object->textarray1[v2].u, object->textarray1[v2].v);
                if (object->hasTexture2())
                    fprintf(ofile, " %.5f %.5f", object->textarray2[v2].u, object->textarray2[v2].v);
                if (object->hasTexture3())
                    fprintf(ofile, " %.5f %.5f", object->textarray3[v2].u, object->textarray3[v2].v);
                fprintf(ofile, "\n");
            }
        }
        for (unsigned int j = debj; j < StripLength[i]; j++)
        {
            v0 = StripPoint[StripStart[i] + j];
            /*printf("adding point %d\n",v0);*/

            if (writeit == 0)
            {
                stripvertexarray[k].indice = v1;
                stripvertexarray[k].uv = object->textarray[v1];
                stripvertexarray[k].saved = false;
                k++;
                stripvertexarray[k].indice = v2;
                stripvertexarray[k].uv = object->textarray[v2];
                stripvertexarray[k].saved = false;
                k++;
                stripvertexarray[k].indice = v0;
                stripvertexarray[k].uv = object->textarray[v0];
                stripvertexarray[k].saved = false;
                k++;
                if ((tri % 2) == 0)
                {
                    v1 = v0;
                }
                else
                {
                    v2 = v0;
                }
            }
            else
            {
                if (multitex == 0)
                    fprintf(ofile, "%d %.5f %.5f\n", v0, object->textarray[v0].u, object->textarray[v0].v);
                else
                {
                    fprintf(ofile, "%d %.5f %.5f", v0, object->textarray[v0].u, object->textarray[v0].v);
                    if (object->hasTexture1())
                        fprintf(ofile, " %.5f %.5f", object->textarray1[v0].u, object->textarray1[v0].v);
                    if (object->hasTexture2())
                        fprintf(ofile, " %.5f %.5f", object->textarray2[v0].u, object->textarray2[v0].v);
                    if (object->hasTexture3())
                        fprintf(ofile, " %.5f %.5f", object->textarray3[v0].u, object->textarray3[v0].v);
                    fprintf(ofile, "\n");
                }
            }

            tri++;
        }
        tritotal += tri;
    }

    printf("strips for %s : number of strips %u : average of points triangles by strips %.2f\n",
           object->name.c_str(), NumStrips,
           (float) ((float) tritotal - (float) dege) / ((float) NumStrips));
    if (writeit == 0)
    {
        if (tritotal != object->numsurf)
        {
            printf("warning: error nb surf= %d != %d  degenerated triangles %d  tritotal=%d for %s\n",
                   tritotal, object->numsurf, dege, tritotal - dege,
                   object->name.c_str());
        }
        object->vertexarray = stripvertexarray;
        object->numvertice = k;
        object->numsurf = k / 3;
    }
    free(StripPoint);
    free(StripStart);
    free(StripLength);
}

void computeSaveAC3DM(const std::string & OutputFilename, ob_t * object, const std::vector<mat_t> &materials)
{
    char name2[256];
    ob_t * tmpob = NULL;
    int deltav = 1;
    int ind = 0;
    FILE * ofile;

    if ((ofile = fopen(OutputFilename.c_str(), "w")) == NULL)
    {
        fprintf(stderr, "failed to open %s\n", OutputFilename.c_str());
        return;
    }

    printMaterials(ofile, materials);

    texnum = 0;
    tmpob = object;
    while (tmpob != NULL)
    {
        if (tmpob->canSkip())
        {
            tmpob = tmpob->next;
            continue;
        }
        tmpob = tmpob->next;
    }

    for (int i = 0; i < texnum; i++)
    {

        printf("analysing  %s\n", tex[i]);
        char *p = tex[i];
        char *q = name2;
        while (*p)
        {
            if ((*p <= 'Z' && *p >= 'A'))
            {
                *p = (*p - 'A') + 'a';
            }

            if ((*p >= 'a' && *p <= 'z') || (*p >= '0' && *p <= '9')
                    || (*p == '.'))
            {
                *q = *p;
                q++;
                *q = '\0';
            }
            p++;
        }
        int j = 0;
        while (name2[j] != '\0')
        {
            if (name2[j] == '.')
            {
                name2[j] = '\0';
                break;
            }
            j++;
            if (j == 8)
            {
                name2[j] = '\0';
                break;
            }
        }

        printf("texture file %s will be stored as %s.png\n", tex[i], name2);
    }

    tmpob = object;
    computeTriNorm(tmpob);
    tmpob = object;
    smoothTriNorm(tmpob);
    fprintf(ofile, "g\n");

    tmpob = object;
    while (tmpob != NULL)
    {
        if (tmpob->canSkip())
        {
            tmpob = tmpob->next;
            continue;
        }
        for (int i = 0; i < tmpob->numvert; i++)
        {
            fprintf(ofile, "v %lf %lf %lf\n", tmpob->vertex[i].x,
                    tmpob->vertex[i].y, tmpob->vertex[i].z);
        }
        tmpob = tmpob->next;
    }

    tmpob = object;
    while (tmpob != NULL)
    {
        if (tmpob->canSkip())
        {
            tmpob = tmpob->next;
            continue;
        }
        for (int i = 0; i < tmpob->numvert; i++)
        {
            fprintf(ofile, "vt %lf %lf 0.0\n", tmpob->textarray[i].u, tmpob->textarray[i].v);
        }
        tmpob = tmpob->next;
    }

    tmpob = object;
    while (tmpob != NULL)
    {
        if (tmpob->canSkip())
        {
            tmpob = tmpob->next;
            continue;
        }
        for (int i = 0; i < tmpob->numvert; i++)
        {
            fprintf(ofile, "vn %lf %lf %lf\n", tmpob->snorm[i].x,
                    tmpob->snorm[i].y, tmpob->snorm[i].z);
        }
        tmpob = tmpob->next;
    }
    fprintf(ofile, "g OB1\n");
    fprintf(ofile, "s 1\n");
    fprintf(ofile, "usemtl default\n");
    tmpob = object;
    while (tmpob != NULL)
    {
        if (tmpob->canSkip())
        {
            tmpob = tmpob->next;
            continue;
        }
        ind = tmpob->numvert;
        printf("making obj face for %s\n", tmpob->name.c_str());

        for (int i = 0; i < tmpob->numsurf; i++)
        {
            int v1, v2, v3;
            v1 = tmpob->vertexarray[i * 3].indice;
            v2 = tmpob->vertexarray[i * 3 + 1].indice;
            v3 = tmpob->vertexarray[i * 3 + 2].indice;
            fprintf(ofile, "f %d/%d/%d %d/%d/%d %d/%d/%d\n", v1 + deltav,
                    v1 + deltav, v1 + deltav, v2 + deltav, v2 + deltav,
                    v2 + deltav, v3 + deltav, v3 + deltav, v3 + deltav);
        }
        deltav += ind;
        tmpob = tmpob->next;
    }
    fprintf(ofile, "end\n");
    fclose(ofile);
}

void mapNormalToSphere(ob_t *object)
{
    double xmin = 9999;
    double ymin = 9999;
    double zmin = 9999;
    double xmax = -9999;
    double ymax = -9999;
    double zmax = -9999;
    double pospt = 0;
    double ddmax = 0;
    double ddmin = 10000;
    ob_t * tmpob = object;

    while (tmpob != NULL)
    {
        if (tmpob->canSkip())
        {
            tmpob = tmpob->next;
            continue;
        }
        if (tmpob->x_min < xmin)
            xmin = tmpob->x_min;
        if (tmpob->y_min < ymin)
            ymin = tmpob->y_min;
        if (tmpob->z_min < zmin)
            zmin = tmpob->z_min;

        if (tmpob->x_max > xmax)
            xmax = tmpob->x_max;
        if (tmpob->y_max > ymax)
            ymax = tmpob->y_max;
        if (tmpob->z_max > zmax)
            zmax = tmpob->z_max;

        for (int i = 0; i < tmpob->numvert; i++)
        {
            /* compute the same normal for each points in the surface */
            pospt = sqrt(tmpob->vertex[i].x * tmpob->vertex[i].x +
                         tmpob->vertex[i].y * tmpob->vertex[i].y +
                         tmpob->vertex[i].z * tmpob->vertex[i].z);
            if (pospt > ddmax)
                ddmax = pospt;
            if (pospt < ddmin)
                ddmin = pospt;
        }
        tmpob = tmpob->next;
    }
    ddmin = (ddmax - ddmin) / 2 + ddmin;
    tmpob = object;
    while (tmpob != NULL)
    {
        if (tmpob->canSkip())
        {
            tmpob = tmpob->next;
            continue;
        }
        for (int i = 0; i < tmpob->numvert; i++)
        {
            double fact = 0;
            /* compute the same normal for each points in the surface */
            pospt = sqrt(tmpob->vertex[i].x * tmpob->vertex[i].x +
                         tmpob->vertex[i].y * tmpob->vertex[i].y +
                         tmpob->vertex[i].z * tmpob->vertex[i].z);
            fact = ddmin / pospt;
            if (fact > 1.0)
                fact = 1.0;
            tmpob->snorm[i] *= fact;
        }

        tmpob = tmpob->next;
    }
}

void mapTextureEnv(ob_t *object)
{
    double x, y, z, zt, lg;
    double z_min = 10000;
    double z_max = -10000;
    ob_t * tmpob = object;

    while (tmpob != NULL)
    {
        if (tmpob->canSkip())
        {
            tmpob = tmpob->next;
            continue;
        }
        for (int j = 0; j < tmpob->numvert; j++)
        {
            z = tmpob->vertex[j].z + tmpob->snorm[j].z / 3.0;
            if (z > z_max)
                z_max = z;
            if (z < z_min)
                z_min = z;
        }
        tmpob = tmpob->next;
    }

    tmpob = object;
    while (tmpob != NULL)
    {
        if (tmpob->canSkip())
        {
            tmpob = tmpob->next;
            continue;
        }
        /* create the new vertex array */
        tmpob->textarray1 = tmpob->textarray;
        tmpob->textarray2 = tmpob->textarray;
        tmpob->texture1 = tmpob->texture;
        tmpob->texture2 = tmpob->texture;
        for (int i = 0; i < tmpob->numvert; i++)
        {
            x = tmpob->vertex[i].x;
            y = tmpob->vertex[i].y;
            z = tmpob->vertex[i].z;
            lg = sqrt(x * x + y * y + z * z);
            if (lg != 0.0)
            {
                x /= lg;
                y /= lg;
                z /= lg;
            }
            else
            {
                x = 0;
                y = 0;
                z = 1;
            }
            //z_min = 0;
            tmpob->textarray1[i].u = 0.5 + x / 2.0;
            zt = (z + tmpob->snorm[i].z / 3.0 - z_min) / (z_max - z_min);
            tmpob->textarray1[i].v = zt;

            if (tmpob->textarray1[i].v > 1.0)
                tmpob->textarray1[i].v = 0.999;
            else if (tmpob->textarray1[i].v < 0.0)
                tmpob->textarray1[i].v = 0.001;

            tmpob->textarray2[i].u = 0.5 + y / 2.0;
            tmpob->textarray2[i].v = z;
        }
        tmpob = tmpob->next;
    }
}

void mapTextureEnvOld(ob_t *object)
{
    ob_t * tmpob = NULL;
    double x_min = 10000;
    double x_max = -10000;
    double y_min = 10000;
    double y_max = -10000;
    double z_min = 10000;
    double z_max = -10000;
    double u_min = 10000;
    double u_max = -10000;
    double v_min = 10000;
    double v_max = -10000;

    double u2_min = 10000;
    double u2_max = -10000;
    double v2_min = 10000;
    double v2_max = -10000;

    tmpob = object;
    while (tmpob != NULL)
    {
        if (tmpob->canSkip())
        {
            tmpob = tmpob->next;
            continue;
        }
        for (int j = 0; j < tmpob->numvert; j++)
        {
            if (tmpob->vertex[j].x > x_max)
                x_max = tmpob->vertex[j].x;
            if (tmpob->vertex[j].x < x_min)
                x_min = tmpob->vertex[j].x;

            if (tmpob->vertex[j].y > y_max)
                y_max = tmpob->vertex[j].y;
            if (tmpob->vertex[j].y < y_min)
                y_min = tmpob->vertex[j].y;

            if (tmpob->vertex[j].z > z_max)
                z_max = tmpob->vertex[j].z;
            if (tmpob->vertex[j].z < z_min)
                z_min = tmpob->vertex[j].z;
        }
        tmpob = tmpob->next;
    }
    tmpob = object;
    while (tmpob != NULL)
    {
        if (tmpob->canSkip())
        {
            tmpob = tmpob->next;
            continue;
        }
        /* create the new vertex array */
        tmpob->textarray1 = tmpob->textarray;
        tmpob->textarray2 = tmpob->textarray;
        tmpob->texture1 = tmpob->texture;
        tmpob->texture2 = tmpob->texture;
        for (int i = 0; i < tmpob->numvert; i++)
        {
            tmpob->textarray1[i].u = (tmpob->vertex[i].x - x_min)
                    / (x_max - x_min) + (tmpob->snorm[i].x) / 2;
            tmpob->textarray1[i].v = ((tmpob->vertex[i].z - z_min)
                    / (z_max - z_min)) + (tmpob->snorm[i].z) / 2;
            tmpob->textarray2[i].u = ((tmpob->vertex[i].x - x_min)
                    / (x_max - x_min)) + (tmpob->snorm[i].x) / 2;
            tmpob->textarray2[i].v = ((tmpob->vertex[i].y - y_min)
                    / (x_max - x_min)) + (tmpob->snorm[i].y) / 2;

            if (tmpob->textarray1[i].u > u_max)
                u_max = tmpob->textarray1[i].u;

            if (tmpob->textarray1[i].v > v_max)
                v_max = tmpob->textarray1[i].v;

            if (tmpob->textarray1[i].u < u_min)
                u_min = tmpob->textarray1[i].u;

            if (tmpob->textarray1[i].v < v_min)
                v_min = tmpob->textarray1[i].v;

            if (tmpob->textarray2[i].u > u2_max)
                u2_max = tmpob->textarray2[i].u;

            if (tmpob->textarray2[i].v > v2_max)
                v2_max = tmpob->textarray2[i].v;

            if (tmpob->textarray2[i].u < u2_min)
                u2_min = tmpob->textarray2[i].u;

            if (tmpob->textarray2[i].v < v2_min)
                v2_min = tmpob->textarray2[i].v;

        }
        tmpob = tmpob->next;
    }

    /* clamp the texture coord */
    tmpob = object;
    while (tmpob != NULL)
    {
        if (tmpob->canSkip())
        {
            tmpob = tmpob->next;
            continue;
        }

        for (int i = 0; i < tmpob->numvert; i++)
        {
            tmpob->textarray1[i].u = (tmpob->textarray1[i].u - u_min) / (u_max - u_min);
            tmpob->textarray1[i].v = (tmpob->textarray1[i].v - v_min) / (v_max - v_min);

            tmpob->textarray2[i].u = (tmpob->textarray2[i].u - u2_min) / (u2_max - u2_min) - 0.5;
            tmpob->textarray2[i].v = (tmpob->textarray2[i].v - v2_min) / (v2_max - v2_min) - 0.5;
        }
        tmpob = tmpob->next;
    }
}

void mapNormalToSphere2(ob_t *object)
{
    ob_t * tmpob = object;

    while (tmpob != NULL)
    {
        if (tmpob->canSkip())
        {
            tmpob = tmpob->next;
            continue;
        }

        for (int i = 0; i < tmpob->numvert; i++)
        {
            /* compute the same normal for each points in the surface */
            /*      tmpob->norm[i] = tmpob->vertex[i]; */
            /*      normalize(&tmpob->norm[i]); */
            /*      tmpob->snorm[i] += tmpob->norm[i]; */
            normalize(&tmpob->snorm[i]);
        }
        tmpob = tmpob->next;
    }
}

void normalMap(ob_t * object)
{
    double x_min = 99999;
    double y_min = 99999;
    double z_min = 99999;
    double x_max = -99999;
    double y_max = -99999;
    double z_max = -99999;
    ob_t * tmpob = object;

    while (tmpob != NULL)
    {
        if (tmpob->canSkip())
        {
            tmpob = tmpob->next;
            continue;
        }

        for (int j = 0; j < tmpob->numvert; j++)
        {
            if (tmpob->vertex[j].x > x_max)
                x_max = tmpob->vertex[j].x;
            if (tmpob->vertex[j].x < x_min)
                x_min = tmpob->vertex[j].x;

            if (tmpob->vertex[j].y > y_max)
                y_max = tmpob->vertex[j].y;
            if (tmpob->vertex[j].y < y_min)
                y_min = tmpob->vertex[j].y;

            if (tmpob->vertex[j].z > z_max)
                z_max = tmpob->vertex[j].z;
            if (tmpob->vertex[j].z < z_min)
                z_min = tmpob->vertex[j].z;
        }
        tmpob = tmpob->next;
    }
    tmpob = object;

    while (tmpob != NULL)
    {
        if (tmpob->canSkip())
        {
            tmpob = tmpob->next;
            continue;
        }
        printf("normalMap : handling %s\n", tmpob->name.c_str());
        for (int i = 0; i < tmpob->numvert; i++)
        {
            tmpob->textarray[i].u = (tmpob->vertex[i].x - x_min) / (x_max - x_min);
            tmpob->textarray[i].v = (tmpob->vertex[i].y - y_min) / (y_max - y_min);
        }
        tmpob->texture = shadowtexture;
        tmpob = tmpob->next;
    }
}

void normalMap01(ob_t * object)
{
    double x_min = 99999;
    double y_min = 99999;
    double z_min = 99999;
    double x_max = -99999;
    double y_max = -99999;
    double z_max = -99999;
    ob_t * tmpob = object;

    while (tmpob != NULL)
    {
        if (tmpob->canSkip())
        {
            tmpob = tmpob->next;
            continue;
        }
        for (int j = 0; j < tmpob->numvert; j++)
        {
            if (tmpob->vertex[j].x > x_max)
                x_max = tmpob->vertex[j].x;
            if (tmpob->vertex[j].x < x_min)
                x_min = tmpob->vertex[j].x;

            if (tmpob->vertex[j].y > y_max)
                y_max = tmpob->vertex[j].y;
            if (tmpob->vertex[j].y < y_min)
                y_min = tmpob->vertex[j].y;

            if (tmpob->vertex[j].z > z_max)
                z_max = tmpob->vertex[j].z;
            if (tmpob->vertex[j].z < z_min)
                z_min = tmpob->vertex[j].z;
        }

        tmpob = tmpob->next;
    }
    tmpob = object;

    while (tmpob != NULL)
    {
        if (tmpob->canSkip())
        {
            tmpob = tmpob->next;
            continue;
        }
        tmpob->textarray3.resize(tmpob->numvert);
        printf("normalMap : handling %s\n", tmpob->name.c_str());
        for (int i = 0; i < tmpob->numvert; i++)
        {
            tmpob->textarray3[i].u = (tmpob->vertex[i].x - x_min) / (x_max - x_min) - 0.5;
            tmpob->textarray3[i].v = (tmpob->vertex[i].y - y_min) / (y_max - y_min) - 0.5;
        }
        tmpob->texture3 = tmpob->texture;
        tmpob = tmpob->next;
    }
}

void computeSaveAC3DStrip(const std::string & OutputFilename, ob_t * object, const std::vector<mat_t> &materials)
{
    ob_t * tmpob = NULL;
    int numg = 0;
    char *p;
    char *q = NULL;
    bool lastpass = false;
    int nborder = 0;
    bool ordering = false;
    FILE * ofile = NULL;

    if ((ofile = fopen(OutputFilename.c_str(), "w")) == NULL)
    {
        fprintf(stderr, "failed to open %s\n", OutputFilename.c_str());
        return;
    }
    smoothTriNorm(object);
    if (isobjectacar)
    {
        mapNormalToSphere2(object);
        normalMap01(object);

        if (extendedEnvCoord)
            mapTextureEnv(object);
        if (collapseObject)
            mergeSplitted(&object);
    }
    fprintf(ofile, "AC3Db\n");
    printMaterials(ofile, materials);
    fprintf(ofile, "OBJECT world\n");

    if (OrderString)
    {
        fprintf(stderr, "ordering objects according to  %s\n", OrderString);
        p = OrderString;
        ordering = true;
        nborder = 1;
        while (true)
        {
            q = strstr(p, ";");
            if (q != NULL)
                nborder++;
            else
                break;
            p = q + 1;
            if (*p == '\0')
            {
                nborder--;
                break;
            }
        }
    }
    else
    {
        ordering = false;
        nborder = 0;
    }

    tmpob = object;
    while (tmpob != NULL)
    {
        if (!tmpob->hasName())
        {
            tmpob = tmpob->next;
            continue;
        }
        if (tmpob->name == "world" || (tmpob->type == "world" && tmpob->numvert == 0 && tmpob->numsurf == 0))
        {
            tmpob = tmpob->next;
            continue;
        }
        if (tmpob->name == "root")
        {
            tmpob = tmpob->next;
            continue;
        }
        if (tmpob->name == "group" || (tmpob->type == "group" && tmpob->numvert == 0 && tmpob->numsurf == 0))
        {
            tmpob = tmpob->next;
            continue;
        }
        /* don't count empty objects */
        if (tmpob->type == "poly" && tmpob->numvert == 0 && tmpob->numsurf == 0 && tmpob->kids == 0)
        {
            tmpob = tmpob->next;
            continue;
        }
        numg++;
        tmpob->saved = false;
        tmpob = tmpob->next;
    }

    fprintf(ofile, "kids %d\n", numg);

    texnum = 0;

    p = OrderString;
    q = OrderString;
    nborder++;
    for (int ik = 0; ik < nborder; ik++)
    {
        if (ordering)
        {
            /* look to the current object name to save */
            if (p == NULL)
                lastpass = true;
            else
            {
                q = p;
                p = strstr(p, ";");
                if (p != NULL)
                {
                    *p = '\0';
                    p++;
                }
            }
        }
        tmpob = object;
        while (tmpob != NULL)
        {
            int texnofound = 0;
            if (tmpob->canSkip())
            {
                tmpob = tmpob->next;
                continue;
            }
            texnofound = 1;
            for (int i = 0; i < texnum; i++)
            {
                if (!tmpob->hasTexture())
                {
                    texnofound = 0;
                    break;
                }
                if (!strncmp(tex[i], tmpob->texture.c_str(), 13))
                {
                    texnofound = 0;
                    break;
                }
                else
                    texnofound = 1;
            }
            if (texnofound == 1)
            {
                if (tmpob->hasTexture())
                {
                    strcpy(tex[texnum], tmpob->texture.c_str());
                    tex[texnum][13] = '\0';
                    /*sprintf(tex[texnum],"%s",tmpob->texture);*/
                }
                texnum++;
            }
            printf("name=%s x_min=%.1f y_min=%.1f x_max=%.1f y_max=%.1f\n",
                    tmpob->name.c_str(), tmpob->x_min, tmpob->y_min, tmpob->x_max,
                    tmpob->y_max);

            tmpob = tmpob->next;
        }

        tmpob = object;
        tmpob->kids_o = 0;

        tmpob = object;
        while (tmpob != NULL)
        {
            tmpob->inkids_o = false;
            tmpob = tmpob->next;
        }

        tmpob = object;
        while (tmpob != NULL)
        {
            if (tmpob->canSkip())
            {
                tmpob = tmpob->next;
                continue;
            }
            if (!tmpob->saved)
            {
                if (ordering && !lastpass)
                {
                    if (tmpob->name == q)
                    {
                        printOb(ofile, tmpob);
                        printf("object =%s num kids_o=%d test with %s\n", tmpob->name.c_str(),
                               tmpob->kids_o, q);
                    }
                    else
                    {
                        std::string nameBuf(q);
                        nameBuf += 's';
                        if (tmpob->name == nameBuf)
                        {
                            printOb(ofile, tmpob);
                            printf("object =%s num kids_o=%d\n", tmpob->name.c_str(),
                                   tmpob->kids_o);
                        }
                    }
                }
                else
                {
                    printOb(ofile, tmpob);
                    printf("object =%s num kids_o=%d\n", tmpob->name.c_str(),
                            tmpob->kids_o);
                }
            }
            tmpob = tmpob->next;
        }
    }
    tmpIndice = 0;
    /* do the job */
    printf("\nend\n");

    fclose(ofile);
}

ob_t * mergeObject(ob_t *ob1, ob_t * ob2, char * nameS)
{
    ob_t tobS;
    static int oldva1[10000];
    static int oldva2[10000];
    int n = 0;
    int numtri = ob1->numsurf + ob2->numsurf;

    printf("merging %s with %s  tri=%d\n", ob1->name.c_str(), ob2->name.c_str(), numtri);
    memset(oldva1, -1, sizeof(oldva1));
    memset(oldva2, -1, sizeof(oldva2));
    tobS.numsurf = ob1->numsurf;
    tobS.vertexarray.resize(numtri * 3);
    tobS.vertex.resize(numtri * 3);
    tobS.norm.resize(numtri * 3);
    tobS.snorm.resize(numtri * 3);
    tobS.textarray.resize(numtri * 3);
    tobS.textarray1.resize(numtri * 3);
    tobS.textarray2.resize(numtri * 3);
    tobS.textarray3.resize(numtri * 3);

    std::copy_n(ob1->vertex.begin(), ob1->vertex.size(), tobS.vertex.begin());
    std::copy_n(ob1->norm.begin(), ob1->norm.size(), tobS.norm.begin());
    std::copy_n(ob1->snorm.begin(), ob1->snorm.size(), tobS.snorm.begin());
    std::copy_n(ob1->vertexarray.begin(), ob1->vertexarray.size(), tobS.vertexarray.begin());
    std::copy_n(ob1->textarray.begin(), ob1->textarray.size(), tobS.textarray.begin());

    if (ob1->hasTexture1())
        std::copy_n(ob1->textarray1.begin(), ob1->textarray1.size(), tobS.textarray1.begin());

    if (ob1->hasTexture2())
        std::copy_n(ob1->textarray2.begin(), ob1->textarray2.size(), tobS.textarray2.begin());

    if (ob1->hasTexture3())
        std::copy_n(ob1->textarray3.begin(), ob1->textarray3.size(), tobS.textarray3.begin());

    n = ob1->numvert;
    for (int i = 0; i < ob2->numvert; i++)
    {
        for (int j = 0; j < ob1->numvert; j++)
        {
            if (ob2->vertex[i] == ob1->vertex[j] && ob2->textarray[i] == ob1->textarray[j])
            {
                oldva1[i] = j;
            }
        }
    }

    for (int i = 0; i < ob2->numvert; i++)
    {
        if (oldva1[i] == -1)
        {
            oldva1[i] = n;
            tobS.textarray[n] = ob2->textarray[i];
            if (ob2->hasTexture1())
                tobS.textarray1[n] = ob2->textarray1[i];
            if (ob2->hasTexture2())
                tobS.textarray2[n] = ob2->textarray2[i];
            if (ob2->hasTexture3())
                tobS.textarray3[n] = ob2->textarray3[i];
            tobS.snorm[n] = ob2->snorm[i];
            tobS.norm[n] = ob2->norm[i];
            tobS.vertex[n] = ob2->vertex[i];

            n++;
        }
    }
    tobS.numvert = n;
    for (int i = 0; i < ob2->numsurf; i++)
    {
        bool found = false;
        for (int j = 0; j < ob1->numsurf; j++)
        {
            if (tobS.vertexarray[j * 3].indice == oldva1[ob2->vertexarray[i * 3].indice] &&
                tobS.vertexarray[j * 3 + 1].indice == oldva1[ob2->vertexarray[i * 3 + 1].indice] &&
                tobS.vertexarray[j * 3 + 2].indice == oldva1[ob2->vertexarray[i * 3 + 2].indice])
            {
                /* this face is OK */
                found = true;
                break;
            }
        }
        if (!found)
        {
            int k = tobS.numsurf;
            /* add the triangle */
            tobS.vertexarray[k * 3].indice = oldva1[ob2->vertexarray[i * 3].indice];
            tobS.vertexarray[k * 3 + 1].indice = oldva1[ob2->vertexarray[i * 3 + 1].indice];
            tobS.vertexarray[k * 3 + 2].indice = oldva1[ob2->vertexarray[i * 3 + 2].indice];
            tobS.numsurf++;
        }
    }

    ob1->numsurf = tobS.numsurf;
    ob1->numvert = tobS.numvert;
    ob1->vertex = tobS.vertex;
    ob1->norm = tobS.norm;
    ob1->snorm = tobS.snorm;
    ob1->vertexarray = tobS.vertexarray;
    ob1->textarray = tobS.textarray;
    ob1->textarray1 = tobS.textarray1;
    ob1->textarray2 = tobS.textarray2;
    ob1->textarray3 = tobS.textarray3;

    return ob1;
}

int mergeSplitted(ob_t **object)
{

    int k = 0;
    char nameS[256];
    char *p;
    ob_t * tob = NULL;
    ob_t * tob0 = NULL;
    ob_t * tobP = NULL;
#ifdef NEWSRC
    int numtri;
#endif
    int reduced = 0;

    tob = *object;
    while (tob)
    {
        if (isobjectacar)
        {
            if (!tob->hasName() || tob->nameHasStr("_s_"))
            {
                tob = tob->next;
                continue;
            }
        }
        else if (tob->nameHasStr("__split__"))
        {
            tob = tob->next;
            continue;
        }
        tobP = tob;
        tob0 = tob->next;
        sprintf(nameS, "%s", tob->name.c_str());
        if (isobjectacar)
        {
            p = strstr(nameS, "_s_");
        }
        else
            p = strstr(nameS, "__split__");
        if (p == NULL)
        {
            tob = tob->next;
            continue;
        }
        printf("looking for merge : %s\n", nameS);
        if (isobjectacar)
            p = p + strlen("_s_");
        else
            p = p + strlen("__split__");
        *p = '\0';
        k = 0;
#ifdef NEWSRC
        numtri = tob->numsurf;
#endif
        while (tob0)
        {
            if (tob0->canSkip() || tob0->type == "group")
            {
                tobP = tob0;
                tob0 = tob0->next;
                continue;
            }

            if (tob0->nameStartsWith(nameS))
            {
                ob_t *oo;
                mergeObject(tob, tob0, nameS);
                /*printf("merging %s with %s\n",nameS, tob0->name);*/
                reduced++;
                tobP->next = tob0->next;
                oo = tob0;
                tob0 = tob0->next;
                delete oo;
                k++;
                continue;
            }
            tobP = tob0;
            tob0 = tob0->next;
        }

        if (k == 0)
        {
            tob = tob->next;
            continue;
        }
        printf("need merge for %s : %d objects found\n", tob->name.c_str(), k + 1);

        /* we know that nameS has k+1 objects and need to be merged */

        /* allocate the new object */
#ifdef NEWSRC
        tobS = new ob_t;
        tobS->x_min=1000000;
        tobS->y_min=1000000;
        tobS->z_min=1000000;
        tobS->numsurf=numtri;
        tobS->vertexarray=(tcoord_t *) malloc(sizeof(tcoord_t)*numtri*3);
        tobS->norm=(point_t*)malloc(sizeof(point_t)*numtri*3);
        tobS->snorm=(point_t*)malloc(sizeof(point_t)*numtri*3);
        tobS->vertex=(point_t*)malloc(sizeof(point_t)*numtri*3);
        memset(tobS->snorm,0,sizeof(point_t )*numtri*3);
        memset(tobS->norm,0,sizeof(point_t )*numtri*3);
        tobS->textarray=(double *) malloc(sizeof(double)* numtri*2*3);
        tobS->attrSurf=tob->attrSurf;
        tobS->attrMat=tob->attrMat;
        tobS->name=nameS;
        tobS->texture=nameS;
        tobS->type= tob->type;
        tobS->data=tob->data;

        memcpy(tobS->vertex, tob->vertex,tob->numvert*sizeof(point_t));
        memcpy(tobS->vertexarray, tob->vertexarray,tob->numsurf*sizeof(tcoord_t ));
        memcpy(tobS->textarray, tob->textarray,tob->numvert*sizeof(double)*2);

        if (tob->texture1)
        {
            memcpy(tobS->textarray1, tob->textarray1,tob->numvert*2*sizeof(double));
            memcpy(tobS->vertexarray1, tob->vertexarray1,tob->numsurf*sizeof(tcoord_t ));
            tobS->texture1=tob->texture1;
        }
        if (tob->texture2)
        {
            memcpy(tobS->textarray2, tob->textarray2,tob->numvert*2*sizeof(double));
            memcpy(tobS->vertexarray2, tob->vertexarray2,tob->numsurf*sizeof(tcoord_t ));
            tobS->texture2=tob->texture2;
        }
        if (tob->texture3)
        {
            memcpy(tobS->textarray3, tob->textarray3,tob->numvert*2*sizeof(double));
            memcpy(tobS->vertexarray3, tob->vertexarray3,tob->numsurf*sizeof(tcoord_t ));
            tobS->texture3=tob->texture3;
        }

        n=tob->numvert;

        /* now add the new points */
        tob0=tob->next;
        while (tob0)
        {
            if (strnicmp(tob0->name,nameS,strlen(nameS)))
            {
                tob0=tob0->next;
                continue;
            }

            for (int j=0; j<tob0->numvert; j++)
                tobS->vertex[j] = tob->vertex[j];
            for (int j=0; j<tob->numsurf; j++)
            {
                tobS->vertexarray[j*3].indice=tob->vertexarray[j*3].indice;
                tobS->vertexarray[j*3].u=tob->vertexarray[j*3].u;
                tobS->vertexarray[j*3].v=tob->vertexarray[j*3].v;
                tobS->vertexarray[j*3+1].indice=tob->vertexarray[j*3+1].indice;
                tobS->vertexarray[j*3+1].u=tob->vertexarray[j*3+1].u;
                tobS->vertexarray[j*3+1].v=tob->vertexarray[j*3+1].v;
                tobS->vertexarray[j*3+2].indice=tob->vertexarray[j*3+2].indice;
                tobS->vertexarray[j*3+2].u=tob->vertexarray[j*3+2].u;
                tobS->vertexarray[j*3+2].v=tob->vertexarray[j*3+2].v;
            }
            for (int j=0; j<tob->numsurf*3; j++)
            {
                tobS->textarray[tobS->vertexarray[j].indice*2]=tob->vertexarray[j].u;
                tobS->textarray[tobS->vertexarray[j].indice*2+1]=tob->vertexarray[j].v;
                tobS->textarray1[tobS->vertexarray[j].indice*2]=tob->vertexarray1[j].u;
                tobS->textarray1[tobS->vertexarray[j].indice*2+1]=tob->vertexarray1[j].v;
            }
            tob0=tob0->next;
        }
        for (int j=0; j<tobS->numvert; j++)
        {
            if (tobS->vertex[j].x>tobS->x_max)
            tobS->x_max=tobS->vertex[j].x;
            if (tobS->vertex[j].x<tobS->x_min)
            tobS->x_min=tobS->vertex[j].x;

            if (tobS->vertex[j].y>tobS->y_max)
            tobS->y_max=tobS->vertex[j].y;
            if (tobS->vertex[j].y<tobS->y_min)
            tobS->y_min=tobS->vertex[j].y;

            if (tobS->vertex[j].z>tobS->z_max)
            tobS->z_max=tobS->vertex[j].z;
            if (tobS->vertex[j].z<tobS->z_min)
            tobS->z_min=tobS->vertex[j].z;
        }
#endif
        tob = tob->next;
    }

    return reduced;
}

int findPoint(point_t * vertexArray, int sizeVertexArray, point_t * theVertex)
{
    return -1;
}

#define P2(x) ((x)*(x))

double findDistmin(ob_t * ob1, ob_t *ob2)
{
    double di[16];
    double d = 100000;

    for (int i = 0; i < ob1->numvert; i++)
    {
        for (int j = 0; j < ob2->numvert; j++)
        {
            double a1 = ob1->vertex[i].x;
            double b1 = ob1->vertex[i].y;
            double a2 = ob2->vertex[j].x;
            double b2 = ob2->vertex[j].y;
            di[0] = P2(a1-a2) + P2(b1-b2);
            if (di[0] < d)
                d = di[0];
        }
    }

    return d;
}

void printMaterials(FILE *file, const std::vector<mat_t> &materials)
{
    for (size_t j = 0, end = materials.size(); j < end; j++) //for (const auto & material : mat0)
    {
        const mat_t& material = materials[j];
        fprintf(file,
            "MATERIAL %s rgb %1.2f %1.2f %1.2f amb %1.2f %1.2f %1.2f emis %1.2f %1.2f %1.2f spec %1.2f %1.2f %1.2f shi %3d trans 0\n",
            material.name.c_str(), material.rgb.r, material.rgb.g, material.rgb.b, material.amb.r,
            material.amb.g, material.amb.b, material.emis.r, material.emis.g,
            material.emis.b, material.spec.r, material.spec.g, material.spec.b,
            (int)material.shi);
    }
}
