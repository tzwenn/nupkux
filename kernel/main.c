#include <squaros.h>
#include <kernel/ktextio.h>
#include <kernel/sish.h>

void reboot()
{
	volatile UCHAR in = 0x02;

	_kout("Will now reboot");
	while (in & 0x02)
		in=inportb(0x64);
	outportb(0x64,0xFE);
}

void halt()
{
	_kout("Will now halt");
}

int _int0x21()
{
	_kout("Interupt\n");
	return 0;
}

int _kmain()
{
	int ret;
	
	_kclear();
	_kout("Squaros booted ...\n");
	ret=sish();
	switch (ret) {
	  case SISH_REBOOT: reboot();
			    return 0;	
			    break;
	  case SISH_HALT:   halt();
			    return 0;
			    break;
	  default: 	    _kout("sish returned unknown value.\nStop system");
			    return 0;
			    break;
	}
	//while (1);
	return 0;
}

