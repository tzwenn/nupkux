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

#include <task.h>
#include <time.h>
#include <kernel/ktextio.h>
#include <drivers/acpi.h>
#include <errno.h>

#define RB_HALT_SYSTEM	0x01
#define RB_AUTOBOOT	0x02
#define RB_POWEROFF	0x04

static void do_halt(void)
{
	printf("System halted.");
	cli();
	hlt();
}

static void do_poweroff(void)
{
	printf("Power down.");
	acpiPowerOff();
	sleep(1000);
	printf("\nACPI Power off failed!\nTurn off the computer manually!");  //Just in case
	cli();
	hlt();
}

static void do_reboot(void)
{
	printf("Restarting system.");
	while (inportb(0x64) & 0x02);
	outportb(0x64, 0xFE);
}

#include <fs/mount.h>

int sys_reboot(int howto)
{
	if (!I_AM_ROOT()) return -EPERM;
	//TODO: Send SIGTERM and SIGKILL
	//FIXME: I unmount devfs and so delete the tty I'm printing on
	printf("Unmount devfs (/dev) ... \n");
	sys_umount("/dev");
	printf("Unmount initrd (/) ... \n");
	sys_umount("/");
	printf("Close VFS ... \n");
	close_vfs();
	switch (howto) {
	case RB_HALT_SYSTEM:
		do_halt();
		break;
	case RB_POWEROFF:
		do_poweroff();
		break;
	case RB_AUTOBOOT:
		do_reboot();
		break;
	}
	return 0;
}
