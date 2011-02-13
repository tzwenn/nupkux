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
#include <mm.h>

extern inode_operations initrd_i_ops;

static super_block *read_initrd_sb(super_block *sb, void *data, int verbose);

filesystem_t initrd_fs_type = {
name: "initrdfs"
	,
	flags: 0,
read_super:
	&read_initrd_sb,
next:
	NULL
};

static void initrd_read_inode(vnode *node) //dev, sb, ino set
{
	initrd_discr *discr = (initrd_discr *) node->sb->u.pdata;
	if (node->ino > discr->initrdheader->inodecount) return;
	initrd_inode itnode = discr->initrd_inodes[node->ino];
	node->flags = itnode.flags;
	node->ctime = node->atime = node->mtime = 0;
	node->gid = itnode.gid;
	node->uid = itnode.uid;
	node->mode = itnode.mode;
	node->nlinks = 1;
	node->size = itnode.size;
	node->i_op = &initrd_i_ops;
	node->u.initrdfs_i = &(discr->initrd_inodes[itnode.inode]);
}

static void initrd_put_super(super_block *sb)
{
	free(sb->u.pdata);
}

static super_operations initrd_s_ops = {
read_inode:
	&initrd_read_inode,
put_super:
	&initrd_put_super,
};

static super_block *read_initrd_sb(super_block *sb, void *data, int verbose)
{
	if (*(UINT*)data != INITRD_MAGIC) return 0;
	sb->s_op = &initrd_s_ops;
	sb->blocksize = 1;
	sb->blocksize_bits = 0;
	initrd_discr *discr = malloc(sizeof(initrd_discr));
	discr->location = (char *)data;
	discr->initrdheader = (initrd_header *)discr->location;
	discr->initrd_inodes = (initrd_inode *)(discr->location + sizeof(initrd_header));
	sb->u.pdata = discr;
	sb->root = iget(sb, 0);
	return sb;
}
