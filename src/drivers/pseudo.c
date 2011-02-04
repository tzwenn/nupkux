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

#include <drivers/drivers.h>
#include <lib/memory.h>
#include <unistd.h>
#include <task.h>

extern void setup_urandom_file(void);

static int drv_stdin_read(vnode *node, off_t offset, size_t size, char *buffer)
{
	FILE *f=current_task->files[STDIN_FILENO];
	if (!f || !f->node || f->node==node) return 0;
	return read_fs(f->node,offset,size,buffer);
}

static int drv_stdout_write(vnode *node, off_t offset, size_t size, const char *buffer)
{
	FILE *f=current_task->files[STDOUT_FILENO];
	if (!f || !f->node || f->node==node) return 0;
	return write_fs(f->node,offset,size,buffer);
}

static int drv_stderr_write(vnode *node, off_t offset, size_t size, const char *buffer)
{
	FILE *f=current_task->files[STDERR_FILENO];
	if (!f || !f->node || f->node==node) return 0;
	return write_fs(f->node,offset,size,buffer);
}

static int drv_null_read(vnode *node, off_t offset, size_t size, char *buffer)
{
	return 0;
}

int drv_null_write(vnode *node, off_t offset, size_t size, const char *buffer)
{
	return size;
}

static int drv_zero_read(vnode *node, off_t offset, size_t size, char *buffer)
{
	memset(buffer,0,size);
	return size;
}

static file_operations stdin_ops  = {
		read: &drv_stdin_read,
		write: &drv_null_write,};
static file_operations stdout_ops = {
		read: &drv_null_read,
		write: &drv_stdout_write,};
static file_operations stderr_ops = {
		read: &drv_null_read,
		write: &drv_stderr_write,};
static file_operations null_ops = {
		read: &drv_null_read,
		write: &drv_null_write,};
static file_operations zero_ops = {
		read: &drv_zero_read,
		write: &drv_null_write,};

void setup_pseudo_devices(void)
{
	devfs_register_device(NULL,"stdin",0444,FS_UID_ROOT,FS_GID_ROOT,FS_CHARDEVICE,&stdin_ops);
	devfs_register_device(NULL,"stdout",0222,FS_UID_ROOT,FS_GID_ROOT,FS_CHARDEVICE,&stdout_ops);
	devfs_register_device(NULL,"stderr",0222,FS_UID_ROOT,FS_GID_ROOT,FS_CHARDEVICE,&stderr_ops);
	devfs_register_device(NULL,"null",0666,FS_UID_ROOT,FS_GID_ROOT,FS_CHARDEVICE,&null_ops);
	devfs_register_device(NULL,"zero",0666,FS_UID_ROOT,FS_GID_ROOT,FS_CHARDEVICE,&zero_ops);
	setup_urandom_file();
}
