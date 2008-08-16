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

#ifndef _PAGING_H
#define _PAGING_H

#include <kernel.h>
#include <mm.h>

#define FRAME_SIZE		0x1000
#define PAGE_FLAG_PRESENT	0x01
#define PAGE_FLAG_WRITE		0x02
#define PAGE_FLAG_USERMODE	0x04
//not for "daily use"
#define PAGE_FLAG_ACCESSED	0x20
#define PAGE_FLAG_DIRTY		0x40
//dummies
#define PAGE_FLAG_NOTPRESENT	0x00
#define PAGE_FLAG_READONLY	0x00
#define PAGE_FLAG_KERNELMODE	0x00

#define KERNEL_PAGE_BUFFER	0x32000

#define CHECK_ALIGN(VALUE)	((VALUE) & 0x00000FFF)
#define ASSERT_ALIGN(VALUE)	if (CHECK_ALIGN(VALUE)) {	\
					VALUE&=0xFFFFF000;	\
					VALUE+=FRAME_SIZE;	\
				}

typedef struct _page_directory page_directory;
typedef struct _page_table page_table;
typedef struct _page page;

struct _page { 
   UINT flags: 12;
   UINT frame: 20;
};

struct _page_table {
	page entries[1024];
};

struct _page_directory {
	UINT physTabs[1024];  //Must be first, so I can use _kmalloc_pa
	UINT physPos;
	page_table *tables[1024];
};

extern UINT kernel_end;		//Defined in link.ld
extern ULONG memory_end;	//Defined in main.c
extern UINT __working_memstart;
extern page_directory *current_directory;

extern page_directory* clone_directory(page_directory* src);
extern void free_directory(page_directory *dir);
extern page *make_page(UINT address, UINT flags, page_directory *directory, int alloc);
extern page *get_page(UINT address, int make, page_directory *directory);
extern page *free_page(UINT address, page_directory *directory);
extern void setup_paging(void);

#endif
