/***************************************************************************

    file                 : main.cpp
    created              : Sat Mar 18 23:54:30 CET 2000
    copyright            : (C) 2000 by Eric Espie
    email                : torcs@free.fr
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

#include <config.h>

#include <cstdlib>
#include <cstring>

#include <tgf.h>
#include <tgfclient.h>

#include <raceengine.h>
#include <legacymenu.h>

#include <client.h>

#include "linuxspec.h"


static void
init_args(int argc, char **argv)
{
	// Initialize run-time detected installation dir.
	GfInitInstallDir(argv[0]);
	
	// Parse command line args.
    const char *localdir = 0;
    const char *libdir = 0;
    const char *datadir = 0;
    const char *bindir = 0;
    int i = 1;
    while (i < argc) 
    {
        // -l option : User settings dir (named "local dir")
        if (!strncmp(argv[i], "-l", 2))
        {
            if (++i < argc)
                localdir = GfSetLocalDir(argv[i]);
        }
        // -L option : Libraries dir (root dir of the tree where loadable modules are installed)
        else if (!strncmp(argv[i], "-L", 2))
        {
            if (++i < argc)
                libdir = GfSetLibDir(argv[i]);
        }
        // -D option : Data dir (root dir of the data tree)
        else if (!strncmp(argv[i], "-D", 2))
        {
            if (++i < argc)
                datadir = GfSetDataDir(argv[i]);
        }
        // -B option : Binaries dir (the dir where game exe and DLLs are installed)
        else if (!strncmp(argv[i], "-B", 2))
        {
            if (++i < argc)
                bindir = GfSetBinDir(argv[i]);
        }
        // -m option : Allow the hardware mouse cursor
        else if (!strncmp(argv[i], "-m", 2))
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
                    GfLogSetFile(argv[i]);
        }

        // Next arg (even if current not recognized).
        i++;
    }

    // If any of the game dirs not run-time specified / empty, 
    // use associated compile-time variable SD_XXDIR to get default value
    if (!(localdir && strlen(localdir)))
        localdir = GfSetLocalDir(SD_LOCALDIR);
    if (!(libdir && strlen(libdir)))
        libdir = GfSetLibDir(SD_LIBDIR);
    if (!(bindir && strlen(bindir)))
        bindir = GfSetBinDir(SD_BINDIR);
    if (!(datadir && strlen(datadir)))
        datadir = GfSetDataDir(SD_DATADIR);

    // Check if ALL the game dirs have a usable value, and exit if not.
    if (!(localdir && strlen(localdir)) || !(libdir && strlen(libdir)) 
        || !(bindir && strlen(bindir)) || !(datadir && strlen(datadir)))
    {
        GfTrace("SD_LOCALDIR : '%s'\n", SD_LOCALDIR);
        GfTrace("SD_LIBDIR   : '%s'\n", SD_LIBDIR);
        GfTrace("SD_BINDIR   : '%s'\n", SD_BINDIR);
        GfTrace("SD_DATADIR  : '%s'\n", SD_DATADIR);
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
 *    LINUX entry point of Speed Dreams
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

    LinuxSpecInit();         /* init specific linux functions */
        
    GfFileSetup();           /* Update user settings files from installed ones */

    GfScrInit(argc, argv);     /* init screen */

	// Give the menu system to the race engine.
	RaceEngine::self().setUserInterface(LegacyMenu::self());

	// Give the race engine to the menu system.
	LegacyMenu::self().setRaceEngine(RaceEngine::self());
	
    if (MenuEntry())         /* launch the game */
    {
        GfelMainLoop();   /* Main event loop */
        exit(0);
    }

    GfLogFatal("Exiting from Speed Dreams for some fatal reason (see above).\n");
    exit(1);                 /* If we got here, something bad happened ... */          
}

