/***************************************************************************
                               guieventloop
                             -------------------
    created              : Mon Apr 21 22:30:04 CEST 2011
    copyright            : (C) 2009 Brian Gavin, 2011 Jean-Philippe Meuret
    web                  : http://www.speed-dreams.org
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

#include <SDL.h>

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

    void (*cbRecompute)(void);
    void (*cbRecomputeArgsFn)(unsigned ms, void *args);
    void *cbRecomputeArgsData;

    void (*cbTimer)(int value);

    // Variables.
    bool bQuit; // Flag to go to the end of event loop.

    //! Current "locked" key modifiers.		// Caps-Lock bug
    int nLockedModifiers;

private: // Private data members.

    //! Initialization flag for the underlying software layers.
    static bool _bInitialized;

    //! Unicode for each typed SDL key sym + modifier
    std::map<Uint32, Uint16> _mapUnicodes;
};

GfEventLoop::Private::Private() :
    cbKeyboardDown(nullptr),
    cbKeyboardUp(nullptr),
    cbRecompute(nullptr),
    cbRecomputeArgsFn(nullptr),
    cbRecomputeArgsData(nullptr),
    cbTimer(nullptr),
    bQuit(false),
    nLockedModifiers(KMOD_NONE)
{
}

// Translation function from SDL key to unicode if possible (or SDL key sym otherwise)
// As the unicode is not available on KEYUP events, we store it on KEYDOWN event,
// (and then, we can get it on KEYUP event, that ALWAYS come after a KEYDOWN event).
// The unicode is stored in a map that grows each time 1 new key sym + modifiers combination
// is typed ; we never clears this map, but assume not that many such combinations
// will be typed in a game session (could theorically go up to 2^23 combinations, though ;-)
// Known issues (TODO): No support for Caps and NumLock keys ... don't use them !

// As of SDL2, this translation to unicode is no longer needed. SDL2 has unicode support
// It can now be used to translate keycodes to other keycodes. See SDLK_KP_ENTER
// below, we do not want to treat the two <Enter> keys as distinct.
int GfEventLoop::Private::translateKeySym(int code, int modifier, int unicode)
{
    int keyUnicode = code; //default to returning code

    // Make the Numpad <Enter> key behave like the regular <Enter> key
    if(SDLK_KP_ENTER == code)
        keyUnicode = SDLK_RETURN;

    else
    {
        const Uint32 keyId = ((Uint32)code & GF_MAX_KEYCODE) | (((Uint32)modifier) << 9);

        // If unicode - add to the map...
        if(unicode)
        {
            // Truncate unicodes above GF_MAX_KEYCODE (no need for more).
            keyUnicode = (unsigned short)(unicode & GF_MAX_KEYCODE);
            _mapUnicodes[keyId] = (unsigned short)keyUnicode;
            GfLogDebug("translateKeySym(c=%X, m=%X, u=%X) : '%c', id=%X, ucode=%X (nk=%zu)\n",
                   code, modifier, unicode, // Truncate high bits for MSVC 2010 bugs.
                   (keyUnicode > 0 && keyUnicode < 128 && isprint(keyUnicode & 0x7F))
                   ? (char)(keyUnicode & 0x7F) : ' ',
                   keyId, keyUnicode, _mapUnicodes.size());
        }
        else
        {
            // Search it in our unicode map.
            const std::map<Uint32, Uint16>::const_iterator itUnicode = _mapUnicodes.find(keyId);
            if (itUnicode != _mapUnicodes.end())
            {
                keyUnicode = (*itUnicode).second;
            }
        }
    }

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
    // Update locked modifiers state, as SDL doesn't always do it. // Caps-Lock bug
    switch (code)
    {
    case SDLK_CAPSLOCK:
        _pPrivate->nLockedModifiers ^= KMOD_CAPS;
        GfLogDebug("injectKeyboardEvent(c=%X) : lockedMod=%X (SDL says %X)\n",
                   code, _pPrivate->nLockedModifiers, SDL_GetModState());
        return;

    case SDLK_NUMLOCKCLEAR:
        _pPrivate->nLockedModifiers ^= KMOD_NUM;
        GfLogDebug("injectKeyboardEvent(c=%X) : lockedMod=%X (SDL says %X)\n",
                   code, _pPrivate->nLockedModifiers, SDL_GetModState());
        return;
    }
    // Translate KMOD_RXX modifiers to KMOD_LXX (we make no difference)
    // and ignore modifiers other than SHIFT, ALT, CTRL and META.
    if (modifier)
    {
        if (modifier & KMOD_RSHIFT) modifier |= KMOD_LSHIFT;
        if (modifier & KMOD_RCTRL) modifier |= KMOD_LCTRL;
        if (modifier & KMOD_RALT) modifier |= KMOD_LALT;
        if (modifier & KMOD_RGUI) modifier |= KMOD_LGUI;

        modifier &= (KMOD_LSHIFT | KMOD_LCTRL | KMOD_LALT | KMOD_LGUI);
    }

    // Toggle the Shift modifier if the Caps-Lock key is on. // Caps-Lock bug
    if (_pPrivate->nLockedModifiers & KMOD_CAPS)
    {
        GfLogDebug("injectKeyboardEvent(c=%X, m=%X, u=%X)",
                   code, modifier, unicode);
        modifier ^= KMOD_LSHIFT;
        GfLogDebug(" => m=%X\n", modifier);
    }

    // Call the relevant call-back funtion if any registered.
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
                    injectKeyboardEvent(event.key.keysym.sym, event.key.keysym.mod, 0, 0);
                    break;

                case SDL_KEYUP:
                    injectKeyboardEvent(event.key.keysym.sym, event.key.keysym.mod, 1, 0);
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
            // Recompute if anything to.
            recompute(0);
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

void GfEventLoop::setRecomputeCB(void (*func)(void))
{
    _pPrivate->cbRecompute = func;
}

void GfEventLoop::setRecomputeCB(void (*func)(unsigned, void *), void *args)
{
    _pPrivate->cbRecomputeArgsFn = func;
    _pPrivate->cbRecomputeArgsData = args;
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

void GfEventLoop::recompute(unsigned ms)
{
    // Call the 'recompute' callback if any.
    if (_pPrivate->cbRecomputeArgsFn)
        _pPrivate->cbRecomputeArgsFn(ms, _pPrivate->cbRecomputeArgsData);
    else if (_pPrivate->cbRecompute)
        _pPrivate->cbRecompute();
}
