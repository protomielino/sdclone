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

#include "tgf.h"
#include "tgfclient.h"


/** The event loop class
    
     @ingroup	gui
*/

class EventLoop
{
  public: // Member functions.

	//! Constructor
	EventLoop();

	//! The real event loop function.
	void operator()(void);

	//! Set the "key pressed" callback function.
	void setKeyboardDownCB(void (*func)(int key, int modifiers, int x, int y));

	//! Set the "key realeased" callback function.
	void setKeyboardUpCB(void (*func)(int key, int modifiers, int x, int y));

	//! Set the "mouse button pressed" callback function.
	void setMouseButtonCB(void (*func)(int button, int state, int x, int y));

	//! Set the "mouse motion with button pressed" callback function.
	void setMouseMotionCB(void (*func)(int x, int y));

	//! Set the "mouse motion without button pressed" callback function.
	void setMousePassiveMotionCB(void (*func)(int x, int y));

	//! Set the "idle" callback function (run at the end of _every_ event loop).
	void setIdleCB(void (*func)(void));

	//! Set a timer callback function with given delay.
	void setTimerCB(unsigned int millis, void (*func)(int value));

	//! Set the "redisplay/refresh" callback function. 
	void setDisplayCB(void (*func)(void));

	//! Set the "reshape" callback function with given new screen/window geometry.
	void setReshapeCB(void (*func)(int width, int height));

	//! Post a "redisplay/refresh" event to the event loop. 
	void postReDisplay(void);

	//! Force a call to the "redisplay/refresh" callback function. 
	void forceReDisplay();

	//! Request the event loop to terminate on next loop. 
	void quit();

  private: // Member functions.

	//! Translation function from SDL key to unicode if possible (or SDL key sym otherwise)
	int translateKeySym(const SDL_keysym& sdlKeySym);

	//! Call the timer callback function (time has come).
	void callTimerCB(int value);

	//! SDL-callable wrapper for the timer callback function.
	static Uint32 wrapTimerCB(Uint32 interval, void *param);

  private: // Member data.
	
	// Callback functions
	void (*_keyboardDownCB)(int key, int modifiers, int x, int y);
	void (*_keyboardUpCB)(int key, int modifiers, int x, int y);
	
	void (*_mouseButtonCB)(int button, int state, int x, int y);
	void (*_mouseMotionCB)(int x, int y);
	void (*_mousePassiveMotionCB)(int x, int y);
	
	void (*_idleCB)(void);
	
	void (*_timerCB)(int value);
	
	void (*_displayCB)(void);
	void (*_reshapeCB)(int width, int height);

	// Variables
	bool _bQuit; // Flag to go to the end of event loop.
	bool _bReDisplay; // Flag to say if a redisplay is necessary.
	std::map<Uint32, Uint16> _mapUnicodes; // Unicode for each typed SDL key sym + modifier
};

/** Accessor to the EventLoop singleton
	
	Implemented that way in order to guarantee everything static is initialized
	before being first used.

	Example of classic implementation that doesn't work with some compilers
	like for ... kakuri ? (but known to work with Linux GCC 4.4, MSVC 2005, MSVC 2008):
	
	static std::map<Uint32, Uint16> g_mapUnicodes;
	int getKeyUnicode(const SDL_keysym& sdlKeySym)
	{
	    // Here, nothing in C++ guarantees that the map constructor has been called.
	    const std::map<Uint32, Uint16>::const_iterator itUnicode =
		    g_mapUnicodes.find(sdlKeySym.keysym);

		...
	}

     @ingroup	gui
*/

static EventLoop& 
gfelEventLoop()
{
	static EventLoop eventLoop;
	return eventLoop;
}


// Constructor
EventLoop::EventLoop()
{
	_displayCB = 0;
	_reshapeCB = 0;
	_keyboardDownCB = 0;
	_mouseButtonCB = 0;
	_mouseMotionCB = 0;
	_mousePassiveMotionCB = 0;
	_idleCB = 0;
	_timerCB = 0;
	_keyboardUpCB = 0;

	_bQuit = false;
	
	_bReDisplay = false;
	
	SDL_EnableUNICODE(/*enable=*/1); // For keyboard "key press" event key code translation
}

void EventLoop::setKeyboardDownCB(void (*func)(int key, int modifiers, int x, int y))
{
	_keyboardDownCB = func;
}

void EventLoop::setKeyboardUpCB(void (*func)(int key, int modifiers, int x, int y))
{
	_keyboardUpCB = func;
}

void EventLoop::setMouseButtonCB(void (*func)(int button, int state, int x, int y))
{
	_mouseButtonCB = func;
}

void EventLoop::setMouseMotionCB(void (*func)(int x, int y))
{
	_mouseMotionCB = func;
}

void EventLoop::setMousePassiveMotionCB(void (*func)(int x, int y))
{
	_mousePassiveMotionCB = func;
}

void EventLoop::setIdleCB(void (*func)(void))
{
	_idleCB = func;
}

Uint32 EventLoop::wrapTimerCB(Uint32 interval, void *param)
{
	gfelEventLoop().callTimerCB(1);

	//Returning zero will prevent callback from happening again
	return 0;
}

void EventLoop::setTimerCB(unsigned int millis, void (*func)(int value))
{
	_timerCB = func;
	SDL_AddTimer(millis, EventLoop::wrapTimerCB, (void *)1);
}

void EventLoop::callTimerCB(int value)
{
	if (_timerCB)
		_timerCB(value);
}

void EventLoop::setDisplayCB(void (*func)(void))
{
	_displayCB = func;
}

void EventLoop::setReshapeCB(void (*func)(int width, int height))
{
	_reshapeCB = func;
}

void EventLoop::postReDisplay(void)
{
	_bReDisplay = true;
}

void EventLoop::forceReDisplay()
{
	if (_displayCB)
		_displayCB();
}

void EventLoop::quit()
{
	_bQuit = true;
}

// Translation function from SDL key to unicode if possible (or SDL key sym otherwise)
// As the unicode is not available on KEYUP events, we store it on KEYDOWN event,
// (and then, we can get it on KEYUP event, that ALWAYS come after a KEYDOWN event).
// The unicode is stored in a map that grows each time 1 new key sym + modifiers combination
// is typed ; we never clears this map, but assume not that many such combinations
// will be typed in a game session (could theorically go up to 2^23 combinations, though ;-)
// Known issues (TODO): No support for Caps and NumLock keys ... don't use them !
int EventLoop::translateKeySym(const SDL_keysym& sdlKeySym)
{
	int keyUnicode;
	
	const Uint32 keyId = ((Uint32)sdlKeySym.sym & 0x1FF) | (((Uint32)sdlKeySym.mod) << 9);
	
    const std::map<Uint32, Uint16>::const_iterator itUnicode = _mapUnicodes.find(keyId);
    
    if (itUnicode == _mapUnicodes.end())
	{
		// Truncate unicodes above GFUIK_MAX (no need for more).
		keyUnicode = sdlKeySym.unicode ? (sdlKeySym.unicode & GFUIK_MAX) : sdlKeySym.sym;
		_mapUnicodes[keyId] = keyUnicode;
		//GfOut("EventLoop: New key id=0x%08X, unicode=%d (%d)\n", keyId, keyUnicode, _mapUnicodes.size());
	}
	else
		keyUnicode = (*itUnicode).second;

	return keyUnicode;
}


// The main event management / re-display loop
void EventLoop::operator()(void)
{
	SDL_Event event; // Event structure
	
	// Check for events
	while (!_bQuit)
	{  	
		// Loop until there are no events left on the queue
		while (!_bQuit && SDL_PollEvent(&event))
		{
		    // Process the appropiate event type
			switch(event.type)
			{  
				case SDL_KEYDOWN:
#ifndef WIN32
					// Hard-coded Alt+Enter shortcut, to enable the user to quit/re-enter
					// the full-screen mode ; as in SDL full screen mode, events never traverse
					// the Window Manager, we need this trick for the user to enjoy
					// its WM keyboard shortcuts (didn't find any other way yet).
					if (event.key.keysym.sym == SDLK_RETURN	&& SDL_GetModState() & KMOD_ALT)
					{
						if (!SDL_WM_ToggleFullScreen(gfScrGetScreenSurface()))
							GfLogError("SDL_WM_ToggleFullScreen failed\n");
					}
					else
#endif
					if (_keyboardDownCB)
					{
						int x,y;
						SDL_GetMouseState(&x, &y);
						_keyboardDownCB(translateKeySym(event.key.keysym),
										event.key.keysym.mod, x, y);

						//GfOut("EventLoop: KeyDown(k=%d, m=%x)\n", 
						//	  event.key.keysym.sym, event.key.keysym.mod);
					}
					break;
				
				case SDL_KEYUP:
					
					if (_keyboardUpCB)
					{
						int x,y;
						SDL_GetMouseState(&x, &y);
						_keyboardUpCB(translateKeySym(event.key.keysym),
									  event.key.keysym.mod, x, y);
					}
					break;

				case SDL_MOUSEMOTION:

					if (event.motion.state == 0)
					{
						if (_mousePassiveMotionCB)
							_mousePassiveMotionCB(event.motion.x, event.motion.y);
					}
					else
					{
						if (_mouseMotionCB)
							_mouseMotionCB(event.motion.x, event.motion.y);
					}
					break;

				
				case SDL_MOUSEBUTTONDOWN:

					if (_mouseButtonCB)
						_mouseButtonCB(event.button.button, event.button.state,
										event.button.x, event.button.y);
					break;

				case SDL_MOUSEBUTTONUP:

					if (_mouseButtonCB)
						_mouseButtonCB(event.button.button, event.button.state,
										event.button.x, event.button.y);
					break;

				case SDL_QUIT:

					_bQuit = true;
					break;
				
				case SDL_VIDEOEXPOSE:

					if (_displayCB)
						_displayCB();
					break;
			}
		}

		if (!_bQuit)
		{
			// Refresh display if needed
			if (_bReDisplay)
			{
				_bReDisplay = false;
				if (_displayCB)
					_displayCB();
			}
			
			// Call Idle callback if any
			if (_idleCB)
				_idleCB();
			
			// ... otherwise let CPU take breath (and fans stay at low and quiet speed)
			else
				SDL_Delay(1); // ms.
		}
	}

	GfLogTrace("Quitting event loop.\n");
}

/** Initialize the event loop management layer
    
     @ingroup	gui
*/

void 
GfelInitialize()
{
}

/** Set the call-back for "key pressed" events
    
     @ingroup	gui
*/

void 
GfelSetKeyboardDownCB(void (*func)(int key, int modifiers, int x, int y))
{
	gfelEventLoop().setKeyboardDownCB(func);
}

/** Set the call-back for "key released" events
    
     @ingroup	gui
*/

void 
GfelSetKeyboardUpCB(void (*func)(int key, int modifiers, int x, int y))
{
	gfelEventLoop().setKeyboardUpCB(func);
}


/** Set the call-back for "mouse button/wheel state change" events
    
     @ingroup	gui
*/

void 
GfelSetMouseButtonCB(void (*func)(int button, int state, int x, int y))
{
	gfelEventLoop().setMouseButtonCB(func);
}

/** Set the call-back for "mouse move while button pressing" events
    
     @ingroup	gui
*/

void 
GfelSetMouseMotionCB(void (*func)(int x, int y))
{
	gfelEventLoop().setMouseMotionCB(func);
}

/** Set the call-back for "mouse move without button pressing" events
    
     @ingroup	gui
*/

void 
GfelSetMousePassiveMotionCB(void (*func)(int x, int y))
{
	gfelEventLoop().setMousePassiveMotionCB(func);
}

/** Set the function to be called when nothing to do
    
     @ingroup	gui
*/

void 
GfelSetIdleCB(void (*func)(void))
{
	gfelEventLoop().setIdleCB(func);
}

/** Initialize a timer and the associated time-out call-back function
    
     @ingroup	gui
*/

void 
GfelSetTimerCB(unsigned int millis, void (*func)(int value))
{
	gfelEventLoop().setTimerCB(millis, func);
}

/** Set the function to call for re-displaying the screen
    
     @ingroup	gui
*/

void 
GfelSetDisplayCB(void (*func)(void))
{
	gfelEventLoop().setDisplayCB(func);
}

/** Set the function to call for re-shaping
    
     @ingroup	gui
*/

void 
GfelSetReshapeCB(void (*func)(int width, int height))
{
	gfelEventLoop().setReshapeCB(func);
}

/** State that a redisplay is to be done in next main loop
    
     @ingroup	gui
*/

void 
GfelPostRedisplay(void)
{
	gfelEventLoop().postReDisplay();
}

/** Force a redisplay now, without waiting for the main loop to do so
    
     @ingroup	gui
*/

void 
GfelForceRedisplay()
{
	gfelEventLoop().forceReDisplay();
}


/** The main event management / re-display loop
    
     @ingroup	gui
*/

void 
GfelMainLoop(void)
{
	gfelEventLoop()();
}

/** Request the event loop to terminate on next loop
    
     @ingroup	gui
*/

void 
GfelQuit()
{
	gfelEventLoop().quit();
}


