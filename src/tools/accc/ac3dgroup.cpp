/***************************************************************************

 file        : ac3dgroup.cpp
 created     : Fri Apr 18 23:11:36 CEST 2003
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

#include <cstdio>
#include <cstdlib>
#include <ctype.h>
#include <cstring>
#include <cmath>
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <portability.h>
#include "accc.h"

void reorder(ob_t * ob, ob_t * ob2, uv_t *textarray, tcoord_t *vertexarray);
void collapseTextures(ob_t * ob0, ob_t * ob1, ob_t * ob2, ob_t * ob3);

// check if materials need to be merged
bool materialNeedsMerge(const std::vector<mat_t> &mat1, const std::vector<mat_t> &mat2)
{
    size_t mat1_count = mat1.size();
    size_t mat2_count = mat2.size();
    if (mat2_count > mat1_count)
        return true;
    for (size_t i = 0, end = mat2.size(); i < end; ++i)
    {
        if (mat1[i] != mat2[i])
            return true;
    }
    return false;
}

void loadAndGroup(const char *OutputFileName)
{
    ob_t * ob0 = NULL;
    ob_t * ob1 = NULL;
    ob_t * ob2 = NULL;
    ob_t * ob3 = NULL;
    ob_t * tmpob = NULL;
    ob_t * tmpob2 = NULL;
    std::vector<mat_t> mat0;
    std::vector<mat_t> mat1;
    std::vector<mat_t> mat2;
    std::vector<mat_t> mat3;
    FILE * ofile;
    int num_tkmn = 0;
    ob_groups_t * array_groups;
    int good_group = 0;
    int i = 0;
    double dist = 0;

    /* disable object splitting during load. We split them
     * after merging the texture channels.
     */
    splitObjectsDuringLoad = false;

    if (fileL0)
    {
        fprintf(stderr, "\nloading file %s\n", fileL0);
        loadAC(fileL0, &ob0, mat0);
    }
    if (fileL1)
    {
        fprintf(stderr, "\nloading file %s\n", fileL1);
        loadAC(fileL1, &ob1, mat1);
    }
    if (fileL2)
    {
        fprintf(stderr, "\nloading file %s\n", fileL2);
        loadAC(fileL2, &ob2, mat2);
    }
    if (fileL3)
    {
        fprintf(stderr, "\nloading file %s\n", fileL3);
        loadAC(fileL3, &ob3, mat3);
    }
    /* now collapse the texture and texture  arrays of 1 2 3 in 0 */

    smoothTriNorm(ob0);

    printf("collapsing textures\n");
    fprintf(stderr, "\ncollapsing textures\n");

    collapseTextures(ob0, ob1, ob2, ob3);

    // todo: merge materials
    if (!mat1.empty() && materialNeedsMerge(mat0, mat1))
    {
        fprintf(stderr, "materials in %s and %s need merging\n", fileL0, fileL1);
        exit(-1);
    }
    if (!mat2.empty() && materialNeedsMerge(mat0, mat2))
    {
        fprintf(stderr, "materials in %s and %s need merging\n", fileL0, fileL2);
        exit(-1);
    }
    if (!mat3.empty() && materialNeedsMerge(mat0, mat3))
    {
        fprintf(stderr, "materials in %s and %s need merging\n", fileL0, fileL3);
        exit(-1);
    }

    ob0 = splitObjects(ob0);

    /* now make groups from ob0 */

    fprintf(stderr, "making groups\n");
    tmpob = ob0;
    num_tkmn = 0;
    while (tmpob != NULL)
    {
        if (tmpob->name == NULL)
        {
            tmpob = tmpob->next;
            continue;
        }
        if (!strcmp(tmpob->name, "root"))
        {
            tmpob = tmpob->next;
            continue;
        }
        if (!strcmp(tmpob->name, "world"))
        {
            tmpob = tmpob->next;
            continue;
        }
        if (!strnicmp(tmpob->name, "tkmn", 4) && strcmp(tmpob->type, "group"))
        {
            tmpob = tmpob->next;
            num_tkmn++;
            continue;
        }

        tmpob = tmpob->next;
    }

    printf("found %d tkmn\n", num_tkmn);
    if (num_tkmn == 0)
    {
        fprintf(stderr,
                "\nERROR: cannot find any object tkmn for grouping\nAborting\n");
        exit(-1);
    }

    i = 0;
    tmpob = ob0;
    array_groups = (ob_groups_t *) malloc(sizeof(ob_groups_t) * num_tkmn);
    while (tmpob != NULL)
    {
        if (tmpob->name == NULL)
        {
            tmpob = tmpob->next;
            continue;
        }
        if (!strcmp(tmpob->name, "root"))
        {
            tmpob = tmpob->next;
            continue;
        }
        if (!strcmp(tmpob->name, "world"))
        {
            tmpob = tmpob->next;
            continue;
        }
        if (!strnicmp(tmpob->name, "tkmn", 4) && strcmp(tmpob->type, "group"))
        {
            array_groups[i].tkmn = tmpob;
            array_groups[i].numkids = 1;
            array_groups[i].name = tmpob->name;
            array_groups[i].tkmnlabel = atoi(tmpob->name + 4);
            array_groups[i].kids = NULL;
            array_groups[i].kids0 = NULL;
            array_groups[i].kids1 = NULL;
            array_groups[i].kids2 = NULL;
            array_groups[i].kids3 = NULL;
            array_groups[i].numkids0 = 0;
            array_groups[i].numkids1 = 0;
            array_groups[i].numkids2 = 0;
            array_groups[i].numkids3 = 0;
            tmpob = tmpob->next;
            i++;
            continue;
        }
        tmpob = tmpob->next;
    }

    fprintf(stderr, "dispatching objects in groups\n");
    tmpob = ob0;
    while (tmpob != NULL)
    {
        if (tmpob->name == NULL)
        {
            tmpob = tmpob->next;
            continue;
        }
        if (!strcmp(tmpob->name, "root"))
        {
            tmpob = tmpob->next;
            continue;
        }
        if (!strcmp(tmpob->name, "world"))
        {
            tmpob = tmpob->next;
            continue;
        }
        if (!strnicmp(tmpob->name, "tkmn", 4))
        {
            tmpob = tmpob->next;
            continue;
        }

        good_group = -1;
        tmpob->dist_min = 1000000;
        for (i = 0; i < num_tkmn; i++)
        {
            dist = findDistmin(array_groups[i].tkmn, tmpob);
            if (dist < tmpob->dist_min)
            {
                tmpob->dist_min = dist;
                good_group = i;
            }
            if (!strnicmp(tmpob->name, "t0RB", 4)
                    || !strnicmp(tmpob->name, "t1RB", 4)
                    || !strnicmp(tmpob->name, "t2RB", 4)
                    || !strnicmp(tmpob->name, "tkRS", 4)
                    || !strnicmp(tmpob->name, "t0LB", 4)
                    || !strnicmp(tmpob->name, "t1LB", 4)
                    || !strnicmp(tmpob->name, "t2LB", 4)
                    || !strnicmp(tmpob->name, "tkLS", 4)
                    || !strnicmp(tmpob->name, "BOLt", 4)
                    || !strnicmp(tmpob->name, "BORt", 4))
            {
                if (atoi(tmpob->name + 4) == array_groups[i].tkmnlabel)
                {
                    printf("object %s is forced in group %d\n", tmpob->name,
                            array_groups[i].tkmnlabel);
                    good_group = i;
                    break;
                }
            }
        }
        if (good_group == -1)
        {
            printf("an object in no group %s\n", tmpob->name);
            tmpob = tmpob->next;
            continue;
        }
        printf("object %s is going to group %s  at dist=%f\n", tmpob->name,
                array_groups[good_group].name, sqrt(tmpob->dist_min));
        if (array_groups[good_group].kids == NULL)
        {
            array_groups[good_group].kids = tmpob;
            tmpob = tmpob->next;
            array_groups[good_group].numkids++;
            array_groups[good_group].kids->next = NULL;
        }
        else
        {
            tmpob2 = array_groups[good_group].kids;
            array_groups[good_group].kids = tmpob;
            tmpob = tmpob->next;
            array_groups[good_group].kids->next = tmpob2;
            array_groups[good_group].numkids++;
        }

        /*tmpob=tmpob->next;*/
    }
    /* now each tkmn group contains the list of its kids */

    /* for all tkmn groups the kids are dispatched in the different group levels */

#define INSERTINGROUP(kids,ob)   {\
     ob->next=kids;\
     kids=ob;\
   }

    for (i = 0; i < num_tkmn; i++)
    {
        ob_t * tmpobnext;
        tmpob = array_groups[i].kids;
        printf("grouping level for %s\n", array_groups[i].name);
        while (tmpob != NULL)
        {
            tmpobnext = tmpob->next;
            if (tmpob->name == NULL)
            {
                tmpob = tmpobnext;
                continue;
            }
            if (!strnicmp(tmpob->name, "tkrb", 4))
            {
                array_groups[i].numkids--;
                array_groups[i].numkids0++;
                INSERTINGROUP(array_groups[i].kids0, tmpob);
                printf("inserting %s in group 0 of %s\n", tmpob->name,
                        array_groups[i].name);
                tmpob = tmpobnext;
                continue;
            }
            if (!strnicmp(tmpob->name, "tklb", 4))
            {
                array_groups[i].numkids--;
                array_groups[i].numkids0++;
                INSERTINGROUP(array_groups[i].kids0, tmpob);
                printf("inserting %s in group 0 of %s\n", tmpob->name,
                        array_groups[i].name);
                tmpob = tmpobnext;
                continue;
            }
            if (!strnicmp(tmpob->name, "tkrs", 4))
            {
                array_groups[i].numkids--;
                array_groups[i].numkids0++;
                INSERTINGROUP(array_groups[i].kids0, tmpob);
                printf("inserting %s in group 0 of %s\n", tmpob->name,
                        array_groups[i].name);
                tmpob = tmpobnext;
                continue;
            }
            if (!strnicmp(tmpob->name, "tkls", 4))
            {
                array_groups[i].numkids--;
                array_groups[i].numkids0++;
                INSERTINGROUP(array_groups[i].kids0, tmpob);
                printf("inserting %s in group 0 of %s\n", tmpob->name,
                        array_groups[i].name);
                tmpob = tmpobnext;
                continue;
            }

            if (tmpob->dist_min < d1 * d1)
            {
                array_groups[i].numkids--;
                array_groups[i].numkids1++;
                INSERTINGROUP(array_groups[i].kids1, tmpob);
                printf("inserting %s in group 1 of %s\n", tmpob->name,
                        array_groups[i].name);
            }
            else if (tmpob->dist_min < d2 * d2)
            {
                array_groups[i].numkids--;
                array_groups[i].numkids2++;
                INSERTINGROUP(array_groups[i].kids2, tmpob);
                printf("inserting %s in group 2 of %s\n", tmpob->name,
                        array_groups[i].name);
            }
            else if (tmpob->dist_min < d3 * d3)
            {
                array_groups[i].numkids--;
                array_groups[i].numkids3++;
                INSERTINGROUP(array_groups[i].kids3, tmpob);
                printf("inserting %s in group 3 of %s\n", tmpob->name,
                        array_groups[i].name);
            }
            else
            {
                printf("cannot insert object %s in group %s\n", tmpob->name,
                        array_groups[i].name);
            }
            /*if (!strnicmp(tmpob->name, "tk",2)){
             tmpob2=tmpob;
             tmpob=tmpob->next;
             continue;
             }*/

            tmpob = tmpobnext;
        }
        if (array_groups[i].numkids == 0)
            array_groups[i].kids = NULL;
        printf("in group %s\n", array_groups[i].name);
        printf("    found in l0  %d\n", array_groups[i].numkids0);
        printf("    found in l1  %d\n", array_groups[i].numkids1);
        printf("    found in l2  %d\n", array_groups[i].numkids2);
        printf("    found in l3  %d\n", array_groups[i].numkids3);
        printf("    staying kids  %d\n", array_groups[i].numkids - 1); /* because of the tkmn not moved */

    }

    /*#ifdef NEWSRC*/
    for (i = 0; i < num_tkmn; i++)
    {
        int red = 0;
        if (array_groups[i].numkids3 > 0)
        {
            red = mergeSplitted(&(array_groups[i].kids3));
            array_groups[i].numkids3 -= red;
        }
        if (array_groups[i].numkids2 > 0)
        {
            red = mergeSplitted(&(array_groups[i].kids2));
            array_groups[i].numkids2 -= red;
        }
        if (array_groups[i].numkids1 > 0)
        {
            red = mergeSplitted(&(array_groups[i].kids1));
            array_groups[i].numkids1 -= red;
        }
    }
    /*#endif*/

    fprintf(stderr, "writing destination file %s\n", OutputFileName);

    if ((ofile = fopen(OutputFileName, "w")) == NULL)
    {
        fprintf(stderr, "failed to open %s\n", OutputFileName);
        return;
    }
    fprintf(ofile, "AC3Db\n");
    printMaterials(ofile, mat0);
    fprintf(ofile, "OBJECT world\n");
    fprintf(ofile, "kids %d\n", num_tkmn);

    for (i = 0; i < num_tkmn; i++)
    {
        int numg = 0;
        fprintf(ofile, "OBJECT group\n");
        fprintf(ofile, "name \"%s_g\"\n", array_groups[i].tkmn->name);
        numg = (array_groups[i].kids3 == 0 ? 0 : 1)
                + (array_groups[i].kids2 == 0 ? 0 : 1)
                + (array_groups[i].kids1 == 0 ? 0 : 1) + 1;
        fprintf(ofile, "kids %d\n", numg);
        /*printOb(array_groups[i].tkmn);*/

        if (array_groups[i].numkids3 > 0)
        {
            fprintf(ofile, "OBJECT group\n");
            fprintf(ofile, "name \"___%s_gl3\"\n", array_groups[i].tkmn->name);
            fprintf(ofile, "kids %d\n", array_groups[i].numkids3);
            printf("writing group: ___%s_gl3\n", array_groups[i].tkmn->name);
            tmpob = array_groups[i].kids3;
            while (tmpob != NULL)
            {

                printOb(ofile, tmpob);
                printf("%s\n", tmpob->name);
                tmpob = tmpob->next;
            }
        }

        if (array_groups[i].numkids2 > 0)
        {
            fprintf(ofile, "OBJECT group\n");
            fprintf(ofile, "name \"%%___%s_gl2\"\n",
                    array_groups[i].tkmn->name);
            fprintf(ofile, "kids %d\n", array_groups[i].numkids2);
            printf("writing group: ___%s_gl2\n", array_groups[i].tkmn->name);
            tmpob = array_groups[i].kids2;
            while (tmpob != NULL)
            {
                printOb(ofile, tmpob);
                printf("%s\n", tmpob->name);
                tmpob = tmpob->next;
            }
        }
        if (array_groups[i].numkids1 > 0)
        {
            fprintf(ofile, "OBJECT group\n");
            fprintf(ofile, "name \"___%s_gl1\"\n", array_groups[i].tkmn->name);
            fprintf(ofile, "kids %d\n", array_groups[i].numkids1);
            printf("writing group: ___%s_gl1\n", array_groups[i].tkmn->name);
            tmpob = array_groups[i].kids1;
            while (tmpob != NULL)
            {
                printOb(ofile, tmpob);
                printf("%s\n", tmpob->name);
                tmpob = tmpob->next;
            }
        }

        /* there is always a group 0 with the tkmn at leat */
        fprintf(ofile, "OBJECT group\n");
        fprintf(ofile, "name \"___%s_gl0\"\n", array_groups[i].tkmn->name);
        fprintf(ofile, "kids %d\n", array_groups[i].numkids0 + 1);
        printf("writing group: ___%s_gl0\n", array_groups[i].tkmn->name);
        tmpob = array_groups[i].kids0;
        while (tmpob != NULL)
        {
            printOb(ofile, tmpob);
            printf("%s\n", tmpob->name);
            tmpob = tmpob->next;
        }
        printOb(ofile, array_groups[i].tkmn);
    }

    return;
}

void reorder(ob_t * ob, ob_t * ob2, uv_t *textarray, tcoord_t *vertexarray)
{
    int k = 0;

    for (int i = 0; i < ob->numvert; i++)
    {
        if (ob->vertex[i] != ob2->vertex[i])
        {
            for (int j = 0; j < ob->numvert; j++)
            {
                if (ob->vertex[i] == ob2->vertex[i])
                {
                    k++;

                    point_t p = ob2->vertex[i];
                    ob2->vertex[i] = ob2->vertex[j];
                    ob2->vertex[j] = p;

                    tcoord_t t = vertexarray[i];
                    vertexarray[i] = vertexarray[j];
                    vertexarray[j] = t;

                    uv_t text = textarray[i];
                    textarray[i] = textarray[j];
                    textarray[j] = text;
                }
            }
        }
    }
    printf("%s : reordered %d points\n", ob->name, k);
    return;
}

/** Returns 0 if the given object has no name or is root, world or a group.
 */
bool isNamedAndPolygon(ob_t * ob)
{
    if (ob->name == NULL)
        return false;
    if (!strcmp(ob->name, "root"))
        return false;
    if (!strcmp(ob->name, "world"))
        return false;
    if (ob->type != NULL && !strcmp(ob->type, "group"))
        return false;

    return true;
}

/** collapse the given tiledob into the texture channel 1 of tarobj */
void collapseMapTiledTextures(ob_t * tarob, ob_t * tiledob);
/** collapse the given skidsob into the texture channel 2 of tarobj */
void collapseSkidsGrassTextures(ob_t * tarob, ob_t * skidsob);
/** collapse the given shadob into the texture channel 3 of tarobj */
void collapseShadowTextures(ob_t * tarob, ob_t * shadob);

/** Match textures from ob1, ob2 and ob3 with ob0. In case a match is found
 *  add them as additional texture channels in ob0.
 */
void collapseTextures(ob_t * ob0, ob_t * ob1, ob_t * ob2, ob_t * ob3)
{
    ob_t * tmpob = NULL;

    tmpob = ob0;
    while (tmpob != NULL)
    {
        if (!isNamedAndPolygon(tmpob))
        {
            tmpob = tmpob->next;
            continue;
        }

        collapseMapTiledTextures(tmpob, ob1);
        collapseSkidsGrassTextures(tmpob, ob2);
        collapseShadowTextures(tmpob, ob3);

        tmpob = tmpob->next;
    }
}

/** copy the texture, textarray and vertexarray properties of srcob
 *  into the corresponding places in destob based on the given channel.
 *  The channel may be 1,2 or 3.
 */
void copyTextureChannel(ob_t * destob, ob_t * srcob, int channel)
{
    char* tex = srcob->texture;
    uv_t* texarr = srcob->textarray;
    tcoord_t* vertarr = srcob->vertexarray;

    if (channel == 1)
    {
        if (tex)
            destob->texture1 = strdup(tex);
        destob->textarray1 = (uv_t*)malloc(sizeof(uv_t) * srcob->numvertice);
        memcpy(destob->textarray1, texarr, sizeof(uv_t) * srcob->numvertice);
        destob->vertexarray1 = (tcoord_t*)malloc(sizeof(tcoord_t) * srcob->numsurf * 3);
        memcpy(destob->vertexarray1, vertarr, sizeof(tcoord_t) * srcob->numsurf * 3);
    }
    else if (channel == 2)
    {
        if (tex)
            destob->texture2 = strdup(tex);
        destob->textarray2 = (uv_t*)malloc(sizeof(uv_t) * srcob->numvertice);
        memcpy(destob->textarray2, texarr, sizeof(uv_t) * srcob->numvertice);
        destob->vertexarray2 = (tcoord_t*)malloc(sizeof(tcoord_t) * srcob->numsurf * 3);
        memcpy(destob->vertexarray2, vertarr, sizeof(tcoord_t) * srcob->numsurf * 3);
    }
    else if (channel == 3)
    {
        if (tex)
            destob->texture3 = strdup(tex);
        destob->textarray3 = (uv_t*)malloc(sizeof(uv_t) * srcob->numvertice);
        memcpy(destob->textarray3, texarr, sizeof(uv_t) * srcob->numvertice);
        destob->vertexarray3 = (tcoord_t*)malloc(sizeof(tcoord_t) * srcob->numsurf * 3);
        memcpy(destob->vertexarray3, vertarr, sizeof(tcoord_t) * srcob->numsurf * 3);
    }
}

void collapseMapTiledTextures(ob_t * tarob, ob_t * tiledob)
{
    ob_t * curtiledob = tiledob;
    bool notinsameorder = false;
    int curvert = 0;

    while (curtiledob != NULL)
    {
        if (!isNamedAndPolygon(curtiledob))
        {
            curtiledob = curtiledob->next;
            continue;
        }
        notinsameorder = false;
        if (!stricmp(curtiledob->name, tarob->name)
        && tarob->numvert == curtiledob->numvert)
        {
            /* found an ob in ob1 */
            copyTextureChannel(tarob, curtiledob, 1);
            for (curvert = 0; curvert < tarob->numvert; curvert++)
            {
                if (fabs(tarob->vertex[curvert].x - curtiledob->vertex[curvert].x) > MINVAL
                || fabs(tarob->vertex[curvert].y - curtiledob->vertex[curvert].y) > MINVAL
                || fabs(tarob->vertex[curvert].z - curtiledob->vertex[curvert].z) > MINVAL)
                {
                    notinsameorder = true;
                }
            }

            if (notinsameorder)
            {
                printf(
                        "%s : points not in the same order, reordering ...\n",
                        tarob->name);
                reorder(tarob, curtiledob, tarob->textarray1,
                        tarob->vertexarray1);
                printf("%s : reordering ... done\n", tarob->name);
            }
            break;
        }
        curtiledob = curtiledob->next;
    }
}

void collapseSkidsGrassTextures(ob_t * tarob, ob_t * skidsob)
{
    ob_t * curskidsob = skidsob;

    while (curskidsob != NULL)
    {
        if (!isNamedAndPolygon(curskidsob))
        {
            curskidsob = curskidsob->next;
            continue;
        }

        if (!stricmp(curskidsob->name, tarob->name)
        && tarob->numvert == curskidsob->numvert)
        {
            /* found an ob in ob2 */
            copyTextureChannel(tarob, curskidsob, 2);
            break;
        }

        curskidsob = curskidsob->next;
    }
}

void collapseShadowTextures(ob_t * tarob, ob_t * shadob)
{
    ob_t * curshadob = shadob;

    while (curshadob != NULL)
    {
        if (!isNamedAndPolygon(curshadob))
        {
            curshadob = curshadob->next;
            continue;
        }

        if (!stricmp(curshadob->name, tarob->name)
        && tarob->numvert == curshadob->numvert)
        {
            /* found an ob in ob2 */
            copyTextureChannel(tarob, curshadob, 3);
            if (tarob->texture3)
            {
                for (int curvert = 0; curvert < tarob->numvert; curvert++)
                {
                    if (tarob->textarray3[curvert] != tarob->textarray[curvert])
                    {
                        printf("name=%s %.2lf!=%.2lf %.2lf!=%.2lf\n",
                                tarob->name, tarob->textarray[curvert].u,
                                tarob->textarray3[curvert].u,
                                tarob->textarray[curvert].v,
                                tarob->textarray3[curvert].v);
                    }
                }
            }

            break;
        }
        curshadob = curshadob->next;
    }
}
