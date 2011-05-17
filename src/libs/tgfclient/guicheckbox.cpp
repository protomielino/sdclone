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

#include "tgfclient.h"

#include "gui.h"
#include "guifont.h"


void
gfuiCheckboxInit(void)
{
}

static void
gfuiChecked(void *idv)
{
	GfuiCheckboxSetChecked(GfuiScreen,(long)idv,false);
	GfuiUnSelectCurrent();

    tGfuiObject		*object;
    tGfuiCheckbox *checkbox;

    object = gfuiGetObject(GfuiScreen, (long)idv);
    if (object == NULL) {
	return;
    }

	checkbox = &(object->u.checkbox);

	if (checkbox->onChange)
		checkbox->onChange(checkbox->pInfo);

}

static void
gfuiUnchecked(void *idv)
{
	GfuiCheckboxSetChecked(GfuiScreen,(long)idv,true);
	GfuiUnSelectCurrent();
    tGfuiObject		*object;
    tGfuiCheckbox *checkbox;

    object = gfuiGetObject(GfuiScreen, (long)idv);
    if (object == NULL) {
	return;
    }

	checkbox = &(object->u.checkbox);

	if (checkbox->onChange)
		checkbox->onChange(checkbox->pInfo);

}

int 
GfuiCheckboxCreate(void *scr, int font, int x, int y, int imagewidth, int imageheight,
				   int align ,int style, const char *pszText, bool bChecked,
				   void* userData, tfuiCheckboxCallback onChange, 
				   void *userDataOnFocus, tfuiCallback onFocus, tfuiCallback onFocusLost)
{
    tGfuiCheckbox	*Checkbox;
    tGfuiObject		*object;
    tGfuiScreen		*screen = (tGfuiScreen*)scr;
  
    object = (tGfuiObject*)calloc(1, sizeof(tGfuiObject));
	object->widget = GFUI_CHECKBOX;
	object->focusMode = GFUI_FOCUS_NONE;
    object->id = screen->curId++;
    object->visible = 1;

	Checkbox = &(object->u.checkbox);
	Checkbox->onChange = onChange;
	Checkbox->pInfo = new tCheckBoxInfo;
	Checkbox->pInfo->bChecked = bChecked;
	Checkbox->pInfo->userData = userData;
	Checkbox->scr = scr;

	int height = gfuiFont[font]->getHeight() - gfuiFont[font]->getDescender();

	int xm = x+imagewidth+5;


	Checkbox->labelId = GfuiLabelCreate(scr,pszText,font,xm,y,GFUI_ALIGN_HL_VC,strlen(pszText));
	int width = imagewidth+5+gfuiFont[font]->getWidth(pszText);
	
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

	Checkbox->checkId =
		GfuiGrButtonCreate(scr, "data/img/checked.png", "data/img/checked.png",
						   "data/img/checked.png", "data/img/checked.png",
						   x, y, imagewidth, imageheight, GFUI_ALIGN_HL_VC, GFUI_MOUSE_UP,
						   (void*)(object->id), gfuiChecked,
						   userDataOnFocus, onFocus, onFocusLost);

	Checkbox->uncheckId =
		GfuiGrButtonCreate(scr, "data/img/unchecked.png", "data/img/unchecked.png",
						   "data/img/unchecked.png", "data/img/unchecked.png",
						   x, y, imagewidth, imageheight, GFUI_ALIGN_HL_VC, GFUI_MOUSE_UP,
						   (void*)(object->id), gfuiUnchecked, 0, 0, 0);
	// We avoid sharing the same userDataOnFocus among multiple controls (otherwise multiple frees).

    gfuiAddObject(screen, object);

	GfuiCheckboxSetChecked(scr,object->id,bChecked);
    return object->id;
}



void
gfuiDrawCheckbox(tGfuiObject *obj)
{
//Do nothing because children already draw themselves
}

unsigned int
GfuiCheckboxAddText(void *scr, int id, const char *text)
{
    tGfuiObject *curObject;
    tGfuiScreen	*screen = (tGfuiScreen*)scr;
	unsigned int index = 0;
    
    curObject = screen->objects;
    if (curObject != NULL) {
	do {
	    curObject = curObject->next;
	    if (curObject->id == id) {
		if (curObject->widget == GFUI_COMBOBOX) 
		{
			tGfuiCheckbox *Check = &(curObject->u.checkbox);
			GfuiLabelSetText(Check->scr,Check->labelId,text);
		}
		return index;
	    }
	} while (curObject != screen->objects);
    }

	return index;
}

void
GfuiCheckboxSetChecked(void *scr, int id, bool bChecked)
{
    tGfuiObject *curObject;
    tGfuiScreen	*screen = (tGfuiScreen*)scr;
    
    curObject = screen->objects;
    if (curObject != NULL) {
	do {
	    curObject = curObject->next;
	    if (curObject->id == id) {
		if (curObject->widget == GFUI_CHECKBOX)
		{
			tGfuiCheckbox *Check = &(curObject->u.checkbox);
			Check->pInfo->bChecked = bChecked;
			GfuiVisibilitySet(scr,Check->checkId,bChecked);
			GfuiVisibilitySet(scr,Check->uncheckId,!bChecked);
		}
		return;
	    }
	} while (curObject != screen->objects);
    }

}

void
GfuiCheckboxSetTextColor(void *scr, int id, const GfuiColor& color)
{
    tGfuiObject *curObject;
    tGfuiScreen	*screen = (tGfuiScreen*)scr;
    
    curObject = screen->objects;
    if (curObject != NULL) {
	do {
	    curObject = curObject->next;
	    if (curObject->id == id) {
		if (curObject->widget == GFUI_CHECKBOX) 
		{
			tGfuiCheckbox *Check = &(curObject->u.checkbox);
			GfuiLabelSetColor(Check->scr,Check->labelId,color.toFloatRGBA());
		}
		return;
	    }
	} while (curObject != screen->objects);
    }

	return;
}

void
gfuiReleaseCheckbox(tGfuiObject *obj)
{
    tGfuiCheckbox	*checkbox;

    checkbox = &(obj->u.checkbox);
    free(obj);
}
