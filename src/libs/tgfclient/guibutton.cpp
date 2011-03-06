/***************************************************************************
                             guibutton.cpp                             
                             -------------------                                         
    created              : Fri Aug 13 22:18:21 CEST 1999
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
    		GUI Buttons Management.
    @author	<a href=mailto:torcs@free.fr>Eric Espie</a>
    @version	$Id$
    @ingroup	gui
*/

#ifdef WIN32
#include <windows.h>
#endif

#include "gui.h"


void
gfuiButtonInit(void)
{

}

/** Initialize a graphical button.
    @ingroup	gui
    @param	button		The button to initialize
    @param	disabled	filename of the image when the button is disabled
    @param	enabled		filename of the image when the button is enabled
    @param	focused		filename of the image when the button is focused
    @param	pushed		filename of the image when the button is pushed
    @param	width		Width on the screen (0 = image width)
    @param	height		Height on the screen (0 = image height)
    @param	mouse		Mouse behavior:
                        <br>GFUI_MOUSE_UP Action performed when the mouse right button is released
                        <br>GFUI_MOUSE_DOWN Action performed when the mouse right button is pushed
    @param	userDataOnPush	Parameter to the Push callback
    @param	onPush		Push callback function
    @param	userDataOnFocus	Parameter to the Focus (and lost) callback
    @param	onFocus		Focus callback function
    @param	onFocusLost	Focus Lost callback function
    @return	Button Id
		<br>-1 Error
 */
void
gfuiGrButtonInit(tGfuiGrButton* button, const char *disabled, const char *enabled,
				 const char *focused, const char *pushed,
				 int x, int y, int align, int width, int height, int mouse,
				 void *userDataOnPush, tfuiCallback onPush, 
				 void *userDataOnFocus, tfuiCallback onFocus, tfuiCallback onFocusLost)
{
    button->state = GFUI_BTN_RELEASED;

	int w, h;
    button->disabled = GfTexReadTexture(disabled, &w, &h);
    button->enabled = GfTexReadTexture(enabled, &w, &h);
    button->focused = GfTexReadTexture(focused, &w, &h);
    button->pushed = GfTexReadTexture(pushed, &w, &h);
	
	// Warning: All the images are supposed to be the same size.
	button->width = width <= 0 ? w : width;
	button->height = height <= 0 ? h : height;

	button->align = align;

    switch(align)
	{
		case GFUI_ALIGN_HL_VB:
			button->x = x;
			button->y = y;
			break;

		case GFUI_ALIGN_HL_VC:
			button->x = x;
			button->y = y - button->height / 2;
			break;

		case GFUI_ALIGN_HL_VT:
			button->x = x;
			button->y = y - button->height;
			break;

		case GFUI_ALIGN_HC_VB:
			button->x = x - button->width / 2;
			button->y = y;
			break;

		case GFUI_ALIGN_HC_VC:
			button->x = x - button->width / 2;
			button->y = y - button->height / 2;
			break;

		case GFUI_ALIGN_HC_VT:
			button->x = x - button->width / 2;
			button->y = y - button->height;
			break;

		case GFUI_ALIGN_HR_VB:
			button->x = x - button->width;
			button->y = y;
			break;

		case GFUI_ALIGN_HR_VC:
			button->x = x - button->width;
			button->y = y - button->height / 2;
			break;

		case GFUI_ALIGN_HR_VT:
			button->x = x - button->width;
			button->y = y - button->height;
			break;
	}

    button->buttonType = GFUI_BTN_PUSH;
    button->mouseBehaviour = mouse;

    button->userDataOnPush = userDataOnPush;
    button->onPush = onPush;
    button->userDataOnFocus = userDataOnFocus;
    button->onFocus = onFocus;
    button->onFocusLost = onFocusLost;
}

/** Add a graphical button to a screen.
    @ingroup	gui
    @param	scr		Screen
    @param	disabled	filename of the image when the button is disabled
    @param	enabled		filename of the image when the button is enabled
    @param	focused		filename of the image when the button is focused
    @param	pushed		filename of the image when the button is pushed
    @param	x		X position on screen (0 = left)
    @param	y		Y position on screen (0 = bottom)
    @param	width		Width on the screen (0 = image width)
    @param	height		Height on the screen (0 = image height)
    @param	align		Button alignment
    @param	mouse		Mouse behavior:
                        <br>GFUI_MOUSE_UP Action performed when the mouse right button is released
                        <br>GFUI_MOUSE_DOWN Action performed when the mouse right button is pushed
    @param	userDataOnPush	Parameter to the Push callback
    @param	onPush		Push callback function
    @param	userDataOnFocus	Parameter to the Focus (and lost) callback
    @param	onFocus		Focus callback function
    @param	onFocusLost	Focus Lost callback function
    @return	Button Id
		<br>-1 Error
 */

int
GfuiGrButtonCreate(void *scr, const char *disabled, const char *enabled,
				   const char *focused, const char *pushed,
				   int x, int y, int width, int height, int align, int mouse,
				   void *userDataOnPush, tfuiCallback onPush, 
				   void *userDataOnFocus, tfuiCallback onFocus, tfuiCallback onFocusLost)
{
    tGfuiGrButton	*button;
    tGfuiObject		*object;
    tGfuiScreen		*screen = (tGfuiScreen*)scr;
    int			w, h;
    
    object = (tGfuiObject*)calloc(1, sizeof(tGfuiObject));
    object->widget = GFUI_GRBUTTON;
    object->focusMode = GFUI_FOCUS_MOUSE_MOVE;
    object->id = screen->curId++;
    object->visible = 1;
    
    button = &(object->u.grbutton);
	gfuiGrButtonInit(button, disabled, enabled, focused, pushed,
					 x, y, align, width, height, mouse,
					 userDataOnPush, onPush, userDataOnFocus, onFocus, onFocusLost);

    switch (align)
	{
		case GFUI_ALIGN_HR_VB:
			object->xmin = x - button->width;
			object->xmax = x;
			object->ymin = y;
			object->ymax = y + button->height;
			break;
		case GFUI_ALIGN_HR_VC:
			object->xmin = x - button->width;
			object->xmax = x;
			object->ymin = y - button->height / 2;
			object->ymax = y + button->height / 2;
			break;
		case GFUI_ALIGN_HR_VT:
			object->xmin = x - button->width;
			object->xmax = x;
			object->ymin = y - button->height;
			object->ymax = y;
			break;
		case GFUI_ALIGN_HC_VB:
			object->xmin = x - button->width / 2;
			object->xmax = x + button->width / 2;
			object->ymin = y;
			object->ymax = y + button->height;
			break;
		case GFUI_ALIGN_HC_VC:
			object->xmin = x - button->width / 2;
			object->xmax = x + button->width / 2;
			object->ymin = y - button->height / 2;
			object->ymax = y + button->height / 2;
			break;
		case GFUI_ALIGN_HC_VT:
			object->xmin = x - button->width / 2;
			object->xmax = x + button->width / 2;
			object->ymin = y - button->height;
			object->ymax = y;
			break;
		case GFUI_ALIGN_HL_VB:
			object->xmin = x;
			object->xmax = x + button->width;
			object->ymin = y;
			object->ymax = y + button->height;
			break;
		case GFUI_ALIGN_HL_VC:
			object->xmin = x;
			object->xmax = x + button->width;
			object->ymin = y - button->height / 2;
			object->ymax = y + button->height / 2;
			break;
		case GFUI_ALIGN_HL_VT:
			object->xmin = x;
			object->xmax = x + button->width;
			object->ymin = y - button->height;
			object->ymax = y;
			break;
		default:
			break;
    }

    gfuiAddObject(screen, object);
	
    return object->id;
}

/** Add a state button to a screen.
    @ingroup	gui
    @param	scr		Screen
    @param	text		Button label
    @param	font		Font id
    @param	x		X position on screen
    @param	y		Y position on screen (0 = bottom)
    @param	width		width of the button (0 = text size)
    @param	align		Button alignment:
    			<br>GFUI_ALIGN_HR_VB	horizontal right, vertical bottom
    			<br>GFUI_ALIGN_HR_VC	horizontal right, vertical center
    			<br>GFUI_ALIGN_HR_VT	horizontal right, vertical top
    			<br>GFUI_ALIGN_HC_VB	horizontal center, vertical bottom
    			<br>GFUI_ALIGN_HC_VC	horizontal center, vertical center
    			<br>GFUI_ALIGN_HC_VT	horizontal center, vertical top
    			<br>GFUI_ALIGN_HL_VB	horizontal left, vertical bottom
    			<br>GFUI_ALIGN_HL_VC	horizontal left, vertical center
    			<br>GFUI_ALIGN_HL_VT	horizontal left, vertical top
    @param	mouse		Mouse behavior:
    				<br>GFUI_MOUSE_UP Action performed when the mouse right button is released
				<br>GFUI_MOUSE_DOWN Action performed when the mouse right button is pushed
    @param	userDataOnPush	Parameter to the Push callback
    @param	onPush		Push callback function
    @param	userDataOnFocus	Parameter to the Focus (and lost) callback
    @param	onFocus		Focus callback function
    @param	onFocusLost	Focus Lost callback function
    @return	Button Id
		<br>-1 Error
 */
int
GfuiButtonStateCreate(void *scr, const char *text, int font, int x, int y, int width, int align, int mouse,
		      void *userDataOnPush, tfuiCallback onPush, 
		      void *userDataOnFocus, tfuiCallback onFocus, tfuiCallback onFocusLost)
{
	int id = GfuiButtonCreate(scr, text, font, x, y, width, align, mouse, userDataOnPush, onPush, userDataOnFocus,
	onFocus, onFocusLost);

	tGfuiScreen	*screen = (tGfuiScreen*)scr;
	tGfuiObject *curObject = screen->objects;
	if (curObject) {
		do {
			curObject = curObject->next;
			if (curObject->id == id) {
				if (curObject->widget == GFUI_BUTTON) {
					tGfuiButton	*button = &(curObject->u.button);
					button->buttonType = GFUI_BTN_STATE;
				}
			return id;
			}
		} while (curObject != screen->objects);
	}
	return id;
}//GfuiButtonStateCreate


/** Add a button to a screen.
    @ingroup	gui
    @param	scr		Screen
    @param	text		Button label
    @param	font		Font id
    @param	x		X position on screen
    @param	y		Y position on screen (0 = bottom)
    @param	width		width of the button (0 = text size)
    @param	align		Button alignment:
    			<br>GFUI_ALIGN_HR_VB	horizontal right, vertical bottom
    			<br>GFUI_ALIGN_HR_VC	horizontal right, vertical center
    			<br>GFUI_ALIGN_HR_VT	horizontal right, vertical top
    			<br>GFUI_ALIGN_HC_VB	horizontal center, vertical bottom
    			<br>GFUI_ALIGN_HC_VC	horizontal center, vertical center
    			<br>GFUI_ALIGN_HC_VT	horizontal center, vertical top
    			<br>GFUI_ALIGN_HL_VB	horizontal left, vertical bottom
    			<br>GFUI_ALIGN_HL_VC	horizontal left, vertical center
    			<br>GFUI_ALIGN_HL_VT	horizontal left, vertical top
    @param	mouse		Mouse behavior:
    				<br>GFUI_MOUSE_UP Action performed when the mouse right button is released
				<br>GFUI_MOUSE_DOWN Action performed when the mouse right button is pushed
    @param	userDataOnPush	Parameter to the Push callback
    @param	onPush		Push callback function
    @param	userDataOnFocus	Parameter to the Focus (and lost) callback
    @param	onFocus		Focus callback function
    @param	onFocusLost	Focus Lost callback function
    @return	Button Id
		<br>-1 Error
 */
int
GfuiButtonCreate(void *scr, const char *text, int font, int x, int y, int width, int align,
				 int mouse, void *userDataOnPush, tfuiCallback onPush, 
				 void *userDataOnFocus, tfuiCallback onFocus, tfuiCallback onFocusLost)
{
    tGfuiButton	*button;
    tGfuiLabel	*label;
    tGfuiObject	*object;
    tGfuiScreen	*screen = (tGfuiScreen*)scr;

    object = (tGfuiObject*)calloc(1, sizeof(tGfuiObject));
    object->widget = GFUI_BUTTON;
    object->focusMode = GFUI_FOCUS_MOUSE_MOVE;
    object->id = screen->curId++;
    object->visible = 1;
    
    button = &(object->u.button);
    button->state = GFUI_BTN_RELEASED;
    button->userDataOnPush = userDataOnPush;
    button->onPush = onPush;
    button->userDataOnFocus = userDataOnFocus;
    button->onFocus = onFocus;
    button->onFocusLost = onFocusLost;
    button->mouseBehaviour = mouse;
    button->buttonType = GFUI_BTN_PUSH;
    button->bShowBox = true;

    button->disabled = 0;
    button->enabled = 0;
    button->focused = 0;
    button->pushed = 0;

    button->bgColor[0] = GetColor(&(GfuiColor[GFUI_BGBTNDISABLED][0]));
    button->bgColor[1] = GetColor(&(GfuiColor[GFUI_BGBTNENABLED][0]));
    button->bgColor[2] = GetColor(&(GfuiColor[GFUI_BGBTNCLICK][0]));
    button->bgFocusColor[0] = GetColor(&(GfuiColor[GFUI_BGBTNDISABLED][0]));
    button->bgFocusColor[1] = GetColor(&(GfuiColor[GFUI_BGBTNFOCUS][0]));
    button->bgFocusColor[2] = GetColor(&(GfuiColor[GFUI_BGBTNCLICK][0]));
    button->fgColor[0] = GetColor(&(GfuiColor[GFUI_BTNDISABLED][0]));
    button->fgColor[1] = GetColor(&(GfuiColor[GFUI_BTNENABLED][0]));
    button->fgColor[2] = GetColor(&(GfuiColor[GFUI_BTNCLICK][0]));
    button->fgFocusColor[0] = GetColor(&(GfuiColor[GFUI_BTNDISABLED][0]));
    button->fgFocusColor[1] = GetColor(&(GfuiColor[GFUI_BTNFOCUS][0]));
    button->fgFocusColor[2] = GetColor(&(GfuiColor[GFUI_BTNCLICK][0]));

	gfuiLabelInit(&button->label, text, 100, x, y - gfuiFont[font]->getDescender(), align, width,
				  font, 0, 0, 0, 0, 0, 0, 0);
	
    if (width <= 0)
		width = gfuiFont[font]->getWidth((const char *)text);

    switch(align&0xF0)
	{
		case GFUI_ALIGN_HL_VB /* LEFT */:
			object->xmin = x;
			object->ymin = y;
			object->xmax = x + width;
			object->ymax = y + gfuiFont[font]->getHeight() - gfuiFont[font]->getDescender();
			break;
		case GFUI_ALIGN_HC_VB /* CENTER */:
			object->xmin = x - width / 2;
			object->ymin = y;
			object->xmax = x + width / 2;
			object->ymax = y + gfuiFont[font]->getHeight() - gfuiFont[font]->getDescender();
			break;
		case GFUI_ALIGN_HR_VB /* RIGHT */:
			object->xmin = x - width;
			object->ymin = y;
			object->xmax = x;
			object->ymax = y + gfuiFont[font]->getHeight() - gfuiFont[font]->getDescender();
			break;
    }
	static const int HORIZ_MARGIN = 10;
    object->xmin -= HORIZ_MARGIN;
    object->xmax += HORIZ_MARGIN;

    gfuiAddObject(screen, object);
	
    return object->id;
}

/** Change the label of a button.
    @ingroup	gui
    @param	scr	Screen
    @param	id	Button Id
    @param	text	New label of the button
 */
void
GfuiButtonSetText(void *scr, int id, const char *text)
{
	tGfuiObject *object = gfuiGetObject(scr, id);
	if (object && object->widget == GFUI_BUTTON)
	{
		gfuiLabelSetText(&(object->u.button.label), text);
	}
}//GfuiButtonSetText


/** Show/hide the framing box around a button.
    @ingroup	gui
    @param	scr	Screen
    @param	id	Button Id
    @param	bShow	True-Show frame, False-Dont show
 */
void
GfuiButtonShowBox(void *scr, int id, bool bShow)
{
	tGfuiObject *object = gfuiGetObject(scr, id);
	if (object && object->widget == GFUI_BUTTON)
	{
		object->u.button.bShowBox = bShow;
	}
}//GfuiButtonShowBox


/** Change the colour of a button.
    @ingroup	gui
    @param	scr	Screen
    @param	id	Button Id
    @param	color	New colour of the button
 */
void
GfuiButtonSetColor(void *scr, int id, Color color)
{
	tGfuiObject *object = gfuiGetObject(scr, id);
	if (object && object->widget == GFUI_BUTTON)
	{
		object->u.button.fgColor[1] = color;
	}
}//GfuiButtonSetColor


/** Define button images for different states.
    @ingroup	gui
    @param	scr	Screen
    @param	id	Button Id
    @param	x		X position on screen
    @param	y		Y position on screen (0 = bottom)
    @param	w		width of the button image
    @param	h		height of the button image
    @param	disableFile	file that holds the disabled button image version
    @param	enableFile	file that holds the enabled button image version
    @param	focusedFile	file that holds the focused button image version
    @param	pushedFile	file that holds the pushed button image version
*/
void
GfuiButtonSetImage(void *scr, int id, int x, int y, int w, int h,
	const char *disableFile, const char *enableFile,
	const char *focusedFile, const char *pushedFile)
{
	GLuint disable = 0;
	GLuint enable = 0;
	GLuint focused = 0;
	GLuint pushed = 0;

	if (strlen(disableFile) != 0)
		disable = GfTexReadTexture(disableFile);
	if (strlen(enableFile) != 0)
		enable = GfTexReadTexture(enableFile);
	if (strlen(focusedFile) != 0)
		focused = GfTexReadTexture(focusedFile);
	if (strlen(pushedFile) != 0)
		pushed = GfTexReadTexture(pushedFile);

	tGfuiObject *object = gfuiGetObject(scr, id);
	if (object && object->widget == GFUI_BUTTON)
	{
		object->u.button.disabled = disable;
		object->u.button.enabled = enable;
		object->u.button.focused = focused;
		object->u.button.pushed = pushed;
		object->u.button.imgX = x;
		object->u.button.imgY = y;
		object->u.button.imgWidth = w;
		object->u.button.imgHeight = h;
	}//if curObject
}//GfuiButtonSetImage


/** Change the focused colour of a button.
    @ingroup	gui
    @param	scr	Screen
    @param	id	Button Id
    @param	focuscolor	New focus colour of the button
 */
void
GfuiButtonSetFocusColor(void *scr, int id, Color focuscolor)
{
	tGfuiObject *object = gfuiGetObject(scr, id);
	if (object && object->widget == GFUI_BUTTON)
	{
		object->u.button.fgFocusColor[1] = focuscolor;
	}
}//GfuiButtonSetFocusColor


/** Change the pushed colour of a button.
    @ingroup	gui
    @param	scr	Screen
    @param	id	Button Id
    @param	pushcolor	New pushed colour of the button
 */
void
GfuiButtonSetPushedColor(void *scr, int id, Color pushcolor)
{
	tGfuiObject *object = gfuiGetObject(scr, id);
	if (object && object->widget == GFUI_BUTTON)
	{
		object->u.button.fgFocusColor[2] = pushcolor;
	}    
}//GfuiButtonSetPushedColor


/** Get the Id of the button focused in the current screen.
    @ingroup	gui
    @return	Button Id
		<br>-1 if no button or no screen or the focus is not on a button
 */
int
GfuiButtonGetFocused(void)
{
	if (GfuiScreen) {
		tGfuiObject *curObject = GfuiScreen->objects;
		if (curObject) {
			do {
				curObject = curObject->next;
				if (curObject->focus) {
					if (curObject->widget == GFUI_BUTTON) {
						return curObject->id;
					}
				return -1;
				}
			} while (curObject != GfuiScreen->objects);
		}
	}
	return -1;
}//GfuiButtonGetFocused


/** Actually draw the given button object.
    @ingroup	gui
    @param	obj	button object to draw
 */
void
gfuiDrawButton(tGfuiObject *obj)
{
	Color fgColor;
	Color bgColor;

	// Determine the fore/background colors, according to the state/focus.
	tGfuiButton *button = &(obj->u.button);
	if (obj->state == GFUI_DISABLE) {
		button->state = GFUI_BTN_DISABLE;
		}
	if (obj->focus) {
		fgColor = button->fgFocusColor[button->state];
		bgColor = button->bgFocusColor[button->state];
	} else {
		fgColor = button->fgColor[button->state];
		bgColor = button->bgColor[button->state];
	}//if obj->focus

	// Draw the bouding box / background if specified and visible.
	if (bgColor.alpha && button->bShowBox) {
		glColor4fv(bgColor.GetPtr());
		glBegin(GL_QUADS);
		glVertex2i(obj->xmin, obj->ymin);
		glVertex2i(obj->xmin, obj->ymax);
		glVertex2i(obj->xmax, obj->ymax);
		glVertex2i(obj->xmax, obj->ymin);
		glEnd();
		glColor4fv(fgColor.GetPtr());
		glBegin(GL_LINE_STRIP);
		glVertex2i(obj->xmin, obj->ymin);
		glVertex2i(obj->xmin, obj->ymax);
		glVertex2i(obj->xmax, obj->ymax);
		glVertex2i(obj->xmax, obj->ymin);
		glVertex2i(obj->xmin, obj->ymin);
		glEnd();	
	}//if (bgColor && button->bShowBox)

	// Draw the image if any, according to the state/focus.
	GLuint img = 0;
	if (obj->state == GFUI_DISABLE) {
		img = button->disabled;
	} else if (button->state == GFUI_BTN_PUSHED) {
		img = button->pushed;
	} else if (obj->focus) {
		img = button->focused;
	} else {
		img = button->enabled;
	}

	if (img) {
		const int x1 = obj->xmin + button->imgX;
		const int x2 = x1 + button->imgWidth;
		const int y1 = obj->ymin + button->imgY;
		const int y2 = y1 + button->imgHeight;

		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		glColor3f(1.0,1.0,1.0); //set color to mix with image

		glEnable(GL_BLEND);
		glEnable(GL_TEXTURE_2D);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glBindTexture(GL_TEXTURE_2D, img);
		glBegin(GL_QUADS);

		glTexCoord2f (0.0, 0.0);
		glVertex2i(x1,y1);

		glTexCoord2f (0.0, 1.0);
		glVertex2i(x1,y2);

		glTexCoord2f (1.0, 1.0);
		glVertex2i(x2,y2);

		glTexCoord2f (1.0, 0.0);
		glVertex2i(x2,y1);

		glEnd();
		glDisable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	// Draw the label.
	tGfuiLabel *label = &(button->label);
	glColor4fv(fgColor.GetPtr());
	gfuiPrintString(label->x, label->y, label->font, label->text);
}//gfuiDrawButton


/** Actually draw the given graphical button.
    @ingroup	gui
    @param	obj	graphical button to draw
 */
void
gfuiGrButtonDraw(tGfuiGrButton *button, int state, int focus)
{
	// Determine the image to draw, according to the state/focus.
	GLuint img;
	if (state == GFUI_DISABLE) {
		img = button->disabled;
	} else if (button->state == GFUI_BTN_PUSHED) {
		img = button->pushed;
	} else if (focus) {
		img = button->focused;
	} else {
		img = button->enabled;
	}

	// Draw the image.
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glColor3f(1.0, 1.0, 1.0); //set color to mix with image

	glEnable(GL_BLEND);
	glEnable(GL_TEXTURE_2D);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	glBindTexture(GL_TEXTURE_2D, img);
	glBegin(GL_QUADS);

	glTexCoord2f(0.0, 0.0);
	glVertex2i(button->x, button->y);
	
	glTexCoord2f(0.0, 1.0);
	glVertex2i(button->x, button->y + button->height);
	
	glTexCoord2f(1.0, 1.0);
	glVertex2i(button->x + button->width, button->y + button->height);
	
	glTexCoord2f(1.0, 0.0);
	glVertex2i(button->x + button->width, button->y);

	glEnd();
	glDisable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);

/* What is this supposed to do ?
	int sw, sh, vw, vh;
	GfScrGetSize(&sw, &sh, &vw, &vh);
	glRasterPos2i(obj->xmin, obj->ymin);
	glPixelZoom((float)vw / (float)GfuiScreen->width, (float)vh / (float)GfuiScreen->height);
	glDrawPixels(button->width, button->height, GL_RGBA, GL_UNSIGNED_BYTE, img);
*/
}

/** Actually draw the given graphical button object.
    @ingroup	gui
    @param	obj	button object to draw
 */
void
gfuiDrawGrButton(tGfuiObject *obj)
{
	gfuiGrButtonDraw(&obj->u.grbutton, obj->state, obj->focus);
}//gfuiDrawGrButton

/** Check if the mouse is in the given graphical button
    @ingroup	gui
    @param	button	graphical button object to check
 */
bool
gfuiGrButtonMouseIn(tGfuiGrButton *button)
{
    return
		GfuiMouse.X >= button->x && GfuiMouse.X <= button->x + button->width
		&& GfuiMouse.Y >= button->y && GfuiMouse.Y <= button->y + button->height;
}

/** Handles the graphical button action.
    @ingroup	gui
    @param	action	action
 */
void
gfuiGrButtonAction(int action) {

	if (GfuiScreen->hasFocus->state == GFUI_DISABLE) 
		return;

	tGfuiGrButton	*button = &(GfuiScreen->hasFocus->u.grbutton);

	switch (button->buttonType) {
		case GFUI_BTN_PUSH:
			if (action == 2) { /* enter key */
				if (button->onPush) {
					button->onPush(button->userDataOnPush);
				}
			} else if (action == 1) { /* mouse up */
				if (button->state != GFUI_BTN_RELEASED) {
					button->state = GFUI_BTN_RELEASED;
					if (button->mouseBehaviour == GFUI_MOUSE_UP) {
						if (button->onPush) {
							button->onPush(button->userDataOnPush);
						}
					}
				}
			} else { /* mouse down */
				if (button->state != GFUI_BTN_PUSHED) {
					button->state = GFUI_BTN_PUSHED;
					if (button->mouseBehaviour == GFUI_MOUSE_DOWN) {
						if (button->onPush) {
							button->onPush(button->userDataOnPush);
						}
					}
				}
			}
		break;	//GFUI_BTN_PUSH

		case GFUI_BTN_STATE:
			if (action == 2) { /* enter key */
				if (button->state == GFUI_BTN_RELEASED) {
					button->state = GFUI_BTN_PUSHED;
					if (button->onPush) {
						button->onPush(button->userDataOnPush);
					}
				} else {
					button->state = GFUI_BTN_RELEASED;
				}
			} else if (action == 1) { /* mouse up */
				if (button->mouseBehaviour == GFUI_MOUSE_UP) {
					if (button->state == GFUI_BTN_RELEASED) {
						button->state = GFUI_BTN_PUSHED;
						if (button->onPush) {
							button->onPush(button->userDataOnPush);
						}
					} else {
						button->state = GFUI_BTN_RELEASED;
					}
				}
			} else { /* mouse down */
				if (button->mouseBehaviour == GFUI_MOUSE_DOWN) {
					if (button->state == GFUI_BTN_RELEASED) {
						button->state = GFUI_BTN_PUSHED;
						if (button->onPush) {
							button->onPush(button->userDataOnPush);
						}
					} else {
						button->state = GFUI_BTN_RELEASED;
					}
				}
			}
		break;	//GFUI_BTN_STATE
	}//switch
}//guifGrButtonAction


/** Handles the button action.
    @ingroup	gui
    @param	action	action
 */
void
gfuiButtonAction(int action)
{
	if (GfuiScreen->hasFocus->state == GFUI_DISABLE) 
	{
		return;
    }

	tGfuiButton	*button = &(GfuiScreen->hasFocus->u.button);

	switch (button->buttonType) {
		case GFUI_BTN_PUSH:
			if (action == 2) { /* enter key */
				if (button->onPush) {
					button->onPush(button->userDataOnPush);
				}
			} else if (action == 1) { /* mouse up */
				button->state = GFUI_BTN_RELEASED;
				if (button->mouseBehaviour == GFUI_MOUSE_UP) {
					if (button->onPush) {
						button->onPush(button->userDataOnPush);
					}
				}
			} else { /* mouse down */
				button->state = GFUI_BTN_PUSHED;
				if (button->mouseBehaviour == GFUI_MOUSE_DOWN) {
					if (button->onPush) {
						button->onPush(button->userDataOnPush);
					}
				}
			}
		break;	//GFUI_BTN_PUSH

		case GFUI_BTN_STATE:
			if (action == 2) { /* enter key */
				if (button->state == GFUI_BTN_RELEASED) {
					button->state = GFUI_BTN_PUSHED;
					if (button->onPush) {
						button->onPush(button->userDataOnPush);
					}
				} else {
					button->state = GFUI_BTN_RELEASED;
				}
			} else if (action == 1) { /* mouse up */
				if (button->mouseBehaviour == GFUI_MOUSE_UP) {
					if (button->state == GFUI_BTN_RELEASED) {
						button->state = GFUI_BTN_PUSHED;
						if (button->onPush) {
							button->onPush(button->userDataOnPush);
						}
					} else {
						button->state = GFUI_BTN_RELEASED;
					}
				}
			} else { /* mouse down */
				if (button->mouseBehaviour == GFUI_MOUSE_DOWN) {
					if (button->state == GFUI_BTN_RELEASED) {
						button->state = GFUI_BTN_PUSHED;
						if (button->onPush) {
							button->onPush(button->userDataOnPush);
						}
					} else {
						button->state = GFUI_BTN_RELEASED;
					}
				}
			}
		break;	//GFUI_BTN_STATE
	}//switch
}//gfuiButtonAction


/** Releases all resources connected with a button.
    @ingroup	gui
    @param	obj	button object to release
 */
void
gfuiReleaseButton(tGfuiObject *obj)
{
	tGfuiButton *button = &(obj->u.button);;

	GfTexFreeTexture(button->disabled);
	GfTexFreeTexture(button->enabled);
	GfTexFreeTexture(button->focused);
	GfTexFreeTexture(button->pushed);

	tGfuiLabel *label = &(button->label);

	freez(button->userDataOnFocus);
	free(label->text);
	free(obj);
}//gfuiReleaseButton


/** Releases all resources connected with a graphical button.
    @ingroup	gui
    @param	obj	button object to release
 */
void
gfuiReleaseGrButton(tGfuiObject *obj)
{
	tGfuiGrButton	*button = &(obj->u.grbutton);

	GfTexFreeTexture(button->disabled);
	GfTexFreeTexture(button->enabled);
	GfTexFreeTexture(button->focused);
	GfTexFreeTexture(button->pushed);

	free(obj);
}//gfuiReleaseGrButton
