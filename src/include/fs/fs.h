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

#define FS_UID_ROOT	0
#define FS_GID_ROOT	0

#define IS_REG(NODE)	((NODE->flags&0x7)==FS_FILE)
#define IS_DIR(NODE)	((NODE->flags&0x7)==FS_DIRECTORY)
#define IS_CHR(NODE)	((NODE->flags&0x7)==FS_CHARDEVICE)
#define IS_BLK(NODE)	((NODE->flags&0x7)==FS_BLOCKDEVICE)
#define IS_PIP(NODE)	((NODE->flags&0x7)==FS_PIPE)
#define IS_LNK(NODE)	((NODE->flags&0x7)==FS_SYMLINK)
#define IS_MNT(NODE)	(NODE->flags&FS_MOUNTPOINT)

#define NR_OPEN		32

#define FMODE_READ		1
#define FMODE_WRITE		2

typedef struct _fs_node fs_node;
typedef struct _mountinfo mountinfo;

typedef void (*open_proto)(fs_node*);
typedef int (*read_proto)(fs_node*,off_t,size_t,char*);
typedef int (*write_proto)(fs_node*,off_t,size_t,const char*);
typedef void (*close_proto)(fs_node*);
typedef struct dirent *(*readdir_proto)(fs_node*,UINT);
typedef fs_node *(*finddir_proto)(fs_node*,const char *);
typedef void (*free_p_data_proto)(fs_node*);
typedef int (*ioctl_proto)(fs_node*,UINT,ULONG);

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
	free_p_data_proto free_p_data;
	ioctl_proto ioctl;
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
	fs_node *parent_dir;
	fs_node *root;
	mountinfo *next;
};

typedef struct file {
	UINT fd;
	UINT flags;
	fs_node *node;
	off_t offset;
	UINT count;
} FILE;

//VFS functions

extern void open_fs(fs_node *node, UCHAR read, UCHAR write);
extern int read_fs(fs_node *node, off_t offset, size_t size, char *buffer);
extern int write_fs(fs_node *node, off_t offset, size_t size, const char *buffer);
extern void close_fs(fs_node *node);
extern struct dirent *readdir_fs(fs_node *node, UINT index);
extern fs_node *finddir_fs(fs_node *node, const char *name);
extern void free_p_data_fs(fs_node *node);
extern int ioctl_fs(fs_node *node, UINT cmd, ULONG arg);

extern fs_node *get_root_fs_node(void);

//Managment of mountpoints

extern UINT setup_vfs(void);
extern mountinfo *fs_add_mountpoint(UINT fs_type, void *discr, fs_node *mountpoint, fs_node *device, fs_node *root);
extern UINT fs_del_mountpoint(mountinfo *mi);
extern UINT close_vfs(void);
extern fs_node *resolve_node(fs_node *node);

//functions NOT in fs.c

extern fs_node *namei(const char *filename);

#endif
