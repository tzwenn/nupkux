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

#ifndef _DRIVERS_H
#define _DRIVERS_H

#include <fs/devfs.h>
#include <task.h>
#include <kernel/syscall.h>
#include <signal.h>

typedef struct _request request_t;

struct _request {
	pid_t pid;
	request_t *next;
};

typedef struct _device {
	void *pdata;
	fs_node *node;
	request_t *queue;
	UINT bsize;   // Blocksize
	ULONG bcount; // Blockcount if blk-dev
} device_t;

#define REQUEST_READ	0x2EAD
#define REQUEST_WRITE	0x217E

extern UINT setup_drivers(fs_node *devfs);

extern inline void *device_pdata(fs_node *node);
extern inline void set_device_pdata(fs_node *node, void *pdata);
extern inline device_t *device_discr(fs_node *node);
extern void device_lock(fs_node *node);
extern void device_unlock(fs_node *node);
extern inline pid_t requesting_pid(fs_node *node);
extern inline void outportb(USHORT port, UCHAR value);
extern inline UCHAR inportb(USHORT port);
extern inline void outportw(USHORT port, USHORT value);
extern inline USHORT inportw(USHORT port);

#endif
