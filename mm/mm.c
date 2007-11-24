#include <mm.h>
#include <kernel/ktextio.h>

UINT kmalloc_pos = WORKING_MEMSTART;
heap *kheap = 0;

extern page_directory *kernel_directory;

UINT _kmalloc(UINT sz)
{
	UINT res=kmalloc_pos;
	
	kmalloc_pos+=sz;
	return res;
}

UINT _kmalloc_a(UINT sz)
{
	UINT res;

	ASSERT_ALIGN(kmalloc_pos);
	res=kmalloc_pos;
	kmalloc_pos+=sz;
	return res;
}

UINT _kmalloc_pa(UINT sz, UINT *phys)
{
	UINT res;

	ASSERT_ALIGN(kmalloc_pos);
	*phys=kmalloc_pos;
	res=kmalloc_pos;
	kmalloc_pos+=sz;
	return res;
}

void expand_heap(UINT new_size, heap *aheap)
{
	UINT i;

	if (new_size<aheap->end-aheap->start) return;
	ASSERT_ALIGN(new_size);
	if (aheap->start+new_size>aheap->memend) return;
	for (i=aheap->end-aheap->start;i<new_size;i+=FRAME_SIZE) 
		make_page(aheap->start+i,aheap->pageflags,kernel_directory,1);
	aheap->end=aheap->start+new_size;
}

UINT contract_heap(UINT new_size, heap *aheap)
{
	UINT i;

	if (new_size>=aheap->end-aheap->start) return aheap->end-aheap->start;
	printf("contract new_size: 0x%X -> ",new_size);
	ASSERT_ALIGN(new_size);
	printf("0x%X\n",new_size);
	if (new_size<MM_KHEAP_MIN) {
		new_size=MM_KHEAP_MIN;
	}
	for (i=aheap->end-aheap->start-FRAME_SIZE;new_size<i;i-=FRAME_SIZE)
		free_page(aheap->start+i,kernel_directory);
	aheap->end=aheap->start+new_size;
	return new_size;
}

UINT find_smallest_hole(UINT size, UCHAR page_align, heap *aheap)  //Binary search?
{
	UINT i,location;
	mm_header *entry;
	int offset = 0;

	for (i=0;i<aheap->entrycount;i++) {
		entry=aheap->entries[i];
		printf("entry->size==0x%X,size==0x%X\n",entry->size,size);
		if (page_align) {
			printf("check align\n");
			location=(UINT)entry;
			if (CHECK_ALIGN(location+sizeof(mm_header)))
				offset=FRAME_SIZE-(location+sizeof(mm_header))%FRAME_SIZE;
			if ((int)entry->size-offset>=(int)size) return i;
		} else if (entry->size>=size) return i;
	}
	return MM_NO_HOLE;
}

void heap_add_entry(mm_header *entry, heap *aheap)
{
	UINT i;
	mm_header *tmp, *tmp2;

	for (i=0;i<aheap->entrycount && aheap->entries[i]->size<entry->size;i++);
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

void heap_del_entry(UINT i, heap *aheap)
{
	for (;i<aheap->entrycount;i++)
		aheap->entries[i]=aheap->entries[i+1];
	aheap->entrycount--;
}

heap *create_heap(UINT start, UINT end, UINT memend, UINT pageflags)
{
	heap *newheap = (heap *)_kmalloc(sizeof(heap));
	mm_header *initholestart;
	mm_footer *initholeend;

	if (CHECK_ALIGN(start) || CHECK_ALIGN(end)) return 0;
	newheap->entries=(mm_header **)start;
	start+=MM_INDEX_COUNT*sizeof(mm_header *);
	ASSERT_ALIGN(start);
	newheap->start=start;
	newheap->end=end;
	newheap->memend=memend;
	newheap->pageflags=pageflags;

	initholestart=(mm_header *) newheap->start;
	newheap->entries[0]=initholestart;
	newheap->entries[0]->magic=MM_MAGIC;
	newheap->entries[0]->size=end-start;
	newheap->entries[0]->flag=MM_FLAG_HOLE;
	
	initholeend=(mm_footer *) (end-sizeof(mm_footer));
	initholeend->magic=MM_MAGIC;
	initholeend->header=newheap->entries[0];
	
	newheap->entrycount=1;
	newheap->maxentries=MM_INDEX_COUNT;
	
	return newheap;
}

void *heap_malloc(UINT size, UCHAR page_align, heap *aheap)
{
	UINT newsize=size+sizeof(mm_header)+sizeof(mm_footer);
	UINT hole_pos=find_smallest_hole(newsize,page_align,aheap),oldsize,oldpos,newpos,oldend;
	mm_header *header, *newheader;
	mm_footer *newfooter;

	if (!aheap) return 0;
	printf("hole_pos==%d\n",hole_pos);
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
		newheader=(mm_header *) (oldpos+size+sizeof(mm_header)+sizeof(mm_footer));  //Das ist auch mal newsize
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

void heap_free(void *ptr, heap *aheap)
{
	mm_header *header, *tmpheader;
	mm_footer *footer, *tmpfooter;
	UINT oldsize,i,newsize;
	UCHAR opt = 1;

	if ((!ptr) || (!aheap)) return;
	header=(mm_header *)((UINT)ptr-sizeof(mm_header));
	oldsize=header->size;
	footer=(mm_footer *)((UINT)header+header->size-sizeof(mm_footer));
	if ((header->magic!=MM_MAGIC) || (footer->magic!=MM_MAGIC)) return;
	header->flag=MM_FLAG_HOLE;
	tmpfooter=(mm_footer *) ((UINT)header-sizeof(mm_footer));
	if ((tmpfooter->magic==MM_MAGIC) && (tmpfooter->header->magic==MM_MAGIC) && (tmpfooter->header->flag==MM_FLAG_HOLE)) { 
		opt=0;
		header=tmpfooter->header;
		footer->header=header;
		header->size+=oldsize;
	}
	tmpheader=(mm_header *) ((UINT)footer+sizeof(mm_footer));
	if ((tmpheader->magic==MM_MAGIC) && (tmpheader->flag==MM_FLAG_HOLE) /*&& (((mm_footer *) ((UINT) tmpheader+tmpheader->size-sizeof(mm_footer)))->magic==MM_MAGIC)*/) {
		header->size+=tmpheader->size; 
		tmpfooter=(mm_footer *) ((UINT)tmpheader+tmpheader->size-sizeof(mm_footer));
		footer=tmpfooter;
		for (i=0;(i<aheap->entrycount) && ((aheap->entries[i])!=(void *)tmpheader);i++);
		if (i<aheap->entrycount)
			heap_del_entry(i,aheap);
	}
	if ((UINT) footer+sizeof(mm_footer)==aheap->end) {
		oldsize=aheap->end-aheap->start;
		newsize=contract_heap((UINT)header-aheap->start,aheap);
		if (newsize+header->size>oldsize) {
			//printf("if\nheader->size=0x%X\n",header->size);
			header->size-=oldsize-newsize;
			//printf("0x%X-0x%X=0x%X\n",oldsize,newsize,oldsize-newsize);
			//printf("header->size=0x%X\n",header->size);
			//printf("header: 0x%X\n",(UINT)header);
			footer=(mm_footer*) ((UINT)header+header->size-sizeof(mm_footer));
			//printf("footer: 0x%X\n",footer);
			//printf("end: 0x%X\n",aheap->end); 
			if ((UINT)footer<aheap->end)  {
				footer->magic=MM_MAGIC;
				footer->header=header;
			}
		} else {
			for (i=0;(i<aheap->entrycount) && ((aheap->entries[i])!=(void *)tmpheader);i++);
			if (i<aheap->entrycount)
				heap_del_entry(i,aheap);
		}
	}
	heap_add_entry(header,aheap);
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
	void *res = calloc(size,1);
	
	if (!res) return 0;
	memcpy(res,ptr,size);
	free(ptr);
	return res;
}
