/***************************************************************************

    file                 : main.cpp
    created              : Sat Sep  2 10:40:47 CEST 2000
    copyright            : (C) 2000 by Patrice & Eric Espie
    email                : torcs@free.fr
    version              : $Id: main.cpp,v 1.9 2005/09/19 19:02:22 berniw Exp $

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifdef WIN32
#include <windows.h>
#include <shlobj.h>
#include <stdlib.h>
#endif
#include <GL/glut.h>
#include <tgfclient.h>
#include <client.h>
#include <portability.h>

#include "windowsspec.h"

static void
init_args(int argc, char **argv)
{
    int i;
    i = 1;
    while (i < argc) {
	if (!strncmp(argv[i], "-s", 2) || !strncmp(argv[i], "/s", 2)) {
	    i++;
	    SetSingleTextureMode ();
	} else
	    i++;	// Ignore bad args
    }

    static const int BUFSIZE = MAX_PATH;
    char buf[BUFSIZE];
    strncpy(buf, argv[0], BUFSIZE);
    buf[BUFSIZE-1] = '\0';    // Guarantee zero termination for next operation.
    char *end = strrchr(buf, '\\');

    // Did we find the last '\' and do we get a complete path?
    if (end && buf[1] == ':') {
	end++;
	*(end) = '\0';
	// replace '\' with '/'
	for (i = 0; i < BUFSIZE && buf[i] != '\0'; i++) {
	    if (buf[i] == '\\')
		buf[i] = '/';
	}

	SetDataDir(buf);
	SetLibDir("");
    } else {
	if (_fullpath(buf, argv[0], BUFSIZE) &&
	    (strcmp(argv[0], "wtorcs") == 0 ||
	     strcmp(argv[0], "wtorcs.exe") == 0)
	   )
	{
	    end = strrchr(buf, '\\');
	    end++;
	    *(end) = '\0';
	    // replace '\' with '/'
	    for (i = 0; i < BUFSIZE && buf[i] != '\0'; i++) {
		if (buf[i] == '\\')
		    buf[i] = '/';
	    }
	    SetDataDir(buf);
	    SetLibDir("");
	} else {
	    printf("Run wtorcs.exe either from the GUI or from the directory which contains wtorcs.exe\n");
	    exit(1);
	}
    }
    
    // Set LocalDir to the user settings dir for Torcs-NG 
    // (in My documents, to give access to the user for advanced settings).
    //snprintf(buf, BUFSIZE, "%s", getenv("USERPROFILE"));
//#if (WINVER < 0x0600)
//    if (SHGetSpecialFolderPath(0, buf, CSIDL_PERSONAL, 0))
//#else
//  How can we do that under Vista / 7 as CSIDL_PERSONAL is a virtual folder
//  and SHGetSpecialFolderPath is told to fail in this case ?
//#endif
    LPITEMIDLIST pidl;
    if (SUCCEEDED(SHGetSpecialFolderLocation(NULL, CSIDL_PERSONAL, &pidl))
	&& SHGetPathFromIDList(pidl, buf))
    {
	for (i = 0; i < BUFSIZE && buf[i]; i++)
	    if (buf[i] == '\\')
		buf[i] = '/';
	if (buf[0] && buf[strlen(buf)-1] != '/')
	    strcat(buf, "/");
	strcat(buf, ".torcs-ng/");
	SetLocalDir(buf);
    }
    else
    {
	printf("Could not get user's My documents folder path\n");
	exit(1);
    }
}

/*
 * Function
 *    main
 *
 * Description
 *    Win32 entry point of TORCS
 *
 * Parameters
 *
 *
 * Return
 *
 *
 * Remarks
 *
 */
int
main(int argc, char *argv[])
{
    init_args(argc, argv);

    WindowsSpecInit();	     /* init specific windows functions */

    GfScrInit(argc, argv);   /* init screen */

    TorcsEntry();	     /* launch TORCS */

    glutMainLoop();	     /* event loop of glut */

    return 0;	             /* just for the compiler, never reached */
}

