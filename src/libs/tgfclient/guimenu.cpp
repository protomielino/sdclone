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

#ifdef WIN32
#include <windows.h>
#endif

#include "tgfclient.h"
#include "gui.h"
#include "guimenu.h"

void
gfMenuInit(void)
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
GfuiMenuDefaultKeysAdd(void *scr)
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
void *
GfuiMenuScreenCreate(const char *title)
{
    void        *scr;

    scr = GfuiScreenCreate();
    GfuiTitleCreate(scr, title, strlen(title));

    GfuiMenuDefaultKeysAdd(scr);

    return scr;
}

static void
dispInfo(void *cbinfo)
{
    GfuiVisibilitySet(((tMnuCallbackInfo*)cbinfo)->screen, ((tMnuCallbackInfo*)cbinfo)->labelId, 1);
}

static void
remInfo(void *cbinfo)
{
    GfuiVisibilitySet(((tMnuCallbackInfo*)cbinfo)->screen, ((tMnuCallbackInfo*)cbinfo)->labelId, 0);
}


/** Add a button to a menu screen.
    @ingroup    gui
    @param      scr             Screen (menu) handle
    @param      text            Text of the button
    @param      tip             Text of the tip displayed when the button is focused
    @param      userData        Parameter of the Push function
    @param      style           Alignment horizontally/vertically
    @param      onpush          Callback when the button is pushed
    @return     Button 3
 */
int
GfuiMenuButtonCreate(void *scr, const char *text, const char *tip,
        void *userData, const int style, tfuiCallback onpush)
{
    int nbItems = ((tGfuiScreen*)scr)->nbItems++;
    if (nbItems > 22) {
        GfTrace("Too many items in that menu !!!\n");
        return -1;
    }
    
    int xpos = 270;
    int ypos = 380 - 30 * (nbItems % 11);

    tMnuCallbackInfo *cbinfo = (tMnuCallbackInfo*)calloc(1, sizeof(tMnuCallbackInfo));
    cbinfo->screen = scr;
    cbinfo->labelId = GfuiTipCreate(scr, tip, strlen(tip));

    GfuiVisibilitySet(scr, cbinfo->labelId, 0);
    
    int bId = GfuiButtonCreate(scr,
                           text,
                           GFUI_FONT_LARGE,
                           xpos, ypos, GFUI_BTNSZ, style, 0,
                           userData, onpush,
                           (void*)cbinfo, dispInfo,
                           remInfo);

    return bId;
}//GfuiMenuButtonCreate


/** Add a button to a menu screen.
    @ingroup    gui
    @param      scr             Screen (menu) handle
    @param      text            Text of the button
    @param      tip             Text of the tip displayed when the button is focused
    @param      userData        Parameter of the Push function
    @param      onpush          Callback when the button is pushed
    @return     Button Id
 */
int
GfuiMenuButtonCreateEx(void *scr, const char *text, const char *tip, void *userData, tfuiCallback onpush,int xpos,int ypos,int fontSize,int align)
{
    tMnuCallbackInfo    *cbinfo;
    //int                       xpos, ypos;
    //int                       nbItems = ((tGfuiScreen*)scr)->nbItems++;
    int                 bId;

/*
    if (nbItems < 11) {
        ypos = ypos - 30 * nbItems;
    } else {
        if (nbItems > 22) {
            GfTrace("Too many items in that menu !!!\n");
            return -1;
        }
        //xpos = 380;
        ypos = ypos - 30 * (nbItems - 11);
    }
*/
    cbinfo = (tMnuCallbackInfo*)calloc(1, sizeof(tMnuCallbackInfo));
    cbinfo->screen = scr;
    cbinfo->labelId = GfuiTipCreate(scr, tip, strlen(tip));

    GfuiVisibilitySet(scr, cbinfo->labelId, 0);
    
    bId = GfuiButtonCreate(scr,
                           text,
                           fontSize,
                           xpos, ypos, GFUI_BTNSZ, align, 0,
                           userData, onpush,
                           (void*)cbinfo, dispInfo,
                           remInfo);

    return bId;
}

/** Add the "Back" or "Quit" button at the bottom of the menu screen.
    @ingroup    gui
    @param      scr     Screen or Menu handle
    @param      text    Text of the button
    @param      tip     Text to display when the button is focused
    @param      userData        Parameter of the Push function
    @param      onpush          Callback when the button is pushed
    @return     Button Id
 */
int
GfuiMenuBackQuitButtonCreate(void *scr, const char *text, const char *tip, void *userData, tfuiCallback onpush)
{
    tMnuCallbackInfo    *cbinfo;
    int                 xpos, ypos;
    int                 bId;
    
    xpos = 320;
    ypos = 40;

    cbinfo = (tMnuCallbackInfo*)calloc(1, sizeof(tMnuCallbackInfo));
    cbinfo->screen = scr;
    cbinfo->labelId = GfuiTipCreate(scr, tip, strlen(tip));

    GfuiVisibilitySet(scr, cbinfo->labelId, 0);
    
    bId = GfuiButtonCreate(scr,
                        text,
                        GFUI_FONT_LARGE,
                        xpos, ypos, GFUI_BTNSZ, GFUI_ALIGN_HC_VB, 0,
                        userData, onpush,
                        (void*)cbinfo, dispInfo,
                        remInfo);

    GfuiAddKey(scr, GFUIK_ESCAPE, tip, userData, onpush, NULL);

    return bId;
}


/** Add the "Back" or "Quit" button at the bottom of the menu screen.
    @ingroup    gui
    @param      scr     Screen or Menu handle
    @param      text    Text of the button
    @param      tip     Text to display when the button is focused
    @param      userData        Parameter of the Push function
    @param      onpush          Callback when the button is pushed
    @return     Button Id
 */
int
GfuiMenuBackQuitButtonCreateEx(void *scr, const char *text, const char *tip, void *userData, tfuiCallback onpush,int xpos,int ypos,int fontSize,int align)
{
    tMnuCallbackInfo    *cbinfo;
    int                 bId;
    
    cbinfo = (tMnuCallbackInfo*)calloc(1, sizeof(tMnuCallbackInfo));
    cbinfo->screen = scr;
    cbinfo->labelId = GfuiTipCreate(scr, tip, strlen(tip));

    GfuiVisibilitySet(scr, cbinfo->labelId, 0);
    
    bId = GfuiButtonCreate(scr,
                        text,
                        fontSize,
                        xpos, ypos, GFUI_BTNSZ, align, 0,
                        userData, onpush,
                        (void*)cbinfo, dispInfo,
                        remInfo);

    GfuiAddKey(scr, GFUIK_ESCAPE, tip, userData, onpush, NULL);

    return bId;
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

int 
GetFontSize(const char* pszTextsize)
{
    const TMapFontSize::const_iterator itFontSize = MapFontSize.find(pszTextsize);
    
    if (itFontSize != MapFontSize.end())
        return (*itFontSize).second;
    else
        return GFUI_FONT_MEDIUM; // Default size.
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

int 
GetAlignment(const char* pszAlH, const char* pszAlV)
{
    std::string strAlign(pszAlH);
    if (strlen(pszAlH) == 0)
        strAlign += "left"; // Default horizontal alignment
    strAlign += '.';
    strAlign += pszAlV;
    if (strlen(pszAlV) == 0)
        strAlign += "bottom"; // Default horizontal alignment
    
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

int 
GetHAlignment(const char* pszAlignH)
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

int 
GetScrollBarPosition(const char* pszPos)
{
    const TMapScrollBarPos::const_iterator itScrollBarPos = MapScrollBarPos.find(pszPos);
    
    if (itScrollBarPos != MapScrollBarPos.end())
        return (*itScrollBarPos).second;
    else
        return GFUI_SB_NONE; // Default horizontal alignement = left.
}

Color GetColor(unsigned int color)
{
        Color c;
        c.alpha = 1.0;
        c.red = ((color&0x00ff0000)>>16)/255.0;
        c.green = ((color&0xff00)>>8)/255.0;
        c.blue = ((color&0xff))/255.0;

        return c;
}

bool GetColorFromXML(void *param,const char *pControlName,const char *pField,Color &color)
{
        const char* pszValue = GfParmGetStr(param,pControlName,pField,"");
        if (strlen(pszValue) == 0)
                return false;

        color = GetColor((unsigned int)strtol(pszValue, NULL, 0));

        return true;
}


bool 
ReadBoolean(void *param,const char *pControlName,const char *pField, bool bDefault)
{
        const char* pszValue = GfParmGetStr(param, pControlName, pField, "");
        if (strlen(pszValue) == 0)
                return bDefault;
        else if (!strcmp(pszValue, "yes"))
                return true;
        else if (!strcmp(pszValue, "no"))
                return false;

        return bDefault;
}

int 
CreateStaticImage(void *menuHandle,void *param,const char *pControlName)
{
        const char* pszImage = GfParmGetStr(param, pControlName, "image", "");

        const int x = (int)GfParmGetNum(param,pControlName,"x",NULL,0.0);
        const int y = (int)GfParmGetNum(param,pControlName,"y",NULL,0.0);
        const int w = (int)GfParmGetNum(param,pControlName,"width",NULL,0.0);
        const int h = (int)GfParmGetNum(param,pControlName,"height",NULL,0.0);

        const char* pszAlignH = GfParmGetStr(param, pControlName, "alignH", "");
        const char* pszAlignV = GfParmGetStr(param, pControlName, "alignV", "");
        const int alignment = GetAlignment(pszAlignH,pszAlignV);

        int id = GfuiStaticImageCreateEx(menuHandle,x,y,w,h,pszImage,alignment);

        char buffer[256];
        for (int i=2; i<= MAX_STATIC_IMAGES;i++)
        {
                sprintf(buffer, "image%i", i);
                const char* pszIndexedImage = GfParmGetStr(param, pControlName, buffer, 0);
                if (pszIndexedImage && strlen(pszIndexedImage) > 0)
                        GfuiStaticImageSet(menuHandle, id, pszIndexedImage, i-1);
        }

        return id;
}

int 
CreateBackgroundImage(void *menuHandle,void *param,const char *pControlName)
{
        const char* pszImage = GfParmGetStr(param, pControlName, "image", "");
        GfuiScreenAddBgImg(menuHandle, pszImage);
        return 1;
}

int
CreateStaticImageControl(void *menuHandle,void *param,const char *pControlName)
{
        std::string strControlName("dynamiccontrols/");
        strControlName += pControlName;
        
        return CreateStaticImage(menuHandle,param,strControlName.c_str());
}

int 
CreateLabel(void *menuHandle,void *param,const char *pControlName)
{
        if (strcmp(GfParmGetStr(param, pControlName, "type", ""), "label"))
        {
                GfError("Error: Control '%s' is not a 'label'\n", pControlName);
                return -1;
        }
        
        const char* pszText = GfParmGetStr(param, pControlName, "text", "");
        const int x = (int)GfParmGetNum(param,pControlName,"x",NULL,0.0);
        const int y = (int)GfParmGetNum(param,pControlName,"y",NULL,0.0);
        const char* pszTextsize = GfParmGetStr(param, pControlName, "textsize", "");
        const int textsize = GetFontSize(pszTextsize);
        const char* pszAlignH = GfParmGetStr(param, pControlName, "alignH", "");
        const char* pszAlignV = GfParmGetStr(param, pControlName, "alignV", "");
        const int alignment = GetAlignment(pszAlignH,pszAlignV);
        const int maxlen = (int)GfParmGetNum(param,pControlName,"maxlen",NULL,32.0);
        
        int labelId = GfuiLabelCreate(menuHandle, pszText, textsize, x, y, alignment, maxlen);

        Color c;
        const bool bColor = GetColorFromXML(param,pControlName,"color",c);

        if (bColor)
                GfuiLabelSetColor(menuHandle, labelId, c.GetPtr());

    return labelId;
}


int 
CreateLabelControl(void *menuHandle,void *param,const char *pControlName)
{
        std::string strControlName("dynamiccontrols/");
        strControlName += pControlName;

        return CreateLabel(menuHandle,param,strControlName.c_str());
}

int 
CreateTextButtonControl(void *menuHandle,void *param,const char *pControlName,void *userData, tfuiCallback onpush, void *userDataOnFocus, tfuiCallback onFocus, tfuiCallback onFocusLost)
{
        const char* pszTip = GfParmGetStr(param, pControlName, "tip", "");

        if (strlen(pszTip) > 0)
        {
                tMnuCallbackInfo * cbinfo = (tMnuCallbackInfo*)calloc(1, sizeof(tMnuCallbackInfo));
                cbinfo->screen = menuHandle;
                cbinfo->labelId = GfuiTipCreate(menuHandle, pszTip, strlen(pszTip));
                GfuiVisibilitySet(menuHandle, cbinfo->labelId, 0);

                userDataOnFocus = (void*)cbinfo;
                onFocus = dispInfo;
                onFocusLost = remInfo;
        }

        const char* pszText = GfParmGetStr(param, pControlName, "text", "");
        const int x = (int)GfParmGetNum(param,pControlName,"x",NULL,0.0);
        const int y = (int)GfParmGetNum(param,pControlName,"y",NULL,0.0);
        const char* pszTextsize = GfParmGetStr(param, pControlName, "textsize", "");
        const int textsize = GetFontSize(pszTextsize);
        const char* pszAlignH = GfParmGetStr(param, pControlName, "alignH", "");
        const int alignH = GetHAlignment(pszAlignH);
        
        int width = (int)GfParmGetNum(param,pControlName,"width",NULL,0.0);
        if (width == 0)
            width = GFUI_BTNSZ;

        int id = GfuiButtonCreate(menuHandle,
                                  pszText,
                                  textsize,
                                  x, y, width, alignH, GFUI_MOUSE_UP,
                                  userData, onpush,
                                  userDataOnFocus, onFocus,
                                  onFocusLost);

        const bool bShowbox = ReadBoolean(param,pControlName,"showbox", true);

        GfuiButtonShowBox(menuHandle,id,bShowbox);

        const char* pszDisabledImage = GfParmGetStr(param, pControlName, "disabledimage", "");
        const char* pszEnabledImage = GfParmGetStr(param, pControlName, "enabledimage", "");
        const char* pszFocusedImage = GfParmGetStr(param, pControlName, "focusedimage", "");
        const char* pszPushedImage = GfParmGetStr(param, pControlName, "pushedimage", "");

        const int imgX = (int)GfParmGetNum(param,pControlName,"imagex",NULL,0.0);
        const int imgY = (int)GfParmGetNum(param,pControlName,"imagey",NULL,0.0);
        const int imgWidth = (int)GfParmGetNum(param,pControlName,"imagewidth",NULL,20.0);
        const int imgHeight = (int)GfParmGetNum(param,pControlName,"imageheight",NULL,20.0);

        GfuiButtonSetImage(menuHandle,id,imgX,imgY,imgWidth,imgHeight,
                           pszDisabledImage,pszEnabledImage,
                           pszFocusedImage,pszPushedImage);

        Color c,fc,pc;
        const bool bColor = GetColorFromXML(param,pControlName,"color",c);
        const bool bFocusColor = GetColorFromXML(param,pControlName,"focuscolor",fc);
        const bool bPushedColor = GetColorFromXML(param,pControlName,"pushedcolor",pc);

        if (bColor)
                GfuiButtonSetColor(menuHandle,id,c);

        if (bFocusColor)
                GfuiButtonSetFocusColor(menuHandle,id,fc);
        
        if (bPushedColor)
                GfuiButtonSetPushedColor(menuHandle,id,pc);
        
        return id;
}

int 
CreateImageButtonControl(void *menuHandle,void *param,const char *pControlName,void *userData, tfuiCallback onpush, void *userDataOnFocus, tfuiCallback onFocus, tfuiCallback onFocusLost)
{
        const char* pszTip = GfParmGetStr(param, pControlName, "tip", "");

        if (strlen(pszTip) > 0)
        {
                tMnuCallbackInfo * cbinfo = (tMnuCallbackInfo*)calloc(1, sizeof(tMnuCallbackInfo));
                cbinfo->screen = menuHandle;
                cbinfo->labelId = GfuiTipCreate(menuHandle, pszTip, strlen(pszTip));
                GfuiVisibilitySet(menuHandle, cbinfo->labelId, 0);

                userDataOnFocus = (void*)cbinfo;
                onFocus = dispInfo;
                onFocusLost = remInfo;
        }

        const char* pszDisabledImage = GfParmGetStr(param, pControlName, "disabledimage", "");
        const char* pszEnabledImage = GfParmGetStr(param, pControlName, "enabledimage", "");
        const char* pszFocusedImage = GfParmGetStr(param, pControlName, "focusedimage", "");
        const char* pszPushedImage = GfParmGetStr(param, pControlName, "pushedimage", "");

        const int x = (int)GfParmGetNum(param,pControlName,"x",NULL,0.0);
        const int y = (int)GfParmGetNum(param,pControlName,"y",NULL,0.0);
        const int w = (int)GfParmGetNum(param,pControlName,"width",NULL,0.0);
        const int h = (int)GfParmGetNum(param,pControlName,"height",NULL,0.0);

        const char* pszAlignH = GfParmGetStr(param, pControlName, "alignH", "");
        const char* pszAlignV = GfParmGetStr(param, pControlName, "alignV", "");
        const int alignment = GetAlignment(pszAlignH,pszAlignV);

        int id = -1;
        if (w == 0 && h==0)
        {
                id = GfuiGrButtonCreate(menuHandle,
                                        pszDisabledImage,pszEnabledImage,pszFocusedImage,pszPushedImage,
                                        x,y,alignment,GFUI_MOUSE_UP,
                                        userData,onpush,
                                        userDataOnFocus,onFocus,onFocusLost);
        }
        else
        {
                id = GfuiGrButtonCreateEx(menuHandle,
                                          pszDisabledImage,pszEnabledImage,pszFocusedImage,pszPushedImage,
                                          x,y,w,h,alignment,GFUI_MOUSE_UP,
                                          userData,onpush,
                                          userDataOnFocus,onFocus,onFocusLost);
        }

        return id;
}

int 
CreateButtonControl(void *menuHandle,void *param,const char *pControlName,void *userData, tfuiCallback onpush)
{
        return CreateButtonControlEx(menuHandle,param,pControlName,userData,onpush,NULL,NULL,NULL);
}

int 
CreateButtonControlEx(void *menuHandle,void *param,const char *pControlName,void *userData, tfuiCallback onpush, void *userDataOnFocus, tfuiCallback onFocus, tfuiCallback onFocusLost)
{
        std::string strControlName("dynamiccontrols/");
        strControlName += pControlName;

        const char* pszType = GfParmGetStr(param, strControlName.c_str(), "type", "");
        if (!strcmp(pszType, "textbutton"))
                return CreateTextButtonControl(menuHandle,param,strControlName.c_str(),userData,onpush,NULL,NULL,NULL);
        else if(!strcmp(pszType, "imagebutton"))
                return CreateImageButtonControl(menuHandle,param,strControlName.c_str(),userData,onpush,NULL,NULL,NULL);
        else
            GfError("Error: Unknown button type '%s' for Control %s\n", pszType,pControlName);

        return -1;
}

int 
CreateEditControl(void *menuHandle,void *param,const char *pControlName,void *userDataOnFocus, tfuiCallback onFocus, tfuiCallback onFocusLost)
{
        std::string strControlName("dynamiccontrols/");
        strControlName += pControlName;

        const char* pszType = GfParmGetStr(param, strControlName.c_str(), "type", "");
        if (strcmp(pszType, "editbox"))
        {
                GfError("Error: Control '%s' is not an 'editbox' \n", pControlName);
                return -1;
        }

// TODO : Add real support for tips (the onFocus/onFocusLost system is already used
//        for user input management)
//         const char* pszTip = GfParmGetStr(param, pControlName, "tip", "");
//         if (strlen(pszTip) > 0)
//         {
//                 tMnuCallbackInfo * cbinfo = (tMnuCallbackInfo*)calloc(1, sizeof(tMnuCallbackInfo));
//                 cbinfo->screen = menuHandle;
//                 cbinfo->labelId = GfuiTipCreate(menuHandle, pszTip, strlen(pszTip));
//                 GfuiVisibilitySet(menuHandle, cbinfo->labelId, 0);

//                 userDataOnFocus = (void*)cbinfo;
//                 onFocus = dispInfo;
//                 onFocusLost = remInfo;
//         }

        const char* pszText = GfParmGetStr(param, strControlName.c_str(), "text", "");
        const int x = (int)GfParmGetNum(param,strControlName.c_str(),"x",NULL,0.0);
        const int y = (int)GfParmGetNum(param,strControlName.c_str(),"y",NULL,0.0);
        const char* pszTextsize = GfParmGetStr(param, strControlName.c_str(), "textsize", "");
        const int textsize = GetFontSize(pszTextsize);
        
        int width = (int)GfParmGetNum(param,strControlName.c_str(),"width",NULL,0.0);
        if (width == 0)
            width =  GFUI_BTNSZ;

        const int maxlen = (int)GfParmGetNum(param,strControlName.c_str(),"maxlen",NULL,0.0);

        int id = GfuiEditboxCreate(menuHandle,pszText, textsize,x,y,width,maxlen,
                                   userDataOnFocus,onFocus,onFocusLost);

        Color c,fc;
        const bool bColor = GetColorFromXML(param,pControlName,"color",c);
        const bool bFocusColor = GetColorFromXML(param,pControlName,"focuscolor",fc);

        if (bColor)
                GfuiEditboxSetColor(menuHandle,id,c);
        
        if (bFocusColor)
                GfuiEditboxSetFocusColor(menuHandle,id,fc);             

        return id;
}

int 
CreateComboboxControl(void *menuHandle,void *param,const char *pControlName,void *userData,tfuiComboboxCallback onChange)
{
	std::string strControlName("dynamiccontrols/");
	strControlName += pControlName;

	const std::string strType = GfParmGetStr(param, strControlName.c_str(), "type", "");
	if (strType != "combobox")
		return -1;

	int id = -1;
	
	std::string strText,strTip;
	int textsize;
	int x,y,width;

	x = (int)GfParmGetNum(param,strControlName.c_str(),"x",NULL,0.0);
	y = (int)GfParmGetNum(param,strControlName.c_str(),"y",NULL,0.0);

	std::string strTextsize = GfParmGetStr(param, strControlName.c_str(), "textsize", "");
	textsize = GetFontSize(strTextsize.c_str());

	const char * pszAlignH = GfParmGetStr(param, strControlName.c_str(), "alignH", "");
	const char * pszAlignV = GfParmGetStr(param, strControlName.c_str(), "alignV", "");
	int align = GetAlignment(pszAlignH,pszAlignV);

	
	width = (int)GfParmGetNum(param,strControlName.c_str(),"width",NULL,0.0);
	if (width == 0)
	    width = 200;

    const char* pszText = GfParmGetStr(param, strControlName.c_str(), "text", "");

	id = GfuiComboboxCreate(menuHandle,textsize,x,y,width,align,0,pszText,userData,onChange);

	Color c;
	bool bColor = GetColorFromXML(param,pControlName,"color",c);
	if(bColor)
		GfuiComboboxSetTextColor(menuHandle,id,c);

	return id;
}

int 
CreateScrollListControl(void *menuHandle,void *param,const char *pControlName,void *userData, tfuiCallback onSelect)
{
        std::string strControlName("dynamiccontrols/");
        strControlName += pControlName;

        const char* pszType = GfParmGetStr(param, strControlName.c_str(), "type", "");
        if (strcmp(pszType, "scrolllist"))
        {
                GfError("Error: Control '%s' is not a 'scrolllist' \n", pControlName);
                return -1;
        }

        const int x = (int)GfParmGetNum(param,strControlName.c_str(),"x",NULL,0.0);
        const int y = (int)GfParmGetNum(param,strControlName.c_str(),"y",NULL,0.0);
        const int w = (int)GfParmGetNum(param,strControlName.c_str(),"width",NULL,0.0);
        const int h = (int)GfParmGetNum(param,strControlName.c_str(),"height",NULL,0.0);
        
        const char* pszTextsize = GfParmGetStr(param, strControlName.c_str(), "textsize", "");
        const int textsize = GetFontSize(pszTextsize);

        const char* pszAlignH = GfParmGetStr(param, pControlName, "alignH", "");
        const char* pszAlignV = GfParmGetStr(param, pControlName, "alignV", "");
        const int alignment = GetAlignment(pszAlignH,pszAlignV);

        const char* pszScrollBarPos = GfParmGetStr(param,strControlName.c_str(),"scrollbarposition","none");
        int scrollbarpos = GetScrollBarPosition(pszScrollBarPos);

        int id = GfuiScrollListCreate(menuHandle, textsize,x,y,alignment,w,h,scrollbarpos,userData,onSelect);

        Color c,sc;
        bool bColor = GetColorFromXML(param,pControlName,"color",c);
        bool bSelectColor = GetColorFromXML(param,pControlName,"selectcolor",sc);
        
        if (bColor)
                GfuiScrollListSetColor(menuHandle,id,c);

        if (bSelectColor)
                GfuiScrollListSetSelectColor(menuHandle,id,sc);

        return id;
}

bool 
CreateStaticControls(void *param,void *menuHandle)
{
	if (param==NULL)
	{
		GfError("ERROR: XML menu is not loaded");
		return false;
	}

    char buf[32];

    for (int i=1; i <= GfParmGetEltNb(param, "staticcontrols"); i++)
    {
            sprintf(buf, "staticcontrols/%i", i);
            const char* pszType = GfParmGetStr(param, buf, "type", "");
    
            if (!strcmp(pszType, "label"))
            {
                    CreateLabel(menuHandle,param,buf);
            }
            else if (!strcmp(pszType, "staticimage"))
            {
                    CreateStaticImage(menuHandle,param,buf);
            }
            else if (!strcmp(pszType, "backgroundimage"))
            {
                    CreateBackgroundImage(menuHandle,param,buf);
            }
            else
            {
                    GfError("Errot: Unknown static control type '%s'\n", pszType);
            }
    }

    return true;
}



void *
LoadMenuXML(const char *pszMenuPath)
{
        std::string strPath("data/menu/");
        strPath += pszMenuPath;
        
        char buf[1024];
        sprintf(buf, "%s%s", GetDataDir(), strPath.c_str());

        return GfParmReadFile(buf, GFPARM_RMODE_STD);
}

int 
CreateCheckboxControl(void *menuHandle,void *param,const char *pControlName,void* userData,tfuiCheckboxCallback onChange)
{
	std::string strControlName("dynamiccontrols/");
	strControlName += pControlName;

	const std::string strType = GfParmGetStr(param, strControlName.c_str(), "type", "");
	if (strType != "checkbox")
		return -1;

	int id = -1;
	
	std::string strText,strTip;
	int textsize;
	int x,y,imagewidth,imageheight;

	x = (int)GfParmGetNum(param,strControlName.c_str(),"x",NULL,0.0);
	y = (int)GfParmGetNum(param,strControlName.c_str(),"y",NULL,0.0);

	std::string strTextsize = GfParmGetStr(param, strControlName.c_str(), "textsize", "");
	textsize = GetFontSize(strTextsize.c_str());

    const char* pszText = GfParmGetStr(param, strControlName.c_str(), "text", "");

	const char * pszAlignH = GfParmGetStr(param, strControlName.c_str(), "alignH", "");
	const char * pszAlignV = GfParmGetStr(param, strControlName.c_str(), "alignV", "");
	int align = GetAlignment(pszAlignH,pszAlignV);

	
	imagewidth = (int)GfParmGetNum(param,strControlName.c_str(),"imagewidth",NULL,0.0);
	if (imagewidth == 0)
	    imagewidth = 30;

	imageheight = (int)GfParmGetNum(param,strControlName.c_str(),"imageheight",NULL,0.0);
	if (imageheight == 0)
	    imageheight = 30;

    const bool bChecked = ReadBoolean(param,strControlName.c_str(),"checked", true);


	id = GfuiCheckboxCreate(menuHandle,textsize,x,y,imagewidth,imageheight,align,0,pszText,bChecked,userData,onChange);

	Color c;
	bool bColor = GetColorFromXML(param,pControlName,"color",c);
	if(bColor)
		GfuiComboboxSetTextColor(menuHandle,id,c);

	return id;
}


int 
CreateProgressbarControl(void *menuHandle,void *param,const char *pControlName)
{
		std::string strControlName("dynamiccontrols/");
		strControlName += pControlName;

		const std::string strType = GfParmGetStr(param, strControlName.c_str(), "type", "");
		if (strType != "progressbar")
			return -1;

		const char* pszProgressbackgroundImage = GfParmGetStr(param, strControlName.c_str(), "image", "data/img/progressbackground.png");
		const char* pszProgressbarImage = GfParmGetStr(param, pControlName, "image", "data/img/progressbar.png");

        const int x = (int)GfParmGetNum(param,strControlName.c_str(),"x",NULL,0.0);
        const int y = (int)GfParmGetNum(param,strControlName.c_str(),"y",NULL,0.0);
        const int w = (int)GfParmGetNum(param,strControlName.c_str(),"width",NULL,0.0);
        const int h = (int)GfParmGetNum(param,strControlName.c_str(),"height",NULL,0.0);

        const char* pszAlignH = GfParmGetStr(param, strControlName.c_str(), "alignH", "");
        const char* pszAlignV = GfParmGetStr(param, strControlName.c_str(), "alignV", "");
		const float min = GfParmGetNum(param, strControlName.c_str(), "min",NULL,0.0);
		const float max = GfParmGetNum(param, strControlName.c_str(), "max",NULL,100.0);
		const float value = GfParmGetNum(param, strControlName.c_str(), "value",NULL,100.0);
        const int alignment = GetAlignment(pszAlignH,pszAlignV);

		int id = GfuiProgressbarCreate(menuHandle,x,y,w,h,pszProgressbackgroundImage,pszProgressbarImage,alignment,min,max,value);

		return id;
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
	CloseXMLDescriptor();
	if (m_priv->menuHdle)
		GfuiScreenRelease(m_priv->menuHdle);
	delete m_priv;
}

void GfuiMenuScreen::CreateMenu()
{
	m_priv->menuHdle = GfuiScreenCreate();
}

void GfuiMenuScreen::CreateMenuEx(float *bgColor, 
								  void *userDataOnActivate, tfuiCallback onActivate, 
								  void *userDataOnDeactivate, tfuiCallback onDeactivate, 
								  int mouseAllowed)
{
	m_priv->menuHdle = GfuiScreenCreateEx(bgColor, userDataOnActivate, onActivate, 
										  userDataOnDeactivate, onDeactivate, mouseAllowed);
}

void GfuiMenuScreen::SetMenuHandle(void* hdle)
{
	m_priv->menuHdle = hdle;
}

void* GfuiMenuScreen::GetMenuHandle() const
{
	return m_priv->menuHdle;
}

void GfuiMenuScreen::SetPreviousMenuHandle(void* hdle)
{
	m_priv->prevMenuHdle = hdle;
}

void* GfuiMenuScreen::GetPreviousMenuHandle() const
{
	return m_priv->prevMenuHdle;
}

bool GfuiMenuScreen::OpenXMLDescriptor()
{
    m_priv->xmlDescParmHdle = LoadMenuXML(m_priv->strXMLDescFileName.c_str());
	return m_priv->xmlDescParmHdle != 0;
}

bool GfuiMenuScreen::CloseXMLDescriptor()
{
	if (m_priv->xmlDescParmHdle)
	{
		GfParmReleaseHandle(m_priv->xmlDescParmHdle);
		m_priv->xmlDescParmHdle = 0;
		return true;
	}
	return false;
}

bool GfuiMenuScreen::CreateStaticControls()
{
	if (!m_priv->xmlDescParmHdle)
		OpenXMLDescriptor();
	
	return m_priv->menuHdle && m_priv->xmlDescParmHdle
		&& ::CreateStaticControls(m_priv->xmlDescParmHdle, m_priv->menuHdle);
}

int GfuiMenuScreen::CreateButtonControl(const char *pszName, void *userData, tfuiCallback onPush)
{
	if (!m_priv->xmlDescParmHdle)
		OpenXMLDescriptor();
	
	if (m_priv->mapControlIds.find(pszName) == m_priv->mapControlIds.end())
	{
		const int nCtrlId =
			::CreateButtonControl(m_priv->menuHdle, m_priv->xmlDescParmHdle, pszName,
								  userData, onPush);
		if (nCtrlId >= 0)
			m_priv->mapControlIds[pszName] = nCtrlId;

		return nCtrlId;
	}

	GfError("GfuiMenuScreen::CreateButtonControl(%s) : Duplicate name\n", pszName);
	return -1;
}

int GfuiMenuScreen::CreateButtonControlEx(const char *pszName, void *userDataOnPush,
										  tfuiCallback onPush,
										  void *userDataOnFocus, tfuiCallback onFocus,
										  tfuiCallback onFocusLost)
{
	if (!m_priv->xmlDescParmHdle)
		OpenXMLDescriptor();
	
	if (m_priv->mapControlIds.find(pszName) == m_priv->mapControlIds.end())
	{
		const int nCtrlId =
			::CreateButtonControlEx(m_priv->menuHdle, m_priv->xmlDescParmHdle, pszName,
									userDataOnPush, onPush, userDataOnFocus, onFocus, onFocusLost);
		if (nCtrlId >= 0)
			m_priv->mapControlIds[pszName] = nCtrlId;

		return nCtrlId;
	}

	GfError("GfuiMenuScreen::CreateButtonControlEx(%s) : Duplicate name\n", pszName);
	return -1;
}

int GfuiMenuScreen::CreateStaticImageControl(const char *pszName)
{
	if (!m_priv->xmlDescParmHdle)
		OpenXMLDescriptor();

	if (m_priv->mapControlIds.find(pszName) == m_priv->mapControlIds.end())
	{
		const int nCtrlId =
			::CreateStaticImageControl(m_priv->menuHdle, m_priv->xmlDescParmHdle, pszName);
		if (nCtrlId >= 0)
			m_priv->mapControlIds[pszName] = nCtrlId;

		return nCtrlId;
	}

	GfError("GfuiMenuScreen::CreateStaticImageControl(%s) : Duplicate name\n", pszName);
	return -1;
}

int GfuiMenuScreen::CreateLabelControl(const char *pszName)
{
	if (!m_priv->xmlDescParmHdle)
		OpenXMLDescriptor();
	
	if (m_priv->mapControlIds.find(pszName) == m_priv->mapControlIds.end())
	{
		const int nCtrlId =
			::CreateLabelControl(m_priv->menuHdle, m_priv->xmlDescParmHdle, pszName);
		if (nCtrlId >= 0)
			m_priv->mapControlIds[pszName] = nCtrlId;

		return nCtrlId;
	}

	GfError("GfuiMenuScreen::CreateLabelControl(%s) : Duplicate name\n", pszName);
	return -1;
}

int GfuiMenuScreen::CreateEditControl(const char *pszName,
									  void *userDataOnFocus, tfuiCallback onFocus,
									  tfuiCallback onFocusLost)
{
	if (!m_priv->xmlDescParmHdle)
		OpenXMLDescriptor();
	
	if (m_priv->mapControlIds.find(pszName) == m_priv->mapControlIds.end())
	{
		const int nCtrlId =
			::CreateEditControl(m_priv->menuHdle, m_priv->xmlDescParmHdle, pszName,
							  userDataOnFocus, onFocus, onFocusLost);
		if (nCtrlId >= 0)
			m_priv->mapControlIds[pszName] = nCtrlId;

		return nCtrlId;
	}

	GfError("GfuiMenuScreen::CreateEditControl(%s) : Duplicate name\n", pszName);
	return -1;
}

int GfuiMenuScreen::CreateScrollListControl(const char *pszName,
											void *userData, tfuiCallback onSelect)
{
	if (!m_priv->xmlDescParmHdle)
		OpenXMLDescriptor();
	
	if (m_priv->mapControlIds.find(pszName) == m_priv->mapControlIds.end())
	{
		const int nCtrlId =
			::CreateScrollListControl(m_priv->menuHdle, m_priv->xmlDescParmHdle, pszName,
									userData, onSelect);
		if (nCtrlId >= 0)
			m_priv->mapControlIds[pszName] = nCtrlId;

		return nCtrlId;
	}

	GfError("GfuiMenuScreen::CreateScrollListControl(%s) : Duplicate name\n", pszName);
	return -1;
}

int GfuiMenuScreen::CreateComboboxControl(const char *pszName,
										  void *userData, tfuiComboboxCallback onChange)
{
	if (!m_priv->xmlDescParmHdle)
		OpenXMLDescriptor();
	
	if (m_priv->mapControlIds.find(pszName) == m_priv->mapControlIds.end())
	{
		const int nCtrlId =
			::CreateComboboxControl(m_priv->menuHdle, m_priv->xmlDescParmHdle, pszName,
									userData, onChange);
		if (nCtrlId >= 0)
			m_priv->mapControlIds[pszName] = nCtrlId;

		return nCtrlId;
	}

	GfError("GfuiMenuScreen::CreateComboboxControl(%s) : Duplicate name\n", pszName);
	return -1;
}

int GfuiMenuScreen::CreateCheckboxControl(const char *pszName,
										  void *userData, tfuiCheckboxCallback onChange)
{
	if (!m_priv->xmlDescParmHdle)
		OpenXMLDescriptor();
	
	if (m_priv->mapControlIds.find(pszName) == m_priv->mapControlIds.end())
	{
		const int nCtrlId = 
			::CreateCheckboxControl(m_priv->menuHdle, m_priv->xmlDescParmHdle, pszName,
									userData, onChange);
		if (nCtrlId >= 0)
			m_priv->mapControlIds[pszName] = nCtrlId;

		return nCtrlId;
	}

	GfError("GfuiMenuScreen::CreateCheckboxControl(%s) : Duplicate name\n", pszName);
	return -1;
}

int GfuiMenuScreen::CreateProgressbarControl(const char *pszName)
{
	if (!m_priv->xmlDescParmHdle)
		OpenXMLDescriptor();
	
	if (m_priv->mapControlIds.find(pszName) == m_priv->mapControlIds.end())
	{
		const int nCtrlId =
			::CreateProgressbarControl(m_priv->menuHdle, m_priv->xmlDescParmHdle, pszName);
		if (nCtrlId >= 0)
			m_priv->mapControlIds[pszName] = nCtrlId;

		return nCtrlId;
	}

	GfError("GfuiMenuScreen::CreateProgressbarControl(%s) : Duplicate name\n", pszName);
	return -1;
}

void GfuiMenuScreen::AddDefaultShortcuts()
{
	GfuiMenuDefaultKeysAdd(m_priv->menuHdle);
}
		
void GfuiMenuScreen::AddShortcut(int key, const char *descr, void *userData,
								 tfuiCallback onKeyPressed, tfuiCallback onKeyReleased)
{
	GfuiAddKey(m_priv->menuHdle, key, descr, userData, onKeyPressed, onKeyReleased);
}


int GfuiMenuScreen::GetDynamicControlId(const char *pszName) const
{
	std::map<std::string, int>::const_iterator iterCtrlId = m_priv->mapControlIds.find(pszName);

	return iterCtrlId == m_priv->mapControlIds.end() ? -1 : (*iterCtrlId).second;
}

void GfuiMenuScreen::RunMenu()
{
	GfuiScreenActivate(m_priv->menuHdle);
}
