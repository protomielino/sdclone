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
	checkSupport();
}

// Initialization.
GfglFeatures* GfglFeatures::self()
{
	if (!_pSelf)
		_pSelf = new GfglFeatures;

	return _pSelf;
}

void GfglFeatures::setSelectionLoader(void (*funcLoad)())
{
	_funcLoadSelection = funcLoad;
}

void GfglFeatures::setSelectionStorer(void (*funcStore)())
{
	_funcStoreSelection = funcStore;
}

void GfglFeatures::checkSupport()
{
	int nValue;
	bool bValue;
	
	GfLogInfo("Supported OpenGL features :\n");

	// a) Max texture size.
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &nValue);
 	if (nValue > 16384) // Max in-game supported value (must be consistent with openglconfig.cpp)
 		nValue = 16384;
	_mapSupportedInt[TextureMaxSize] = nValue;
	GfLogInfo("  Max texture size        : %d\n", nValue);

	// b) Texture compression.
	//    Note: Check if at least one internal format is vailable. This is a workaround for
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
	
	// c) Multi-texturing (automatically select all the texturing units).
	bValue = gfglIsOpenGLExtensionSupported("GL_ARB_multitexture");
	glGetIntegerv(GL_MAX_TEXTURE_UNITS_ARB, &nValue);
	if (nValue < 2)
		bValue = false;
	_mapSupportedBool[MultiTexturing] = bValue;
	GfLogInfo("  Multi-texturing         : %s\n", bValue ? "Yes" : "No");
	_mapSupportedInt[MultiTexturingUnits] = nValue;
	GfLogInfo("  Multi-texturing units   : %d\n", nValue);

	_mapSelectedInt[MultiTexturingUnits] = nValue; // Auto-select.
	
	// d) Rectangle textures.
	bValue = gfglIsOpenGLExtensionSupported("GL_ARB_texture_rectangle");
	_mapSupportedBool[TextureRectangle] = bValue;
	GfLogInfo("  Rectangle textures      : %s\n", bValue ? "Yes" : "No");

	// e) Non-power-of-2 textures.
	bValue = gfglIsOpenGLExtensionSupported("GL_ARB_texture_non_power_of_two");
	_mapSupportedBool[TextureNonPowerOf2] = bValue;
	GfLogInfo("  Non power-of-2 textures : %s\n", bValue ? "Yes" : "No");
	
	// f) Multi-sampling (anti-aliasing).
	bValue = gfglIsOpenGLExtensionSupported("GL_ARB_multisample");
	if (bValue) 
	{
		// Work-in-progress.
		// TODO: More checks needed : number of samples
		bValue = false;
	}
	_mapSupportedBool[MultiSampling] = bValue;
	GfLogInfo("  Multi-sampling          : %s (not yet implemented)\n", bValue ? "Yes" : "No");
	
}

void GfglFeatures::loadSelection()
{
	if (_funcLoadSelection)
		_funcLoadSelection();
	dumpSelection();
}

void GfglFeatures::storeSelection() const
{
	dumpSelection();
	if (_funcStoreSelection)
		_funcStoreSelection();
}

void GfglFeatures::dumpSelection() const
{
	GfLogInfo("Selected OpenGL features :\n");
	GfLogInfo("  Max texture size      : %d\n", getSelected(TextureMaxSize));
	GfLogInfo("  Texture compression   : %s\n", isSelected(TextureCompression) ? "On" : "Off");
	GfLogInfo("  Multi-texturing       : %s\n", isSelected(MultiTexturing) ? "On" : "Off");
	GfLogInfo("  Multi-texturing units : %d\n", getSelected(MultiTexturingUnits));
	GfLogInfo("  Multi-sampling        : %s\n", isSelected(MultiSampling)  ? "Yes" : "No");
}

bool GfglFeatures::isSelected(EFeatureBool eFeature) const
{
	const std::map<EFeatureBool, bool>::const_iterator itFeature =
		_mapSelectedBool.find(eFeature);
	return itFeature == _mapSelectedBool.end() ? false : itFeature->second;
}

bool GfglFeatures::isSupported(EFeatureBool eFeature) const
{
	const std::map<EFeatureBool, bool>::const_iterator itFeature =
		_mapSupportedBool.find(eFeature);
	return itFeature == _mapSupportedBool.end() ? false : itFeature->second;
}

void GfglFeatures::select(EFeatureBool eFeature, bool bSelected)
{
	if (isSupported(eFeature))
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

int GfglFeatures::getSupported(EFeatureInt eFeature) const
{
	const std::map<EFeatureInt, int>::const_iterator itFeature =
		_mapSupportedInt.find(eFeature);
	return itFeature == _mapSupportedInt.end() ? -1 : itFeature->second;
}

void GfglFeatures::select(EFeatureInt eFeature, int nSelectedValue)
{
	if (getSupported(eFeature) != -1)
		_mapSelectedInt[eFeature] = nSelectedValue;
// 	GfLogDebug("GfglFeatures::select(Int:%d, %d) : supp=%s, new=%d\n",
// 			   (int)eFeature, nSelectedValue, getSupported(eFeature) >= 0 ? "true" : "false",
// 			   _mapSelectedInt[eFeature]);
}

void* GfglFeatures::getProcAddress(const char* pszName)
{
	return SDL_GL_GetProcAddress(pszName);
}
