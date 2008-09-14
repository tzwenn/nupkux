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
#include <mm.h>

/*
 * I've decided to create an (poor) interface resembling the Linux-VFS
 * as found at http://www.cse.unsw.edu.au/~neilb/oss/linux-commentary/vfs.htm
 */

#define MNT_FLAG_REQDEV	0x01

#define FS_FILE		0x01
#define FS_DIRECTORY	0x02
#define FS_CHARDEVICE	0x03
#define FS_BLOCKDEVICE	0x04
#define FS_PIPE		0x05
#define FS_SYMLINK 	0x06
#define FS_MOUNTPOINT	0x08

#define IS_REG(NODE)	((NODE->flags&0x7)==FS_FILE)
#define IS_DIR(NODE)	((NODE->flags&0x7)==FS_DIRECTORY)
#define IS_CHR(NODE)	((NODE->flags&0x7)==FS_CHARDEVICE)
#define IS_BLK(NODE)	((NODE->flags&0x7)==FS_BLOCKDEVICE)
#define IS_PIP(NODE)	((NODE->flags&0x7)==FS_PIPE)
#define IS_LNK(NODE)	((NODE->flags&0x7)==FS_SYMLINK)
#define IS_MNT(NODE)	(NODE->mount)
#define IS_MNTED(NODE)	(NODE->cover)

typedef struct _super_operations super_operations;
typedef struct _inode_operations inode_operations;
typedef struct _file_operations file_operations;
typedef struct _vnode vnode;
typedef struct _filesystem_t filesystem_t;
typedef struct _super_block super_block;

#ifndef _INITRD_INODE
#define _INITRD_INODE
typedef struct _initrd_inode initrd_inode;
#endif

#ifndef _DEVFS_HANDLE
#define _DEVFS_HANDLE
typedef struct _devfs_handle devfs_handle;
#endif

#ifndef _VFSMOUNT
#define _VFSMOUNT
typedef struct _vfsmount vfsmount;
#endif

#define FS_UID_ROOT     0
#define FS_GID_ROOT     0

#define NR_OPEN         32

#define FMODE_READ              1
#define FMODE_WRITE             2

struct dirent {
	ULONG d_ino;
	char d_name[255];
	int d_namlen;
	UINT d_type;
};

typedef struct file {
        UINT fd;
        UINT flags;
        vnode *node;
        off_t offset;
        UINT count;
} FILE;

struct _super_operations {
	void (*read_inode) (vnode *);
	void (*write_inode) (vnode *);
	void (*put_inode) (vnode *);
	void (*put_super) (super_block *);
	void (*write_super) (super_block *);
	int (*remount_fs) (super_block *,int *,char *);
};

struct _inode_operations {
	file_operations *f_op;
	int (*create) (vnode *,const char *,int);
	vnode *(*lookup) (vnode *,const char *);
	int (*link) (vnode *,vnode *,const char *);
	int (*unlink) (vnode *,const char *);
	int (*symlink) (vnode *,const char *,const char *);
	int (*mkdir) (vnode *,const char *,int);
	int (*rmdir) (vnode *,const char *);
	int (*rename) (vnode *,const char *,vnode *,const char *);
	vnode *(*follow_link) (vnode *,const char *); //Maybe we should use resolve_link

};

struct _file_operations {
	int (*open) (vnode *,FILE *);
	int (*read) (vnode *,off_t,size_t,char *);
	int (*write) (vnode *,off_t,size_t,const char *);
	int (*readdir) (vnode *, off_t, struct dirent *);
	int (*close) (vnode *);
	int (*ioctl) (vnode *,UINT,ULONG);
	int (*request) (vnode *,int,ULONG,ULONG,char *);
	void (*free_pdata) (void *);
};

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
	vnode *dev;
	super_block *sb;
	inode_operations *i_op;
	union {
		devfs_handle *devfs_i;
		initrd_inode *initrdfs_i;
		void *pdata;
	} u;
	// VFS-only
	vnode *mount;
	vnode *cover;
	vnode *cache_next;
};

struct _super_block {
	vnode *dev;
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

extern int setup_vfs(void);
extern int register_filesystem(filesystem_t *fs);
extern int unregister_filesystem(filesystem_t *fs);
extern int close_vfs(void);

extern int namei_match(const char *s1, const char *s2);
extern vnode *namei(const char *filename, int *status);

extern vnode *iget(super_block *sb, ULONG ino);
extern void iput(vnode *node);

extern int open_fs(vnode *node, FILE *f);
extern int read_fs(vnode *node, off_t offset, size_t size, char *buffer);
extern int write_fs(vnode *node, off_t offset, size_t size, const char *buffer);
extern int close_fs(vnode *node);
extern int readdir_fs(vnode *node, off_t index, struct dirent *buf);
extern int ioctl_fs(vnode *node, UINT cmd, ULONG arg);


#endif
