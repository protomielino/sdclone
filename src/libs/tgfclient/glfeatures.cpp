/***************************************************************************

    file                 : glfeatures.cpp
    created              : Wed Jun 1 14:56:31 CET 2005
    copyright            : (C) 2005 by Bernhard Wymann
    version              : $Id: glfeatures.cpp,v 1.2 19 Mar 2006 20:37:46 berniw Exp $

***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


/*
	Functions to check if features seems to be available and requested by the
	user. The isAvailable functions should return if a feature is working on
	the system, the isEnabled feature should check if the user wants to enable
	it as well.
	It should NOT check if the features are really working, that is subject
	to another part eventually.
*/

#include <SDL/SDL.h>

#include "glfeatures.h"


/** Report if a given OpenGL extension is supported (1) or not (0)

    Warning: Should not be called before any successfull call to SDL_SetVideoMode()

    Note: Copied from freeGLUT 2.4.0
*/

int GfglIsOpenGLExtensionSupported(const char* extension)
{
  const char *extensions, *start;
  const int len = strlen(extension);

  /* TODO: Make sure there is a current window, and thus a current context available */

  if (strchr(extension, ' '))
    return 0;

  start = extensions = (const char *)glGetString(GL_EXTENSIONS);

  if (!extensions)
	return 0;

  while (1)
  {
     const char *p = strstr(extensions, extension);
     if (!p)
        return 0;  /* not found */
     /* check that the match isn't a super string */
     if ((p == start || p[-1] == ' ') && (p[len] == ' ' || p[len] == 0))
        return 1;
     /* skip the false match and continue */
     extensions = p + len;
  }

  return 0;
}

/*
	----------------------- Texture Compression
*/


static bool bCompressARBAvailable;
static bool bCompressARBEnabled;

// Feature checks, GL_ARB_texture_compression.
void checkCompressARBAvailable(bool &result)
{
	// Query if the extension is available at the runtime system (true, if > 0).
	int compressARB = GfglIsOpenGLExtensionSupported("GL_ARB_texture_compression");
	
	// Check if at least one internal format is vailable. This is a workaround for
	// driver problems and not a bugfix. According to the specification OpenGL should
	// choose an uncompressed alternate format if it can't provide the requested
	// compressed one... but it does not on all cards/drivers.
	if (compressARB) 
	{
		int numformats;
		glGetIntegerv(GL_NUM_COMPRESSED_TEXTURE_FORMATS_ARB, &numformats);
		if (numformats == 0) 
		{
			compressARB = 0;
		}
	}

	result = compressARB != 0;
}


void checkCompressARBEnabled(bool &result)
{
	if (!GfglIsCompressARBAvailable()) 
	{
		// Feature not available, do not use it.
		result = false;
	} 
	else
	{
		// Feature available, check if the user wants to use it.
		// TODO: put this enabled/disable stuff in one function (it is used in grsound.cpp as well).
		const char *tcEnabledStr = GR_ATT_TEXTURECOMPRESSION_ENABLED;
		char fnbuf[1024];
		sprintf(fnbuf, "%s%s", GetLocalDir(), GR_PARAM_FILE);
		void *paramHandle = GfParmReadFile(fnbuf, GFPARM_RMODE_REREAD | GFPARM_RMODE_CREAT);
		const char *optionName = GfParmGetStr(paramHandle, GR_SCT_GLFEATURES, GR_ATT_TEXTURECOMPRESSION, GR_ATT_TEXTURECOMPRESSION_DISABLED);

		result = strcmp(optionName, tcEnabledStr) == 0;
		GfParmReleaseHandle(paramHandle);
	}
}


void GfglUpdateCompressARBEnabled(void)
{
	checkCompressARBEnabled(bCompressARBEnabled);
}


// GL_ARB_texture_compression
bool GfglIsCompressARBAvailable(void)
{
	return bCompressARBAvailable;
}


bool GfglIsCompressARBEnabled(void) 
{
	return bCompressARBEnabled;
}


/*
	----------------------- Texture downsizing.
*/
static int nGLTextureMaxSize;
static int nUserTextureMaxSize;


void GfglGetGLTextureMaxSize(int &result)
{
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &result);
	if (result > 16384) 
	{
		result = 16384;
	}
}


void GfglGetUserTextureMaxSize(int &result)
{
	char fnbuf[1024];
	sprintf(fnbuf, "%s%s", GetLocalDir(), GR_PARAM_FILE);
	void *paramHandle = GfParmReadFile(fnbuf, GFPARM_RMODE_REREAD | GFPARM_RMODE_CREAT);
	result = (int) GfParmGetNum(paramHandle, GR_SCT_GLFEATURES, GR_ATT_TEXTURESIZE, (char*)NULL, (tdble) nGLTextureMaxSize);
	if (result > nGLTextureMaxSize) 
	{
		result = nGLTextureMaxSize;
	}
	GfParmReleaseHandle(paramHandle);
}


void GfglUpdateUserTextureMaxSize(void)
{
	GfglGetUserTextureMaxSize(nUserTextureMaxSize);
}

int GfglGetGLTextureMaxSize(void)
{
	return nGLTextureMaxSize;
}


int GfglGetUserTextureMaxSize(void)
{
	return nUserTextureMaxSize;
}


/*
	----------------------- Non-power of 2 size texture support.
*/
static bool bTextureRectangleARBAvailable;
static bool bTextureNonPowerOf2ARBAvailable;

// Feature checks, GL_ARB_texture_rectangle.
void checkTextureRectangleARBAvailable(bool &result)
{
	// Query if the extension is available at the runtime system (true, if not 0).
	result = GfglIsOpenGLExtensionSupported("GL_ARB_texture_rectangle") != 0;
}

// Feature checks, GL_ARB_texture_rectangle.
void checkTextureNonPowerOf2ARBAvailable(bool &result)
{
	// Query if the extension is available at the runtime system (true if not 0).
	result = GfglIsOpenGLExtensionSupported("GL_ARB_texture_non_power_of_two") != 0;
}

bool GfglIsTextureRectangleARBAvailable(void)
{
	return bTextureRectangleARBAvailable;
}

bool GfglIsTextureNonPowerOf2ARBAvailable(void)
{
	return bTextureNonPowerOf2ARBAvailable;
}

/*
	----------------------- Initialization.
*/

void gfglCheckGLFeatures(void) 
{
	checkCompressARBAvailable(bCompressARBAvailable);
	checkCompressARBEnabled(bCompressARBEnabled);
	GfglGetGLTextureMaxSize(nGLTextureMaxSize);
	GfglGetUserTextureMaxSize(nUserTextureMaxSize);
	checkTextureRectangleARBAvailable(bTextureRectangleARBAvailable);
	checkTextureNonPowerOf2ARBAvailable(bTextureNonPowerOf2ARBAvailable);
}
