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

#include <drivers/drivers.h>
#include <kernel/ktextio.h>
#include <lib/memory.h>

extern void setup_floppy(fs_node *);
extern void setup_urandom_file(fs_node *);
extern void setup_serial(fs_node *);

inline void outportb(USHORT port, UCHAR value)
{
    asm volatile ("outb %%al,%%dx"::"d" (port), "a" (value));
}

inline UCHAR inportb(USHORT port)
{
 	UCHAR value;

	asm volatile ("inb %%dx,%%al":"=a" (value):"d"(port));
	return value;
}

inline void outportw(USHORT port, USHORT value)
{
    asm volatile ("outw %%ax,%%dx"::"d"(port), "a"(value));
}

inline USHORT inportw(USHORT port)
{
 	USHORT value;

	asm volatile ("inw %%dx,%%ax":"=a"(value):"d"(port));
	return value;
}

static int drv_stdin_read(fs_node *node, off_t offset, size_t size, char *buffer)
{
	return 0;
}

static int drv_stdout_write(fs_node *node, off_t offset, size_t size, const char *buffer)
{
	size_t i=size;

	while (i--)
		_kputc(*(buffer++));

	return size;
}

static int drv_null_read(fs_node *node, off_t offset, size_t size, char *buffer)
{
	return 0;
}

int drv_null_write(fs_node *node, off_t offset, size_t size, const char *buffer)
{
	return size;
}

static int drv_zero_read(fs_node *node, off_t offset, size_t size, char *buffer)
{
	memset(buffer,0,size);
	return size;
}

static node_operations stdin_ops  = {
		read: &drv_stdin_read,
		write: &drv_null_write,};
static node_operations stdout_ops = {
		read: &drv_null_read,
		write: &drv_stdout_write,};
static node_operations null_ops = {
		read: &drv_null_read,
		write: &drv_null_write,};
static node_operations zero_ops = {
		read: &drv_zero_read,
		write: &drv_null_write,};

UINT setup_drivers(fs_node *devfs)
{
	if (!devfs) return 2;

	devfs_register_device(devfs,"stdin",0444,FS_UID_ROOT,FS_GID_ROOT,FS_CHARDEVICE,&stdin_ops);
	devfs_register_device(devfs,"stdout",0222,FS_UID_ROOT,FS_GID_ROOT,FS_CHARDEVICE,&stdout_ops);
	devfs_register_device(devfs,"stderr",0222,FS_UID_ROOT,FS_GID_ROOT,FS_CHARDEVICE,&stdout_ops);
	devfs_register_device(devfs,"null",0666,FS_UID_ROOT,FS_GID_ROOT,FS_CHARDEVICE,&null_ops);
	devfs_register_device(devfs,"zero",0666,FS_UID_ROOT,FS_GID_ROOT,FS_CHARDEVICE,&zero_ops);
	setup_urandom_file(devfs);
	setup_serial(devfs);
	setup_floppy(devfs);
	return 0;
}
