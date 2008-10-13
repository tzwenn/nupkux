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
#include <errno.h>

#define NR_SYSCALLS	128

extern void setup_syscalls(void);

extern int sys_putchar(char chr);
extern int sys_exit(int status);
extern pid_t sys_fork(void);
extern int sys_read(int fd, char *buffer, size_t size);
extern int sys_write(int fd, const char *buffer, size_t size);
extern int sys_open(const char *filename,int flag,int mode);
extern int sys_close(int fd);
extern pid_t sys_waitpid(pid_t pid, int *statloc, int options);
extern int sys_execve(const char *file,const char **argv,const char **envp);
extern int sys_chdir(const char *name);
extern int sys_mknod(const char *name, int mode, int addr);
extern pid_t sys_getpid(void);
extern int sys_mount(	const char *source, const char *target,
			const char *filesystemtype, unsigned long mountflags,
			void *data);
extern int sys_pause(void);
extern int sys_kill(pid_t pid,int sign);
extern int sys_dup(int fd);
extern int sys_umount(const char *target);
extern int sys_ioctl(int fd, UINT cmd, ULONG arg);
extern int sys_chroot(const char *name);
extern int sys_dup2(int fd, int fd2);
extern pid_t sys_getppid(void);
extern int sys_reboot(int howto);

#endif
