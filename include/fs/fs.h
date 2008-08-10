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

#ifndef _FS_H
#define _FS_H

#include <kernel.h>

#define NODE_NAME_LEN 256

#define FS_FILE		0x01
#define FS_DIRECTORY	0x02
#define FS_CHARDEVICE	0x03
#define FS_BLOCKDEVICE	0x04
#define FS_PIPE		0x05
#define FS_SYMLINK 	0x06
#define FS_MOUNTPOINT	0x08

#define FS_TYPE_FAT32	0xB
#define FS_TYPE_INITRD	0x1120
#define FS_TYPE_DEVFS	0xDEF5

#define FS_MODE_R	4
#define FS_MODE_RX	5
#define FS_MODE_RW	6
#define FS_MODE_RWX	7
#define FS_MODE_USER	6
#define FS_MODE_GROUP	3
#define FS_MODE_OTHER	0
#define FS_UID_ROOT	0
#define FS_GID_ROOT	0

#define NR_OPEN		32
#define NO_FILE		NR_OPEN+1

typedef struct _fs_node fs_node;
typedef struct _mountinfo mountinfo;

typedef void (*open_proto)(fs_node*);
typedef UINT (*read_proto)(fs_node*,off_t,size_t,UCHAR*);
typedef UINT (*write_proto)(fs_node*,off_t,size_t,UCHAR*);
typedef void (*close_proto)(fs_node*);
typedef struct dirent *(*readdir_proto)(fs_node*,UINT);
typedef fs_node *(*finddir_proto)(fs_node*,char *name);

struct dirent
{
	UINT d_ino;
	char d_name[NODE_NAME_LEN];
	USHORT d_namlen;
	UCHAR d_type;
};

typedef struct _node_operations {
	open_proto open;
	read_proto read;
	write_proto write;
	close_proto close;
	readdir_proto readdir;
	finddir_proto finddir;
} node_operations;

struct _fs_node {
	UINT mode;
	UINT uid;
	UINT gid;
	UCHAR flags;
	UINT inode;
	size_t size;
	UCHAR nlinks;
	mountinfo *mi; //impl
	node_operations *f_op;
	void *p_data;   //This is filesystem specific stuff. DO NOT ACCESS FROM "OUTSIDE"(VFS)
	fs_node *ptr;
};

//For every mountpoint there is such one

struct _mountinfo {
	UINT fs_type;
	void *discr;
	fs_node *device;
	fs_node *mountpoint;
	fs_node *root;
	mountinfo *next;
};

typedef struct file {
	UINT fd;
	USHORT mode;
	fs_node *node;
	off_t offset;
} FILE;

//VFS functions

extern void open_fs(fs_node *node, UCHAR read, UCHAR write);
extern UINT read_fs(fs_node *node, off_t offset, size_t size, UCHAR *buffer);
extern UINT write_fs(fs_node *node, off_t offset, size_t size, UCHAR *buffer);
extern void close_fs(fs_node *node);
extern struct dirent *readdir_fs(fs_node *node, UINT index);
extern fs_node *finddir_fs(fs_node *node, char *name);

extern fs_node *get_root_fs_node();

//Managment of mountpoints

extern UINT setup_vfs();
extern mountinfo *fs_add_mountpoint(UINT fs_type, void *discr, fs_node *mountpoint, fs_node *device, fs_node *root);
extern UINT fs_del_mountpoint(mountinfo *mi);
extern UINT close_vfs();
extern fs_node *resolve_node(fs_node *node);

//functions NOT in fs.c

extern fs_node *namei(char *filename);

#endif
