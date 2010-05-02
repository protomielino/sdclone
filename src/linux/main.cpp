/***************************************************************************

    file                 : main.cpp
    created              : Sat Mar 18 23:54:30 CET 2000
    copyright            : (C) 2000 by Eric Espie
    email                : torcs@free.fr
    version              : $Id: main.cpp,v 1.14 2006 20:50:12 berniw Exp $

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

#include <stdlib.h>
#include <cstring>

#include <tgfclient.h>
#include <client.h>

#include "linuxspec.h"


static void
init_args(int argc, char **argv)
{
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

		// Next arg (even if current not recognized).
		i++;
	}

	// If any of the game dirs not run-time specified / empty, 
	// use associated compile-time variable TORCS_XXDIR to get default value
	if (!(localdir && strlen(localdir)))
		localdir = SetLocalDir(TORCS_LOCALDIR);
	if (!(libdir && strlen(libdir)))
		libdir = SetLibDir(TORCS_LIBDIR);
	if (!(bindir && strlen(bindir)))
		bindir = SetBinDir(TORCS_BINDIR);
	if (!(datadir && strlen(datadir)))
		datadir = SetDataDir(TORCS_DATADIR);

	// Check if ALL the game dirs have a usable value, and exit if not.
	if (!(localdir && strlen(localdir)) || !(libdir && strlen(libdir)) 
		|| !(bindir && strlen(bindir)) || !(datadir && strlen(datadir)))
	{
		GfTrace("TORCS_LOCALDIR : '%s'\n", TORCS_LOCALDIR);
		GfTrace("TORCS_LIBDIR   : '%s'\n", TORCS_LIBDIR);
		GfTrace("TORCS_BINDIR   : '%s'\n", TORCS_BINDIR);
		GfTrace("TORCS_DATADIR  : '%s'\n", TORCS_DATADIR);
		GfFatal("Could not start Speed Dreams : at least 1 of local/data/lib/bin dir is empty\n\n");
		exit(1);
	}

	// If "data dir" specified in any way, cd to it.
	if (datadir && strlen(datadir))
		chdir(datadir);
}

/*
 * Function
 *	main
 *
 * Description
 *	LINUX entry point of Speed Dreams
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

	GfFileSetup();           /* Update user settings files from an old version */

	LinuxSpecInit();         /* init specific linux functions */
		
	GfScrInit(argc, argv);	 /* init screen */

	if (GameEntry())         /* launch the game */
	{
		GfelMainLoop();   /* Main event loop */
		exit(0);
	}

	GfError("\nExiting from Speed Dreams for some fatal reason (see above).\n");
	exit(1);                 /* If we got here, something bad happened ... */          
}

