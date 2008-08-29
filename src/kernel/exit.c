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
#include <signal.h>

extern volatile task tasks[NR_TASKS];

int send_signal(volatile task *atask, int sign) //Just inside the kernel
{
	if (I_AM_ROOT() || atask->uid==current_task->uid || atask->uid==current_task->euid
			|| atask->euid==current_task->uid || atask->euid==current_task->euid || sign==SIGCONT) {
		atask->signals|=(1<<(sign-1));
		return 0;
	}
	return -EPERM;
}

int sys_kill(pid_t pid, int sign)
{
	if (sign<1 || sign>32)
		return -EINVAL;
	pid_t i=0;
	int ret=-1,perm=0,found=0;
	if (!pid) for (;i<NR_TASKS;i++) {
			if (tasks[i].pid!=NO_TASK && tasks[i].pgrp==current_task->pgrp) {
				ret=send_signal(&(tasks[i]),sign);
				found=1;
				if (ret!=-EPERM) perm=1;
			}
	} else if (pid==-1) for (;i<NR_TASKS;i++) {
			if (tasks[i].pid!=NO_TASK) {
				ret=send_signal(&(tasks[i]),sign);
				found=1;
				if (ret!=-EPERM) perm=1;
			}
	} else if (pid<-1) for (;i<NR_TASKS;i++) {
		if (tasks[i].pid!=NO_TASK && tasks[i].pgrp==-pid) {
			ret=send_signal(&(tasks[i]),sign);
			found=1;
			if (ret!=-EPERM) perm=1;
		}
	} else if (pid>NR_TASKS || tasks[pid].pid==NO_TASK) {
		return -ESRCH;
	} else return send_signal(&(tasks[pid]),sign);
	if (!found) return -ESRCH;
	if (!perm) return -EPERM;
	return 0;
}

int sys_exit(int status)
{
	UINT i;
	cli(); //switch_task does sti()
	for (i=0;i<NR_TASKS;i++)
		if (tasks[i].pid!=NO_TASK && tasks[i].parent==current_task->pid)
			tasks[i].parent=1; //INIT inherits the orphan
	for (i=NR_OPEN;i--;)
		if (current_task->files[i].fd!=NO_FILE)
			sys_close(i);
	if (!current_task->pid) {
		printf("\e[91mKernel Aborted. Halt System!\e[m\n");
		cli();
		hlt();
	}
	current_task->state=TASK_ZOMBIE;
	current_task->exit_code=status;
	sys_kill(current_task->parent,SIGCHLD);
	free_directory(current_task->directory);
	free((void *)current_task->kernel_stack);
	switch_task();
	return -EGENERIC;
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
