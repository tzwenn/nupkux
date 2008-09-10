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
#include <lib/string.h>

static int initrd_open(vnode *node, FILE2 *file)
{
	node->nlinks++;
	return 0;
}

static int initrd_close(vnode *node)
{
	node->nlinks--;
	return 0;
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
		open: &initrd_open,
		close: &initrd_close,
};

inode_operations initrd_i_ops = {
		f_ops: &initrd_f_ops,
		lookup: &initrd_lookup,
};
