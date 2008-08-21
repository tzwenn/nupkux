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
#include <task.h>
#include <unistd.h>

//I've taken a look at a list for the Linux kernel, so I hope there is
//any resemblence to POSIX here

//and some newlib requirements
//Das ist deutsch, da es für mich ist: Mach ein "environ,isatty"
#define SYS_PUTCHAR	__NR_putchar
#define SYS_EXIT	__NR_exit
#define SYS_FORK	__NR_fork
#define SYS_READ	__NR_read
#define SYS_WRITE	__NR_write
#define SYS_OPEN	__NR_open
#define SYS_CLOSE	__NR_close
#define SYS_WAITPID	__NR_waitpid
#define SYS_CREAT	__NR_creat
#define SYS_LINK	__NR_link
#define SYS_UNLINK	__NR_unlink
#define	SYS_EXECVE	__NR_execve
#define SYS_CHDIR	__NR_chdir
#define SYS_TIME	__NR_time
#define SYS_MKNOD	__NR_mknod
#define SYS_CHMOD	__NR_chmod
#define SYS_CHOWN	__NR_chown
#define SYS_BREAK	__NR_break
#define SYS_STAT	__NR_stat
#define SYS_LSEEK	__NR_lseek
#define SYS_GETPID	__NR_getpid
#define SYS_MOUNT	__NR_mount
#define SYS_UMOUNT	__NR_umount
#define SYS_FSTAT	__NR_fstat
#define SYS_KILL	__NR_kill
#define SYS_TIMES	__NR_times
#define SYS_BRK		__NR_brk
#define SYS_IOCTL	__NR_ioctl
#define SYS_FCNTL	__NR_fcntl
#define SYS_CHROOT	__NR_chroot

#define NR_SYSCALLS	64

extern void setup_syscalls(void);

extern int sys_putchar(char chr);
extern int sys_exit(int status);
extern pid_t sys_fork(void);
extern int sys_read(int fd, char *buffer, size_t size);
extern int sys_write(int fd, const char *buffer, size_t size);
extern int sys_open(const char *filename,int flag,int mode);
extern int sys_close(int fd);
extern pid_t sys_waitpid(pid_t pid, int *statloc, int options);
extern int sys_execve(const char *file,char **argv,char **envp);
extern int sys_chdir(const char *name);
extern int sys_mknod(const char *name, int mode, int addr);
extern pid_t sys_getpid(void);
extern int sys_ioctl(int fd, UINT cmd, ULONG arg);
extern int sys_chroot(const char *name);

#endif
