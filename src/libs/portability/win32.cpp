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
#include <winerror.h>
#include <shellapi.h>
#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static char *getdir(const char *path)
{
    /* SHFileOperation requires a double null-terminated string. */
    size_t sz = strlen(path) + sizeof (const char[]){'\0', '\0'};
    char *ret = malloc(sz);

    if (!ret)
    {
        fprintf(stderr, "%s: malloc(3): %s\n", __func__, strerror(errno));
        goto failure;
    }

    strcpy(ret, path);
    ret[sz - 1] = '\0';
    return ret;

failure:
    free(ret);
    return NULL;
}

int portability::rmdir_r(const char *path)
{
    int ret = -1;
    char *dir = dir = getdir(path);

    if (!dir)
    {
        fprintf(stderr, "%s: getdir failed\n", __func__);
        goto end;
    }

    SHFILEOPSTRUCT op = {
        .wFunc = FO_DELETE,
        .pFrom = dir,
        .fFlags = FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_SILENT,
        .lpszProgressTitle = ""
    };

    int res = SHFileOperation(&op);

    if (res)
    {
        fprintf(stderr, "%s: SHFileOperation failed with %#x\n", __func__, res);
        goto end;
    }
    else if (op.fAnyOperationsAborted)
    {
        fprintf(stderr, "%s: operation aborted\n", __func__);
        goto end;
    }

    ret = 0;

end:
    free(dir);
    return ret;
}
