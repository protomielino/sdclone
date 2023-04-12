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
            emis[0] = std::stod(tokens.at(1));;
            emis[1] = std::stod(tokens.at(2));;
            emis[2] = std::stod(tokens.at(3));;
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
            surf = std::stoi(tokens.at(1));
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
            fout << vertex.x << " " << vertex.y << " " << vertex.z << std::endl;
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
    mat.rgb = { 0.4, 0.4, 0.4 };
    mat.amb = { 0.8, 0.8, 0.8 };
    mat.emis = { 0.4, 0.4, 0.4 };
    mat.spec = { 0.5, 0.5, 0.5 };
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
