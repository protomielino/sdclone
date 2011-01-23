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

    @version	$Id$
*/

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <graphic.h>
#include <portability.h>
#include <tgfclient.h>
#include <glfeatures.h>

#include "openglconfig.h"

				  
// Texture compression.
static const char *ATextureCompTexts[] =
	{GR_ATT_TEXTURECOMPRESSION_DISABLED, GR_ATT_TEXTURECOMPRESSION_ENABLED};
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

static const int NDefaultTextSize = 64; // In case everything goes wrong.

// Multi-texturing.
static const char *AMultiTextureTexts[] =
	{GR_ATT_MULTITEXTURING_DISABLED, GR_ATT_MULTITEXTURING_ENABLED};
static const int NMultiTextures =
	sizeof(AMultiTextureTexts) / sizeof(AMultiTextureTexts[0]);
static int NCurMultiTextureIndex = 0;

static int MultiTextureLabelId;
static int MultiTextureLeftButtonId;
static int MultiTextureRightButtonId;

// Multi-sampling.
static const char *AMultiSampleTexts[] =
	{GR_ATT_MULTISAMPLING_DISABLED, GR_ATT_MULTISAMPLING_ENABLED};
static const int NMultiSamples =
	sizeof(AMultiSampleTexts) / sizeof(AMultiSampleTexts[0]);
static int NCurMultiSampleIndex = 0;

static int MultiSampleLabelId;
static int MultiSampleLeftButtonId;
static int MultiSampleRightButtonId;

// GUI screen handles.
static void	*ScrHandle = NULL;
static void	*PrevHandle = NULL;


void OpenGLLoadSelectedFeatures()
{
	char buf[512];

	// Read OpenGL configuration from graph.xml, and select relevant OpenGL features.
	snprintf(buf, sizeof(buf), "%s%s", GfLocalDir(), GR_PARAM_FILE);
	void* paramHandle = GfParmReadFile(buf, GFPARM_RMODE_REREAD | GFPARM_RMODE_CREAT);

	// 1) Texture compression.
	const char* pszTexComp =
		GfParmGetStr(paramHandle, GR_SCT_GLFEATURES, GR_ATT_TEXTURECOMPRESSION,
					 GR_ATT_TEXTURECOMPRESSION_DISABLED);
	GfglFeatures::self()->select(GfglFeatures::TextureCompression,
								 strcmp(pszTexComp, GR_ATT_TEXTURECOMPRESSION_ENABLED) ? false : true);

	// 2) Max texture size.
	const int sizelimit =
		GfglFeatures::self()->getSupported(GfglFeatures::TextureMaxSize);
	int tsize =
		(int)GfParmGetNum(paramHandle, GR_SCT_GLFEATURES, GR_ATT_MAXTEXTURESIZE,
						  (char*)NULL, (tdble)sizelimit);
	if (tsize > sizelimit)
		tsize = sizelimit;

	GfglFeatures::self()->select(GfglFeatures::TextureMaxSize, tsize);

	// 3) Multi-texturing.
	const char* pszMultiTex =
		GfParmGetStr(paramHandle, GR_SCT_GLFEATURES, GR_ATT_MULTITEXTURING,
					 GR_ATT_MULTITEXTURING_DISABLED);
	GfglFeatures::self()->select(GfglFeatures::MultiTexturing,
								 strcmp(pszMultiTex, GR_ATT_MULTITEXTURING_ENABLED) ? false : true);

	// 4) Multi-sampling.
	const char* pszMultiSamp =
		GfParmGetStr(paramHandle, GR_SCT_GLFEATURES, GR_ATT_MULTISAMPLING,
					 GR_ATT_MULTISAMPLING_DISABLED);
	GfglFeatures::self()->select(GfglFeatures::MultiSampling,
								 strcmp(pszMultiSamp, GR_ATT_MULTISAMPLING_ENABLED) ? false : true);

	// Close graphic params.
	GfParmReleaseHandle(paramHandle);
}


// Save the choosen values in the corresponding parameter file.
void OpenGLStoreSelectedFeatures()
{
	// Save settings to graph.xml
	char buf[512];
	snprintf(buf, sizeof(buf), "%s%s", GfLocalDir(), GR_PARAM_FILE);
	void *paramHandle = GfParmReadFile(buf, GFPARM_RMODE_REREAD | GFPARM_RMODE_CREAT);

	GfParmSetStr(paramHandle, GR_SCT_GLFEATURES, GR_ATT_TEXTURECOMPRESSION,
				 GfglFeatures::self()->isSelected(GfglFeatures::TextureCompression)
				 ? GR_ATT_TEXTURECOMPRESSION_ENABLED : GR_ATT_TEXTURECOMPRESSION_DISABLED);
	GfParmSetNum(paramHandle, GR_SCT_GLFEATURES, GR_ATT_MAXTEXTURESIZE, (char*)NULL,
				 (tdble)GfglFeatures::self()->getSelected(GfglFeatures::TextureMaxSize));
	GfParmSetStr(paramHandle, GR_SCT_GLFEATURES, GR_ATT_MULTITEXTURING,
				 GfglFeatures::self()->isSelected(GfglFeatures::MultiTexturing)
				 ? GR_ATT_MULTITEXTURING_ENABLED : GR_ATT_MULTITEXTURING_DISABLED);
	GfParmSetStr(paramHandle, GR_SCT_GLFEATURES, GR_ATT_MULTISAMPLING,
				 GfglFeatures::self()->isSelected(GfglFeatures::MultiSampling)
				 ? GR_ATT_MULTISAMPLING_ENABLED : GR_ATT_MULTISAMPLING_DISABLED);

	GfParmWriteFile(NULL, paramHandle, "graph");
	GfParmReleaseHandle(paramHandle);
}

static void onAccept(void *)
{
	// Store current state of settings to the GL features layer.
	GfglFeatures::self()->select(GfglFeatures::TextureCompression,
								 strcmp(ATextureCompTexts[NCurTextureCompIndex],
										GR_ATT_TEXTURECOMPRESSION_ENABLED) ? false : true);
	GfglFeatures::self()->select(GfglFeatures::TextureMaxSize,
								 AMaxTextureSizeTexts[NCurMaxTextureSizeIndex]);
	GfglFeatures::self()->select(GfglFeatures::MultiTexturing,
								 strcmp(AMultiTextureTexts[NCurMultiTextureIndex],
										GR_ATT_MULTITEXTURING_ENABLED) ? false : true);
	GfglFeatures::self()->select(GfglFeatures::MultiSampling,
								 strcmp(AMultiSampleTexts[NCurMultiSampleIndex],
										GR_ATT_MULTISAMPLING_ENABLED) ? false : true);

	// Store settings from the GL features layer to the graph.xml file.
	GfglFeatures::self()->storeSelection();
	
	// Return to previous screen.
	GfuiScreenActivate(PrevHandle);
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
	GfuiLabelSetText(ScrHandle, MultiSampleLabelId, AMultiSampleTexts[NCurMultiSampleIndex]);
}


// Scroll through texture sizes smaller or equal the system limit.
static void changeMaxTextureSizeState(void *vp)
{
	char valuebuf[10];

	long delta = (long)vp;
	NCurMaxTextureSizeIndex += delta;
	if (NCurMaxTextureSizeIndex < 0) {
		NCurMaxTextureSizeIndex = NMaxTextureSizes - 1;
	} else if (NCurMaxTextureSizeIndex >= NMaxTextureSizes) {
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
	if (GfglFeatures::self()->isSupported(GfglFeatures::TextureCompression))
	{
		const char *pszTexComp =
			GfglFeatures::self()->isSelected(GfglFeatures::TextureCompression)
			? GR_ATT_TEXTURECOMPRESSION_ENABLED : GR_ATT_TEXTURECOMPRESSION_DISABLED;
		for (i = 0; i < NTextureComps; i++) {
			if (!strcmp(pszTexComp, ATextureCompTexts[i])) {
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
		GfglFeatures::self()->getSupported(GfglFeatures::TextureMaxSize);
	int tsize =
		GfglFeatures::self()->getSelected(GfglFeatures::TextureMaxSize);

	int maxsizenb = 0;
	for (i = 0; i < NMaxTextureSizes; i++) {
		if (AMaxTextureSizeTexts[i] <= sizelimit) {
			maxsizenb = i;
		} else {
			break;
		}
	}

	// Limit to available sizes.
	NMaxTextureSizes = maxsizenb + 1;

	bool found = false;
	for (i = 0; i < NMaxTextureSizes; i++) {
		if (AMaxTextureSizeTexts[i] == tsize) {
			NCurMaxTextureSizeIndex = i;
			found = true;
			break;
		}
	}

	if (!found) {
		// Should never come here if there is no bug in OpenGL.
		tsize = NDefaultTextSize;
		for (i = 0; i < NMaxTextureSizes; i++) {
			if (AMaxTextureSizeTexts[i] == tsize) {
				NCurMaxTextureSizeIndex = i;
				break;
			}
		}
	}
	
	snprintf(valuebuf, sizeof(valuebuf), "%d", AMaxTextureSizeTexts[NCurMaxTextureSizeIndex]);
	GfuiLabelSetText(ScrHandle, MaxTextureSizeLabelId, valuebuf);
	
	// 3) Multi-texturing.
	if (GfglFeatures::self()->isSupported(GfglFeatures::MultiTexturing))
	{
		const char *pszMultiTex =
			GfglFeatures::self()->isSelected(GfglFeatures::MultiTexturing)
			? GR_ATT_MULTITEXTURING_ENABLED : GR_ATT_MULTITEXTURING_DISABLED;
		for (i = 0; i < NMultiTextures; i++) {
			if (!strcmp(pszMultiTex, AMultiTextureTexts[i])) {
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
	
	// 4) Multi-sampling.
	if (GfglFeatures::self()->isSupported(GfglFeatures::MultiSampling))
	{
		const char *pszMultiSamp =
			GfglFeatures::self()->isSelected(GfglFeatures::MultiSampling)
			? GR_ATT_MULTISAMPLING_ENABLED : GR_ATT_MULTISAMPLING_DISABLED;
		for (i = 0; i < NMultiSamples; i++) {
			if (!strcmp(pszMultiSamp, AMultiSampleTexts[i])) {
				NCurMultiSampleIndex = i;
				break;
			}
		}

		GfuiLabelSetText(ScrHandle, MultiSampleLabelId,
						 AMultiSampleTexts[NCurMultiSampleIndex]);
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
	if (ScrHandle) {
		return ScrHandle;
	}

	PrevHandle = prevMenu;

	ScrHandle = GfuiScreenCreateEx((float*)NULL, NULL, onActivate, NULL, (tfuiCallback)NULL, 1);
	void *param = LoadMenuXML("opengloptionsmenu.xml");
	CreateStaticControls(param,ScrHandle);

	// Texture compression.
	TextureCompLeftButtonId =
		CreateButtonControl(ScrHandle, param, "TextureCompressionLeftArrowButton", (void*)-1,
							changeTextureCompressionState);
	TextureCompRightButtonId =
		CreateButtonControl(ScrHandle, param, "TextureCompressionRightArrowButton", (void*)+1,
							changeTextureCompressionState);
	TextureCompLabelId = CreateLabelControl(ScrHandle,param,"TextureCompressionLabel");

	// Texture sizing.
	CreateButtonControl(ScrHandle,param,"MaxTextureSizeLeftArrowButton", (void*)-1,
						changeMaxTextureSizeState);
	CreateButtonControl(ScrHandle,param,"MaxTextureSizeRightArrowButton", (void*)+1,
						changeMaxTextureSizeState);
	MaxTextureSizeLabelId = CreateLabelControl(ScrHandle,param,"MaxTextureSizeLabel");

	// Multi-texturing.
	MultiTextureLeftButtonId =
		CreateButtonControl(ScrHandle, param, "MultiTextureLeftArrowButton", (void*)-1,
							changeMultiTextureState);
	MultiTextureRightButtonId =
		CreateButtonControl(ScrHandle, param, "MultiTextureRightArrowButton", (void*)+1,
							changeMultiTextureState);
	MultiTextureLabelId = CreateLabelControl(ScrHandle,param,"MultiTextureLabel");

	// Multi-sampling.
	MultiSampleLeftButtonId =
		CreateButtonControl(ScrHandle, param, "MultiSampleLeftArrowButton", (void*)-1,
							changeMultiSampleState);
	MultiSampleRightButtonId =
		CreateButtonControl(ScrHandle, param, "MultiSampleRightArrowButton", (void*)+1,
							changeMultiSampleState);
	MultiSampleLabelId = CreateLabelControl(ScrHandle,param,"MultiSampleLabel");

	CreateButtonControl(ScrHandle,param,"AcceptButton",NULL, onAccept);
	CreateButtonControl(ScrHandle,param,"CancelButton",prevMenu, GfuiScreenActivate);

	GfParmReleaseHandle(param);

	GfuiAddKey(ScrHandle, GFUIK_RETURN, "Save", NULL, onAccept, NULL);
	GfuiAddKey(ScrHandle, GFUIK_ESCAPE, "Cancel Selection", prevMenu, GfuiScreenActivate, NULL);
	GfuiAddKey(ScrHandle, GFUIK_F1, "Help", ScrHandle, GfuiHelpScreen, NULL);
	GfuiAddKey(ScrHandle, GFUIK_F12, "Screen-Shot", NULL, GfuiScreenShot, NULL);
	GfuiAddKey(ScrHandle, GFUIK_LEFT, "Decrease Texture Size Limit", (void*)-1, changeMaxTextureSizeState, NULL);
	GfuiAddKey(ScrHandle, GFUIK_RIGHT, "Increase Texture Size Limit", (void*)1, changeMaxTextureSizeState, NULL);
	GfuiAddKey(ScrHandle, ' ', "Toggle Texture Compression", (void*)1, changeTextureCompressionState, NULL);

	return ScrHandle;
}
