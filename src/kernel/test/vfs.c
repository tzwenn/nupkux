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

#include "vfs.h"
#include "mount.h"
#include <lib/string.h>
#include <errno.h>

static filesystem_t *filesystems = 0;
/*static */vnode *root_vnode = 0;

filesystem_t *vfs_get_fs(const char *name)
{
	filesystem_t *fs=filesystems;

	if (!name) return 0;
	while (fs) {
		if (!strcmp(fs->name,name)) break;
		fs=fs->next;
	}
	return fs;
}

int register_filesystem(filesystem_t *fs)
{
	if (!fs) return -EINVAL;
	if (fs->next || vfs_get_fs(fs->name)) return -EBUSY;
	fs->next=filesystems;
	filesystems=fs;
	return 0;
}

int unregister_filesystem(filesystem_t *fs)
{
	filesystem_t *prev=0, *tmp=filesystems;

	if (!fs || !fs->name) return -EINVAL;
	while (tmp) {
		if (!strcmp(fs->name,tmp->name)) break;
		prev=tmp;
		tmp=tmp->next;
	}
	if (!tmp) return -EINVAL;
	if (!prev) filesystems=filesystems->next;
		else {
			prev->next=tmp->next;
			tmp->next=0;
		}
	fs->next=0;
	return 0;
}

/* This defines a very simple rootfs, where other systems can be mounted on ("/") */

static void rootfs_read_inode(vnode *node)
{
	// We need nothing, just a vnode to mount on => directory
	node->flags=FS_DIRECTORY;
	node->mode=0777;
}

static super_operations rootfs_sb_ops = {
		read_inode: &rootfs_read_inode,
};

static super_block *read_rootfs_sb(super_block *sb, void *data, int verbose)
{
	sb->s_op=&rootfs_sb_ops;
	sb->blocksize=1;
	sb->blocksize_bits=0;
	sb->root=iget(sb,0);
	return sb;
}

static filesystem_t rootfs_type = {
		name: "rootfs",
		flags: 0,
		read_super: &read_rootfs_sb,
		next: 0
};

extern int d_mount(vfsmount *mnt);
extern int d_umount(super_block *sb);
extern void free_sb_inodes(super_block *sb);

int setup_vfs_v2(void)
{
	register_filesystem(&rootfs_type);
	vfsmount *mnt=calloc(1,sizeof(vfsmount));
	super_block *sb=calloc(1,sizeof(super_block));
	sb->mi=mnt;
	sb->type=&rootfs_type;
	mnt->sb=read_rootfs_sb(sb,0,0);
	root_vnode=sb->root;
	d_mount(mnt);
	return 0;
}

int close_vfs_v2(void)
{
	if (!root_vnode) return -1;
	super_block *sb=root_vnode->sb;
	free_sb_inodes(sb);
	d_umount(sb);
	free(sb);
	unregister_filesystem(&rootfs_type);
	root_vnode=0;
	return 0;
}
