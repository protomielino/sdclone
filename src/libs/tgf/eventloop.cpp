/***************************************************************************
                               guieventloop
                             -------------------
    created              : Mon Apr 21 22:30:04 CEST 2011
    copyright            : (C) 2009 Brian Gavin, 2011 Jean-Philippe Meuret
    web                  : http://www.speed-dreams.org
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

/* Mimics the behavior of the GLUT callback system. 
   The idea was to minimize code changes when switching form GLUT to SDL */

#include <map>

#include <SDL/SDL.h>

#include "tgf.hpp"


// Private data (pimp pattern) =============================================
class GfEventLoop::Private
{
 public:

	//!  Constructor.
	Private();
	
	//! Translation function from SDL key to unicode if possible (or SDL key sym otherwise)
	int translateKeySym(int code, int modifier, int unicode);

	//! SDL-callable wrapper for the timer callback function.
	static Uint32 callTimerCB(Uint32 interval, void *pEvLoopPriv);

 public: // Public data members.

	// Callback function pointers.
	void (*cbKeyboardDown)(int key, int modifiers, int x, int y);
	void (*cbKeyboardUp)(int key, int modifiers, int x, int y);
	
	void (*cbIdle)(void);
	
	void (*cbTimer)(int value);
	
	// Variables.
	bool bQuit; // Flag to go to the end of event loop.

private: // Private data members.

	//! Initialization flag for the underlying software layers.
	static bool _bInitialized;

	//! Unicode for each typed SDL key sym + modifier
	std::map<Uint32, Uint16> _mapUnicodes;
};

//! Initialization flag for the underlying software layers.
bool GfEventLoop::Private::_bInitialized = 0;


GfEventLoop::Private::Private()
: cbKeyboardDown(0), cbKeyboardUp(0), cbIdle(0), cbTimer(0), bQuit(false)
{
	if (!_bInitialized)
	{
		SDL_EnableUNICODE(/*enable=*/1); // For keyboard "key press" event key code translation
		_bInitialized = true;
	}
}

// Translation function from SDL key to unicode if possible (or SDL key sym otherwise)
// As the unicode is not available on KEYUP events, we store it on KEYDOWN event,
// (and then, we can get it on KEYUP event, that ALWAYS come after a KEYDOWN event).
// The unicode is stored in a map that grows each time 1 new key sym + modifiers combination
// is typed ; we never clears this map, but assume not that many such combinations
// will be typed in a game session (could theorically go up to 2^23 combinations, though ;-)
// Known issues (TODO): No support for Caps and NumLock keys ... don't use them !
int GfEventLoop::Private::translateKeySym(int code, int modifier, int unicode)
{
	int keyUnicode;
	
	const Uint32 keyId = ((Uint32)code & 0x1FF) | (((Uint32)modifier) << 9);
	
    const std::map<Uint32, Uint16>::const_iterator itUnicode = _mapUnicodes.find(keyId);
    
    if (itUnicode == _mapUnicodes.end())
	{
		// Truncate unicodes above GF_MAX_KEYCODE (no need for more).
		keyUnicode = unicode ? (unicode & GF_MAX_KEYCODE) : code;
		_mapUnicodes[keyId] = keyUnicode;
		//GfLogDebug("translateKeySym: New key id=0x%08X, unicode=%d (%d)\n",
		//		   keyId, keyUnicode, _mapUnicodes.size());
	}
	else
		keyUnicode = (*itUnicode).second;

	return keyUnicode;
}

Uint32 GfEventLoop::Private::callTimerCB(Uint32 interval, void *pEvLoopPriv)
{
	Private* pPriv = reinterpret_cast<Private*>(pEvLoopPriv);
	if (pPriv->cbTimer)
		pPriv->cbTimer(1);

	// Returning zero will prevent the callback from being called again.
	return 0;
}


// GfEventLoop class ============================================================

GfEventLoop::GfEventLoop()
{
	_pPrivate = new Private;
}

GfEventLoop::~GfEventLoop()
{
	delete _pPrivate;
}

void GfEventLoop::injectKeyboardEvent(int code, int modifier, int state,
									  int unicode, int x, int y)
{
	if (state == 0)
	{
		if (_pPrivate->cbKeyboardDown)
			_pPrivate->cbKeyboardDown(_pPrivate->translateKeySym(code, modifier, unicode),
									  modifier, x, y);
	}
	else
	{
		if (_pPrivate->cbKeyboardUp)
			_pPrivate->cbKeyboardUp(_pPrivate->translateKeySym(code, modifier, unicode),
									modifier, x, y);
	}
}

// The event loop itself.
void GfEventLoop::operator()()
{
	SDL_Event event; // Event structure
	
	// Check for events.
	while (!_pPrivate->bQuit)
	{  	
		// Loop until there are no events left in the queue.
		while (!_pPrivate->bQuit && SDL_PollEvent(&event))
		{
		    // Process events we care about, and ignore the others.
			switch(event.type)
			{  
				case SDL_KEYDOWN:
					injectKeyboardEvent(event.key.keysym.sym, event.key.keysym.mod, 0,
										event.key.keysym.unicode);
					break;
				
				case SDL_KEYUP:
					injectKeyboardEvent(event.key.keysym.sym, event.key.keysym.mod, 1,
										event.key.keysym.unicode);
					break;

				case SDL_QUIT:
					postQuit();
					break;

				default:
					break;
			}
		}

		if (!_pPrivate->bQuit)
		{
			// Spend idle time as requested.
			spendIdleTime();
		}
	}

	GfLogTrace("Quitting event loop.\n");
}

void GfEventLoop::setKeyboardDownCB(void (*func)(int key, int modifiers, int x, int y))
{
	_pPrivate->cbKeyboardDown = func;
}

void GfEventLoop::setKeyboardUpCB(void (*func)(int key, int modifiers, int x, int y))
{
	_pPrivate->cbKeyboardUp = func;
}

void GfEventLoop::setIdleCB(void (*func)(void))
{
	_pPrivate->cbIdle = func;
}

void GfEventLoop::setTimerCB(unsigned int millis, void (*func)(int value))
{
	_pPrivate->cbTimer = func;
	SDL_AddTimer(millis, Private::callTimerCB, (void*)_pPrivate);
}

void GfEventLoop::postQuit()
{
	_pPrivate->bQuit = true;
}

bool GfEventLoop::quitRequested() const
{
	return _pPrivate->bQuit;
}

void GfEventLoop::spendIdleTime()
{
	// Call Idle callback if any.
	if (_pPrivate->cbIdle)
		_pPrivate->cbIdle();
	
	// ... otherwise let CPU take breath (and fans stay at low and quiet speed)
	else
		SDL_Delay(1); // ms.
}
