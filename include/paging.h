#ifndef _PAGING_H
#define _PAGING_H

#include <squaros.h>

#define WORKING_MEMSTART ((UINT) &kernel_end)   /* By the way, this will be a minumum of required space, what a suprise ;-) */
#define WORKING_MEMEND 		memory_end	/* And this is accessable maximum of memory */
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
	UINT physTabs[1024];		//Must be first, so I can use _kmalloc_pa
	UINT physPos;
	page_table *tables[1024];
};

extern UINT kernel_end;		//Defined in link.ld
extern ULONG memory_end;	//Defined in main.c

#endif
