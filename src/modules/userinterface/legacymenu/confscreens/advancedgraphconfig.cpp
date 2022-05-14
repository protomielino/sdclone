/***************************************************************************

    file                 :  advancedgraphconfig.cpp
    created              : Sun May  13 16:30:25 CEST 2012
    copyright            : (C) 2012 by Xavier Bertaux
    email                :  bertauxx@yahoo.fr
    version              : $Id: advancedgraphconfig.cpp 4542 2012-05-13 17:24:29Z torcs-ng $

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

    @author	<a href=mailto:bertauxx@yahoo.fr>Xavier Bertaux</a>
    @version	$Id: advancedgraphconfig.cpp 4542 2012-05-13 17:24:29Z torcs-ng $
*/

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <portability.h>
#include <tgfclient.h>
#include <graphic.h>

#include "advancedgraphconfig.h"

static const char* ShadowValues[] = { GR_ATT_SHADOW_NONE, GR_ATT_SHADOW_SM, GR_ATT_SHADOW_SSM, GR_ATT_SHADOW_PSSM, GR_ATT_SHADOW_LSPM, GR_ATT_SHADOW_VDSM};
static const int NbShadowValues = sizeof(ShadowValues) / sizeof(ShadowValues[0]);
static const char* TexSizeValues[] = { GR_ATT_SHADOW_512, GR_ATT_SHADOW_1024, GR_ATT_SHADOW_2048, GR_ATT_SHADOW_4096, GR_ATT_SHADOW_8192 };
static const int NbTexSizeValues = sizeof(TexSizeValues) / sizeof(TexSizeValues[0]);
static const char* QualityValues[] = { GR_ATT_AGR_LITTLE, GR_ATT_AGR_MEDIUM, GR_ATT_AGR_FULL };
static const int NbQualityValues = sizeof(QualityValues) / sizeof(QualityValues[0]);
static const char* ShadersValues[] = { GR_ATT_AGR_NULL, GR_ATT_AGR_LITTLE, GR_ATT_AGR_MEDIUM, GR_ATT_AGR_FULL, GR_ATT_AGR_HIGH, GR_ATT_AGR_ULTRA };
static const int NbShadersValues = sizeof(ShadersValues) / sizeof(ShadersValues[0]);

static void	*ScrHandle = NULL;

static int	ShadowLabelId, ShadowLeftButtonId, ShadowRightButtonId;
static int	TexSizeLabelId, TexSizeLeftButtonId, TexSizeRightButtonId;
static int	QualityLabelId, QualityLeftButtonId, QualityRightButtonId;
static int	ShadersLabelId, ShadersLeftButtonId, ShadersRightButtonId;

static int	ShadowIndex = 0;
static int	TexSizeIndex = 0;
static int	QualityIndex = 0;
static int  ShadersIndex = 0;

// Options IO functions ===================================================================

static void
loadOptions()
{
    void* grHandle = GfParmReadFileLocal(GR_PARAM_FILE, GFPARM_RMODE_STD | GFPARM_RMODE_CREAT);

    ShadowIndex = 0; // Default value index, in case file value not found in list.
    const char* pszShadow = GfParmGetStr(grHandle, GR_SCT_GRAPHIC, GR_ATT_SHADOW_TYPE, GR_ATT_SHADOW_NONE);

    for (int i = 0; i < NbShadowValues; i++)
    {
        if (!strcmp(pszShadow, ShadowValues[i]))
        {
            ShadowIndex = i;
            break;
        }
    }

    TexSizeIndex = 0; // Default value index, in case file value not found in list.
    const char* pszTexSize = GfParmGetStr(grHandle, GR_SCT_GRAPHIC, GR_ATT_SHADOW_SIZE, GR_ATT_SHADOW_1024);

    for (int i = 0; i < NbTexSizeValues; i++)
    {
        if (!strcmp(pszTexSize, TexSizeValues[i]))
        {
            TexSizeIndex = i;
            break;
        }
    }

    QualityIndex = 0; // Default value index, in case file value not found in list.
    const char* pszQuality = GfParmGetStr(grHandle, GR_SCT_GRAPHIC, GR_ATT_AGR_QUALITY, GR_ATT_AGR_NULL);

    for (int i = 0; i < NbQualityValues; i++)
    {
        if (!strcmp(pszQuality, QualityValues[i]))
        {
            QualityIndex = i;
            break;
        }
    }

    ShadersIndex = 0; // Default value index, in case file value not found in list.
    const char* pszShaders = GfParmGetStr(grHandle, GR_SCT_GRAPHIC, GR_ATT_SHADERS, GR_VAL_NO);

    for (int i = 0; i < NbShadersValues; i++)
    {
        if (!strcmp(pszShaders, ShadersValues[i]))
        {
            ShadersIndex = i;
            break;
        }
    }

    GfParmReleaseHandle(grHandle);
}

static void
saveOptions()
{
    // Force current edit to loose focus (if one has it) and update associated variable.
    GfuiUnSelectCurrent();

    void* grHandle = GfParmReadFileLocal(GR_PARAM_FILE, GFPARM_RMODE_STD | GFPARM_RMODE_CREAT);

    GfParmSetStr(grHandle, GR_SCT_GRAPHIC, GR_ATT_SHADOW_TYPE, ShadowValues[ShadowIndex]);
    GfParmSetStr(grHandle, GR_SCT_GRAPHIC, GR_ATT_SHADOW_SIZE, TexSizeValues[TexSizeIndex]);
    GfParmSetStr(grHandle, GR_SCT_GRAPHIC, GR_ATT_AGR_QUALITY, QualityValues[QualityIndex]);
    GfParmSetStr(grHandle, GR_SCT_GRAPHIC, GR_ATT_SHADERS, ShadersValues[ShadersIndex]);

    GfParmWriteFile(NULL, grHandle, "graph");

    GfParmReleaseHandle(grHandle);
}

// GUI callback functions ===================================================================

static void
onChangeShadow(void* vp)
{
    const long delta = (long)vp;
    ShadowIndex = (ShadowIndex + NbShadowValues + delta) % NbShadowValues;
    GfuiLabelSetText(ScrHandle, ShadowLabelId, ShadowValues[ShadowIndex]);
}

static void
onChangeTexSize(void* vp)
{
    const long delta = (long)vp;
    TexSizeIndex = (TexSizeIndex + NbTexSizeValues + delta) % NbTexSizeValues;
    GfuiLabelSetText(ScrHandle, TexSizeLabelId, TexSizeValues[TexSizeIndex]);
}

static void
onChangeQuality(void* vp)
{
    const long delta = (long)vp;
    QualityIndex = (QualityIndex + NbQualityValues + delta) % NbQualityValues;
    GfuiLabelSetText(ScrHandle, QualityLabelId, QualityValues[QualityIndex]);
}

static void
onChangeShaders(void* vp)
{
    const long delta = (long)vp;
    ShadersIndex = (ShadersIndex + NbShadersValues + delta) % NbShadersValues;
    GfuiLabelSetText(ScrHandle, ShadersLabelId, ShadersValues[ShadersIndex]);
}


static void
onActivate(void* /* dummy */)
{
    loadOptions();

    // Load GUI control values.
    onChangeShadow(0);
    onChangeTexSize(0);
    onChangeQuality(0);
    onChangeShaders(0);
}

static void
onAccept(void* tgtScrHdle)
{
    saveOptions();

    GfuiScreenActivate(tgtScrHdle);
}

static void
onCancel(void* tgtScrHdle)
{
    GfuiScreenActivate(tgtScrHdle);
}


// Menu initialization ==================================================================

void*
AdvancedGraphMenuInit(void* prevMenu)
{
    // Don't do it twice.
    if (ScrHandle)
        return ScrHandle;

    ScrHandle = GfuiScreenCreate((float*)NULL, NULL, onActivate, NULL, (tfuiCallback)NULL, 1);

    void* param = GfuiMenuLoad("advancedgraphconfigmenu.xml");

    GfuiMenuCreateStaticControls(ScrHandle, param);

    ShadowLeftButtonId =
        GfuiMenuCreateButtonControl(ScrHandle, param, "shadowleftarrow", (void*)-1, onChangeShadow);
    ShadowRightButtonId =
        GfuiMenuCreateButtonControl(ScrHandle, param, "shadowrightarrow", (void*)1, onChangeShadow);
    ShadowLabelId =	GfuiMenuCreateLabelControl(ScrHandle, param, "shadowlabel");

    TexSizeLeftButtonId =
        GfuiMenuCreateButtonControl(ScrHandle, param, "texsizeleftarrow", (void*)-1, onChangeTexSize);
    TexSizeRightButtonId =
        GfuiMenuCreateButtonControl(ScrHandle, param, "texsizerightarrow", (void*)1, onChangeTexSize);
    TexSizeLabelId = GfuiMenuCreateLabelControl(ScrHandle, param, "texsizelabel");

    QualityLeftButtonId =
        GfuiMenuCreateButtonControl(ScrHandle, param, "qualityleftarrow", (void*)-1, onChangeQuality);
    QualityRightButtonId =
        GfuiMenuCreateButtonControl(ScrHandle, param, "qualityrightarrow", (void*)1, onChangeQuality);
    QualityLabelId = GfuiMenuCreateLabelControl(ScrHandle, param, "qualitylabel");

    ShadersLeftButtonId =
        GfuiMenuCreateButtonControl(ScrHandle, param, "carleftarrow", (void*)-1, onChangeShaders);
    ShadersRightButtonId =
        GfuiMenuCreateButtonControl(ScrHandle, param, "carrightarrow", (void*)1, onChangeShaders);
    ShadersLabelId = GfuiMenuCreateLabelControl(ScrHandle, param, "carlabel");

    GfuiMenuCreateButtonControl(ScrHandle, param, "ApplyButton", prevMenu, onAccept);
    GfuiMenuCreateButtonControl(ScrHandle, param, "CancelButton", prevMenu, onCancel);

    GfParmReleaseHandle(param);

    GfuiMenuDefaultKeysAdd(ScrHandle);
    GfuiAddKey(ScrHandle, GFUIK_RETURN, "Apply", prevMenu, onAccept, NULL);
    GfuiAddKey(ScrHandle, GFUIK_ESCAPE, "Cancel", prevMenu, onCancel, NULL);

    return ScrHandle;
}
