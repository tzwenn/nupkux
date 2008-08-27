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

#include <multiboot.h>
#include <kernel/ktextio.h>
#include <kernel/nish.h>
#include <kernel/syscall.h>
#include <lib/string.h>
#include <time.h>
#include <task.h>
#include <mm.h>
#include <fs/initrd.h>
#include <drivers/drivers.h>

char _kabort_func = 0;
int errno = 0;
UINT initial_esp = 0;
char kernel_cmdline[256] = {0,};
boot_module boot_modules[NR_BOOT_MODULES];
UINT		boot_modules_count = 0;
ULONG memory_end = 0;
UINT __working_memstart = 0;
UINT initrd_location = 0;
extern UINT kmalloc_pos;
fs_node *root = 0, *devfs = 0;

extern int setup_ACPI(void);

static void read_multiboot_info(multiboot_info_t* mbd)
{
	int i;
	multiboot_module_info_t *mod_infos;

	/* According to http://www.gnu.org/software/grub/manual/multiboot/multiboot.html
	   GRUB can store its values anywhere
	   I've discovered there are all in the first 640K, but I don't want to risk anything */
	if (mbd->flags&0x01) memory_end=mbd->mem_upper*1024; //TODO: A map would be nicer
			else memory_end=ASSUMED_WORKING_MEMEND;
	if (mbd->flags&0x04) {
		strncpy(kernel_cmdline,(char *)mbd->cmdline,256);
		kernel_cmdline[255]=0;
	}
	if (mbd->flags&0x08) {
		memset(boot_modules,0,NR_BOOT_MODULES*sizeof(boot_module));
		boot_modules_count=mbd->mods_count;
		if (boot_modules_count>NR_BOOT_MODULES)
			boot_modules_count=NR_BOOT_MODULES;
		mod_infos=(multiboot_module_info_t *)mbd->mods_addr;
		for (i=0;i<boot_modules_count;i++) {
			boot_modules[i].addr=mod_infos[i].mod_start;
			boot_modules[i].size=mod_infos[i].mod_end-mod_infos[i].mod_start;
			memset(boot_modules[i].string,0,BOOT_MODULE_STRLEN);
			strncpy(boot_modules[i].string,(char*)mod_infos[i].string,BOOT_MODULE_STRLEN);
			boot_modules[i].string[BOOT_MODULE_STRLEN-1]=0;
			if (*((UINT*)boot_modules[i].addr)==INITRD_MAGIC)
				initrd_location=boot_modules[i].addr;
		}
	}
	if (boot_modules_count)
		__working_memstart=boot_modules[boot_modules_count-1].addr+boot_modules[boot_modules_count-1].size;
		else __working_memstart=(UINT) &kernel_end;
	ASSERT_ALIGN(__working_memstart);
	kmalloc_pos=WORKING_MEMSTART+IPC_MEMSIZE;
}

int init(void)
{
	int ret;
	pid_t pid;
	if (!(pid=sys_fork())) {
		set_kernel_stack(current_task->kernel_stack+KERNEL_STACK_SIZE);
		asm volatile ("int $0x80":"=a"(ret):"a"(SYS_EXECVE),"b"("/bin/init"),"c"(0),"d"(0));
		asm volatile ("int $0x80"::"a"(SYS_EXIT),"b"(ret));
		for (;;);
	} else sys_waitpid(pid,&ret,0); //TODO: Exception handling
	return ret;
}

int _kmain(multiboot_info_t* mbd, UINT magic, UINT initial_stack)
{
	read_multiboot_info(mbd);
	initial_esp=initial_stack;
	_kclear();
	printf("Nupkux loaded ... Stack at 0x%X\nAmount of RAM: %d Bytes.\nSet up Descriptors ... ",initial_esp,memory_end);
	setup_dts();
	printf("Finished.\nEnable Interrupts and PIC ... ");
	sti();
	setup_timer();
	setup_ACPI();
	printf("Finished.\nEnable Paging and Memory Manager ... ");
	setup_paging();
	printf("Finished.\nSetup Tasking ... ");
	setup_tasking();
	printf("Finished.\nSetup VFS ... ");
	setup_vfs();
	printf("Finished.\nMount initrd read-only on root ... ");
	root=setup_initrd(initrd_location,get_root_fs_node());
	current_task->pwd=root;
	if (root) printf("Finished.\n");
		else printf("FAILED.\n");
	if ((devfs=namei("/dev"))) {
		printf("Populating Devfs ... ");
		devfs=setup_devfs(devfs);
		setup_drivers(devfs);
		if (devfs) printf("Finished.\n");
			else printf("FAILED.\n");
	}
	setup_syscalls();
	nish();
	init();
	for (;;);
	return 0;
}

