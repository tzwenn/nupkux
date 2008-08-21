/*
 *  Copyright (C) 2008 Sven Köhler
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

#ifndef _UINSTD_H
#define _UINSTD_H

//Will there be ever something "nearly POSIX" by me?
#define _POSIX_VERSION 198808L

#ifndef _SIZE_T
#define _SIZE_T
typedef unsigned int size_t;
#endif

#ifndef _PID_T
#define _PID_T
typedef int pid_t;
#endif

#ifndef _SSIZE_T
#define _SSIZE_T
typedef int ssize_t;
#endif

#define STDIN_FILENO	0
#define STDOUT_FILENO	1
#define STDERR_FILENO	2

#ifndef NULL
#define NULL    ((void *)0)
#endif

//file access
#define F_OK	0
#define X_OK	1
#define W_OK	2
#define R_OK	4

//lseek starts
#define SEEK_SET	0
#define SEEK_CUR	1
#define SEEK_END	2

#define __NR_putchar	0
#define __NR_exit	1
#define __NR_fork	2
#define __NR_read	3
#define __NR_write	4
#define __NR_open	5
#define __NR_close	6
#define __NR_waitpid	7
#define __NR_creat	8
#define __NR_link	9
#define __NR_unlink	10
#define __NR_execve	11
#define __NR_chdir	12
#define __NR_time	13
#define __NR_mknod	14
#define __NR_chmod	15
#define __NR_chown	16
#define __NR_break	17
#define __NR_stat	18
#define __NR_lseek	19
#define __NR_getpid	20
#define __NR_mount	21
#define __NR_umount	22
#define __NR_fstat	28
#define __NR_kill	37
#define __NR_times	43
#define __NR_brk	45
#define __NR_ioctl	54
#define __NR_fcntl	55
#define __NR_chroot	61

#define _syscall0(type,name) \
type name(void) \
{ \
type __res; \
__asm__ volatile ("int $0x80" \
	: "=a" (__res) \
	: "0" (__NR_##name)); \
if (__res >= 0) \
	return __res; \
errno = -__res; \
return -1; \
}

#define _syscall1(type,name,atype,a) \
type name(atype a) \
{ \
type __res; \
__asm__ volatile ("int $0x80" \
	: "=a" (__res) \
	: "0" (__NR_##name),"b" (a)); \
if (__res >= 0) \
	return __res; \
errno = -__res; \
return -1; \
}

#define _syscall2(type,name,atype,a,btype,b) \
type name(atype a,btype b) \
{ \
type __res; \
__asm__ volatile ("int $0x80" \
	: "=a" (__res) \
	: "0" (__NR_##name),"b" (a),"c" (b)); \
if (__res >= 0) \
	return __res; \
errno = -__res; \
return -1; \
}

#define _syscall3(type,name,atype,a,btype,b,ctype,c) \
type name(atype a,btype b,ctype c) \
{ \
type __res; \
__asm__ volatile ("int $0x80" \
	: "=a" (__res) \
	: "0" (__NR_##name),"b" (a),"c" (b),"d" (c)); \
if (__res<0) \
	errno=-__res , __res = -1; \
return __res;\
}

#define _decl_syscall0(type,name) \
type name(void)

#define _decl_syscall1(type,name,atype,a) \
type name(atype a)

#define _decl_syscall2(type,name,atype,a,btype,b) \
type name(atype a,btype b)

#define _decl_syscall3(type,name,atype,a,btype,b,ctype,c) \
type name(atype a,btype b,ctype c)

#ifndef _ERRNO
#define _ERRNO
extern int errno;
#endif

_decl_syscall1(int,putchar,char,chr);
_decl_syscall1(int,exit,int,status);
_decl_syscall0(pid_t,fork);
_decl_syscall3(int,read,int,fd,char,*buffer,size_t,size);
_decl_syscall3(int,write,int,fd,const char,*buffer,size_t,size);
_decl_syscall3(int,open,const char,*filename,int,flag,int,mode);
_decl_syscall1(int,close,int,fd);
_decl_syscall3(pid_t,waitpid,pid_t,pid,int,*status,int,options);
_decl_syscall3(int,execve,const char,*file,char,**argv,char,**envp);
_decl_syscall1(int,chdir,const char,*name);
_decl_syscall3(int,mknod,const char,*name,int,mode,int,addr);
_decl_syscall0(pid_t,getpid);
_decl_syscall1(int,chroot,const char,*name);

#endif
