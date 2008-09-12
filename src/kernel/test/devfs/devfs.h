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

#ifndef _DEVFS_H
#define _DEVFS_H

#include "../vfs.h"

#define DEVFS_FILENAME_LEN	16

#ifndef _DEVFS_HANDLE
#define _DEVFS_HANDLE
typedef struct _devfs_handle devfs_handle;
#endif

struct _devfs_handle {
	/* VFS */
	ULONG ino;
	UCHAR nlinks;
	USHORT uid;
	USHORT gid;
	UINT mode;
	UINT flags;
	size_t size;
	/* Driver */
	void *pdata;
	//request_t *queue;
	UINT bsize;   // Blocksize
	ULONG bcount; // Blockcount if blk-dev
	file_operations *f_op;
	/* DevFS */
	devfs_handle *cache_next, *parent;
	vnode *node;
};

typedef struct _devfs_d_entry
{
	ULONG inode;
	char filename[DEVFS_FILENAME_LEN+1];
} devfs_d_entry;

extern devfs_handle *devfs_register_device_v2(devfs_handle *dir, const char *name, UINT mode, UINT uid, UINT gid, UINT type, file_operations *f_op);
extern void devfs_unregister_device_v2(devfs_handle *device);

extern void devfs_add_to_cache(devfs_handle *handle);
extern void devfs_del_from_cache(devfs_handle *handle);
devfs_handle *devfs_iget(ULONG ino);
extern void devfs_free_cache(void);

#endif
