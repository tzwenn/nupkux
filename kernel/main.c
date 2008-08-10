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
#include <time.h>
#include <task.h>
#include <mm.h>
#include <fs/initrd.h>
#include <drivers/drivers.h>

char _kabort_func = 0;
int errno;
UINT initrd_location = 0;
UINT initial_esp;
ULONG memory_end = 0;
UINT __working_memstart = 0;
extern UINT kmalloc_pos;
extern UINT initrd_location;

void reboot()
{
	volatile UCHAR in = 0x02;

	printf("Will now reboot");
	while (in & 0x02)
		in=inportb(0x64);
	outportb(0x64,0xFE);
}

extern int setup_ACPI();
extern void acpiPowerOff();

static void halt()
{	
	printf("Will now halt");
	acpiPowerOff();
	printf("\nACPI Power off failed!\nTurn off the computer manually!");  //Just in case
	cli();
	hlt();
}

int _kmain(multiboot_info_t* mbd, UINT initial_stack, UINT magic)
{
	int ret;
	fs_node *root, *devfs;
	
	_kclear();
	if (mbd->flags&0x01) memory_end=mbd->mem_upper*1024;
		else memory_end=0x400000;
	initial_esp=initial_stack;
	printf("Nupkux loaded ... Stack at 0x%X\nAmount of RAM: %d Bytes.\nSet up Descriptors ... ",initial_esp,memory_end);
	if (mbd->mods_count>0) {
		initrd_location = *((UINT*)mbd->mods_addr);
		__working_memstart=*(UINT*)(mbd->mods_addr+4);
	} else __working_memstart=(UINT) &kernel_end;
	kmalloc_pos=__working_memstart;
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

