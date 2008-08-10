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

#include <mm.h>

UINT kmalloc_pos;
heap *kheap = 0;

static void *heap_malloc(UINT size, UCHAR page_align, heap *aheap);

extern page_directory *kernel_directory;

static UINT _kmalloc_base(UINT sz, UINT *phys, UCHAR align)
{
	UINT res;
	
	if (kheap) {
		res=(UINT)heap_malloc(sz,align,kheap);
		if (phys) {
			page *apage = get_page(res,0,kernel_directory);
            		*phys=(apage->frame*FRAME_SIZE)+(res&0xFFF);
		}
	} else {
		if (align) ASSERT_ALIGN(kmalloc_pos);
		if (phys) *phys=kmalloc_pos;
		res=kmalloc_pos;
		kmalloc_pos+=sz;
	}
	return res;
}

UINT _kmalloc_a(UINT sz)
{
	return _kmalloc_base(sz,0,1);
}

UINT _kmalloc(UINT sz)
{
	return _kmalloc_base(sz,0,0);
}

UINT _kmalloc_pa(UINT sz, UINT *phys)
{
	return _kmalloc_base(sz,phys,1);
}

heap *create_heap(UINT start, UINT end, UINT memend, UINT pageflags)
{
	heap *newheap = (heap *)_kmalloc(sizeof(heap));
	mm_header *initholestart;
	mm_footer *initholeend;

	ASSERT_ALIGN(start);
	ASSERT_ALIGN(end);
	ASSERT_ALIGN(memend);
	newheap->entries=(mm_header **)start;
	start+=MM_INDEX_COUNT*sizeof(mm_header *);
	ASSERT_ALIGN(start);
	newheap->start=start;
	newheap->end=end;
	newheap->memend=memend;
	newheap->pageflags=pageflags;

	initholestart=(mm_header *) start;
	initholestart->magic=MM_MAGIC;
	initholestart->size=end-start;
	initholestart->flag=MM_FLAG_HOLE;
	newheap->entries[0]=initholestart;
	
	initholeend=(mm_footer *) (end-sizeof(mm_footer));
	initholeend->magic=MM_MAGIC;
	initholeend->header=newheap->entries[0];
	
	newheap->entrycount=1;
	newheap->maxentries=MM_INDEX_COUNT;
	
	return newheap;
}

static void expand_heap(UINT new_size, heap *aheap)
{
	UINT i;

	if (new_size<=aheap->end-aheap->start) return;
	ASSERT_ALIGN(new_size);
	if (aheap->start+new_size>aheap->memend) return;
	for (i=aheap->end-aheap->start;i<new_size;i+=FRAME_SIZE) 
		make_page(aheap->start+i,aheap->pageflags,kernel_directory,1);
	aheap->end=aheap->start+new_size;
}

static UINT contract_heap(UINT new_size, heap *aheap)
{
	UINT i;

	if (new_size>=aheap->end-aheap->start) return aheap->end-aheap->start;
	ASSERT_ALIGN(new_size);
	if (new_size<MM_KHEAP_MIN) new_size=MM_KHEAP_MIN;
	for (i=aheap->end-aheap->start-FRAME_SIZE;new_size<i;i-=FRAME_SIZE)
		free_page(aheap->start+i,kernel_directory);
	aheap->end=aheap->start+new_size;
	return new_size;
}

static void heap_add_entry(mm_header *entry, heap *aheap)
{
	UINT i;
	mm_header *tmp, *tmp2;

	for (i=0;(i<aheap->entrycount) && (aheap->entries[i]->size<entry->size);i++);
	if (i==aheap->entrycount)
		aheap->entries[aheap->entrycount++]=entry;
	else {
		tmp=aheap->entries[i];
		aheap->entries[i]=entry;
		while (i++<aheap->entrycount) {
			tmp2=aheap->entries[i];
			aheap->entries[i]=tmp;
			tmp=tmp2;
		}
		aheap->entrycount++;
	}	
}

static void heap_del_entry(UINT i, heap *aheap)
{
	if (i==MM_NO_HOLE) return;
	for (;i<aheap->entrycount;i++)
		aheap->entries[i]=aheap->entries[i+1];
	aheap->entrycount--;
}

static UINT heap_find_entry(mm_header *entry, heap *aheap)
{
	UINT i;
	for (i=0;i<aheap->entrycount;i++)
		if (aheap->entries[i]==entry) return i;
	return MM_NO_HOLE;
}

static UINT find_smallest_hole(UINT size, UCHAR page_align, heap *aheap)  //Binary search?
{
	UINT i,location;
	mm_header *entry;
	int offset;

	for (i=0;i<aheap->entrycount;i++) {
		entry=aheap->entries[i];
		if (page_align) {
			location=(UINT)entry;
			offset=0;
			if (CHECK_ALIGN(location+sizeof(mm_header)))
				offset=FRAME_SIZE-(location+sizeof(mm_header))%FRAME_SIZE;
			if ((int)entry->size-offset>=(int)size) return i;
		} else if (entry->size>=size) return i;
	}
	return MM_NO_HOLE;
}

static void *heap_malloc(UINT size, UCHAR page_align, heap *aheap)
{
	UINT newsize=size+sizeof(mm_header)+sizeof(mm_footer);
	UINT hole_pos=find_smallest_hole(newsize,page_align,aheap),oldsize,oldpos,newpos,oldend;
	mm_header *header, *newheader;
	mm_footer *newfooter;

	if ((!size) || (!aheap)) return 0;
	if (hole_pos==MM_NO_HOLE) {
		oldsize=aheap->end-aheap->start;
		oldend=aheap->end;
		expand_heap(oldsize+newsize,aheap);
		newsize=aheap->end-aheap->start; 
		UINT idx=MM_NO_HOLE; UINT value=0;
		for (hole_pos=0;hole_pos<aheap->entrycount;hole_pos++) {
			UINT tmp = (UINT) aheap->entries[hole_pos];
			if (tmp>value) {
				value=tmp;
				idx=hole_pos;
			}
		}
		if (idx==MM_NO_HOLE) {
			header=(mm_header *)oldend;
			header->size=newsize-oldsize;
			header->flag=MM_FLAG_HOLE;
			newfooter=(mm_footer *) (oldend+header->size-sizeof(mm_footer));
			header->magic=newfooter->magic=MM_MAGIC;
			newfooter->header=header;
			heap_add_entry(header,aheap);
		} else  {
           		header=aheap->entries[idx];
			header->size+=newsize-oldsize;
			newfooter=(mm_footer *) ((UINT)header+header->size-sizeof(mm_footer));
			newfooter->header=header;
			newfooter->magic=MM_MAGIC;
		}
		return heap_malloc(size,page_align,aheap);
	}
	header=aheap->entries[hole_pos];
	if (header->size-sizeof(mm_header)-sizeof(mm_footer)<newsize) {
		size+=header->size-newsize;
		newsize=header->size;
	}
	oldpos=(UINT)header;
	oldsize=header->size;
	if (page_align && (oldpos & 0xFFFFF000)) {
		newpos=oldpos+FRAME_SIZE-(oldpos & 0xFFF)-sizeof(mm_header);
		newheader=(mm_header *) oldpos;
		newfooter=(mm_footer *) ((UINT)newpos-sizeof(mm_footer));
		newheader->size=newpos-oldpos;
		newheader->flag=MM_FLAG_HOLE;
		newheader->magic=newfooter->magic=MM_MAGIC;
		newfooter->header=newheader;
		oldpos=newpos;
		oldsize=oldsize-newheader->size;
	} else  {
		heap_del_entry(hole_pos,aheap);
	}
	header=(mm_header *) oldpos;
	newfooter=(mm_footer *) (oldpos+sizeof(mm_header)+size);
	header->flag=MM_FLAG_BLOCK;
	header->size=newsize;
	header->magic=newfooter->magic=MM_MAGIC;
	newfooter->header=header; 
	if (oldsize-newsize>0) {
		newheader=(mm_header *) (oldpos+newsize);
		newheader->magic=MM_MAGIC;
		newheader->flag=MM_FLAG_HOLE;
		newheader->size=oldsize-newsize;
		newfooter = (mm_footer *) ((UINT)newheader+newheader->size-sizeof(mm_footer));
		if ((UINT)newfooter<aheap->end)  {
			newfooter->magic=MM_MAGIC;
			newfooter->header=newheader;
		}
		heap_add_entry(newheader,aheap);
	}
	return (void *) ((UINT)header+sizeof(mm_header));
}

static void heap_free(void *ptr, heap *aheap)
{
	mm_header *header, *tmpheader;
	mm_footer *footer, *tmpfooter;
	UINT oldsize,newsize;
	UCHAR opt = 1;

	if ((!ptr) || (!aheap)) return;
	header=(mm_header*) ((UINT)ptr-sizeof(mm_header));
	footer=(mm_footer*) ((UINT)header+header->size-sizeof(mm_footer));
	if ((header->magic!=MM_MAGIC) || (footer->magic!=MM_MAGIC)) return;
	oldsize=header->size;
	header->flag=MM_FLAG_HOLE;
	tmpfooter=(mm_footer*) ((UINT)header-sizeof(mm_footer));
	if ((tmpfooter->magic==MM_MAGIC) && (tmpfooter->header->magic==MM_MAGIC) && (tmpfooter->header->flag==MM_FLAG_HOLE)) {
		opt=0;
		header=tmpfooter->header;   
		footer->header=header;        
		header->size+=oldsize;     
	}

	tmpheader=(mm_header*) ((UINT)footer+sizeof(mm_footer));
	if ((tmpheader->magic==MM_MAGIC) && (tmpheader->flag==MM_FLAG_HOLE)) {//&&
//		 (((mm_footer*) ((UINT)tmpheader+tmpheader->size-sizeof(mm_footer)))->magic=MM_MAGIC)) {
		header->size+=tmpheader->size;
		footer=(mm_footer*) ((UINT)tmpheader+tmpheader->size-sizeof(mm_footer));
		heap_del_entry(heap_find_entry(tmpheader,aheap),aheap);
	}
	if ((UINT)footer+sizeof(mm_footer)==aheap->end) {
		oldsize=aheap->end-aheap->start;
		newsize=contract_heap((UINT)header-aheap->start,aheap);
		if (newsize+header->size>oldsize) {
			header->size-=oldsize-newsize;
			footer=(mm_footer*) ((UINT)header+header->size-sizeof(mm_footer));
			footer->magic=MM_MAGIC;
			footer->header=header;
		} else heap_del_entry(heap_find_entry(header,aheap),aheap);
	}
	if (opt) heap_add_entry(header,aheap); 
}

static void *heap_realloc(void *ptr, UINT size, heap *aheap)
{
	mm_header *header, *tmpheader;
	mm_footer *footer, *tmpfooter;
	
	if ((!ptr) || (!aheap)) return 0;
	if (!size) {
		heap_free(ptr,aheap);
		return 0;
	}
	header=(mm_header*) ((UINT)ptr-sizeof(mm_header));
	footer=(mm_footer*) ((UINT)header+header->size-sizeof(mm_footer));
	if ((header->magic!=MM_MAGIC) || (footer->magic!=MM_MAGIC)) return 0;
	if (size==header->size-sizeof(mm_header)-sizeof(mm_footer)) return ptr;
	else if (size<header->size-sizeof(mm_header)-sizeof(mm_footer)) {
		if (header->size-size-2*(sizeof(mm_header)-sizeof(mm_footer))<0) return ptr;
		tmpheader=(mm_header*) (ptr+size+sizeof(mm_footer));
		tmpfooter=(mm_footer*) (ptr+size);
		footer->header=tmpheader;
		tmpfooter->header=header;
		tmpheader->magic=tmpfooter->magic=MM_MAGIC;
		tmpheader->flag=MM_FLAG_BLOCK;
		tmpheader->size=(UINT)footer-(UINT)tmpheader+sizeof(mm_footer);
		header->size=sizeof(mm_header)+size+sizeof(mm_footer);
		heap_free((void *)((UINT)tmpheader+sizeof(mm_header)),aheap);
		return ptr;
	} else {
		void *res = heap_malloc(size,0,aheap);
	
		if (!res) return 0;
		memcpy(res,ptr,size);
		heap_free(ptr,aheap);
		return res;
	}
}


void *malloc(UINT size)
{
	if (!kheap) return (void *)_kmalloc(size);
		else return heap_malloc(size,0,kheap);
}

void *calloc(UINT num, UINT size)
{
	void *res = malloc(num*size);

	if (!res) return 0;
	memset(res,0,num*size);
	return res;
}

void free(void *ptr)
{
	heap_free(ptr,kheap);
}

void *realloc(void *ptr, UINT size)
{
	return heap_realloc(ptr,size,kheap);
}
