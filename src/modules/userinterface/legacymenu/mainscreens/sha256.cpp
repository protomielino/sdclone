/*
 * Speed Dreams, a free and open source motorsport simulator.
 * Copyright (C) 2024 Xavier Del Campo Romero
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "hash.h"
#include "sha256.h"
#include <tgfclient.h>
#include <openssl/evp.h>
#include <cstddef>
#include <cstdio>
#include <fstream>
#include <stdexcept>
#include <string>

sha256::sha256() :
    c(EVP_MD_CTX_new())
{
    if (!c)
        throw std::runtime_error("EVP_MD_CTX_new failed");
    else if (!(md = EVP_MD_fetch(NULL, "SHA256", NULL)))
        throw std::runtime_error("EVP_MD_fetch failed");
}

sha256::~sha256()
{
    EVP_MD_free(md);
    EVP_MD_CTX_free(c);
}

int sha256::run(const std::string &path, std::string &out)
{
    std::ifstream f(path, std::ios::binary);

    if (!EVP_DigestInit_ex(c, md, NULL))
    {
        GfLogError("EVP_DigestInit_ex failed\n");
        return -1;
    }

    for (;;)
    {
        char buf[BUFSIZ];
        std::streamsize r = f.readsome(buf, sizeof buf);

        if (!r)
            break;
        else if (!f.good())
        {
            GfLogError("Failed to read %zu bytes from %s\n", sizeof buf,
                path.c_str());
            return -1;
        }
        else if (!EVP_DigestUpdate(c, buf, r))
        {
            GfLogError("EVP_DigestUpdate failed\n");
            return -1;
        }
    }

    unsigned n;
    unsigned char result[EVP_MAX_MD_SIZE];

    if (!EVP_DigestFinal_ex(c, result, &n))
    {
        GfLogError("EVP_DigestFinal_ex failed\n");
        return -1;
    }

    for (unsigned i = 0; i < n; i++)
    {
        char s[sizeof "00"];
        int n = snprintf(s, sizeof s, "%02x", result[i]);

        if (n < 0 || n >= static_cast<int>(sizeof s))
        {
            GfLogError("snprintf(3) failed with %d\n", n);
            return -1;
        }

        out += s;
    }

    return 0;
}
