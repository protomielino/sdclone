/***************************************************************************

    file                 : trackgen.cpp
    created              : Sat Dec 23 09:27:43 CET 2000
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

/** @file   
    		
    @author	<a href=mailto:torcs@free.fr>Eric Espie</a>
    @version	$Id$
*/
#include <cstdio>
#include <cstdlib>
#include <ctype.h>
#include <cstring>
#include <cmath>

#include <sstream>

#ifndef WIN32
#include <unistd.h>
#endif

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#ifndef WIN32
#include <getopt.h>
#endif

#include <plib/ul.h>
#include <plib/ssg.h>
#include <SDL/SDL.h>

#include <config.h>
#include <tgf.hpp>
#include <portability.h>
#include <itrackloader.h>

#include "ac3d.h"
#include "easymesh.h"
#include "objects.h"
#include "elevation.h"
#include "trackgen.h"


float	GridStep = 40.0;
float	TrackStep = 5.0;
float	Margin = 100.0;
float	ExtHeight = 5.0;

int	HeightSteps = 30;

int	Bump = 0;
int	UseBorder = 1;

char		*OutputFileName;
char		*TrackName;
char		*TrackCategory;

void		*TrackHandle;
void		*CfgHandle;

tTrack		*Track;
ITrackLoader*	PiTrackLoader;

int		TrackOnly;
int		JustCalculate;
int		MergeAll;
int		MergeTerrain;

static char	buf[1024];
static char	buf2[1024];
static char	trackdef[1024];

char		*OutTrackName;
char		*OutMeshName;

tModList	*modlist = NULL;

int		DoSaveElevation;
char		*ElevationFile;

class Application : public GfApplication
{
public:

	//! Constructor.
	Application(int argc, char** argv);

	//! Parse the command line options.
	// TODO: Move to the GfApplication way of parsing options ?
	bool parseOptions();
	
	void generate();
};

//! Constructor.
Application::Application(int argc, char** argv)
: GfApplication("TrackGen", "Terrain generator for tracks 1.5.2.1", argc, argv)
{
	// Help on specific options.
	_optionsHelp.lstSyntaxLines.push_back
		("-c category -n name [-a] [-m] [-s] [-S] [-E <n> [-H <nb>]]");
	_optionsHelp.lstSyntaxLines.push_back
		("[-h|--help] [-v|--version]");
	
    _optionsHelp.lstExplainLines.push_back
		("-c category    : track category (road, speedway, dirt...)");
    _optionsHelp.lstExplainLines.push_back
		("-n name        : track name");
    _optionsHelp.lstExplainLines.push_back
		("-b             : draw bump track");
    _optionsHelp.lstExplainLines.push_back
		("-B             : Don't use terrain border (relief supplied int clockwise, ext CC)");
    _optionsHelp.lstExplainLines.push_back
		("-a             : draw all (default is track only)");
    _optionsHelp.lstExplainLines.push_back
		("-z             : Just calculate track parameters and exit");
    _optionsHelp.lstExplainLines.push_back
		("-s             : split the track and the terrain");
    _optionsHelp.lstExplainLines.push_back
		("-S             : split all");
    _optionsHelp.lstExplainLines.push_back
		("-E <n>         : save elevation file n");
    _optionsHelp.lstExplainLines.push_back
		("                  0: all elevatation files");
    _optionsHelp.lstExplainLines.push_back
		("                  1: elevation file of terrain + track");
    _optionsHelp.lstExplainLines.push_back
		("                  2: elevation file of terrain with track white");
    _optionsHelp.lstExplainLines.push_back
		("                  3: track only");
    _optionsHelp.lstExplainLines.push_back
		("                  4: track elevations with height steps");
    _optionsHelp.lstExplainLines.push_back
		("-H <nb>        : nb of height steps for 4th elevation file [30]");
}

// Parse the command line options.
bool Application::parseOptions()
{
	// First the standard ones.
	if (!GfApplication::parseOptions())
		return false;

	// Then the specific ones.	
    TrackOnly = 1;
	JustCalculate = 0;
    MergeAll = 1;
    MergeTerrain = 1;
    TrackName = NULL;
    TrackCategory = NULL;
    DoSaveElevation = -1;

	std::list<std::string> lstNewOptionsLeft;
	std::list<std::string>::const_iterator itOpt;
    for (itOpt = _lstOptionsLeft.begin(); itOpt != _lstOptionsLeft.end(); itOpt++)
    {
        // -m option : Allow the hardware mouse cursor
        if (*itOpt == "-v" || *itOpt == "--version")
        {
			printf("%s\n", _strDesc.c_str());
			exit(0);
		}
		else if (*itOpt == "-a")
        {
			TrackOnly = 0;
		}
		else if (*itOpt == "-z")
		{
			JustCalculate = 1;
		}
		else if (*itOpt == "-b")
		{
			Bump = 1;
		}
		else if (*itOpt == "-s")
		{
			MergeAll = 0;
			MergeTerrain = 1;
		}
		else if (*itOpt == "-B")
		{
			UseBorder = 0;
		}
		else if (*itOpt == "-S")
		{
			MergeAll = 0;
			MergeTerrain = 0;
		}
		else if (*itOpt == "-n")
		{
			itOpt++;
            if (itOpt != _lstOptionsLeft.end())
 				TrackName = strdup(itOpt->c_str());
			else
			{
				printUsage("Track name expected after -n");
				exit(1);
			}
		}
		else if (*itOpt == "-E")
		{
			itOpt++;
            if (itOpt != _lstOptionsLeft.end())
				DoSaveElevation = strtol(itOpt->c_str(), NULL, 0);
			else
			{
				printUsage("Save elevation option # expected after -E");
				exit(1);
			}
			TrackOnly = 0;
		}
		else if (*itOpt == "-c")
		{
			itOpt++;
            if (itOpt != _lstOptionsLeft.end())
				TrackCategory = strdup(itOpt->c_str());
			else
			{
				printUsage("Track category expected after -c");
				exit(1);
			}
		}
		else if (*itOpt == "-H")
		{
			itOpt++;
            if (itOpt != _lstOptionsLeft.end())
				HeightSteps = strtol(itOpt->c_str(), NULL, 0);
			else
			{
				printUsage("Nb of height steps expected after -H");
				exit(1);
			}
		}
		else
		{
			std::ostringstream ossMsg;
			ossMsg << "Unsupported option << " << *itOpt;
			printUsage(ossMsg.str().c_str());
			exit(1);
		}
    }

	if (!TrackName || !TrackCategory)
	{
		printUsage("No track name or category specified");
		exit(1);
    }

	return true;
}

// #ifdef WIN32
// #define INSTBASE "./"
// #endif

void Application::generate()
{
	const char *extName;
	FILE *outfd = NULL;

	// Get the trackgen paramaters.
	sprintf(buf, "%s", CFG_FILE);
	CfgHandle = GfParmReadFile(buf, GFPARM_RMODE_STD | GFPARM_RMODE_CREAT);

	// Load and initialize the track loader module.
	GfLogInfo("Loading Track Loader ...\n");
	std::ostringstream ossModLibName;
	ossModLibName << GfLibDir() << "modules/track/" << "track" << '.' << DLLEXT;
	GfModule* pmodTrkLoader = GfModule::load(ossModLibName.str());

	// Check that it implements ITrackLoader.
	ITrackLoader* PiTrackLoader = 0;
	if (pmodTrkLoader)
		PiTrackLoader = pmodTrkLoader->getInterface<ITrackLoader>();
	if (!PiTrackLoader)
		return;

	// This is the track definition.
	sprintf(trackdef, "%stracks/%s/%s/%s.xml", GfDataDir(), TrackCategory, TrackName, TrackName);
	TrackHandle = GfParmReadFile(trackdef, GFPARM_RMODE_STD);
	if (!TrackHandle) {
		fprintf(stderr, "Cannot find %s\n", trackdef);
		exit(1);
	}

	// Build the track structure with graphic extensions.
	Track = PiTrackLoader->load(trackdef, true);

	if (!JustCalculate) {
		// Get the output file radix.
		sprintf(buf2, "%stracks/%s/%s/%s", GfDataDir(), Track->category, Track->internalname, Track->internalname);
		OutputFileName = strdup(buf2);

		// Number of goups for the complete track.
		if (TrackOnly) {
			sprintf(buf2, "%s.ac", OutputFileName);
			// Track.
			outfd = Ac3dOpen(buf2, 1);
		} else if (MergeAll) {
			sprintf(buf2, "%s.ac", OutputFileName);
			// track + terrain + objects.
			outfd = Ac3dOpen(buf2, 2 + GetObjectsNb(TrackHandle));
		}

		// Main Track.
		if (Bump) {
			extName = "trk-bump";
		} else {
			extName = "trk";
		}

		sprintf(buf2, "%s-%s.ac", OutputFileName, extName);
		OutTrackName = strdup(buf2);
	}

	if (JustCalculate){
		CalculateTrack(Track, TrackHandle, Bump);
		return;
	}

	GenerateTrack(Track, TrackHandle, OutTrackName, outfd, Bump);

	if (TrackOnly) {
		return;
	}

	// Terrain.
	if (MergeTerrain && !MergeAll) {
		sprintf(buf2, "%s.ac", OutputFileName);
		/* terrain + objects  */
		outfd = Ac3dOpen(buf2, 1 + GetObjectsNb(TrackHandle));
	}

	extName = "msh";
	sprintf(buf2, "%s-%s.ac", OutputFileName, extName);
	OutMeshName = strdup(buf2);

	GenerateTerrain(Track, TrackHandle, OutMeshName, outfd, DoSaveElevation);

	if (DoSaveElevation != -1) {
		if (outfd) {
			Ac3dClose(outfd);
		}
		switch (DoSaveElevation) {
			case 0:
			case 1:
				sprintf(buf2, "%s.ac", OutputFileName);
				sprintf(buf, "%s-elv.png", OutputFileName);
				SaveElevation(Track, TrackHandle, buf, buf2, 1);
				if (DoSaveElevation) {
					break;
				}
			case 2:
				sprintf(buf, "%s-elv2.png", OutputFileName);
				SaveElevation(Track, TrackHandle, buf, OutMeshName, 1);
				if (DoSaveElevation) {
					break;
				}
			case 3:
				sprintf(buf, "%s-elv3.png", OutputFileName);
				SaveElevation(Track, TrackHandle, buf, OutMeshName, 0);
				if (DoSaveElevation) {
					break;
				}
			case 4:
				sprintf(buf, "%s-elv4.png", OutputFileName);
				SaveElevation(Track, TrackHandle, buf, OutTrackName, 2);
				break;
		}
		return;
	}

	GenerateObjects(Track, TrackHandle, CfgHandle, outfd, OutMeshName);
}


int main(int argc, char **argv)
{
	// Create the application
	Application app(argc, argv);
	
	// Parse the command line options
    if (!app.parseOptions())
		app.exit(1);

	// Why initialize the video / window caption / SDL_Quit ? Really needed ?
    if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0)
	{
        GfLogError("Couldn't initialize SDL video subsystem: %s\n", SDL_GetError());
        app.exit(1);
    }

    atexit(SDL_Quit);

    SDL_WM_SetCaption(argv[1],NULL);
	// End why ...
	
	app.generate();
	
 	// That's all.
	app.exit(0);
	
	// Make the compiler happy (never reached).
	return 0;
}
