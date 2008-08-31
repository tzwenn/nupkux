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

#include <unistd.h>

int errno = 0;

_syscall1(int,putchar,char,chr);
_syscall1(int,exit,int,status);
_syscall0(pid_t,fork);
_syscall3(int,read,int,fd,char *,buffer,size_t,size);
_syscall3(int,write,int,fd,const char *,buffer,size_t,size);
_syscall3(int,open,const char *,filename,int,flag,int,mode);
_syscall1(int,close,int,fd);
_syscall3(pid_t,waitpid,pid_t,pid,int *,status,int,options);
_syscall3(int,execve,const char *,file,const char **,argv,const char **,envp);
_syscall1(int,chdir,const char *,name);
_syscall3(int,mknod,const char *,name,int,mode,int,addr);
_syscall0(pid_t,getpid);
_syscall0(int,pause);
_syscall2(int,kill,pid_t,pid,int,sign);
_syscall1(int,dup,int,fd);
_syscall3(int,ioctl,int,fd,unsigned int,cmd,unsigned long,arg);
_syscall1(int,chroot,const char *,name);
_syscall2(int,dup2,int,fd,int,fd2);
_syscall0(pid_t,getppid);
_syscall1(int,reboot,int,howto);



