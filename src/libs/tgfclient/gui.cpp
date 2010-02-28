/***************************************************************************
                               gui.cpp -- gui                   
                             -------------------                                         
    created              : Fri Aug 13 22:01:33 CEST 1999
    copyright            : (C) 1999 by Eric Espie                         
    email                : torcs@free.fr   
    version              : $Id: gui.cpp,v 1.7.2.2 2008/08/17 02:45:03 berniw Exp $                                  
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
    		This API is used to manage all the menu screens.
    @author	<a href=mailto:torcs@free.fr>Eric Espie</a>
    @version	$Id: gui.cpp,v 1.7.2.2 2008/08/17 02:45:03 berniw Exp $
    @ingroup	gui
*/

#include <stdlib.h>
#include <stdio.h>
#include <cstring>
#include <time.h>

#include <SDL/SDL.h>
#include "tgfclient.h"
#include "gui.h"
#include <string>

#include <raceman.h>

#include <portability.h>

tGfuiScreen	*GfuiScreen;	/* current screen */
static int	GfuiMouseVisible = 1;
tMouseInfo	GfuiMouse;

int		GfuiMouseHW = 0;

float		GfuiColor[GFUI_COLORNB][4];
static		char buf[1024];


static int	ScrW, ScrH, ViewW, ViewH;

static tdble DelayRepeat;
static double LastTimeClick;
#define REPEAT1	1.0
#define REPEAT2 0.2


static void
gfuiColorInit(void)
{
	void *hdle;
	int  i, j;
	const char *rgba[4] = {GFSCR_ATTR_RED, GFSCR_ATTR_GREEN, GFSCR_ATTR_BLUE, GFSCR_ATTR_ALPHA};
	
	const char *clr[GFUI_COLORNB] = {
		GFSCR_ELT_BGCOLOR, GFSCR_ELT_TITLECOLOR, GFSCR_ELT_BGBTNFOCUS, GFSCR_ELT_BGBTNCLICK,
		GFSCR_ELT_BGBTNENABLED, GFSCR_ELT_BGBTNDISABLED, GFSCR_ELT_BTNFOCUS, GFSCR_ELT_BTNCLICK,
		GFSCR_ELT_BTNENABLED, GFSCR_ELT_BTNDISABLED, GFSCR_ELT_LABELCOLOR, GFSCR_ELT_TIPCOLOR,
		GFSCR_ELT_MOUSECOLOR1, GFSCR_ELT_MOUSECOLOR2, GFSCR_ELT_HELPCOLOR1, GFSCR_ELT_HELPCOLOR2,
		GFSCR_ELT_BGSCROLLIST, GFSCR_ELT_SCROLLIST, GFSCR_ELT_BGSELSCROLLIST, GFSCR_ELT_SELSCROLLIST,
		GFSCR_ELT_EDITCURSORCLR, GFSCR_ELT_LABELCOLORDRIVERCONFIG, GFSCR_ELT_BASECOLORBGIMAGE, GFSCR_ELT_EDITBOXCOLOR,
		GFSCR_ELt_LABELCOLOROPTIONS, GFSCR_ELT_TABLEHEADER
	};

	sprintf(buf, "%s%s", GetLocalDir(), GFSCR_CONF_FILE);
	hdle = GfParmReadFile(buf, GFPARM_RMODE_STD | GFPARM_RMODE_CREAT);

	for (i = 0; i < GFUI_COLORNB; i++) {
		for (j = 0; j < 4; j++) {
			sprintf(buf, "%s/%s/%s", GFSCR_SECT_MENUCOL, GFSCR_LIST_COLORS, clr[i]);
			GfuiColor[i][j] = GfParmGetNum(hdle, buf, rgba[j], (char*)NULL, 1.0);
		}
	}

	GfParmReleaseHandle(hdle);
	
	/* Remove the X11/Windows cursor  */
	if (!GfuiMouseHW) {
		sdlSetCursor(/*on=*/0);
	}
	
	GfuiMouseVisible = 1;
}


void
gfuiInit(void)
{
	gfuiButtonInit();
	gfuiComboboxInit();
	gfuiHelpInit();
	gfuiLabelInit();
	gfuiObjectInit();
	gfuiColorInit();
	gfuiLoadFonts();


}

Color 
GetColor(float *color)
{
     Color c;
     c.red = color[0];
     c.green = color[1];
     c.blue = color[2];
     c.alpha = color[3];

     return c;
}


/** Dummy display function for sdl.
    Declare this function to sdl if nothing is to be displayed by the redisplay mechanism.
     @ingroup	gui
*/

void
GfuiDisplayNothing(void)
{
}

/** Idle function for the GUI to be called during Idle loop of sdl.
     @ingroup	gui
  */
void
GfuiIdle(void)
{
	double		curtime = GfTimeClock();
	
	if ((curtime - LastTimeClick) > DelayRepeat) {
		DelayRepeat = REPEAT2;
		LastTimeClick = curtime;
		if (GfuiScreen->mouse == 1) {
			/* button down */
			gfuiUpdateFocus();
			gfuiMouseAction((void*)0);
			sdlPostRedisplay();
		}
	}
}

/** Display function for the GUI to be called during redisplay of sdl.
     @ingroup	gui
 */
void
GfuiDisplay(void)
{
	tGfuiObject	*curObj;
	
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_CULL_FACE);
	glDisable(GL_ALPHA_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	GfScrGetSize(&ScrW, &ScrH, &ViewW, &ViewH);
	
	glViewport((ScrW-ViewW) / 2, (ScrH-ViewH) / 2, ViewW, ViewH);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, GfuiScreen->width, 0, GfuiScreen->height);
	glMatrixMode(GL_MODELVIEW);  
	glLoadIdentity();
	
	if (GfuiScreen->bgColor.alpha != 0.0) {
		glClearColor(GfuiScreen->bgColor.red,
			     GfuiScreen->bgColor.green,
			     GfuiScreen->bgColor.blue,
			     GfuiScreen->bgColor.alpha);
		glClear(GL_COLOR_BUFFER_BIT);
	}
	
	// Display backround image if any.
	if (GfuiScreen->bgImage) {

		// Prepare texture display.
		glDisable(GL_BLEND);
		glEnable(GL_TEXTURE_2D);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		glColor3f(GfuiColor[GFUI_BASECOLORBGIMAGE][0], 
			  GfuiColor[GFUI_BASECOLORBGIMAGE][1],
			  GfuiColor[GFUI_BASECOLORBGIMAGE][2]);
		glBindTexture(GL_TEXTURE_2D, GfuiScreen->bgImage);

		// Get real 2^N x 2^P texture size (may have been 0 padded at load time
		// if the original image was not 2^N x 2^P)
		// This 2^N x 2^P stuff is needed by some low-end OpenGL hardware/drivers.
		int bgPow2Width = 1, bgPow2Height = 1;
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &bgPow2Width);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &bgPow2Height);

		// Compute the initial width of the right area and the height of the bottom area
		// of the texture that will not be displayed
		// (We display only the top left rectangle of the 2^N x 2^P texture
		//  that corresponds to the original image).
		GLfloat tx1 = 0.0f;
		GLfloat tx2 = GfuiScreen->bgWidth / (GLfloat)bgPow2Width;

		GLfloat ty1 = 1.0f - GfuiScreen->bgHeight / (GLfloat)bgPow2Height;
 		GLfloat ty2 = 1.0;

		// Compute the width/height of the symetrical left/right / top/bottom
		// areas of original image that will need to be clipped
		// in order to keep its aspect ratio.
		const GLfloat rfactor = GfuiScreen->bgWidth * (GLfloat)ViewH
		                        / GfuiScreen->bgHeight / (GLfloat)ViewW;

		if (rfactor >= 1.0f) {
			// If aspect ratio of view is smaller than image's one, "cut off" sides.
			const GLfloat tdx = GfuiScreen->bgWidth * (rfactor - 1.0f) / bgPow2Width / 2.0f;
			tx1 += tdx;
			tx2 -= tdx;
		} else {
			// If aspect ratio of view is larger than image's one, 
			// "cut off" top and bottom.
			const GLfloat tdy = GfuiScreen->bgHeight * (1.0f / rfactor - 1.0f) / bgPow2Height / 2.0f;
			ty1 += tdy;
			ty2 -= tdy;
		}

		// Display texture.
		glBegin(GL_QUADS);

		glTexCoord2f(tx1, ty1); glVertex3f(0.0, 0.0, 0.0);
		glTexCoord2f(tx1, ty2); glVertex3f(0.0, GfuiScreen->height, 0.0);
		glTexCoord2f(tx2, ty2); glVertex3f(GfuiScreen->width, GfuiScreen->height, 0.0);
		glTexCoord2f(tx2, ty1); glVertex3f(GfuiScreen->width, 0.0, 0.0);

		glEnd();
		glDisable(GL_TEXTURE_2D);
		glEnable(GL_BLEND);
	}
	
	// Display other screen objects
	curObj = GfuiScreen->objects;
	if (curObj) 
	{
		do 
		{
			curObj = curObj->next;
			GfuiDraw(curObj);
		} while (curObj != GfuiScreen->objects);
	}
	
	// Display mouse cursor if needed/specified
	if (!GfuiMouseHW && GfuiMouseVisible && GfuiScreen->mouseAllowed) 
		GfuiDrawCursor();

	glDisable(GL_BLEND);
	sdlSwapBuffers();
}

/** Hide the mouse cursor
    @ingroup	gui
    @return	none
*/
void
GfuiMouseHide(void)
{
    GfuiScreen->mouseAllowed = 0;
}

/** Show the mouse cursor
    @ingroup	gui
    @return	none
*/
void
GfuiMouseShow(void)
{
    GfuiScreen->mouseAllowed = 1;
}

/** Toggle the mouse cursor visibility
    @ingroup	gui
    @return	none
*/
void
GfuiMouseToggleVisibility(void)
{
    GfuiScreen->mouseAllowed = 1 - GfuiScreen->mouseAllowed;
}

/** Force the hardware mouse pointer
    @ingroup	ctrl
    @return	<tt>0 ... </tt>Ok
		<br><tt>-1 .. </tt>Error
*/
void
GfuiMouseSetHWPresent(void)
{
    GfuiMouseHW = 1;
}

static void
gfuiKeyboard(int unicode, int modifier, int /* x */, int /* y */)
{
	tGfuiKey	*curKey;
	tGfuiObject	*obj;
	
	/* user preempt key */
	if (GfuiScreen->onKeyAction && GfuiScreen->onKeyAction(unicode, modifier, GFUI_KEY_DOWN)) 
	{
		return;
	}
	
	/* now see the user's defined keys */
	if (GfuiScreen->userKeys) {
		curKey = GfuiScreen->userKeys;
		do 
		{
			curKey = curKey->next;
			if (curKey->unicode == unicode && (curKey->modifier == 0 || (curKey->modifier & modifier) != 0))
			{
				if (curKey->onPress) curKey->onPress(curKey->userData);
				break;
			}
		} while (curKey != GfuiScreen->userKeys);
	}
	
	obj = GfuiScreen->hasFocus;
	if (obj) 
	{
		switch (obj->widget) 
		{
			case GFUI_EDITBOX:
				gfuiEditboxKey(obj, 0, unicode, modifier);
				break;
		}
	}
	sdlPostRedisplay();
}

static void
gfuiSpecial(int key, int modifier, int /* x */, int /* y */)
{
	tGfuiKey	*curKey;
	tGfuiObject	*obj;
	
	/* user preempt key */
	if (GfuiScreen->onSKeyAction && GfuiScreen->onSKeyAction(key, modifier, GFUI_KEY_DOWN)) 
	{
		return;
	}
	
	/* now see the user's defined keys */
	if (GfuiScreen->userSpecKeys)
	{
		curKey = GfuiScreen->userSpecKeys;
		do 
		{
			curKey = curKey->next;
			if (curKey->key == key && (curKey->modifier == 0 || (curKey->modifier & modifier) != 0)) 
			{
				if (curKey->onPress) curKey->onPress(curKey->userData);
				break;
			}
		} while (curKey != GfuiScreen->userSpecKeys);
	}

	obj = GfuiScreen->hasFocus;
	if (obj) 
	{
		switch (obj->widget) 
		{
		case GFUI_EDITBOX:
			gfuiEditboxKey(obj, key, 0, modifier);
			break;
		}
	}
	sdlPostRedisplay();
}

static void
gfuiKeyboardUp(int unicode, int modifier, int /* x */, int /* y */)
{
	tGfuiKey	*curKey;
	
	/* user preempt key */
	if (GfuiScreen->onKeyAction && GfuiScreen->onKeyAction(unicode, modifier, GFUI_KEY_UP)) 
	{
		return;
	}
	
	/* now see the user's defined keys */
	if (GfuiScreen->userKeys) 
	{
		curKey = GfuiScreen->userKeys;
		do 
		{
			curKey = curKey->next;
			if (curKey->unicode == unicode && (curKey->modifier == 0 || (curKey->modifier & modifier) != 0)) 
			{
				if (curKey->onRelease) curKey->onRelease(curKey->userData);
				break;
			}
		} while (curKey != GfuiScreen->userKeys);
	}
	
	sdlPostRedisplay();
}

static void
gfuiSpecialUp(int key, int modifier, int /* x */, int /* y */)
{
	tGfuiKey	*curKey;
	
	/* user preempt key */
	if (GfuiScreen->onSKeyAction && GfuiScreen->onSKeyAction(key, modifier, GFUI_KEY_UP)) 
	{
		return;
	}
	
	/* now see the user's defined keys */
	if (GfuiScreen->userSpecKeys) 
	{
		curKey = GfuiScreen->userSpecKeys;
		do 
		{
			curKey = curKey->next;
			if (curKey->key == key && (curKey->modifier == 0 || (curKey->modifier & modifier) != 0)) 
			{
				if (curKey->onRelease) curKey->onRelease(curKey->userData);
				break;
			}
		} while (curKey != GfuiScreen->userSpecKeys);
	}
	
	sdlPostRedisplay();
}

/** Get the mouse information (position and buttons)
    @ingroup	gui
    @return	mouse information
*/
tMouseInfo *GfuiMouseInfo(void)
{
	return &GfuiMouse;
}

/** Set the mouse position
    @ingroup	gui
    @param	x	mouse x pos
    @param	y	mouse y pos
    @return	none
*/
void GfuiMouseSetPos(int x, int y)
{
	sdlWarpPointer(x, y);
}


static void
gfuiMouse(int button, int state, int x, int y)
{
	/* Consider only left, middle and right buttons */
	if (button >= 1 && button <= 3) 
	{
		GfuiMouse.X = (x - (ScrW - ViewW)/2) * (int)GfuiScreen->width / ViewW;
		GfuiMouse.Y = (ViewH - y + (ScrH - ViewH)/2) * (int)GfuiScreen->height / ViewH;
	
		GfuiMouse.button[button-1] = (state == SDL_PRESSED); /* SDL 1st button has index 1 */
	
		DelayRepeat = REPEAT1;
		LastTimeClick = GfTimeClock();
		if (state == SDL_PRESSED) 
		{
			GfuiScreen->mouse = 1;
			gfuiUpdateFocus();
			gfuiMouseAction((void*)0);
		} 
		else 
		{
			GfuiScreen->mouse = 0;
			gfuiUpdateFocus();
			gfuiMouseAction((void*)1);
		}
		sdlPostRedisplay();
	}
}

static void
gfuiMotion(int x, int y)
{
	GfuiMouse.X = (x - (ScrW - ViewW)/2) * (int)GfuiScreen->width / ViewW;
	GfuiMouse.Y = (ViewH - y + (ScrH - ViewH)/2) * (int)GfuiScreen->height / ViewH;
	gfuiUpdateFocus();
	gfuiMouseAction((void*)(1 - GfuiScreen->mouse));
	sdlPostRedisplay();
	DelayRepeat = REPEAT1;
}

static void
gfuiPassiveMotion(int x, int y)
{
	GfuiMouse.X = (x - (ScrW - ViewW)/2) * (int)GfuiScreen->width / ViewW;
	GfuiMouse.Y = (ViewH - y + (ScrH - ViewH)/2) * (int)GfuiScreen->height / ViewH;
	gfuiUpdateFocus();
	sdlPostRedisplay();
}

/** Tell if the screen is active or not.
    @ingroup	gui
    @param	screen	Screen to activate
    @return	1 if active and 0 if not.
 */
int
GfuiScreenIsActive(void *screen)
{
	return (GfuiScreen == screen);
}

/** Activate a screen and make it current.
    @ingroup	gui
    @param	screen	Screen to activate
    @warning	The current screen at the call time is deactivated.
 */
void
GfuiScreenActivate(void *screen)
{
	if (GfuiScreen && GfuiScreen->onDeactivate) 
		GfuiScreen->onDeactivate(GfuiScreen->userDeactData);
	
	GfuiScreen = (tGfuiScreen*)screen;
	
  	sdlKeyboardFunc(gfuiKeyboard);
	sdlSpecialFunc(gfuiSpecial);
	sdlKeyboardUpFunc(gfuiKeyboardUp);
	sdlSpecialUpFunc(gfuiSpecialUp);
   	sdlMouseFunc(gfuiMouse);
	sdlMotionFunc(gfuiMotion);
	sdlPassiveMotionFunc(gfuiPassiveMotion);
	sdlIdleFunc(0);

	if (GfuiScreen->keyAutoRepeat)
		SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
	else
		SDL_EnableKeyRepeat(0, 0);

	if (GfuiScreen->onlyCallback == 0) 
	{
		if (GfuiScreen->hasFocus == NULL) 
		{
			gfuiSelectNext(NULL);
		}
		sdlDisplayFunc(GfuiDisplay);
	} 
	else 
	{
		sdlDisplayFunc(GfuiDisplayNothing);
	}
	
	if (GfuiScreen->onActivate)
		GfuiScreen->onActivate(GfuiScreen->userActData);
	
	if (GfuiScreen->onlyCallback == 0) 
	{
		GfuiDisplay();
		sdlPostRedisplay();
	}
}


/** Activate a screen and make it current plus release the current screen.
    @ingroup	gui
    @param	screen	Screen to activate
    @warning	The current screen at the call time is deactivated.
 */
void
GfuiScreenReplace(void *screen)
{
	tGfuiScreen	*oldScreen = GfuiScreen;

	if (oldScreen) 
		GfuiScreenRelease(oldScreen);
	GfuiScreenActivate(screen);
}

/** Deactivate the current screen.
    @ingroup	gui
 */
void
GfuiScreenDeactivate(void)
{
	if (GfuiScreen->onDeactivate) GfuiScreen->onDeactivate(GfuiScreen->userDeactData);
	
	GfuiScreen = (tGfuiScreen*)NULL;
	
  	sdlKeyboardFunc(0);
  	sdlSpecialFunc(0);
 	sdlKeyboardUpFunc(0);
 	sdlSpecialUpFunc(0);
	sdlMouseFunc(0);
  	sdlMotionFunc(0);
	sdlPassiveMotionFunc(0);
 	sdlIdleFunc(0);
 	sdlDisplayFunc(GfuiDisplayNothing);
}

/** Create a new screen.
    @ingroup	gui
    @return	New screen instance
		<br>NULL if Error
 */
void *
GfuiScreenCreate(void)
{
	tGfuiScreen	*screen;
	
	screen = (tGfuiScreen*)calloc(1, sizeof(tGfuiScreen));
	
	screen->width = 640.0;
	screen->height = 480.0;

	screen->bgColor = GetColor(&GfuiColor[GFUI_BGCOLOR][0]);

	screen->mouseColor[0] = &(GfuiColor[GFUI_MOUSECOLOR1][0]);
	screen->mouseColor[1] = &(GfuiColor[GFUI_MOUSECOLOR2][0]);

	screen->mouseAllowed = 1;
	
	screen->keyAutoRepeat = 1; // Default key auto-repeat on.
 
	return (void*)screen;
}

/** Create a screen.
    @ingroup	gui
    @param	bgColor			pointer on color array (RGBA) (if NULL default color is used)
    @param	userDataOnActivate	Parameter to the activate function
    @param	onActivate		Function called when the screen is activated
    @param	userDataOnDeactivate	Parameter to the deactivate function
    @param	onDeactivate		Function called when the screen is deactivated
    @param	mouseAllowed		Flag to tell if the mouse cursor can be displayed
    @return	New screen instance
		<br>NULL if Error
    @bug	Only black background work well
 */
void *
GfuiScreenCreateEx(float *bgColor,
		   void *userDataOnActivate, tfuiCallback onActivate, 
		   void *userDataOnDeactivate, tfuiCallback onDeactivate,
		   int mouseAllowed)
{

	tGfuiScreen	*screen;
	
	screen = (tGfuiScreen*)calloc(1, sizeof(tGfuiScreen));
	
	screen->width = 640.0;
	screen->height = 480.0;
	
	if (bgColor != NULL) {
		screen->bgColor = GetColor(bgColor);
	} else {
		screen->bgColor = GetColor(&GfuiColor[GFUI_BGCOLOR][0]);
	}

	screen->mouseColor[0] = &(GfuiColor[GFUI_MOUSECOLOR1][0]);
	screen->mouseColor[1] = &(GfuiColor[GFUI_MOUSECOLOR2][0]);
	screen->onActivate = onActivate;
	screen->userActData = userDataOnActivate;
	screen->onDeactivate = onDeactivate;
	screen->userDeactData = userDataOnDeactivate;
	
	screen->mouseAllowed = mouseAllowed;
	
	screen->keyAutoRepeat = 1; // Default key auto-repeat on.
 
	return (void*)screen;
}

/** Release the given screen.
    @ingroup	gui
    @param	scr	Screen to release
    @warning	If the screen was activated, it is deactivated.
 */
void
GfuiScreenRelease(void *scr)
{
	tGfuiObject *curObject;
	tGfuiObject *nextObject;
	tGfuiKey *curKey;
	tGfuiKey *nextKey;
	tGfuiScreen *screen = (tGfuiScreen*)scr;

	if (GfuiScreen == screen) {
		GfuiScreenDeactivate();
	}

	if (screen->bgImage != 0) {
		glDeleteTextures(1, &screen->bgImage);
	}

	curObject = screen->objects;
	if (curObject != NULL) {
		do {
			nextObject = curObject->next;
			gfuiReleaseObject(curObject);
			curObject = nextObject;
		} while (curObject != screen->objects);
	}

	curKey = screen->userKeys;
	if (curKey != NULL) {
		do {
			nextKey = curKey->next;
			free(curKey->name);
			free(curKey->descr);
			free(curKey);
			curKey = nextKey;
		} while (curKey != screen->userKeys);
	}
	curKey = screen->userSpecKeys;
	if (curKey != NULL) {
		do {
			nextKey = curKey->next;
			free(curKey->name);
			free(curKey->descr);
			free(curKey);
			curKey = nextKey;
		} while (curKey != screen->userSpecKeys);
	}
	free(screen);
}

/** Create a callback hook.
    @ingroup	gui
    @param	userDataOnActivate	Parameter to the activate function
    @param	onActivate		Function called when the screen is activated
    @return	New hook instance
		<br>NULL if Error
 */
void *
GfuiHookCreate(void *userDataOnActivate, tfuiCallback onActivate)
{
	tGfuiScreen	*screen;
	
	screen = (tGfuiScreen*)calloc(1, sizeof(tGfuiScreen));
	screen->onActivate = onActivate;
	screen->userActData = userDataOnActivate;
	screen->onlyCallback = 1;
	
	return (void*)screen;
}

/** Release the given hook.
    @ingroup	gui
    @param	hook	Hook to release
 */
void
GfuiHookRelease(void *hook)
{
	free(hook);
}

void
GfuiKeyEventRegister(void *scr, tfuiKeyCallback onKeyAction)
{
	tGfuiScreen	*screen = (tGfuiScreen*)scr;
	
	screen->onKeyAction = onKeyAction;
}


void
GfuiSKeyEventRegister(void *scr, tfuiSKeyCallback onSKeyAction)
{
	tGfuiScreen	*screen = (tGfuiScreen*)scr;
	
	screen->onSKeyAction = onSKeyAction;
}

void
GfuiKeyEventRegisterCurrent(tfuiKeyCallback onKeyAction)
{
	GfuiScreen->onKeyAction = onKeyAction;
}


void
GfuiSKeyEventRegisterCurrent(tfuiSKeyCallback onSKeyAction)
{
	GfuiScreen->onSKeyAction = onSKeyAction;
}


/** Add a Keyboard callback to a screen.
    @ingroup	gui
    @param	scr		Screen
    @param	unicode		Key unicode (only 7 low bits considered, i.e. ASCII code)
    @param	descr		Description for help screen
    @param	userData	Parameter to the callback function
    @param	onKeyPressed	Callback function
    @param	onKeyReleased	Callback function
 */
void
GfuiAddKey(void *scr, int unicode, const char *descr, void *userData, tfuiCallback onKeyPressed, tfuiCallback onKeyReleased)
{
	tGfuiKey	*curKey;
	tGfuiScreen	*screen = (tGfuiScreen*)scr;
	char	buf[16];
	
	curKey = (tGfuiKey*)calloc(1, sizeof(tGfuiKey));
	curKey->unicode = unicode;
	curKey->userData = userData;
	curKey->onPress = onKeyPressed;

	if (descr == NULL) {
		curKey->descr = strdup("");
	} else {
		curKey->descr = strdup(descr);
	}

	switch(unicode){
		case 8:
			curKey->name = strdup("backspace");
			break;
		case 9:
			curKey->name = strdup("tab");
			break;
		case 13:
			curKey->name = strdup("enter");
			break;
		case 27:
			curKey->name = strdup("esc");
			break;
		case ' ':
			curKey->name = strdup("space");
			break;
		default:
			sprintf(buf, "%c", (char)(unicode & 0x007F));
			curKey->name = strdup(buf);
			break;
	}
	
	if (screen->userKeys == NULL) {
		screen->userKeys = curKey;
		curKey->next = curKey;
	} else {
		curKey->next = screen->userKeys->next;
		screen->userKeys->next = curKey;
	}
}

/** Add a Keyboard callback to the current screen.
    @ingroup	gui
    @param	unicode		Key unicode (only 7 low bits considered, i.e. ASCII code)
    @param	descr		Description for help screen
    @param	userData	Parameter to the callback function
    @param	onKeyPressed	Callback function called when the specified key is pressed
    @param	onKeyReleased	Callback function
 */
void
GfuiRegisterKey(int unicode, const char *descr, void *userData, tfuiCallback onKeyPressed, tfuiCallback onKeyReleased)
{
	GfuiAddKey(GfuiScreen, unicode, descr, userData, onKeyPressed, onKeyReleased);
}

/** Add a Special Keyboard shortcut to the screen.
    (see glut for normal and special keys)
    @ingroup	gui
    @param	scr		Screen
    @param	key		Key code (sdl value)
    @param	descr		Description for help screen
    @param	userData	Parameter to the callback function
    @param	onKeyPressed	Callback function
    @param	onKeyReleased	Callback function
 */
void
GfuiAddSKey(void *scr, int key, const char *descr, void *userData, tfuiCallback onKeyPressed, tfuiCallback onKeyReleased)
{
	tGfuiKey	*curKey;
	tGfuiScreen	*screen = (tGfuiScreen*)scr;
	
	curKey = (tGfuiKey*)calloc(1, sizeof(tGfuiKey));
	curKey->key = key;
	curKey->userData = userData;
	curKey->onPress = onKeyPressed;

	if (descr == NULL) {
		curKey->descr = strdup("");
	} else {
		curKey->descr = strdup(descr);
	}

	switch(key) {
		case GFUIK_F1:
			curKey->name = strdup("F1");
			break;
		case GFUIK_F2:
			curKey->name = strdup("F2");
			break;
		case GFUIK_F3:
			curKey->name = strdup("F3");
			break;
		case GFUIK_F4:
			curKey->name = strdup("F4");
			break;
		case GFUIK_F5:
			curKey->name = strdup("F5");
			break;
		case GFUIK_F6:
			curKey->name = strdup("F6");
			break;
		case GFUIK_F7:
			curKey->name = strdup("F7");
			break;
		case GFUIK_F8:
			curKey->name = strdup("F8");
			break;
		case GFUIK_F9:
			curKey->name = strdup("F9");
			break;
		case GFUIK_F10:
			curKey->name = strdup("F10");
			break;
		case GFUIK_F11:
			curKey->name = strdup("F11");
			break;
		case GFUIK_F12:
			curKey->name = strdup("F12");
			break;
		case GFUIK_LEFT:
			curKey->name = strdup("Left Arrow");
			break;
		case GFUIK_UP:
			curKey->name = strdup("Up Arrow");
			break;
		case GFUIK_RIGHT:
			curKey->name = strdup("Right Arrow");
			break;
		case GFUIK_DOWN:
			curKey->name = strdup("Down Arrow");
			break;
		case GFUIK_PAGEUP:
			curKey->name = strdup("Page Up");
			break;
		case GFUIK_PAGEDOWN:
			curKey->name = strdup("Page Down");
			break;
		case GFUIK_HOME:
			curKey->name = strdup("Home");
			break;
		case GFUIK_END:
			curKey->name = strdup("End");
			break;
		case GFUIK_INSERT:
			curKey->name = strdup("Insert");
			break;
		default:
			curKey->name = strdup("<Unknown SKey>");
	}
	
	
	if (screen->userSpecKeys == NULL) {
		screen->userSpecKeys = curKey;
		curKey->next = curKey;
	} else {
		curKey->next = screen->userSpecKeys->next;
		screen->userSpecKeys->next = curKey;
		screen->userSpecKeys = curKey;
	}
}

/** Enable/disable the key auto-repeat for the given screen.
    @ingroup	screen
    @param	scr		Screen
    @param	on		Flag
 */
void
GfuiSetKeyAutoRepeat(void *scr, int on)
{
	tGfuiScreen	*screen = (tGfuiScreen*)scr;
	screen->keyAutoRepeat = on;
}

/** Save a screen shot in png format.
    @ingroup	screen
 */
void
GfuiScreenShot(void * /* notused */)
{
	unsigned char *img;
	char buf[1024];
	struct tm *stm;
	time_t t;
	int sw, sh, vw, vh;
	char path[1024];
	
	snprintf(path, 1024, "%sscreenshots", GetLocalDir());
	// Ensure that screenshot directory exists.
	if (GfCreateDir(path) == GF_DIR_CREATED) {
	
		GfScrGetSize(&sw, &sh, &vw, &vh);
		img = (unsigned char*)malloc(vw * vh * 3);
		if (img == NULL) {
			return;
		}
		
		glPixelStorei(GL_PACK_ROW_LENGTH, 0);
		glPixelStorei(GL_PACK_ALIGNMENT, 1);
		glReadBuffer(GL_FRONT);
		glReadPixels((sw-vw)/2, (sh-vh)/2, vw, vh, GL_RGB, GL_UNSIGNED_BYTE, (GLvoid*)img);
		
		t = time(NULL);
		stm = localtime(&t);
		snprintf(buf, 1024, "%s/sd-%4d%02d%02d%02d%02d%02d.png",
			path,
			stm->tm_year+1900,
			stm->tm_mon+1,
			stm->tm_mday,
			stm->tm_hour,
			stm->tm_min,
			stm->tm_sec);
		GfTexWritePng(img, buf, vw, vh);
		
		free(img);
	}
}

/** Add an image background to a screen.
    @ingroup	gui
    @param	scr		Screen
    @param	filename	file name of the bg image
    @return	None.
 */
void
GfuiScreenAddBgImg(void *scr, const char *filename)
{
	tGfuiScreen	*screen = (tGfuiScreen*)scr;
	void *handle;
	float screen_gamma;
	GLbyte *tex;
	int pow2Width, pow2Height;

	if (screen->bgImage != 0) {
		glDeleteTextures(1, &screen->bgImage);
	}

	sprintf(buf, "%s%s", GetLocalDir(), GFSCR_CONF_FILE);
	handle = GfParmReadFile(buf, GFPARM_RMODE_STD);
	screen_gamma = (float)GfParmGetNum(handle, GFSCR_SECT_PROP, GFSCR_ATT_GAMMA, (char*)NULL, 2.0);

	// Note: Here, we save the original image size (may be not 2^N x 2^P)
	//       in order to be able to hide padding pixels added in texture to enforce 2^N x 2^P.
	tex = (GLbyte*)GfTexReadPng(filename, &screen->bgWidth, &screen->bgHeight, 
				    screen_gamma, &pow2Width, &pow2Height);
	// TODO: Enable implicit image format (PNG or JPEG), with a new GfTexRead
	//tex = (GLbyte*)GfTexReadJpeg(filename, &w, &h, screen_gamma);
	if (!tex) {
		GfParmReleaseHandle(handle);
		return;
	}

	glGenTextures(1, &screen->bgImage);
	glBindTexture(GL_TEXTURE_2D, screen->bgImage);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, pow2Width, pow2Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid *)(tex));
	free(tex);

	GfParmReleaseHandle(handle);
}

/** Passive wait (no CPU use) for the current thread.
    @ingroup	gui
    @param	delay		The number of seconds to sleep (real granularity is platform-dependant)
    @return	None.
 */
void
GfuiSleep(double delay)
{
  SDL_Delay(delay*1000); // ms.
}
