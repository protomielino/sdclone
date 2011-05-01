/***************************************************************************

    file                 : glfeatures.cpp
    created              : Wed Jun 1 14:56:31 CET 2005
    copyright            : (C) 2005 by Bernhard Wymann
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


#include <sstream>

#include <SDL/SDL.h>

#include "glfeatures.h"


/** Report if a given OpenGL extension is supported

    Warning: Should not be called before any successfull call to SDL_SetVideoMode()

    Note: Copied from freeGLUT 2.4.0
*/

static bool gfglIsOpenGLExtensionSupported(const char* extension)
{
  const char *extensions, *start;
  const int len = strlen(extension);

  /* TODO: Make sure there is a current window, and thus a current context available */

  if (strchr(extension, ' '))
    return false;

  start = extensions = (const char *)glGetString(GL_EXTENSIONS);

  if (!extensions)
	return false;

  while (1)
  {
     const char *p = strstr(extensions, extension);
     if (!p)
        return 0;  /* not found */
     /* check that the match isn't a super string */
     if ((p == start || p[-1] == ' ') && (p[len] == ' ' || p[len] == 0))
        return true;
     /* skip the false match and continue */
     extensions = p + len;
  }

  return false;
}

// GfglFeatures singleton --------------------------------------------------------

GfglFeatures* GfglFeatures::_pSelf = 0;

// Initialization.
GfglFeatures::GfglFeatures()
{
}

// Initialization.
GfglFeatures& GfglFeatures::self()
{
	if (!_pSelf)
		_pSelf = new GfglFeatures;

	return *_pSelf;
}

// Warning: Should not be called before any successfull call to SDL_SetVideoMode()
void GfglFeatures::checkSupport()
{
	int nValue;
	bool bValue;
	
	GfLogInfo("Video hardware info :\n");
	GfLogInfo("  Vendor   : %s\n", glGetString(GL_VENDOR));
	GfLogInfo("  Renderer : %s\n", glGetString(GL_RENDERER));
	GfLogInfo("  Version  : %s\n", glGetString(GL_VERSION));

	GfLogInfo("Supported OpenGL features :\n");

	// 1) Double-buffer : if it is supported, it is set, so we simply read it (see GfScrInit).
	SDL_GL_GetAttribute(SDL_GL_DOUBLEBUFFER, &nValue);
	_mapSupportedBool[DoubleBuffer] = nValue ? true : false;
	GfLogInfo("  Double buffer           : %s\n", nValue ? "Yes" : "No");

	// Update selection if needed.
	if (!nValue)
		_mapSelectedBool[DoubleBuffer] = false;

	// 2) Color buffer depth.
	glGetIntegerv(GL_DEPTH_BITS, &nValue);
	_mapSupportedInt[ColorDepth] = nValue;
	GfLogInfo("  Color depth             : %d bits\n", nValue);

	// Update selection if needed.
	if (getSelected(ColorDepth) > nValue)
		_mapSelectedInt[ColorDepth] = nValue;

	// 3) Alpha channel depth.
	glGetIntegerv(GL_ALPHA_BITS, &nValue);
	_mapSupportedInt[AlphaDepth] = nValue;
	GfLogInfo("  Alpha channel           : %s", nValue > 0 ? "Yes" : "No");
	if (nValue > 0)
		GfLogInfo(" (%d bits)", nValue);
	GfLogInfo("\n");

	// Update selection if needed.
	if (getSelected(AlphaDepth) > nValue)
		_mapSelectedInt[AlphaDepth] = nValue;

	// 4) Max texture size.
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &nValue);
 	if (nValue > 16384) // Max in-game supported value (must be consistent with openglconfig.cpp)
 		nValue = 16384;
	_mapSupportedInt[TextureMaxSize] = nValue;
	GfLogInfo("  Max texture size        : %d\n", nValue);

	// Update selection if needed.
	if (getSelected(TextureMaxSize) > nValue)
		_mapSelectedInt[TextureMaxSize] = nValue;
	
	// 5) Texture compression.
	//    Note: Check if at least one internal format is available. This is a workaround for
	//    driver problems and not a bugfix. According to the specification OpenGL should
	//    choose an uncompressed alternate format if it can't provide the requested
	//    compressed one... but it does not on all cards/drivers.
	bValue = gfglIsOpenGLExtensionSupported("GL_ARB_texture_compression");
	if (bValue) 
	{
		int nFormats;
		glGetIntegerv(GL_NUM_COMPRESSED_TEXTURE_FORMATS_ARB, &nFormats);
		if (nFormats == 0) 
			bValue = false;
	}
	_mapSupportedBool[TextureCompression] = bValue;
	GfLogInfo("  Texture compression     : %s\n", bValue ? "Yes" : "No");

	// Update selection if needed.
	if (!bValue)
		_mapSelectedBool[TextureCompression] = false;
	
	// 6) Multi-texturing (automatically select all the texturing units).
	bValue = gfglIsOpenGLExtensionSupported("GL_ARB_multitexture");
	glGetIntegerv(GL_MAX_TEXTURE_UNITS_ARB, &nValue);
	if (nValue < 2)
		bValue = false;
	_mapSupportedBool[MultiTexturing] = bValue;
	_mapSupportedInt[MultiTexturingUnits] = nValue;
	GfLogInfo("  Multi-texturing         : %s", bValue ? "Yes" : "No", nValue);
	if (bValue)
		GfLogInfo(" (%d units)", nValue);
	GfLogInfo("\n");

	// Update selection if needed.
	if (!bValue)
		_mapSelectedBool[MultiTexturing] = false;
	_mapSelectedInt[MultiTexturingUnits] = nValue; // Auto-select.
	
	// 7) Rectangle textures.
	bValue = gfglIsOpenGLExtensionSupported("GL_ARB_texture_rectangle");
	_mapSupportedBool[TextureRectangle] = bValue;
	GfLogInfo("  Rectangle textures      : %s\n", bValue ? "Yes" : "No");

	// Force selection.
	_mapSelectedBool[TextureRectangle] = bValue;

	// 8) Non-power-of-2 textures.
	bValue = gfglIsOpenGLExtensionSupported("GL_ARB_texture_non_power_of_two");
	_mapSupportedBool[TextureNonPowerOf2] = bValue;
	GfLogInfo("  Non power-of-2 textures : %s\n", bValue ? "Yes" : "No");
	
	// Force selection.
	_mapSelectedBool[TextureNonPowerOf2] = bValue;

	// 9) Multi-sampling = anti-aliasing
	//    Warning: Detection not done here (too late) :
	//             should already have been stored through setSupported calls.
	// bValue = gfglIsOpenGLExtensionSupported("GL_ARB_multisample");
	// if (bValue)
	// {
	// 	// Check if it is enabled.
	// 	glGetIntegerv(GL_SAMPLE_BUFFERS, &nValue);
	// 	bValue = nValue == 1;
	// 	bool bTryForced = false;
	// 	if (!bValue)
	// 	{
	// 		// Not enabled : try to enable to see if possible.
	// 		bTryForced = true;
	// 		bValue = SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1) != -1;
	// 	}
	// 	if (bValue)
	// 	{
	// 		// After forcing MULTISAMPLEBUFFERS, can't get the real SAMPLES ...
	// 		if (bTryForced)
	// 			nValue = 2; // ... so can't do better than minimum.
	// 		else
	// 			glGetIntegerv(GL_SAMPLES, &nValue);
	// 		if (nValue < 2)
	// 			bValue = false;
	// 	}

	// 	// Disable it if was not selected and we had to try it.
	// 	if (bTryForced && !isSelected(MultiSampling))
	// 		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
	// }
	// _mapSupportedBool[MultiSampling] = bValue;
	// _mapSupportedInt[MultiSamplingSamples] = nValue;
	bValue = _mapSupportedBool[MultiSampling];
	nValue = _mapSupportedInt[MultiSamplingSamples];
	GfLogInfo("  Multi-sampling          : %s", bValue ? "Yes" : "No");
	if (bValue && nValue > 1)
		GfLogInfo(" (%d samples)", nValue);
	GfLogInfo("\n");
	
	// Update selection if needed.
	if (!bValue)
		_mapSelectedBool[MultiSampling] = false;
}

// Load the selected OpenGL features from the config file.
void GfglFeatures::loadSelection(void* hparmConfig)
{
	// Open the config file if not already done.
	void* hparm;
	if (!hparmConfig)
	{
		std::ostringstream ossParm;
		ossParm << GfLocalDir() << GFSCR_CONF_FILE;
		hparm = GfParmReadFile(ossParm.str().c_str(), GFPARM_RMODE_STD | GFPARM_RMODE_CREAT);
	}
	else
	{
		hparm = hparmConfig;
	}

	// Read the OpenGL configuration from screen.xml (or hard coded values).
	// and select the OpenGL features accordingly
	// (by default, select the max possible values ; we'll see later through checkSupport
	//  if we really can).

	// 1) Double-buffer : no choice, hard-coded.
	_mapSelectedBool[DoubleBuffer] = true;

	// 2) Color buffer depth : no choice, hard-coded.
	_mapSelectedInt[ColorDepth] = 24;

	// 3) Alpha-channel depth : no choice, hard-coded.
	_mapSelectedInt[AlphaDepth] = 8;

	// 4) Max texture size : load from config file.
	const int nTSize =
		(int)GfParmGetNum(hparm, GfSCR_SECT_GLFEATURES, GfSCR_ATT_MAXTEXTURESIZE,
						  (char*)NULL, (tdble)1024);
	_mapSelectedInt[TextureMaxSize] = nTSize;

	// 5) Texture compression : load from config file.
	const std::string strTexComp =
		GfParmGetStr(hparm, GfSCR_SECT_GLFEATURES, GfSCR_ATT_TEXTURECOMPRESSION,
					 GfSCR_ATT_TEXTURECOMPRESSION_ENABLED);
	_mapSelectedBool[TextureCompression] = strTexComp == GfSCR_ATT_TEXTURECOMPRESSION_ENABLED;

	// 6) Multi-texturing : load from config file.
	const std::string strMultiTex =
		GfParmGetStr(hparm, GfSCR_SECT_GLFEATURES, GfSCR_ATT_MULTITEXTURING,
					 GfSCR_ATT_MULTITEXTURING_ENABLED);
	_mapSelectedBool[MultiTexturing] = strMultiTex == GfSCR_ATT_MULTITEXTURING_ENABLED;

	// 7) Rectangle textures : no choice, selected = supported (see checkSupport).

	// 8) Non-power-of-2 textures : no choice, selected = supported (see checkSupport).

	// 9) Multi-sampling : load from config file (but Buffers from actually supported value).
	const std::string strMultiSamp =
		GfParmGetStr(hparm, GfSCR_SECT_GLFEATURES, GfSCR_ATT_MULTISAMPLING,
					 GfSCR_ATT_MULTISAMPLING_ENABLED);
	_mapSelectedBool[MultiSampling] = strMultiSamp == GfSCR_ATT_MULTISAMPLING_ENABLED;
	const int nSamples =
		(int)GfParmGetNum(hparm, GfSCR_SECT_GLFEATURES, GfSCR_ATT_MULTISAMPLING_SAMPLES,
						  (char*)NULL, (tdble)8); // Good but reasonable value.
	_mapSelectedInt[MultiSamplingSamples] = nSamples;

	// Close params.
	if (!hparmConfig)
		GfParmReleaseHandle(hparm);
}


// Save settings to screen.xml
void GfglFeatures::storeSelection(void* hparmConfig) const
{
	// Display what we have selected.
	dumpSelection();

	// Open the config file if not already done.
	void* hparm;
	if (!hparmConfig)
	{
		std::ostringstream ossParm;
		ossParm << GfLocalDir() << GFSCR_CONF_FILE;
		hparm = GfParmReadFile(ossParm.str().c_str(), GFPARM_RMODE_STD | GFPARM_RMODE_CREAT);
	}
	else
	{
		hparm = hparmConfig;
	}

	// Write new values.
	GfParmSetStr(hparm, GfSCR_SECT_GLFEATURES, GfSCR_ATT_TEXTURECOMPRESSION,
				 isSelected(TextureCompression)
				 ? GfSCR_ATT_TEXTURECOMPRESSION_ENABLED : GfSCR_ATT_TEXTURECOMPRESSION_DISABLED);
	GfParmSetNum(hparm, GfSCR_SECT_GLFEATURES, GfSCR_ATT_MAXTEXTURESIZE, (char*)NULL,
				 (tdble)getSelected(TextureMaxSize));
	GfParmSetStr(hparm, GfSCR_SECT_GLFEATURES, GfSCR_ATT_MULTITEXTURING,
				 isSelected(MultiTexturing)
				 ? GfSCR_ATT_MULTITEXTURING_ENABLED : GfSCR_ATT_MULTITEXTURING_DISABLED);
	GfParmSetStr(hparm, GfSCR_SECT_GLFEATURES, GfSCR_ATT_MULTISAMPLING,
				 isSelected(MultiSampling)
				 ? GfSCR_ATT_MULTISAMPLING_ENABLED : GfSCR_ATT_MULTISAMPLING_DISABLED);
	GfParmSetNum(hparm, GfSCR_SECT_GLFEATURES, GfSCR_ATT_MULTISAMPLING_SAMPLES, (char*)NULL,
				 (tdble)getSelected(MultiSamplingSamples));

	// Force 'best possible' mode for video initialization when anti-aliasing selected
	// (we can only activate this at GfScrInit time, and only this mode try and activates it).
	if (isSelected(MultiSampling))
		GfParmSetStr(hparm, GFSCR_SECT_PROP, GFSCR_ATT_VINIT, GFSCR_VAL_VINIT_BEST);
	
	// Write new params to disk.
	GfParmWriteFile(NULL, hparm, "Screen");
	
	// Close params.
	if (!hparmConfig)
		GfParmReleaseHandle(hparm);
}

void GfglFeatures::dumpSelection() const
{
	GfLogInfo("Selected OpenGL features :\n");
	GfLogInfo("  Double buffer           : %s\n", isSelected(DoubleBuffer) ? "On" : "Off");
	GfLogInfo("  Color depth             : %d bits\n", getSelected(ColorDepth));
	GfLogInfo("  Alpha channel           : %s",
			  getSelected(AlphaDepth) > 0 ? "On" : "Off");
	if (getSelected(AlphaDepth) > 0)
		GfLogInfo(" (%d bits)", getSelected(AlphaDepth));
	GfLogInfo("\n");
	GfLogInfo("  Max texture size        : %d\n", getSelected(TextureMaxSize));
	GfLogInfo("  Texture compression     : %s\n", isSelected(TextureCompression) ? "On" : "Off");
	GfLogInfo("  Multi-texturing         : %s", isSelected(MultiTexturing) ? "On" : "Off");
	if (isSelected(MultiTexturing))
		GfLogInfo(" (%d units)", getSelected(MultiTexturingUnits));
	GfLogInfo("\n");
	GfLogInfo("  Rectangle textures      : %s\n", isSelected(TextureRectangle) ? "On" : "Off");
	GfLogInfo("  Non power-of-2 textures : %s\n", isSelected(TextureNonPowerOf2) ? "On" : "Off");
	GfLogInfo("  Multi-sampling          : %s", isSelected(MultiSampling) ? "On" : "Off");
	if (isSelected(MultiSampling))
		GfLogInfo(" (%d samples)", getSelected(MultiSamplingSamples));
	GfLogInfo("\n");
}

bool GfglFeatures::isSelected(EFeatureBool eFeature) const
{
	const std::map<EFeatureBool, bool>::const_iterator itFeature =
		_mapSelectedBool.find(eFeature);
	return itFeature == _mapSelectedBool.end() ? false : itFeature->second;
}

void GfglFeatures::setSupported(EFeatureBool eFeature, bool bSupported)
{
	_mapSupportedBool[eFeature] = bSupported;
	if (!bSupported && isSelected(eFeature))
		_mapSelectedBool[eFeature] = bSupported;
}

bool GfglFeatures::isSupported(EFeatureBool eFeature) const
{
	const std::map<EFeatureBool, bool>::const_iterator itFeature =
		_mapSupportedBool.find(eFeature);
	return itFeature == _mapSupportedBool.end() ? false : itFeature->second;
}

void GfglFeatures::select(EFeatureBool eFeature, bool bSelected)
{
	if (!bSelected || isSupported(eFeature))
		_mapSelectedBool[eFeature] = bSelected;
// 	GfLogDebug("GfglFeatures::select(Bool:%d, %s) : supp=%s, new=%s\n",
// 			   (int)eFeature, bSelected ? "true" : "false",
// 			   isSupported(eFeature) ? "true" : "false",
// 			   _mapSelectedBool[eFeature] ? "true" : "false");
}

int GfglFeatures::getSelected(EFeatureInt eFeature) const
{
	const std::map<EFeatureInt, int>::const_iterator itFeature =
		_mapSelectedInt.find(eFeature);
	return itFeature == _mapSelectedInt.end() ? -1 : itFeature->second;
}

void GfglFeatures::setSupported(EFeatureInt eFeature, int nSupportedValue)
{
	_mapSupportedInt[eFeature] = nSupportedValue;
	if (nSupportedValue < getSelected(eFeature))
		_mapSelectedInt[eFeature] = nSupportedValue;
}

int GfglFeatures::getSupported(EFeatureInt eFeature) const
{
	const std::map<EFeatureInt, int>::const_iterator itFeature =
		_mapSupportedInt.find(eFeature);
	return itFeature == _mapSupportedInt.end() ? -1 : itFeature->second;
}

void GfglFeatures::select(EFeatureInt eFeature, int nSelectedValue)
{
	if (nSelectedValue > getSupported(eFeature))
		nSelectedValue = getSupported(eFeature);
	_mapSelectedInt[eFeature] = nSelectedValue;
// 	GfLogDebug("GfglFeatures::select(Int:%d, %d) : supp=%s, new=%d\n",
// 			   (int)eFeature, nSelectedValue, getSupported(eFeature) >= 0 ? "true" : "false",
// 			   _mapSelectedInt[eFeature]);
}

void* GfglFeatures::getProcAddress(const char* pszName)
{
	return SDL_GL_GetProcAddress(pszName);
}
