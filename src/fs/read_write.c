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

int sys_ioctl(int fd, UINT cmd, ULONG arg)
{
	if (fd < 0 || fd >= NR_OPEN) return -EBADF;
	FILE *f = current_task->files[fd];
	if (!f) return -EBADF;
	return ioctl_fs(f->node, cmd, arg);
}

int sys_read(int fd, char *buffer, size_t size)
{
	if (fd < 0 || fd >= NR_OPEN) return -EBADF;
	FILE *f = current_task->files[fd];
	if (!f) return -EBADF;
	if (!f->flags & FMODE_READ) return -EBADF;
	if (!access_ok(VERIFY_WRITE, buffer, size)) return -EFAULT;
	size = read_fs(f->node, f->offset, size, buffer);
	if (!IS_CHR(f->node)) f->offset += size;
	return size;
}

int sys_write(int fd, const char *buffer, size_t size)
{
	if (fd < 0 || fd >= NR_OPEN) return -EBADF;
	FILE *f = current_task->files[fd];
	if (!f) return -EBADF;
	if (!(f->flags & FMODE_WRITE)) return -EBADF;
	if (!access_ok(VERIFY_READ, buffer, size)) return -EFAULT;
	size = write_fs(f->node, f->offset, size, buffer);
	if (!IS_CHR(f->node)) f->offset += size;
	return size;
}
