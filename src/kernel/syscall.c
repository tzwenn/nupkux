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
#include <lib/memory.h>
#include <errno.h>

static void *sys_call_table[NR_SYSCALLS];

int sys_putchar(char chr)
{
	_kputc(chr);
	return chr;
}

int sys_mknod(const char *name, int mode, int addr)
{
	return -ENOSYS;
}

void SysCallHandler(registers *regs)
{
	if (regs->eax>=NR_SYSCALLS) {
		regs->eax=-1;
		return;
	}
	void *sys_call=sys_call_table[regs->eax];

	if (!sys_call) {
		regs->eax=-ENOSYS;
		return;
	}
	int ret;
	asm volatile (	"push %1\n\t"
			"push %2\n\t"
			"push %3\n\t"
			"push %4\n\t"
			"push %5\n\t"
			"call *%6\n\t"
			"pop %%ebx\n\t"
			"pop %%ebx\n\t"
			"pop %%ebx\n\t"
			"pop %%ebx\n\t"
			"pop %%ebx\n\t"
			:"=a"(ret)
			:"r"(regs->edi),"r"(regs->esi),"r"(regs->edx),"r"(regs->ecx),"r"(regs->ebx),"r"(sys_call));
	regs->eax=ret;
}

void setup_syscalls()
{
	memset(sys_call_table,0,NR_SYSCALLS*sizeof(void *));

	sys_call_table[SYS_PUTCHAR]=&sys_putchar;
	sys_call_table[SYS_EXIT]=&sys_exit;
	sys_call_table[SYS_FORK]=&sys_fork;
	sys_call_table[SYS_READ]=&sys_read;
	sys_call_table[SYS_WRITE]=&sys_write;
	sys_call_table[SYS_OPEN]=&sys_open;
	sys_call_table[SYS_CLOSE]=&sys_close;
	sys_call_table[SYS_WAITPID]=&sys_waitpid;
	sys_call_table[SYS_EXECVE]=&sys_execve;
	sys_call_table[SYS_CHDIR]=&sys_chdir;
	sys_call_table[SYS_MKNOD]=&sys_mknod;
	sys_call_table[SYS_GETPID]=&sys_getpid;
	sys_call_table[SYS_PAUSE]=&sys_pause;
	sys_call_table[SYS_IOCTL]=&sys_ioctl;
	sys_call_table[SYS_CHROOT]=&sys_chroot;
	sys_call_table[SYS_GETPPID]=&sys_getppid;
	sys_call_table[SYS_REBOOT]=&sys_reboot;

	register_interrupt_handler(0x80,&SysCallHandler);
}
