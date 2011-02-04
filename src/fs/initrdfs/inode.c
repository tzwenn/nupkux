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

#include <fs/initrdfs.h>
#include <errno.h>
#include <lib/string.h>

static int initrd_read(vnode *node, off_t offset, size_t size, char *buffer)
{
	initrd_discr *discr = (initrd_discr *) node->sb->u.pdata;
	initrd_inode inode = *(node->u.initrdfs_i);
	if (offset>inode.size)
		return 0;
	if (offset+size>inode.size)
		size=inode.size-offset;
	memcpy(buffer,(UCHAR*)(inode.offset+offset+discr->location),size);
	return size;
}

static int initrd_write(vnode *node, off_t offset, size_t size, const char *buffer)
{
	return -EINVAL;
}

static int initrd_readdir(vnode *dir, off_t index, struct dirent *buf)
{
	initrd_discr *discr = (initrd_discr *) dir->sb->u.pdata;
	initrd_inode inode = *(dir->u.initrdfs_i);
	initrd_d_entry *entries = (initrd_d_entry *) (inode.offset+discr->location);

	if (index>=inode.size/sizeof(initrd_d_entry)) return -EINVAL;
	inode=discr->initrd_inodes[entries[index].inode];
	strncpy(buf->d_name,entries[index].filename,INITRD_FILENAME_LEN);
	buf->d_name[INITRD_FILENAME_LEN-1]=0;
	buf->d_namlen=strlen(buf->d_name);
	buf->d_ino=entries[index].inode;
	buf->d_type=inode.flags;
	return 0;
}

static vnode *initrd_lookup(vnode *dir,const char *name)
{
	initrd_discr *discr = (initrd_discr *) dir->sb->u.pdata;
	initrd_inode inode = *(dir->u.initrdfs_i);
	initrd_d_entry *entries = (initrd_d_entry *) (inode.offset+discr->location);
	UINT i = inode.size/sizeof(initrd_d_entry);
	while (i--) {
		if (namei_match(name,entries[i].filename))
			return iget(dir->sb,entries[i].inode);
	}
	return 0;
}

static file_operations initrd_f_ops = {
		read: &initrd_read,
		write: &initrd_write,
		readdir: &initrd_readdir,
};

inode_operations initrd_i_ops = {
		f_op: &initrd_f_ops,
		lookup: &initrd_lookup,
};
