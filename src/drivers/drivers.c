/*
 *  Copyright (C) 2007,2008 Sven Köhler
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
#include <drivers/fdc.h>
#include <drivers/tty.h>
#include <drivers/ramdisk.h>
#include <mm.h>
#include <kernel/syscall.h>

extern void setup_pseudo_devices(void); //pseudo.c
extern void setup_serial(void); //serial.c

inline void *devicen_pdata(vnode *node)
{
	if (!node || !node->u.devfs_i) return 0;
	return node->u.devfs_i->pdata;
}

inline void *device_pdata(devfs_handle *handle)
{
	if (!handle) return 0;
	return handle->pdata;
}

inline void set_device_pdata(devfs_handle *handle, void *pdata)
{
	if (handle)
		handle->pdata=pdata;
}

inline devfs_handle *device_discr(vnode *node)
{
	if (!node) return 0;
	return  node->u.devfs_i;
}

void device_lock(vnode *node)
{
	devfs_handle *dev=device_discr(node);
	if (!dev) return;
	cli();
	request_t *newreq=malloc(sizeof(request_t)), *tmp;
	newreq->pid=current_task->pid;
	newreq->next=0;
	if (!dev->queue) dev->queue=newreq;
	else {
		tmp=dev->queue;
		while (tmp->next) tmp=tmp->next;
		tmp->next=newreq;
	}
	sti();
	while (dev->queue->pid!=current_task->pid) sys_pause();
}

void device_unlock(vnode *node)
{
	devfs_handle *dev=device_discr(node);
	if (!dev) return;
	request_t *req=dev->queue;
	if (req->pid!=current_task->pid) return;
	cli();
	dev->queue=req->next;
	free(req);
	sti();
	if (dev->queue) sys_kill(dev->queue->pid,SIGCONT);
}

inline pid_t requesting_pid(vnode *node)
{
	devfs_handle *dev=device_discr(node);
	if (!dev || !dev->queue) return -1;
	return dev->queue->pid;
}

inline void outportb(USHORT port, UCHAR value)
{
    asm volatile ("outb %%al,%%dx"::"d" (port), "a" (value));
}

inline UCHAR inportb(USHORT port)
{
 	UCHAR value;

	asm volatile ("inb %%dx,%%al":"=a" (value):"d"(port));
	return value;
}

inline void outportw(USHORT port, USHORT value)
{
    asm volatile ("outw %%ax,%%dx"::"d"(port), "a"(value));
}

inline USHORT inportw(USHORT port)
{
 	USHORT value;

	asm volatile ("inw %%dx,%%ax":"=a"(value):"d"(port));
	return value;
}

UINT setup_drivers(void)
{
	setup_pseudo_devices();
	setup_tty();
	setup_serial();
	setup_floppy();
	setup_ramdisk();
	return 0;
}
