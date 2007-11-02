#include <squaros.h>
#include <paging.h>
#include <memory.h>
#include <kernel/ktextio.h>

page_directory *current_directory, *kernel_directory;
page_table *table;

UINT kmalloc_pos = WORKING_MEMSTART, framecount = 0;
UINT *framemap;

#define set_page_directory(PAGE_DIR)	asm volatile ("cli\n\t"				\
						      "movl %%eax,%%cr3\n\t"		\
						      "movl %%cr0,%%eax\n\t"		\
						      "orl  $0x80000000,%%eax\n\t"	\
						      "movl %%eax,%%cr0\n\t"		\
						      "sti"::"a"(PAGE_DIR->physTabs))
UINT _kmalloc(UINT sz)
{
	UINT res=kmalloc_pos;
	
	kmalloc_pos+=sz;
	return res;
}

UINT _kmalloc_a(UINT sz)
{
	UINT res;

	if (kmalloc_pos & 0xFFFFF000) {
		kmalloc_pos&=0xFFFFF000;
		kmalloc_pos+=0x1000;
	}
	res=kmalloc_pos;
	kmalloc_pos+=sz;
	return res;
}

UINT _kmalloc_pa(UINT sz, UINT *phys)
{
	UINT res;

	if (kmalloc_pos & 0xFFFFF000) {
		kmalloc_pos&=0xFFFFF000;
		kmalloc_pos+=0x1000;
	}
	*phys=kmalloc_pos;
	res=kmalloc_pos;
	kmalloc_pos+=sz;
	return res;
}

page alloc_frame(UINT number, UINT flags)
{
	page res; 

	res.frame=number;
	res.flags=flags;
	framemap[number/32]|=(1<<(number%32));
	return res;
}

void free_frame(page *_page)
{
	UINT number=_page->frame;
	
	_page->frame=0;
	if (number) framemap[number/32]&=~(1<<(number%32));
}

UINT first_frame()
{
	UINT i,j;

	for (i=0;i<framecount/32;i++) 
		if (framemap[i]!=0xFFFFFFFF)
			for (j=0;j<32;j++) 
				if (!(framemap[i]&(1<<j)))
					return i*32+j;
	return 0xFFFFFFFF;
}

page_table *make_table(UINT index, UINT flags, page_directory *directory)
{
	page_table *res=(page_table *)_kmalloc_pa(sizeof(page_table),&(directory->physTabs[index]));

	directory->physTabs[index]|=flags;
	memset(res,0,sizeof(page_table));
	directory->tables[index]=res;
	return res;
}

page *make_page(UINT address, int make, page_directory *directory)
{
	UINT index = address/FRAME_SIZE;
	UINT tab = index/1024;

	if (!directory->physTabs[tab]) make_table(tab,flags,directory);
	directory->tables[tab]->entries[index%1024]=alloc_frame(index,flags);
	return &(directory->tables[tab]->entries[index%1024]);
}

page *get_page(UINT address, int make, page_directory *directory)
{
	UINT index = address/FRAME_SIZE;
	UINT tab = index/1024;
	   
	if (directory->physTabs[tab]) 
		return &(directory->tables[tab]->entries[index%1024]);
	else if (make) 
		return make_page(address,PAGE_FLAG_PRESENT | PAGE_FLAG_WRITE | PAGE_FLAG_USERMODE,directory);
	return 0;
}

void paging_setup()
{
	UINT i;
	
	framecount=WORKING_MEMEND/FRAME_SIZE;
	framemap=(UINT *)_kmalloc(framecount/32);
	memset(framemap,0,framecount/32);
	kernel_directory=(page_directory *)_kmalloc_pa(sizeof(page_directory),&i);
	kernel_directory->physPos=i;
	memset(kernel_directory->physTabs,0,0x1000);
	i=0;
	while (i<kmalloc_pos) {
		make_page(i,PAGE_FLAG_WRITE | PAGE_FLAG_PRESENT,kernel_directory);
		i+=0x1000;
	}
	current_directory=kernel_directory;
	set_page_directory(kernel_directory);
}

int page_fault_handler(struct regs *r)
{
	UINT faultaddr;

 	asm volatile ("mov %%cr2,%%eax":"=a"(faultaddr));

	printf("\nPagefault at 0x%X: %s%s%s%s\n",faultaddr,(!(r->err_code&1))?"present ":"",(r->err_code&2)?"read-only ":"",(r->err_code&4)?"user-mode ":"",(r->err_code&8)?"reserved ":"");
	printf("System halted.\n");
	cli();
	hlt();
	return 0;
}


