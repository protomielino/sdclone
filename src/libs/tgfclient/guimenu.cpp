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
#include <SDL/SDL.h>

#include "tgfclient.h"
#include "gui.h"
#include "guimenu.h"
#include <string>

void
gfMenuInit(void)
{
}

/** Add the default menu keyboard callback to a screen.
    The keys are:
    <br><tt>F1 .......... </tt>Help
    <br><tt>Escape ...... </tt>Quit the menu
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
    GfuiAddKey(scr, SDLK_TAB, "Select Next Entry", NULL, gfuiSelectNext, NULL);
    GfuiAddKey(scr, SDLK_RETURN, "Perform Action", (void*)2, gfuiMouseAction, NULL);
    GfuiAddSKey(scr, SDLK_UP, "Select Previous Entry", NULL, gfuiSelectPrev, NULL);
    GfuiAddSKey(scr, SDLK_DOWN, "Select Next Entry", NULL, gfuiSelectNext, NULL);
    GfuiAddSKey(scr, SDLK_PAGEUP, "Select Previous Entry", NULL, gfuiSelectPrev, NULL);
    GfuiAddSKey(scr, SDLK_PAGEDOWN, "Select Next Entry", NULL, gfuiSelectNext, NULL);
    GfuiAddSKey(scr, SDLK_F1, "Help", scr, GfuiHelpScreen, NULL);
    GfuiAddSKey(scr, SDLK_F12, "Screen-Shot", NULL, GfuiScreenShot, NULL);
    
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

    GfuiAddKey(scr, SDLK_ESCAPE, tip, userdata, onpush, NULL);

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

    GfuiAddKey(scr, SDLK_ESCAPE, tip, userdata, onpush, NULL);

    return bId;
}

int 
GetFontSize(std::string strTextsize)
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
GetAlignment(std::string strAlignH,std::string strAlignV)
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
GetHAlignment(std::string strAlignH)
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
	std::string strValue = GfParmGetStr(param,pControlName,pField,"");
	if (strValue == "")
		return false;

	unsigned int c = strtol(strValue.c_str(),NULL,0);
	color = GetColor(c);

	return true;
}


bool 
ReadBoolean(void *param,const char *pControlName,const char *pField)
{
	std::string strValue = GfParmGetStr(param, pControlName,pField,"yes");
	if (strValue == "no")
		return false;

	return true;
}

bool
GetControlValues(void *param,const char *pControlName,std::string &strText,std::string &strTip,int &x,int &y,int &textSize,int &alignment)
{
	std::string strControlName = pControlName;
	strControlName = "dynamiccontrols/"+strControlName;

	strText = GfParmGetStr(param, strControlName.c_str(), "text", "");
	strTip = GfParmGetStr(param, strControlName.c_str(), "tip", "");
	x = (int)GfParmGetNum(param,strControlName.c_str(),"x",NULL,0.0);
	y = (int)GfParmGetNum(param,strControlName.c_str(),"y",NULL,0.0);
	std::string strTextsize = GfParmGetStr(param, strControlName.c_str(), "textsize", "");
	textSize = GetFontSize(strTextsize);
	std::string strAlignH = GfParmGetStr(param, strControlName.c_str(), "alignH", "");
	std::string strAlignV = GfParmGetStr(param, strControlName.c_str(), "alignV", "");
	alignment = GetAlignment(strAlignH,strAlignV);
	

	return true;
}


int 
CreateStaticImage(void *menuHandle,void *param,const char *pControlName)
{
	std::string strImage;
	//int id;
	int x,y,w,h;

	strImage = GfParmGetStr(param, pControlName, "image", "");

	x = (int)GfParmGetNum(param,pControlName,"x",NULL,0.0);
	y = (int)GfParmGetNum(param,pControlName,"y",NULL,0.0);
	w = (int)GfParmGetNum(param,pControlName,"width",NULL,0.0);
	h = (int)GfParmGetNum(param,pControlName,"height",NULL,0.0);

	return GfuiStaticImageCreate(menuHandle,x,y,w,h,strImage.c_str());
}

int 
CreateBackgroundImage(void *menuHandle,void *param,const char *pControlName)
{
	std::string strImage;
	strImage = GfParmGetStr(param, pControlName, "image", "");
	GfuiScreenAddBgImg(menuHandle, strImage.c_str());
	return 1;
}

int
CreateStaticImageControl(void *menuHandle,void *param,const char *pControlName)
{
	int labelId;
	std::string strControlName = pControlName;
	strControlName = "dynamiccontrols/"+strControlName;
	
	labelId = CreateStaticImage(menuHandle,param,strControlName.c_str());
	return labelId;
}

int 
CreateLabel(void *menuHandle,void *param,const char *pControlName)
{
	std::string strType = GfParmGetStr(param, pControlName, "type", "");
	if (strType!="label")
	{
		printf("Error not label type\n");
		return -1;
	}
	
	std::string strText,strTip;
	int textsize;
	int alignment;
	int labelId;
	int x,y;

	strText = GfParmGetStr(param, pControlName, "text", "");
	x = (int)GfParmGetNum(param,pControlName,"x",NULL,0.0);
	y = (int)GfParmGetNum(param,pControlName,"y",NULL,0.0);
	std::string strTextsize = GfParmGetStr(param, pControlName, "textsize", "");
	textsize = GetFontSize(strTextsize);
	std::string strAlignH = GfParmGetStr(param, pControlName, "alignH", "");
	std::string strAlignV = GfParmGetStr(param, pControlName, "alignV", "");
	alignment = GetAlignment(strAlignH,strAlignV);
	
	Color c;
	bool bColor = GetColorFromXML(param,pControlName,"color",c);


    	labelId = GfuiLabelCreate(menuHandle, strText.c_str(), textsize, x, y, alignment, 32);
	if (bColor)
		GfuiLabelSetColor(menuHandle, labelId, c.GetPtr());

    return labelId;
}


int 
CreateLabelControl(void *menuHandle,void *param,const char *pControlName)
{
    	int labelId;
    	std::string strControlName = pControlName;
    	strControlName = "dynamiccontrols/"+strControlName;

    	labelId = CreateLabel(menuHandle,param,strControlName.c_str());
    	return labelId;
}

int 
CreateTextButtonControl(void *menuHandle,void *param,const char *pControlName,void *userdata, tfuiCallback onpush)
{
	std::string strText,strTip;
	int textsize;
	int id;
	int x,y,width;

	strText = GfParmGetStr(param, pControlName, "text", "");
	strTip = GfParmGetStr(param, pControlName, "tip", "");
	x = (int)GfParmGetNum(param,pControlName,"x",NULL,0.0);
	y = (int)GfParmGetNum(param,pControlName,"y",NULL,0.0);
	std::string strTextsize = GfParmGetStr(param, pControlName, "textsize", "");
	textsize = GetFontSize(strTextsize);
	std::string strAlignH = GfParmGetStr(param, pControlName, "alignH", "");
	int alignH = GetHAlignment(strAlignH);

	
	width = (int)GfParmGetNum(param,pControlName,"width",NULL,0.0);
	if (width == 0)
	    width =  GFUI_BTNSZ;

	Color c,fc,pc;
	bool bColor = GetColorFromXML(param,pControlName,"color",c);
	bool bFocusColor = GetColorFromXML(param,pControlName,"focuscolor",fc);
	bool bPushedColor = GetColorFromXML(param,pControlName,"pushedcolor",pc);

	bool bShowbox = ReadBoolean(param,pControlName,"showbox");

	std::string strEnabledImage,strDisabledImage,strFocusedImage,strPushedImage;
	strDisabledImage = GfParmGetStr(param, pControlName, "disabledimage", "");
	strEnabledImage = GfParmGetStr(param, pControlName, "enabledimage", "");
	strFocusedImage = GfParmGetStr(param, pControlName, "focusedimage", "");
	strPushedImage = GfParmGetStr(param, pControlName, "pushedimage", "");

	int imgX,imgY,imgWidth,imgHeight;
	imgX = (int)GfParmGetNum(param,pControlName,"imagex",NULL,0.0);
	imgY = (int)GfParmGetNum(param,pControlName,"imagey",NULL,0.0);
	imgWidth = (int)GfParmGetNum(param,pControlName,"imagewidth",NULL,20.0);
	imgHeight = (int)GfParmGetNum(param,pControlName,"imageheight",NULL,20.0);
	



    	tMnuCallbackInfo * cbinfo = NULL;

    	if (strTip!="")
	{
    		cbinfo = (tMnuCallbackInfo*)calloc(1, sizeof(tMnuCallbackInfo));
    		cbinfo->screen = menuHandle;
    		cbinfo->labelId = GfuiTipCreate(menuHandle, strTip.c_str(), strTip.length());

	    	GfuiVisibilitySet(menuHandle, cbinfo->labelId, 0);
		id = GfuiButtonCreate(menuHandle,
			   strText.c_str(),
			   textsize,
			   x, y, width, alignH, GFUI_MOUSE_UP,
			   userdata, onpush,
			   (void*)cbinfo, dispInfo,
			   remInfo);
		GfuiButtonShowBox(menuHandle,id,bShowbox);
		GfuiButtonSetImage(menuHandle,id,imgX,imgY,imgWidth,imgHeight,
			strDisabledImage.c_str(),strEnabledImage.c_str(),strFocusedImage.c_str(),strPushedImage.c_str());

	}
	else
	{
		id = GfuiButtonCreate(menuHandle,
			   strText.c_str(),
			   textsize,
			   x, y, width, alignH, GFUI_MOUSE_UP,
			   userdata, onpush,
			   NULL, NULL,
			   NULL);
		GfuiButtonShowBox(menuHandle,id,bShowbox);
		GfuiButtonSetImage(menuHandle,id,imgX,imgY,imgWidth,imgHeight,
			strDisabledImage.c_str(),strEnabledImage.c_str(),strFocusedImage.c_str(),strPushedImage.c_str());

    	}

		if (bColor)
			GfuiButtonSetColor(menuHandle,id,c);

		if (bFocusColor)
			GfuiButtonSetFocusColor(menuHandle,id,fc);

		if (bPushedColor)
			GfuiButtonSetPushedColor(menuHandle,id,pc);

	
    return id;
}

int 
CreateImageButtonControl(void *menuHandle,void *param,const char *pControlName,void *userdata, tfuiCallback onpush)
{
	std::string strTip,strText;
	//int textsize;
	int alignment;
	int id = -1;
	int x,y,w,h;
	std::string strEnabledImage,strDisabledImage,strFocusedImage,strPushedImage;


	strTip = GfParmGetStr(param, pControlName, "tip", "");
	strDisabledImage = GfParmGetStr(param, pControlName, "disabledimage", "");
	strEnabledImage = GfParmGetStr(param, pControlName, "enabledimage", "");
	strFocusedImage = GfParmGetStr(param, pControlName, "focusedimage", "");
	strPushedImage = GfParmGetStr(param, pControlName, "pushedimage", "");

	x = (int)GfParmGetNum(param,pControlName,"x",NULL,0.0);
	y = (int)GfParmGetNum(param,pControlName,"y",NULL,0.0);
	w = (int)GfParmGetNum(param,pControlName,"width",NULL,0.0);
	h = (int)GfParmGetNum(param,pControlName,"height",NULL,0.0);

	std::string strAlignH = GfParmGetStr(param, pControlName, "alignH", "");
	std::string strAlignV = GfParmGetStr(param, pControlName, "alignV", "");
	alignment = GetAlignment(strAlignH,strAlignV);

	id = GfuiGrButtonCreate(menuHandle,
		strDisabledImage.c_str(),strEnabledImage.c_str(),strFocusedImage.c_str(),strPushedImage.c_str(),
		x,y,alignment,GFUI_MOUSE_UP
		,userdata,
		onpush,
		NULL,
		(tfuiCallback)NULL,
		(tfuiCallback)NULL);

    	return id;
}

int 
CreateButtonControl(void *menuHandle,void *param,const char *pControlName,void *userdata, tfuiCallback onpush)
{
	std::string strControlName = pControlName;
	strControlName = "dynamiccontrols/"+strControlName;

	std::string strType = GfParmGetStr(param, strControlName.c_str(), "type", "");
	if (strType == "textbutton")
		return CreateTextButtonControl(menuHandle,param,strControlName.c_str(),userdata,onpush);
	else if(strType == "imagebutton")
		return CreateImageButtonControl(menuHandle,param,strControlName.c_str(),userdata,onpush);
	else
		printf("ERROR: unknown button type\n");

	return -1;
}

int 
CreateEditControl(void *menuHandle,void *param,const char *pControlName,void *userDataOnFocus, tfuiCallback onFocus, tfuiCallback onFocusLost)
{
	std::string strControlName = pControlName;
	strControlName = "dynamiccontrols/"+strControlName;

	std::string strType = GfParmGetStr(param, strControlName.c_str(), "type", "");
	if (strType != "editbox")
		return -1;

	int id = -1;
	
	std::string strText,strTip;
	int textsize;
	int x,y,width,maxlen;

	strText = GfParmGetStr(param, strControlName.c_str(), "text", "");
	x = (int)GfParmGetNum(param,strControlName.c_str(),"x",NULL,0.0);
	y = (int)GfParmGetNum(param,strControlName.c_str(),"y",NULL,0.0);
	std::string strTextsize = GfParmGetStr(param, strControlName.c_str(), "textsize", "");
	textsize = GetFontSize(strTextsize);
	std::string strAlignH = GfParmGetStr(param, strControlName.c_str(), "alignH", "");

	
	width = (int)GfParmGetNum(param,strControlName.c_str(),"width",NULL,0.0);
	if (width == 0)
	    width =  GFUI_BTNSZ;

	maxlen = (int)GfParmGetNum(param,strControlName.c_str(),"maxlen",NULL,0.0);

	Color c,fc;

	bool bColor = GetColorFromXML(param,pControlName,"color",c);
	bool bFocusColor = GetColorFromXML(param,pControlName,"focuscolor",fc);


	id = GfuiEditboxCreate(menuHandle,strText.c_str(), textsize,x,y,width,maxlen,userDataOnFocus,onFocus,onFocusLost);
	if (bColor)
		GfuiEditboxSetColor(menuHandle,id,c);
	
	if (bFocusColor)
		GfuiEditboxSetFocusColor(menuHandle,id,fc);		

	return id;
}

int 
CreateScrollListControl(void *menuHandle,void *param,const char *pControlName,void *userdata, tfuiCallback onSelect)
{
	std::string strControlName = pControlName;
	strControlName = "dynamiccontrols/"+strControlName;

	std::string strType = GfParmGetStr(param, strControlName.c_str(), "type", "");
	if (strType != "scrolllist")
		return -1;

	int x,y,w,h,textsize;
	int scrollbarpos = GFUI_SB_NONE;
	int alignment;

	x = (int)GfParmGetNum(param,strControlName.c_str(),"x",NULL,0.0);
	y = (int)GfParmGetNum(param,strControlName.c_str(),"y",NULL,0.0);
	w = (int)GfParmGetNum(param,strControlName.c_str(),"width",NULL,0.0);
	h = (int)GfParmGetNum(param,strControlName.c_str(),"height",NULL,0.0);
	
	std::string strVal = GfParmGetStr(param,strControlName.c_str(),"scrollbarposition","none");
	if (strVal == "none")
		scrollbarpos = GFUI_SB_NONE;
	else if (strVal =="left")
		scrollbarpos = GFUI_SB_LEFT;
	else if (strVal == "right")
		scrollbarpos = GFUI_SB_RIGHT;

	std::string strTextsize = GfParmGetStr(param, strControlName.c_str(), "textsize", "");
	textsize = GetFontSize(strTextsize);


	std::string strAlignH = GfParmGetStr(param, strControlName.c_str(), "alignH", "");
	std::string strAlignV = GfParmGetStr(param, strControlName.c_str(), "alignV", "");
	alignment = GetAlignment(strAlignH,strAlignV);

	Color c,sc;
	bool bColor = GetColorFromXML(param,pControlName,"color",c);
	bool bSelectColor = GetColorFromXML(param,pControlName,"selectcolor",sc);
	
	int id = -1;

	id = GfuiScrollListCreate(menuHandle, textsize,x,y,alignment,w,h,scrollbarpos,userdata,onSelect);

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
		char buf[1024];
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
			printf("ERROR unknown static control type = %s\n",strType.c_str());
		}
	}

	return true;
}



void *
LoadMenuXML(const char *pMenuPath)
{
	std::string strPath = pMenuPath;
	strPath = "config/"+strPath;
    void *param = NULL;

    char buf[1024];
	sprintf(buf, "%s%s", GetLocalDir(),strPath.c_str());
    param = GfParmReadFile(buf, GFPARM_RMODE_STD);

   return param;
}
