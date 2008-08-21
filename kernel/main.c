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
#include <kernel.h>
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

void reboot(void)
{
	volatile UCHAR in = 0x02;

	printf("Will now reboot");
	while (in & 0x02)
		in=inportb(0x64);
	outportb(0x64,0xFE);
}

extern int setup_ACPI(void);
extern void acpiPowerOff(void);

static void halt(void)
{
	printf("Will now halt");
	acpiPowerOff();
	printf("\nACPI Power off failed!\nTurn off the computer manually!");  //Just in case
	cli();
	hlt();
}

static void read_multiboot_info(multiboot_info_t* mbd)
{
	int i;
	multiboot_module_info_t *mod_infos;

	/* According to http://www.gnu.org/software/grub/manual/multiboot/multiboot.html
	   GRUB can store it values anywhere
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

int _kmain(multiboot_info_t* mbd, UINT initial_stack, UINT magic)
{
	int ret;
	fs_node *root, *devfs;

	read_multiboot_info(mbd);
	initial_esp=initial_stack;
	_kclear();
	printf("Nupkux loaded ... Stack at 0x%X\nAmount of RAM: %d Bytes.\nSet up Descriptors ... ",initial_esp,memory_end);
	setup_dts();
	setup_input();
	printf("Finished.\nEnable Interrupts and PIC ... ");
	sti();
	setup_timer();
	setup_ACPI();
	printf("Finished.\nEnable Paging and Memory Manager ... ");
	setup_paging();
	setup_ktexto();
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
	printf("Booted up!\n");

	printf("nish returned with 0x%X.\n\n",ret=nish());

	printf("Unmount devfs (/dev) ... \n");
	remove_devfs(devfs);
	printf("Unmount initrd (/) ... \n");
	remove_initrd(root);
	printf("Close VFS ... \n");
	close_vfs();
	printf("OK\n");
	switch (ret) {
	  case NISH_REBOOT: reboot();
			    return 0;
			    break;
	  case NISH_HALT:   halt();
			    return 0;
			    break;
	  default: 	    printf("Stop system");
			    return 0;
			    break;
	}
	while (1);
	return 0;
}

