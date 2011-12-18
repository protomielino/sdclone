/***************************************************************************

    file                 : main.cpp
    created              : Sat Sep  2 10:40:47 CEST 2000
    copyright            : (C) 2000 by Patrice & Eric Espie
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

#include <string>
#include <iostream>
#include <sstream>

#include <portability.h>
#include <tgfclient.h>

#ifdef WIN32
#ifndef HAVE_CONFIG_H
#define HAVE_CONFIG_H
#endif
#endif

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <raceengine.h>
#include <iuserinterface.h>


/*
 * Function
 *    main
 *
 * Description
 *    Main function of the game
 *
 * Parameters
 *    argc Number of command line args, + 1 for the executable name
 *    argv Array of zero-terminated strings, 1 for each arg
 *
 * Return
 *    0 status code if OK, non-0 otherwise.
 */
int
main(int argc, char *argv[])
{
	// Look for the "text-only" option flag in the command-line args.
	bool bTextOnly = false;
	for (int i = 1; i < argc; i++)
		if (!strcmp(argv[i], "-x") || !strcmp(argv[i], "--textonly"))
		{
			bTextOnly = true;
			break;
		}

	// Create the application (graphical or text-only UI).
	GfApplication* pApp;
	if (bTextOnly)
		pApp = new GfApplication("Speed Dreams", VERSION_LONG,
								 "an Open Motorsport Sim", argc, argv);
	else
		pApp = new GfuiApplication("Speed Dreams", VERSION_LONG,
								   "an Open Motorsport Sim", argc, argv);

	// Register app. specific options and help text.
	pApp->registerOption("s", "startrace", /* nHasValue = */ true);
	pApp->registerOption("x", "textonly", /* nHasValue = */ false);
	
	pApp->addOptionsHelpSyntaxLine("[-s|--startrace <race name> [-x|--textonly] ]");
	pApp->addOptionsHelpExplainLine
	 	("- race name : Name without extension of the selected raceman file,");
	pApp->addOptionsHelpExplainLine
	 	("              among the .xml files in <user settings>/config/raceman (no default)");
	pApp->addOptionsHelpExplainLine
	 	("- text-only : Run the specified race without any GUI (suitable for a headless computer)");

	// Parse the command line for registered options.
    if (!pApp->parseOptions())
		return 1;

	// Some more checks about command line options.
	std::string strRaceToStart;
	if (bTextOnly && (!pApp->hasOption("startrace", strRaceToStart) || strRaceToStart.empty()))
	{
		std::cerr << "Exiting from " << pApp->name()
				  << " because no race specified in text-only mode." << std::endl;
		return 1;
	}
	
	// If "data dir" specified in any way, cd to it.
	if(chdir(GfDataDir()))
	{
		GfLogError("Could not start %s : failed to cd to the datadir '%s' (%s)\n",
				   pApp->name().c_str(), GfDataDir(), strerror(errno));
		return 1;
	}

	// Update user settings files from installed ones.
    pApp->updateUserSettings();

   // Initialize the event loop management layer (graphical or text-only UI).
	GfEventLoop* pEventLoop;
	if (bTextOnly)
		pEventLoop = new GfEventLoop;
	else
		pEventLoop = new GfuiEventLoop;
	pApp->setEventLoop(pEventLoop);

	// When there's a GUI, setup the window / screen and menu infrastructure.
    if (!bTextOnly && !dynamic_cast<GfuiApplication*>(pApp)->setupWindow())
	{
		std::cerr << "Exiting from " << pApp->name()
				  << " after some error occurred (see above)." << std::endl;
		return 1;
	}

	// Load the user interface module (graphical or text-only UI).
	std::ostringstream ossModLibName;
	ossModLibName << GfLibDir() << "modules/userinterface/"
				  << (bTextOnly ?  "textonly" : "legacymenu") << '.' << DLLEXT;
	GfModule* pmodUserItf = GfModule::load(ossModLibName.str());

	// Check that it implements IUserInterface.
	IUserInterface* piUserItf = 0;
	if (pmodUserItf)
	{
		piUserItf = pmodUserItf->getInterface<IUserInterface>();
	}

 	// Initialize the race engine and the user interface modules.
	if (piUserItf)
	{
		RaceEngine::self().setUserInterface(*piUserItf);
		piUserItf->setRaceEngine(RaceEngine::self());
	}
	
	if (piUserItf)
	{
		// Enter the user interface.
		if (piUserItf->activate())
		{
			// Game event loop (when it returns, it's simply because we are exiting).
			pApp->eventLoop()();
		}
		
		// Shutdown the user interface.
		piUserItf->shutdown();

		// Shutdown the race engine.
		RaceEngine::self().shutdown();
		
		// Unload the user interface module.
		GfModule::unload(pmodUserItf);
	}

	// Done with the app instance.
	const std::string strAppName(pApp->name());
	delete pApp;
	
 	// That's all (but trace what we are doing).
	if (piUserItf)
		GfLogInfo("Exiting normally from %s.\n", strAppName.c_str());
	else
		std::cerr << "Exiting from " << strAppName
				  << " after some error occurred (see above)." << std::endl;
	
	return piUserItf ? 0 : 1;
}

