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

#include <fcntl.h>
#include <fs/fs.h>
#include <task.h>
#include <kernel/syscall.h>

int sys_dup2(int fd, int fd2)
{
	if (fd<0 || fd>=NR_OPEN || fd2<0 || fd2>=NR_OPEN) return -EBADF;
	if (!current_task->files[fd]) return -EBADF;
	sys_close(fd2);
	current_task->close_on_exec&=~(1<<fd2);
	current_task->files[fd2]=current_task->files[fd];
	current_task->files[fd2]->count++;
	return fd2; //man 2 dup on BSD told me to return 0 here, but Linux returns fd
}

int sys_dup(int fd)
{
	int fd2;
	for (fd2=0;fd2<NR_OPEN;fd2++)
		if (!current_task->files[fd2]) break;
	if (fd2>=NR_OPEN) return -EMFILE;
	return sys_dup2(fd,fd2);
}
