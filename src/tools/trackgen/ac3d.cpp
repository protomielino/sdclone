/***************************************************************************

    file                 : ac3d.cpp
    created              : Wed May 29 22:11:21 CEST 2002
    copyright            : (C) 2001 by Eric Espie
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

#include "ac3d.h"

#include <algorithm>
#include <iterator>
#include <sstream>
#include <iomanip>

Ac3d::Material::Material(const std::vector<std::string> &tokens)
{
    if (tokens.size() != 22)
        throw Exception("Invalid material");

    if (tokens[2] != "rgb" || tokens[6] != "amb" || tokens[10] != "emis" ||
        tokens[14] != "spec" || tokens[18] != "shi" || tokens[20] != "trans")
        throw Exception("Invalid material");

    name = tokens[1];
    rgb[0] = std::stod(tokens[3]);
    rgb[1] = std::stod(tokens[4]);
    rgb[2] = std::stod(tokens[5]);
    amb[0] = std::stod(tokens[7]);
    amb[1] = std::stod(tokens[8]);
    amb[2] = std::stod(tokens[9]);
    emis[0] = std::stod(tokens[11]);
    emis[1] = std::stod(tokens[12]);
    emis[2] = std::stod(tokens[13]);
    spec[0] = std::stod(tokens[15]);
    spec[1] = std::stod(tokens[16]);
    spec[2] = std::stod(tokens[17]);
    shi = std::stoi(tokens[19]);
    trans = std::stod(tokens[21]);
}

Ac3d::Material::Material(std::ifstream &fin, const std::string &name) : name(name)
{
    std::string line;

    while (std::getline(fin, line))
    {
        std::vector<std::string> tokens;
        std::istringstream       iss(line);

        std::copy(std::istream_iterator<std::string>(iss),
                  std::istream_iterator<std::string>(),
                  std::back_inserter(tokens));

        if (tokens.empty())
            continue;
        if (tokens.at(0) == "ENDMAT")
            break;
        if (tokens.at(0) == "rgb")
        {
            rgb[0] = std::stod(tokens.at(1));
            rgb[1] = std::stod(tokens.at(2));
            rgb[2] = std::stod(tokens.at(3));
        }
        else if (tokens.at(0) == "amb")
        {
            amb[0] = std::stod(tokens.at(1));
            amb[1] = std::stod(tokens.at(2));
            amb[2] = std::stod(tokens.at(3));
        }
        else if (tokens.at(0) == "emis")
        {
            emis[0] = std::stod(tokens.at(1));
            emis[1] = std::stod(tokens.at(2));
            emis[2] = std::stod(tokens.at(3));
        }
        else if (tokens.at(0) == "spec")
        {
            spec[0] = std::stod(tokens.at(1));
            spec[1] = std::stod(tokens.at(2));
            spec[2] = std::stod(tokens.at(3));
        }
        else if (tokens.at(0) == "shi")
            shi = std::stoi(tokens.at(1));
        else if (tokens.at(0) == "trans")
            trans = std::stod(tokens.at(1));
        else if (tokens.at(0) == "data")
        {
            const size_t length = std::stoi(tokens.at(1));
            std::string text;
            while (getline(fin, text))
            {
                data += text;
                if (data.size() >= length)
                    break;
            }
        }
        else
            throw Exception("Invalid AC3D file;");
    }
}

void Ac3d::Material::write(std::ofstream &fout, bool versionC) const
{
    if (versionC)
    {
        fout << "MAT " << name << std::endl;
        fout << "rgb " << rgb[0] << " " << rgb[1] << " " << rgb[2] << std::endl;
        fout << "amb " << amb[0] << " " << amb[1] << " " << amb[2] << std::endl;
        fout << "emis " << emis[0] << " " << emis[1] << " " << emis[2] << std::endl;
        fout << "spec " << spec[0] << " " << spec[1] << " " << spec[2] << std::endl;
        fout << "shi " << shi << std::endl;
        fout << "trans " << trans << std::endl;
        fout << "ENDMAT" << std::endl;
    }
    else
    {
        fout << "MATERIAL " << name
             << " rgb " << rgb[0] << " " << rgb[1] << " " << rgb[2]
             << "  amb " << amb[0] << " " << amb[1] << " " << amb[2]
             << "  emis " << emis[0] << " " << emis[1] << " " << emis[2]
             << "  spec " << spec[0] << " " << spec[1] << " " << spec[2]
             << "  shi " << shi << "  trans " << trans << std::endl;
    }
}

bool Ac3d::Material::same(const Material &material) const
{
    return rgb == material.rgb &&
           amb == material.amb &&
           emis == material.emis &&
           spec == material.spec &&
           shi == material.shi &&
           trans == material.trans;
}

Ac3d::Surface::Surface(std::ifstream &fin)
{
    std::string line;

    while (std::getline(fin, line))
    {
        std::vector<std::string> tokens;
        std::istringstream       iss(line);

        std::copy(std::istream_iterator<std::string>(iss),
            std::istream_iterator<std::string>(),
            std::back_inserter(tokens));

        if (tokens.empty())
            continue;
        if (tokens.at(0) == "SURF")
            surf = std::stoi(tokens.at(1), nullptr, 16);
        else if (tokens.at(0) == "mat")
            mat = std::stoi(tokens.at(1));
        else if (tokens.at(0) == "refs")
        {
            const int numRefs = std::stoi(tokens.at(1));
            refs.resize(numRefs);
            for (int i = 0; i < numRefs; )
            {
                tokens.clear();
                std::getline(fin, line);
                if (line.empty())
                    continue;
                std::istringstream iss1(line);

                std::copy(std::istream_iterator<std::string>(iss1),
                          std::istream_iterator<std::string>(),
                          std::back_inserter(tokens));

                refs[i].index = std::stoi(tokens.at(0));
                refs[i].coord[0] = std::stod(tokens.at(1));
                refs[i].coord[1] = std::stod(tokens.at(2));
                i++;
            }
            break;
        }
        else
            throw Exception("Invalid AC3D file");
    }
}

void Ac3d::Surface::write(std::ofstream &fout) const
{
    fout << "SURF 0x" << std::hex << surf << std::dec << std::endl;
    fout << "mat " << mat << std::endl;
    fout << "refs " << refs.size() << std::endl;
    for (const auto &ref : refs)
        fout << ref.index << " " << ref.coord[0] << " " << ref.coord[1] << std::endl;
}

Ac3d::Object::Object(std::ifstream &fin)
{
    std::string line;

    while (std::getline(fin, line))
    {
        std::vector<std::string> tokens;
        std::istringstream       iss(line);

        std::copy(std::istream_iterator<std::string>(iss),
                  std::istream_iterator<std::string>(),
                  std::back_inserter(tokens));

        if (tokens.empty())
            continue;
        if (tokens.at(0) == "OBJECT")
            parse(fin, tokens.at(1));
        else
            throw Exception("Invalid AC3D file");
    }
}

void Ac3d::Object::parse(std::ifstream &fin, const std::string &objType)
{
    type = objType;

    std::string line;

    while (std::getline(fin, line))
    {
        std::vector<std::string> tokens;
        std::istringstream       iss(line);

        std::copy(std::istream_iterator<std::string>(iss),
                  std::istream_iterator<std::string>(),
                  std::back_inserter(tokens));

        if (tokens.empty())
            continue;
        if (tokens.at(0) == "name")
            name = tokens.at(1);
        else if (tokens.at(0) == "data")
        {
            const size_t length = std::stoi(tokens.at(1));
            std::string text;
            while (getline(fin, text))
            {
                data += text;
                if (data.size() >= length)
                    break;
            }
        }
        else if (tokens.at(0) == "texture")
        {
            texture = tokens.at(1);
        }
        else if (tokens.at(0) == "texrep")
        {
            texrep.initialized = true;
            texrep[0] = std::stod(tokens.at(1));
            texrep[1] = std::stod(tokens.at(2));
        }
        else if (tokens.at(0) == "texoff")
        {
            texoff.initialized = true;
            texoff[0] = std::stod(tokens.at(1));
            texoff[1] = std::stod(tokens.at(2));
        }
        else if (tokens.at(0) == "subdiv")
        {
            subdiv.initalized = true;
            subdiv = std::stoi(tokens.at(1));
        }
        else if (tokens.at(0) == "crease")
        {
            crease.initalized = true;
            crease = std::stod(tokens.at(1));
        }
        else if (tokens.at(0) == "rot")
        {
            rot.initialized = true;
            for (size_t i = 0; i < 9; i++)
                rot[i] = std::stod(tokens[i + 1]);
        }
        else if (tokens.at(0) == "loc")
        {
            loc.initialized = true;
            loc[0] = std::stod(tokens.at(1));
            loc[1] = std::stod(tokens.at(2));
            loc[2] = std::stod(tokens.at(3));
        }
        else if (tokens.at(0) == "url")
            url = tokens.at(1);
        else if (tokens.at(0) == "hidden")
            hidden = true;
        else if (tokens.at(0) == "locked")
            locked = true;
        else if (tokens.at(0) == "folded")
            folded = true;
        else if (tokens.at(0) == "numvert")
        {
            const int numvert = std::stoi(tokens.at(1));
            for (int i = 0; i < numvert; )
            {
                tokens.clear();
                std::getline(fin, line);
                if (line.empty())
                    continue;

                std::istringstream       iss1(line);

                std::copy(std::istream_iterator<std::string>(iss1),
                          std::istream_iterator<std::string>(),
                          std::back_inserter(tokens));

                vertices.emplace_back(std::stod(tokens.at(0)), std::stod(tokens.at(1)), std::stod(tokens.at(2)));
                i++;
            }
        }
        else if (tokens.at(0) == "numsurf")
        {
            const int numsurf = std::stoi(tokens.at(1));
            for (int i = 0; i < numsurf; i++)
                surfaces.emplace_back(fin);
        }
        else if (tokens.at(0) == "kids")
        {
            const int numKids = std::stoi(tokens.at(1));
            for (int i = 0; i < numKids; i++)
                kids.emplace_back(fin);
        }
    }
}

void Ac3d::Object::write(std::ofstream &fout) const
{
    fout << "OBJECT " << type << std::endl;
    if (!name.empty())
    {
        if (name[0] != '\"')
            fout << "name \"" << name << "\"" << std::endl;
        else
            fout << "name " << name << std::endl;
    }
    if (!data.empty())
    {
        fout << "data " << data.length() << std::endl;
        fout << data << std::endl;
    }
    if (!texture.empty())
    {
        if (texture[0] != '\"')
            fout << "texture \"" << texture << "\"" << std::endl;
        else
            fout << "texture " << texture << std::endl;
    }
    if (texrep.initialized)
        fout << "texrep " << texrep[0] << " " << texrep[1] << std::endl;
    if (texoff.initialized)
        fout << "texoff " << texoff[0] << " " << texoff[1] << std::endl;
    if (subdiv.initalized)
        fout << "subdiv " << subdiv.value << std::endl;
    if (crease.initalized)
        fout << "crease " << crease.value << std::endl;
    if (rot.initialized)
    {
        fout << "rot "
             << rot[0] << " " << rot[1] << " " << rot[2] << " "
             << rot[3] << " " << rot[4] << " " << rot[5] << " "
             << rot[6] << " " << rot[7] << " " << rot[8] << std::endl;
    }
    if (loc.initialized)
    {
        fout << "loc " << loc[0] << " " << loc[1] << " " << loc[2] << std::endl;
    }
    if (!url.empty())
        fout << "url " << url << std::endl;
    if (hidden)
        fout << "hidden" << std::endl;
    if (locked)
        fout << "locked" << std::endl;
    if (folded)
        fout << "foulded" << std::endl;
    if (!vertices.empty())
    {
        fout << "numvert " << vertices.size() << std::endl;
        for (const auto &vertex : vertices)
            fout << vertex[0] << " " << vertex[1] << " " << vertex[2] << std::endl;
    }
    if (!surfaces.empty())
    {
        fout << "numsurf " << surfaces.size() << std::endl;
        for (const auto &surface : surfaces)
            surface.write(fout);
    }
    fout << "kids " << kids.size() << std::endl;
    for (const auto &kid : kids)
        kid.write(fout);
}

void Ac3d::Object::transform(const Matrix &matrix)
{
    Matrix thisMatrix;

    if (loc.initialized)
    {
        thisMatrix.setLocation(loc);
        loc.initialized = false;
    }
    loc[0] = 0;
    loc[1] = 0;
    loc[2] = 0;

    if (rot.initialized)
    {
        thisMatrix.setRotation(rot);
        rot.initialized = false;
    }

    rot[0] = 1; rot[1] = 0; rot[2] = 0;
    rot[3] = 0; rot[4] = 1; rot[5] = 0;
    rot[6] = 0; rot[7] = 0; rot[8] = 1;

    const Matrix newMatrix = thisMatrix.multiply(matrix);

    if (type == "poly")
    {
        for (auto &vertex : vertices)
            newMatrix.transformPoint(vertex);
    }
    else
    {
        for (auto &kid : kids)
            kid.transform(newMatrix);
    }
}

Ac3d::Matrix::Matrix()
{
    makeIdentity();
}

Ac3d::Matrix::Matrix(double m0, double m1, double m2, double m3,
                     double m4, double m5, double m6, double m7,
                     double m8, double m9, double m10, double m11,
                     double m12, double m13, double m14, double m15)
{
    (*this)[0][0] = m0;  (*this)[0][1] = m1;  (*this)[0][2] = m2;  (*this)[0][3] = m3;
    (*this)[1][0] = m4;  (*this)[1][1] = m5;  (*this)[1][2] = m6;  (*this)[1][3] = m7;
    (*this)[2][0] = m8;  (*this)[2][1] = m9;  (*this)[2][2] = m10; (*this)[2][3] = m11;
    (*this)[3][0] = m12; (*this)[3][1] = m13; (*this)[3][2] = m14; (*this)[3][3] = m15;
}

void Ac3d::Matrix::setLocation(const v3 &location)
{
    (*this)[3][0] = location[0];
    (*this)[3][1] = location[1];
    (*this)[3][2] = location[2];
}

void Ac3d::Matrix::setLocation(double x, double y, double z)
{
    (*this)[3][0] = x;
    (*this)[3][1] = y;
    (*this)[3][2] = z;
}

void Ac3d::Matrix::setRotation(const std::array<double, 9> &rotation)
{
    (*this)[0][0] = rotation[0]; (*this)[0][1] = rotation[1]; (*this)[0][2] = rotation[2];
    (*this)[1][0] = rotation[3]; (*this)[1][1] = rotation[4]; (*this)[1][2] = rotation[5];
    (*this)[2][0] = rotation[6]; (*this)[2][1] = rotation[7]; (*this)[2][2] = rotation[8];
}

const double PI = 3.14159265358979323846;

void Ac3d::Matrix::setRotation(double x, double y, double z)
{
    double cx, sx, cy, sy, cz, sz, szsy, czsy, szcy;

    if (x == 0)
    {
        cx = 1;
        sx = 0;
    }
    else
    {
        const double ax = x * PI / 180.0;
        sx = sin(ax);
        cx = cos(ax);
    }

    if (y == 0)
    {
        cy = 1;
        sy = 0;
    }
    else
    {
        const double ay = y * PI / 180.0;
        sy = sin(ay);
        cy = cos(ay);
    }

    if (z == 0)
    {
        cz = 1;
        sz = 0;
        szsy = 0;
        szcy = 0;
        czsy = sy;
    }
    else
    {
        const double az = z * PI / 180.0;
        sz = sin(az);
        cz = cos(az);
        szsy = sz * sy;
        czsy = cz * sy;
        szcy = sz * cy;
    }

    (*this)[0][0] = cx * cz - sx * szsy;
    (*this)[1][0] = -sx * cy;
    (*this)[2][0] = sz * cx + sx * czsy;
    (*this)[3][0] = 0;

    (*this)[0][1] = cz * sx + szsy * cx;
    (*this)[1][1] = cx * cy;
    (*this)[2][1] = sz * sx - czsy * cx;
    (*this)[3][1] = 0;

    (*this)[0][2] = -szcy;
    (*this)[1][2] = sy;
    (*this)[2][2] = cz * cy;
    (*this)[3][2] = 0;

    (*this)[0][3] = 0;
    (*this)[1][3] = 0;
    (*this)[2][3] = 0;
    (*this)[3][3] = 1;
}

void Ac3d::Matrix::setScale(double scale)
{
    (*this)[0][0] = scale;
    (*this)[1][1] = scale;
    (*this)[2][2] = scale;
    (*this)[3][2] = 1;
}

void Ac3d::Matrix::setScale(double x, double y, double z)
{
    (*this)[0][0] = x;
    (*this)[1][1] = y;
    (*this)[2][2] = z;
    (*this)[3][2] = 1;
}

void Ac3d::Matrix::makeIdentity()
{
    (*this)[0][0] = 1; (*this)[0][1] = 0; (*this)[0][2] = 0; (*this)[0][3] = 0;
    (*this)[1][0] = 0; (*this)[1][1] = 1; (*this)[1][2] = 0; (*this)[1][3] = 0;
    (*this)[2][0] = 0; (*this)[2][1] = 0; (*this)[2][2] = 1; (*this)[2][3] = 0;
    (*this)[3][0] = 0; (*this)[3][1] = 0; (*this)[3][2] = 0; (*this)[3][3] = 1;
}

void Ac3d::Matrix::makeLocation(const v3 &location)
{
    (*this)[0][0] = 1; (*this)[0][1] = 0; (*this)[0][2] = 0; (*this)[0][3] = 0;
    (*this)[1][0] = 0; (*this)[1][1] = 1; (*this)[1][2] = 0; (*this)[1][3] = 0;
    (*this)[2][0] = 0; (*this)[2][1] = 0; (*this)[2][2] = 1; (*this)[2][3] = 0;
    (*this)[3][0] = location[0];
    (*this)[3][1] = location[1];
    (*this)[3][2] = location[2];
    (*this)[3][3] = 1;
}

void Ac3d::Matrix::makeLocation(double x, double y, double z)
{
    (*this)[0][0] = 1; (*this)[0][1] = 0; (*this)[0][2] = 0; (*this)[0][3] = 0;
    (*this)[1][0] = 0; (*this)[1][1] = 1; (*this)[1][2] = 0; (*this)[1][3] = 0;
    (*this)[2][0] = 0; (*this)[2][1] = 0; (*this)[2][2] = 1; (*this)[2][3] = 0;
    (*this)[3][0] = x; (*this)[3][1] = y; (*this)[3][2] = z; (*this)[3][3] = 1;
}

void Ac3d::Matrix::makeRotation(const std::array<double, 9> &rotation)
{
    (*this)[0][0] = rotation[0]; (*this)[0][1] = rotation[1]; (*this)[0][2] = rotation[2]; (*this)[0][3] = 0;
    (*this)[1][0] = rotation[3]; (*this)[1][1] = rotation[4]; (*this)[1][2] = rotation[5]; (*this)[1][3] = 0;
    (*this)[2][0] = rotation[6]; (*this)[2][1] = rotation[7]; (*this)[2][2] = rotation[8]; (*this)[2][3] = 0;
    (*this)[3][0] = 0; (*this)[3][1] = 0; (*this)[3][2] = 0; (*this)[3][3] = 1;

}

void Ac3d::Matrix::makeRotation(double x, double y, double z)
{
    makeIdentity();
    setRotation(x, y, z);
}

void Ac3d::Matrix::makeScale(double scale)
{
    (*this)[0][0] = scale; (*this)[0][1] = 0; (*this)[0][2] = 0; (*this)[0][3] = 0;
    (*this)[1][0] = 0; (*this)[1][1] = scale; (*this)[1][2] = 0; (*this)[1][3] = 0;
    (*this)[2][0] = 0; (*this)[2][1] = 0; (*this)[2][2] = scale; (*this)[2][3] = 0;
    (*this)[3][0] = 0; (*this)[3][1] = 0; (*this)[3][2] = 0; (*this)[3][3] = 1;
}

void Ac3d::Matrix::makeScale(double x, double y, double z)
{
    (*this)[0][0] = x; (*this)[0][1] = 0; (*this)[0][2] = 0; (*this)[0][3] = 0;
    (*this)[1][0] = 0; (*this)[1][1] = y; (*this)[1][2] = 0; (*this)[1][3] = 0;
    (*this)[2][0] = 0; (*this)[2][1] = 0; (*this)[2][2] = z; (*this)[2][3] = 0;
    (*this)[3][0] = 0; (*this)[3][1] = 0; (*this)[3][2] = 0; (*this)[3][3] = 1;
}

void Ac3d::Matrix::transformPoint(v3d &point) const
{
    v3d dst;

    const double t0 = point[0];
    const double t1 = point[1];
    const double t2 = point[2];

    dst[0] = t0 * (*this)[0][0] + t1 * (*this)[1][0] + t2 * (*this)[2][0] + (*this)[3][0];
    dst[1] = t0 * (*this)[0][1] + t1 * (*this)[1][1] + t2 * (*this)[2][1] + (*this)[3][1];
    dst[2] = t0 * (*this)[0][2] + t1 * (*this)[1][2] + t2 * (*this)[2][2] + (*this)[3][2];

    point = dst;
}

void Ac3d::Matrix::transformNormal(v3d &normal) const
{
    v3d dst;

    const double t0 = normal[0];
    const double t1 = normal[1];
    const double t2 = normal[2];

    dst[0] = t0 * (*this)[0][0] + t1 * (*this)[1][0] + t2 * (*this)[2][0];
    dst[1] = t0 * (*this)[0][1] + t1 * (*this)[1][1] + t2 * (*this)[2][1];
    dst[2] = t0 * (*this)[0][2] + t1 * (*this)[1][2] + t2 * (*this)[2][2];

    normal = dst;
}

Ac3d::Matrix Ac3d::Matrix::multiply(const Matrix &matrix)
{
    Matrix result;

    for (int j = 0; j < 4; j++)
    {
        result[0][j] = matrix[0][0] * (*this)[0][j] +
                       matrix[0][1] * (*this)[1][j] +
                       matrix[0][2] * (*this)[2][j] +
                       matrix[0][3] * (*this)[3][j];

        result[1][j] = matrix[1][0] * (*this)[0][j] +
                       matrix[1][1] * (*this)[1][j] +
                       matrix[1][2] * (*this)[2][j] +
                       matrix[1][3] * (*this)[3][j];

        result[2][j] = matrix[2][0] * (*this)[0][j] +
                       matrix[2][1] * (*this)[1][j] +
                       matrix[2][2] * (*this)[2][j] +
                       matrix[2][3] * (*this)[3][j];

        result[3][j] = matrix[3][0] * (*this)[0][j] +
                       matrix[3][1] * (*this)[1][j] +
                       matrix[3][2] * (*this)[2][j] +
                       matrix[3][3] * (*this)[3][j];
    }

    return result;
}

Ac3d::Ac3d()
{
    root.type = "world";
    stack.push(&root);
}

void Ac3d::addObject(Object &object)
{
    stack.top()->kids.push_back(object);
    stack.push(&stack.top()->kids.back());
}

void Ac3d::addDefaultMaterial()
{
    Ac3d::Material mat;

    mat.name = "\"\"";
    mat.rgb.set(0.4, 0.4, 0.4);
    mat.amb.set(0.8, 0.8, 0.8);
    mat.emis.set(0.4, 0.4, 0.4);
    mat.spec.set(0.5, 0.5, 0.5);
    mat.shi = 50;
    mat.trans = 0;

    materials.push_back(mat);
}

void Ac3d::readFile(const std::string &fileName)
{
    std::ifstream   fin(fileName);

    if (!fin)
        throw Exception("Couldn't open file");

    std::string line;

    if (!std::getline(fin, line))
        throw Exception("Couldn't read file");

    if (line == "AC3Db")
        versionC = false;
    else if (line == "AC3Dc")
        versionC = true;
    else
        throw Exception("Not AC3D file");
    try
    {
        while (std::getline(fin, line))
        {
            std::vector<std::string> tokens;
            std::istringstream       iss(line);

            std::copy(std::istream_iterator<std::string>(iss),
                      std::istream_iterator<std::string>(),
                      std::back_inserter(tokens));

            if (tokens.empty())
                continue;
            if (tokens.at(0) == "MATERIAL")
                materials.emplace_back(tokens);
            else if (tokens.at(0) == "MAT" && versionC)
                materials.emplace_back(fin, tokens.at(1));
            else if (tokens.at(0) == "OBJECT")
                root.parse(fin, tokens.at(1));
            else
                throw Exception("Invalid AC3D file;");
        }
    }
    catch (std::out_of_range &)
    {
        throw Exception("Invalid AC3D file;");
    }
}

void Ac3d::writeFile(const std::string &fileName) const
{
    std::ofstream   fout(fileName);

    if (!fout)
        throw Exception("Couldn't open file");

    if (versionC)
        fout << "AC3Dc" << std::endl;
    else
        fout << "AC3Db" << std::endl;

    for (const auto &material : materials)
        material.write(fout, versionC);

    root.write(fout);
}

void Ac3d::transform(const Matrix &matrix)
{
    root.transform(matrix);
}

void Ac3d::flattenGeometry()
{
    const Matrix matrix;

    transform(matrix);
}
