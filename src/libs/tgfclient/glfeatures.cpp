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
#include <limits>

#include <SDL/SDL.h>

#include "glfeatures.h"

// Avoid C lib <cstdlib> "max" to overload <limits> ones.
#undef min


static const char* pszNoUnit = 0;


/** Report if a given OpenGL extension is supported

    Warning: Should not be called before any successfull call to SDL_SetVideoMode()

    Note: Copied from freeGLUT 2.4.0
*/

static bool gfglIsOpenGLExtensionSupported(const char* extension)
{
  const char *extensions, *start;
  const int len = strlen(extension);

  // TODO: Make sure there is a current window, and thus a current context available

  if (strchr(extension, ' '))
    return false;

  start = extensions = (const char *)glGetString(GL_EXTENSIONS);

  if (!extensions)
	return false;

  while (true)
  {
     const char *p = strstr(extensions, extension);
     if (!p)
		 return 0;  // Not found
	 
     // Check that the match isn't a super string
     if ((p == start || p[-1] == ' ') && (p[len] == ' ' || p[len] == 0))
        return true;
	 
     // Skip the false match and continue
     extensions = p + len;
  }

  return false;
}

// GfglFeatures singleton --------------------------------------------------------

// Initialization.
GfglFeatures* GfglFeatures::_pSelf = 0;

GfglFeatures::GfglFeatures()
//: hparmConfig(0)
{
}

GfglFeatures& GfglFeatures::self()
{
	if (!_pSelf)
		_pSelf = new GfglFeatures;

	return *_pSelf;
}

// Values for the "not supported" / "not selected" numerical cases.
int GfglFeatures::InvalidInt = std::numeric_limits<int>::min();

// Config file management.
void* GfglFeatures::openConfigFile()
{
	std::ostringstream ossParm;
	ossParm << GfLocalDir() << GFSCR_CONF_FILE;

	return GfParmReadFile(ossParm.str().c_str(), GFPARM_RMODE_STD | GFPARM_RMODE_CREAT);
}

void GfglFeatures::closeConfigFile(void* hparmConfig, bool bWrite)
{
	// Write if specified.
	if (bWrite)
		GfParmWriteFile(NULL, hparmConfig, "Screen");
	
	// Close.
	GfParmReleaseHandle(hparmConfig);
}

// Standard supported features detection.
void GfglFeatures::detectStandardSupport()
{
	// 1) Double-buffer.
	int nValue;
	SDL_GL_GetAttribute(SDL_GL_DOUBLEBUFFER, &nValue);
	_mapSupportedBool[DoubleBuffer] = nValue ? true : false;

	// 2) Color buffer depth.
	SDL_GL_GetAttribute(SDL_GL_DEPTH_SIZE, &nValue);
	//glGetIntegerv(GL_DEPTH_BITS, &nValue);
	_mapSupportedInt[ColorDepth] = nValue;

	// 3) Alpha channel depth.
	SDL_GL_GetAttribute(SDL_GL_ALPHA_SIZE, &nValue);
	//glGetIntegerv(GL_ALPHA_BITS, &nValue);
	_mapSupportedInt[AlphaDepth] = nValue;

	// 4) Max texture size.
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &nValue);
 	if (nValue > 16384) // Max in-game supported value (must be consistent with openglconfig.cpp)
 		nValue = 16384;
	_mapSupportedInt[TextureMaxSize] = nValue;

	// 5) Texture compression.
	//    Note: Check if at least one internal format is available. This is a workaround for
	//    driver problems and not a bugfix. According to the specification OpenGL should
	//    choose an uncompressed alternate format if it can't provide the requested
	//    compressed one... but it does not on all cards/drivers.
	bool bValue = gfglIsOpenGLExtensionSupported("GL_ARB_texture_compression");
	if (bValue) 
	{
		int nFormats;
		glGetIntegerv(GL_NUM_COMPRESSED_TEXTURE_FORMATS_ARB, &nFormats);
		if (nFormats == 0) 
			bValue = false;
	}
	_mapSupportedBool[TextureCompression] = bValue;

	// 6) Multi-texturing (automatically select all the texturing units).
	bValue = gfglIsOpenGLExtensionSupported("GL_ARB_multitexture");
	glGetIntegerv(GL_MAX_TEXTURE_UNITS_ARB, &nValue);
	if (nValue < 2)
		bValue = false;
	_mapSupportedBool[MultiTexturing] = bValue;
	if (bValue)
		_mapSupportedInt[MultiTexturingUnits] = nValue;

	// 7) Rectangle textures.
	_mapSupportedBool[TextureRectangle] =
		gfglIsOpenGLExtensionSupported("GL_ARB_texture_rectangle");

	// 8) Non-power-of-2 textures.
	_mapSupportedBool[TextureNonPowerOf2] =
		gfglIsOpenGLExtensionSupported("GL_ARB_texture_non_power_of_two");
}

// Best supported features detection for the given specs of the frame buffer.
bool GfglFeatures::detectBestSupport(int& nWidth, int& nHeight, int& nDepth,
									 bool& bAlpha, bool& bFullScreen)
{
	GfLogInfo("Detecting best supported features for a %dx%dx%d%s frame buffer.\n",
			  nWidth, nHeight, nDepth, bFullScreen ? " full-screen" : "");

	// I) Detection of the max possible values for requested features.
	//    (to do that, we need to try setting up the video modes for real).
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	SDL_Surface* pWinSurface = 0;

	int nAlphaChannel;
	int nCurrDepth;
	int nFullScreen = bFullScreen ? 1 : 0;
	while (!pWinSurface && nFullScreen >= 0)
	{
		GfLogTrace("Trying %s mode\n", nFullScreen ? "full-screen" : "windowed");
		const int bfVideoMode = SDL_OPENGL | (nFullScreen ? SDL_FULLSCREEN : 0);

		nAlphaChannel = bAlpha ? 1 : 0;
		while (!pWinSurface && nAlphaChannel >= 0)
		{
			GfLogTrace("Trying with%s alpha channel\n", nAlphaChannel ? "" : "out");
			nCurrDepth = nDepth;
			while (!pWinSurface && nCurrDepth >= 16)
			{
				GfLogTrace("Trying %d bits RVB+A color depth\n", nCurrDepth);
				SDL_GL_SetAttribute(SDL_GL_RED_SIZE, nCurrDepth/4);
				SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, nCurrDepth/4);
				SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, nCurrDepth/4);
				SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, (3*nCurrDepth)/4);
				SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, nAlphaChannel ? nCurrDepth/4 : 0);
		
				// Anti-aliasing : detect the max supported number of samples
				// (assumed to be <= 32).
#ifndef WIN32
				SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
#endif
				int nMaxMultiSamples = 32; // Hard coded max value for the moment.
				while (!pWinSurface && nMaxMultiSamples > 1)
				{
					// Set the anti-aliasing attributes and setup the video mode.
#ifdef WIN32
					SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
#endif
					SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, nMaxMultiSamples);
					pWinSurface = SDL_SetVideoMode(nWidth, nHeight, nCurrDepth, bfVideoMode);

					// Now check if we have a video mode, and if it actually features
					// what we specified.
#ifdef WIN32
					int nActualSampleBuffers = 0;
					SDL_GL_GetAttribute(SDL_GL_MULTISAMPLEBUFFERS, &nActualSampleBuffers);
					int nActualMultiSamples = 0;
					SDL_GL_GetAttribute(SDL_GL_MULTISAMPLESAMPLES, &nActualMultiSamples);
// 					GfLogDebug("nMaxMultiSamples=%d : nActualSampleBuffers=%d, nActualMultiSamples=%d\n",
// 							   nMaxMultiSamples, nActualSampleBuffers, nActualMultiSamples);

					// If not, try a lower number of samples.
					if (nActualSampleBuffers == 0 || nActualMultiSamples != nMaxMultiSamples)
						pWinSurface = 0;
#endif
					if (!pWinSurface)
					{
						GfLogTrace("%d+%d bit %dx anti-aliased double-buffer not supported\n",
								   3*nCurrDepth/4, nCurrDepth/4, nMaxMultiSamples);
						nMaxMultiSamples /= 2;
					}
				}
				
				// Failed : try without anti-aliasing.
				if (!pWinSurface)
				{
					SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
					SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);
					pWinSurface = SDL_SetVideoMode(nWidth, nHeight, nCurrDepth, bfVideoMode);
					if (!pWinSurface)
						GfLogTrace("%d+%d bit double-buffer not supported\n",
								   3*nCurrDepth/4, nCurrDepth/4);
				}

				// Failed : try with lower color depth.
				if (!pWinSurface)
					nCurrDepth -= 8;
			}

			// Failed : try without alpha channel if not already done
			// (Note: it this really relevant ?).
			if (!pWinSurface)
				nAlphaChannel--;
		}

		// Failed : try a windowed mode if not already done.
		if (!pWinSurface)
			nFullScreen--;
	}

	// Failed : no more idea :-(
	if (!pWinSurface)
	{
		// Reset support data (will result in emptying the section when storing,
		// thus forcing new detection when checkSupport will be called again).
		_mapSupportedBool.clear();
		_mapSupportedInt.clear();
		
		GfLogError("No supported 'best' video mode found for a %dx%dx%d%s frame buffer.\n",
				   nWidth, nHeight, nDepth, bFullScreen ? " full-screen" : "");
		
		return false;
	}	
	
	// II) Read-out what we have from the up-and-running frame buffer
	//     and set "supported" values accordingly.
	
	// 1) Standard features.
	detectStandardSupport();
		
	// 2) Multi-sampling = anti-aliasing
	int nValue;
	SDL_GL_GetAttribute(SDL_GL_MULTISAMPLEBUFFERS, &nValue);
	_mapSupportedBool[MultiSampling] = nValue != 0;
	//GfLogDebug("SDL_GL_GetAttribute(SDL_GL_MULTISAMPLEBUFFERS) = %d\n", nValue);
	if (nValue)
	{
		SDL_GL_GetAttribute(SDL_GL_MULTISAMPLESAMPLES, &nValue);
		//GfLogDebug("SDL_GL_GetAttribute(SDL_GL_MULTISAMPLESAMPLES) = %d\n", nValue);
		if (nValue > 1)
			_mapSupportedInt[MultiSamplingSamples] = nValue;
		else
			_mapSupportedBool[MultiSampling] = false;
	}

	// III) Return the updated frame buffer specs.
	//nWidth = nWidth; // Unchanged.
	//nHeight = nHeight; // Unchanged.
	nDepth = nCurrDepth;
	bFullScreen = nFullScreen ? true : false;
	bAlpha = nAlphaChannel ? true : false;
	
	return true;
}

bool GfglFeatures::loadSupport(int &nWidth, int &nHeight, int &nDepth,
							   bool &bAlpha, bool &bFullScreen, void* hparmConfig)
{
	// Clear support data.
	_mapSupportedBool.clear();
	_mapSupportedInt.clear();

	// Open the config file if not already done.
	void* hparm = hparmConfig ? hparmConfig : openConfigFile();

	// Load the frame buffer specs for the stored supported features.
    nWidth =
		(int)GfParmGetNum(hparm, GFSCR_SECT_GLDETSPECS, GFSCR_ATT_WIN_X, pszNoUnit, 0);
    nHeight =
		(int)GfParmGetNum(hparm, GFSCR_SECT_GLDETSPECS, GFSCR_ATT_WIN_Y, pszNoUnit, 0);
    nDepth =
		(int)GfParmGetNum(hparm, GFSCR_SECT_GLDETSPECS, GFSCR_ATT_BPP, pszNoUnit, 0);
    bAlpha =
		std::string(GfParmGetStr(hparm, GFSCR_SECT_GLDETSPECS, GFSCR_ATT_ALPHACHANNEL, GFSCR_VAL_NO))
		== GFSCR_VAL_YES;
    bFullScreen =
		std::string(GfParmGetStr(hparm, GFSCR_SECT_GLDETSPECS, GFSCR_ATT_FSCR, GFSCR_VAL_NO))
		== GFSCR_VAL_YES;

	// Check that we have something supported, and return if not.
	if (nWidth == 0 || nHeight == 0 || nDepth == 0)
	{
		GfLogTrace("No info found about best supported features for these specs ; "
				   "will need a detection pass.\n");

		// Close config file if we open it.
		if (!hparmConfig)
			closeConfigFile(hparm);

		return false;
	}
		
	
	// Here, we only update _mapSupportedXXX only if something relevant in the config file
	// If there's nothing or something not expected, it means no support.
	
	// 1) Double-buffer.
	const std::string strDoubleBuffer =
		GfParmGetStr(hparm, GFSCR_SECT_GLDETFEATURES, GFSCR_ATT_DOUBLEBUFFER, "");
	if (strDoubleBuffer == GFSCR_VAL_YES)
		_mapSupportedBool[DoubleBuffer] = true;
	else if (strDoubleBuffer == GFSCR_VAL_NO)
		_mapSupportedBool[DoubleBuffer] = false;

	// 2) Color buffer depth.
	const int nColorDepth =
		(int)GfParmGetNum(hparm, GFSCR_SECT_GLDETFEATURES, GFSCR_ATT_COLORDEPTH,
						  pszNoUnit, (tdble)0);
	if (nColorDepth > 0)
		_mapSupportedInt[ColorDepth] = nColorDepth;

	// 3) Alpha-channel depth.
	const int nAlphaDepth =
		(int)GfParmGetNum(hparm, GFSCR_SECT_GLDETFEATURES, GFSCR_ATT_ALPHADEPTH,
						  pszNoUnit, (tdble)-1);
	if (nAlphaDepth >= 0)
		_mapSupportedInt[AlphaDepth] = nAlphaDepth;

	// 4) Max texture size.
	const int nMaxTexSize =
		(int)GfParmGetNum(hparm, GFSCR_SECT_GLDETFEATURES, GFSCR_ATT_MAXTEXTURESIZE,
						  pszNoUnit, (tdble)0);
	if (nMaxTexSize > 0)
		_mapSupportedInt[TextureMaxSize] = nMaxTexSize;

	// 5) Texture compression.
	const std::string strTexComp =
		GfParmGetStr(hparm, GFSCR_SECT_GLDETFEATURES, GFSCR_ATT_TEXTURECOMPRESSION, "");
	if (strTexComp == GFSCR_VAL_YES)
		_mapSupportedBool[TextureCompression] = true;
	else if (strTexComp == GFSCR_VAL_NO)
		_mapSupportedBool[TextureCompression] = false;

	// 6) Multi-texturing.
	const std::string strMultiTex =
		GfParmGetStr(hparm, GFSCR_SECT_GLDETFEATURES, GFSCR_ATT_MULTITEXTURING, "");
	if (strMultiTex == GFSCR_VAL_YES)
		_mapSupportedBool[MultiTexturing] = true;
	else if (strMultiTex == GFSCR_VAL_NO)
		_mapSupportedBool[MultiTexturing] = false;

	const int nMultiTexUnits =
		(int)GfParmGetNum(hparm, GFSCR_SECT_GLDETFEATURES, GFSCR_ATT_MULTITEXTURINGUNITS,
						  pszNoUnit, (tdble)0);
	if (nMultiTexUnits > 0)
		_mapSupportedInt[MultiTexturingUnits] = nMultiTexUnits;

	// 7) Rectangle textures).
	const std::string strRectTex =
		GfParmGetStr(hparm, GFSCR_SECT_GLDETFEATURES, GFSCR_ATT_RECTANGLETEXTURES, "");
	if (strRectTex == GFSCR_VAL_YES)
		_mapSupportedBool[TextureRectangle] = true;
	else if (strRectTex == GFSCR_VAL_NO)
		_mapSupportedBool[TextureRectangle] = false;

	// 8) Non-power-of-2 textures.
	const std::string strNonPoTTex =
		GfParmGetStr(hparm, GFSCR_SECT_GLDETFEATURES, GFSCR_ATT_NONPOTTEXTURES, "");
	if (strNonPoTTex == GFSCR_VAL_YES)
		_mapSupportedBool[TextureNonPowerOf2] = true;
	else if (strNonPoTTex == GFSCR_VAL_NO)
		_mapSupportedBool[TextureNonPowerOf2] = false;

	// 9) Multi-sampling.
	const std::string strMultiSamp =
		GfParmGetStr(hparm, GFSCR_SECT_GLDETFEATURES, GFSCR_ATT_MULTISAMPLING, "");
	if (strMultiSamp == GFSCR_VAL_YES)
		_mapSupportedBool[MultiSampling] = true;
	else if (strMultiSamp == GFSCR_VAL_NO)
		_mapSupportedBool[MultiSampling] = false;
	
	const int nMultiSampSamples =
		(int)GfParmGetNum(hparm, GFSCR_SECT_GLDETFEATURES, GFSCR_ATT_MULTISAMPLINGSAMPLES,
						  pszNoUnit, (tdble)0);
	if (nMultiSampSamples > 0)
		_mapSupportedInt[MultiSamplingSamples] = nMultiSampSamples;

	// Close config file if we open it.
	if (!hparmConfig)
		closeConfigFile(hparm);

	// Trace best supported features.
	dumpSupport();

	return true;
}

void GfglFeatures::storeSupport(int nWidth, int nHeight, int nDepth,
								bool bAlpha, bool bFullScreen, void* hparmConfig)
{
	// Open the config file if not already done.
	void* hparm = hparmConfig ? hparmConfig : openConfigFile();

	// If there's support for nothing, remove all.
	if (_mapSupportedBool.empty() && _mapSupportedInt.empty())
	{
		// Frame buffer specs.
		GfParmRemove(hparm, GFSCR_SECT_GLDETSPECS, GFSCR_ATT_WIN_X);
		GfParmRemove(hparm, GFSCR_SECT_GLDETSPECS, GFSCR_ATT_WIN_Y);
		GfParmRemove(hparm, GFSCR_SECT_GLDETSPECS, GFSCR_ATT_BPP);
		GfParmRemove(hparm, GFSCR_SECT_GLDETSPECS, GFSCR_ATT_ALPHACHANNEL);
		GfParmRemove(hparm, GFSCR_SECT_GLDETSPECS, GFSCR_ATT_FSCR);
	
		// Supported values.
		// 1) Double-buffer.
		GfParmRemove(hparm, GFSCR_SECT_GLDETFEATURES, GFSCR_ATT_DOUBLEBUFFER);

		// 2) Color buffer depth.
		GfParmRemove(hparm, GFSCR_SECT_GLDETFEATURES, GFSCR_ATT_COLORDEPTH);

		// 3) Alpha-channel depth.
		GfParmRemove(hparm, GFSCR_SECT_GLDETFEATURES, GFSCR_ATT_ALPHADEPTH);

		// 4) Max texture size.
		GfParmRemove(hparm, GFSCR_SECT_GLDETFEATURES, GFSCR_ATT_MAXTEXTURESIZE);
		
		// 5) Texture compression.
		GfParmRemove(hparm, GFSCR_SECT_GLDETFEATURES, GFSCR_ATT_TEXTURECOMPRESSION);
		
		// 6) Multi-texturing.
		GfParmRemove(hparm, GFSCR_SECT_GLDETFEATURES, GFSCR_ATT_MULTITEXTURING);
		GfParmRemove(hparm, GFSCR_SECT_GLDETFEATURES, GFSCR_ATT_MULTITEXTURINGUNITS);

		// 7) Rectangle textures).
		GfParmRemove(hparm, GFSCR_SECT_GLDETFEATURES, GFSCR_ATT_RECTANGLETEXTURES);

		// 8) Non-power-of-2 textures.
		GfParmRemove(hparm, GFSCR_SECT_GLDETFEATURES, GFSCR_ATT_NONPOTTEXTURES);

		// 9) Multi-sampling.
		GfParmRemove(hparm, GFSCR_SECT_GLDETFEATURES, GFSCR_ATT_MULTISAMPLING);
		GfParmRemove(hparm, GFSCR_SECT_GLDETFEATURES, GFSCR_ATT_MULTISAMPLINGSAMPLES);
	}

	// If there's support for anything, store it.
	else
	{
		// Write new frame buffer specs for the stored supported features.
		GfParmSetNum(hparm, GFSCR_SECT_GLDETSPECS, GFSCR_ATT_WIN_X, pszNoUnit,
					 (tdble)nWidth);
		GfParmSetNum(hparm, GFSCR_SECT_GLDETSPECS, GFSCR_ATT_WIN_Y, pszNoUnit,
					 (tdble)nHeight);
		GfParmSetNum(hparm, GFSCR_SECT_GLDETSPECS, GFSCR_ATT_BPP, pszNoUnit,
					 (tdble)nDepth);
		GfParmSetStr(hparm, GFSCR_SECT_GLDETSPECS, GFSCR_ATT_ALPHACHANNEL,
					 bAlpha ? GFSCR_VAL_YES : GFSCR_VAL_NO);
		GfParmSetStr(hparm, GFSCR_SECT_GLDETSPECS, GFSCR_ATT_FSCR,
					 bFullScreen ? GFSCR_VAL_YES : GFSCR_VAL_NO);
	
		// Write new values (remove the ones with no value supported).
		// 1) Double-buffer.
		GfParmSetStr(hparm, GFSCR_SECT_GLDETFEATURES, GFSCR_ATT_DOUBLEBUFFER,
					 isSupported(DoubleBuffer) ? GFSCR_VAL_YES : GFSCR_VAL_NO);

		// 2) Color buffer depth.
		if (getSupported(ColorDepth) != InvalidInt)
			GfParmSetNum(hparm, GFSCR_SECT_GLDETFEATURES, GFSCR_ATT_COLORDEPTH, pszNoUnit,
						 (tdble)getSupported(ColorDepth));
		else
			GfParmRemove(hparm, GFSCR_SECT_GLDETFEATURES, GFSCR_ATT_COLORDEPTH);

		// 3) Alpha-channel depth.
		if (getSupported(AlphaDepth) != InvalidInt)
			GfParmSetNum(hparm, GFSCR_SECT_GLDETFEATURES, GFSCR_ATT_ALPHADEPTH, pszNoUnit,
						 (tdble)getSupported(AlphaDepth));
		else
			GfParmRemove(hparm, GFSCR_SECT_GLDETFEATURES, GFSCR_ATT_ALPHADEPTH);

		// 4) Max texture size.
		if (getSupported(TextureMaxSize) != InvalidInt)
			GfParmSetNum(hparm, GFSCR_SECT_GLDETFEATURES, GFSCR_ATT_MAXTEXTURESIZE, pszNoUnit,
						 (tdble)getSupported(TextureMaxSize));
		else
			GfParmRemove(hparm, GFSCR_SECT_GLDETFEATURES, GFSCR_ATT_MAXTEXTURESIZE);
		
		// 5) Texture compression.
		GfParmSetStr(hparm, GFSCR_SECT_GLDETFEATURES, GFSCR_ATT_TEXTURECOMPRESSION,
					 isSupported(TextureCompression) ? GFSCR_VAL_YES : GFSCR_VAL_NO);
		
		// 6) Multi-texturing.
		GfParmSetStr(hparm, GFSCR_SECT_GLDETFEATURES, GFSCR_ATT_MULTITEXTURING,
					 isSupported(MultiTexturing) ? GFSCR_VAL_YES : GFSCR_VAL_NO);
		if (getSupported(MultiTexturingUnits) != InvalidInt)
			GfParmSetNum(hparm, GFSCR_SECT_GLDETFEATURES, GFSCR_ATT_MULTITEXTURINGUNITS, pszNoUnit,
						 (tdble)getSupported(MultiTexturingUnits));
		else
			GfParmRemove(hparm, GFSCR_SECT_GLDETFEATURES, GFSCR_ATT_MULTITEXTURINGUNITS);

		// 7) Rectangle textures).
		GfParmSetStr(hparm, GFSCR_SECT_GLDETFEATURES, GFSCR_ATT_RECTANGLETEXTURES,
					 isSupported(TextureRectangle) ? GFSCR_VAL_YES : GFSCR_VAL_NO);

		// 8) Non-power-of-2 textures.
		GfParmSetStr(hparm, GFSCR_SECT_GLDETFEATURES, GFSCR_ATT_NONPOTTEXTURES,
					 isSupported(TextureNonPowerOf2) ? GFSCR_VAL_YES : GFSCR_VAL_NO);

		// 9) Multi-sampling.
		GfParmSetStr(hparm, GFSCR_SECT_GLDETFEATURES, GFSCR_ATT_MULTISAMPLING,
					 isSupported(MultiSampling) ? GFSCR_VAL_YES : GFSCR_VAL_NO);
	
		if (getSupported(MultiSamplingSamples) != InvalidInt)
			GfParmSetNum(hparm, GFSCR_SECT_GLDETFEATURES, GFSCR_ATT_MULTISAMPLINGSAMPLES, pszNoUnit,
						 (tdble)getSupported(MultiSamplingSamples));
		else
			GfParmRemove(hparm, GFSCR_SECT_GLDETFEATURES, GFSCR_ATT_MULTISAMPLINGSAMPLES);
	}
	
	// Write new params to config file.
	GfParmWriteFile(NULL, hparm, "Screen");
	
	// Close config file if we open it.
	if (!hparmConfig)
		closeConfigFile(hparm);

	// Trace resulting best supported features.
	dumpSupport();
}

bool GfglFeatures::checkBestSupport(int nWidth, int nHeight, int nDepth,
									bool bAlpha, bool bFullScreen, void* hparmConfig)
{
	// Open the config file if not already done.
	void* hparm = hparmConfig ? hparmConfig : openConfigFile();

	// Get the frame buffer specs that are associated with the detected
	// Open GL features in the config file, if any.
	int nDetWidth, nDetHeight, nDetDepth;
	bool bDetFullScreen, bDetAlpha;
	bool bPrevSupportFound =
		loadSupport(nDetWidth, nDetHeight, nDetDepth, bDetAlpha, bDetFullScreen, hparm);

	// Compare with the requested frame buffer specs
	// and run a new supported feature detection if any diffference.
	bool bSupportFound = true;
	if (!bPrevSupportFound || nWidth != nDetWidth || nHeight != nDetHeight || nDepth != nDetDepth
		|| bAlpha != bDetAlpha || bFullScreen != bDetFullScreen)
	{
		nDetWidth = nWidth;
		nDetHeight = nHeight;
		nDetDepth = nDepth;
		bDetFullScreen = bFullScreen;
		bDetAlpha = bAlpha;
		bSupportFound =
			detectBestSupport(nDetWidth, nDetHeight, nDetDepth, bDetAlpha, bDetFullScreen);

		// Store support data in any case.
		storeSupport(nDetWidth, nDetHeight, nDetDepth, bDetAlpha, bDetFullScreen, hparm);

		// If frame buffer specs supported, update relevant user settings and restart.
		if (bSupportFound)
		{
			// Write new user settings about the frame buffer specs
			// (the detection process might have down-casted them ...).
			GfParmSetNum(hparm, GFSCR_SECT_PROP, GFSCR_ATT_WIN_X, pszNoUnit,
						 (tdble)nDetWidth);
			GfParmSetNum(hparm, GFSCR_SECT_PROP, GFSCR_ATT_WIN_Y, pszNoUnit,
						 (tdble)nDetHeight);
			GfParmSetNum(hparm, GFSCR_SECT_PROP, GFSCR_ATT_BPP, pszNoUnit,
						 (tdble)nDetDepth);
			GfParmSetStr(hparm, GFSCR_SECT_PROP, GFSCR_ATT_ALPHACHANNEL,
						 bDetAlpha ? GFSCR_VAL_YES : GFSCR_VAL_NO);
			GfParmSetStr(hparm, GFSCR_SECT_PROP, GFSCR_ATT_FSCR,
						 bDetFullScreen ? GFSCR_VAL_YES : GFSCR_VAL_NO);

			// Write new params to config file.
			GfParmWriteFile(NULL, hparm, "Screen");

			// Close the config file ...
			closeConfigFile(hparm);

			// ... as we are restarting ...
			GfuiApp().restart();

			// Next time we pass in this function, loadSupport() will give
			// the right values for all features ...
		}
	}

	if (!hparmConfig)
		closeConfigFile(hparm);
	
	return bSupportFound;
}

void GfglFeatures::dumpHardwareInfo() const
{
	GfLogInfo("Video hardware info :\n");
	GfLogInfo("  Vendor   : %s\n", glGetString(GL_VENDOR));
	GfLogInfo("  Renderer : %s\n", glGetString(GL_RENDERER));
	GfLogInfo("  Version  : %s\n", glGetString(GL_VERSION));
}

void GfglFeatures::dumpSupport() const
{
	GfLogInfo("Supported OpenGL features :\n");

	if (_mapSupportedBool.empty() && _mapSupportedInt.empty())
	{
		GfLogInfo("  Unknown (detection failed).\n");
		return;
	}
	
	GfLogInfo("  Double buffer           : %s\n",
			  isSupported(DoubleBuffer) ? "Yes" : "No");
	GfLogInfo("  Color depth             : %d bits\n",
			  getSupported(ColorDepth));
	GfLogInfo("  Alpha channel           : %s",
			  getSupported(AlphaDepth) > 0 ? "Yes" : "No");
	if (getSupported(AlphaDepth) > 0)
		GfLogInfo(" (%d bits)", getSupported(AlphaDepth));
	GfLogInfo("\n");
	GfLogInfo("  Max texture size        : %d\n",
			  getSupported(TextureMaxSize));
	GfLogInfo("  Texture compression     : %s\n",
			  isSupported(TextureCompression) ? "Yes" : "No");
	GfLogInfo("  Multi-texturing         : %s",
			  isSupported(MultiTexturing) ? "Yes" : "No");
	if (isSupported(MultiTexturing))
		GfLogInfo(" (%d units)", getSupported(MultiTexturingUnits));
	GfLogInfo("\n");
	GfLogInfo("  Rectangle textures      : %s\n",
			  isSupported(TextureRectangle) ? "Yes" : "No");
	GfLogInfo("  Non power-of-2 textures : %s\n",
			  isSupported(TextureNonPowerOf2) ? "Yes" : "No");
	GfLogInfo("  Multi-sampling          : %s",
			  isSupported(MultiSampling) ? "Yes" : "No");
	if (isSupported(MultiSampling) && getSupported(MultiSamplingSamples) > 1)
		GfLogInfo(" (%d samples)", getSupported(MultiSamplingSamples));
	GfLogInfo("\n");
}

// Load the selected OpenGL features from the config file.
void GfglFeatures::loadSelection(void* hparmConfig)
{
	// Open the config file if not already done.
	void* hparm = hparmConfig ? hparmConfig : openConfigFile();

	// Select the OpenGL features according to the user settings (when relevant)
	// or/and to the supported values (by default, select the max supported values).

	// 1) Double-buffer : not user-customizable.
	_mapSelectedBool[DoubleBuffer] = isSupported(DoubleBuffer);

	// 2) Color buffer depth : not user-customizable.
	_mapSelectedInt[ColorDepth] = getSupported(ColorDepth);

	// 3) Alpha-channel depth : not user-customizable.
	_mapSelectedInt[AlphaDepth] = getSupported(AlphaDepth);

	// 4) Max texture size : load from config file.
	_mapSelectedInt[TextureMaxSize] =
		(int)GfParmGetNum(hparm, GFSCR_SECT_GLSELFEATURES, GFSCR_ATT_MAXTEXTURESIZE,
						  pszNoUnit, (tdble)getSupported(TextureMaxSize));
	if (_mapSelectedInt[TextureMaxSize] > getSupported(TextureMaxSize))
		_mapSelectedInt[TextureMaxSize] = getSupported(TextureMaxSize);

	// 5) Texture compression : load from config file.
	_mapSelectedBool[TextureCompression] =
		isSupported(TextureCompression)
		&& std::string(GfParmGetStr(hparm, GFSCR_SECT_GLSELFEATURES, GFSCR_ATT_TEXTURECOMPRESSION,
									GFSCR_ATT_TEXTURECOMPRESSION_ENABLED))
		   == GFSCR_ATT_TEXTURECOMPRESSION_ENABLED;

	// 6) Multi-texturing : load from config file.
	_mapSelectedBool[MultiTexturing] =
		isSupported(MultiTexturing)
		&& std::string(GfParmGetStr(hparm, GFSCR_SECT_GLSELFEATURES, GFSCR_ATT_MULTITEXTURING,
									GFSCR_ATT_MULTITEXTURING_ENABLED))
		   == GFSCR_ATT_MULTITEXTURING_ENABLED;
	_mapSelectedInt[MultiTexturingUnits] = 
		(int)GfParmGetNum(hparm, GFSCR_SECT_GLSELFEATURES, GFSCR_ATT_MULTITEXTURINGUNITS,
						  pszNoUnit, (tdble)getSupported(TextureMaxSize));
	if (_mapSelectedInt[MultiTexturingUnits] > getSupported(MultiTexturingUnits))
		_mapSelectedInt[MultiTexturingUnits] = getSupported(MultiTexturingUnits);

	// 7) Rectangle textures : not user-customizable.
	_mapSelectedBool[TextureRectangle] = isSupported(TextureRectangle);

	// 8) Non-power-of-2 textures : not user-customizable.
	_mapSelectedBool[TextureNonPowerOf2] = isSupported(TextureNonPowerOf2);

	// 9) Multi-sampling : load from config file.
	const std::string strMultiSamp =
		GfParmGetStr(hparm, GFSCR_SECT_GLSELFEATURES, GFSCR_ATT_MULTISAMPLING,
					 GFSCR_ATT_MULTISAMPLING_ENABLED);
	_mapSelectedBool[MultiSampling] =
		isSupported(MultiSampling)
		&& std::string(GfParmGetStr(hparm, GFSCR_SECT_GLSELFEATURES, GFSCR_ATT_MULTISAMPLING,
									GFSCR_ATT_MULTISAMPLING_ENABLED))
		   == GFSCR_ATT_MULTISAMPLING_ENABLED;
	
	_mapSelectedInt[MultiSamplingSamples] =
		(int)GfParmGetNum(hparm, GFSCR_SECT_GLSELFEATURES, GFSCR_ATT_MULTISAMPLINGSAMPLES,
						  pszNoUnit, (tdble)8); // Good but reasonable value.
	if (_mapSelectedInt[MultiSamplingSamples] > getSupported(MultiSamplingSamples))
		_mapSelectedInt[MultiSamplingSamples] = getSupported(MultiSamplingSamples);

	// Close config file if we open it.
	if (!hparmConfig)
		closeConfigFile(hparm);
	
	// Display what we have selected.
	dumpSelection();
}


// Save settings to screen.xml
void GfglFeatures::storeSelection(void* hparmConfig) const
{
	// Display what we have selected.
	dumpSelection();

	// Open the config file if not already done.
	void* hparm = hparmConfig ? hparmConfig : openConfigFile();

	// Write new values.
	GfParmSetStr(hparm, GFSCR_SECT_GLSELFEATURES, GFSCR_ATT_TEXTURECOMPRESSION,
				 isSelected(TextureCompression)
				 ? GFSCR_ATT_TEXTURECOMPRESSION_ENABLED : GFSCR_ATT_TEXTURECOMPRESSION_DISABLED);
	if (getSupported(TextureMaxSize) != InvalidInt)
		GfParmSetNum(hparm, GFSCR_SECT_GLSELFEATURES, GFSCR_ATT_MAXTEXTURESIZE, pszNoUnit,
					 (tdble)getSelected(TextureMaxSize));
	else
		GfParmRemove(hparm, GFSCR_SECT_GLSELFEATURES, GFSCR_ATT_MAXTEXTURESIZE);
		
	GfParmSetStr(hparm, GFSCR_SECT_GLSELFEATURES, GFSCR_ATT_MULTITEXTURING,
				 isSelected(MultiTexturing)
				 ? GFSCR_ATT_MULTITEXTURING_ENABLED : GFSCR_ATT_MULTITEXTURING_DISABLED);
	GfParmSetStr(hparm, GFSCR_SECT_GLSELFEATURES, GFSCR_ATT_MULTISAMPLING,
				 isSelected(MultiSampling)
				 ? GFSCR_ATT_MULTISAMPLING_ENABLED : GFSCR_ATT_MULTISAMPLING_DISABLED);
	if (getSupported(MultiSamplingSamples) != InvalidInt)
		GfParmSetNum(hparm, GFSCR_SECT_GLSELFEATURES, GFSCR_ATT_MULTISAMPLINGSAMPLES, pszNoUnit,
					 (tdble)getSelected(MultiSamplingSamples));
	else
		GfParmRemove(hparm, GFSCR_SECT_GLSELFEATURES, GFSCR_ATT_MULTISAMPLINGSAMPLES);

	// Force 'best possible' mode for video initialization when anti-aliasing selected
	if (isSelected(MultiSampling))
	 	GfParmSetStr(hparm, GFSCR_SECT_PROP, GFSCR_ATT_VINIT, GFSCR_VAL_VINIT_BEST);
	
	// Write new params to config file.
	GfParmWriteFile(NULL, hparm, "Screen");
	
	// Close config file if we open it.
	if (!hparmConfig)
		closeConfigFile(hparm);
}

void GfglFeatures::dumpSelection() const
{
	GfLogInfo("Selected OpenGL features :\n");
	GfLogInfo("  Double buffer           : %s\n", isSelected(DoubleBuffer) ? "On" : "Off");
	if (getSelected(ColorDepth) != InvalidInt)
		GfLogInfo("  Color depth             : %d bits\n", getSelected(ColorDepth));
	else
		GfLogInfo("  Color depth             : no selection\n");
	GfLogInfo("  Alpha channel           : %s",
			  getSelected(AlphaDepth) > 0 ? "On" : "Off");
	if (getSelected(AlphaDepth) > 0)
		GfLogInfo(" (%d bits)", getSelected(AlphaDepth));
	GfLogInfo("\n");
	if (getSelected(TextureMaxSize) != InvalidInt)
		GfLogInfo("  Max texture size        : %d\n", getSelected(TextureMaxSize));
	else
		GfLogInfo("  Max texture size        : no selection\n");
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

// Bool features management.
bool GfglFeatures::isSelected(EFeatureBool eFeature) const
{
	const std::map<EFeatureBool, bool>::const_iterator itFeature =
		_mapSelectedBool.find(eFeature);
	return itFeature == _mapSelectedBool.end() ? false : itFeature->second;
}

// void GfglFeatures::setSupported(EFeatureBool eFeature, bool bSupported)
// {
// 	_mapSupportedBool[eFeature] = bSupported;
// 	if (!bSupported && isSelected(eFeature))
// 		_mapSelectedBool[eFeature] = false; // Deselect if selected and not supported.
// }

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

// Int features management.
int GfglFeatures::getSelected(EFeatureInt eFeature) const
{
	const std::map<EFeatureInt, int>::const_iterator itFeature =
		_mapSelectedInt.find(eFeature);
	return itFeature == _mapSelectedInt.end() ? InvalidInt : itFeature->second;
}

// void GfglFeatures::setSupported(EFeatureInt eFeature, int nSupportedValue)
// {
// 	_mapSupportedInt[eFeature] = nSupportedValue;
// 	if (getSelected(eFeature) > nSupportedValue) // Selected can't be greater than supported.
// 		_mapSelectedInt[eFeature] = nSupportedValue;
// }

int GfglFeatures::getSupported(EFeatureInt eFeature) const
{
	const std::map<EFeatureInt, int>::const_iterator itFeature =
		_mapSupportedInt.find(eFeature);
	return itFeature == _mapSupportedInt.end() ? InvalidInt : itFeature->second;
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

// Other services.
void* GfglFeatures::getProcAddress(const char* pszName)
{
	return SDL_GL_GetProcAddress(pszName);
}
