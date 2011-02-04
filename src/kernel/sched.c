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

extern volatile task tasks[NR_TASKS];

volatile task* schedule(void)
{
	pid_t i;
	volatile task *new_task=current_task;
	for (i=NR_TASKS;i>=0;i--) {
		if (tasks[i].pid!=NO_TASK && tasks[i].signals && tasks[i].state==TASK_BLOCKED)
			tasks[i].state=TASK_WAITING;
	}

	for (i=new_task->pid+1;i<NR_TASKS;i++)
		if (tasks[i].pid!=NO_TASK && tasks[i].state==TASK_WAITING) break;
	if (i==NR_TASKS) {
		new_task=tasks; //This runs the kernel task, even if it's paused
	} else new_task=&(tasks[i]);
	return new_task;
}

int sys_pause(void)
{
	current_task->state=TASK_BLOCKED;
	switch_task();
	return 0;
}

