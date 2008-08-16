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

#ifndef _DEVFS_H
#define _DEVFS_H

#include <kernel.h>
#include <fs/fs.h>

#define DEVFS_INODE_FREE	0xFFFFFFFF
#define DEVFS_INODE_LINK	0xFFFFFFFE
#define DEVFS_FILENAME_LEN	16
#define DEVFS_INODES_PER_BLOCK	32

typedef struct _devfs_d_entry
{
	UINT inode;
	char filename[DEVFS_FILENAME_LEN];
} devfs_d_entry;

typedef struct _devfs_discr
{
	fs_node *nodes;  //Here is something special: just a linked list
	fs_node *root;
} devfs_discr;

extern fs_node *setup_devfs(fs_node *mountpoint);
//TODO A seperated mount may be achieved
extern UINT remove_devfs(fs_node *devfs);

extern fs_node *devfs_register_device(fs_node *dir, const char *name, UINT mode, UINT uid, UINT gid, UINT type, node_operations *f_op);
extern void devfs_unregister_device(fs_node *device);

#endif
