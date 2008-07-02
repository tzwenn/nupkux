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

#ifndef _INITRD_H
#define _INITRD_H

#include <kernel.h>
#include <fs/fs.h>

#define INITRD_FILENAME_LEN	64

typedef struct _initrd_header
{
	UINT inodecount;
	UINT entries;
} initrd_header;


typedef struct _initrd_inode
{
	UINT inode;
	UINT offset;
	UINT mode;
	UINT gid;
	UINT uid;
	UCHAR flags;
	UINT size;
} initrd_inode;

typedef struct _initrd_d_entry
{
	UINT inode;
	char filename[INITRD_FILENAME_LEN];
	
} initrd_d_entry;

typedef struct _initrd_discr
{
	UINT location;
	initrd_header *initrdheader;
	initrd_inode *initrd_inodes;
	UINT initrd_inode_count;
	fs_node *nodes, *root;
} initrd_discr;

extern fs_node *setup_initrd(UINT location, fs_node *mountpoint);
extern UINT remove_initrd(fs_node *node);

#endif
