/*
 *  Copyright (C) 2007,2008 Sven Köhler
 *
 *  This file is part of Nupkux.
 *
 *  Nupkux is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Nupkux is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Nupkux.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _MM_H
#define _MM_H

#include <kernel.h>
#include <paging.h>
#include <lib/memory.h>

//#define WORKING_MEMSTART ((UINT) &kernel_end)   /* By the way, this will be a minumum of required space, what a suprise ;-) */
#define WORKING_MEMSTART 	(__working_memstart)
#define WORKING_MEMEND 		(memory_end)	/* And this is accessable maximum of memory */

#define MM_MAGIC	0x68747552
#define MM_KHEAP_START	0x100000	//1MB after kmalloc_pos
#define MM_KHEAP_SIZE	0x300000	//Add this to above and clear up the mess!
#define MM_KHEAP_MIN	0x100000
#define MM_INDEX_COUNT	0x400

#define MM_FLAG_BLOCK	0
#define MM_FLAG_HOLE	1

#define MM_NO_HOLE	-1

typedef struct _mm_header mm_header;
typedef struct _mm_footer mm_footer;
typedef struct _heap heap;

struct _mm_header {
	UINT magic;
	UINT size: 31;
	UCHAR flag: 1; 
}; 

struct _mm_footer {
	UINT magic;
	mm_header *header;
}; 

struct _heap {
	mm_header **entries;
	UINT entrycount;
	UINT maxentries;
	UINT start;
	UINT end;
	UINT memend;
	UINT pageflags;
};

extern UINT _kmalloc(UINT sz);

extern void *malloc(UINT size);
extern void *calloc(UINT num, UINT size);
extern void free(void *ptr);
extern void *realloc(void *ptr, UINT size);

extern heap *create_heap(UINT start, UINT end, UINT memend, UINT pageflags);

#endif
