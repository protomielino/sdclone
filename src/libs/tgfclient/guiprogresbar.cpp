/***************************************************************************

    file                 : guiprogressbar.cpp
	created              : Feb 17 2010
    copyright            : (C) 2010 by Brian Gavin

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

#include "tgfclient.h"

#include "gui.h"


/** Create a new progress bar
    This kind of image is not clickable.
    @ingroup	gui
    @param	scr	Screen where to add the label
    @param	x	Position of the left of the image on the screen
    @param	y	Position of the bottom of the image on the screen
    @param	w	Width of the image on the screen
    @param	h	Height of the image on the screen
    @param	pszprogressbackimg	Filename on the image use for behind progress bar (png)
    @param	pszprogressbarimg	Filename on the image use for progress bar will be scaled bar (png)
	@param align image alignment
    @return	Image Id
		<br>-1 Error
    @warning	the image must be square and its size must be a power of 2.
*/
int GfuiProgressbarCreate(void *scr, int x, int y, int w, int h,
						  const char *pszProgressbackImg, const char *progressbarimg,
						  int align, float min, float max, float value, 
						  void *userDataOnFocus, tfuiCallback onFocus, tfuiCallback onFocusLost)
{
	tGfuiProgressbar *progress;
	tGfuiObject *object;
	tGfuiScreen *screen = (tGfuiScreen*)scr;

	object = (tGfuiObject*)calloc(1, sizeof(tGfuiObject));
	object->widget = GFUI_PROGRESSBAR;
	object->focusMode = (onFocus || onFocusLost) ? GFUI_FOCUS_MOUSE_MOVE : GFUI_FOCUS_NONE;
	object->visible = 1;
	object->id = screen->curId++;

	progress = &(object->u.progressbar);

	progress->progressbarimage = GfTexReadTexture(progressbarimg);
	if (!progress->progressbarimage) {
		free(object);
		return -1;
	}

	progress->min = min;
	progress->max = max;
	if (value > progress->max)
		value = progress->max;
	else if (value < progress->min)
		value = progress->min;
	progress->value = value;

    progress->userDataOnFocus = userDataOnFocus;
    progress->onFocus = onFocus;
    progress->onFocusLost = onFocusLost;

    switch (align) {
    case GFUI_ALIGN_HR_VB:
	object->xmin = x - w;
	object->xmax = x;
	object->ymin = y;
	object->ymax = y + h;
	break;
    case GFUI_ALIGN_HR_VC:
	object->xmin = x - w;
	object->xmax = x;
	object->ymin = y - h / 2;
	object->ymax = y + h / 2;
	break;
    case GFUI_ALIGN_HR_VT:
	object->xmin = x - w;
	object->xmax = x;
	object->ymin = y - h;
	object->ymax = y;
	break;
    case GFUI_ALIGN_HC_VB:
	object->xmin = x - w / 2;
	object->xmax = x + w / 2;
	object->ymin = y;
	object->ymax = y + h;
	break;
    case GFUI_ALIGN_HC_VC:
	object->xmin = x - w / 2;
	object->xmax = x + w / 2;
	object->ymin = y - h / 2;
	object->ymax = y + h / 2;
	break;
    case GFUI_ALIGN_HC_VT:
	object->xmin = x - w / 2;
	object->xmax = x + w / 2;
	object->ymin = y - h;
	object->ymax = y;
	break;
    case GFUI_ALIGN_HL_VB:
	object->xmin = x;
	object->xmax = x + w;
	object->ymin = y;
	object->ymax = y + h;
	break;
    case GFUI_ALIGN_HL_VC:
	object->xmin = x;
	object->xmax = x + w;
	object->ymin = y - h / 2;
	object->ymax = y + h / 2;
	break;
    case GFUI_ALIGN_HL_VT:
	object->xmin = x;
	object->xmax = x + w;
	object->ymin = y - h;
	object->ymax = y;
	break;
    default:
	break;
    }

	gfuiAddObject(screen, object);

	return object->id;
}

void GfuiProgressbarSetValue(void *scr, int id, float value)
{
    tGfuiObject *curObject;
    tGfuiScreen	*screen = (tGfuiScreen*)scr;
    
    curObject = screen->objects;
    if (curObject) {
		do {
			curObject = curObject->next;
			if (curObject->id == id) {
				if (curObject->widget == GFUI_PROGRESSBAR) {
					if (value > curObject->u.progressbar.max)
						value = curObject->u.progressbar.max;
					else if (value < curObject->u.progressbar.min)
						value = curObject->u.progressbar.min;
					curObject->u.progressbar.value = value;
				}
				return;
			}
		} while (curObject != screen->objects);
    }
}

void
gfuiReleaseProgressbar(tGfuiObject *obj)
{
	tGfuiProgressbar *progress;

	progress = &(obj->u.progressbar);
	GfTexFreeTexture(progress->progressbarimage);
	freez(progress->userDataOnFocus);

	free(obj);
}

void
gfuiDrawProgressbar(tGfuiObject *obj)
{
	tGfuiProgressbar *progress;

	progress = &(obj->u.progressbar);

    glColor4f(1.0,1.0,1.0,0.25);
    glBegin(GL_QUADS);
    glVertex2i(obj->xmin, obj->ymin);
    glVertex2i(obj->xmin, obj->ymax);
    glVertex2i(obj->xmax, obj->ymax);
    glVertex2i(obj->xmax, obj->ymin);
    glEnd();

	//calculate and draw progress bar
	const float width = obj->xmax- obj->xmin;
	const float range = progress->max - progress->min;
	const float umax = (progress->value - progress->min) / range;
	const float endx = obj->xmin+(width*umax);

    glEnable(GL_TEXTURE_2D);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glBindTexture(GL_TEXTURE_2D,progress->progressbarimage );

    glBegin(GL_TRIANGLE_STRIP);
	glColor4f(1.0, 1.0, 1.0, 1.0);
	glTexCoord2f(0.0, 0.0); glVertex2f(obj->xmin, obj->ymin);
	glTexCoord2f(0.0, 1.0); glVertex2f(obj->xmin, obj->ymax);
	glTexCoord2f(umax, 0.0); glVertex2f(endx, obj->ymin);
	glTexCoord2f(umax, 1.0); glVertex2f(endx, obj->ymax);
    glEnd();
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);

	//draw outline
	glColor4f(1.0,0.0,0.0,1.0);
	glBegin(GL_LINE_STRIP);
    glVertex2i(obj->xmin, obj->ymin);
    glVertex2i(obj->xmin, obj->ymax);
    glVertex2i(obj->xmax, obj->ymax);
    glVertex2i(obj->xmax, obj->ymin);
    glVertex2i(obj->xmin, obj->ymin);
    glEnd();	
}
