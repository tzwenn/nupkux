#include <multiboot.h>
#include <squaros.h>
#include <kernel/ktextio.h>
#include <kernel/sish.h>
#include <time.h>
#include <kernel/devices/fdc.h>
#include <mm.h>

char _kabort_func = 0;
int errno;
ULONG memory_end = 0;

void reboot()
{
	volatile UCHAR in = 0x02;

	printf("Will now reboot");
	while (in & 0x02)
		in=inportb(0x64);
	outportb(0x64,0xFE);
}

void halt()
{	
	printf("Will now halt");
	printf("\n\nYou can turn off the computer.");
	cli();
	hlt();
}

int _kmain(multiboot_info_t* mbd, unsigned int magic) 
{
	int ret;
	
	_kclear();
	if (mbd->flags&0x01) memory_end=mbd->mem_upper*1024;
		else memory_end=0x400000;
	printf("Squaros booted ...\nAmount of RAM: %d Bytes.\nSet up Descriptors ... ",memory_end);
	gdt_install();
	idt_install();
	printf("Finished.\nInstall IRQ & ISRS ... ");
	isrs_install();
	irq_install();
	printf("Finished.\nStart Keyboard Controller ... ");
	input_setup();
	printf("Finished.\nEnable Interrupts ... ");
	sti();
	printf("Finished.\nEnable Paging and Memory Manager... ");
	paging_setup();
	init_ktexto();
	printf("Finished.\nSet up timer ... ");
	timer_install();
	printf("Finished.\nFloppydrive support  ... ");
	init_floppy();
	if (!floppy_drives) printf("No drives found.\n");
		else printf("%s found.\n",(floppy_drives & 0x0F)?"2 drives":"1 drive");
	printf("Booted up!\n");
	ret=sish();
	switch (ret) {
	  case SISH_REBOOT: reboot();
			    return 0;	
			    break;
	  case SISH_HALT:   halt();
			    return 0;
			    break;
	  default: 	    printf("sish returned with 0x%X.\nStop system",ret);
			    return 0;
			    break;
	}
	while (1);
	return 0;
}

