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
#include <sstream>

#include <portability.h>
#include <tgfclient.h>

#include <raceengine.h>
#include <iuserinterface.h>

class Application : public GfuiApplication
{
 public:

	//! Constructor.
	Application(int argc, char **argv)
	: GfuiApplication("Speed Dreams", "an Open Motorsport Sim", argc, argv)
	{
	}
};

/*
 * Function
 *    main
 *
 * Description
 *    Main function of the game
 *
 * Parameters
 *    argc Number of command line options, + 1 for the executable name
 *    argv Array of zero-terminated strings, 1 for each option
 *
 * Return
 *    Never returns, exits with 0 status code if OK, non-0 otherwise.
 */
int
main(int argc, char *argv[])
{
	// Create the application
	Application app(argc, argv);
	
	// Parse the command line options
    if (!app.parseOptions())
		app.exit(1);

	// Update user settings files from installed ones.
    app.updateUserSettings();

    // Initialize the event loop management layer.
	GfuiEventLoop* pEventLoop = new GfuiEventLoop;
	app.setEventLoop(pEventLoop);

	// Setup the window / screen and menu infrastructure (needs an event loop).
    if (!app.setupWindow())
		app.exit(1);

	// Load the user interface module.
	std::ostringstream ossModLibName;
	ossModLibName << GfLibDir() << "modules/userinterface/" << "legacymenu" << '.' << DLLEXT;
	GfModule* pmodUserItf = GfModule::load(ossModLibName.str());

	// Check that it implements IUserInterface.
	IUserInterface* piUserItf = 0;
	if (pmodUserItf)
	{
		piUserItf = pmodUserItf->getInterface<IUserInterface>();
	}

	// Initialize the race engine and the user interface module.
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
			// Game event loop.
			app.eventLoop()();
		}
		
		// Shutdown the user interface.
		piUserItf->shutdown();
		
		// Unload the user interface module.
		GfModule::unload(pmodUserItf);
	}
	
 	// That's all.
    app.exit(piUserItf ? 0 : 1);

	// Make the compiler happy (never reached).
	return 0;
}

