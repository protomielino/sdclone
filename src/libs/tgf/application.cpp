/***************************************************************************
                      application.cpp -- Application base
                             -------------------
    created              : Mon Apr 14 22:30:04 CEST 2011
    copyright            : (C) 2011 by Jean-Philippe Meuret
    web                  : http://www.speed-dreams.org
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

/** @file   
    		Application base
    @version	$Id$
    @ingroup	tgf
*/

#include <iostream>

#include <config.h>
#include <portability.h>

#include "tgf.hpp"


// The singleton.
GfApplication* GfApplication::_pSelf = 0;

GfApplication& GfApplication::self()
{
	if (!_pSelf)
	{
		GfLogError("GfApplication not yet created ; exiting");
		::exit(1);
	}
	
	return *_pSelf;
}

GfApplication::GfApplication(const char* pszName, const char* pszDesc, int argc, char **argv)
: _strName(pszName ? pszName : "GfApplication"), _strDesc(pszDesc ? pszDesc : ""), _pEventLoop(0)
{
	// Check that we are the only instance.
	if (_pSelf)
	{
		GfLogError("More than one GfApplication instance ; exiting");
		_pSelf->exit(1);
	}

	// Register oneself as the one.
	_pSelf = this;
	
	// Initialize the gaming framework.
    GfInit();
	
	// Store the command line options.
	if (argv)
		for (int i = 0; i < argc; i++)
			_lstOptions.push_back(argv[i]);
	_lstOptionsLeft = _lstOptions;

	// Help about the common options.
	_optionsHelp.lstSyntaxLines.push_back("[-ld|--localdir <dir path>] [-Ld|--libdir <dir path>]");
	_optionsHelp.lstSyntaxLines.push_back("[-Bd|--bindir <dir path>] [-Dd|--datadir <dir path>]");
#ifdef TRACE_OUT
	_optionsHelp.lstSyntaxLines.push_back("[-tl|--tracelevel <integer>]"
										  " [-ts|--tracestream stdout|stderr|<file name>]");
#endif
	
	_optionsHelp.lstExplainLines.push_back
		("- locadir : Root dir of the tree where user settings files are stored");
	_optionsHelp.lstExplainLines.push_back
		("            (default=" SD_LOCALDIR ")");
	_optionsHelp.lstExplainLines.push_back
		("- libdir  : Root dir of the tree where loadable modules are installed");
	_optionsHelp.lstExplainLines.push_back
		("            (default=" SD_LIBDIR ")");
	_optionsHelp.lstExplainLines.push_back
		("- bindir  : Dir where the game exe and DLLs are installed");
	_optionsHelp.lstExplainLines.push_back
		("            (default=" SD_BINDIR ")");
	_optionsHelp.lstExplainLines.push_back
		("- datadir : Root dir of the data tree (cars, tracks, ...)");
	_optionsHelp.lstExplainLines.push_back
		("            (default=" SD_DATADIR ")");
#ifdef TRACE_OUT
	_optionsHelp.lstExplainLines.push_back
		("- tracelevel  : Trace level threshold");
	_optionsHelp.lstExplainLines.push_back
		("                (0=Fatal, 1=Error, 2=Warning, 3=Info, 4=Trace, 5=Debug, ... ; default=5)");
	_optionsHelp.lstExplainLines.push_back
		("- tracestream : Target output stream for the traces");
	_optionsHelp.lstExplainLines.push_back
		("                (default=stderr)");
#endif
}

GfApplication::~GfApplication()
{
	_pSelf = 0;
}

void GfApplication::updateUserSettings()
{
    GfFileSetup();
}

void GfApplication::setEventLoop(GfEventLoop* pEventLoop)
{
	_pEventLoop = pEventLoop;
}

GfEventLoop& GfApplication::eventLoop()
{
	if (!_pEventLoop)
	{
		GfLogError("GfApplication has no event loop ; exiting\n");
		exit(1);
	}
	
    return *_pEventLoop;
}

void GfApplication::exit(int nStatusCode)
{
	if (!nStatusCode)
		GfLogInfo("Exiting normally from %s.\n", _strName.c_str());
	else
		std::cerr << "Exiting from " << _strName
				  << " after some error occurred (see above)." << std::endl;

	// Shutdown the gaming framework.
	GfShutdown();

	// Delete the event loop if any.
	delete _pEventLoop;
	
	// The end.
	::exit(nStatusCode);
}

void GfApplication::printUsage(const char* pszErrMsg) const
{
	if (pszErrMsg)
		std::cerr << std::endl << "Error: " << pszErrMsg << std::endl << std::endl;
	
	std::cerr << "Usage: " << _lstOptions.front() << " ..." << std::endl;

	std::list<std::string>::const_iterator itSynLine = _optionsHelp.lstSyntaxLines.begin();
    while (itSynLine != _optionsHelp.lstSyntaxLines.end())
	{
		std::cerr << "       " << *itSynLine << std::endl;
		itSynLine++;
	}

	std::list<std::string>::const_iterator itExplLine = _optionsHelp.lstExplainLines.begin();
    while (itExplLine != _optionsHelp.lstExplainLines.end())
	{
		std::cerr << " " << *itExplLine << std::endl;
		itExplLine++;
	}
}

bool GfApplication::parseOptions()
{
	// Determine and store run-time install root dir.
	GfInitInstallDir(_lstOptions.front().c_str());

	// Parse options.
    const char *localdir = 0;
    const char *libdir = 0;
    const char *datadir = 0;
    const char *bindir = 0;

	std::list<std::string> lstNewOptionsLeft;
	std::list<std::string>::const_iterator itOpt = _lstOptionsLeft.begin();
    for (itOpt++; itOpt != _lstOptionsLeft.end(); itOpt++)
    {
		// Help about command line
		if (*itOpt == "-h" || *itOpt == "--help")
        {
			printUsage();
			exit(0);
        }
        // Local dir (root dir of the tree where user settings files are stored)
        else if (*itOpt == "-ld" || *itOpt == "--localdir")
        {
			itOpt++;
			if (itOpt != _lstOptionsLeft.end())
				localdir = GfSetLocalDir(itOpt->c_str());
        }
        // Libraries dir (root dir of the tree where loadable modules are installed)
        else if (*itOpt == "-Ld" || *itOpt == "--libdir")
        {
			itOpt++;
			if (itOpt != _lstOptionsLeft.end())
				libdir = GfSetLibDir(itOpt->c_str());
        }
        // Binaries dir (the dir where the game exe and DLLs are installed)
        else if (*itOpt == "-Bd" || *itOpt == "--bindir")
        {
			itOpt++;
			if (itOpt != _lstOptionsLeft.end())
				bindir = GfSetBinDir(itOpt->c_str());
        }
        // Data dir (root dir of the data tree)
        else if (*itOpt == "-Dd" || *itOpt == "--datadir")
        {
			itOpt++;
            if (itOpt != _lstOptionsLeft.end())
                datadir = GfSetDataDir(itOpt->c_str());
        }
        // Trace level threshold (only #ifdef TRACE_OUT)
        else if (*itOpt == "-tl" || *itOpt == "--tracelevel")
        {
            int nTraceLevel;
			itOpt++;
            if (itOpt != _lstOptionsLeft.end() && sscanf(itOpt->c_str(), "%d", &nTraceLevel) == 1)
                GfLogSetLevelThreshold(nTraceLevel);
        }
        // Target trace stream (only #ifdef TRACE_OUT)
        else if (*itOpt == "-ts" || *itOpt == "--tracestream")
        {
			itOpt++;
            if (itOpt != _lstOptionsLeft.end())
			{
                if (*itOpt == "stderr")
                    GfLogSetStream(stderr);
                else if (*itOpt == "stdout")
                    GfLogSetStream(stdout);
                else
                    GfLogSetFile(itOpt->c_str());
			}
        }
		else
		{
			// Save this option : it is "left".
			lstNewOptionsLeft.push_back(*itOpt);
		}
	}

	// Store the new list of left options after parsing.
	_lstOptionsLeft = lstNewOptionsLeft;

    // If any of the Speed-Dreams dirs not run-time specified / empty, 
    // use associated compile-time variable SD_XXDIR to get default value
    if (!(localdir && strlen(localdir)))
		localdir = GfSetLocalDir(SD_LOCALDIR);
	if (!(libdir && strlen(libdir)))
		libdir = GfSetLibDir(SD_LIBDIR);
    if (!(bindir && strlen(bindir)))
		bindir = GfSetBinDir(SD_BINDIR);
    if (!(datadir && strlen(datadir)))
		datadir = GfSetDataDir(SD_DATADIR);
	
    // Check if ALL the Speed-dreams dirs have a usable value, and exit if not.
    if (!(localdir && strlen(localdir)) || !(libdir && strlen(libdir)) 
		|| !(bindir && strlen(bindir)) || !(datadir && strlen(datadir)))
    {
        GfLogTrace("SD_LOCALDIR : '%s'\n", GfLocalDir());
        GfLogTrace("SD_LIBDIR   : '%s'\n", GfLibDir());
        GfLogTrace("SD_BINDIR   : '%s'\n", GfBinDir());
        GfLogTrace("SD_DATADIR  : '%s'\n", GfDataDir());
        GfLogError("Could not start Speed Dreams : at least 1 of local/data/lib/bin dir is empty\n\n");
        return false;
    }

	// TODO: Move this to the main ?
    // If "data dir" specified in any way, cd to it.
    if (datadir && strlen(datadir))
		chdir(datadir);

	return true;
}

