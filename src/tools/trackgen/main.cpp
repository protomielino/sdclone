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
#include <cctype>

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

#ifdef WIN32
#ifndef HAVE_CONFIG_H
#define HAVE_CONFIG_H
#endif
#endif

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <tgf.hpp>
#include <portability.h>
#include <itrackloader.h>

#include "ac3d.h"
#include "easymesh.h"
#include "objects.h"
#include "elevation.h"
#include "trackgen.h"
#include "util.h"


class Application : public GfApplication
{
    //! Command line options.
    int HeightSteps;
    bool Bump;
    bool Raceline;
    bool UseBorder;
    std::string TrackName;
    std::string TrackCategory;
    bool TrackOnly;
    bool JustCalculate;
    bool MergeAll;
    bool MergeTerrain;
    int DoSaveElevation;
    bool Bridge;
    bool DumpTrack;
    bool MultipleMaterials;
    std::string TrackXMLFilePath;   // Full path to track XML input file to read from (mainly for unittests)
    std::string TrackACFilePath;    // Full path to track AC output file to write into (mainly for unittests)

public:

    //! Constructor.
    Application();

    //! Initialization.
    virtual void initialize(bool bLoggingEnabled, int argc = 0, char **argv = 0);

    //! Parse the command line options.
    bool parseOptions();

    //! Generate the track.
    int generate();
};

//! Constructor.
Application::Application()
: GfApplication("TrackGen", "1.6.0.1", "Terrain generator for tracks")
, HeightSteps(30)
, Bump(false)
, Raceline(false)
, UseBorder(true)
, TrackOnly(true)
, JustCalculate(false)
, MergeAll(true)
, MergeTerrain(true)
, DoSaveElevation(-1)
, Bridge(true)
, DumpTrack(false)
, MultipleMaterials(false)
{
}

void Application::initialize(bool bLoggingEnabled, int argc, char **argv)
{
    // Base initialization first.
    GfApplication::initialize(bLoggingEnabled, argc, argv);

    // Specific options.
    registerOption("c", "category", /* bHasValue = */ true);
    registerOption("n", "name", /* bHasValue = */ true);
    registerOption("b", "bump", /* bHasValue = */ false);
    registerOption("r", "raceline", /* bHasValue = */ false);
    registerOption("B", "noborder", /* bHasValue = */ false);
    registerOption("a", "all", /* bHasValue = */ false);
    registerOption("z", "calc", /* bHasValue = */ false);
    registerOption("s", "split", /* bHasValue = */ false);
    registerOption("S", "splitall", /* bHasValue = */ false);
    registerOption("E", "saveelev", /* bHasValue = */ true);
    registerOption("H", "height4", /* bHasValue = */ true);
    registerOption("nb", "nobridge", /* bHasValue = */ false);
    registerOption("dt", "dumptrack", /* bHasValue = */ false);
    registerOption("i", "inpath", /* bHasValue = */ true);
    registerOption("o", "outpath", /* bHasValue = */ true);
    registerOption("m", "materials", /* bHasValue = */ false);

    // Help on specific options.
    addOptionsHelpSyntaxLine("-c|--category <cat> -n|--name <name> [-b|bump] [-r|--raceline] [-B|--noborder] [-nb|--nobridge] [-m|--materials]");
    addOptionsHelpSyntaxLine("[-a|--all] [-z|--calc] [-s|split] [-S|splitall]");
    addOptionsHelpSyntaxLine("[-E|--saveelev <#ef> [-H|height4 <#hs>]] [-dt|--dumptrack]");
    addOptionsHelpSyntaxLine("[-i xml_path] [-o ac_path]");

    addOptionsHelpExplainLine("<cat>    : track category (road, speedway, dirt...)");
    addOptionsHelpExplainLine("<name>   : track name");
    addOptionsHelpExplainLine("bump     : draw bump track");
    addOptionsHelpExplainLine("raceline : draw raceline track\n");
    addOptionsHelpExplainLine("noborder : don't use terrain border "
                              "(relief supplied int clockwise, ext CC)");
    addOptionsHelpExplainLine("all      : draw all (default is track only)");
    addOptionsHelpExplainLine("calc     : only calculate track parameters and exit");
    addOptionsHelpExplainLine("split    : split the track and the terrain");
    addOptionsHelpExplainLine("splitall : split all");
    addOptionsHelpExplainLine("<#ef>    : # of the elevation file to save");
    addOptionsHelpExplainLine("  0: all elevation files");
    addOptionsHelpExplainLine("  1: elevation file of terrain + track");
    addOptionsHelpExplainLine("  2: elevation file of terrain with track white");
    addOptionsHelpExplainLine("  3: track only");
    addOptionsHelpExplainLine("  4: track elevations with height steps");
    addOptionsHelpExplainLine("<#hs> : nb of height steps for 4th elevation file [30]");
    addOptionsHelpExplainLine("materials : mulitple materials");
    addOptionsHelpExplainLine("dumptrack : dump track segments to file name.dump");
}

// Parse the command line options.
bool Application::parseOptions()
{
    // Parse command line for registered options, and interpret standard ones.
    if (!GfApplication::parseOptions())
        return false;

    std::list<Option>::const_iterator itOpt;
    for (itOpt = _lstOptions.begin(); itOpt != _lstOptions.end(); ++itOpt)
    {
        // Not found in the command line => ignore / default value.
        if (!itOpt->bFound)
            continue;

        if (itOpt->strLongName == "all")
        {
            TrackOnly = false;
        }
        else if (itOpt->strLongName == "calc")
        {
            JustCalculate = true;
        }
        else if (itOpt->strLongName == "bump")
        {
            Bump = true;
        }
        else if (itOpt->strLongName == "raceline")
        {
            Raceline = true;
        }
        else if (itOpt->strLongName == "split")
        {
            MergeAll = false;
            MergeTerrain = true;
        }
        else if (itOpt->strLongName == "splitall")
        {
            MergeAll = false;
            MergeTerrain = false;
        }
        else if (itOpt->strLongName == "noborder")
        {
            UseBorder = false;
        }
        else if (itOpt->strLongName == "name")
        {
            TrackName = itOpt->strValue;
        }
        else if (itOpt->strLongName == "saveelev")
        {
            DoSaveElevation = strtol(itOpt->strValue.c_str(), nullptr, 0);
            TrackOnly = false;
        }
        else if (itOpt->strLongName == "category")
        {
            TrackCategory = itOpt->strValue;
        }
        else if (itOpt->strLongName == "steps4")
        {
            HeightSteps = strtol(itOpt->strValue.c_str(), nullptr, 0);
        }
        else if (itOpt->strLongName == "nobridge")
        {
            Bridge = false;
        }
        else if (itOpt->strLongName == "dumptrack")
        {
            DumpTrack = true;
        }
        else if (itOpt->strLongName == "inpath")
        {
            TrackXMLFilePath = itOpt->strValue;
        }
        else if (itOpt->strLongName == "outpath")
        {
            TrackACFilePath = itOpt->strValue;
        }
        else if (itOpt->strLongName == "materials")
        {
            MultipleMaterials = true;
        }
    }

    if (TrackName.empty() || TrackCategory.empty())
    {
        printUsage("No track name or category specified");
        return false;
    }

    return true;
}

int Application::generate()
{
    ssgAddTextureFormat(".rgb", ssgLoadSGI);
    ssgAddTextureFormat(".rgba", ssgLoadSGI);
    ssgAddTextureFormat(".int", ssgLoadSGI);
    ssgAddTextureFormat(".inta", ssgLoadSGI);
    ssgAddTextureFormat(".bw", ssgLoadSGI);
    ssgAddTextureFormat(".png", loadPngTexture);
    ssgAddTextureFormat(".jpg", loadJpegTexture);

    // Get the trackgen paramaters.
    void *CfgHandle = GfParmReadFile(CFG_FILE, GFPARM_RMODE_STD | GFPARM_RMODE_CREAT);

    // Load and initialize the track loader module.
    GfLogInfo("Loading Track Loader ...\n");
    std::ostringstream ossModLibName;
    ossModLibName << GfLibDir() << "modules/track/" << "trackv1" << DLLEXT;
    GfModule *pmodTrkLoader = GfModule::load(ossModLibName.str());
    if (!pmodTrkLoader) {
        GfLogError("Cannot find %s\n", ossModLibName.str().c_str());
        GfParmReleaseHandle(CfgHandle);
        return EXIT_FAILURE;
    }

    // Check that it implements ITrackLoader.
    ITrackLoader* PiTrackLoader = pmodTrkLoader->getInterface<ITrackLoader>();
    if (!PiTrackLoader) {
        GfLogError("Cannot find ITrackLoader interface\n");
        GfParmReleaseHandle(CfgHandle);
        return EXIT_FAILURE;
    }

    // This is the track definition.
    char	trackdef[1024];
    if (TrackXMLFilePath.empty())
        snprintf(trackdef, sizeof(trackdef), "%stracks/%s/%s/%s.xml", GfDataDir(), TrackCategory.c_str(), TrackName.c_str(), TrackName.c_str());
    else
        snprintf(trackdef, sizeof(trackdef), "%s/%s.xml", TrackXMLFilePath.c_str(), TrackName.c_str());

    void *TrackHandle = GfParmReadFile(trackdef, GFPARM_RMODE_STD);
    if (!TrackHandle) {
        GfLogError("Cannot find %s\n", trackdef);
        GfParmReleaseHandle(CfgHandle);
        return EXIT_FAILURE;
    }

    // Build the track structure with graphic extensions.
    tTrack *Track = PiTrackLoader->load(trackdef, true);

    if (JustCalculate) {
        CalculateTrack(Track, TrackHandle, Bump, Raceline, Bridge);
        GfParmReleaseHandle(CfgHandle);
        GfParmReleaseHandle(TrackHandle);
        return EXIT_SUCCESS;
    }

    // dump the track segments to a file for debugging
    if (DumpTrack)
        dumpTrackSegs(Track);

    // Get the output file radix.
    char	buf2[1024];
    if (TrackACFilePath.empty())
        snprintf(buf2, sizeof(buf2), "%stracks/%s/%s/%s", GfDataDir(), Track->category, Track->internalname, Track->internalname);
    else
        snprintf(buf2, sizeof(buf2), "%s/%s", TrackACFilePath.c_str(), Track->internalname);
    std::string OutputFileName(buf2);

    // Number of groups for the complete track.
    Ac3d allAc3d;
    allAc3d.addDefaultMaterial();
    bool all = false;

    if (TrackOnly) {
        // Track.
        if (!Bump && !Raceline)
            all = true;
    } else if (MergeAll) {
        // track + terrain + objects.
        all = true;
    }

    // Main Track.
    const char *extName;
    if (Bump) {
        extName = "trk-bump";
    } else if (Raceline) {
        extName = "trk-raceline";
    } else {
        extName = "trk";
    }

    sprintf(buf2, "%s-%s.ac", OutputFileName.c_str(), extName);
    std::string OutTrackName(buf2);

    GenerateTrack(Track, TrackHandle, OutTrackName, allAc3d, all, Bump, Raceline, Bridge);

    if (TrackOnly) {
        GfParmReleaseHandle(CfgHandle);
        GfParmReleaseHandle(TrackHandle);
        return EXIT_SUCCESS;
    }

    // Terrain.
    if (MergeTerrain && !MergeAll) {
        /* terrain + objects  */
        all = true;
    }

    std::string OutMeshName(OutputFileName + "-msh.ac");

    GenerateTerrain(Track, TrackHandle, OutMeshName, allAc3d, all, DoSaveElevation, UseBorder);

    if (DoSaveElevation != -1) {
        if (all) {
            //Ac3dClose(outfd);
            allAc3d.writeFile(OutputFileName + ".ac", false);
        }
        switch (DoSaveElevation) {
            case 0:
            case 1:
                SaveElevation(Track, TrackHandle, OutputFileName + "-elv.png", OutputFileName + ".ac", 1, HeightSteps);
                if (DoSaveElevation) {
                    break;
                }
                SD_FALLTHROUGH // [[fallthrough]]
            case 2:
                SaveElevation(Track, TrackHandle, OutputFileName + "-elv2.png", OutMeshName, 1, HeightSteps);
                if (DoSaveElevation) {
                    break;
                }
                SD_FALLTHROUGH // [[fallthrough]]
            case 3:
                SaveElevation(Track, TrackHandle, OutputFileName + "-elv3.png", OutMeshName, 0, HeightSteps);
                if (DoSaveElevation) {
                    break;
                }
                SD_FALLTHROUGH // [[fallthrough]]
            case 4:
                SaveElevation(Track, TrackHandle, OutputFileName + "-elv4.png", OutTrackName, 2, HeightSteps);
                break;
        }

        GfParmReleaseHandle(TrackHandle);
        GfParmReleaseHandle(CfgHandle);

        return EXIT_SUCCESS;
    }

    GenerateObjects(Track, TrackHandle, CfgHandle, allAc3d, all, OutMeshName, OutputFileName, MultipleMaterials);

    allAc3d.writeFile(OutputFileName + ".ac", false);
    GfParmReleaseHandle(TrackHandle);
    GfParmReleaseHandle(CfgHandle);

    return EXIT_SUCCESS;
}


int main(int argc, char **argv)
{
    // Create and initialize the application
    Application app;
    app.initialize(/*bLoggingEnabled=*/true, argc, argv);

    // Parse the command line options
    if (!app.parseOptions())
        return EXIT_FAILURE;

    // If "data dir" specified in any way, cd to it.
    if (chdir(GfDataDir()))
    {
        GfLogError("Could not start %s : failed to cd to the datadir '%s' (%s)\n",
                   app.name().c_str(), GfDataDir(), strerror(errno));
        return EXIT_FAILURE;
    }

    // Do the requested job.
    return app.generate();
}
