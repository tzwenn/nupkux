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
#include <lib/memory.h>
#include <kernel/dts.h>
#include <fs/fs.h>
#include <errno.h>
#include <kernel/syscall.h>
#include <kernel/ktextio.h>

volatile task *current_task = 0;
volatile task tasks[NR_TASKS];

extern page_directory *kernel_directory; //paging.c
extern UINT _kmalloc_a(UINT sz); //mm.c

extern volatile task* schedule(void);	//sched.c
extern UINT read_eip(void);		//process.S
extern UINT initial_esp;		//main.c

void move_stack(void *new_stack, UINT size)
{
	UINT i,old_esp,old_ebp,new_esp,new_ebp,tmp,offset;

	for (i=(UINT)new_stack;i>=((UINT)new_stack-size);i-=FRAME_SIZE)
		make_page(i,PAGE_FLAG_PRESENT | PAGE_FLAG_WRITE | PAGE_FLAG_USERMODE,current_directory,1);
	flush_tlb();
	asm volatile (	"movl %%esp,%0\n\t"
					"movl %%ebp,%1\n\t":"=r"(old_esp),"=r"(old_ebp));
	offset=(UINT)new_stack-initial_esp;
	new_esp=old_esp+offset;
	new_ebp=old_ebp+offset;
	memcpy((void *)new_esp,(void*)old_esp,initial_esp-old_esp);
	for (i=(UINT)new_stack;i>(UINT)new_stack-size;i-=sizeof(UINT)) {
		tmp=*(UINT *)i;
		if ((old_esp<tmp) && (tmp<initial_esp))
			*((UINT *)i)=tmp+offset;
	}
	asm volatile (	"movl %0,%%esp\n\t"
					"movl %1,%%ebp\n\t"::"r"(new_esp),"r"(new_ebp));
}

void setup_tasking()
{
	cli();
	move_stack((void*)0xE0000000, 0x2000);
	UINT i;
	for (i=1;i<NR_TASKS;i++)
		tasks[i].pid=NO_TASK;
	current_task=tasks;
	current_task->pid=0;
	current_task->parent=0;
	current_task->esp=current_task->ebp=0;
	current_task->eip=0;
	current_task->state=TASK_RUNNING;
	current_task->directory=current_directory;
	current_task->gid=FS_GID_ROOT; //root runs it
	current_task->uid=FS_UID_ROOT;
	current_task->pwd=0;
	current_task->root=0;//get_root_fs_node();
	current_task->signals=0;
	for (i=NR_OPEN;i--;)
		current_task->files[i].fd=NO_FILE;
	current_task->kernel_stack=_kmalloc_a(KERNEL_STACK_SIZE);
	sti();
}

void switch_task()
{
	if (!current_task) return;
	cli();
	UINT esp,ebp,eip;
	asm volatile (	"movl %%esp,%0\n\t"
			"movl %%ebp,%1\n\t":"=r"(esp),"=r"(ebp));
	if ((eip=read_eip())==0x2DF) {
		sti();
		return; //Just switched
	}
	current_task->eip=eip;
	current_task->esp=esp;
	current_task->ebp=ebp;
	if (current_task->state==TASK_RUNNING)
		current_task->state=TASK_WAITING;
	current_task=schedule();
	current_task->state=TASK_RUNNING;
	eip=current_task->eip;
	esp=current_task->esp;
	ebp=current_task->ebp;
	current_directory=current_task->directory;
	set_kernel_stack(current_task->kernel_stack+KERNEL_STACK_SIZE);
	asm volatile(	"movl %0,%%ecx\n\t"
			"movl %1,%%esp\n\t"
			"movl %2,%%ebp\n\t"
			"movl %3,%%cr3\n\t"
			"movl $0x2DF,%%eax\n\t"
			"jmp *%%ecx\n\t"::"r"(eip),"r"(esp),"r"(ebp),"r"(current_directory->physPos));
}

pid_t sys_fork()
{
	cli();
	task *parent_task=(task *)current_task;
	pid_t i;
	for (i=0;i<NR_TASKS;i++)
		if (tasks[i].pid==NO_TASK) break;
	if (i==NR_TASKS) return -1;
	page_directory *directory=clone_directory(current_directory);
	volatile task *newtask=(&(tasks[i]));
	newtask->pid=i;
	newtask->esp=0;
	newtask->ebp=0;
	newtask->eip=0;
	newtask->exit_code=0;
	newtask->state=TASK_WAITING;
	newtask->parent=parent_task->pid;
	newtask->gid=parent_task->gid;
	newtask->uid=parent_task->uid;
	newtask->directory=directory;
	newtask->pwd=parent_task->pwd;
	newtask->root=parent_task->root;
	newtask->signals=0;
	memcpy(&newtask->files,&parent_task->files,NR_OPEN*sizeof(FILE));
	current_task->kernel_stack=_kmalloc_a(KERNEL_STACK_SIZE);
	UINT eip=read_eip();
	if (current_task==parent_task) {
		UINT esp,ebp;
		asm volatile (	"movl %%esp,%0\n\t"
				"movl %%ebp,%1\n\t":"=r"(esp),"=r"(ebp));
		newtask->esp=esp;
		newtask->ebp=ebp;
		newtask->eip=eip;
		sti();
		return newtask->pid;
	} else {
		sti();
		return 0;
	}
}

pid_t sys_getpid()
{
	return current_task->pid;
}

pid_t sys_getppid()
{
	return current_task->parent;
}

void abort_current_process()
{
	sys_exit(0);
}

int set_task_state(pid_t pid, char state)
{
	if (pid<0 || pid>=NR_TASKS) return -EGENERIC;
	volatile task *atask=&(tasks[pid]);
	if (atask->pid==NO_TASK) return -EGENERIC;
	atask->state=state;
	return 0;
}
