/***************************************************************************

 file        : accc.h
 created     : Fri Apr 18 23:09:53 CEST 2003
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

#ifndef _ACCC_H_
#define _ACCC_H_

#define FALSE 0
#define TRUE 1

extern char *OutputFileName;
extern char *ReliefFileName;
extern char * fileL0;
extern char * fileL1;
extern char * fileL2;
extern char * fileL3;
extern int d1;
extern int d2;
extern int d3;
extern int useStrip;
extern int extendedStrips;
extern int extendedTriangles;
extern int extendedEnvCoord;
extern int notexturesplit;
extern int isobjectacar;
extern int normalMapping;
extern char *OrderString;
extern int collapseObject;

extern void loadAndGroup(char *OutputFileName);
extern int loadAC(char * inputFilename, char * outputFilename, int saveIn);

#define _AC3DTO3DS 1
#define _3DSTOAC3D 2
/** optimized version of ac3d using groups by section */
#define _AC3DTOAC3D 3
#define _AC3DTOOBJ 4
#define _AC3DTOAC3DM 5
#define _AC3DTOAC3DS 6
#define _AC3DTOAC3DGROUP 7

typedef struct point
{
    double x;
    double y;
    double z;
} point_t;

void copyPoint(point_t * dest, point_t * src);

typedef struct tcoord
{
    int indice;
    double u;
    double v;
    int saved;
} tcoord_t;

void copyTexCoord(tcoord_t * dest, tcoord_t * src);
void storeTexCoord(tcoord_t * dest, int indice, double u, double v, int saved);

typedef struct ob
{
    char * name;
    char * type;
    int kids;
    point_t loc;
    int attrSurf;
    int attrMat;
    char * texture;
    char * texture1;
    char * texture2;
    char * texture3;
    char * data;
    double texrep_x;
    double texrep_y;
    int numvert;
    int numsurf;
    int numvertice; /* the real number of vertices */
    point_t * vertex;
    point_t * norm;
    point_t * snorm;
    tcoord_t * vertexarray;     /* array of indices that make up surfaces. in AC3D: one ref line */
    tcoord_t * vertexarray1;
    tcoord_t * vertexarray2;
    tcoord_t * vertexarray3;
    int * va;
    double * textarray;     /* subset of vertexarray: contains only the texture coords */
    double * textarray1;
    double * textarray2;
    double * textarray3;
    int * surfrefs;
    struct ob * next;
    double x_min;
    double y_min;
    double z_min;
    double x_max;
    double y_max;
    double z_max;
    double dist_min;
    struct ob* ob1;
    struct ob* ob2;
    struct ob* ob3;
    int saved;
    int kids_o;
    int inkids_o;
} ob_t;

typedef struct ob_groups
{
    struct ob * kids;
    int numkids;
    struct ob * tkmn;
    char * name;
    int tkmnlabel;
    struct ob * kids0;
    int numkids0;
    struct ob * kids1;
    int numkids1;
    struct ob * kids2;
    int numkids2;
    struct ob * kids3;
    int numkids3;

} ob_groups_t;

typedef struct col
{
    double r;
    double g;
    double b;
} color_t;

typedef struct mat
{
    char * name;
    color_t rgb;
    color_t amb;
    color_t emis;
    color_t spec;
    int shi;
    double trans;
    struct mat * next;
} mat_t;

/** Helper function for copySingleVertexData(). Stores a single texture channel, i.e. copies
 *  data from the srcvert into the destination arrays based on the given indices.
 *  @sa copySingleVertexData()
 */
void copyTexChannel(double * desttextarray, tcoord_t * destvertexarray, tcoord_t * srcvert,
    int storedptidx, int destptidx, int destvertidx);

/** Copies the data of a single vertex from srcob to destob.
 *  In particular the vertexarray and textarray variables of destob will be modified.
 *  This includes the data of additional texture channels.
 *
 *  This function is used in splitting specifically.
 *
 *  @param destob the destination object
 *  @param srcob the source object
 *  @param storedptidx the value of the indice variable in the destination vertex
 *  @param destptidx the index in the "vertex" array to be used for modifying the textarray
 *  @param destvertidx the index in the destination's vertexarray to be modified
 *  @param srcvertidx the index in the vertexarray in the source object to take the data from
 */
void copySingleVertexData(ob_t * destob, ob_t * srcob,
    int storedptidx, int destptidx, int destvertidx, int srcvertidx);

/** Clears the saved flag for a single entry in ob's vertexarray and does so
 *  for all texture channels.
 */
void clearSavedInVertexArrayEntry(ob_t * ob, int vertidx);

/** Allocates all texture channels in destob based on the texture channels present in srcob.
 *  In particular, srcob's vertexarray properties determine whether a texture channel is present
 *  and from numsurf and numvertice the number of points/indices are calculated.
 */
void allocTexChannelArrays(ob_t * destob, ob_t * srcob);

/** Creates vertexarray{,1,2,3} and textarray{,1,2,3} in ob based on the given channel
 *  and on the given number of vertices.
 *
 *  @param ob the object in which to allocate the texture channel
 *  @param channel which texture channel, value in range [0,3]
 *  @param numpt number of points, in ob_t corresponds to numvertice
 *  @param numvert number of index points, in ob_t corresponds to numsurf*3
 */
void allocSingleTexChannelArrays(ob_t * ob, int channel, int numpt, int numvert);

extern int typeConvertion;
extern ob_t * root_ob;

extern int terrainSplitOb(ob_t **object);
extern int mergeSplitted(ob_t **object);
extern int distSplit;

/** Whether to split objects during loading, i.e. calls to loadAC() and loadACo().
 *  The default behavior is to split them during loading
 *  (splitObjectsDuringLoad != 0). However, in loadAndGroup()
 *  we want to manually trigger the object splitting, i.e. just after setting up the
 *  texture channels. In that case the object splitting has to be skipped in loading
 *  (splitObjectsDuringLoad == 0).
 */
extern int splitObjectsDuringLoad;

/** Go through all given objects, check whether a normal split or a terrain
 *  split is necessary and execute the split.
 */
void splitObjects(ob_t** object);

extern int freeobject(ob_t *o);
double findDistmin(ob_t * ob1, ob_t *ob2);

#define freez(x) {if ((x)) free((x)); }
#define SPLITX 75
#define SPLITY 75
#define MINVAL 0.001

#ifndef WIN32
#define stricmp strcasecmp
#define strnicmp strncasecmp
#else
#include <windows.h>
#include <float.h>
#endif


#endif /* _ACCC_H_ */ 

