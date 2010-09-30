/***************************************************************************

    file                 : main.cpp
    created              : Sat Sep  2 10:40:47 CEST 2000
    copyright            : (C) 2000 by Patrice & Eric Espie
    email                : torcs@free.fr
    version              : $Id: main.cpp,v 1.9 20 Mar 2006 04:24:27 berniw Exp $

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <config.h>

#ifdef WIN32
#include <windows.h>
#include <direct.h>
#include <shlobj.h>
#include <cstdlib>
#endif

#include <tgf.h>
#include <tgfclient.h>
#include <client.h>
#include <portability.h>

#include "windowsspec.h"


static void
init_args(int argc, char **argv)
{
	// Determine and store run-time install root dir.
	GfInitInstallDir(argv[0]);

	// Parse command line args.
    int i = 1;
    const char *localdir = 0;
    const char *libdir = 0;
    const char *datadir = 0;
    const char *bindir = 0;

    while (i < argc)
    {
	// -l or /l option : User settings dir (named "local dir")
	if (!strncmp(argv[i], "-l", 2) || !strncmp(argv[i], "/l", 2))
        {
  	    if (++i < argc)
	        localdir = SetLocalDir(argv[i]);
        }
        // -L or /L option : Libraries dir (root dir of the tree where loadable modules are installed)
        else if (!strncmp(argv[i], "-L", 2) || !strncmp(argv[i], "/L", 2))
        {
	    if (++i < argc)
	        libdir = SetLibDir(argv[i]);
        }
        // -D or /D option : Data dir (root dir of the data tree)
        else if (!strncmp(argv[i], "-D", 2) || !strncmp(argv[i], "/D", 2))
        {
            if (++i < argc)
                datadir = SetDataDir(argv[i]);
        }
        // -s or /s option : Single texture mode (= disable multi-texturing)
        else if (!strncmp(argv[i], "-s", 2) || !strncmp(argv[i], "/s", 2))
        {
            SetSingleTextureMode ();
        }
        // -m or /m option : Allow the hardware mouse cursor
        else if (!strncmp(argv[i], "-m", 2) || !strncmp(argv[i], "/m", 2))
        {
           GfuiMouseSetHWPresent();
        }
        // -t option : Trace level threshold (only #ifdef TRACE_OUT)
        else if (!strncmp(argv[i], "-t", 2))
        {
            int nTraceLevel;
            if (++i < argc && sscanf(argv[i], "%d", &nTraceLevel) == 1)
                GfLogSetLevelThreshold(nTraceLevel);
        }
        // -r option : Trace stream (only #ifdef TRACE_OUT)
        else if (!strncmp(argv[i], "-r", 2))
        {
            if (++i < argc)
                if (!strncmp(argv[i], "stderr", 6))
                    GfLogSetStream(stderr);
                else if (!strncmp(argv[i], "stdout", 6))
                    GfLogSetStream(stdout);
                else
                    GfLogSetStream(fopen(argv[i], "w"));
        }

        // Next arg (even if current not recognized).
        i++;
    }

    // If any of the Speed-Dreams dirs not run-time specified / empty, 
    // use associated compile-time variable SD_XXDIR to get default value
    if (!(localdir && strlen(localdir)))
		localdir = SetLocalDir(SD_LOCALDIR);
	if (!(libdir && strlen(libdir)))
		libdir = SetLibDir(SD_LIBDIR);
    if (!(bindir && strlen(bindir)))
		bindir = SetBinDir(SD_BINDIR);
    if (!(datadir && strlen(datadir)))
		datadir = SetDataDir(SD_DATADIR);
	
    // Check if ALL the Speed-dreams dirs have a usable value, and exit if not.
    if (!(localdir && strlen(localdir)) || !(libdir && strlen(libdir)) 
         || !(bindir && strlen(bindir)) || !(datadir && strlen(datadir)))
    {
        GfTrace("SD_LOCALDIR : '%s'\n", GetLocalDir());
        GfTrace("SD_LIBDIR   : '%s'\n", GetLibDir());
        GfTrace("SD_BINDIR   : '%s'\n", GetBinDir());
        GfTrace("SD_DATADIR  : '%s'\n", GetDataDir());
        GfFatal("Could not start Speed Dreams : at least 1 of local/data/lib/bin dir is empty\n\n");
        exit(1);
    }

    // If "data dir" specified in any way, cd to it.
    if (datadir && strlen(datadir))
         chdir(datadir);
}

/*
 * Function
 *    main
 *
 * Description
 *    Win32 entry point of the game
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
    GfInit();

    init_args(argc, argv);

    WindowsSpecInit();      /* init specific windows functions */

    GfFileSetup();          /* Update user settings files from installed ones */

    GfScrInit(argc, argv);  /* init screen */

    if (MenuEntry())         /* launch the game */
    {
        GfelMainLoop();   /* Main event loop */
        exit(0);
    }
    
    GfLogFatal("Exiting from Speed Dreams for some fatal reason (see above).\n");
    exit(1);                 /* If we got here, something bad happened ... */          
}

