/***************************************************************************
                               guieventloop
                             -------------------                                         
    created              : Thu Mar 8 10:00:00 CEST 2006
    copyright            : (C) 2006 by Brian Gavin ; 2008, 2010 Jean-Philippe Meuret
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/* Mimics the behavior of the GLUT callback system. 
   The idea was to minimize code changes when switching form GLUT to SDL */

#include <map>

#include <SDL/SDL.h>

#include "tgfclient.h"


// Globals
// * Callback functions
static void (*g_keyboardDownCB)(int key, int modifiers, int x, int y);
static void (*g_keyboardUpCB)(int key, int modifiers, int x, int y);

static void (*g_mouseButtonCB)(int button, int state, int x, int y);
static void (*g_mouseMotionCB)(int x, int y);
static void (*g_MousePassiveMotionCB)(int x, int y);

static void (*g_idleCB)(void);

static void (*g_timerCB)(int value);

static void (*g_displayCB)(void);
static void (*g_reshapeCB)(int width, int height);

// * Variables
static bool g_bReDisplay; // Flag to say if a redisplay is necessary.
//Uint16 g_keyUnicode[SDLK_LAST]; // Unicode for each possible SDL key sym
static std::map<Uint32, Uint16> g_keyUnicode; // Unicode for each typed SDL key sym + modifier

/** Initialize the event loop management layer
    
     @ingroup	gui
*/

void 
GfelInitialize()
{
	g_displayCB = 0;
	g_reshapeCB = 0;
	g_keyboardDownCB = 0;
	g_mouseButtonCB = 0;
	g_mouseMotionCB = 0;
	g_MousePassiveMotionCB = 0;
	g_idleCB = 0;
	g_timerCB = 0;
	g_keyboardUpCB = 0;

	g_bReDisplay = false;

	SDL_EnableUNICODE(/*enable=*/1); /* For keyboard "key press" event key code translation */
	for (int keySym = 0; keySym < SDLK_LAST; keySym++)
		g_keyUnicode[keySym] = 0;
}

/** Set the call-back for "key pressed" events
    
     @ingroup	gui
*/

void 
GfelSetKeyboardDownCB(void (*func)(int key, int modifiers, int x, int y))
{
	g_keyboardDownCB = func;
}

/** Set the call-back for "key released" events
    
     @ingroup	gui
*/

void 
GfelSetKeyboardUpCB(void (*func)(int key, int modifiers, int x, int y))
{
	g_keyboardUpCB = func;
}


/** Set the call-back for "mouse button/wheel state change" events
    
     @ingroup	gui
*/

void 
GfelSetMouseButtonCB(void (*func)(int button, int state, int x, int y))
{
	g_mouseButtonCB = func;
}

/** Set the call-back for "mouse move while button pressing" events
    
     @ingroup	gui
*/

void 
GfelSetMouseMotionCB(void (*func)(int x, int y))
{
	g_mouseMotionCB = func;
}

/** Set the call-back for "mouse move without button pressing" events
    
     @ingroup	gui
*/

void 
GfelSetMousePassiveMotionCB(void (*func)(int x, int y))
{
	g_MousePassiveMotionCB = func;
}

/** Set the function to be called when nothing to do
    
     @ingroup	gui
*/

void 
GfelSetIdleCB(void (*func)(void))
{
	g_idleCB = func;
}

/** SDL wrapper function for a timed-out call-back function (see GfelSetTimeDelayedCB).
    
     @ingroup	gui
*/

static Uint32 
wrapTimerCB(Uint32 interval, void *param)
{
	if (g_timerCB)
	  g_timerCB(1);

	//Returning zero will prevent callback from happening again
	return 0;
}

/** Initialize a timer and the associated time-out call-back function
    
     @ingroup	gui
*/

void 
GfelSetTimerCB(unsigned int millis, void (*func)(int value))
{
	g_timerCB = func;
	SDL_AddTimer(millis, wrapTimerCB, (void *)1);
}

/** Set the function to call for re-displaying the screen
    
     @ingroup	gui
*/

void 
GfelSetDisplayCB(void (*func)(void))
{
	g_displayCB = func;
}

/** Set the function to call for re-shaping
    
     @ingroup	gui
*/

void 
GfelSetReshapeCB(void (*func)(int width, int height))
{
	g_reshapeCB = func;
}

/** State that a redisplay is to be done in next main loop
    
     @ingroup	gui
*/

void 
GfelPostRedisplay(void)
{
	g_bReDisplay = true;
}

/** Force a redisplay now, without waiting for the main loop to do so
    
     @ingroup	gui
*/

void 
GfelForceRedisplay()
{
	if (g_displayCB)
		g_displayCB();
}


// Translation function from SDL key to unicode if possible (of SDL key sym otherwise)
// As the unicode is not available on KEYUP events, we store it on KEYDOWN event,
// (and then, we can get it on KEYUP event, that ALWAYS come after a KEYDOWN event).
// The unicode is stored in a map that grows each time 1 new key sym + modifiers combination
// is typed ; we never clears this map, but assume not that many such combinations
// will be typed in a game session (could theorically go up to 2^21 combinations, though ;-)
// Known issues (TODO): No support for Caps and NumLock keys ... don't use them !
static int
translateKeySym(const SDL_keysym& sdlKeySym)
{
	int keyUnicode;
	
	const Uint32 keyId = ((Uint32)sdlKeySym.sym) | (((Uint32)sdlKeySym.mod) << 16);
	
    const std::map<Uint32, Uint16>::const_iterator itUnicode = g_keyUnicode.find(keyId);
    
    if (itUnicode == g_keyUnicode.end())
	{
		keyUnicode = sdlKeySym.unicode ? sdlKeySym.unicode : sdlKeySym.sym;
        g_keyUnicode[keyId] = keyUnicode;
		//GfOut("New key 0x%08X (%d)\n", keyId, g_keyUnicode.size());
	}
	else
		keyUnicode = (*itUnicode).second;

	return keyUnicode;
}


/** The main event management / re-display loop
    
     @ingroup	gui
*/

void 
GfelMainLoop(void)
{
	SDL_Event event; /* Event structure */
	
	/* Check for events */
	while(true)
	{  	
		/* Loop until there are no events left on the queue */
		while (SDL_PollEvent(&event))
		{
		    /* Process the appropiate event type */
			switch(event.type)
			{  
				case SDL_KEYDOWN:
					
					if (g_keyboardDownCB)
					{
						int x,y;
						SDL_GetMouseState(&x, &y);
						g_keyboardDownCB(translateKeySym(event.key.keysym),
										 event.key.keysym.mod, x, y);

						//GfOut("GfelMainLoop: KeyDown(k=%d, m=%x)\n", 
						//	  event.key.keysym.sym, event.key.keysym.mod);
					}
					break;
				
				case SDL_KEYUP:
					
					if (g_keyboardUpCB)
					{
						int x,y;
						SDL_GetMouseState(&x, &y);
						g_keyboardUpCB(translateKeySym(event.key.keysym),
									   event.key.keysym.mod, x, y);
					}
					break;

				case SDL_MOUSEMOTION:

					if (event.motion.state == 0)
					{
						if (g_MousePassiveMotionCB)
						{
							g_MousePassiveMotionCB(event.motion.x, event.motion.y);
						}
					}
					else
					{
						if (g_mouseMotionCB)
						{
							g_mouseMotionCB(event.motion.x, event.motion.y);
						}
					}
					break;

				
				case SDL_MOUSEBUTTONDOWN:

					if (g_mouseButtonCB)
					{
						g_mouseButtonCB(event.button.button, event.button.state,
										event.button.x, event.button.y);
					}
					break;

				case SDL_MOUSEBUTTONUP:

					if (g_mouseButtonCB)
					{
						g_mouseButtonCB(event.button.button, event.button.state,
										event.button.x, event.button.y);
					}

					break;

				case SDL_QUIT:
					
					exit(0);
					break;
				
				case SDL_VIDEOEXPOSE:

					if (g_displayCB)
					{
						g_displayCB();
					}
					break;
			}
		}

		// Refresh display if needed
		if (g_bReDisplay)
		{
			g_bReDisplay = false;
			if (g_displayCB)
			{
				g_displayCB();
			}
		}

		// Call Idle callback if any
		if (g_idleCB)
			g_idleCB();

		// ... otherwise let CPU take breath (and fans stay at low and quiet speed)
		else
			SDL_Delay(1); // ms.
	}


}

