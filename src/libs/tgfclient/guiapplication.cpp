/***************************************************************************
                      guiapplication.cpp -- GUI Application base
                             -------------------
    created              : Sat Apr 16 19:30:04 CEST 2011
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

#include "tgfclient.h"


GfuiApplication::GfuiApplication(const char* pszName, const char* pszDesc, int argc, char **argv)
: GfApplication((pszName ? pszName : "GfuiApplication"), pszDesc, argc, argv)
{
	// Help about the options.
	_optionsHelp.lstSyntaxLines.push_back("[-m|--hardmouse]");
	_optionsHelp.lstExplainLines.push_back("- hardmouse : Use hardware mouse cursor");
}

bool GfuiApplication::parseOptions()
{
	// First the standard ones.
	if (!GfApplication::parseOptions())
		return false;

	// Then the specific ones.
	std::list<std::string> lstNewOptionsLeft;
	std::list<std::string>::const_iterator itOpt;
    for (itOpt = _lstOptionsLeft.begin(); itOpt != _lstOptionsLeft.end(); itOpt++)
    {
        // -m option : Allow the hardware mouse cursor
        if (*itOpt == "-m" || *itOpt == "--hardmouse")
        {
			GfuiMouseSetHWPresent();
        }
		else
		{
			// Save this option : it is "left".
			lstNewOptionsLeft.push_back(*itOpt);
		}
	}
	
	// Store the new list of left options after parsing.
	_lstOptionsLeft = lstNewOptionsLeft;

	return true;
}

bool GfuiApplication::setupWindow(bool bNoMenu)
{
	// Initialize the window/screen.
	_bWindowUp = true; // In case, GfScrInit() would call restart() ...
	_bWindowUp = GfScrInit();

	// Initialize the UI menu infrastructure.
	if (_bWindowUp && !bNoMenu)
		GfuiInit();

	return _bWindowUp;
}

GfuiEventLoop& GfuiApplication::eventLoop()
{
	if (!_pEventLoop)
	{
		GfLogError("GfuiApplication has no event loop ; exiting\n");
		exit(1);
	}
	
    return *dynamic_cast<GfuiEventLoop*>(_pEventLoop);
}

void GfuiApplication::restart()
{
	// Shutdown the window/screen.
	if (_bWindowUp)
	{
		GfuiShutdown();
		_bWindowUp = false;
	}

	// Shutdown the gaming framework.
	GfShutdown();

	// Delete the event loop if any.
	delete _pEventLoop;

	// Restart.
	GfRestart(GfuiMouseIsHWPresent());
}

GfuiApplication::~GfuiApplication()
{
	// Shutdown the window/screen.
	if (_bWindowUp)
	{
		GfuiShutdown();
		_bWindowUp = false;
	}

	// Note: GfApplication (base class) destructor called now.
}
