/***************************************************************************

    file                 : glfeatures.h
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

#ifndef _GLFEATURES_H_
#define _GLFEATURES_H_

// Multi-plateform Open GL includes : use this header files when calling OpenGL

#ifdef WIN32
#  include <windows.h>
#endif //WIN32

#ifdef __APPLE__
#  include <OpenGL/gl.h>
#  include <OpenGL/glu.h>
#  include <OpenGL/glext.h>
#else //__APPLE__
#  include <GL/gl.h>
#  include <GL/glu.h>
#  include <GL/glext.h>
#endif 

#include "tgfclient.h"


/* OpenGL features interface 

   Makes the difference between "selected", "supported" and "enabled" features :
   - "selected" means that the user choosed to use the feature
     (through the OpenGL option menu or the screen.xml file),
   - "supported" means that the underlying hardware/driver actually supports the feature,
   - "enabled" means that the feature is actually enabled in the underlying hardware/driver.
   
   GfglFeatures generally doesn't automatically select features : call select() for this
   (Exceptions: MultiTexturingUnits = all available ones, MultiSamplingSamples = max level).
   GfglFeatures doesn't enables features : not done here.
   
   A feature that is not supported can not be selected (or enabled).
   A feature that is selected is not necessarily enabled (not done here).
   
   Warning: GfglFeatures::checkSupport() mustn't be used before the 1st successfull call
            to SDL_SetVideoMode() (needs an up-and-running frame buffer).
*/

class TGFCLIENT_API GfglFeatures
{
 public:
	
	// Access to the unique instance.
	static GfglFeatures& self();

	// Load selected features from the config file (default = GFSCR_CONF_FILE).
	void loadSelection(void* hparmConfig = 0);
	
	// Check supported features (ask OpenGL), and update selection as needed.
	// Warning: Must not be called before any successfull call to SDL_SetVideoMode()
	void checkSupport();

	// Store selected features to the config file (default = GFSCR_CONF_FILE).
	void storeSelection(void* hparmConfig = 0) const;
	
	// Dump selected features (in the current trace stream).
	void dumpSelection() const;
	
	// Bool-valued features.
	enum EFeatureBool
	{
		DoubleBuffer,
		TextureCompression, // GL_ARB_texture_compression
		TextureRectangle, // GL_ARB_texture_rectangle, in case mipmapping NOT needed.
		TextureNonPowerOf2, // GL_ARB_texture_non_power_of_two, in case mipmapping needed.
		MultiTexturing, // GL_ARB_multitexture
		MultiSampling // GL_ARB_multisample
	};
	void select(EFeatureBool eFeature, bool bSelected);
	bool isSelected(EFeatureBool eFeature) const;
	bool isSupported(EFeatureBool eFeature) const;
	void setSupported(EFeatureBool eFeature, bool bSupported);

	// Integer-valued features (WARNING: For the moment, -1 means "not supported").
	enum EFeatureInt
	{
		ColorDepth, AlphaDepth,
		TextureMaxSize,
		MultiTexturingUnits,
		MultiSamplingSamples
	};
	void select(EFeatureInt eFeature, int nSelectedValue);
	int getSelected(EFeatureInt eFeature) const;
	int getSupported(EFeatureInt eFeature) const;
	void setSupported(EFeatureInt eFeature, int nSupportedValue);

	// Get the pointer to the named OpenGL extension function.
	static void* getProcAddress(const char* pszName);
	
 private:
	
	GfglFeatures(); // Singleton pattern => private constructor.

 private:

	// The unique instance.
	static GfglFeatures* _pSelf;

	// Maps of supported features (bool and int-valued).
	std::map<EFeatureBool, bool> _mapSupportedBool;
	std::map<EFeatureInt, int>   _mapSupportedInt;

	// Maps of selected features (bool and int-valued).
	std::map<EFeatureBool, bool> _mapSelectedBool;
	std::map<EFeatureInt, int>   _mapSelectedInt;
};

#endif // _GLFEATURES_H_

