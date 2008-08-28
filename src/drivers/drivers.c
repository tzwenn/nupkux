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

#include <drivers/drivers.h>
#include <drivers/fdc.h>
#include <drivers/tty.h>

extern void setup_pseudo_devices(fs_node *devfs); //pseudo.c
extern void setup_serial(fs_node *); //serial.c

inline void outportb(USHORT port, UCHAR value)
{
    asm volatile ("outb %%al,%%dx"::"d" (port), "a" (value));
}

inline UCHAR inportb(USHORT port)
{
 	UCHAR value;

	asm volatile ("inb %%dx,%%al":"=a" (value):"d"(port));
	return value;
}

inline void outportw(USHORT port, USHORT value)
{
    asm volatile ("outw %%ax,%%dx"::"d"(port), "a"(value));
}

inline USHORT inportw(USHORT port)
{
 	USHORT value;

	asm volatile ("inw %%dx,%%ax":"=a"(value):"d"(port));
	return value;
}

UINT setup_drivers(fs_node *devfs)
{
	if (!devfs) return 2;

	setup_pseudo_devices(devfs);
	setup_tty(devfs);
	setup_serial(devfs);
	setup_floppy(devfs);
	return 0;
}
