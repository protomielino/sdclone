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

#include <stdlib.h>
#include <cstring>
#ifdef WIN32
#include <windows.h>
#endif
#include <SDL/SDL.h>
#include "tgfclient.h"
#include "gui.h"
#include "guifont.h"

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
    if (object == NULL) {
	return;
    }

	combobox = &(object->u.combobox);

	if (combobox->pInfo->nPos>0)
		combobox->pInfo->nPos--;
	else
		combobox->pInfo->nPos = combobox->pInfo->vecChoices.size()-1;

	if (combobox->pInfo->vecChoices.size()>0)
		GfuiLabelSetText(combobox->scr, combobox->labelId,
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
    if (object == NULL) {
	return;
    }

	combobox = &(object->u.combobox);
	combobox->pInfo->nPos++;

	unsigned int max = combobox->pInfo->vecChoices.size()-1; 
	if (combobox->pInfo->nPos>max)
		combobox->pInfo->nPos = 0;
	
	if (combobox->pInfo->vecChoices.size()>0)
		GfuiLabelSetText(combobox->scr, combobox->labelId,
						 combobox->pInfo->vecChoices[combobox->pInfo->nPos].c_str());
	
	if (combobox->onChange)
		combobox->onChange(combobox->pInfo);
}

int
GfuiComboboxCreate(void *scr, int font, int x, int y, int width,
				   int align, int style, const char *pszText,
				   void *userData, tfuiComboboxCallback onChange, 
				   void *userDataOnFocus, tfuiCallback onFocus, tfuiCallback onFocusLost)
{
    tGfuiCombobox	*combobox;
    tGfuiObject		*object;
    tGfuiScreen		*screen = (tGfuiScreen*)scr;
  
    object = (tGfuiObject*)calloc(1, sizeof(tGfuiObject));
	object->widget = GFUI_COMBOBOX;
	object->focusMode = GFUI_FOCUS_NONE; // Children controls take care of this.
    object->id = screen->curId++;
    object->visible = 1;

	combobox = &(object->u.combobox);
	combobox->onChange = onChange;
    combobox->pInfo = new tComboBoxInfo;
	combobox->pInfo->nPos = 0;
	combobox->pInfo->userData = userData;
	combobox->scr = scr;

	int height = gfuiFont[font]->getHeight() - gfuiFont[font]->getDescender();
	
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

	// Create label control.
	int xm = object->xmin + (object->xmax-object->xmin)/2;
	int ym = object->ymin + (object->ymax-object->ymin)/2;

	combobox->labelId =
		GfuiLabelCreateEx(scr, pszText, 0, font, xm, ym, GFUI_ALIGN_HC_VC, 99,
						  userDataOnFocus, onFocus, onFocusLost);

	// Create the left arrow button control.
    combobox->leftButtonId =
		GfuiGrButtonCreate(scr, "data/img/arrow-left-disabled.png", "data/img/arrow-left.png",
						   "data/img/arrow-left.png", "data/img/arrow-left-pushed.png",
						   object->xmin, ym, GFUI_ALIGN_HL_VC, GFUI_MOUSE_UP,
						   (void*)(object->id), gfuiLeftArrow, 0, 0, 0);

	// Create the right arrow button control.
    combobox->rightButtonId =
		GfuiGrButtonCreate(scr, "data/img/arrow-right-disabled.png", "data/img/arrow-right.png",
						   "data/img/arrow-right.png", "data/img/arrow-right-pushed.png",
						   object->xmax, ym, GFUI_ALIGN_HR_VC, GFUI_MOUSE_UP,
						   (void*)(object->id), gfuiRightArrow, 0, 0, 0);

    gfuiAddObject(screen, object);
	
    return object->id;
}



void
gfuiDrawCombobox(tGfuiObject *obj)
{
//Do nothing because children already draw themselves
}

static tGfuiCombobox*
gfuiGetCombobox(void *scr, int id)
{
    tGfuiScreen	*screen = (tGfuiScreen*)scr;
    tGfuiObject* curObject = screen->objects;

    if (curObject)
	{
		do
		{
			curObject = curObject->next;
			if (curObject->id == id && curObject->widget == GFUI_COMBOBOX)
					return &(curObject->u.combobox);
		}
		while (curObject != screen->objects);
    }

	return 0;
}
				  
unsigned int
GfuiComboboxAddText(void *scr, int id, const char *text)
{
	unsigned int index = 0;
	tGfuiCombobox* combo = gfuiGetCombobox(scr, id);
	
    if (combo)
	{
		combo->pInfo->vecChoices.push_back(text);
		index = combo->pInfo->vecChoices.size();
		GfuiLabelSetText(combo->scr, combo->labelId,
						 combo->pInfo->vecChoices[combo->pInfo->nPos].c_str());
    }

	return index;
}

void
GfuiComboboxSetSelectedIndex(void *scr, int id, unsigned int index)
{
	tGfuiCombobox* combo = gfuiGetCombobox(scr, id);
	
    if (combo && index < combo->pInfo->vecChoices.size())
	{
		combo->pInfo->nPos = index;
		GfuiLabelSetText(combo->scr, combo->labelId,
						 combo->pInfo->vecChoices[combo->pInfo->nPos].c_str());
	}
}

void
GfuiComboboxSetTextColor(void *scr, int id, const Color& color)
{
	tGfuiCombobox* combo = gfuiGetCombobox(scr, id);
	
    if (combo)
		GfuiLabelSetColor(combo->scr, combo->labelId, color.GetPtr());
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
	
    if (combo && combo->pInfo->nPos >= 0 && combo->pInfo->nPos < combo->pInfo->vecChoices.size())
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
	tGfuiCombobox* combo = gfuiGetCombobox(scr, id);
	
    if (combo)
	{
		combo->pInfo->nPos = 0;
		combo->pInfo->vecChoices.clear();
		GfuiLabelSetText(combo->scr, combo->labelId, "");
	}
}

void
gfuiReleaseCombobox(tGfuiObject *obj)
{
	delete obj->u.combobox.pInfo;
    free(obj);
}
