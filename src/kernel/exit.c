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
#include <kernel/syscall.h>
#include <errno.h>
#include <kernel/ktextio.h>

extern volatile task tasks[NR_TASKS];

int sys_exit(int status)
{
	UINT i;
	cli(); //switch_task does sti()
	for (i=NR_OPEN;i--;)
		if (current_task->files[i].fd!=NO_FILE)
			sys_close(i);
	if (!current_task->pid) {
		printf("Kernel Aborted. Halt System!\n");
		hlt();
	}
	current_task->state=TASK_ZOMBIE;
	current_task->exit_code=status;
	free_directory(current_task->directory);
	free((void *)current_task->kernel_stack);
	switch_task();
	return status;
}

pid_t sys_waitpid(pid_t pid, int *statloc, int options)
{
	if (pid<=0 || pid>=NR_TASKS) return -ECHILD;
	while (tasks[pid].state!=TASK_ZOMBIE);
	if (statloc) {
		if (!access_ok(VERIFY_WRITE,statloc,sizeof(int))) {
			tasks[pid].pid=NO_TASK;
			return -EFAULT;
		}
		*statloc=tasks[pid].exit_code;
	}
	tasks[pid].pid=NO_TASK;
	return pid;
}
