/*
	Squaros intern shell
*/

#ifndef _SISH_H
#define _SISH_H

#include <squaros.h>

#define SISH_EXIT	0xE0
#define SISH_HALT	0xE1
#define SISH_REBOOT	0xE2

extern int sish();
extern void _echo_pc_speaker(UINT freq);
extern void _stop_pc_speaker();

#endif
