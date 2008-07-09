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
#include <fs/fs.h>

#define TASK_RUNNING		0
#define TASK_INTERRUPTIBLE	1
#define TASK_UNINTERRUPTIBLE	2
#define TASK_ZOMBIE		3
#define TASK_STOPPED		4

#define TASK_NOTASK		-1
#define NR_TASKS 64

typedef struct _task task;

struct _task
{
	long pid,parent;
	UINT esp,ebp;
	UINT eip;
	page_directory *directory;
	USHORT uid, gid;
	long priority,state;
	FILE files[NR_OPEN];
	fs_node *pwd;
};

extern volatile task *current_task;
extern void setup_tasking();
extern void switch_task();
extern long fork();
extern void move_stack(void *new_stack, UINT size);
extern long getpid();

#endif