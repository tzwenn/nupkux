#include <kernel/ktextio.h>
#include <squaros.h>

void _init_pc_speaker()
{
	outportb(0x61,inportb(0x61) | 0x03);
}

void _stop_pc_speaker()
{
	outportb(0x61,inportb(0x61) & 0xFC);
}

void _echo_pc_speaker(UINT freq)
{
	UINT caller;
	caller=0x1234DD/freq;
	outportb(0x43,0xB6);
	outportb(0x42,caller&0xFF);
	outportb(0x42,caller>>8);
	_init_pc_speaker();
}

