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
_syscall3(int,ioctl,int,fd,unsigned int,cmd,unsigned long,arg);
_syscall1(int,chroot,const char *,name);
_syscall0(pid_t,getppid);
_syscall1(int,reboot,int,howto);



