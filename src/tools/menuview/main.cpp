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

#ifdef WIN32
static void
init_args(int argc, char **argv)
{
    const char *localdir = 0;
    const char *libdir = 0;
    const char *datadir = 0;
    const char *bindir = 0;
    int i;

    static const int BUFSIZE = MAX_PATH;
    char buf[BUFSIZE];

    i = 1;
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

    // If any of the Speed-Dreams dirs not run-time specified / empty, 
    // use associated compile-time variable TORCS_XXDIR to get default value
    if (!(localdir && strlen(localdir)))
    {
      // Interpret a heading '~' in TORCS_LOCALDIR as the <My documents> folder path
      // (to give the user an easy access to advanced settings).
      if (strlen(TORCS_LOCALDIR) > 1 && TORCS_LOCALDIR[0] == '~' 
	  && (TORCS_LOCALDIR[1] == '/' || TORCS_LOCALDIR[1] == '\\'))
      {
	LPITEMIDLIST pidl;
	if (SUCCEEDED(SHGetSpecialFolderLocation(NULL, CSIDL_PERSONAL, &pidl))
	    && SHGetPathFromIDList(pidl, buf))
	{
	  if (buf[0] && buf[strlen(buf)-1] != '/')
	    strcat(buf, "/");
	  strcat(buf, TORCS_LOCALDIR+2);
	  for (i = 0; i < BUFSIZE && buf[i]; i++)
	    if (buf[i] == '\\')
	      buf[i] = '/';
	  localdir = SetLocalDir(buf);
	}
	else
	{
	  printf("Could not get user's My documents folder path\n");
	  exit(1);
	}
      }
      else
	localdir = SetLocalDir(TORCS_LOCALDIR);
    }

		//Default paths
	TCHAR szDir[MAX_PATH];
	GetCurrentDirectory(MAX_PATH,szDir);
	GfOut("Current Dir %s\n",szDir);
	std::string strDefault = szDir;
	std::string strEnd = strDefault.substr(strDefault.size()-4,4);
	if (strEnd == "\\bin")
		strDefault = strDefault.substr(0,strDefault.size()-4);

	std::string strBinDir = strDefault+"\\bin";
	std::string strLibDir = strDefault+"\\lib";
	std::string strDataDir = strDefault+"\\share";

    if (!(libdir && strlen(libdir)))
		libdir = SetLibDir(strLibDir.c_str());
    if (!(bindir && strlen(bindir)))
		bindir = SetBinDir(strBinDir.c_str());
    if (!(datadir && strlen(datadir)))
		datadir = SetDataDir(strDataDir.c_str());

    // Check if ALL the Speed-dreams dirs have a usable value, and exit if not.
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
#else
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
		else
		{
			g_strMenuFile = argv[i];
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
#endif

void ShowMenu(const char *pMenuFile)
{
	GfInitClient();
	PreviewMenuInit(pMenuFile);
	PreviewMenuRun();
}

int
main(int argc, char *argv[])
{
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
		printf("No menu file specified\nUSEAGE\nsd-menuview menufile.xml\n");
		return 0;
	}

	SetDataDir("c:/users/gavin/sd-game/share/");
	ShowMenu(g_strMenuFile.c_str());

    sdlMainLoop();          /* event loop of sdl */

    return 0;	            /* just for the compiler, never reached */
}

