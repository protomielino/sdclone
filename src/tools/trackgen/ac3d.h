/***************************************************************************

    file                 : ac3d.h
    created              : Wed May 29 22:15:37 CEST 2002
    copyright            : (C) 2001 by Eric Espié
    email                : Eric.Espie@torcs.org
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
    		
    @author	<a href=mailto:torcs@free.fr>Eric Espie</a>
    @version	$Id$
*/

#ifndef _AC3D_H_
#define _AC3D_H_

#include <array>
#include <stack>
#include <string>
#include <vector>
#include <list>
#include <fstream>
#include <limits>
#include <map>

struct Ac3d
{
    struct V3d : public std::array<double, 3>
    {
        V3d()
        {
            at(0) = 0;
            at(1) = 0;
            at(2) = 0;
        }
        V3d(double x, double y, double z)
        {
            at(0) = x;
            at(1) = y;
            at(2) = z;
        }

        V3d operator+(const V3d &other) const;
        V3d operator-(const V3d &other) const;
        V3d operator/(double scalar) const;
        double dot(const V3d &other) const;
        V3d cross(const V3d &other) const;
        double length() const;
    };

    struct Color : public std::array<double, 3>
    {
        Color()
        {
            at(0) = 0;
            at(1) = 0;
            at(2) = 0;
        }
        void set(double r, double g, double b)
        {
            at(0) = r;
            at(1) = g;
            at(2) = b;
        }
    };

    class Exception : public std::exception
    {
    private:
        const char *message;

    public:
        explicit Exception(const char *msg) : message(msg)
        {
        }
        const char *what()
        {
            return message;
        }
    };

    struct Material
    {
        std::string	name;
        Color       rgb;
        Color       amb;
        Color       emis;
        Color       spec;
        int         shi = 0;
        double      trans = 0;
        std::string	data;

        Material() = default;
        explicit Material(const std::vector<std::string> &tokens);
        Material(std::ifstream &fin, const std::string &name);
        void write(std::ofstream &fout, bool versionC) const;
        bool same(const Material &material) const;
    };

    struct Surface
    {
        struct Ref
        {
            int                     index = 0;
            std::array<double, 2>   coord{ 0, 0 };

            Ref() = default;
            Ref(int index, double u, double v) : index(index), coord{ u, v } { }
        };

        int                 surf = 0;
        int                 mat = 0;
        std::vector<Ref>    refs;

        Surface() = default;
        explicit Surface(std::ifstream &fin);
        void write(std::ofstream &fout) const;
    };

    class v2 : public std::array<double, 2>
    {
    public:
        bool initialized = false;
    };
    class v3 : public V3d
    {
    public:
        bool initialized = false;
    };
    class v9 : public std::array<double, 9>
    {
    public:
        bool initialized = false;
    };
    template <typename T> struct Value
    {
        T       value = 0;
        bool    initalized = false;

        operator T() const { return value; }
        T operator = (T i) { value = i; return value; }
    };

    class Matrix : public std::array<std::array<double, 4>, 4>
    {
    public:
        Matrix();
        Matrix(double m0, double m1, double m2, double m3,
               double m4, double m5, double m6, double m7,
               double m8, double m9, double m10, double m11,
               double m12, double m13, double m14, double m15);
        void setLocation(const v3 &location);
        void setLocation(double x, double y, double z);
        void setRotation(const std::array<double, 9> &rotation);
        void setRotation(double x, double y, double z);
        void setScale(double scale);
        void setScale(double x, double y, double z);
        void makeIdentity();
        void makeLocation(const v3 &location);
        void makeLocation(double x, double y, double z);
        void makeRotation(const std::array<double, 9> &rotation);
        void makeRotation(double x, double y, double z);
        void makeScale(double scale);
        void makeScale(double x, double y, double z);
        void transformPoint(V3d &point) const;
        void transformNormal(V3d &normal) const;
        Matrix multiply(const Matrix &matrix);
    };

    struct BoundingBox
    {
        V3d min{ std::numeric_limits<double>::max(),
                 std::numeric_limits<double>::max(),
                 std::numeric_limits<double>::max() };
        V3d max{ -std::numeric_limits<double>::max(),
                 -std::numeric_limits<double>::max(),
                 -std::numeric_limits<double>::max() };

        void extend(const V3d &vertex);
        void extend(const BoundingBox &boundingBox);
        bool pointInside(double x, double y) const;
    };

    struct BoundingSphere
    {
        V3d     center{ 0, 0, 0 };
        double  radius = 0;

        void extend(const BoundingBox &boundingBox);
    };

    typedef std::map<size_t, size_t> MaterialMap;

    struct Object
    {
        std::string		        type;
        std::string		        name;
        std::string		        data;
        std::string		        texture;
        v2                      texrep;
        v2                      texoff;
        Value<int>				subdiv;
        Value<double>			crease;
        v9                      rot;
        v3			            loc;
        std::string				url;
        bool				    hidden = false;
        bool				    locked = false;
        bool				    folded = false;
        std::vector<V3d>        vertices;
        std::list<Surface>      surfaces;
        std::list<Object>       kids;

        // not part of AC3D file
        mutable bool            hasBoundingBox = false;
        mutable BoundingBox     boundingBox;
        mutable bool            hasBoundingSphere = false;
        mutable BoundingSphere  boundingSphere;

        Object() = default;
        Object(const std::string &type, const std::string &name) : type(type), name(name) { }
        explicit Object(std::ifstream &fin);
        void parse(std::ifstream &fin, const std::string &objType);
        void write(std::ofstream &fout) const;
        void transform(const Matrix &matrix);
        void flipAxes(bool in);
        const BoundingBox &getBoundingBox() const;
        const BoundingSphere &getBoundingSphere() const;
        void remapMaterials(const MaterialMap &materialMap);
        void generateTriangles();
        void getTerrainHeight(double x, double y, double &terrainHeight, V3d &normal) const;
        bool pointInside(const Surface &surface, double x, double y, double &z, V3d &normal) const;
    };

    bool                    versionC = false;
    std::vector<Material>   materials;
    Object                  root;
    std::stack<Object *>    stack;

    Ac3d();
    void addObject(Object &object);
    void addDefaultMaterial();
    void readFile(const std::string &fileName);
    void writeFile(const std::string &fileName) const;
    void flattenGeometry();
    void transform(const Matrix &matrix);
    void flipAxes(bool in);
    void generateTriangles();
    void merge(const Ac3d &ac3d);
    double getTerrainHeight(double x, double y) const;
    double getTerrainAngle(double x, double y) const;
};

#endif /* _AC3D_H_ */
