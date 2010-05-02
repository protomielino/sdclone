/***************************************************************************

    file                 : splash.cpp
    created              : Sat Mar 18 23:49:03 CET 2000
    copyright            : (C) 2000 by Eric Espie
    email                : torcs@free.fr
    version              : $Id: splash.cpp,v 1.4.2.2 2008/08/16 14:58:51 berniw Exp $

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
#include <windows.h>
#ifndef HAVE_CONFIG_H
#define HAVE_CONFIG_H
#endif
#endif
#include <stdio.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#include "version.h"
#endif
#include <SDL/SDL.h>
#include <tgfclient.h>

#include "splash.h"
#include "mainmenu.h"

static int s_imgWidth, s_imgHeight; // Real image size (from image file).
static int s_imgPow2Width, s_imgPow2Height; // Smallest possible containing 2^N x 2^P.
static GLuint s_texture = 0;
static int SplashDisplaying;
static int SplashTimedOut;
static char buf[1024];

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
 *  check if spash screen must be closed and close it if so.
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
		
	if (s_texture != 0) 
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
	GfuiPrintString(VERSION_LONG, grWhite, GFUI_FONT_SMALL_C, 640-8, 8, GFUI_ALIGN_HR_VB);
#endif

	GfuiSwapBuffers();
}

static void splashMouse(int /* b */, int s, int /* x */, int /* y */)
{
	if (s == SDL_RELEASED) 
	{
		splashClose();
	}
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
	void	*handle;
	float	screen_gamma;
	//const char	*filename = "data/img/splash.png";
	const char	*filename = "data/img/splash.jpg";
	
	if (s_texture) 
	{
		glDeleteTextures(1, &s_texture); 
	}
	
	sprintf(buf, "%s%s", GetLocalDir(), GFSCR_CONF_FILE);
	handle = GfParmReadFile(buf, GFPARM_RMODE_STD | GFPARM_RMODE_CREAT);
	screen_gamma = (float)GfParmGetNum(handle, GFSCR_SECT_PROP, GFSCR_ATT_GAMMA, (char*)NULL, 2.0);	
	// TODO: Add a new GfTexRead function for implicit image format (use file extension to decide)
  //GLbyte *tex = (GLbyte*)GfTexReadPng(filename, &s_imgWidth, &s_imgHeight, screen_gamma, &s_imgPow2Width, &s_imgPow2Height);
	GLbyte *tex = (GLbyte*)GfTexReadJpeg(filename, &s_imgWidth, &s_imgHeight, screen_gamma, &s_imgPow2Width, &s_imgPow2Height);
	if (!tex) 
	{
		GfParmReleaseHandle(handle);
		GfTrace("Couldn't load splash screen image %s\n", filename);
		return false;
	}

	glGenTextures(1, &s_texture);
	glBindTexture(GL_TEXTURE_2D, s_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, s_imgWidth, s_imgHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid *)(tex));
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, s_imgWidth, s_imgHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, (GLvoid *)(tex));
	free(tex);
	
	GfelSetDisplayCB(splashDisplay);
	GfelSetKeyboardDownCB(splashKey);
	GfelSetTimerCB(7000, splashTimer);
	GfelSetMouseButtonCB(splashMouse);
	GfelSetIdleCB(splashIdle);
    
	return true;
}

