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

#include <kernel/syscall.h>
#include <kernel/ktextio.h>

int sys_putchar(struct regs *r)
{
	printf("%c",r->ebx);
	return 0;
}

int SysCallHandler(struct regs *r)
{
	switch (r->eax) {
		case SYS_PUTCHAR: return sys_putchar(r);
				  break;
		default: printf("SysCall 0x%X (%d)\n",r->eax,r->eax);
	}
	return 0;
}
