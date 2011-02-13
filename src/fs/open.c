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
#include <fs/vfs.h>
#include <errno.h>
#include <fcntl.h>

int sys_chdir(const char *name)
{
	if (!current_task) return -EGENERIC;
	if (!access_ok(VERIFY_READ, name, VERIFY_STRLEN)) return -EFAULT;
	int status;
	vnode *node = namei(name, &status);

	if (node) {
		if (IS_DIR(node))
			current_task->pwd = node;
		else return -ENOTDIR;
	} else return status;
	return 0;
}

int sys_chroot(const char *name)
{
	if (!current_task) return -EGENERIC;
	if (!I_AM_ROOT()) return -EPERM;
	if (!access_ok(VERIFY_READ, name, VERIFY_STRLEN)) return -EFAULT;
	int status;
	vnode *node = namei(name, &status);

	if (node) {
		if (IS_DIR(node))
			current_task->root = node;
		else return -ENOTDIR;
	} else return status;
	return 0;
}

int sys_open(const char *filename, int flag, int mode)
{
	//TODO Check access
	int fd, status;
	FILE *f;

	if (!access_ok(VERIFY_READ, filename, VERIFY_STRLEN)) return -EFAULT;
	for (fd = 0; fd < NR_OPEN; fd++)
		if (!current_task->files[fd]) break;
	if (fd >= NR_OPEN) return -EMFILE;
	f = malloc(sizeof(FILE));
	f->flags = 0;
	f->node = namei(filename, &status);
	if (!f->node) {
		free(f);
		return status;
	}
	if (flag & O_APPEND) f->offset = f->node->size;
	else f->offset = 0;
	/////////////////////////////////////
	// A lot more goes here and there an everywhere
	/////////////////////////////////////
	if ((!(flag & 3)) || (flag & O_RDWR)) f->flags |= FMODE_READ;
	if ((flag & O_WRONLY) || (flag & O_RDWR)) f->flags |= FMODE_WRITE;
	open_fs(f->node, f);
	f->count = 1;
	current_task->close_on_exec &= ~(1 << fd);
	f->fd = fd;
	current_task->files[fd] = f;
	return fd;
}

int sys_close(int fd)
{
	if (fd < 0 || fd >= NR_OPEN) return -EBADF;
	if (!current_task->files[fd]) return -EBADF;
	FILE *f = current_task->files[fd];
	current_task->close_on_exec &= ~(1 << fd);
	current_task->files[fd] = 0;
	if (f->count && !(--f->count)) {
		close_fs(f->node);
		iput(f->node);
		free(f);
	}
	return 0;
}
