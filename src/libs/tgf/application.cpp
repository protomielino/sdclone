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

#include <cerrno>
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

GfApplication::GfApplication(const char* pszName, const char* pszVersion, const char* pszDesc,
							 int argc, char **argv)
: _strName(pszName ? pszName : "GfApplication"), _strDesc(pszDesc ? pszDesc : ""),
  _strVersion(pszVersion ? pszVersion : ""), _pEventLoop(0)
{
	// Check that we are the only instance.
	if (_pSelf)
	{
		GfLogError("More than one GfApplication instance ; exiting");
		::exit(1);
	}

	// Register oneself as the one.
	_pSelf = this;
	
	// Initialize the gaming framework.
	GfInit();

	// Store the command line args.
	if (argv)
		for (int i = 0; i < argc; i++)
			_lstArgs.push_back(argv[i]);

	// Register the command line options (to be parsed).
	registerOption("h", "help", /* nHasValue = */ false);
	registerOption("v", "version", /* nHasValue = */ false);
	registerOption("ld", "localdir", /* nHasValue = */ true);
	registerOption("Ld", "libdir", /* nHasValue = */ true);
	registerOption("Bd", "bindir", /* nHasValue = */ true);
	registerOption("Dd", "datadir", /* nHasValue = */ true);
	registerOption("tl", "tracelevel", /* nHasValue = */ true);
	registerOption("ts", "tracestream", /* nHasValue = */ true);
	
	// Help about the command line options.
	addOptionsHelpSyntaxLine("[-ld|--localdir <dir path>] [-Ld|--libdir <dir path>]");
	addOptionsHelpSyntaxLine("[-Bd|--bindir <dir path>] [-Dd|--datadir <dir path>]");
#ifdef TRACE_OUT
	addOptionsHelpSyntaxLine("[-tl|--tracelevel <integer>]"
							 " [-ts|--tracestream stdout|stderr|<file name>]");
#endif
	addOptionsHelpSyntaxLine("[-v|--version]");
	
	addOptionsHelpExplainLine
		("- locadir : Root dir of the tree where user settings files are stored");
	addOptionsHelpExplainLine
		("            (default=" SD_LOCALDIR ")");
	addOptionsHelpExplainLine
		("- libdir  : Root dir of the tree where loadable modules are installed");
	addOptionsHelpExplainLine
		("            (default=" SD_LIBDIR ")");
	addOptionsHelpExplainLine
		("- bindir  : Dir where the game exe and DLLs are installed");
	addOptionsHelpExplainLine
		("            (default=" SD_BINDIR ")");
	addOptionsHelpExplainLine
		("- datadir : Root dir of the data tree (cars, tracks, ...)");
	addOptionsHelpExplainLine
		("            (default=" SD_DATADIR ")");
#ifdef TRACE_OUT
	addOptionsHelpExplainLine
		("- tracelevel  : Trace level threshold");
	addOptionsHelpExplainLine
		("                (0=Fatal, 1=Error, 2=Warning, 3=Info, 4=Trace, 5=Debug, ... ; default=5)");
	addOptionsHelpExplainLine
		("- tracestream : Target output stream for the traces");
	addOptionsHelpExplainLine
		("                (default=stderr)");
#endif
}

const std::string& GfApplication::name() const
{
	return _strName;
}

const std::string& GfApplication::description() const
{
	return _strDesc;
}

const std::string& GfApplication::version() const
{
	return _strVersion;
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
		GfLogError("GfApplication has no event loop ; crashing !\n");
	
	return *_pEventLoop;
}

void GfApplication::restart()
{
	// Shutdown the gaming framework.
	GfShutdown();

	// Delete the event loop if any.
	delete _pEventLoop;

	// Restart.
	GfRestart();
}

void GfApplication::printUsage(const char* pszErrMsg) const
{
	if (pszErrMsg)
		std::cerr << std::endl << "Error: " << pszErrMsg << std::endl << std::endl;
	
	std::cerr << "Usage: " << _lstArgs.front() << " ..." << std::endl;

	std::list<std::string>::const_iterator itSynLine = _optionsHelp.lstSyntaxLines.begin();
	while (itSynLine != _optionsHelp.lstSyntaxLines.end())
	{
		std::cerr << "         " << *itSynLine << std::endl;
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
	GfInitInstallDir(_lstArgs.front().c_str());

	// Parse args, looking for registered options.
	std::list<std::string>::const_iterator itArg = _lstArgs.begin();
	for (itArg++; itArg != _lstArgs.end(); itArg++)
	{
		bool bArgEaten = false;
		if (itArg->find('-') == 0)
		{
			// We've probably got an option flag : check this a bit more in depth.
			std::list<Option>::iterator itOpt;
			for (itOpt = _lstOptions.begin(); itOpt != _lstOptions.end(); itOpt++)
			{
				if (itOpt->strShortName == itArg->substr(1, std::string::npos)
					|| itOpt->strLongName == itArg->substr(2, std::string::npos))
				{
					// We've got a registered option flag : check if there's a value arg or not.
					if (itOpt->bHasValue)
					{
						itArg++;
						if (itArg != _lstArgs.end() // Some extra arg available ...
							&& itArg->find('-') != 0) // ... and not an option flag :
						{
							itOpt->strValue = *itArg; // We've got the value.
						}
						else
						{
							// Should have a value arg, but it's not there ... error !
							printUsage();
							return false;
						}
					}

					// Value or not, we've got an option, and we eat the arg(s) : done.
					itOpt->bFound = true;
					bArgEaten = true;
					break;
				}
			}
		}

		// Save any ignored arg in the "remaining" list.
		if (!bArgEaten)
			_vecRemArgs.push_back(*itArg);
	}
	
	// Interpret the detected command line options.
	const char *pszLocalDir = 0;
	const char *pszLibDir = 0;
	const char *pszDataDir = 0;
	const char *pszBinDir = 0;

	std::list<Option>::const_iterator itOpt;
	for (itOpt = _lstOptions.begin(); itOpt != _lstOptions.end(); itOpt++)
	{
		// Not found in the command line => ignore / default value.
		if (!itOpt->bFound)
			continue;
		
		// Help about command line
		if (itOpt->strLongName == "help")
		{
			printUsage();
			return false;
		}
		// Version information
		else if (itOpt->strLongName == "version")
		{
			std::cerr << _strName << ' ' << _strVersion << std::endl;
			return false;
		}
		// Local dir (root dir of the tree where user settings files are stored)
		else if (itOpt->strLongName == "localdir")
		{
			pszLocalDir = GfSetLocalDir(itOpt->strValue.c_str());
		}
		// Libraries dir (root dir of the tree where loadable modules are installed)
		else if (itOpt->strLongName == "libdir")
		{
			pszLibDir = GfSetLibDir(itOpt->strValue.c_str());
		}
		// Binaries dir (the dir where the game exe and DLLs are installed)
		else if (itOpt->strLongName == "bindir")
		{
			pszBinDir = GfSetBinDir(itOpt->strValue.c_str());
		}
		// Data dir (root dir of the data tree)
		else if (itOpt->strLongName == "datadir")
		{
			pszDataDir = GfSetDataDir(itOpt->strValue.c_str());
		}
		// Trace level threshold (only #ifdef TRACE_OUT)
		else if (itOpt->strLongName == "tracelevel")
		{
			int nTraceLevel;
			if (sscanf(itOpt->strValue.c_str(), "%d", &nTraceLevel) == 1)
				GfLogSetLevelThreshold(nTraceLevel);
			else
			{
				printUsage("Error: Could not convert trace level to an integer");
				return false;
			}
		}
		// Target trace stream (only #ifdef TRACE_OUT)
		else if (itOpt->strLongName == "tracestream")
		{
			if (itOpt->strValue == "stderr")
				GfLogSetStream(stderr);
			else if (itOpt->strValue == "stdout")
				GfLogSetStream(stdout);
			else
				GfLogSetFile(itOpt->strValue.c_str());
		}
		else
		{
			// If we get here, this is normal : the derived classes might have declared
			// specific options.
		}
	}

	// If any of the Speed-Dreams dirs not run-time specified / empty, 
	// use associated compile-time variable SD_XXDIR to get default value
	if (!(pszLocalDir && strlen(pszLocalDir)))
		pszLocalDir = GfSetLocalDir(SD_LOCALDIR);
	if (!(pszLibDir && strlen(pszLibDir)))
		pszLibDir = GfSetLibDir(SD_LIBDIR);
	if (!(pszBinDir && strlen(pszBinDir)))
		pszBinDir = GfSetBinDir(SD_BINDIR);
	if (!(pszDataDir && strlen(pszDataDir)))
		pszDataDir = GfSetDataDir(SD_DATADIR);
	
	// Check if ALL the Speed-dreams dirs have a usable value, and exit if not.
	if (!(pszLocalDir && strlen(pszLocalDir)) || !(pszLibDir && strlen(pszLibDir)) 
		|| !(pszBinDir && strlen(pszBinDir)) || !(pszDataDir && strlen(pszDataDir)))
	{
		GfLogTrace("SD_LOCALDIR : '%s'\n", GfLocalDir());
		GfLogTrace("SD_LIBDIR   : '%s'\n", GfLibDir());
		GfLogTrace("SD_BINDIR   : '%s'\n", GfBinDir());
		GfLogTrace("SD_DATADIR  : '%s'\n", GfDataDir());
		
		GfLogError("Could not start %s :"
				   " at least 1 of local/data/lib/bin dir is empty\n\n", _strName.c_str());
		
		return false;
	}

	return true;
}

void GfApplication::registerOption(const std::string& strShortName,
								   const std::string& strLongName,
								   bool bHasValue)
{
	_lstOptions.push_back(Option(strShortName, strLongName, bHasValue));
}

void GfApplication::addOptionsHelpSyntaxLine(const std::string& strTextLine)
{
	_optionsHelp.lstSyntaxLines.push_back(strTextLine);
}

void GfApplication::addOptionsHelpExplainLine(const std::string& strTextLine)
{
	_optionsHelp.lstExplainLines.push_back(strTextLine);
}

bool GfApplication::hasOption(const std::string& strLongName) const
{
	std::list<Option>::const_iterator itOpt;
	for (itOpt = _lstOptions.begin(); itOpt != _lstOptions.end(); itOpt++)
		if (itOpt->bFound && itOpt->strLongName == strLongName)
			return true;

	return false;
}

bool GfApplication::hasOption(const std::string& strLongName,
							  std::string& strValue) const
{
	std::list<Option>::const_iterator itOpt;
	for (itOpt = _lstOptions.begin(); itOpt != _lstOptions.end(); itOpt++)
		if (itOpt->bFound && itOpt->strLongName == strLongName)
		{
			strValue = itOpt->strValue;
			return true;
		}

	return false;
}

// bool GfApplication::hasUnregisteredOption(const std::string& strShortName,
// 										  const std::string& strLongName) const
// {
// 	std::list<std::string>::const_iterator itArg;
// 	for (itArg = _lstRemArgs.begin(); itArg != _lstRemArgs.end(); itArg++)
// 		if (*itArg == "-" + strShortName || *itArg == "--" + strLongName)
// 			return true;

// 	return false;
// }

// bool GfApplication::hasUnregisteredOption(const std::string& strShortName,
// 										  const std::string& strLongName,
// 										  std::string& strValue) const
// {
// 	std::list<std::string>::const_iterator itArg;
// 	for (itArg = _lstRemArgs.begin(); itArg != _lstRemArgs.end(); itArg++)
// 		if (*itArg == "-" + strShortName || *itArg == "--" + strLongName)
// 		{
// 			itArg++;
// 			if (itArg != _lstRemArgs.end() // Some extra arg available ...
// 				&& itArg->find('-') != 0) // ... but not an option flag.
// 			{
// 				strValue = *itArg;
// 				return true;
// 			}
// 			else
// 				break; // Value not found. TODO: Error handling ?
// 		}

// 	return false;
// }

GfApplication::~GfApplication()
{
	// Shutdown the gaming framework.
	GfShutdown();

	// Delete the event loop if any.
	delete _pEventLoop;
	_pEventLoop = 0;

	// Really shutdown the singleton.
	_pSelf = 0;
}
