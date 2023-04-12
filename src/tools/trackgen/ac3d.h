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
#include <fstream>
#include <v3_t.h>

typedef v3t<double> v3d;

struct Ac3d
{
    typedef std::array<double, 3> Color;

    class Exception : public std::exception
    {
    private:
        const char *message;

    public:
        Exception(const char *msg) : message(msg)
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
        int         shi;
        double      trans;
        std::string	data;

        Material() = default;

        Material(const std::vector<std::string> &tokens);
        Material(std::ifstream &fin, const std::string &name);
        void write(std::ofstream &fout, bool versionC) const;
    };

    struct Surface
    {
        struct Ref
        {
            int                     index;
            std::array<double,2>    coord;

            Ref() = default;
            Ref(int index, double u, double v) : index(index), coord{ u, v } { }
        };

        int                 surf = 0;
        int                 mat = 0;
        std::vector<Ref>    refs;

        Surface() = default;
        Surface(std::ifstream &fin);
        void write(std::ofstream &fout) const;
    };

    class v2 : public std::array<double, 2>
    {
    public:
        bool initialized = false;
    };
    class v3 : public std::array<double, 3>
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
        std::vector<v3d>        vertices;
        std::vector<Surface>    surfaces;
        std::vector<Object>     kids;

        Object() = default;

        Object(const std::string &type, const std::string &name) : type(type), name(name)
        {
        }

        Object(std::ifstream &fin);
        void parse(std::ifstream &fin, const std::string &objType);
        void write(std::ofstream &fout) const;
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
};

#endif /* _AC3D_H_ */
