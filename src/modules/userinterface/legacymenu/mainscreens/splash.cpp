/***************************************************************************

    file                 : splash.cpp
    created              : Sat Mar 18 23:49:03 CET 2000
    copyright            : (C) 2000 by Eric Espie
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

#ifdef WIN32
#ifndef HAVE_CONFIG_H
#define HAVE_CONFIG_H
#endif
#endif

#include <cstdio>

#ifdef HAVE_CONFIG_H
#include "config.h"
#include "version.h"
#endif

#include <SDL/SDL.h>
#include <tgf.hpp>
#include <tgfclient.h>
#include <glfeatures.h>

#include "splash.h"
#include "mainmenu.h"


static int s_imgWidth, s_imgHeight; // Real image size (from image file).
static int s_imgPow2Width, s_imgPow2Height; // Smallest possible containing 2^N x 2^P.
static GLuint s_texture = 0;
static int SplashDisplaying;
static int SplashTimedOut;
static int MainMenuReady;

/*
 * Function
 *	splashClose
 *
 * Description
 *	Close the splash screen and start main menu
 *
 * Parameters
 *	None
 *
 * Return
 *	Nothing
 *
 * Remarks
 *	
 */
static void splashClose()
{
	if (!MainMenuReady)
		return;
	
	SplashDisplaying = 0;
	glDeleteTextures(1, &s_texture);
	s_texture = 0;
	MainMenuRun();
}

/*
 * Function
 *	splashIdle
 *
 * Description
 *	Called by main loop when nothing to do : 
 *  check if splash screen must be closed and close it if so.
 *
 * Parameters
 *	
 *
 * Return
 *	
 *
 * Remarks
 *	
 */
static void splashIdle()
{
	// A kind of background main menus loading (as it may me a bit long).
	MainMenuInit();
	MainMenuReady = 1;
	if (SplashTimedOut)
		splashClose();
}

/*
 * Function
 *	splashKey
 *
 * Description
 *	
 *
 * Parameters
 *	
 *
 * Return
 *	
 *
 * Remarks
 *	
 */
static void splashKey(int /* key */, int /* modifiers */, int /* x */, int /* y */)
{
	splashClose();
}

/*
 * Function
 *	splashTimer
 *
 * Description
 *	End of splash timer callback : can't close spash screen itself, as not run under the control
 *  of the thread that created it
 *
 * Parameters
 *	None
 *
 * Return
 *	None
 *
 * Remarks
 *	
 */
static void splashTimer(int /* value */)
{
	if (SplashDisplaying) 
		SplashTimedOut = 1;
}
	

/*
 * Function
 *	splashDisplay
 *
 * Description
 *
 *
 * Parameters
 *	
 *
 * Return
 *	
 *
 * Remarks
 *	
 */
static void splashDisplay( void )
{
	int ScrW, ScrH, ViewW, ViewH;
	
	SplashDisplaying = 1;
	SplashTimedOut = 0;
		
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	glDisable(GL_CULL_FACE);
	glDisable(GL_ALPHA_TEST);
	
	GfScrGetSize(&ScrW, &ScrH, &ViewW, &ViewH);
	
	glViewport((ScrW-ViewW) / 2, (ScrH-ViewH) / 2, ViewW, ViewH);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, ScrW, 0, ScrH);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
		
	if (s_texture) 
	{
		// Prepare texture display.
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, s_texture);

		// Compute the initial width of the right area and the height of the bottom area
		// of the texture that will not be displayed
		// (We display only the top left rectangle of the quad texture
		//  that corresponds to the original image).
		GLfloat tx1 = 0.0f;
		GLfloat tx2 = s_imgWidth / (GLfloat)s_imgPow2Width;

		GLfloat ty1 = 1.0f - s_imgHeight / (GLfloat)s_imgPow2Height;
 		GLfloat ty2 = 1.0;

		// Compute the width/height of the symetrical left/right / top/bottom
		// areas of original image that will need to be clipped
		// in order to keep its aspect ratio.
		const GLfloat rfactor = s_imgWidth * (GLfloat)ViewH / s_imgHeight / (GLfloat)ViewW;

		if (rfactor >= 1.0f) {
			// If aspect ratio of view is smaller than image's one, "cut off" sides.
			const GLfloat tdx = s_imgWidth * (rfactor - 1.0f) / s_imgPow2Width / 2.0f;
			tx1 += tdx;
			tx2 -= tdx;
		} else {
			// If aspect ratio of view is larger than image's one, 
			// "cut off" top and bottom.
			const GLfloat tdy = s_imgHeight * (1.0f / rfactor - 1.0f) / s_imgPow2Height / 2.0f;
			ty1 += tdy;
			ty2 -= tdy;
		}

		// Display texture.
		glBegin(GL_QUADS);
		glTexCoord2f(tx1, ty1); glVertex3f(0.0, 0.0, 0.0);
		glTexCoord2f(tx1, ty2); glVertex3f(0.0, ScrH, 0.0);
		glTexCoord2f(tx2, ty2); glVertex3f(ScrW, ScrH, 0.0);
		glTexCoord2f(tx2, ty1); glVertex3f(ScrW, 0.0, 0.0);
		glEnd();
		glDisable(GL_TEXTURE_2D);
	}
		
#ifdef HAVE_CONFIG_H
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, 640, 0, 480);
	
	static float grWhite[4] = {1.0, 1.0, 1.0, 1.0};
	GfuiDrawString(VERSION_LONG, grWhite, GFUI_FONT_SMALL_C, 440-8, 8, 200, GFUI_ALIGN_HR);
#endif

	GfuiSwapBuffers();
}

static void splashMouse(int /* b */, int s, int /* x */, int /* y */)
{
	if (s == SDL_RELEASED) 
		splashClose();
}


/*
 * Function
 *	SplashScreen
 *
 * Description
 *	Display the splash screen and load the main menus in the background.
 *      On mouse click or 7 second time-out, open the main menu.
 *
 * Parameters
 *	None
 *
 * Return
 *	true on success, false in anything bad happened.
 *
 * Remarks
 *	
 */
bool SplashScreen(void)
{
	// Free splash texture if was loaded already.
	if (s_texture) 
		GfTexFreeTexture(s_texture); 

	// Get screen gamma from graphics configuration.
	//static char buf[512];
	//sprintf(buf, "%s%s", GfLocalDir(), GFSCR_CONF_FILE);
	//void* handle = GfParmReadFile(buf, GFPARM_RMODE_STD | GFPARM_RMODE_CREAT);
	//float screen_gamma =
	//	(float)GfParmGetNum(handle, GFSCR_SECT_VALIDPROPS, GFSCR_ATT_GAMMA, (char*)NULL, 2.0);
	
	// Load splash texture from file.
	s_texture = GfTexReadTexture("data/img/splash.jpg",
								 &s_imgWidth, &s_imgHeight, &s_imgPow2Width, &s_imgPow2Height);

	// Prevent MainMenuRun being called (by splashClose) before MainMenuInit (by splashIdle)
	MainMenuReady = 0;

	// Setup event loop callbacks.
	GfuiApp().eventLoop().setRedisplayCB(splashDisplay);
	GfuiApp().eventLoop().setKeyboardDownCB(splashKey);
	GfuiApp().eventLoop().setTimerCB(7000, splashTimer);
	GfuiApp().eventLoop().setMouseButtonCB(splashMouse);
	GfuiApp().eventLoop().setRecomputeCB(splashIdle);
    
	return true;
}

