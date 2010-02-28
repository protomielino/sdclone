/***************************************************************************
                               sdlcallbacks.cpp -- sdlcallbacks                   
                             -------------------                                         
    created              : Thu Mar 8 10:00:00 CEST 2006
    copyright            : (C) 2006 by Brian Gavin
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/* sdlcallbacks mimics the behavior of the GLUT callback system. 
   The idea was to minimize code changes when switching form GLUT to SDL */

#ifndef _SDLCALLBACKS_H_
#define _SDLCALLBACKS_H_

// SDL wrapper initialization
extern void sdlInitCallbacks();

// Callback functions
extern void sdlKeyboardFunc(void (*func)(int unicode, int modifiers, int x, int y));
extern void sdlSpecialFunc(void (*func)(int key, int modifiers, int x, int y));
extern void sdlKeyboardUpFunc(void (*func)(int key, int modifiers, int x, int y));
extern void sdlSpecialUpFunc(void (*func)(int key, int modifiers, int x, int y));

extern void sdlMouseFunc(void (*func)(int button, int state, int x, int y));
extern void sdlMotionFunc(void (*func)(int x, int y));
extern void sdlPassiveMotionFunc(void (*func)(int x, int y));

extern void sdlIdleFunc(void (*func)(void));

extern void sdlDisplayFunc(void (*func)(void));
extern void sdlReshapeFunc(void (*func)(int width, int height));

// Timer functions
extern void sdlTimeDelayedFunc(unsigned int millis, void (*func)(int value));

// GUI functions
extern void sdlInitWindowPosition(int x, int y);
extern void sdlInitWindowSize(int x, int y);

extern void sdlSwapBuffers(void);
extern void sdlPostRedisplay(void);
extern void sdlForceRedisplay();

extern void sdlSetCursor(int cursor);
extern void sdlWarpPointer(int x, int y);

extern void sdlLeaveGameMode(void);

// Event management / re-display main loop
extern void sdlMainLoop(void);

#endif
