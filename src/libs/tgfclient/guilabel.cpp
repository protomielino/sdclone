/***************************************************************************
                        guilabel.cpp -- labels management                           
                             -------------------                                         
    created              : Fri Aug 13 22:22:12 CEST 1999
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
    		GUI labels management.
    @author	<a href=mailto:torcs@free.fr>Eric Espie</a>
    @version	$Id$
    @ingroup	gui
*/

#include <cstdlib>
#include <cstring>

#include "gui.h"
#include "guimenu.h"


static int NTipX = 320;
static int NTipY = 15;
static int NTipWidth = 620;
static int NTipFontId = GFUI_FONT_SMALL;
static int NTipAlign = GFUI_ALIGN_HC_VB;


void
gfuiInitLabel(void)
{
	char path[512];

	// Get tip layout properties from the screen config file.
	sprintf(path, "%s%s", GfLocalDir(), GFSCR_CONF_FILE);
	void* hparmScr = GfParmReadFile(path, GFPARM_RMODE_STD | GFPARM_RMODE_CREAT);

	sprintf(path, "%s/%s", GFSCR_SECT_MENUSETTINGS, GFSCR_SECT_TIP);
	NTipX = (int)GfParmGetNum(hparmScr, path, GFSCR_ATT_X, 0, 320.0);
	NTipY = (int)GfParmGetNum(hparmScr, path, GFSCR_ATT_Y, 0, 10.0);
	NTipWidth = (int)GfParmGetNum(hparmScr, path, GFSCR_ATT_WIDTH, 0, 640.0);
	NTipFontId = gfuiMenuGetFontId(GfParmGetStr(hparmScr, path, GFSCR_ATT_FONT, "small"));
	NTipAlign = gfuiMenuGetAlignment(GfParmGetStr(hparmScr, path, GFSCR_ATT_ALIGN, "center"));

	GfParmReleaseHandle(hparmScr);
}

/** Initialize a label
    @ingroup	gui
    @param	label	The label to initialize
    @param	text	Text to display
    @param	maxlen	Maximum length of the button string (used when the label is changed)
                    <br>0 for the text length.
    @param	x	Position of the label on the screen (pixels)
    @param	y	Position of the label on the screen (pixels)
    @param	align	Alignment:
    			<br>GFUI_ALIGN_HR_VB	horizontal right, vertical bottom
    			<br>GFUI_ALIGN_HR_VC	horizontal right, vertical center
    			<br>GFUI_ALIGN_HR_VT	horizontal right, vertical top
    			<br>GFUI_ALIGN_HC_VB	horizontal center, vertical bottom
    			<br>GFUI_ALIGN_HC_VB	horizontal center, vertical center
    			<br>GFUI_ALIGN_HC_VB	horizontal center, vertical top
    			<br>GFUI_ALIGN_HL_VB	horizontal left, vertical bottom
    			<br>GFUI_ALIGN_HL_VB	horizontal left, vertical center
    			<br>GFUI_ALIGN_HL_VB	horizontal left, vertical top
    @param	width	Width (pixels) of the display bounding box
                    <br>0 for the text width (from the font specs and maxlen).
    @param	font	Font id
    @param	bgColor	Pointer to static RGBA background color array
                    <br>0 for gfuiColors[GFUI_BGCOLOR] 
    @param	fgColor	Pointer on static RGBA foreground color array
                    <br>0 for gfuiColors[GFUI_LABELCOLOR] 
    @param	bgFocusColor	Pointer to static RGBA focused background color array
                    <br>0 for bgColor
    @param	fgFocusColor	Pointer on static RGBA focused foreground color array
                    <br>0 for fgColor
    @param	userDataOnFocus	User data given to the onFocus[Lost] call back functions
    @param	onFocus	Call back function called when getting the focus
    @param	onFocusLost	Call back function called when loosing the focus
    @return	None
 */
void
gfuiLabelInit(tGfuiLabel *label, const char *text, int maxlen,
			  int x, int y, int align, int width, int font,
			  const float *bgColor, const float *fgColor,
			  const float *bgFocusColor, const float *fgFocusColor,
			  void *userDataOnFocus, tfuiCallback onFocus, tfuiCallback onFocusLost)
{
    if (maxlen <= 0)
		maxlen = strlen(text);
    label->text = (char*)calloc(maxlen+1, 1);
    strncpy(label->text, text, maxlen);
    label->maxlen = maxlen;
    
    label->bgColor = GfuiColor::build(bgColor ? bgColor : gfuiColors[GFUI_BGCOLOR]);
    label->fgColor = GfuiColor::build(fgColor ? fgColor : gfuiColors[GFUI_LABELCOLOR]);
    label->bgFocusColor = bgFocusColor ? GfuiColor::build(bgFocusColor) : label->bgColor;
    label->fgFocusColor = fgFocusColor ? GfuiColor::build(fgFocusColor) : label->fgColor;

    label->font = gfuiFont[font];
    if (width == 0)
		width = gfuiFont[font]->getWidth((const char *)text);
	const int height = gfuiFont[font]->getHeight() - gfuiFont[font]->getDescender();
    
	label->align = align;
	
    label->userDataOnFocus = userDataOnFocus;
    label->onFocus = onFocus;
    label->onFocusLost = onFocusLost;
	
    switch(align)
	{
		case GFUI_ALIGN_HL_VB:
			label->x = x;
			label->y = y - gfuiFont[font]->getDescender();
			break;

		case GFUI_ALIGN_HL_VC:
			label->x = x;
			label->y = y - height / 2;
			break;

		case GFUI_ALIGN_HL_VT:
			label->x = x;
			label->y = y;
			break;

		case GFUI_ALIGN_HC_VB:
			label->x = x - width / 2;
			label->y = y - gfuiFont[font]->getDescender();
			break;

		case GFUI_ALIGN_HC_VC:
			label->x = x - width / 2;
			label->y = y - height / 2;
			break;

		case GFUI_ALIGN_HC_VT:
			label->x = x - width / 2;
			label->y = y;
			break;

		case GFUI_ALIGN_HR_VB:
			label->x = x - width;
			label->y = y - gfuiFont[font]->getDescender();
			break;

		case GFUI_ALIGN_HR_VC:
			label->x = x - width;
			label->y = y - height / 2;
			break;

		case GFUI_ALIGN_HR_VT:
			label->x = x - width;
			label->y = y;
			break;
	}
}

/** Create a new label
    @ingroup	gui
    @param	scr	Screen where to add the label
    @param	text	Text of the label
    @param	font	Font id
    @param	x	Position of the label on the screen
    @param	y	Position of the label on the screen
    @param	align	Alignment:
    			<br>GFUI_ALIGN_HR_VB	horizontal right, vertical bottom
    			<br>GFUI_ALIGN_HR_VC	horizontal right, vertical center
    			<br>GFUI_ALIGN_HR_VT	horizontal right, vertical top
    			<br>GFUI_ALIGN_HC_VB	horizontal center, vertical bottom
    			<br>GFUI_ALIGN_HC_VB	horizontal center, vertical center
    			<br>GFUI_ALIGN_HC_VB	horizontal center, vertical top
    			<br>GFUI_ALIGN_HL_VB	horizontal left, vertical bottom
    			<br>GFUI_ALIGN_HL_VB	horizontal left, vertical center
    			<br>GFUI_ALIGN_HL_VB	horizontal left, vertical top
    @param	maxlen	Maximum length of the button string (used when the label is changed)
    			<br>0 for the text length.
    @param	fgColor	Pointer on static RGBA color array (0 => default)
    @param	fgFocusColor	Pointer on static RGBA focused color array (0 => fgColor)
    @param	userDataOnFocus	User data given to the onFocus[Lost] call back functions
    @param	onFocus	Call back function called when getting the focus
    @param	onFocusLost	Call back function called when loosing the focus
    @return	label Id
    @see	GfuiSetLabelText
 */
int 
GfuiLabelCreate(void *scr, const char *text, int font, int x, int y, int align, int maxlen,
				const float *fgColor, const float *fgFocusColor,
				void *userDataOnFocus, tfuiCallback onFocus, tfuiCallback onFocusLost)
{
    tGfuiLabel	*label;
    tGfuiObject	*object;
	tGfuiScreen	*screen = (tGfuiScreen*)scr;

    object = (tGfuiObject*)calloc(1, sizeof(tGfuiObject));
    object->widget = GFUI_LABEL;
    object->focusMode = (onFocus || onFocusLost) ? GFUI_FOCUS_MOUSE_MOVE : GFUI_FOCUS_NONE;
    object->visible = 1;
    object->id = screen->curId++;
    
    label = &(object->u.label);
	gfuiLabelInit(label, text, maxlen, x, y, align, 0, font,
				  screen->bgColor.toFloatRGBA(), fgColor, screen->bgColor.toFloatRGBA(), fgFocusColor, 
				  userDataOnFocus, onFocus, onFocusLost);

	const int width = gfuiFont[font]->getWidth((const char *)text);
	const int height = gfuiFont[font]->getHeight() - gfuiFont[font]->getDescender();

	switch(align)
	{
		case GFUI_ALIGN_HL_VB:
			object->xmin = x;
			object->ymin = y;
			break;

		case GFUI_ALIGN_HL_VC:
			object->xmin = x;
			object->ymin = y - height / 2;
			break;

		case GFUI_ALIGN_HL_VT:
			object->xmin = x;
			object->ymin = y;
			break;

		case GFUI_ALIGN_HC_VB:
			object->xmin = x - width / 2;
			object->ymin = y;
			break;

		case GFUI_ALIGN_HC_VC:
			object->xmin = x - width / 2;
			object->ymin = y - height / 2;
			break;

		case GFUI_ALIGN_HC_VT:
			object->xmin = x - width / 2;
			object->ymin = y;
			break;

		case GFUI_ALIGN_HR_VB:
			object->xmin = x - width;
			object->ymin = y;
			break;

		case GFUI_ALIGN_HR_VC:
			object->xmin = x - width;
			object->ymin = y - height / 2;
			break;

		case GFUI_ALIGN_HR_VT:
			object->xmin = x - width;
			object->ymin = y;
			break;
	}

	object->xmax = object->xmin + width;
	object->ymax = object->ymin + height;

    gfuiAddObject(screen, object);

    return object->id;
}

/** Add a Tip (generally associated with a button).
    @param	scr	Screen where to add the label
    @param	text	Text of the label
    @param	maxlen	Maximum length of the button string (used when the label is changed)
    @return	label Id
    @see	GfuiSetLabelText
 */
int
GfuiTipCreate(void *scr, const char *text, int maxlen)
{
    return GfuiLabelCreate(scr, text, NTipFontId, NTipX, NTipY,
						   NTipAlign, maxlen, gfuiColors[GFUI_TIPCOLOR]);
}

/** Change the text of a label.
    @ingroup	gui
    @param	label	Label itself
    @param	obj	Label object
    @param	text	Text of the label
    @warning	The maximum length is set at the label creation
    @see	GfuiAddLabel
 */
void
gfuiLabelSetText(tGfuiLabel *label, const char *text)
{
    if (!text)
		return;

	// Reallocate label->text if maxlen is nul (in case label->text is empty).
	const int prevWidth = label->font->getWidth((const char *)label->text);
 	if (label->maxlen <= 0)
	{
		free(label->text);
		label->maxlen = strlen(text);
		label->text = (char*)calloc(label->maxlen+1, 1);
	}
	
	// Update the text.
   strncpy(label->text, text, label->maxlen);
	
	// Update the text position.
	const int width = label->font->getWidth((const char *)label->text);
	switch(label->align&0xF0)
	{
		case GFUI_ALIGN_HL_VB /* LEFT */:
			// No change.
			break;
		case GFUI_ALIGN_HC_VB /* CENTER */:
			label->x += (prevWidth - width) / 2;
			break;
		case GFUI_ALIGN_HR_VB /* RIGHT */:
			label->x += prevWidth - width;
			break;
	}
}

/** Change the text of a label.
    @ingroup	gui
    @param	scr	Screen of the label
    @param	id	Id of the label
    @param	text	Text of the label
    @warning	The maximum length is set at the label creation
    @see	GfuiAddLabel
 */
void
GfuiLabelSetText(void *scr, int id, const char *text)
{
    tGfuiObject* object = gfuiGetObject(scr, id);
    
    if (object && object->widget == GFUI_LABEL)
	{
		tGfuiLabel* label = &(object->u.label);
		
		// Update the text.
		gfuiLabelSetText(label, text);

		// Update the bounding box.
		object->xmin = label->x;
		object->xmax = object->xmin + label->font->getWidth((const char *)text);
	}
}

/** Change the color of a label.
    @ingroup	gui
    @param	label	Label to modify
    @param	color	an array of 4 floats (RGBA)
    @see	GfuiAddLabel
 */
void
gfuiLabelSetColor(tGfuiLabel *label, const float *color)
{
	label->fgColor = GfuiColor::build(color);
}

/** Change the color of a label object.
    @ingroup	gui
    @param	scr	Screen where to add the label
    @param	id	Id of the label
    @param	color	an array of 4 floats (RGBA)
    @see	GfuiAddLabel
 */
void
GfuiLabelSetColor(void *scr, int id, const float* color)
{
    tGfuiObject* object = gfuiGetObject(scr, id);
    
    if (object && object->widget == GFUI_LABEL)
	{
		gfuiLabelSetColor(&object->u.label, color);
	}
}


/** Actually draw the given label.
    @ingroup	gui
    @param	obj	label to draw
 */
void
gfuiLabelDraw(tGfuiLabel *label, int focus)
{
	// Draw the text, according to the state/focus.
    glColor4fv(focus ? label->fgFocusColor.toFloatRGBA() : label->fgColor.toFloatRGBA());
    gfuiDrawString(label->x, label->y, label->font, label->text);
}

/** Actually draw the given label object.
    @ingroup	gui
    @param	obj	label object to draw
 */
void
gfuiDrawLabel(tGfuiObject *obj)
{
    tGfuiLabel *label = &(obj->u.label);

	// Draw the background if visible.
    if (label->bgColor.alpha)
	{
		glColor4fv(obj->focus ? label->bgFocusColor.toFloatRGBA() : label->bgColor.toFloatRGBA());
		glBegin(GL_QUADS);
		glVertex2i(obj->xmin, obj->ymin);
		glVertex2i(obj->xmin, obj->ymax);
		glVertex2i(obj->xmax, obj->ymax);
		glVertex2i(obj->xmax, obj->ymin);
		glEnd();
    }

	// Draw the label text itself.
    gfuiLabelDraw(label, obj->focus);
}

void
gfuiReleaseLabel(tGfuiObject *obj)
{
    tGfuiLabel	*label;

    label = &(obj->u.label);

	freez(label->userDataOnFocus);
    free(label->text);
    free(obj);
}
