#include <squaros.h>
#include <kernel/ktextio.h>
#include <kernel/sish.h>
#include <time.h>
#include <kernel/devices/fdc.h>

char _kabort_func = 0;
int errno;

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
}

int _kmain()
{
	int ret;
	
	_kclear();
	printf("Squaros booted ...\nSet up Descriptors ... ");
	gdt_install();
	idt_install();
	printf("Finished.\nInstall IRQ & ISRS ... ");
	isrs_install();
	irq_install();
	printf("Finished.\nEnable Interrupts ... ");
	asm volatile ("sti\n\t");
	printf("Finished.\nStart Keyboard Controller ... ");
	input_setup();
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

