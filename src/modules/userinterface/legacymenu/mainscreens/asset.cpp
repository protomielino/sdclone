/*
 * Speed Dreams, a free and open source motorsport simulator.
 * Copyright (C) 2024 Xavier Del Campo Romero
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "asset.h"
#include <tgf.h>
#include <cjson/cJSON.h>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <string>
#include <vector>

Asset::Asset(const enum type type) :
    type(type)
{
}

int Asset::parse(const std::string &s, unsigned long long &out)
{
    try
    {
        size_t pos;
        unsigned long long sizel = std::stoull(s, &pos);

        if (pos != s.length())
        {
            GfLogError("Invalid number: %s\n", s.c_str());
            return -1;
        }

        out = sizel;
    }
    catch (const std::invalid_argument &e)
    {
        GfLogError("caught std::invalid_argument with %s\n", s.c_str());
        return -1;
    }
    catch (const std::out_of_range &e)
    {
        GfLogError("caught std::out_of_range with %s\n", s.c_str());
        return -1;
    }

    return 0;
}

int Asset::check_dir(const std::string &d) const
{
    static const char s[] = "abcdefghijklmnopqrstuvwxyz"
        "ABCDEFGHIKLMNOPQRSTUVWXYZ"
        "0123456789"
        "-_";

    if (strspn(d.c_str(), s) != d.size())
    {
        GfLogError("\"%s\" contains invalid characters\n", d.c_str());
        return -1;
    }

    return 0;
}

std::string Asset::path() const
{
    switch (type)
    {
        case Asset::car:
            return "cars/models/";

        case Asset::driver:
            return "drivers/";

        case Asset::track:
            return "tracks/" + category + "/";
    }

    return "";
}

int Asset::parse(const cJSON *c)
{
    struct field
    {
        field(const char *key, std::string &ref)
            : key(key), value(NULL), c(NULL) , out(ref) {}
        const char *key, *value;
        const cJSON *c;
        std::string &out;
    };

    std::vector<field> fields;
    std::string revision, size;

    fields.emplace_back("name", name);
    fields.emplace_back("category", category);
    fields.emplace_back("author", author);
    fields.emplace_back("license", license);
    fields.emplace_back("url", url);
    fields.emplace_back("hash", hash);
    fields.emplace_back("hashtype", hashtype);
    fields.emplace_back("thumbnail", thumbnail);
    fields.emplace_back("directory", directory);
    fields.emplace_back("size", size);
    fields.emplace_back("revision", revision);

    for (auto &f : fields)
    {
        if (!(f.c = cJSON_GetObjectItem(c, f.key)))
        {
            GfLogError("Missing field %s\n", f.key);
            return -1;
        }
        else if (!(f.value = cJSON_GetStringValue(f.c)))
        {
            GfLogError("Could not get value for key %s\n", f.key);
            return -1;
        }

        f.out = f.value;
    }

    unsigned long long sizeull;

    if (parse(size, sizeull))
    {
        GfLogError("parse size failed\n");
        return -1;
    }
    else if (sizeull > SIZE_MAX)
    {
        GfLogError("Number exceeds maximum value: %llu > %zu\n", sizeull,
            SIZE_MAX);
        return -1;
    }
    else if (parse(revision, this->revision))
    {
        GfLogError("parse revision failed\n");
        return -1;
    }
    else if (check_dir(directory))
    {
        GfLogError("check_dir directory failed\n");
        return -1;
    }
    else if (type == Asset::track && check_dir(category))
    {
        GfLogError("check_dir category failed\n");
        return -1;
    }

    this->size = sizeull;
    return 0;
}

bool Asset::operator==(const Asset &other) const
{
    return
        name == other.name &&
        category == other.category &&
        url == other.url &&
        author == other.author &&
        license == other.license &&
        hash == other.hash &&
        hashtype == other.hashtype &&
        thumbnail == other.thumbnail &&
        directory == other.directory &&
        size == other.size;
}
