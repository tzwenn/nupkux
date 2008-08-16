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

#include <kernel.h>
#include <kernel/ktextio.h>

void _init_pc_speaker(void)
{
	outportb(0x61,inportb(0x61) | 0x03);
}

void _stop_pc_speaker(void)
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

