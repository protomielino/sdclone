/***************************************************************************
                          menu.cpp -- menu management                            
                             -------------------                                         
    created              : Fri Aug 13 22:23:19 CEST 1999
    copyright            : (C) 1999 by Eric Espie                         
    email                : torcs@free.fr   
    version              : $Id: guimenu.cpp,v 1.2 20 Mar 2006 04:31:22 torcs Exp $                                  
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
    @author	<a href=mailto:torcs@free.fr>Eric Espie</a>
    @version	$Id: guimenu.cpp,v 1.2 2003/06/24 21:02:25 torcs Exp $
    @ingroup	gui
*/


#include <stdio.h>
#include <cstring>
#include <stdlib.h>
#ifdef WIN32
#include <windows.h>
#endif

#include "tgfclient.h"
#include "gui.h"
#include "guimenu.h"
#include <string>
#include "gui.h"

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
    @ingroup	gui
    @param	scr	Screen Id
 */
void
GfuiMenuDefaultKeysAdd(void *scr)
{
    GfuiAddKey(scr, 9, "Select Next Entry", NULL, gfuiSelectNext, NULL);
    GfuiAddKey(scr, 13, "Perform Action", (void*)2, gfuiMouseAction, NULL);
    GfuiAddSKey(scr, GLUT_KEY_UP, "Select Previous Entry", NULL, gfuiSelectPrev, NULL);
    GfuiAddSKey(scr, GLUT_KEY_DOWN, "Select Next Entry", NULL, gfuiSelectNext, NULL);
    GfuiAddSKey(scr, GLUT_KEY_PAGE_UP, "Select Previous Entry", NULL, gfuiSelectPrev, NULL);
    GfuiAddSKey(scr, GLUT_KEY_PAGE_DOWN, "Select Next Entry", NULL, gfuiSelectNext, NULL);
    GfuiAddSKey(scr, GLUT_KEY_F1, "Help", scr, GfuiHelpScreen, NULL);
    GfuiAddSKey(scr, GLUT_KEY_F12, "Screen-Shot", NULL, GfuiScreenShot, NULL);
    
}

/** Create a new menu screen.
    Set the title of the menu
    Add the default keyboard callbacks to the menu.
    @ingroup	gui
    @param	title	title of the screen
    @return	Handle of the menu
 */
void *
GfuiMenuScreenCreate(const char *title)
{
    void	*scr;

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
    @ingroup	gui
    @param	scr		Screen (menu) handle
    @param	text		Text of the button
    @param	tip		Text of the tip displayed when the button is focused
    @param	userdata	Parameter of the Push function
    @param	onpush		Callback when the button is pushed
    @return	Button 3
 */
int
GfuiMenuButtonCreate(void *scr, const char *text, const char *tip, void *userdata, tfuiCallback onpush)
{
    tMnuCallbackInfo	*cbinfo;
    int			xpos, ypos;
    int			nbItems = ((tGfuiScreen*)scr)->nbItems++;
    int			bId;

    if (nbItems < 11) {
	xpos = 320;
	ypos = 380 - 30 * nbItems;
    } else {
	if (nbItems > 22) {
	    GfTrace("Too many items in that menu !!!\n");
	    return -1;
	}
	xpos = 380;
	ypos = 380 - 30 * (nbItems - 11);
    }

    cbinfo = (tMnuCallbackInfo*)calloc(1, sizeof(tMnuCallbackInfo));
    cbinfo->screen = scr;
    cbinfo->labelId = GfuiTipCreate(scr, tip, strlen(tip));

    GfuiVisibilitySet(scr, cbinfo->labelId, 0);
    
    bId = GfuiButtonCreate(scr,
			   text,
			   GFUI_FONT_LARGE,
			   xpos, ypos, GFUI_BTNSZ, GFUI_ALIGN_HC_VB, 0,
			   userdata, onpush,
			   (void*)cbinfo, dispInfo,
			   remInfo);

    return bId;
}


/** Add a button to a menu screen.
    @ingroup	gui
    @param	scr		Screen (menu) handle
    @param	text		Text of the button
    @param	tip		Text of the tip displayed when the button is focused
    @param	userdata	Parameter of the Push function
    @param	onpush		Callback when the button is pushed
    @return	Button Id
 */
int
GfuiMenuButtonCreateEx(void *scr, const char *text, const char *tip, void *userdata, tfuiCallback onpush,int xpos,int ypos,int fontSize,int align)
{
    tMnuCallbackInfo	*cbinfo;
    //int			xpos, ypos;
    //int			nbItems = ((tGfuiScreen*)scr)->nbItems++;
    int			bId;

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
			   userdata, onpush,
			   (void*)cbinfo, dispInfo,
			   remInfo);

    return bId;
}

/** Add the "Back" or "Quit" button at the bottom of the menu screen.
    @ingroup	gui
    @param	scr	Screen or Menu handle
    @param	text	Text of the button
    @param	tip	Text to display when the button is focused
    @param	userdata	Parameter of the Push function
    @param	onpush		Callback when the button is pushed
    @return	Button Id
 */
int
GfuiMenuBackQuitButtonCreate(void *scr, const char *text, const char *tip, void *userdata, tfuiCallback onpush)
{
    tMnuCallbackInfo	*cbinfo;
    int			xpos, ypos;
    int			bId;
    
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
			userdata, onpush,
			(void*)cbinfo, dispInfo,
			remInfo);

    GfuiAddKey(scr, 27, tip, userdata, onpush, NULL);

    return bId;
}


/** Add the "Back" or "Quit" button at the bottom of the menu screen.
    @ingroup	gui
    @param	scr	Screen or Menu handle
    @param	text	Text of the button
    @param	tip	Text to display when the button is focused
    @param	userdata	Parameter of the Push function
    @param	onpush		Callback when the button is pushed
    @return	Button Id
 */
int
GfuiMenuBackQuitButtonCreateEx(void *scr, const char *text, const char *tip, void *userdata, tfuiCallback onpush,int xpos,int ypos,int fontSize,int align)
{
    tMnuCallbackInfo	*cbinfo;
    int			bId;
    
    cbinfo = (tMnuCallbackInfo*)calloc(1, sizeof(tMnuCallbackInfo));
    cbinfo->screen = scr;
    cbinfo->labelId = GfuiTipCreate(scr, tip, strlen(tip));

    GfuiVisibilitySet(scr, cbinfo->labelId, 0);
    
    bId = GfuiButtonCreate(scr,
			text,
			fontSize,
			xpos, ypos, GFUI_BTNSZ, align, 0,
			userdata, onpush,
			(void*)cbinfo, dispInfo,
			remInfo);

    GfuiAddKey(scr, 27, tip, userdata, onpush, NULL);

    return bId;
}

int 
GetFontSize(const std::string& strTextsize)
{
	int tSize = GFUI_FONT_MEDIUM;

	if (strTextsize=="medium")
		tSize = GFUI_FONT_MEDIUM;
	else if (strTextsize=="large")
		tSize = GFUI_FONT_LARGE;
	else if (strTextsize=="small")
		tSize = GFUI_FONT_SMALL;
	else if (strTextsize=="big")
		tSize = GFUI_FONT_BIG;

	return tSize;
}

int 
GetAlignment(const std::string& strAlignH, const std::string& strAlignV)
{
	
	int align = GFUI_ALIGN_HL_VB;
	if (strAlignH == "left")
	{
		
		align = GFUI_ALIGN_HL_VB;
	}
	else if (strAlignH == "center")
	{
		align = GFUI_ALIGN_HC_VB;
	}
	else if (strAlignH == "right")
	{
		align = GFUI_ALIGN_HR_VB;
	}

	return align;
}

int 
GetHAlignment(const std::string& strAlignH)
{
	int align = 0; //left

	if (strAlignH == "left")
	{
		align = 0;
	}
	else if (strAlignH == "center")
	{
		align = 0x10;
	}
	else if (strAlignH == "right")
	{
		align = 0x20;
	}

	return align;
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
	const std::string strValue = GfParmGetStr(param,pControlName,pField,"");
	if (strValue == "")
		return false;

	unsigned int c = strtol(strValue.c_str(),NULL,0);
	color = GetColor(c);

	return true;
}


bool 
ReadBoolean(void *param,const char *pControlName,const char *pField)
{
	const std::string strValue = GfParmGetStr(param, pControlName,pField,"yes");
	if (strValue == "no")
		return false;

	return true;
}

bool
GetControlValues(void *param,const char *pControlName,std::string &strText,std::string &strTip,int &x,int &y,int &textSize,int &alignment)
{
	std::string strControlName("dynamiccontrols/");
	strControlName += pControlName;

	strText = GfParmGetStr(param, strControlName.c_str(), "text", "");
	strTip = GfParmGetStr(param, strControlName.c_str(), "tip", "");

	x = (int)GfParmGetNum(param,strControlName.c_str(),"x",NULL,0.0);
	y = (int)GfParmGetNum(param,strControlName.c_str(),"y",NULL,0.0);

	const char* pszTextsize = GfParmGetStr(param, strControlName.c_str(), "textsize", "");
	textSize = GetFontSize(pszTextsize);

	const char* pszAlignH = GfParmGetStr(param, pControlName, "alignH", "");
	const char* pszAlignV = GfParmGetStr(param, pControlName, "alignV", "");
	alignment = GetAlignment(pszAlignH,pszAlignV);

	return true;
}


int 
CreateStaticImage(void *menuHandle,void *param,const char *pControlName)
{
	const char* pszImage = GfParmGetStr(param, pControlName, "image", "");

	const int x = (int)GfParmGetNum(param,pControlName,"x",NULL,0.0);
	const int y = (int)GfParmGetNum(param,pControlName,"y",NULL,0.0);
	const int w = (int)GfParmGetNum(param,pControlName,"width",NULL,0.0);
	const int h = (int)GfParmGetNum(param,pControlName,"height",NULL,0.0);

	return GfuiStaticImageCreate(menuHandle,x,y,w,h,pszImage);
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
	const std::string strType = GfParmGetStr(param, pControlName, "type", "");

	if (strType!="label")
	{
		GfError("Error: Control '%s' is not a label\n", pControlName);
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
	// Note: Defaul maxlen = 32 because if 0, strlen(pszText) will be used for ever.
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
CreateTextButtonControl(void *menuHandle,void *param,const char *pControlName,void *userdata, tfuiCallback onpush, void *userDataOnFocus, tfuiCallback onFocus, tfuiCallback onFocusLost)
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
				  userdata, onpush,
				  userDataOnFocus, onFocus,
				  onFocusLost);

	const bool bShowbox = ReadBoolean(param,pControlName,"showbox");

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
CreateImageButtonControl(void *menuHandle,void *param,const char *pControlName,void *userdata, tfuiCallback onpush, void *userDataOnFocus, tfuiCallback onFocus, tfuiCallback onFocusLost)
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
	//const int w = (int)GfParmGetNum(param,pControlName,"width",NULL,0.0);
	//const int h = (int)GfParmGetNum(param,pControlName,"height",NULL,0.0);

	const char* pszAlignH = GfParmGetStr(param, pControlName, "alignH", "");
	const char* pszAlignV = GfParmGetStr(param, pControlName, "alignV", "");
	const int alignment = GetAlignment(pszAlignH,pszAlignV);

	return GfuiGrButtonCreate(menuHandle,
				  pszDisabledImage,pszEnabledImage,
				  pszFocusedImage,pszPushedImage,
				  x,y,alignment,GFUI_MOUSE_UP,
				  userdata,
				  onpush,
				  userDataOnFocus, 
				  onFocus,
				  onFocusLost);
}

int 
CreateButtonControl(void *menuHandle,void *param,const char *pControlName,void *userdata, tfuiCallback onpush)
{
	return CreateButtonControlEx(menuHandle,param,pControlName,userdata,onpush,NULL,NULL,NULL);
}

int 
CreateButtonControlEx(void *menuHandle,void *param,const char *pControlName,void *userdata, tfuiCallback onpush, void *userDataOnFocus, tfuiCallback onFocus, tfuiCallback onFocusLost)
{
	std::string strControlName("dynamiccontrols/");
	strControlName += pControlName;

	const std::string strType = GfParmGetStr(param, strControlName.c_str(), "type", "");
	if (strType == "textbutton")
		return CreateTextButtonControl(menuHandle,param,strControlName.c_str(),userdata,onpush,NULL,NULL,NULL);
	else if(strType == "imagebutton")
		return CreateImageButtonControl(menuHandle,param,strControlName.c_str(),userdata,onpush,NULL,NULL,NULL);
	else
	    GfError("Error: Unknown button type '%s'\n", strType.c_str());

	return -1;
}

int 
CreateEditControl(void *menuHandle,void *param,const char *pControlName,void *userDataOnFocus, tfuiCallback onFocus, tfuiCallback onFocusLost)
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

	std::string strControlName("dynamiccontrols/");
	strControlName += pControlName;

	const std::string strType = GfParmGetStr(param, strControlName.c_str(), "type", "");
	if (strType != "editbox")
		return -1;

	const char* pszText = GfParmGetStr(param, strControlName.c_str(), "text", "");
	const int x = (int)GfParmGetNum(param,strControlName.c_str(),"x",NULL,0.0);
	const int y = (int)GfParmGetNum(param,strControlName.c_str(),"y",NULL,0.0);
	const char* pszTextsize = GfParmGetStr(param, strControlName.c_str(), "textsize", "");
	const int textsize = GetFontSize(pszTextsize);
	std::string strAlignH = GfParmGetStr(param, strControlName.c_str(), "alignH", "");

	
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
CreateScrollListControl(void *menuHandle,void *param,const char *pControlName,void *userdata, tfuiCallback onSelect)
{
	std::string strControlName("dynamiccontrols/");
	strControlName += pControlName;

	std::string strType = GfParmGetStr(param, strControlName.c_str(), "type", "");
	if (strType != "scrolllist")
		return -1;

	const int x = (int)GfParmGetNum(param,strControlName.c_str(),"x",NULL,0.0);
	const int y = (int)GfParmGetNum(param,strControlName.c_str(),"y",NULL,0.0);
	const int w = (int)GfParmGetNum(param,strControlName.c_str(),"width",NULL,0.0);
	const int h = (int)GfParmGetNum(param,strControlName.c_str(),"height",NULL,0.0);
	
	const char* pszTextsize = GfParmGetStr(param, strControlName.c_str(), "textsize", "");
	const int textsize = GetFontSize(pszTextsize);

	const char* pszAlignH = GfParmGetStr(param, pControlName, "alignH", "");
	const char* pszAlignV = GfParmGetStr(param, pControlName, "alignV", "");
	const int alignment = GetAlignment(pszAlignH,pszAlignV);

	std::string strVal = GfParmGetStr(param,strControlName.c_str(),"scrollbarposition","none");
	int scrollbarpos = GFUI_SB_NONE;
	if (strVal == "none")
		scrollbarpos = GFUI_SB_NONE;
	else if (strVal =="left")
		scrollbarpos = GFUI_SB_LEFT;
	else if (strVal == "right")
		scrollbarpos = GFUI_SB_RIGHT;

	int id = GfuiScrollListCreate(menuHandle, textsize,x,y,alignment,w,h,scrollbarpos,userdata,onSelect);

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

	int nControls = GfParmGetEltNb(param, "staticcontrols");

	for (int i=1;i<=nControls;i++)
	{
		std::string strType;
		char buf[32];
	    	sprintf(buf, "staticcontrols/%i",i);
		strType = GfParmGetStr(param, buf, "type", "");
	
		if (strType == "label")
		{
			/*int handle = */CreateLabel(menuHandle,param,buf);
		}
		else if (strType == "staticimage")
		{
			/*int handle = */CreateStaticImage(menuHandle,param,buf);
		}
		else if (strType == "backgroundimage")
		{
			CreateBackgroundImage(menuHandle,param,buf);
		}
		else
		{
			GfError("Errot: Unknown static control type '%s'\n", strType.c_str());
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
	sprintf(buf, "%s%s", GetDataDir(),strPath.c_str());

	return GfParmReadFile(buf, GFPARM_RMODE_STD);
}
