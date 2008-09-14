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

#ifndef _INITRDFS_H
#define _INITRDFS_H

#include <fs/vfs.h>

#define INITRD_FILENAME_LEN	64
#define INITRD_MAGIC		0x54494E49

#ifndef _INITRD_INODE
#define _INITRD_INODE
typedef struct _initrd_inode initrd_inode;
#endif

typedef struct _initrd_header
{
	UINT magic;
	UINT inodecount;
	UINT entries;
} initrd_header;


struct _initrd_inode
{
	UINT inode;
	UINT offset;
	UINT mode;
	UINT gid;
	UINT uid;
	UCHAR flags;
	UINT size;
};

typedef struct _initrd_d_entry
{
	UINT inode;
	char filename[INITRD_FILENAME_LEN];

} initrd_d_entry;

typedef struct _initrd_discr {
	char *location;
	initrd_header *initrdheader;
	initrd_inode *initrd_inodes;
} initrd_discr;

#endif
