/***************************************************************************

    file        : openglconfig.cpp
    created     : Fri Jun 3 12:52:07 CET 2004
    copyright   : (C) 2005 Bernhard Wymann
    email       : berniw@bluewin.ch
    version     : $Id$

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
             Open GL options menu
    @version	$Id$
*/

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <vector>
#include <sstream>

#include <portability.h>
#include <tgfclient.h>
#include <glfeatures.h>

#include "legacymenu.h"
#include "openglconfig.h"

				  
// Texture compression.
static const char *ATextureCompTexts[] =
	{GfSCR_ATT_TEXTURECOMPRESSION_DISABLED, GfSCR_ATT_TEXTURECOMPRESSION_ENABLED};
static const int NTextureComps =
	sizeof(ATextureCompTexts) / sizeof(ATextureCompTexts[0]);
static int NCurTextureCompIndex = 0;
static int TextureCompLabelId;
static int TextureCompLeftButtonId;
static int TextureCompRightButtonId;

// Max texture size (WARNING: the order in the list is important, do not change).
static int AMaxTextureSizeTexts[] = {8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384};
static int NMaxTextureSizes = sizeof(AMaxTextureSizeTexts) / sizeof(AMaxTextureSizeTexts[0]);
static int NCurMaxTextureSizeIndex = 0;
static int MaxTextureSizeLabelId;

static const int NDefaultTextureSize = 64; // In case everything goes wrong.

// Multi-texturing.
static const char *AMultiTextureTexts[] =
	{GfSCR_ATT_MULTITEXTURING_DISABLED, GfSCR_ATT_MULTITEXTURING_ENABLED};
static const int NMultiTextures =
	sizeof(AMultiTextureTexts) / sizeof(AMultiTextureTexts[0]);
static int NCurMultiTextureIndex = 0;

static int MultiTextureLabelId;
static int MultiTextureLeftButtonId;
static int MultiTextureRightButtonId;

// Multi-sampling (initialized in OpenGLMenuInit).
static std::vector<std::string> VecMultiSampleTexts;
static int NMultiSamples = 0;
static int NCurMultiSampleIndex = 0;

static int MultiSampleLabelId;
static int MultiSampleLeftButtonId;
static int MultiSampleRightButtonId;

// GUI screen handles.
static void	*ScrHandle = NULL;
static void	*PrevHandle = NULL;

static bool BMultiSamplingWasSelected = false;
static int BPrevMultiSamplingSamples = 0;

static void onAccept(void *)
{
	// Store current state of settings to the GL features layer.
	GfglFeatures::self().select(GfglFeatures::TextureCompression,
								 strcmp(ATextureCompTexts[NCurTextureCompIndex],
										GfSCR_ATT_TEXTURECOMPRESSION_ENABLED) ? false : true);
	GfglFeatures::self().select(GfglFeatures::TextureMaxSize,
								 AMaxTextureSizeTexts[NCurMaxTextureSizeIndex]);
	GfglFeatures::self().select(GfglFeatures::MultiTexturing,
								 strcmp(AMultiTextureTexts[NCurMultiTextureIndex],
										GfSCR_ATT_MULTITEXTURING_ENABLED) ? false : true);
	GfglFeatures::self().select(GfglFeatures::MultiSampling,
								VecMultiSampleTexts[NCurMultiSampleIndex]
								!= GfSCR_ATT_MULTISAMPLING_DISABLED);
	if (VecMultiSampleTexts[NCurMultiSampleIndex] != GfSCR_ATT_MULTISAMPLING_DISABLED)
		GfglFeatures::self().select(GfglFeatures::MultiSamplingSamples,
									(int)pow(2, NCurMultiSampleIndex));

	// Store settings from the GL features layer to the graph.xml file.
	GfglFeatures::self().storeSelection();
	
	// Return to previous screen.
	GfuiScreenActivate(PrevHandle);

	// But actually restart the game if the multi-sampling feature settings changed
	// (we can't change this without re-initializing the video mode).
	if (GfglFeatures::self().isSelected(GfglFeatures::MultiSampling) != BMultiSamplingWasSelected
		|| GfglFeatures::self().getSelected(GfglFeatures::MultiSamplingSamples) != BPrevMultiSamplingSamples)
	{
		// TODO: Simply call GfuiApp().restart() (when it is implemented ;-).
		
		// Shutdown the user interface.
		LegacyMenu::self().shutdown();
		
		// Restart the game.
		GfRestart(GfuiMouseIsHWPresent());

		// TODO: A nice system to get back to previous display settings if the chosen ones
		//       keep the game from really restarting (ex: unsupported full screen size) ?
	}
}

// Toggle texture compression state enabled/disabled.
static void changeTextureCompressionState(void *vp)
{
	NCurTextureCompIndex = (NCurTextureCompIndex + (int)(long)vp + NTextureComps) % NTextureComps;
	GfuiLabelSetText(ScrHandle, TextureCompLabelId, ATextureCompTexts[NCurTextureCompIndex]);
}

// Toggle multi-texturing state enabled/disabled.
static void changeMultiTextureState(void *vp)
{
	NCurMultiTextureIndex = (NCurMultiTextureIndex + (int)(long)vp + NMultiTextures) % NMultiTextures;
	GfuiLabelSetText(ScrHandle, MultiTextureLabelId, AMultiTextureTexts[NCurMultiTextureIndex]);
}

// Toggle multi-sampling state enabled/disabled.
static void changeMultiSampleState(void *vp)
{
	NCurMultiSampleIndex = (NCurMultiSampleIndex + (int)(long)vp + NMultiSamples) % NMultiSamples;
	GfuiLabelSetText(ScrHandle, MultiSampleLabelId,
					 VecMultiSampleTexts[NCurMultiSampleIndex].c_str());
}


// Scroll through texture sizes smaller or equal the system limit.
static void changeMaxTextureSizeState(void *vp)
{
	char valuebuf[10];

	long delta = (long)vp;
	NCurMaxTextureSizeIndex += delta;
	if (NCurMaxTextureSizeIndex < 0)
	{
		NCurMaxTextureSizeIndex = NMaxTextureSizes - 1;
	} else if (NCurMaxTextureSizeIndex >= NMaxTextureSizes)
	{
		NCurMaxTextureSizeIndex= 0;
	}

	snprintf(valuebuf, sizeof(valuebuf), "%d", AMaxTextureSizeTexts[NCurMaxTextureSizeIndex]);
	GfuiLabelSetText(ScrHandle, MaxTextureSizeLabelId, valuebuf);
}


static void onActivate(void * /* dummy */)
{
	int i;
	char valuebuf[10];

	// Initialize current state and GUI from the GL features layer.
	// 1) Texture compression.
	if (GfglFeatures::self().isSupported(GfglFeatures::TextureCompression))
	{
		const char *pszTexComp =
			GfglFeatures::self().isSelected(GfglFeatures::TextureCompression)
			? GfSCR_ATT_TEXTURECOMPRESSION_ENABLED : GfSCR_ATT_TEXTURECOMPRESSION_DISABLED;
		for (i = 0; i < NTextureComps; i++)
		{
			if (!strcmp(pszTexComp, ATextureCompTexts[i]))
			{
				NCurTextureCompIndex = i;
				break;
			}
		}

		GfuiLabelSetText(ScrHandle, TextureCompLabelId,
						 ATextureCompTexts[NCurTextureCompIndex]);
	}
	else
	{
		GfuiEnable(ScrHandle, TextureCompLeftButtonId, GFUI_DISABLE);
		GfuiEnable(ScrHandle, TextureCompRightButtonId, GFUI_DISABLE);
		GfuiLabelSetText(ScrHandle, TextureCompLabelId, "Not supported");
	}

	// 2) Max texture size.
	int sizelimit =
		GfglFeatures::self().getSupported(GfglFeatures::TextureMaxSize);
	int tsize =
		GfglFeatures::self().getSelected(GfglFeatures::TextureMaxSize);

	int maxsizenb = 0;
	for (i = 0; i < NMaxTextureSizes; i++)
	{
		if (AMaxTextureSizeTexts[i] <= sizelimit)
		{
			maxsizenb = i;
		} else {
			break;
		}
	}

	// Limit to available sizes.
	NMaxTextureSizes = maxsizenb + 1;

	bool found = false;
	for (i = 0; i < NMaxTextureSizes; i++)
	{
		if (AMaxTextureSizeTexts[i] == tsize)
		{
			NCurMaxTextureSizeIndex = i;
			found = true;
			break;
		}
	}

	if (!found)
	{
		// Should never come here if there is no bug in OpenGL.
		tsize = NDefaultTextureSize;
		for (i = 0; i < NMaxTextureSizes; i++)
		{
			if (AMaxTextureSizeTexts[i] == tsize)
			{
				NCurMaxTextureSizeIndex = i;
				break;
			}
		}
	}
	
	snprintf(valuebuf, sizeof(valuebuf), "%d", AMaxTextureSizeTexts[NCurMaxTextureSizeIndex]);
	GfuiLabelSetText(ScrHandle, MaxTextureSizeLabelId, valuebuf);
	
	// 3) Multi-texturing.
	if (GfglFeatures::self().isSupported(GfglFeatures::MultiTexturing))
	{
		const char *pszMultiTex =
			GfglFeatures::self().isSelected(GfglFeatures::MultiTexturing)
			? GfSCR_ATT_MULTITEXTURING_ENABLED : GfSCR_ATT_MULTITEXTURING_DISABLED;
		for (i = 0; i < NMultiTextures; i++)
		{
			if (!strcmp(pszMultiTex, AMultiTextureTexts[i]))
			{
				NCurMultiTextureIndex = i;
				break;
			}
		}

		GfuiLabelSetText(ScrHandle, MultiTextureLabelId,
						 AMultiTextureTexts[NCurMultiTextureIndex]);
	}
	else
	{
		GfuiEnable(ScrHandle, MultiTextureLeftButtonId, GFUI_DISABLE);
		GfuiEnable(ScrHandle, MultiTextureRightButtonId, GFUI_DISABLE);
		GfuiLabelSetText(ScrHandle, MultiTextureLabelId, "Not supported");
	}
	
	// 4) Multi-sampling (anti-aliasing).
	if (GfglFeatures::self().isSupported(GfglFeatures::MultiSampling))
	{
		BMultiSamplingWasSelected =
			GfglFeatures::self().isSelected(GfglFeatures::MultiSampling);
		BPrevMultiSamplingSamples =
			GfglFeatures::self().getSelected(GfglFeatures::MultiSamplingSamples);

		if (!BMultiSamplingWasSelected)
			NCurMultiSampleIndex = 0;
		else
		{
			NCurMultiSampleIndex = 0;
			int nSamples = 1;
			while (nSamples < BPrevMultiSamplingSamples)
			{
				NCurMultiSampleIndex++;
				nSamples *= 2;
			}
		}

		GfuiLabelSetText(ScrHandle, MultiSampleLabelId,
						 VecMultiSampleTexts[NCurMultiSampleIndex].c_str());
	}
	else
	{
		GfuiEnable(ScrHandle, MultiSampleLeftButtonId, GFUI_DISABLE);
		GfuiEnable(ScrHandle, MultiSampleRightButtonId, GFUI_DISABLE);
		GfuiLabelSetText(ScrHandle, MultiSampleLabelId, "Not supported");
	}
}


// OpenGL menu
void* OpenGLMenuInit(void *prevMenu)
{
	// Has screen already been created?
	if (ScrHandle)
		return ScrHandle;

	PrevHandle = prevMenu;

	ScrHandle = GfuiScreenCreateEx((float*)NULL, NULL, onActivate, NULL, (tfuiCallback)NULL, 1);
	void *hparmMenu = LoadMenuXML("opengloptionsmenu.xml");
	CreateStaticControls(hparmMenu,ScrHandle);

	// Texture compression.
	TextureCompLeftButtonId =
		CreateButtonControl(ScrHandle, hparmMenu, "TextureCompressionLeftArrowButton", (void*)-1,
							changeTextureCompressionState);
	TextureCompRightButtonId =
		CreateButtonControl(ScrHandle, hparmMenu, "TextureCompressionRightArrowButton", (void*)+1,
							changeTextureCompressionState);
	TextureCompLabelId = CreateLabelControl(ScrHandle,hparmMenu,"TextureCompressionLabel");

	// Texture sizing.
	CreateButtonControl(ScrHandle,hparmMenu,"MaxTextureSizeLeftArrowButton", (void*)-1,
						changeMaxTextureSizeState);
	CreateButtonControl(ScrHandle,hparmMenu,"MaxTextureSizeRightArrowButton", (void*)+1,
						changeMaxTextureSizeState);
	MaxTextureSizeLabelId = CreateLabelControl(ScrHandle,hparmMenu,"MaxTextureSizeLabel");

	// Multi-texturing.
	MultiTextureLeftButtonId =
		CreateButtonControl(ScrHandle, hparmMenu, "MultiTextureLeftArrowButton", (void*)-1,
							changeMultiTextureState);
	MultiTextureRightButtonId =
		CreateButtonControl(ScrHandle, hparmMenu, "MultiTextureRightArrowButton", (void*)+1,
							changeMultiTextureState);
	MultiTextureLabelId = CreateLabelControl(ScrHandle,hparmMenu,"MultiTextureLabel");

	// Multi-sampling.
	MultiSampleLeftButtonId =
		CreateButtonControl(ScrHandle, hparmMenu, "MultiSampleLeftArrowButton", (void*)-1,
							changeMultiSampleState);
	MultiSampleRightButtonId =
		CreateButtonControl(ScrHandle, hparmMenu, "MultiSampleRightArrowButton", (void*)+1,
							changeMultiSampleState);
	MultiSampleLabelId = CreateLabelControl(ScrHandle,hparmMenu,"MultiSampleLabel");

	CreateButtonControl(ScrHandle,hparmMenu,"AcceptButton",NULL, onAccept);
	CreateButtonControl(ScrHandle,hparmMenu,"CancelButton",prevMenu, GfuiScreenActivate);

	GfParmReleaseHandle(hparmMenu);

	GfuiAddKey(ScrHandle, GFUIK_RETURN, "Save", NULL, onAccept, NULL);
	GfuiAddKey(ScrHandle, GFUIK_ESCAPE, "Cancel Selection", prevMenu, GfuiScreenActivate, NULL);
	GfuiAddKey(ScrHandle, GFUIK_F1, "Help", ScrHandle, GfuiHelpScreen, NULL);
	GfuiAddKey(ScrHandle, GFUIK_F12, "Screen-Shot", NULL, GfuiScreenShot, NULL);
	GfuiAddKey(ScrHandle, GFUIK_LEFT, "Decrease Texture Size Limit", (void*)-1, changeMaxTextureSizeState, NULL);
	GfuiAddKey(ScrHandle, GFUIK_RIGHT, "Increase Texture Size Limit", (void*)1, changeMaxTextureSizeState, NULL);
	GfuiAddKey(ScrHandle, ' ', "Toggle Texture Compression", (void*)1, changeTextureCompressionState, NULL);

	// Initialize multi-sampling levels.
	NMultiSamples = 1;
	VecMultiSampleTexts.push_back(GfSCR_ATT_MULTISAMPLING_DISABLED);
	if (GfglFeatures::self().isSupported(GfglFeatures::MultiSampling)
		&& GfglFeatures::self().getSupported(GfglFeatures::MultiSamplingSamples) > 1)
	{
		const int nMaxSamples =
			GfglFeatures::self().getSupported(GfglFeatures::MultiSamplingSamples);
		NMultiSamples += (int)(log(nMaxSamples) / log(2));
		std::ostringstream ossVal;
		for (int nVal = 2; nVal <= nMaxSamples; nVal *= 2)
		{
			ossVal.str("");
			ossVal << nVal << "x";
			VecMultiSampleTexts.push_back(ossVal.str());
		}
		GfLogDebug("OpenGLMenuInit: nMaxSamples=%d, NMultiSamples=%d, VecMultiSampleTexts.size=%u\n", nMaxSamples, NMultiSamples, VecMultiSampleTexts.size());
	}
	
	return ScrHandle;
}
