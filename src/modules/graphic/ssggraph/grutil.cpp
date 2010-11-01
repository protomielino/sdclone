/***************************************************************************

    file                 : grutil.cpp
    created              : Wed Nov  1 21:33:22 CET 2000
    copyright            : (C) 2000 by Eric Espie
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

#ifdef WIN32
#include <windows.h>
#endif

#include <plib/ssg.h>

#include <tgfclient.h>

#include "grutil.h"
#include "grscene.h"
#include "grtexture.h"

float grGammaValue = 1.8;
int grMipMap = 0;

char *grFilePath;			// Multiple path (';' separated) used to search for files.


int grGetFilename(const char *filename, const char *filepath, char *buf)
{
	const char *c1, *c2;
	int found = 0;
	int lg;

	if (filepath) {
		c1 = filepath;
		c2 = c1;
		while (!found && c2) {
			c2 = strchr(c1, ';');
			if (c2 == NULL) {
				sprintf(buf, "%s/%s", c1, filename);
			} else {
				lg = c2 - c1;
				strncpy(buf, c1, lg);
				buf[lg] = '/';
				strcpy(buf + lg + 1, filename);
			}
			if (ulFileExists(buf)) {
				found = 1;
			}
			c1 = c2 + 1;
		}
	} else {
		strcpy(buf, filename);
		if (ulFileExists(buf)) {
			found = 1;
		}
	}

	return found;
}


typedef struct stlist
{
    struct stlist	*next;
    grManagedState *state;
    char		*name;
} stlist;


static stlist * stateList = NULL;


static grManagedState * grGetState(const char *img)
{
    stlist	*curr;

    curr = stateList;
    while (curr != NULL) {
	if (strcmp(curr->name, img) == 0) {
	    return curr->state;
	}
	curr = curr->next;
    }
    return NULL;
}


void grShutdownState(void)
{
	stlist *curr;
	stlist *next;

	curr = stateList;
	while (curr != NULL) {
		next = curr->next;
		//GfTrace("Still in list : %s\n", curr->name);
		free(curr->name);
		//ssgDeRefDelete(curr->state);
		free(curr);
		curr = next;
	}
	stateList = NULL;
}


static void grSetupState(grManagedState *st, char *buf)
{
	st->ref();			// cannot be removed
	st->enable(GL_LIGHTING);
	st->enable(GL_TEXTURE_2D);
	st->enable(GL_BLEND);
	st->setColourMaterial(GL_AMBIENT_AND_DIFFUSE);	

	stlist *curr = (stlist*)calloc(sizeof(stlist), 1);
	curr->next = stateList;
	stateList = curr;
	curr->state = st;
	curr->name = strdup(buf);

	GfLogTrace("Loading texture %s\n", buf);
}


ssgState * grSsgLoadTexState(const char *img, int errIfNotFound)
{
	char buf[256];
	const char *s;
	grManagedState *st; 

	// remove the directory
	s = strrchr(img, '/');
	if (s == NULL) {
		s = img;
	} else {
		s++;
	}

	if (!grGetFilename(s, grFilePath, buf)) {
		if (errIfNotFound)
			GfLogError("Texture file %s not found in %s\n", s, grFilePath);
		return NULL;
	}

	st = grGetState(buf);
	if (st != NULL) {
		return (ssgState*)st;
	}

	st = grStateFactory();
	grSetupState(st, buf);
	st->setTexture(buf);
	
	return (ssgState*)st;
}

ssgState * grSsgEnvTexState(const char *img, int errIfNotFound)
{
	char buf[256];
	const char *s;
	grMultiTexState *st;

	// remove the directory
	s = strrchr(img, '/');
	if (s == NULL) {
		s = img;
	} else {
		s++;
    }

	if (!grGetFilename(s, grFilePath, buf)) {
		if (errIfNotFound)
			GfLogError("Env. texture file %s not found in %s\n", s, grFilePath);
		return NULL;
    }

	st = new grMultiTexState;
	grSetupState(st, buf);
	st->setTexture(buf);

	return (ssgState*)st;
}

ssgState *
grSsgLoadTexStateEx(const char *img, const char *filepath,
					int wrap, int mipmap, int errIfNotFound)
{
	char buf[256];
	const char *s;
	grManagedState *st; 

	// remove the directory path
	s = strrchr(img, '/');
	if (s == NULL) {
		s = img;
	} else {
		s++;
	}

	if (!grGetFilename(s, filepath, buf)) {
		if (errIfNotFound)
			GfLogError("Texture file (ex) %s not found in %s\n", s, filepath);
		return NULL;
	}

	st = grGetState(buf);
	if (st != NULL) {
		return (ssgState*)st;
	}

	st = grStateFactory();
	grSetupState(st, buf);
	st->setTexture(buf, wrap, wrap, mipmap);

	return (ssgState*)st;
}


/*
 * 
 * name: grWriteTime
 * Formats and outputs the time as a string.
 * 
 * @param color: colour to use
 * @param font: font to use
 * @param x: X coord to start output
 * @param y: Y coord to start output
 * @param sec: time to format, in seconds
 * @param sgn: whether use +/- signs or not
 * 
 * @return
 */
void  grWriteTime(float *color, int font, int x, int y, tdble sec, int sgn)
{
	char  buf[256];
	
	grWriteTimeBuf(buf, sec, sgn);
  GfuiPrintString(buf, color, font, x, y, GFUI_ALIGN_HR_VB);
}


/*
 * 
 * name: grWriteTimeBuf
 * Formats and write the time into the supplied character buffer.
 * 
 * @param buf: character buffer. Caller is totally responsible for this.
 * @param sec: time to format, in seconds
 * @param sgn: whether use +/- signs or not
 * 
 * @return
 */
void  grWriteTimeBuf(char *buf, tdble sec, int sgn)
{
	const char* sign;

	if (sec < 0.0) {
		sec = -sec;
		sign = "-";
    } else {
		if (sgn) {
			sign = "+";
		} else {
			sign = "  ";
		}
    }

    int h = (int)(sec / 3600.0);
    sec -= 3600 * h;
    int m = (int)(sec / 60.0);
    sec -= 60 * m;
    int s = (int)(sec);
    sec -= s;
    int c = (int)floor((sec) * 100.0);
    if (h) {
			sprintf(buf, "%s%2.2d:%2.2d:%2.2d:%2.2d", sign,h,m,s,c);
    } else if (m) {
			sprintf(buf, "   %s%2.2d:%2.2d:%2.2d", sign,m,s,c);
    } else {
			sprintf(buf, "      %s%2.2d:%2.2d", sign,s,c);
    }
}


// TODO: more efficient solution, this one is slow.
float grGetHOT(float x, float y)
{
	sgVec3 test_vec;
	sgMat4 invmat;
	sgMakeIdentMat4(invmat);

	invmat[3][0] = -x;
	invmat[3][1] = -y;
	invmat[3][2] =  0.0f         ;

	test_vec [0] = 0;
	test_vec [1] = 0;
	test_vec [2] = 100000.0f;

	ssgHit *results;
	int num_hits = ssgHOT (TheScene, test_vec, invmat, &results);
	float hot = -1000000.0f;

	for (int i = 0; i < num_hits; i++) {
		ssgHit *h = &results[i];

		float hgt = (h->plane[2] == 0.0 ? 0.0 : - h->plane[3] / h->plane[2]);

		if (hgt >= hot) {
			hot = hgt;
		}
	}

	return hot;
}



