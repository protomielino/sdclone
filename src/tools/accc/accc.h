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

extern char * fileL0;
extern char * fileL1;
extern char * fileL2;
extern char * fileL3;
extern int d1;
extern int d2;
extern int d3;
extern bool extendedStrips;
extern bool extendedTriangles;
extern bool extendedEnvCoord;
extern bool notexturesplit;
extern bool isobjectacar;
extern bool normalMapping;
extern char *OrderString;
extern bool collapseObject;

extern void loadAndGroup(const char *OutputFileName);
/** Loads the file with inputFilename to the global root_ob and root_material
 *  variables and optionally outputs the loaded object to outputFilename
 *  based on the current value of the global typeConvertion variable.
 *
 *  @returns 0 on success, a value != 0 on failure
 */
int loadAC(const char * inputFilename, const char * outputFilename = NULL);

enum conv_t {
    _UNSPECIFIED,
    _AC3DTO3DS,
    _3DSTOAC3D,
    /** optimized version of ac3d using groups by section */
    _AC3DTOAC3D,
    _AC3DTOOBJ,
    _AC3DTOAC3DM,
    _AC3DTOAC3DS,
    _AC3DTOAC3DGROUP
};

struct point_t
{
    double x;
    double y;
    double z;

    point_t() { } // = default;
    point_t(double _x, double _y, double _z) : x(_x), y(_y), z(_z) { }
    void set(double _x, double _y, double _z)
    {
        x = _x;
        y = _y;
        z = _z;
    }
    bool operator == (const point_t &rhs) const
    {
        return x == rhs.x && y == rhs.y && z == rhs.z;
    }
    bool operator != (const point_t &rhs) const
    {
        return !(*this == rhs);
    }
    point_t operator + (const point_t &rhs)
    {
        return point_t(x + rhs.x, y + rhs.y, z + rhs.z);
    }
    point_t operator - (const point_t &rhs)
    {
        return point_t(x - rhs.x, y - rhs.y, z - rhs.z);
    }
    point_t &operator += (const point_t &rhs)
    {
        x += rhs.x;
        y += rhs.y;
        z += rhs.z;
        return *this;
    }
    point_t &operator *= (const point_t &rhs)
    {
        x *= rhs.x;
        y *= rhs.y;
        z *= rhs.z;
        return *this;
    }
    point_t &operator *= (double rhs)
    {
        x *= rhs;
        y *= rhs;
        z *= rhs;
        return *this;
    }
    point_t & operator /= (double rhs)
    {
        x /= rhs;
        y /= rhs;
        z /= rhs;
        return *this;
    }
};

struct tcoord_t
{
    int indice;
    double u;
    double v;
    bool saved;

    void set(int _indice, double _u, double _v, bool _saved)
    {
        indice = _indice;
        u = _u;
        v = _v;
        saved = _saved;
    }
};

struct ob_t
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
    int dataSize;
    char * data;
    double texrep_x;
    double texrep_y;
    int numvert;
    int numsurf;
    int numvertice; /* the real number of vertices */
    /* the actual points, to which the entries in the vertexarray point to
     * size: numvertice
     */
    point_t * vertex;
    /* the normals corresponding to entries in the above "vertex" array
     * size: numvertice
     */
    point_t * norm;
    /* the smoothed normals corresponding to entries in the above "vertex" array
     * size: numvertice
     */
    point_t * snorm;
    /* array of indices into the "vertex" array, that make up surfaces. In AC3D: one line in
     * "refs" section
     * size: numsurf * 3
     */
    tcoord_t * vertexarray;
    tcoord_t * vertexarray1;
    tcoord_t * vertexarray2;
    tcoord_t * vertexarray3;
    int * va;
    /* Holds the texture coordinates of the vertices stored in "vertex" array
     * size: numvertice * 2
     */
    double * textarray;
    double * textarray1;
    double * textarray2;
    double * textarray3;
    int * surfrefs;
    ob_t * next;
    double x_min;
    double y_min;
    double z_min;
    double x_max;
    double y_max;
    double z_max;
    double dist_min;
    ob_t* ob1;
    ob_t* ob2;
    ob_t* ob3;
    bool saved;
    int kids_o;
    bool inkids_o;
};

/** Creates an instance of the ob_t struct and zeroes it.
 */
ob_t * obCreate();

/** Free the given object. This will only free the resources of the
 * object itself. It will not touch the object->next.
 */
void obFree(ob_t *o);

/** Appends srcob to the end of the list given by destob.
 *
 * @return the new list: destob if it exists, else srcob itself.
 */
ob_t * obAppend(ob_t * destob, ob_t * srcob);

/** Initializes the min and max properties of the given object
 *  must be set: numvertice, vertex
 */
void obInitSpacialExtend(ob_t * ob);

/** Creates and zeroes the "vertexarray" properties.
 *  must be set: "texture" properties (decide whether to create arrays), numsurf
 */
void obCreateVertexArrays(ob_t * ob);

/** Creates and initializes ob's textarray properties, based on the "vertexarray" data
 *  must be set: numsurf, numvertice, vertexarray
 */
void obCreateTextArrays(ob_t * ob);

/** copies the "texture" properties from srcob to destob. */
void obCopyTextureNames(ob_t * destob, const ob_t * srcob);

/** Assigns the given "newIndex" to the indice property of all active "vertexarray"s at index
 *  "vaIdx".
 */
void obSetVertexArraysIndex(ob_t * ob, int vaIdx, int newIndex);

struct ob_groups_t
{
    ob_t * kids;
    int numkids;
    ob_t * tkmn;
    char * name;
    int tkmnlabel;
    ob_t * kids0;
    int numkids0;
    ob_t * kids1;
    int numkids1;
    ob_t * kids2;
    int numkids2;
    ob_t * kids3;
    int numkids3;
};

struct color_t
{
    double r;
    double g;
    double b;
};

struct mat_t
{
    char * name;
    color_t rgb;
    color_t amb;
    color_t emis;
    color_t spec;
    double shi;
    double trans;
    mat_t * next;
};

/** Copies a single surface from the "vertexarray" attributes of srcob to the ones of destob.
 *  It decides whether to copy multitexture data based on srcob's "vertexarray" attributes.
 *
 *  In particular it copies 3 entries starting at srcSurfIdx * 3 from srcob->vertexarray
 *  to entries starting at destSurfIdx * 3 in destob->vertexarray. The same goes for the
 *  multitexture entries.
 */
void copyVertexArraysSurface(ob_t * destob, int destSurfIdx, ob_t * srcob, int srcSurfIdx);

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

/** Creates all texture channels in destob based on the texture channels present in srcob.
 *  In particular, srcob's vertexarray properties determine whether a texture channel is present
 *  and from numsurf and numvertice the number of points/indices are calculated.
 *  Creation means allocation and copying of the corresponding channels from srcob to destob.
 */
void createTexChannelArrays(ob_t * destob, const ob_t * srcob);

/** Creates vertexarray{,1,2,3} and textarray{,1,2,3} in ob based on the given channel
 *  and on the given number of vertices. Creation means allocation and copying of the
 *  corresponding channels from srcob to destob.
 *
 *  @param ob the object in which to allocate the texture channel
 *  @param channel which texture channel, value in range [0,3]
 */
void createSingleTexChannelArrays(ob_t * destob, const ob_t * srcob, int channel);

/** Computes the centroid of a triangle surface of the given object.
 *
 *  @param ob the object from which to take the surface
 *  @param obsurf the surface index in the object (obsurf * 3 is the first entry in the vertexarray considered)
 *  @param out the computed centroid
 */
void computeObSurfCentroid(const ob_t * ob, int obsurf, point_t * out);

extern conv_t typeConvertion;
extern ob_t * root_ob;

/** Splits the given object and returns the split objects.
 *
 * The original object will not be modified.
 */
ob_t* splitOb(ob_t *object);

/** Performs a terrain split on the given object and returns the split objects.
 *
 * The original object will not be modified.
 */
ob_t* terrainSplitOb(ob_t *object);

extern int mergeSplitted(ob_t **object);
extern int distSplit;

/** Whether to split objects during loading, i.e. calls to loadAC().
 *  The default behavior is to split them during loading
 *  (splitObjectsDuringLoad != 0). However, in loadAndGroup()
 *  we want to manually trigger the object splitting, i.e. just after setting up the
 *  texture channels. In that case the object splitting has to be skipped in loading
 *  (splitObjectsDuringLoad == 0).
 */
extern bool splitObjectsDuringLoad;

/** Go through all given objects, check whether a normal split or a terrain
 *  split is necessary and execute the split.
 */
ob_t * splitObjects(ob_t* object);
double findDistmin(ob_t * ob1, ob_t *ob2);

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

