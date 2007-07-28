#include <squaros.h>
#include <kernel/ktextio.h>
#include <kernel/sish.h>
#include <time.h>

int _kernel_start_time;

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
	
	_kernel_start_time=time(0);
	_kclear();
	printf("Squaros booted ...\n");
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
	//while (1);
	return 0;
}

