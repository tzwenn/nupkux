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
#include <mm.h>
#include <kernel/syscall.h>

extern void setup_pseudo_devices(fs_node *devfs); //pseudo.c
extern void setup_serial(fs_node *); //serial.c

inline void *device_pdata(fs_node *node)
{
	if (!node || !node->pdata) return 0;
	return ((device_t *)node->pdata)->pdata;
}

inline void set_device_pdata(fs_node *node, void *pdata)
{
	if (node && node->pdata)
		((device_t *)node->pdata)->pdata=pdata;
}

inline device_t *device_discr(fs_node *node)
{
	if (!node) return 0;
	return (device_t *)node->pdata;
}

void device_lock(fs_node *node)
{
	if (!node || !node->pdata) return;
	cli();
	device_t *dev=(device_t *) node->pdata;
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

void device_unlock(fs_node *node)
{
	if (!node || !node->pdata) return;
	device_t *dev=(device_t *) node->pdata;
	request_t *req=dev->queue;
	if (req->pid!=current_task->pid) return;
	cli();
	dev->queue=req->next;
	free(req);
	sti();
	if (dev->queue) sys_kill(dev->queue->pid,SIGCONT);
}

inline pid_t requesting_pid(fs_node *node)
{
	device_t *dev=device_discr(node);
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

UINT setup_drivers(fs_node *devfs)
{
	if (!devfs) return 2;

	setup_pseudo_devices(devfs);
	setup_tty(devfs);
	setup_serial(devfs);
	setup_floppy(devfs);
	return 0;
}
