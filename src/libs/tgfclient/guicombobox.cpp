/***************************************************************************

    file                 : guiedit.cpp
	created              : Nov 23 2009
    copyright            : (C) 2000 by Brian Gavin

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <cstdlib>
#include <cstring>

#include <SDL/SDL.h>

#include "gui.h"


void
gfuiComboboxInit(void)
{
}

static void
gfuiLeftArrow(void *idv)
{
    tGfuiObject		*object;
    tGfuiCombobox	*combobox;

    object = gfuiGetObject(GfuiScreen, (long)idv);
    if (!object)
		return;

	combobox = &(object->u.combobox);

	if (combobox->pInfo->vecChoices.empty())
		return;
	
	if (combobox->pInfo->nPos > 0)
		combobox->pInfo->nPos--;
	else
		combobox->pInfo->nPos = combobox->pInfo->vecChoices.size() - 1;

	gfuiLabelSetText(&combobox->label,
					 combobox->pInfo->vecChoices[combobox->pInfo->nPos].c_str());
	
	if (combobox->onChange)
		combobox->onChange(combobox->pInfo);
}

static void
gfuiRightArrow(void *idv)
{
    tGfuiObject		*object;
    tGfuiCombobox	*combobox;

    object = gfuiGetObject(GfuiScreen, (long)idv);
    if (!object)
		return;

	combobox = &(object->u.combobox);
	
	if (combobox->pInfo->vecChoices.empty())
		return;
	
	if (combobox->pInfo->nPos < combobox->pInfo->vecChoices.size() - 1)
		combobox->pInfo->nPos++;
	else
		combobox->pInfo->nPos = 0;

	gfuiLabelSetText(&combobox->label,
					 combobox->pInfo->vecChoices[combobox->pInfo->nPos].c_str());
	
	if (combobox->onChange)
		combobox->onChange(combobox->pInfo);
}

int
GfuiComboboxCreate(void *scr, int font, int x, int y, int width,
				   int align, int style, const char *pszText,
				   const float *fgColor, const float *fgFocusColor,
				   void *userData, tfuiComboboxCallback onChange, 
				   void *userDataOnFocus, tfuiCallback onFocus, tfuiCallback onFocusLost)
{
    tGfuiCombobox	*combobox;
    tGfuiObject		*object;
    tGfuiScreen		*screen = (tGfuiScreen*)scr;
  
    object = (tGfuiObject*)calloc(1, sizeof(tGfuiObject));
	object->widget = GFUI_COMBOBOX;
	object->focusMode = GFUI_FOCUS_MOUSE_MOVE;
    object->id = screen->curId++;
    object->visible = 1;

	combobox = &(object->u.combobox);
	
    combobox->userDataOnFocus = userDataOnFocus;
    combobox->onFocus = onFocus;
    combobox->onFocusLost = onFocusLost;
	combobox->onChange = onChange;
	
    combobox->pInfo = new tComboBoxInfo;
	combobox->pInfo->nPos = 0;
	combobox->pInfo->userData = userData;
	combobox->scr = scr;

	const int height = gfuiFont[font]->getHeight() - gfuiFont[font]->getDescender();
	
    switch (align) {
		case GFUI_ALIGN_HR_VB:
			object->xmin = x - width;
			object->xmax = x;
			object->ymin = y;
			object->ymax = y + height;
			break;
		case GFUI_ALIGN_HR_VC:
			object->xmin = x - width;
			object->xmax = x;
			object->ymin = y - height / 2;
			object->ymax = y + height / 2;
			break;
		case GFUI_ALIGN_HR_VT:
			object->xmin = x - width;
			object->xmax = x;
			object->ymin = y - height;
			object->ymax = y;
			break;
		case GFUI_ALIGN_HC_VB:
			object->xmin = x - width / 2;
			object->xmax = x + width / 2;
			object->ymin = y;
			object->ymax = y + height;
			break;
		case GFUI_ALIGN_HC_VC:
			object->xmin = x - width / 2;
			object->xmax = x + width / 2;
			object->ymin = y - height / 2;
			object->ymax = y + height / 2;
			break;
		case GFUI_ALIGN_HC_VT:
			object->xmin = x - width / 2;
			object->xmax = x + width / 2;
			object->ymin = y - height;
			object->ymax = y;
			break;
		case GFUI_ALIGN_HL_VB:
			object->xmin = x;
			object->xmax = x + width;
			object->ymin = y;
			object->ymax = y + height;
			break;
		case GFUI_ALIGN_HL_VC:
			object->xmin = x;
			object->xmax = x + width;
			object->ymin = y - height / 2;
			object->ymax = y + height / 2;
			break;
		case GFUI_ALIGN_HL_VT:
			object->xmin = x;
			object->xmax = x + width;
			object->ymin = y - height;
			object->ymax = y;
			break;
		default:
			break;
    }

	// Initialize the label child.
	int xm = object->xmin + (object->xmax - object->xmin) / 2;
	int ym = object->ymin + (object->ymax - object->ymin) / 2;

	gfuiLabelInit(&combobox->label, pszText, 100, xm, ym, GFUI_ALIGN_HC_VC, 0,
				  font, 0, fgColor, 0, fgFocusColor, 0, 0, 0);

	// Initialize the left and right arrow button children.
	// TODO: Make graphic properties XML-customizable (images, ...)
	gfuiGrButtonInit(&combobox->leftButton,
					 "data/img/arrow-left-disabled.png", "data/img/arrow-left.png",
					 "data/img/arrow-left-focused.png", "data/img/arrow-left-pushed.png",
					 object->xmin, ym, GFUI_ALIGN_HL_VC, 0, 0, GFUI_MOUSE_UP,
					 (void*)(object->id), gfuiLeftArrow, 0, 0, 0);
	gfuiGrButtonInit(&combobox->rightButton,
					 "data/img/arrow-right-disabled.png", "data/img/arrow-right.png",
					 "data/img/arrow-right-focused.png", "data/img/arrow-right-pushed.png",
					 object->xmax, ym, GFUI_ALIGN_HR_VC, 0, 0, GFUI_MOUSE_UP,
					 (void*)(object->id), gfuiRightArrow, 0, 0, 0);

	// Add the combo control to the display list.
    gfuiAddObject(screen, object);
	
    return object->id;
}

void
gfuiDrawCombobox(tGfuiObject *obj)
{
	gfuiLabelDraw(&obj->u.combobox.label, obj->focus);
	gfuiGrButtonDraw(&obj->u.combobox.leftButton, obj->state, obj->focus);
	gfuiGrButtonDraw(&obj->u.combobox.rightButton, obj->state, obj->focus);
}

static tGfuiCombobox*
gfuiGetCombobox(void *scr, int id)
{
    tGfuiObject* object = gfuiGetObject(scr, id);

    if (object && object->widget == GFUI_COMBOBOX)
		return &(object->u.combobox);

	return 0;
}
				  
unsigned int
GfuiComboboxAddText(void *scr, int id, const char *text)
{
	unsigned int index = 0;
    tGfuiObject* object = gfuiGetObject(scr, id);

    if (object && object->widget == GFUI_COMBOBOX)
	{
		tGfuiCombobox* combo = &(object->u.combobox);
		combo->pInfo->vecChoices.push_back(text);
		index = combo->pInfo->vecChoices.size();
		gfuiLabelSetText(&combo->label,
						 combo->pInfo->vecChoices[combo->pInfo->nPos].c_str());
    }

	return index;
}

void
GfuiComboboxSetSelectedIndex(void *scr, int id, unsigned int index)
{
    tGfuiObject* object = gfuiGetObject(scr, id);

    if (object && object->widget == GFUI_COMBOBOX)
	{
		tGfuiCombobox* combo = &(object->u.combobox);
		if (index < combo->pInfo->vecChoices.size())
		{
			combo->pInfo->nPos = index;
			gfuiLabelSetText(&combo->label,
							 combo->pInfo->vecChoices[combo->pInfo->nPos].c_str());
		}
	}
}

void
GfuiComboboxSetTextColor(void *scr, int id, const GfuiColor& color)
{
    tGfuiObject* object = gfuiGetObject(scr, id);

    if (object && object->widget == GFUI_COMBOBOX)
	{
		tGfuiCombobox* combo = &(object->u.combobox);
		gfuiLabelSetColor(&combo->label, color.toFloatRGBA());
	}
}



void
GfuiComboboxSetPosition(void *scr, int id, unsigned int pos)
{
	tGfuiCombobox* combo = gfuiGetCombobox(scr, id);
	
    if (combo)
		combo->pInfo->nPos = pos;
}

unsigned int
GfuiComboboxGetPosition(void *scr, int id)
{
	unsigned int index = 0;
    
	tGfuiCombobox* combo = gfuiGetCombobox(scr, id);
	
    if (combo)
		index = combo->pInfo->nPos;

	return index;
}

const char*
GfuiComboboxGetText(void *scr, int id)
{
	const char* pszText = 0;
    
	tGfuiCombobox* combo = gfuiGetCombobox(scr, id);
	
    if (combo && combo->pInfo->nPos < combo->pInfo->vecChoices.size())
		pszText = combo->pInfo->vecChoices[combo->pInfo->nPos].c_str();

	return pszText;
}

unsigned
GfuiComboboxGetNumberOfChoices(void *scr, int id)
{
	unsigned nChoices = 0;
    
	tGfuiCombobox* combo = gfuiGetCombobox(scr, id);
	
    if (combo)
		nChoices = combo->pInfo->vecChoices.size();

	return nChoices;
}
				  

void
GfuiComboboxClear(void *scr, int id)
{
    tGfuiObject* object = gfuiGetObject(scr, id);

    if (object && object->widget == GFUI_COMBOBOX)
	{
		tGfuiCombobox* combo = &(object->u.combobox);
		combo->pInfo->nPos = 0;
		combo->pInfo->vecChoices.clear();
		gfuiLabelSetText(&combo->label, "");
	}
}

/** Handles the button action.
    @ingroup	gui
    @param	action	action
 */
void
gfuiComboboxAction(int action)
{
	if (GfuiScreen->hasFocus->state == GFUI_DISABLE) 
		return;

	tGfuiCombobox* combo = &(GfuiScreen->hasFocus->u.combobox);

	if (action == 2) { /* enter key */
		if (gfuiGrButtonMouseIn(&combo->leftButton)) {
			if (combo->leftButton.onPush)
				combo->leftButton.onPush(combo->leftButton.userDataOnPush);
		} else if (gfuiGrButtonMouseIn(&combo->rightButton)) {
			if (combo->rightButton.onPush)
				combo->rightButton.onPush(combo->rightButton.userDataOnPush);
		}
	} else if (action == 1) { /* mouse up */
		if (gfuiGrButtonMouseIn(&combo->leftButton)) {
			combo->leftButton.state = GFUI_BTN_RELEASED;
			if (combo->leftButton.mouseBehaviour == GFUI_MOUSE_UP && combo->leftButton.onPush)
				combo->leftButton.onPush(combo->leftButton.userDataOnPush);
		} else if (gfuiGrButtonMouseIn(&combo->rightButton)) {
			combo->rightButton.state = GFUI_BTN_RELEASED;
			if (combo->rightButton.mouseBehaviour == GFUI_MOUSE_UP && combo->rightButton.onPush)
				combo->rightButton.onPush(combo->rightButton.userDataOnPush);
		}
	} else { /* mouse down */
		if (gfuiGrButtonMouseIn(&combo->leftButton)) {
			combo->leftButton.state = GFUI_BTN_PUSHED;
			if (combo->leftButton.mouseBehaviour == GFUI_MOUSE_DOWN && combo->leftButton.onPush)
				combo->leftButton.onPush(combo->leftButton.userDataOnPush);
		} else if (gfuiGrButtonMouseIn(&combo->rightButton)) {
			combo->rightButton.state = GFUI_BTN_PUSHED;
			if (combo->rightButton.mouseBehaviour == GFUI_MOUSE_DOWN && combo->rightButton.onPush)
				combo->rightButton.onPush(combo->rightButton.userDataOnPush);
		}
	}
}//gfuiComboboxAction

void
gfuiReleaseCombobox(tGfuiObject *obj)
{
	delete obj->u.combobox.pInfo;
	freez(obj->u.combobox.userDataOnFocus);
    free(obj);
}
