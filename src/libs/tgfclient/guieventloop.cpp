/***************************************************************************
                guieventloop.cpp -- Event loop for GfuiApplications
                             -------------------
    created              : Thu Mar 8 10:00:00 CEST 2006
    copyright            : (C) 2006 by Brian Gavin ; 2008, 2010 Jean-Philippe Meuret
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

#include <SDL.h>

#include "tgfclient.h"

#include <vector>
#include <string>
#include <sstream>
#include <ctime>

//string splitting utils
std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}

std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    split(s, delim, elems);
    return elems;
}
//to string (from c++11)
template <typename T>
std::string to_string(T value)
{
	std::ostringstream os ;
	os << value ;
	return os.str() ;
}



// Private data (pimp pattern) =============================================
class GfuiEventLoop::Private
{
 public:

	//!  Constructor.
	Private();

 public: // Public data members.

	// Callback function pointers.
	void (*cbMouseButton)(int button, int state, int x, int y);
	void (*cbMouseMotion)(int x, int y);
	void (*cbMousePassiveMotion)(int x, int y);
#if SDL_JOYSTICK
	void (*cbJoystickAxis)(int joy, int axis, float value);
	void (*cbJoystickButton)(int joy, int button, int value);
#endif

	void (*cbDisplay)(void);
	void (*cbReshape)(int width, int height);

	// Variables.
	bool bRedisplay; // Flag to say if a redisplay is necessary.
};

GfuiEventLoop::Private::Private()
: cbMouseButton(0), cbMouseMotion(0), cbMousePassiveMotion(0),
  cbDisplay(0), cbReshape(0), bRedisplay(false)
{
}

// GfuiEventLoop class ============================================================

GfuiEventLoop::GfuiEventLoop()
: GfEventLoop()
{
	_pPrivate = new Private;
}

GfuiEventLoop::~GfuiEventLoop()
{
	delete _pPrivate;
}

void GfuiEventLoop::injectKeyboardEvent(int code, int modifier, int state,
										int unicode, int x, int y)
{
#ifndef WIN32
	// Hard-coded Alt+Enter shortcut, to enable the user to quit/re-enter
	// the full-screen mode ; as in SDL's full screen mode, events never reach
	// the Window Manager, we need this trick for the user to enjoy
	// its WM keyboard shortcuts (didn't find any other way yet).
	if (code == SDLK_RETURN	&& (modifier & KMOD_ALT) && state == 0)
	{
		if (!GfScrToggleFullScreen())
			GfLogError("Failed to toggle on/off the full-screen mode\n");
	}
	else
#endif
	{
		SDL_GetMouseState(&x, &y);
		GfEventLoop::injectKeyboardEvent(code, modifier, state, unicode, x, y);
	}
	//printf("Key %x State %x mod %x\n", code, state, modifier);
}

void GfuiEventLoop::injectMouseMotionEvent(int state, int x, int y)
{
	if (state == 0)
	{
		if (_pPrivate->cbMousePassiveMotion)
			_pPrivate->cbMousePassiveMotion(x, y);
	}
	else
	{
		if (_pPrivate->cbMouseMotion)
			_pPrivate->cbMouseMotion(x, y);
	}
}

void GfuiEventLoop::injectMouseButtonEvent(int button, int state, int x, int y)
{
	if (_pPrivate->cbMouseButton)
		_pPrivate->cbMouseButton(button, state, x, y);
}
#if SDL_JOYSTICK
void GfuiEventLoop::injectJoystickAxisEvent(int joy, int axis, float value)
{
	if (_pPrivate->cbJoystickAxis)
		_pPrivate->cbJoystickAxis(joy, axis, value);
}

void GfuiEventLoop::injectJoystickButtonEvent(int joy, int button, int value)
{
	if (_pPrivate->cbJoystickButton)
		_pPrivate->cbJoystickButton(joy, button, value);
}
#endif

// The event loop itself.
void GfuiEventLoop::operator()()
{
	SDL_Event event; // Event structure
#if SDL_MAJOR_VERSION >= 2
	static int unicode = 0;
   static SDL_Keymod modifier = KMOD_NONE;
#endif

	// Check for events.
	while (!quitRequested())
	{
		// Loop until there are no events left in the queue.
		while (!quitRequested() && SDL_PollEvent(&event))
		{
		    // Process events we care about, and ignore the others.
			switch(event.type)
			{
				case SDL_KEYDOWN:
#if SDL_MAJOR_VERSION < 2
					   injectKeyboardEvent(event.key.keysym.sym, event.key.keysym.mod, 0,event.key.keysym.unicode);
#else
					if((event.key.keysym.sym & SDLK_SCANCODE_MASK) == SDLK_SCANCODE_MASK)
					{
						injectKeyboardEvent(event.key.keysym.sym, event.key.keysym.mod, 0,0);
					}
					else if(false == isprint(event.key.keysym.sym))
					{
						injectKeyboardEvent(event.key.keysym.sym, event.key.keysym.mod, 0,0);
					}
					else if((event.key.keysym.mod != KMOD_NONE)
							&&(event.key.keysym.mod != KMOD_LSHIFT)
							&&(event.key.keysym.mod != KMOD_RSHIFT))
					{
						injectKeyboardEvent(event.key.keysym.sym, event.key.keysym.mod, 0,0);
					}
#if 0
					else
					{
						printf("SDL_KEYDOWN: %c\r\n",(char)event.key.keysym.sym);
					}
#endif
#endif
					break;

#if SDL_MAJOR_VERSION >= 2
				case SDL_TEXTINPUT:
					unicode = (int)(event.text.text[0]);
					modifier = SDL_GetModState();
					injectKeyboardEvent(unicode,modifier, 0,0);
					//printf("SDL_TEXTINPUT: %c %X\r\n",(char)unicode,modifier);
					break;
#endif

				case SDL_KEYUP:
#if SDL_MAJOR_VERSION < 2
					injectKeyboardEvent(event.key.keysym.sym, event.key.keysym.mod, 1,event.key.keysym.unicode);
#else
					if((event.key.keysym.sym & SDLK_SCANCODE_MASK) == SDLK_SCANCODE_MASK)
					{
						injectKeyboardEvent(event.key.keysym.sym, event.key.keysym.mod, 1,0);
					}
					else if(false == isprint(event.key.keysym.sym))
					{
						injectKeyboardEvent(event.key.keysym.sym, event.key.keysym.mod, 1,0);
					}
					else if((event.key.keysym.mod != KMOD_NONE)
							&&(event.key.keysym.mod != KMOD_LSHIFT)
							&&(event.key.keysym.mod != KMOD_RSHIFT))
					{
						injectKeyboardEvent(event.key.keysym.sym, event.key.keysym.mod, 1,0);
					}
					else
					{
						injectKeyboardEvent(unicode, event.key.keysym.mod, 1,0);
						//printf("SDL_KEYUP: %c unicode = %c\r\n",(char)event.key.keysym.sym,unicode);
					}
#endif
					break;

				case SDL_MOUSEMOTION:
					injectMouseMotionEvent(event.motion.state, event.motion.x, event.motion.y);
					break;


				case SDL_MOUSEBUTTONDOWN:
				case SDL_MOUSEBUTTONUP:
					injectMouseButtonEvent(event.button.button, event.button.state,
										   event.button.x, event.button.y);
					break;

				case SDL_QUIT:
					postQuit();
					break;

#if SDL_MAJOR_VERSION >= 2
#if SDL_JOYSTICK
				case SDL_JOYAXISMOTION:
					injectJoystickAxisEvent(event.jaxis.which, event.jaxis.axis, (float) event.jaxis.value / 32768);
					break;

				case SDL_JOYBUTTONDOWN:
					injectJoystickButtonEvent(event.jbutton.which, event.jbutton.button, SDL_PRESSED);
					break;

				case SDL_JOYBUTTONUP:
					injectJoystickButtonEvent(event.jbutton.which, event.jbutton.button, 0);
					break;
#endif
				case SDL_WINDOWEVENT_EXPOSED:
#else
				case SDL_VIDEOEXPOSE:
#endif
					forceRedisplay();
					break;
			}
		}

		if (!quitRequested())
		{
			// Recompute if anything to.
			recompute();

			// Redisplay if anything to.
			redisplay();
		}
	}

	GfLogTrace("Quitting GFUI event loop.\n");
}

void GfuiEventLoop::setMouseButtonCB(void (*func)(int button, int state, int x, int y))
{
	_pPrivate->cbMouseButton = func;
}

void GfuiEventLoop::setMouseMotionCB(void (*func)(int x, int y))
{
	_pPrivate->cbMouseMotion = func;
}

void GfuiEventLoop::setMousePassiveMotionCB(void (*func)(int x, int y))
{
	_pPrivate->cbMousePassiveMotion = func;
}

void GfuiEventLoop::setRedisplayCB(void (*func)(void))
{
	_pPrivate->cbDisplay = func;
}
#if SDL_JOYSTICK
void GfuiEventLoop::setJoystickAxisCB(void (*func)(int joy, int axis, float value))
{
	_pPrivate->cbJoystickAxis = func;
}

void GfuiEventLoop::setJoystickButtonCB(void (*func)(int joy, int button, int value))
{
	_pPrivate->cbJoystickButton = func;
}
#endif

void GfuiEventLoop::setReshapeCB(void (*func)(int width, int height))
{
	_pPrivate->cbReshape = func;
}

void GfuiEventLoop::postRedisplay(void)
{
	_pPrivate->bRedisplay = true;
}


/////////////////////////////////////////////////
std::vector<std::string> msglist;
const int WEBSERVER_IDLE = 0;
const int WEBSERVER_SENDING = 1;
const int WEBSERVER_RECEIVING = 2;
int webserverState = WEBSERVER_IDLE;

class NotificationManager {

	public:
		//constructor
		NotificationManager();
	
		//destructor
		~NotificationManager();

		void updateStatus();


	private:
		void startNewNotification();
		void runAnimation();
		void removeOldUi();
		void createUi();
		void updateWebserverStatusUi();
		
		void* screenHandle;
		void* prevScreenHandle;	
		void* menuXMLDescHdle;
		int	notifyUiIdBg;//the bg image uiid
		int notifyUiIdBusyIcon; //the webserver busy icon
		std::vector<int> notifyUiId;//the text lines uiid
		bool busy;
		int textPadding;
		std::clock_t animationStartTime; //when the animation started
		std::clock_t animationRestStartTime; //when the animation started
		std::clock_t animationLastExecTime; //the current time
		float totalAnimationDuration;//how much the animation should take to fully run in one direction
		float animationRestTime; //how much wes should wait when we a re fully displayed
		int animationDirection;
		int propertyFinalValue;
		std::vector<std::string> messageLines;
		int propertyChangeNeeded;
		
};
NotificationManager::NotificationManager(){

	this->busy = false;
	this->notifyUiIdBg = -1;//the bg image ui id
	this->notifyUiIdBusyIcon = -1;//the bg image ui id

	this->animationRestTime = 4 ; //how much wes should wait when we a re fully displayed
	this->totalAnimationDuration = 0.3 ;//how much the animation should take to fully run in one direction
	this->animationLastExecTime = std::clock(); //the current time

}
NotificationManager::~NotificationManager(){

}
void NotificationManager::updateStatus(){

	//get the current screen
	this->screenHandle = GfuiGetScreen();

	//get the ui descriptor
	this->menuXMLDescHdle = GfuiMenuLoad("notifications.xml");
	
	//if we are doing nothing and we have some message to display: let's do it
	if(this->busy==false && !msglist.empty()){

		this->startNewNotification();

	}

	//if we are running an animation
	if(this->busy==true){

		this->runAnimation();

	}
	
	//update webserver status icon
	this->updateWebserverStatusUi();
	
	//remember the current screen for the next run
	this->prevScreenHandle = this->screenHandle;

}

void NotificationManager::startNewNotification(){

	//we are running an animation
	this->busy=true;
	
	//set the animation direction
	this->animationDirection=1;
	
	//retrieve the message to display
	std::string newText = msglist.front().c_str();
	
	//divide the current message in lines
	this->messageLines = split(msglist.front().c_str(), '\n');
	
	//reset the start time(s)
	this->animationStartTime = this->animationLastExecTime = std::clock();
	this->animationRestStartTime = 0;
	
	//reset the start property
	int propertyCurrentValue=(int)GfParmGetNum(this->menuXMLDescHdle, "dynamic controls/slide", "x", "null", 0);
	this->propertyChangeNeeded=(int)GfParmGetNum(this->menuXMLDescHdle, "dynamic controls/slide", "width", "null", 0);
	this->propertyFinalValue = propertyCurrentValue + this->propertyChangeNeeded;

	//padding between the text and the bg image
	this->textPadding = propertyCurrentValue - (int)GfParmGetNum(this->menuXMLDescHdle, "dynamic controls/slidebg", "x", "null", 0);

	//start
	this->animationDirection = 1;
	this->runAnimation();

}
void NotificationManager::runAnimation(){
	
	//read the initial state of the UI
	int propertyCurrentValue = (int)GfParmGetNum(this->menuXMLDescHdle, "dynamic controls/slide", "x", "null", 0);
	// change needed from current status of the animation to the end
	int remainingChangeNeeded = (this->propertyFinalValue - propertyCurrentValue);

	//log current time
	std::clock_t currentTime = std::clock();

	//CASE 1	
	//we still need to apply some change to reach the final value
	if(remainingChangeNeeded != 0){

		//how much time is we are running the animation
		float animationRunningTime = (currentTime - this->animationStartTime) / (float) CLOCKS_PER_SEC;

		//how much time is passed from the last run
		float animationTimeFromLastStep = (currentTime - this->animationLastExecTime) / (float) CLOCKS_PER_SEC;
		//time remaining for the animation
		float animationTimeRemaining = this->totalAnimationDuration - animationRunningTime;

		//
		//int propertyStepChange = remainingChangeNeeded / animationTimeRemaining * animationTimeFromLastStep;
		//int propertyStepChange = remainingChangeNeeded / animationTimeRemaining;
		//if we have not arhieving 30fps slow down the animation
		if(animationTimeFromLastStep > 0,033333333){

			animationTimeFromLastStep = animationTimeFromLastStep;

		}
		int propertyStepChange = this->propertyChangeNeeded / this->totalAnimationDuration * this->animationDirection * animationTimeFromLastStep;

		// if the change is too little we round it up to 1 unit at least
		if((propertyStepChange * this->animationDirection) < 1 ){

			propertyStepChange = 1 * this->animationDirection;	

		}

		//new value for the property
		int propertyNewValue = propertyCurrentValue + propertyStepChange;

		//it he new value with the change applied is greater that the final result we want we correct it to be equal to the final result
		if (propertyNewValue * this->animationDirection  > propertyFinalValue * this->animationDirection ){

			propertyNewValue = propertyFinalValue;

		}
		
		//apply the new values
		GfParmSetNum(this->menuXMLDescHdle, "dynamic controls/slide", "x", "null", propertyNewValue);
		GfParmSetNum(this->menuXMLDescHdle, "dynamic controls/slidebg", "x", "null", propertyNewValue - this->textPadding);

		//remember the time we ran the last(this) animation frame
		this->animationLastExecTime = currentTime;
		
		/*
		GfLogInfo("###############################\n");
		GfLogInfo("StartTime: %d \n",this->animationStartTime);
		GfLogInfo("CurrentTime: %d \n",currentTime);
		GfLogInfo("RunningTime: %f \n ",(currentTime - this->animationStartTime) / (float) CLOCKS_PER_SEC);
		GfLogInfo("RunningTime: %f \n ",(currentTime - this->animationStartTime));
		GfLogInfo("RunningTime: %f \n ",(float) CLOCKS_PER_SEC);

		GfLogInfo("\n ");
		GfLogInfo("AnimationDuration: %f \n ",this->totalAnimationDuration);
		GfLogInfo("TimeRemaining: %f \n ",animationTimeRemaining);
		GfLogInfo("\n ");
		GfLogInfo("FinalValue: %i \n ",this->propertyFinalValue);
		GfLogInfo("CurrentValue: %i \n ",propertyCurrentValue);
		GfLogInfo("Change Needed: %i \n ",remainingChangeNeeded);
		GfLogInfo("StepChange: %i \n ",propertyStepChange);
		GfLogInfo("\n ");
		GfLogInfo("Direction: %i \n ",this->animationDirection);
		*/
		
		this->removeOldUi();
		this->createUi();

	}
	
	
	
	//CASE 2
	// no change needed while running the runOutAnimation
	if(remainingChangeNeeded == 0 && this->animationDirection == -1){

		//delette this message from the queque
		msglist.erase (msglist.begin());

		//we are no longer busy
		this->busy=false;

	}
	
	
	//CASE 3	
	// no change needed while running the runInAnimation: we have ended the runInAnimation
	if(remainingChangeNeeded == 0 && this->animationDirection == 1){
		if(this->animationRestStartTime==0){

			//we are just done runnig the runInAnimation
			//log the time we start waiting while fully displayed
			this->animationRestStartTime = std::clock();

		}else{

			//if rest time has expired: start the runOutAnimation
			if(((currentTime - this->animationRestStartTime) / (float) CLOCKS_PER_SEC) > this->animationRestTime){

				//change the animation direction
				this->animationDirection = -1;

				//reset the animation start time
				this->animationStartTime = this->animationLastExecTime = std::clock(); //when the animation started

				//read property info
				this->propertyChangeNeeded= (int)GfParmGetNum(this->menuXMLDescHdle, "dynamic controls/slide", "width", "null", 0);
				this->propertyFinalValue = propertyCurrentValue - this->propertyChangeNeeded;	

			}

		}

	}

}

void NotificationManager::removeOldUi(){

	//if there is a prev screen 
	if( GfuiScreenIsActive(this->prevScreenHandle) ){

		//if there is some prev ui around hide it
		if(this->notifyUiIdBg > 0){

			GfuiVisibilitySet(this->prevScreenHandle, this->notifyUiIdBg, GFUI_INVISIBLE);

		}
		
		//iterate trougth ui and set them invisible
		for (int i; i < notifyUiId.size(); i++) {

			GfuiVisibilitySet(this->prevScreenHandle, this->notifyUiId[i], GFUI_INVISIBLE);

		 }

	}

	//delete the prev ui's
	this->notifyUiId.clear();
	this->notifyUiIdBg=-1;
}

void NotificationManager::createUi(){

	//create the new UI	
	this->notifyUiIdBg = GfuiMenuCreateStaticImageControl(this->screenHandle, this->menuXMLDescHdle, "slidebg");			
	GfuiVisibilitySet(this->screenHandle, this->notifyUiIdBg, GFUI_VISIBLE);
	
	//get first line vertical position
	int ypos=(int)GfParmGetNum(this->menuXMLDescHdle, "dynamic controls/slide", "y", "null", 0);
	int yposmod= ypos;

	//iterate trougth lines
	for (int i; i < this->messageLines.size(); i++) {

		int uiId;
		uiId= GfuiMenuCreateLabelControl(this->screenHandle, this->menuXMLDescHdle, "slide");
		
		//change the vertical position
		int yposmod = ypos - (i+1)*(10);
		GfParmSetNum(this->menuXMLDescHdle, "dynamic controls/slide", "y", "null", yposmod);

		GfuiLabelSetText(this->screenHandle, uiId, this->messageLines[i].c_str());
		GfuiVisibilitySet(this->screenHandle, uiId, GFUI_VISIBLE);
		this->notifyUiId.push_back(uiId);

	 }

	//reset ypos
	GfParmSetNum(this->menuXMLDescHdle, "dynamic controls/slide", "y", "null", ypos);
}
void NotificationManager::updateWebserverStatusUi(){
	
	//if there is some prev ui around hide it
	if(this->notifyUiIdBusyIcon > 0){
		
		GfuiVisibilitySet(this->prevScreenHandle, this->notifyUiIdBusyIcon, GFUI_INVISIBLE);

	}	
	
	if(this->screenHandle > 0){
		//if webserver is busy display busy icon
		std::string webServerIcon = "busyicon";
		webServerIcon.append(to_string(webserverState));

		this->notifyUiIdBusyIcon = GfuiMenuCreateStaticImageControl(this->screenHandle, this->menuXMLDescHdle, webServerIcon.c_str());			
		GfuiVisibilitySet(this->screenHandle, this->notifyUiIdBusyIcon, GFUI_VISIBLE);		
		
	}

}

NotificationManager notifications;

void GfuiEventLoop::forceRedisplay()
{
	notifications.updateStatus();
	
	if (_pPrivate->cbDisplay)
		_pPrivate->cbDisplay();
}


void GfuiEventLoop::redisplay()
{
	//temp
	_pPrivate->bRedisplay=true;

	// Refresh display if requested and if any redisplay CB.
	if (_pPrivate->bRedisplay)
	{
		// Acknowledge the request
		// (Note: do it before forceRedisplay(), in case it calls postRedisplay() ;-).
		_pPrivate->bRedisplay = false;

		// Really call the redisplay call-back if any.
		forceRedisplay();
	}
}
