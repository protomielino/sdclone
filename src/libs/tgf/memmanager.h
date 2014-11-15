/***************************************************************************
	memmanager.h -- Interface file for The Memory Manager
                             -------------------                                         
    created              : Wed Nov 12 17:54:00:00 CEST 2014
    copyright            : (C) 2014 by Wolf-Dieter Beelitz
    email                : wdb at wdbee.de
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

#ifndef __MEMORYMANAGER__H__
#define __MEMORYMANAGER__H__

#include <cstdio>
#include "tgf.h"

// WDB test ...
#ifdef __DEBUG_MEMORYMANAGER__

//
// Interface
//
TGF_API bool GfMemoryManagerAllocate(void);	// Initialize memory manager
TGF_API void GfMemoryManagerRelease(bool Dump = true);	// Release memory manager at Shutdown
TGF_API bool GfMemoryManagerRunning(void);	// Is the memory manager running?
TGF_API void GfMemoryManagerSetup(int AddedSpace);
TGF_API void GfMemoryManagerDoAccept(void);
TGF_API void GfMemoryManagerDoFree(void);
//

//
// Implementation
//

// Memory manager states
#define GF_MM_STATE_NULL 0	// memory manager was created
#define GF_MM_STATE_INIT 1	// memory manager was initialized
#define GF_MM_STATE_USED 2	// memory manager was used

// Memory manager allocation types
#define GF_MM_ALLOCTYPE_MEMMAN 0	// allocation for memory manager
#define GF_MM_ALLOCTYPE_NEW 1		// allocation by new
#define GF_MM_ALLOCTYPE_NEWARRAY 2	// allocation by new
#define GF_MM_ALLOCTYPE_MALLOC 3	// allocation by calloc

// Memory manager check markers
#define MM_MARKER_BEGIN 11223344	// Value of marker at start of the block
#define MM_MARKER_ID 123456789		// Value of marker in front of the ID
#define MM_MARKER_END 44332211		// Value of marker at the end of the block

// Memory manager histogram
#define MAXBLOCKSIZE 4096	// Definition of the max block size for histogram
//

// Memory manager worker functions
void* GfMemoryManagerAlloc(size_t size, unsigned int type, void* retAddr);
void GfMemoryManagerFree(void* b, unsigned int type);
void GfMemoryManagerHist(size_t size);
//

// Block to link allocated memory blocks in a 
// double linked list and some additional flags to check
// integrity of block at call of free
typedef struct tDSMMLinkBlock
{	
	unsigned int Mark;		// Marker to identify it as tDSMMLinkBlock
	int Size;				// Size of allocated block
	void* RAdr;				// Return address of new/malloc
	tDSMMLinkBlock* Prev;	// Previous memory block
	tDSMMLinkBlock* Next;	// Next memory block
	unsigned int Type;		// Type of allocation
	unsigned int IDMk;		// Marker to check ID is still valid
	unsigned int BLID;		// ID of allocated memory block

} tDSMMLinkBlock;

//
// Memory Manager
//
typedef struct
{
	tDSMMLinkBlock RootOfList;			// Root of the double linked list
	tDSMMLinkBlock* GarbageCollection;	// Double linked list of allocated memory blocks
	size_t Size;						// Size of memory manager
	int State;							// State of memory manager
	int AddedSpace;						// Number of bytes added to each block
	bool DoNotFree;						// Do not free the blocks if flag is set

	unsigned int BigB;					// Number of big blocks requested
	unsigned int Hist[MAXBLOCKSIZE];	// Histogram of the buufer sizes

} tMemoryManager;
//

#endif // #ifdef __DEBUG_MEMORYMANAGER__
// ... WDB test

#endif /* __MEMORYMANAGER__H__ */


