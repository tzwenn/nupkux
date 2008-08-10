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
#include <kernel/dts.h>

//I've taken a look at a list for the Linux kernel, so I hope there is
//any resemblence to POSIX here

//and some newlib requirements
//Das ist deutsch, da es für mich ist: Mach ein "environ,isatty"
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
#define SYS_CHDIR	12
#define SYS_TIME	13
#define SYS_MKNOD	14
#define SYS_CHMOD	15
#define SYS_CHOWN	16
#define SYS_BREAK	17
#define SYS_STAT	18
#define SYS_LSEEK	19
#define SYS_GETPID	20
#define SYS_MOUNT	21
#define SYS_UMOUNT	22
#define SYS_FSTAT	28
#define SYS_KILL	37
#define SYS_TIMES	43
#define SYS_BRK		45

#define NR_SYSCALLS	64

extern void setup_syscalls();

extern int sys_putchar(char chr);
extern int sys_exit(int status);
extern int sys_fork();
extern int sys_close(int fd);
extern int sys_execve(char *file,char **argv,char **envp);
extern int sys_chdir(char *name);
extern int sys_mknod(char *name, int mode, int addr);
extern int sys_getpid();

#endif
