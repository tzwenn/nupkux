#ifndef _SYSCALL_H
#define _SYSCALL_H

#include <squaros.h>

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
