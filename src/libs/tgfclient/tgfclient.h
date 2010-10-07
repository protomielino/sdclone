/***************************************************************************
                    tgfclient.h -- Interface file for The Gaming Framework                                    
                             -------------------                                         
    created              : Fri Aug 13 22:32:14 CEST 1999
    copyright            : (C) 1999 by Eric Espie                         
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

/** @file   
        The Gaming Framework API (client part).
    @author     <a href=mailto:torcs@free.fr>Eric Espie</a>
    @version    $Id$
*/

#ifndef __TGFCLIENT__H__
#define __TGFCLIENT__H__

#include <string>
#include <vector>

#ifdef WIN32
#  include <windows.h>
//Disable some MSVC warnings
#  pragma warning (disable:4244)
#  pragma warning (disable:4996)
#  pragma warning (disable:4305)
#endif

#ifdef __APPLE__
#  include <OpenGL/gl.h>
#  include <OpenGL/glu.h>
#  include <js.h>
#else
#  include <GL/gl.h>
#  include <GL/glu.h>
#  include <plib/js.h>
#endif
#include <SDL/SDL_keysym.h>

#include <tgf.h>

#include "guiscreen.h"


// DLL exported symbols declarator for Windows.
#ifdef WIN32
# ifdef TGFCLIENT_DLL
#  define TGFCLIENT_API __declspec(dllexport)
# else
#  define TGFCLIENT_API __declspec(dllimport)
# endif
#else
# define TGFCLIENT_API
#endif


/******************** 
 * Initialization   *
 ********************/

TGFCLIENT_API void GfInitClient(void);


/******************** 
 * Screen Interface *
 ********************/

typedef struct ScreenSize
{
    int width;  // Width in pixels.
    int height; // Height in pixels.
} tScreenSize;

TGFCLIENT_API void GfScrInit(int argc, char *argv[]);
TGFCLIENT_API void GfScrShutdown(void);
TGFCLIENT_API void GfScrGetSize(int *scrW, int *scrH, int *viewW, int *viewH);
TGFCLIENT_API unsigned char* GfScrCaptureAsImage(int* viewW, int *viewH);
TGFCLIENT_API int GfScrCaptureAsPNG(const char *filename);

TGFCLIENT_API int* GfScrGetPossibleColorDepths(int* pnDepths);
TGFCLIENT_API tScreenSize* GfScrGetPossibleSizes(int nColorDepth, bool bFullScreen, int* pnSizes);
TGFCLIENT_API tScreenSize* GfScrGetDefaultSizes(int* pnSizes);

/*****************************
 * GUI interface (low-level) *
 *****************************/

/* Widget type */
#define GFUI_LABEL      0
#define GFUI_BUTTON     1
#define GFUI_GRBUTTON   2
#define GFUI_SCROLLIST  3
#define GFUI_SCROLLBAR  4
#define GFUI_EDITBOX    5
#define GFUI_COMBOBOX	6
#define GFUI_CHECKBOX	7
#define GFUI_PROGRESSBAR 8

/* Alignment */
#define GFUI_ALIGN_HL_VB        0x00
#define GFUI_ALIGN_HL_VC        0x01
#define GFUI_ALIGN_HL_VT        0x02
#define GFUI_ALIGN_HC_VB        0x10
#define GFUI_ALIGN_HC_VC        0x11
#define GFUI_ALIGN_HC_VT        0x12
#define GFUI_ALIGN_HR_VB        0x20
#define GFUI_ALIGN_HR_VC        0x21
#define GFUI_ALIGN_HR_VT        0x22

/* Mouse action */
#define GFUI_MOUSE_UP   0
#define GFUI_MOUSE_DOWN 1

/* Keyboard action */
#define GFUI_KEY_UP     0
#define GFUI_KEY_DOWN   1

/* Scroll Bar position */
#define GFUI_SB_NONE    0
#define GFUI_SB_RIGHT   1
#define GFUI_SB_LEFT    2
#define GFUI_SB_TOP     3
#define GFUI_SB_BOTTOM  4

/* Scroll bar orientation */
#define GFUI_HORI_SCROLLBAR     0
#define GFUI_VERT_SCROLLBAR     1

// Some keyboard key / special key codes, to avoid SDLK constants everywhere.
#define GFUIK_BACKSPACE	SDLK_BACKSPACE
#define GFUIK_TAB	SDLK_TAB
#define GFUIK_SPACE	SDLK_SPACE
#define GFUIK_CLEAR	SDLK_CLEAR
#define GFUIK_RETURN	SDLK_RETURN
#define GFUIK_PAUSE	SDLK_PAUSE
#define GFUIK_ESCAPE	SDLK_ESCAPE
#define GFUIK_DELETE	SDLK_DELETE

#define GFUIK_UP	SDLK_UP
#define GFUIK_DOWN	SDLK_DOWN
#define GFUIK_RIGHT	SDLK_RIGHT
#define GFUIK_LEFT	SDLK_LEFT
#define GFUIK_INSERT	SDLK_INSERT
#define GFUIK_HOME	SDLK_HOME
#define GFUIK_END	SDLK_END
#define GFUIK_PAGEUP	SDLK_PAGEUP
#define GFUIK_PAGEDOWN	SDLK_PAGEDOWN

#define GFUIK_F1	SDLK_F1
#define GFUIK_F2	SDLK_F2
#define GFUIK_F3	SDLK_F3
#define GFUIK_F4	SDLK_F4
#define GFUIK_F5	SDLK_F5
#define GFUIK_F6	SDLK_F6
#define GFUIK_F7	SDLK_F7
#define GFUIK_F8	SDLK_F8
#define GFUIK_F9	SDLK_F9
#define GFUIK_F10	SDLK_F10
#define GFUIK_F11	SDLK_F11
#define GFUIK_F12	SDLK_F12
#define GFUIK_F13	SDLK_F13
#define GFUIK_F14	SDLK_F14
#define GFUIK_F15	SDLK_F15

// Add needed other GFUIK_* here or above.

// Maximun value of a key code (Has to be the least greater  2^N - 1 >= SDLK_LAST)
#define GFUIK_MAX	0x1FF

#if (GFUIK_MAX < SDLK_LAST)
# error SDLK_MAX has grown too much, please increase GFUIK_MAX to the least greater power of 2 minus 1.
#endif


/** Scroll bar call-back information */
typedef struct ScrollBarInfo
{
    int         pos;            /**< Current scroll bar position */
    void        *userData;      /**< Associated user data */
} tScrollBarInfo;

/** Combo-box call-back information */
typedef struct ComboBoxInfo
{
	unsigned int nPos;                    /**< Currently selected choice */
	std::vector<std::string> vecChoices;  /**< Possible choices */
    void *userData;                       /**< Associated user data */
} tComboBoxInfo;


/** Check-box call-back information */
typedef struct CheckBoxInfo
{
    bool         bChecked;      /**< Check-box state */
    void        *userData;      /**< Associated user data */
} tCheckBoxInfo;

typedef void (*tfuiCallback)(void * /* userData */);
typedef void (*tfuiSBCallback)(tScrollBarInfo *);
typedef int (*tfuiKeyCallback)(int key, int modifier, int state);  /**< return 1 to prevent normal key computing */
typedef void (*tfuiComboboxCallback)(tComboBoxInfo *);
typedef void (*tfuiCheckboxCallback)(tCheckBoxInfo *);


/* Event loop callback functions (should be called explicitely if the corresponding
   event loop callback is overriden after a call to GfuiActivateScreen */
TGFCLIENT_API void GfuiDisplay(void);
TGFCLIENT_API void GfuiDisplayNothing(void);
TGFCLIENT_API void GfuiIdle(void);
TGFCLIENT_API void GfuiIdleMenu(void);

struct RmInfo;
typedef struct RmInfo tRmInfo;
TGFCLIENT_API void (*tfuiIdleCB(tRmInfo *info))(void);

/* Screen management */
TGFCLIENT_API void *GfuiScreenCreate(void);
TGFCLIENT_API void *GfuiScreenCreateEx(float *bgColor, 
                                void *userDataOnActivate, tfuiCallback onActivate, 
                                void *userDataOnDeactivate, tfuiCallback onDeactivate, 
                                int mouseAllowed);
TGFCLIENT_API void GfuiScreenRelease(void *screen);
TGFCLIENT_API void GfuiScreenActivate(void *screen);
TGFCLIENT_API int  GfuiScreenIsActive(void *screen);
TGFCLIENT_API void GfuiScreenReplace(void *screen);
TGFCLIENT_API void GfuiScreenDeactivate(void);
TGFCLIENT_API void *GfuiHookCreate(void *userDataOnActivate, tfuiCallback onActivate);
TGFCLIENT_API void GfuiHookRelease(void *hook);
TGFCLIENT_API void GfuiAddKey(void *scr, int key, const char *descr, void *userData, tfuiCallback onKeyPressed, tfuiCallback onKeyReleased);
TGFCLIENT_API void GfuiRegisterKey(int key, const char *descr, void *userData, tfuiCallback onKeyPressed, tfuiCallback onKeyReleased);
TGFCLIENT_API void GfuiSetKeyAutoRepeat(void *scr, int on);
TGFCLIENT_API void GfuiHelpScreen(void *prevScreen);
TGFCLIENT_API void GfuiScreenShot(void *notused);
TGFCLIENT_API void GfuiScreenAddBgImg(void *scr, const char *filename);
TGFCLIENT_API void GfuiKeyEventRegister(void *scr, tfuiKeyCallback onKeyAction);
TGFCLIENT_API void GfuiKeyEventRegisterCurrent(tfuiKeyCallback onKeyAction);
TGFCLIENT_API void GfuiSleep(double delay);
TGFCLIENT_API void GfuiInitWindowPosition(int x, int y);
TGFCLIENT_API void GfuiInitWindowSize(int x, int y);
TGFCLIENT_API void GfuiSwapBuffers(void);

class TGFCLIENT_API GfuiMenuScreen
{
public:
	GfuiMenuScreen(const char* pszXMLDescFile);
	virtual ~GfuiMenuScreen();

	void CreateMenu();
	void CreateMenuEx(float *bgColor, 
					  void *userDataOnActivate, tfuiCallback onActivate, 
					  void *userDataOnDeactivate, tfuiCallback onDeactivate, 
					  int mouseAllowed);
	void SetMenuHandle(void* hdle);
	void* GetMenuHandle() const;
	void SetPreviousMenuHandle(void* hdle);
	void* GetPreviousMenuHandle() const;

	bool OpenXMLDescriptor();

	bool CreateStaticControls();

	int CreateButtonControl(const char *pszName, void *userData, tfuiCallback onPush);
	int CreateButtonControlEx(const char *pszName, void *userData, tfuiCallback onPush,
							  void *userDataOnFocus, tfuiCallback onFocus,
							  tfuiCallback onFocusLost);
	int CreateStaticImageControl(const char *pszName);
	int CreateLabelControl(const char *pszName);
	int CreateEditControl(const char *pszName, void *userDataOnFocus, tfuiCallback onFocus,
						  tfuiCallback onFocusLost);
	int CreateScrollListControl(const char *pszName,void *userData, tfuiCallback onSelect);
	int CreateComboboxControl(const char *pszName, void *userData, tfuiComboboxCallback onChange);
	int CreateCheckboxControl(const char *pszName, void *userData, tfuiCheckboxCallback onChange);
	int CreateProgressbarControl(const char *pszName);
	
	int GetDynamicControlId(const char *pszName) const;

	void AddDefaultShortcuts();
	void AddShortcut(int key, const char *descr, void *userData,
					 tfuiCallback onKeyPressed, tfuiCallback onKeyReleased);
	
	bool CloseXMLDescriptor();
	
	void RunMenu();

private:
	struct gfuiMenuPrivateData* m_priv;
};

/* Mouse management */
typedef struct MouseInfo
{
    int X;
    int Y;
    int button[3];
} tMouseInfo;

TGFCLIENT_API tMouseInfo *GfuiMouseInfo(void);
TGFCLIENT_API void GfuiMouseSetPos(int x, int y);
TGFCLIENT_API void GfuiMouseHide(void);
TGFCLIENT_API void GfuiMouseShow(void);
TGFCLIENT_API void GfuiMouseToggleVisibility(void);
TGFCLIENT_API void GfuiMouseSetHWPresent(void);
TGFCLIENT_API bool GfuiMouseIsHWPresent(void);


/* All widgets */
#define GFUI_VISIBLE    1       /**< Object visibility flag  */
#define GFUI_INVISIBLE  0       /**< Object invisibility flag  */
TGFCLIENT_API int GfuiVisibilitySet(void *scr, int id, int visible);
#define GFUI_DISABLE    1
#define GFUI_ENABLE     0
TGFCLIENT_API int GfuiEnable(void *scr, int id, int flag);
TGFCLIENT_API void GfuiUnSelectCurrent(void);

/* Labels */
#define GFUI_FONT_BIG           0
#define GFUI_FONT_LARGE         1
#define GFUI_FONT_MEDIUM        2
#define GFUI_FONT_SMALL         3
#define GFUI_FONT_BIG_C         4
#define GFUI_FONT_LARGE_C       5
#define GFUI_FONT_MEDIUM_C      6
#define GFUI_FONT_SMALL_C       7
#define GFUI_FONT_DIGIT         8
TGFCLIENT_API int GfuiLabelCreate(void *scr, const char *text, 
                        int font, int x, int y, int align, int maxlen);
TGFCLIENT_API int GfuiLabelCreateEx(void *scr, const char *text, const float *fgColorPtr, int font, int x, int y, int align, int maxlen);

TGFCLIENT_API void GfuiSetTipPosition(int x,int y);
TGFCLIENT_API int GfuiTipCreate(void *scr, const char *text, int maxlen);
TGFCLIENT_API int GfuiTitleCreate(void *scr, const char *text, int maxlen);

TGFCLIENT_API void GfuiLabelSetText(void *scr, int id, const char *text);
TGFCLIENT_API void GfuiLabelSetColor(void *scr, int id, const float * colorPtr);

TGFCLIENT_API void GfuiPrintString(const char *text, float *fgColor, int font, int x, int y, int align);
TGFCLIENT_API int  GfuiFontHeight(int font);
TGFCLIENT_API int  GfuiFontWidth(int font, const char *text);


/* Buttons */
#define GFUI_BTNSZ      300
TGFCLIENT_API int GfuiButtonCreate(void *scr, const char *text, int font,
                            int x, int y, int width, int align, int mouse,
                            void *userDataOnPush, tfuiCallback onPush, 
                            void *userDataOnFocus, tfuiCallback onFocus, tfuiCallback onFocusLost);
TGFCLIENT_API int GfuiButtonStateCreate(void *scr, const char *text, int font, int x, int y, int width, int align, int mouse,
                                 void *userDataOnPush, tfuiCallback onPush, 
                                 void *userDataOnFocus, tfuiCallback onFocus, tfuiCallback onFocusLost);
TGFCLIENT_API int GfuiGrButtonCreate(void *scr, const char *disabled, const char *enabled, const char *focused, const char *pushed,
                              int x, int y, int align, int mouse,
                              void *userDataOnPush, tfuiCallback onPush, 
                              void *userDataOnFocus, tfuiCallback onFocus, tfuiCallback onFocusLost);
TGFCLIENT_API int GfuiGrButtonCreateEx(void *scr, const char *disabled, const char *enabled, const char *focused, const char *pushed,
                   int x, int y, int imageWidth,int imageHeight,int align, int mouse,
                   void *userDataOnPush, tfuiCallback onPush, 
                   void *userDataOnFocus, tfuiCallback onFocus, tfuiCallback onFocusLost);
TGFCLIENT_API int GfuiComboboxCreate(void *scr, int font, int x, int y, int width,int align ,int style,const char *pszText,void* userData,tfuiComboboxCallback onChange);
TGFCLIENT_API int GfuiCheckboxCreate(void *scr, int font, int x, int y, int imagewidth,int imageheight,int align ,int style,const char *pszText,bool bChecked,void* userData,tfuiCheckboxCallback onChange);


class TGFCLIENT_API Color
{
  public:
    float red, green, blue, alpha;
    const float *GetPtr() const { return (float*)this; }
};

TGFCLIENT_API unsigned int GfuiComboboxAddText(void *scr, int id, const char *text);
TGFCLIENT_API void GfuiComboboxSetTextColor(void *scr, int id, const Color& color);
TGFCLIENT_API void GfuiComboboxSetSelectedIndex(void *scr, int id, unsigned int index);
TGFCLIENT_API void GfuiComboboxSetPosition(void *scr, int id, unsigned int pos);
TGFCLIENT_API unsigned int GfuiComboboxGetPosition(void *scr, int id);
TGFCLIENT_API const char* GfuiComboboxGetText(void *scr, int id);
TGFCLIENT_API void GfuiComboboxClear(void *scr, int id);

TGFCLIENT_API int GfuiProgressbarCreate(void *scr, int x, int y, int w, int h, const char *pszProgressbackImg,const char *progressbarimg, int align,float min,float max,float value);


TGFCLIENT_API int GfuiGrButtonCreateEx(void *scr, const char *disabled, const char *enabled, const char *focused, const char *pushed,
		   int x, int y, int imageWidth,int imageHeight,int align, int mouse,
		   void *userDataOnPush, tfuiCallback onPush, 
		   void *userDataOnFocus, tfuiCallback onFocus, tfuiCallback onFocusLost);


TGFCLIENT_API void GfuiButtonShowBox(void *scr, int id,bool bShow);
TGFCLIENT_API void GfuiButtonSetColor(void *scr, int id, Color color);
TGFCLIENT_API void GfuiButtonSetFocusColor(void *scr, int id,Color focuscolor);
TGFCLIENT_API void GfuiButtonSetPushedColor(void *scr, int id,Color pushcolor);
TGFCLIENT_API void GfuiButtonSetImage(void *scr,int id,int x,int y,int w,int h,const char *disableFile,const char *enableFile,const char*focusedFile,const char *pushedFile);
TGFCLIENT_API void GfuiCheckboxSetChecked(void *scr, int id, bool bChecked);


TGFCLIENT_API void GfuiButtonSetText(void *scr, int id, const char *text);
TGFCLIENT_API int GfuiButtonGetFocused(void);

/* Edit Boxes */
TGFCLIENT_API int GfuiEditboxCreate(void *scr, const char *text, int font, int x, int y, int width, int maxlen,
                             void *userDataOnFocus, tfuiCallback onFocus, tfuiCallback onFocusLost);
TGFCLIENT_API int GfuiEditboxGetFocused(void);
TGFCLIENT_API char *GfuiEditboxGetString(void *scr, int id);
TGFCLIENT_API void GfuiEditboxSetString(void *scr, int id, const char *text);
TGFCLIENT_API void GfuiEditboxSetColor(void *scr, int id,Color color);
TGFCLIENT_API void GfuiEditboxSetFocusColor(void *scr, int id,Color focuscolor);


/* Scrolling lists */
TGFCLIENT_API int GfuiScrollListCreate(void *scr, int font, int x, int y, int align,
                                int width, int height, int scrollbar, void *userDataOnSelect, tfuiCallback onSelect);
TGFCLIENT_API int GfuiScrollListInsertElement(void *scr, int Id, const char *element, int index, void *userData);
TGFCLIENT_API int GfuiScrollListMoveSelectedElement(void *scr, int Id, int delta);
TGFCLIENT_API const char *GfuiScrollListExtractSelectedElement(void *scr, int Id, void **userData);
TGFCLIENT_API const char *GfuiScrollListExtractElement(void *scr, int Id, int index, void **userData);
TGFCLIENT_API void GfuiScrollListClear(void *scr, int Id);
TGFCLIENT_API const char *GfuiScrollListGetSelectedElement(void *scr, int Id, void **userData);
TGFCLIENT_API bool GfuiScrollListSetSelectedElement(void *scr, int Id, unsigned int selectElement);
TGFCLIENT_API const char *GfuiScrollListGetElement(void *scr, int Id, int index, void **userData);
TGFCLIENT_API void GfuiScrollListShowElement(void *scr, int Id, int index);
TGFCLIENT_API void GfuiScrollListSetColor(void *scr, int id,Color color);
TGFCLIENT_API void GfuiScrollListSetSelectColor(void *scr, int id,Color color);


/* Scroll bars */
TGFCLIENT_API int GfuiScrollBarCreate(void *scr, int x, int y, int align, int width, int orientation,
                               int min, int max, int len, int start, 
                               void *userData, tfuiSBCallback onScroll);
TGFCLIENT_API void GfuiScrollBarPosSet(void *scr, int id, int min, int max, int len, int start);
TGFCLIENT_API int GfuiScrollBarPosGet(void *scr, int id);

/* Images */
TGFCLIENT_API int GfuiStaticImageCreate(void *scr, int x, int y, int w, int h, const char *name,
										int align = GFUI_ALIGN_HL_VB, bool canDeform = true);
TGFCLIENT_API void GfuiStaticImageSet(void *scr, int id, const char *name,
									  unsigned index = 0, bool canDeform = true);
TGFCLIENT_API void GfuiStaticImageSetActive(void *scr, int id, int index);

/*****************************
 * Menu Management Interface *
 *****************************/

TGFCLIENT_API void *GfuiMenuScreenCreate(const char *title);
TGFCLIENT_API void  GfuiMenuDefaultKeysAdd(void *scr);
TGFCLIENT_API int   GfuiMenuButtonCreate(void *menu, const char *text, const char *tip, void *userData, const int style, tfuiCallback onpush);
TGFCLIENT_API int   GfuiMenuBackQuitButtonCreate(void *menu, const char *text, const char *tip, void *userData, tfuiCallback onpush);

/*******************************************
 * New XML based Menu Management Interface *
 *******************************************/

TGFCLIENT_API void *LoadMenuXML(const char *pFilePath);
TGFCLIENT_API bool CreateStaticControls(void *param,void *menuHandle);

TGFCLIENT_API int CreateButtonControl(void *menuHandle,void *param,const char *pControlName,void *userData, tfuiCallback onPush);
TGFCLIENT_API int CreateButtonControlEx(void *menuHandle,void *param,const char *pControlName,void *userData, tfuiCallback onPush, void *userDataOnFocus, tfuiCallback onFocus, tfuiCallback onFocusLost);
TGFCLIENT_API int CreateStaticImageControl(void *menuHandle,void *param,const char *pControlName);
TGFCLIENT_API int CreateLabelControl(void *menuHandle,void *param,const char *pControlName);
TGFCLIENT_API int CreateEditControl(void *menuHandle,void *param,const char *pControlName,void *userDataOnFocus, tfuiCallback onFocus, tfuiCallback onFocusLost);
TGFCLIENT_API int CreateScrollListControl(void *menuHandle,void *param,const char *pControlName,void *userData, tfuiCallback onSelect);
TGFCLIENT_API int CreateComboboxControl(void *menuHandle,void *param,const char *pControlName,void *userData,tfuiComboboxCallback onChange);
TGFCLIENT_API int CreateCheckboxControl(void *menuHandle,void *param,const char *pControlName,void *userData,tfuiCheckboxCallback onChange);
TGFCLIENT_API int CreateProgressbarControl(void *menuHandle,void *param,const char *pControlName);

TGFCLIENT_API void GfuiButtonShowBox(void *scr, int id,bool bShow);
TGFCLIENT_API void GfuiButtonSetColor(void *scr, int id,Color color);
TGFCLIENT_API void GfuiButtonSetFocusColor(void *scr, int id,Color focuscolor);
TGFCLIENT_API void GfuiButtonSetPushedColor(void *scr, int id,Color pushcolor);
TGFCLIENT_API void GfuiButtonSetImage(void *scr,int id,int x,int y,int w,int h,const char *disableFile,const char *enableFile,const char*focusedFile,const char *pushedFile);

/*****************************
 * Texture / image interface *
 *****************************/

TGFCLIENT_API unsigned char *GfTexReadImageFromFile(const char *filename, float screen_gamma, int *pWidth, int *pHeight, int *pPow2Width = 0, int *pPow2Height = 0);
TGFCLIENT_API unsigned char *GfTexReadImageFromPNG(const char *filename, float screen_gamma, int *pWidth, int *pHeight, int *pPow2Width = 0, int *pPow2Height = 0);
TGFCLIENT_API unsigned char *GfTexReadImageFromJPEG(const char *filename, float screen_gamma, int *pWidth, int *pHeight, int *pPow2Width = 0, int *pPow2Height = 0);

TGFCLIENT_API int GfTexWriteImageToPNG(unsigned char *img, const char *filename, int width, int height);
TGFCLIENT_API void GfTexFreeTexture(GLuint glTexId);
TGFCLIENT_API GLuint GfTexReadTexture(const char *filename, int* pWidth = 0, int* pHeight = 0,
									  int *pPow2Width = 0, int *pPow2Height = 0);


/*********************
 * Control interface *
 *********************/

#define GFCTRL_TYPE_NOT_AFFECTED        0
#define GFCTRL_TYPE_JOY_AXIS            1
#define GFCTRL_TYPE_JOY_BUT             2
#define GFCTRL_TYPE_KEYBOARD            3
#define GFCTRL_TYPE_MOUSE_BUT           4
#define GFCTRL_TYPE_MOUSE_AXIS          5

typedef struct
{
    int         index;
    int         type;
} tCtrlRef;


#define GFCTRL_JOY_UNTESTED     -1
#define GFCTRL_JOY_NONE         0
#define GFCTRL_JOY_PRESENT      1

#define GFCTRL_JOY_NUMBER       8 /* Max number of managed joysticks */
#define GFCTRL_JOY_MAX_BUTTONS  32       /* Size of integer so don't change please */
#define GFCTRL_JOY_MAX_AXES     _JS_MAX_AXES

/** Joystick Information Structure */
typedef struct
{
    int         oldb[GFCTRL_JOY_NUMBER];
    float       ax[GFCTRL_JOY_MAX_AXES * GFCTRL_JOY_NUMBER];         /**< Axis values */
    int         edgeup[GFCTRL_JOY_MAX_BUTTONS * GFCTRL_JOY_NUMBER];  /**< Button transition from down (pressed) to up */
    int         edgedn[GFCTRL_JOY_MAX_BUTTONS * GFCTRL_JOY_NUMBER];  /**< Button transition from up to down */
    int         levelup[GFCTRL_JOY_MAX_BUTTONS * GFCTRL_JOY_NUMBER]; /**< Button state (1 = up) */
} tCtrlJoyInfo;

TGFCLIENT_API tCtrlJoyInfo *GfctrlJoyInit(void);
TGFCLIENT_API int GfctrlJoyIsPresent(void);
TGFCLIENT_API int GfctrlJoyGetCurrent(tCtrlJoyInfo *joyInfo);
TGFCLIENT_API void GfctrlJoyRelease(tCtrlJoyInfo *joyInfo);


/** Mouse information structure */
typedef struct
{
    int         edgeup[3];      /**< Button transition from down (pressed) to up */
    int         edgedn[3];      /**< Button transition from up to down */
    int         button[3];      /**< Button state (1 = up) */
    float       ax[4];          /**< mouse axis position (mouse considered as a joystick) */
} tCtrlMouseInfo;

TGFCLIENT_API tCtrlMouseInfo *GfctrlMouseInit(void);
TGFCLIENT_API int GfctrlMouseGetCurrent(tCtrlMouseInfo *mouseInfo);
TGFCLIENT_API void GfctrlMouseRelease(tCtrlMouseInfo *mouseInfo);
TGFCLIENT_API void GfctrlMouseCenter(void);
TGFCLIENT_API void GfctrlMouseInitCenter(void);
TGFCLIENT_API tCtrlRef *GfctrlGetRefByName(const char *name);
TGFCLIENT_API const char *GfctrlGetNameByRef(int type, int index);


/************************
 * Event loop interface *
 ************************/

// Callbacks initialization
TGFCLIENT_API void GfelInitialize();

// Specific callbacks setup
TGFCLIENT_API void GfelSetKeyboardDownCB(void (*func)(int key, int modifiers, int x, int y));
TGFCLIENT_API void GfelSetKeyboardUpCB(void (*func)(int key, int modifiers, int x, int y));

TGFCLIENT_API void GfelSetMouseButtonCB(void (*func)(int button, int state, int x, int y));
TGFCLIENT_API void GfelSetMouseMotionCB(void (*func)(int x, int y));
TGFCLIENT_API void GfelSetMousePassiveMotionCB(void (*func)(int x, int y));

TGFCLIENT_API void GfelSetIdleCB(void (*func)(void));

TGFCLIENT_API void GfelSetDisplayCB(void (*func)(void));
TGFCLIENT_API void GfelSetReshapeCB(void (*func)(int width, int height));

TGFCLIENT_API void GfelSetTimerCB(unsigned int millis, void (*func)(int value));

// Event loop management
TGFCLIENT_API void GfelPostRedisplay(void);
TGFCLIENT_API void GfelForceRedisplay();

// The event loop itself (never returns)
TGFCLIENT_API void GfelMainLoop(void);

/*******************************
 * Graphics features interface *
 *******************************/

TGFCLIENT_API int GfglIsOpenGLExtensionSupported(const char* extension);

// GL_ARB_texture_compression
TGFCLIENT_API bool GfglIsCompressARBAvailable(void);
TGFCLIENT_API bool GfglIsCompressARBEnabled(void);
TGFCLIENT_API void GfglUpdateCompressARBEnabled(void);

// Texture max size
TGFCLIENT_API int GfglGetUserTextureMaxSize(void);
TGFCLIENT_API int GfglGetGLTextureMaxSize(void);
TGFCLIENT_API void GfglUpdateUserTextureMaxSize(void);

// Texture non-power-of-2 support
TGFCLIENT_API bool GfglIsTextureRectangleARBAvailable(void); // In case mipmapping NOT needed.
TGFCLIENT_API bool GfglIsTextureNonPowerOf2ARBAvailable(void); // In case mipmapping needed.

// Multi-texturing support
TGFCLIENT_API bool GfglIsMultiTexturingEnabled();
TGFCLIENT_API void GfglEnableMultiTexturing(bool bEnable = true);


#endif /* __TGFCLIENT__H__ */


