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
#endif

#include <portability.h>

#ifdef WIN32
#include "windowsspec.h"
#else
#include "linuxspec.h"
#endif

#include <client.h>

#include "mainmenu.h"
#include "splash.h"
#include "previewmenu.h"

std::string g_strMenuFile;

static void
init_args(int argc, char **argv)
{
    const char *localdir = 0;
    const char *libdir = 0;
    const char *datadir = 0;
    const char *bindir = 0;

	// Determine and store run-time install root dir.
	GfInitInstallDir(argv[0]);

	// Parse command line args.
#ifdef WIN32

    int i = 1;
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
        // -B or /B option : Binaries dir (the dir where Speed Dreams exe and DLLs are installed)
        else if (!strncmp(argv[i], "-B", 2) || !strncmp(argv[i], "/B", 2))
        {
            if (++i < argc)
                bindir = SetBinDir(argv[i]);
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
		else
		{
			g_strMenuFile = argv[i];
		}
		
        // Next arg (even if current not recognized).
        i++;
    }

#else

    int i = 1;
	while (i < argc) 
	{
		// -l option : User settings dir (named "local dir")
		if (!strncmp(argv[i], "-l", 2))
		{
			if (++i < argc)
				localdir = SetLocalDir(argv[i]);
		}
		// -L option : Libraries dir (root dir of the tree where loadable modules are installed)
		else if (!strncmp(argv[i], "-L", 2))
		{
			if (++i < argc)
				libdir = SetLibDir(argv[i]);
		}
		// -D option : Data dir (root dir of the data tree)
		else if (!strncmp(argv[i], "-D", 2))
		{
			if (++i < argc)
				datadir = SetDataDir(argv[i]);
		}
		// -B option : Binaries dir (the dir where game exe and DLLs are installed)
		else if (!strncmp(argv[i], "-B", 2))
		{
			if (++i < argc)
				bindir = SetBinDir(argv[i]);
		}
		// -s option : Single texture mode (= disable multi-texturing)
		else if (!strncmp(argv[i], "-s", 2))
		{
			SetSingleTextureMode ();
		}
		// -m option : Allow the hardware mouse cursor
		else if (!strncmp(argv[i], "-m", 2))
		{
			GfuiMouseSetHWPresent();
		}
		else
		{
			g_strMenuFile = argv[i];
		}

		// Next arg (even if current not recognized).
		i++;
	}
#endif
	
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

void ShowMenu(const char *pMenuFile)
{
	GfInitClient();
	PreviewMenuInit(pMenuFile);
	PreviewMenuRun();
}

int
main(int argc, char *argv[])
{
    //GfInit(); Not useful here (apart if we want the trace system)

    init_args(argc, argv);

    GfFileSetup();          /* Update user settings files from an old version */

#ifdef WIN32
    WindowsSpecInit();      /* init specific windows functions */
#else
	LinuxSpecInit();
#endif

    GfScrInit(argc, argv);  /* init screen */

	if (g_strMenuFile == "")
	{
		printf("No menu file specified\nUSAGE\nsd-menuview menufile.xml\n");
		return 0;
	}

	ShowMenu(g_strMenuFile.c_str());

    GfelMainLoop();          /* event loop */

    return 0;	            /* just for the compiler, never reached */
}

