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

#include <stdio.h>
#include <stdlib.h>

#include <SDL/SDL.h>
#include "sdlcallbacks.h"
#include "network.h"

// Globals
// Callback functions
void (*g_keyboardFuncCB)(int unicode, int modifiers, int x, int y);
void (*g_specialFuncCB)(int key, int modifiers, int x, int y);
void (*g_keyboardupFuncCB)(int key, int modifiers, int x, int y);
void (*g_specialupFuncCB)(int key, int modifiers, int x, int y);

void (*g_mouseFuncCB)(int button, int state, int x, int y);
void (*g_motionFuncCB)(int x, int y);
void (*g_passivemotionFuncCB)(int x, int y);

void (*g_idleFuncCB)(void);

void (*g_timerFuncCB)(int value);

void (*g_displayFuncCB)(void);
void (*g_reshapeFuncCB)(int width, int height);

// Variables
int g_bReDisplay; // Flag to say if a redisplay is necessary.
SDL_Surface *g_OpenGLSurface; // The SDL OpenGL surface.
Uint16 g_lastKeyDownUnicode; // The unicode of the last KEYDOWN event (used for KEYUP events).

/** Initialize SDL wrapper
    
     @ingroup	sdl
*/

void 
sdlInitCallbacks()
{
	g_displayFuncCB = 0;
	g_reshapeFuncCB = 0;
	g_keyboardFuncCB = 0;
	g_mouseFuncCB = 0;
	g_motionFuncCB = 0;
	g_passivemotionFuncCB = 0;
	g_idleFuncCB = 0;
	g_timerFuncCB = 0;
	g_specialFuncCB = 0;
	g_keyboardupFuncCB = 0;
	g_specialupFuncCB = 0;

	g_bReDisplay = 0;
	g_OpenGLSurface = 0;

	g_lastKeyDownUnicode = 0;

	SDL_EnableUNICODE(/*enable=*/1); /* For keyboard "key press" event key code translation */

}

/** Set the call-back for "key pressed" events
    
     @ingroup	sdl
*/

void 
sdlKeyboardFunc(void (*func)(int unicode, int modifiers, int x, int y))
{
	g_keyboardFuncCB = func;
}

/** Set the call-back for "special key pressed" events
    
     @ingroup	sdl
*/

void 
sdlSpecialFunc(void (*func)(int key, int modifiers, int x, int y))
{
	g_specialFuncCB = func;
}


/** Set the call-back for "key released" events
    
     @ingroup	sdl
*/

void 
sdlKeyboardUpFunc(void (*func)(int key, int modifiers, int x, int y))
{
	g_keyboardupFuncCB = func;
}


/** Set the call-back for "special key released" events
    
     @ingroup	sdl
*/

void 
sdlSpecialUpFunc(void (*func)(int key, int modifiers, int x, int y))
{
	g_specialupFuncCB = func;
}

/** Set the call-back for "mouse button/wheel state change" events
    
     @ingroup	sdl
*/

void 
sdlMouseFunc(void (*func)(int button, int state, int x, int y))
{
	g_mouseFuncCB = func;
}

/** Set the call-back for "mouse move while button pressing" events
    
     @ingroup	sdl
*/

void 
sdlMotionFunc(void (*func)(int x, int y))
{
	g_motionFuncCB = func;
}

/** Set the call-back for "mouse move without button pressing" events
    
     @ingroup	sdl
*/

void 
sdlPassiveMotionFunc(void (*func)(int x, int y))
{
	g_passivemotionFuncCB = func;
}

/** Set the function to be called when nothing to do
    
     @ingroup	sdl
*/

void 
sdlIdleFunc(void (*func)(void))
{
	g_idleFuncCB = func;
}

/** SDL wrapper function for a time-out call-back function (see sdlTimeDelayedFunc).
    
     @ingroup	sdl
*/

static Uint32 
timercallback(Uint32 interval, void *param)
{
	if (g_timerFuncCB)
	{
	  g_timerFuncCB(1);
	}

	//Returning zero will prevent callback from happening again
	return 0;
}

/** Initialize a timer and the associated time-out call-back function
    
     @ingroup	sdl
*/

void 
sdlTimeDelayedFunc(unsigned int millis, void (*func)(int value))
{
	g_timerFuncCB = func;
	SDL_AddTimer(millis, timercallback, (void *)1);
}

/** Set the function to call for re-displaying the screen
    
     @ingroup	sdl
*/

void 
sdlDisplayFunc(void (*func)(void))
{
	g_displayFuncCB = func;
}

/** Set the function to call for re-shaping
    
     @ingroup	sdl
*/

void 
sdlReshapeFunc(void (*func)(int width, int height))
{
	g_reshapeFuncCB = func;
}

/** Initialize window position
    
     @ingroup	sdl
*/

void 
sdlInitWindowPosition(int x, int y)
{

}

/** Initialize window size
    
     @ingroup	sdl
*/

void 
sdlInitWindowSize(int x, int y)
{

}


/** Swap display buffers (double buffering)
    
     @ingroup	sdl
*/

void 
sdlSwapBuffers(void)
{
	SDL_GL_SwapBuffers();
}

/** State that a redisplay is to be done in next main loop
    
     @ingroup	sdl
*/

void 
sdlPostRedisplay(void)
{
	g_bReDisplay = 1;
}

/** Force a redisplay now, without waiting for the main loop to do so
    
     @ingroup	sdl
*/

void 
sdlForceRedisplay()
{
	if (g_displayFuncCB)
	{
		g_displayFuncCB();
	}
}

/** Set SDL OpenGL surface
    
     @ingroup	sdl
*/

void 
sdlSetOpenGLSurface(SDL_Surface *surface)
{
	g_OpenGLSurface = surface;
}


/** Get current SDL OpenGL surface
    
     @ingroup	sdl
*/

SDL_Surface * 
sdlGetOpenGLSurface()
{
	return g_OpenGLSurface;
}

/** Set mouse cursor visibility
    
     @ingroup	sdl
*/

void 
sdlSetCursor(int on)
{
	SDL_ShowCursor (on ? SDL_ENABLE : SDL_DISABLE);
}

/** Force the position of the mouse cursor
    
     @ingroup	sdl
*/

void 
sdlWarpPointer(int x, int y)
{
	SDL_WarpMouse(x,y);
}

/** Leave game mode
    
     @ingroup	sdl
*/

void 
sdlLeaveGameMode(void)
{
	//Do nothing
}


/** The main event management / re-display loop
    
     @ingroup	sdl
*/

void 
sdlMainLoop(void)
{

	int bEnd = 0;

	SDL_Event event; /* Event structure */
	/* Check for events */
	while(!bEnd)
	{  	
		/* Loop until there are no events left on the queue */
		while (SDL_PollEvent(&event))
		{
		    /* Process the appropiate event type */
			switch(event.type)
			{  
				/* Handle a KEYDOWN event :
				   Special keys are those for which no unicode is generated */
				case SDL_KEYDOWN:
					if (g_keyboardFuncCB)
					{
						int x,y;
						SDL_GetMouseState(&x, &y);

						// Save key info for next associated key up event
						g_lastKeyDownUnicode = event.key.keysym.unicode;
						//GfOut("sdlMainLoop: KeyDown(k=%d, u=%d, m=%x)\n", 
						//	   event.key.keysym.sym, event.key.keysym.unicode,
						//	   event.key.keysym.mod);

						if (g_lastKeyDownUnicode)
							g_keyboardFuncCB(g_lastKeyDownUnicode,
											 event.key.keysym.mod, x, y);
						else if (g_specialFuncCB)
							g_specialFuncCB(event.key.keysym.sym,
											event.key.keysym.mod, x, y);
					}
				break;
				
				/* Handle a KEYUP event :
				   Special keys are those for which no unicode was generated
				   on associated preceding KeyDown event */
				case SDL_KEYUP:
					if (g_keyboardupFuncCB)
					{
						int x,y;
						SDL_GetMouseState(&x, &y);
						if (g_lastKeyDownUnicode)
							g_keyboardupFuncCB(g_lastKeyDownUnicode,
											   event.key.keysym.mod, x, y);
						else if (g_specialupFuncCB)
							g_specialupFuncCB(event.key.keysym.sym,
											  event.key.keysym.mod, x, y);
					}
				break;

				case SDL_MOUSEMOTION:
				{
					if (event.motion.state == 0)
					{
						if (g_passivemotionFuncCB)
						{
							g_passivemotionFuncCB(event.motion.x,event.motion.y);
						}
					}
					else
					{
						if (g_motionFuncCB)
						{
							g_motionFuncCB(event.motion.x,event.motion.y);
						}
					}
					break;
				}
				
				case SDL_MOUSEBUTTONDOWN:
				{
					if (g_mouseFuncCB)
					{
						g_mouseFuncCB(event.button.button,event.button.state,event.button.x,event.button.y);
					}
					break;
				}

				case SDL_MOUSEBUTTONUP:
				{
					if (g_mouseFuncCB)
					{
						g_mouseFuncCB(event.button.button,event.button.state,event.button.x,event.button.y);
					}

					break;
				}

				case SDL_QUIT:
					bEnd = 1;
					exit(0);
				break;
				
				case SDL_VIDEOEXPOSE:
					//Redraw the screen
					if (g_displayFuncCB)
					{
						g_displayFuncCB();
					}
				break;

			}
		}

		// Refresh display if needed
		if (g_bReDisplay)
		{
			g_bReDisplay = 0;
			if (g_displayFuncCB)
			{
				g_displayFuncCB();
			}
		}

		// Call Idle function if any
		if (g_idleFuncCB)
			g_idleFuncCB();

		// ... otherwise let CPU take breath (and fans stay at low and quiet speed)
		else
			SDL_Delay(1); // ms.
	}


}

