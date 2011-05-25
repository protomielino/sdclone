/***************************************************************************
                          menu.cpp -- menu management                            
                             -------------------                                         
    created              : Fri Aug 13 22:23:19 CEST 1999
    copyright            : (C) 1999 by Eric Espie                         
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
                GUI menu management.
    @author     <a href=mailto:torcs@free.fr>Eric Espie</a>
    @version    $Id$
    @ingroup    gui
*/


#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <string>
#include <map>

#include "tgfclient.h"
#include "gui.h"
#include "guimenu.h"

void
gfuiMenuInit(void)
{
}

/** Add the default menu keyboard callback to a screen.
    The keys are:
    <br><tt>F1 .......... </tt>Help
    <br><tt>Enter ....... </tt>Perform Action
    <br><tt>Up Arrow .... </tt>Select Previous Entry
    <br><tt>Down Arrow .. </tt>Select Next Entry
    <br><tt>Page Up ..... </tt>Select Previous Entry
    <br><tt>Page Down ... </tt>Select Next Entry
    <br><tt>Tab ......... </tt>Select Next Entry
    <br><tt>F12 ......... </tt>Screen shot
    @ingroup    gui
    @param      scr     Screen Id
 */
void
GfuiMenuDefaultKeysAdd(void* scr)
{
    GfuiAddKey(scr, GFUIK_TAB, "Select Next Entry", NULL, gfuiSelectNext, NULL);
    GfuiAddKey(scr, GFUIK_RETURN, "Perform Action", (void*)2, gfuiMouseAction, NULL);
    GfuiAddKey(scr, GFUIK_UP, "Select Previous Entry", NULL, gfuiSelectPrev, NULL);
    GfuiAddKey(scr, GFUIK_DOWN, "Select Next Entry", NULL, gfuiSelectNext, NULL);
    GfuiAddKey(scr, GFUIK_PAGEUP, "Select Previous Entry", NULL, gfuiSelectPrev, NULL);
    GfuiAddKey(scr, GFUIK_PAGEDOWN, "Select Next Entry", NULL, gfuiSelectNext, NULL);
    GfuiAddKey(scr, GFUIK_F1, "Help", scr, GfuiHelpScreen, NULL);
    GfuiAddKey(scr, GFUIK_F12, "Screen-Shot", NULL, GfuiScreenShot, NULL);
}

/** Create a new menu screen.
    Set the title of the menu
    Add the default keyboard callbacks to the menu.
    @ingroup    gui
    @param      title   title of the screen
    @return     Handle of the menu
 */
void* 
GfuiMenuScreenCreate(const char* title)
{
    void* scr;

    scr = GfuiScreenCreate();
    GfuiTitleCreate(scr, title, strlen(title));

    GfuiMenuDefaultKeysAdd(scr);

    return scr;
}

static void
onFocusShowTip(void* cbinfo)
{
    GfuiVisibilitySet(((tMenuCallbackInfo*)cbinfo)->screen,
					  ((tMenuCallbackInfo*)cbinfo)->labelId, GFUI_VISIBLE);
}

static void
onFocusLostHideTip(void* cbinfo)
{
    GfuiVisibilitySet(((tMenuCallbackInfo*)cbinfo)->screen,
					  ((tMenuCallbackInfo*)cbinfo)->labelId, GFUI_INVISIBLE);
}

/***********************************************************************************
 * Menu XML descriptor management
*/

// Font size map : Gives the integer size from the size name.
typedef std::map<std::string, int> TMapFontSize;
static const TMapFontSize::value_type AMapFontSize[] = 
{ 
    TMapFontSize::value_type("big",      GFUI_FONT_BIG),
    TMapFontSize::value_type("large",    GFUI_FONT_LARGE),
    TMapFontSize::value_type("medium",   GFUI_FONT_MEDIUM),
    TMapFontSize::value_type("small",    GFUI_FONT_SMALL),

    TMapFontSize::value_type("big_c",    GFUI_FONT_BIG_C),
    TMapFontSize::value_type("large_c",  GFUI_FONT_LARGE_C),
    TMapFontSize::value_type("medium_c", GFUI_FONT_MEDIUM_C),
    TMapFontSize::value_type("small_c",  GFUI_FONT_SMALL_C),

    TMapFontSize::value_type("digit",    GFUI_FONT_DIGIT),
};

static const TMapFontSize MapFontSize(AMapFontSize, AMapFontSize + sizeof(AMapFontSize) / sizeof(TMapFontSize::value_type)); 

static int
getFontId(const char* pszFontName)
{
    const TMapFontSize::const_iterator itFontId = MapFontSize.find(pszFontName);
    
    if (itFontId != MapFontSize.end())
        return (*itFontId).second;
    else
        return GFUI_FONT_MEDIUM; // Default font.
}

// Alignment map : Gives the integer size from the size name.
typedef std::map<std::string, int> TMapAlign;
static const TMapAlign::value_type AMapAlign[] = 
{ 
    TMapAlign::value_type("left.bottom",   GFUI_ALIGN_HL_VB),
    TMapAlign::value_type("center.bottom", GFUI_ALIGN_HC_VB),
    TMapAlign::value_type("right.bottom",  GFUI_ALIGN_HR_VB),
    TMapAlign::value_type("left.center",   GFUI_ALIGN_HL_VC),
    TMapAlign::value_type("center.center", GFUI_ALIGN_HC_VC),
    TMapAlign::value_type("right.center",  GFUI_ALIGN_HR_VC),
    TMapAlign::value_type("left.top",      GFUI_ALIGN_HL_VT),
    TMapAlign::value_type("center.top",    GFUI_ALIGN_HC_VT),
    TMapAlign::value_type("right.top",     GFUI_ALIGN_HR_VT),
};

static const TMapAlign MapAlign(AMapAlign, AMapAlign + sizeof(AMapAlign) / sizeof(TMapAlign::value_type)); 

static int 
getAlignment(const char* pszAlH, const char* pszAlV)
{
    std::string strAlign(pszAlH);
    if (!pszAlH || strlen(pszAlH) == 0)
        strAlign += "left"; // Default horizontal alignment
    strAlign += '.';
    strAlign += pszAlV;
    if (!pszAlV || strlen(pszAlV) == 0)
        strAlign += "bottom"; // Default bottom alignment
    
    const TMapAlign::const_iterator itAlign = MapAlign.find(strAlign);
    
    if (itAlign != MapAlign.end())
        return (*itAlign).second;
    else
        return GFUI_ALIGN_HL_VB; // Default alignment.
}

// Horizontal alignment map : Gives the integer value from the name.
typedef std::map<std::string, int> TMapHorizAlign;
static const TMapHorizAlign::value_type AMapHorizAlign[] = 
{ 
    TMapHorizAlign::value_type("left",   GFUI_ALIGN_HL_VB),
    TMapHorizAlign::value_type("center", GFUI_ALIGN_HC_VB),
    TMapHorizAlign::value_type("right",  GFUI_ALIGN_HR_VB),
};

static const TMapHorizAlign MapHorizAlign(AMapHorizAlign, AMapHorizAlign + sizeof(AMapHorizAlign) / sizeof(TMapHorizAlign::value_type)); 

static int 
getHAlignment(const char* pszAlignH)
{
    const TMapHorizAlign::const_iterator itHorizAlign = MapHorizAlign.find(pszAlignH);
    
    if (itHorizAlign != MapHorizAlign.end())
        return (*itHorizAlign).second;
    else
        return 0; // Default horizontal alignement = left.
}

// Scrollbar position map : Gives the integer value from the name.
typedef std::map<std::string, int> TMapScrollBarPos;
static const TMapScrollBarPos::value_type AMapScrollBarPos[] = 
{ 
    TMapScrollBarPos::value_type("none",  GFUI_SB_NONE),
    TMapScrollBarPos::value_type("left",  GFUI_SB_LEFT),
    TMapScrollBarPos::value_type("right", GFUI_SB_RIGHT),
};

static const TMapScrollBarPos MapScrollBarPos(AMapScrollBarPos, AMapScrollBarPos + sizeof(AMapScrollBarPos) / sizeof(TMapScrollBarPos::value_type)); 

static int 
getScrollBarPosition(const char* pszPos)
{
    const TMapScrollBarPos::const_iterator itScrollBarPos = MapScrollBarPos.find(pszPos);
    
    if (itScrollBarPos != MapScrollBarPos.end())
        return (*itScrollBarPos).second;
    else
        return GFUI_SB_NONE; // Default horizontal alignement = left.
}

static bool
getControlColor(void* hparm, const char* pszPath, const char* pszField,
				GfuiColor &color)
{
	const char* pszValue = GfParmGetStr(hparm, pszPath, pszField, 0);
	if (!pszValue)
		return false;

	color = GfuiColor::build(pszValue);

	// GfLogDebug("getControlColor(%s) = RGBA(%.1f,%.1f,%.1f,%.1f) \n",
	// 		   pszPath, color.red, color.green, color.blue, color.alpha);

	return true;
}


static bool 
getControlBoolean(void* hparm, const char* pszPath, const char* pszFieldName, bool bDefault)
{
	const char* pszValue = GfParmGetStr(hparm, pszPath, pszFieldName, 0);
	if (pszValue)
	{
		if (!strcmp(pszValue, "yes") || !strcmp(pszValue, "true"))
			return true;
		else if (!strcmp(pszValue, "no") || !strcmp(pszValue, "false"))
			return false;
	}

	return bDefault;
}

static int 
createStaticImage(void* hscr, void* hparm, const char* pszName)
{
	const char* pszImage = GfParmGetStr(hparm, pszName, "image", "");

	const int x = (int)GfParmGetNum(hparm,pszName,"x",NULL,0.0);
	const int y = (int)GfParmGetNum(hparm,pszName,"y",NULL,0.0);
	const int w = (int)GfParmGetNum(hparm,pszName,"width",NULL,0.0);
	const int h = (int)GfParmGetNum(hparm,pszName,"height",NULL,0.0);

	const char* pszAlignH = GfParmGetStr(hparm, pszName, "alignH", "");
	const char* pszAlignV = GfParmGetStr(hparm, pszName, "alignV", "");
	const int alignment = getAlignment(pszAlignH,pszAlignV);
	const bool canDeform = getControlBoolean(hparm, pszName, "canDeform", true);

	int id = GfuiStaticImageCreate(hscr,x,y,w,h,pszImage,alignment,canDeform);

	char pszImageFieldName[32];
	for (int i=2; i<= MAX_STATIC_IMAGES;i++)
	{
		sprintf(pszImageFieldName, "image%i", i);
		const char* pszFileName = GfParmGetStr(hparm, pszName, pszImageFieldName, 0);
		if (pszFileName)
			GfuiStaticImageSet(hscr, id, pszFileName, i-1, canDeform);
		else
			break; // Assume indexed image list has no hole inside.
	}

	return id;
}

static int 
createBackgroundImage(void* hscr, void* hparm, const char* pszName)
{
	const char* pszImage = GfParmGetStr(hparm, pszName, "image", "");
	GfuiScreenAddBgImg(hscr, pszImage);
	return 1;
}

int
GfuiMenuCreateStaticImageControl(void* hscr, void* hparm, const char* pszName)
{
	std::string strControlPath("dynamiccontrols/");
	strControlPath += pszName;
        
	return createStaticImage(hscr, hparm, strControlPath.c_str());
}

static int 
createLabel(void* hscr, void* hparm, const char* pszPath,
			bool bFromTemplate = false,
			const char* text = GFUI_TPL_TEXT, int x = GFUI_TPL_X, int y = GFUI_TPL_Y,
			int font = GFUI_TPL_FONTID, int align = GFUI_TPL_ALIGN, int maxlen = GFUI_TPL_MAXLEN, 
			const float* aFgColor = GFUI_TPL_COLOR,
			const float* aFgFocusColor = GFUI_TPL_FOCUSCOLOR)
{
	if (strcmp(GfParmGetStr(hparm, pszPath, "type", ""), "label"))
	{
		GfLogError("Failed to create label control '%s' : not a 'label'\n", pszPath);
		return -1;
	}
        
	const char* pszText =
		bFromTemplate && text != GFUI_TPL_TEXT
		? text : GfParmGetStr(hparm, pszPath, "text", "");
	const int nX = 
		bFromTemplate && x != GFUI_TPL_X
		? x : (int)GfParmGetNum(hparm, pszPath, "x", NULL, 0);
	const int nY = 
		bFromTemplate && y != GFUI_TPL_Y
		? y : (int)GfParmGetNum(hparm, pszPath, "y", NULL, 0);
	const int nFontId = 
		bFromTemplate && font != GFUI_TPL_FONTID
		? font : getFontId(GfParmGetStr(hparm, pszPath, "font", ""));
	const char* pszAlignH = GfParmGetStr(hparm, pszPath, "alignH", "");
	const char* pszAlignV = GfParmGetStr(hparm, pszPath, "alignV", "");
	const int nAlign = 
		bFromTemplate && align != GFUI_TPL_ALIGN
		? align : getAlignment(pszAlignH, pszAlignV);
	int nMaxLen = 
		bFromTemplate && maxlen != GFUI_TPL_MAXLEN
		? maxlen : (int)GfParmGetNum(hparm, pszPath, "maxlen", NULL, 0);
	
	GfuiColor color;
	const float* aColor = 0;
	if (bFromTemplate && aFgColor != GFUI_TPL_COLOR)
		aColor = aFgColor;
	else if (getControlColor(hparm, pszPath, "color", color))
		aColor = color.toFloatRGBA();

	GfuiColor focusColor;
	const float* aFocusColor = 0;
	if (bFromTemplate && aFgFocusColor != GFUI_TPL_FOCUSCOLOR)
		aFocusColor = aFgFocusColor;
	else if (getControlColor(hparm, pszPath, "focuscolor", focusColor))
		aFocusColor = focusColor.toFloatRGBA();

	void* userDataOnFocus = 0;
	tfuiCallback onFocus = 0;
	tfuiCallback onFocusLost = 0;
	const char* pszTip = GfParmGetStr(hparm, pszPath, "tip", 0);
	if (pszTip && strlen(pszTip) > 0)
	{
		tMenuCallbackInfo * cbinfo = (tMenuCallbackInfo*)calloc(1, sizeof(tMenuCallbackInfo));
		cbinfo->screen = hscr;
		cbinfo->labelId = GfuiTipCreate(hscr, pszTip, strlen(pszTip));
		GfuiVisibilitySet(hscr, cbinfo->labelId, GFUI_INVISIBLE);

		userDataOnFocus = (void*)cbinfo;
		onFocus = onFocusShowTip;
		onFocusLost = onFocusLostHideTip;
	}

	int labelId = GfuiLabelCreate(hscr, pszText, nFontId, nX, nY, nAlign, nMaxLen,
								  aColor, aFocusColor, userDataOnFocus, onFocus, onFocusLost);

    return labelId;
}


int 
GfuiMenuCreateLabelControl(void* hscr, void* hparm, const char* pszName,
						   bool bFromTemplate,
						   const char* text, int x, int y, int font, int align, int maxlen, 
						   const float* fgColor, const float* fgFocusColor)
{
	std::string strControlPath = bFromTemplate ? "templatecontrols/" : "dynamiccontrols/";
	strControlPath += pszName;

	return createLabel(hscr, hparm, strControlPath.c_str(), bFromTemplate,
					   text, x, y, font, align, maxlen, fgColor, fgFocusColor);
}

static int 
createTextButton(void* hscr, void* hparm, const char* pszPath,
				 void* userDataOnPush, tfuiCallback onPush,
				 void* userDataOnFocus, tfuiCallback onFocus, tfuiCallback onFocusLost,
				 bool bFromTemplate = false,
				 const char* text = GFUI_TPL_TEXT,
				 const char* tip = GFUI_TPL_TIP,
				 int x = GFUI_TPL_X, int y = GFUI_TPL_Y,
				 int width = GFUI_TPL_WIDTH,
				 int font = GFUI_TPL_FONTID, int align = GFUI_TPL_ALIGN,
				 const float* aFgColor = GFUI_TPL_COLOR,
				 const float* aFgFocusColor = GFUI_TPL_FOCUSCOLOR,
				 const float* aFgPushedColor = GFUI_TPL_PUSHEDCOLOR)
{
	if (strcmp(GfParmGetStr(hparm, pszPath, "type", ""), "textbutton"))
	{
		GfLogError("Failed to create text button control '%s' : not a 'textbutton'\n",
				   pszPath);
		return -1;
	}
        
	const char* pszText =
		bFromTemplate && text != GFUI_TPL_TEXT
		? text : GfParmGetStr(hparm, pszPath, "text", "");
	const char* pszTip =
		bFromTemplate && tip != GFUI_TPL_TIP
		? tip : GfParmGetStr(hparm, pszPath, "tip", "");
	const int nX = 
		bFromTemplate && x != GFUI_TPL_X
		? x : (int)GfParmGetNum(hparm, pszPath, "x", NULL, 0);
	const int nY = 
		bFromTemplate && y != GFUI_TPL_Y
		? y : (int)GfParmGetNum(hparm, pszPath, "y", NULL, 0);
	int nWidth = 
		bFromTemplate && width != GFUI_TPL_WIDTH
		? width : (int)GfParmGetNum(hparm, pszPath, "width", NULL, 0);
	if (nWidth == 0)
		nWidth = GFUI_BTNSZ;
	const int nFontId = 
		bFromTemplate && font != GFUI_TPL_FONTID
		? font : getFontId(GfParmGetStr(hparm, pszPath, "font", ""));
	const char* pszAlignH = GfParmGetStr(hparm, pszPath, "alignH", "");
	// TODO: Fix vertical alignement issue (only the default 'bottom' works).
	//const char* pszAlignV = GfParmGetStr(hparm, pszPath, "alignV", "");
	const int nAlign = 
		bFromTemplate && align != GFUI_TPL_ALIGN
		? align : getHAlignment(pszAlignH); //getAlignment(pszAlignH, pszAlignV);
	
	GfuiColor color;
	const float* aColor = 0;
	if (bFromTemplate && aFgColor != GFUI_TPL_COLOR)
		aColor = aFgColor;
	else if (getControlColor(hparm, pszPath, "color", color))
		aColor = color.toFloatRGBA();

	GfuiColor focusColor;
	const float* aFocusColor = 0;
	if (bFromTemplate && aFgFocusColor != GFUI_TPL_FOCUSCOLOR)
		aFocusColor = aFgFocusColor;
	else if (getControlColor(hparm, pszPath, "focuscolor", focusColor))
		aFocusColor = focusColor.toFloatRGBA();

	GfuiColor pushedColor;
	const float* aPushedColor = 0;
	if (bFromTemplate && aFgPushedColor != GFUI_TPL_PUSHEDCOLOR)
		aPushedColor = aFgPushedColor;
	else if (getControlColor(hparm, pszPath, "pushedcolor", pushedColor))
		aPushedColor = pushedColor.toFloatRGBA();

	if (pszTip && strlen(pszTip) > 0)
	{
		tMenuCallbackInfo * cbinfo = (tMenuCallbackInfo*)calloc(1, sizeof(tMenuCallbackInfo));
		cbinfo->screen = hscr;
		cbinfo->labelId = GfuiTipCreate(hscr, pszTip, strlen(pszTip));
		GfuiVisibilitySet(hscr, cbinfo->labelId, GFUI_INVISIBLE);

		// TODO: In this case, we crudely overwrite onFocus/onFocusLost !
		userDataOnFocus = (void*)cbinfo;
		onFocus = onFocusShowTip;
		onFocusLost = onFocusLostHideTip;
	}

	int butId = GfuiButtonCreate(hscr, pszText, nFontId,
								 nX, nY, nWidth, nAlign, GFUI_MOUSE_UP,
								 userDataOnPush, onPush,
								 userDataOnFocus, onFocus, onFocusLost);

	const bool bShowbox = getControlBoolean(hparm, pszPath, "showbox", true);

	GfuiButtonShowBox(hscr, butId, bShowbox);

	const char* pszDisabledImage = GfParmGetStr(hparm, pszPath, "disabledimage", "");
	const char* pszEnabledImage = GfParmGetStr(hparm, pszPath, "enabledimage", "");
	const char* pszFocusedImage = GfParmGetStr(hparm, pszPath, "focusedimage", "");
	const char* pszPushedImage = GfParmGetStr(hparm, pszPath, "pushedimage", "");

	const int imgX = (int)GfParmGetNum(hparm, pszPath, "imagex", NULL, 0.0);
	const int imgY = (int)GfParmGetNum(hparm, pszPath, "imagey", NULL, 0.0);
	const int imgWidth = (int)GfParmGetNum(hparm, pszPath, "imagewidth",NULL, 20.0);
	const int imgHeight = (int)GfParmGetNum(hparm, pszPath, "imageheight",NULL, 20.0);

	GfuiButtonSetImage(hscr, butId, imgX, imgY, imgWidth, imgHeight,
					   pszDisabledImage, pszEnabledImage,
					   pszFocusedImage, pszPushedImage);

	GfuiColor c;
	const bool bColor = getControlColor(hparm, pszPath, "color", c);
	if (bColor)
		GfuiButtonSetColor(hscr, butId, c);

	GfuiColor fc;
	const bool bFocusColor = getControlColor(hparm, pszPath, "focuscolor", fc);
	if (bFocusColor)
		GfuiButtonSetFocusColor(hscr, butId, fc);
        
	GfuiColor pc;
	const bool bPushedColor = getControlColor(hparm, pszPath, "pushedcolor", pc);
	if (bPushedColor)
		GfuiButtonSetPushedColor(hscr, butId, pc);
        
	return butId;
}

static int 
createImageButton(void* hscr, void* hparm, const char* pszPath,
				  void* userDataOnPush, tfuiCallback onPush,
				  void* userDataOnFocus, tfuiCallback onFocus, tfuiCallback onFocusLost,
				  bool bFromTemplate = false,
				  const char* tip = GFUI_TPL_TIP,
				  int x = GFUI_TPL_X, int y = GFUI_TPL_Y,
				  int width = GFUI_TPL_WIDTH, int height = GFUI_TPL_HEIGHT,
				  int align = GFUI_TPL_ALIGN)
{
	if (strcmp(GfParmGetStr(hparm, pszPath, "type", ""), "imagebutton"))
	{
		GfLogError("Failed to create image button control '%s' : not an 'imagebutton'\n",
				   pszPath);
		return -1;
	}
        
	const char* pszTip =
		bFromTemplate && tip != GFUI_TPL_TIP
		? tip : GfParmGetStr(hparm, pszPath, "tip", "");
	const int nX = 
		bFromTemplate && x != GFUI_TPL_X
		? x : (int)GfParmGetNum(hparm, pszPath, "x", NULL, 0);
	const int nY = 
		bFromTemplate && y != GFUI_TPL_Y
		? y : (int)GfParmGetNum(hparm, pszPath, "y", NULL, 0);
	int nWidth = 
		bFromTemplate && width != GFUI_TPL_WIDTH
		? width : (int)GfParmGetNum(hparm, pszPath, "width", NULL, 0);
	int nHeight = 
		bFromTemplate && height != GFUI_TPL_HEIGHT
		? height : (int)GfParmGetNum(hparm, pszPath, "height", NULL, 0);
	const char* pszAlignH = GfParmGetStr(hparm, pszPath, "alignH", "");
	const char* pszAlignV = GfParmGetStr(hparm, pszPath, "alignV", "");
	const int nAlign = 
		bFromTemplate && align != GFUI_TPL_ALIGN
		? align : getAlignment(pszAlignH, pszAlignV);

	if (strlen(pszTip) > 0)
	{
		tMenuCallbackInfo * cbinfo = (tMenuCallbackInfo*)calloc(1, sizeof(tMenuCallbackInfo));
		cbinfo->screen = hscr;
		cbinfo->labelId = GfuiTipCreate(hscr, pszTip, strlen(pszTip));
		GfuiVisibilitySet(hscr, cbinfo->labelId, GFUI_INVISIBLE);

		// TODO: In this case, we crudely overwrite onFocus/onFocusLost !
		userDataOnFocus = (void*)cbinfo;
		onFocus = onFocusShowTip;
		onFocusLost = onFocusLostHideTip;
	}

	const char* pszDisabledImage = GfParmGetStr(hparm, pszPath, "disabledimage", "");
	const char* pszEnabledImage = GfParmGetStr(hparm, pszPath, "enabledimage", "");
	const char* pszFocusedImage = GfParmGetStr(hparm, pszPath, "focusedimage", "");
	const char* pszPushedImage = GfParmGetStr(hparm, pszPath, "pushedimage", "");

	int butId =
		GfuiGrButtonCreate(hscr,
						   pszDisabledImage, pszEnabledImage, pszFocusedImage, pszPushedImage,
						   nX, nY, nWidth, nHeight, nAlign, GFUI_MOUSE_UP,
						   userDataOnPush, onPush,
						   userDataOnFocus, onFocus, onFocusLost);

	return butId;
}

int 
GfuiMenuCreateTextButtonControl(void* hscr, void* hparm, const char* pszName,
								void* userDataOnPush, tfuiCallback onPush,
								void* userDataOnFocus, tfuiCallback onFocus, tfuiCallback onFocusLost,
								bool bFromTemplate,
								const char* text, const char* tip,
								int x, int y, int width, int font, int align,
								const float* fgColor, const float* fgFocusColor, const float* fgPushedColor)
{
	std::string strControlPath = bFromTemplate ? "templatecontrols/" : "dynamiccontrols/";
	strControlPath += pszName;

	return createTextButton(hscr, hparm, strControlPath.c_str(),
							userDataOnPush, onPush,
							userDataOnFocus, onFocus, onFocusLost,
							bFromTemplate,
							text, tip, x, y, width, font, align,
							fgColor, fgFocusColor, fgPushedColor);
}

int 
GfuiMenuCreateImageButtonControl(void* hscr, void* hparm, const char* pszName,
								 void* userDataOnPush, tfuiCallback onPush,
								 void* userDataOnFocus, tfuiCallback onFocus, tfuiCallback onFocusLost,
								 bool bFromTemplate,
								 const char* tip, int x, int y, int width, int height, int align)
{
	std::string strControlPath = bFromTemplate ? "templatecontrols/" : "dynamiccontrols/";
	strControlPath += pszName;

	return createImageButton(hscr, hparm, strControlPath.c_str(),
							 userDataOnPush, onPush,
							 userDataOnFocus, onFocus, onFocusLost,
							 bFromTemplate,
							 tip, x, y, width, height, align);
}

int 
GfuiMenuCreateButtonControl(void* hscr, void* hparm, const char* pszName,
							void* userDataOnPush, tfuiCallback onPush,
							void* userDataOnFocus, tfuiCallback onFocus, tfuiCallback onFocusLost)
{
	std::string strControlPath = "dynamiccontrols/";
	strControlPath += pszName;

	const char* pszType = GfParmGetStr(hparm, strControlPath.c_str(), "type", "");
	if (!strcmp(pszType, "textbutton"))
		return createTextButton(hscr, hparm, strControlPath.c_str(),
								userDataOnPush, onPush,
								userDataOnFocus, onFocus, onFocusLost);
	else if(!strcmp(pszType, "imagebutton"))
		return createImageButton(hscr, hparm, strControlPath.c_str(),
								 userDataOnPush, onPush,
								 userDataOnFocus, onFocus, onFocusLost);
	else
		GfLogError("Failed to create button control '%s' of unknown type '%s'\n",
				   pszName, pszType);

	return -1;
}

int 
GfuiMenuCreateEditControl(void* hscr, void* hparm, const char* pszName,
						  void* userDataOnFocus, tfuiCallback onFocus, tfuiCallback onFocusLost)
{
	std::string strControlPath("dynamiccontrols/");
	strControlPath += pszName;

	const char* pszType = GfParmGetStr(hparm, strControlPath.c_str(), "type", "");
	if (strcmp(pszType, "editbox"))
	{
		GfLogError("Failed to create control '%s' : not an 'editbox' \n", pszName);
		return -1;
	}

	// TODO : Add real support for tips (the onFocus/onFocusLost system is already used
	//        for user input management)
	//         const char* pszTip = GfParmGetStr(hparm, pszName, "tip", "");
	//         if (strlen(pszTip) > 0)
	//         {
	//                 tMenuCallbackInfo * cbinfo = (tMenuCallbackInfo*)calloc(1, sizeof(tMenuCallbackInfo));
	//                 cbinfo->screen = hscr;
	//                 cbinfo->labelId = GfuiTipCreate(hscr, pszTip, strlen(pszTip));
	//                 GfuiVisibilitySet(hscr, cbinfo->labelId, GFUI_INVISIBLE);
	//
	//                 // TODO: In this case, we simply ignore onFocus/onFocusLost !
	//                 userDataOnFocus = (void*)cbinfo;
	//                 onFocus = onFocusShowTip;
	//                 onFocusLost = onFocusLostHideTip;
	//         }

	const char* pszText = GfParmGetStr(hparm, strControlPath.c_str(), "text", "");
	const int x = (int)GfParmGetNum(hparm,strControlPath.c_str(),"x",NULL,0.0);
	const int y = (int)GfParmGetNum(hparm,strControlPath.c_str(),"y",NULL,0.0);
	const char* pszFontName = GfParmGetStr(hparm, strControlPath.c_str(), "font", "");
	const int font = getFontId(pszFontName);
        
	int width = (int)GfParmGetNum(hparm,strControlPath.c_str(),"width",NULL,0.0);
	if (width == 0)
		width =  GFUI_BTNSZ;

	const int maxlen = (int)GfParmGetNum(hparm,strControlPath.c_str(),"maxlen",NULL,0.0);

	int id = GfuiEditboxCreate(hscr,pszText, font,x,y,width,maxlen,
							   userDataOnFocus,onFocus,onFocusLost);

	GfuiColor c,fc;
	const bool bColor = getControlColor(hparm,pszName,"color",c);
	const bool bFocusColor = getControlColor(hparm,pszName,"focuscolor",fc);

	if (bColor)
		GfuiEditboxSetColor(hscr,id,c);
        
	if (bFocusColor)
		GfuiEditboxSetFocusColor(hscr,id,fc);             

	return id;
}

int 
GfuiMenuCreateComboboxControl(void* hscr, void* hparm, const char* pszName,void* userData,tfuiComboboxCallback onChange)
{
	std::string strControlPath("dynamiccontrols/");
	strControlPath += pszName;

	const std::string strType = GfParmGetStr(hparm, strControlPath.c_str(), "type", "");
	if (strType != "combobox")
		return -1;

	int id = -1;
	
	const int x = (int)GfParmGetNum(hparm,strControlPath.c_str(),"x",NULL,0.0);
	const int y = (int)GfParmGetNum(hparm,strControlPath.c_str(),"y",NULL,0.0);

	std::string strFontName = GfParmGetStr(hparm, strControlPath.c_str(), "font", "");
	const int font = getFontId(strFontName.c_str());

	const char*  pszAlignH = GfParmGetStr(hparm, strControlPath.c_str(), "alignH", "");
	const char*  pszAlignV = GfParmGetStr(hparm, strControlPath.c_str(), "alignV", "");
	const int align = getAlignment(pszAlignH,pszAlignV);

	
	int width = (int)GfParmGetNum(hparm,strControlPath.c_str(),"width",NULL,0.0);
	if (width == 0)
	    width = 200;

    const char* pszText = GfParmGetStr(hparm, strControlPath.c_str(), "text", "");

	const char* pszTip = GfParmGetStr(hparm, strControlPath.c_str(), "tip", 0);
	
	void* userDataOnFocus = 0;
	tfuiCallback onFocus = 0;
	tfuiCallback onFocusLost = 0;
	if (pszTip && strlen(pszTip) > 0)
	{
		tMenuCallbackInfo * cbinfo = (tMenuCallbackInfo*)calloc(1, sizeof(tMenuCallbackInfo));
		cbinfo->screen = hscr;
		cbinfo->labelId = GfuiTipCreate(hscr, pszTip, strlen(pszTip));
		GfuiVisibilitySet(hscr, cbinfo->labelId, GFUI_INVISIBLE);
		
		userDataOnFocus = (void*)cbinfo;
		onFocus = onFocusShowTip;
		onFocusLost = onFocusLostHideTip;
	}

	const float* aColor = 0;
	GfuiColor color;
	if (getControlColor(hparm, strControlPath.c_str(), "color", color))
		aColor = color.toFloatRGBA();
	
	const float* aFocusColor = 0;
	GfuiColor focusColor;
	if (getControlColor(hparm, strControlPath.c_str(), "focuscolor", focusColor))
		aFocusColor = focusColor.toFloatRGBA();
	
	id = GfuiComboboxCreate(hscr, font, x, y, width, align, 0, pszText,
							aColor, aFocusColor,
							userData, onChange, userDataOnFocus, onFocus, onFocusLost);

	return id;
}

int 
GfuiMenuCreateScrollListControl(void* hscr, void* hparm, const char* pszName,void* userData, tfuiCallback onSelect)
{
	std::string strControlPath("dynamiccontrols/");
	strControlPath += pszName;

	const char* pszType = GfParmGetStr(hparm, strControlPath.c_str(), "type", "");
	if (strcmp(pszType, "scrolllist"))
	{
		GfLogError("Failed to create control '%s' : not a 'scrolllist' \n", pszName);
		return -1;
	}

	const int x = (int)GfParmGetNum(hparm,strControlPath.c_str(),"x",NULL,0.0);
	const int y = (int)GfParmGetNum(hparm,strControlPath.c_str(),"y",NULL,0.0);
	const int w = (int)GfParmGetNum(hparm,strControlPath.c_str(),"width",NULL,0.0);
	const int h = (int)GfParmGetNum(hparm,strControlPath.c_str(),"height",NULL,0.0);
        
	const char* pszFontName = GfParmGetStr(hparm, strControlPath.c_str(), "font", "");
	const int font = getFontId(pszFontName);

	const char* pszAlignH = GfParmGetStr(hparm, pszName, "alignH", "");
	const char* pszAlignV = GfParmGetStr(hparm, pszName, "alignV", "");
	const int alignment = getAlignment(pszAlignH,pszAlignV);

	const char* pszScrollBarPos = GfParmGetStr(hparm,strControlPath.c_str(),"scrollbarposition","none");
	int scrollbarpos = getScrollBarPosition(pszScrollBarPos);

	int id = GfuiScrollListCreate(hscr, font,x,y,alignment,w,h,scrollbarpos,userData,onSelect);

	GfuiColor c,sc;
	bool bColor = getControlColor(hparm,pszName,"color",c);
	bool bSelectColor = getControlColor(hparm,pszName,"selectcolor",sc);
        
	if (bColor)
		GfuiScrollListSetColor(hscr,id,c);

	if (bSelectColor)
		GfuiScrollListSetSelectColor(hscr,id,sc);

	return id;
}

int 
GfuiMenuCreateCheckboxControl(void* hscr, void* hparm, const char* pszName,void* userData,tfuiCheckboxCallback onChange)
{
	std::string strControlPath("dynamiccontrols/");
	strControlPath += pszName;

	const std::string strType = GfParmGetStr(hparm, strControlPath.c_str(), "type", "");
	if (strType != "checkbox")
		return -1;

	int id = -1;
	
	std::string strText,strTip;
	int font;
	int x,y,imagewidth,imageheight;

	x = (int)GfParmGetNum(hparm,strControlPath.c_str(),"x",NULL,0.0);
	y = (int)GfParmGetNum(hparm,strControlPath.c_str(),"y",NULL,0.0);

	std::string strFontName = GfParmGetStr(hparm, strControlPath.c_str(), "font", "");
	font = getFontId(strFontName.c_str());

    const char* pszText = GfParmGetStr(hparm, strControlPath.c_str(), "text", "");

	const char*  pszAlignH = GfParmGetStr(hparm, strControlPath.c_str(), "alignH", "");
	const char*  pszAlignV = GfParmGetStr(hparm, strControlPath.c_str(), "alignV", "");
	int align = getAlignment(pszAlignH,pszAlignV);

	
	imagewidth = (int)GfParmGetNum(hparm,strControlPath.c_str(),"imagewidth",NULL,0.0);
	if (imagewidth == 0)
	    imagewidth = 30;

	imageheight = (int)GfParmGetNum(hparm,strControlPath.c_str(),"imageheight",NULL,0.0);
	if (imageheight == 0)
	    imageheight = 30;

    const bool bChecked = getControlBoolean(hparm,strControlPath.c_str(),"checked", true);

	const char* pszTip = GfParmGetStr(hparm, strControlPath.c_str(), "tip", "");
	
	void* userDataOnFocus = 0;
	tfuiCallback onFocus = 0;
	tfuiCallback onFocusLost = 0;
	if (strlen(pszTip) > 0)
	{
		tMenuCallbackInfo * cbinfo = (tMenuCallbackInfo*)calloc(1, sizeof(tMenuCallbackInfo));
		cbinfo->screen = hscr;
		cbinfo->labelId = GfuiTipCreate(hscr, pszTip, strlen(pszTip));
		GfuiVisibilitySet(hscr, cbinfo->labelId, GFUI_INVISIBLE);
		
		userDataOnFocus = (void*)cbinfo;
		onFocus = onFocusShowTip;
		onFocusLost = onFocusLostHideTip;
	}


	id = GfuiCheckboxCreate(hscr, font, x, y, imagewidth, imageheight, align, 0,
							pszText, bChecked, userData, onChange,
							userDataOnFocus, onFocus, onFocusLost);

	GfuiColor c;
	bool bColor = getControlColor(hparm,pszName,"color",c);
	if(bColor)
		GfuiComboboxSetTextColor(hscr,id,c);

	return id;
}


int 
GfuiMenuCreateProgressbarControl(void* hscr, void* hparm, const char* pszName)
{
	std::string strControlPath("dynamiccontrols/");
	strControlPath += pszName;
	
	const std::string strType = GfParmGetStr(hparm, strControlPath.c_str(), "type", "");
	if (strType != "progressbar")
		return -1;
	
	const char* pszProgressbackgroundImage = GfParmGetStr(hparm, strControlPath.c_str(), "image", "data/img/progressbackground.png");
	const char* pszProgressbarImage = GfParmGetStr(hparm, pszName, "image", "data/img/progressbar.png");
	
	const int x = (int)GfParmGetNum(hparm,strControlPath.c_str(),"x",NULL,0.0);
	const int y = (int)GfParmGetNum(hparm,strControlPath.c_str(),"y",NULL,0.0);
	const int w = (int)GfParmGetNum(hparm,strControlPath.c_str(),"width",NULL,0.0);
	const int h = (int)GfParmGetNum(hparm,strControlPath.c_str(),"height",NULL,0.0);
	
	const char* pszAlignH = GfParmGetStr(hparm, strControlPath.c_str(), "alignH", "");
	const char* pszAlignV = GfParmGetStr(hparm, strControlPath.c_str(), "alignV", "");
	const float min = GfParmGetNum(hparm, strControlPath.c_str(), "min",NULL,0.0);
	const float max = GfParmGetNum(hparm, strControlPath.c_str(), "max",NULL,100.0);
	const float value = GfParmGetNum(hparm, strControlPath.c_str(), "value",NULL,100.0);
	const int alignment = getAlignment(pszAlignH,pszAlignV);
	
	const char* pszTip = GfParmGetStr(hparm, strControlPath.c_str(), "tip", "");
	
	void* userDataOnFocus = 0;
	tfuiCallback onFocus = 0;
	tfuiCallback onFocusLost = 0;
	if (strlen(pszTip) > 0)
	{
		tMenuCallbackInfo * cbinfo = (tMenuCallbackInfo*)calloc(1, sizeof(tMenuCallbackInfo));
		cbinfo->screen = hscr;
		cbinfo->labelId = GfuiTipCreate(hscr, pszTip, strlen(pszTip));
		GfuiVisibilitySet(hscr, cbinfo->labelId, GFUI_INVISIBLE);
		
		userDataOnFocus = (void*)cbinfo;
		onFocus = onFocusShowTip;
		onFocusLost = onFocusLostHideTip;
	}

	int id = GfuiProgressbarCreate(hscr, x, y, w, h,
								   pszProgressbackgroundImage, pszProgressbarImage,
								   alignment, min, max, value,
								   userDataOnFocus, onFocus, onFocusLost);
	
	return id;
}

bool 
GfuiMenuCreateStaticControls(void* hscr, void* hparm)
{
	if (!hparm)
	{
		GfLogError("Failed to create static controls (XML menu descriptor not yet loaded)\n");
		return false;
	}

    char buf[32];

    for (int i = 1; i <= GfParmGetEltNb(hparm, "staticcontrols"); i++)
    {
		sprintf(buf, "staticcontrols/%d", i);
		const char* pszType = GfParmGetStr(hparm, buf, "type", "");
    
		if (!strcmp(pszType, "label"))
		{
			createLabel(hscr, hparm, buf);
		}
		else if (!strcmp(pszType, "staticimage"))
		{
			createStaticImage(hscr, hparm, buf);
		}
		else if (!strcmp(pszType, "backgroundimage"))
		{
			createBackgroundImage(hscr, hparm, buf);
		}
		else
		{
			GfLogWarning("Failed to create static control '%s' of unknown type '%s'\n",
						 pszType);
		}
    }

    return true;
}

void* 
GfuiMenuLoad(const char* pszMenuPath)
{
	std::string strPath("data/menu/");
	strPath += pszMenuPath;
        
	char buf[1024];
	sprintf(buf, "%s%s", GfDataDir(), strPath.c_str());

	return GfParmReadFile(buf, GFPARM_RMODE_STD);
}

tdble
GfuiMenuGetNumProperty(void* hparm, const char* pszName, tdble nDefVal, const char* pszUnit)
{
	return GfParmGetNum(hparm, "properties", pszName, pszUnit, nDefVal);
}

const char*
GfuiMenuGetStrProperty(void* hparm, const char* pszName, const char* pszDefVal)
{
	return GfParmGetStr(hparm, "properties", pszName, pszDefVal);
}

//===================================================================================
// GfuiMenuScreen class implementation

struct gfuiMenuPrivateData
{
	void* menuHdle;
	void* prevMenuHdle;
	std::string strXMLDescFileName;
	void* xmlDescParmHdle;
	std::map<std::string, int> mapControlIds;
};


GfuiMenuScreen::GfuiMenuScreen(const char* pszXMLDescFile)
: m_priv(new gfuiMenuPrivateData)
{
	m_priv->menuHdle = 0;
	m_priv->prevMenuHdle = 0;
	m_priv->strXMLDescFileName = pszXMLDescFile;
	m_priv->xmlDescParmHdle = 0;
	m_priv->prevMenuHdle = 0;
}

GfuiMenuScreen::~GfuiMenuScreen()
{
	closeXMLDescriptor();
	if (m_priv->menuHdle)
		GfuiScreenRelease(m_priv->menuHdle);
	delete m_priv;
}

void GfuiMenuScreen::createMenu(float* bgColor, 
								void* userDataOnActivate, tfuiCallback onActivate, 
								void* userDataOnDeactivate, tfuiCallback onDeactivate, 
								int mouseAllowed)
{
	m_priv->menuHdle = GfuiScreenCreate(bgColor, userDataOnActivate, onActivate, 
										  userDataOnDeactivate, onDeactivate, mouseAllowed);
}

void GfuiMenuScreen::setMenuHandle(void* hdle)
{
	m_priv->menuHdle = hdle;
}

void* GfuiMenuScreen::getMenuHandle() const
{
	return m_priv->menuHdle;
}

void GfuiMenuScreen::setPreviousMenuHandle(void* hdle)
{
	m_priv->prevMenuHdle = hdle;
}

void* GfuiMenuScreen::getPreviousMenuHandle() const
{
	return m_priv->prevMenuHdle;
}

bool GfuiMenuScreen::openXMLDescriptor()
{
    m_priv->xmlDescParmHdle = GfuiMenuLoad(m_priv->strXMLDescFileName.c_str());
	return m_priv->xmlDescParmHdle != 0;
}

bool GfuiMenuScreen::closeXMLDescriptor()
{
	if (m_priv->xmlDescParmHdle)
	{
		GfParmReleaseHandle(m_priv->xmlDescParmHdle);
		m_priv->xmlDescParmHdle = 0;
		return true;
	}
	return false;
}

bool GfuiMenuScreen::createStaticControls()
{
	if (!m_priv->xmlDescParmHdle && !openXMLDescriptor())
		return false;
	
	return m_priv->menuHdle && m_priv->xmlDescParmHdle
		   && ::GfuiMenuCreateStaticControls(m_priv->menuHdle, m_priv->xmlDescParmHdle);
}

int GfuiMenuScreen::createButtonControl(const char* pszName,
										void* userDataOnPush, tfuiCallback onPush,
										void* userDataOnFocus, tfuiCallback onFocus, tfuiCallback onFocusLost)
{
	if (!m_priv->xmlDescParmHdle && !openXMLDescriptor())
		return -1;
	
	if (m_priv->mapControlIds.find(pszName) == m_priv->mapControlIds.end())
	{
		const int nCtrlId =
			::GfuiMenuCreateButtonControl(m_priv->menuHdle, m_priv->xmlDescParmHdle, pszName,
										  userDataOnPush, onPush,
										  userDataOnFocus, onFocus, onFocusLost);
		if (nCtrlId >= 0)
			m_priv->mapControlIds[pszName] = nCtrlId;

		return nCtrlId;
	}

	GfLogError("Failed to create button control '%s' : duplicate name\n", pszName);
	return -1;
}

int GfuiMenuScreen::createTextButtonControl(const char* pszName, void* userDataOnPush,
											tfuiCallback onPush,
											void* userDataOnFocus, tfuiCallback onFocus,
											tfuiCallback onFocusLost,
											bool bFromTemplate,
											const char* tip, const char* text,
											int x, int y, int width, int font, int align, 
											const float* fgColor, const float* fgFocusColor, const float* fgPushedColor)
{
	if (!m_priv->xmlDescParmHdle && !openXMLDescriptor())
		return -1;
	
	if (m_priv->mapControlIds.find(pszName) == m_priv->mapControlIds.end())
	{
		const int nCtrlId =
			::GfuiMenuCreateTextButtonControl(m_priv->menuHdle, m_priv->xmlDescParmHdle, pszName,
											  userDataOnPush, onPush,
											  userDataOnFocus, onFocus, onFocusLost,
											  bFromTemplate,
											  text, tip, x, y, width, font, align,
											  fgColor, fgFocusColor, fgPushedColor);
		if (nCtrlId >= 0)
			m_priv->mapControlIds[pszName] = nCtrlId;

		return nCtrlId;
	}

	GfLogError("Failed to create text button control '%s' : duplicate name\n", pszName);
	return -1;
}

int GfuiMenuScreen::createImageButtonControl(const char* pszName,
											 void* userDataOnPush, tfuiCallback onPush,
											 void* userDataOnFocus, tfuiCallback onFocus, tfuiCallback onFocusLost,
											 bool bFromTemplate,
											 const char* tip,
											 int x, int y, int width, int height, int align)
{
	if (!m_priv->xmlDescParmHdle && !openXMLDescriptor())
		return -1;
	
	if (m_priv->mapControlIds.find(pszName) == m_priv->mapControlIds.end())
	{
		const int nCtrlId =
			::GfuiMenuCreateImageButtonControl(m_priv->menuHdle, m_priv->xmlDescParmHdle, pszName,
											   userDataOnPush, onPush,
											   userDataOnFocus, onFocus, onFocusLost,
											   bFromTemplate,
											   tip, x, y, width, height, align);
		if (nCtrlId >= 0)
			m_priv->mapControlIds[pszName] = nCtrlId;

		return nCtrlId;
	}

	GfLogError("Failed to create image button control '%s' : duplicate name\n", pszName);
	return -1;
}

int GfuiMenuScreen::createStaticImageControl(const char* pszName)
{
	if (!m_priv->xmlDescParmHdle && !openXMLDescriptor())
		return -1;

	if (m_priv->mapControlIds.find(pszName) == m_priv->mapControlIds.end())
	{
		const int nCtrlId =
			::GfuiMenuCreateStaticImageControl(m_priv->menuHdle, m_priv->xmlDescParmHdle, pszName);
		if (nCtrlId >= 0)
			m_priv->mapControlIds[pszName] = nCtrlId;

		return nCtrlId;
	}

	GfLogError("Failed to create static image control '%s' : duplicate name\n", pszName);
	return -1;
}

int GfuiMenuScreen::createLabelControl(const char* pszName, bool bFromTemplate,
									   const char* pszText, int nX, int nY,
									   int nFontId, int nAlignId, int nMaxLen, 
									   const float* aFgColor, const float* aFgFocusColor)
{
	if (!m_priv->xmlDescParmHdle && !openXMLDescriptor())
		return -1;
	
	if (m_priv->mapControlIds.find(pszName) == m_priv->mapControlIds.end())
	{
		const int nCtrlId =
			::GfuiMenuCreateLabelControl(m_priv->menuHdle, m_priv->xmlDescParmHdle, pszName,
										 bFromTemplate, pszText, nX, nY,
										 nFontId, nAlignId, nMaxLen, aFgColor, aFgFocusColor);
		if (nCtrlId >= 0)
			m_priv->mapControlIds[pszName] = nCtrlId;

		return nCtrlId;
	}

	GfLogError("Failed to create label control '%s' : duplicate name\n", pszName);
	return -1;
}

int GfuiMenuScreen::createEditControl(const char* pszName,
									  void* userDataOnFocus, tfuiCallback onFocus,
									  tfuiCallback onFocusLost)
{
	if (!m_priv->xmlDescParmHdle && !openXMLDescriptor())
		return -1;
	
	if (m_priv->mapControlIds.find(pszName) == m_priv->mapControlIds.end())
	{
		const int nCtrlId =
			::GfuiMenuCreateEditControl(m_priv->menuHdle, m_priv->xmlDescParmHdle, pszName,
							  userDataOnFocus, onFocus, onFocusLost);
		if (nCtrlId >= 0)
			m_priv->mapControlIds[pszName] = nCtrlId;

		return nCtrlId;
	}

	GfLogError("Failed to create edit control '%s' : duplicate name\n", pszName);
	return -1;
}

int GfuiMenuScreen::createScrollListControl(const char* pszName,
											void* userData, tfuiCallback onSelect)
{
	if (!m_priv->xmlDescParmHdle && !openXMLDescriptor())
		return -1;
	
	if (m_priv->mapControlIds.find(pszName) == m_priv->mapControlIds.end())
	{
		const int nCtrlId =
			::GfuiMenuCreateScrollListControl(m_priv->menuHdle, m_priv->xmlDescParmHdle, pszName,
									userData, onSelect);
		if (nCtrlId >= 0)
			m_priv->mapControlIds[pszName] = nCtrlId;

		return nCtrlId;
	}

	GfLogError("Failed to create scroll-list control '%s' : duplicate name\n", pszName);
	return -1;
}

int GfuiMenuScreen::createComboboxControl(const char* pszName,
										  void* userData, tfuiComboboxCallback onChange)
{
	if (!m_priv->xmlDescParmHdle && !openXMLDescriptor())
		return -1;
	
	if (m_priv->mapControlIds.find(pszName) == m_priv->mapControlIds.end())
	{
		const int nCtrlId =
			::GfuiMenuCreateComboboxControl(m_priv->menuHdle, m_priv->xmlDescParmHdle, pszName,
									userData, onChange);
		if (nCtrlId >= 0)
			m_priv->mapControlIds[pszName] = nCtrlId;

		return nCtrlId;
	}

	GfLogError("Failed to create combo-box control '%s' : duplicate name\n", pszName);
	return -1;
}

int GfuiMenuScreen::createCheckboxControl(const char* pszName,
										  void* userData, tfuiCheckboxCallback onChange)
{
	if (!m_priv->xmlDescParmHdle && !openXMLDescriptor())
		return -1;
	
	if (m_priv->mapControlIds.find(pszName) == m_priv->mapControlIds.end())
	{
		const int nCtrlId = 
			::GfuiMenuCreateCheckboxControl(m_priv->menuHdle, m_priv->xmlDescParmHdle, pszName,
									userData, onChange);
		if (nCtrlId >= 0)
			m_priv->mapControlIds[pszName] = nCtrlId;

		return nCtrlId;
	}

	GfLogError("Failed to create check-box control '%s' : duplicate name\n", pszName);
	return -1;
}

int GfuiMenuScreen::createProgressbarControl(const char* pszName)
{
	if (!m_priv->xmlDescParmHdle && !openXMLDescriptor())
		return -1;
	
	if (m_priv->mapControlIds.find(pszName) == m_priv->mapControlIds.end())
	{
		const int nCtrlId =
			::GfuiMenuCreateProgressbarControl(m_priv->menuHdle, m_priv->xmlDescParmHdle, pszName);
		if (nCtrlId >= 0)
			m_priv->mapControlIds[pszName] = nCtrlId;

		return nCtrlId;
	}

	GfLogError("Failed to create progress-bar control '%s' : duplicate name\n", pszName);
	return -1;
}

void GfuiMenuScreen::addDefaultShortcuts()
{
	if (!m_priv->xmlDescParmHdle && !openXMLDescriptor())
		return;
	
	GfuiMenuDefaultKeysAdd(m_priv->menuHdle);
}
		
void GfuiMenuScreen::addShortcut(int key, const char* descr, void* userData,
								 tfuiCallback onKeyPressed, tfuiCallback onKeyReleased)
{
	if (!m_priv->xmlDescParmHdle && !openXMLDescriptor())
		return;
	
	GfuiAddKey(m_priv->menuHdle, key, descr, userData, onKeyPressed, onKeyReleased);
}


int GfuiMenuScreen::getDynamicControlId(const char* pszName) const
{
	std::map<std::string, int>::const_iterator iterCtrlId = m_priv->mapControlIds.find(pszName);

	return iterCtrlId == m_priv->mapControlIds.end() ? -1 : (*iterCtrlId).second;
}

void GfuiMenuScreen::runMenu()
{
	if (!m_priv->xmlDescParmHdle && !openXMLDescriptor())
		return;
	
	GfuiScreenActivate(m_priv->menuHdle);
}
