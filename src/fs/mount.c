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

#include <fs/mount.h>
#include <mm.h>
#include <errno.h>
#include <task.h>

extern filesystem_t *vfs_get_fs(const char *name);
extern void free_sb_inodes(super_block *sb);

static vfsmount *vfs_mounts = 0;

int d_mount(vfsmount *mnt)
{
	if (!mnt) return -EINVAL;
	if (mnt->next) return -EBUSY;
	mnt->next=vfs_mounts;
	vfs_mounts=mnt;
	return 0;
}

int d_umount(super_block *sb)
{
	if (!sb) return 0;
	vfsmount *tmp=vfs_mounts, *prev=0;
	while (tmp) {
		if (sb->mi==tmp) break;
		prev=tmp;
		tmp=tmp->next;
	}
	if (!prev) vfs_mounts=tmp->next;
		else prev->next=tmp->next;
	free(tmp);
	return 0;
}

int sys_mount(	const char *source, const char *target,
		const char *filesystemtype, unsigned long mountflags,
		void *data)
{
	if (!I_AM_ROOT()) return -EPERM;
	//TODO: Check for EFAULT
	filesystem_t *type=vfs_get_fs(filesystemtype);
	if (!type) return -EINVAL;
	int status;
	vnode *mountpoint=namei(target,&status);
	if (!mountpoint) return status;
	vnode *dev=0;
	if (type->flags&MNT_FLAG_REQDEV) {
		dev=namei(source,&status);
		if (!dev) {
			iput(mountpoint);
			return status;
		}
		/* Device stuff goes here: open/request */
	}
	vfsmount *mnt=calloc(1,sizeof(vfsmount));
	super_block *sb=calloc(1,sizeof(super_block));
	mnt->devname=source;
	mnt->dirname=target;
	mnt->flags=mountflags;
	sb->dev=dev;
	sb->flags=mountflags;
	sb->mi=mnt;
	sb->type=type;
	sb->cache=vfs_create_cache();
	mnt->sb=type->read_super(sb,data,0);
	if (!sb) {
		free(mnt);
		free(sb);
		iput(mountpoint);
		//close Device
		iput(dev);
		return -EINVAL;
	}
	d_mount(mnt);
	mountpoint->mount=sb->root;
	sb->root->cover=mountpoint;
	return 0;
}

int sys_umount(const char *target)
{
	if (!I_AM_ROOT()) return -EPERM;
	if (!target || !*target) return -EFAULT;
	int status;
	vnode *node=namei(target,&status),*newnode;
	if (!node) return status;
	if (!IS_MNT(node)) { //Not a mountpoint
		iput(node);
		return -EINVAL;
	}
	newnode=node->mount;
	iput(node);
	super_block *sb=newnode->sb;
	if (sb->s_op->put_super)
		sb->s_op->put_super(sb);
	/*
	 * FIXME: I've to be sure, no inodes are in the cache any
	 * more, so the device isn't busy
	 */
	//Close device
	iput(sb->dev);
	free_sb_inodes(sb);
	d_umount(sb);
	free(sb);
	return 0;
}
