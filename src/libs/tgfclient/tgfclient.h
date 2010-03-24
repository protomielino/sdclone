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


#include <string>
#include <vector>

#ifndef __TGFCLIENT__H__
#define __TGFCLIENT__H__

#ifdef WIN32
#  include <windows.h>
//Disable some warnings
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
#include "sdlcallbacks.h"

#include "screen_properties.h"


class Color
{
  public:
    float red, green, blue, alpha;
    float *GetPtr() { return (float*)this;}
};

struct RmInfo;
typedef struct RmInfo tRmInfo;

extern void GfInitClient(void);

/******************** 
 * Screen Interface *
 ********************/

extern void GfScrInit(int argc, char *argv[]);
extern void GfScrShutdown(void);
extern void *GfScrMenuInit(void *precMenu);
extern char *GfTime2Str(tdble sec, int sgn);
extern void GfScrGetSize(int *ScrW, int *ScrH, int *ViewW, int *ViewH);
extern void GfScrReinit(void*);


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


/** Scroll bar call-back information */
typedef struct ScrollBarInfo
{
    int         pos;            /**< Current scroll bar position */
    void        *userData;      /**< Associated user data */
} tScrollBarInfo;

typedef struct ChoiceInfo
{
	unsigned int nPos;
	std::vector<std::string> vecChoices;
} tChoiceInfo;

typedef void (*tfuiCallback)(void * /* userdata */);
typedef void (*tfuiSBCallback)(tScrollBarInfo *);
typedef int (*tfuiKeyCallback)(int unicode, int modifier, int state); /**< return 1 to prevent normal key computing */
typedef int (*tfuiSKeyCallback)(int key, int modifier, int state);  /**< return 1 to prevent normal key computing */
typedef void (*tfuiComboCallback)(tChoiceInfo * pInfo);
typedef void (*tfuiCheckboxCallback)(bool bChecked);


/* Event loop callback functions (should be called explicitely if the corresponding
   SDL Func is overriden after a call to GfuiActivateScreen */
extern void GfuiDisplay(void);
extern void GfuiDisplayNothing(void);
extern void (*GfuiIdleFunc(tRmInfo *info))(void);
extern void GfuiIdle(void);
extern void GfuiIdleMenu(void);

/* Screen management */
extern void *GfuiScreenCreate(void);
extern void *GfuiScreenCreateEx(float *bgColor, 
                                void *userDataOnActivate, tfuiCallback onActivate, 
                                void *userDataOnDeactivate, tfuiCallback onDeactivate, 
                                int mouseAllowed);
extern void GfuiScreenRelease(void *screen);
extern void GfuiScreenActivate(void *screen);
extern int  GfuiScreenIsActive(void *screen);
extern void GfuiScreenReplace(void *screen);
extern void GfuiScreenDeactivate(void);
extern void *GfuiHookCreate(void *userDataOnActivate, tfuiCallback onActivate);
extern void GfuiHookRelease(void *hook);
extern void GfuiAddKey(void *scr, int unicode, const char *descr, void *userData, tfuiCallback onKeyPressed, tfuiCallback onKeyReleased);
extern void GfuiRegisterKey(int unicode, const char *descr, void *userData, tfuiCallback onKeyPressed, tfuiCallback onKeyReleased);
extern void GfuiAddSKey(void *scr, int key, const char *descr, void *userData, tfuiCallback onKeyPressed, tfuiCallback onKeyReleased);
extern void GfuiSetKeyAutoRepeat(void *scr, int on);
extern void GfuiHelpScreen(void *prevScreen);
extern void GfuiScreenShot(void *notused);
extern void GfuiScreenAddBgImg(void *scr, const char *filename);
extern void GfuiKeyEventRegister(void *scr, tfuiKeyCallback onKeyAction);
extern void GfuiSKeyEventRegister(void *scr, tfuiSKeyCallback onSKeyAction);
extern void GfuiKeyEventRegisterCurrent(tfuiKeyCallback onKeyAction);
extern void GfuiSKeyEventRegisterCurrent(tfuiSKeyCallback onSKeyAction);
extern void GfuiSleep(double delay);
extern int GfuiOpenGLExtensionSupported(const char* extension);

/* mouse */
typedef struct MouseInfo
{
    int X;
    int Y;
    int button[3];
} tMouseInfo;

extern tMouseInfo *GfuiMouseInfo(void);
extern void GfuiMouseSetPos(int x, int y);
extern void GfuiMouseHide(void);
extern void GfuiMouseShow(void);
extern void GfuiMouseToggleVisibility(void);
extern void GfuiMouseSetHWPresent(void);

/* all widgets */
#define GFUI_VISIBLE    1       /**< Object visibility flag  */
#define GFUI_INVISIBLE  0       /**< Object invisibility flag  */
extern int GfuiVisibilitySet(void *scr, int id, int visible);
#define GFUI_DISABLE    1
#define GFUI_ENABLE     0
extern int GfuiEnable(void *scr, int id, int flag);
extern void GfuiUnSelectCurrent(void);

/* labels */
#define GFUI_FONT_BIG           0
#define GFUI_FONT_LARGE         1
#define GFUI_FONT_MEDIUM        2
#define GFUI_FONT_SMALL         3
#define GFUI_FONT_BIG_C         4
#define GFUI_FONT_LARGE_C       5
#define GFUI_FONT_MEDIUM_C      6
#define GFUI_FONT_SMALL_C       7
#define GFUI_FONT_DIGIT         8
extern int GfuiLabelCreate(void *scr, const char *text, 
                        int font, int x, int y, int align, int maxlen);
extern int GfuiLabelCreateEx(void *scr, const char *text, const float *fgColorPtr, int font, int x, int y, int align, int maxlen);

extern void GfuiSetTipPosition(int x,int y);
extern int GfuiTipCreate(void *scr, const char *text, int maxlen);
extern int GfuiTitleCreate(void *scr, const char *text, int maxlen);

extern void GfuiLabelSetText(void *scr, int id, const char *text);
extern void GfuiLabelSetColor(void *scr, int id, const float * colorPtr);

extern void GfuiPrintString(const char *text, float *fgColor, int font, int x, int y, int align);
extern int  GfuiFontHeight(int font);
extern int  GfuiFontWidth(int font, const char *text);


/* buttons */
#define GFUI_BTNSZ      300
extern int GfuiButtonCreate(void *scr, const char *text, int font,
                            int x, int y, int width, int align, int mouse,
                            void *userDataOnPush, tfuiCallback onPush, 
                            void *userDataOnFocus, tfuiCallback onFocus, tfuiCallback onFocusLost);
extern int GfuiButtonStateCreate(void *scr, const char *text, int font, int x, int y, int width, int align, int mouse,
                                 void *userDataOnPush, tfuiCallback onPush, 
                                 void *userDataOnFocus, tfuiCallback onFocus, tfuiCallback onFocusLost);
extern int GfuiGrButtonCreate(void *scr, const char *disabled, const char *enabled, const char *focused, const char *pushed,
                              int x, int y, int align, int mouse,
                              void *userDataOnPush, tfuiCallback onPush, 
                              void *userDataOnFocus, tfuiCallback onFocus, tfuiCallback onFocusLost);
extern int GfuiGrButtonCreateEx(void *scr, const char *disabled, const char *enabled, const char *focused, const char *pushed,
                   int x, int y, int imageWidth,int imageHeight,int align, int mouse,
                   void *userDataOnPush, tfuiCallback onPush, 
                   void *userDataOnFocus, tfuiCallback onFocus, tfuiCallback onFocusLost);
extern int GfuiComboboxCreate(void *scr, int font, int x, int y, int width,int align ,int style,const char *pszText,tfuiComboCallback onChange);
extern int GfuiCheckboxCreate(void *scr, int font, int x, int y, int imagewidth,int imageheight,int align ,int style,const char *pszText,bool bChecked,tfuiCheckboxCallback onChange);


extern unsigned int GfuiComboboxAddText(void *scr, int id, const char *text);
extern void GfuiComboboxSetSelectedIndex(void *scr, int id, unsigned int index);

extern int GfuiProgressbarCreate(void *scr, int x, int y, int w, int h, const char *pszProgressbackImg,const char *progressbarimg, int align,float min,float max,float value);


extern int GfuiGrButtonCreateEx(void *scr, const char *disabled, const char *enabled, const char *focused, const char *pushed,
		   int x, int y, int imageWidth,int imageHeight,int align, int mouse,
		   void *userDataOnPush, tfuiCallback onPush, 
		   void *userDataOnFocus, tfuiCallback onFocus, tfuiCallback onFocusLost);


extern void GfuiButtonShowBox(void *scr, int id,bool bShow);
extern void GfuiButtonSetColor(void *scr, int id,Color color);
extern void GfuiButtonSetFocusColor(void *scr, int id,Color focuscolor);
extern void GfuiButtonSetPushedColor(void *scr, int id,Color pushcolor);
extern void GfuiButtonSetImage(void *scr,int id,int x,int y,int w,int h,const char *disableFile,const char *enableFile,const char*focusedFile,const char *pushedFile);
extern void GfuiCheckboxSetChecked(void *scr, int id, bool bChecked);


extern void GfuiButtonSetText(void *scr, int id, const char *text);
extern int GfuiButtonGetFocused(void);

/* Edit Box */
extern int GfuiEditboxCreate(void *scr, const char *text, int font, int x, int y, int width, int maxlen,
                             void *userDataOnFocus, tfuiCallback onFocus, tfuiCallback onFocusLost);
extern int GfuiEditboxGetFocused(void);
extern char *GfuiEditboxGetString(void *scr, int id);
extern void GfuiEditboxSetString(void *scr, int id, const char *text);
extern void GfuiEditboxSetColor(void *scr, int id,Color color);
extern void GfuiEditboxSetFocusColor(void *scr, int id,Color focuscolor);


/* Scrolling lists */
extern int GfuiScrollListCreate(void *scr, int font, int x, int y, int align,
                                int width, int height, int scrollbar, void *userDataOnSelect, tfuiCallback onSelect);
extern int GfuiScrollListInsertElement(void *scr, int Id, const char *element, int index, void *userData);
extern int GfuiScrollListMoveSelectedElement(void *scr, int Id, int delta);
extern const char *GfuiScrollListExtractSelectedElement(void *scr, int Id, void **userData);
extern const char *GfuiScrollListExtractElement(void *scr, int Id, int index, void **userData);
extern const char *GfuiScrollListGetSelectedElement(void *scr, int Id, void **userData);
extern bool GfuiScrollListSetSelectedElement(void *scr, int Id, unsigned int selectElement);
extern const char *GfuiScrollListGetElement(void *scr, int Id, int index, void **userData);
extern void GfuiScrollListShowElement(void *scr, int Id, int index);
extern void GfuiScrollListSetColor(void *scr, int id,Color color);
extern void GfuiScrollListSetSelectColor(void *scr, int id,Color color);


/* scroll bars */
extern int GfuiScrollBarCreate(void *scr, int x, int y, int align, int width, int orientation,
                               int min, int max, int len, int start, 
                               void *userData, tfuiSBCallback onScroll);
extern void GfuiScrollBarPosSet(void *scr, int id, int min, int max, int len, int start);
extern int GfuiScrollBarPosGet(void *scr, int id);

/* Images */
extern int GfuiStaticImageCreate(void *scr, int x, int y, int w, int h, const char *name);
extern int GfuiStaticImageCreateEx(void *scr, int x, int y, int w, int h, const char *name, int align);
extern void GfuiStaticImageSet(void *scr, int id, const char *name);
extern void GfuiStaticImageSetEx(void *scr, int id, const char *name,unsigned int index);
extern void GfuiStaticImageSetActive(void *scr, int id, int index);

/*****************************
 * Menu Management Interface *
 *****************************/

extern void *GfuiMenuScreenCreate(const char *title);
extern void  GfuiMenuDefaultKeysAdd(void *scr);
extern int   GfuiMenuButtonCreate(void *menu, const char *text, const char *tip, void *userdata, const int style, tfuiCallback onpush);
extern int   GfuiMenuBackQuitButtonCreate(void *menu, const char *text, const char *tip, void *userdata, tfuiCallback onpush);

/*******************************************
 * New XML based Menu Management Interface *
 *******************************************/

extern void *LoadMenuXML(const char *pFilePath);
extern bool CreateStaticControls(void *param,void *menuHandle);

extern int CreateButtonControl(void *menuHandle,void *param,const char *pControlName,void *userdata, tfuiCallback onpush);
extern int CreateButtonControlEx(void *menuHandle,void *param,const char *pControlName,void *userdata, tfuiCallback onpush, void *userDataOnFocus, tfuiCallback onFocus, tfuiCallback onFocusLost);
extern int CreateStaticImageControl(void *menuHandle,void *param,const char *pControlName);
extern int CreateLabelControl(void *menuHandle,void *param,const char *pControlName);
extern int CreateEditControl(void *menuHandle,void *param,const char *pControlName,void *userDataOnFocus, tfuiCallback onFocus, tfuiCallback onFocusLost);
extern int CreateScrollListControl(void *menuHandle,void *param,const char *pControlName,void *userdata, tfuiCallback onSelect);
extern int CreateComboboxControl(void *menuHandle,void *param,const char *pControlName,tfuiComboCallback onChange);
extern int CreateCheckboxControl(void *menuHandle,void *param,const char *pControlName,tfuiCheckboxCallback onChange);
extern int CreateProgressbarControl(void *menuHandle,void *param,const char *pControlName);

extern void GfuiButtonShowBox(void *scr, int id,bool bShow);
extern void GfuiButtonSetColor(void *scr, int id,Color color);
extern void GfuiButtonSetFocusColor(void *scr, int id,Color focuscolor);
extern void GfuiButtonSetPushedColor(void *scr, int id,Color pushcolor);
extern void GfuiButtonSetImage(void *scr,int id,int x,int y,int w,int h,const char *disableFile,const char *enableFile,const char*focusedFile,const char *pushedFile);

/*****************************
 * Texture / image interface *
 *****************************/

// TODO: Add a GfTexReadImageFromFile for implicit source file format 
// (would call GfTexReadPng/Jpeg ... according to the file extension,
//  and probably also return the number of bytes per pixel)
extern unsigned char *GfTexReadPng(const char *filename, int *widthp, int *heightp, float gamma, int *pow2_widthp, int *pow2_heightp);
extern unsigned char *GfTexReadJpeg(const char *filename, int *widthp, int *heightp, float screen_gamma, int *pow2_widthp, int *pow2_heightp);

extern int GfTexWritePng(unsigned char *img, const char *filename, int width, int height);
extern void GfTexFreeTex(GLuint tex);
extern GLuint GfTexReadTex(const char *filename);
extern GLuint GfTexReadTex(const char *filename,int &height,int &width);


/*********************
 * Control interface *
 *********************/

#define GFCTRL_TYPE_NOT_AFFECTED        0
#define GFCTRL_TYPE_JOY_AXIS            1
#define GFCTRL_TYPE_JOY_BUT             2
#define GFCTRL_TYPE_KEYBOARD            3
#define GFCTRL_TYPE_MOUSE_BUT           4
#define GFCTRL_TYPE_MOUSE_AXIS          5
#define GFCTRL_TYPE_SKEYBOARD           6

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

extern tCtrlJoyInfo *GfctrlJoyInit(void);
extern int GfctrlJoyIsPresent(void);
extern int GfctrlJoyGetCurrent(tCtrlJoyInfo *joyInfo);
extern void GfctrlJoyRelease(tCtrlJoyInfo *joyInfo);


/** Mouse information structure */
typedef struct
{
    int         edgeup[3];      /**< Button transition from down (pressed) to up */
    int         edgedn[3];      /**< Button transition from up to down */
    int         button[3];      /**< Button state (1 = up) */
    float       ax[4];          /**< mouse axis position (mouse considered as a joystick) */
} tCtrlMouseInfo;

extern tCtrlMouseInfo *GfctrlMouseInit(void);
extern int GfctrlMouseGetCurrent(tCtrlMouseInfo *mouseInfo);
extern void GfctrlMouseRelease(tCtrlMouseInfo *mouseInfo);
extern void GfctrlMouseCenter(void);
extern void GfctrlMouseInitCenter(void);
extern tCtrlRef *GfctrlGetRefByName(const char *name);
extern const char *GfctrlGetNameByRef(int type, int index);


#endif /* __TGFCLIENT__H__ */


