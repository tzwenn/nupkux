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

#ifndef _SYSCALL_H
#define _SYSCALL_H

#include <kernel.h>

//I've taken a look at a list for the Linux kernel (2.2 ;-P )
//and newlib requirements


//Das ist deutsch, da es für mich ist: Mach ein "environ,sbrk,isatty,wait"
#define SYS_PUTCHAR	0
#define SYS_EXIT	1
#define SYS_FORK	2
#define SYS_READ	3
#define SYS_WRITE	4
#define SYS_OPEN	5
#define SYS_CLOSE	6
#define SYS_WAITPID	7
#define SYS_CREAT	8
#define SYS_LINK	9
#define SYS_UNLINK	10
#define	SYS_EXECVE	11
#define SYS_STAT	18
#define SYS_LSEEK	19
#define SYS_GETPID	20
#define SYS_FSTAT	28
#define SYS_KILL	37
#define SYS_TIMES	43


extern int SysCallHandler(struct regs *r);

#endif
