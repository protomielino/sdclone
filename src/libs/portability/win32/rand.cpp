/*
 * Speed Dreams, a free and open source motorsport simulator.
 * Copyright (C) 2024 Xavier Del Campo Romero
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "portability.h"
#include <windows.h>
#include <wincrypt.h>
#include <cstddef>
#include <cstdio>

static int ensure(HCRYPTPROV *prov)
{
    if (!CryptAcquireContextA(prov, NULL, NULL, PROV_RSA_FULL, 0)
        && !CryptAcquireContextA(prov, NULL, NULL, PROV_RSA_FULL,
            CRYPT_NEWKEYSET))
    {
        fprintf(stderr, "CryptAcquireContextA failed with %#x\n",
            GetLastError());
        return -1;
    }

    return 0;
}

int portability::rand(void *buf, size_t n)
{
    int ret = -1;
    HCRYPTPROV prov;

    if (ensure(&prov))
    {
        fprintf(stderr, "ensure failed\n");
        return -1;
    }
    else if (!CryptGenRandom(prov, n, buf))
    {
        fprintf(stderr, "CryptGenRandom failed with %#x\n", GetLastError());
        goto end;
    }

    ret = 0;

end:

    if (!CryptReleaseContext(prov, 0))
    {
        fprintf(stderr, "CryptReleaseContext failed with %#x\n",
            GetLastError());
        ret = -1;
    }

    return ret;
}
