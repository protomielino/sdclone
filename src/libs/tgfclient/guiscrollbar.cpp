/***************************************************************************

    file                 : guiscrollbar.cpp
    created              : Mon Aug 23 22:11:37 CEST 1999
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
    		GUI scrollbar management.
    @author	<a href=mailto:torcs@free.fr>Eric Espie</a>
    @version	$Id$
    @ingroup	gui
*/

#include <cstdlib>
#include <cstring>

#include "tgfclient.h"

#include "gui.h"

void
gfuiInitScrollBar(void)
{
}

static void
gfuiScrollPlus(void *idv)
{
    tGfuiObject		*object;
    tGfuiScrollBar	*scrollbar;
    tScrollBarInfo	info;

    object = gfuiGetObject(GfuiScreen, (long)idv);
    if (object == NULL) {
	return;
    }
    scrollbar = &(object->u.scrollbar);
    scrollbar->pos++;
    if (scrollbar->pos > scrollbar->max) {
	scrollbar->pos = scrollbar->max;
    } else if (scrollbar->onScroll != NULL) {
	info.pos = scrollbar->pos;
	info.userData = scrollbar->userData;
	scrollbar->onScroll(&info);
    }
}

static void
gfuiScrollMinus(void *idv)
{
    tGfuiObject		*object;
    tGfuiScrollBar	*scrollbar;
    tScrollBarInfo	info;

    object = gfuiGetObject(GfuiScreen, (long)idv);
    if (object == NULL) {
	return;
    }
    scrollbar = &(object->u.scrollbar);
    scrollbar->pos--;
    if (scrollbar->pos < scrollbar->min) {
	scrollbar->pos = scrollbar->min;
    } else if (scrollbar->onScroll != NULL) {
	info.pos = scrollbar->pos;
	info.userData = scrollbar->userData;
	scrollbar->onScroll(&info);
    }
}

/** Create a new scroll bar.
    @ingroup	gui
    @param	scr	Screen where to create the scroll bar
    @param	x	X position
    @param	y	Y position
    @param	width	width (horiz) / height (vert) (arrows included)
    @param	height	height (horiz) / width (vert) (arrows included)
    @param	orientation	Scroll bar orientation:
				<br>GFUI_HORI_SCROLLBAR	Horizontal
				<br>GFUI_VERT_SCROLLBAR	Vertical
    @param	min	Minimum value
    @param	max	Maximum value
    @param	len	Visible length
    @param	start	Starting position
    @param	userData	User data given to the call back function
    @param	onScroll	Call back function called when the position change
    @return	Scroll Bar Id
		<br>-1 Error
 */
int
GfuiScrollBarCreate(void *scr, int x, int y, int width, int height, int orientation,
					int min, int max, int len, int start, 
					void *userData, tfuiSBCallback onScroll)
{
    tGfuiScreen* screen = (tGfuiScreen*)scr;
    
    tGfuiObject* object = (tGfuiObject*)calloc(1, sizeof(tGfuiObject));
    object->widget = GFUI_SCROLLBAR;
    object->focusMode = GFUI_FOCUS_MOUSE_CLICK;
    object->id = screen->curId++;
    object->visible = 1;

    tGfuiScrollBar* scrollbar = &(object->u.scrollbar);
    scrollbar->userData = userData;
    scrollbar->onScroll = onScroll;

	// Warning: All the arrow images are supposed to be the same size.
    switch (orientation) {
		case GFUI_HORI_SCROLLBAR:
		{
			const int arrowButId =
				GfuiGrButtonCreate(scr, "data/img/arrow-left.png", "data/img/arrow-left.png",
								   "data/img/arrow-left-focused.png", "data/img/arrow-left-pushed.png",
								   x, y, 0, height, 1, false,
								   (void*)(object->id), gfuiScrollMinus,
								   NULL, (tfuiCallback)NULL, (tfuiCallback)NULL);
			const tGfuiGrButton* pArrowBut = &(gfuiGetObject(scr, arrowButId)->u.grbutton);
			GfuiGrButtonCreate(scr, "data/img/arrow-right.png", "data/img/arrow-right.png",
							   "data/img/arrow-right-focused.png", "data/img/arrow-right-pushed.png",
							   x + width - pArrowBut->width, y, 0, height, 1, false,
							   (void*)(object->id), gfuiScrollPlus,
							   NULL, (tfuiCallback)NULL, (tfuiCallback)NULL);	    
			break;
		}
		case GFUI_VERT_SCROLLBAR:
		{
			const int arrowButId =
				GfuiGrButtonCreate(scr, "data/img/arrow-down.png", "data/img/arrow-down.png",
								   "data/img/arrow-down-focused.png", "data/img/arrow-down-pushed.png",
								   x, y, height, 0, 1, false,
								   (void*)(object->id), gfuiScrollPlus,
								   NULL, (tfuiCallback)NULL, (tfuiCallback)NULL);	    
			const tGfuiGrButton* pArrowBut = &(gfuiGetObject(scr, arrowButId)->u.grbutton);
			GfuiGrButtonCreate(scr, "data/img/arrow-up.png", "data/img/arrow-up.png",
							   "data/img/arrow-up-focused.png", "data/img/arrow-up-pushed.png",
							   x, y + width - pArrowBut->height, height, 0, 1, false,
							   (void*)(object->id), gfuiScrollMinus,
							   NULL, (tfuiCallback)NULL, (tfuiCallback)NULL);	    
			break;
		}
		default:
			break;
    }
    
    gfuiAddObject(screen, object);
	
    GfuiScrollBarPosSet(scr, object->id, min, max, len, start);
	
    return object->id;
}

/** Get the current position of a scroll bar.
    @ingroup	gui
    @param	scr	Screen
    @param	id	Scroll bar Id
    @return	Current position
		<br>-1 Error
 */
int
GfuiScrollBarPosGet(void *scr, int id)
{
    tGfuiObject		*object;
    tGfuiScrollBar	*scrollbar;

    object = gfuiGetObject(scr, id);
    if (object == NULL) {
	return -1;
    }
    scrollbar = &(object->u.scrollbar);

    return scrollbar->pos;
}

/** Set new values for position.
    @ingroup	gui
    @param	scr	Screen
    @param	id	Scroll bar Id
    @param	min	New minimum value
    @param	max	New maximum value
    @param	len	New visible length
    @param	start	New starting position
 */
void
GfuiScrollBarPosSet(void *scr, int id, int min, int max, int len, int start)
{
    tGfuiObject		*object;
    tGfuiScrollBar	*scrollbar;

    object = gfuiGetObject(scr, id);
    if (object == NULL) {
	return;
    }
    scrollbar = &(object->u.scrollbar);

    scrollbar->min = min;
    scrollbar->max = max;
    scrollbar->len = len;
    scrollbar->pos = start;
}

void
gfuiReleaseScrollbar(tGfuiObject *curObject)
{
    free(curObject);
}
