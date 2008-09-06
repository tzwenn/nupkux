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

/* This file contains stuff related to the new virtual filesystem */

#ifndef _VFS_H
#define _VFS_H

#include <time.h>

/*
 * I've decided to create an (poorly) interface resembling the Linux-VFS
 * as found at http://www.cse.unsw.edu.au/~neilb/oss/linux-commentary/vfs.htm
 */

#define DNAME_INLINE_LEN 16

typedef struct _super_operations super_operations;
typedef struct _inode_operations inode_operations;
typedef struct _file_operations file_operations;
typedef struct _vnode vnode;
typedef struct _filesystem_t filesystem_t;
typedef struct _super_block super_block;

struct _super_operations {
	void (*read_inode) (vnode *);
	void (*write_inode) (vnode *);
	void (*put_inode) (vnode *);
	void (*put_super) (super_block *);
	void (*write_super) (super_block *);
	int (*remount_fs) (super_block *, int *, char *);
};

struct _inode_operations {
	file_operations *f_ops;
	int (*create) (vnode *,const char *,int);
	vnode *(*lookup) (vnode *,const char *);
	int (*link) (vnode *,vnode *,const char *);
	int (*unlink) (vnode *,const char *);
	int (*symlink) (vnode *,const char *,const char *);
	int (*mkdir) (vnode *,const char *,int);
	int (*rmdir) (vnode *,const char *);
	int (*rename) (vnode *,const char *,vnode *,const char *);
	vnode *(*follow_link) (vnode *,const char *);

};

struct _file_operations {

};

typedef UINT device_t; // To use this I have to include devfs.h and would mix up my code with old-vfs stuff

struct _vnode {
	ULONG ino;
	UCHAR count;
	UCHAR nlinks;
	USHORT uid;
	USHORT gid;
	UINT mode;
	UINT flags;
	size_t size;
	time_t atime,mtime,ctime;
	device_t *dev;
	super_block *sb;
	inode_operations *i_op;
	union {
		void *pdata;
	} u;
	// VFS-only
	vnode *mount;
	vnode *cover;
	vnode *cache_next;
};

#ifndef _VFSMOUNT
#define _VFSMOUNT
typedef struct _vfsmount vfsmount;
#endif

struct _super_block {
	device_t *dev;
	int flags;
	filesystem_t *type;
	super_operations *s_op;
	ULONG blocksize;
	UCHAR blocksize_bits;
	vnode *root;
	union {
		void *pdata;
	} u;
	vfsmount *mi;
	vnode *cache; // VFS-only
};

struct _filesystem_t {
	const char *name;
	int flags;
	super_block *(*read_super)(super_block*,void *,int);
	filesystem_t *next;
};

extern int init_vfs(void);
extern int register_filesystem(filesystem_t *fs);
extern int unregister_filesystem(filesystem_t *fs);

extern vnode *iget(super_block *sb, ULONG ino);
extern void iput(vnode *node);

#endif
