/***************************************************************************

    file                 : raceline.h
    created              : Wed Mai 14 19:53:00 CET 2003
    copyright            : (C) 2003-2004 by Bernhard Wymann
    email                : berniw@bluewin.ch
    version              : $Id: raceline.h,v 1.1 2008/02/11 00:53:10 andrew Exp $

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _MOD_H_
#define _MOD_H_

#define LMOD_DATA 200

typedef struct {
  double dval;
  int ival;
  int divstart;
  int divend;
} LRLModData;

typedef struct {
  LRLModData data[LMOD_DATA];
  int used;
} LRLMod;


static void AddMod( LRLMod *mod, int divstart, int divend, double dval, int ival )
{
 if (!mod) return;

 mod->data[mod->used].divstart = divstart;
 mod->data[mod->used].divend = divend;
 mod->data[mod->used].dval = dval;
 mod->data[mod->used].ival = ival;
 mod->used++;
}


static double GetModD( LRLMod *mod, int div )
{
 int i;

 if (!mod)
  return 0.0;

 for (i=0; i<mod->used; i++)
 {
  if (div >= mod->data[i].divstart && div <= mod->data[i].divend)
   return mod->data[i].dval;
 }
 return 0.0;
}

static int GetModI( LRLMod *mod, int div )
{
 int i;

 if (!mod)
  return 0;

 for (i=0; i<mod->used; i++)
 {
  if (div >= mod->data[i].divstart && div <= mod->data[i].divend)
   return mod->data[i].ival;
 }
 return 0;
}

#endif
