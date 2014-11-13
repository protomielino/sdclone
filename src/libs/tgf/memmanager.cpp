/***************************************************************************
	memmanager.cpp -- The Memory Manager
                             -------------------                                         
    created              : Wed Nov 12 17:54:00:00 CEST 2014
    copyright            : (C) 2014 by Wolf-Dieter Beelitz
    email                : wdb at wdbee.de
    version              : $Id: memmanager.cpp $
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// WDB test ...
#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h> // getcwd, access
#endif

#include "memmanager.h"
#ifdef __DEBUG_MEMORYMANAGER__

#include <stdio.h>
#include <intrin.h>

#pragma intrinsic(_ReturnAddress)

//
// Configuration (depending on the compiler)
//
//#define ANSI_ISO  // ANSI/ISO compliant behavior: exeption if allo failed
#undef ANSI_ISO		// Older Visual C++ versions returning NULL instad of exception

//
// Private variables
//
static tMemoryManager* GfMM = NULL;		// The one and only memory manager!
static unsigned int GfMM_Counter = 0;	// Counter of memory blocks
//

//
// Override the global new operator
//
void* operator new (size_t size)
{
#if defined(WIN32) // Has to be replaced by a definition of VC++ versus gcc
	void* RetAddr = _ReturnAddress(); // VC++
#else
	void* RetAddr = __builtin_return_address (0);  // gcc
#endif
	return GfMemoryManagerAlloc(size, GF_MM_ALLOCTYPE_NEW,RetAddr);
}
//

//
// Override the global delete operator
//
void operator delete (void *b)
{
	GfMemoryManagerFree(b, GF_MM_ALLOCTYPE_NEW);
}
//

#if defined(WIN32)

#include <crtdbg.h>
#include <assert.h>

void * _tgf_win_malloc(size_t size)
{
#if defined(WIN32) // Has to be replaced by a definition of VC++ versus gcc
	void* RetAddr = _ReturnAddress(); // VC++
#else
	void* RetAddr = __builtin_return_address (0); // gcc
#endif
	return GfMemoryManagerAlloc(size, GF_MM_ALLOCTYPE_MALLOC,RetAddr);
}

void _tgf_win_free(void * b)
{
	GfMemoryManagerFree(b, GF_MM_ALLOCTYPE_MALLOC);
}

void _tgf_win_accept(void * b)
{
	GfMemoryManagerAccept(b, GF_MM_ALLOCTYPE_MALLOC);
}

void * _tgf_win_calloc(size_t num, size_t size)
{
	void * p = _tgf_win_malloc(num * size);
	memset(p, 0, num * size);
	return p;
}


void * _tgf_win_realloc(void * memblock, size_t size)
{
	if (size == 0) {
		_tgf_win_free(memblock);
		return NULL;
	}

	void * p = _tgf_win_malloc(size);
	if (p == NULL) {
		return NULL;
	}

	if (memblock != NULL) 
	{
		if (GfMemoryManagerRunning())
		{
			size_t s = MIN(*(int*)((char*)memblock - sizeof(tDSMMLinkBlock) - sizeof(int) - GfMM->AddedSpace), (int)size);
			memcpy(p, memblock, s);
		}
		else
		{
			size_t s = MIN(*(int*)((char*)memblock - sizeof(int)), (int)size);
			memcpy(p, memblock, s);
		}

		_tgf_win_free(memblock);
	}
	return p;
}


char * _tgf_win_strdup(const char * str)
{
	char * s = (char*)_tgf_win_malloc(strlen(str)+1);
	strcpy(s,str);

	return s;
}
#endif // if defined(WIN32)
//

//
// Create the one and only global memory manager (allocate memory for data)
//
tMemoryManager* GfMemoryManager()
{
    tMemoryManager* MemoryManager = (tMemoryManager*)  
		malloc(sizeof(tMemoryManager));            

	MemoryManager->Size = sizeof(tMemoryManager);	
	MemoryManager->State = GF_MM_STATE_NULL;     
	MemoryManager->AddedSpace = 0;

	MemoryManager->RootOfList.Mark = MM_MARKER;
	MemoryManager->RootOfList.Type = GF_MM_ALLOCTYPE_MEMMAN;
	MemoryManager->RootOfList.Prev = NULL;
	MemoryManager->RootOfList.Next = NULL;
	MemoryManager->RootOfList.Size = sizeof(tMemoryManager);	
	MemoryManager->RootOfList.ID = GfMM_Counter++;	

	MemoryManager->GarbageCollection = (tDSMMLinkBlock*) MemoryManager;

	return MemoryManager;
}
//

//
// Allocate memory
//
void* GfMemoryManagerAlloc (size_t size, unsigned int Type, void* RetAddr)
{
	if (GfMemoryManagerRunning())
	{
		// Need additional space for linked list
		int blocksize = sizeof(tDSMMLinkBlock) + sizeof(int) + size + GfMM->AddedSpace;
		tDSMMLinkBlock* c = (tDSMMLinkBlock*) GlobalAlloc(GMEM_FIXED, blocksize); 
		if (c == NULL)				// did malloc succeed?
#ifdef ANSI_ISO
			throw std::bad_alloc();	// ANSI/ISO compliant behavior
#else
			return c;
#endif

		if (GfMM->RootOfList.Next != NULL)
		{
			tDSMMLinkBlock* n = GfMM->RootOfList.Next;
			n->Prev = c;
			c->Next = n;
		}
		else
			c->Next = NULL;

		GfMM->RootOfList.Next = c;
		c->Prev = &GfMM->RootOfList;

		c->Mark = MM_MARKER;
		c->Type = Type;
		c->Size = size;
		c->ReturnAddress = RetAddr;
		int ID = c->ID = GfMM_Counter++;

		// Now we have here
		// c: tDSMMLinkBlock* to the current linked list data block
		// n: tDSMMLinkBlock* to the next linked list data block

		int* s = (int *) ++c;
		*s = size;

		// Now we have here
		// s: (int*) pointer to the size of the allocated block

		void* b = (void *) ++s;

		// Now we have here
		// b: (void*) official pointer to the new data block

		// Hunting memory leaks ...
#define	IDTOSTOP 343884	// ID of block you are looking for

		if (ID == IDTOSTOP)
		{
			ID = 0;	// set breakpoint here 
					// to stop at allocation of 
					// block with ID = IDTOSTOP
		}
		// ... Hunting memory leaks

		return b;
	}
	else
	{
		int* b = (int*) GlobalAlloc(GMEM_FIXED, sizeof(int) + size); 
		if (b == NULL)				// did malloc succeed?
		{
#ifdef ANSI_ISO
			throw std::bad_alloc();	// ANSI/ISO compliant behavior
#else
			return b;
#endif
		}
		*b = size;
		return ++b;
	}
}
//

//
// Release memory
//
void GfMemoryManagerFree (void *b, unsigned int type)
{
	if (b == NULL)
		return;

	if (GfMemoryManagerRunning())
	{
		// Get start of data block ...
		int* s = (int*) b;
		tDSMMLinkBlock* c = (tDSMMLinkBlock*) --s;
		c = --c;
		// ... Get start of data block

		// Now we have here
		// b: (void*) official pointer to data block
		// s: (in*) to size of allocated block
		// c: (tDSMMLinkBlock*) to the current linked list data block
		// n: (tDSMMLinkBlock*) to the next linked list data block
		// p: (tDSMMLinkBlock*) to the previous linked list data block

		if (c->Type != type)
		{
			fprintf(stderr,"Block address %p\n",c);
			if (c->Mark == MM_MARKER)
				fprintf(stderr,"operator delete called with data of wrong type\n");
			else
				fprintf(stderr,"operator delete wrong data\n");
		}
		else
		{
			tDSMMLinkBlock* n = c->Next;
			tDSMMLinkBlock* p = c->Prev;
			if (c->Mark == MM_MARKER)
			{
				p->Next = n;
				if ((n != NULL) && (n->Mark == MM_MARKER))
					n->Prev = p;
				GlobalFree(c);
			}
		}
	}
	else
		GlobalFree(b); 
}
//

//
// Accept memory
//
void GfMemoryManagerAccept (void *b, unsigned int type)
{
	if (b == NULL)
		return;

	if (GfMemoryManagerRunning())
	{
		// Get start of data block ...
		int* s = (int*) b;
		tDSMMLinkBlock* c = (tDSMMLinkBlock*) --s;
		c = --c;
		// ... Get start of data block

		// Now we have here
		// b: (void*) official pointer to data block
		// s: (in*) to size of allocated block
		// c: (tDSMMLinkBlock*) to the current linked list data block
		// n: (tDSMMLinkBlock*) to the next linked list data block
		// p: (tDSMMLinkBlock*) to the previous linked list data block

		if (c->Type != type)
		{
			fprintf(stderr,"Block address %p\n",c);
			if (c->Mark == MM_MARKER)
				fprintf(stderr,"operator delete called with data of wrong type\n");
			else
				fprintf(stderr,"operator delete wrong data\n");
		}
		else
		{
			tDSMMLinkBlock* n = c->Next;
			tDSMMLinkBlock* p = c->Prev;
			if (c->Mark == MM_MARKER)
			{
				fprintf(stderr,"accept block %d\n",c->ID);
				p->Next = n;
				if ((n != NULL) && (n->Mark == MM_MARKER))
					n->Prev = p;
			}
		}
	}
}
//

//
// Setup data of memory manager
//
void GfMemoryManagerSetup(int AddedSpace)
{
	if (GfMM != NULL)
	{
		if (GfMM->State != GF_MM_STATE_INIT)
		{
			// Setup data for memory manager
			GfMM->AddedSpace = AddedSpace;
		}
		GfMM->State = GF_MM_STATE_INIT; 
	}
}
//

//
// Initialize the global memory manager
//
bool GfMemoryManagerAllocate()
{
	if (GfMM == NULL)
	{
		GfMM = GfMemoryManager();
		return true;
	}
	else
	{
		if (GfMM->State != GF_MM_STATE_NULL)
		{
			GfMemoryManagerRelease();
			GfMM = NULL;     
			return GfMemoryManagerAllocate();
		}
	}
	return false;
}
//

//
// Destroy the one and only global memory manager and it's allocated data
//
void GfMemoryManagerRelease()
{
	int LeakSizeTotal = 0;
	int LeakSizeNewTotal = 0;
	int LeakSizeMallocTotal = 0;

	int MaxLeakSizeTotal = 0;
	int MaxLeakSizeNewTotal = 0;
	int MaxLeakSizeMallocTotal = 0;

    if (GfMM != NULL)                           
	{
		tDSMMLinkBlock* Block = GfMM->GarbageCollection;
		GfMM = NULL;                           

		tDSMMLinkBlock* CurrentBlock = Block->Next;

		int n = 0;
		while (CurrentBlock)
		{
			tDSMMLinkBlock* ToFree = CurrentBlock;
			CurrentBlock = CurrentBlock->Next;

			if (ToFree->Mark == MM_MARKER)
			{
				LeakSizeTotal += ToFree->Size;
				if (MaxLeakSizeTotal < ToFree->Size)
					MaxLeakSizeTotal = ToFree->Size;

				if (ToFree->Type == 1)
				{
					fprintf(stderr,"%04.4d Block: %04.4d Size: %06.6d ReturnAddress: %p BlockAddress: %p Type: new/delete\n",
						++n,ToFree->ID,ToFree->Size,ToFree->ReturnAddress,ToFree);
					LeakSizeNewTotal += ToFree->Size;
					if (MaxLeakSizeNewTotal < ToFree->Size)
						MaxLeakSizeNewTotal = ToFree->Size;
					delete(ToFree);
				}
				else
				{
					fprintf(stderr,"%04.4d Block: %04.4d Size: %06.6d ReturnAddress: %p BlockAddress: %p Type: malloc/free\n",
						++n,ToFree->ID,ToFree->Size,ToFree->ReturnAddress,ToFree);
					LeakSizeMallocTotal += ToFree->Size;
					if (MaxLeakSizeMallocTotal < ToFree->Size)
						MaxLeakSizeMallocTotal = ToFree->Size;
					free(ToFree);
				}
			}
			else
			{
				fprintf(stderr,"%d Block corrupted\n",++n);
				CurrentBlock = NULL;
			}
		}

		fprintf(stderr,"\nMemory manager leak statistics:\n\n");

		fprintf(stderr,"Number of allocated blocks     : %d\n",GfMM_Counter);
		fprintf(stderr,"Number of memory leaks         : %d\n\n",n);

		fprintf(stderr,"Total leak size new/delete     : %d [Byte]\n",LeakSizeNewTotal);
		fprintf(stderr,"Total leak size malloc/free    : %d [Byte]\n",LeakSizeMallocTotal);
		fprintf(stderr,"Total leak size total          : %d [Byte]\n\n",LeakSizeTotal);

		fprintf(stderr,"Max leak block size new/delete : %d [Byte]\n",MaxLeakSizeNewTotal);
		fprintf(stderr,"Max leak block size malloc/free: %d [Byte]\n",MaxLeakSizeMallocTotal);
		fprintf(stderr,"Max leak block size total      : %d [Byte]\n\n",MaxLeakSizeTotal);

		delete(Block); // Delete the memory manager itself
	}

	fprintf(stderr,"Press [Enter] to close the program\n");
	getchar();
}
//

//
// Check memory manager
//
bool GfMemoryManagerRunning()
{
	if (GfMM != NULL)
		return true;
	else
		return false;
}

#endif
// ... WDB test

