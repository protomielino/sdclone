/***************************************************************************
	memmanager.h -- Interface file for The Memory Manager
                             -------------------                                         
    created              : Wed Nov 12 17:54:00:00 CEST 2014
    copyright            : (C) 2014 by Wolf-Dieter Beelitz
    email                : wdb at wdbee.de
    version              : $Id: memmanager.h $
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
TGF_API bool GfMemoryManagerAllocate();	// Initialize memory manager
TGF_API void GfMemoryManagerRelease();	// Release memory manager at Shutdown
TGF_API bool GfMemoryManagerRunning();	// Is the memory manager running?
TGF_API void GfMemoryManagerSetup(int AddedSpace);
//

//
// Implementation
//

// Memory manager states
#define GF_MM_STATE_NULL 0	// memory manager was created
#define GF_MM_STATE_INIT 1	// memory manager was initialized
#define GF_MM_STATE_USED 2	// memory manager was used

#define GF_MM_ALLOCTYPE_MEMMAN 0	// allocation for memory manager
#define GF_MM_ALLOCTYPE_NEW 1		// allocation by new
#define GF_MM_ALLOCTYPE_NEWARRAY 2	// allocation by new
#define GF_MM_ALLOCTYPE_MALLOC 3	// allocation by calloc

#define MM_MARKER 11223344
//

// Memory manager worker functions
void* GfMemoryManagerAlloc(size_t size, unsigned int type, void* RetAddr);
void GfMemoryManagerFree(void* b, unsigned int type);
//

// Block to link allocated memory blocks in a 
// double linked list.
//
typedef struct tDSMMLinkBlock
{	
	unsigned int Mark;		// Marker to identify it as tDSMMLinkBlock
	int Size;				// Size of allocated block
	void* ReturnAddress;    // Return address of new/malloc
	tDSMMLinkBlock* Prev;	// Previous memory block
	tDSMMLinkBlock* Next;	// Next memory block
	unsigned int Type;		// Type of allocation
	unsigned int ID;		// ID of allocated memory block

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

} tMemoryManager;
//

#endif // #ifdef __DEBUG_MEMORYMANAGER__
// ... WDB test

#endif /* __MEMORYMANAGER__H__ */


