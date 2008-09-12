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

#include "initrdfs.h"
#include <errno.h>

static int initrd_read(vnode *node, off_t offset, size_t size, char *buffer)
{
	initrd_discr *discr = (initrd_discr *) node->sb->u.pdata;
	if (node->ino>discr->initrdheader->inodecount) return 0;
	initrd_inode inode = discr->initrd_inodes[node->ino];
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

static vnode *initrd_lookup(vnode *dir,const char *name)
{
	initrd_discr *discr = (initrd_discr *) dir->sb->u.pdata;
	if (dir->ino>discr->initrdheader->inodecount) return 0;
	initrd_inode inode=discr->initrd_inodes[dir->ino];
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
};

inode_operations initrd_i_ops = {
		f_op: &initrd_f_ops,
		lookup: &initrd_lookup,
};
