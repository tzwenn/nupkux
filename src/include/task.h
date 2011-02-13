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

#ifndef _TASK_H
#define _TASK_H

#include <kernel.h>
#include <paging.h>
#include <fs/vfs.h>

#define TASK_RUNNING		0
#define TASK_WAITING		1
#define TASK_UNINTERRUPTIBLE	2
#define TASK_BLOCKED		4
#define TASK_ZOMBIE		5

#define ROOT_UID	0
#define I_AM_ROOT()	(!(current_task->uid && current_task->euid))

#define NR_TASKS	64
#define NO_TASK		(-1)

#define KERNEL_STACK_SIZE 2048

typedef struct _task task;

#ifndef _PID_T
#define _PID_T
typedef int pid_t;
#endif

struct _task
{
	pid_t pid, parent, pgrp;
	char priority, state;
	UINT esp, ebp, eip;
	page_directory *directory;
	UINT kernel_stack;
	USHORT uid, euid;
	USHORT gid, egid;
	int exit_code;
	UINT signals;
	vnode *pwd, *root;
	ULONG close_on_exec;
	FILE *files[NR_OPEN];
};

extern volatile task *current_task;
extern void setup_tasking(void);
extern void switch_task(void);
extern void move_stack(void *new_stack, UINT size);
extern void abort_current_process(void);

#endif
